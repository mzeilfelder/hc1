# Get google-play-services working. Used for admob and in-game billing.

if [ -z "$ANDROID_HOME" ]
then 
	echo "ANDROID_HOME must be set to your local Android-sd path (like for example /myhome/android/adt-bundle-linux-x86_64-20131030/sdk)"
	exit 1
fi
if [ -z "$HCRAFT_HOME" ]
then 
	export HCRAFT_HOME=..
fi

if [ -z "$BUILD_TARGET" ]
then 
	export BUILD_TARGET="android-22"
fi


# Copy the google-play-services lib 
cd $HCRAFT_HOME/build/android/libs
cp -R $ANDROID_HOME/extras/google/google_play_services/libproject/google-play-services_lib .
# something to update project files - not quite sure what those steps do exactly
cd $HCRAFT_HOME/build/android
android update project --target $BUILD_TARGET --name hcraft1 --path . --subprojects
cd $HCRAFT_HOME/build/android/libs/google-play-services_lib 
android update project --path . --target $BUILD_TARGET --subprojects
# Copy android-support-v4.jar (probably not needed when only Android >= 4.x is targeted). 
cp $ANDROID_HOME/extras/android/support/v4/android-support-v4.jar $HCRAFT_HOME/build/android/libs
