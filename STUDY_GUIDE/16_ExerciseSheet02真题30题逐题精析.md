# Exercise Sheet 02 真题 30 题 · 逐题精析

> ## 🔴 为什么这是期末最高价值的一份材料
>
> **你提供的关键情报**：期中 35 题里有 **10 多道是 Exercise Sheet 01 的原题**。
> 我交叉核对了你期中的 10 道错题 —— **没有一道来自 Sheet 01**（两个关键词命中是假阳性，落在 Sheet 01 的别的题里）。
>
> ⇒ **结论：被复用的原题你全对，丢的分 100% 在"新题"上。**
>
> **对期末的直接推论**：
> | 事实 | 推论 |
> |:---|:---|
> | 期中复用了 Sheet 01 约 1/3 的题 | **期末大概率复用 Sheet 02 的题**（Sheet 02 = 期中后内容 = 期末的 80% 区） |
> | 你 Sheet 02 只考了 **16/30** | ⭐ **这 14 道错题就是最可能被原样搬上考卷、而你又答不对的题——这是全项目里最确定的失分点** |
> | 期中丢分全在 Sheet 01 没覆盖的新题 | ⭐ **期末的新题会集中在 Sheet 02 没覆盖的地方：`module_11`、`module_12`、论文 P5–P12、ch14/15/18** |
>
> **所以期末的两条战线：**
> 1. **保住复用分** ← 本文档（把这 30 题连每个选项都吃透）
> 2. **攻新题** ← `14_期末总复习手册.md` Part C.5/C.6（module_11/12）+ `精讲版/论文重点_P5-P12`
>
> ---
> **你的成绩：16 / 30（53%）。错题 14 道：Q1、Q3、Q6、Q11、Q15、Q17、Q18、Q19、Q21、Q23、Q26、Q27、Q29、Q30。**

---

## 📊 先看你的错题结构

| 主题 | 错题 | 数量 | 占错题比 |
|:---|:---|:--:|:--:|
| ⭐⭐ **学习索引 Learned Index** | Q3、Q17、Q21、Q26 | **4** | **29%** |
| ⭐⭐ **算子框架 / 算子优化** | Q1、Q11、Q18、Q27 | **4** | **29%** |
| **R-Tree** | Q19、Q29 | 2 | 14% |
| **倒排索引** | Q23、Q30 | 2 | 14% |
| C++ 代码 | Q6 | 1 | 7% |
| 聚合 | Q15 | 1 | 7% |

> 🎯 **学习索引 + 算子框架 = 8/14 = 57% 的失分。这两块是你在期中后内容里的核心短板。**
> 📌 顺带一提：**Sheet 02 真题 30 题里，教材 ch14 / ch15 / ch18 的内容一道都没有** —— 没有 join 代价、没有外部排序、没有并发控制。**全部 30 题都出自 lecture transcript（module_7–10）。** 这印证了课程调研里那句"主要考 lecture"。

---

# Part 1 · 你错的 14 道（逐题深挖）

---

## ❌ Q1 · Scan 算子的 `next()`（算子框架）

> **Which statement about implementing `next()` in a Scan operator is NOT correct?**
> - A. It should be robust to encountering empty or logically deleted tuples and skip them when producing output.
> - B. Correct implementations typically maintain explicit page and slot cursors and centralize boundary checks to keep the logic maintainable. ← **你选的**
> - C. **It must detect when the last page (and first slot) has been read and signal end-of-stream to avoid infinite loops.** ← **正确答案**
> - D. The operator must correctly advance across slots within a page and then across pages within the relation.

### 为什么 C 是错的 —— **一个词的陷阱**
end-of-stream 的条件是读完 **最后一页的【最后一个】slot**，不是"**first slot**"。
把 `last slot` 换成 `first slot`，整句就从对变错。**如果你按 first slot 来判断结束，扫描会在每页刚开始时就误判结束**（或者反过来永远结束不了，取决于实现）。

### 你为什么选了 B
B 描述的是"显式维护 page/slot 游标 + 集中做边界检查"——**这是完全正确的工程实践**，而且正是 BuzzDB Scan 算子的写法。你大概觉得"centralize boundary checks"听起来像是多余的修饰。

> ⚠️ **这是你的【根因 A】再现**：B 是一句朴素的工程实话，你却觉得它可疑。
> 🔑 **本题的通用教训：反选题里要逐词读，特别是 first/last、before/after、increase/decrease 这类"一个词翻转全句"的位置。** 出题人在这一整套卷子里反复用这招（见 Q8、Q18、Q19、Q26、Q30）。

---

## ❌ Q3 · 学习索引 vs B+ 树（⭐ 学习索引第 1 题）

> **Which statement about learned indexes, compared to traditional B+ Trees, is NOT correct?**
> - A. **They support better worst-case lookup time than a B+ Tree for complex key distributions.** ← **正确答案**
> - B. They reduce index size by replacing parts of the tree structure with compact model parameters. ← **你选的**
> - C. They are organized hierarchically, with coarse models routing keys to sub-models trained on smaller key ranges.
> - D. They exploit regularity in the key distribution to reduce the average number of memory accesses for point lookups.

### 为什么 A 是错的 —— ⭐⭐ **最坏情况正是学习索引的软肋**
| | B+ 树 | 学习索引 |
|:---|:---|:---|
| **最坏情况查找** | ⭐ **保证 O(log n)**，与数据分布**无关** | ⭐ **没有更好的最坏情况保证**。分布越复杂 → 模型误差越大 → 误差窗口内的局部搜索范围越大 |
| **平均情况** | O(log n) | ⭐ **可能好得多**（分布规整时接近 O(1)） |
| 兜底手段 | — | ⭐ **hybrid index：叶模型误差超阈值就换成 B-Tree** —— 注意措辞：这是把最坏情况**兜到 B 树的水平**，**不是超过它** |

💡 **一句话记住：学习索引赢在【平均】，B+ 树赢在【最坏】。** 论文原话也只敢说 *"can have significant advantages"*，从不宣称最坏情况更优。

### B、C、D 为什么都对
- **B 对**：用紧凑的模型参数替换部分树结构 → 论文实测**小最多两个数量级**。
- **C 对**：这就是 **RMI（递归模型索引）**——粗模型把键路由到在更小键范围上训练的子模型。
- **D 对**：利用键分布的规律性降低点查的**平均**内存访问次数（注意是 average，措辞很小心）。

> 🔑 **本题与 Q17、Q21、Q26 是同一族**：出题人反复考"学习索引的**局限**"，不是它的优点。**复习学习索引时，把"它做不到什么"单独列一张表。**

---

## ❌ Q6 · placement new（C++ 代码题）

```cpp
auto new_page_id = next_page_id++;
auto new_leaf = std::static_pointer_cast<LeafNode>(getNode(new_page_id));
new(new_leaf.get()) LeafNode();     // ← 问这一行
```
> **What is the primary purpose of this syntax in the context of the buffer manager?**
> - A. To instruct the OS to eagerly allocate a fresh, zero-initialized page on the physical storage drive. ← **你选的**
> - B. **To invoke the constructor directly onto the existing buffer frame, initializing it without new heap memory.** ← **正确答案**
> - C. To register the node with the global C++ garbage collector.
> - D. To dynamically cast a `Node*` into a `LeafNode*`, throwing `std::bad_cast` if corrupted.

### 必须搞懂：placement new 是什么
```cpp
new T();          // 普通 new：① 向堆申请内存  ② 在那块内存上调用构造函数
new (ptr) T();    // placement new：① 不申请内存  ② 【只】在 ptr 指向的已有内存上调用构造函数
```
**为什么缓冲管理器一定要用它**：页面的内存**已经存在了**——它就是缓冲池里的一个 frame，由 buffer manager 管理。你不能也不应该再去堆上分配；你要做的只是**在这块既有内存上把对象"造出来"**（初始化 vtable 指针、成员变量等）。

### 三个错误选项分别是什么手法
- **A（你选的）** = **手法④不存在的机制**：C++ 的 `new` 完全不与操作系统的物理磁盘分配打交道，更不会"eagerly 分配零初始化的页到物理盘"。
- **C** = **手法④**：⭐ **C++ 没有 garbage collector。**
- **D** = **手法⑤张冠李戴**：那是 `dynamic_cast`。而且代码里用的是 `static_pointer_cast`（**不做运行时检查、不抛异常**）。

> 🔑 **顺带把这组 cast 记全**（Sheet 01 也考过 `reinterpret_cast`）：
> | Cast | 何时检查 | 失败行为 |
> |:---|:---|:---|
> | `static_cast` / `static_pointer_cast` | **编译期** | **不检查**，错了是 UB |
> | `dynamic_cast` | **运行期**（需要多态类型） | 指针版返回 `nullptr`，引用版抛 `std::bad_cast` |
> | `reinterpret_cast` | 不检查 | 纯粹重新解释比特 |
> | `const_cast` | 不检查 | 只增删 const/volatile |

---

## ❌ Q11 · Select 算子的优化策略（算子优化）

> **Which is NOT a typical optimization strategy for a Select operator whose predicate rejects many tuples?**
> - A. Pushing the Select as close as possible to the Scan to avoid materializing tuples that will be discarded.
> - B. Applying short-circuit evaluation to avoid computing parts of a complex predicate when the outcome is already determined. ← **你选的**
> - C. **Reordering conjuncts (e.g., `WHERE price < 100 AND stock > 0 AND classify(text) = 'fiction'`) so that expensive or more selective predicates are evaluated first.** ← **正确答案**
> - D. Using available indexes to retrieve only tuples that are likely to satisfy the predicate.

### 为什么 C 是错的 —— ⭐ **"expensive … first" 方向反了**
合取谓词重排序的正确原则是：
```
把【便宜】且【选择性高】的谓词放【前面】
```
- 例子里 `price < 100` 和 `stock > 0` 是**简单的整数比较，极便宜**；
- `classify(text) = 'fiction'` 是一个 **UDF / 文本分类调用，极其昂贵**。
- 正确顺序：先跑两个便宜的比较，**大部分元组在这里就被淘汰了**，只有少数幸存者才需要付出昂贵的 `classify()` 代价。

⭐ **标准的代价模型**：谓词按 **`选择性 / 代价`** 的比值从高到低排（每单位代价能淘汰最多元组的排最前）。

**C 的措辞 `expensive **or** more selective ... first` 把两个方向捆在一起**——"more selective first"是对的，"expensive first"是**完全反的**。这是 **手法⑥半真半假**：一句话里塞了一对一真一假。

### B 为什么是对的（你不该排除它）
短路求值（short-circuit）：`A AND B` 里若 A 已为 false，就**不必计算 B**。这是**标准且普遍**的优化，C++ 的 `&&` 本身就是短路的。

> ⚠️ **【根因 A】第三次出现**：B 是朴素、正确、人人都知道的做法，你却觉得它"太普通不像答案"。
> 🔑 **规则 3 要真的用起来：一个选项平平无奇地正确，往往就是干扰项；错的那个通常在某个方向词上翻了车。**

---

## ❌ Q15 · HashAggregation 的内部数据结构（聚合）

> **In a Hash Aggregation operator, which statement about the internal data structure is NOT correct?**
> - A. A hash table keyed by the group-by attributes is a common choice because it supports expected O(1) lookups and updates.
> - B. **Using an ordered balanced tree instead of a hash table can make sense if the output must be produced in key-sorted order without an extra Sort operator.** ← **正确答案**
> - C. Hash Aggregation must store the group keys (or a representation of them) so that aggregate states can be associated with specific groups. ← **你选的**
> - D. Some implementations spill hash buckets to disk when the in-memory hash table grows beyond a configured size.
> - E. Each hash table entry typically stores the group key and one or more aggregate state variables (e.g., sum, count).

### 为什么 B 被判为错 —— ⭐ **题目的作用域是"a Hash Aggregation operator"**
这题问的是「**Hash** Aggregation 算子**内部**的数据结构」。把哈希表换成有序平衡树，**它就不再是 Hash Aggregation 了**——那是**基于排序 / 有序聚合（sort-based / ordered aggregation）**，是**另一个算子**。

> ⚠️ **这题确实微妙，值得单独警惕。** 因为在官方例题（Sheet 02 example Q4 选项 D）里，"分组键 = 排序键时可用 ordered aggregation 策略替代单独的 Sort" 被判为**正确**。
> **两者不矛盾，区别在作用域：**
> | 说法 | 判定 | 为什么 |
> |:---|:--:|:---|
> | "**规划器可以选用** ordered aggregation **策略** 来代替 Sort" | ✅ 对 | 这是**算子选择层面**的话 |
> | "在 **Hash** Aggregation 算子**内部**用平衡树替换哈希表" | ❌ 错 | 这是**算子实现层面**的话，替换后它就不是 hash aggregation 了 |
>
> 🔑 **教训：读题时先锁定"这句话在讲哪一层"——规划器层？算子层？数据结构层？** 本卷的 Q27 也是靠这招（把"模块化"和"重编译执行器代码"混为一谈）。

### C 为什么对
必须存组键（或它的某种表示），否则无法把聚合状态关联到具体的组——这是**任何**聚合实现的必然要求。

---

## ❌ Q17 · 单个线性回归做学习索引（⭐ 学习索引第 2 题）

> **Which statement about using a single linear regression model as a learned index is NOT correct?**
> - A. Its limited capacity can make it less robust to complex distributions than a small hierarchy of specialized sub-models. ← **你选的**
> - B. **It eliminates the risk of overfitting to the training data in practice.** ← **正确答案**
> - C. It may underfit highly non-linear key→position mappings, leading to large local prediction errors.
> - D. Its simplicity can make it cheaper to evaluate than deep neural models.

### 为什么 B 是错的 —— ⭐ **手法①绝对化：`eliminates`**
线性模型**降低**过拟合风险（模型容量小），但**不能消除**。
- 数据点少、噪声大时，即使一条直线也可能过度贴合样本。
- 更重要的是：⭐ **学习索引的"训练集"就是全部数据本身**，它面对的真正问题是 **underfit（欠拟合）**，而**不是**"过拟合被消除了"——B 用一个听起来像好事的绝对化断言把这层关系搞反了。

> 🔑 **看到 `eliminates / guarantees / always / never / no risk of` 立刻标红。** 这条规则你在期中 Q6 也栽过（`always return deterministic order`）。**这是你第二次在同一手法上失分。**

### A、C、D 为什么都对
- **A 对**：容量有限 → 面对复杂分布不如"一小撮专门化子模型的层级结构"（**这就是 RMI 存在的理由**）。
- **C 对**：对高度非线性的 key→position 映射会 **underfit**，造成大的局部预测误差。
- **D 对**：简单 → 求值比深度神经网络便宜（论文里第二阶段就是用**简单线性模型**，因为"最后一英里不值得跑复杂模型"）。

---

## ❌ Q18 · 算子间传递元组时的深拷贝（算子框架 / C++）

> **Which statement about deep copying tuples between operators is NOT correct?**
> - A. Deep copying can prevent subtle aliasing bugs in which two operators unintentionally share and modify the same in-memory tuple.
> - B. Deep copying ensures that an upstream operator can safely reuse or overwrite its internal buffers without affecting downstream operators. ← **你选的**
> - C. **Some engines use reference counting or ownership flags to perform deep copies when tuples are only read, not modified.** ← **正确答案**
> - D. Deep copying increases memory and CPU overhead, so it is often selectively applied only where necessary.

### 为什么 C 是错的 —— ⭐ **手法②方向反转，一个词之差**
引用计数 / 所有权标志的用途是 **AVOID（避免）** 深拷贝，不是 **perform（执行）** 深拷贝。
```
只读场景 → 用引用计数共享同一份数据，【不拷贝】
要写了   → 才真正拷贝一份            ← 这就是 copy-on-write（写时复制）
```
C 把 "avoid" 换成 "perform"，整句就反了：**如果元组只被读，恰恰是最不需要深拷贝的时候。**

### B 为什么对（你不该排除它）
上游算子把元组深拷贝给下游后，就可以**放心地复用/覆盖自己的内部缓冲区**（比如 Scan 复用同一个页缓冲），因为下游拿的是独立副本。⭐ **这正是 BuzzDB 的 `SelectOperator` 在 `getOutput()` 里做深拷贝的理由。**

> 🔑 **本题 + Q1 + Q26 + Q30 都是"一个词翻转"型**。做反选题时，把每个选项里的**动词和方向词**（perform/avoid、first/last、increase/reduce、requires/allows）圈出来单独验证。

---

## ❌ Q19 · R-Tree 如何建模空间（R-Tree 第 1 题）

> **Which statement about how an R-Tree models space is NOT correct?**
> - A. **Internal MBRs are compact and may not contain regions of space where no actual object resides.** ← **正确答案**
> - B. Each internal node stores, for every child, an MBR that encloses all objects in that child's subtree.
> - C. Internal nodes store only bounding rectangles, not exact polygonal geometry of all objects in their subtree. ← **你选的**
> - D. Leaf nodes typically store either the MBRs of actual objects or object identifiers with their associated MBRs.

### 为什么 A 是错的 —— ⭐ **MBR 必然包含"死空间"**
**MBR = 轴对齐的矩形**。要用一个矩形去包住若干个**不规则形状 / 分散分布**的对象，**必然会圈进大量没有任何对象的空白区域**（这叫 **dead space，死空间**）。

```
   ┌──────────────────────┐   ← MBR
   │  ▲                   │
   │        ●             │   ← 这些空白全是 dead space
   │              ■       │
   └──────────────────────┘
```
⭐ **死空间正是 R-Tree 的核心弱点**，也是：
- 为什么 **MBR 会重叠**（Q5、Q8 考的）；
- 为什么**一次搜索可能要下到多棵子树、没有好的最坏情况保证**；
- 为什么 **SplitNode 的目标是"最小化两个覆盖矩形的总面积"**（就是在压缩死空间）；
- 为什么**维度越高性能越差**（Q8：高维下重叠**增加**，趋向全扫描）。

A 说 MBR "compact 且**可能不包含**没有对象的区域"——**方向完全反了**。

### C 为什么对
内部节点确实**只存外包矩形，不存对象的精确多边形几何**——这正是"用 MBR 做近似"的定义。你大概觉得这句话"太显然"了。

> ⚠️ **【根因 A】第四次出现。**

---

## ❌ Q21 · 神经网络做学习索引的挑战（⭐ 学习索引第 3 题）

> **Which is NOT a realistic challenge when using neural networks as learned indexes?**
> - A. Handling data distribution drift, which can degrade the accuracy of the learned mapping over time.
> - B. Ensuring that the model's prediction error stays within a bounded window so that local search remains efficient. ← **你选的**
> - C. **Increasing model complexity to reduce memory footprint and lower lookup latency when deployed inside the learned index.** ← **正确答案**
> - D. Choosing appropriate hyperparameters, such as learning rate, network depth, and number of epochs.

### 为什么 C 是错的 —— ⭐ **它本身就是个自相矛盾的句子**
```
增加模型复杂度  →  内存占用【增大】、查找延迟【增大】
                   ❌ 不可能"reduce memory footprint and lower lookup latency"
```
这不是"一个挑战"，这是**一句在物理上讲不通的话**。论文里的真实权衡恰恰相反：*"The challenge is to **balance the complexity of the model with its accuracy**"*——复杂度和精度之间要**折中**，因为复杂度是要**付出代价**的。

### A、B、D 为什么都是真实挑战
- **A**：**分布漂移（distribution drift）** —— 论文附录 D.1 明确列为开放问题："**分布变化了怎么办？能否检测？**"
- **B**：**把预测误差控制在有界窗口内**，好让局部搜索保持高效 —— 这正是 RMI 要存 min/max error 的原因（**Q26 考的就是这个**）。
- **D**：**超参数选择**（学习率、网络深度、epoch 数）—— 论文说目前用**简单的 grid search** 调参。

---

## ❌ Q23 · 位置倒排索引的邻近查询顺序（倒排索引第 1 题）

> **Two conceptual stages: (1) compute the set of doc IDs containing all query terms; (2) within a document, merge the positional lists of the terms to check distances. Which order is typical?**
> - A. (2) only  B. (1) only  C. **(1) followed by (2)** ← **正确答案**  D. (2) followed by (1) ← **你选的**

### 为什么是 (1) → (2) —— ⭐ **便宜的过滤先做**
| 阶段 | 做什么 | 代价 |
|:---|:---|:---|
| **(1)** | 求各 term 的 **posting list 交集**，得到"同时含全部查询词"的文档集合 | ⭐ **便宜**：只比文档 ID，可用跳表/位图加速 |
| **(2)** | 在**幸存的文档内部**归并各 term 的**位置列表**，检查距离 ≤ k | ⭐ **贵**：要读并归并位置数据 |

**如果反过来先做 (2)**，你就得对**每一篇文档**（包括那些根本不含全部查询词的）去读位置列表——**绝大部分是白读的**。

💡 **这和 Q11（谓词重排序）是同一个原理的两个面**：**先用便宜且选择性高的条件缩小候选集，再对幸存者做昂贵计算。**
💡 **也和 module_12 列存的"先扫 timestamp 收 offset，再取 temperature 页"完全同构**（late materialization）。

> 🔑 **这三处（Q11 谓词重排、Q23 邻近查询、列存晚物化）是同一条思想在不同层面的体现——期末很可能拿其中任一个出题。**

---

## ❌ Q26 · 学习索引的邻域搜索（⭐ 学习索引第 4 题）

> **Which statement about "neighborhood search" around the model-predicted position is NOT correct?**
> - A. **Even if the model's error bound is known, the window size cannot be chosen to guarantee correctness while limiting scan cost.** ← **正确答案**
> - B. It relies on the fact that data is stored in key-sorted order, so nearby positions are good candidates when the prediction is off.
> - C. It compensates for model error by scanning a bounded window around the predicted index in the sorted array.
> - D. Neighborhood search can be combined with a small fallback binary search inside the window to further reduce comparisons. ← **你选的**

### 为什么 A 是错的 —— ⭐ **知道误差界，恰恰就能选出保证正确的窗口**
这正是 **RMI 的核心机制**：
```
训练后，对【每个最后阶段的模型】分别记录 min-error 和 max-error
      （= 该模型在所有键上的最坏低估和最坏高估）
查找时窗口 = [pos − min_err, pos + max_err]
⭐ 只要键存在，它【一定】落在这个窗口内 → 正确性有保证
⭐ 窗口宽度 = 该模型的实际误差，而不是全局最坏值 → 扫描代价被限制住
```
论文原话：*"we store the standard and min- and max-error for every model on the last stage. That has the advantage, that we can **individually restrict the search space based on the used model for every key**."*

A 说"即使知道误差界也**无法**选出既保证正确又限制代价的窗口"——**把学习索引最关键的可行性直接否定了**。

### D 为什么对（你不该排除它）
窗口内当然可以再用二分搜索进一步减少比较次数——论文的默认策略 **Model Biased Search** 就是"**把二分搜索的第一个中点设为模型预测值**"；还有 **Biased Quaternary Search**（三个初始分裂点 `pos−σ, pos, pos+σ`）。

> ⚠️ 你在**同一份卷子上的四道学习索引题里错了四道中的三道**（Q3、Q17、Q21、Q26），且**每次都是选了一个真陈述**。
> 🔑 **学习索引这块必须重学**，重点是 `14_期末总复习手册.md` §C.2.4 + `精讲版/论文重点_P5-P12` 的 Paper 7 全章（office hour 已经在这篇上出了 2 道题）。

---

## ❌ Q27 · 模块化算子引擎（算子框架）

> **In a modular operator-based query execution engine (Scan, Select, Join, etc.), which statement is NOT correct?**
> - A. Different physical implementations of the same logical operator (e.g., nested-loop join vs. hash join) can be swapped without changing the rest of the pipeline. ← **你选的**
> - B. **Operator modularity supports query optimization passes that rewrite pipelines by recompiling low-level executor code.** ← **正确答案**
> - C. Modularity can make it easier to introduce new operators, such as a custom Aggregation or Sort, without rewriting existing code.
> - D. Operators expose a standard interface (e.g., open, next, close) so they can be composed into pipelines.

### 为什么 B 是错的 —— ⭐⭐ **跨技术张冠李戴（把编译执行的性质安给模块化）**
**模块化（modularity）的收益机制是"稳定接口 + 组合"**：
```
所有算子实现同一个 open/next/close 接口
   → 可以任意组合、替换、插入
   → 【完全不需要】重编译任何低层执行器代码
```
而"**重编译低层执行器代码**"是 ⭐ **查询编译（query compilation）**的做法 —— 那是 `module_11` 和 **P10 HyPer** 的技术，**而且它恰恰是要"离开算子框架"、放弃模块化**来换性能的。

> ⭐⭐ **这道题是 Final Practice Q7（"查询编译不是 C-Store 的技术"）的同类**——**跨技术归属错配**。
> **两者的关系必须记清：**
> | | 模块化算子框架 | 查询编译 |
> |:---|:---|:---|
> | 手段 | **统一接口 + 运行期组合** | **生成/重编译机器码** |
> | 代价 | ⭐ 每 tuple 的**虚函数分派 + 多次函数调用** | **编译耗时 + 灵活性差** |
> | 关系 | ⭐ **两者是对立的取舍**，不是互相支持 | |
>
> 🔑 **`module_11` 的整节课就是在讲"为了性能放弃模块化"** —— 所以"模块化支持重编译执行器代码"从根上就说不通。

### A 为什么对
同一个逻辑算子的不同物理实现（nested-loop join ↔ hash join）**因为接口相同，可以互换而不影响流水线其余部分**——这是模块化最经典的收益，也是查询优化器能选择物理算子的前提。

---

## ❌ Q29 · ND R-Tree 的典型应用（R-Tree 第 2 题）

> **Which is NOT a typical application of ND R-Trees?**
> - A. **Sorting text files lexicographically by treating each character as a separate dimension.** ← **正确答案**
> - B. Finding all pairs of objects whose regions intersect. ← **你选的**
> - C. Indexing multi-dimensional feature vectors for nearest-neighbor queries.
> - D. Spatial indexing for GIS managing maps, polygons, and points.

### 为什么 A 是错的 —— ⭐ **字典序排序是一维有序问题**
- 字典序（lexicographic order）本质上是**单一全序**上的排序 → 该用 **B+ 树 / trie / 直接排序**。
- 把"每个字符当一个维度"塞进 R-Tree 是**荒谬的**：字符串长度不定 → 维度数不定；而且 R-Tree **根本不维护全序**（这正是 Q5 考的："R-Tree 不存在按坐标排序的中序遍历"）。
- 再加上**维度爆炸**：几十个字符就是几十维，**curse of dimensionality** 会让它退化成全扫描（Q8 考的）。

### B 为什么是典型应用（你不该排除它）
"找出所有区域相交的对象对" = ⭐ **spatial join（空间连接）**，这是 R-Tree 的**教科书级典型应用**——用 MBR 相交做初筛，正是 R-Tree 最擅长的事。

> 🔑 **R-Tree 的三道题（Q5、Q8、Q19、Q29）全部围绕一个核心：MBR 是【近似】，近似带来【重叠】和【死空间】，重叠带来【多路径搜索】和【高维退化】。** 把这条因果链背下来，四道题一起解决。

---

## ❌ Q30 · proximitySearch 的形式化定义（倒排索引第 2 题）

> 定义：`proximitySearch(w1, w2, k) = { d ∈ D | ∃ p ∈ Pos(w1,d), q ∈ Pos(w2,d), |p − q| ≤ k }`
> **Which statement is NOT correct?**
> - A. The implementation typically short-circuits once any qualifying pair is discovered for a document.
> - B. The check is symmetric in order; either term may appear first in the text. ← **你选的**
> - C. **When k ≥ 1, the routine requires `pos2 = pos1 + k`; otherwise the document is rejected.** ← **正确答案**
> - D. A document qualifies if ∃ pos1, pos2 with |pos1 − pos2| ≤ k in that document's position lists.

### 为什么 C 是错的 —— ⭐ **把"≤"偷换成"="**
```
定义写的是   |p − q| ≤ k       ← 距离【不超过】k，是一个【范围】
C 说的是     pos2 = pos1 + k   ← 距离【恰好】等于 k，且【方向固定】
```
**两处都错**：① `≤` 变成 `=`；② 绝对值消失了，变成有向的 `pos1 + k`。
按 C 的说法，`k=3` 时距离为 1 或 2 的文档会被**错误拒绝**——而按定义它们显然合格。

### B 为什么对（你不该排除它）
⭐ **绝对值 `|p − q|` 天然就是对称的**：`|p−q| = |q−p|`，所以哪个词先出现无所谓。这句话是**定义的直接推论**。

> 🔑 **本题的通用教训：给了数学定义的题，把定义抄在草稿纸上，逐个选项【代进去验】。**
> 这和期中 Q33（`π_X(π_Y(R))` 是交集还是并集）是**完全一样的题型和完全一样的失误**——⭐ **你在"带公式的形式化陈述"上已经栽了两次了。**
> **应对法：拿最小例子代入。** 本题：设 `Pos(w1)={5}`, `Pos(w2)={6}`, `k=3` → `|5−6|=1 ≤ 3` 合格；但按 C 要求 `pos2 = 5+3 = 8 ≠ 6` → 被拒 → **C 与定义矛盾，立刻判死。**

---

# Part 2 · 你做对的 16 道（快速确认，别丢分）

> 这些你已经会了，**但期末如果原样复用，一分都不能丢**。每题只记"正确答案 + 一句话理由"。

| # | 题目 | ✅ NOT-correct 的那句（即答案） | 一句话理由 |
|:--|:---|:---|:---|
| **Q2** | R-Tree 最近邻的 best-first 遍历用什么结构 | **Priority queue（优先队列）** ← 这题是选"正确的" | 按"到查询点的距离"排序展开节点，**必须用优先队列**；栈=DFS、队列=BFS 都不能保证 best-first |
| **Q4** | Select 谓词 direct/indirect | "Indirect references are **heavily used in index lookups** and can be highly optimized" | ⭐ 索引查找用的是**键值/常量**，那是 **direct** reference；indirect 的用途是**同元组内两列比较** |
| **Q5** | R-Tree vs B+ Tree | "R-Trees support an **in-order traversal** that enumerates rectangles **sorted by coordinates**" | ⭐ **R-Tree 没有全序**，不存在这种中序遍历 |
| **Q7** | Select 谓词（Q4 的变体） | "Some systems lower **indirect** references to an efficient special case…" | ⭐ 方向反了：**被下沉成高效特例的是 direct（字段 vs 常量）**，indirect 才是需要通用表示的一般情形 |
| **Q8** | R-Tree 维度升高 | "Overlaps between MBRs typically **reduce**, forcing more branches to be visited" | ⭐ **自相矛盾**：重叠若减少，访问的分支应**变少**。真相是维度升高 → **重叠增加** → 剪枝失效 → 趋向全扫描（curse of dimensionality） |
| **Q9** | 流水线插入 Projection | "…introducing Projection requires **rewriting the Join operator** to call Projection explicitly" | 统一接口的全部意义就是**插入新算子不用改邻居的代码** |
| **Q10** | HashAggregation 的 runningSum/runningCount | "**MIN(x) does not need additional per-group state**" | ⭐ MIN 需要**自己的运行最小值**；sum 和 count 推不出 MIN。（COUNT←runningCount、SUM←runningSum、**VARIANCE 需要额外的平方和**） |
| **Q12** | 层级学习索引的子模型 | "Sub-models adapt the **same model architecture and hyperparameters**…" | ⭐ 子模型**可以用不同架构**——顶层小神经网络、底层成千上万个线性模型、难学的地方直接换 B-Tree（**hybrid index**） |
| **Q13** | B+树"先分裂后插入" | 正确答案：**"It prevents temporarily writing past the fixed-size array boundaries, avoiding a buffer overflow."** | 若先插再分裂，会**先写到定长数组界外**——那一瞬间就已经是缓冲区溢出（UB） |
| **Q14** | DDL vs DML | "A user allowed to perform **DML** on a table is typically allowed to perform **DDL** on it" | ⭐ 权限完全不同：能 SELECT/INSERT ≠ 能 DROP TABLE/ALTER。**DDL 权限远高于 DML** |
| **Q16** | GROUP BY + ORDER BY 流水线 | "When ORDER BY uses the same key(s) as GROUP BY, planners **can assume** HashAggregation's output will be in correct key order" | ⭐ **哈希聚合输出顺序由哈希函数决定，与键的大小顺序无关** |
| **Q20** | Patricia trie 插入与分裂 | "Patricia-tries maintain **one-character edges**; splits expand into chains of character-sized edges" | ⭐ **Patricia 的边存【子串】**（单字符边那是普通 trie）；分裂只是把一条边拆成两条，**不产生字符链** |
| **Q22** | CreateIndex 放在 open() | "If index construction is expensive, doing it in open() will **increase the latency of every next() call**" | ⭐ 放在 open() 的**全部意义**就是"只做一次"，next() 反而**不再承担**这个代价 |
| **Q24** | Select 算子的作用 | "It can significantly **increase** the number of tuples processed by downstream operators when predicates are **selective**" | ⭐ 选择性高的谓词是**减少**下游元组量 |
| **Q25** | Web 级倒排索引分片 | 正确答案：**document-partitioned**（按文档切 shard、查询广播、coordinator 归并） | ⭐ transcript 教的是 term-partitioned，但**生产主流是 document-partitioned**——"超出 transcript 一小步" |
| **Q28** | B+树 rangeQuery 代码 | 正确答案：**"It iterates through the leaf keys and follows the `next` pointer to the sibling leaf."** | ⭐ **叶子间的兄弟指针**就是 B+ 树支持范围查询的关键（不用回父节点、不用按页号猜、不用逐键 lookup） |

---

# Part 3 · 从这 30 道真题反推的出题人画像（比之前准确得多）

## 3.1 题型分布（真实数据，取代我之前的错误估计）

| 题型 | 数量 | 占比 |
|:---|:--:|:--:|
| **"Which statement is NOT correct?"** | **26** | ⭐ **87%** |
| C++ / BuzzDB 代码题 | 3（Q6、Q13、Q28） | 10% |
| 短事实题（选"正确的"） | 1（Q2） | 3% |
| ⭐ **计算题** | ⭐ **0** | ⭐ **0%** |

> 🔴 **我之前说"期末计算题占 40%"是错的。** 那是拿官方 Final Practice 的 7 题外推的（3/7=43%），而**真实卷子 Sheet 01（35 题）+ Sheet 02（30 题）共 65 题里只有 3–4 道计算题（≈5%）**。
> **Final Practice 的 7 题应当被理解为"教授在点名新考点的主题"，而不是"题型分布的样本"。**

## 3.2 主题密度（Sheet 02 = module_7–10）

| 主题 | 题数 | 占比 |
|:---|:--:|:--:|
| ⭐ **学习索引** | 5（Q3、Q12、Q17、Q21、Q26） | **17%** |
| ⭐ **R-Tree** | 5（Q2、Q5、Q8、Q19、Q29） | **17%** |
| **Select / 谓词** | 4（Q4、Q7、Q11、Q24） | 13% |
| **算子框架 / 模块化** | 3（Q1、Q9、Q27） | 10% |
| **HashAggregation** | 3（Q10、Q15、Q16） | 10% |
| **倒排索引** | 3（Q23、Q25、Q30） | 10% |
| **B+ 树** | 2（Q13、Q28）**且两道都是代码题** | 7% |
| Patricia trie | 1（Q20） | 3% |
| DDL/DML、深拷贝、placement new、CreateIndex | 各 1 | 13% |
| ⭐ **教材 ch14 / ch15 / ch18** | ⭐ **0** | ⭐ **0%** |

> 🔴 **两个纠正**：
> 1. **B+ 树被我严重高估**（我给了 ★★★ 最高权重，真实只有 2/30，且都是代码题）。**真正的重头是 Module 8 的高级索引——R-Tree + 学习索引合计 34%。**
> 2. **教材章节一道题都没有。** 我在 `14_期末总复习手册` Part D 里塞的 18 个计算模板，**大半来自 ch15（join 代价、外部排序），按这份证据基本用不上**。留着当保险，但**不要花主要时间**。

## 3.3 ⭐ 场景复用实证（比 Sheet 01 更极端）
- **Select 的 direct/indirect 引用考了 2 次**（Q4、Q7），措辞几乎一样，**答案是同一个知识点的两面**。
- **R-Tree 5 次、学习索引 5 次、HashAggregation 3 次、倒排索引 3 次。**
> 🔑 **复习必须"吃透场景"**：每个核心场景要能从"机制是什么 / 为什么这样设计 / 反过来会怎样 / 局限在哪"四个角度回答。

## 3.4 ⭐⭐ 你的失分模式（跨期中 + Sheet 02，共 24 道错题）

| 根因 | Sheet 02 命中 | 期中命中 | 合计 |
|:---|:---|:---|:--:|
| ⭐ **A. 排除了"平平无奇但正确"的选项** | Q1、Q11、Q18、Q19、Q26、Q29、Q30 | Q4、Q19、Q28 | **10** |
| ⭐ **B. 绝对化词没抓住** | Q17（`eliminates`） | Q6（`always`） | **2** |
| **C. 带公式/定义的形式化陈述没代入验算** | Q30 | Q33 | **2** |
| **D. 跨技术归属混淆** | Q27（模块化 vs 编译） | — | **1** |
| **E. 学习索引知识不牢** | Q3、Q17、Q21、Q26 | — | **4** |
| **F. C++ 机制不熟** | Q6（placement new） | — | **1** |

> 🎯 **【根因 A】以 10 次遥遥领先，是你唯一真正的系统性问题。**
> **它的表现永远是同一个样子：四个选项里有一句"听起来太普通、太像常识"的话，你把它当成了错的；而真正的错误藏在另一句"读起来很顺、但某个方向词翻了车"的话里。**
>
> ### ⭐ 针对根因 A 的强制流程（考场上照做）
> ```
> 反选题不要"找哪个最怪"，而是：
>   ① 逐个选项标 T / F，四个都标完
>   ② 对每个标了 T 的，问："它哪里不对？"——答不上来就是真 T
>   ③ 重点检查每个选项里的【方向词】：
>      first/last、before/after、increase/reduce、perform/avoid、
>      requires/allows、eliminates/reduces、more/fewer、≤/=
>   ④ 只有 F 那个才是答案。若出现两个 F，回去重读题干作用域
>      （规划器层？算子层？数据结构层？）
> ```

---

# 📋 行动清单

| 优先级 | 做什么 | 为什么 |
|:---:|:---|:---|
| **P0** | **把本文档 Part 1 的 14 道错题连每个选项都吃透**，能口述"这句为什么对/错" | 最可能被原样复用且你答不对的题 |
| **P0** | **学习索引专项**：`14_期末总复习手册` §C.2.4 + `精讲版/论文重点_P5-P12` Paper 7 全章 | Sheet 02 错了 4 道 + office hour 出了 2 道 |
| **P0** | **把 §3.4 的反选题四步流程练成条件反射** | 根因 A 占你全部错题的 42% |
| **P1** | Part 2 的 16 道快速过一遍，确认没有"侥幸做对" | 复用时不能丢分 |
| **P1** | **新题风险区**：`module_11`、`module_12`、论文 P5–P12 | 期中的新题全在 Sheet 01 未覆盖处；期末同理 |
| **P2** | R-Tree 因果链（近似→重叠→死空间→多路径→高维退化） | 5/30 的密度 |
| **P3** | ch14/15/18 的计算公式 | Sheet 02 里 0 题，**降到最低优先级** |

---

*本文档基于 `extracted/Excerise_sheet_02.txt`（30 题真题，含你的作答与官方正确答案）逐题分析。所有"正确答案"均取自文件中的 `正确答案：` 标记，非推断。*
