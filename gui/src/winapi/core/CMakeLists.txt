target_sources(gui PRIVATE
    message_loop.cpp
    message_loop.hpp
    window_life.cpp
    window_life.hpp
)

target_sources(winapi_unittest PRIVATE
    message_loop_unittest.cpp
    window_life_unittest.cpp
)
