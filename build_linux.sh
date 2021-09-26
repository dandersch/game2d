#!/bin/bash

# TODO statically link SDL2_image
# TODO use `sdl2-config --cflags`
# TODO use `sdl2-config --libs`
# uncomment first line if you don't need a compile_commands.json
#bear -- \
#c++ -g -Wall -DIMGUI \
#	../src/*.cpp \
#	-o ../bin/megastruct_no_cmake \
#	-lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -ldl -lpthread \
#	-L`pwd`/../dep \
#	-Wl,-rpath=`pwd`/../dep/ \
#	-limgui_sdl \
#	-ltmxlite \
#	-I../ -I../src/ -I../dep/ -I../dep/imgui-1.82/ -I/usr/include/SDL2

# TODO use -fno-gnu-unique for hotloading (?)

# build game as dll
bear -- \
clang++ -g -Wall -DIMGUI -fPIC --shared \
                     ../src/game.cpp \
                     ../src/collision.cpp \
                     ../src/command.cpp \
                     ../src/entitymgr.cpp \
                     ../src/gamelayer.cpp \
                     ../src/globals.cpp \
                     ../src/camera.cpp \
                     ../src/input.cpp \
                     ../src/menulayer.cpp \
                     ../src/player.cpp \
                     ../src/reset.cpp \
                     ../src/resourcemgr.cpp \
                     ../src/rewind.cpp \
                     -include ../src/base.h \
                     -o ../dep/libgame.so \
                     -ldl -lpthread \
                     -L`pwd`/../dep \
                     -Wl,-rpath=`pwd`/../dep/ \
                     -limgui_sdl \
                     -I../ -I../src/ -I../dep/ -I../dep/imgui-1.82/
                     #-ltmxlite \


# build platform layer as executable
bear --append -- \
clang++ -g -Wall -fPIC -DIMGUI ../src/platform_sdl.cpp -o ../bin/megastruct_platform \
    -include ../src/base.h \
    -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -ldl -lpthread \
    -L`pwd`/../dep -Wl,-rpath=`pwd`/../dep/ -limgui_sdl -ldl -ltmxlite \
    -I../ -I../src/ -I../dep/ -I../dep/imgui-1.82/ -I/usr/include/SDL2
