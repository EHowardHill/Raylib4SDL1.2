/*
 * input.cpp — Keyboard and mouse input, with Raylib→SDL 1.2 key translation.
 */

#include "raylib.h"

#include <SDL.h>
#include <ctype.h>
#include <string.h>

/* ---- Global Input State (owned here, externed in core.cpp) ---- */
unsigned char rl_currentKeyState[SDLK_LAST];
unsigned char rl_previousKeyState[SDLK_LAST];

unsigned char rl_currentMouseState[8]; /* indices 1..3 used for L/M/R */
unsigned char rl_previousMouseState[8];

int rl_mouseX = 0;
int rl_mouseY = 0;
float rl_mouseWheelMove = 0.0f;
int rl_lastKeyPressed = 0;
int rl_lastCharPressed = 0;

/* ------------------------------------------------------------------ */
/*  Raylib Key → SDL 1.2 Key translation                             */
/* ------------------------------------------------------------------ */

static int MapRaylibKeyToSDL(int key) {
  /* Standard alphabet: Raylib uses uppercase ASCII, SDL 1.2 uses SDLK_a..z */
  if (key >= KEY_A && key <= (KEY_A + 25))
    return SDLK_a + (key - KEY_A);

  /* Digits and common punctuation share ASCII values */
  if (key >= 32 && key <= 126)
    return key;

  switch (key) {
  case KEY_ESCAPE:
    return SDLK_ESCAPE;
  case KEY_ENTER:
    return SDLK_RETURN;
  case KEY_TAB:
    return SDLK_TAB;
  case KEY_BACKSPACE:
    return SDLK_BACKSPACE;
  case KEY_INSERT:
    return SDLK_INSERT;
  case KEY_DELETE:
    return SDLK_DELETE;
  case KEY_RIGHT:
    return SDLK_RIGHT;
  case KEY_LEFT:
    return SDLK_LEFT;
  case KEY_DOWN:
    return SDLK_DOWN;
  case KEY_UP:
    return SDLK_UP;
  case KEY_PAGE_UP:
    return SDLK_PAGEUP;
  case KEY_PAGE_DOWN:
    return SDLK_PAGEDOWN;
  case KEY_HOME:
    return SDLK_HOME;
  case KEY_END:
    return SDLK_END;
  case KEY_CAPS_LOCK:
    return SDLK_CAPSLOCK;
  case KEY_SCROLL_LOCK:
    return SDLK_SCROLLOCK;
  case KEY_NUM_LOCK:
    return SDLK_NUMLOCK;
  case KEY_PRINT_SCREEN:
    return SDLK_PRINT;
  case KEY_PAUSE:
    return SDLK_PAUSE;
  case KEY_F1:
    return SDLK_F1;
  case KEY_F2:
    return SDLK_F2;
  case KEY_F3:
    return SDLK_F3;
  case KEY_F4:
    return SDLK_F4;
  case KEY_F5:
    return SDLK_F5;
  case KEY_F6:
    return SDLK_F6;
  case KEY_F7:
    return SDLK_F7;
  case KEY_F8:
    return SDLK_F8;
  case KEY_F9:
    return SDLK_F9;
  case KEY_F10:
    return SDLK_F10;
  case KEY_F11:
    return SDLK_F11;
  case KEY_F12:
    return SDLK_F12;
  case KEY_LEFT_SHIFT:
    return SDLK_LSHIFT;
  case KEY_LEFT_CONTROL:
    return SDLK_LCTRL;
  case KEY_LEFT_ALT:
    return SDLK_LALT;
  case KEY_LEFT_SUPER:
    return SDLK_LSUPER;
  case KEY_RIGHT_SHIFT:
    return SDLK_RSHIFT;
  case KEY_RIGHT_CONTROL:
    return SDLK_RCTRL;
  case KEY_RIGHT_ALT:
    return SDLK_RALT;
  case KEY_RIGHT_SUPER:
    return SDLK_RSUPER;
  default:
    return SDLK_UNKNOWN;
  }
}

static int MapRaylibMouseToSDL(int button) {
  switch (button) {
  case MOUSE_BUTTON_LEFT:
    return 1; /* SDL_BUTTON_LEFT   */
  case MOUSE_BUTTON_MIDDLE:
    return 2; /* SDL_BUTTON_MIDDLE */
  case MOUSE_BUTTON_RIGHT:
    return 3; /* SDL_BUTTON_RIGHT  */
  default:
    return 0;
  }
}

/* ------------------------------------------------------------------ */
/*  Keyboard                                                          */
/* ------------------------------------------------------------------ */

bool IsKeyDown(int key) {
  int sdlKey = MapRaylibKeyToSDL(key);
  if (sdlKey == SDLK_UNKNOWN)
    return false;
  return rl_currentKeyState[sdlKey] != 0;
}

bool IsKeyPressed(int key) {
  int sdlKey = MapRaylibKeyToSDL(key);
  if (sdlKey == SDLK_UNKNOWN)
    return false;
  return (rl_currentKeyState[sdlKey] != 0) &&
         (rl_previousKeyState[sdlKey] == 0);
}

bool IsKeyReleased(int key) {
  int sdlKey = MapRaylibKeyToSDL(key);
  if (sdlKey == SDLK_UNKNOWN)
    return false;
  return (rl_currentKeyState[sdlKey] == 0) &&
         (rl_previousKeyState[sdlKey] != 0);
}

bool IsKeyUp(int key) {
  int sdlKey = MapRaylibKeyToSDL(key);
  if (sdlKey == SDLK_UNKNOWN)
    return false;
  return rl_currentKeyState[sdlKey] == 0;
}

int GetKeyPressed(void) {
  int key = rl_lastKeyPressed;
  rl_lastKeyPressed = 0;
  return key;
}

int GetCharPressed(void) {
  int ch = rl_lastCharPressed;
  rl_lastCharPressed = 0;
  return ch;
}

/* ------------------------------------------------------------------ */
/*  Mouse                                                             */
/* ------------------------------------------------------------------ */

bool IsMouseButtonDown(int button) {
  int sdlButton = MapRaylibMouseToSDL(button);
  if (sdlButton == 0)
    return false;
  return rl_currentMouseState[sdlButton] != 0;
}

bool IsMouseButtonPressed(int button) {
  int sdlButton = MapRaylibMouseToSDL(button);
  if (sdlButton == 0)
    return false;
  return (rl_currentMouseState[sdlButton] != 0) &&
         (rl_previousMouseState[sdlButton] == 0);
}

bool IsMouseButtonReleased(int button) {
  int sdlButton = MapRaylibMouseToSDL(button);
  if (sdlButton == 0)
    return false;
  return (rl_currentMouseState[sdlButton] == 0) &&
         (rl_previousMouseState[sdlButton] != 0);
}

bool IsMouseButtonUp(int button) {
  int sdlButton = MapRaylibMouseToSDL(button);
  if (sdlButton == 0)
    return false;
  return rl_currentMouseState[sdlButton] == 0;
}

int GetMouseX(void) { return rl_mouseX; }
int GetMouseY(void) { return rl_mouseY; }

Vector2 GetMousePosition(void) {
  Vector2 pos = {(float)rl_mouseX, (float)rl_mouseY};
  return pos;
}

void SetMousePosition(int x, int y) {
  SDL_WarpMouse((Uint16)x, (Uint16)y);
  rl_mouseX = x;
  rl_mouseY = y;
}

float GetMouseWheelMove(void) {
  float move = rl_mouseWheelMove;
  /* Note: consumption happens in WindowShouldClose() now */
  return move;
}

/* ------------------------------------------------------------------ */
/* Gamepad / Joystick (SDL 1.2)                                       */
/* ------------------------------------------------------------------ */

#define MAX_GAMEPADS 4
#define MAX_GAMEPAD_BUTTONS 32

static SDL_Joystick *rl_joysticks[MAX_GAMEPADS] = {NULL};
static unsigned char rl_currentGamepadState[MAX_GAMEPADS][MAX_GAMEPAD_BUTTONS];
static unsigned char rl_previousGamepadState[MAX_GAMEPADS][MAX_GAMEPAD_BUTTONS];
static int rl_lastGamepadButtonPressed = 0;

/* Internal hook to initialize joysticks (Call this in InitWindow) */
void InitGamepads(void) {
  SDL_InitSubSystem(SDL_INIT_JOYSTICK);
  int numJoysticks = SDL_NumJoysticks();
  if (numJoysticks > MAX_GAMEPADS)
    numJoysticks = MAX_GAMEPADS;

  for (int i = 0; i < numJoysticks; i++) {
    rl_joysticks[i] = SDL_JoystickOpen(i);
  }

  memset(rl_currentGamepadState, 0, sizeof(rl_currentGamepadState));
  memset(rl_previousGamepadState, 0, sizeof(rl_previousGamepadState));
}

/* Internal hook to close joysticks (Call this in CloseWindow) */
void CloseGamepads(void) {
  for (int i = 0; i < MAX_GAMEPADS; i++) {
    if (rl_joysticks[i]) {
      SDL_JoystickClose(rl_joysticks[i]);
      rl_joysticks[i] = NULL;
    }
  }
}

/* Internal hook to update state (Call this in WindowShouldClose) */
void UpdateGamepads(void) {
  SDL_JoystickUpdate();
  rl_lastGamepadButtonPressed = 0;

  for (int i = 0; i < MAX_GAMEPADS; i++) {
    if (!rl_joysticks[i])
      continue;

    int numButtons = SDL_JoystickNumButtons(rl_joysticks[i]);
    if (numButtons > MAX_GAMEPAD_BUTTONS)
      numButtons = MAX_GAMEPAD_BUTTONS;

    for (int b = 0; b < numButtons; b++) {
      rl_previousGamepadState[i][b] = rl_currentGamepadState[i][b];
      rl_currentGamepadState[i][b] = SDL_JoystickGetButton(rl_joysticks[i], b);

      if (rl_currentGamepadState[i][b] && !rl_previousGamepadState[i][b]) {
        /* Store the raw SDL button index as the last pressed button */
        rl_lastGamepadButtonPressed = b;
      }
    }
  }
}

/* * Generic Mapping: Since SDL 1.2 doesn't have a standard mapping,
 * we map standard Raylib face buttons to SDL buttons 0-3 sequentially,
 * which covers 90% of old PC gamepads.
 */
static int MapRaylibGamepadToSDL(int button) {
  switch (button) {
  case GAMEPAD_BUTTON_RIGHT_FACE_DOWN:
    return 0; /* 'A' or 'Cross' */
  case GAMEPAD_BUTTON_RIGHT_FACE_RIGHT:
    return 1; /* 'B' or 'Circle' */
  case GAMEPAD_BUTTON_RIGHT_FACE_LEFT:
    return 2; /* 'X' or 'Square' */
  case GAMEPAD_BUTTON_RIGHT_FACE_UP:
    return 3; /* 'Y' or 'Triangle' */
  case GAMEPAD_BUTTON_LEFT_TRIGGER_1:
    return 4; /* L1 */
  case GAMEPAD_BUTTON_RIGHT_TRIGGER_1:
    return 5; /* R1 */
  case GAMEPAD_BUTTON_MIDDLE_LEFT:
    return 6; /* Select */
  case GAMEPAD_BUTTON_MIDDLE_RIGHT:
    return 7; /* Start */
  default:
    return -1;
  }
}

/* --- Raylib Gamepad Public API --- */

bool IsGamepadAvailable(int gamepad) {
  if (gamepad < 0 || gamepad >= MAX_GAMEPADS)
    return false;
  return rl_joysticks[gamepad] != NULL;
}

const char *GetGamepadName(int gamepad) {
  if (IsGamepadAvailable(gamepad)) {
    return SDL_JoystickName(gamepad);
  }
  return "Unknown";
}

bool IsGamepadButtonDown(int gamepad, int button) {
  if (!IsGamepadAvailable(gamepad))
    return false;
  int sdlBtn = MapRaylibGamepadToSDL(button);
  if (sdlBtn < 0 || sdlBtn >= MAX_GAMEPAD_BUTTONS)
    return false;
  return rl_currentGamepadState[gamepad][sdlBtn] != 0;
}

bool IsGamepadButtonPressed(int gamepad, int button) {
  if (!IsGamepadAvailable(gamepad))
    return false;
  int sdlBtn = MapRaylibGamepadToSDL(button);
  if (sdlBtn < 0 || sdlBtn >= MAX_GAMEPAD_BUTTONS)
    return false;
  return (rl_currentGamepadState[gamepad][sdlBtn] != 0) &&
         (rl_previousGamepadState[gamepad][sdlBtn] == 0);
}

bool IsGamepadButtonReleased(int gamepad, int button) {
  if (!IsGamepadAvailable(gamepad))
    return false;
  int sdlBtn = MapRaylibGamepadToSDL(button);
  if (sdlBtn < 0 || sdlBtn >= MAX_GAMEPAD_BUTTONS)
    return false;
  return (rl_currentGamepadState[gamepad][sdlBtn] == 0) &&
         (rl_previousGamepadState[gamepad][sdlBtn] != 0);
}

bool IsGamepadButtonUp(int gamepad, int button) {
  if (!IsGamepadAvailable(gamepad))
    return false;
  int sdlBtn = MapRaylibGamepadToSDL(button);
  if (sdlBtn < 0 || sdlBtn >= MAX_GAMEPAD_BUTTONS)
    return false;
  return rl_currentGamepadState[gamepad][sdlBtn] == 0;
}

int GetGamepadButtonPressed(void) { return rl_lastGamepadButtonPressed; }

int GetGamepadAxisCount(int gamepad) {
  if (!IsGamepadAvailable(gamepad))
    return 0;
  return SDL_JoystickNumAxes(rl_joysticks[gamepad]);
}

float GetGamepadAxisMovement(int gamepad, int axis) {
  if (!IsGamepadAvailable(gamepad))
    return 0.0f;

  /* SDL 1.2 axis values range from -32768 to 32767. Raylib expects -1.0f
   * to 1.0f */
  Sint16 value = SDL_JoystickGetAxis(rl_joysticks[gamepad], axis);

  /* Add a small deadzone to prevent drift on old controllers */
  if (abs(value) < 3200)
    return 0.0f;

  return (float)value / 32767.0f;
}