target_sources(gui PRIVATE
    messages.cpp
    messages.hpp
    window_close.cpp
    window_close.hpp
    window_life.cpp
    window_life.hpp
    window_size.cpp
    window_size.hpp
    window_visibility.cpp
    window_visibility.hpp
)

target_sources(winapi_unittest PRIVATE
    messages_unittest.cpp
    window_close_unittest.cpp
    window_life_unittest.cpp
    window_size_unittest.cpp
    window_visibility_unittest.cpp
)
