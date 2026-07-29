# paper9

> **说明**：paper9.pdf 是 TIFF 扫描件（HyperGEAR 转换，**无文字层**），`pdftotext`/PyMuPDF 抽不出任何文字。
> 本文件是**逐页阅读扫描图后人工整理的内容记录**（非逐字 OCR 全文），保留了论文全部论点、术语、图表数据与章节结构。
> 原始图片：`extracted/paper9_pages/page01.png` … `page14.png`（200 dpi）。正文从 page05.png 开始（前 4 页是封面与声明页）。

**Encapsulation of Parallelism in the Volcano Query Processing System**
Goetz Graefe，University of Colorado at Boulder
技术报告 CU-CS-458-90（1990 年 3 月）；发表于 **ACM SIGMOD 1990**

---

## Abstract（摘要，逐句要点）

- Volcano 是一个新的 **dataflow query processing system**，为数据库系统的**研究与教学**而开发。
- 算子之间的 **uniform interface（统一接口）**使 Volcano 可以被新算子扩展。
- **所有算子都是按"单进程系统"来设计和编码的**（as if they were meant for a single-process system only）。
- 并行化 Volcano 时，要在两种并行化模型之间选择：**bracket model（括号模型）** 与 **operator model（算子模型）**。
- 论文说明**为什么不选 bracket model**，提出新的 operator model，并给出 Volcano 的 **exchange operator（交换算子）**的细节——它把**其他所有算子**都并行化了。
- exchange 支持：在分区数据集上的 **intra-operator parallelism（算子内并行）**，以及 **vertical（垂直）和 horizontal（水平）inter-operator parallelism（算子间并行）**。
- exchange operator **封装了全部并行性问题**，因此让并行数据库算法的实现显著更容易、更健壮。
- 这个封装里包含：**进程内 demand-driven dataflow（需求驱动/惰性）** 与 **进程间 data-driven dataflow（数据驱动/急切）** 之间的转换。
- 由于 Volcano 算子间接口与"真实的"商业系统类似，本文技术可用于并行化其他查询处理引擎。

## 1. Introduction

- 目标：为数据库教学与科研提供可扩展、模块化的查询处理系统；要**在不牺牲效率的前提下获得灵活性与可扩展性**。
- 结果是一个小系统：**不到两打（< 24）核心模块，约 15,000 行 C 代码**。
- 模块包括：file system、buffer management、sorting、top-down B⁺-trees，以及 natural join / semi-join / outer join / anti-join / aggregation / duplicate elimination / division / union / intersection / difference / anti-difference / Cartesian product **各两种算法**。
- **单独一个模块**（exchange）就能让上面列出的所有算法并行执行。
- exchange 模块是本文焦点，它是在**其他查询处理模块都写完之后**才设计实现的。
- 设计目标：**在不修改现有算法实现的前提下把它们全部并行化**；等价地说，让"尚未发明的新算法"也能被并行化，而无需这些算法在实现时考虑并行。
- 该目标**几乎完全达成**：唯一改动是设备名/编号，以支持跨多磁盘的 **horizontal partitioning（水平分区，又称 disk striping 磁盘条带化）**。
- "用一个*算子*来并行化查询求值引擎"是**新想法**：早期研究项目（GAMMA、Bubba）用的是**包住具体算子的 template process（模板进程）**——即 bracket model。
- Volcano 的设计遵循 Batory 等人 GENESIS 的许多思想；只要新算子使用并提供 Volcano 的标准算子间接口，就能被放入查询计划并被 exchange 并行化。
- Volcano 在单进程内同步多个算子、交换数据项的机制，与许多商业系统（Ingres、System R 家族）非常相似 → 本文技术可迁移。

## 2. Previous Work

- 1987 年起步时认为 **WiSS** 和 **GAMMA** 的部分决策在性能/通用性上不最优。例如 WiSS 为保护缓冲区，每次请求都要把记录**拷入拷出**，且扫描时**每条记录都重新请求一次缓冲页**——开销太大。（该批评只针对 [11] 中描述的原始 WiSS 版本；GAMMA 用的版本已重新考虑了这两点。）
- 但 Volcano 的许多设计决策**深受** WiSS 与 GAMMA 经验影响；**算子间数据交换机制是与 GAMMA 设计为数不多的激进分歧之一**。
- EXODUS storage manager 的设计过程重新审视了这些问题；**E 语言**的开发强化了"用 iterator 做查询处理"的思想；**GENESIS** 强调了**统一 iterator 接口**的重要性。
- 其他影响：Ingres、System R、Bubba、Starburst、Postgres、XPRS。
- 设计取向：追求**简洁**、追求"能支撑多种 **policies（策略）** 的 **mechanisms（机制）**"、追求每个细节的效率。
  ✅ 观点原文：**查询执行引擎应提供 mechanisms，查询优化器应决定 policies。**
- 与本工作独立地，**Tandem** 设计了一个叫 **parallel operator** 的算子，与 Volcano 的 exchange 非常相似，已在 Tandem 查询执行引擎中证明有用，但未公开发表。分布式数据库 **R\*** 用过类似技术在节点间传数据，但**仅用于数据传输，不支持数据并行或算子内并行**。

### 2.1 The Bracket Model of Parallelization（括号模型）

- 用于 **GAMMA** 和 **Bubba**。
- 有一个 **generic process template（通用进程模板）**：能收发数据，且**任一时刻只能执行恰好一个算子**（Figure 1：模板里放 JOIN 或 AGGREGATION）。
- 模板代码调用算子，然后由**算子控制执行**；收发两侧的 **network I/O 作为服务提供给算子**，实现为算子可调用的过程。
- 任一时刻可活跃的输入数被限制为 **2 个**（因为大多数数据库系统只有一元和二元算子）。
- 算子被通用模板代码"包住"（bracket 的由来），与环境隔离——包括与产生其输入、消费其输出的算子隔离。

**bracket model 的问题（考点）：**
1. **每个 locus of control（控制点）都必须被创建**，通常需要一个**独立的 scheduler process（调度器进程）**；每次扩展算子集合都要额外开发软件 → **不适合可扩展系统**。
2. 算子被编码成"**network I/O 是获取输入、交付输出的唯一手段**"（scan/store 算子除外）。因为每个算子是自己的控制点，必须用**网络流控**来协调多个算子（例如匹配生产者-消费者速度）。
3. 因此，**把一个数据项从一个算子传到另一个算子，总是要走昂贵的 IPC（inter-process communication）系统调用**——**即使整个查询在单机上求值**（本可无 IPC 单进程完成），或数据无需在网络节点间重分区时也一样。
   - 例：Wisconsin Benchmark 的三路连接 "joinCselAselB" 两次二路连接用同一连接属性，本不需要重分区。
   - 结论：**在多算子查询里（几乎所有查询），IPC 及其开销是强制的而非可选的。**
- 相比之下，多数**单进程**查询引擎里，算子之间用**过程调用（procedure call）**互相调度，比系统调用高效得多。

## 3. Volcano System Design

- Volcano 文件系统相当常规：管理 devices、buffer pools、files、records、B⁺-trees 的模块。
- 查询表示为复杂代数表达式；代数算子就是查询处理算法。
- **所有代数算子都实现为 iterators**，支持简单的 **open–next–close 协议**（与常规文件扫描类似）。
- 每个算法关联一个 **state record（状态记录）**：算法参数放在状态记录里。
- 记录上的所有操作（如比较、哈希）由**放在状态记录里作为参数传给 iterator 的 support functions（支持函数）**完成 → 查询处理模块**无需知道数据对象的内部结构、也不受其约束**。
- 多算子查询中，state records 通过 **input pointers** 串起来；input pointer 指向一个 **QEP 结构**，该结构含**四个指针**：三个过程入口（open、next、close）+ 一个 state record。
- 一个算法可在同一查询中多次使用（放多个 state record 即可）。
- **anonymous inputs / streams（匿名输入/流）**：算子**不需要知道是什么算子在产生它的输入**，也不需要知道输入来自复杂查询树还是简单文件扫描。
  - streams 是简单但强大的抽象，能把任意数量算子组合起来求值复杂查询。
  - 与 iterator 控制范式一起，streams 是单进程查询求值中**时间上（同步算子的开销）和空间上（任一时刻必须驻留内存的记录数）最高效**的执行模型。
- 对最上层算子调用 **open** → 实例化关联 state record（如分配哈希表）并对所有输入调用 open；**查询中所有 iterator 被递归初始化**。
- 反复调用最上层 **next** 直到返回 **end-of-stream** 指示符；最后 **close** 递归"关闭"所有 iterator。
- 该模型与 E 语言设计、Starburst 的代数求值系统非常接近。
- 树形 QEP 用 **demand-driven dataflow（需求驱动数据流）** 执行。
- **next 的返回值**除状态值外，是一个叫 **NEXT_RECORD** 的结构：**record identifier + 缓冲池中的记录地址**。该记录在缓冲区中被 **pinned（fixed，钉住）**。
- **fix/unfix 协议**：缓冲区中每条被钉住的记录，**任一时刻恰好被一个算子拥有（owned）**。算子收到记录后可持有一段时间（例如放进哈希表）、可 unfix（例如谓词不满足时）、或传给下一个算子。像 join 这样创建新记录的复杂操作，**必须先把新记录 fix 在缓冲区再往下传，并且要 unfix 它的输入记录**。
- **virtual devices（虚拟设备）**用于中间结果：这类设备的页**只存在于缓冲区**，unfix 时丢弃。好处：可给中间结果记录分配**唯一 RID**，所有算子可以像对待真实磁盘设备一样管理这些记录 → **算子可以当作"输入全来自磁盘文件、输出写到磁盘文件"来编程**。

## 4. The Operator Model of Parallelization（算子模型）⭐

- 把 Volcano 移植到多处理器机器时，希望**完全不改**上述单进程查询处理代码。结果是**非常干净的 self-scheduling（自调度）并行处理**。
- 称此新方法为并行化查询求值引擎的 **operator model**。该模型中，**所有控制问题都局部化在一个算子里**，这个算子对查询树中它上面和下面的算子**使用并提供标准 iterator 接口**。
- 负责并行执行与同步的模块叫 **exchange iterator**。
  ✅ **注意：它是一个 iterator，有 open、next、close 过程；因此可以插入复杂查询树的任意一处或多处。**
- Figure 2 展示了一个含数据处理算子（file scan、join）与 exchange 算子（XCHG）的复杂查询执行计划。

### 4.1 Vertical Parallelism（垂直并行 = 进程间流水线）

- exchange 的第一个功能：提供 **vertical parallelism**，即**进程之间的 pipelining（流水线）**。
- **open 过程**：先在共享内存中创建一个用于同步和数据交换的数据结构 **port（端口）**，然后**创建一个新进程**。
- 子进程用 UNIX **fork** 系统调用创建，是父进程的**精确复制**；exchange 算子随后在父/子进程中走**不同的路径**。
- **父进程 = consumer（消费者）；子进程 = producer（生产者）。**
- **消费者进程中的 exchange 表现为普通 iterator**，与其他 iterator 唯一的区别是：它**通过 IPC 而不是 iterator（过程）调用来接收输入**。
  - 创建子进程后 `open_exchange` 在消费者侧完成。
  - `next_exchange` 等待数据经由 port 到达，**一次返回一条记录**。
  - `close_exchange` 通知生产者可以关闭，等待确认，然后返回。
- **生产者进程中的 exchange 成为 exchange 以下查询树的 driver（驱动器）**，对其输入调用 open、next、close。
- next 的输出被收集进 **packets（包）**——即 NEXT_RECORD 结构的数组。
  - **packet size 是 exchange iterator state record 中的一个参数，可在 1 到 32,000 条记录之间设置。**
  - packet 填满后插入 port 中的链表，并用 **semaphore（信号量）** 通知消费者有新包。
  - **包中的记录被 fix 在共享缓冲区中，必须由消费算子 unfix。**
- 输入耗尽时，生产者侧 exchange 给最后一个包打上 **end-of-stream tag**，交给消费者，然后**等待消费者允许关闭所有打开的文件**。
  - 这个延迟在 Volcano 中是必要的：虚拟设备上的文件**在其所有记录被 unpin 之前不能关闭**。这是 Volcano 其他设计决策造成的特殊性，**不是 exchange iterator 或 operator model 固有的**。

**⭐ 范式转换（最核心考点）：**
- exchange 模块使用的 dataflow 范式**与其他所有算子不同**：
  - 其他模块基于 **demand-driven dataflow（iterators, lazy evaluation 惰性求值）**；
  - exchange 的生产者-消费者关系使用 **data-driven dataflow（eager evaluation 急切求值）**。
- 两个原因：
  1. 还打算把 exchange 用于 **horizontal parallelism**，而水平并行**用 data-driven dataflow 更容易实现**。
  2. 该方案**消除了 request messages（请求消息）的需要**。虽然带请求消息的方案（如用信号量）在共享内存机器上性能大概可接受，但会造成不必要的控制开销与延迟。作者认为极高并行度与高性能查询求值需要"共享内存机器的紧耦合网络（如 hypercube）"，故采用已在 shared-nothing 数据库机器上被证明有效的数据交换范式。

**flow control / back pressure（流控/背压）：**
- exchange 的一个**运行时开关**启用 **flow control（流控）**，又称 **back pressure（背压）**，用一个额外的信号量实现。
- 动机：若**生产者显著快于消费者**，生产者可能**钉住（pin）缓冲区的很大一部分**，从而拖累整个系统性能。
- 机制：启用流控后，生产者向 port 插入新包后**必须请求流控信号量**；消费者从 port 取走一个包后**释放流控信号量**。
- **流控信号量的初值（例如 4）决定了生产者可以领先消费者多少个包。**
- ⭐ **flow control ≠ demand-driven dataflow**（易混考点）：
  1. 流控允许生产者与消费者同步上有一些 **"slack"（松弛）**，因而是**真正的重叠执行**；而 demand-driven dataflow 是一种相当**刚性**的请求-交付结构，消费者要等生产者产出下一个输出。
  2. **data-driven dataflow 更容易与水平并行和分区高效结合。**

### 4.2 Horizontal Parallelism（水平并行）

两种形式：

**(a) bushy parallelism（浓密树并行）**
- 不同 CPU 执行复杂查询树的**不同子树**。
- **bushy parallelism 与 vertical parallelism 都属于 inter-operator parallelism（算子间并行）。**
- 实现很简单：在查询树里插入一两个 exchange 算子即可。
  - 例：为并行地把两个输入排序后送入 merge-join，把第一个或两个输入用 exchange 与 merge-join 分开。父进程在 fork 出"产生第一个已排序输入"的子进程后**立即转向第二个 sort** → **两个 sort 并行工作**。

**(b) intra-operator parallelism（算子内并行）**
- 多个 CPU 对一个已存数据集或中间结果的**不同子集执行同一个算子**。
- **intra-operator parallelism 需要 data partitioning（数据分区）。**
  - 已存数据集的分区：使用**多个文件**，最好放在**不同设备**上。
  - 中间结果的分区：在一个 **port 中放置多个 queue（队列）**。若有多个消费者进程，**每个消费者用自己的输入队列**。
  - 生产者用一个 **support function** 决定输出记录该进哪个队列（实际上是进哪个正在填充的包）。
  - ⭐ **用 support function 使得可以实现 round-robin、key-range、hash 分区。**

**（脚注 2）四种/更多并行形式：** 第四种是 **inter-query parallelism（查询间并行）**，即 DBMS 并发处理多个查询的能力——**当前版本的 Volcano 不支持 inter-query parallelism**。第五、六种涉及硬件向量处理与指令执行流水线；Volcano 是软件架构，不考虑硬件并行。

**master / slave 与进程创建：**
- 若一个算子或算子子树由一**组（group）**进程并行执行，其中一个被指定为 **master（主进程）**。
- 查询树被 open 时只有一个进程在跑，它自然是 master。master fork 出生产者-消费者关系中的子进程时，**子进程成为它那一组内的 master**。
- master producer 的第一个动作是调用适当的 support function **确定需要多少 slave**；若生产者操作要并行运行，master producer 再 fork 其余生产者进程。
- Gerber 指出这种**集中式方案在高并行度下次优**。把"由 master fork 所有生产者进程"改成 **propagation tree scheme（传播树方案）**后观察到显著性能提升——master fork 一个 slave，然后这两个各 fork 一个新 slave，然后四个各 fork 一个，如此下去（该方案在二进制 hypercube 的广播通信与同步中非常有效）。
- 即使优化了 fork 方案，其开销仍不可忽略。作者考虑使用 **primed processes（预热进程）**，即一直存在、等待工作包的进程（许多商业数据库系统使用）。由于编译代码（支持函数）的可移植分发不简单，该改动被推迟到"多共享内存机器"环境时再做。
- 所有生产者进程 fork 完后，它们**之间无需进一步同步**运行，两个例外：
  1. 访问**共享数据结构**（如通向消费者的 port、缓冲表）时，必须获取**持续一次链表插入时长的 short-term locks（短期锁）**。
  2. 当一个生产者组**同时也是消费者组**（即垂直流水线中涉及至少两个 exchange 算子和三个进程组）时，**既是消费者又是生产者的那些进程要同步两次**。在（很短的）两次同步之间，该组的 master 创建一个服务于组内所有进程的 port。
- **close 请求向下传播**到第一个 exchange 算子时，master consumer 的 `close_exchange` 用前述信号量通知所有生产者进程可以关闭；若生产者进程同时也是消费者，该进程组的 master 再通知它的生产者，如此下去。
  → **所有算子被有序关闭，整个查询求值是 self-scheduling（自调度）的。**

### 4.3 An Example（例子，Figure 3a–h）

- 四个算子 A、B、C、D，A 调 B 的、B 调 C 的、C 调 D 的 open/close/next。
- 该计划要在**三个进程组** A、BC、D 中运行 → 需要在 A 与 B 之间放一个 exchange **X**，在 C 与 D 之间放一个 exchange **Y**。**B 和 C 之间仍是简单过程调用，不跨进程边界。**
- A 作为单进程 A₀ 运行；BC 和 D 并行运行于 BC₀–BC₂ 与 D₀–D₃，**共 8 个进程**。
- A 调用 X 的 open/close/next 而非 B 的（Figure 3a）——**它并不知道会跨越进程边界**（这正是 Volcano *anonymous inputs* 的结果）。
- X 被 open 时，为 A₀ 创建一个带**一个输入队列**的 port，并 fork BC₀（3b），BC₀ 再 fork BC₁ 和 BC₂（3c）。
- BC 组 open Y 时，BC₀–BC₂ **同步**，等待 BC₀ 进程中的 Y 算子初始化好一个带**三个输入队列**的 port；BC₀ 创建 port 并把位置存到只有 BC 进程知道的地址；然后 BC₀–BC₂ **再次同步**，BC₁、BC₂ 从该位置取得 port 信息。接着 BC₀ fork D₀（3d），D₀ 再 fork D₁–D₃（3e）。
- D 算子输入耗尽时，向 Y 的 driver 部分返回 **end-of-stream**。每个 D 进程中，Y 给**每个 BC 进程**的最后一个包打上 end-of-stream 标签（共 **3×4 = 12** 个打标包），然后在信号量上等待关闭许可。
- BC 进程中的 Y 副本**计数打标包**；收到 **4 个**标签（生产者/D 进程的数量）后即认为输入耗尽，C 对 Y 的 next 调用返回 end-of-stream ⇒ **end-of-stream 指示符从 D 算子传播到了 C 算子**。依次传到 C、B，然后 X 的 driver 部分收到 end-of-stream。A₀ 中的 X 的 next 在收到 **3 个**打标包后向 A 指示 end-of-stream。
- end-of-stream 到达根算子 A 后关闭查询树：关闭 X 包括释放让 BC 进程关停的信号量（3f）；每个 BC 进程中的 X driver 关闭它的输入算子 B，B 关 C，C 关 Y（BC₁、BC₂ 中关 Y 是空操作）。BC₀ 关 exchange Y 时，Y 通过释放信号量准许 D 进程关停。D 组进程关闭所有文件、释放所有临时数据结构（如哈希表）后，用另一个信号量告知 BC₀ 中的 Y，Y 的 close 返回给调用者 C 的 close，D 进程终止（3g）。所有 BC 进程关停后，X 的 close 告知 A₀，查询求值结束（3h）。

### 4.4 Variants of the Exchange Operator（exchange 的变体）

1. **replicate / broadcast（复制/广播到所有消费者）**
   - 动机：hash-division 的两种分区方法之一要求**除数被复制**并与被除数的每个分区一起使用；Baru 的并行连接算法中一个输入关系完全不移动、另一个被送过所有处理器。
   - 实现：通过 state record 里的开关，指示 exchange 把**所有记录送给所有消费者**，事先在缓冲池里**多次 pin** 它们。
   - ⭐ **注意：不需要拷贝记录**，因为它们驻留在共享缓冲池；只要 pin 到"每个消费者都能像自己是唯一使用者那样 unpin"即可。
   - 效果：实现该特性后，用除数分区和商分区两种方法并行化 hash-division 程序**只花了约三小时**，并取得不小的加速。
2. **merge（归并）相关的两个特性**（并行排序基准测试中加入）
   - (i) 实现 **merge network（归并网络）**：一些处理器产生已排序流，由其他处理器并发归并。Volcano 的 sort iterator 产生有序流；**merge iterator 由 sort 模块很容易派生**——它用**单层归并**而非 sort 中的级联归并；**merge iterator 的输入是一个 exchange**。
   - ⭐ 与其他算子不同，**merge iterator 需要按生产者区分输入记录**。对 join 而言输入记录在哪产生并不重要，所有输入可以汇入单一输入流；**对 merge 而言，必须按生产者区分输入记录**才能正确归并多个有序流。
   - 于是 exchange 模块被改造成可以**按生产者分开保存输入记录**（由 state record 中一个参数字段切换）；`next_exchange` 的**第三个参数**用于把 merge 所需的生产者传达给 exchange iterator。
   - 还增加了 exchange 使用的输入缓冲区数量、生产者/消费者之间使用的信号量数量（含流控），以及 end-of-stream 的逻辑。这些改动都支持**多级归并树**（如并行二叉归并树），**归并路径自动选择，使每层负载尽量均匀**。
   - (ii) 实现了把"随机分区在多磁盘上的数据"排序为"**range-partitioned 有序分区**"（分布在多磁盘上的有序文件）的排序算法。当处理器数与磁盘数相同时，**每 CPU 用两个进程**：一个做 file scan 与分区、另一个做 sort。
   - ⭐ **发现：创建并运行"比处理器还多的进程"带来显著代价**，因为这些进程竞争 CPU，需要操作系统调度。虽然调度开销本身可能不算太大，但在"中央运行队列允许进程自由迁移 + 每 CPU 带大 cache"的环境下，**频繁的 cache migration（缓存迁移）增加了显著成本**。
3. **exchange 可位于算子树中间（不 fork 进程的模式）**
   - 为更好利用处理能力，决定把进程数**减半**，实际变成**每磁盘一个进程**。
   - 在此之前，exchange 算子只能"活在"一个进程算子树的**顶端或底端**；改造后，**exchange 也可以位于一个进程算子树的中间**。
   - 此时 exchange 被 open 时**不 fork 任何进程**，只建立用于数据交换的通信 port。**next 操作向它的输入树请求记录，可能把它们送给组内其他进程，直到找到属于自己分区的记录为止。**
   - ⭐ **这种运行模式还使 flow control 变得多余（obsolete）**：一个进程只有在"没有可给消费者的输入"时才运行生产者（并为其他进程生产输入）。因此若生产者有超跑消费者的危险，**没有任何生产者算子会被调度**，消费者就把可用记录消费掉。
   - （脚注 5：exchange 是 fork 新生产者进程（4.1 的原始设计）还是使用现有进程组来执行生产者操作，是一个**运行时开关**。）

**4 节小结（原文要点）：** operator model 在可扩展数据库系统中提供了**自调度的并行查询求值**。这一新方法最重要的性质是：
1. **新模块在单一模块内实现了三种并行处理形式**；
2. **它使并行查询处理完全自调度**；
3. **它没有要求对现有查询处理模块做任何修改**，因而极大地复用了在这些模块上花的时间与精力，并允许新算法轻松地并行实现。

## 5. Overhead and Performance（开销与性能）

**实验环境：** Sequent Symmetry，12 颗 Intel 16 MHz 80386 CPU，**shared-memory** 机器，每 CPU **64 KB cache**，每 CPU 约 **4 MIPS**；用硬件微秒时钟计时；Sequent 的 **DYNIX** 操作系统提供与 Berkeley 4.2 BSD / System V UNIX 完全相同的接口，且**在所有处理器上运行（执行系统调用）**。

**测试程序：** 创建记录、填入四个随机整数、**跨三个进程边界传递记录**、然后在缓冲区中 unfix。

| 实验 | 结果 |
|:---|:---|
| 无 exchange 算子：创建 100,000 条记录并在缓冲区中释放 | **20.28 秒** |
| exchange 切换到"不创建新进程"模式（相当于每条记录多加 3 次过程调用） | **28.00 秒** |
| ⇒ 该模式下**每条记录每个 exchange 算子**的开销 | (28.00−20.28)/3/100,000 = **25.73 μsec** |
| exchange 创建新进程（4 进程流水线），**启用流控** | **16.21 秒** |
| 同上，**禁用流控** | **16.16 秒** |

- ⭐ **这两个时间小于单进程执行时间**，说明**用 exchange 做数据传输非常快，流水线式多进程执行是值得的**。

**Table 1 — Exchange Performance**（100,000 条记录，从一个生产者进程组经两个中间进程组到一个消费者进程；每组 3 个进程，故每个生产者进程创建 33,333 条记录；所有实验启用流控，每 exchange 3 个 "slack" 包；每个 exchange iterator 用不同的 hash 分区函数，确保记录走遍所有可能数据路径而非三条独立流水线）：

| Packet Size [Records] | Elapsed Time [Seconds] |
|---:|---:|
| 1 | 176.4 |
| 2 | 97.6 |
| 5 | 45.27 |
| 10 | 27.67 |
| 20 | 20.15 |
| 50 | 15.71 |
| 100 | 13.76 |
| 200 | 12.87 |
| 250 | 12.73 |

- ⭐ **极小包的性能惩罚显著**：包大小从 1 增到 2，耗时几乎减半（176 → 98 秒）；增到 50 为 15.71 秒，250 为 12.73 秒。
- 回归/相关分析（包数 = 100,000/包大小 对 耗时）：**截距（基线时间）12.18 秒，斜率 0.001654 秒/包，相关性 > 0.99**。
- 考虑到数据跨**三个进程边界**交换、其中**两个边界上有三个生产者和三个消费者**，估计开销为 **1654 μsec / 1.667 = 992 μsec 每包每进程边界**。

**两个结论（原文）：**
1. **若数据传输开销小，即使对非常简单的查询计划，vertical parallelism 也能带来收益。**
2. **由于 packet size 可设为任意值，Volcano exchange iterator 的开销可以忽略不计。**

## 6. Summary and Conclusions

- Volcano **在进程内**用 dataflow 技术（**demand-driven dataflow 由 iterators 实现**），**在进程间**用 **data-driven dataflow** 在生产者与消费者之间高效交换数据；必要时 data-driven dataflow 可用 **flow control / back pressure** 增强。
- **horizontal partitioning** 同时用于**已存数据集与中间数据集**，以支持 **intra-operator parallelism**。
- ⭐ **exchange 算子的设计体现了 vertical、bushy、intra-operator 三种并行的并行执行机制，并完成 demand-driven ↔ data-driven dataflow 的相互转换。**

**operator model 相对 bracket model 的优势（逐条，极高频考点）：**
1. **它对所有其他算子隐藏了"正在使用并行"这一事实**；因此其他算子的实现可以完全不考虑并行。
2. **exchange 对其输入和输出使用相同接口，因此可以放在树中任何位置、与任何其他算子组合**；因而可用于并行化新算子，**有效地把可扩展性与并行性结合起来**。
3. **不需要独立的 scheduler process（调度器进程）**，因为调度（含初始化、流控、最终清理）是算子的一部分，在标准 **open-next-close** iterator 范式内完成。这在两种情况下变成优势：
   - 新算子集成进系统时，bracket model 需要修改 scheduler 和 template process，而 **exchange 算子不需要任何修改**；
   - 系统移植到新环境时，**只需修改一个模块（exchange iterator），而不是两个模块（template process 和 scheduler）**。
4. **不要求并行查询求值系统中的算子用 IPC 交换数据**；因此**每个进程可以执行复杂查询计划的任意子树**。
5. **单个进程可以有任意多个输入，而不只是一两个。**
6. **该算子可以（并且已经）被实现成能在生产者与消费者之间复用（multiplex）单个进程**；某种意义上它高效地实现了应用特定的**协程（co-routines）或线程**。

**未来工作：** 扩展到同时支持共享内存与分布式内存（"shared-nothing architecture"），并在紧耦合的共享内存多机网络中结合两者，同时保持封装性质（可能需要 "primed" 进程池与解释支持函数）；跨进程与跨机器透明的错误与异常管理；用 exchange 并行化面向对象数据库系统的查询处理。

**收尾定性：** **Volcano 是第一个把可扩展性与并行性结合起来的已实现查询求值系统；把所有并行性问题封装进一个模块，是让这一结合成为可能的关键。** Volcano 中的并行封装允许**新查询处理算法按单进程执行来编码，却能在高度并行环境中不加修改地运行**。

## Acknowledgements / References（第 9–10 页起）

- 致谢：Frank Symonds、Leonard Shapiro；Jerry Borgvedt 实现了分布式内存 exchange 算子原型。NSF 合同 IRI-8805200 与 IRI-8912618 支持；Sequent Computer Systems 提供大型机器机时。
- 参考文献 1–31，含 Mach [1]、Alexander & Copeland（Bubba 进程与数据流控制）[2]、System R [3]、Baru "Join on a Cube" [4]、Batory GENESIS [5]、Bitton/DeWitt/Turbyfill 基准 [6]、Bitton/Boral/DeWitt/Wilkinson 并行关系算法 [7]、Boral & DeWitt "Database Machines: An Idea Whose Time Has Passed?" [8]、WiSS [11]、GAMMA [12,13]、Tandem [14]、Gerber [15]、Volcano 详述 [17]、hash-division [19]、并行排序 [21]、Starburst [22]、E 语言 [24]、disk striping [25]、Ingres [27]、Postgres [28]、XPRS [29]、R* [31] 等。
