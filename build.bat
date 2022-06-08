@echo off

REM Some compiler flags that could be useful:
REM     /Yc   create pch file
REM     /Yu   use pch file
REM     /EHc  extern "C" defaults to nothrow
REM     /EHs  enable C++ EH (no SEH exceptions)
REM     /EHa- disable C++ EH (w/ SEH exceptions)
REM     /WL   enable one line diagnostics
REM     /MT   link with LIBCMT.LIB
REM     /MTd  link with LIBCMT.LIB debug lib
REM     /GR-  no C++ RTTI
REM     /Gm-  disable minimal rebuild

set COMMON_COMPILER_FLAGS=/nologo /Fo.\bin\ /std:c++14 /I .\src\
set COMMON_COMPILER_FLAGS=-DENABLE_ASSERTS=1 %COMMON_COMPILER_FLAGS%
set COMMON_LINKER_FLAGS=/NOIMPLIB /NOEXP

REM -------- build game dll -----------------------
cl.exe %COMMON_COMPILER_FLAGS% src\game.cpp /LD /link %COMMON_LINKER_FLAGS% /OUT:dep\libgame.dll


REM -------- build platform exe -------------------

REM dependencies:
REM   SDL2.dll >= 2.00.16 (?)
REM   SDL2 header files (SDL2.h, ...)
REM   SDL2.lib
REM   SDL2main.lib (which needs shell32.lib)
REM   for sdl renderer:
REM     SDL2_image.lib
REM   for opengl renderer:
REM     opengl32.lib

set COMMON_COMPILER_FLAGS=%COMMON_COMPILER_FLAGS% /I .\dep\sdl_inc\
set COMMON_LINKER_FLAGS=%COMMON_LINKER_FLAGS% dep\SDL2main.lib dep\SDL2.lib shell32.lib /SUBSYSTEM:CONSOLE

REM SDL w/ OpenGL renderer
cl.exe %COMMON_COMPILER_FLAGS% -DUSE_OPENGL=1 src\platform_sdl.cpp /link opengl32.lib %COMMON_LINKER_FLAGS% /OUT:bin\megastruct.exe

REM SDL w/ SDL_renderer (not working right now)
REM cl.exe %COMMON_COMPILER_FLAGS% -I dep\sdl_image_inc\ src\platform_sdl.cpp /link %COMMON_LINKER_FLAGS% dep\SDL2_image.lib /OUT:bin\megastruct_sdl.exe
