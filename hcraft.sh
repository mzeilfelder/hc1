#!/bin/sh
#
# h-craft championship startup script
#

# Function to find the real directory a program resides in.
# Feb. 17, 2000 - Sam Lantinga, Loki Entertainment Software
FindPath()
{
    fullpath="`echo $1 | grep /`"
    if [ "$fullpath" = "" ]; then
        oIFS="$IFS"
        IFS=:
        for path in $PATH
        do if [ -x "$path/$1" ]; then
               if [ "$path" = "" ]; then
                   path="."
               fi
               fullpath="$path/$1"
               break
           fi
        done
        IFS="$oIFS"
    fi
    if [ "$fullpath" = "" ]; then
        fullpath="$1"
    fi

    # Is the sed/ls magic portable?
    if [ -L "$fullpath" ]; then
        fullpath=`ls -l "$fullpath" |sed -e 's/.* -> //' |sed -e 's/\*//'`
    fi
    dirname $fullpath
}

# Set the home if not already set.
if [ "${HCRAFT_APP_PATH}" = "" ]; then
    HCRAFT_APP_PATH="`FindPath $0`"
fi
	
if [ -x "${HCRAFT_APP_PATH}/hcraft_bin" ]
then
	cd "${HCRAFT_APP_PATH}/"
	exec "./hcraft_bin" $*
fi
echo "Couldn't run h-craft championship (hcraft_bin). Try to cd manually in the that path and start it from there."
exit 1


