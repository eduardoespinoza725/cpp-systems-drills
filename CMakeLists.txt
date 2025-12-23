cmake_minimum_required(VERSION 3.20)
project(cpp-systems-drills LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(drills INTERFACE)
target_compile_options(drills INTERFACE
    -Wall -Wextra -Wpedantic
    -Wshadow -Wconversion -Wsign-conversion
    -Wformat=2 -Wnull-dereference -Wdouble-promotion
    -pg
)

option(ENABLE_SANITIZERS "Enable ASan/UBSan in Debug" ON)
if(ENABLE_SANITIZERS AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(drills INTERFACE -fsanitize=address,undefined -fno-omit-frame-pointer)
    target_link_options(drills INTERFACE -fsanitize=address,undefined -fno-omit-frame-pointer)
endif()

enable_testing()
add_executable(test_unique_fd tests/test_unique_fd.cpp)
target_link_libraries(test_unique_fd PRIVATE drills -pg)
add_test(NAME test_unique_fd COMMAND test_unique_fd)
