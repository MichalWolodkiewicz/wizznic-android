LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := wizznic

LOCAL_STATIC_LIBRARIES := SDL2_static

include $(BUILD_SHARED_LIBRARY)
$(call import-module,SDL)LOCAL_PATH := $(call my-dir)
