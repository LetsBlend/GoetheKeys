cmake_minimum_required(VERSION 3.30)
project(GoetheKeys)

set(CMAKE_CXX_STANDARD 20)

add_executable(GoetheKeys WIN32 src/main.cpp)

target_compile_definitions(GoetheKeys PRIVATE
    $<$<CONFIG:Debug>:DEBUG>
)
