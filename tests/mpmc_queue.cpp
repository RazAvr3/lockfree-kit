#include <iostream>
#include <thread>
#include <vector>

#include "mpmc_queue.hpp"

int main() {
    using namespace lockfreekit;

    // Example 1: Dynamic-capacity queue (runtime)
    MPMCQueue<int> dyn_queue(8);  // capacity = 8

    // Enqueue a few numbers
    for (int i = 0; i < 5; ++i) {
        if (dyn_queue.enqueue(i)) {
            std::cout << "Enqueued " << i << " (dynamic)\n";
        }
    }

    // Dequeue them
    while (auto val = dyn_queue.dequeue()) {
        std::cout << "Dequeued " << *val << " (dynamic)\n";
    }

    std::cout << "Dynamic queue approx size: " << dyn_queue.approx_size() << "\n\n";

    // Example 2: Static-capacity queue (compile-time, 16 slots)
    MPMCQueue<int, 16> static_queue;

    // Producer thread
    std::thread producer([&] {
        for (int i = 100; i < 105; ++i) {
            while (!static_queue.enqueue(i)) {
                std::this_thread::yield();  // wait until space
            }
            std::cout << "Produced " << i << " (static)\n";
        }
    });

    // Consumer thread
    std::thread consumer([&] {
        for (int count = 0; count < 5; ) {
            auto val = static_queue.dequeue();
            if (val) {
                std::cout << "Consumed " << *val << " (static)\n";
                ++count;
            } else {
                std::this_thread::yield();  // wait for producer
            }
        }
    });

    producer.join();
    consumer.join();

    std::cout << "Static queue approx size: " << static_queue.approx_size() << "\n";
    return 0;
}