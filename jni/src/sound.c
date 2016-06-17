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

#include "sound.h"
#include "pack.h"
#include "settings.h"
#include <SDL.h>
#include <SDL_mixer.h>
#include "states.h"
#include "input.h"
#include "text.h"
#include "mbrowse.h"
#include "menu.h"

#include "defs.h"

#define MUSIC_FADETIME 500


static Mix_Music* mus[2] = {0,0};
static double mPos[2] = { 0,0 }; //For keeping track of track positions since SDL_Mixer cant.

static char lastLoadedSongFn[2048];

static char* loadedSamples[NUMSAMPLES];
static Mix_Chunk* samples[NUMSAMPLES];
static int lastPlayed[NUMSAMPLES];


int initSound()
{
  int audio_rate = SOUND_RATE;
  Uint16 audio_format = SOUND_FORMAT;
  int audio_channels = 2;
  int audio_buffers = SOUND_BUFFERS;

  if(Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers))
  {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't open audio: '%s'\n", Mix_GetError());
    return(0);
  }

  Mix_AllocateChannels(SOUND_MIX_CHANNELS);

  //Set arrays of pointers to null.
  memset(samples, 0, sizeof(Mix_Chunk*)*NUMSAMPLES);
  memset(loadedSamples, 0, sizeof(char*)*NUMSAMPLES);
  memset(lastPlayed, 0, sizeof(int)*NUMSAMPLES);

  loadMenuSamples();

  lastLoadedSongFn[0]=0;

  return(1);
}


int loadSample(const char* fileName, int index)
{
  lastPlayed[index]=0;
#ifdef DEBUG
  SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "loadSample(); Open: %s\n", fileName);
#endif
  //Check if we should load it
  if(!loadedSamples[index] || strcmp(loadedSamples[index], fileName)!=0)
  {
    //printf("loadSample: %s not loaded before, loading it now...\n", fileName);
    //Free string memory if used previously.
    if(loadedSamples[index])
      free(loadedSamples[index]);

    //Copy string
    loadedSamples[index] = malloc( sizeof(char)*(strlen(fileName)+1) );
    strcpy(loadedSamples[index], fileName);

    //Free chunck memory if used previously
    if(samples[index])
      Mix_FreeChunk(samples[index]);

    //Load sample
    samples[index] = Mix_LoadWAV(fileName);
    if(!samples[index])
    {
      SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "loadSample(); Warning: Couldn't load %s\n",fileName);
      return(0);
    }
  }

  return(1);
}

void loadSamples(const char* sndDir, const char* musicFile)
{
  loadSample( packGetFile(sndDir,"click.ogg"), SND_CLICK );
  loadSample( packGetFile(sndDir,"brickmove.ogg"), SND_BRICKMOVE );
  loadSample( packGetFile(sndDir,"brickgrab.ogg"), SND_BRICKGRAB );
  loadSample( packGetFile(sndDir,"brickbreak.ogg"), SND_BRICKBREAK );
  loadSample( packGetFile(sndDir,"scoretick.ogg"), SND_SCORECOUNT );
  loadSample( packGetFile(sndDir,"countdown.ogg"), SND_COUNTDOWNTOSTART );
  loadSample( packGetFile(sndDir,"start.ogg"), SND_START );
  loadSample( packGetFile(sndDir,"timeout.ogg"), SND_TIMEOUT );
  loadSample( packGetFile(sndDir,"victory.ogg"), SND_VICTORY );
  loadSample( packGetFile(sndDir,"timeout.ogg"), SND_TIMEOUT );
  loadSample( packGetFile(sndDir,"onewaymove.ogg"), SND_ONEWAY_MOVE );
  loadSample( packGetFile(sndDir,"teleported.ogg"), SND_TELEPORTED );
  loadSample( packGetFile(sndDir,"switchactivate.ogg"), SND_SWITCH_ACTIVATED );
  loadSample( packGetFile(sndDir,"switchinactive.ogg"), SND_SWITCH_DEACTIVATED );
  loadSample( packGetFile(sndDir,"brickswap.ogg"), SND_BRICKSWAP );
  loadSample( packGetFile(sndDir,"brickcopy.ogg"), SND_BRICKCOPY );
  loadSample( packGetFile(sndDir,"brickswapdenied.ogg"), SND_BRICKSWAP );
  loadSample( packGetFile(sndDir,"brickcopydenied.ogg"), SND_BRICKCOPY );
  loadSample( packGetFile(sndDir, "winner.ogg"), SND_WINNER);
  loadSample( packGetFile(sndDir, "loser.ogg"), SND_LOSER);

  //Music load code
  if(setting()->disableMusic) return;
  if( !musicFile ) return;

  //Load ingame song if not allready loaded.
  if(strcmp(lastLoadedSongFn, packGetFile(NULL,musicFile))!=0 && !setting()->userMusic)
  {
    //Free old song if loaded.
    if(mus[1])
    {
      Mix_FreeMusic(mus[1]);
    }
    strcpy(lastLoadedSongFn, packGetFile(NULL,musicFile));
    mus[1]=Mix_LoadMUS( lastLoadedSongFn );
    if(!mus[1])
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't load music: '%s'\n",packGetFile(NULL,musicFile));

    mPos[1] = 0.0f;

    if( !Mix_PlayingMusic() )
    {
      Mix_FadeInMusicPos(mus[1], -1, MUSIC_FADETIME,mPos[1]);
    }
  }

}

void loadMenuSamples()
{
  loadSample( "data/snd/menumove.ogg", SND_MENUMOVE );
  loadSample( "data/snd/menuclick.ogg", SND_MENUCLICK );
  loadSample( "data/snd/rocketboom.ogg", SND_ROCKETBOOM );
  loadSample( "data/snd/rocketlaunch.ogg", SND_ROCKETLAUNCH );
  loadSample( packGetFile("themes/oldskool/snd", "winner.ogg"), SND_WINNER);
  loadSample( packGetFile("themes/oldskool/snd", "loser.ogg"), SND_LOSER);
}

void sndPlay(int sample, int posX)
{
  int freeChannel=-1;
  int i;

  //Find free channel
  for(i=0;i<16;i++)
  {
    if(!Mix_Playing(i))
    {
      freeChannel=i;
      break;
    }
  }

  int right = (posX*255)/SCREENW;

  Mix_SetPanning(freeChannel, 255-right, right );

  Mix_PlayChannel(freeChannel, samples[sample], 0);
}

//Only play if sample was not played the last couple of frames
void sndPlayOnce(int sample, int posX)
{
  int ticks=SDL_GetTicks();
  if(ticks-lastPlayed[sample] < 50) return;

  lastPlayed[sample]=ticks;
  sndPlay(sample, posX);
}


static int lastState = STATEMENU;
static int fadeOut=0;
static int userSong=0;
static int numUserSongs=0;
#define SHOW_SONG_TIME 1500
static int showSNCD=0;
#define CMSTATE_MENU 1
#define CMSTATE_GAME 2
static int cmState = 0; //Current music state

void soundRun(SDL_Surface* screen,int state)
{
  //Wiz-Volume control
  #if defined (GP2X) || defined (WIZ)
  if(getButton( C_BTNVOLUP ) )
  {
    resetBtn( C_BTNVOLUP );
    WIZ_AdjustVolume(VOLUME_UP);
  } else if(getButton( C_BTNVOLDOWN ) )
  {
    resetBtn( C_BTNVOLDOWN );
    WIZ_AdjustVolume(VOLUME_DOWN);
  }
  WIZ_ShowVolume(screen);
  #endif

  //Rest of code controls music, we return now if music is not playing.
  if( !setting()->musicVol || setting()->disableMusic ) return;

  if(setting()->userMusic && numUserSongs)
  {
    //Check if we should change track because the track stopped
    if(!Mix_PlayingMusic())
    {
      userSong++;
      if(userSong==numUserSongs)
        userSong=0;

      soundPlayUserSongNum(userSong, lastLoadedSongFn);
    }

    //Change track because player wants to
    if(getButton(C_SHOULDERA))
    {
      resetBtn(C_SHOULDERA);
      userSong++;
      if(userSong==numUserSongs)
        userSong=0;
      soundPlayUserSongNum(userSong, lastLoadedSongFn);
    }
    if(getButton(C_SHOULDERB))
    {
      resetBtn(C_SHOULDERB);
      userSong--;
      if(userSong<0)
        userSong=numUserSongs-1;
      soundPlayUserSongNum(userSong, lastLoadedSongFn);
    }

    //Should we show the song title?
    if(showSNCD>0)
    {
      showSNCD-=getTicks();
      txtWriteCenter(screen, FONTSMALL, lastLoadedSongFn, HSCREENW, HSCREENH+80);
    }

  } else {
    if(showSNCD>0)
    {
      showSNCD-=getTicks();
      txtWriteCenter(screen, FONTSMALL, "Select Music Directory in Options", HSCREENW, HSCREENH+80);
    } else if(getButton(C_SHOULDERA) || getButton(C_SHOULDERB))
    {
      resetBtn(C_SHOULDERA);
      resetBtn(C_SHOULDERB);
      showSNCD=3000;
    }

    if(fadeOut > 0)
    {
      fadeOut -= getTicks();
      if(fadeOut < 0)
      {
        fadeOut=0,
        lastState=state;

        switch(state)
        {
          case STATEPLAY:
            cmState=CMSTATE_GAME;
            Mix_FadeInMusicPos(mus[1], -1, MUSIC_FADETIME,mPos[1]);
          break;
          case STATEMENU:
            cmState=CMSTATE_MENU;
            Mix_FadeInMusicPos(mus[0], -1, MUSIC_FADETIME,mPos[0]);
          break;
        }
      }
    }

    if(lastState!=state && fadeOut == 0 )
    {
        //We won't change the music the menuparts which are logically "ingame".
        if( !( getMenuState() == menuStateFinishedLevel || (getMenuState() == menuStateNextLevel && cmState==CMSTATE_GAME) ) )
        {
          fadeOut = MUSIC_FADETIME+20; //We add an extra frames time to make sure the previous fade was completed.
          Mix_FadeOutMusic(MUSIC_FADETIME);
        }
    }

    switch(lastState)
    {
      case STATEPLAY:
        mPos[1] += (double)getTicks() / 1000.0f;
      break;
      case STATEMENU:
        mPos[0] += (double)getTicks() / 1000.0f;
      break;
    }
  }//In-Game music

}

void soundPlayUserSongNum(int num, char* songName)
{
  if(!setting()->musicVol) return;
  listItem* it=&fileList()->begin;
  fileListItem_t* file;
  int i=0;
  while( LISTFWD(fileList(),it) )
  {
    file=(fileListItem_t*)it->data;
    if(!file->dir)
    {
      if(i==num)
      {
        //Unload current song.
        Mix_HaltMusic();
        if(mus[1])
        {
          Mix_FreeMusic(mus[1]);
          mus[1]=0;
        }
        //Load new song
        mus[1]=Mix_LoadMUS( file->fullName );
        if(!mus[1])
        {
          SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't load music: '%s'\n",file->fullName);
        }
        else
        {
          SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Now Playing: '%s'\n",file->fullName);
          if(songName)
          {
            showSNCD=SHOW_SONG_TIME;
            strcpy(songName,file->name);
          }
        }

        Mix_FadeInMusicPos(mus[1], 0, MUSIC_FADETIME,mPos[1]);


        //return; We don't return, instead we let it run, to count number of usertracks.
      }
      i++;
    }
  }
  numUserSongs=i;
}


//Sets music to either in-game or to user-selected.
void soundSetMusic()
{
  if(mus[0])
  {
    Mix_FreeMusic(mus[0]);
    mus[0]=0;
  }

  if(mus[1])
  {
    Mix_FreeMusic(mus[1]);
    mus[1]=0;
  }
  mPos[0]=0.0f;
  mPos[1]=0.0f;

  if(setting()->disableMusic) return;

  //Load list of userMusic and load that
  if(setting()->userMusic)
  {

    fileListMake( setting()->musicDir );
    //Load first song
    soundPlayUserSongNum(0,0); //Sets number of tracks too.
  } else {//Or load in-game music
    //Load the menu-song
    mus[0] = Mix_LoadMUS( "data/menu-music.ogg" );

    if(!mus[0])
    {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "initSound(); Error; couldn't load menu-music.\n");
    }

    Mix_FadeInMusic(mus[0], -1, 1500);
  } //Ingame
}

void soundSetMusVol(int v)
{
  if(v==0)
  {
    if( !Mix_PausedMusic() )
    {
      Mix_PauseMusic();
    }
  } else {
    if( Mix_PausedMusic() )
    {
      Mix_ResumeMusic();
    }
  }
  Mix_VolumeMusic(setting()->musicVol);
}
