---
name: feedback-question-generation
description: 用户反馈"上次出的 60 题基本一道没压中"——命题前必须先校准真题形态，含三条具体修正
metadata:
  type: feedback
---

用户 2026-07-29 反馈：为 Exercise Sheet 02 出的 60 题模拟卷 **"基本上一道题都没压中"**。复盘后确认三个根因，命题前必须先做真题形态校准（读 `extracted/Excerise_sheet_01.txt` 35 题真题 + `Excerise_sheet_02_example.txt` 8 题官方例题 + `final_practice.txt` 7 题）：

1. **计算题严重不足**：我出的只有 ~15%，而官方 Final Practice 是 **3/7 = 43%**。→ 期末卷必须配到 **35–40%**。
2. **题目写得太长太"论文体"**：真题里 **20% 是一句话短事实题**（"Why is DRAM considered volatile memory?" 四个选项各 5–8 词），我几乎全写成多子句的长场景题。→ 长度分布要贴真题。
3. ⭐ **完全低估了 C++/BuzzDB 实现题**：Sheet 01 真题里 **14/35 = 40%** 是围绕具体代码构件的题（unique_ptr 序列化、reinterpret_cast、from_chars vs stoi、std::atomic、vtable、深浅拷贝、槽页、BufferManager 策略指针）。我之前记的"10–15%"严重偏低。→ 期末要用**期中后的代码构件**出题：算子抽象基类、`next()` vs `getOutput()`、谓词 direct/indirect、HashAggregation、regex 解析、编译版查询函数、bit-packing 位运算、SIMD intrinsics。

另外两条真题规律：
- ⭐ **同一场景出多个变体**：Sheet 01 里 BufferManager 策略指针考了 3 次（Q15/Q18/Q24）、槽页 3 次、Enrollments 主键 2 次。→ 核心场景要出 2–3 个不同角度的变体，而不是每个知识点只出一题。
- **会考"超出 transcript 一小步"的业界常识**（PostgreSQL 默认 buffer 128MB 的原因、生产倒排索引用 document-partitioned、DRAM 电容漏电）。→ 每套卷子要留几题这类。

**Why:** 命题不校准真题形态就等于凭空猜，用户为此浪费了一轮复习时间。
**How to apply:** 为该课程出任何模拟卷之前，**先读真题样本统计题型/长度/知识域分布，把统计表写进卷首**，再按分布配题。新卷 `STUDY_GUIDE/15_期末全真模拟_60题.md` 已按此重建（计算 24 / 跨论文归属 6 / 概念反选 18 / C++ 代码 6 / 短事实 6）。

相关：[[exam-question-patterns]] [[user-error-profile]] [[deliverables-log]]
