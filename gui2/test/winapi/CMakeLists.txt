add_executable(windowproc_unittest ./windowproc_unittest.cpp)
target_include_directories(windowproc_unittest PRIVATE ${ISTOK_GUI_DIR}/src)
target_link_libraries(windowproc_unittest PRIVATE
    gui Catch2::Catch2WithMain)
add_test(NAME windowproc_unittest COMMAND gui_unittest)
