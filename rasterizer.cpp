#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_surface.h>
#include <algorithm>
#include "rasterizer.h"

void sdldie(const char *msg)
{
	printf("%s: %s\n", msg, SDL_GetError());
	SDL_Quit();
	exit(1);
}

void sdl_init(struct sdl_ctx *ctx) {
    ctx->xrez = 1024;
    ctx->yrez = 768;

    ctx->frameDelay = 16;
    ctx->fps = 1000.0 / ctx->frameDelay;
    ctx->ticksPerFps = 8;
    ctx->tickDelay = 16 / ctx->ticksPerFps;

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
		sdldie("Unable to initialize SDL");

	ctx->mainwindow = SDL_CreateWindow("simple rasterizer", SDL_WINDOWPOS_CENTERED, 
                                       SDL_WINDOWPOS_CENTERED, ctx->xrez, ctx->yrez, SDL_WINDOW_SHOWN);

	if (!ctx->mainwindow) { /* Die if creation failed */
		sdldie("Unable to create window");
    }

    ctx->renderer = SDL_CreateRenderer(ctx->mainwindow, -1, SDL_RENDERER_ACCELERATED );
    // SDL_SetRenderDrawColor(ctx->renderer, 255, 0, 0, 255);
    SDL_RenderClear(ctx->renderer);

    ctx->tr = new triangle(100,100, 600, 600, 900, 100);
}

int edge_func (int X, int dX, int Y, int dY, int x, int y) {
    return (x - X) * dY - (y - Y) * dX;
}

void get_triangle_bb(triangle *tr, int *sx, int *sy, int *ex, int *ey) {
    *sx = std::min(tr->Ax, std::min(tr->Bx, tr->Cx));
    *sy = std::min(tr->Ay, std::min(tr->By, tr->Cy));

    *ex = std::max(tr->Ax, std::max(tr->Bx, tr->Cx));
    *ey = std::max(tr->Ay, std::max(tr->By, tr->Cy));
}

void render_triangle(sdl_ctx *ctx, triangle *tr) {
    int sx, sy, ex, ey;
    get_triangle_bb(tr, &sx, &sy, &ex, &ey);

    for (int x = sx; x < ex; x++)
        for (int y = sy; y < ey; y++) {
            int sd1 = edge_func(tr->Ax, tr->Bx - tr->Ax, tr->Ay, tr->By - tr->Ay, x, y);
            int sd2 = edge_func(tr->Bx, tr->Cx - tr->Bx, tr->By, tr->Cy - tr->By, x, y);
            int sd3 = edge_func(tr->Cx, tr->Ax - tr->Cx, tr->Cy, tr->Ay - tr->Cy, x, y);

            if (sd1 > 0 && sd2 > 0 && sd3 > 0) {
                SDL_RenderDrawPoint(ctx->renderer, x, y);
            }
        }
}

int sdl_loop(sdl_ctx *ctx) {
    int quit = 0;
    long long ticks = 0;
    
    while(!quit) {
        SDL_Event event;
    
        while(SDL_PollEvent(&event)) {
            if(event.type == SDL_KEYDOWN)
            {
                if (event.key.keysym.scancode == SDL_SCANCODE_Q) {
                    quit = 1;
                    break;
                }
            }
        }
        // ctx->tr->Ax  = 40;        
        render_triangle(ctx, ctx->tr);

        if ((ticks % 10) == 0) {
            printf ("sdfs\n");
            ctx->tr->Ax++;
        }

        SDL_RenderPresent(ctx->renderer);
        SDL_UpdateWindowSurface( ctx->mainwindow );
        SDL_Delay(ctx->tickDelay);

        ticks++;
    }
}

int main() {
    sdl_ctx ctx = {0};

    sdl_init(&ctx);
    sdl_loop(&ctx);
    return 0;
}
