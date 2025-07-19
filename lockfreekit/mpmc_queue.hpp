// This MPMC queue is based on Dmitry Vyukov’s bounded MPMC queue algorithm.
// Original implementation: https://www.1024cores.net/home/lock-free-algorithms/queues/bounded-mpmc-queue
// Copyright (c) 2010-2011, Dmitry Vyukov (BSD-like license).
// Modified for modern C++

#include <atomic>
#include <cstddef>
#include <vector>
#include <stdexcept>

namespace lockfreekit {

template <typename T>
class MPMCQueue {
   public:
    explicit MPMCQueue(size_t capacity) : capacity_(capacity), buffer_(capacity) {
        if (capacity_ < 1) {
            throw std::invalid_argument("Capacity must be > 0");
        }
        // Initialize sequence numbers
        for (size_t i = 0; i < capacity_; ++i) {
            buffer_[i].sequence.store(i, std::memory_order_relaxed);
        }
    }

    bool enqueue(const T& value) {
        size_t pos = tail_.load(std::memory_order_relaxed);

        for (;;) {
            Cell& cell = buffer_[pos % capacity_];
            size_t seq = cell.sequence.load(std::memory_order_acquire);
            intptr_t diff = static_cast<intptr_t>(seq) - static_cast<intptr_t>(pos);

            if (diff == 0) {
                // This slot is free (it's our turn)
                if (tail_.compare_exchange_weak(pos, pos + 1, std::memory_order_relaxed)) {
                    cell.value = value;
                    // Publish to consumers
                    cell.sequence.store(pos + 1, std::memory_order_release);
                    return true;
                }
            } else if (diff < 0) {
                // Slot not free yet → queue full
                return false;
            } else {
                // Another producer won → retry
                pos = tail_.load(std::memory_order_relaxed);
            }
        }
    }

    bool dequeue(T& result) {
        size_t pos = head_.load(std::memory_order_relaxed);

        for (;;) {
            Cell& cell = buffer_[pos % capacity_];
            size_t seq = cell.sequence.load(std::memory_order_acquire);
            intptr_t diff = static_cast<intptr_t>(seq) - static_cast<intptr_t>(pos + 1);

            if (diff == 0) {
                // This slot has data (it's our turn)
                if (head_.compare_exchange_weak(pos, pos + 1, std::memory_order_relaxed)) {
                    result = std::move(cell.value);
                    // Mark slot as free for producers
                    cell.sequence.store(pos + capacity_, std::memory_order_release);
                    return true;
                }
            } else if (diff < 0) {
                // Queue empty
                return false;
            } else {
                // Another consumer won → retry
                pos = head_.load(std::memory_order_relaxed);
            }
        }
    }

   private:
    struct Cell {
        std::atomic<size_t> sequence;
        T value;
    };

    const size_t capacity_;
    std::vector<Cell> buffer_;
    alignas(64) std::atomic<size_t> head_{0};
    alignas(64) std::atomic<size_t> tail_{0};
};

}  // namespace lockfreekit
