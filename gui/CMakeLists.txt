add_library(gui INTERFACE)
target_include_directories(gui INTERFACE ./)
target_link_libraries(gui INTERFACE tools ecs)
