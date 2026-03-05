add_library(gui INTERFACE)
target_include_directories(gui INTERFACE ./include)
target_link_libraries(gui INTERFACE ecs)
target_link_libraries(gui INTERFACE logging)

enable_testing()
find_package(Catch2 REQUIRED)

add_executable(gui_unittest ./test/gui_unittest.cpp)
target_include_directories(gui_unittest INTERFACE ./test)
target_link_libraries(gui_unittest PRIVATE gui)
target_link_libraries(gui_unittest PRIVATE Catch2::Catch2WithMain)
add_test(NAME gui_unittest COMMAND gui_unittest)
