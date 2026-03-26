target_sources(gui PRIVATE
    real_winapi.cpp
    real_winapi.hpp
    setup.cpp
    setup.hpp
)
target_link_libraries(gui PRIVATE dwmapi)

add_executable(winapi_unittest)
target_include_directories(winapi_unittest PRIVATE ${ISTOK_GUI_DIR}/include)
target_include_directories(winapi_unittest PRIVATE ${ISTOK_GUI_DIR}/src)
target_link_libraries(winapi_unittest PRIVATE
    gui Catch2::Catch2WithMain)
add_test(NAME winapi_unittest COMMAND winapi_unittest)

add_subdirectory(base)
add_subdirectory(core)
add_subdirectory(units)
