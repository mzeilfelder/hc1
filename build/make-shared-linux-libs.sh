# copy the shared libs which we distribute with binary releases into SHARED_TARGET

export TOOL_FOLDER=`pwd`
if test `basename "$TOOL_FOLDER"` != build
then
    echo must be started from build folder
    exit -1
fi

LIBS_TARGET=../libs
SHARED_TARGET=$LIBS_TARGET/linux_shared

rm -rf $SHARED_TARGET
mkdir $SHARED_TARGET

# Better multi-platform support still needs to be done.
# For now just look in different folders and copy the libs which are found. Complain if they are not found at all.
if test -d $LIBS_TARGET"/freealut/linux/lib"
then
    cp $LIBS_TARGET/freealut/linux/lib/libalut.so.0 $SHARED_TARGET
elif  test -d "$LIBS_TARGET/freealut/linux/lib64"
then
	cp $LIBS_TARGET/freealut/linux/lib64/libalut.so.0 $SHARED_TARGET
else
	echo "Missing libalut.so.0"
fi

if test -d "$LIBS_TARGET/openal/linux/lib"
then
	cp $LIBS_TARGET/openal/linux/lib/libopenal.so.1 $SHARED_TARGET
elif test -d "$LIBS_TARGET/openal/linux/lib64"
then
	cp $LIBS_TARGET/openal/linux/lib64/libopenal.so.1 $SHARED_TARGET
else
	echo "Missing libopenal.so.1"
fi

if test -d "$LIBS_TARGET/sdl/linux/lib"
then
	cp $LIBS_TARGET/sdl/linux/lib/libSDL-1.2.so.0 $SHARED_TARGET
elif test -d "$LIBS_TARGET/sdl/linux/lib64"
then
	cp $LIBS_TARGET/sdl/linux/lib64/libSDL-1.2.so.0 $SHARED_TARGET
else
	echo "Missing libopenal.so.1"
fi


# That was for 32-bit, 64-bit libs are named a little different
# But really no idea yet how to handle that in a good way on Linux distros which will work in the long run.
#cp /usr/lib/libstdc++.so.6 $SHARED_TARGET
#cp /lib/libgcc_s.so.1 $SHARED_TARGET
#chmod -x $SHARED_TARGET/*
