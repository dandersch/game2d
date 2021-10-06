#!/bin/bash
#
# TODO update

# NOTE: "There is experimental support for true dynamic libraries, loaded as
# runtime, either via dlopen or as a shared library. See that link for the
# details and limitations."
# https://github.com/emscripten-core/emscripten/wiki/Linking

# builds js app & start local webserver on localhost:8000
/usr/lib/emscripten/emcc -v \
    ../src/*.cpp \
    ../dep/libtmxlite_ems.a \
    -I../dep/imgui-1.82/ \
    -I../ -I../dep/ -I../src/ \
    -I/usr/include/SDL2/ \
    -s USE_SDL=2 \
    -s USE_SDL_IMAGE=2 \
    -s USE_SDL_MIXER=2 \
    -s SDL2_IMAGE_FORMATS='[bmp,png,xpm]' \
    -s USE_SDL_TTF=2 \
    -s LLD_REPORT_UNDEFINED \
    -s TOTAL_MEMORY=1310720000 \
    -ldl \
    -lpthread \
    -o sdl_test.html \
    --preload-file ../res \
    --shell-file ./shell_minimal.html \
    -O3 \
    && python -m http.server

# BUILD GAME DLL
/usr/lib/emscripten/emcc -v -O3 ../src/game.cpp -o sdl_test.html \
    -I../dep/imgui-1.82/ -I../ -I../dep/ -I../src/ -I/usr/include/SDL2/ \
    -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s USE_SDL_MIXER=2 \
    -s SDL2_IMAGE_FORMATS='[bmp,png,xpm]' -s USE_SDL_TTF=2 \
    -s LLD_REPORT_UNDEFINED -s TOTAL_MEMORY=1310720000 \
    -ldl -lpthread \
    --preload-file ../res --shell-file ../shell_minimal.html \
    && python -m http.server
