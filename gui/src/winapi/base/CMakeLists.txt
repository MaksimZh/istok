target_sources(gui PRIVATE
    delegate.hpp
    dispatcher.cpp
    dispatcher.hpp
    environment.cpp
    environment.hpp
    message.cpp
    message.hpp
    window.cpp
    window.hpp
)

target_sources(winapi_unittest PRIVATE
    dispatcher_unittest.cpp
    environment_unittest.cpp
    window_test_utils.hpp
    window_unittest.cpp
)
