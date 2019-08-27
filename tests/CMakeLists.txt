enable_testing()

project(invokable_ut)

set(SOURCE_FILES
    invocable_ut.cpp)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} catch)

add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})
