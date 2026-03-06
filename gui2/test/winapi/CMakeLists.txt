add_executable(winapi_unittest
    dispatcher_unittest.cpp
    message_loop_unittest.cpp
    utils.hpp
    window_unittest.cpp
    windowproc_unittest.cpp
)
target_include_directories(winapi_unittest PRIVATE ${ISTOK_GUI_DIR}/src)
target_link_libraries(winapi_unittest PRIVATE gui Catch2::Catch2WithMain)
add_test(NAME winapi_unittest COMMAND winapi_unittest)
