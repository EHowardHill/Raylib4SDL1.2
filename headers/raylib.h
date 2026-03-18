/*******************************************************************************************
*   raylib-sdl12 - A Raylib 5-compatible API backed by SDL 1.2
*
*   Purpose: Port Raylib-based 2D projects to retro platforms (Win95, Classic Mac OS, etc.)
*   NOTE: 3D functions are declared as stubs for ABI compatibility but are NOT implemented.
*
*   DIFFERENCES FROM UPSTREAM RAYLIB:
*     - Textures are SDL_Surface* stored in the 'id' field (cast to unsigned int)
*     - Fonts use SDL_ttf; the TTF_Font* is smuggled in font.texture.id
*     - Audio uses SDL_mixer; Mix_Chunk*/Mix_Music* stored in stream.buffer/ctxData
*     - No GPU acceleration — everything is software-rendered
*     - Rotation/scaling on textures only works via SDL_SoftStretch (no rotation)
*     - Color tinting on blits is ignored for performance on retro CPUs
*
********************************************************************************************/

#ifndef RAYLIB_H
#define RAYLIB_H

#ifndef __cplusplus
#include <stdbool.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

  /* --------------------------------------------------------------------------
   */
  /*  Fundamental Types (must come before any function that uses them) */
  /* --------------------------------------------------------------------------
   */

  typedef struct Vector2 {
    float x;
    float y;
  } Vector2;

  typedef struct Vector3 {
    float x;
    float y;
    float z;
  } Vector3;

  typedef struct Vector4 {
    float x;
    float y;
    float z;
    float w;
  } Vector4;

  typedef Vector4 Quaternion;

  typedef struct Matrix {
    float m0, m4, m8, m12;
    float m1, m5, m9, m13;
    float m2, m6, m10, m14;
    float m3, m7, m11, m15;
  } Matrix;

  typedef struct Color {
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
  } Color;

  typedef struct Rectangle {
    float x;
    float y;
    float width;
    float height;
  } Rectangle;

  /* --------------------------------------------------------------------------
   */
  /*  Color Constants */
  /* --------------------------------------------------------------------------
   */

#define LIGHTGRAY (Color){200, 200, 200, 255}
#define GRAY (Color){130, 130, 130, 255}
#define DARKGRAY (Color){80, 80, 80, 255}
#define YELLOW (Color){253, 249, 0, 255}
#define GOLD (Color){255, 203, 0, 255}
#define ORANGE (Color){255, 161, 0, 255}
#define PINK (Color){255, 109, 194, 255}
#define RED (Color){230, 41, 55, 255}
#define MAROON (Color){190, 33, 55, 255}
#define GREEN (Color){0, 228, 48, 255}
#define LIME (Color){0, 158, 47, 255}
#define DARKGREEN (Color){0, 117, 44, 255}
#define SKYBLUE (Color){102, 191, 255, 255}
#define BLUE (Color){0, 121, 241, 255}
#define DARKBLUE (Color){0, 82, 172, 255}
#define PURPLE (Color){200, 122, 255, 255}
#define VIOLET (Color){135, 60, 190, 255}
#define DARKPURPLE (Color){112, 31, 126, 255}
#define BEIGE (Color){211, 176, 131, 255}
#define BROWN (Color){127, 106, 79, 255}
#define DARKBROWN (Color){76, 63, 47, 255}
#define WHITE (Color){255, 255, 255, 255}
#define BLACK (Color){0, 0, 0, 255}
#define BLANK (Color){0, 0, 0, 0}
#define MAGENTA (Color){255, 0, 255, 255}
#define RAYWHITE (Color){245, 245, 245, 255}

  /* --------------------------------------------------------------------------
   */
  /*  Useful Math Constants */
  /* --------------------------------------------------------------------------
   */

#ifndef PI
#define PI 3.14159265358979323846f
#endif
#ifndef DEG2RAD
#define DEG2RAD (PI / 180.0f)
#endif
#ifndef RAD2DEG
#define RAD2DEG (180.0f / PI)
#endif

  /* --------------------------------------------------------------------------
   */
  /*  Enumerations */
  /* --------------------------------------------------------------------------
   */

  typedef enum {
    MOUSE_BUTTON_LEFT = 0,
    MOUSE_BUTTON_RIGHT = 1,
    MOUSE_BUTTON_MIDDLE = 2,
    MOUSE_BUTTON_SIDE = 3,
    MOUSE_BUTTON_EXTRA = 4,
    MOUSE_BUTTON_FORWARD = 5,
    MOUSE_BUTTON_BACK = 6,
  } MouseButton;

  typedef enum {
    KEY_NULL = 0,
    KEY_APOSTROPHE = 39,
    KEY_COMMA = 44,
    KEY_MINUS = 45,
    KEY_PERIOD = 46,
    KEY_SLASH = 47,
    KEY_ZERO = 48,
    KEY_ONE = 49,
    KEY_TWO = 50,
    KEY_THREE = 51,
    KEY_FOUR = 52,
    KEY_FIVE = 53,
    KEY_SIX = 54,
    KEY_SEVEN = 55,
    KEY_EIGHT = 56,
    KEY_NINE = 57,
    KEY_SEMICOLON = 59,
    KEY_EQUAL = 61,
    KEY_A = 65,
    KEY_B = 66,
    KEY_C = 67,
    KEY_D = 68,
    KEY_E = 69,
    KEY_F = 70,
    KEY_G = 71,
    KEY_H = 72,
    KEY_I = 73,
    KEY_J = 74,
    KEY_K = 75,
    KEY_L = 76,
    KEY_M = 77,
    KEY_N = 78,
    KEY_O = 79,
    KEY_P = 80,
    KEY_Q = 81,
    KEY_R = 82,
    KEY_S = 83,
    KEY_T = 84,
    KEY_U = 85,
    KEY_V = 86,
    KEY_W = 87,
    KEY_X = 88,
    KEY_Y = 89,
    KEY_Z = 90,
    KEY_SPACE = 32,
    KEY_ESCAPE = 256,
    KEY_ENTER = 257,
    KEY_TAB = 258,
    KEY_BACKSPACE = 259,
    KEY_INSERT = 260,
    KEY_DELETE = 261,
    KEY_RIGHT = 262,
    KEY_LEFT = 263,
    KEY_DOWN = 264,
    KEY_UP = 265,
    KEY_PAGE_UP = 266,
    KEY_PAGE_DOWN = 267,
    KEY_HOME = 268,
    KEY_END = 269,
    KEY_CAPS_LOCK = 280,
    KEY_SCROLL_LOCK = 281,
    KEY_NUM_LOCK = 282,
    KEY_PRINT_SCREEN = 283,
    KEY_PAUSE = 284,
    KEY_F1 = 290,
    KEY_F2 = 291,
    KEY_F3 = 292,
    KEY_F4 = 293,
    KEY_F5 = 294,
    KEY_F6 = 295,
    KEY_F7 = 296,
    KEY_F8 = 297,
    KEY_F9 = 298,
    KEY_F10 = 299,
    KEY_F11 = 300,
    KEY_F12 = 301,
    KEY_LEFT_SHIFT = 340,
    KEY_LEFT_CONTROL = 341,
    KEY_LEFT_ALT = 342,
    KEY_LEFT_SUPER = 343,
    KEY_RIGHT_SHIFT = 344,
    KEY_RIGHT_CONTROL = 345,
    KEY_RIGHT_ALT = 346,
    KEY_RIGHT_SUPER = 347,
  } KeyboardKey;

  typedef enum {
    FLAG_VSYNC_HINT = 0x00000040,
    FLAG_FULLSCREEN_MODE = 0x00000002,
    FLAG_WINDOW_RESIZABLE = 0x00000004,
    FLAG_WINDOW_UNDECORATED = 0x00000008,
    FLAG_WINDOW_HIDDEN = 0x00000080,
    FLAG_WINDOW_MINIMIZED = 0x00000200,
    FLAG_WINDOW_MAXIMIZED = 0x00000400,
    FLAG_WINDOW_UNFOCUSED = 0x00000800,
    FLAG_WINDOW_TOPMOST = 0x00001000,
    FLAG_WINDOW_ALWAYS_RUN = 0x00000100,
    FLAG_WINDOW_TRANSPARENT = 0x00000010,
    FLAG_WINDOW_HIGHDPI = 0x00002000,
    FLAG_WINDOW_MOUSE_PASSTHROUGH = 0x00004000,
    FLAG_MSAA_4X_HINT = 0x00000020,
    FLAG_INTERLACED_HINT = 0x00010000,
  } ConfigFlags;

  /* --- Blend Modes --- */
  typedef enum {
    BLEND_ALPHA = 0,         /* Blend textures considering alpha (default) */
    BLEND_ADDITIVE,          /* Blend textures adding colors */
    BLEND_MULTIPLIED,        /* Blend textures multiplying colors */
    BLEND_ADD_COLORS,        /* Alternative Additive */
    BLEND_SUBTRACT_COLORS,   /* Subtract colors */
    BLEND_ALPHA_PREMULTIPLY, /* Premultiplied alpha */
    BLEND_CUSTOM,            /* Custom blend mode */
    BLEND_CUSTOM_SEPARATE    /* Custom blend mode with separate alpha */
  } BlendMode;

  void BeginBlendMode(int mode);
  void EndBlendMode(void);

  /* --- NPatch Layout Enums --- */
  typedef enum {
    NPATCH_NINE_PATCH = 0,
    NPATCH_THREE_PATCH_VERTICAL,
    NPATCH_THREE_PATCH_HORIZONTAL
  } NPatchLayout;

  void DrawTextureNPatch(Texture2D texture, NPatchInfo nPatchInfo,
                         Rectangle dest, Vector2 origin, float rotation,
                         Color tint);

  /* --------------------------------------------------------------------------
   */
  /*  2D Rendering and Image Types */
  /* --------------------------------------------------------------------------
   */

  typedef struct Image {
    void *data; /* Pointer to SDL_Surface */
    int width;
    int height;
    int mipmaps; /* Always 1 */
    int format;
  } Image;

  typedef struct Texture {
    unsigned int id; /* SDL_Surface* cast to unsigned int */
    int width;
    int height;
    int mipmaps; /* Always 1 */
    int format;
  } Texture;

  typedef Texture Texture2D;
  typedef Texture TextureCubemap;

  typedef struct RenderTexture {
    unsigned int id;
    Texture texture;
    Texture depth;
  } RenderTexture;

  typedef RenderTexture RenderTexture2D;

  typedef struct NPatchInfo {
    Rectangle source;
    int left;
    int top;
    int right;
    int bottom;
    int layout;
  } NPatchInfo;

  typedef struct GlyphInfo {
    int value;
    int offsetX;
    int offsetY;
    int advanceX;
    Image image;
  } GlyphInfo;

  typedef struct Font {
    int baseSize;
    int glyphCount;
    int glyphPadding;
    Texture2D texture; /* TTF_Font* smuggled in texture.id */
    Rectangle *recs;
    GlyphInfo *glyphs;
  } Font;

  /* --------------------------------------------------------------------------
   */
  /*  Camera Types */
  /* --------------------------------------------------------------------------
   */

  typedef struct Camera2D {
    Vector2 offset;
    Vector2 target;
    float rotation;
    float zoom;
  } Camera2D;

  typedef struct Camera3D {
    Vector3 position;
    Vector3 target;
    Vector3 up;
    float fovy;
    int projection;
  } Camera3D;

  typedef Camera3D Camera;

  /* --------------------------------------------------------------------------
   */
  /*  3D / Shader Types (declared for ABI compatibility, NOT implemented) */
  /* --------------------------------------------------------------------------
   */

  typedef struct Shader {
    unsigned int id;
    int *locs;
  } Shader;

  typedef struct MaterialMap {
    Texture2D texture;
    Color color;
    float value;
  } MaterialMap;

  typedef struct Material {
    Shader shader;
    MaterialMap *maps;
    float params[4];
  } Material;

  typedef struct Transform {
    Vector3 translation;
    Quaternion rotation;
    Vector3 scale;
  } Transform;

  typedef struct BoneInfo {
    char name[32];
    int parent;
  } BoneInfo;

  typedef struct Mesh {
    int vertexCount;
    int triangleCount;
    float *vertices;
    float *texcoords;
    float *texcoords2;
    float *normals;
    float *tangents;
    unsigned char *colors;
    unsigned short *indices;
    float *animVertices;
    float *animNormals;
    unsigned char *boneIds;
    float *boneWeights;
    unsigned int vaoId;
    unsigned int *vboId;
  } Mesh;

  typedef struct Model {
    Matrix transform;
    int meshCount;
    int materialCount;
    Mesh *meshes;
    Material *materials;
    int *meshMaterial;
    int boneCount;
    BoneInfo *bones;
    Transform *bindPose;
  } Model;

  typedef struct ModelAnimation {
    int boneCount;
    int frameCount;
    BoneInfo *bones;
    Transform **framePoses;
    char name[32];
  } ModelAnimation;

  typedef struct Ray {
    Vector3 position;
    Vector3 direction;
  } Ray;

  typedef struct RayCollision {
    bool hit;
    float distance;
    Vector3 point;
    Vector3 normal;
  } RayCollision;

  typedef struct BoundingBox {
    Vector3 min;
    Vector3 max;
  } BoundingBox;

  /* --------------------------------------------------------------------------
   */
  /*  Audio Types */
  /* --------------------------------------------------------------------------
   */

  typedef struct Wave {
    unsigned int frameCount;
    unsigned int sampleRate;
    unsigned int sampleSize;
    unsigned int channels;
    void *data;
  } Wave;

  typedef struct AudioStream {
    void *buffer; /* Mix_Chunk* for sounds */
    void *processor;
    unsigned int sampleRate;
    unsigned int sampleSize;
    unsigned int channels;
  } AudioStream;

  typedef struct Sound {
    AudioStream stream;
    unsigned int frameCount;
  } Sound;

  typedef struct Music {
    AudioStream stream;
    unsigned int frameCount;
    bool looping;
    int ctxType;
    void *ctxData; /* Mix_Music* for music streams */
  } Music;

  /* --------------------------------------------------------------------------
   */
  /*  Utility Types */
  /* --------------------------------------------------------------------------
   */

  typedef struct VrDeviceInfo {
    int hResolution;
    int vResolution;
    float hScreenSize;
    float vScreenSize;
    float vScreenCenter;
    float eyeToScreenDistance;
    float lensSeparationDistance;
    float interpupillaryDistance;
    float lensDistortionValues[4];
    float chromaAbCorrection[4];
  } VrDeviceInfo;

  typedef struct VrStereoConfig {
    Matrix projection[2];
    Matrix viewOffset[2];
    float leftLensCenter[2];
    float rightLensCenter[2];
    float leftScreenCenter[2];
    float rightScreenCenter[2];
    float scale[2];
    float scaleIn[2];
  } VrStereoConfig;

  typedef struct FilePathList {
    unsigned int capacity;
    unsigned int count;
    char **paths;
  } FilePathList;

  typedef struct AutomationEvent {
    unsigned int frame;
    unsigned int type;
    int params[4];
  } AutomationEvent;

  typedef struct AutomationEventList {
    unsigned int capacity;
    unsigned int count;
    AutomationEvent *events;
  } AutomationEventList;

  /* ==========================================================================
   */
  /*  Function Declarations */
  /* ==========================================================================
   */

  /* --- Window and System --- */
  void InitWindow(int width, int height, const char *title);
  void CloseWindow(void);
  bool WindowShouldClose(void);
  bool IsWindowReady(void);
  bool IsWindowFullscreen(void);
  bool IsWindowHidden(void);
  bool IsWindowMinimized(void);
  bool IsWindowMaximized(void);
  bool IsWindowFocused(void);
  bool IsWindowResized(void);
  void SetWindowTitle(const char *title);
  void SetWindowPosition(int x, int y);
  void SetWindowMinSize(int width, int height);
  void SetWindowSize(int width, int height);
  int GetScreenWidth(void);
  int GetScreenHeight(void);
  void SetTargetFPS(int fps);
  int GetFPS(void);
  float GetFrameTime(void);
  double GetTime(void);
  void SetConfigFlags(unsigned int flags);
  void ToggleFullscreen(void);

  /* --- Drawing --- */
  void BeginDrawing(void);
  void EndDrawing(void);
  void ClearBackground(Color color);

  /* --- Camera 2D --- */
  void BeginMode2D(Camera2D camera);
  void EndMode2D(void);

  /* --- Keyboard Input --- */
  bool IsKeyPressed(int key);
  bool IsKeyDown(int key);
  bool IsKeyReleased(int key);
  bool IsKeyUp(int key);
  int GetKeyPressed(void);
  int GetCharPressed(void);

  /* --- Mouse Input --- */
  bool IsMouseButtonPressed(int button);
  bool IsMouseButtonDown(int button);
  bool IsMouseButtonReleased(int button);
  bool IsMouseButtonUp(int button);
  int GetMouseX(void);
  int GetMouseY(void);
  Vector2 GetMousePosition(void);
  void SetMousePosition(int x, int y);
  float GetMouseWheelMove(void);
  void ShowCursor(void);
  void HideCursor(void);
  bool IsCursorHidden(void);

  /* --- Basic Shapes --- */
  void DrawPixel(int posX, int posY, Color color);
  void DrawPixelV(Vector2 position, Color color);
  void DrawLine(int startPosX, int startPosY, int endPosX, int endPosY,
                Color color);
  void DrawLineV(Vector2 startPos, Vector2 endPos, Color color);
  void DrawLineEx(Vector2 startPos, Vector2 endPos, float thick, Color color);
  void DrawCircle(int centerX, int centerY, float radius, Color color);
  void DrawCircleV(Vector2 center, float radius, Color color);
  void DrawCircleLines(int centerX, int centerY, float radius, Color color);
  void DrawRectangle(int posX, int posY, int width, int height, Color color);
  void DrawRectangleV(Vector2 position, Vector2 size, Color color);
  void DrawRectangleRec(Rectangle rec, Color color);
  void DrawRectangleLines(int posX, int posY, int width, int height,
                          Color color);
  void DrawRectangleLinesEx(Rectangle rec, float lineThick, Color color);
  void DrawRectangleGradientV(int posX, int posY, int width, int height,
                              Color top, Color bottom);
  void DrawRectangleGradientH(int posX, int posY, int width, int height,
                              Color left, Color right);
  void DrawRectangleRounded(Rectangle rec, float roundness, int segments,
                            Color color);
  void DrawTriangle(Vector2 v1, Vector2 v2, Vector2 v3, Color color);
  void DrawTriangleLines(Vector2 v1, Vector2 v2, Vector2 v3, Color color);
  void DrawPoly(Vector2 center, int sides, float radius, float rotation,
                Color color);
  void DrawPolyLines(Vector2 center, int sides, float radius, float rotation,
                     Color color);

  /* --- Splines --- */
  Vector2 GetSplinePointLinear(Vector2 startPos, Vector2 endPos, float t);
  void DrawSplineLinear(const Vector2 *points, int pointCount, float thick,
                        Color color);
  void DrawSplineBezierCubic(const Vector2 *points, int pointCount, float thick,
                             Color color);

  /* --- Collision Detection --- */
  bool CheckCollisionRecs(Rectangle rec1, Rectangle rec2);
  bool CheckCollisionCircles(Vector2 center1, float radius1, Vector2 center2,
                             float radius2);
  bool CheckCollisionCircleRec(Vector2 center, float radius, Rectangle rec);
  bool CheckCollisionPointRec(Vector2 point, Rectangle rec);
  bool CheckCollisionPointCircle(Vector2 point, Vector2 center, float radius);
  bool CheckCollisionPointTriangle(Vector2 point, Vector2 p1, Vector2 p2,
                                   Vector2 p3);
  Rectangle GetCollisionRec(Rectangle rec1, Rectangle rec2);

  /* --- Image / Texture Management --- */
  Image LoadImage(const char *fileName);
  Image GenImageColor(int width, int height, Color color);
  void UnloadImage(Image image);
  Image ImageCopy(Image image);
  void ImageClearBackground(Image * dst, Color color);
  void ImageDraw(Image * dst, Image src, Rectangle srcRec, Rectangle dstRec,
                 Color tint);
  void ImageFlipVertical(Image * image);
  void ImageFlipHorizontal(Image * image);
  void ImageResize(Image * image, int newWidth, int newHeight);
  Color *LoadImageColors(Image image);
  void UnloadImageColors(Color * colors);

  Texture2D LoadTexture(const char *fileName);
  Texture2D LoadTextureFromImage(Image image);
  void UnloadTexture(Texture2D texture);
  bool IsTextureValid(Texture2D texture);
  void UpdateTexture(Texture2D texture, const void *pixels);

  void DrawTexture(Texture2D texture, int posX, int posY, Color tint);
  void DrawTextureV(Texture2D texture, Vector2 position, Color tint);
  void DrawTextureRec(Texture2D texture, Rectangle source, Vector2 position,
                      Color tint);
  void DrawTexturePro(Texture2D texture, Rectangle source, Rectangle dest,
                      Vector2 origin, float rotation, Color tint);

  /* --- Text / Font --- */
  Font GetFontDefault(void);
  Font LoadFont(const char *fileName);
  Font LoadFontEx(const char *fileName, int fontSize, int *codepoints,
                  int codepointCount);
  void UnloadFont(Font font);
  bool IsFontValid(Font font);

  void DrawText(const char *text, int posX, int posY, int fontSize,
                Color color);
  void DrawTextEx(Font font, const char *text, Vector2 position, float fontSize,
                  float spacing, Color tint);
  int MeasureText(const char *text, int fontSize);
  Vector2 MeasureTextEx(Font font, const char *text, float fontSize,
                        float spacing);

  /* --- Text Strings Management --- */
  unsigned int TextLength(const char *text);
  const char *TextFormat(const char *text, ...);
  int TextCopy(char *dst, const char *src);
  bool TextIsEqual(const char *text1, const char *text2);
  const char *TextToUpper(const char *text);
  const char *TextToLower(const char *text);

  /* --- Image-Text --- */
  Image ImageText(const char *text, int fontSize, Color color);
  Image ImageTextEx(Font font, const char *text, float fontSize, float spacing,
                    Color tint);
  void ImageDrawText(Image * dst, const char *text, int posX, int posY,
                     int fontSize, Color color);

  /* --- Color Utilities --- */
  bool ColorIsEqual(Color col1, Color col2);
  Color Fade(Color color, float alpha);
  Color ColorAlpha(Color color, float alpha);
  int ColorToInt(Color color);
  Color GetColor(unsigned int hexValue);

  /* --- Audio Device Management --- */
  void InitAudioDevice(void);
  void CloseAudioDevice(void);
  bool IsAudioDeviceReady(void);
  void SetMasterVolume(float volume);

  /* --- Sound --- */
  Sound LoadSound(const char *fileName);
  void UnloadSound(Sound sound);
  void PlaySound(Sound sound);
  void StopSound(Sound sound);
  void PauseSound(Sound sound);
  void ResumeSound(Sound sound);
  bool IsSoundPlaying(Sound sound);
  void SetSoundVolume(Sound sound, float volume);

  /* --- Music --- */
  Music LoadMusicStream(const char *fileName);
  void UnloadMusicStream(Music music);
  void PlayMusicStream(Music music);
  bool IsMusicStreamPlaying(Music music);
  void UpdateMusicStream(Music music);
  void StopMusicStream(Music music);
  void PauseMusicStream(Music music);
  void ResumeMusicStream(Music music);
  void SetMusicVolume(Music music, float volume);
  float GetMusicTimeLength(Music music);
  float GetMusicTimePlayed(Music music);

  /* --- Misc / Utility --- */
  int GetRandomValue(int min, int max);
  void TraceLog(int logLevel, const char *text, ...);

#ifdef __cplusplus
}
#endif

#endif /* RAYLIB_H */
