// This code is a modern C++ implementation based on Dmitry Vyukov’s bounded MPMC queue algorithm.

#include <atomic>
#include <cstddef>
#include <vector>
#include <stdexcept>
#include <optional>

namespace lockfreekit {

template <typename T, size_t static_capacity = 0>
class MPMCQueue {
public:
    // Dynamic-capacity constructor (only enabled when static_capacity == 0)
    explicit MPMCQueue(size_t capacity) requires (static_capacity == 0)
        : capacity_(capacity), dynamic_buffer_(capacity) {
        if (capacity_ < 1) {
            throw std::invalid_argument("Queue capacity must be > 0");
        }
        for (size_t i = 0; i < capacity_; ++i) {
            dynamic_buffer_[i].sequence.store(i, std::memory_order_relaxed);
        }
    }

    // Static-capacity constexpr constructor (only when static_capacity > 0)
     constexpr MPMCQueue() requires (static_capacity > 0)
        : capacity_(static_capacity) {
            static_buffer_[i].sequence.store(i, std::memory_order_relaxed);
        }
    }


    [[nodiscard]] bool enqueue(const T& value) {
        size_t pos = tail_.load(std::memory_order_relaxed);

        for (;;) {
            Slot& slot = buffer_[pos % capacity_];
            const size_t seq = slot.sequence.load(std::memory_order_acquire);
            const std::ptrdiff_t diff = static_cast<std::ptrdiff_t>(seq) - static_cast<std::ptrdiff_t>(pos);

            if (diff == 0) {
                // This slot is free (it's our turn)
                if (tail_.compare_exchange_weak(pos, pos + 1, std::memory_order_relaxed)) {
                    slot.value = value;
                    // Publish to consumers
                    slot.sequence.store(pos + 1, std::memory_order_release);
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

    [[nodiscard]] std::optional<T> dequeue() {
        size_t pos = head_.load(std::memory_order_relaxed);

        for (;;) {
            Slot& slot = buffer_[pos % capacity_];
            const size_t seq = slot.sequence.load(std::memory_order_acquire);
            const std::ptrdiff_t diff = static_cast<std::ptrdiff_t>(seq) - static_cast<std::ptrdiff_t>(pos + 1);

            if (diff == 0) {
                // This slot has data (it's our turn)
                if (head_.compare_exchange_weak(pos, pos + 1, std::memory_order_relaxed)) {
                    T value = std::move(slot.value);
                    // Mark slot as free for producers
                    slot.sequence.store(pos + capacity_, std::memory_order_release);
                    return value;  // Successfully dequeued
                }
            } else if (diff < 0) {
                return std::nullopt;  // Queue empty
            } else {
                // Another consumer won → retry
                pos = head_.load(std::memory_order_relaxed);
            }
        }
    }

    [[nodiscard]] size_t approx_size() const noexcept {
        const auto head = head_.load(std::memory_order_relaxed);
        const auto tail = tail_.load(std::memory_order_relaxed);
        return tail - head;
    }

    [[nodiscard]] size_t capacity() const noexcept { return capacity_; }

    // This function must only be called when no threads
    // are concurrently enqueuing or dequeuing.
    void thread_unsafe_clear() noexcept {
        head_.store(0, std::memory_order_relaxed);
        tail_.store(0, std::memory_order_relaxed);
        for (size_t i = 0; i < capacity_; ++i) {
            buffer_[i].sequence.store(i, std::memory_order_relaxed);
        }
    }

    // Delete copy and move operations
    MPMCQueue(const MPMCQueue&) = delete;
    MPMCQueue& operator=(const MPMCQueue&) = delete;
    MPMCQueue(MPMCQueue&&) = delete;
    MPMCQueue& operator=(MPMCQueue&&) = delete;

   private:
    struct Slot {
        std::atomic<size_t> sequence;
        T value;
    };

    static constexpr auto CACHE_LINE_SIZE_ = 64;

    const size_t capacity_;
    std::vector<Slot> buffer_;
    alignas(CACHE_LINE_SIZE_) std::atomic<size_t> head_;
    char pad0[CACHE_LINE_SIZE_ - sizeof(head_)];  // Padding to avoid false sharing
    alignas(CACHE_LINE_SIZE_) std::atomic<size_t> tail_;
    char pad1[CACHE_LINE_SIZE_ - sizeof(tail_)];  // Padding to avoid false sharing
};

}  // namespace lockfreekit
