#include <iostream>
#include <thread>

#include "spsc_queue.hpp"

int main () {
  spsc_queue<int> q{8};
  std::atomic_bool done{false};
  static constexpr auto max = 500;

  std::thread producer{[&] () {
    for (auto ctr = 0; ctr < max; ++ctr) {
      while (!q.push (ctr)) {
        std::this_thread::yield ();
      }
    }
  }};
  std::thread consumer{[&] () {
    auto expected = 0;
    bool running = true;
    while (running) {
      if (std::optional<int> const v = q.pop ()) {
        assert (v == expected);
        ++expected;
      } else {
        running = !done.load (std::memory_order_relaxed);
        std::this_thread::yield ();
      }
    }
    assert (expected == max);
  }};

  producer.join ();
  // Tell the consumer to stop once it has drained the queue.
  done.store (true, std::memory_order_relaxed);
  consumer.join ();
}
