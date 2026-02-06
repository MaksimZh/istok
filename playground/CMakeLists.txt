add_executable(ecsbased ecsbased.cpp)
target_link_libraries(ecsbased PRIVATE logging ecs gui)

add_executable(second second.cpp)
target_link_libraries(second PRIVATE logging ecs2 gui)
