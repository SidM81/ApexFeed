#ifndef CORE_SPSC_RING_BUFFER_H
#define CORE_SPSC_RING_BUFFER_H

#include <array>
#include <atomic>
#include <cstddef>
#include <utility>

namespace apex {

template <typename T, std::size_t Capacity>
class SpscRingBuffer {
    static_assert(Capacity > 1, "Capacity must be > 1");

public:
    SpscRingBuffer() = default;
    ~SpscRingBuffer() = default;

    SpscRingBuffer(const SpscRingBuffer&) = delete;
    SpscRingBuffer& operator=(const SpscRingBuffer&) = delete;

    bool tryPush(const T& value) {
        const std::size_t head = head_.load(std::memory_order_relaxed);
        const std::size_t next = increment(head);
        if (next == tail_.load(std::memory_order_acquire)) {
            return false;
        }

        buffer_[head] = value;
        head_.store(next, std::memory_order_release);
        return true;
    }

    bool tryPush(T&& value) {
        const std::size_t head = head_.load(std::memory_order_relaxed);
        const std::size_t next = increment(head);
        if (next == tail_.load(std::memory_order_acquire)) {
            return false;
        }

        buffer_[head] = std::move(value);
        head_.store(next, std::memory_order_release);
        return true;
    }

    bool tryPop(T& out) {
        const std::size_t tail = tail_.load(std::memory_order_relaxed);
        if (tail == head_.load(std::memory_order_acquire)) {
            return false;
        }

        out = std::move(buffer_[tail]);
        tail_.store(increment(tail), std::memory_order_release);
        return true;
    }

    [[nodiscard]] std::size_t capacity() const {
        return Capacity - 1;
    }

    [[nodiscard]] std::size_t size() const {
        const std::size_t head = head_.load(std::memory_order_acquire);
        const std::size_t tail = tail_.load(std::memory_order_acquire);
        if (head >= tail) {
            return head - tail;
        }
        return Capacity - (tail - head);
    }

private:
    static constexpr std::size_t increment(std::size_t idx) {
        return (idx + 1) % Capacity;
    }

private:
    alignas(64) std::array<T, Capacity> buffer_{};
    alignas(64) std::atomic<std::size_t> head_{0};
    alignas(64) std::atomic<std::size_t> tail_{0};
};

}  // namespace apex

#endif
