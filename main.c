#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "raylib.h"

#define WIN_SCALAR 	1
#define WIN_WIDTH 	1024 * WIN_SCALAR
#define WIN_HEIGHT 	768 * WIN_SCALAR

#define FPS_FALLBACK 	60

#define QUIT_KEY	81
#define DEL_KEY		261
#define ALT_KEY_L 	342
#define ALT_KEY_R 	346
#define ENTER_KEY 	257

/* Sounds */
#define SOUND_CLICK
#define SOUND_CRUMPLE

void quit(void) {
  CloseWindow();
  CloseAudioDevice();
}

typedef struct {
  short x;
  short y;
  Color c;
} pixel_point;

typedef struct {
  pixel_point *data;
  size_t len;
  size_t cap;
} pixel_points;

void push_drawing(pixel_points *p, short x, short y, Color c) {
  if (p->len >= p->cap) {
    p->cap = p->cap ? p->cap * 2 : 4;
    p->data = realloc(p->data, p->cap * sizeof(pixel_point));
  }
  p->data[p->len++] = (pixel_point){x, y, c};
}

float pixel_distance(float x1, float y1, float x2, float y2) {
  float dx = x2 - x1;
  float dy = y2 - y1;
  return sqrtf(dx * dx + dy * dy);
}

void clear_drawing(pixel_points *p) {
  free(p->data);
  p->data = NULL;
  p->len = 0;
  p->cap = 0;
}

void shift_random(short *xy, short shift_amount) {
  *xy += ((rand() % shift_amount) - (shift_amount / 2));
}

void jiggle_point(pixel_point *p, short jiggle_range) {
  shift_random(&p->x, jiggle_range);
  shift_random(&p->y, jiggle_range);
}

Color random_color(void) {
  short r = rand() % 255;
  short g = rand() % 255;
  short b = rand() % 255;
  Color c = CLITERAL(Color){r, g, b, 255};

  return c;
}

int main(void) {
  srand(time(NULL));

  InitWindow(WIN_WIDTH, WIN_HEIGHT, "RAYLIB TINKERING");
  //SetTargetFPS(GetMonitorRefreshRate(GetCurrentMonitor()));
  /* SetTargetFPS (FPS_FALLBACK); */

  InitAudioDevice();
  Sound sound_crumple = LoadSound("./data/sound/crumple.ogg");
  Sound sound_click = LoadSound("./data/sound/mouse_click.ogg");

  if (!IsWindowFocused())
    SetWindowFocused();

  pixel_points drawing = {0};

  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(BLACK);
    DrawFPS(50, 50);
    if (IsMouseButtonDown(0)) {
      short draw_x = GetMouseX();
      short draw_y = GetMouseY();

      Color draw_c = random_color();

      push_drawing(&drawing, draw_x, draw_y, draw_c);
    }
    for (size_t i = 0; i < drawing.len; i++) {
      short x = drawing.data[i].x;
      short y = drawing.data[i].y;
      Color c = drawing.data[i].c;
      short last_x = i > 0 ? drawing.data[i - 1].x : 0;
      short last_y = i > 0 ? drawing.data[i - 1].y : 0;

      int render_height = GetRenderHeight();
      if (drawing.data[i].y >= render_height) {
        drawing.data[i].y = render_height;
      }

      if (pixel_distance(x, y, last_x, last_y) > 1 &&
          pixel_distance(x, y, last_x, last_y) <= 4) {
        DrawLine(last_x, last_y, x, y, c);
        DrawCircle(x, y, 2, c);
      } else {
        DrawPixel(x, y, c);
      }

      if (rand() % 100 < 50)
        jiggle_point(&drawing.data[i], 5);
      else
        drawing.data[i].y++;
    }
    EndDrawing();

    if (IsMouseButtonPressed(0)) {
      if (IsAudioDeviceReady()) {
        SetMasterVolume(1.0);
        StopSound(sound_click);
        short pitch_range = 200; // In thousanths
        float pitch =
            ((rand() % pitch_range) + (1000 - (pitch_range / 2))) / 1000.0;
        printf("%.3f\n", pitch);
        SetSoundPitch(sound_click, pitch);
        PlaySound(sound_click);
      }
    }

    short k = GetKeyPressed();

    if (k)
      printf("keycode: %d\n", k);

    if (k == QUIT_KEY)
      break;

    if (k == DEL_KEY) {
      clear_drawing(&drawing);
      PlaySound(sound_crumple);
    }

    if ((IsKeyDown(ALT_KEY_L) || IsKeyDown(ALT_KEY_R)) &&
        IsKeyPressed(ENTER_KEY))
      ToggleBorderlessWindowed();
  }

  UnloadSound(sound_click);
  UnloadSound(sound_crumple);

  free(drawing.data);

  quit();
  return 0;
}
