target_sources(gui2 PRIVATE
    dispatcher.cpp
    dispatcher.hpp
    message.cpp
    message.hpp
    window.cpp
    window.hpp
)

target_sources(winapi_unittest2 PRIVATE
    dispatcher_unittest.cpp
    window_unittest.cpp
)