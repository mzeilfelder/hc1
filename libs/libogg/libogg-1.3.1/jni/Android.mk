# LOCAL_PATH _must_ be set. Usually set to the path of the Android.mk itself, but we set it to our c++ sources.
LOCAL_PATH := $(call my-dir)/../src
$(info $(LOCAL_PATH))

include $(CLEAR_VARS)
LOCAL_MODULE := ogg
MODULE       := ogg

LOCAL_C_INCLUDES := ../include

LOCAL_CFLAGS    +=  -fno-exceptions \
					-fno-rtti \
                    -O3 \
                    -g
					
LOCAL_SRC_FILES := bitwise.c framing.c

include $(BUILD_STATIC_LIBRARY)
