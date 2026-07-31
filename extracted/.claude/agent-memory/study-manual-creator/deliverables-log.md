---
name: deliverables-log
description: 已为各 quiz 交付的复习材料清单（文件位置、覆盖范围、结构约定）
metadata:
  type: project
---

Quiz 2（2026-07-15 交付，目录 `gatech_database_implementation/extracted/`）：
- `Exercise_sheet_02_study_guide.md`：第 0 章出题人分析 + 第 1-10 章考点（M7 B+树 / ch14 索引 / M8 trie·倒排·R-tree·学习索引 / M9-10 算子 / ch15 查询处理 / ch18 并发控制）+ 第 11 章公式速查与 Top20 易错点。
- `Exercise_sheet_02_practice_questions.md`：60 题（Part A-F 按主题分组，★~★★★ 难度标注）+ 完整详解 + 得分自评表。

结构约定（用户已接受，后续沿用）：指南按"出题人分析 → 分章考点(what/why/how+表格+算例+易错点) → 公式速查"组织；练习题与答案同文件、答案区置底；全中文、术语保留英文。论文类材料用 ✅EN(出题人风格英文) + ❌Trap + 💡中文 三段式。

论文重点（2026-07-29 交付，目录 `STUDY_GUIDE/精讲版/`）：
- `论文重点_出题风格英文+中文详解.md`：Paper 1–4。
- `论文重点_P5-P12_出题风格英文+中文详解.md`：Paper 5–12（Bw-Tree/R-Tree/Learned Index/Parallel DB/Volcano/HyPer-LLVM/Vectorwise/C-Store），约 1900 行；Part 0 收录教授 office hour 5 道原题；含 5 张跨论文横向对比表（表 A 执行模型四代演进、表 B 四种索引、表 C 三架构+并行术语对照、表 D 列存三家、表 E 同名异义词）+ 30 道自测题。

期末材料（2026-07-29 交付，三件套）：
- `STUDY_GUIDE/13_期末考试_出题人分析与错题诊断.md`（约 1290 行）：Part 0 80/20 范围划分表；Part 1 Final Practice 7 题逐题推导 + 6 个新信号；Part 2 期中 10 道错题诊断（根因分类 + 6 条应试行为规则）；**Part 3/4 补齐 `module_11`（查询解析 regex + 查询编译）与 `module_12`（列存 + 压缩四算法 + 向量化/SIMD）—— 这两个模块此前所有指南零覆盖**；Part 5 路线图；Part 6 20 题自测。
- ⭐ `STUDY_GUIDE/14_期末总复习手册.md`（约 850 行）：**全范围主线文档，一份看到尾就能上考场**。Part A 考试形态+8 手法+6 条行为规则；Part B 20% 区速通（期中范围，只留高频+可计算）；Part C 80% 区主体（B+树/高级索引/算子/module_11/module_12/ch15 代价/ch18 并发/P5–P12 速记）；⭐ **Part D 计算题弹药库 18 个模板（每个含 模板+算例+半步错误诱饵）**；⭐ **Part E 跨论文归属总表 + 同名异义表**；Part F 易错点 46 条。
- ⭐ `STUDY_GUIDE/15_期末全真模拟_60题.md`（约 685 行）：按期末新比例重建 —— **计算 24 / 跨论文归属 6 / 概念反选 18 / C++ 代码 6 / 短事实 6**；80% 期中后 + 20% 期中范围；卷首写明与上一版的差异表；全部计算题给完整推导 + 诱饵分析；含分 Part 自评表。

⭐ 转录文件编号陷阱：文件 `module_N` ↔ 课程 Module N+2（两个 prep 占 M1/M2）。已有材料里两套编号混用过（00_总索引用课程号，Exercise_sheet_02 用文件号）——**写新材料一律用"文件名 + 主题"做锚点**。
文件号 ↔ 主题：module_1 存储管理、2 槽页、3 缓冲、4 多线程、5 哈希索引、6 线程安全哈希表、7 B+树、8 高级索引、9 算子框架、10 高级算子、11 查询解析与编译、12 列存/压缩/SIMD。
期中范围 = prep01/02 + module_1~6 + ch1/2/12/13 + Paper 1–4；期末 80% 区 = module_7~12 + ch14/15/18 + Paper 5–12。
