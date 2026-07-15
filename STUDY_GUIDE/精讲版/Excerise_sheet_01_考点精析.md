# Excerise_sheet_01 逐题精析（完整题目 + 选项 + 详解）

> **最重要的习题**（你真做过的测验，和考试同风格）。每题按这个模板：
> **【题目】** 完整英文原题 → **【选项】** 全部 A/B/C/D → **【侧重点 & 知识点】** 英文知识点 + 中文人人能懂的解释 → **【选项对错】** 每个选项为什么对/错（中英文）。
> ✓ = 你当时做对　⚠️ = 你当时做错（重点）。注意大量是 **"Which is NOT correct"（选错误的那个）**。

---

### Q1 ｜ CSV 解析 ｜ ✓

**【题目】** Consider a CSV parser:
```cpp
bool parse_user_row(const std::string& line) {
  std::stringstream ss(line);
  std::string id, name, loc, extra;
  std::getline(ss, id, ','); std::getline(ss, name, ','); std::getline(ss, loc, ',');
  if (std::getline(ss, extra, ',')) return false; // Extra read
  return true;
}
```
The function explicitly attempts an **extra `std::getline`** after the three required columns, returning false if it succeeds. **Why?**

**【选项】**
- A) To extract a hidden primary-key UUID that BuzzDB appends to the end of every CSV row.
- B) Because `std::stringstream` throws a fatal `std::out_of_range` if it sees unread delimiters, crashing the DB.
- **C) To rigorously enforce the schema by detecting and rejecting rows with more comma-separated fields than expected.** ✅
- D) To consume and ignore a hidden carriage-return (`\r`) appended by Windows editors.

**【侧重点 & 知识点】**
- 🔬 **EN:** Strict **CSV schema validation** — a user row must have exactly 3 fields; a 4th field means the row is malformed.
- 💡 **中文：** 考"严格的列数校验(schema validation)"。一行用户数据**正好 3 列**;能读出第 4 列说明这行是脏数据。🔵 类比:表格规定填 3 格,有人填了第 4 格,直接判作废。

**【选项对错】**
- A ❌ — Fabricated; there is no hidden UUID. ｜ 编造的,没有隐藏 UUID。
- B ❌ — `stringstream` does **not** throw on extra delimiters; it just reads. ｜ stringstream 遇多余分隔符不会抛异常。
- C ✅ — Exactly: detect & reject rows with too many fields. ｜ 正是:检测并拒绝列数超标的行。
- D ❌ — `\r` handling is unrelated to this check. ｜ 跟处理 `\r` 无关。

---

### Q2 ｜ unique_ptr 与序列化 ｜ ⚠️

**【题目】**
```cpp
struct Page { std::unique_ptr<char[]> data; uint32_t len; };
```
Serialization writes the page to disk; deserialization allocates a fresh buffer and reads bytes in. **Which statement about `std::unique_ptr` and serialization is NOT correct?**

**【选项】**
- **A) Passing/moving a `std::unique_ptr` into a write function serializes the pointed-to data.** ✅(这就是"错的"那句)
- B) Serialization must write the values (len and the byte payload) and reconstruct a new buffer on load.
- C) `std::unique_ptr` does not constrain the on-disk page format.
- D) Serializing the raw pointer value is meaningless across runs.

**⚠️ 你当时选了 D（D 其实是对的，所以不该选它）。正确答案是 A。**

**【侧重点 & 知识点】**
- 🔬 **EN:** A smart pointer manages **memory lifetime only**; it does NOT perform **serialization**. You must explicitly write the value (length + bytes).
- 💡 **中文：** 智能指针只管"内存的生死",**不等于会把数据写到磁盘**。序列化必须**手动写出值**(长度 + 字节)。🔵 类比:有人帮你管房子(指针),不代表他会把房子"打包邮寄"(序列化)——那是两回事。

**【选项对错】**
- A ❌(=NOT correct，本题答案) — Moving a unique_ptr does NOT serialize the data; serialization must write bytes explicitly. ｜ 传/move 指针**不会**序列化数据。
- B ✅(真) — Correct: must write len + payload, rebuild on load. ｜ 对:要写长度+字节,加载时重建。
- C ✅(真) — Correct: unique_ptr is in-memory only, irrelevant to disk format. ｜ 对:指针只在内存,不约束磁盘格式。
- D ✅(真) — Correct: a raw pointer value is meaningless across program runs. ｜ 对:裸指针值跨运行无意义(你误选了这个)。

---

### Q3 ｜ 序列化元数据布局 ｜ ⚠️

**【题目】** A page/segment format writes the **tuple-count metadata after all tuple bytes** (no header pointer to the footer). **What is the likely impact on deserialization?**

**【选项】**
- **A) The reader must scan/seek to the end to discover the tuple count, complicating deserialization.** ✅
- B) Deserialization becomes faster because the reader can skip the metadata entirely.
- C) There is no impact; the OS page cache provides the count transparently.
- D) Security improves because the tuple count is not placed until all data is read.

**【侧重点 & 知识点】**
- 🔬 **EN:** **Metadata placement** — counts/headers should go at the **front** so the reader knows how many items to read before looping.
- 💡 **中文：** 考"元数据该放哪"。元组数量应放**头部**,读者一上来就知道要循环几次。放末尾就得先 seek/扫到末尾才知道,麻烦。🔵 类比:一摞试卷,封面写"共 30 份"你才好清点;若把"共几份"写在最后一页,你得先翻到底。

**【选项对错】**
- A ✅ — Correct: count at the end forces a seek/scan to the end first. ｜ 对:数量在末尾→必须先 seek 到尾。
- B ❌ — Wrong: you can't skip the count; you NEED it. ｜ 错:数量是必读的,跳不得。
- C ❌ — Wrong: the OS cache doesn't parse your format. ｜ 错:OS 缓存不懂你的格式。
- D ❌ — Wrong: this has nothing to do with security. ｜ 错:与安全无关。

---

### Q4 ｜ 深拷贝 ｜ ⚠️

**【题目】** The Field class implements a copy constructor that performs a **deep copy** of its internal string. **Which statement about this design is NOT correct?**

**【选项】**
- A) Deep-copy semantics make it safe to pass Field by value without lifetime/aliasing issues.
- **B) Deep copy minimizes memory usage by sharing common buffers among multiple Field objects.** ✅(=错的那句)
- C) Deep copy ensures each Field has its own independent data, preventing unintended sharing.
- D) Deep copy avoids dangling pointers: when one Field is destroyed, other copies still hold valid data.

**【侧重点 & 知识点】**
- 🔬 **EN:** **Deep copy vs shallow copy** — deep copy makes an **independent** copy (uses MORE memory, but safe). **Sharing buffers = shallow copy.**
- 💡 **中文：** 考"深拷贝 vs 浅拷贝"。深拷贝=各拷一份独立的(更费内存、但安全);"共享缓冲省内存"是**浅拷贝**。🔵 类比:深拷贝=把文件**打印一份**给同事(各管各的);浅拷贝=发个**链接**给同事(共享一份,他删了你也没了)。

**【选项对错】**
- A ✅(真) — Correct: independent copies make pass-by-value safe. ｜ 对:独立副本→按值传递安全。
- B ❌(=NOT correct，答案) — Wrong: deep copy does NOT share buffers; that's shallow copy. ｜ 错:深拷贝不共享缓冲,那是浅拷贝。
- C ✅(真) — Correct: each Field owns independent data. ｜ 对:各有独立数据。
- D ✅(真) — Correct: independent copies avoid dangling. ｜ 对:独立副本不会悬空。

---

### Q5 ｜ SSD 上的缓冲管理 ｜ ✓

**【题目】** SSDs significantly reduce the penalty of random I/O compared to HDDs. **Which statement about buffer-management priorities on SSD-backed systems is NOT correct?**

**【选项】**
- **A) Configuring the buffer-pool size to the workload is not as critical as with HDDs.** ✅(=错的那句)
- B) Sophisticated sequential prefetching becomes less critical because random page fetches are already fast.
- C) Batching dirty-page flushes can reduce write amplification and improve SSD lifetime.
- D) Caching frequently accessed pages in memory remains important, since SSDs are still slower than DRAM.

**【侧重点 & 知识点】**
- 🔬 **EN:** **SSD is still much slower than DRAM**, so the buffer pool (in-memory cache) still matters a lot.
- 💡 **中文：** 考"SSD 再快也比内存慢"。所以缓冲池大小依然关键。🔵 类比:高铁(SSD)比绿皮车(HDD)快多了,但还是比"瞬移"(DRAM)慢——能瞬移就尽量瞬移。

**【选项对错】**
- A ❌(=答案) — Wrong: SSD ≠ DRAM; buffer-pool sizing is still critical. ｜ 错:SSD 不等于内存,缓冲池大小仍关键。
- B ✅(真) — Correct: random is fast on SSD, so heavy prefetch matters less. ｜ 对:随机已快,预取没那么关键。
- C ✅(真) — Correct: batching writes reduces write amplification, extends life. ｜ 对:批量刷脏页减写放大、延寿。
- D ✅(真) — Correct: SSD still slower than DRAM, so caching helps. ｜ 对:SSD 仍慢于 DRAM,缓存仍重要。

---

### Q6 ｜ unique_ptr 与异常 (RAII) ｜ ✓

**【题目】**
```cpp
void risky() {
  auto buf = std::make_unique<char[]>(4096);
  throw std::runtime_error("fail");
}
```
**Which statement about `std::unique_ptr` here is NOT correct?**

**【选项】**
- A) When the exception is thrown, stack unwinding destroys buf and frees the 4096 bytes—no manual `delete[]`.
- **B) To avoid a leak, `delete[]` must be called on buf inside the catch block.** ✅(=错的那句)
- C) If ownership had been moved before the throw, the new owner frees it; otherwise this scope frees it.
- D) RAII applies regardless of where the exception is thrown: as buf goes out of scope, its destructor releases it.

**【侧重点 & 知识点】**
- 🔬 **EN:** **RAII + stack unwinding** — when an exception propagates, local objects' destructors run automatically, freeing memory. No manual delete needed.
- 💡 **中文：** 考 RAII。抛异常时"栈展开"会自动调用局部对象的析构、自动释放内存。🔵 类比:自动感应灯——你(异常)冲出房间,灯(内存)自动灭,不用回头关。

**【选项对错】**
- A ✅(真) — Correct: stack unwinding auto-frees. ｜ 对:栈展开自动释放。
- B ❌(=答案) — Wrong: no manual delete needed; RAII handles it. ｜ 错:不用手动 delete。
- C ✅(真) — Correct: ownership transfer determines who frees. ｜ 对:谁拥有谁释放。
- D ✅(真) — Correct: RAII frees on scope exit regardless. ｜ 对:出作用域必释放。

---

### Q7 ｜ 内存表示 vs 磁盘表示 ｜ ⚠️

**【题目】** A Document holds text in `std::string` (pointer-backed) and serializes to disk as `[length][bytes]`. **Which statement best captures the distinction between in-memory and on-disk representations?**

**【选项】**
- A) On-disk format keeps file-relative pointers for random access, mirroring the in-memory layout for speed.
- **B) In memory, std::string uses pointers/capacity for dynamic growth; on disk, data is linearized into a byte stream with length, so raw pointers are neither stored nor meaningful.** ✅
- C) To avoid copies, the serializer writes the std::string's pointer value and reconstructs the same address on load.
- D) Both in-memory and on-disk forms preserve pointers so rollbacks can restore prior addresses.

**【侧重点 & 知识点】**
- 🔬 **EN:** **Serialization linearizes data** — in RAM a string is pointer-backed; on disk it's a flat byte stream `[length][bytes]`; pointers/addresses are meaningless across runs.
- 💡 **中文：** 考"序列化=线性化"。内存里字符串靠指针动态增长;磁盘上压成"长度+字节"的平直字节流,指针存了也没意义(地址会变)。🔵 见 M3 精讲的"便利贴/宜家打包"类比。

**【选项对错】**
- A ❌ — Wrong: disk doesn't keep meaningful pointers. ｜ 错:磁盘不保留有意义的指针。
- B ✅ — Correct: linearized byte stream, pointers meaningless. ｜ 对:线性化字节流,指针无意义。
- C ❌ — Wrong: writing a pointer value is useless across runs. ｜ 错:写指针值跨运行没用。
- D ❌ — Wrong: pointers aren't preserved on disk. ｜ 错:磁盘不保留指针。

---

### Q8 ｜ 封装与模块化 ｜ ✓

**【题目】** **Which statement about encapsulation and modularity in a database system is NOT correct?**

**【选项】**
- A) Encapsulation hides internal data structures behind a stable API, allowing implementation changes without breaking clients.
- **B) Encapsulation should be avoided for performance by ensuring all developers agree not to misuse internal data structures.** ✅(=错的那句)
- C) Modularity improves maintainability by separating concerns (e.g., isolating the storage manager from the query processor).
- D) Modularity lets components evolve independently as long as interface contracts hold.

**【侧重点 & 知识点】**
- 🔬 **EN:** **Encapsulation** enforces boundaries via a stable API (a mechanism), not a "gentlemen's agreement."
- 💡 **中文：** 考封装的本质——靠**机制**(私有成员/接口)强制,不靠"大家说好别乱碰"。🔵 类比:保险柜锁起来(封装),不是贴张纸条"请勿打开"。

**【选项对错】**
- A ✅(真) — Correct: stable API hides internals. ｜ 对。
- B ❌(=答案) — Wrong: relying on agreement is fragile; encapsulation is good. ｜ 错:靠约定脆弱。
- C ✅(真) — Correct: separation of concerns. ｜ 对:关注点分离。
- D ✅(真) — Correct: stable contracts allow independent evolution. ｜ 对。

---

### Q9 ｜ OLTP 工作负载 ｜ ✓

**【题目】** **Which statement about OLTP workloads is NOT correct?**

**【选项】**
- A) They target high transactional throughput with tight tail-latency SLOs.
- B) They emphasize short, latency-sensitive transactions and point reads/writes.
- **C) They are primarily long, scan-heavy transactions.** ✅(=错的那句)
- D) Occasionally long analytical queries appear but do not dominate.

**【侧重点 & 知识点】**
- 🔬 **EN:** **OLTP vs OLAP** — OLTP = short, frequent point operations; **long scan-heavy = OLAP**.
- 💡 **中文：** 考 OLTP/OLAP 区分。OLTP=短平快点操作(转账下单);长扫描分析是 OLAP。🔵 OLTP=便利店收银台;OLAP=财务部年度报表。

**【选项对错】**
- A ✅(真) — Correct: high throughput, tight tail latency. ｜ 对。
- B ✅(真) — Correct: short point reads/writes. ｜ 对。
- C ❌(=答案) — Wrong: that describes OLAP. ｜ 错:那是 OLAP。
- D ✅(真) — Correct: occasional long queries, not dominant. ｜ 对。

---

### Q10 ｜ 索引物理设计 ｜ ✓

**【题目】**
```sql
SELECT u.Username, COUNT(p.PostID) AS NumberOfPosts
FROM Users u JOIN Posts p ON u.UserID = p.UserID
WHERE u.Location = 'New York'
GROUP BY u.Username;
```
Users has indexes on (UserID) and (Location); Posts has (UserID). **Which modification most likely improves performance?**

**【选项】**
- A) Replace the join with a subquery so the optimizer can push down the Location filter.
- B) Remove the Users(Location) index to reduce write maintenance.
- **C) Add a composite index on Users(Location, Username) to accelerate grouping.** ✅
- D) Update Posts index from (UserID) to (PostID), since aggregation counts PostID.

**【侧重点 & 知识点】**
- 🔬 **EN:** **Composite index** column order — put the **filter** column (Location) first, then the **GROUP BY** column (Username), enabling an index-only path.
- 💡 **中文：** 考复合索引的列顺序——过滤列(Location)在前、分组列(Username)在后,一举两得。🔵 类比:电话簿先按城市分、城市内再按名字排,查"纽约的人按名字分组"就飞快。

**【选项对错】**
- A ❌ — Optimizer already pushes filters; subquery rewrite gives no real gain. ｜ 错:优化器本会下推,无实质收益。
- B ❌ — Removing the Location index hurts the filter. ｜ 错:删它反而变慢。
- C ✅ — Composite (Location, Username) serves filter + grouping. ｜ 对。
- D ❌ — The join is on UserID; changing to PostID doesn't help. ｜ 错:join 按 UserID。

---

### Q11 ｜ 槽页 ｜ ✓

**【题目】** Slotted pages use a slot directory mapping slot IDs to (offset, length). **Which statement about this design is NOT correct?**

**【选项】**
- A) Slot directories allow logical deletions by marking slots free without physically shifting other tuples immediately.
- B) The slot directory enables tuple movement during compaction while preserving stable slot IDs.
- C) Slot directories add a level of indirection (slightly higher lookup cost, far below scanning variable-length records).
- **D) Slot directories ensure tuples can grow in place without relocation, reducing fragmentation.** ✅(=错的那句)

**【侧重点 & 知识点】**
- 🔬 **EN:** A slotted page's **indirection** decouples slot ID from physical position; it does NOT guarantee **in-place growth**.
- 💡 **中文：** 考槽页的"间接层"。它让 slot ID 与物理位置解耦,但**不保证元组能原地长大**——变大放不下就得搬走、更新槽。🔵 见 M4 精讲"储物间挂牌"类比。

**【选项对错】**
- A ✅(真) — Correct: logical delete by marking slot free. ｜ 对:逻辑删除。
- B ✅(真) — Correct: move tuples, keep slot IDs stable. ｜ 对:移动元组、slot ID 不变。
- C ✅(真) — Correct: indirection is cheap vs scanning. ｜ 对:间接层成本远低于扫描。
- D ❌(=答案) — Wrong: no guaranteed in-place growth; relocation may be needed. ｜ 错:不保证原地增长。

---

### Q12 ｜ 范式化 ｜ ✓

**【题目】** Given a flat-file DB with separate files for Users/Posts/Interactions, **which choice best captures the normalization benefit when moving to a relational DB?**

**【选项】**
- **A) Eliminate repeated user attributes from Posts/Interactions by storing them once in Users and relating via foreign keys, preventing update anomalies.** ✅
- B) Introduce a materialized view that pre-joins Users and Posts so queries never need joins.
- C) Keep duplicated user attributes and add a trigger to update all duplicates on change.
- D) Embed Interactions as a column inside Posts to avoid multi-table lookups.

**【侧重点 & 知识点】**
- 🔬 **EN:** **Normalization** removes redundancy: store data once, reference via **foreign keys** → no update anomalies.
- 💡 **中文：** 考范式化的好处——用户名只存一份、外键关联,改名只改一处,不会漏改导致矛盾。🔵 见 M1 精讲"通讯录存一遍"。

**【选项对错】**
- A ✅ — Correct: store once + FK, prevent anomalies. ｜ 对。
- B ❌ — A materialized view isn't the normalization benefit. ｜ 错:物化视图不是范式化。
- C ❌ — Keeping duplicates + triggers is treating the symptom. ｜ 错:保留重复治标不治本。
- D ❌ — Embedding makes it worse (more nesting). ｜ 错:塞进去更糟。

---

### Q13 ｜ reinterpret_cast ｜ ⚠️

**【题目】** C++ provides `reinterpret_cast` for low-level conversions. **Which statement about why it is preferred (over C-style casts) is NOT correct?**

**【选项】**
- A) It makes potentially dangerous conversions explicit, signaling programmer responsibility.
- B) Unlike C-style casts, it only reinterprets; it doesn't silently combine const_cast/static_cast/downcasts.
- **C) It is a portable and safe mechanism for converting any pointer type to any other, as long as sizes match.** ✅(=错的那句)
- D) It makes code easier to audit because reviewers can quickly identify bit-level conversions.

**【侧重点 & 知识点】**
- 🔬 **EN:** `reinterpret_cast` does a **bit-level reinterpretation** — it is **NOT portable or safe**; its value is being *explicit* about the danger.
- 💡 **中文：** 考 reinterpret_cast。它是危险的"换一套解码表读字节",平台相关、不安全;它的好处是"把危险显式化",不是"安全可移植"。
- ⚠️ 你当时以为它安全可移植。

**【选项对错】**
- A ✅(真) — Correct: makes the danger explicit. ｜ 对:让危险显式。
- B ✅(真) — Correct: only reinterprets, no silent combos. ｜ 对:只重解释,不悄悄组合。
- C ❌(=答案) — Wrong: it is NOT portable/safe; it's a bit-level hack. ｜ 错:不可移植不安全。
- D ✅(真) — Correct: easy to spot in review. ｜ 对:易审计。

---

### Q14 ｜ 原子性 ｜ ✓

**【题目】** For a funds-transfer transaction (debit A, credit B), **what does atomicity guarantee?**

**【选项】**
- A) Balances are always application-level consistent (e.g., non-negative) throughout execution.
- **B) Either both updates take effect or neither does; no partial effects remain if the transaction aborts.** ✅
- C) No other transaction can read intermediate states produced by this transfer.
- D) Once committed, the new balances survive crashes and power loss.

**【侧重点 & 知识点】**
- 🔬 **EN:** **ACID — Atomicity = all-or-nothing.** (A=Consistency, C=Isolation, D=Durability.)
- 💡 **中文：** 考 ACID 的 A=全有或全无。别和一致(A 选项)/隔离(C 选项)/持久(D 选项)混。

**【选项对错】**
- A ❌ — That's Consistency. ｜ 那是一致性。
- B ✅ — Atomicity: both or neither. ｜ 原子性。
- C ❌ — That's Isolation. ｜ 那是隔离性。
- D ❌ — That's Durability. ｜ 那是持久性。

---

### Q15 ｜ 策略多态(编译期/运行期) ｜ ⚠️

**【题目】** BufferManager uses a polymorphic policy pointer (FIFO/LRU/2Q). **Which statement is NOT correct?**

**【选项】**
- A) It narrows the testing surface by isolating policy-specific invariants.
- **B) It allows swapping policies at compile time; at runtime the choice cannot change without relinking.** ✅(=错的那句)
- C) It encapsulates replacement logic so the BufferManager core avoids policy-specific branches.
- D) It enforces a stable interface, enabling A/B evaluation of policies without rewriting BufferManager.

**【侧重点 & 知识点】**
- 🔬 **EN:** **Runtime polymorphism** — a base-class pointer + virtual functions lets you swap the policy **at runtime** (point it at a different object), no relinking.
- 💡 **中文：** 考运行时多态。基类指针+虚函数→运行时就能换策略(指针重指向),不用重新链接。
- ⚠️ 你当时以为只能编译期换。

**【选项对错】**
- A ✅(真) — Correct: isolates per-policy invariants. ｜ 对。
- B ❌(=答案) — Wrong: runtime swapping IS possible via the pointer. ｜ 错:运行时也能换。
- C ✅(真) — Correct: core avoids policy branches. ｜ 对。
- D ✅(真) — Correct: stable interface → A/B testing. ｜ 对。

---

### Q16 ｜ 索引治查找慢 ｜ ✓

**【题目】** Reflecting on flat-file slow tuple lookups, **which relational feature most directly addresses search/retrieval efficiency?**

**【选项】**
- A) Stored procedures that encapsulate common queries.
- B) Declarative joins combining rows from multiple tables.
- C) ACID properties ensuring transactional reliability.
- **D) Secondary indexing (e.g., B+Trees) leveraged by a cost-based optimizer to choose index scans over full scans.** ✅

**【侧重点 & 知识点】**
- 🔬 **EN:** A **secondary index** (B+Tree) is the *direct* fix for slow lookups; the optimizer chooses an **index scan** over a **full scan**.
- 💡 **中文：** 考"查找慢的直接解药=索引"。其它(存储过程/join/ACID)解决别的问题。🔵 索引=书的目录。

**【选项对错】**
- A ❌ — Reduces round-trips, not lookup speed. ｜ 减往返,非查找速度。
- B ❌ — Joins combine tables, don't speed single-table lookup. ｜ join 是拼表。
- C ❌ — ACID is about reliability, not search. ｜ ACID 管可靠性。
- D ✅ — Indexing directly speeds retrieval. ｜ 索引直接加速。

---

### Q17 ｜ 定位 I/O 的 API 设计 ｜ ⚠️

**【题目】** You're replacing `seekg()+read()` with a single positional call `read_at(offset, buf, len)`. Focusing on **API design**, **which statement is NOT correct?**

**【选项】**
- A) Keeping both idioms can fragment usage unless one style is enforced.
- **B) Adopting positional I/O can significantly improve performance by avoiding a chain of function calls.** ✅(=错的那句)
- C) Even if the number of functions grows, the semantic surface can shrink because callers no longer manipulate shared stream state; invariants (offset, length) are explicit.
- D) Introducing positional I/O often leads to more function variants, increasing API entry points.

**【侧重点 & 知识点】**
- 🔬 **EN:** Positional I/O's benefit is **API clarity/correctness** (explicit offset/length, no shared stream state) — **NOT performance** (saving a call is negligible).
- 💡 **中文：** 考 API 设计。定位 I/O 的价值是"清晰/正确"(offset/length 显式、不依赖共享流状态),**不是性能**(省一两次函数调用微不足道)。
- ⚠️ 你当时以为是为性能。

**【选项对错】**
- A ✅(真) — Correct: mixing idioms fragments usage. ｜ 对。
- B ❌(=答案) — Wrong: the gain is clarity, not performance. ｜ 错:是清晰非性能。
- C ✅(真) — Correct: explicit invariants, no shared state. ｜ 对。
- D ✅(真) — Correct: more variants to maintain. ｜ 对。

---

### Q18 ｜ 纯虚函数 (AbstractPolicy) ｜ ⚠️

**【题目】** In a buffer manager, `AbstractPolicy` declares `virtual void touch(PageId)` and `virtual PageId evict() = 0;`. **Which statement is NOT true?**

**【选项】**
- A) When calling through an `AbstractPolicy*`, `touch()`/`evict()` dispatch at runtime via the vtable.
- B) Because `evict()` is `= 0`, the class is abstract and cannot be instantiated directly.
- **C) Marking `evict()` as `= 0` enables compile-time binding and eliminates virtual-call overhead.** ✅(=错的那句)
- D) Derived policies (FIFO/LRU/2Q) must implement `evict()` to be concrete.

**【侧重点 & 知识点】**
- 🔬 **EN:** `= 0` makes a **pure virtual function** → the class is **abstract**, and calls go through the **vtable at runtime** (overhead remains). It is NOT compile-time binding.
- 💡 **中文：** 考纯虚函数。`=0`=纯虚→类变抽象(不能实例化),调用走 vtable **运行时分派**(开销还在),**不是**编译期绑定。
- ⚠️ 你当时以为 `=0` 是编译期绑定。

**【选项对错】**
- A ✅(真) — Correct: runtime vtable dispatch. ｜ 对:运行时分派。
- B ✅(真) — Correct: `=0` → abstract, can't instantiate. ｜ 对:抽象类。
- C ❌(=答案) — Wrong: it does NOT enable compile-time binding. ｜ 错:不是编译期绑定。
- D ✅(真) — Correct: derived must implement evict(). ｜ 对:派生类必须实现。

---

### Q19 ｜ 持久性与可见性 ｜ ✓

**【题目】** A slotted page is modified in memory (a tuple deleted), then explicitly written back; reloading shows the deletion. **Which statement about persistence and visibility is NOT correct?**

**【选项】**
- A) Writing the page back updates on-disk state, so future reloads reflect the deletion.
- B) Other transactions/processes reading from disk won't see the change until the write-back occurs.
- **C) Every deletion must be immediately forced to disk with an fsync to preserve ACID correctness, even before the transaction commits.** ✅(=错的那句)
- D) In-memory changes become durable only after the page is written back or flushed.

**【侧重点 & 知识点】**
- 🔬 **EN:** Forcing an **fsync before commit** is NOT an ACID requirement; durability comes after write-back/flush at commit.
- 💡 **中文：** 考持久性的正确理解。未提交就强制 fsync 不是 ACID 要求;持久性是在提交时写回/刷盘后才有。

**【选项对错】**
- A ✅(真) — Correct: write-back updates disk. ｜ 对。
- B ✅(真) — Correct: others see it only after write-back. ｜ 对。
- C ❌(=答案) — Wrong: no need to fsync every delete pre-commit. ｜ 错:未提交不必次次 fsync。
- D ✅(真) — Correct: durable only after flush. ｜ 对。

---

### Q20 ｜ unfix_page 竞态 ｜ ⚠️

**【题目】**
```cpp
void unfix_page(int frame_id) {
  BufferFrame& frame = buffer_pool[frame_id];
  // global buffer_mutex is dropped here
  if (frame.exclusive) { /* ... release exclusive lock */ }
}
```
`unfix_page` checks `exclusive` **after** dropping the global lock. **Which statement regarding why this is dangerous is NOT correct?**

**【选项】**
- A) Another thread could fix_page that frame and set `exclusive = false` simultaneously.
- B) The thread might unlock a shared lock using `unlock()` instead of `unlock_shared()`, triggering UB.
- C) Even if pin count hits 0, the frame is vulnerable to eviction; another thread could overwrite its metadata first.
- **D) It immediately triggers a kernel-level thread panic because the OS detects an unprotected concurrent read on an unpinned page.** ✅(=错的那句)

**【侧重点 & 知识点】**
- 🔬 **EN:** Checking shared state **after** releasing the lock is a **TOCTOU (time-of-check/time-of-use) race**; the harm is **silent data corruption**, NOT a kernel panic.
- 💡 **中文：** 考竞态。先放锁后检查共享状态=经典 TOCTOU 竞态;后果是**悄悄的数据损坏**,OS **不会** panic。
- ⚠️ 你当时选了"内核 panic"(编造的)。

**【选项对错】**
- A ✅(真) — Correct: another thread can flip the flag. ｜ 对:另一线程可改标志。
- B ✅(真) — Correct: wrong unlock variant → UB. ｜ 对:用错解锁函数→未定义行为。
- C ✅(真) — Correct: pin 0 → evictable → metadata overwrite. ｜ 对:pin=0 可被驱逐。
- D ❌(=答案) — Wrong: the OS doesn't panic; it's a silent data race. ｜ 错:OS 不会 panic。

---

### Q21 ｜ 主键设计 ｜ ✓

**【题目】** Students(StudentID PK), 20,000 students; Enrollments(StudentID, CourseID), 100,000 rows (~5 courses/student). Designs: (1) PK=StudentID only; (2) PK=(StudentID,CourseID), StudentID FK→Students; (3) PK=CourseID only, StudentID FK. **Which statement is correct?**

**【选项】**
- A) With Design 1, all student enrollments can be stored.
- B) None of the designs allow storing all enrollments without conflicts.
- C) With Design 3, all student enrollments can be stored.
- **D) With Design 2, all student enrollments can be stored.** ✅

**【侧重点 & 知识点】**
- 🔬 **EN:** A **primary key must be unique**; a many-to-many relation needs a **composite PK (StudentID, CourseID)**.
- 💡 **中文：** 考主键唯一性。多对多关系要用复合主键。单列主键→每学生/每课只能一行,存不下。

**【选项对错】**
- A ❌ — PK=StudentID → one row per student only. ｜ 错:每学生一行。
- B ❌ — Design 2 works, so "none" is false. ｜ 错:设计2可行。
- C ❌ — PK=CourseID → one row per course only. ｜ 错:每课一行。
- D ✅ — Composite PK uniquely identifies each enrollment. ｜ 对:复合主键。

---

### Q22 ｜ mutex ｜ ✓

**【题目】** **Which statement about mutexes is NOT correct?**

**【选项】**
- **A) A mutex ensures fairness across all threads trying to access the resource.** ✅(=错的那句)
- B) RAII wrappers (e.g., std::lock_guard) reduce unlock mistakes on exceptional paths.
- C) Misusing mutex scope can create deadlocks or excessive contention.
- D) A mutex provides mutual exclusion for critical sections that touch shared state.

**【侧重点 & 知识点】**
- 🔬 **EN:** A **mutex = mutual exclusion (one at a time)**, but it does **NOT** guarantee fairness (ordering among waiters is unspecified).
- 💡 **中文：** 考 mutex 本质。互斥(一次一个)≠公平(谁先拿到不一定)。🔵 类比:厕所一次进一个,但不保证排队公平。

**【选项对错】**
- A ❌(=答案) — Wrong: no fairness guarantee. ｜ 错:不保证公平。
- B ✅(真) — Correct: lock_guard reduces missed unlocks. ｜ 对。
- C ✅(真) — Correct: bad scope → deadlock/contention. ｜ 对。
- D ✅(真) — Correct: mutual exclusion for critical sections. ｜ 对。

---

### Q23 ｜ IOPS 计算 ｜ ✓

**【题目】** Random 4KB reads. HDD seek+rotation ≈ 8 ms; SSD random read ≈ 80 µs. Assuming a single outstanding I/O, **approximate ratio of random-read IOPS (SSD vs HDD)?**

**【选项】**
- A) ≈10,000× higher on SSD
- B) ≈10× higher on SSD
- **C) ≈100× higher on SSD** ✅
- D) ≈1,000× higher on SSD

**【侧重点 & 知识点】**
- 🔬 **EN:** **IOPS ∝ 1/latency.** Ratio = HDD latency ÷ SSD latency, watching ms↔µs.
- 💡 **中文：** 考 IOPS=延迟反比。8ms ÷ 80µs = 8000µs ÷ 80µs = **100**。注意 1ms=1000µs。

**【选项对错】**
- A ❌ — 10,000× would need an 800-ms HDD. ｜ 错:数量级不对。
- B ❌ — 10× too low. ｜ 错:太小。
- C ✅ — 8ms/80µs = 100. ｜ 对:=100。
- D ❌ — 1,000× too high. ｜ 错:太大。

---

### Q24 ｜ 策略何时可换 ｜ ⚠️

**【题目】** BufferManager uses a polymorphic policy pointer. **When can the policy be changed without modifying BufferManager's core logic?**

**【选项】**
- A) Neither at compile time nor at run time.
- B) Runtime only.
- C) Compile time only.
- **D) Both at compile time and run time.** ✅

**【侧重点 & 知识点】**
- 🔬 **EN:** A virtual `Policy*` allows swapping **at both compile time and runtime**.
- 💡 **中文：** 多态指针编译期和运行期都能换策略。
- ⚠️ 你当时选了"只编译期"。

**【选项对错】**
- A ❌ — Both are possible. ｜ 错:都可以。
- B ❌ — Not runtime-only. ｜ 错:不止运行时。
- C ❌ — Not compile-time-only (you picked this). ｜ 错:不止编译期(你选的)。
- D ✅ — Both. ｜ 对:都行。

---

### Q25 ｜ std::atomic ｜ ⚠️

**【题目】**
```cpp
class StorageManager { std::atomic<uint64_t> num_pages; };
```
**Why is `num_pages` declared `std::atomic` instead of a plain integer?**

**【选项】**
- A) Because plain `uint64_t` increments are capped at 4GB on 32-bit targets.
- B) Because `std::atomic` lets BufferManager manipulate StorageManager's state without friend privileges.
- C) Because the OS file-system driver requires a thread-safe atomic pointer to map virtual pages to SSD sectors.
- **D) Because concurrent threads might read `num_pages` for bounds checking while another thread extends the file.** ✅

**【侧重点 & 知识点】**
- 🔬 **EN:** **`std::atomic` prevents a data race** on a shared counter read/written by concurrent threads.
- 💡 **中文：** 考 atomic。多个线程一边读 num_pages 做边界检查、一边有线程扩文件→需原子防 data race。

**【选项对错】**
- A ❌ — Fabricated; no 4GB cap. ｜ 编造。
- B ❌ — Fabricated; unrelated to friend. ｜ 编造。
- C ❌ — Fabricated; OS doesn't require this. ｜ 编造。
- D ✅ — Concurrent read/extend → atomic needed. ｜ 对。

---

### Q26 ｜ 封装影响范围 ｜ ✓

**【题目】** PAGE_SIZE is referenced in 5 modules. Changing 4KB→8KB: how many modules must change **with encapsulation** (PAGE_SIZE private to StorageManager) **vs without** (global)?

**【选项】**
- A) With: 0; Without: 0 (macros auto-update).
- B) With: 2; Without: only StorageManager.
- C) With: 5; Without: the same 5.
- **D) With encapsulation: 1 module; Without encapsulation: all 5 modules.** ✅

**【侧重点 & 知识点】**
- 🔬 **EN:** **Encapsulation localizes change** — a private constant changes in 1 place; a global ripples to all 5.
- 💡 **中文：** 考封装"把改动局部化"。私有常量改 1 处;全局的改 5 处。

**【选项对错】**
- A ❌ — Not zero; the value really changes. ｜ 错:不是 0。
- B ❌ — Backwards numbers. ｜ 错:数字反了。
- C ❌ — Encapsulation isn't 5. ｜ 错。
- D ✅ — 1 vs 5. ｜ 对。

---

### Q27 ｜ from_chars vs stoi ｜ ⚠️

**【题目】**
```cpp
int parse_id(const std::string& s) {
  int val = 0; std::from_chars(s.data(), s.data()+s.size(), val); return val;
}
```
The parser uses `std::from_chars` instead of `std::stoi`. **Which statement about this choice is NOT correct?**

**【选项】**
- **A) std::stoi safely rejects trailing characters (like "100abc") by throwing std::invalid_argument, making it safer for strict CSV parsing.** ✅(=错的那句)
- B) std::from_chars returns a result structure with a pointer to the first unparsed character.
- C) Using std::from_chars lets the parser strictly verify no extraneous alphabetic characters follow the number.
- D) std::from_chars does not throw exceptions or allocate memory, making it a high-performance alternative.

**【侧重点 & 知识点】**
- 🔬 **EN:** `std::stoi("100abc")` does **NOT** throw — it parses **100** and ignores "abc". It only throws when there's no leading number. So it does NOT strictly reject trailing chars; `from_chars` does.
- 💡 **中文：** 考 stoi 的"宽容"行为。`stoi("100abc")` 不抛异常,会返回 100 忽略 abc;只有完全无数字才抛。严格校验要用 from_chars(它返回首个未解析字符的指针)。
- ⚠️ 你当时以为 stoi 会抛异常拒绝。

**【选项对错】**
- A ❌(=答案) — Wrong: stoi parses the prefix, doesn't reject trailing chars. ｜ 错:stoi 解析前缀,不拒绝尾随。
- B ✅(真) — Correct: from_chars returns a ptr to first unparsed char. ｜ 对。
- C ✅(真) — Correct: from_chars enables strict checking. ｜ 对。
- D ✅(真) — Correct: no throw, no alloc, fast. ｜ 对。

---

### Q28 ｜ 空闲空间图 (FSM) ｜ ⚠️

**【题目】** Some DBMSs store free-space metadata in dedicated pages (e.g., Postgres Free Space Map). **Which statement about this design is NOT correct?**

**【选项】**
- **A) Dedicated free-space metadata pages help reduce fragmentation of pages and the need for page compaction.** ✅(=错的那句)
- B) Dedicated metadata pages can become hot spots under heavy insert load if not cached/partitioned.
- C) Centralizing free-space metadata avoids scanning every data page, speeding up insertion-path selection.
- D) Removing metadata from data pages slightly increases tuple density but may require extra I/O to consult free space during inserts.

**【侧重点 & 知识点】**
- 🔬 **EN:** A **Free Space Map (FSM)** only helps **find a page with free space** (insertion path); it does NOT reduce **in-page fragmentation** or the need for **compaction**.
- 💡 **中文：** 考 FSM 的作用边界。FSM 只帮你快速找"哪页有空",**不解决**页内碎片/压缩(那是另一回事)。
- ⚠️ 你当时以为 FSM 能减碎片。

**【选项对错】**
- A ❌(=答案) — Wrong: FSM doesn't reduce in-page fragmentation. ｜ 错:不减页内碎片。
- B ✅(真) — Correct: can be a hot spot under heavy inserts. ｜ 对:可能成热点。
- C ✅(真) — Correct: avoids scanning every page. ｜ 对:免扫每页。
- D ✅(真) — Correct: denser pages, extra I/O to consult FSM. ｜ 对。

---

### Q29 ｜ 隔离性缺失 ｜ ✓

**【题目】** Two transactions concurrently update and read the same row without proper isolation. **What can happen?**

**【选项】**
- A) Atomicity of each transaction is violated, leaving partial effects after abort.
- B) Writes may become non-durable, risking data loss after commit.
- **C) Phenomena such as dirty reads, non-repeatable reads, lost updates, or phantom reads can occur.** ✅
- D) Deadlocks become impossible because locks are not held.

**【侧重点 & 知识点】**
- 🔬 **EN:** Missing **isolation** → concurrency anomalies: **dirty read, non-repeatable read, lost update, phantom read**.
- 💡 **中文：** 考隔离性缺失的后果=四种并发异常(脏读/不可重复读/丢失更新/幻读)。

**【选项对错】**
- A ❌ — That's an atomicity issue, not isolation. ｜ 那是原子性。
- B ❌ — That's a durability issue. ｜ 那是持久性。
- C ✅ — The classic isolation anomalies. ｜ 对:隔离异常四件套。
- D ❌ — No locks → MORE problems, not fewer deadlocks claim. ｜ 错。

---

### Q30 ｜ 槽页查找延迟 ｜ ⚠️

**【题目】** A page stores 64 tuples on average. Design 1 (no-slots): contiguous, lookup scans i tuples. Design 2 (with-slots): slot directory gives constant-time lookup. Avg scan cost = 0.05 µs/tuple; slot lookup = 0.20 µs. **Which design has lower average lookup latency per tuple?**

**【选项】**
- A) Both designs have roughly similar lookup latency.
- **B) Design 2 (With slots).** ✅
- C) Designs cannot be compared without knowing tuple size.
- D) Design 1 (No slots).

**【侧重点 & 知识点】**
- 🔬 **EN:** With-slots = constant 0.20 µs; no-slots = scan **avg 32** tuples × 0.05 = **1.6 µs**. Slots win.
- 💡 **中文：** 考槽页的"间接层用固定开销换掉线性扫描"。无槽平均扫 32 个(64/2)×0.05=1.6µs;有槽固定 0.20µs→有槽快得多。
- ⚠️ 你当时选了"无法比较"。

**【选项对错】**
- A ❌ — Not similar; 1.6 vs 0.2. ｜ 错:差很多。
- B ✅ — 0.20 µs < 1.6 µs. ｜ 对。
- C ❌ — We're given per-tuple cost, so it's comparable. ｜ 错:给了每元组成本。
- D ❌ — No-slots is slower. ｜ 错:无槽更慢。

---

### Q31 ｜ 持久性 ｜ ✓

**【题目】** A funds-transfer transaction. **Which best describes the durability property?**

**【选项】**
- A) The transaction must execute in isolation from other concurrent transactions.
- B) The balances must be updated instantly to always reflect the true state.
- C) Both account updates must succeed together, or neither should affect the database.
- **D) Once the transaction commits, its effects must persist even after a crash or power failure.** ✅

**【侧重点 & 知识点】**
- 🔬 **EN:** **ACID — Durability = committed effects survive crashes** (via write-ahead logging).
- 💡 **中文：** 考持久性=提交后崩溃/断电也不丢(靠写前日志 WAL)。

**【选项对错】**
- A ❌ — That's Isolation. ｜ 隔离性。
- B ❌ — "instant true state" ≈ Consistency-ish, not durability. ｜ 非持久性。
- C ❌ — That's Atomicity. ｜ 原子性。
- D ✅ — Survives crash = Durability. ｜ 对:持久性。

---

### Q32 ｜ 性价比计算 ｜ ⚠️

**【题目】** Two hierarchies: A (≈100 ns, $1000/mo), B (≈400 ns, $300/mo). Throughput ∝ 1/latency; price/performance = $/Mops (dollars per million ops/sec; lower is better). **Which has better price/performance, and approximately what value?**

**【选项】**
- A) Hierarchy B; about $30/Mops
- B) Hierarchy A; about $400/Mops
- **C) Hierarchy A; about $100/Mops** ✅
- D) Hierarchy B; about $120/Mops

**【侧重点 & 知识点】**
- 🔬 **EN:** throughput = 1/latency; price/perf = cost ÷ throughput (lower better). A: 1/100ns = 10 Mops → $1000/10 = $100. B: 2.5 Mops → $300/2.5 = $120. **A wins.**
- 💡 **中文：** 考性价比=成本÷吞吐(越低越好)。A=10Mops→$100;B=2.5Mops→$120。A 更划算。
- ⚠️ 你当时算反了(选了 B)。

**【选项对错】**
- A ❌ — Wrong system + wrong number. ｜ 系统和数都错。
- B ❌ — A is right but $400 is wrong (that'd ignore throughput). ｜ A 对但 $400 错。
- C ✅ — A, ≈$100/Mops. ｜ 对。
- D ❌ — B is worse ($120), and B isn't the winner. ｜ 错:B 更差。

---

### Q33 ｜ ACID 的意义 ｜ ✓

**【题目】** **Why are ACID properties crucial in a multi-user DBMS?**

**【选项】**
- A) They limit concurrency to prevent overload, ensuring stable throughput.
- B) They keep the system lightweight and guarantee faster reads/writes than file systems.
- C) They force the optimizer to choose the optimal query plan for each workload.
- **D) They ensure predictable behavior under concurrency and failures, preserving correctness and data integrity.** ✅

**【侧重点 & 知识点】**
- 🔬 **EN:** **ACID = correctness/predictability under concurrency and failures.**
- 💡 **中文：** 考 ACID 的目的=并发与故障下保证正确、可预测。

**【选项对错】**
- A ❌ — ACID isn't about limiting concurrency. ｜ 非限制并发。
- B ❌ — Not about being lightweight/faster. ｜ 非更轻更快。
- C ❌ — Not about the optimizer. ｜ 非优化器。
- D ✅ — Correctness under concurrency + failures. ｜ 对。

---

### Q34 ｜ 静态 vs 动态类型 ｜ ✓

**【题目】** Static (C++) vs dynamic (Python) typing. **Which statement captures a fundamental difference most relevant to DB engine implementations?**

**【选项】**
- A) In statically typed languages, a variable's type can change at runtime if the optimizer decides it's profitable.
- B) Dynamic typing prevents runtime type errors by deferring checks to execution, improving safety vs static typing.
- C) Static typing cannot support parametric polymorphism (templates/generics), limiting reuse in systems code.
- **D) Static typing associates types with variables/expressions known at compile time, enabling code specialization (e.g., fixed-width tuple layouts).** ✅

**【侧重点 & 知识点】**
- 🔬 **EN:** **Static typing** binds types at **compile time**, enabling **code specialization** (e.g., fixed-width tuple layouts) → faster DB engine code.
- 💡 **中文：** 考静态类型=编译期定类型→能做代码特化(定长 tuple 布局),性能好。

**【选项对错】**
- A ❌ — Static types don't change at runtime. ｜ 错:运行时不改类型。
- B ❌ — Dynamic typing isn't safer; errors surface at runtime. ｜ 错:动态非更安全。
- C ❌ — Templates ARE parametric polymorphism. ｜ 错:模板就是泛型。
- D ✅ — Compile-time types → specialization. ｜ 对。

---

### Q35 ｜ 主外键建模 ｜ ✓

**【题目】** Students(StudentID PK), Courses(CourseID PK), Enrollments(StudentID, CourseID). **Which implementation correctly uses keys to maintain integrity, and why?**

**【选项】**
- A) Set CourseID as the PK of Enrollments and StudentID as a FK referencing Students.
- **B) Set a primary key (StudentID, CourseID) on Enrollments; StudentID and CourseID are foreign keys referencing Students(StudentID) and Courses(CourseID) respectively.** ✅
- C) Set StudentID as the sole PK of Enrollments with no FKs to avoid join overhead.
- D) Use EnrollmentID as a surrogate PK to maximize flexibility.

**【侧重点 & 知识点】**
- 🔬 **EN:** A many-to-many link table uses a **composite PK (StudentID, CourseID)** + **two FKs** referencing each parent → enforces referential integrity.
- 💡 **中文：** 考多对多建模=复合主键 + 两个外键(各引用 Students/Courses)→保证引用完整性。

**【选项对错】**
- A ❌ — CourseID-only PK → one row per course. ｜ 错:每课一行。
- B ✅ — Composite PK + 2 FKs: correct M:N modeling. ｜ 对。
- C ❌ — Sole StudentID PK + no FK → loses integrity & limits rows. ｜ 错:丢完整性。
- D ❌ — A surrogate key alone doesn't enforce the M:N uniqueness/integrity here. ｜ 错。

---

## 📊 你的 14 道错题清单（按重做优先级）

| # | 一句话考点（EN term 中文） | 模块 |
|:--|:---|:--:|
| Q2 | serialization ≠ smart pointer（序列化≠智能指针） | M3 |
| Q3 | metadata in header（元数据放头部） | M3 |
| Q4 | deep vs shallow copy（深/浅拷贝） | M3 |
| Q7 | linearize, pointers meaningless（线性化,指针无意义） | M3 |
| Q13 | reinterpret_cast unsafe（不可移植不安全） | M4 |
| Q15 / Q24 | runtime polymorphism（运行时多态可换） | M5 |
| Q17 | API clarity not performance（API清晰非性能） | M4 |
| Q18 | pure virtual = runtime dispatch（纯虚=运行时分派） | M5 |
| Q20 | TOCTOU race not panic（竞态非内核panic） | M8 |
| Q25 | atomic for shared counter（原子防计数器竞态） | M6 |
| Q27 | stoi is lenient（stoi宽容,from_chars严格） | M3 |
| Q28 | FSM finds space ≠ compaction（FSM找空位≠碎片整理） | M3 |
| Q30 | slots beat scan（有槽优于扫描） | M4 |
| Q32 | price/perf = cost÷throughput（性价比算法） | M3 |

> 🔴 **最该攻克：M3**（Q2/3/4/7/27/28/32）+ **M5 多态**（Q15/18/24）。配套新题 → `12_全真模拟题库` Part C/E。
