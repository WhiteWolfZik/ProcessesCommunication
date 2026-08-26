cmake_minimum_required(VERSION 3.16)
project(ProcessesCommunication LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

add_compile_options(-Wall -Wextra -Wpedantic)

option(BUILD_TESTING "Build unit tests" ON)

add_subdirectory(common)
add_subdirectory(producer)
add_subdirectory(consumer)

if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()
