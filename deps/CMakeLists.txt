project(catch2)

add_library(${PROJECT_NAME} catch.cpp)
target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Catch2/single_include>)
