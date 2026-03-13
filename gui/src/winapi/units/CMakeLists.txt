target_sources(gui PRIVATE
    window_close.cpp
    window_close.hpp
    window_size.cpp
    window_size.hpp
    window_visibility.cpp
    window_visibility.hpp
)

target_sources(winapi_unittest PRIVATE
    window_close_unittest.cpp
    window_size_unittest.cpp
    window_visibility_unittest.cpp
)
