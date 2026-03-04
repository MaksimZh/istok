add_library(logging STATIC ./src/logging.cpp)
target_include_directories(logging PUBLIC ./include)

enable_testing()
find_package(Catch2 REQUIRED)

add_executable(logging_unittest ./test/logging_unittest.cpp)
target_link_libraries(logging_unittest PRIVATE logging)
target_link_libraries(logging_unittest PRIVATE Catch2::Catch2WithMain)
add_test(NAME logging_unittest COMMAND logging_unittest)
