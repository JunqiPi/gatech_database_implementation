# ch2 — Relational Model（关系模型）精讲

> **模板**：每个知识点 = **【原句】**(章节英文原句) → **【侧重点 & 知识点】**(🔬英文 + 💡中文易懂)。
> ch2 对应 **Module 1**。考点:关系的性质、键(四层)、关系代数(σ Π ∪ − × ρ + join + γ)。

---

## 1. 关系的组成（relation / tuple / attribute / domain）

**【原句】** *"R = (A1, A2, …, An) is a relation schema... An element t of relation r is called a tuple... The set of allowed values for each attribute is called the domain of the attribute."*

**【侧重点 & 知识点】**
- 🔬 **EN:** A **relation** = a table. A **tuple** = a row. An **attribute** = a column. A **domain** = the allowed value set for an attribute; values are normally **atomic** (indivisible).
- 💡 **中文:** relation(关系)=表,tuple(元组)=行,attribute(属性)=列,domain(域)=一列允许的取值范围。值要**atomic(原子,不可再分)**——一个格子不能塞一个列表。🔵 就是 Excel:表/行/列。

---

## 2. 关系是"集合" → 无序、不重复

**【原句】** *"Order of tuples is irrelevant (tuples may be stored in an arbitrary order)."* （及关系的性质:所有行互不相同）

**【侧重点 & 知识点】**
- 🔬 **EN:** A relation is a **set** of tuples → **order of rows is irrelevant** and **all rows are distinct** (no duplicates).
- 💡 **中文:** 关系是"集合",所以**行的顺序无所谓**、**行不重复**。🔵 类比:一个班的学生名单,谁先谁后无所谓,但不会有两个一模一样的人。

---

## 3. null 值

**【原句】** *"The special value null is a member of every domain. Indicated that the value is 'unknown'. The null value causes complications in the definition of many operations."*

**【侧重点 & 知识点】**
- 🔬 **EN:** `null` means "unknown/missing"; it belongs to every domain and **complicates many operations** (comparisons, aggregates).
- 💡 **中文:** null="未知/没填",属于每个域;它会让很多运算变复杂(比如 null 和任何值比较都不是 true/false 而是 unknown)。

---

## 4. 键的四个层次（Keys）⭐

**【原句】** *"K is a superkey of R if values for K are sufficient to identify a unique tuple... Superkey K is a candidate key if K is minimal... One of the candidate keys is selected to be the primary key... Foreign key constraint: Value in one relation must appear in another."*

**【侧重点 & 知识点】**
- 🔬 **EN:** Four levels:
  - **superkey** — a set of attributes that uniquely identifies a tuple (may have extra/redundant attributes).
  - **candidate key** — a **minimal** superkey (remove any attribute and it's no longer unique).
  - **primary key** — the one candidate key chosen as the main identifier.
  - **foreign key** — an attribute whose values must appear as the primary key of another relation.
- 💡 **中文:**
  - **superkey(超键)**:能唯一认出一行的列组合,但可能带多余列(如 `{ID, name}`,光 ID 就够)。
  - **candidate key(候选键)**:**最小**的超键(去掉任一列就不唯一,如 `{ID}`)。
  - **primary key(主键)**:从候选键里**选一个**当代表(身份证号)。
  - **foreign key(外键)**:它的值必须是另一张表的主键(指过去)。
- 🔑 **关系链**:superkey ⊇ candidate key → 选一个 = primary key;foreign key 跨表引用。

---

## 5. 关系代数:六个基本运算 + 连接 + 聚合 ⭐

**【原句】** *"Six basic operators: select σ, project Π, union ∪, set difference −, Cartesian product ×, rename ρ."*

**【侧重点 & 知识点】**
- 🔬 **EN:** Relational algebra operators take relation(s) → produce a relation, so they **compose**:

| 符号 | 名字(EN) | 名字(中文) | 干嘛 | SQL |
|:---:|:---|:---|:---|:---|
| **σ** | select | 选择 | **挑行**(留满足条件的行) | WHERE |
| **Π** | project | 投影 | **挑列**(留某几列,去重) | SELECT 列 |
| **∪** | union | 并 | 两表行合并 | UNION |
| **−** | set difference | 差 | 在 A 不在 B 的行 | EXCEPT |
| **×** | Cartesian product | 笛卡尔积 | 每行配每行 | CROSS JOIN |
| **ρ** | rename | 改名 | 给表/列改名 | AS |
| **⋈** | join | 连接 | 按条件拼两表 | JOIN ON |
| **γ** | aggregation | 分组聚合 | 分组+统计 | GROUP BY + SUM/COUNT |

- 💡 **中文:** 每个运算吃表、吐表,所以能像乐高一样层层组合(这就是 SQL 的威力来源)。
- ⚠️ **最易混(考点)**:**σ(select 选择)挑行 = SQL 的 WHERE**;**Π(project 投影)挑列 = SQL 的 SELECT 列**。别被 SQL 里 `SELECT` 这个词骗了——它其实是投影 Π!

---

## 6. 运算可组合 + 连接是怎么来的

**【原句】** *"σ instructor.id = teaches.id (instructor × teaches)"* —— 即:笛卡尔积 + 选择 = 连接。

**【侧重点 & 知识点】**
- 🔬 **EN:** **Cartesian product (×)** pairs every row with every row (huge, mostly meaningless). Adding a **selection (σ)** with a matching condition gives a meaningful **join (⋈)**: `σ_{A.id=B.id}(A × B) = A ⋈ B`.
- 💡 **中文:** 笛卡尔积 A×B 把每行和每行都配一遍(超大、大多无意义);加一个 σ 筛选条件(只留 id 相等的),就变成有意义的**连接 ⋈**。🔵 连接 = 笛卡尔积 + 筛选。

---

## 7. 聚合 + 分组（Aggregate / Group By）

**【原句】** *"γavg(salary)(instructor)... dept_name γ avg(salary)(instructor)"* —— 聚合函数 avg/min/max/sum/count;可按列分组。

**【侧重点 & 知识点】**
- 🔬 **EN:** Aggregate functions (**avg, min, max, sum, count**) compute one value over a column. With a **group-by** subscript, they aggregate per group: `_{dept}γ_{avg(salary)}(instructor)` = average salary per department.
- 💡 **中文:** 聚合函数(平均/最小/最大/求和/计数)把一列算成一个值;加分组下标就"按组算"。🔵 例:`按部门 γ 平均(薪资)` = 每个部门的平均薪资。对应 SQL 的 `GROUP BY dept` + `AVG(salary)`。

---

## 8. 等价查询（Equivalent Queries）

**【原句】** *"There is more than one way to write a query in relational algebra... they are equivalent — they give the same result on any database."*

**【侧重点 & 知识点】**
- 🔬 **EN:** The same query can be written multiple ways that are **equivalent** (same result), e.g., `σ_{A∧B}(r) ≡ σ_A(σ_B(r))`. This is the basis of **query optimization** (the optimizer picks the cheapest equivalent form).
- 💡 **中文:** 同一个查询有多种写法、结果一样(如把两个条件合并 vs 分两步筛)。**查询优化器**就靠这个——挑最省的那种写法执行。

---

## ✅ ch2 速记
1. relation=表、tuple=行、attribute=列、domain=取值范围;值要原子。
2. 关系是集合 → **行无序、不重复**。
3. 键四层:**superkey ⊇ candidate key →(选一个)primary key**;**foreign key** 跨表引用。
4. 关系代数:**σ 挑行(WHERE)、Π 挑列(SELECT 列)**、∪/−/×/ρ、⋈ 连接、γ 分组聚合。
5. **× + σ = ⋈**;同一查询多种等价写法 → 查询优化。

> 📌 配套:M1 精讲(关系代数大白话)、Paper 1(Codd)。
