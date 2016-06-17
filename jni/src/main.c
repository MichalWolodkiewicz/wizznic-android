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

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <SDL.h>
#include <SDL_image.h>

#include "defs.h"
#include "board.h"
#include "cursor.h"
#include "draw.h"
#include "input.h"
#include "sprite.h"
#include "text.h"
#include "sound.h"
#include "states.h"
#include "game.h"
#include "levels.h"
#include "leveleditor.h"
#include "particles.h"
#include "settings.h"
#include "pack.h"
#include "stats.h"
#include "credits.h"
#include "userfiles.h"
#include "strings.h"
#include "swscale.h"
#include "pointer.h"
#include "transition.h"
#include "platform/libDLC.h"


SDL_Surface* swScreen()
{
  SDL_Surface *screen = SDL_CreateRGBSurface(0, 1024, 600, 32,
                                        0x00FF0000,
                                        0x0000FF00,
                                        0x000000FF,
                                        0xFF000000);
	
  if( !screen ) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,  "Screen surface create failed. Time to exit ...");
    return(NULL);
  } else {
	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,  "Window create ok ...");
    setting()->glEnable=0;
  }
  return (screen);
}

int main(int argc, char *argv[]) {
  int doScale = 0; // 0=Undefined, 1=320x240, -1=OpenGL, >1=SwScale
  char* dumpPack = NULL;
  int state = 1; //Game, Menu, Editor, Quit
  int sdlVideoModeFlags = SDL_SWSURFACE;
  int i;

  //initialize path strings
  initUserPaths();

  //Read settings
  initSettings();
  SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,  "initSettings() completed");
  #if defined(WITH_OPENGL)
  SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,  "WITH_OPENGL is defined");
  //We start by enabling glScaling if it was enabled in settings, it can then be overwritten by command line options.
  if( setting()->glEnable && doScale==0 )
    doScale=-1;
  #endif

  //Set scaling
  setting()->scaleFactor=1.0;

  atexit(SDL_Quit);

  //Init SDL
  if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK | SDL_INIT_TIMER ) < 0 ) {
	SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_Init failed: %s\n", SDL_GetError());
    return(-1);
  }
  SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,  "SDL_Init is ok");

  SDL_Surface* screen = NULL;
  
  SDL_Window *sdlWindow = SDL_CreateWindow("Wizznic Android",
							SDL_WINDOWPOS_UNDEFINED,
							SDL_WINDOWPOS_UNDEFINED,
                            0, 0,
                            SDL_WINDOW_FULLSCREEN_DESKTOP);
  
  SDL_Renderer *sdlRenderer = SDL_CreateRenderer(sdlWindow, -1, 0);
  
  SDL_Texture *sdlTexture = SDL_CreateTexture(sdlRenderer,
                                            SDL_PIXELFORMAT_ARGB8888,
                                            SDL_TEXTUREACCESS_STREAMING,
                                            1024, 600);
  
  SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,  "Parameters passed ok.");

  if(doScale)
  {
	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,  "doScale == true");
    //Hardware accelerated scaling
    if( doScale == -1 )
    {
    #ifdef HAVE_ACCELERATION
      SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Enabling platform specific accelerated scaling.\n");
      screen = platformInitAccel(sdlVideoModeFlags);
      if( !screen )
      {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Failed to set platform accelerated scaling, falling back to software window.\n");
        screen=swScreen();
        doScale=0;
      }
    #else
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "\nError:\n  Not compiled with hardware-scaling support, don't give me -z -1\n  Exiting...\n");
      return(-1);
    #endif
    } else if( doScale > 0 )
    {
    #ifdef WANT_SWSCALE
      //Set up software scaling
      SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Enabling slow software-based scaling to %ix%i.\n",320*doScale, 240*doScale);
      screen = swScaleInit(sdlVideoModeFlags,doScale);
    #else
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "\nError:\n  I don't support software scaling, don't give me any -z options\n  Exiting...\n");
      return(-1);
    #endif
    }
  } else {
	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,  "doScale == false");
    screen=swScreen();
	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,  "Screen was returned");
    doScale=0;
  }

  if( screen == NULL )
  {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,  "Couldnt init video. Time to exit ...");
    return(-1);
  }
 
  setting()->bpp = SDL_GetWindowPixelFormat(sdlWindow);
  setAlphaCol( setting()->bpp );
  
  //Load fonts
  txtInit();
  SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,  "Fonts init ok.");

  //Menu Graphics
  if(!initMenu(screen))
  {
	SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,  "Couldn't load menu graphics");
    return(-1);
  }
  SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,  "initMenu() ok.");

  //Init controls
  initControls();
  SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,  "initControls() ok.");
	
  //Init stats
  statsInit();
  SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,  "statsInit() ok.");
  
  //Init packs
  packInit();
  SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,  "packInit() ok.");
 
  //Load sounds
  if(!initSound())
  {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,  "Could not init sound.");
    return(-1);
  }
  
  SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,  "initSound() ok.");

  //Scan userlevels dir
  makeUserLevelList(screen);
  SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,  "makeUserLevelList() ok.");
	
  //Init particles
  initParticles(screen);
  SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,  "initParticles() ok.");

  //Seed the pseudo random number generator (for particles 'n' stuff)
  srand( (int)time(NULL) );
  SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,  "srand( (int)time(NULL) );() ok.");

  //init starfield
  initStars(screen);
  SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,  "initStars() ok.");

  //Init pointer
  initPointer(screen);
  SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,  "initPointer() ok.");
  
  //Apply settings (has to be done after packs are inited)
  applySettings();
  SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,  "applySettings() ok.");
  
  //Set Pack
  packSetByPath( setting()->packDir );
  SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,  "packSetByPath() ok.");

  #if defined( PLATFORM_SUPPORTS_STATSUPLOAD )
  if( (setting()->uploadStats) && !(setting()->firstRun) )
  {
    statsUpload(0,0,0,0,0,"check",1, &(setting()->session) );
    statsUpload(0,0,0,0,0,"q_solved",1, &(setting()->solvedWorldWide) );

    //DLC only works when stats-uploading is enabled so we can use the same nag-screen.
    dlcCheckOnline();
  }
  #endif

  //Start playing music (has to be done after readong settings)
  soundSetMusic();
  SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,  "soundSetMusic() ok.");

  //Initialize credits
  initCredits(screen);
  SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,  "initCredits(screen) ok."); 
  
  initTransition();
  SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,  "initTransition() ok."); 
	
  SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,  "Screen size is %dx%d", SCREENW, SCREENH); 	
	
#if SCREENW != 320 || SCREENH != 240
  SDL_Rect *borderSrcRect = malloc(sizeof(SDL_Rect));
  SDL_Surface* border = loadImg( BORDER_IMAGE );
  if( border )
  {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Border image loaded.\n");
    borderSrcRect->x=(border->w-SCREENW)/2;
    borderSrcRect->y=(border->h-SCREENH)/2;
    borderSrcRect->w=SCREENW;
    borderSrcRect->h=SCREENH;
    SDL_BlitSurface( border, borderSrcRect, screen, NULL );
    SDL_FreeSurface(border);
  } else {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Could not load border image: %s\n", BORDER_IMAGE);
    SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0,0,0));
  }
  free(borderSrcRect);
  borderSrcRect=NULL;

#endif

  int lastTick;
  SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,  "start main loop ... :D"); 
  while(state!=STATEQUIT)
  {
    lastTick=SDL_GetTicks();

    frameStart();

    if(runControls()) state=STATEQUIT;
    switch(state)
    {
      case STATEPLAY:
        state = runGame(screen);
      break;

      case STATEMENU:
        state = runMenu(screen);
      break;

      case STATEEDIT:
        state=runEditor(screen);
      break;
    }

    drawPointer(screen);

    soundRun(screen,state);

    runTransition(screen);

    if(setting()->showFps)
      drawFPS(screen);

    switch( doScale )
    {
      #if defined(HAVE_ACCELERATION)
      case -1:
        platformDrawScaled(screen);
        break;
      #endif
      case 0:
		  SDL_UpdateTexture(sdlTexture, NULL, screen->pixels, screen->pitch);
		  SDL_RenderClear(sdlRenderer);
		  SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, NULL);
		  SDL_RenderPresent(sdlRenderer);
        break;
      #if defined(WANT_SWSCALE)
      default:
        swScale(screen,doScale);
        break;
      #else
      default:
        state=STATEQUIT;
      break;
      #endif
    }

    #if defined(CRUDE_TIMING)
    //Oh how I loathe this, is there no better way?
    while(SDL_GetTicks()-lastTick <= PLATFORM_CRUDE_TIMING_TICKS)
    {
      //Burn, burn baby burn!
    }
    #else
    int t=SDL_GetTicks()-lastTick;
    if(t < 20)
    {
      SDL_Delay( 20 -t);
    }
    #endif
  }
 SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,  "after main loop ... time to leave wizznic :D"); 
  #if defined(PLATFORM_NEEDS_EXIT)
  platformExit();
  #endif

  SDL_Quit();

  return(0);
}
