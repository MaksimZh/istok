add_library(ecs INTERFACE)
target_include_directories(ecs INTERFACE ./include)
target_link_libraries(ecs INTERFACE logging)

enable_testing()
find_package(Catch2 REQUIRED)

add_executable(ecs_entity_unittest ./test/entity_unittest.cpp)
target_link_libraries(ecs_entity_unittest PRIVATE ecs)
target_link_libraries(ecs_entity_unittest PRIVATE Catch2::Catch2WithMain)
add_test(NAME ecs_entity_unittest COMMAND ecs_entity_unittest)

add_executable(ecs_component_unittest ./test/component_unittest.cpp)
target_link_libraries(ecs_component_unittest PRIVATE ecs)
target_link_libraries(ecs_component_unittest PRIVATE Catch2::Catch2WithMain)
add_test(NAME ecs_component_unittest COMMAND ecs_component_unittest)

add_executable(ecs_unittest ./test/ecs_unittest.cpp)
target_link_libraries(ecs_unittest PRIVATE ecs)
target_link_libraries(ecs_unittest PRIVATE Catch2::Catch2WithMain)
add_test(NAME ecs_unittest COMMAND ecs_unittest)
