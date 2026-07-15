Assignment 2: Buffer Manager
Description
For the second programming assignment, you will design and implement a buffer manager that handles page management for a database system. The buffer manager will allow pages to be loaded, locked, and accessed concurrently by multiple threads. The core functionality of the buffer manager includes fetching pages from disk, storing them in memory, managing page replacement using the 2Q caching policy, and ensuring efficient thread-safe access to pages. Your implementation should support concurrency and ensure the correct reading and writing of pages to disk.

This assignment involves a fair amount of code, and we strongly recommend starting early to ensure sufficient time for debugging and testing.

Key Requirements
Locks and Concurrency
Use std::shared_mutex via FrameLockTable to handle shared/exclusive access to pages. Pages locked in shared mode can be accessed by multiple readers, while exclusive locks are required for write access.

When fixing a page, increment the use counter to track how many threads are accessing the page. Decrement this counter when the page is unfixed.

Ensure that locks are held for the shortest possible duration, especially during disk I/O operations, to improve concurrency.

2Q Replacement Strategy
Pages start in the FIFO queue when first loaded into memory. If they are accessed again while still in the FIFO queue, they are moved to the LRU queue.

When evicting pages, attempt to evict pages from the FIFO queue first. If all FIFO pages are still in use, eviction should occur from the LRU queue.

Dirty pages (modified pages) must be written back to disk before eviction. This operation should be done under an exclusive lock.

BufferFrame Dirty Flag
Modified pages should be marked as dirty. Dirty pages must be written to disk when evicted to prevent data loss. Track when a page is modified and handle the flushing of dirty pages during eviction.

Implementation Overview
We provide skeleton code required to implement the buffer manager. The lab is structured into a modular layout, separating interfaces (headers) and implementation. You will complete your implementation entirely within the src/buffer/buffer_manager.cpp file. All header files located in the src/include/ directory are read-only and contain the definitions of the classes and methods you must implement. The key classes include:

Page: A simple wrapper around a fixed-size page (4096 bytes) of raw memory. This is the basic unit of storage managed by the buffer manager.

BufferFrame: This class handles individual pages. It stores metadata such as page IDs, dirty flags, and whether the page is locked exclusively. A frame stores exactly one single page. If a page is “pinned”, that means it’s undergoing some operation (read/write) and it cannot be removed from a cache. However, it does not directly manage concurrency or page eviction.

BufferManager: This class controls concurrency, page replacement, and the 2Q page replacement strategy. It is responsible for managing shared and exclusive locks, page eviction, and fixing/unfixing pages. You will implement these operations, ensuring proper thread safety and efficient page access.

PageGuard: An RAII wrapper that manages the lifecycle of a fixed page. When a PageGuard goes out of scope, it unfixes the page and releases the lock via unfix_page. This design prevents resource leaks and simplifies error handling.

Policy: An interface for page replacement policies. The provided TwoQPolicy class skeleton needs to be completed to implement the 2Q caching strategy, managing FIFO and LRU queues to determine which pages to evict.

Implementation Details
The fix_page and unfix_page methods are the core of your buffer manager implementation. unfix_page is called from the PageGuard RAII wrapper. Here’s a high-level guide for implementing the key methods.

fix_page(page_id, exclusive)
Description: Retrieves a page from the buffer pool, loading it from disk if necessary, and locks it for either shared (read) or exclusive (write) access. This mechanism ensures controlled, concurrent access to pages in a multi-threaded environment.

Parameters:

page_id: The unique identifier of the page to be accessed.

exclusive: Determines the lock mode:

true: Locks the page exclusively for write operations.

false: Locks the page in shared mode for read operations.

Returns: A PageGuard object that wraps the BufferFrame. The guard automatically unfixes the page when it goes out of scope.

Steps to follow:

Check if Page is in Memory:
If the page is already in the buffer pool, update its access info in the policy.

Lock the page (shared or exclusive) and increment the use counter.

Return a PageGuard wrapping the frame.

Page Not Found in Memory:
If the page is not in memory, find a free slot in the buffer pool.

If the buffer is full, get an eviction candidate from the policy.

Ensure that the eviction process is thread-safe.

Load Page from Disk:
Load the page from disk using StorageManager.

Add the new page to the policy.

Initialize frame metadata (e.g., page ID, lock, use count).

Lock the Page:
Lock the page based on the requested access type (shared or exclusive).

If the page is locked in shared mode, multiple threads may access it concurrently. If locked exclusively, only one thread may access it.

Increment the use counter for the page.

Write Back Dirty Pages:
If evicting a dirty page (i.e., a modified page), write it back to disk before removing it from memory.

Remove the page from the policy.

unfix_page(page, is_dirty) (Private Method)
Description: This method is called automatically by the PageGuard destructor when a page guard goes out of scope. You typically won’t call this method directly. It releases a previously fixed page from the buffer pool, updating its status based on whether it was modified.

Parameters:

page: The page to be unfixed.

is_dirty: Indicates whether the page was modified during its fixed period:

true: The page was modified and should be marked as dirty

false: The page remains unmodified.

Steps to follow:

Mark Page as Dirty:
If is_dirty is true, mark the frame as dirty so it will be written to disk before eviction.

Decrement Use Counter:
Atomically decrement the use counter for the page. If the counter reaches zero, the page can potentially be evicted.

Unlock the Page:
Release the lock on the frame via FrameLockTable (shared or exclusive, based on how it was originally locked).

PageGuard
The PageGuard class follows the RAII (Resource Acquisition Is Initialization) pattern. When you call fix_page(), you receive a PageGuard object. You are required to implement the PageGuard constructor, destructor, move constructor, and move assignment operator.

{
    auto guard = buffer_manager.fix_page(page_id, false);
    // Access page data
    uint64_t* data = reinterpret_cast<uint64_t*>(guard->page_data.get());
    // if data is modified, mark the guard as dirty
    guard.setDirty();
}  // guard goes out of scope here -> page is unfixed
Key properties to implement for PageGuard:

Constructor: Must initialize the guard with a BufferManager instance and the target BufferFrame.

Automatic unfixing via Destructor: No need to manually call unfix_page(). The PageGuard’s destructor must automatically call BufferManager::unfix_page(…) to ensure the frame’s use counter is decremented and locks are released properly.

Move semantics: Guards can be moved but not copied, ensuring single ownership. You must implement the move constructor and move assignment operator to cleanly transfer ownership of the underlying page pointer without triggering an accidental unfix_page on the old moved-from object.

Exception safety: Even if an exception is thrown, the page will be properly unfixed.

Explicit dirty marking: Call guard.setDirty() when you modify the page.

Implementation Clarifications
FrameID: It can be treated as an identifier or index for a BufferFrame.

FrameLockTable: The FrameLockTable is responsible for providing fine-grained synchronization at the frame level. Instead of using a single giant lock for the entire buffer pool (which would destroy concurrency), you must use FrameLockTable::lock_shared(FrameID), lock_exclusive(FrameID), and corresponding unlock methods to protect individual BufferFrame`s while they are pinned in memory. For instance, when `fix_page retrieves a frame, it should acquire the appropriate lock using FrameLockTable before returning the PageGuard. When unfix_page is called, the lock should be released.

Use StorageManager methods to load or flush pages when necessary.

Policy class: The provided TwoQPolicy class needs to be completed. It manages the FIFO and LRU queues and determines eviction candidates.

Buffer eviction policy:

Evict from FIFO queue first.

If no evictable pages in FIFO, then evict from LRU queue.

If neither queue has evictable pages (all pages are pinned), you must throw a buffer_full_error.

2Q policy Implementation:

First access of a page: Page enters FIFO queue.

Second access while in FIFO: Page is promoted to LRU queue.

Subsequent accesses in LRU: Page is moved to the end of the LRU vector, which represents the most recently used position.

The total number of pages across both queues must be less than or equal to MAX_PAGES_IN_MEMORY.

Implementation Guidelines
Ensure thread-safety by properly managing locks and atomic operations. You need read locks for reads and write locks for modifications to avoid concurrency issues.

It is fine to use a coarse lock to synchronize access to class-level variables, but you must use FrameLockTable to ensure fine-grained read/write synchronization for pinned frames.

Don’t miss to consistently update any auxiliary data structures you create.

General Guidelines
We encourage you to complete the single-threaded implementation before moving to multi-threaded test cases.

Your implementation should pass all tests. Tests with Multithread in their names have a timeout to prevent deadlocks from blocking the testing process.

Passing earlier tests doesn’t guarantee correctness. Later cases might expose deeper flaws in eviction and synchronization logic.

Note: Gradescope will run a few hidden tests in addition to the provided tests. Ensure your implementation is robust and handles all cases correctly.

Building the Code
We have provided a Makefile to simplify the build process. You can build the code with the following command:

make clean && make
Make sure that your project compiles without any warnings, as we treat warnings as errors. The compilation will produce an executable in the build/ directory.

Testing the Code
To run a particular test case, use:

./build/lab2.out <test_number>
For example, ./build/lab2.out 2 runs the second test case. If no test number is provided, all test cases will be executed one by one.