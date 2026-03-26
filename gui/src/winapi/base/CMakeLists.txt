target_sources(gui2 PRIVATE
    dispatcher2.cpp
    dispatcher2.hpp
    environment2.cpp
    environment2.hpp
    message.cpp
    message.hpp
    window2.cpp
    window2.hpp
)

target_sources(winapi_unittest2 PRIVATE
    dispatcher2_unittest.cpp
    environment2_unittest.cpp
    window2_unittest.cpp
)

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
