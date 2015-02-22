struct triangle {
        union {
            struct {
                int Ax; int Ay;
                int Bx; int By;
                int Cx; int Cy;
            };
            struct {
                int x1; int y1;
                int x2; int y2;
                int x3; int y3;
            };

        };

    int det;

triangle(): 
    Ax(0), Ay(0), Bx(0), By(0), Cx(0), Cy(0), det() {

}

triangle(int Ax_, int Ay_, int Bx_, int By_, int Cx_, int Cy_): 
    Ax(Ax_), Ay(Ay_), Bx(Bx_), By(By_), Cx(Cx_), Cy(Cy_), det(0.0) {
    }

    void compute_det() {
        det = (y2 - y3) * (x1 - x3) + (x3 - x2) * (y1 - y3);
        printf ("Triangle (%d, %d), (%d, %d), (%d, %d), D = %d\n", 
                x1,  y1, x2,  y2, x3,  y3, det);
    }
};

struct uvmap {
    float u1, v1;
    float u2, v2;
    float u3, v3;

uvmap(float u1_, float v1_,float u2_,float v2_,float u3_,float v3_):
    u1(u1_), v1(v1_), u2(u2_), v2(v2_), u3(u3_), v3(v3_) {}
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
    uvmap *uv;
    SDL_Surface *tex_surface;
};
