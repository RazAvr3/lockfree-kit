docker build -t lockfreekit_workspace .
docker run -it --rm -v $(pwd):/lockfreekit_workspace lockfreekit_workspace

focus on implementaion first
use the raylib project stuff to configure farther

TODO:
!!!!understand the algorithm deeper + memory ordersl!!!!
add perfect farwarding and move on to the below tasks

add tests (catch2)
add examples
add documentation
add benchmarks
add CI/CD
add build scripts