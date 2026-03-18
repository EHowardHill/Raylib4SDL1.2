/*
 * core.cpp — Window management, frame timing, Camera2D, and utility functions.
 *
 * Owns the global `screen` surface and the frame clock.
 */

#include "raylib.h"

#include <SDL.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ---- Text subsystem hooks (text.cpp) ---- */
extern void InitTextSystem(void);
extern void CloseTextSystem(void);

/* ---- Input subsystem hooks (input.cpp) ---- */
extern void InitGamepads(void);
extern void CloseGamepads(void);
extern void UpdateGamepads(void);

/* ---- Input state (input.cpp owns these) ---- */
extern unsigned char rl_currentKeyState[];
extern unsigned char rl_previousKeyState[];
extern unsigned char rl_currentMouseState[];
extern unsigned char rl_previousMouseState[];
extern int rl_mouseX;
extern int rl_mouseY;
extern float rl_mouseWheelMove;
extern int rl_lastKeyPressed;
extern int rl_lastCharPressed;

/* ---- Globals owned by this translation unit ---- */
SDL_Surface *rl_screen = NULL; /* Visible to textures / shapes / text */
SDL_Surface *rl_activeSurface = NULL;

static int screenWidth = 0;
static int screenHeight = 0;
static bool shouldClose = false;
static int targetFPS = 60;
static Uint32 frameStart = 0;
static float frameDelta = 0.0f; /* Seconds elapsed last frame */
static Uint32 initTicks = 0;    /* SDL_GetTicks() at InitWindow */
static unsigned int configFlags = 0;
static bool cursorHidden = false;

/* ---- Camera2D state ---- */
static bool camera2DActive = false;
static Camera2D currentCamera2D = {{0, 0}, {0, 0}, 0.0f, 1.0f};

/* ======================================================================== */
/*  Window / System                                                         */
/* ======================================================================== */

void InitWindow(int width, int height, const char *title) {
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0) {
    fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
    return;
  }

  Uint32 flags = SDL_SWSURFACE;
  if (configFlags & FLAG_FULLSCREEN_MODE)
    flags |= SDL_FULLSCREEN;

  rl_screen = SDL_SetVideoMode(width, height, 32, flags);
  rl_activeSurface = rl_screen;

  if (!rl_screen) {
    fprintf(stderr, "SDL_SetVideoMode failed: %s\n", SDL_GetError());
    return;
  }

  SDL_WM_SetCaption(title, NULL);
  screenWidth = width;
  screenHeight = height;
  shouldClose = false;
  initTicks = SDL_GetTicks();

  /* Zero out input state */
  memset(rl_currentKeyState, 0, sizeof(unsigned char) * SDLK_LAST);
  memset(rl_previousKeyState, 0, sizeof(unsigned char) * SDLK_LAST);
  memset(rl_currentMouseState, 0, 8);
  memset(rl_previousMouseState, 0, 8);

  InitTextSystem();
  InitGamepads();
}

void CloseWindow(void) {
  CloseTextSystem();
  CloseGamepads();
  SDL_Quit();
  rl_screen = NULL;
}

bool WindowShouldClose(void) {
  /* 1. Save previous frame's input state */
  memcpy(rl_previousKeyState, rl_currentKeyState, SDLK_LAST);
  memcpy(rl_previousMouseState, rl_currentMouseState, 8);

  UpdateGamepads();

  /* Reset per-frame input values */
  rl_mouseWheelMove = 0.0f;
  rl_lastKeyPressed = 0;
  rl_lastCharPressed = 0;

  /* 2. Process all pending SDL events */
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    switch (event.type) {
    case SDL_QUIT:
      shouldClose = true;
      break;

    case SDL_KEYDOWN:
      rl_lastKeyPressed = event.key.keysym.sym;
      /* Store printable characters for GetCharPressed() */
      if (event.key.keysym.unicode > 0 && event.key.keysym.unicode < 128) {
        rl_lastCharPressed = (int)event.key.keysym.unicode;
      }
      break;

    case SDL_MOUSEBUTTONDOWN:
      if (event.button.button == SDL_BUTTON_WHEELUP)
        rl_mouseWheelMove = 1.0f;
      else if (event.button.button == SDL_BUTTON_WHEELDOWN)
        rl_mouseWheelMove = -1.0f;
      break;

    default:
      break;
    }
  }

  /* 3. Update current frame's state from SDL */
  Uint8 *keys = SDL_GetKeyState(NULL);
  memcpy(rl_currentKeyState, keys, SDLK_LAST);

  Uint8 mouseButtons = SDL_GetMouseState(&rl_mouseX, &rl_mouseY);
  rl_currentMouseState[1] =
      (mouseButtons & SDL_BUTTON(SDL_BUTTON_LEFT)) ? 1 : 0;
  rl_currentMouseState[2] =
      (mouseButtons & SDL_BUTTON(SDL_BUTTON_MIDDLE)) ? 1 : 0;
  rl_currentMouseState[3] =
      (mouseButtons & SDL_BUTTON(SDL_BUTTON_RIGHT)) ? 1 : 0;

  /* Enable Unicode translation for GetCharPressed */
  SDL_EnableUNICODE(1);

  return shouldClose;
}

void SetConfigFlags(unsigned int flags) { configFlags = flags; }

bool IsWindowReady(void) { return (rl_screen != NULL); }
bool IsWindowFullscreen(void) {
  return (rl_screen && (rl_screen->flags & SDL_FULLSCREEN));
}
bool IsWindowHidden(void) { return false; }
bool IsWindowMinimized(void) {
  return false; /* SDL 1.2 has no minimize query */
}
bool IsWindowMaximized(void) { return false; }
bool IsWindowFocused(void) { return true; /* Assume focused */ }
bool IsWindowResized(void) {
  return false; /* We don't support resize events */
}

void SetWindowTitle(const char *title) { SDL_WM_SetCaption(title, NULL); }
void SetWindowPosition(int x, int y) { /* Not supported in SDL 1.2 */ }
void SetWindowMinSize(int width, int height) { /* Not supported in SDL 1.2 */ }

void SetWindowSize(int width, int height) {
  rl_screen = SDL_SetVideoMode(width, height, 32, SDL_SWSURFACE);
  if (rl_screen) {
    screenWidth = width;
    screenHeight = height;
  }
}

int GetScreenWidth(void) { return screenWidth; }
int GetScreenHeight(void) { return screenHeight; }

void ToggleFullscreen(void) {
  /* SDL 1.2 requires re-creating the video surface */
  Uint32 flags = SDL_SWSURFACE;
  if (!(rl_screen->flags & SDL_FULLSCREEN))
    flags |= SDL_FULLSCREEN;

  rl_screen = SDL_SetVideoMode(screenWidth, screenHeight, 32, flags);
}

/* ======================================================================== */
/*  Timing                                                                  */
/* ======================================================================== */

void SetTargetFPS(int fps) { targetFPS = fps; }

int GetFPS(void) {
  if (frameDelta > 0.0f)
    return (int)(1.0f / frameDelta);
  return 0;
}

float GetFrameTime(void) { return frameDelta; }

double GetTime(void) { return (double)(SDL_GetTicks() - initTicks) / 1000.0; }

/* ======================================================================== */
/*  Drawing                                                                 */
/* ======================================================================== */

void BeginDrawing(void) { frameStart = SDL_GetTicks(); }

void EndDrawing(void) {
  SDL_Flip(rl_screen);

  if (targetFPS > 0) {
    Uint32 frameTime = SDL_GetTicks() - frameStart;
    Uint32 targetTime = 1000 / targetFPS;
    if (frameTime < targetTime) {
      SDL_Delay(targetTime - frameTime);
    }
  }

  Uint32 frameEnd = SDL_GetTicks();
  frameDelta = (float)(frameEnd - frameStart) / 1000.0f;
  if (frameDelta <= 0.0f)
    frameDelta = 0.001f; /* Avoid division by zero */
}

void ClearBackground(Color color) {
  if (!rl_screen)
    return;
  Uint32 mapped = SDL_MapRGB(rl_screen->format, color.r, color.g, color.b);
  SDL_FillRect(rl_screen, NULL, mapped);
}

/* ======================================================================== */
/*  Camera 2D                                                               */
/* ======================================================================== */

void BeginMode2D(Camera2D camera) {
  camera2DActive = true;
  currentCamera2D = camera;
  /* The actual transform is applied in shapes / texture draw calls
     by reading rl_GetCamera2D() — or we apply a global offset.
     For a minimal SDL 1.2 port, we just store the state; individual
     draw helpers can query it if needed. */
}

void EndMode2D(void) {
  camera2DActive = false;
  currentCamera2D.offset = (Vector2){0, 0};
  currentCamera2D.target = (Vector2){0, 0};
  currentCamera2D.rotation = 0.0f;
  currentCamera2D.zoom = 1.0f;
}

/* Accessor for other translation units that need Camera2D info */
bool rl_IsCamera2DActive(void) { return camera2DActive; }
Camera2D rl_GetCamera2D(void) { return currentCamera2D; }

/* ======================================================================== */
/*  Cursor                                                                  */
/* ======================================================================== */

void ShowCursor(void) {
  SDL_ShowCursor(SDL_ENABLE);
  cursorHidden = false;
}
void HideCursor(void) {
  SDL_ShowCursor(SDL_DISABLE);
  cursorHidden = true;
}
bool IsCursorHidden(void) { return cursorHidden; }

/* ======================================================================== */
/*  Misc Utilities                                                          */
/* ======================================================================== */

int GetRandomValue(int min, int max) {
  if (min > max) {
    int t = min;
    min = max;
    max = t;
  }
  return min + (rand() % (max - min + 1));
}

void TraceLog(int logLevel, const char *text, ...) {
  va_list args;
  va_start(args, text);
  vfprintf(stderr, text, args);
  fprintf(stderr, "\n");
  va_end(args);
}

/* Convert a World Space 2D position to Screen Space using a Camera2D */
Vector2 GetWorldToScreen2D(Vector2 position, Camera2D camera) {
  float x = position.x - camera.target.x;
  float y = position.y - camera.target.y;

  /* Apply Rotation if needed */
  if (camera.rotation != 0.0f) {
    float angle = camera.rotation * DEG2RAD;
    float cosRot = cosf(angle);
    float sinRot = sinf(angle);

    float rotX = x * cosRot - y * sinRot;
    float rotY = x * sinRot + y * cosRot;
    x = rotX;
    y = rotY;
  }

  /* Apply Zoom and Offset */
  x = (x * camera.zoom) + camera.offset.x;
  y = (y * camera.zoom) + camera.offset.y;

  return (Vector2){x, y};
}

/* ======================================================================== */
/* Scissor Mode (Clipping)                                                  */
/* ======================================================================== */

void BeginScissorMode(int x, int y, int width, int height) {
  if (!rl_activeSurface)
    return;

  SDL_Rect clipRect;
  clipRect.x = (Sint16)x;
  clipRect.y = (Sint16)y;
  clipRect.w = (Uint16)width;
  clipRect.h = (Uint16)height;

  SDL_SetClipRect(rl_activeSurface, &clipRect);
}

void EndScissorMode(void) {
  if (!rl_activeSurface)
    return;
  SDL_SetClipRect(rl_activeSurface, NULL); /* Passing NULL removes the clip */
}

/* ======================================================================== */
/* Blend Mode State                                                         */
/* ======================================================================== */

int rl_currentBlendMode = 0; /* Defaults to BLEND_ALPHA */

void BeginBlendMode(int mode) { rl_currentBlendMode = mode; }

void EndBlendMode(void) {
  rl_currentBlendMode = 0; /* Reset back to standard alpha blending */
}