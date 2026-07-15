# M2 精讲版+ · BuzzDB、为何用 C++、事务与 ACID

> **格式**：每个概念 = 📖**原文**(讲座英文原句) + 💡**大白话**(类比+例子) + 📝**考试这么考**(✅对的 / ❌陷阱)。
> M2 对应讲座 "BuzzDB"，配 ch1(事务节)、Paper 2(NoSQL 与 ACID)。先看术语表。

---

## 📒 M2 术语英中对照

| English | 中文 | 一句话含义 |
|:---|:---|:---|
| transaction | 事务 | 当成一个整体的一组操作 |
| ACID | ACID | 事务的四大保证 |
| atomicity | 原子性 | 全做或全不做 |
| consistency | 一致性 | 从一个合法状态到另一个合法状态 |
| isolation | 隔离性 | 并发事务互不串味 |
| durability | 持久性 | 提交后崩溃也不丢 |
| commit | 提交 | 确认事务、永久生效 |
| rollback / abort | 回滚/中止 | 撤销事务的所有改动 |
| write-ahead logging (WAL) | 写前日志 | 先写日志到磁盘再提交(保证持久性) |
| encapsulation | 封装 | 把数据和操作打包、隐藏内部 |
| public / private | 公有/私有 | 外部可访问 / 仅类内可访问 |
| static typing | 静态类型 | 编译期就定好类型(C++) |
| compiled language | 编译型语言 | 直接编成机器码(C++,快) |
| std::vector | 向量 | 自动扩容的动态数组(当"表") |
| std::map | 映射 | 有序键值容器(当"索引") |
| tuple | 元组 | 表里的一行 |
| aggregation | 聚合 | 分组求和等统计(GROUP BY) |

---

## 1. BuzzDB 的最小骨架：tuple / vector / map

**📖 原文：** *"In BuzzDB, our basic data unit is the tuple class... This class stores tuples using a vector container in C++ and indexes the tuples using a map container."*

**💡 大白话：** 最早期的 BuzzDB 极简,就两个 C++ 容器:
- **`tuple`(元组)** = 表里一行,最初就是个 key-value 对(如"客户ID → 金额")。
- **`std::vector`** = 自动扩容的动态数组,BuzzDB 用它**存所有 tuple**(这就是那张"表")。
- **`std::map`** = 有序的键值容器,BuzzDB 用它当**索引**(key → 该 key 的多个值)。
> 🔵 类比:vector 像可伸缩的书架(放所有书);map 像通讯录(名字→号码,按字母排,查得快)。

**📝 考试这么考：**
- ✅ *"BuzzDB stores tuples in a `std::vector` (the table) and indexes them in a `std::map` (the index)."*
- ❌ *"`std::map` is a hash table giving O(1) lookup."* —— 错！`std::map` 是平衡 BST(O(log n)、有序);哈希表是 `unordered_map`。

---

## 2. 类与封装（class / encapsulation）

**📖 原文：** *"Encapsulation... helps protect access to the attributes and methods of a class. There are two types of access specifiers, public and private."*

**💡 大白话：** **类(class)** = 对象的蓝图(把"数据"和"操作数据的函数"打包)。**封装(encapsulation)** = 用 `public`(外部可访问) / `private`(仅类内可访问) 控制访问,**隐藏内部实现**。
> 🔵 类比:封装像把贵重物品锁进保险柜,只留一个取物口(public 接口);别人不能直接翻你内部(private)。好处:你改内部实现,外面不受影响 → 模块化。

**📝 考试这么考：**
- ✅ *"Encapsulation hides internal data structures behind a stable API, allowing implementation changes without breaking clients."*
- ❌ *"Encapsulation should be avoided for performance by trusting developers not to misuse internals."* —— 错(测验 Q8)！靠"约定"脆弱,封装靠机制。

---

## 3. 为什么用 C++ 而不是 Python ⭐

**📖 原文：** *"Performing the same operation in Python takes... around 5.24 seconds. That's roughly 50,000 times slower than C++."* 及 *"C++ is a statically typed language... variable types are known at compile time."*

**💡 大白话：** BuzzDB 用 C++,四个理由:
| 维度 | C++ | Python |
|:---|:---|:---|
| 执行 | **编译型**,直接编成机器码 | 解释型,运行时逐句解释 |
| 速度 | 算 1 亿整数和 ≈ 100 µs | ≈ 5.24 s(**慢约 5 万倍**) |
| 类型 | **静态类型**(编译期定) | 动态类型(运行时定) |
| 内存 | 精细控制(智能指针) | 自动 GC,控制弱 |
> 🔵 类比:速度差距像"亚特兰大飞纽约 2 小时" vs "飞冥王星 9.5 年"。

**静态类型的好处**:编译期就知道类型 → 能**提前抓类型错误** + 能做**代码特化**(如定长 tuple 布局)→ 更快。

**📝 考试这么考：**
- ✅ *"Static typing binds types at compile time, enabling code specialization (e.g., fixed-width tuple layouts)."*（测验 Q34）
- ❌ *"Dynamic typing is safer because it defers type checks to runtime."* —— 错！静态类型编译期拦错更安全。

---

## 4. 聚合查询（aggregation = GROUP BY + SUM）

**📖 原文：** *"The aggregation query in BuzzDB sums up values associated with keys. This is similar to a group by clause in SQL."*

**💡 大白话：** BuzzDB 的 `select_group_by_sum` 遍历索引里每个 key,把它关联的所有 value 求和。等价于 SQL 的 `GROUP BY key + SUM(value)`。
> 🔵 例子:索引里 key 1 → [100, 200],聚合后输出 key 1 → 300。

---

## 5. 什么是事务（Transaction）

**📖 原文：** *"A transaction is a series of operations in a database treated as a single atomic unit."*

**💡 大白话：** 查询(select/group-by)只**读**数据;要**改**数据就得用**事务**——把一组操作当成**一个不可分割的整体**。
> 🔵 经典例子:**转账**——A 扣 $100、B 加 $100,必须**一起成功或一起失败**,绝不能只扣不加。SQL 里用 `BEGIN TRANSACTION ... COMMIT`。

---

## 6. ⭐⭐ ACID 四性质（整个期中最高频考点）

**📖 原文：** *"Atomicity guarantees that the transaction changes all or nothing... Consistency ensures that each transaction only brings the database from one valid consistent state to another... Isolation prevents transactions from interfering with each other... durability ensures that once a transaction is committed, it remains so."*

**💡 大白话：** 用**转账**串起来记:

| 字母 | 名字(EN) | 大白话 | 转账中保证 |
|:---:|:---|:---|:---|
| **A** | Atomicity 原子性 | **全做或全不做** | A 扣了 B 必加;中途失败整体回滚 |
| **C** | Consistency 一致性 | 从合法态到合法态 | 转账前后总额守恒 |
| **I** | Isolation 隔离性 | 并发**互不串味** | 两个柜员同时存款不互相覆盖 |
| **D** | Durability 持久性 | 提交后**崩溃也不丢** | 取了现金,记录绝不消失 |

**隔离性的"两个柜员"例子**(原文):两人都读到初始余额 $1000、各自算,后提交的覆盖先提交的 → 错成 $1500 或 $1300(正确应是 $1800)。
**无隔离的后果**:dirty read(脏读)、non-repeatable read(不可重复读)、lost update(丢失更新)、phantom read(幻读)。

**📝 考试这么考：**
- ✅ *"Atomicity: either both updates take effect or neither does."*（Q14）
- ✅ *"Durability: once committed, effects persist even after a crash."*（Q31）
- ✅ *"Without isolation: dirty reads, non-repeatable reads, lost updates, phantoms."*（Q29）
- ❌ 把字母张冠李戴(把"全有全无"说成持久性等)——A/C/I/D 别混。

---

## 7. 持久性怎么实现：写前日志（WAL）

**📖 原文：** *"The database system ensures this by using a technique called write ahead logging, where the transaction details are first returned to a log stored on a durable disk before the transaction is marked as committed."*

**💡 大白话：** **持久性(D)** 靠 **WAL(Write-Ahead Logging 写前日志)** 实现:先把事务细节**写到磁盘上的日志**,再标记 commit。崩溃后用日志重建已提交状态。
> 🔵 类比:你转账前,银行先在小本子(日志)上记一笔"准备转 100",再真正动账户;就算系统崩了,小本子还在,能据此恢复。

**🌟 Paper 2 关联:** 2000 年代 NoSQL 喊"ACID 没必要、用弱一致 BASE",结果**几乎都把 ACID 加了回来**——说明 ACID 是刚需。

**📝 考试这么考：**
- ✅ *"Durability is implemented via write-ahead logging (WAL): log to durable disk before marking committed."*

---

## 📝 M2 出题风格英文速记（✅对的 / ❌陷阱）

| 考点 | ✅ 对的英文 | ❌ 陷阱 |
|:---|:---|:---|
| vector/map | vector = table, map = index | "std::map is a hash table O(1)"(错,是 BST) |
| 封装 | encapsulation hides internals behind a stable API | "avoid encapsulation for performance"(错) |
| 静态类型 | static typing → compile-time + specialization | "dynamic typing is safer"(错) |
| 原子性 | both updates or neither | 把它说成持久/隔离(错) |
| 持久性 | committed effects survive crash (via WAL) | 把它说成隔离(错) |
| 隔离性 | missing → dirty/non-repeatable/lost/phantom | "abort leaves partial effects"(那是原子性) |
| NoSQL+ACID | NoSQL later added ACID back | "ACID can't scale, NoSQL avoids it forever"(错) |

---

## ✅ M2 一页总结
1. BuzzDB 骨架:**vector 存表、map 当索引**、tuple 是行。
2. **封装**(public/private)隐藏内部 → 模块化。
3. 用 C++:编译型快(~5万倍)、**静态类型**(编译期定 + 代码特化)、控内存。
4. **事务** = 一组操作当整体(改数据用)。
5. **ACID**:原子(全有全无)、一致(合法态)、隔离(不串味)、持久(不丢)——必背。
6. 持久性靠 **WAL 写前日志**。
7. NoSQL 当年弃 ACID 后来又加回(Paper 2)。

> 📌 配套:`ch1_导论_精讲`(事务管理)、`论文重点`(NoSQL+ACID)、`Excerise_sheet_01_考点精析` Q14/29/31/33/34。
