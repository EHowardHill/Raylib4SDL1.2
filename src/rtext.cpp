/*
 * rtext.cpp — Font loading, text drawing, and string utilities.
 *
 * Merges the functionality of the old text.cpp and rtext.cpp into a single
 * translation unit, backed by SDL_ttf 2.0.
 */

#include "raylib.h"

#include <SDL.h>
#include <SDL_ttf.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

/* ======================================================================== */
/* Globals & Externs                                                       */
/* ======================================================================== */

/* From core.cpp */
extern SDL_Surface *rl_activeSurface;

/* Owned by this file, exported for textures.cpp (ImageText) */
TTF_Font *rl_defaultFont = NULL;

/* ======================================================================== */
/* Internal Engine Hooks                                                   */
/* ======================================================================== */

void InitTextSystem(void) {
  if (TTF_Init() == -1) {
    fprintf(stderr, "TTF_Init failed: %s\n", TTF_GetError());
    return;
  }

  /*
   * Raylib normally embeds its default font. For a retro port, it's
   * safer/easier to require a physical file. Place "default.ttf" next to your
   * executable.
   */
  rl_defaultFont = TTF_OpenFont("default.ttf", 20);
  if (!rl_defaultFont) {
    fprintf(stderr, "Warning: Could not load default.ttf: %s\n",
            TTF_GetError());
  }
}

void CloseTextSystem(void) {
  if (rl_defaultFont) {
    TTF_CloseFont(rl_defaultFont);
    rl_defaultFont = NULL;
  }
  TTF_Quit();
}

/* ======================================================================== */
/* Basic Text Drawing                                                      */
/* ======================================================================== */

void DrawText(const char *text, int posX, int posY, int fontSize, Color color) {
  if (!rl_defaultFont || !rl_activeSurface || !text || text[0] == '\0')
    return;

  /* * SDL_ttf locks the font size at load time. To support dynamic sizes
   * accurately in SDL 1.2, users must use LoadFontEx and DrawTextEx.
   */
  (void)fontSize;

  /* SDL 1.2's SDL_Color does not use an alpha channel for solid rendering */
  SDL_Color sdlColor = {color.r, color.g, color.b, 0};

  /* TTF_RenderText_Solid is the fastest method (no alpha blending overhead) */
  SDL_Surface *textSurface =
      TTF_RenderText_Solid(rl_defaultFont, text, sdlColor);

  if (textSurface) {
    SDL_Rect destRect = {(Sint16)posX, (Sint16)posY, 0, 0};
    SDL_BlitSurface(textSurface, NULL, rl_activeSurface, &destRect);
    SDL_FreeSurface(textSurface);
  }
}

void DrawTextEx(Font font, const char *text, Vector2 position, float fontSize,
                float spacing, Color tint) {
  if (text == NULL || text[0] == '\0' || rl_activeSurface == NULL)
    return;

  TTF_Font *ttf = (TTF_Font *)(uintptr_t)font.texture.id;
  if (ttf == NULL)
    ttf = rl_defaultFont;
  if (ttf == NULL)
    return;

  (void)fontSize; /* SDL_ttf locks size at load time */
  (void)spacing;  /* SDL_ttf handles kerning internally via font files */

  SDL_Color sdlColor = {tint.r, tint.g, tint.b, 0};

  SDL_Surface *textSurface = TTF_RenderText_Solid(ttf, text, sdlColor);
  if (textSurface) {
    SDL_Rect destRect = {(Sint16)position.x, (Sint16)position.y, 0, 0};
    SDL_BlitSurface(textSurface, NULL, rl_activeSurface, &destRect);
    SDL_FreeSurface(textSurface);
  }
}

/* ======================================================================== */
/* Font Loading & Management                                               */
/* ======================================================================== */

Font GetFontDefault(void) {
  Font font;
  memset(&font, 0, sizeof(font));
  font.baseSize = 20;
  font.texture.id = (unsigned int)(uintptr_t)rl_defaultFont;
  return font;
}

Font LoadFont(const char *fileName) {
  Font font;
  memset(&font, 0, sizeof(font));
  font.baseSize = 32; /* Raylib default size for LoadFont */

  TTF_Font *ttf = TTF_OpenFont(fileName, font.baseSize);
  if (ttf != NULL) {
    font.texture.id = (unsigned int)(uintptr_t)ttf;
  }
  return font;
}

Font LoadFontEx(const char *fileName, int fontSize, int *codepoints,
                int codepointCount) {
  Font font;
  memset(&font, 0, sizeof(font));
  font.baseSize = fontSize;

  (void)codepoints;
  (void)codepointCount;

  TTF_Font *ttf = TTF_OpenFont(fileName, fontSize);
  if (ttf != NULL) {
    font.texture.id = (unsigned int)(uintptr_t)ttf;
  }
  return font;
}

void UnloadFont(Font font) {
  TTF_Font *ttf = (TTF_Font *)(uintptr_t)font.texture.id;
  if (ttf != NULL && ttf != rl_defaultFont) {
    TTF_CloseFont(ttf);
  }
}

bool IsFontValid(Font font) { return (font.texture.id != 0); }

/* ======================================================================== */
/* Text Measurement                                                        */
/* ======================================================================== */

int MeasureText(const char *text, int fontSize) {
  if (text == NULL || rl_defaultFont == NULL)
    return 0;

  (void)fontSize;
  int w = 0, h = 0;
  if (TTF_SizeText(rl_defaultFont, text, &w, &h) == 0) {
    return w;
  }
  return 0;
}

Vector2 MeasureTextEx(Font font, const char *text, float fontSize,
                      float spacing) {
  Vector2 size = {0.0f, 0.0f};
  if (text == NULL)
    return size;

  (void)fontSize;
  (void)spacing;

  TTF_Font *ttf = (TTF_Font *)(uintptr_t)font.texture.id;
  if (ttf == NULL)
    ttf = rl_defaultFont;
  if (ttf == NULL)
    return size;

  int w = 0, h = 0;
  if (TTF_SizeText(ttf, text, &w, &h) == 0) {
    size.x = (float)w;
    size.y = (float)h;
  }
  return size;
}

/* ======================================================================== */
/* Text Strings Management                                                 */
/* ======================================================================== */

unsigned int TextLength(const char *text) {
  if (text == NULL)
    return 0;
  return (unsigned int)strlen(text);
}

int TextCopy(char *dst, const char *src) {
  if (dst == NULL || src == NULL)
    return 0;
  strcpy(dst, src);
  return (int)strlen(dst);
}

bool TextIsEqual(const char *text1, const char *text2) {
  if (text1 == NULL || text2 == NULL)
    return false;
  return (strcmp(text1, text2) == 0);
}

const char *TextFormat(const char *text, ...) {
  /* Raylib uses a ring buffer to allow multiple TextFormat calls per frame */
  static char buffers[4][1024];
  static int index = 0;

  char *currentBuffer = buffers[index];
  index = (index + 1) % 4;

  va_list args;
  va_start(args, text);
  vsnprintf(currentBuffer, 1024, text, args);
  va_end(args);

  return currentBuffer;
}

const char *TextToUpper(const char *text) {
  static char buffer[1024];
  int i = 0;
  if (text == NULL)
    return "";

  while (text[i] != '\0' && i < 1023) {
    buffer[i] = (char)toupper((unsigned char)text[i]);
    i++;
  }
  buffer[i] = '\0';
  return buffer;
}

const char *TextToLower(const char *text) {
  static char buffer[1024];
  int i = 0;
  if (text == NULL)
    return "";

  while (text[i] != '\0' && i < 1023) {
    buffer[i] = (char)tolower((unsigned char)text[i]);
    i++;
  }
  buffer[i] = '\0';
  return buffer;
}