add_executable(demo demo.cpp)
target_link_libraries(demo PRIVATE ecs gui)

add_executable(upgrade upgrade.cpp)
target_link_libraries(upgrade PRIVATE ecs gui)
