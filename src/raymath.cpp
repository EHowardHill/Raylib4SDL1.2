/*
 * raymath.cpp — Vector2, Vector3, Matrix, and Quaternion math.
 */

#include "raymath.h"

#include <math.h>

/* ======================================================================== */
/*  Vector2                                                                 */
/* ======================================================================== */

Vector2 Vector2Zero(void) {
  Vector2 r = {0.0f, 0.0f};
  return r;
}
Vector2 Vector2One(void) {
  Vector2 r = {1.0f, 1.0f};
  return r;
}

Vector2 Vector2Add(Vector2 v1, Vector2 v2) {
  Vector2 r = {v1.x + v2.x, v1.y + v2.y};
  return r;
}

Vector2 Vector2Subtract(Vector2 v1, Vector2 v2) {
  Vector2 r = {v1.x - v2.x, v1.y - v2.y};
  return r;
}

Vector2 Vector2Scale(Vector2 v, float scale) {
  Vector2 r = {v.x * scale, v.y * scale};
  return r;
}

Vector2 Vector2Multiply(Vector2 v1, Vector2 v2) {
  Vector2 r = {v1.x * v2.x, v1.y * v2.y};
  return r;
}

Vector2 Vector2Negate(Vector2 v) {
  Vector2 r = {-v.x, -v.y};
  return r;
}

Vector2 Vector2Divide(Vector2 v1, Vector2 v2) {
  Vector2 r = {v1.x / v2.x, v1.y / v2.y};
  return r;
}

float Vector2Length(Vector2 v) {
  return (float)sqrt((double)(v.x * v.x + v.y * v.y));
}

float Vector2LengthSqr(Vector2 v) { return v.x * v.x + v.y * v.y; }

float Vector2Distance(Vector2 v1, Vector2 v2) {
  float dx = v1.x - v2.x;
  float dy = v1.y - v2.y;
  return (float)sqrt((double)(dx * dx + dy * dy));
}

float Vector2DistanceSqr(Vector2 v1, Vector2 v2) {
  float dx = v1.x - v2.x;
  float dy = v1.y - v2.y;
  return dx * dx + dy * dy;
}

float Vector2Angle(Vector2 v1, Vector2 v2) {
  float dot = v1.x * v2.x + v1.y * v2.y;
  float cross = v1.x * v2.y - v1.y * v2.x;
  return (float)atan2((double)cross, (double)dot);
}

Vector2 Vector2Normalize(Vector2 v) {
  Vector2 r = {0.0f, 0.0f};
  float len = Vector2Length(v);
  if (len > EPSILON) {
    float inv = 1.0f / len;
    r.x = v.x * inv;
    r.y = v.y * inv;
  }
  return r;
}

float Vector2DotProduct(Vector2 v1, Vector2 v2) {
  return v1.x * v2.x + v1.y * v2.y;
}

float Vector2CrossProduct(Vector2 v1, Vector2 v2) {
  return v1.x * v2.y - v1.y * v2.x;
}

Vector2 Vector2Lerp(Vector2 v1, Vector2 v2, float amount) {
  Vector2 r;
  r.x = v1.x + (v2.x - v1.x) * amount;
  r.y = v1.y + (v2.y - v1.y) * amount;
  return r;
}

Vector2 Vector2Rotate(Vector2 v, float angle) {
  float c = cosf(angle);
  float s = sinf(angle);
  Vector2 r;
  r.x = v.x * c - v.y * s;
  r.y = v.x * s + v.y * c;
  return r;
}

Vector2 Vector2Clamp(Vector2 v, Vector2 min, Vector2 max) {
  Vector2 r;
  r.x = (v.x < min.x) ? min.x : (v.x > max.x) ? max.x : v.x;
  r.y = (v.y < min.y) ? min.y : (v.y > max.y) ? max.y : v.y;
  return r;
}

Vector2 Vector2ClampValue(Vector2 v, float min, float max) {
  float len = Vector2Length(v);
  if (len < EPSILON)
    return v;
  if (len < min)
    return Vector2Scale(Vector2Normalize(v), min);
  if (len > max)
    return Vector2Scale(Vector2Normalize(v), max);
  return v;
}

bool Vector2Equals(Vector2 p, Vector2 q) {
  return (fabsf(p.x - q.x) <= EPSILON) && (fabsf(p.y - q.y) <= EPSILON);
}

/* ======================================================================== */
/*  Vector3 (minimal — for type compatibility)                              */
/* ======================================================================== */

Vector3 Vector3Zero(void) {
  Vector3 r = {0, 0, 0};
  return r;
}
Vector3 Vector3One(void) {
  Vector3 r = {1, 1, 1};
  return r;
}

Vector3 Vector3Add(Vector3 v1, Vector3 v2) {
  Vector3 r = {v1.x + v2.x, v1.y + v2.y, v1.z + v2.z};
  return r;
}

Vector3 Vector3Subtract(Vector3 v1, Vector3 v2) {
  Vector3 r = {v1.x - v2.x, v1.y - v2.y, v1.z - v2.z};
  return r;
}

Vector3 Vector3Scale(Vector3 v, float scalar) {
  Vector3 r = {v.x * scalar, v.y * scalar, v.z * scalar};
  return r;
}

float Vector3Length(Vector3 v) {
  return (float)sqrt((double)(v.x * v.x + v.y * v.y + v.z * v.z));
}

Vector3 Vector3Normalize(Vector3 v) {
  Vector3 r = {0, 0, 0};
  float len = Vector3Length(v);
  if (len > EPSILON) {
    float inv = 1.0f / len;
    r.x = v.x * inv;
    r.y = v.y * inv;
    r.z = v.z * inv;
  }
  return r;
}

float Vector3DotProduct(Vector3 v1, Vector3 v2) {
  return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

Vector3 Vector3CrossProduct(Vector3 v1, Vector3 v2) {
  Vector3 r;
  r.x = v1.y * v2.z - v1.z * v2.y;
  r.y = v1.z * v2.x - v1.x * v2.z;
  r.z = v1.x * v2.y - v1.y * v2.x;
  return r;
}

/* ======================================================================== */
/*  Matrix (minimal)                                                        */
/* ======================================================================== */

Matrix MatrixIdentity(void) {
  Matrix r;
  r.m0 = 1;
  r.m4 = 0;
  r.m8 = 0;
  r.m12 = 0;
  r.m1 = 0;
  r.m5 = 1;
  r.m9 = 0;
  r.m13 = 0;
  r.m2 = 0;
  r.m6 = 0;
  r.m10 = 1;
  r.m14 = 0;
  r.m3 = 0;
  r.m7 = 0;
  r.m11 = 0;
  r.m15 = 1;
  return r;
}

Matrix MatrixMultiply(Matrix left, Matrix right) {
  Matrix r;
  r.m0 = left.m0 * right.m0 + left.m1 * right.m4 + left.m2 * right.m8 +
         left.m3 * right.m12;
  r.m1 = left.m0 * right.m1 + left.m1 * right.m5 + left.m2 * right.m9 +
         left.m3 * right.m13;
  r.m2 = left.m0 * right.m2 + left.m1 * right.m6 + left.m2 * right.m10 +
         left.m3 * right.m14;
  r.m3 = left.m0 * right.m3 + left.m1 * right.m7 + left.m2 * right.m11 +
         left.m3 * right.m15;
  r.m4 = left.m4 * right.m0 + left.m5 * right.m4 + left.m6 * right.m8 +
         left.m7 * right.m12;
  r.m5 = left.m4 * right.m1 + left.m5 * right.m5 + left.m6 * right.m9 +
         left.m7 * right.m13;
  r.m6 = left.m4 * right.m2 + left.m5 * right.m6 + left.m6 * right.m10 +
         left.m7 * right.m14;
  r.m7 = left.m4 * right.m3 + left.m5 * right.m7 + left.m6 * right.m11 +
         left.m7 * right.m15;
  r.m8 = left.m8 * right.m0 + left.m9 * right.m4 + left.m10 * right.m8 +
         left.m11 * right.m12;
  r.m9 = left.m8 * right.m1 + left.m9 * right.m5 + left.m10 * right.m9 +
         left.m11 * right.m13;
  r.m10 = left.m8 * right.m2 + left.m9 * right.m6 + left.m10 * right.m10 +
          left.m11 * right.m14;
  r.m11 = left.m8 * right.m3 + left.m9 * right.m7 + left.m10 * right.m11 +
          left.m11 * right.m15;
  r.m12 = left.m12 * right.m0 + left.m13 * right.m4 + left.m14 * right.m8 +
          left.m15 * right.m12;
  r.m13 = left.m12 * right.m1 + left.m13 * right.m5 + left.m14 * right.m9 +
          left.m15 * right.m13;
  r.m14 = left.m12 * right.m2 + left.m13 * right.m6 + left.m14 * right.m10 +
          left.m15 * right.m14;
  r.m15 = left.m12 * right.m3 + left.m13 * right.m7 + left.m14 * right.m11 +
          left.m15 * right.m15;
  return r;
}

/* ======================================================================== */
/*  Quaternion (minimal)                                                    */
/* ======================================================================== */

Quaternion QuaternionIdentity(void) {
  Quaternion q = {0.0f, 0.0f, 0.0f, 1.0f};
  return q;
}
