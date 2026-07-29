# paper5

_(共 12 页，双栏按坐标重排为左栏→右栏；跨栏标题保持原位)_

## Page 1

The Bw-Tree: A B-tree for New Hardware
Platforms
Justin J. Levandoski 1, David B. Lomet 2, Sudipta Sengupta 3
Microsoft Research
Redmond, WA 98052, USA
1justin.levandoski@microsoft.com, 2lomet@microsoft.com, 3sudipta@microsoft.com
Abstract— The emergence of new hardware and platforms has operations without setting latches. Our approach also carefully
led to reconsideration of how data management systems are avoids cache line invalidations, hence leading to substantially
designed. However, certain basic functions such as key indexed better caching performance as well. We describe how we use
access to records remain essential. While we exploit the common
architectural layering of prior systems, we make radically new
design decisions about each layer. Our new form of B-tree, called
the Bw-tree achieves its very high performance via a latch-free
approach that effectively exploits the processor caches of modern
multi-core chips. Our storage manager uses a unique form of log
structuring that blurs the distinction between a page and a record
store and works well with ﬂash storage. This paper describes the
architecture and algorithms for the Bw-tree, focusing on the main
our log structured storage manager at a high level, but leave
the speciﬁcs to another paper.
B. The New Environment
Database systems have mostly exploited the same storage
and CPU infrastructure since the 1970s. That infrastructure
used disks for persistent storage. Disk latency is now analo-
memory aspects. The paper includes results of our experiments gous to a round trip to Pluto [4]. It used processors whose
that demonstrate that this fresh approach produces outstanding uni-processor performance increased with Moore’s Law, thus
performance.
I. INTRODUCTION
A. Atomic Record Stores
There has been much recent discussion of No-SQL systems,
which are essentially atomic record stores (ARSs) [1]. While
limiting the need for high levels of concurrent execution on a
single machine. Processors are no longer providing ever higher
uni-core performance. Succinctly, ”this changes things”.
1) Design for Multi-core: We live in a high peak perfor-
mance multi-core world. Uni-core speed will at best increase
modestly, thus we need to get better at exploiting a large
some of these systems are intended as stand-alone products, number of cores by addressing at least two important aspects:
an atomic record store can also be a component of a more
complete transactional system, given appropriate control op-
erations [2], [3]. Indeed, one can regard a database system as
including an atomic record store as part of its kernel.
An ARS supports the reading and writing of individual
records, each identiﬁed by a key. Further, a tree-based ARS
supports high performance key-sequential access to designated
1) Multi-core cpus mandate high concurrency. But, as the
level of concurrency increases, latches are more likely
to block, limiting scalability [5].
2) Good multi-core processor performance depends on high
CPU cache hit ratios. Updating memory in place results
in cache invalidations, so how and when updates are
done needs great care.
subranges of the keys. It is this combination of random and Addressing the ﬁrst issue, the Bw-tree is latch-free, ensuring a
key-sequential access that has made B-trees the indexing method thread never yields or even re-directs its activity in the face of
of choice within database systems.
conﬂicts. Addressing the second issue, the Bw-tree performs
However, an ARS is more than an access method. It includes “delta” updates that avoid updating a page in place, hence
the management of stable storage and the requirement that preserving previously cached lines of pages.
its updates be recoverable should the system crash. It is the
2) Design for Modern Storage Devices: Disk latency is a
performance of the ARS of this more inclusive form that is major problem. But even more crippling is their low I/O ops
the foundation for the performance of any system in which the per second. Flash storage offers higher I/O ops per second at
ARS is embedded, including full function database systems.
lower cost. This is key to reducing costs for OLTP systems.
This paper introduces a new ARS that provides very high Indeed Amazon’s DynamoDB includes an explicit ability to
performance. We base our ARS on a new form of B-tree that exploit ﬂash [6]. Thus the Bw-tree targets ﬂash storage.
we call the Bw-tree. The techniques that we introduce make
Flash has some performance idiosyncracies, however. While
the Bw-tree and its associated storage manager particularly ap- ﬂash has fast random and sequential reads, it needs an erase cy-
propriate for the new hardware environment that has emerged cle prior to write, making random writes slower than sequential
over the last several years.
writes [7]. While ﬂash SSDs typically have a mapping layer
Our focus in this paper is on the main memory aspects of (the FTL) to hide this discrepancy from users, a noticeable
the Bw-tree. We describe the details of our latch-free tech- slowdown still exists. As of 2011, even high-end FusionIO
nique, i.e., how we can do updates and structure modiﬁcation drives exhibit a 3x faster sequential write performance than

## Page 2

random writes [8]. The Bw-tree performs log structuring it-
self at its storage layer. This approach avoids dependence on
the FTL and ensures that our write performance is as high
as possible for both high-end and low-end ﬂash devices and
hence, not the system bottleneck.
C. Our Contributions
We now describe our contributions, which are:
1) The Bw-tree is organized around a mapping table that
virtualizes both the location and the size of pages. This
virtualization is essential for both our main memory
latch-free approach and our log structured storage.
2) We update Bw-tree nodes by prepending update deltas
to the prior page state. Because our delta updating pre-
serves the prior page state, it improves processor cache
performance. Having the new node state at a new storage
location permits us to use the atomic compare and swap
instructions to update state. Hence, the Bw-tree is latch-
free in the classic sense of allowing concurrent access
to pages by multiple threads.
3) We have devised page splitting and merging structure
modiﬁcation operations (SMOs) for the Bw-tree. SMOs
API
Tree-based search/update logic
Bw-tree Layer In-memory pages only
Cache Layer Logical page abstraction for B-tree layer
Maintains mapping table, brings pages
Mapping Table
from flash to RAM as necessary
Flash Layer Manages writes to flash storage
Flash garbage collection
Fig. 1. The architecture of our Bw-tree atomic record store.
II. BW-TREE ARCHITECTURE
The Bw-tree atomic record store (ARS) is a classic B+-
tree [9] in many respects. It provides logarithmic access to
keyed records from a one-dimensional key range, while pro-
viding linear time access to sub-ranges. Our ARS has a classic
architecture as depicted in Figure 1. The access method layer,
our Bw-tree Layer, is at the top. It interacts with the middle
Cache Layer. The cache manager is built on top of the Storage
Layer, which implements our log-structured store (LSS). The
LSS currently exploits ﬂash storage, but it could manage with
either ﬂash or disk.
This design is architecturally compatible with existing
are realized via multiple atomic operations, each of which database kernels, while also being suitable as a standalone
leaves the Bw-tree well-formed. Further, threads observ-
“data component” in a decoupled transactional system [2],
ing an in-progress SMO do not block, but rather take [3]. However, there are signiﬁcant departures from this classic
steps to complete the SMO.
picture. In this section, we provide an architectural overview
4) Our log structured store (LSS), while nominally a page of the Bw-tree ARS, describing why it is uniquely well-suited
store, uses storage very efﬁciently by mostly posting for multi-core processors and ﬂash based stable storage.
page change deltas (one or a few records). Pages are
eventually made contiguous via consolidating delta up-
dates, and also during a ﬂash “cleaning” process. LSS
will be described fully in a another paper.
A. Modern Hardware Sensitive
In our Bw-tree design, threads almost never block. Elimi-
nating latches is our main technique. Instead of latches, we in-
5) We have designed and implemented an ARS based on stall state changes using the atomic compare and swap (CAS)
the Bw-tree and LSS. We have measured its performance instruction. The Bw-tree only blocks when it needs to fetch
using real and synthetic workloads, and report on its very a page from stable storage (the LSS), which is rare with a
high performance, greatly out-performing both Berke- large main memory cache. This persistence of thread execution
leyDB, an existing system designed for magnetic disks, helps preserve core instruction caches, and avoids thread idle
and latch-free skip lists in main memory.
In drawing broader lessons from this work, we believe that
latch free techniques and state changes that avoid update-in-
place are the keys to high performance on modern processors.
Further, we believe that log structuring is the way to provide
high storage performance, not only with ﬂash, but also with
disks. We think these “design paradigms” are applicable more
widely to realize high performance data management systems.
D. Paper Outline
time and context switch costs. Further, the Bw-tree performs
node updates via ”delta updates” (attaching the update to an
existing page), not via update-in-place (updating the existing
page memory). Avoiding update-in-place reduces CPU cache
invalidation, resulting in higher cache hit ratios. Reducing
cache misses increases the instructions executed per cycle.
Performance of data management systems is frequently
gated by I/O access rates. We have chosen to target ﬂash
storage to ease that problem. But even with ﬂash, when
attached as an SSD, I/O access rates can limit performance.
We present an overview of Bw-tree architecture in Section 2. Our log structure storage layer enables writing large buffers,
In Sections 3 through 5, we describe the system we built. effectively eliminating any write bottle neck. Flash storage’s
We start at the top layer with in-memory page organization high random read access rates coupled with a large main
in Section 3, followed by Bw-tree organization and structure memory cache minimizes blocking on reads. Writing large
modiﬁcations in Section 4. Section 5 details how the cache is multi-page buffers permits us to write variable size pages that
managed. In Section 6, we describe our experiments and the do no contain “ﬁller” to align to a uniform size boundary.
performance results of them. Section 7 describes related work
The rest of this section summarizes the major architectural
and how we differ signiﬁcantly in our approach. We conclude and algorithmic innovations that make concrete the points de-
with a short discussion in Section 8.
scribed above.

## Page 3

B. The Mapping Table
Our cache layer maintains a mapping table, that maps logi-
cal pages to physical pages, logical pages being identiﬁed by a
D. Bw-tree Structure Modiﬁcations
Latches do not protect parts of our index tree during struc-
ture modiﬁcations (SMOs) such as page splits. This introduces
logical ”page identiﬁer” or PID. The mapping table translates a problem. For example, a page split introduces changes to
a PID into either (1) a ﬂash offset, the address of a page on more than one page: the original overly large page O, the new
stable storage, or (2) a memory pointer, the address of the page page N that will receive half O′s contents, and the parent in-
in memory. The mapping table is thus the central location for dex page P that points down to O, and that must subsequently
managing our “paginated” tree. While this indirection tech- point to both O and N. Thus, we cannot install a page split
nique is not unique to our approach, we exploit it as the base with a single CAS. A similar but harder problem arises when
for several innovations. We use PIDs in the Bw-tree to link the we merge nodes that have become too small.
nodes of the tree. For instance, all downward “search” pointers
between Bw-tree nodes are PIDs, not physical pointers.
To deal with this problem, we break an SMO into a sequence
of atomic actions, each installable via a CAS. We use a B-link
The mapping table severs the connection between physical design [11] to make this easier. With a side link in each page,
location and inter-node links. This enables the physical loca- we can decompose a node split into two “half split” atomic
tion of a Bw-tree node to change on every update and every actions. In order to make sure that no thread has to wait for
time a page is written to stable storage, without requiring that an SMO to complete, a thread that sees a partial SMO will
the location change be propagated to the root of the tree (i.e., complete it before proceeding with its own operation. This
updating inter-node links). This “relocation” tolerance directly ensures that no thread needs to wait for an SMO to complete.
enables both delta updating of the node in main memory and
log structuring of our stable storage, as described below.
Bw-tree nodes are thus logical and do not occupy ﬁxed
E. Log Structured Store
Our LSS has the usual advantages of log structuring [12].
physical locations, either on stable storage or in main memory. Pages are written sequentially in a large batch, greatly reducing
Hence we are free to mold them to our needs. A “page” for a
the number of separate write I/Os required. However, because
node thus suggests a policy, not a requirement, either in terms of garbage collection, log structuring normally incurs extra
of how we represent nodes or how large they might become. writes to relocate pages that persist in reclaimed storage areas
We permit page size to be elastic, meaning that we can split of the log. Our LSS design greatly reduces this problem.
when convenient as size constraints do not impose a splitting
requirement.
C. Delta Updating
When ﬂushing a page, the LSS need only ﬂush the deltas
that represent the changes made to the page since its previous
ﬂush. This dramatically reduces how much data is written
during a ﬂush, increasing the number of pages that ﬁt in the
Page state changes are done by creating a delta record (de- ﬂush buffer, and hence reducing the number of I/O’s per page.
scribing the change) and prepending it to an existing page There is a penalty on reads, however, as the discontinuous parts
state. We install the (new) memory address of the delta record of pages all must be read to return a page to the main memory
into the page’s physical address slot in the mapping table cache. Here is when the very high random read performance
using the atomic compare and swap (CAS) instruction1. If
successful, the delta record address becomes the new physi-
of ﬂash really contributes to our ARS performance.
The LSS cleans prior parts of ﬂash representing the old parts
cal address for the page. This strategy is used both for data of its log storage. Delta ﬂushing reduces pressure on the LSS
changes (e.g., inserting a record) and management changes cleaner by reducing the amount of storage used per page. This
(e.g., a page being split or ﬂushing a page to stable storage).
reduces the “write ampliﬁcation” that is a characteristic of log
Occasionally, we consolidate pages (create a new page that structuring. During cleaning, LSS makes pages and their deltas
applies all delta changes) to both reduce memory footprint and contiguous for improved access performance.
to improve search performance. A consolidated form of the
page is also installed with a CAS, and the prior page structure F. Managing Transactional Logs
is garbage collected (i.e., its memory reclaimed). A reference
As in a conventional database system, our ARS needs to
to the entire data structure for the page, including deltas, is ensure that updates persist across system crashes. We tag each
placed on a pending list all of which will be reclaimed when update operation with a unique identiﬁer that is typically the
safe. We use a form of epoch to accomplish safe garbage log sequence number (LSN) of the update on the transactional
collection, [10].
log (maintained elsewhere, e.g., in a transactional component).
Our delta updating simultaneously enables latch-free access LSNs are managed so as to support recovery idempotence, i.e.,
in the Bw-tree and preserves processor data caches by avoiding ensuring that operations are executed at most once.
update-in-place. The Bw-tree mapping table is the key enabler
Like conventional systems, pages are ﬂushed lazily while
of these features via its ability to isolate the effects of node honoring the write-ahead log protocol (WAL). Unconvention-
updates to that node alone.
1The CAS is an atomic instruction that compares a given old value to a
L,
current value at memory location if the values are equal the instruction
writes a new value to L, replacing current.
ally, however, we do not block page ﬂushes to enforce WAL.
Instead, because the recent updates are separate deltas from
the rest of the page, we can remove “recent” updates (not yet
on the stable transactional log) from pages when ﬂushing.

## Page 4

LPID Ptr ∆D LPID Ptr Consolidated
Page P
P CAS P CAS
Page P
∆ ∆ ∆
Page P
(a) Update using delta record (b) Consolidating a page
the current address in the mapping table. If the current address
equals P, the CAS successfully installs D, otherwise it fails
(we discuss CAS failures later). Since all pointers between
Bw-tree nodes are via PIDs, the CAS on the mapping table
entry is the only physical pointer change necessary to install a
page update. Furthermore, this latch-free technique is the only
way to update a page in the Bw-tree, and is uniform across
Fig. 2. In-memory pages. Page updates use compare-and-swap (CAS) on all operations that modify a page. In the rest of this paper,
the physical pointer in the mapping table.
III. IN-MEMORY LATCH FREE PAGES
we refer to using the CAS to update a page as “installing” an
operation.
Figure 2(a) depicts the update process showing a delta record
D prepended to page P. The dashed line from the mapping
This section describes Bw-tree in-memory pages. We begin table to P represents P’s old address, while the solid line to
by discussing the basic page structure and how we update the delta record represents P’s new physical address. Since
pages in a latch-free manner. We then discuss occasional page updates are atomic, only one updater can “win” if there are
consolidation used to make search more efﬁcient. Using a
competing threads trying install a delta against the same “old”
tree based index enables range scans, and we describe how state in the mapping table. A thread must retry its update if it
this is accomplished. Finally, we discuss in-memory garbage fails2.
collection with our epoch safety mechanism.
A. Elastic Virtual Pages
After several updates, a “delta chain” forms on a page as
depicted in the lower right of Figure 2(b) (showing a chain
of three deltas). Each new successful updates forms the new
The information stored on a Bw-tree page is similar to that “root” of the delta chain. This means the physical structure of
of a typical B+-tree. Internal index nodes contain (separator a Bw-tree page consists of a delta chain prepended to a base
key,pointer) pairs sorted by key that direct searches down the page (i.e., a consolidate B-tree node). For clarity, we refer
tree. Data (leaf) nodes contain (key, record) pairs. In addition, to a base page as the structure to which the delta chain is
pages also contain (1) a low key representing the smallest key prepended, and refer to a page as the a base page along with
value that can be stored on the page (and in the subtree below), its (possibly empty) delta chain.
and (2) a high key representing the largest key value that can
Leaf-level update operations. At the leaf page level, up-
be stored on the page, and (3) a side link pointer that points dates (deltas) are one of three types: (1) insert, representing
to the node’s immediate right sibling on the same level in the a new record inserted on the page; (2) modify, representing
tree, as in a B-link tree [11].
a modiﬁcation to an existing record in the page; (3) delete,
Two distinct features make the Bw-tree page design unique. representing the removal of an existing record in the page. All
First, Bw-tree pages are logical, meaning they do not occupy update deltas contain an LSN provided by the client issuing the
ﬁxed physical locations or have ﬁxed sizes. (1) We identify update. We use this LSN for transactional recovery involving
a page using its PID index into the mapping table. Accessors a transaction log manager with its need to enforce the write-
to the page use the mapping table to translate the PID into ahead-log (WAL) protocol (we discuss WAL and LSNs further
its current physical address. All links between Bw-tree nodes in Section V-A). Insert and update deltas contain a record
(“search” pointers, side links) are PIDs. (2) Pages are elastic,
representing the new payload, while delete deltas only contain
meaning there is no hard limit on how large a page may grow. the key of the record to be removed. In the rest of this sec-
Pages grow by having “delta records” prepended to them. tion, we discuss only record updates to leaf pages, postponing
A delta record represents a single record modiﬁcation (e.g., discussion of other updates (e.g., index pages, splits, ﬂushes)
insert, update), or system management operation (e.g., page until later in the paper.
split).
Page search. Leaf page search involves traversing the delta
Updates. We never update a Bw-tree page in place (i.e., chain (if present). The search stops at the ﬁrst occurrence of
modify its memory contents). Rather, we create a delta record the search key in the chain. If the delta containing the key
describing the update and prepend it to the existing page. represents an insert or update, the search succeeds and returns
Delta records allow us to incrementally update page state in the record. If the delta represents a delete, the search fails. If
a latch-free manner. We ﬁrst create a new delta record D that the delta chain does not contain the key, the search performs
(physically) points to the page’s current address P. We obtain a binary search on the base page in typical B-tree fashion. We
P from the page’s entry in the mapping table. The memory discuss index page search with delta chains in Section IV.
address of the delta record will serve as the new memory
address (the new state) for the page.
To install the new page state in the mapping table (making
it “live” in the index), we use the atomic compare and swap
(CAS) instruction (described in Section II) to replace the cur-
B. Page Consolidation
Search performance eventually degrades if delta chains grow
too long. To combat this, we occasionally perform page con-
2The
retry protocol will depend on the speciﬁc update operation. We discuss speciﬁc
rent address P with the address of D. The CAS compares P to retry protocols where appropriate.

## Page 5

solidation that creates a new “re-organized” base page contain-
ing all the entries from the original base page as modiﬁed by
the updates from the delta chain. We trigger consolidation if
an accessor thread, during a page search, notices a delta chain
length has exceeded a system threshold. The thread performs
consolidation after attempting its update (or read) operation.
When consolidating, the thread ﬁrst creates a new base page
(a new block of memory). It then populates the base page with
a sorted vector containing the most recent version of a record
from either the delta chain or old base page (deleted records
are discarded). The thread then installs the new address of
the consolidated page in the mapping table with a CAS. If
it succeeds, the thread requests garbage collection (memory
reclamation) of the old page state. Figure 2(b) provides an
LPID Ptr LPID Ptr
O
O Split ∆
P P CAS
Page P Page R Page P Page R
Q Q
Page Q Page Q
(a) Creating sibling page Q (b) Installing split delta
Index entry ∆
LPID Ptr
O CAS
P
Split ∆
Q Page P Page R
Page Q
(c) Installing index entry delta
example depicting the consolidation of page P that incorpo- Fig. 3. Split example. Dashed arrows represent logical pointers, while solid
rates deltas into a new “Consolidated Page P”. If this CAS arrows represent physical pointers.
fails, the thread abandons the operation by deallocating the
new page. The thread does not retry, as a subsequent thread deallocate the old page state while another thread still accesses
will eventually perform a successful consolidation.
C. Range Scans
it. Similar concerns arise when a page is removed from the
Bw-tree. That is, other threads may still be able to access
the now removed page. We must protect these threads from
A range scan is speciﬁed by a key range (low key, high accessing reclaimed and potentially “repurposed” objects by
key). Either of the boundary keys can be omitted, meaning that preventing reclamation until such access is no longer possible.
one end of the range is open-ended. A scan will also specify This is done by a thread executing within an “epoch”.
either an ascending or descending key order for delivering the
An epoch mechanism is a way of protecting objects be-
records. Our description here assumes both boundary keys are ing deallocated from being re-used too early [10]. A thread
provided and the ordering is ascending. The other scan options joins an epoch when it wants to protect objects it is using
are simple variants.
(e.g., searching) from being reclaimed. It exits the epoch when
A scan maintains a cursor providing a key indicating how this dependency is ﬁnished. Typically, a thread’s duration in
far the search has progressed. For a new scan, the remembered an epoch is for a single operation (e.g. insert, next-record).
key is lowkey. When a data page containing data in the range Threads “enrolled” in epoch E might have seen earlier ver-
is ﬁrst accessed, we construct a vector of records containing sions of objects that are being deallocated in epoch E. How-
all records on the page to be processed as part of the scan. ever, a thread enrolled in epoch E cannot have seen objects
In the absence of changes to the page during the scan, this deallocated in epoch E-1 because it had not yet started its
allows us to efﬁciently provide the “next-record” functionality. dependency interval. Hence, once all threads enrolled in epoch
This is the common case and needs to be executed with high E have completed and exited the epoch (“drained”), it is safe
performance.
to recycle the objects deallocated in epoch E. We use epochs
We treat each ”next-record” operation as an atomic unit. The to protect both storage and deallocated PIDs. Until the epoch
entire scan is not atomic. Transactional locking will prevent has drained such objects cannot be recycled.
modiﬁcations to records we have seen (assuming serializable
transactions) but we do not know the form of concurrency
control used for records we have not yet delivered. So before
IV. BW-TREE STRUCTURE MODIFICATIONS
All Bw-tree structure modiﬁcation operations (SMOs) are
delivering a record from our vector, we check whether an performed in a latch-free manner. To our knowledge, this has
update has affected the yet unreturned subrange in our record never been done before, and is crucial for our design. We ﬁrst
vector. If such an update has occurred, we reconstruct the describe node splits, followed by node merges. We then dis-
record vector accordingly.
D. Garbage Collection
cuss how to ensure an SMO has completed prior to performing
actions that depend on the SMO. This is essential both to avoid
confusion in main memory and to prevent possible corrupt
A latch-free environment does not permit exclusive access trees should the system crash at an inopportune time.
to shared data structures (e.g., Bw-tree pages), meaning one or
more readers can be active in a page state even as it is being
updated. We do not want to deallocate memory still accessed
A. Node Split
Splits are triggered by an accessor thread that notices a page
by another thread. For example, during consolidation, a thread size has grown beyond a system threshold. After attempting
“swaps out” the old state of a page (i.e., delta chain plus base its operation, the thread performs the split.
page) for a new consolidated state and requests that the old
The Bw-tree employs the B-link atomic split installation
state be garbage collected. However, we must take care not to technique that works in two phases [11]. We ﬁrst atomically

## Page 6

install the split at the child (e.g., leaf) level. This is called a
must now traverse a delta chain on the index nodes, ﬁnding
half split. We then atomically update the parent node with the a boundary key delta in the chain such that a search key v
new index term containing a new separator key and a pointer is greater than KP and less than or equal to KQ allows the
to the newly created split page. This process may continue re- search to end instantly and follow the logical pointer down to
cursively up the tree as necessary. The B-link structure allows Q. Otherwise, the search continues into the base page, which is
us to separate the split into two atomic actions, since the side
searched with a simple binary search to ﬁnd the correct pointer
link provides a valid search tree after installing the split at the to follow. Figure 3(c) depicts our running split example after
child level.
prepending the index entry delta to parent page O, where the
Child Split. To split a node P, the B-tree layer requests dashed line represents the logical pointer to page Q.
(from the cache layer) allocation for a new node Q in the
Consolidations. Posting deltas decreases latency when in-
mapping table (Q is the new right sibling of P). We then stalling splits, relative to creating and installing completely
ﬁnd the appropriate separator key KP from P that provides a
new base pages. Decreasing latency decreases the chance of
balanced split and proceed to create a new consolidated base “failed splits”, i.e., the case that other updates sneak in before
state for Q containing the records from P with keys greater we try to install the split (and fail). However, we eventually
than KP . Page Q also contains a side link to the former right consolidate split pages at a later point in time. For pages with
sibling of P (call this page R). We next install the physical split deltas, consolidation involves creating a new base page
address of Q in the mapping table. This installation is done containing records with keys less than the separator key in the
without a CAS, since Q is visible to only the split thread. split delta. For pages with index entry deltas, we create a new
Figure 3(a) depicts this scenario, where a new sibling page consolidated base page containing the new separator keys and
Q contains half the records of P, and (logically) points to pointers.
page R (the right sibling of P). At this point, the original
(unsplit) state of P is still present in the mapping table, and
Q is invisible to the rest of the index.
B. Node Merge
Like splits, node merges are triggered when a thread en-
We atomically install the split by prepending a split delta counters a node that is below some threshold size. We peform
record to P. The split delta contains two pieces of information: the node merge in a latch-free manner, which is illustrated in
(1) the separator key KP used to invalidate all records within Figure 4. Merges are decidedly more complicated than splits,
P greater than KP , since Q now contains these records and and we need more atomic actions to accomplish them.
(2) a logical side pointer to the new sibling Q. This installation
completes the ﬁrst “half split”. Figure 3(b) depicts such a
Marking for Delete. The node R to be merged (to be
removed) is updated with a remove node delta, as depicted
scenario after prepending a split delta to page P pointing to in Figure 4(a). This stops all further use of node R. A thread
its new sibling page Q. At this point, the index is valid, even encountering a remove node delta in R needs to read or update
without the presence of an index term for Q in the parent node the contents of R previously contained in R by going to the
O. All searches for a key contained within Q will ﬁrst go to left sibling, into which data from R will be merged.
P. Upon encountering the split delta on P, the search will
Merging Children.The left sibling L of R is updated with a
traverse the side link to Q when the search key is greater than node merge delta that physically points (via a memory address)
separator key KP . Meanwhile, all searches for keys less than to the contents of R. Figure 4(b) illustrates what L and R
the KP remain at P.
look like during a node merge. Note that the the node merge
Parent Update. In order to direct searches directly to Q, delta indicates that the contents of R are to be included in L.
we prepend an index term delta record to the parent of P and Further, it points directly to this state, which is now logically
Q to complete the second half split. This index delta contains considered to be part of L. This storage for R’s state is now
(1) KP , the separator key between P and Q, (2) a logical transferred to L (except for the remove node delta itself). It
pointer to Q, and (3) KQ, the separator key for Q (formerly will only be reclaimed when L is consolidated. This turns what
the separator directing searches to P). We remember our path had previously been a linear delta chain representing a page
down the tree (i.e. the PIDs of nodes on the path) and hence state into a tree.
can immediately identify the parent. Most of the time, the
remembered parent on the path will be the correct one and we
When we search L (now responsible for containing both
its original key space and the key space that had been R’s)
do the posting immediately. Occasionally the parent may have the search becomes a tree search which directs the accessing
been merged into another node. But our epoch mechanism thread to either L’s original page or to the page that it sub-
guarantees that we will see the appropriate deleted state that sumed from R as a result of the merge. To enable this, the
will tell us this has happened. That is, we are guaranteed that node merge delta includes the separator key that enables the
the parent PID will not be a dangling reference. When we
detect a deleted state, we go up the tree to the grandparent
search to proceed to the correct node.
Parent Update. The parent node P of R is now updated
node, etc., and do a re-traversal down the tree to ﬁnd the by deleting its index term associated with R, shown in Fig-
parent that is “still alive”.
ure 4(c). This is done by posting an index term delete delta
Having KP and KQ present in the boundary key delta that includes not only that R is being deleted, but also that L
is an optimization to improve search speed. Since searches will now include data from the key space formerly contained

## Page 7

LPID Ptr
LPID Ptr
P Remove P Merge ∆ Remove
Node ∆ Node ∆
L L
R Page L Page R Page S R Page L Page R Page S
S S
(a) Posting remove node delta (b) Posting merge delta
∆ Delete Index
Term for R
LPID Ptr
P Remove
Merge ∆ Node ∆
L
R Page L Page R Page S
S
(c) Posting index term delete delta
Fig. 4. Merge example. Dashed arrows represent logical pointers, while solid
arrows represent physical pointers.
by R. The new range for L is explicitly included with a low
key equal to L’s prior low key and a high key equal to R’s
prior high key. As with node splits, this permits us to recognize
when a search needs to be directed to the newly changed part
it must complete the split SMO by posting the new index term
delta to the parent. Only then can it continue on to its own
activity. That forces the incomplete SMO to be “committed”
and to serialize before the interrupted action the thread had
started upon.
The same principle applies regardless of whether the SMO
is a split or a node merge. When deleting a node R, we
access its left sibling L to post the merge delta. If we ﬁnd
that L is being deleted, we are seeing an in progress and
incomplete “earlier” system transaction. We need the delete
of R to serialize after the delete of L in this case. Hence
the thread deleting R needs to ﬁrst complete the delete of
L. Only then can this thread complete the delete of R. All
combinations of SMO are serialized in the same manner. This
can lead to the processing of a “stack” of SMOs, but given
the rarity of this situation, it should not occur often, and is
reasonably straightforward to implement recursively.
V. CACHE MANAGEMENT
The cache layer is responsible for reading, ﬂushing, and
of the tree. Further, it enables any search that drops through swapping pages between memory and ﬂash. It maintains the
all deltas to the base page to ﬁnd the right index term by a
simple binary search.
Once the index term delete delta is posted, all paths to R
mapping table and provides the abstraction of logical pages
to the Bw-tree layer. When the Bw-tree layer requests a page
reference using a PID, the cache layer returns the address in
are blocked. At this point we initiate the process of reclaiming the mapping table if it is a memory pointer. Otherwise, if
Rs PID. This involves posting the PID to the pending delete the address is a ﬂash offset (the page is not in memory), it
list of PIDs for the currently active epoch. R’s PID will not reads the page from the LSS to memory, installs the memory
be recycled until all other threads that might have seen an address in the mapping table (using a CAS), and returns the
earlier state of R have exited the epoch. This is the same epoch new memory pointer. All updates to pages, including those
mechanism we use to protect page memory from premature involving page management operations like split and page
recycling (Section III-D).
C. Serializing Structure Modiﬁcations and Updates
ﬂush involve CAS operations on the mapping table at the
location indexed by the PID.
Pages in main memory are occasionally ﬂushed) to stable
Our Bw-tree implementation assumes that conﬂicting data storage for a number of reasons. For instance, the cache layer
update operations are prevented by concurrency contol that is will ﬂush updates to enable the checkpointing of a transac-
elsewhere in the system. This could be in the lock manager tional log when the Bw-tree is part of a transactional system.
of an integrated database system, a transactional component Page ﬂushes also precede a page swapout, installing a ﬂash
of a decoupled transactional system (e.g., Deuteronomy [2]), offset in the mapping table and reclaiming page memory in
or ﬁnally, an arbitrary interleaving of concurrent updates as order to reduce memory usage. With multiple threads ﬂush-
enabled as by an atomic record store.
ing pages to ﬂash, multiple page ﬂushes that need a correct
However, “inside” the Bw-tree, we need to correctly seri- ordering must be done very carefully. We describe one such
alize data updates with SMOs and SMOs with other SMOs.
That is, we must be able to construct a serial schedule for
scenario for node splits in this section.
To keep track of which version of the page is on stable
everything that occurs in the Bw-tree, where data updates and storage (the LSS) and where it is, we use a ﬂush delta record,
SMOs are treated as the units of atomicity. This is nontrivial. which is installed at the mapping table entry for the page
We want to treat an SMO as atomic (think of them as system using a CAS. Flush delta records also record which changes
transactions), and we are doing this without using latches that to a page have been ﬂushed so that subsequent ﬂushes send
could conceal the fact that there are multiple steps involved in only incremental page changes to stable storage. When a page
an SMO. One way to think about this is that if a thread stum- ﬂush succeeds, the ﬂush delta contains the new ﬂash offset
bles upon an incomplete SMO, it is like seeing uncommitted and ﬁelds describing the state of the page that was ﬂushed.
state. Being latch-free, the Bw-tree cannot prevent this from
The rest section focuses on how the cache layer interacts
happening. Our response is to require that such a thread must with the LSS layer by preparing and performing page ﬂushes
complete and commit the SMO it encounters before it can (LSS details covered in future work). We start by describing
either (1) post its update or (2) continue with its own SMO.
how ﬂushes to LSS are coordinated with the demands of a
For page splits, that means that when an updater or another separate transactional mechanism. We then describe the me-
SMO would traverse a side pointer to reach the correct page, chanics of how ﬂushes are executed.

## Page 8

A. Write Ahead Log Protocol and LSNs
The Bw-tree is an ARS that can be included in a transac-
tional system. When included, it needs to manage the trans-
actional aspects imposed on it. The Deuteronomy [2] archi-
tecture makes those aspects overt as part of its protocol be-
tween transactional (TC) and data component (DC), so we
use Deuteronomy terminology to describe this functionality.
Similar considerations apply in other transactional settings.
LSNs. Record insert and update deltas in a the Bw-tree page
are tagged with the Log Sequence Number (LSN) of their op-
erations. The highest LSN among updates ﬂushed is recorded
in the ﬂush delta describing the ﬂush. LSNs are generated by
the higher-level TC and are used by its transactional log.
Transaction Log Coordination. Whenever the TC appends
(ﬂushes) to its stable transactional log, it updates the End of
Stable Log (ESL) LSN value. ESL is an LSN such that all
lower valued LSNs are deﬁnitely in the stable log. Periodically,
it sends an updated ESL value to the DC. It is necessary for
enforcing causality via the write-ahead log protocol (WAL)
that the DC not make durable any operation with an LSN
greater than the latest ESL. This ensures that the DC is “run-
ning behind” the TC in terms of operations made durable. To
enforce this rule, records on a page that have LSNs larger than
the ESL are not included in a page when ﬂushed to the LSS.
Page ﬂushes in the DC are required by the TC when it ad-
vances its Redo-Scan-Start-Point (RSSP). When the TC wishes
to advance the RSSP, it proposes an RSSPit to the DC. The
B. Flushing Pages to the LSS
The LSS provides a large buffer into which the cache man-
ager posts pages and system transactions describing Bw-tree
structure modiﬁcations. We give a brief overview of this func-
tionality. Details will be described in another paper.
Page Marshalling. The cache manager marshalls the bytes
from the pointer representation of the page in main memory
into a linear representation that can be written to the ﬂush
buffer. The page state is captured at the time it is intended to
be ﬂushed. This is important as later updates might violate the
WAL protocol or a page split may have removed records that
need to be captured in LSS. For example, the page may be split
and consolidated while an earlier ﬂush request for it is being
posted to the ﬂush buffer. If the bytes for the earlier ﬂush are
marshalled after the split has removed the upper order keys in
the pre-split page, the version of the page captured in the LSS
will not have these records. Should the system crash before
the rest of the split itself is ﬂushed, those records will be lost.
When marshalling records on a page for ﬂush, multiple delta
records are consolidated so that they appear contiguously in
the LSS.
Incremental Flushing. When ﬂushing a page, the cache man-
ager only marshals those delta records which have an LSN
between the previously ﬂushed largest LSN on that page and
the current ESL value. The previously ﬂushed largest LSN
information is contained in the latest ﬂush delta record on the
page (as described above).
Incremental ﬂushing of pages means that the LSS consumes
intent is for this to permit the TC to truncate (drop) the portion much less storage for a page than is the case for full page
of the transactional log earlier than the RSSP. The TC will ﬂushing. This is very valuable for a log structured store such
then wait for an acknowledgement from the DC indicating that as our LSS for two reasons. (1) It means that a ﬂush buffer
the DC has made every update with an LSN < RSSP stable.
can hold far more updates across different pages than if the
Because these operations results are stable, the TC no longer entire state of every page were ﬂushed. This increases the
has to send these operations to the DC during redo recovery. writing efﬁciency on a per page basis. (2) The log structured
For the DC to comply, it needs to ﬂush the records on every store cleaner (garbage collector) does not need to work as
page that have LSNs < RSSP before it acknowledges to the
hard since storage is not being consumed as fast. This reduces
TC. Such ﬂushes are never blocked as the cache manager can the execution cost per page for the cleaner. It also reduces
exclude every record with an LSN > ESL. Non-blocking is not
possible with a conventional update-in-place approach using a
single page-wide LSN.
the ”write ampliﬁcation”, i.e. the requirement of re-writing
unchanged pages when the cleaner encounters them.
Flush Activity. The ﬂush buffer aggregates writes to LSS up
To enable this non-blocking behavior, we restrict page con- to a conﬁgurable threshold (currently set at 1MB) to reduce
solidation (Section III-B) to include only update deltas that I/O overhead. It uses ping-pong (double) buffers and alternates
have LSNs less than or equal to the ESL. Because of this, we
can always exclude these deltas when we ﬂush the page.
between them with asynchronous I/O calls to the LSS so that
the buffer for the next batch of page ﬂushes can be prepared
Bw-tree Structure Modiﬁcations. We wrap a system trans- while the current one is in progress.
action [3] around the pages we log as part of Bw-tree SMOs.
After the I/O completes for a ﬂush buffer, the states of the
This solves the problem of concurrent SMOs that can result respective pages are updated in the mapping table. The result
from our latch-free approach (e.g., two threads trying to split of the ﬂush is captured in the mapping table by means of a
the same page). To keep what is in the LSS consistent with ﬂush delta describing the ﬂush, which is prepended to the state
main memory, we do not commit the SMO system transaction and installed via a CAS like any other delta. If the ﬂush has
with its updated pages until we know that its thread has “won” captured all the updates to the page, the page is “clean” in
the race to install an SMO delta record at the appropriate page. that there are no uncaptured updates not stable on the LSS.
Thus, we allow concurrent SMOs, but ensure that at most one
The cache manager monitors the memory used by the Bw-
of them can commit. The begin and end system transaction tree, and when it exceeds a conﬁgurable threshold, it attempts
records are among the very few objects ﬂushed to the LSS to swap out pages to the LSS. Once a page is clean, it can be
that are not instances of pages.
evicted from the cache. The storage for the state of an evicted

## Page 9

Synthetic Xbox
12
(M)
Operations/Second 10
8
6
4
2
0
2 4 8 16 32
Delta Chain Length
Fig. 5. Effect of Delta Chain Length
Failed Splits Failed Consolidates Failed Updates
Dedup 0.25% 1.19% 0.0013%
Xbox 1.27% 0.22% 0.0171%
Synthetic 8.88% 7.35% 0.0003%
TABLE I
LATCH-FREE DELTA UPDATE FAILURES
It decides to install an element at the next highest layer (the
skip list towers or “express lanes”) with a probability of 1 2.
The maximum height of our skip list is 32 layers.
Experiment machine. Our experiment machine is an Intel
Xeon W3550 (at 3.07GHz) with 24 GB of RAM. The machine
page is reclaimed via our epoch-based memory garbage col- contains four cores that we hyperthread to eight logical cores
lector (Section III-D).
VI. PERFORMANCE EVALUATION
This section provides experimental evaluation of the Bw-
in all of our experiments.
Evaluation Datasets. Our experiments use three workloads,
two from real-world applications and one synthetic.
1) Xbox LIVE. This workload contains 27 Million get-set
tree when compared to a “traditional” B-tree architecture (Berke- operations obtained from Microsoft’s Xbox LIVE Prime-
leyDB run in B-tree mode) as well as latch-free skiplists. Our
experiments use a mix of real-world and synthetic workloads
running on real system implementations. Since this paper fo-
cuses on in-memory aspects of the Bw-tree, our experiments
explicitly focus on in-memory system performance. Perfor-
mance evaluation on secondary storage will be the topic of
future work.
A. Implementation and Setup
Bw-Tree. We implemented the Bw-Tree as a
standalone atomic record store in approximately
10,000 lines of C++ code. We use the Win32 native
InterlockedCompareExchange64 to perform the CAS
update installation. Our entire implementation was latch-free.
BerkeleyDB. We compare the Bw-tree to the BerkeleyDB
key-value database. We chose BerkeleyDB due to its good
performance as a standalone storage engine. Furthermore, it
does not need to traverse a query processing layer as done in
a complete database system. We use the C implementation of
BerkeleyDB running in B-tree mode, which is a standalone B-
tree index sitting over a buffer pool cache that manages pages,
representing a typical B-tree architecture. We use BerkeleyDB
in non-transactional mode (meaning better performance) that
supports a single writer and multiple readers using page-level
latching (the lowest latch granularity in BerkeleyDB) to max-
imize concurrency. With this conﬁguration, we believe Berke-
leyDB represents a fair comparison to the Bw-tree. For all
experiments, the BerkeleyDB buffer pool size is large enough
to accommodate the entire workload (thus does not touch
secondary storage).
Skip list. We also compare the Bw-tree to a latch-free skip
list implementation [13]. The skip list has become a popular
databases3
alternative to B-trees for memory-optimized since
they can be implemented latch-free, exhibit fast insert perfor-
mance, and maintain logarithmic search cost. Our implemen-
tation installs an element in the bottom level (a linked list)
using a CAS to change the pointer of the preceding element.
time online multi-player game [15]. Keys are alpha-
numeric strings averaging 94 bytes with payloads av-
eraging 1200 bytes. The read-to-write ratio is approxi-
mately 7.5 to 1.
2) Storage deduplication trace. This workload comes from
a real enterprise deduplication trace used to generate
a sequence of chunk hashes for a root ﬁle directory
and compute the number of deduplicated chunks and
storage bytes. This trace contains 27 Million total chunks
and 12 Million unique chunks, and has a read to write
ration of 2.2 to 1. Keys are 20-byte SHA-1 hash values
that uniquely identify a chunk, while the value payload
contains a 44-byte metadata string.
3) Synthetic. We also use a synthetic data set that generates
8-byte integer keys with a 8-byte integer payload. The
workload begins with an index of 1M entries generated
using a uniform random distribution. It performs 42 mil-
lion operations with a read to write ratio of 5 to 1.
Defaults. Unless mentioned otherwise, our primary perfor-
mance metric is throughput measured in (Million) operations
per second. We use 8 worker threads for each workload, equal
to the number of logical cores on our experiment machine.
The default page size for both BerkeleyDB and the Bw-tree
is 8K (the skip list is a linked list and does not use page
organization).
B. Bw-Tree Tuning and Properties
In this section, we evaluate two aspects of the Bw-tree:
(1) the effect of delta chain length on performance and (2) the
latch-free failure rate for posting delta updates.
1) Delta chain length: Figure 5 depicts the performance
of the Bw-tree run over the Xbox and synthetic workloads
for varying delta chain length thresholds, i.e., the maximum
length a delta chain grows before we trigger page consol-
idation. For small lengths, worker threads perform consol-
idation frequently. This overhead cuts into overall system
performance. For the Xbox workload, search deteriorates for
3The MemSQL in-memory database uses a skip list as its ordered index [14] sequential scans larger than four deltas. While sequential scans

## Page 10

BW-Tree BerkeleyDB
12.0
10.40
10.0
(M)
Operations/Sec 8.0
6.0
3.83
4.0
2.84
2.0
0.56 0.66
0.33
0.0
Xbox Synthetic Deduplication
Fig. 6. Bw-tree and BerkeleyDB
over linked delta chains are good for branch prediction and
prefetching in general, the Xbox workload has large 100-byte
records, meaning fewer deltas will ﬁt into the L1 cache during
a scan. The synthetic workload contains small 8-byte keys,
which are more amenable to prefetching and caching. Thus,
delta chain lengths can grow longer (to about eight deltas)
without performance consequences.
2) Update Failure in Latch-Free Environment: Given the
Bw-tree Skip List
Synthetic workload 3.83M ops/sec 1.02M ops/sec
Read-only workload 5.71M ops/sec 1.30M ops/sec
TABLE II
BW-TREE AND LATCH-FREE SKIP LIST
In general, we believe two main aspects lead to the supe-
rior performance of the Bw-tree: (1) Latch-freedom: no thread
blocks on updates or reads on the Bw-tree, while BerkeleyDB
uses page-level latching to block readers during updates, re-
ducing concurrency. The Bw-tree executes with a processor
utilization of about 99% while BerkeleyDB runs at about 60%.
(2) CPU cache efﬁciency: since the Bw-tree uses delta records
to update immutable base pages, the CPU caches of other
threads are rarely invalidated on an update. Meanwhile, Berke-
leyDB updates pages in place. An insert into a typical B-
tree page involves including a new element in vector of key-
ordered records, on average moving half the elements and
invalidating multiple cache lines.
D. Comparing Bw-tree to a Latch-Free Skip List
We also compare the performance of the Bw-tree to a latch-
latch-free nature of the Bw-tree, some operations will inevitably free skip list implementation. The skip list provides key-ordered
fail in the race to update page state. Table I provides the access with logarithmic search overhead, and can easily be
failure rate for splits, consolidates, and record updates for implemented in a latch-free manner. For these reasons, it is
each workload. The record update failure rate (e.g., inserts, starting to receive attention as a B-tree alternative in memory-
updates, deletes) is extremely low, below 0.02% for all work- optimized databases [14]. The ﬁrst row of Table II reports the
loads. Meanwhile, the failure rates for the split and consoli- results of the synthetic workload run over both the Bw-tree
date operations are larger than the update failures at around and the latch-free skiplist. The Bw-tree outperforms the skip
1.25% for both the Xbox and deduplication workloads, and list by a factor of 3.7x. To further investigate the source of the
8.88% for the synthetic workload. This is expected, since Bw-tree performance gain, we ran both systems using a read-
splits and consolidates must compete with the faster record only workload that performs 30M key lookups on an index
update operations. However, we believe these rates are still of 30M records. We report these results in the second row of
manageable. The synthetic workload failure rates represent Table II, showing the Bw-tree with a 4.4x performance advan-
a worst-case scenario. For the synthetic data, preparing and tage. These results suggest the Bw-tree has a clear advantage
installing a record update delta is extremely fast since records in search performance. We suspect this is due to the Bw-tree
are extremely small. In this case, a competing (and more having better CPU cache efﬁciency than the skip list, which
expensive) split operation trying to install its delta on the same we explore in the next section.
page has a high probability of failure.
C. Comparing Bw-tree to a Traditional B-tree Architecture
E. Cache Performance
This experiment measures the CPU cache efﬁciency of the
Bw-tree compared to the skip list. We use the Intel VTune
This experiment compares the in-memory performance of proﬁler5 to capture CPU cache hit events while running the
the Bw-tree to BerkeleyDB4, representing a traditional B-
read-only workload on each system. The workload was run
tree architecture. Figure 6 reports the results for the Xbox, single-threaded in order to collect accurate statistics. Figure 7
deduplication, and synthetic workloads run on both systems.
plots the distribution of retired memory loads over the CPU
For the Xbox workload, the Bw-tree exhibits a throughput of cache hierarchy for each system. As we expected, the Bw-tree
10.4M operations/second, while BerkeleyDB has a throughput exhibits very good cache efﬁciency compared to the skip list.
of 555K operations/second, representing a speedup of 18.7x. Almost 90% of its memory reads come from either the L1
The throughput of both systems drops for the update-intensive or L2 cache, compared to 75% for the skip list. We suspect
deduplication workload, however the Bw-tree maintains a 8.6x the Bw-tree’s search efﬁciency is the primary reason for this
speedup over BerkeleyDB. The performance gap is closer difference. The skip list must traverse a physical pointer before
for the synthetic workload (5.8x Bw-tree speedup) due to every comparison to traverse to the next “tower” or linked list
the higher latch-free failure rates observed for the Bw-tree record at a particular level. This can cause erratic CPU cache
observed in Section VI-B.2.
4We use BerkeleyDB’s memp stat function to ensure it runs in memory
behavior, especially when branch prediction fails. Meanwhile,
5http://software.intel.com/en-us/articles/intel-vtune-ampliﬁer-xe/

## Page 11

L1 hits L2 hits L3 hits RAM
100%
90%
80%
70%
60%
50%
40%
30%
20%
10%
0%
Bw-tree Skiplist
Fig. 7. Cache efﬁciency
Transactional log records, if they exist at all, are elsewhere.
We gain our latch-free capability from these deltas as well.
We have treated our ﬂash SSD as a generic storage device.
Hence, we have not tried to exploit parallelism within it, as
done by [20]. Exploiting this parallelism might well provide
even higher performance than we have achieved thus far.
Log structuring was ﬁrst applied to ﬁle systems [12]. But
the approach is now more widely used for both disks and as
the translation layer for ﬂash. In our system, we use solid state
disks, i.e. ﬂash based devices that are accessed via the tradi-
tional I/O interface and use a translation layer. Despite this,
we implemented our own log structured store. This enabled us
to pack pages together in our buffer so that no empty space
Bw-tree search is cache-friendly since most of its time is spent exists on ﬂash. Further, it permitted us to blur the distinction
performing binary search on a base page of keys compacted between a page store and a record store. This reduces even
into a contiguous memory block (a few deltas may be present further the storage consumed in ﬂushing pages. Storage sav-
as well). This implies the Bw-tree will do much less pointer ings improve LSS performance as garbage collection, which
chasing than the skip list.
consumes cycles, ampliﬁes writes, and wears out ﬂash, is the
The cache friendliness of the Bw-tree becomes clearer when largest cost in log structuring.
we consider the set of nodes accessed in each data structure
up to the point of reaching the data page. Over all possible
search pathways, this is precisely the set of internal nodes in
the Bw-tree, which is 1% or less of the occupied memory. For
D. Combination Efforts
Earlier indexes have used some of the elements above, though
the skip list, this is about 50% of the nodes. So, for a given not in exactly the same way. Hyder [21] uses a log structured
cache size, more of the memory accesses involved in a search store built on ﬂash memory. In Hyder, all changes propagate
hit the cache for Bw-tree over skip list.
VII. RELATED WORK
to the root of the tree. The changes at the root are batched, and
the paths being included are compressed. Hyder, at least in one
mode, supports transactions directly, with their log structured
We have beneﬁtted from prior work and in some cases have store used as both the database and the log. We do not do that,
built upon it.
A. B-trees
Anyone working with B-trees owes a debt to Bayer and
McCreight [16]. The variant that we use is the B+tree [9]. We
stretch the meaning of “page-oriented” by exploiting elastic
pages, in discontiguous pieces. The notion of elastic pages
evolved from the hashed access method record lists used by
SkimpyStash [8]. Like SkimpyStash, we consolidate the dis-
contiguous pieces of a “page” when we do garbage collection.
B. Latch Free
Until our work, it was not clear that B-trees could be made
lock or latch-free. Skip lists [13] serve as an alternative latch-
free “tree” when multiple threads must access the same page.
Partitioning is another way to avoid latches so that each thread
has its own partition of the key space [17], [18]. The Bw-tree
avoids the overhead of partitioning by using the compare and
relying instead on a transactional component when transaction
support is desired.
BFTL [22] is another example of an index implemented
over log structured ﬂash. It is perhaps the closest to the Bw-
tree among earlier work in how it’s b-tree manages storage. It
has a mapping table, called a node translation table, and writes
deltas (in our terminology) to ﬂash. It also has a process for
making the deltas contiguous on ﬂash when the number of
deltas gets large. However, the BTFL b-tree does not handle
multi-threading, concurrency control and cache management,
topics that we view as crucial to providing high throughput
performance for an atomic record store.
VIII. DISCUSSION
A. Performance Results
Our Bw-tree implementation achieves very high performance.
swap (CAS) instruction for all state changes, including the And it is sufﬁciently complete that we can measure normal
“management” state changes, e.g., SMOs and ﬂushes
C. Flash Based, Log Structured
operation performance reliably and consistently. We exploit
the database ability to cache updates until it is convenient to
post them to stable storage. This is a key to database system
Early work with ﬂash storage exploited in-page logging performance, and is explicitly enabled via control operations
[19], placing log records near a B-tree page instead of updating in the Deuteronomy architecture’s interface between TC and
the page “in-place” which would require an erase cycle. While
DC. However, exploiting it, as we have done, means that one
our delta records resemble somewhat in-page log records, they needs to be careful in comparing Bw-tree performance with
have nothing to do with transactional recovery. We view them atomic record stores that immediately make update operations
not as log records, but as the new versions of the records. stable.

## Page 12

B. Log Structured Store
fruit. While we were “conﬁdent” in our design choices, we
We have focused this paper on the Bw-tree part of our were nonetheless pleasantly surprised by how good the results
atomic record store (ARS) or data component (DC). Thus, we
were. Supporting millions of operations per second on a single
have not discussed issues related to managing LSS. A latch- “vanilla” cpu offers strong conﬁrmation for our design choices.
free environoment is very challenging for many elements of That we out-performed very good competing approaches by
data management systems, including storage management as such a large margin was “icing on the cake”.
done by our LSS. We will describe details of how the LSS is
implemented in another paper. Here we want to provide a few
highlights of areas needing attention.
• We manage our ﬂush buffer in a latch-free way. To our
knowledge this has not been done before. This means that
there is no thread blocking when items are posted to the
buffer.
• We use system transactions to capture Bw-tree structure
modiﬁcation operations (SMOs). This ensures that each
SMO can be considered atomic during recovery.
• We must keep main memory state consistent with the
state being written to LSS, non-trivial without latches.
We gave much thougth to this subtle aspect in designing
the interface to the LSS.
C. In-Page Search Optimization
Our excellent performance results were achieved without
REFERENCES
[1] “MongoDB. http://www.mongodb.org/.”
[2] J. J. Levandoski, D. B. Lomet, M. F. Mokbel, and K. Zhao, “Deuteron-
omy: Transaction Support for Cloud Data,” in CIDR, 2011, pp. 123–133.
[3] D. Lomet, A. Fekete, G. Weikum, and M. Zwilling, “Unbundling
Transaction Services in the Cloud,” in CIDR, 2009, pp. 123–133.
[4] C. Nyberg, T. Barclay, Z. Cvetanovic, J. Gray, and D. B. Lomet,
“AlphaSort: A Cache-Sensitive Parallel External Sort,” VLDB Journal,
vol. 4, no. 4, pp. 603–627, 1995.
[5] A. Ailamaki, D. J. DeWitt, M. D. Hill, and D. A. Wood, “DBMSs on
a Modern Processor: Where Does Time Go?” in VLDB, 1999, pp. 266–
277.
[6] “Amazon DynamoDB. http://aws.amazon.com/dynamodb/.”
[7] X.-Y. Hu, E. Eleftheriou, R. Haas, I. Iliadis, and R. Pletka, “Write
ampliﬁcation analysis in ﬂash-based solid state drives,” in Proceedings
of SYSTOR 2009: The Israeli Experimental Systems Conference, ser.
SYSTOR ’09, 2009, pp. 10:1–10:9.
[8] B. Debnath, S. Sengupta, and J. Li, “SkimpyStash: RAM Space Skimpy
Key-Value Store on Flash-based Storage,” in SIGMOD, 2011, pp. 25–36.
[9] D. Comer, “The Ubiquitous B-Tree,” ACM Comput. Surv., vol. 11, no. 2,
pp. 121–137, 1979.
tuning search performance using a cache sensitive page search [10] H. T. Kung and P. L. Lehman, “Concurrent manipulation of binary search
technique [23], [24]. We expect further improvement in Bw-
tree search performance as a result of implementing techniques
trees,” TODS, vol. 5, no. 3, pp. 354–382, 1980.
[11] P. L. Lehman and S. B. Yao, “Efﬁcient Locking for Concurrent Opera-
tions on B-Trees,” TODS, vol. 6, no. 4, pp. 650–670, 1981.
like these. We have no concerns about update performance, as [12] M. Rosenblum and J. Ousterhout, “The Design and Implementation of a
we would continue to use our delta record technique for that,
with all of its advantages.
D. Conclusion
We have designed and implemented the Bw-tree such that
Log-Structured File System,” ACM Trans. Comput. Syst., vol. 10, no. 1,
pp. 26–52, 1992.
[13] W. Pugh, “Skip Lists: A Probabilistic Alternative to Balanced Trees,”
Commun. ACM, vol. 33, no. 6, pp. 668–676, 1990.
[14] “MemSQL Indexes.
http://developers.memsql.com/docs/1b/indexes.html.”
[15] “Xbox LIVE. http://www.xbox.com/live.”
it can exist as a free standing atomic record store, be a data [16] R. Bayer and E. M. McCreight, “Organization and Maintenance of Large
component in a Deuteronomy style system, or be embedded in
a traditional database system. We have followed the classic ar-
Ordered Indices,” Acta Inf., vol. 1, no. 1, pp. 173–189, 1972.
[17] I. Pandis, P. T¨oz¨un, R. Johnson, and A. Ailamaki, “PLP: Page Latch-free
Shared-everything OLTP,” PVLDB, vol. 4, no. 10, pp. 610–621, 2011.
chitecture of access method layered on cache manager layered [18] J. Sewall, J. Chhugani, C. Kim, N. Satish, and P. Dubey, “PALM: Parallel
on storage manager. But at every level, we have introduced
innovations that stretch prior methods and tailor our system
for the newer hardware setting of multi-core processors and
ﬂash storage.
Our innovations, e.g. elastic pages, delta updates, shared
Architecture-Friendly Latch-Free Modiﬁcations to B+ Trees on Many-
Core Processors,” PVLDB, vol. 4, no. 11, pp. 795–806, 2011.
[19] S.-W. Lee and B. Moon, “Design of Flash-Based DBMS: An In-Page
Logging Approach,” in SIGMOD, 2007, pp. 55–66.
[20] H. Roh, S. Park, S. Kim, M. Shin, and S.-W. Lee, “B+-tree Index
Optimizations by Exploiting Internal Parallelism of Flash-based Solid
State Drives,” PVLDB, vol. 5, no. 4, pp. 286–297, 2012.
read-only state, latch-free operation, and log structured stor- [21] P. A. Bernstein, C. W. Reid, and S. Das, “Hyder - a transactional record
age, eliminate thread blocking, improve processor cache effec-
manager for shared ﬂash,” in CIDR, 2011, pp. 9–20.
tiveness, and reduce I/O demands. These techniques should [22] C.-H. Wu, T.-W. Kuo, and L. P. Chang, “An Efﬁcient B-tree Layer Im-
work well in other settings as well, including hashing and
multi-attribute access methods.
We were acutely aware that we were implementing a com-
ponent that had been successfully implemented any number
of times. In such a case, when all is said and done, it is
system performance that determines whether the effort bears
plementation for Flash-Memory Storage Systems,” ACM Trans. Embed.
Comput. Syst., vol. 6, no. 3, July 2007.
[23] S. Chen, P. B. Gibbons, T. C. Mowry, and G. Valentin, “Fractal
Prefetching B±Trees: Optimizing Both Cache and Disk Performance,”
in SIGMOD, 2002, pp. 157–168.
[24] D. B. Lomet, “The Evolution of Effective B-tree: Page Organization and
Techniques: A Personal Account,” SIGMOD Record, vol. 30, no. 3, pp.
64–69, 2001.
