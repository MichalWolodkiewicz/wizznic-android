LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := main

SDL_PATH := ../SDL2

LOCAL_C_INCLUDES := $(LOCAL_PATH)/$(SDL_PATH)/include

# Add your application source files here...
LOCAL_SRC_FILES := $(SDL_PATH)/src/main/android/SDL_android_main.c bundle.c draw.c mbrowse.c sound.c stats.c ticks.c about.c levels.c pixel.c scrollbar.c swscale.c credits.c game.c menu.c sprite.c strings.c transition.c levelselector.c settings.c teleport.c cursor.c input.c pack.c player.c stars.c strinput.c userfiles.c board.c skipleveldialog.c text.c leveleditor.c main.c particles.c pointer.c switch.c waveimg.c

LOCAL_SHARED_LIBRARIES := SDL2 SDL2_image

LOCAL_LDLIBS := -lGLESv1_CM -lGLESv2 -llog

include $(BUILD_SHARED_LIBRARY)

