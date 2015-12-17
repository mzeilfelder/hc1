# LOCAL_PATH _must_ be set. Usually set to the path of the Android.mk itself, but we set it to our c++ sources.
LOCAL_PATH := $(call my-dir)/../../../src
$(info $(LOCAL_PATH))

include $(CLEAR_VARS)
LOCAL_MODULE := Irrlicht
# path is relative to LOCAL_PATH
LOCAL_SRC_FILES := ../libs/irrlicht_ogl-es/lib/Android/libIrrlicht.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := freetype
# path is relative to LOCAL_PATH
LOCAL_SRC_FILES := ../libs/libfreetype/android/lib/libfreetype.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := openal
LOCAL_SRC_FILES := ../libs/openal/android/lib/$(TARGET_ARCH_ABI)/libopenal.so 
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := freealut
LOCAL_SRC_FILES := ../libs/freealut/android/lib/$(TARGET_ARCH_ABI)/libfreealut.so 
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := ogg
LOCAL_SRC_FILES := ../libs/libogg/android/lib/$(TARGET_ARCH_ABI)/libogg.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := tremor
LOCAL_SRC_FILES := ../libs/tremor/android/lib/$(TARGET_ARCH_ABI)/libtremor.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := hcraft1

LOCAL_CFLAGS := -pipe -fno-exceptions -fno-rtti -fstrict-aliasing

ifndef NDEBUG
LOCAL_CFLAGS += -g -ggdb -O0 -D_DEBUG
else
LOCAL_CFLAGS += -fexpensive-optimizations -O3
endif

ifdef HOVER_RELEASE
LOCAL_CFLAGS += -DHOVER_RELEASE
endif

ifeq ($(TARGET_ARCH_ABI),x86)
LOCAL_CFLAGS += -fno-stack-protector
endif

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../libs/irrlicht_ogl-es/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../libs/libfreetype/android/include/freetype2
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../libs/tremor/android/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../libs/libogg/android/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../libs/openal/android/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../libs/freealut/android/include
$(info $(LOCAL_C_INCLUDES))

LOCAL_SRC_FILES := main.cpp config.cpp irrlicht_manager.cpp logging.cpp string_table.cpp keycode_strings.cpp random.cpp recorder.cpp 
LOCAL_SRC_FILES += mobile/android_tools.cpp mobile/app_restrictions.cpp mobile/advert_admob.cpp mobile/billing_googleplay.cpp
LOCAL_SRC_FILES += timer.cpp streaming.cpp physics.cpp helper_xml.cpp app_tester.cpp helper_str.cpp helper_irr.cpp helper_math.cpp helper_file.cpp 
LOCAL_SRC_FILES += device_keyboard.cpp device_mouse.cpp device_joystick.cpp device_joystick_settings.cpp device_touch.cpp device_touch_settings.cpp 
LOCAL_SRC_FILES += controller.cpp event_receiver_base.cpp input_device.cpp input_device_manager.cpp mesh_texture_loader.cpp
LOCAL_SRC_FILES += ai_track.cpp championship.cpp game.cpp highscores.cpp hover.cpp level.cpp level_manager.cpp track_marker.cpp player.cpp profiles.cpp rivalsmode.cpp
LOCAL_SRC_FILES += CLMOMeshFileLoader.cpp camera.cpp node_manager.cpp ogles2_materials.cpp grid_triangle_selector.cpp
LOCAL_SRC_FILES += CGUITextSlider.cpp font_manager.cpp gui.cpp gui_dialog.cpp CGUITTFont.cpp
LOCAL_SRC_FILES += sound_settings.cpp music_manager.cpp sound_openal.cpp ogg_stream.cpp ov_callbacks.cpp
LOCAL_SRC_FILES += editor.cpp start_browser.cpp 
LOCAL_SRC_FILES += convert_utf.c
LOCAL_SRC_FILES += gui_dialogs/gui_menu_keyboard.cpp
LOCAL_SRC_FILES += gui_dialogs/gui_touch.cpp                       gui_dialogs/gui_menu_licenses.cpp
LOCAL_SRC_FILES += gui_dialogs/gui_ai.cpp                          gui_dialogs/gui_menu_graphics.cpp
LOCAL_SRC_FILES += gui_dialogs/gui_controller_display.cpp          gui_dialogs/gui_menu_highscores.cpp
LOCAL_SRC_FILES += gui_dialogs/gui_developer_settings.cpp          gui_dialogs/gui_menu_hover_unlocked.cpp
LOCAL_SRC_FILES += gui_dialogs/gui_dlg_okcancel.cpp                gui_dialogs/gui_menu_loadingscreen.cpp
LOCAL_SRC_FILES += gui_dialogs/gui_editor.cpp                      gui_dialogs/gui_menu_main.cpp
LOCAL_SRC_FILES += gui_dialogs/gui_game.cpp                        gui_dialogs/gui_menu_nagscreen.cpp
LOCAL_SRC_FILES += gui_dialogs/gui_helper.cpp                      gui_dialogs/gui_menu_newprofile.cpp
LOCAL_SRC_FILES += gui_dialogs/gui_hud.cpp                         gui_dialogs/gui_menu_options.cpp
LOCAL_SRC_FILES += gui_dialogs/gui_intro.cpp                       gui_dialogs/gui_menu_pause.cpp
LOCAL_SRC_FILES += gui_dialogs/gui_levelmanager.cpp                gui_dialogs/gui_menu_profiles.cpp
LOCAL_SRC_FILES += gui_dialogs/gui_menu_analog.cpp                 gui_dialogs/gui_menu_replaytheatre.cpp
LOCAL_SRC_FILES += gui_dialogs/gui_menu_arcade.cpp                 gui_dialogs/gui_menu_rivals.cpp
LOCAL_SRC_FILES += gui_dialogs/gui_menu_championship.cpp           gui_dialogs/gui_menu_rivals_score.cpp
LOCAL_SRC_FILES += gui_dialogs/gui_menu_championship_progress.cpp  gui_dialogs/gui_menu_selecthover.cpp
LOCAL_SRC_FILES += gui_dialogs/gui_menu_championship_winner.cpp    gui_dialogs/gui_menu_timeattack.cpp
LOCAL_SRC_FILES += gui_dialogs/gui_menu_controls.cpp               gui_dialogs/gui_menu_tutorial3.cpp
LOCAL_SRC_FILES += gui_dialogs/gui_menu_credits.cpp                gui_dialogs/gui_modellist.cpp
LOCAL_SRC_FILES += gui_dialogs/gui_menu_gameend_champ.cpp          gui_dialogs/gui_nodemanager.cpp
LOCAL_SRC_FILES += gui_dialogs/gui_menu_gameend.cpp                gui_dialogs/gui_user_controls.cpp
LOCAL_SRC_FILES += gui_dialogs/gui_menu_gameend_rivals.cpp
LOCAL_SRC_FILES += tinyxml/tinystr.cpp tinyxml/tinyxml.cpp tinyxml/tinyxmlerror.cpp tinyxml/tinyxmlparser.cpp

LOCAL_LDLIBS := -lEGL -llog -lGLESv1_CM -lGLESv2 -lz -landroid

LOCAL_STATIC_LIBRARIES := freetype tremor ogg Irrlicht android_native_app_glue
LOCAL_SHARED_LIBRARIES := openal freealut

include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/native_app_glue)
