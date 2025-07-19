#include "mpmc_queue.hpp"

int main() {
    lockfreekit::MPMCQueue<int> q(4);
    q.enqueue(1);
    q.enqueue(2);
    q.enqueue(3);
    q.enqueue(4);
    q.enqueue(5);
    q.enqueue(5);
    q.enqueue(5);
    q.enqueue(5);
    q.enqueue(5);

    q.enqueue(5);
    q.enqueue(5);

}