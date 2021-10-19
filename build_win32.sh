# dll's needed:
# - SDL2.dll
# - SDL2_ttf.dll
# - SDL2_image.dll
# - libimgui      (optional)
# - libimgui_ogl3 (optional)

x86_64-w64-mingw32-g++ \
	-static -static-libgcc -static-libstdc++ \
        ../src/*.cpp \
	-mwindows \
	-g \
	-Wall \
        -o ../bin/mega.exe \
	-lpthread \
	-I../ \
	-I../src/ \
        -I../dep/ \

	#-lgdi32
	#-ldl \
	#./dep/libSDL2main.a \
	#-DIMGUI \
	#-l../dep/win32/SDL2.dll \
	#-lSDL2_image \
	#-I../dep/glm/ \
	#-I../dep/imgui-1.82/ \
	#-I/usr/include/SDL2/
