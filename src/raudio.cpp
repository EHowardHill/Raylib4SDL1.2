/*
 * raudio.cpp — Audio device, sound effects, and music streaming via SDL_mixer.
 *
 * SDL_mixer runs audio on a background thread, so UpdateMusicStream() is a
 * no-op (unlike real Raylib which feeds the buffer manually).
 */

#include "raylib.h"

#include <SDL.h>
#include <SDL_mixer.h>
#include <string.h>

static bool audioInitialized = false;
static float masterVolume = 1.0f;

/* ======================================================================== */
/*  Audio Device Management                                                 */
/* ======================================================================== */

void InitAudioDevice(void) {
  if (audioInitialized)
    return;

  if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0)
    return;

  /* 44.1 kHz, 16-bit signed, stereo, 2048-byte buffer */
  if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
    return;
  }

  Mix_AllocateChannels(16);
  audioInitialized = true;
}

void CloseAudioDevice(void) {
  if (audioInitialized) {
    Mix_CloseAudio();
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
    audioInitialized = false;
  }
}

bool IsAudioDeviceReady(void) { return audioInitialized; }

void SetMasterVolume(float volume) {
  if (volume < 0.0f)
    volume = 0.0f;
  if (volume > 1.0f)
    volume = 1.0f;
  masterVolume = volume;

  Mix_Volume(-1, (int)(masterVolume * MIX_MAX_VOLUME));
  Mix_VolumeMusic((int)(masterVolume * MIX_MAX_VOLUME));
}

/* ======================================================================== */
/*  Sound (short effects — Mix_Chunk)                                       */
/* ======================================================================== */

Sound LoadSound(const char *fileName) {
  Sound sound;
  memset(&sound, 0, sizeof(sound));
  if (!audioInitialized)
    return sound;

  Mix_Chunk *chunk = Mix_LoadWAV(fileName);
  if (chunk != NULL) {
    sound.stream.buffer = (void *)chunk;
  }
  return sound;
}

void UnloadSound(Sound sound) {
  if (sound.stream.buffer != NULL) {
    Mix_FreeChunk((Mix_Chunk *)sound.stream.buffer);
  }
}

void PlaySound(Sound sound) {
  if (sound.stream.buffer == NULL)
    return;
  Mix_PlayChannel(-1, (Mix_Chunk *)sound.stream.buffer, 0);
}

void StopSound(Sound sound) {
  /*
   * SDL_mixer doesn't track which channel a chunk is playing on.
   * A proper port would map chunks to channels; we halt all as fallback.
   */
  (void)sound;
  Mix_HaltChannel(-1);
}

void PauseSound(Sound sound) {
  (void)sound;
  Mix_Pause(-1);
}
void ResumeSound(Sound sound) {
  (void)sound;
  Mix_Resume(-1);
}

bool IsSoundPlaying(Sound sound) {
  (void)sound;
  return Mix_Playing(-1) > 0;
}

void SetSoundVolume(Sound sound, float volume) {
  if (sound.stream.buffer == NULL)
    return;
  if (volume < 0.0f)
    volume = 0.0f;
  if (volume > 1.0f)
    volume = 1.0f;

  Mix_VolumeChunk((Mix_Chunk *)sound.stream.buffer,
                  (int)(volume * masterVolume * MIX_MAX_VOLUME));
}

/* ======================================================================== */
/*  Music (long streaming tracks — Mix_Music)                               */
/* ======================================================================== */

Music LoadMusicStream(const char *fileName) {
  Music music;
  memset(&music, 0, sizeof(music));
  if (!audioInitialized)
    return music;

  Mix_Music *mus = Mix_LoadMUS(fileName);
  if (mus != NULL) {
    music.ctxData = (void *)mus;
    music.looping = true;
  }
  return music;
}

void UnloadMusicStream(Music music) {
  if (music.ctxData != NULL) {
    Mix_FreeMusic((Mix_Music *)music.ctxData);
  }
}

void PlayMusicStream(Music music) {
  if (music.ctxData == NULL)
    return;
  Mix_PlayMusic((Mix_Music *)music.ctxData, music.looping ? -1 : 1);
}

bool IsMusicStreamPlaying(Music music) {
  (void)music;
  return Mix_PlayingMusic() == 1;
}

void UpdateMusicStream(Music music) {
  /*
   * SDL_mixer streams music on a background thread, so there is nothing
   * to feed here.  We keep the function for Raylib API compatibility.
   */
  (void)music;
}

void StopMusicStream(Music music) {
  (void)music;
  Mix_HaltMusic();
}
void PauseMusicStream(Music music) {
  (void)music;
  Mix_PauseMusic();
}
void ResumeMusicStream(Music music) {
  (void)music;
  Mix_ResumeMusic();
}

void SetMusicVolume(Music music, float volume) {
  (void)music;
  if (volume < 0.0f)
    volume = 0.0f;
  if (volume > 1.0f)
    volume = 1.0f;
  Mix_VolumeMusic((int)(volume * masterVolume * MIX_MAX_VOLUME));
}

float GetMusicTimeLength(Music music) {
  (void)music;
  /* SDL_mixer 1.2 doesn't expose total duration easily.
     Return 0 as a safe fallback. */
  return 0.0f;
}

float GetMusicTimePlayed(Music music) {
  (void)music;
  /* SDL_mixer 1.2 doesn't expose current playback position.
     Return 0 as a safe fallback. */
  return 0.0f;
}
