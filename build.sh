#!/bin/bash
#
# NOTE as of right now the build relies on clang features (pch's & generation of
# compile database json)

start_timer=$(date +%s.%N)

mkdir -p bin
rm -f compile_commands.json

# NOTE -std=c++14 for "list-initializing"
CmnFlags="-g -std=c++14 -DENABLE_ASSERTS -fPIC -fno-rtti
          -Wall -Wfatal-errors -Wno-missing-braces -Wno-char-subscripts
          -Wno-unused-function -Wno-unused-variable -fno-exceptions -std=c++14 "
CmnFlags+="-DIMGUI "
# other useful flags: -Werror -Wno-comment -Wno-multichar -Wno-write-strings
# -Wno-sign-compare -Wno-unused-result -Wno-strict-aliasing
# -Wno-int-to-pointer-cast -Wno-switch Wno-logical-not-parentheses
# -Wno-return-type -Wno-array-bounds -maes msse4.1
CmnIncludes="-I./src/ -I./dep/imgui-1.82"
CmnLibs="-L$(pwd)/dep -Wl,-rpath=$(pwd)/dep/ -limgui_sdl"

# precompiled header for game layer
clang++ -MJ json.a -c ${CmnFlags} ${CmnIncludes} ./src/game.hpp -o game.pch &&

# build game as dll
clang++ -MJ json.b ${CmnFlags} --shared -include ./src/base.h ${CmnIncludes} ${CmnLibs} \
        -include-pch game.pch ./src/game.cpp -o ./dep/libgame.so &&

# add platform specific flags
CmnFlags+=$(sdl2-config --cflags)
SDL2Libs="$(sdl2-config --libs) -lSDL2_image -lSDL2_ttf "

# add opengl flags (comment out to use SDL renderer)
CmnFlags+=" -DUSE_OPENGL -I/usr/include/GL "
GLLibs=" -lGL -lGLU "

# pch for platform layer (sdl headers) (see https://clang.llvm.org/docs/PCHInternals.html)
clang++ -MJ json.c -c -pthread ${CmnFlags} ${CmnIncludes} ./src/platform_sdl.hpp -o platform_sdl.pch &&

# build platform layer as executable
clang++ -MJ json.d ${CmnFlags} ${CmnIncludes} ${SDL2Libs} ${GLLibs} -ldl ${CmnLibs} \
           -include-pch platform_sdl.pch ./src/platform_sdl.cpp -o ./bin/megastruct &&

# put together compile_commands.json
# see https://github.com/Sarcasm/notes/blob/master/dev/compilation-database.rst#clang
sed -e '1s/^/[\'$'\n''/' -e '$s/,$/\'$'\n'']/' json.* > compile_commands.json &&
rm json.* &&
end_timer=$(date +%s.%N)
compile_time=$(echo "$end_timer - $start_timer" | bc -l)
echo "Compile time (real): ${compile_time}s" \
#&& ./bin/megastruct
