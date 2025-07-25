docker build -t lockfreekit_workspace .
docker run -it --rm -v $(pwd):/workspace lockfreekit_workspace

focus on implementaion first
use the raylib project stuff to configure farther

TODO:
understand the algorithm
refactor it:
	add nodiscard
	add noexcept
	add const/constexpr where possible
	add error handling
	add performance optimizations
	add more features - size, clear, empty, etc.
	delete other constructors/assignments.
	perfect farwarding
	emplace?
	logging only on debug builds? make a nicer logging system
	read about false sharing and padding

add tests
add examples
add documentation
add benchmarks
add CI/CD
add build scripts