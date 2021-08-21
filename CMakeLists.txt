cmake_minimum_required(VERSION 3.7)

project(any_invokable)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(./include)

add_subdirectory(deps)

enable_testing()
add_subdirectory(test)
