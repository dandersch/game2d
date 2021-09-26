#!/bin/bash

# TODO use `sdl2-config --cflags`
# TODO use `sdl2-config --libs`

# uncomment `bear -- \` line if you don't need a compile_commands.json

# TODO use -fno-gnu-unique for hotloading (?) (only gcc)

# build game as dll
bear -- \
clang++ -g -Wall -DIMGUI -fPIC --shared \
                     ../src/game.cpp \
                     -include ../src/base.h \
                     -o ../dep/libgame.so \
                     -ldl -lpthread \
                     -L`pwd`/../dep \
                     -Wl,-rpath=`pwd`/../dep/ \
                     -limgui_sdl \
                     -I../ -I../src/ -I../dep/ -I../dep/imgui-1.82/

# TODO compilation w/ gcc seems broken here (gets stuck)
# build platform layer as executable
bear --append -- \
clang++ -g -Wall -fPIC -DIMGUI ../src/platform_sdl.cpp -o ../bin/megastruct_platform \
    -include ../src/base.h \
    -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -ldl -lpthread \
    -L`pwd`/../dep -Wl,-rpath=`pwd`/../dep/ -limgui_sdl -ldl -ltmxlite \
    -I../ -I../src/ -I../dep/ -I../dep/imgui-1.82/ -I/usr/include/SDL2
