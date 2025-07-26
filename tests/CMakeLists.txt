# Create test executable
add_executable(tests
    mpmc_queue.cpp
)

# Add the header directory for the test
target_include_directories(tests PRIVATE
${CMAKE_SOURCE_DIR}/include
)
