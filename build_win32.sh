# NOTE bear doesn't work w/ mingw apparently
# bear \
x86_64-w64-mingw32-g++ \
	-static -static-libgcc -static-libstdc++ \
        ../src/*.cpp \
	../dep/tmxlite/src/*.cpp \
	../dep/tmxlite/src/*.c \
	../dep/tmxlite/src/detail/*.cpp \
	-mwindows \
	-g \
	-Wall \
        -o ../bin/mega.exe \
	-lpthread \
	-I../ \
	-I../src/ \
        -I../dep/ \
	-I../dep/tmxlite/src/ \

	#-lgdi32

    	#../dep/libtmxlite.a \
	#-ldl \
	#../dep/imgui-1.82/imgui*.cpp \
	#./dep/libSDL2main.a \
	#./dep/libbox2d.a \
	#-DIMGUI \
	#-l../dep/win32/SDL2.dll \
	#-lSDL2_image \
	#-I../dep/glm/ \
	#-I../dep/imgui-1.82/ \
	#-I/usr/include/SDL2/
