/************************************************************************
 * This file is part of Wizznic.                                        *
 * Copyright 2009-2015 Jimmy Christensen <dusted@dusted.dk>             *
 * Wizznic is free software: you can redistribute it and/or modify      *
 * it under the terms of the GNU General Public License as published by *
 * the Free Software Foundation, either version 3 of the License, or    *
 * (at your option) any later version.                                  *
 *                                                                      *
 * Wizznic is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of       *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        *
 * GNU General Public License for more details.                         *
 *                                                                      *
 * You should have received a copy of the GNU General Public License    *
 * along with Wizznic.  If not, see <http://www.gnu.org/licenses/>.     *
 ************************************************************************/

#include "userfiles.h"
#include "defs.h"

#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static char* strConfDir;
static char* strEditLvlDir;
static char* strUsrPackDir;
static char* strHsDir;

void Java_com_game_wizznic_HelloSDL2Activity_initAppFolders(JNIEnv *env, jobject clazz, jstring storageRootPath) {
	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "---- init folders start");
	strConfDir = (*env)->GetStringUTFChars(env, storageRootPath, 0);
	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "---- init folders end strConfDir =  %s", strConfDir);
}

void initUserPaths()
{
  SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "initUserPaths");
  strEditLvlDir = malloc( sizeof(char)*( strlen(strConfDir)+strlen("/editorlevels")+1 ) );
  sprintf( strEditLvlDir, "%s/editorlevels", strConfDir );

  strHsDir = malloc( sizeof(char)*( strlen(strConfDir)+strlen("/highscores")+1 ) );
  sprintf( strHsDir, "%s/highscores", strConfDir );

  strUsrPackDir = malloc( sizeof(char)*( strlen(strConfDir)+strlen("/dlc")+1 ) );
  sprintf( strUsrPackDir, "%s/dlc", strConfDir );

  //Check if dir exists (we check each dir, since ./ might exist without the others
  struct stat s;

  if (stat(strConfDir,&s) != 0) PLATFORM_MKDIR(strConfDir);
  if (stat(strEditLvlDir,&s) != 0) PLATFORM_MKDIR(strEditLvlDir);
  if (stat(strHsDir,&s) != 0) PLATFORM_MKDIR(strHsDir);
  if (stat(strUsrPackDir,&s) != 0) PLATFORM_MKDIR(strUsrPackDir);
}

char* getConfigDir() { return( strConfDir ); }
char* getHighscoreDir() { return( strHsDir ); }
char* getUserLevelDir() { return( strEditLvlDir ); }
char* getUsrPackDir() { return( strUsrPackDir ); }
