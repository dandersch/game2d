#!/bin/bash

# builds js app & start local webserver on localhost:8000
/usr/lib/emscripten/emcc -v ../main.cpp -I../dep/imgui-1.82/ -I../ -I../dep/ -I../src/ -I/usr/include/SDL2/ -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s SDL2_IMAGE_FORMATS='[bmp,png,xpm]' -s USE_SDL_TTF=2 -ldl -lpthread -o sdl_test.html --preload-file ../res && python -m http.server
