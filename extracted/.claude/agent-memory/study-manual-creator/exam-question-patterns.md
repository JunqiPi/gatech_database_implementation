---
name: exam-question-patterns
description: GaTech 数据库课程 quiz/期末出题人的题型规律（Sheet 01 真题 35 题 + Sheet 01/02 例题 + Final Practice 7 题 + 期中真实错题）
metadata:
  type: project
---

题型结构（quiz/期中）：~55-60% "Which is NOT correct"（3 真 1 假）；~20% "correct/best describes"；~15% 多步计算；~10-15% C++ 代码题。全部单选，多为 4 选项。

⭐ **期末（final）的比例明显不同**，依据 `extracted/final_practice.txt` 官方 7 题：
- **多步计算暴涨到 ~30-40%**（Final Practice 3/7：B+树层数、双重哈希走位、Volcano vs 向量化吞吐比）；NOT correct 降到 ~45-50%。
- 🆕 **新题型「执行模型量化对比」**：给 ns 级成本模型算加速比。核心洞察 = **只有控制开销被批量摊薄，计算开销不变 → 加速比上限 = 总成本/计算成本**。
- 🆕 **新题型「跨论文张冠李戴」**：问"以下哪项**不是**论文 X 的核心技术"，错误项是**真实存在但属于另一篇论文**的技术（Final Practice Q7：把 HyPer 的查询编译安给 C-Store）。四个选项全是学过的真东西，只有归属错——需要专门背归属表。
- **C++ 题从"语法/RAII"转向"正确性陷阱"**（Final Practice Q5 = 二分查找 `(l+h)/2` 整数溢出，即 JDK binary search bug）。
- **20% 的"期中已考"部分，优先挑能出计算题的 topic**（Final Practice Q3 双重哈希属期中范围）。
- 高危归属配对表 + 7 题逐题推导写在 `STUDY_GUIDE/13_期末考试_出题人分析与错题诊断.md` Part 1。

错误选项 6 手法（识别信号）：① 绝对化（must/always/immediately/never）② 方向反转（深拷贝"共享内存"）③ 编译期/运行期偷换（`=0` "编译期绑定"）④ 不存在的机制（kernel panic）⑤ 张冠李戴（R-tree "中序遍历"——那是 B+ 树的）⑥ 半真半假（mutex "保证公平"）。

计算题诱饵 = "半步错误"：B+ 树计数把根的键数按孩子数算；外部排序漏 "+1"；位图比例套错记录大小。

其他规律：题库按场景组织、同场景出变体（背题无用，吃透场景）；会考超出 transcript 一小步的业界常识（例题 8：transcript 教 term-partitioned，答案是生产主流 document-partitioned）；C++ 考点每卷 4-6 题（RAII/unique_ptr、vtable、深浅拷贝、模板编译期实例化、lower_bound/upper_bound、序列化指针→偏移量）。

**Why:** 完整分析写在 `Exercise_sheet_02_study_guide.md` 第 0 章；此处存跨 quiz 可复用的精华。
**How to apply:** 为该课程出模拟题时按此比例配题型，每题的错误项从 6 手法中选一种制造；计算题选项必须包含半步错误诱饵。
