add_executable(test_acceptance_gui gui/main.cpp)
target_link_libraries(test_acceptance_gui PRIVATE Catch2::Catch2WithMain)

add_test(NAME test_acceptance_gui COMMAND test_acceptance_gui)
