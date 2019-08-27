project(catch)

set(HEADER_FILE
    catch.h)
set(SOURCE_FILES
    catch.cpp)

add_library(${PROJECT_NAME} ${SOURCE_FILES} ${HEADER_FILES})
