add_executable(winapi_unittest
    dispatcher_unittest.cpp
    events_unittest.cpp
    message_loop_unittest.cpp
    utils.hpp
    window_close_unittest.cpp
    window_size_unittest.cpp
    window_unittest.cpp
    windowproc_unittest.cpp
)
target_include_directories(winapi_unittest PRIVATE ${ISTOK_GUI_DIR}/src)
target_link_libraries(winapi_unittest PRIVATE gui Catch2::Catch2WithMain)
add_test(NAME winapi_unittest COMMAND winapi_unittest)

if (0)
    add_executable(experimental_unittest
        window_size_unittest.cpp
    )
    target_include_directories(experimental_unittest PRIVATE ${ISTOK_GUI_DIR}/src)
    target_link_libraries(experimental_unittest PRIVATE gui Catch2::Catch2WithMain)
    add_test(NAME experimental_unittest COMMAND experimental_unittest)
endif (0)
