#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_surface.h>
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

    ctx->tr = new triangle(0,0, 100, 100, 100, 0);
}

void render_triangle(sdl_ctx *ctx, triangle *tr) {
    SDL_RenderDrawLine(ctx->renderer, tr->Ax, tr->Ay, tr->Bx, tr->By);
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

        render_triangle(ctx, ctx->tr);
        
        SDL_RenderPresent(ctx->renderer);
        SDL_UpdateWindowSurface( ctx->mainwindow );
        SDL_Delay(ctx->tickDelay);
    }
}

int main() {
    sdl_ctx ctx = {0};

    sdl_init(&ctx);
    sdl_loop(&ctx);
    return 0;
}
