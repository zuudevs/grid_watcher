/**
 * @file util/ts_queue.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Defines a thread-safe queue template class for inter-thread communication.
 * @version 1.0.0
 * @date 2025-12-05
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>
#include <optional>

/**
 * @brief A thread-safe queue implementation using mutex and condition variable.
 * 
 * This class allows multiple threads to push and pop elements safely.
 * It supports a shutdown mechanism to unblock waiting consumers.
 * 
 * @tparam T The type of elements stored in the queue.
 */
template<typename T>
class ThreadSafeQueue {
    std::queue<T> queue_;
    mutable std::mutex mtx_;
    std::condition_variable cv_;
    bool shutdown_ = false;

public:
    /**
     * @brief Pushes an item into the queue.
     * 
     * This operation is thread-safe. If the queue is shut down, the item is discarded.
     * 
     * @param item The item to push (moved into the queue).
     */
    void push(T&& item) {
        {
            std::lock_guard<std::mutex> lock(mtx_);
            if(shutdown_) return;
            queue_.push(std::move(item));
        }
        cv_.notify_one();
    }
    
    /**
     * @brief Pops an item from the queue.
     * 
     * Blocks the calling thread until an item is available or the queue is shut down.
     * 
     * @return std::optional<T> The popped item, or std::nullopt if the queue was shut down and empty.
     */
    std::optional<T> pop() {
        std::unique_lock<std::mutex> lock(mtx_);
        cv_.wait(lock, [this]{ return !queue_.empty() || shutdown_; });
        
        if(shutdown_ && queue_.empty()) {
            return std::nullopt;
        }
        
        T item = std::move(queue_.front());
        queue_.pop();
        return item;
    }
    
    /**
     * @brief Signals the queue to shut down.
     * 
     * Sets the shutdown flag and notifies all waiting threads.
     * Subsequent pushes will be ignored, and pops will return std::nullopt once the queue is empty.
     */
    void shutdown() {
        {
            std::lock_guard<std::mutex> lock(mtx_);
            shutdown_ = true;
        }
        cv_.notify_all();
    }
    
    /**
     * @brief Returns the current number of elements in the queue.
     * 
     * @return size_t The number of elements.
     */
    size_t size() const {
        std::lock_guard<std::mutex> lock(mtx_);
        return queue_.size();
    }
};