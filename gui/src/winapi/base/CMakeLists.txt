target_sources(gui PRIVATE
    delegate.hpp
    dispatcher.cpp
    dispatcher.hpp
    message.cpp
    message.hpp
    tools.cpp
    tools.hpp
    window.cpp
    window.hpp
)

target_sources(winapi_unittest PRIVATE
    dispatcher_unittest.cpp
    tools_unittest.cpp
    window_unittest.cpp
)
