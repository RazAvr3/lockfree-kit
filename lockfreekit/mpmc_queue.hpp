#include <atomic>
#include <cstddef>
#include <vector>
#include <array>
#include <optional>
#include <stdexcept>
#include <concepts>
#include <type_traits>

namespace lockfreekit {

template <typename T>
concept QueueValue = std::default_initializable<T> &&(std::movable<T> || std::copyable<T>);

template <typename T, size_t static_capacity = 0>
requires QueueValue<T>
class MPMCQueue {
   public:
    // Dynamic-capacity constructor
    explicit MPMCQueue(size_t capacity) requires(static_capacity == 0)
        : capacity_(capacity), dynamic_buffer_(capacity) {
        if (capacity_ < 1) {
            throw std::invalid_argument("Queue capacity must be > 0");
        }
        for (size_t i = 0; i < capacity_; ++i) {
            dynamic_buffer_[i].sequence.store(i, std::memory_order_relaxed);
        }
    }

    // Static-capacity consetxpr constructor
    constexpr MPMCQueue() requires(static_capacity > 0) {
        for (size_t i = 0; i < static_capacity; ++i) {
            static_buffer_[i].sequence.store(i, std::memory_order_relaxed);
        }
    }

    [[nodiscard]] bool enqueue(const T& value) {
        size_t pos = tail_.load(std::memory_order_relaxed);

        for (;;) {
            Slot& slot = buffer_at(pos);
            // Acquire ensures that if the slot's sequence indicates it is free (diff == 0),
            // then any writes by the previous consumer (like resetting the slot's state and
            // moving out its value) — which happened before its release-store — are now
            // visible to us. This guarantees we won't write into a slot before the consumer
            // has fully finished with it.
            const size_t seq = slot.sequence.load(std::memory_order_acquire);
            const std::ptrdiff_t diff = static_cast<std::ptrdiff_t>(seq) - static_cast<std::ptrdiff_t>(pos);

            if (diff == 0) {
                if (tail_.compare_exchange_weak(pos, pos + 1, std::memory_order_relaxed)) {
                    slot.value = value;
                    slot.sequence.store(pos + 1, std::memory_order_release);
                    return true;
                }
            } else if (diff < 0) {
                return false;  // Full
            } else {
                pos = tail_.load(std::memory_order_relaxed);
            }
        }
    }

    [[nodiscard]] std::optional<T> dequeue() {
        size_t pos = head_.load(std::memory_order_relaxed);

        for (;;) {
            Slot& slot = buffer_at(pos);
            // Acquire ensures that if `seq` shows the slot is ready (diff == 0),
            // then any writes to slot.value by the producer (done before its release-store)
            // are visible here, so we can safely read/move the value.
            const size_t seq = slot.sequence.load(std::memory_order_acquire);
            const std::ptrdiff_t diff = static_cast<std::ptrdiff_t>(seq) - static_cast<std::ptrdiff_t>(pos + 1);

            if (diff == 0) {
                if (head_.compare_exchange_weak(pos, pos + 1, std::memory_order_relaxed)) {
                    T value = std::move(slot.value);
                    slot.sequence.store(pos + capacity(), std::memory_order_release);
                    return value;
                }
            } else if (diff < 0) {
                return std::nullopt;  // Empty
            } else {
                pos = head_.load(std::memory_order_relaxed);
            }
        }
    }

    [[nodiscard]] size_t approx_size() const noexcept {
        return tail_.load(std::memory_order_relaxed) - head_.load(std::memory_order_relaxed);
    }

    [[nodiscard]] constexpr size_t capacity() const noexcept {
        if constexpr (static_capacity > 0) {
            return static_capacity;
        } else {
            return capacity_;
        }
    }

    void thread_unsafe_clear() noexcept {
        head_.store(0, std::memory_order_relaxed);
        tail_.store(0, std::memory_order_relaxed);
        for (size_t i = 0; i < capacity(); ++i) {
            buffer_at(i).sequence.store(i, std::memory_order_relaxed);
        }
    }

    // Delete copy/move constructors and assignment operators
    MPMCQueue(const MPMCQueue&) = delete;
    MPMCQueue& operator=(const MPMCQueue&) = delete;
    MPMCQueue(MPMCQueue&&) = delete;
    MPMCQueue& operator=(MPMCQueue&&) = delete;

   private:
    struct Slot {
        std::atomic<size_t> sequence;
        T value;
    };

    Slot& buffer_at(size_t pos) noexcept {
        if constexpr (static_capacity > 0) {
            // Optimize for power-of-two capacity with bitmask
            if constexpr ((static_capacity & (static_capacity - 1)) == 0) {
                return static_buffer_[pos & (static_capacity - 1)];
            } else {
                return static_buffer_[pos % static_capacity];
            }
        } else {
            return dynamic_buffer_[pos % capacity_];
        }
    }

    static constexpr size_t CACHE_LINE_SIZE_ = 64;

    [[no_unique_address]] std::conditional_t<(static_capacity > 0), std::array<Slot, static_capacity>, char>
        static_buffer_{};
    std::vector<Slot> dynamic_buffer_;         // Only used if static_capacity == 0
    const size_t capacity_ = static_capacity;  // Only meaningful for dynamic case

    alignas(CACHE_LINE_SIZE_) std::atomic<size_t> head_{};
    char pad0[CACHE_LINE_SIZE_ - sizeof(head_)]{};
    alignas(CACHE_LINE_SIZE_) std::atomic<size_t> tail_{};
    char pad1[CACHE_LINE_SIZE_ - sizeof(tail_)]{};
};

}  // namespace lockfreekit
