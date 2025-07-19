cmake_minimum_required(VERSION 3.16)
project(LockfreeKit LANGUAGES CXX)

# C++23
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# For clangd (compile_commands.json)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Add subdirectory for tests
add_subdirectory(tests)
