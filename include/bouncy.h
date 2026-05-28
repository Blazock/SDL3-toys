#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_hints.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_oldnames.h>
#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_surface.h>
#include <SDL3/SDL_timer.h>
#include <SDL3/SDL_video.h>
#include <bits/time.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>

enum {
    width = 1920 >> 1,
    height = 1080 >> 1,
};

static const float PI = 3.14159265;
static const float GRAVITY = 1000.0f;
static const int NUM_SEGMENTS = 64;
static const Uint8 TRAJECTORY_LENGTH = 255;

typedef struct Circle {
    float x, y, radius, vx, vy;
} Circle;

void DrawCircle(SDL_Renderer *renderer, Circle *c, SDL_FColor Color);

void DrawTrajectory(SDL_Renderer *renderer,
                    Circle trajectory[TRAJECTORY_LENGTH], Uint8 head,
                    Uint8 current_trajectory_index);

void step(Circle *circle, float delta_time_in_seconds);

double diff_s(struct timespec start, struct timespec end);

void bouncy_ball();
