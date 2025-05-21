file(GLOB GUI_SOURCES CONFIGURE_DEPENDS "gui/*.cpp")
add_executable(test_unit_gui ${GUI_SOURCES})
target_link_libraries(test_unit_gui PRIVATE gui Catch2::Catch2WithMain)
add_test(NAME test_unit_gui COMMAND test_unit_gui)
