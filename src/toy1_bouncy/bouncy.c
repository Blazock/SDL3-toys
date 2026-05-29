#include "../../include/bouncy.h"
#include <stdio.h>

void bouncy_ball() {
    printf("Hello, Bouncy Ball!\n");

    // 1. Initialize SDL
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "SDL_Init failed: %s\n",
                     SDL_GetError());
        return;
    }

    SDL_SetHint(SDL_HINT_VIDEO_WAYLAND_ALLOW_LIBDECOR,
                "0"); /* disable libdecor on Wayland, use plain xdg-shell */
    // 2. Create Window
    SDL_Window *window =
        SDL_CreateWindow("Bouncy Ball", width, height, SDL_WINDOW_BORDERLESS);
    if (window == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not create window: %s\n",
                     SDL_GetError());
        SDL_Quit();
        return;
    }

    // 3. Create renderer (required by Wayland)
    SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);
    if (renderer == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not create renderer: %s\n",
                     SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return;
    }

    // 4. Ensure window is visible
    SDL_ShowWindow(window);

    // 5. Initialize presentation (required by Wayland)
    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xff);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    Circle circle = {
        .x = width >> 1,
        .y = height >> 1,
        .vx = 200,
        .vy = 200,
        .radius = height >> 4,
    };
    /* ring buffer for trajectory: holds at most TRAJECTORY_LENGTH positions */
    Circle trajectory[TRAJECTORY_LENGTH];
    Uint64 last = SDL_GetPerformanceCounter();

    bool done = false;
    Uint8 head = 0,
          cnt = 0; /* head: next write slot; cnt: number of records so far */
    while (!done) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                done = true;
            }
            if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_Q) {
                done = true;
            }
        }

        Uint64 now = SDL_GetPerformanceCounter();
        /* dt = real time difference (seconds) between frames, for
         * frame-rate-independent physics */
        float dt = (float)(now - last) / SDL_GetPerformanceFrequency();
        last = now;

        /* physics step: update position and velocity by dt */
        step(&circle, dt);

        SDL_RenderClear(renderer);
        SDL_FColor color = {.r = 0.1f, .g = 1.0f, .b = 1.0f, .a = 0.2f};

        trajectory[head++] = (Circle){circle.x, circle.y, 0, 0, 0};
        if (cnt < TRAJECTORY_LENGTH)
            cnt++;
        DrawTrajectory(renderer, trajectory, head, cnt);
        DrawCircle(renderer, &circle, color);
        /* push current center into ring buffer, head wraps around automatically
         */
        SDL_RenderPresent(renderer);

        /* cap frame rate at ~60 FPS: sleep if this frame finished early */
        float target_frame_time = 1.0f / 60.0f;
        if (dt < target_frame_time) {
            SDL_Delay((Uint32)((target_frame_time - dt) * 1000.0f));
        }
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void DrawCircle(SDL_Renderer *renderer, Circle *circle, SDL_FColor color) {
    /* draw filled circle as a triangle fan: center + NUM_SEGMENTS points on the
     * circumference */
    SDL_Vertex vertices[NUM_SEGMENTS + 2];
    float step = 2.0f * PI / NUM_SEGMENTS;

    /* vertex 0 = center */
    vertices[0].position = (SDL_FPoint){circle->x, circle->y};
    vertices[0].color = color;
    vertices[0].tex_coord = (SDL_FPoint){0, 0};

    /* vertices 1..NUM_SEGMENTS+1 = points on circle (first == last to close the
     * fan) */
    for (int i = 1; i <= NUM_SEGMENTS + 1; ++i) {
        float angle = step * ((i - 1) % NUM_SEGMENTS);
        vertices[i].position =
            (SDL_FPoint){circle->x + circle->radius * SDL_cosf(angle),
                         circle->y + circle->radius * SDL_sinf(angle)};
        vertices[i].color = vertices[0].color;
        vertices[i].tex_coord = (SDL_FPoint){0, 0};
    }

    /* index buffer: every 3 indices form a triangle (0, i+1, i+2) */
    int indices[NUM_SEGMENTS * 3];
    for (int i = 0; i < NUM_SEGMENTS; ++i) {
        indices[i * 3 + 0] = 0;
        indices[i * 3 + 1] = i + 1;
        indices[i * 3 + 2] = i + 2;
    }
    SDL_RenderGeometry(renderer, NULL, vertices, NUM_SEGMENTS + 2, indices,
                       NUM_SEGMENTS * 3);
}

void DrawTrajectory(SDL_Renderer *renderer,
                    Circle trajectory[TRAJECTORY_LENGTH], Uint8 head,
                    Uint8 count) {
    if (count == 0)
        return;
    /* locate the oldest record in the ring buffer as the start */
    Uint8 start = (head - count);
    for (int i = 0; i < count; ++i) {
        Uint8 idx = (start + i);
        float t = (float)i / count;
        /* oldest -> newest: alpha increases, radius decreases, creating a
         * fading trail */
        Uint8 alpha = (Uint8)(t * 255.0f);
        SDL_FColor color = {1.0f, 1.0f, 1.0f, alpha / 255.0f};
        trajectory[idx].radius = 5.0f * t;
        DrawCircle(renderer, &trajectory[idx], color);
    }
}

/* physics integration + wall collision handling */
void step(Circle *circle, float delta_time_in_seconds) {
    float dt = delta_time_in_seconds;

    /* origin at top-left, x right, y down */
    circle->vy += GRAVITY * dt;   /* accumulate gravity into velocity */
    circle->x += circle->vx * dt; /* horizontal displacement */
    circle->y += circle->vy * dt; /* vertical displacement */

    /* bounce off walls (20% energy loss, coefficient 0.8) and clamp position */
    if (circle->x + circle->radius > width) {
        circle->x = width - circle->radius;
        circle->vx = -circle->vx * 0.8f;
    }

    if (circle->x - circle->radius < 0) {
        circle->x = circle->radius;
        circle->vx = -circle->vx * 0.8f;
    }

    if (circle->y - circle->radius < 0) {
        circle->y = circle->radius;
        circle->vy = -circle->vy * 0.8f;
    }

    if (circle->y + circle->radius > height) {
        circle->y = height - circle->radius;
        circle->vy = -circle->vy * 0.8f;
    }
}
