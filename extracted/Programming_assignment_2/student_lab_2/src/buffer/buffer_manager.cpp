#include "buffer/buffer_manager.hpp"
#include "buffer/exceptions.hpp"
#include <algorithm>

using namespace std;

// TwoQPolicy
void TwoQPolicy::touch(PageID page_id) {
    lock_guard<mutex> lock(mutex_);

    // If we see this page in FIFO, then it means this page has been accessed for the second time, so we move it to the back of LRU.
    auto fifoIn = find(fifo_queue_.begin(), fifo_queue_.end(), page_id);
    if (fifoIn != fifo_queue_.end()) {
        fifo_queue_.erase(fifoIn);
        lru_queue_.push_back(page_id);
        return;
    }

    // If we see this page in LRU, then we move it to the front of LRU to mark it as most recently used.
    auto lruIn = find(lru_queue_.begin(), lru_queue_.end(), page_id);
    if (lruIn != lru_queue_.end()) {
        lru_queue_.erase(lruIn);
        lru_queue_.push_back(page_id);
        return;
    }

    // If we see this page for the first time
    fifo_queue_.push_back(page_id);
}

void TwoQPolicy::evict(PageID page_id) {
    lock_guard<mutex> lock(mutex_);
    auto fifoIn = find(fifo_queue_.begin(), fifo_queue_.end(), page_id);
    if (fifoIn != fifo_queue_.end()) {
        fifo_queue_.erase(fifoIn);
        return;
    }

    auto lruIn = find(lru_queue_.begin(), lru_queue_.end(), page_id);
    if (lruIn != lru_queue_.end()) {
        lru_queue_.erase(lruIn);
        return;
    }
}

PageID TwoQPolicy::select_victim(const std::set<PageID>& pinned_pages) {
    lock_guard<mutex> lock(mutex_);

    // First try to evict from FIFO queue, then try LRU queue.
    for (PageID page_id : fifo_queue_) {
        if (pinned_pages.count(page_id) == 0) {
            return page_id;
        }
    }

    for (PageID page_id : lru_queue_) {
        if (pinned_pages.count(page_id) == 0) {
            return page_id;
        }
    }

    // If all pages are pinned, we cannot evict any page.
    return INVALID_VALUE;
}

std::vector<PageID> TwoQPolicy::get_fifo_list() const {
    lock_guard<mutex> lock(mutex_);
    return fifo_queue_;
}

std::vector<PageID> TwoQPolicy::get_lru_list() const {
    lock_guard<mutex> lock(mutex_);
    return lru_queue_;
}

// BufferFrame
BufferFrame::BufferFrame() = default;

BufferFrame::BufferFrame(PageID pid, FrameID fid, std::unique_ptr<Page> p)
    : page_id_(pid), frame_id_(fid), page(std::move(p)) {}

PageID BufferFrame::getPageId() const { return page_id_; }
FrameID BufferFrame::getFrameId() const { return frame_id_; }
bool BufferFrame::isDirty() const { return dirty_; }

BufferManager::BufferManager(unique_ptr<Policy> policy) : storage_manager_() {
    capacity_ = BufferManager::get_max_pages_in_memory();

    if (policy) {
        policy_ = move(policy);
    } else {
        policy_ = make_unique<TwoQPolicy>();
    }

    buffer_pool_.resize(capacity_);
    for (FrameID i = 0; i < capacity_; ++i) {
        buffer_pool_[i] = make_unique<BufferFrame>();
        buffer_pool_[i]->frame_id_ = i;
    }
}

BufferManager::~BufferManager() {
    // persist all dirty pages to disk before destruction
    for (auto& frame : buffer_pool_) {
        if (frame) {
            flushPage(frame->frame_id_);
        }
    }
}

void BufferManager::flushPage(FrameID frame_id) {
    BufferFrame& frame = *buffer_pool_[frame_id];
    if (frame.isDirty() && frame.page_id_ != INVALID_VALUE) {
        storage_manager_.flush(frame.page_id_, *frame.page);
        frame.dirty_ = false;
    }
}

PageGuard BufferManager::load_and_pin_page(PageID page_id, bool exclusive, BufferFrame* new_frame) {
    UNUSED(new_frame);
    return fix_page(page_id, exclusive);
}

PageGuard BufferManager::fix_page(PageID page_id, bool exclusive) {
    // Extend storage if needed
    if (page_id >= storage_manager_.num_pages) {
        storage_manager_.extend(page_id);
    }

    FrameID frame_id = 0;
    {
        lock_guard<mutex> lock(buffer_mutex_);

        // Check if page is already in buffer pool
        auto it = page_to_frame_.find(page_id);
        if (it != page_to_frame_.end()) {
            frame_id = it->second;
        } else {
            if (page_to_frame_.size() < capacity_) {
                for (FrameID i = 0; i < capacity_; ++i) {
                    if (buffer_pool_[i]->page_id_ == INVALID_VALUE) {
                        frame_id = i;
                        break;
                    }
                }
            } else {
                PageID victim_page_id = policy_->select_victim(pinned_pages_);
                if (victim_page_id == INVALID_VALUE) {
                    throw buffer_full_error();
                }

                frame_id = page_to_frame_[victim_page_id];
                flushPage(frame_id);
                page_to_frame_.erase(victim_page_id);
                policy_->evict(victim_page_id);
            }

            BufferFrame& frame = *buffer_pool_[frame_id];
            frame.page_id_ = page_id;
            frame.page = storage_manager_.load(page_id);
            frame.dirty_ = false;
            frame.frame_id_ = frame_id;
            page_to_frame_[page_id] = frame_id;
        }

        policy_->touch(page_id);
        use_counters_[frame_id]++;
        pinned_pages_.insert(page_id);
    }

    BufferFrame& frame = *buffer_pool_[frame_id];
    if (exclusive) {
        frame_lock_table_.lock_exclusive(frame_id);
    } else {
        frame_lock_table_.lock_shared(frame_id);
    }
    frame.exclusive_.store(exclusive);

    return PageGuard(this, &frame);
}

// Evict is not used in fix_page, since eviction logic is handled in fix_page itself.
void BufferManager::evict_page() {
    throw method_not_implemented_exception();
}

void BufferManager::unfix_page(BufferFrame& frame, bool dirty) {
    if (dirty) {
        frame.dirty_ = true;
    }

    bool was_exclusive = frame.exclusive_.load();
    FrameID frame_id = frame.frame_id_;
    PageID page_id = frame.page_id_;

    if (was_exclusive) {
        frame_lock_table_.unlock_exclusive(frame_id);
    } else {
        frame_lock_table_.unlock_shared(frame_id);
    }

    lock_guard<mutex> lock(buffer_mutex_);
    if (--use_counters_[frame_id] == 0) {
        pinned_pages_.erase(page_id);
    }
}

// PageGuard
PageGuard::PageGuard(BufferManager* bm, BufferFrame* frame) {
    bm_ = bm;
    frame_ = frame;
}

PageGuard::~PageGuard() {
    if (bm_ && frame_) {
        bm_->unfix_page(*frame_, dirty_);
    }
}

PageGuard::PageGuard(PageGuard&& other) noexcept {
    bm_ = other.bm_;
    frame_ = other.frame_;
    dirty_ = other.dirty_;

    other.bm_ = nullptr;
    other.frame_ = nullptr;
    other.dirty_ = false;
}

PageGuard& PageGuard::operator=(PageGuard&& other) noexcept {
    if (this != &other) {
        if (bm_ && frame_) {
            bm_->unfix_page(*frame_, dirty_);
        }

        bm_ = other.bm_;
        frame_ = other.frame_;
        dirty_ = other.dirty_;

        other.bm_ = nullptr;
        other.frame_ = nullptr;
        other.dirty_ = false;
    }
    return *this;
}
