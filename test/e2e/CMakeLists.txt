file(GLOB GUI_SOURCES CONFIGURE_DEPENDS "gui/*.cpp")
add_executable(test_e2e_gui ${GUI_SOURCES})
target_link_libraries(test_e2e_gui PRIVATE Catch2::Catch2WithMain)
add_test(NAME test_e2e_gui COMMAND test_e2e_gui)
