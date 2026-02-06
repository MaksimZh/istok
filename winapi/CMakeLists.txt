add_library(winapi STATIC ./src/window.cpp)
target_include_directories(winapi PUBLIC ./include)
target_link_libraries(winapi PUBLIC logging)
target_link_libraries(winapi PUBLIC ecs2)
