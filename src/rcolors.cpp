/*
 * rcolors.cpp — Color utility functions.
 */

#include "raylib.h"

bool ColorIsEqual(Color col1, Color col2) {
  return (col1.r == col2.r && col1.g == col2.g && col1.b == col2.b &&
          col1.a == col2.a);
}

Color Fade(Color color, float alpha) {
  if (alpha < 0.0f)
    alpha = 0.0f;
  if (alpha > 1.0f)
    alpha = 1.0f;

  Color result;
  result.r = color.r;
  result.g = color.g;
  result.b = color.b;
  result.a = (unsigned char)((float)color.a * alpha);
  return result;
}

Color ColorAlpha(Color color, float alpha) { return Fade(color, alpha); }

int ColorToInt(Color color) {
  return ((int)color.r << 24) | ((int)color.g << 16) | ((int)color.b << 8) |
         (int)color.a;
}

Color GetColor(unsigned int hexValue) {
  Color color;
  color.r = (unsigned char)((hexValue >> 24) & 0xFF);
  color.g = (unsigned char)((hexValue >> 16) & 0xFF);
  color.b = (unsigned char)((hexValue >> 8) & 0xFF);
  color.a = (unsigned char)(hexValue & 0xFF);
  return color;
}
