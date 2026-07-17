# Exercise Sheet 02 — Practice Questions (60)

> **Format**: Questions are in **English** to match the real exam. Each question has a one-line 中文题意 (🀄) underneath for quick comprehension. Detailed Chinese explanations for every question are at the end of this file.
> **Difficulty**: ★ basic / ★★ intermediate / ★★★ challenge. Work out the calculation questions on paper before choosing.
> **How to use**: Finish an entire Part before checking answers. All questions are single-choice.

---

## Part A: B+ Trees (Q1–Q12) | Module 7 + Ch14

**Q1. (★)** Which statement about point queries and range queries is **NOT** correct?

A. A point query retrieves all tuples where a column equals a given value; a hash index can answer it with an average O(1) lookup.
B. Range queries are slow on a hash table primarily because computing the hash function is expensive.
C. A range query retrieves all tuples where a column falls within a specified interval.
D. Processing a range query with a hash table requires examining every slot and checking whether each key falls in the range.

🀄 关于点查询与范围查询的说法，哪项不正确？

**Q2. (★)** Which statement about B+ tree node structure is **NOT** correct?

A. Inner nodes store separator keys and child pointers; K child pointers go with K−1 keys.
B. In a leaf node, the number of keys equals the number of values.
C. Actual data (e.g., tuple IDs) lives only in leaf nodes; keys in inner nodes may duplicate keys in leaves.
D. The main purpose of the sibling pointers linking leaf nodes is to speed up exact-match point queries.

🀄 关于 B+ 树节点结构（内部节点 vs 叶子、兄弟指针的用途），哪项不正确？

**Q3. (★★★)** A B+ tree of height 3 (root → inner level → leaf level). The root has 5 children; the 5 inner nodes have 4, 3, 6, 2, and 5 children respectively; every leaf holds exactly 4 records. What is (total internal separator keys, total leaf records)?

A. (19, 80)
B. (20, 80)
C. (19, 76)
D. (24, 80)

🀄 计数题：给出各层孩子数，求（内部分隔键总数，叶子记录总数）。

**Q4. (★★)** A B+ tree holds at most n = 200 pointers per node, and the file contains K = 10⁸ search-key values. According to the textbook height bound, at most how many nodes are accessed on a root-to-leaf lookup?

A. 3
B. 4
C. 8
D. 27

🀄 用教材高度上界公式 ⌈log⌈n/2⌉(K)⌉ 计算一次查找最多访问几个节点。

**Q5. (★★)** BuzzDB's on-disk B+ tree node: each key is 4 bytes, each value 4 bytes, 4 bytes reserved for one extra value, and 9 bytes of node metadata (node size = 8m + 13 bytes). With an 8 KB (8192-byte) page, how many keys fit in one node at most?

A. 510
B. 1022
C. 1024
D. 2046

🀄 用 8m+13 公式算 8KB 页最多装多少个键。

**Q6. (★★)** Which statement about node splits during B+ tree insertion is **NOT** correct?

A. After a leaf split, the smallest key of the new node is inserted into the parent as a separator, and that key also remains in the leaf.
B. When an inner node splits, the median key is promoted to the parent while also remaining in the original inner node.
C. When a split propagates to the root, a new root with a single key and two children is created, increasing the tree height by one.
D. A single insertion can trigger splits that propagate all the way from a leaf to the root.

🀄 关于插入引发的分裂（叶子"抄"vs 内部"搬"、根分裂长高），哪项不正确？

**Q7. (★★)** Which statement about B+ tree deletion is **NOT** correct?

A. If an underfull node and a sibling together fit into one node, they are merged, and the corresponding separator key and pointer are removed from the parent.
B. When entries are redistributed (borrowed) from a sibling, the corresponding separator key in the parent must be updated.
C. Rebalancing caused by a deletion propagates at most one level up and can never reach the root.
D. If the root is left with only one child, the root is removed, its only child becomes the new root, and the height decreases by one.

🀄 关于删除（合并、借用改路标、级联到根），哪项不正确？

**Q8. (★★)** For a B+ tree with n = 8 (at most 8 pointers per node), which node **violates** the textbook occupancy rules?

A. A leaf holding 4 values
B. A non-root internal node with 3 children
C. A root with only 2 children
D. A leaf holding 7 values

🀄 n=8 时哪个节点违反"最少/最多"占用率规则？

**Q9. (★★)** A disk-based B+ tree of height 3 (root, inner level, leaf level; one node per page). The root and the entire inner level are cached in the buffer pool; no leaves are cached. A range query executed the efficient way touches exactly 7 consecutive leaves. How many disk page reads does the query need?

A. 7
B. 9
C. 50
D. 10

🀄 顶两层已缓存，范围查询命中 7 个叶子，实际读几次盘？

**Q10. (★★)** Which statement about serializing an in-memory B+ tree to disk is **NOT** correct?

A. In-memory node pointers must be converted to file offsets during serialization.
B. Writing the numeric value of a `Node*` pointer to disk is sufficient: after a restart, the system can use that value to rebuild the tree.
C. For variable-length key/value types (e.g., strings), extra length metadata must be stored in the byte stream.
D. Nodes are stored at offsets that are multiples of the page size, so any node can be accessed randomly without reading intervening nodes.

🀄 关于序列化（指针→偏移量、变长类型、随机访问），哪项不正确？

**Q11. (★★)** Which statement about B-trees vs B+ trees is **NOT** correct?

A. In a B-tree, every search key appears only once, eliminating redundant key storage.
B. A B-tree lookup may terminate early at an internal node before reaching a leaf.
C. B-tree internal nodes are larger because they carry record pointers, reducing fanout and typically making the tree deeper.
D. Because lookups are faster and the implementation is simpler, B-trees are more widely used in practice than B+ trees.

🀄 B-tree 与 B+ 树对比，哪项不正确？

**Q12. (★)** Which statement about dense and sparse indices is **NOT** correct?

A. A dense index contains an index entry for every search-key value that appears in the file.
B. A sparse index is applicable only when the file is stored sequentially ordered (clustered) on the search key.
C. A secondary index can be made sparse to save space and maintenance overhead.
D. A good tradeoff for a clustering index is a sparse index with one entry per disk block.

🀄 稠密/稀疏索引与二级索引的适用条件，哪项不正确？

---

## Part B: Hashing, LSM Trees, Bitmap & Spatial Indices (Q13–Q20) | Ch14

**Q13. (★)** Which statement about static hashing is **NOT** correct?

A. A bucket is typically one disk block; entries within a bucket still require a sequential scan.
B. Bucket overflow is handled by chaining overflow buckets, a scheme called closed addressing.
C. Open addressing is widely adopted by database systems because deletions are efficient.
D. Skew (many records sharing a key, or a non-uniform hash function) causes overflow, which can be reduced but not eliminated.

🀄 关于静态哈希（桶、溢出链、开放寻址），哪项不正确？

**Q14. (★★)** Which statement about dynamic hashing and index choice in practice is **NOT** correct?

A. Linear hashing performs rehashing incrementally, avoiding a one-shot full reorganization.
B. Extendable hashing can double the number of hash-table entries without doubling the number of buckets.
C. If range queries are common, ordered indices should be preferred over hashing.
D. PostgreSQL officially recommends hash indices over B+ trees for better equality-query performance.

🀄 动态哈希与实际系统的索引选型，哪项不正确？

**Q15. (★★)** Which statement about LSM trees is **NOT** correct?

A. Inserts first go to the in-memory L0 tree; when L0 fills, it is merged into the on-disk L1 tree using a bottom-up build.
B. LSM disk writes are sequential I/O, and the resulting leaves are nearly full.
C. On a delete, the system synchronously locates the entry in every level and physically removes it.
D. A single point lookup may need to search multiple trees.

🀄 关于 LSM 树（写入路径、顺序 I/O、删除标记、多树查找），哪项不正确？

**Q16. (★★★)** Which statement about LSM variants and buffer trees is **NOT** correct?

A. The stepped-merge variant keeps multiple trees per level, making writes cheaper but queries more expensive.
B. Bloom filters can be used to skip lookups in most trees that cannot contain the key.
C. A buffer tree attaches a buffer to each internal node of a B+ tree; inserts accumulate and are pushed down in batches.
D. Buffer trees incur less random I/O than LSM trees, which is why BigTable and Cassandra chose them as their core storage structure.

🀄 stepped-merge、Bloom filter、buffer tree 的对比，哪项不正确？

**Q17. (★★)** A relation has 64 million records of 128 bytes each. A bitmap index is built on an attribute with 2 distinct values. How large is one bitmap, and what is its size relative to the relation?

A. 8 MB; 1/1024
B. 64 MB; 1/128
C. 8 MB; 1/800
D. 16 MB; 1/512

🀄 计算题：6400 万条 ×128B，一张位图多大？占关系的几分之一？

**Q18. (★)** Which statement about bitmap indices is **NOT** correct?

A. Bitmap indices suit attributes with few distinct values (gender, state, income bracket).
B. Multi-attribute conditions are answered efficiently with bitwise AND/OR between bitmaps.
C. Bitmap indices are especially effective for equality queries on a single high-cardinality attribute.
D. Counting matching tuples is even faster than actually fetching them.

🀄 位图索引的适用场景，哪项不正确？

**Q19. (★★)** Which statement about spatial index structures is **NOT** correct?

A. A k-d tree splits the point set roughly in half at each level, cycling through the dimensions.
B. Each non-leaf quadtree node divides its region into four equal quadrants.
C. In an R-tree, the bounding boxes of children of the same node are never allowed to overlap.
D. An R-tree query may follow multiple paths in the worst case but performs acceptably in practice.

🀄 k-d 树 / 四叉树 / R-tree 的划分方式，哪项不正确？

**Q20. (★★)** A B+ tree index exists on the composite key (dept_name, salary) of *instructor*. Which query can **NOT** be supported efficiently by this index?

A. `WHERE dept_name = 'Finance' AND salary = 80000`
B. `WHERE dept_name = 'Finance' AND salary < 80000`
C. `WHERE dept_name < 'Finance' AND salary = 80000`
D. `WHERE dept_name = 'Finance'`

🀄 复合索引 (dept_name, salary) 支持不了哪种查询？

---

## Part C: Tries, Inverted Indexes, R-Trees, Learned Indexes (Q21–Q33) | Module 8

**Q21. (★)** Which statement about tries and prefix queries is **NOT** correct?

A. Storing string keys in a B+ tree means long keys are stored at multiple levels, and every node traversal compares long, similar keys — ill-suited for prefix queries.
B. In a trie, the path from the root to a terminal node spells out one inserted string.
C. The term "trie" comes from the word "retrieval".
D. A prefix search in a trie must traverse every branch of the whole tree to collect the matches.

🀄 trie 与前缀查询的基本性质，哪项不正确？

**Q22. (★★)** Which statement about Patricia trie structure and insertion is **NOT** correct?

A. Each node holds a boolean end-of-word flag, an optional value, and a map from substrings to child nodes.
B. Word termination is marked by a flag on the node; no extra edge is added for an empty suffix.
C. During insertion, if the common prefix between the remaining string and a child edge label is shorter than that label, the edge must be split.
D. Compared with a plain trie, a Patricia trie has more nodes, but since each node is smaller, operations are faster.

🀄 Patricia trie 的节点结构与插入规则，哪项不正确？

**Q23. (★★★)** A Patricia trie already contains {"car", "cart", "card", "care", "cat"} (edges store substrings; terminals are node flags; no empty-suffix edges): root → "ca" → { "r"(end) → {"t"(end), "d"(end), "e"(end)}, "t"(end) }. Now insert {"cargo", "cab"}. Counting only NEW nodes and ADDITIONAL edge-label characters, what is (Δnodes, Δchars)?

A. (2, 3)
B. (2, 5)
C. (3, 3)
D. (2, 8)

🀄 计数题：插入 cargo 和 cab 后，新增节点数与新增字符数。

**Q24. (★★★)** A Patricia trie contains {"in", "inn", "input"}: root → "in"(end) → { "n"(end), "put"(end) }. Insert {"inp", "into"}. What is (Δnodes, Δchars)?

A. (2, 0)
B. (2, 2)
C. (3, 2)
D. (2, 4)

🀄 计数题（含分裂）：插入 inp 和 into 后的 (Δnodes, Δchars)。

**Q25. (★★)** Which comparison between Patricia tries and B+ trees is **NOT** correct?

A. Patricia trie insert/search cost is O(L) (L = average key length), independent of the number of keys N.
B. B+ tree insert/search cost depends logarithmically on the total number of keys.
C. When keys are highly dissimilar, a Patricia trie can be less memory-efficient than a B+ tree.
D. A Patricia trie cannot traverse its nodes in lexicographic order, so it cannot process range queries at all.

🀄 Patricia trie vs B+ 树的复杂度与能力对比，哪项不正确？

**Q26. (★)** Which statement about the inverted index structure and its core functions is **NOT** correct?

A. An inverted index supporting a document library maps each word to (document ID → positions of the word in that document).
B. BuzzDB's C++ implementation uses a nested map: the outer key is a word, the inner key is a document number, and the value is a vector of positions.
C. addDocument lowercases each word before indexing to support case-insensitive matching.
D. getDocuments returns the full position lists of the given word in every document.

🀄 倒排索引的结构与三个核心函数，哪项不正确？

**Q27. (★★)** An inverted index records: doc1 has "storm"@[4,18,40] and "surge"@[11,25,52]; doc2 has "storm"@[7] and "surge"@[30]; doc3 has "storm"@[100,120] and "surge"@[126]. Which documents match proximitySearch("storm", "surge", k=8) (the two words within absolute position distance ≤ 8 in the same document)?

A. Only doc1
B. doc1 and doc3
C. doc1, doc2 and doc3
D. Only doc3

🀄 计算题：邻近搜索 k=8，哪些文档命中？

**Q28. (★★)** Which statement about inverted-index storage optimization is **NOT** correct?

A. Delta encoding turns position lists into fixed-length codes, enabling O(1) random access to any position.
B. Recording the position of every occurrence of every word leads to a large storage footprint.
C. Delta encoding stores differences between consecutive positions; the small deltas can be encoded with fewer bits.
D. The position list [1000, 1006, 1009, 1030] delta-encodes to [1000, 6, 3, 21].

🀄 关于 delta encoding 压缩，哪项不正确？

**Q29. (★★★)** Which statement about sharding a web-scale inverted index is **NOT** correct?

A. In document partitioning, each shard builds a complete local inverted index over its documents; queries are broadcast to all shards and merged by a coordinator.
B. In term partitioning, all postings for a term live on one machine, so a single-term query contacts only one shard.
C. Term partitioning makes index updates simple, because each new document affects only one shard.
D. Replication keeps multiple copies of each shard for load balancing and fault tolerance.

🀄 document-partitioned vs term-partitioned 分片，哪项不正确？

**Q30. (★★)** Which statement about R-tree insertion and node splitting is **NOT** correct?

A. The quadratic split picks the two points **closest** to each other as seeds, keeping the two new nodes compact.
B. Insertion descends by choosing, at each level, the child whose bounding rectangle needs the least enlargement to include the new point.
C. During a split, the remaining points are assigned to the side whose bounding rectangle grows the least.
D. Like a B+ tree, splits and merges keep the R-tree balanced, giving logarithmic search time in the number of points.

🀄 R-tree 插入与 quadratic split（种子怎么选），哪项不正确？

**Q31. (★★)** Which statement about R-tree query algorithms is **NOT** correct?

A. A range query recurses only into children whose bounding rectangles intersect the query rectangle.
B. A kNN query computes the minimum distance from the query point to each child's bounding rectangle and recurses in nearest-first order.
C. A range query must visit every leaf node to guarantee that no results are missed.
D. A kNN query maintains a distance-ordered priority queue and pops the farthest point whenever the queue size exceeds k.

🀄 R-tree 范围查询与 kNN 的算法细节，哪项不正确？

**Q32. (★★★)** A learned index fits a linear regression: keys [100, 200, 300] at positions [1, 3, 5]. After least-squares training, what position does the model predict for key 250?

A. 3
B. 3.5
C. 4
D. 5

🀄 计算题：最小二乘训练后，键 250 的预测位置是多少？

**Q33. (★★)** Which statement about learned indexes is **NOT** correct?

A. In a recursive model index, the root model picks a sub-model based on the key, and the sub-model predicts the key's approximate position within its data cluster.
B. Because predictions are approximate, lookups perform a neighborhood search of a given radius around the predicted position.
C. As long as a key exists in the index, the neighborhood search is guaranteed to find it.
D. Learned indexes reduce memory usage by eliminating large numbers of node pointers.

🀄 学习索引（RMI、邻域搜索、根本局限），哪项不正确？

---

## Part D: Query Execution & Operators (Q34–Q45) | Modules 9–10

**Q34. (★)** Which statement about moving from hard-coded queries to an operator framework is **NOT** correct?

A. Flexibility: operators can be combined in different configurations for different queries without modifying the operators themselves.
B. Isolation: changes to one operator do not affect others, simplifying debugging and optimization.
C. Reusability: implement common operators once and reuse them across queries, reducing redundancy and bugs.
D. Hard-coded query functions, by eliminating inter-operator call overhead, are always easier to maintain and extend than operator pipelines.

🀄 算子框架相对硬编码查询的好处，哪项不正确？

**Q35. (★★)** Which statement about the abstract operator interface (open / next / getOutput / close) is **NOT** correct?

A. open() initializes the operator, setting up the state and resources needed for data retrieval.
B. next() advances to the next tuple and returns true if one is available.
C. getOutput() returns the fields of the current tuple and automatically advances to the next tuple.
D. close() cleans up resources, leaving the system in a clean state.

🀄 四个接口方法各自的职责，哪项不正确？

**Q36. (★)** Which statement about the Scan operator is **NOT** correct?

A. Scan fetches table pages through the buffer manager.
B. Scan maintains the current page index and slot index to iterate over all tuples in a structured way.
C. When the current page is exhausted, Scan advances to the next page; when all pages are scanned, it clears the current tuple to mark the end.
D. For speed, Scan first loads the entire table into memory before emitting any tuple to its parent.

🀄 Scan 算子的工作方式，哪项不正确？

**Q37. (★★)** BuzzDB declares `class IPredicate { public: virtual bool check(...) = 0; };`. Which statement is **NOT** correct?

A. A class with a pure virtual function is abstract and cannot be instantiated directly.
B. Calling check through an `IPredicate*` dispatches at runtime via the vtable to the actual object's implementation.
C. Declaring check as `= 0` lets the compiler bind the call statically at compile time, eliminating virtual-call overhead.
D. SimplePredicate and ComplexPredicate must each implement check to become instantiable concrete classes.

🀄 纯虚函数与运行期分发，哪项不正确？

**Q38. (★)** Which statement about unary and binary operators is **CORRECT**?

A. Select is a binary operator because it has one input table and one output table.
B. HashAggregation is a unary operator; a join is a binary operator holding input_left and input_right.
C. Scan is a unary operator whose input operator is the buffer manager.
D. The framework requires every operator to have exactly one input operator.

🀄 一元/二元算子的分类，哪项正确？

**Q39. (★★)** BuzzDB predicate operands are **direct** (a stored constant) or **indirect** (a column index into the tuple). To express `start_time < end_time` (comparing two columns of the same tuple) and `price < 100` (column vs constant), the correct combination is:

A. The former uses two indirect operands; the latter uses one indirect operand (the price column) plus one direct operand (the constant 100).
B. Both predicates need only direct operands.
C. The former uses direct operands; the latter uses indirect ones.
D. Indirect operands are replaced by constant values at predicate-construction time, so the two kinds are equivalent.

🀄 direct / indirect 操作数分别用在哪，哪个搭配正确？

**Q40. (★★)** A Field class's copy constructor performs a **shallow** copy (copying only the data pointer). After:

```cpp
Field original("Hello");
Field copy(original);
copy.data[0] = 'J';
```

What does `original.data` contain, and what additional risk exists?

A. "Hello"; no other risk.
B. "Jello"; and when both objects are destroyed, the same memory may be freed twice (double-free).
C. "Hello"; but the modification to copy is lost.
D. "Jello"; but there is a risk only if delete is called explicitly.

🀄 浅拷贝后改副本，原件变成什么？还有什么隐患？

**Q41. (★★)** Tuple fields = [5, 12, 7] (column indexes 0, 1, 2). Complex predicate P = (field0 > 2 **AND** field1 < 10) **OR** (field2 == 7). What does P evaluate to?

A. true
B. false
C. It depends on the evaluation order of the sub-predicates
D. Type error; it cannot be evaluated

🀄 计算题：复合谓词对元组 [5,12,7] 的求值结果。

**Q42. (★★)** Which statement about the HashAggregation operator is **NOT** correct?

A. With multi-column grouping, the group key is a vector of fields, which requires a dedicated hasher for field vectors.
B. Per-field hashes are combined with XOR and bit shifts so hash values spread evenly and collisions are reduced.
C. Each group's aggregate is updated dynamically as input tuples are processed (updateAggregate first checks type consistency).
D. As soon as the first tuple of a group arrives, that group's final aggregate can immediately be emitted to the parent.

🀄 哈希聚合（多列组键、动态更新、阻塞性质），哪项不正确？

**Q43. (★)** Table sales(category, profit) contains, in order: (1,500), (2,1000), (1,300), (2,2000), (3,50). HashAggregation groups by category and SUMs profit. What is the output?

A. 3 tuples: (1,800), (2,3000), (3,50)
B. 5 tuples, one per input tuple
C. 2 tuples: (1,800), (2,3000)
D. 1 tuple: (6,3850)

🀄 计算题：按 category 分组求和的输出。

**Q44. (★)** Which statement about the analogy between relational operators and Unix pipes is **NOT** correct?

A. Unix tools' "text in, text out" interface corresponds to operators' "table in, table out" interface.
B. Unix and relational databases were both born in the internet boom of the 1990s.
C. grep filters lines containing a string, playing a role similar to the Select operator.
D. `cat file | grep error | sort | uniq -c` corresponds to a pipeline of scan, filter, sort, and group-count operators.

🀄 Unix 管道类比，哪项不正确？

**Q45. (★★)** Which statement about the CreateIndex operator is **NOT** correct?

A. The column offset specifies which tuple column the index is built on (e.g., salary at offset 1).
B. The index is built entirely in open(): iterate over all tuples of the child operator, extract keys, and insert them into the hash index.
C. Each call to next() returns one freshly indexed tuple.
D. The built index is exposed via getIndex() for other operators to use; CREATE INDEX is DDL, not DML.

🀄 CreateIndex 算子（open 里建完、next 为空），哪项不正确？

---

## Part E: Query Processing & Cost Model (Q46–Q53) | Ch15

**Q46. (★)** Which statement about measuring query cost is **NOT** correct?

A. Disk cost can be estimated as: number of block transfers × tT + number of seeks × tS.
B. The textbook uses total resource consumption rather than response time, since response time is hard to estimate and lowering resource use is better in a shared system.
C. On a high-end magnetic disk, seek time (≈4 ms) far exceeds the transfer time of a 4 KB block (≈0.1 ms).
D. The textbook's operator cost formulas include the cost of writing the final result back to disk.

🀄 代价模型的设定（tT/tS、是否计输出写盘），哪项不正确？

**Q47. (★★★)** A relation occupies 1000 blocks. An equality selection on a non-key attribute via a **secondary** B+ tree index of height h=3 locates n=500 matching records scattered across different blocks. With tT=0.1 ms and tS=4 ms, comparing the secondary index scan (A4) and a linear scan (A1):

A. The index scan takes about 2 ms, far faster than the linear scan.
B. Index scan ≈ (3+500)×4.1 ≈ 2062 ms; linear scan ≈ 4 + 1000×0.1 = 104 ms; the linear scan is about 20× faster.
C. Index scan ≈ 503 ms, slightly faster than the linear scan.
D. The two costs are about the same.

🀄 计算题：二级索引取 500 条散布记录 vs 全表线性扫描，谁快？

**Q48. (★★)** Which statement about PostgreSQL's bitmap index scan is **NOT** correct?

A. It maintains a bitmap with one bit per **page** of the relation.
B. It first performs an index scan setting the bit of each page containing a match, then linearly reads all flagged pages in physical order.
C. With few matches it behaves like an index scan; with many matches, like a linear scan; it never performs much worse than the better of the two.
D. It fetches tuples one by one in index-key order, so its output is naturally sorted.

🀄 bitmap index scan 的机制，哪项不正确？

**Q49. (★★★)** External sort-merge on a relation of br = 4000 blocks, with M = 40 memory pages and bb = 4 buffer blocks per run. How many initial runs, merge passes, and total block transfers (final write not counted)?

A. 100 runs; 2 passes; 20,000
B. 100 runs; 3 passes; 28,000
C. 100 runs; 3 passes; 24,000
D. 40 runs; 3 passes; 28,000

🀄 计算题：外部排序的 run 数、趟数、总块传输。

**Q50. (★★)** r: nr=2000 tuples, br=80 blocks; s: ns=8000 tuples, bs=320 blocks. Worst-case memory (one buffer block per relation). A block nested-loop join with r as the outer relation costs:

A. 25,680 block transfers; 160 seeks
B. 640,080 block transfers (nr×bs+br)
C. 25,680 block transfers; 80 seeks
D. 400 block transfers; 2 seeks

🀄 计算题：BNLJ 以 r 为外表的代价。

**Q51. (★★)** Same r and s as Q50. Now s has a B+ tree index on the join attribute; each index lookup plus record fetch costs c=4. Indexed nested-loop join (r outer) vs the BNLJ of Q50:

A. Indexed NL ≈ 80 + 2000×4 = 8,080, better than BNLJ's 25,680.
B. BNLJ always beats indexed NL because it works block-at-a-time.
C. Indexed NL ≈ 320 + 8000×4 = 32,320, worse than BNLJ.
D. The two cost the same.

🀄 计算题：indexed NL vs BNLJ，谁便宜？

**Q52. (★★)** Which statement about hash join is **NOT** correct?

A. The smaller relation should be the build input, so that each build partition fits in memory.
B. Without recursive partitioning, the cost is about 3(br+bs) block transfers (plus minor partially-filled-block overhead).
C. Each partition of the probe relation must also fit entirely in memory, otherwise the algorithm cannot work.
D. The number of partitions n is typically about ⌈bs/M⌉ × 1.2 (a fudge factor).

🀄 hash join（build 选小表、3(br+bs)、probe 分区是否须装入内存），哪项不正确？

**Q53. (★★)** Which statement about expression evaluation is **NOT** correct?

A. Materialization is always applicable; pipelining is not always applicable.
B. Demand-driven pipelining is a pull model: parents repeatedly request the next tuple; each operator is an iterator (open/next/close) keeping state between calls.
C. Producer-driven pipelining is a push model: operators eagerly produce tuples into inter-operator buffers.
D. Sorting is a blocking operation, so no part of sorting can ever participate in a pipeline.

🀄 物化 vs 流水线、pull vs push、阻塞算子，哪项不正确？

---

## Part F: Concurrency Control (Q54–Q60) | Ch18

**Q54. (★)** Which statement about two-phase locking (2PL) is **NOT** correct?

A. In the growing phase, locks may only be acquired; in the shrinking phase, locks may only be released.
B. Transactions can be serialized in the order of their lock points (the moment each acquires its final lock).
C. Schedules that follow 2PL are guaranteed to be free of deadlocks.
D. Rigorous 2PL holds all locks until commit/abort; transactions serialize in commit order.

🀄 关于 2PL（两阶段、lock point、是否防死锁、rigorous 变体），哪项不正确？

**Q55. (★★)** Which statement about deadlock prevention and detection is **CORRECT**?

A. In wound-wait, an older transaction requesting a lock held by a younger one forces the younger to roll back (it does not wait); in wait-die, the older transaction waits instead.
B. In wait-die, a younger transaction requesting a lock held by an older one enters the wait queue.
C. In both schemes, a rolled-back transaction restarts with a NEW timestamp to keep scheduling fair.
D. A cycle in the wait-for graph is only a necessary condition for deadlock, not a sufficient one.

🀄 wait-die vs wound-wait、原时间戳重启、等待图判环，哪项正确？

**Q56. (★★)** Which statement about multiple-granularity locking and intention locks is **NOT** correct?

A. IX is compatible with IX.
B. S is compatible with IX.
C. To lock node Q in X, SIX, or IX mode, the parent must be held by the same transaction in IX or SIX mode.
D. Locks are acquired top-down (root first) and released bottom-up (children before parent).

🀄 意向锁兼容矩阵与加/放锁顺序，哪项不正确？

**Q57. (★★★)** Which statement about the phantom phenomenon and its prevention is **NOT** correct?

A. Phantoms arise because a predicate read conflicts with an insert/delete/update on the information "which tuples the relation contains", even when no common tuple is accessed.
B. The index-locking protocol requires lookups to S-lock every index leaf accessed — even a leaf containing no tuple satisfying the condition.
C. Next-key locking locks all key values matching the lookup plus the next key value in the index.
D. Using `select ... for update` on the data read is sufficient to prevent phantoms.

🀄 幻读的本质与两种索引锁方案，哪项不正确？

**Q58. (★★★)** Which statement about timestamp ordering (TSO) and Thomas' write rule is **NOT** correct?

A. If TS(Ti) < W-timestamp(Q), Ti's read is rejected and Ti is rolled back.
B. If TS(Ti) < R-timestamp(Q), Ti's write is rejected and Ti is rolled back.
C. TSO transactions never wait, so deadlock is impossible; but schedules may suffer cascading rollbacks and may not even be recoverable.
D. Under Thomas' write rule, an obsolete READ is likewise silently ignored instead of causing a rollback.

🀄 TSO 读写规则与 Thomas 写规则的作用范围，哪项不正确？

**Q59. (★★★)** Under snapshot isolation, initially A=3 and B=17. Transaction Ti executes A := B while Tj executes B := A; they run concurrently, and both commit successfully (they write different items, so first-committer-wins does not fire). The final values are:

A. A=17, B=3 (the values are swapped — write skew)
B. A=17, B=17
C. A=3, B=3
D. One of the transactions must be aborted at commit

🀄 推演题：SI 下两个事务互相赋值，最终 A、B 是多少？

**Q60. (★★)** Which statement about concurrency control in index structures is **NOT** correct?

A. Crabbing: lock the root in shared mode first; release a parent's lock after locking the required children; upgrade leaf locks to exclusive mode for insert/delete.
B. Crabbing guarantees freedom from deadlock, so no abort-and-restart mechanism is needed.
C. The B-link protocol releases the parent's lock before acquiring the child's, relying on link pointers to cope with intervening changes.
D. Latch-free (CAS-based) list updates can suffer the ABA problem: a head node deleted and reinserted makes CAS wrongly conclude "nothing changed."

🀄 索引并发（crabbing 是否无死锁、B-link、ABA），哪项不正确？

---
---

# 答案与详解（Answer Key & Explanations）

## Part A

**Q1 → B**。哈希表范围查询慢的根本原因是**键不按顺序存放**（键 30 可能在 slot 0、键 12 在 slot 2），要找区间内的键只能全表扫描——与哈希函数的计算速度无关（哈希函数恰恰很快）。A、C 是定义；D 描述的正是 BuzzDB `range_query` 遍历所有槽位的低效实现。

**Q2 → D**。兄弟指针的作用是**范围查询的水平遍历**（从下界叶子沿链表右扫到上界），对点查询毫无帮助——点查询垂直下降到单个叶子就结束了。A（K 指针配 K−1 键）、B（叶子键=值）、C（payload 只在叶子）都是必背结构事实。

**Q3 → A (19, 80)**。核心公式：**内部节点键数 = 孩子数 − 1**。根：5 孩子 → 4 键；内部层：(4−1)+(3−1)+(6−1)+(2−1)+(5−1) = 3+2+5+1+4 = 15 键；合计 4+15 = **19**。叶子数 = 内部节点孩子总数 = 4+3+6+2+5 = 20；记录 = 20×4 = **80**。诱饵 B(20,80) 是把根的键数按孩子数算；C(19,76) 是把叶子数算成 19；D(24,80) 是忘了"减一"。

**Q4 → B**。高度上界 = ⌈log⌈n/2⌉(K)⌉ = ⌈log₁₀₀(10⁸)⌉ = ⌈8/2⌉ = 4（因为 100⁴ = 10⁸）。D(27) 是平衡二叉树的层数（log₂10⁸ ≈ 26.6），这正是教材用来对比"为什么 B+ 树赢"的数字。

**Q5 → B**。8m + 13 ≤ 8192 → m ≤ 1022.375 → **1022**。A(510) 是 4KB 页的答案；D(2046) 是 16KB 页的答案；C(1024) 是"感觉应该是 2 的幂"的诱饵。

**Q6 → B**。**内部节点分裂时中位键"搬移"上去（move up）——它离开子节点，只出现在父层**；"同时保留在原节点"描述的是**叶子**分裂的行为（copy up）。这是叶子/内部分裂最重要的区别。A、C、D 均正确（根分裂 → 新根 1 键 2 孩子，树高+1）。

**Q7 → C**。删除引发的合并可以**级联向上直到根**：叶子合并 → 父节点删分隔键 → 父节点也低于下限 → 继续合并……最终根只剩一个孩子时删除根、树高减一（教材删 "Gold" 的例子正是两层级联+删根）。A、B、D 是标准删除规则。

**Q8 → B**。n=8：非根内部节点需 **⌈8/2⌉=4 到 8 个孩子**，3 个孩子不合法。叶子需 ⌈(8−1)/2⌉=4 到 7 个值 → A(4 值)、D(7 值)合法；根最少 2 个孩子 → C 合法。

**Q9 → A**。高效范围查询 = 垂直遍历（根→内部→叶）+ 水平遍历（7 个叶子）。垂直路径上的根和内部节点**都在内存**，不产生磁盘读；7 个叶子都不在内存 → **7 次磁盘页读取**。B(9) 错在把内存命中也算成磁盘读；C(50) 是朴素法扫全部叶子的诱饵。

**Q10 → B**。指针存的是**进程虚拟地址空间**中的地址，重启后完全无意义——序列化必须把节点指针**翻译成文件偏移量**。这与 Sheet 01 的"序列化 unique_ptr 的指针值毫无意义"是同一个考点的变体。A、C、D 都是 Module 7 序列化一节的原话。

**Q11 → D**。教材结论恰好相反：**B-tree 的优点通常不敌缺点**（只有一小部分查找能提前命中；内部节点更大 → 扇出小 → 树更深；插删和实现都更复杂），所以**B+ 树才是被广泛使用的那个**。A、B、C 都是 B-tree 的真实特征。

**Q12 → C**。**二级索引必须是稠密的**：文件不按二级索引的搜索键排序，稀疏索引"找前一个键顺序扫"的策略完全失效，漏掉任何键值都找不回来。A、B、D 是 dense/sparse 的标准结论。

## Part B

**Q13 → C**。教材原话：开放寻址"**不适合数据库应用**"（删除困难、破坏桶与磁盘块的对应关系）。数据库用溢出桶链（closed addressing）。A、B、D 正确。

**Q14 → D**。现实正相反：**PostgreSQL 支持 hash index 但不鼓励使用（性能差）**；Oracle 只支持静态哈希文件组织；SQL Server 只有 B+ 树。A、B、C 是动态哈希与索引选型的标准结论。

**Q15 → C**。LSM 的删除**不做原地物理删除**——插入一条**删除标记（delete entry）**；查找时需将原条目与删除标记配对过滤；只有在**归并**时配对的条目和标记才被一起丢弃。A、B、D 是 LSM 的核心机制。

**Q16 → D**。Buffer tree 的特点是**随机 I/O 比 LSM 多**（这是它的 drawback），查询开销较小、可嫁接到任意树索引（PostgreSQL GiST 采用）。BigTable/Cassandra 采用的是 **LSM（stepped-merge 变体）**，不是 buffer tree。

**Q17 → A**。一张位图 = 每记录 1 bit = 64M bit = **8 MB**。关系 = 64M × 128B = 8 GB。比例 = 8MB / 8GB = **1/1024**——也可直接算：1 bit / 128 byte = 1/(128×8) = 1/1024。C 的 1/800 是教材"100 字节记录"例子的数字，用在 128 字节上就是错的（典型的"背数字不看条件"陷阱）。

**Q18 → C**。位图索引的适用前提是**取值种类少**；高基数属性会产生海量几乎全 0 的位图，毫无优势。且位图索引**对单属性查询本来就没什么用**——它的价值在多属性 AND/OR。A、B、D 正确（计数只需数 1 的个数，比取元组快）。

**Q19 → C**。R-tree 的一个决定性特征恰恰是**允许兄弟包围矩形重叠**（"Bounding boxes of children of a node are allowed to overlap"）——这也是查询可能走多条路径的原因。A（k-d 轮换维度对半分）、B（四叉树四等分）、D 正确。

**Q20 → C**。复合索引按字典序排序，第一列是**范围**条件时索引无法同时利用第二列的等值条件——会取回大量只满足 dept_name<'Finance' 而 salary≠80000 的记录。A、B、D 都能高效支持（前缀匹配原则）。

## Part C

**Q21 → D**。前缀搜索只需**沿前缀字符垂直下行**到对应节点，然后枚举**该子树**——绝不需要遍历整棵树的所有分支。这正是 trie 对前缀查询高效的原因。A、B、C 正确（B+ 树存长字符串键的开销、路径拼串、词源 retrieval）。

**Q22 → D**。方向反了：Patricia trie 通过**合并单字符节点链**使**节点更少**、结构更紧凑，因此插入/查找比普通 trie 更快。A、B、C 是 BuzzDB 实现的准确描述。

**Q23 → A (2, 3)**。逐串模拟：
- **cargo**：消耗 "ca"，剩 "rgo"；与子边 "r" 完全匹配（消耗 "r"），剩 "go"；节点 "r" 的子边 {"t","d","e"} 与 "go" 无公共前缀 → **新建节点 "go"**（终止）。Δnodes +1，Δchars +2。
- **cab**：消耗 "ca"，剩 "b"；子边 {"r","t"} 与 "b" 无公共前缀 → **新建节点 "b"**（终止）。Δnodes +1，Δchars +1。
- 合计 **(2, 3)**。D(2,8) 是把整个单词长度当新增字符的诱饵（cargo5+cab3）；B(2,5) 是没消耗已有路径的 "r" 之类的半步错。

**Q24 → B (2, 2)**。
- **inp**：消耗 "in"，剩 "p"；子边 {"n","put"}，与 "put" 公共前缀 "p" 比 "put" 短 → **分裂**："put" 拆成 "p" + 子节点 "ut"（"ut" 继承 input 的终止标志）；"inp" 的剩余在 "p" 处耗尽 → 终止标志打在 "p" 节点。**Δnodes +1；Δchars = (1+2)−3 = 0（分裂不改变字符总数！）**。
- **into**：消耗 "in"，剩 "to"；子边 {"n","p"} 无公共前缀 → 新建节点 "to"。Δnodes +1，Δchars +2。
- 合计 **(2, 2)**。与官方例题 5 同构：**分裂 = 节点+1、字符+0** 是最容易错的点。

**Q25 → D**。Patricia trie **可以**按字典序遍历节点来支持范围查询——只是要访问很多节点、**不高效**（B+ 树的有序叶子链表才是范围查询的正解）。"完全不能"过于绝对。A、B、C 是标准对比结论。

**Q26 → D**。`getDocuments` 返回的是**包含该词的文档 ID 集合**（`set<int>`），不含位置细节；位置列表是 `proximitySearch` 内部用的。A、B、C 正确。

**Q27 → B**。doc1：|4−11|=7 ≤ 8 ✓（还有 |18−11|=7、|18−25|=7）。doc2：|7−30|=23 ✗。doc3：|120−126|=6 ✓。命中 doc1 和 doc3。

**Q28 → A**。Delta encoding 的输出是**变长**的（小差值用更少字节），它优化的是**空间**，代价恰恰是失去 O(1) 随机访问（必须从头累加差值才能还原绝对位置）。B、C、D 正确（差值：1006−1000=6，1009−1006=3，1030−1009=21）。

**Q29 → C**。方向反了：**document-partitioned** 才让更新简单（新文档整体落在一个 shard）；term-partitioned 下**一个新文档的每个词都可能属于不同 shard**，更新要碰一大批机器。A、B、D 正确。A 正是官方例题 8 的正确答案（生产系统主流）。

**Q30 → A**。quadratic split 的种子是**相距最远**的两个点（nested loop 算所有点对的欧氏距离取最大）——让两个新节点从一开始就离得远，MBR 才不会重叠。"最近"正好相反。B（least enlargement 选路）、C、D 正确。

**Q31 → C**。范围查询的全部意义就在于**剪枝**：内部节点只递归 MBR 与查询矩形**相交**的孩子，不相交的子树整体跳过——绝不需要访问所有叶子。A、B、D 是 Module 8 查询算法的准确描述。

**Q32 → C (位置 4)**。mean_key=200，mean_pos=3。slope = [(100−200)(1−3) + (200−200)(3−3) + (300−200)(5−3)] / [(−100)² + 0² + 100²] = (200+0+200)/20000 = **0.02**。intercept = 3 − 0.02×200 = **−1**。预测 (250) = 0.02×250 − 1 = 5 − 1 = **4**。诱饵 D(5) 是忘了减 intercept。

**Q33 → C**。学习索引的**根本局限**：预测偏差可能超过搜索半径，**radius 内没找到不代表键不存在**——键可能就在 radius 之外，此时查找失败（transcript 原话 "It is possible that we may still not find the key, even though the key exists elsewhere in the index"）。A、B、D 正确。

## Part D

**Q34 → D**。硬编码函数与存储/检索逻辑深度耦合：改查询要求理解整个系统、易引入 bug、难以单独优化——**更难**维护和扩展，这正是引入算子框架的动机。A、B、C 是 transcript 列举的三大好处。

**Q35 → C**。**getOutput() 只返回当前元组的字段，不推进游标**；推进是 next() 的职责（推进并返回是否还有元组）。混淆两者职责是本题唯一的错误点。

**Q36 → D**。Scan 是**流式**的：每次 next() 经 buffer manager 按需取页、逐元组推进——绝不会把整张表一次性读入内存（那会撑爆 buffer pool，也违背算子模型）。A、B、C 是 Scan 实现的准确描述。

**Q37 → C**。`= 0`（纯虚函数）与编译期绑定毫无关系：通过基类指针的调用仍然是**运行期 vtable 动态分发**。这是 Sheet 01 原题（AbstractPolicy 的 evict）的直接变体——出题人喜欢复用场景。A、B、D 正确。

**Q38 → B**。HashAggregation（以及 Select、投影）只有一个输入算子 → 一元；join 需要 input_left/input_right → 二元。A 混淆了"输入表"与"输入算子"；C 错——Scan 没有输入算子（它直接从磁盘表读，是流水线的源头）；D 错——二元算子就有两个输入。

**Q39 → A**。`start_time < end_time` 的**两边都是元组内的列** → 两个 indirect（存列下标，运行时到元组取值）。`price < 100` 是列 vs 常量 → price 列用 indirect、常量 100 用 direct。D 完全违背 indirect 的意义（它就是为了**运行时动态取值**）。

**Q40 → B**。浅拷贝后 `copy.data` 与 `original.data` 指向**同一块堆内存**，改 copy 就是改 original → "Jello"。更严重的隐患：两个对象析构时会对同一内存**各释放一次**（double-free，未定义行为）——这正是 BuzzDB 坚持深拷贝 `getOutput` 的原因。

**Q41 → A**。内层 AND：field0=5 > 2 为 true；field1=12 < 10 为 false → AND = false。外层 OR：false OR (field2=7 == 7 即 true) → **true**。C 是干扰项：AND/OR 的结果与求值顺序无关（短路只影响效率）。

**Q42 → D**。哈希聚合是**阻塞算子**：某组的聚合值会随着后续输入元组不断更新（例：第 3 条元组时某组是 1000，第 4 条到达后变 3000），**必须消费完全部输入**才能确定任何一组的最终值。A、B、C 是 Module 10 的原话。

**Q43 → A**。分组求和：组 1 = 500+300 = 800；组 2 = 1000+2000 = 3000；组 3 = 50。每个唯一组键输出一条 → 3 条元组。

**Q44 → B**。两者都诞生于 **1970 年代**（Unix：Ritchie & Thompson @ Bell Labs；关系数据库：Ted Codd @ IBM），共同动机是对抗当时单体软件的复杂性——与 1990 年代互联网无关。A、C、D 是 transcript 类比的原文。

**Q45 → C**。CreateIndex 的 **next() 是空操作**——整个索引构建都发生在 open() 里（迭代子算子全部元组），它没有逐元组的输出流；使用方通过 getIndex() 拿索引。A、B、D 正确。

## Part E

**Q46 → D**。教材明确**不计最终输出写盘**的开销（"we do not include cost of writing output to disk"）——因为输出可能直接送给上层算子。A、B、C 是代价模型的标准设定。

**Q47 → B**。A4（二级索引、非键等值）：每条匹配记录都可能在不同块，代价 = (h+n)(tT+tS) = (3+500)×(0.1+4) = 503×4.1 ≈ **2062ms**。线性扫描：1 次寻道 + 1000 次块传输 = 4 + 100 = **104ms**。**线性扫描快约 20 倍**——这正是教材警告 A4 "can be very expensive!" 的原因：匹配率高时二级索引不如全表扫。

**Q48 → D**。bitmap index scan 第二阶段**按物理页顺序**读取（这正是它省寻道的关键），输出顺序与索引键顺序无关。A、B、C 是该算法的三个标准性质。

**Q49 → B**。① run 数 = ⌈4000/40⌉ = **100**；② 归并扇入 = ⌊M/bb⌋ − 1 = ⌊40/4⌋ − 1 = 9，趟数 = ⌈log₉(100)⌉ = **3**（9²=81 < 100 ≤ 9³=729）；③ 总传输 = br × (2×趟数 + 1) = 4000 × 7 = **28,000**（run 生成读写 2br + 每趟读写 2br，最后一趟不计写出）。C(24,000) 是漏了"+1"；A 是把趟数算成 2。

**Q50 → A**。BNLJ 最坏代价 = br×bs + br = 80×320 + 80 = **25,680** 次传输；寻道 = 2×br = **160**。B 是 tuple 级 NLJ 的数字（2000×320+80=640,080）——这正是"块级配对"带来的巨大差异。

**Q51 → A**。Indexed NL = br + nr×c = 80 + 2000×4 = **8,080**（传输+寻道合并计），明显优于 BNLJ 的 25,680。C 是把 s 当外表算的诱饵（外表应是 r：教材原则是"对外表的每条元组查内表索引"，且两边都有索引时选**元组少**的做外表）。

**Q52 → C**。**probe 分区不必装进内存**——只有 build 分区必须（要在内存里为它建哈希索引）；probe 分区从盘上逐条读、逐条探测即可。这正是"选小表做 build"的原因。A、B、D 正确。

**Q53 → D**。阻塞算子可以**拆成子阶段**参与流水线：sort = run 生成（可从流水线消费输入）+ 归并（可向流水线产出输出）；hash join = 分区 + build-probe。"任何部分都无法流水线"过于绝对。A、B、C 是物化/流水线的标准结论。

## Part F

**Q54 → C**。**2PL 不防死锁**（教材原话 "Two-phase locking does not ensure freedom from deadlocks"）——两个事务以相反顺序请求两把锁照样互等。它保证的是**冲突可串行化**。A、B、D 正确。

**Q55 → A**。口诀：**wait-die 老等少、少请老则死；wound-wait 老伤少（抢占）、少等老**。B 反了（wait-die 中年轻事务请求老事务的锁时直接回滚，不等待）；C 错——重启用**原时间戳**（越等越老 → 最终必胜 → 防饿死）；D 错——wait-for graph 有环是死锁的**充要**条件。

**Q56 → B**。查兼容矩阵：**S 与 IX 不兼容**——IX 表示"下面有人要加 X 锁改数据"，而 S 要求整棵子树只读，两者矛盾。A（IX-IX 兼容：真正的冲突留到细粒度层判定）、C、D（加锁自顶向下、放锁自底向上）正确。

**Q57 → D**。教材明确：`select ... for update` 能修复一些 SI 异常（把读升级为"视同写"），**但不能处理幻读/谓词读**——它只锁**已存在**的行，挡不住并发插入的新行。A、B、C 是幻读与两种索引锁方案的准确描述。

**Q58 → D**。**Thomas 写规则只修改写规则的第 2 条**：TS(Ti) < W-TS(Q) 的**过时写**被忽略（不回滚）。**读规则完全不变**——TS(Ti) < W-TS(Q) 的读仍然必须回滚（值已被"未来"覆盖，忽略读会返回错数据）。A、B、C 是 TSO 的标准规则与性质。

**Q59 → A**。SI 下两个事务都从**各自开始时的快照**读：Ti 读到 B=17 → 写 A=17；Tj 读到 A=3 → 写 B=3。写的是**不同**数据项 → first-committer-wins 不触发，双双提交 → **A=17，B=3（互换）**。任何串行执行只能得到 (17,17) 或 (3,3)——这就是 **write skew**，SI 不可串行化的经典证据。

**Q60 → B**。Crabbing **可能死锁**：向下爬的查找和因分裂向上爬的更新会互相等待——处理方式是检测后**中止并重启查找**（代价小，不影响事务本身）。A、C、D 正确（B-link 靠 link 指针容忍"先放父锁"造成的中间变化；CAS 有 ABA 问题）。

---

## 得分自评表

| 得分 | 评价 |
|---|---|
| 54–60 | 可以直接上考场 |
| 45–53 | 把错题对应的指南章节重读一遍，重点是计算题的公式条件 |
| 36–44 | 指南第 1、3、7、8 章（B+ 树 + trie + 算子）需要系统重学 |
| < 36 | 从指南第 0 章重新开始，按优先级表逐章推进，做完每章再回来重做对应 Part |

> 提示：错在 "NOT correct" 题多于计算题 → 你的问题是概念精度，用指南第 11.2 节的 Top 20 易错点逐条自测；错在计算题 → 用第 11.1 节公式表默写一遍再重算。

