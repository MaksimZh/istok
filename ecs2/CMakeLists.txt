add_library(ecs2 INTERFACE)
target_include_directories(ecs2 INTERFACE ./include)
target_link_libraries(ecs2 INTERFACE logging)

enable_testing()
find_package(Catch2 REQUIRED)

add_executable(ecs2_entity_unittest ./src/entity_unittest.cpp)
target_link_libraries(ecs2_entity_unittest PRIVATE ecs2)
target_link_libraries(ecs2_entity_unittest PRIVATE Catch2::Catch2WithMain)
add_test(NAME ecs2_entity_unittest COMMAND ecs2_entity_unittest)

add_executable(ecs2_component_unittest ./src/component_unittest.cpp)
target_link_libraries(ecs2_component_unittest PRIVATE ecs2)
target_link_libraries(ecs2_component_unittest PRIVATE Catch2::Catch2WithMain)
add_test(NAME ecs2_component_unittest COMMAND ecs2_component_unittest)
