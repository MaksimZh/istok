enable_testing()
find_package(Catch2 REQUIRED)
find_package(Trompeloeil REQUIRED)

if (WIN32)
    add_subdirectory(winapi)
endif (WIN32)
