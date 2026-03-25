enable_testing()
find_package(Catch2 REQUIRED)
find_package(Trompeloeil REQUIRED)

add_executable(ecs_unittest
    component_unittest.cpp
    ecs_unittest.cpp
    entity_unittest.cpp
    system_unittest.cpp
)
target_link_libraries(ecs_unittest PRIVATE ecs Catch2::Catch2WithMain)
add_test(NAME ecs_unittest COMMAND ecs_unittest)
