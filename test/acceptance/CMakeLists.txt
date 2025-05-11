file(GLOB GUI_SOURCES CONFIGURE_DEPENDS "gui/*.cpp")
add_executable(test_acceptance_gui ${GUI_SOURCES})
target_link_libraries(test_acceptance_gui PRIVATE Catch2::Catch2WithMain)
add_test(NAME test_acceptance_gui COMMAND test_acceptance_gui)
