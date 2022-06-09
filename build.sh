#!/bin/bash

CC=clang++

set -e # stop execution on first fail

mkdir -p bin
mkdir -p dep
rm -f compile_commands.json

# creates compile_commands.json file
command -v bear >/dev/null && DB="bear --append --"

# other useful flags: -Werror -Wno-comment -Wno-multichar -Wno-write-strings -Wno-sign-compare -Wno-unused-result -Wno-strict-aliasing -Wno-int-to-pointer-cast -Wno-switch Wno-logical-not-parentheses -Wno-return-type -Wno-array-bounds -maes msse4.1
CmnCompilerFlags="-I./src/ -g -DENABLE_ASSERTS -fPIC -fno-rtti -Wall -Wfatal-errors -Wno-missing-braces -Wno-char-subscripts -Wno-unused-function -Wno-unused-variable -fno-exceptions -std=c++14 "
CmnLinkerFlags="-L$(pwd)/dep -Wl,-rpath=$(pwd)/dep/"

# build game as dll
${DB} ${CC} --shared -include ./src/base.h ${CmnCompilerFlags} ${CmnLinkerFlags} ./src/game.cpp -o ./dep/libgame.so

# add platform specific flags
CmnCompilerFlags="$(sdl2-config --cflags) ${CmnCompilerFlags}"
CmnLinkerFlags="$(sdl2-config --libs) -ldl ${CmnLinkerFlags}"

# precompiled header for platform layer (saves 1.5 seconds w/ clang)
${DB} ${CC} -c -pthread ${CmnCompilerFlags} -DUSE_OPENGL -x c++-header ./src/platform_sdl.hpp -o platform_sdl.pch

if [ $CC == "clang++" ]; then
  CmnCompilerFlags+=" -include-pch platform_sdl.pch "
fi

# build platform layer as executable (SDL + OpenGL)
${DB} ${CC} ${CmnCompilerFlags} -DUSE_OPENGL -I/usr/include/GL ${CmnLinkerFlags} -lGL -lGLU ./src/platform_sdl.cpp -o ./bin/megastruct

# build SDL + SDL_renderer
# NOTE: pch needs to be compiled without -DUSE_OPENGL
#${DB} ${CC} ${CmnCompilerFlags} ${CmnLinkerFlags} ./src/platform_sdl.cpp -o ./bin/megastruct
