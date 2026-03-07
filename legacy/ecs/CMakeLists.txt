add_library(ecs INTERFACE)
target_include_directories(ecs INTERFACE ./)
target_link_libraries(ecs INTERFACE logging tools)
