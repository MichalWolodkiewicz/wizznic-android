#include "androidUtils.h"

AAssetManager* asset_mgr;

void Java_com_game_wizznic_HelloSDL2Activity_initAndroidUtils(JNIEnv *env, jobject clazz, jobject assetManager) {
	asset_mgr = AAssetManager_fromJava(env, assetManager);
	if (asset_mgr != NULL) {
		SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "AAssetManager_fromJava ok");
	} else {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "AAssetManager_fromJava failed");
	}
}

static int android_read(void* cookie, char* buf, int size) {
  return AAsset_read((AAsset*)cookie, buf, size);
}

static int android_write(void* cookie, const char* buf, int size) {
  return EACCES; // can't provide write access to the apk
}

static fpos_t android_seek(void* cookie, fpos_t offset, int whence) {
  return AAsset_seek((AAsset*)cookie, offset, whence);
}

static int android_close(void* cookie) {
  AAsset_close((AAsset*)cookie);
  return 0;
}

FILE* android_fopen(const char* fname, const char* mode) {
  if(mode[0] == 'w') return NULL;

  AAsset* asset = AAssetManager_open(asset_mgr, fname, 0);
  if(!asset) return NULL;

  return funopen(asset, android_read, android_write, android_seek, android_close);
}