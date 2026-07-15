#pragma once

#include <vector>
#include <mutex>
#include <set>
#include <unordered_map>
#include <array>
#include <atomic>
#include <memory>
#include <iostream>
#include "storage/storage_manager.hpp"
#include "buffer/policy.hpp"
#include "buffer/frame_lock_table.hpp"

class TwoQPolicy : public Policy {
    std::vector<PageID> fifo_queue_;
    std::vector<PageID> lru_queue_;
    mutable std::mutex mutex_;

public:
    /**
     * @brief update data structures to reflect that `page_id` was accessed at this time.
     */
    void touch(PageID page_id) override;

    /**
     * @brief remove `page_id` from the data structures.
     */
    void evict(PageID page_id) override;

    /**
     * @brief return an unpinned page which is an eviction candidate
     */
    PageID select_victim(const std::set<PageID>& pinned_pages) override;
    std::vector<PageID> get_fifo_list() const override;
    std::vector<PageID> get_lru_list() const override;
};

class BufferManager;
class PageGuard;

/**
 * A BufferFrame stores exactly one single page along with its metadata.
 * If a page is "pinned", that means it's undergoing some operation (read/write) 
 * and it cannot be removed from a cache.
 */
class BufferFrame {
    friend class BufferManager;
    friend class PageGuard;

    PageID page_id_ = INVALID_VALUE;
    FrameID frame_id_ = INVALID_VALUE;
    bool dirty_ = false;
    std::atomic<bool> exclusive_{false};

public:
    std::unique_ptr<Page> page;

    BufferFrame();
    BufferFrame(PageID pid, FrameID fid, std::unique_ptr<Page> p);

    PageID getPageId() const;
    FrameID getFrameId() const;
    bool isDirty() const;
};

class BufferManager {
public:
    static constexpr size_t MAX_PAGES = 1000;
    static constexpr size_t MAX_PAGES_IN_MEMORY = 10;
    static constexpr size_t get_max_pages() { return MAX_PAGES; }
    static constexpr size_t get_max_pages_in_memory() { return MAX_PAGES_IN_MEMORY; }

private:
    // =============================================================================
    // Main data structures - DO NOT MODIFY
    // =============================================================================

    friend class PageGuard;

    StorageManager storage_manager_;
    std::vector<std::unique_ptr<BufferFrame>> buffer_pool_;
    std::unordered_map<PageID, FrameID> page_to_frame_;


    std::array<std::atomic<int16_t>, MAX_PAGES_IN_MEMORY> use_counters_{};
    std::set<PageID> pinned_pages_;
    mutable std::mutex pinned_mutex_;

    FrameLockTable frame_lock_table_;
    std::unique_ptr<Policy> policy_;

    size_t capacity_;
    size_t page_counter_ = 0;
    mutable std::mutex buffer_mutex_;

    void evict_page();
    /**
     * Unfixes a page, marking it as no longer in use. If `is_dirty` is true, the page will
     * eventually be written to disk.
     *
     * This method is called from PageGuard's destructor.
     * It is kept private to ensure that it is only called from the friend class `PageGuard`.
     *
     * @param frame The buffer frame to unfix.
     * @param dirty Whether the page was modified.
     */
    void unfix_page(BufferFrame& frame, bool dirty);

public:
    explicit BufferManager(std::unique_ptr<Policy> policy = nullptr);
    ~BufferManager();

    PageGuard load_and_pin_page(PageID page_id, bool exclusive, BufferFrame* new_frame);
    /**
     * @brief Fixes (pins) a page in the buffer pool (loads if it is not already present)
     *  and returns a reference to it. It is thread-safe w.r.t other concurrent calls to
     *  `fix_page()` and `unfix_page()`.
     *
     * @param page_id The ID of the page to fix.
     * @param exclusive If true, acquire an exclusive lock; otherwise, acquire a shared lock.
     * @return A PageGuard that will automatically unfix the page when destroyed.
     */
    PageGuard fix_page(PageID page_id, bool exclusive);
    void flushPage(FrameID frame_id);

    // =============================================================================
    // Buffer APIs - DO NOT MODIFY
    // =============================================================================
    size_t getNumPages() const { return storage_manager_.num_pages; }
    std::vector<PageID> get_fifo_list() const { return policy_->get_fifo_list(); }
    std::vector<PageID> get_lru_list() const { return policy_->get_lru_list(); }
    FrameLockTable& frame_lock_table() { return frame_lock_table_; }
    const FrameLockTable& frame_lock_table() const { return frame_lock_table_; }

    void printBufferPool() const {
        std::cout << "Buffer Pool Contents:\n";
        for (const auto& frame : buffer_pool_) {
            if (frame) {
                std::cout << "Frame ID: " << frame->getFrameId()
                          << ", Page ID: " << frame->getPageId()
                          << ", Dirty: " << (frame->isDirty() ? "Yes" : "No")
                          << "\n";
            }
        }
        std::cout << "---- End of Buffer Pool ----\n";
    }
};

/**
 * PageGuard is an RAII wrapper that automatically unfixes a page when it goes
 * out of scope. Similar to std::unique_ptr, it only supports move semantics.
 *
 * When a PageGuard is destroyed, it calls BufferManager::unfix_page()
 * to release the page lock and update pin counts.
 */
class PageGuard {
    BufferManager* bm_ = nullptr;
    BufferFrame* frame_ = nullptr;
    bool dirty_ = false;

public:
    PageGuard() = default;

    /**
     * @brief Construct a PageGuard that manages a fixed page.
     * @param bm Pointer to the BufferManager that owns the page.
     * @param frame Pointer to the BufferFrame containing the page.
     */
    PageGuard(BufferManager* bm, BufferFrame* frame);

    /**
     * @brief Destructor: automatically unfixes the page if this guard owns one.
     */
    ~PageGuard();

    PageGuard(const PageGuard&) = delete;
    PageGuard& operator=(const PageGuard&) = delete;

    /**
     * @brief Move CTOR: transfers ownership from another PageGuard.
     */
    PageGuard(PageGuard&& other) noexcept;

    /**
     * @brief Move assignment: transfers ownership from another PageGuard.
     */
    PageGuard& operator=(PageGuard&& other) noexcept;

    // =============================================================================
    // Cosmetics operators - DO NOT MODIFY
    // =============================================================================
    Page* operator->() { return frame_ ? frame_->page.get() : nullptr; }
    Page& operator*() { return *frame_->page; }
    const Page* operator->() const { return frame_ ? frame_->page.get() : nullptr; }
    const Page& operator*() const { return *frame_->page; }

    void setDirty() { dirty_ = true; }

    PageID getPageId() const { return frame_ ? frame_->page_id_ : INVALID_VALUE; }
    FrameID getFrameId() const { return frame_ ? frame_->frame_id_ : INVALID_VALUE; }
    BufferFrame* getFrame() { return frame_; }

    explicit operator bool() const { return frame_ != nullptr; }
};
