add_library(ecs INTERFACE)
target_include_directories(ecs INTERFACE ./include)
target_link_libraries(ecs INTERFACE logging)

add_subdirectory(test)
