cmake_minimum_required(VERSION 3.10)

project(MeowNES)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-parentheses -O3 -march=native -flto -fno-plt -ffast-math -fstrict-aliasing -funroll-loops")
find_package(Qt6 REQUIRED Widgets)

if (WIN32)
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static")
endif()

include_directories(include)

file(GLOB_RECURSE SOURCES
    "src/*.cpp"
    "src/mappers/*.cpp"
    "src/filters/*.cpp"
)

add_executable(MeowNES ${SOURCES})

target_link_libraries(MeowNES Qt6::Widgets)
#target_compile_options(MeowNES PRIVATE -fsanitize=address)
#target_link_options(MeowNES PRIVATE -fsanitize=address)

set(CMAKE_CLEAN_FILES "${CMAKE_CURRENT_BINARY_DIR}")
