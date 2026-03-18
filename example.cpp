/*
 * example.cpp — Minimal test program for raylib-sdl12.
 *
 * Build:  make example
 * Run:    ./example_app   (needs default.ttf in the same directory)
 */

#include "headers/raylib.h"
#include "headers/raymath.h"

int main(void) {
  InitWindow(800, 600, "raylib-sdl12 test");
  SetTargetFPS(60);

  InitAudioDevice();

  Vector2 ballPos = {400.0f, 300.0f};
  Vector2 ballSpeed = {3.0f, 2.0f};
  float ballRadius = 20.0f;

  while (!WindowShouldClose()) {
    /* --- Update --- */
    ballPos = Vector2Add(ballPos, ballSpeed);

    if (ballPos.x - ballRadius < 0 || ballPos.x + ballRadius > GetScreenWidth())
      ballSpeed.x = -ballSpeed.x;
    if (ballPos.y - ballRadius < 0 ||
        ballPos.y + ballRadius > GetScreenHeight())
      ballSpeed.y = -ballSpeed.y;

    if (IsKeyPressed(KEY_SPACE))
      ballSpeed = Vector2Scale(ballSpeed, -1.0f);

    /* --- Draw --- */
    BeginDrawing();
    ClearBackground(RAYWHITE);

    DrawCircleV(ballPos, ballRadius, MAROON);
    DrawCircleLines((int)ballPos.x, (int)ballPos.y, ballRadius + 4.0f,
                    DARKGRAY);

    DrawRectangleLines(10, 10, GetScreenWidth() - 20, GetScreenHeight() - 20,
                       GRAY);

    DrawText("raylib-sdl12 — bouncing ball", 20, 20, 20, DARKGRAY);
    DrawText(TextFormat("FPS: %d", GetFPS()), 20, 50, 20, LIME);
    DrawText("Press SPACE to reverse", 20, 80, 20, GRAY);

    EndDrawing();
  }

  CloseAudioDevice();
  CloseWindow();
  return 0;
}
