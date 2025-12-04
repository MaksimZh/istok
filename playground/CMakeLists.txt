add_executable(demo demo.cpp)
target_link_libraries(demo PRIVATE ecs gui)
add_executable(ecsbased ecsbased.cpp)
target_link_libraries(ecsbased PRIVATE logging ecs gui)
