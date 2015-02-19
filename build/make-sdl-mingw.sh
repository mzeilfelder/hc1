#!/bin/sh

# Build the sdl library on MinGW from scratch

export CFLAGS="-O2 -s -Wall"
export CXXFLAGS="-O2 -s -Wall"

export TOOL_FOLDER=`pwd`
if test `basename "$TOOL_FOLDER"` != build
then
    echo must be started from build folder
    exit -1
fi

export LIB_TARGET=${TOOL_FOLDER}/../libs/sdl/Win32-gcc
export LIB_SOURCE=${TOOL_FOLDER}/../libs/sdl/SDL-1.2.11

mkdir -p ${LIB_TARGET}/build
cd ${LIB_TARGET}/build
$LIB_SOURCE/configure --prefix=$LIB_TARGET --enable-joystick --disable-static --disable-timers --disable-audio --disable-video --disable-cdrom --disable-threads --disable-file --disable-loadso --disable-directx 
make
make install
