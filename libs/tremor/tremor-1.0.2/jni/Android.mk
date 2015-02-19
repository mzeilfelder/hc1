# LOCAL_PATH _must_ be set. Usually set to the path of the Android.mk itself, but we set it to our c++ sources.
LOCAL_PATH := $(call my-dir)/..
$(info $(LOCAL_PATH))

include $(CLEAR_VARS)
LOCAL_MODULE := ogg
LOCAL_SRC_FILES := $(OGG_LIB)/$(TARGET_ARCH_ABI)/libogg.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := tremor
MODULE       := tremor

LOCAL_C_INCLUDES := $(OGG_INCLUDES)
LOCAL_STATIC_LIBRARIES := ogg

LOCAL_CFLAGS    +=  -fno-exceptions \
					-fno-rtti \
                    -fpic \
                    -ffunction-sections \
                    -funwind-tables \
                    -fstack-protector \
                    -fno-short-enums \
                    -O3 \
                    -g
					
LOCAL_SRC_FILES := mdct.c block.c window.c \
                   synthesis.c info.c \
                   floor1.c floor0.c vorbisfile.c \
                   res012.c mapping0.c registry.c codebook.c \
                   sharedbook.c

include $(BUILD_STATIC_LIBRARY)
