# ch1 — Introduction（导论）精讲

> **模板**：每个知识点 = **【原句】**(章节里的英文原句) → **【侧重点 & 知识点】**(🔬英文知识点 + 💡中文人人能懂的解释)。
> ch1 是 Silberschatz《Database System Concepts》第 1 章，对应 **Module 1–2**。考点集中在:DBMS 要解决什么、三层抽象、数据独立性、数据库引擎组成。

---

## 1. DBMS 要解决什么（文件系统的毛病）

**【原句】** *"Data redundancy and inconsistency... Difficulty in accessing data... Data isolation... Integrity problems... Atomicity of updates... Concurrent access by multiple users... Security problems."*

**【侧重点 & 知识点】**
- 🔬 **EN:** Before DBMSs, apps were built directly on **file systems**, causing **redundancy/inconsistency, hard data access, data isolation, integrity problems, atomicity failures, uncontrolled concurrency, and security gaps**. A DBMS solves all of these.
- 💡 **中文:** 这就是 M1 讲的 flat-file 六毛病的"课本版"。直接拿文件当数据库,会冗余、取数难、数据孤立、完整性差、更新不原子、并发乱、不安全。DBMS 就是来一次性解决这些的。🔵 类比:用几个记事本管全公司数据 vs 用专业的数据库管家。

---

## 2. 三层数据抽象（View of Data）

**【原句】** *"Physical level: describes how a record is stored. Logical level: describes data stored in database, and the relationships among the data. View level: application programs hide details of data types."*

**【侧重点 & 知识点】**
- 🔬 **EN:** Three levels of **data abstraction** — **physical level** (how bytes are stored), **logical level** (what tables/relationships exist), **view level** (what each app sees; can hide sensitive fields).
- 💡 **中文:** 看数据分三层,越往上越抽象友好:
  - **physical level(物理层)** = 钢筋水泥怎么浇的(磁盘字节布局)。
  - **logical level(逻辑层)** = 楼里有几个房间、怎么连(有哪些表、什么关系)。
  - **view level(视图层)** = 你作为住户看到的,还能藏起敏感信息(如不让员工看别人工资)。

---

## 3. 实例 vs 模式（Instances and Schemas）

**【原句】** *"Logical Schema – the overall logical structure of the database... Instance – the actual content of the database at a particular point in time."*

**【侧重点 & 知识点】**
- 🔬 **EN:** **Schema** = the structure/design (like a variable's *type*); **instance** = the actual data at a moment (like a variable's *value*). Schema rarely changes; instance changes constantly.
- 💡 **中文:** schema(模式)=结构(像变量的"类型","用户表有 id/名字/地点"),instance(实例)=此刻的实际数据(像变量的"值")。🔵 类比:schema 是空白表格模板,instance 是填好的那一份。

---

## 4. 物理数据独立性（Physical Data Independence）⭐

**【原句】** *"Physical Data Independence – the ability to modify the physical schema without changing the logical schema."*

**【侧重点 & 知识点】**
- 🔬 **EN:** **Physical data independence** = you can change *how* data is stored (physical schema) **without** changing the logical schema or the apps that depend on it.
- 💡 **中文:** 物理数据独立性=改"数据怎么存"(物理层)不影响"数据长啥样"(逻辑层)和应用。🔵 类比:餐厅后厨换新灶具(物理变),你点菜方式(逻辑)不变。这正是 Codd(Paper 1)和 M1 反复强调的核心优势。

---

## 5. DDL vs DML（两种数据库语言）

**【原句】** *"Procedural DML – require a user to specify what data are needed and how to get those data. Declarative DML – require a user to specify what data are needed without specifying how to get those data."*

**【侧重点 & 知识点】**
- 🔬 **EN:** **DDL (Data Definition Language)** defines structure (e.g., `create table`). **DML (Data Manipulation Language)** accesses/updates data. DML is **procedural** (say what + how) or **declarative** (say what only). SQL is **declarative** and easier.
- 💡 **中文:** **DDL(数据定义语言)** 定结构(建表);**DML(数据操纵语言)** 增删改查。DML 分两种:**procedural(命令式,要说怎么一步步取)** 和 **declarative(声明式,只说要什么)**。SQL 是声明式,所以好用。🔵 类比:声明式=点外卖说"要碗牛肉面";命令式=自己写"烧水、下面、加牛肉…"。

---

## 6. SQL 不是图灵完备的

**【原句】** *"SQL is NOT a Turing machine equivalent language... To compute complex functions SQL is usually embedded in some higher-level language."*

**【侧重点 & 知识点】**
- 🔬 **EN:** SQL is a **nonprocedural query language**, not Turing-complete — it can't do everything (no UI, no network). It's **embedded in a host language** (C++/Java/Python) for full apps.
- 💡 **中文:** SQL 专门管查询,但不是"万能编程语言"(做不了界面、网络)。所以真实程序里 SQL **嵌在**宿主语言(C++/Java/Python)里用。

---

## 7. 数据库引擎三大件（Database Engine）⭐

**【原句】** *"The functional components of a database system can be divided into: the storage manager, the query processor component, the transaction management component."*

**【侧重点 & 知识点】**
- 🔬 **EN:** A DBMS engine has three parts:
  - **Storage manager** — interfaces with the OS file system; handles storing/retrieving/updating data. Includes the **buffer manager**, file manager, transaction manager, authorization.
  - **Query processor** — DDL interpreter, **DML compiler** (does **query optimization** = picks the lowest-cost plan), query evaluation engine.
  - **Transaction management** — keeps the DB consistent despite failures; **concurrency-control manager** handles concurrent transactions.
- 💡 **中文:** 数据库内部分三块(BuzzDB 整门课就在造这些):
  - **存储管理器(storage manager)**:管数据怎么存磁盘、读写、缓冲(含 **buffer manager**)。→ M3–M5
  - **查询处理器(query processor)**:把 SQL 翻译成执行计划、选最便宜的方案(查询优化)、执行。
  - **事务管理(transaction management)**:保证 ACID、管并发。→ M2/M6/M8

---

## 8. 数据库架构类型（简要）

**【原句】** *"Centralized databases... Parallel databases (shared memory / shared disk / shared nothing)... Distributed databases."*

**【侧重点 & 知识点】**
- 🔬 **EN:** DBMSs run on different architectures: **centralized**, **parallel** (shared-memory / shared-disk / shared-nothing), and **distributed** (geographically spread). Apps are often **two-tier** (app calls DB directly) or **three-tier** (client → app server → DB).
- 💡 **中文:** 数据库可以单机(centralized)、多核/多机并行(parallel)、或地理分布(distributed)。应用常见两层(应用直连数据库)或三层(前端→应用服务器→数据库)。了解即可。

---

## ✅ ch1 速记
1. DBMS 解决文件系统的六毛病(冗余/取数难/孤立/完整性/原子性/并发/安全)。
2. **三层抽象**:physical(怎么存) < logical(有啥表) < view(应用看到的)。
3. **schema**=结构(类型);**instance**=数据(值)。
4. **physical data independence**=改物理存储不动逻辑/应用。
5. **DDL** 定结构、**DML** 增删改查;SQL 是**声明式**。
6. 引擎三大件:**storage manager / query processor / transaction management**。

> 📌 配套:M1/M2 精讲、`论文重点`(Codd 数据独立性)。
