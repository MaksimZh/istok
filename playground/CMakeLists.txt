find_package(GLEW REQUIRED)
find_package(OpenGL REQUIRED)

add_executable(demo demo.cpp)
target_link_libraries(demo PRIVATE gui dwmapi opengl32 GLEW::GLEW)

