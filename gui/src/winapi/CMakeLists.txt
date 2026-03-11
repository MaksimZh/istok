add_subdirectory(base)
add_subdirectory(systems)

target_sources(gui PRIVATE
    real_winapi.cpp
    real_winapi.hpp
    setup.cpp
    setup.hpp
)
