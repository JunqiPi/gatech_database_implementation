---
name: exam-question-patterns
description: GaTech 数据库课出题人的题型规律（基于 Sheet 01 全卷 35 题 + Sheet 02 全卷 30 题 + Final Practice 7 题 + 期中错题）
metadata:
  type: project
---

## ⭐⭐ 最重要的一条：考试会【原样复用】exercise sheet 的题
用户实证：**期中 35 题里有 10 多道是 Exercise Sheet 01 的原题**。交叉核对后确认：**用户期中的 10 道错题没有一道来自 Sheet 01** —— 即被复用的原题他全对，丢分全在"新题"上。
⇒ **推论**：期末大概率复用 Sheet 02（=期中后内容）的题；期末的**新题**会集中在 **Sheet 02 未覆盖处**（module_11、module_12、论文 P5–P12、教材章节）。
⇒ **复习优先级第一位永远是"把真题 sheet 的每一题每一个选项吃透"**，其次才是新出模拟题。

## 题型分布（真实卷子实测，勿用小样本外推）
| 题型 | Sheet 01(35) | Sheet 02(30) | 合计 65 |
|:---|:---:|:---:|:---:|
| "Which is NOT correct" | 57% | ⭐ **87%** | **71%** |
| C++/BuzzDB 代码 | ⭐ **40%**（与反选题重叠） | 10% | — |
| 短事实题 | 20% | 3% | 12% |
| ⭐ **多步计算** | 11% | ⭐ **0%** | ⭐ **≈5%** |

⚠️ **踩过的坑**：曾用官方 Final Practice 的 7 题（3 题计算=43%）外推出"期末计算题 40%"，据此出了一整套模拟卷 —— **被 Sheet 02 全卷（0 道计算题）推翻**。**Final Practice 这类小样本要理解为"教授在点名新考点的主题"，不是题型分布的样本。**

## 主题密度（Sheet 02 = module_7–10 的实测）
学习索引 17%（5题）｜R-Tree 17%（5题）｜Select/谓词 13%｜算子框架 10%｜HashAggregation 10%｜倒排索引 10%｜**B+树仅 7%（2题，且都是代码题）**｜⭐ **教材 ch14/15/18 = 0 题**。
⇒ **B+树容易被高估**；**Module 8 高级索引（R-Tree+学习索引=34%）才是重头**；⭐ **考试主要考 lecture transcript，不考教材的代价公式**。

## 错误选项的 8 种手法
① 绝对化（must/always/never/eliminates/regardless）② 方向反转（perform↔avoid、first↔last、increase↔reduce、≤↔=）③ 编译期/运行期偷换 ④ 不存在的机制（kernel panic、C++ GC）⑤ 张冠李戴（A 结构性质给 B）⑥ 半真半假（一句话里塞一对一真一假，如"expensive **or** more selective first"）⑦ 🆕 **跨技术/跨论文归属错配**（选项全是真技术，只有归属错；Final Practice Q7、Sheet 02 Q27）⑧ 计算题的半步错误。

## 其他规律
- ⭐ **场景复用极端**：Sheet 01 里 BufferManager 策略指针考 3 次、槽页 3 次；Sheet 02 里 Select 的 direct/indirect 考 2 次（措辞几乎相同）、R-Tree 5 次、学习索引 5 次。**复习要吃透场景（机制/为何这样设计/反过来会怎样/局限），不是背题。**
- **会考"超出 transcript 一小步"的业界常识**（生产倒排索引用 document-partitioned、PostgreSQL 默认 buffer 128MB 的原因、DRAM 电容漏电）。
- **反选题里"一个词翻转全句"的位置极多**：first/last、perform/avoid、≤/=、increase/reduce。

**How to apply:** 出题前先读真题样本做形态统计并写进卷首；配题按上表比例；每题的错误项从 8 手法中选一种；核心场景出 2–3 个变体。详见 [[feedback-question-generation]]。
