#include "mpmc_queue.hpp"

int main() {
    lockfreekit::MPMCQueue<int> q(4);
    bool a;
    a = q.enqueue(1);
    a = q.enqueue(2);
    a = q.enqueue(3);
    a = q.enqueue(4);
    a = q.enqueue(5);
    a = q.enqueue(5);
    a = q.enqueue(5);
    a = q.enqueue(5);
    a = q.enqueue(5);
    a = q.enqueue(5);
    a = q.enqueue(5);

}