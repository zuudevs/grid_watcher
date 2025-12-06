#pragma once

/**
 * @file util/ts_queue.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief 
 * @version 1.0
 * @date 2025-12-05
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include <queue>
#include <mutex>
#include <condition_variable>
#include <optional>

template<typename T>
class ThreadSafeQueue {
    std::queue<T> queue_;
    mutable std::mutex mtx_;
    std::condition_variable cv_;
    bool shutdown_ = false;

public:
    void push(T&& item) {
        {
            std::lock_guard<std::mutex> lock(mtx_);
            if(shutdown_) return;
            queue_.push(std::move(item));
        }
        cv_.notify_one();
    }
    
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
    
    void shutdown() {
        {
            std::lock_guard<std::mutex> lock(mtx_);
            shutdown_ = true;
        }
        cv_.notify_all();
    }
    
    size_t size() const {
        std::lock_guard<std::mutex> lock(mtx_);
        return queue_.size();
    }
};