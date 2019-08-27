cmake_minimum_required(VERSION 3.7)
enable_testing()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS_DEBUG "-O0 -fno-omit-frame-pointer -g -fsanitize=address -Wall -Wextra -Werror")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -Wall -Wextra -Werror -g")

project(any_invokable)

include_directories(./)
include_directories(./include)

add_subdirectory(catch)
add_subdirectory(tests)
