/// A single-producer, single-consumer queue based on a public-domain implementation
/// here: <https://github.com/mstump/queues/blob/master/include/spsc-bounded-queue.hpp>.

#ifndef SPSC_QUEUE_HPP
#define SPSC_QUEUE_HPP

#include <array>
#include <atomic>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <vector>

#include "empty_value.hpp"

template <typename T, T EmptyValue = empty_value<T>::value>
class spsc_queue {
public:
    using value_type = T;

    explicit spsc_queue (std::size_t size);

    spsc_queue (spsc_queue const &) = delete;
    spsc_queue & operator= (spsc_queue const &) = delete;

    bool push (T const & input);
    T pop ();

    static constexpr auto empty = EmptyValue;

private:
    using cache_line_pad = std::array<std::uint8_t, 64>;

    cache_line_pad const pad0_;
    std::vector<T> buffer_;
    std::size_t const mask_;

    cache_line_pad const pad1_;
    std::atomic<std::size_t> head_;

    cache_line_pad const pad2_;
    std::atomic<std::size_t> tail_;
};

template <typename T, T EmptyValue>
spsc_queue<T, EmptyValue>::spsc_queue (std::size_t size)
        : pad0_{}
        , buffer_ (size + 1U, empty)
        , mask_{size - 1U}
        , pad1_{}
        , head_{0}
        , pad2_{}
        , tail_{0} {
    // Verify that size is a power of 2.
    assert (size != 0U && (size & (~size + 1U)) == size);
}

template <typename T, T EmptyValue>
bool spsc_queue<T, EmptyValue>::push (T const & input) {
    auto const head = head_.load (std::memory_order_relaxed);
    if (((tail_.load (std::memory_order_acquire) - (head + 1)) & mask_) >= 1) {
        buffer_[head & mask_] = input;
        head_.store (head + 1, std::memory_order_release);
        return true;
    }
    return false;
}

template <typename T, T EmptyValue>
T spsc_queue<T, EmptyValue>::pop () {
    auto const tail = tail_.load (std::memory_order_relaxed);
    if (((head_.load (std::memory_order_acquire) - tail) & mask_) >= 1) {
        auto const output = std::move (buffer_[tail & mask_]);
        tail_.store (tail + 1, std::memory_order_release);
        return output;
    }
    return empty;
}

#endif // SPSC_QUEUE_HPP
