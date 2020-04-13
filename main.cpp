#include <iostream>
#include <thread>

#include "spsc_queue.hpp"

int main () {
    spsc_queue<int> q{512};
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
        constexpr auto empty = spsc_queue<int>::empty;
        auto expected = 0;
        for (;;) {
            int v = 0;
            while ((v = q.pop ()) == empty && !done) {
                std::this_thread::yield ();
            }
            if (v == empty && done.load (std::memory_order_relaxed)) {
                break;
            }
            assert (v == expected);
            ++expected;
        }

        assert (expected == max);
    }};

    producer.join ();
    done.store (true, std::memory_order_relaxed);
    consumer.join ();
}
