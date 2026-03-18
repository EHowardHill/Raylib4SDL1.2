#ifndef RAYMATH_H
#define RAYMATH_H

#include "raylib.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef EPSILON
#define EPSILON 0.000001f
#endif

/* DEG2RAD, RAD2DEG, PI already defined in raylib.h — don't redefine */

typedef struct float3 {
  float v[3];
} float3;
typedef struct float16 {
  float v[16];
} float16;

/* --- Vector2 Math --- */
Vector2 Vector2Zero(void);
Vector2 Vector2One(void);
Vector2 Vector2Add(Vector2 v1, Vector2 v2);
Vector2 Vector2Subtract(Vector2 v1, Vector2 v2);
Vector2 Vector2Scale(Vector2 v, float scale);
Vector2 Vector2Multiply(Vector2 v1, Vector2 v2);
Vector2 Vector2Negate(Vector2 v);
Vector2 Vector2Divide(Vector2 v1, Vector2 v2);
float Vector2Length(Vector2 v);
float Vector2LengthSqr(Vector2 v);
float Vector2Distance(Vector2 v1, Vector2 v2);
float Vector2DistanceSqr(Vector2 v1, Vector2 v2);
float Vector2Angle(Vector2 v1, Vector2 v2);
Vector2 Vector2Normalize(Vector2 v);
float Vector2DotProduct(Vector2 v1, Vector2 v2);
float Vector2CrossProduct(Vector2 v1, Vector2 v2);
Vector2 Vector2Lerp(Vector2 v1, Vector2 v2, float amount);
Vector2 Vector2Rotate(Vector2 v, float angle);
Vector2 Vector2Clamp(Vector2 v, Vector2 min, Vector2 max);
Vector2 Vector2ClampValue(Vector2 v, float min, float max);
bool Vector2Equals(Vector2 p, Vector2 q);

/* --- Minimal Vector3 Math (for 3D type compatibility) --- */
Vector3 Vector3Zero(void);
Vector3 Vector3One(void);
Vector3 Vector3Add(Vector3 v1, Vector3 v2);
Vector3 Vector3Subtract(Vector3 v1, Vector3 v2);
Vector3 Vector3Scale(Vector3 v, float scalar);
float Vector3Length(Vector3 v);
Vector3 Vector3Normalize(Vector3 v);
float Vector3DotProduct(Vector3 v1, Vector3 v2);
Vector3 Vector3CrossProduct(Vector3 v1, Vector3 v2);

/* --- Matrix Math (identity only, stubs for the rest) --- */
Matrix MatrixIdentity(void);
Matrix MatrixMultiply(Matrix left, Matrix right);

/* --- Quaternion (minimal stubs) --- */
Quaternion QuaternionIdentity(void);

/* --- Gamepad Enums --- */
typedef enum {
  GAMEPAD_BUTTON_UNKNOWN = 0,
  GAMEPAD_BUTTON_LEFT_FACE_UP,
  GAMEPAD_BUTTON_LEFT_FACE_RIGHT,
  GAMEPAD_BUTTON_LEFT_FACE_DOWN,
  GAMEPAD_BUTTON_LEFT_FACE_LEFT,
  GAMEPAD_BUTTON_RIGHT_FACE_UP,
  GAMEPAD_BUTTON_RIGHT_FACE_RIGHT,
  GAMEPAD_BUTTON_RIGHT_FACE_DOWN,
  GAMEPAD_BUTTON_RIGHT_FACE_LEFT,
  GAMEPAD_BUTTON_LEFT_TRIGGER_1,
  GAMEPAD_BUTTON_LEFT_TRIGGER_2,
  GAMEPAD_BUTTON_RIGHT_TRIGGER_1,
  GAMEPAD_BUTTON_RIGHT_TRIGGER_2,
  GAMEPAD_BUTTON_MIDDLE_LEFT,  /* PS3 Select */
  GAMEPAD_BUTTON_MIDDLE,       /* PS Button/XBOX Button */
  GAMEPAD_BUTTON_MIDDLE_RIGHT, /* PS3 Start */
  GAMEPAD_BUTTON_LEFT_THUMB,
  GAMEPAD_BUTTON_RIGHT_THUMB
} GamepadButton;

typedef enum {
  GAMEPAD_AXIS_LEFT_X = 0,
  GAMEPAD_AXIS_LEFT_Y = 1,
  GAMEPAD_AXIS_RIGHT_X = 2,
  GAMEPAD_AXIS_RIGHT_Y = 3,
  GAMEPAD_AXIS_LEFT_TRIGGER = 4,
  GAMEPAD_AXIS_RIGHT_TRIGGER = 5
} GamepadAxis;

/* --- Gamepad Functions --- */
bool IsGamepadAvailable(int gamepad);
const char *GetGamepadName(int gamepad);
bool IsGamepadButtonPressed(int gamepad, int button);
bool IsGamepadButtonDown(int gamepad, int button);
bool IsGamepadButtonReleased(int gamepad, int button);
bool IsGamepadButtonUp(int gamepad, int button);
int GetGamepadButtonPressed(void);
int GetGamepadAxisCount(int gamepad);
float GetGamepadAxisMovement(int gamepad, int axis);

#ifdef __cplusplus
}
#endif

#endif /* RAYMATH_H */
