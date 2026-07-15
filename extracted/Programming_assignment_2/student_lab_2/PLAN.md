# Lab 2: Buffer Manager —— 开发计划

> 目标：在**唯一可写文件** `src/buffer/buffer_manager.cpp` 中实现一个线程安全、采用 2Q 替换策略的缓冲池管理器，**一次写对**，通过 test.cpp 的 10 个测试以及 Gradescope 隐藏测试。所有 `src/include/**` 头文件只读，是必须满足的契约。

---

## 0. 契约速查（来自只读头文件，不可改）

| 元素 | 关键约束 |
|---|---|
| `Page` | 4096 字节，`page_data` 是 `unique_ptr<char[]>` |
| `BufferFrame` | 字段：`page_id_`、`frame_id_`、`dirty_(bool)`、`exclusive_(atomic<bool>)`、`page(unique_ptr<Page>)`。`BufferManager`/`PageGuard` 是友元。需实现 3 参构造 `BufferFrame(pid,fid,page)` 与 `getPageId/getFrameId/isDirty` |
| `BufferManager` | `MAX_PAGES=1000`、`MAX_PAGES_IN_MEMORY=10`。数据结构区"DO NOT MODIFY"：`buffer_pool_`(vector<unique_ptr<BufferFrame>>)、`page_to_frame_`(map)、`use_counters_`(array<atomic<int16_t>,10>，**按 FrameID 索引**)、`pinned_pages_`(set<PageID>)、`pinned_mutex_`、`frame_lock_table_`、`policy_`、`capacity_`、`page_counter_`、`buffer_mutex_`(粗粒度锁) |
| `PageGuard` | 私有成员仅 `bm_/frame_/dirty_`。构造签名固定为 `(BufferManager*, BufferFrame*)`，**无 exclusive 参数** → 解锁模式只能从 `frame_->exclusive_` 读取。仅移动不可拷贝 |
| `FrameLockTable` | 提供 `lock_shared/unlock_shared/lock_exclusive/unlock_exclusive(FrameID)`。**autograder 统计这些调用次数**（Test 10：`shared_invocations_total() >= reads`、`exclusive >= writes`）。禁止用一把大锁替代它 |
| `StorageManager` | `load(pid)`(内部断言 `pid < num_pages`)、`flush(pid,page)`、`extend(till_pid)`(扩到 `till_pid+1` 页)。自带 `io_mutex`，I/O 自身线程安全 |
| `Policy/TwoQPolicy` | 需实现 `touch/evict/select_victim/get_fifo_list/get_lru_list`。`TwoQPolicy` 自带 `fifo_queue_`、`lru_queue_`(vector<PageID>)、`mutex_` |
| 异常 | 缓冲满且全部 pinned → 抛 `buffer_full_error`；`select_victim` 无候选返回 `INVALID_VALUE`(=65535) |
| 编译 | `-std=c++17 -Wall -Werror -Wextra -pthread`，**warning 即 error** |

---

## 1. 整体架构与锁分层（核心设计决策）

三层锁，职责严格分离——这是整个 lab 的正确性基石：

1. **`buffer_mutex_`（粗粒度锁，单一元数据锁）**：保护**所有**缓冲池元数据——`page_to_frame_`、`policy_`、`use_counters_`、`pinned_pages_`、空闲帧查找、驱逐决策。题目明确允许"用粗锁保护类级变量"。
   - 决策：**只用 `buffer_mutex_` 一把粗锁管理全部元数据**，不混用 `pinned_mutex_`，避免双锁顺序导致的死锁。`pinned_mutex_` 留作未使用（合法）。
2. **`frame_lock_table_`（细粒度帧锁）**：保护**用户对页数据的并发读写**。读 → `lock_shared`，写 → `lock_exclusive`。这是 autograder 真正检查的部分。
3. **`StorageManager::io_mutex`（已实现）**：保护磁盘 I/O。

### 1.1 黄金法则（防死锁 + 防数据竞争）

- **R1（pin-then-lock）**：在 `fix_page` 中，先在 `buffer_mutex_` 下完成"增引用计数 + 登记 pinned + 更新 map/policy"，**释放 `buffer_mutex_` 之后**再去 `frame_lock_table_` 加帧锁。绝不能持有 `buffer_mutex_` 去抢帧锁（否则与 unfix 形成环路死锁）。
- **R2（unlock-before-bookkeeping）**：在 `unfix_page` 中，**先释放帧锁，再拿 `buffer_mutex_`** 做减引用计数/移除 pinned。这样 unfix 永不"持帧锁等粗锁"，与驱逐路径"持粗锁"不构成环。
- **R3（计数与 pinned 集合原子一致）**：`use_counters_[fid]` 的增/减 与 `pinned_pages_` 的 insert/erase **必须在同一个 `buffer_mutex_` 临界区内**完成，保证不变式：`pinned_pages_` 含 P ⟺ `use_counters_[frame(P)] > 0`。
- **R4（先读后减）**：unfix 中必须在**减引用计数之前**把 `frame.exclusive_` 存进局部变量。因为一旦计数归零，帧可能被别的线程驱逐复用、`exclusive_` 被改写，导致解错锁。（骨架 HINT 已强调）
- **R5（dirty 标志在解锁前置位）**：unfix 中 `if(dirty) frame.dirty_=true` 必须在仍持帧锁时完成，保证驱逐线程看到的 dirty 是最新的。

### 1.2 为什么 `frame.exclusive_` 方案是安全的

`exclusive_` 只在"持有该帧锁"时被写：共享持有者全写 `false`（同值无害），排他持有者唯一且写 `true`。锁模式的切换（shared↔exclusive）本身被 `shared_mutex` 串行化，因此任一持有者在 unfix 时读到的 `exclusive_` 总与它自己的加锁模式一致。配合 R4，跨"计数归零→帧复用"边界也安全。

---

## 2. 分组件详细设计

### 2.1 `TwoQPolicy`（先做，纯单线程逻辑，最易验证）

所有方法在 `mutex_` 下操作。队列语义：**vector 尾部 = 最新（MRU）/最近插入，头部 = 最旧（最先被驱逐）**。

```
touch(pid):                       // 每次 fix 都调用（命中与未命中都调）
  在 fifo 中? → 从 fifo 删除, push_back 到 lru          // 第二次访问：晋升
  否则在 lru 中? → 从 lru 删除, push_back 到 lru        // 后续访问：刷新到 MRU
  否则 → push_back 到 fifo                              // 首次访问：进 FIFO

evict(pid):                       // 仅由 BufferManager 在驱逐时调用
  从 fifo 删除（若在）否则从 lru 删除

select_victim(pinned):            // 只读，不改队列
  顺序遍历 fifo（头→尾）：第一个 ∉ pinned 的返回      // 优先 FIFO，最旧优先
  再顺序遍历 lru（头→尾）：第一个 ∉ pinned 的返回      // 再 LRU，最久未用优先
  都没有 → 返回 INVALID_VALUE

get_fifo_list()/get_lru_list(): 在 mutex_ 下返回队列副本
```

**测试可追溯性**：Test1(首次→FIFO，再访问→LRU)、Test3(fifo=[1..10]驱逐头部 1→[2..11])、Test5(fix2 两次→晋升)、Test6(LRU 内再访问→移到尾 [2,1])，逐条吻合。

### 2.2 `BufferFrame`

- 3 参构造：初始化 `page_id_/frame_id_`、`dirty_=false`、`page=move(p)`、`exclusive_=false`。
- `getPageId/getFrameId` 返回对应字段；`isDirty` 返回 `dirty_`（去掉当前的 `throw`）。
- `exclusive_` 是 atomic，构造里用初始化列表或 `.store(false)`（注意 atomic 不可拷贝，构造函数 body 内赋值）。

### 2.3 `BufferManager` 构造/析构

- 构造：`buffer_pool_.reserve(capacity_)`；循环创建 10 个空帧，`frame_id_=i`、`page_id_=INVALID_VALUE`、`page=nullptr`（按需在 load 时填充）。作为友元可直接设私有字段。`use_counters_` 默认 0。
- 析构：遍历 `buffer_pool_`，对 `page_id_!=INVALID && dirty_` 的帧 `flush` 落盘（复用 `flushPage`）。**Test2 PersistentRestart 依赖此处**：内存中残留的脏页必须在析构时落盘，否则重启后读不到。

### 2.4 `flushPage(FrameID fid)`

```
frame = buffer_pool_[fid].get()
if frame valid && frame->dirty_:
    storage_manager_.flush(frame->page_id_, *frame->page)
    frame->dirty_ = false
```

### 2.5 `fix_page(page_id, exclusive)` —— 核心

```
// 步骤 0：保证磁盘覆盖该页（在粗锁外，extend 自带 io_mutex）
if page_id >= storage_manager_.num_pages:
    storage_manager_.extend(page_id)

FrameID fid;
{
  unique_lock lk(buffer_mutex_);

  // ---- 命中 ----
  if page_to_frame_ 含 page_id:
      fid = page_to_frame_[page_id]
      policy_->touch(page_id)
      use_counters_[fid]++            // R3：与 pinned 同临界区
      pinned_pages_.insert(page_id)
      // 落到下方"加帧锁"
  else {
      // ---- 未命中：取一个空闲帧 ----
      if page_to_frame_.size() < capacity_:
          fid = 扫描 buffer_pool_ 找 page_id_==INVALID 的帧
      else:
          PageID victim = policy_->select_victim(pinned_pages_)
          if victim == INVALID_VALUE: throw buffer_full_error();   // lk 析构自动解锁
          fid = page_to_frame_[victim]
          if buffer_pool_[fid]->dirty_:
              flushPage(fid)          // 脏页落盘（victim 未 pinned 且粗锁保护，无人会再 pin 它）
          policy_->evict(victim)
          page_to_frame_.erase(victim)
          buffer_pool_[fid]->page_id_ = INVALID; buffer_pool_[fid]->dirty_ = false

      // ---- 装入新页 ----
      auto pg = storage_manager_.load(page_id)
      BufferFrame* f = buffer_pool_[fid].get()
      f->page = move(pg); f->page_id_ = page_id; f->frame_id_ = fid; f->dirty_ = false
      page_to_frame_[page_id] = fid
      policy_->touch(page_id)
      use_counters_[fid]++
      pinned_pages_.insert(page_id)
  }
}  // 释放 buffer_mutex_（R1）

// ---- 加细粒度帧锁（粗锁已释放）----
BufferFrame* f = buffer_pool_[fid].get()
if exclusive: frame_lock_table_.lock_exclusive(fid) else frame_lock_table_.lock_shared(fid)
f->exclusive_.store(exclusive)        // 持帧锁时写，安全
return PageGuard(this, f)
```

**设计抉择 —— I/O 是否持粗锁**：基线版本在 `buffer_mutex_` 下执行 `load`/`flush`。10 个帧、单页 4KB、本地文件，I/O 仅微秒级，多线程测试无超时风险；换来的是**零竞争、可证明正确**。"锁应短持有"主要指帧锁与真正的并发读写，这点我们用细粒度帧锁满足。释放粗锁做 I/O 的优化（用 per-frame "loading" 状态位）列为**可选进阶**，通过测试不需要。

### 2.6 `unfix_page(frame, dirty)`（PageGuard 析构调用）

```
if dirty: frame.dirty_ = true                 // R5：仍持帧锁
bool was_excl = frame.exclusive_.load()        // R4：减计数前先读
FrameID fid = frame.frame_id_
PageID  pid = frame.page_id_

// 先解帧锁（R2）
if was_excl: frame_lock_table_.unlock_exclusive(fid)
else:        frame_lock_table_.unlock_shared(fid)

// 再做计数簿记（R3 同临界区）
{
  lock_guard lk(buffer_mutex_)
  if (--use_counters_[fid] == 0)
      pinned_pages_.erase(pid)
}
```

> 注意与骨架 HINT 的"减计数→解锁"顺序相反：我们采用"**解锁→减计数**"以满足 R2 防死锁。两者对单线程等价；多线程下本顺序可证明无环（unfix 永不持帧锁等粗锁），且因为减到 0 之前页仍 pinned、仍在 `pinned_pages_`，不会被提前选为 victim，故无竞争。

### 2.7 `PageGuard`

```
ctor(bm,frame): bm_=bm; frame_=frame; dirty_=false
dtor:           if (bm_ && frame_) bm_->unfix_page(*frame_, dirty_)
move ctor:      偷 bm_/frame_/dirty_; 把 other.bm_=nullptr, other.frame_=nullptr
move assign:    if(this!=&other){ if(bm_&&frame_) bm_->unfix_page(*frame_,dirty_);
                  偷 other; other.bm_=other.frame_=nullptr } return *this
```
默认构造的 guard（`bm_=frame_=nullptr`）析构是 no-op，移走后的源对象也是 no-op —— 保证单一所有权、无重复 unfix、异常安全。

### 2.8 `load_and_pin_page`

骨架声明了它，但 `fix_page` 已可自洽实现。计划：把未命中路径"取帧+装入+pin+加锁+返回 guard"抽到此私有 helper，`fix_page` 命中走快路径、未命中调它，减少重复（DRY）。若抽取增加复杂度则内联，保留函数体抛 `method_not_implemented` 不可接受（会被隐藏测试或链接使用？）——它非纯虚、非测试直接调用，可安全留空实现或复用。**决策：实现它并由 fix_page 调用**，保持单一未命中逻辑。

---

## 3. 边界与异常清单（自检）

- [ ] 空库首次 `fix_page(0,...)`：extend(0) 后 load 读到全 0。✓ Test1
- [ ] 缓冲恰好满（10 帧）再取新页：从 FIFO 头驱逐。✓ Test3
- [ ] 全部 pinned 再取新页：`select_victim`→INVALID→抛 `buffer_full_error`，**抛出前不得持有任何帧锁**（粗锁由 lock_guard 自动释放）。✓ Test4/Test9
- [ ] 脏页驱逐先落盘；干净页驱逐不落盘（避免无谓 I/O）。✓ Test2
- [ ] 析构落盘残留脏页。✓ Test2
- [ ] 同页并发 shared 多读 / exclusive 互斥写（Test8：4 线程×1000 次 ++ → 精确 4000）。✓
- [ ] `use_counters_` 是 `int16_t`，与 `size_t` 比较时注意有符号/无符号告警（-Werror）。
- [ ] 移走的 PageGuard、自移动赋值不重复 unfix。
- [ ] `select_victim` 返回 `INVALID_VALUE`(uint16_t) 赋给 `PageID`(uint64_t) 的比较要正确（直接 `== INVALID_VALUE`）。

---

## 4. -Werror 告警防御

- 删除所有用到的 `UNUSED(...)`；对**确实不用**的形参保留 `UNUSED` 或 `(void)`。
- 循环用 `size_t` 或 `FrameID`，避免 `int` vs `size_t` 比较告警。
- `use_counters_[fid]` 比较：`if (--use_counters_[fid] == 0)`，0 是 int，atomic<int16_t> 减后返回 int16_t，比较前注意提升，必要时显式 `== 0`。
- 未使用的私有字段（如 `page_counter_`、`pinned_mutex_`）若不用会触发 `-Wunused-private-field`？——GCC 默认不报该项（那是 Clang）。本机是 g++ 13，安全；但为稳妥可不留新增未用变量。

---

## 5. 实现顺序（小步快跑，每步可编译可测）

1. **TwoQPolicy 全部方法** + `BufferFrame`(构造/三 getter) + `BufferManager` 构造/析构 + `flushPage`。先让程序能编译链接。
2. **PageGuard 四件套**（构造/析构/移动构造/移动赋值）。
3. **`fix_page` 单线程路径** + **`unfix_page`**（含 extend、命中、未命中、驱逐、落盘）。
4. 跑 **Test 1→6（单线程）** 全绿。题目建议：先把单线程做完再上多线程。
5. 复核三层锁与黄金法则 R1–R5，跑 **Test 7→10（多线程）**。Test10 校验帧锁调用次数 → 确认每次 fix 都走 `frame_lock_table_`。
6. 全量 `./build/lab2.out` 跑通 10 个；针对死锁风险，多跑几轮多线程用例（7/8/9/10 各重复 10+ 次）确认无偶发挂起/段错误。

---

## 6. 本地构建与验证（⚠️ 环境差异）

项目**只有 `CMakeLists.txt`，无 Makefile，本机未装 cmake**。req 里的 `make clean && make` 面向 Gradescope 环境。本地用 g++ 直接编译（与 CMake 同 flag）：

```bash
cd /home/kuzu/Labs/lab2/student_lab_2
mkdir -p build
g++ -std=c++17 -g -DDEBUG -Wall -Werror -Wextra -pthread \
    -Isrc/include \
    test/test.cpp \
    src/buffer/buffer_manager.cpp \
    src/include/storage/storage_manager.cpp \
    src/include/buffer/frame_lock_table.cpp \
    -o build/lab2.out

# 单个用例： ./build/lab2.out 8
# 全部：     ./build/lab2.out
# 多线程稳健性： for i in $(seq 1 20); do ./build/lab2.out 8 || break; done
```
- 每个测试开头会 `std::remove("buzzdb.dat")`，无需手动清库；但跨运行残留的 `buzzdb.dat` 无害。
- 那些 `*:Zone.Identifier` 文件是 Windows 下载产生的 ADS 残留，与构建无关，忽略即可。
- 可选：补一个简单 `Makefile` 封装上面的 g++ 命令，复刻 `make`/`make clean` 体验（不影响 Gradescope）。

---

## 7. 风险登记

| 风险 | 影响 | 缓解 |
|---|---|---|
| 死锁（持粗锁抢帧锁 / 持帧锁等粗锁形成环） | 多线程测试超时 | 严格执行 R1+R2，已论证无环 |
| 计数与 pinned 集合不一致 → 在用页被驱逐 | 数据损坏/段错误 | R3：同一临界区原子更新 |
| 解错锁模式（shared↔exclusive） | `unlock` 行为未定义/崩溃 | R4：减计数前缓存 `exclusive_` |
| 脏页未落盘 | Test2 失败、数据丢失 | 驱逐落盘 + 析构落盘双保险 |
| -Werror 卡编译 | 无法构建 | §4 告警防御 |
| 隐藏测试覆盖未想到的边界 | Gradescope 扣分 | 按契约实现、不投机取巧；保持不变式严格 |

---

## 8. 完成定义（DoD）

- `./build/lab2.out` 输出 `=== All tests passed! ===`，10 个用例全绿。
- 多线程用例重复运行 ≥20 次无挂起、无崩溃、无偶发失败。
- 编译零 warning（`-Wall -Werror -Wextra` 下）。
- 仅修改 `src/buffer/buffer_manager.cpp`，未触碰任何只读头文件。
- 代码可读：命名清晰、未命中/驱逐逻辑无重复、关键并发顺序处有注释说明"为什么"。
