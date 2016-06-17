#ifndef ANDROID_H_UTILS
#define ANDROID_H_UTILS

#include <assert.h>
#include <android/asset_manager_jni.h>
#include <jni.h>
#include <SDL.h>
#include <errno.h>

extern AAssetManager* asset_mgr;

FILE* android_fopen(const char* fname, const char* mode);

#endif