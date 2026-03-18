/*
 * shapes.cpp — Primitive drawing (pixel, line, rect, circle, triangle, poly)
 * and collision detection.
 *
 * Fully supports Camera2D transformations (translation, rotation, zoom)
 * rendered entirely via software rasterization.
 */

#include "raylib.h"

#include <SDL.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

/* From core.cpp */
extern SDL_Surface *rl_activeSurface;
extern bool rl_IsCamera2DActive(void);
extern Camera2D rl_GetCamera2D(void);
extern Vector2 GetWorldToScreen2D(Vector2 position, Camera2D camera);

/* ======================================================================== */
/* Internal Software Rasterizers (Camera-Agnostic)                          */
/* ======================================================================== */

static void PutPixel(SDL_Surface *surface, int x, int y, Uint32 pixel) {
  if (x < 0 || y < 0 || x >= surface->w || y >= surface->h)
    return;

  int bpp = surface->format->BytesPerPixel;
  Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

  switch (bpp) {
  case 1:
    *p = (Uint8)pixel;
    break;
  case 2:
    *(Uint16 *)p = (Uint16)pixel;
    break;
  case 3:
    if (SDL_BYTEORDER == SDL_BIG_ENDIAN) {
      p[0] = (Uint8)((pixel >> 16) & 0xFF);
      p[1] = (Uint8)((pixel >> 8) & 0xFF);
      p[2] = (Uint8)(pixel & 0xFF);
    } else {
      p[0] = (Uint8)(pixel & 0xFF);
      p[1] = (Uint8)((pixel >> 8) & 0xFF);
      p[2] = (Uint8)((pixel >> 16) & 0xFF);
    }
    break;
  case 4:
    *(Uint32 *)p = pixel;
    break;
  }
}

static void RasterizeTriangle(Vector2 v1, Vector2 v2, Vector2 v3, Uint32 col) {
  int minX = (int)fminf(v1.x, fminf(v2.x, v3.x));
  int maxX = (int)fmaxf(v1.x, fmaxf(v2.x, v3.x));
  int minY = (int)fminf(v1.y, fminf(v2.y, v3.y));
  int maxY = (int)fmaxf(v1.y, fmaxf(v2.y, v3.y));

  /* Clamp to screen to avoid rendering off-screen memory */
  if (minX < 0)
    minX = 0;
  if (minY < 0)
    minY = 0;
  if (maxX >= rl_activeSurface->w)
    maxX = rl_activeSurface->w - 1;
  if (maxY >= rl_activeSurface->h)
    maxY = rl_activeSurface->h - 1;

  if (SDL_MUSTLOCK(rl_activeSurface)) {
    if (SDL_LockSurface(rl_activeSurface) < 0)
      return;
  }

  for (int y = minY; y <= maxY; y++) {
    for (int x = minX; x <= maxX; x++) {
      float fx = (float)x;
      float fy = (float)y;

      /* Half-space / edge function test */
      float w0 = (v2.x - v1.x) * (fy - v1.y) - (v2.y - v1.y) * (fx - v1.x);
      float w1 = (v3.x - v2.x) * (fy - v2.y) - (v3.y - v2.y) * (fx - v2.x);
      float w2 = (v1.x - v3.x) * (fy - v3.y) - (v1.y - v3.y) * (fx - v3.x);

      if ((w0 >= 0 && w1 >= 0 && w2 >= 0) || (w0 <= 0 && w1 <= 0 && w2 <= 0)) {
        PutPixel(rl_activeSurface, x, y, col);
      }
    }
  }

  if (SDL_MUSTLOCK(rl_activeSurface))
    SDL_UnlockSurface(rl_activeSurface);
}

/* ======================================================================== */
/* Pixel                                                                    */
/* ======================================================================== */

void DrawPixel(int posX, int posY, Color color) {
  DrawPixelV((Vector2){(float)posX, (float)posY}, color);
}

void DrawPixelV(Vector2 position, Color color) {
  if (!rl_activeSurface)
    return;

  if (rl_IsCamera2DActive()) {
    position = GetWorldToScreen2D(position, rl_GetCamera2D());
  }

  Uint32 mapped = SDL_MapRGB(rl_activeSurface->format, color.r, color.g, color.b);

  if (SDL_MUSTLOCK(rl_activeSurface)) {
    if (SDL_LockSurface(rl_activeSurface) < 0)
      return;
  }
  PutPixel(rl_activeSurface, (int)position.x, (int)position.y, mapped);
  if (SDL_MUSTLOCK(rl_activeSurface))
    SDL_UnlockSurface(rl_activeSurface);
}

/* ======================================================================== */
/* Line (Bresenham)                                                         */
/* ======================================================================== */

void DrawLine(int startPosX, int startPosY, int endPosX, int endPosY,
              Color color) {
  DrawLineV((Vector2){(float)startPosX, (float)startPosY},
            (Vector2){(float)endPosX, (float)endPosY}, color);
}

void DrawLineV(Vector2 startPos, Vector2 endPos, Color color) {
  if (!rl_activeSurface)
    return;

  if (rl_IsCamera2DActive()) {
    Camera2D cam = rl_GetCamera2D();
    startPos = GetWorldToScreen2D(startPos, cam);
    endPos = GetWorldToScreen2D(endPos, cam);
  }

  Uint32 mapped = SDL_MapRGB(rl_activeSurface->format, color.r, color.g, color.b);

  int x0 = (int)startPos.x;
  int y0 = (int)startPos.y;
  int x1 = (int)endPos.x;
  int y1 = (int)endPos.y;

  int dx = abs(x1 - x0);
  int dy = abs(y1 - y0);
  int sx = (x0 < x1) ? 1 : -1;
  int sy = (y0 < y1) ? 1 : -1;
  int err = dx - dy;

  if (SDL_MUSTLOCK(rl_activeSurface)) {
    if (SDL_LockSurface(rl_activeSurface) < 0)
      return;
  }

  for (;;) {
    PutPixel(rl_activeSurface, x0, y0, mapped);
    if (x0 == x1 && y0 == y1)
      break;
    int e2 = 2 * err;
    if (e2 > -dy) {
      err -= dy;
      x0 += sx;
    }
    if (e2 < dx) {
      err += dx;
      y0 += sy;
    }
  }

  if (SDL_MUSTLOCK(rl_activeSurface))
    SDL_UnlockSurface(rl_activeSurface);
}

void DrawLineEx(Vector2 startPos, Vector2 endPos, float thick, Color color) {
  if (thick <= 1.0f) {
    DrawLineV(startPos, endPos, color);
    return;
  }

  /* Compute perpendicular offset for thickness in World Space */
  float dx = endPos.x - startPos.x;
  float dy = endPos.y - startPos.y;
  float len = sqrtf(dx * dx + dy * dy);
  if (len < 0.001f)
    return;

  float nx = -dy / len * thick * 0.5f;
  float ny = dx / len * thick * 0.5f;

  Vector2 v1 = {startPos.x + nx, startPos.y + ny};
  Vector2 v2 = {startPos.x - nx, startPos.y - ny};
  Vector2 v3 = {endPos.x - nx, endPos.y - ny};
  Vector2 v4 = {endPos.x + nx, endPos.y + ny};

  /* DrawTriangle inherently handles the Camera translation/zoom for us */
  DrawTriangle(v1, v2, v3, color);
  DrawTriangle(v1, v3, v4, color);
}

/* ======================================================================== */
/* Rectangle                                                                */
/* ======================================================================== */

void DrawRectangle(int posX, int posY, int width, int height, Color color) {
  if (!rl_activeSurface)
    return;
  Uint32 mapped = SDL_MapRGB(rl_activeSurface->format, color.r, color.g, color.b);

  if (rl_IsCamera2DActive()) {
    Camera2D cam = rl_GetCamera2D();

    /* If camera is rotated, rectangle becomes a polygon. Draw via triangles. */
    if (cam.rotation != 0.0f) {
      Vector2 tl = GetWorldToScreen2D((Vector2){(float)posX, (float)posY}, cam);
      Vector2 tr =
          GetWorldToScreen2D((Vector2){(float)posX + width, (float)posY}, cam);
      Vector2 bl =
          GetWorldToScreen2D((Vector2){(float)posX, (float)posY + height}, cam);
      Vector2 br = GetWorldToScreen2D(
          (Vector2){(float)posX + width, (float)posY + height}, cam);

      RasterizeTriangle(tl, bl, tr, mapped);
      RasterizeTriangle(tr, bl, br, mapped);
      return;
    }

    /* Fast path for non-rotated camera */
    Vector2 screenPos =
        GetWorldToScreen2D((Vector2){(float)posX, (float)posY}, cam);
    SDL_Rect rect = {(Sint16)screenPos.x, (Sint16)screenPos.y,
                     (Uint16)(width * cam.zoom), (Uint16)(height * cam.zoom)};
    SDL_FillRect(rl_activeSurface, &rect, mapped);
  } else {
    /* Fast path: No camera */
    SDL_Rect rect = {(Sint16)posX, (Sint16)posY, (Uint16)width, (Uint16)height};
    SDL_FillRect(rl_activeSurface, &rect, mapped);
  }
}

void DrawRectangleV(Vector2 position, Vector2 size, Color color) {
  DrawRectangle((int)position.x, (int)position.y, (int)size.x, (int)size.y,
                color);
}

void DrawRectangleRec(Rectangle rec, Color color) {
  DrawRectangle((int)rec.x, (int)rec.y, (int)rec.width, (int)rec.height, color);
}

void DrawRectangleLines(int posX, int posY, int width, int height,
                        Color color) {
  /* Passing to DrawRectangle automatically handles Camera Transforms */
  DrawRectangle(posX, posY, width, 1, color);              /* Top    */
  DrawRectangle(posX, posY + height - 1, width, 1, color); /* Bottom */
  DrawRectangle(posX, posY, 1, height, color);             /* Left   */
  DrawRectangle(posX + width - 1, posY, 1, height, color); /* Right  */
}

void DrawRectangleLinesEx(Rectangle rec, float lineThick, Color color) {
  int t = (int)lineThick;
  if (t < 1)
    t = 1;

  int x = (int)rec.x;
  int y = (int)rec.y;
  int w = (int)rec.width;
  int h = (int)rec.height;

  DrawRectangle(x, y, w, t, color);                     /* Top    */
  DrawRectangle(x, y + h - t, w, t, color);             /* Bottom */
  DrawRectangle(x, y + t, t, h - 2 * t, color);         /* Left   */
  DrawRectangle(x + w - t, y + t, t, h - 2 * t, color); /* Right  */
}

void DrawRectangleGradientV(int posX, int posY, int width, int height,
                            Color top, Color bottom) {
  if (height <= 0)
    return;
  for (int y = 0; y < height; y++) {
    float t = (float)y / (float)height;
    Color c;
    c.r = (unsigned char)(top.r + (bottom.r - top.r) * t);
    c.g = (unsigned char)(top.g + (bottom.g - top.g) * t);
    c.b = (unsigned char)(top.b + (bottom.b - top.b) * t);
    c.a = 255;
    DrawRectangle(posX, posY + y, width, 1, c);
  }
}

void DrawRectangleGradientH(int posX, int posY, int width, int height,
                            Color left, Color right) {
  if (width <= 0)
    return;
  for (int x = 0; x < width; x++) {
    float t = (float)x / (float)width;
    Color c;
    c.r = (unsigned char)(left.r + (right.r - left.r) * t);
    c.g = (unsigned char)(left.g + (right.g - left.g) * t);
    c.b = (unsigned char)(left.b + (right.b - left.b) * t);
    c.a = 255;
    DrawRectangle(posX + x, posY, 1, height, c);
  }
}

void DrawRectangleRounded(Rectangle rec, float roundness, int segments,
                          Color color) {
  (void)roundness;
  (void)segments;
  DrawRectangleRec(rec, color); /* Fallback: no rounding on SDL 1.2 */
}

/* ======================================================================== */
/* Circle (Midpoint algorithm)                                              */
/* ======================================================================== */

void DrawCircle(int centerX, int centerY, float radius, Color color) {
  DrawCircleV((Vector2){(float)centerX, (float)centerY}, radius, color);
}

void DrawCircleV(Vector2 center, float radius, Color color) {
  if (!rl_activeSurface)
    return;

  if (rl_IsCamera2DActive()) {
    Camera2D cam = rl_GetCamera2D();
    center = GetWorldToScreen2D(center, cam);
    radius *= cam.zoom;
  }

  Uint32 mapped = SDL_MapRGB(rl_activeSurface->format, color.r, color.g, color.b);
  int r = (int)radius;
  int cx = (int)center.x;
  int cy = (int)center.y;

  if (SDL_MUSTLOCK(rl_activeSurface)) {
    if (SDL_LockSurface(rl_activeSurface) < 0)
      return;
  }

  int x = 0;
  int y = r;
  int d = 1 - r;

  while (x <= y) {
    for (int i = cx - y; i <= cx + y; i++) {
      PutPixel(rl_activeSurface, i, cy + x, mapped);
      PutPixel(rl_activeSurface, i, cy - x, mapped);
    }
    for (int i = cx - x; i <= cx + x; i++) {
      PutPixel(rl_activeSurface, i, cy + y, mapped);
      PutPixel(rl_activeSurface, i, cy - y, mapped);
    }

    if (d < 0) {
      d += 2 * x + 3;
    } else {
      d += 2 * (x - y) + 5;
      y--;
    }
    x++;
  }

  if (SDL_MUSTLOCK(rl_activeSurface))
    SDL_UnlockSurface(rl_activeSurface);
}

void DrawCircleLines(int centerX, int centerY, float radius, Color color) {
  if (!rl_activeSurface)
    return;

  Vector2 center = {(float)centerX, (float)centerY};
  if (rl_IsCamera2DActive()) {
    Camera2D cam = rl_GetCamera2D();
    center = GetWorldToScreen2D(center, cam);
    radius *= cam.zoom;
  }

  Uint32 mapped = SDL_MapRGB(rl_activeSurface->format, color.r, color.g, color.b);
  int r = (int)radius;
  int cx = (int)center.x;
  int cy = (int)center.y;

  int x = 0;
  int y = r;
  int d = 1 - r;

  if (SDL_MUSTLOCK(rl_activeSurface)) {
    if (SDL_LockSurface(rl_activeSurface) < 0)
      return;
  }

  while (x <= y) {
    PutPixel(rl_activeSurface, cx + x, cy + y, mapped);
    PutPixel(rl_activeSurface, cx - x, cy + y, mapped);
    PutPixel(rl_activeSurface, cx + x, cy - y, mapped);
    PutPixel(rl_activeSurface, cx - x, cy - y, mapped);
    PutPixel(rl_activeSurface, cx + y, cy + x, mapped);
    PutPixel(rl_activeSurface, cx - y, cy + x, mapped);
    PutPixel(rl_activeSurface, cx + y, cy - x, mapped);
    PutPixel(rl_activeSurface, cx - y, cy - x, mapped);

    if (d < 0) {
      d += 2 * x + 3;
    } else {
      d += 2 * (x - y) + 5;
      y--;
    }
    x++;
  }

  if (SDL_MUSTLOCK(rl_activeSurface))
    SDL_UnlockSurface(rl_activeSurface);
}

/* ======================================================================== */
/* Triangle                                                                 */
/* ======================================================================== */

void DrawTriangle(Vector2 v1, Vector2 v2, Vector2 v3, Color color) {
  if (!rl_activeSurface)
    return;

  if (rl_IsCamera2DActive()) {
    Camera2D cam = rl_GetCamera2D();
    v1 = GetWorldToScreen2D(v1, cam);
    v2 = GetWorldToScreen2D(v2, cam);
    v3 = GetWorldToScreen2D(v3, cam);
  }

  Uint32 col = SDL_MapRGB(rl_activeSurface->format, color.r, color.g, color.b);
  RasterizeTriangle(v1, v2, v3, col);
}

void DrawTriangleLines(Vector2 v1, Vector2 v2, Vector2 v3, Color color) {
  /* DrawLineV handles the camera transformations */
  DrawLineV(v1, v2, color);
  DrawLineV(v2, v3, color);
  DrawLineV(v3, v1, color);
}

/* ======================================================================== */
/* Polygon                                                                  */
/* ======================================================================== */

void DrawPoly(Vector2 center, int sides, float radius, float rotation,
              Color color) {
  if (sides < 3)
    return;
  float step = 360.0f / (float)sides;

  for (int i = 0; i < sides; i++) {
    float angle1 = (rotation + i * step) * DEG2RAD;
    float angle2 = (rotation + (i + 1) * step) * DEG2RAD;

    Vector2 v2 = {center.x + cosf(angle1) * radius,
                  center.y + sinf(angle1) * radius};
    Vector2 v3 = {center.x + cosf(angle2) * radius,
                  center.y + sinf(angle2) * radius};

    /* DrawTriangle inherently handles the Camera translation/zoom for us */
    DrawTriangle(center, v2, v3, color);
  }
}

void DrawPolyLines(Vector2 center, int sides, float radius, float rotation,
                   Color color) {
  if (sides < 3)
    return;
  float step = 360.0f / (float)sides;

  for (int i = 0; i < sides; i++) {
    float angle1 = (rotation + i * step) * DEG2RAD;
    float angle2 = (rotation + (i + 1) * step) * DEG2RAD;

    Vector2 v2 = {center.x + cosf(angle1) * radius,
                  center.y + sinf(angle1) * radius};
    Vector2 v3 = {center.x + cosf(angle2) * radius,
                  center.y + sinf(angle2) * radius};

    /* DrawLineV inherently handles the Camera translation/zoom for us */
    DrawLineV(v2, v3, color);
  }
}

/* ======================================================================== */
/* Splines                                                                  */
/* ======================================================================== */

Vector2 GetSplinePointLinear(Vector2 startPos, Vector2 endPos, float t) {
  Vector2 result;
  result.x = startPos.x + (endPos.x - startPos.x) * t;
  result.y = startPos.y + (endPos.y - startPos.y) * t;
  return result;
}

void DrawSplineLinear(const Vector2 *points, int pointCount, float thick,
                      Color color) {
  if (points == NULL || pointCount < 2)
    return;
  for (int i = 0; i < pointCount - 1; i++) {
    DrawLineEx(points[i], points[i + 1], thick, color);
  }
}

void DrawSplineBezierCubic(const Vector2 *points, int pointCount, float thick,
                           Color color) {
  /* Stub: cubic Bezier requires groups of 4 control points.
     Falling back to straight segments for the software renderer */
  DrawSplineLinear(points, pointCount, thick, color);
}

/* ======================================================================== */
/* Collision Detection (Always evaluated in World Space)                    */
/* ======================================================================== */

bool CheckCollisionRecs(Rectangle rec1, Rectangle rec2) {
  return (rec1.x < rec2.x + rec2.width) && (rec1.x + rec1.width > rec2.x) &&
         (rec1.y < rec2.y + rec2.height) && (rec1.y + rec1.height > rec2.y);
}

bool CheckCollisionCircles(Vector2 center1, float radius1, Vector2 center2,
                           float radius2) {
  float dx = center2.x - center1.x;
  float dy = center2.y - center1.y;
  float distSq = dx * dx + dy * dy;
  float radiusSum = radius1 + radius2;
  return (distSq <= radiusSum * radiusSum);
}

bool CheckCollisionCircleRec(Vector2 center, float radius, Rectangle rec) {
  float recCX = rec.x + rec.width * 0.5f;
  float recCY = rec.y + rec.height * 0.5f;

  float dx = fabsf(center.x - recCX);
  float dy = fabsf(center.y - recCY);

  if (dx > (rec.width * 0.5f + radius))
    return false;
  if (dy > (rec.height * 0.5f + radius))
    return false;
  if (dx <= rec.width * 0.5f)
    return true;
  if (dy <= rec.height * 0.5f)
    return true;

  float cornerDx = dx - rec.width * 0.5f;
  float cornerDy = dy - rec.height * 0.5f;
  return (cornerDx * cornerDx + cornerDy * cornerDy) <= (radius * radius);
}

bool CheckCollisionPointRec(Vector2 point, Rectangle rec) {
  return (point.x >= rec.x) && (point.x <= rec.x + rec.width) &&
         (point.y >= rec.y) && (point.y <= rec.y + rec.height);
}

bool CheckCollisionPointCircle(Vector2 point, Vector2 center, float radius) {
  return CheckCollisionCircles(point, 0, center, radius);
}

static float TriangleSign(Vector2 p1, Vector2 p2, Vector2 p3) {
  return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
}

bool CheckCollisionPointTriangle(Vector2 point, Vector2 p1, Vector2 p2,
                                 Vector2 p3) {
  float d1 = TriangleSign(point, p1, p2);
  float d2 = TriangleSign(point, p2, p3);
  float d3 = TriangleSign(point, p3, p1);

  bool has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
  bool has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);

  return !(has_neg && has_pos);
}

Rectangle GetCollisionRec(Rectangle rec1, Rectangle rec2) {
  Rectangle overlap = {0, 0, 0, 0};
  if (CheckCollisionRecs(rec1, rec2)) {
    float x1 = fmaxf(rec1.x, rec2.x);
    float x2 = fminf(rec1.x + rec1.width, rec2.x + rec2.width);
    float y1 = fmaxf(rec1.y, rec2.y);
    float y2 = fminf(rec1.y + rec1.height, rec2.y + rec2.height);
    overlap.x = x1;
    overlap.y = y1;
    overlap.width = x2 - x1;
    overlap.height = y2 - y1;
  }
  return overlap;
}