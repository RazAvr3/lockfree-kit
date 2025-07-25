docker build -t lockfreekit_workspace .
docker run -it --rm -v $(pwd):/workspace lockfreekit_workspace

focus on implementaion first
use the raylib project stuff to configure farther

TODO:
!!!!understand the algorithm deeper + memory ordersl!!!!
refactor it:
    use the capactiy option static/dynamic chat suggested. use same capacity
    check templated (concepts, static_Assert, sfine?)
	add const/constexpr where possible
	add performance optimizations
	perfect farwarding
	emplace?

add tests
add examples
add documentation
add benchmarks
add CI/CD
add build scripts