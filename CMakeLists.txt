cmake_minimum_required(VERSION 3.16)
project(BoidsFlocking LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(OpenGL REQUIRED)
find_package(FreeGLUT CONFIG REQUIRED)

add_executable(boids
    src/main.cpp
    src/Flock.cpp
    src/Flock.h
)

target_link_libraries(boids PRIVATE OpenGL::GL FreeGLUT::freeglut)

if(MSVC)
    target_compile_options(boids PRIVATE /W4)
else()
    target_compile_options(boids PRIVATE -Wall -Wextra -Wpedantic)
endif()
