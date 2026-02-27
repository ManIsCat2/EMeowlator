cmake_minimum_required(VERSION 3.10)

project(MeowNES)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O3")
find_package(Qt6 REQUIRED Widgets)

if (WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libstdc++ -static-libgcc")
endif()

include_directories(include)

file(GLOB_RECURSE SOURCES
    "src/*.cpp"
    "src/mappers/*.cpp"
)

add_executable(MeowNES ${SOURCES})

target_link_libraries(MeowNES Qt6::Widgets)

add_custom_command(TARGET MeowNES POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/gui"
    "${CMAKE_CURRENT_BINARY_DIR}/gui"
)

set(CMAKE_CLEAN_FILES "${CMAKE_CURRENT_BINARY_DIR}")
