export TOOL_FOLDER=`pwd`
if test `basename "$TOOL_FOLDER"` != build
then
    echo must be started from build folder
    exit -1
fi

export LIB_TARGET=${TOOL_FOLDER}/../libs/sdl/linux
export LIB_SOURCE=${TOOL_FOLDER}/../libs/sdl/SDL-1.2.11

mkdir -p ${LIB_TARGET}/build
cd ${LIB_TARGET}/build
$LIB_SOURCE/configure CPPFLAGS=-O2 --prefix=$LIB_TARGET --enable-static=no --enable-audio=no --enable-video=no --enable-cdrom=no --enable-threads=no --enable-file=no --enable-loadso=no --disable-input-tslib
make
make install
