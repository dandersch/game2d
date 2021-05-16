#!/bin/bash

# TODO statically link SDL2_image
# TODO use `sdl2-config --cflags`
# TODO use `sdl2-config --libs`
# uncomment first line if you don't need a compile_commands.json
bear -- \
c++ main.cpp  \
	-Wall \
	./src/base.h \
	./dep/libSDL2.a \
	./dep/libSDL2main.a \
	./dep/libbox2d.a \
	./dep/imgui-1.82/imgui*.cpp \
	-o ./bin/hello_sdl \
	-ldl \
	-lpthread \
	-lSDL2_image \
	-I./src/ \
	-I./dep/glm/ \
	-I./dep/imgui-1.82/ \
	-I/usr/include/SDL2 # for better autocompletion
