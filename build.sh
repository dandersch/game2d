#!/bin/bash
#
# build for linux using clang++

# TODO use `sdl2-config --cflags`
# TODO use `sdl2-config --libs`
# TODO use -fno-gnu-unique for hotloading (?) (only gcc)
# NOTE uncomment `bear -- \` line if you don't need a compile_commands.json

# TODO CommonIncludes
CommonFlags="-g -DIMGUI -Wall -Wfatal-errors -Wno-missing-braces -Wno-char-subscripts -fPIC"
# other flags that could be useful:
# -Werror -Wno-comment -Wno-multichar -Wno-write-strings -Wno-unused-variable
# -Wno-unused-function -Wno-sign-compare -Wno-unused-result
# -Wno-strict-aliasing -Wno-int-to-pointer-cast -Wno-switch
# -Wno-logical-not-parentheses -Wno-return-type -Wno-array-bounds -maes
# -msse4.1 -std=c++11 -fno-rtti -fno-exceptions

# build game as dll
bear -- \
clang++ ${CommonFlags} --shared \
                     ./src/game.cpp \
                     -include ./src/base.h \
                     -o ./dep/libgame.so \
                     -ldl -lpthread \
                     -L`pwd`/dep \
                     -Wl,-rpath=`pwd`/dep/ \
                     -limgui_sdl \
                     -I./src/ -I./dep/ -I./dep/imgui-1.82/

# TODO compilation w/ gcc seems broken here (gets stuck)
# build platform layer as executable
bear --append -- \
clang++ $CommonFlags \
        ./src/platform_sdl.cpp -o ./bin/megastruct_platform \
        -include ./src/base.h \
        -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -ldl -lpthread \
        -L`pwd`/dep -Wl,-rpath=`pwd`/dep/ -limgui_sdl -ltmxlite \
        -I./src/ -I./dep/ -I./dep/imgui-1.82/ -I/usr/include/SDL2
