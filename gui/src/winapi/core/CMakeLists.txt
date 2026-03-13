target_sources(gui PRIVATE
    messages.cpp
    messages.hpp
    setup.cpp
    setup.hpp
    window_life.cpp
    window_life.hpp
)

target_sources(winapi_unittest PRIVATE
    messages_unittest.cpp
    window_life_unittest.cpp
)
