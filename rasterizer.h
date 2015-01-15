struct triangle {
    int Ax; int Ay;
    int Bx; int By;
    int Cx; int Cy;

triangle(): 
Ax(0), Ay(0), Bx(0), By(0), Cx(0), Cy(0) {}

triangle(int Ax_, int Ay_, int Bx_, int By_, int Cx_, int Cy_): 
Ax(Ax_), Ay(Ay_), Bx(Bx_), By(By_), Cx(Cx_), Cy(Cy_) {}
};

struct sdl_ctx {
    SDL_Window *mainwindow;
    SDL_Renderer *renderer;
    int xrez;
    int yrez;
    int frameDelay;
    int fps;
    int tickDelay;
    int ticksPerFps;

    struct triangle *tr;
};

