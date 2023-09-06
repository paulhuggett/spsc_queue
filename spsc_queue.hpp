/// A single-producer, single-consumer queue based on a public-domain
/// implementation here:
/// <https://github.com/mstump/queues/blob/master/include/spsc-bounded-queue.hpp>.

#ifndef SPSC_QUEUE_HPP
#define SPSC_QUEUE_HPP

#include <array>
#include <atomic>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <type_traits>
#include <vector>

#ifdef __cpp_lib_hardware_interference_size
using std::hardware_destructive_interference_size;
#else
constexpr inline auto hardware_destructive_interference_size = std::size_t{64};
#endif

template <typename T>
class spsc_queue {
public:
  using value_type = T;

  explicit spsc_queue (std::size_t size);

  spsc_queue (spsc_queue const &) = delete;
  spsc_queue &operator= (spsc_queue const &) = delete;

  bool push (T const &input) { return emplace (input); }
  bool push (T &&input) { return emplace (std::move (input)); }
  template <typename... Args>
  bool emplace (Args &&...args);
  std::optional<T> pop ();

private:
  using buffer_member =
    typename std::aligned_storage<sizeof (T), alignof (T)>::type;
  alignas (
    hardware_destructive_interference_size) std::vector<buffer_member> buffer_;
  std::size_t const mask_ = 0;

  alignas (
    hardware_destructive_interference_size) std::atomic<std::size_t> head_ = 0;
  alignas (
    hardware_destructive_interference_size) std::atomic<std::size_t> tail_ = 0;
};

template <typename T>
spsc_queue<T>::spsc_queue (std::size_t size)
    : buffer_ (size + 1U, buffer_member{}), mask_{size - 1U} {
  // Verify that size is a power of 2.
  assert (size != 0U && (size & (~size + 1U)) == size);
}

template <typename T>
template <typename... Args>
bool spsc_queue<T>::emplace (Args &&...args) {
  auto const head = head_.load (std::memory_order_relaxed);
  // Return immediately if there's no space in the buffer.
  if (((tail_.load (std::memory_order_acquire) - (head + 1)) & mask_) == 0) {
    return false;
  }

  new (&buffer_[head & mask_]) T{std::forward<Args...> (args...)};
  head_.store (head + 1, std::memory_order_release);
  return true;
}

template <typename T>
std::optional<T> spsc_queue<T>::pop () {
  auto const tail = tail_.load (std::memory_order_relaxed);
  if (((head_.load (std::memory_order_acquire) - tail) & mask_) == 0) {
    return {};
  }

  T *const ptr = reinterpret_cast<T *> (&buffer_[tail & mask_]);
  auto output = std::move (reinterpret_cast<T &&> (*ptr));
  ptr->~T ();
  tail_.store (tail + 1, std::memory_order_release);
  return {std::move (output)};
}

#endif  // SPSC_QUEUE_HPP
