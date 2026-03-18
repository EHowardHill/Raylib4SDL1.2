/*
 * rtextures.cpp — Image and Texture load, unload, manipulation, and drawing.
 *
 * Merges textures.cpp and rtextures.cpp, backed by SDL 1.2 and SDL_image.
 * Color utility functions have been omitted to prevent ODR violations
 * (they are handled in rcolors.cpp).
 */

#include "raylib.h"

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* From core.cpp */
extern SDL_Surface *rl_screen;
extern SDL_Surface *rl_activeSurface;
extern int rl_currentBlendMode;

/* From rtext.cpp */
extern TTF_Font *rl_defaultFont;

extern bool rl_IsCamera2DActive(void);
extern Camera2D rl_GetCamera2D(void);
extern Vector2 GetWorldToScreen2D(Vector2 position, Camera2D camera);

/* ======================================================================== */
/* Image Management (CPU / RAM)                                            */
/* ======================================================================== */

Image LoadImage(const char *fileName) {
  Image img;
  memset(&img, 0, sizeof(img));

  SDL_Surface *surface = IMG_Load(fileName);
  if (surface) {
    img.data = (void *)surface;
    img.width = surface->w;
    img.height = surface->h;
    img.mipmaps = 1;
    img.format = 7; /* UNCOMPRESSED_R8G8B8A8 placeholder */
  } else {
    fprintf(stderr, "WARNING: Failed to load image %s: %s\n", fileName,
            IMG_GetError());
  }
  return img;
}

Image GenImageColor(int width, int height, Color color) {
  Image img;
  memset(&img, 0, sizeof(img));

  SDL_Surface *surf =
      SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, 32, 0x00FF0000,
                           0x0000FF00, 0x000000FF, 0xFF000000);

  if (surf) {
    Uint32 clr = SDL_MapRGBA(surf->format, color.r, color.g, color.b, color.a);
    SDL_FillRect(surf, NULL, clr);
    img.data = (void *)surf;
    img.width = width;
    img.height = height;
    img.mipmaps = 1;
    img.format = 7;
  }
  return img;
}

void UnloadImage(Image image) {
  if (image.data != NULL) {
    SDL_FreeSurface((SDL_Surface *)image.data);
  }
}

Image ImageCopy(Image image) {
  Image copy;
  memset(&copy, 0, sizeof(copy));
  if (image.data == NULL)
    return copy;

  SDL_Surface *src = (SDL_Surface *)image.data;
  SDL_Surface *dst = SDL_CreateRGBSurface(
      src->flags, src->w, src->h, src->format->BitsPerPixel, src->format->Rmask,
      src->format->Gmask, src->format->Bmask, src->format->Amask);

  if (dst) {
    SDL_BlitSurface(src, NULL, dst, NULL);
    copy.data = (void *)dst;
    copy.width = dst->w;
    copy.height = dst->h;
    copy.mipmaps = 1;
    copy.format = image.format;
  }
  return copy;
}

void ImageClearBackground(Image *dst, Color color) {
  if (!dst || !dst->data)
    return;
  SDL_Surface *surf = (SDL_Surface *)dst->data;
  Uint32 clr = SDL_MapRGBA(surf->format, color.r, color.g, color.b, color.a);
  SDL_FillRect(surf, NULL, clr);
}

void ImageDraw(Image *dst, Image src, Rectangle srcRec, Rectangle dstRec,
               Color tint) {
  if (!dst || !dst->data || !src.data)
    return;
  (void)tint; /* No software tinting for perf on retro platforms */

  SDL_Surface *dSurf = (SDL_Surface *)dst->data;
  SDL_Surface *sSurf = (SDL_Surface *)src.data;

  SDL_Rect sRect = {(Sint16)srcRec.x, (Sint16)srcRec.y, (Uint16)srcRec.width,
                    (Uint16)srcRec.height};
  SDL_Rect dRect = {(Sint16)dstRec.x, (Sint16)dstRec.y, (Uint16)dstRec.width,
                    (Uint16)dstRec.height};

  if ((int)srcRec.width == (int)dstRec.width &&
      (int)srcRec.height == (int)dstRec.height) {
    SDL_BlitSurface(sSurf, &sRect, dSurf, &dRect);
  } else {
    SDL_SoftStretch(sSurf, &sRect, dSurf, &dRect);
  }
}

void ImageFlipVertical(Image *image) {
  if (!image || !image->data)
    return;
  SDL_Surface *s = (SDL_Surface *)image->data;

  SDL_LockSurface(s);
  int pitch = s->pitch;
  char *temp = (char *)malloc(pitch);
  char *pixels = (char *)s->pixels;

  for (int i = 0; i < s->h / 2; ++i) {
    char *row1 = pixels + i * pitch;
    char *row2 = pixels + (s->h - i - 1) * pitch;
    memcpy(temp, row1, pitch);
    memcpy(row1, row2, pitch);
    memcpy(row2, temp, pitch);
  }
  free(temp);
  SDL_UnlockSurface(s);
}

void ImageFlipHorizontal(Image *image) {
  if (!image || !image->data)
    return;
  SDL_Surface *s = (SDL_Surface *)image->data;
  int bpp = s->format->BytesPerPixel;

  SDL_LockSurface(s);
  for (int y = 0; y < s->h; y++) {
    Uint8 *row = (Uint8 *)s->pixels + y * s->pitch;
    for (int x = 0; x < s->w / 2; x++) {
      Uint8 *left = row + x * bpp;
      Uint8 *right = row + (s->w - 1 - x) * bpp;
      for (int b = 0; b < bpp; b++) {
        Uint8 tmp = left[b];
        left[b] = right[b];
        right[b] = tmp;
      }
    }
  }
  SDL_UnlockSurface(s);
}

void ImageResize(Image *image, int newWidth, int newHeight) {
  if (!image || !image->data)
    return;
  SDL_Surface *src = (SDL_Surface *)image->data;

  SDL_Surface *dst = SDL_CreateRGBSurface(
      src->flags, newWidth, newHeight, src->format->BitsPerPixel,
      src->format->Rmask, src->format->Gmask, src->format->Bmask,
      src->format->Amask);

  if (dst) {
    SDL_Rect sRect = {0, 0, (Uint16)src->w, (Uint16)src->h};
    SDL_Rect dRect = {0, 0, (Uint16)newWidth, (Uint16)newHeight};
    SDL_SoftStretch(src, &sRect, dst, &dRect);

    SDL_FreeSurface(src);
    image->data = (void *)dst;
    image->width = newWidth;
    image->height = newHeight;
  }
}

Color *LoadImageColors(Image image) {
  if (!image.data)
    return NULL;
  SDL_Surface *s = (SDL_Surface *)image.data;
  int count = s->w * s->h;
  Color *colors = (Color *)malloc(count * sizeof(Color));
  if (!colors)
    return NULL;

  SDL_LockSurface(s);
  for (int y = 0; y < s->h; y++) {
    for (int x = 0; x < s->w; x++) {
      int bpp = s->format->BytesPerPixel;
      Uint8 *p = (Uint8 *)s->pixels + y * s->pitch + x * bpp;
      Uint32 pixel = 0;
      switch (bpp) {
      case 1:
        pixel = *p;
        break;
      case 2:
        pixel = *(Uint16 *)p;
        break;
      case 3:
        if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
          pixel = (Uint32)p[0] << 16 | (Uint32)p[1] << 8 | (Uint32)p[2];
        else
          pixel = (Uint32)p[0] | (Uint32)p[1] << 8 | (Uint32)p[2] << 16;
        break;
      case 4:
        pixel = *(Uint32 *)p;
        break;
      }
      Uint8 r, g, b, a;
      SDL_GetRGBA(pixel, s->format, &r, &g, &b, &a);
      int idx = y * s->w + x;
      colors[idx].r = r;
      colors[idx].g = g;
      colors[idx].b = b;
      colors[idx].a = a;
    }
  }
  SDL_UnlockSurface(s);
  return colors;
}

void UnloadImageColors(Color *colors) {
  if (colors)
    free(colors);
}

/* ======================================================================== */
/* Image-Text (renders text to an Image, not to screen)                    */
/* ======================================================================== */

Image ImageText(const char *text, int fontSize, Color color) {
  Image image;
  memset(&image, 0, sizeof(image));
  if (text == NULL || rl_defaultFont == NULL)
    return image;

  (void)fontSize;

  SDL_Color sdlColor = {color.r, color.g, color.b, 0};

  SDL_Surface *textSurf = TTF_RenderText_Solid(rl_defaultFont, text, sdlColor);
  if (textSurf) {
    SDL_Surface *optimized = SDL_DisplayFormatAlpha(textSurf);
    SDL_FreeSurface(textSurf);
    if (optimized) {
      image.data = (void *)optimized;
      image.width = optimized->w;
      image.height = optimized->h;
      image.format = 7;
      image.mipmaps = 1;
    }
  }
  return image;
}

Image ImageTextEx(Font font, const char *text, float fontSize, float spacing,
                  Color tint) {
  Image image;
  memset(&image, 0, sizeof(image));
  if (text == NULL)
    return image;

  (void)fontSize;
  (void)spacing;

  TTF_Font *ttf = (TTF_Font *)(uintptr_t)font.texture.id;
  if (ttf == NULL)
    ttf = rl_defaultFont;
  if (ttf == NULL)
    return image;

  SDL_Color sdlColor = {tint.r, tint.g, tint.b, 0};

  SDL_Surface *textSurf = TTF_RenderText_Solid(ttf, text, sdlColor);
  if (textSurf) {
    SDL_Surface *optimized = SDL_DisplayFormatAlpha(textSurf);
    SDL_FreeSurface(textSurf);
    if (optimized) {
      image.data = (void *)optimized;
      image.width = optimized->w;
      image.height = optimized->h;
      image.format = 7;
      image.mipmaps = 1;
    }
  }
  return image;
}

void ImageDrawText(Image *dst, const char *text, int posX, int posY,
                   int fontSize, Color color) {
  if (dst == NULL || dst->data == NULL)
    return;

  Image textImg = ImageText(text, fontSize, color);
  if (textImg.data != NULL) {
    Rectangle srcRec = {0, 0, (float)textImg.width, (float)textImg.height};
    Rectangle dstRec = {(float)posX, (float)posY, (float)textImg.width,
                        (float)textImg.height};
    ImageDraw(dst, textImg, srcRec, dstRec, WHITE);
    UnloadImage(textImg);
  }
}

/* ======================================================================== */
/* Texture Management (Fake "VRAM" — still CPU surfaces)                   */
/* ======================================================================== */

Texture2D LoadTexture(const char *fileName) {
  Texture2D tex;
  memset(&tex, 0, sizeof(tex));

  SDL_Surface *loaded = IMG_Load(fileName);
  if (loaded) {
    SDL_Surface *optimized = SDL_DisplayFormatAlpha(loaded);
    SDL_FreeSurface(loaded);
    if (optimized) {
      tex.id = (unsigned int)(uintptr_t)optimized;
      tex.width = optimized->w;
      tex.height = optimized->h;
      tex.mipmaps = 1;
    }
  } else {
    fprintf(stderr, "WARNING: Failed to load texture %s: %s\n", fileName,
            IMG_GetError());
  }
  return tex;
}

Texture2D LoadTextureFromImage(Image image) {
  Texture2D tex;
  memset(&tex, 0, sizeof(tex));
  if (image.data == NULL)
    return tex;

  SDL_Surface *original = (SDL_Surface *)image.data;
  SDL_Surface *optimized = SDL_DisplayFormatAlpha(original);
  if (optimized) {
    tex.id = (unsigned int)(uintptr_t)optimized;
    tex.width = optimized->w;
    tex.height = optimized->h;
    tex.mipmaps = 1;
  }
  return tex;
}

void UnloadTexture(Texture2D texture) {
  if (texture.id != 0) {
    SDL_FreeSurface((SDL_Surface *)(uintptr_t)texture.id);
  }
}

bool IsTextureValid(Texture2D texture) { return (texture.id != 0); }

void UpdateTexture(Texture2D texture, const void *pixels) {
  if (texture.id == 0)
    return;
  SDL_Surface *surf = (SDL_Surface *)(uintptr_t)texture.id;

  SDL_LockSurface(surf);
  memcpy(surf->pixels, pixels, surf->pitch * surf->h);
  SDL_UnlockSurface(surf);
}

/* ======================================================================== */
/* Texture Drawing                                                         */
/* ======================================================================== */

void DrawTexture(Texture2D texture, int posX, int posY, Color tint) {
  if (texture.id == 0 || rl_activeSurface == NULL)
    return;
  (void)tint;

  SDL_Surface *src = (SDL_Surface *)(uintptr_t)texture.id;
  SDL_Rect destRect = {(Sint16)posX, (Sint16)posY, 0, 0};

  SDL_BlitSurface(src, NULL, rl_activeSurface, &destRect);
}

void DrawTextureV(Texture2D texture, Vector2 position, Color tint) {
  DrawTexture(texture, (int)position.x, (int)position.y, tint);
}

void DrawTextureRec(Texture2D texture, Rectangle source, Vector2 position,
                    Color tint) {
  if (texture.id == 0 || rl_activeSurface == NULL)
    return;
  (void)tint;

  SDL_Surface *srcSurface = (SDL_Surface *)(uintptr_t)texture.id;

  SDL_Rect srcRect = {(Sint16)source.x, (Sint16)source.y, (Uint16)source.width,
                      (Uint16)source.height};
  SDL_Rect destRect = {(Sint16)position.x, (Sint16)position.y, 0, 0};

  SDL_BlitSurface(srcSurface, &srcRect, rl_activeSurface, &destRect);
}

/* Custom Software Additive Blitter */
static void SoftwareBlitAdditive(SDL_Surface *src, SDL_Rect *sRect,
                                 SDL_Surface *dst, SDL_Rect *dRect) {
  /* Failsafes */
  if (!src || !dst || !sRect || !dRect)
    return;

  /* Lock surfaces for direct memory access */
  if (SDL_MUSTLOCK(src))
    SDL_LockSurface(src);
  if (SDL_MUSTLOCK(dst))
    SDL_LockSurface(dst);

  int srcBpp = src->format->BytesPerPixel;
  int dstBpp = dst->format->BytesPerPixel;

  /* Loop through every pixel in the rectangle */
  for (int y = 0; y < sRect->h && (dRect->y + y) < dst->h; y++) {
    for (int x = 0; x < sRect->w && (dRect->x + x) < dst->w; x++) {

      /* Get source pixel */
      Uint8 *sPixel = (Uint8 *)src->pixels + ((sRect->y + y) * src->pitch) +
                      ((sRect->x + x) * srcBpp);
      Uint32 sColor = (srcBpp == 4)
                          ? *(Uint32 *)sPixel
                          : 0; /* Assuming 32-bit textures for simplicity */

      Uint8 sr, sg, sb, sa;
      SDL_GetRGBA(sColor, src->format, &sr, &sg, &sb, &sa);

      /* Skip fully transparent pixels for performance */
      if (sa == 0)
        continue;

      /* Get destination pixel */
      Uint8 *dPixel = (Uint8 *)dst->pixels + ((dRect->y + y) * dst->pitch) +
                      ((dRect->x + x) * dstBpp);
      Uint32 dColor = (dstBpp == 4) ? *(Uint32 *)dPixel : 0;

      Uint8 dr, dg, db, da;
      SDL_GetRGBA(dColor, dst->format, &dr, &dg, &db, &da);

      /* Additive Math: Add source and destination RGB, factoring in source
       * Alpha */
      float alphaMult = (float)sa / 255.0f;
      int r = dr + (int)(sr * alphaMult);
      int g = dg + (int)(sg * alphaMult);
      int b = db + (int)(sb * alphaMult);

      /* Clamp to 255 to prevent overflow (which causes trippy color wrapping)
       */
      if (r > 255)
        r = 255;
      if (g > 255)
        g = 255;
      if (b > 255)
        b = 255;

      /* Write back to destination */
      Uint32 finalColor =
          SDL_MapRGBA(dst->format, (Uint8)r, (Uint8)g, (Uint8)b, da);
      if (dstBpp == 4)
        *(Uint32 *)dPixel = finalColor;
    }
  }

  if (SDL_MUSTLOCK(dst))
    SDL_UnlockSurface(dst);
  if (SDL_MUSTLOCK(src))
    SDL_UnlockSurface(src);
}

void DrawTexturePro(Texture2D texture, Rectangle source, Rectangle dest,
                    Vector2 origin, float rotation, Color tint) {
  if (texture.id == 0 || !rl_activeSurface)
    return;
  (void)rotation; /* SDL 1.2 can't rotate without SDL_gfx */
  (void)tint;

  SDL_Surface *srcSurf = (SDL_Surface *)(uintptr_t)texture.id;

  SDL_Rect sRect = {(Sint16)source.x, (Sint16)source.y, (Uint16)source.width,
                    (Uint16)source.height};
  SDL_Rect dRect = {(Sint16)(dest.x - origin.x), (Sint16)(dest.y - origin.y),
                    (Uint16)dest.width, (Uint16)dest.height};

  /* Intercept for Custom Blend Modes */
  if (rl_currentBlendMode == BLEND_ADDITIVE) {
    SoftwareBlitAdditive(srcSurf, &sRect, rl_activeSurface, &dRect);
    return;
  }

  /* Default BLEND_ALPHA behavior */
  if ((int)source.width == (int)dest.width &&
      (int)source.height == (int)dest.height) {
    SDL_BlitSurface(srcSurf, &sRect, rl_activeSurface, &dRect);
  } else {
    SDL_SoftStretch(srcSurf, &sRect, rl_activeSurface, &dRect);
  }
}

/* ======================================================================== */
/* RenderTexture Management (Off-screen rendering)                          */
/* ======================================================================== */

RenderTexture2D LoadRenderTexture(int width, int height) {
  RenderTexture2D target;
  memset(&target, 0, sizeof(target));

  /* Create an empty 32-bit RGBA surface to act as our framebuffer */
  SDL_Surface *surf =
      SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, 32, 0x00FF0000,
                           0x0000FF00, 0x000000FF, 0xFF000000);
  if (surf) {
    target.id = (unsigned int)(uintptr_t)surf;

    /* Populate the internal texture struct so it can be drawn with
     * DrawTexture() */
    target.texture.id = target.id;
    target.texture.width = width;
    target.texture.height = height;
    target.texture.mipmaps = 1;
    target.texture.format = 7;
  }
  return target;
}

void UnloadRenderTexture(RenderTexture2D target) {
  if (target.id != 0) {
    SDL_FreeSurface((SDL_Surface *)(uintptr_t)target.id);
  }
}

void BeginTextureMode(RenderTexture2D target) {
  if (target.id != 0) {
    /* Swap the global drawing target to our custom surface */
    rl_activeSurface = (SDL_Surface *)(uintptr_t)target.id;
  }
}

void EndTextureMode(void) {
  /* Restore the global drawing target back to the main window screen */
  rl_activeSurface = rl_screen;
}

/* ======================================================================== */
/* 9-Slice / NPatch Drawing                                                 */
/* ======================================================================== */

void DrawTextureNPatch(Texture2D texture, NPatchInfo nPatchInfo, Rectangle dest,
                       Vector2 origin, float rotation, Color tint) {
  if (texture.id == 0 || !rl_activeSurface)
    return;
  (void)rotation; /* SDL 1.2 software rendering cannot rotate */
  (void)tint;     /* No software tinting for perf */

  SDL_Surface *srcSurf = (SDL_Surface *)(uintptr_t)texture.id;

  /* Base destination coordinates (adjusted for origin) */
  float dx = dest.x - origin.x;
  float dy = dest.y - origin.y;
  float dw = dest.width;
  float dh = dest.height;

  /* Source rectangle parameters */
  float sx = nPatchInfo.source.x;
  float sy = nPatchInfo.source.y;
  float sw = nPatchInfo.source.width;
  float sh = nPatchInfo.source.height;

  int left = nPatchInfo.left;
  int top = nPatchInfo.top;
  int right = nPatchInfo.right;
  int bottom = nPatchInfo.bottom;

  /* Override margins based on layout type */
  if (nPatchInfo.layout == NPATCH_THREE_PATCH_HORIZONTAL) {
    top = 0;
    bottom = 0;
  } else if (nPatchInfo.layout == NPATCH_THREE_PATCH_VERTICAL) {
    left = 0;
    right = 0;
  }

  /* Center dimensions */
  int srcCenterW = (int)sw - left - right;
  int srcCenterH = (int)sh - top - bottom;
  int dstCenterW = (int)dw - left - right;
  int dstCenterH = (int)dh - top - bottom;

  /* Failsafe: if the destination is too small for the corners, just stretch the
   * whole thing */
  if (dstCenterW < 0 || dstCenterH < 0) {
    SDL_Rect sR = {(Sint16)sx, (Sint16)sy, (Uint16)sw, (Uint16)sh};
    SDL_Rect dR = {(Sint16)dx, (Sint16)dy, (Uint16)dw, (Uint16)dh};
    SDL_SoftStretch(srcSurf, &sR, rl_activeSurface, &dR);
    return;
  }

  /* Define the 9 Source Rectangles */
  SDL_Rect sRects[9] = {
      {(Sint16)sx, (Sint16)sy, (Uint16)left, (Uint16)top}, /* Top-Left */
      {(Sint16)(sx + left), (Sint16)sy, (Uint16)srcCenterW,
       (Uint16)top}, /* Top-Center */
      {(Sint16)(sx + left + srcCenterW), (Sint16)sy, (Uint16)right,
       (Uint16)top}, /* Top-Right */
      {(Sint16)sx, (Sint16)(sy + top), (Uint16)left,
       (Uint16)srcCenterH}, /* Mid-Left */
      {(Sint16)(sx + left), (Sint16)(sy + top), (Uint16)srcCenterW,
       (Uint16)srcCenterH}, /* Mid-Center */
      {(Sint16)(sx + left + srcCenterW), (Sint16)(sy + top), (Uint16)right,
       (Uint16)srcCenterH}, /* Mid-Right */
      {(Sint16)sx, (Sint16)(sy + top + srcCenterH), (Uint16)left,
       (Uint16)bottom}, /* Bot-Left */
      {(Sint16)(sx + left), (Sint16)(sy + top + srcCenterH), (Uint16)srcCenterW,
       (Uint16)bottom}, /* Bot-Center */
      {(Sint16)(sx + left + srcCenterW), (Sint16)(sy + top + srcCenterH),
       (Uint16)right, (Uint16)bottom} /* Bot-Right */
  };

  /* Define the 9 Destination Rectangles */
  SDL_Rect dRects[9] = {
      {(Sint16)dx, (Sint16)dy, (Uint16)left, (Uint16)top}, /* Top-Left */
      {(Sint16)(dx + left), (Sint16)dy, (Uint16)dstCenterW,
       (Uint16)top}, /* Top-Center */
      {(Sint16)(dx + left + dstCenterW), (Sint16)dy, (Uint16)right,
       (Uint16)top}, /* Top-Right */
      {(Sint16)dx, (Sint16)(dy + top), (Uint16)left,
       (Uint16)dstCenterH}, /* Mid-Left */
      {(Sint16)(dx + left), (Sint16)(dy + top), (Uint16)dstCenterW,
       (Uint16)dstCenterH}, /* Mid-Center */
      {(Sint16)(dx + left + dstCenterW), (Sint16)(dy + top), (Uint16)right,
       (Uint16)dstCenterH}, /* Mid-Right */
      {(Sint16)dx, (Sint16)(dy + top + dstCenterH), (Uint16)left,
       (Uint16)bottom}, /* Bot-Left */
      {(Sint16)(dx + left), (Sint16)(dy + top + dstCenterH), (Uint16)dstCenterW,
       (Uint16)bottom}, /* Bot-Center */
      {(Sint16)(dx + left + dstCenterW), (Sint16)(dy + top + dstCenterH),
       (Uint16)right, (Uint16)bottom} /* Bot-Right */
  };

  /* Draw all 9 patches */
  for (int i = 0; i < 9; i++) {
    /* Skip rendering if this patch has no width or height */
    if (sRects[i].w == 0 || sRects[i].h == 0 || dRects[i].w == 0 ||
        dRects[i].h == 0) {
      continue;
    }

    /* Fast blit for corners (no scaling), soft stretch for edges/center */
    if (sRects[i].w == dRects[i].w && sRects[i].h == dRects[i].h) {
      SDL_BlitSurface(srcSurf, &sRects[i], rl_activeSurface, &dRects[i]);
    } else {
      SDL_SoftStretch(srcSurf, &sRects[i], rl_activeSurface, &dRects[i]);
    }
  }
}