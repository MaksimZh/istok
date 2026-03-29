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
    test_utils.cpp
    test_utils.hpp
    window_unittest.cpp
)
