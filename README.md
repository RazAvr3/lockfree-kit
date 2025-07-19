docker build -t workspace .
docker run -it --rm -v $(pwd):/workspace lockfreekit_workspace

focus on implementaion first
use the raylib project stuff to configure farther

TODO:
compare the implementation with this https://github.com/couchbase/phosphor/blob/master/thirdparty/dvyukov/include/dvyukov/mpmc_bounded_queue.h
understand it and refactor it.