target_sources(gui PRIVATE
    delegate.hpp
    dispatcher.cpp
    dispatcher.hpp
    message.cpp
    message.hpp
    message_loop.cpp
    message_loop.hpp
    window.cpp
    window.hpp
)

target_sources(winapi_unittest PRIVATE
    dispatcher_unittest.cpp
    message_loop_unittest.cpp
    window_unittest.cpp
)
