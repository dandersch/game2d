cmake_minimum_required(VERSION 3.5)

project(megastruct LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
set(CMAKE_BUILD_TYPE DEBUG)
#add_compile_definitions(IMGUI)
add_compile_definitions(ENABLE_ASSERTS)

# Check if a flag exists and add it to the list of compiler (so, not linker) options
function (ADD_CHECK_CXX_COMPILER_FLAG _CXXFLAGS _CACHE_VAR _FLAG)
        set(SUPPORTED_CHECK_CXX_COMPILER_FLAGS ${SUPPORTED_CHECK_CXX_COMPILER_FLAGS} ${_FLAG} PARENT_SCOPE)
endfunction ()

# Turn on extra warnings
#add_compile_options(-Wsuggest-override)
##add_compile_options(-Wduplicated-cond)
#add_compile_options(-Wnon-virtual-dtor)
#add_compile_options(-Wmissing-variable-declarations)
##add_compile_options(-Wduplicated-branches)
##add_compile_options(-Wrestrict)
#add_compile_options(-Wmissing-field-initializers)
##add_compile_options(-Wunreachable-code-break)
##add_compile_options(-Wlogical-op)
#add_compile_options(-Wrange-loop-analysis)
#add_compile_options(-Wtautological-unsigned-zero-compare)
#add_compile_options(-Wold-style-cast)
## unused-const-variable=1 only checks main compilation unit
## unused-const-variable[=2] would check for unused values from headers as well
##add_compile_options(-Wunused-const-variable=1)
##add_compile_options(-Wno-clobbered)
## Disabled due to problems compiling OpenSSL on macOS.
#add_compile_options(-Wdocumentation)

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

# tmxlite
include_directories("dep/")

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
                      SDL2_mixer
                      SDL2_ttf
                      pthread
                      "${CMAKE_CURRENT_SOURCE_DIR}/dep/libtmxlite.a"
                      )
