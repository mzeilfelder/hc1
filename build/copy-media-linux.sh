# Copy the media from their original location into the working folder
# While usually the data just should live in there already, we can't do that because the media
# is not under source control and so would/will get lost on clones. 
# There is a non-technical reason why media are handled different (about rights).

export TOOL_FOLDER=`pwd`
if test `basename "$TOOL_FOLDER"` != build
then
    echo must be started from build folder
    exit -1
fi

if test -z "MEDIA_PATH"
then
    export MEDIA_PATH=${TOOL_FOLDER}/../../media_hc1
fi

rm -rf ${TOOL_FOLDER}/../media
cp -r ${MEDIA_PATH}/media $TOOL_FOLDER/..
