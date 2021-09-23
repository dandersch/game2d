#!/bin/bash

# TODO statically link SDL2_image
# TODO use `sdl2-config --cflags`
# TODO use `sdl2-config --libs`
# uncomment first line if you don't need a compile_commands.json
#bear -- \
c++ -g -Wall -DIMGUI \
	../src/*.cpp \
	-o ../bin/megastruct_no_cmake \
	-lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -ldl -lpthread \
	-L`pwd`/../dep \
	-Wl,-rpath=`pwd`/../dep/ \
	-limgui_sdl \
	-ltmxlite \
	-I../ -I../src/ -I../dep/ -I../dep/imgui-1.82/ -I/usr/include/SDL2
