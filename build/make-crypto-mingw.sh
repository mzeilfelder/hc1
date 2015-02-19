#!/bin/sh
#Make crypto++ lib on MinGW

export TOOL_FOLDER=`pwd`
if test `basename "$TOOL_FOLDER"` != build
then
    echo must be started from build folder
    exit -1
fi

export LIB_TARGET=${TOOL_FOLDER}/../libs/libcrypto++/Win32-gcc
export LIB_SOURCE=${TOOL_FOLDER}/../libs/libcrypto++/crypto++_5.60

cd $LIB_SOURCE
make clean
make
mkdir -p $LIB_TARGET
mkdir -p $LIB_TARGET/include
mkdir -p $LIB_TARGET/lib
cp $LIB_SOURCE/*.h $LIB_TARGET/include
cp $LIB_SOURCE/*.a $LIB_TARGET/lib
