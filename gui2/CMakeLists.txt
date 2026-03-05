add_library(gui INTERFACE)
target_include_directories(gui INTERFACE ./include)
target_link_libraries(gui INTERFACE ecs)
target_link_libraries(gui INTERFACE logging)

add_subdirectory(test)
