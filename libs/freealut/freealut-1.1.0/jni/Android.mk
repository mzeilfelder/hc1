# LOCAL_PATH _must_ be set. Usually set to the path of the Android.mk itself, but we set it to our c++ sources.
LOCAL_PATH := $(call my-dir)/../src
$(info $(LOCAL_PATH))

include $(CLEAR_VARS)
LOCAL_MODULE := openal
LOCAL_SRC_FILES := $(OPENAL_LIB)/$(TARGET_ARCH_ABI)/libopenal.so 
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := freealut
MODULE       := freealut
LOCAL_C_INCLUDES := $(LOCAL_PATH)/../include
LOCAL_C_INCLUDES += $(OPENAL_INCLUDES)

LOCAL_SHARED_LIBRARIES := openal

LOCAL_CFLAGS    +=  -fno-exceptions \
					-fno-rtti \
                    -fpic \
                    -ffunction-sections \
                    -funwind-tables \
                    -fstack-protector \
                    -fno-short-enums \
                    -DHAVE_GCC_VISIBILITY \
					-DHAVE_STDINT_H -DHAVE_NANOSLEEP -DHAVE_TIME_H \
					-DHAVE_STAT -DHAVE_UNISTD_H \
                    -O3 \
                    -g
					
LOCAL_SRC_FILES := alutBufferData.c alutCodec.c alutError.c alutInit.c \
                   alutLoader.c alutUtil.c alutVersion.c alutWaveform.c \
                   alutInputStream.c alutOutputStream.c
				   
include $(BUILD_SHARED_LIBRARY)
