#add_executable(ecsbased ecsbased.cpp)
#target_link_libraries(ecsbased PRIVATE logging ecs gui)

add_executable(experiment experiment.cpp)
target_link_libraries(experiment PRIVATE logging ecs2)
