#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_surface.h>
#include <SDL2_image/SDL_image.h>

#include <algorithm>
#include "rasterizer.h"

SDL_Surface *load_png(const char *name) {
    static bool first_load = true;

    if (first_load) {
        // load support for the JPG and PNG image formats
        int flags=IMG_INIT_JPG|IMG_INIT_PNG;
        int initted=IMG_Init(flags);
        if(initted&flags != flags) {
            fprintf(stderr, "IMG_Init: Failed to init required jpg and png support!\n");
            fprintf(stderr, "IMG_Init: %s\n", IMG_GetError());
        }
        first_load = false;
    }

    SDL_Surface *surf = IMG_Load(name);

    if (!surf) {
        fprintf (stderr, "Cannot load image: %s\n",  IMG_GetError());
        exit(EXIT_FAILURE);
    }

    return surf;
}

void triangle::compute_det() {
        det = (y2 - y3) * (x1 - x3) + (x3 - x2) * (y1 - y3);
        printf ("Triangle (%d, %d), (%d, %d), (%d, %d), D = %d\n", 
                x1,  y1, x2,  y2, x3,  y3, det);
}


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

    ctx->tex_surface = load_png("./Dogecoin.png");
    ctx->tr = new triangle(100,100, 600, 600, 300, 100);
    ctx->tr->compute_det();
    ctx->uv = new uvmap(0,0, 0, 1, 1, 0);
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

void c2barycentric(triangle &tr, int x, int y, double &l1, double &l2, double &l3) {
    l1 = ((tr.y2 - tr.y3 ) * (x - tr.x3) + (tr.x3 - tr.x2) * (y - tr.y3)) / (float)tr.det;
    l2 = ((tr.y3 - tr.y1 ) * (x - tr.x3) + (tr.x1 - tr.x3) * (y - tr.y3)) / (float)tr.det;
    l3 = (1.0 - l1 - l2);
}

void b2cartesian(uvmap &uv, float l1, float l2, float l3, float &u, float &v) {
    u = l1*uv.u1 + l2*uv.u2 + l3*uv.u3;
    v = l1*uv.v1 + l2*uv.v2 + l3*uv.v3;
}

void sample(float u, float v, SDL_Surface *ts, Uint8 &r, Uint8 &g, Uint8 &b, Uint8 &a) {
    int tx = ts->w * u;
    int ty = ts->h * v;
    int bpp = ts->format->BitsPerPixel / 8;
    int pixnum = (ty * ts->w + tx);

    uint8_t *pp = (uint8_t *)(ts->pixels) + pixnum * bpp;

    r = pp[0];
    g = pp[1];
    b = pp[2];
    a = pp[3];
}

// sample from tex_surface and output pixel to (x, y)
void triangle_output_pixel(sdl_ctx *ctx, uvmap *uv, float l1, float l2, SDL_Surface *tex_surface, int x, int y) {
    double l3 = (1.0 - l1 - l2);
    float u, v;
    b2cartesian(*uv, l1, l2, l3, u, v);

    Uint8 r,g,b,a;
    int dbg_x, dbg_y;

    sample(u, v, tex_surface, r, g, b, a);
    SDL_SetRenderDrawColor(ctx-> renderer,  r, g, b, a);
    SDL_RenderDrawPoint(ctx->renderer, x, y);
}

// using algorithm by Pineda (1988)
// http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.157.4621&rep=rep1&type=pdf
void render_triangle(sdl_ctx *ctx, triangle *tr, uvmap *uv, SDL_Surface *tex_surface) {
    int sx, sy, ex, ey;
    get_triangle_bb(tr, &sx, &sy, &ex, &ey);

    //E(x+1,y) = E(x,y) + dX
    //E(x,y+1) = E(x,y) - dX

    int x = sx, y = sy;

    uvmap *testuv = new uvmap(tr->x1, tr->y1,tr->x2, tr->y2,tr->x3, tr->y3);
    
    for (int y = sy; y < ey; y++) {
        int sd1 = edge_func(tr->Ax, tr->Bx - tr->Ax, tr->Ay, tr->By - tr->Ay, x, y);
        int sd2 = edge_func(tr->Bx, tr->Cx - tr->Bx, tr->By, tr->Cy - tr->By, x, y);
        int sd3 = edge_func(tr->Cx, tr->Ax - tr->Cx, tr->Cy, tr->Ay - tr->Cy, x, y);

        for (int x = sx; x < ex; x++) {
            sd1 = sd1 + tr->By - tr->Ay;
            sd2 = sd2 + tr->Cy - tr->By;
            sd3 = sd3 + tr->Ay - tr->Cy;

            if (sd1 > 0 && sd2 > 0 && sd3 > 0) {
                // ok, we are /inside/
                double l1, l2, l3;
                c2barycentric(*tr, x, y, l1, l2, l3);
                triangle_output_pixel(ctx, uv, l1, l2, tex_surface, x, y);
            }
        }
    }
    SDL_SetRenderDrawColor(ctx-> renderer,  0, 0, 0, 1);
}

inline float func(int x, int y, float A, float B, float C) {

}

void output_pix(SDL_Renderer *renderer, int x, int y, int scf) {
    SDL_Rect r;
    r.x = x*scf;
    r.y = y*scf;
    r.w = scf;
    r.h = scf;

    SDL_RenderFillRect(renderer, &r);
}

int sign(int val) {
    return val > 0 ? 1: -1;
}

void render_line(sdl_ctx *ctx, int x0, int y0, int x1, int y1, int scf) {
    float dx = x1 - x0;
    float dy = y1 - y0;
    float slope = dy/dx;
    int y = y0;
    float error = 0;

    for (int x = x0; x < x1; x++) {
        output_pix(ctx->renderer, x, y, scf);
        error += fabs(slope);

        while (error >= 0.5) {
            output_pix(ctx->renderer, x, y, scf);
            y += sign(y1 - y0);
            error -= 1.0;
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

        SDL_SetRenderDrawColor(ctx->renderer, 0, 0, 0, 255);
        SDL_RenderClear(ctx->renderer);

        render_triangle(ctx, ctx->tr, ctx->uv, ctx->tex_surface);

        SDL_SetRenderDrawColor(ctx-> renderer,  255, 255, 255, 255);
        render_line(ctx, 30, 90,  80,   120, 5);
        render_line(ctx, 80, 120, 150,  80,  5);
        render_line(ctx, 30, 90,  150,  80,  5);

        float om = 3.14/16;
        float t = ticks/6;

        ctx->tr->Ax = 300 + 100 * cos((om+3.14/20) * t);
        ctx->tr->Ay = 300 + 100 * sin(om * t);
        ctx->tr->Bx = 700 + 100 * cos((om * (t + 3.14/16)));
        ctx->tr->By = 200 + 100 * sin((om * (t + 3.14/16)));
        ctx->tr->Cx = 200 + 200 * cos(om * t);
        ctx->tr->Cy = 200 + 100 * sin(om* t);

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
