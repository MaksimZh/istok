target_sources(gui PRIVATE
    dispatcher.cpp
    dispatcher.hpp
    environment.cpp
    environment.hpp
    message.cpp
    message.hpp
    winapi_delegate.hpp
    window.cpp
    window.hpp
)

target_sources(winapi_unittest PRIVATE
    dispatcher_unittest.cpp
    environment_unittest.cpp
    fake_windows.cpp
    fake_windows.hpp
    null_winapi.hpp
    winapi_proxy.hpp
    window_unittest.cpp
)
