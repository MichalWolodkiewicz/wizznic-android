package com.game.wizznic;

import android.os.Bundle;
import android.content.res.AssetManager;
import org.libsdl.app.SDLActivity;

public class HelloSDL2Activity extends SDLActivity
{	
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		initAndroidUtils(getAssets());
		initAppFolders(getFilesDir().getAbsolutePath());
	}
	
	native void initAndroidUtils(AssetManager manager);
	
	native void initAppFolders(String storageRoot);
}
