#!/bin/bash
#
# build on linux w/ sdl2 as a backend

# NOTE comment out `bear -- \` line if you don't need a compile_commands.json
CmnFlags="-g -DIMGUI -Wall -Wfatal-errors -Wno-missing-braces -Wno-char-subscripts -fPIC "
# other flags that could be useful:
# -Werror -Wno-comment -Wno-multichar -Wno-write-strings -Wno-unused-variable
# -Wno-unused-function -Wno-sign-compare -Wno-unused-result
# -Wno-strict-aliasing -Wno-int-to-pointer-cast -Wno-switch
# -Wno-logical-not-parentheses -Wno-return-type -Wno-array-bounds -maes
# -msse4.1 -std=c++11 -fno-rtti -fno-exceptions
CmnFlags+=$(sdl2-config --cflags)
#CmnIncludes="-include ./src/base.h -I./src/ -I./dep/ -I./dep/imgui-1.82"
CmnIncludes="-I./src/ -I./dep/ -I./dep/imgui-1.82"
CmnLibs="-L$(pwd)/dep -Wl,-rpath=$(pwd)/dep/ -limgui_sdl"
SDL2Libs="$(sdl2-config --libs) -lSDL2_image -lSDL2_ttf "

# build game as dll
bear -- \
clang++ ${CmnFlags} --shared -include ./src/base.h ${CmnIncludes} ${CmnLibs} \
        ./src/game.cpp -o ./dep/libgame.so

# NOTE trying out precompiled header for platform layer (sdl stuff)
# see clang docs: https://clang.llvm.org/docs/PCHInternals.html
# TODO use our compile defs (like -DIMGUI)
bear --append -- \
clang++ -c -DIMGUI -I./dep/ -I./dep/imgui-1.82 -I/usr/include/SDL2 -D_REENTRANT -pthread -fPIC ./src/platform_sdl.hpp -o platform_sdl.pch

# TODO compilation w/ gcc seems broken here (gets stuck)
# build platform layer as executable
bear --append -- \
clang++ ${CmnFlags} ${CmnIncludes} ${SDL2Libs} -ldl ${CmnLibs} -ltmxlite \
         -include-pch platform_sdl.pch \
        ./src/platform_sdl.cpp -o ./bin/megastruct
