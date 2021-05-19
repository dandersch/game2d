cmake_minimum_required(VERSION 3.5)

project(megastruct LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
set(CMAKE_BUILD_TYPE DEBUG)
add_compile_definitions(IMGUI)
add_compile_definitions(ENABLE_ASSERTS)

########################
# dependencies
########################
# TODO make platform independent
# find_package(SDL2 REQUIRED) # doesnt work...
set(SDL2_INCLUDE_DIRS "/usr/include/SDL2")
include_directories(${SDL2_INCLUDE_DIRS})

#find_package(Box2D REQUIRED)

# glm
include_directories("dep/glm/")

# imgui
include_directories("dep/imgui-1.82/")
file(GLOB imgui_src "dep/imgui-1.82/imgui*.cpp")

# Internal
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/)
include_directories("src/")
file(GLOB src_files "src/*.cpp")

add_executable(megastruct main.cpp ${src_files} ${imgui_src})
target_precompile_headers(megastruct PRIVATE "pch.h")
target_link_libraries(megastruct
                      ${SDL2_LIBRARIES}
                      SDL2main
                      SDL2
                      SDL2_image
                      SDL2_ttf
                      )
