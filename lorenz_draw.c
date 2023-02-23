#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include "lorenz.h"

#define WINDOW_TITLE "Lorenz Attractor"
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

#define X_SCALE 10.0
#define Y_SCALE 10.0
#define Z_SCALE 5.0

int main(int argc, char *argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init Error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow(WINDOW_TITLE, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == NULL) {
        fprintf(stderr, "SDL_CreateWindow Error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        fprintf(stderr, "SDL_CreateRenderer Error: %s\n", SDL_GetError());
        return 1;
    }

    double x = 1.0;
    double y = 1.0;
    double z = 1.0;
    double s = 10.0;
    double r = 28.0;
    double b = 8.0/3.0;
    int n = 10000;
    double xout[n], yout[n], zout[n];
    lorenz(x, y, z, s, r, b, n, xout, yout, zout);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    for (int i = 1; i < n; i++) {
        double x1 = xout[i-1];
        double y1 = yout[i-1];
        double z1 = zout[i-1];
        double x2 = xout[i];
        double y2 = yout[i];
        double z2 = zout[i];
        int sx1 = (int)(WINDOW_WIDTH/2.0 + X_SCALE*x1);
        int sy1 = (int)(WINDOW_HEIGHT/2.0 - Y_SCALE*y1);
        int sx2 = (int)(WINDOW_WIDTH/2.0 + X_SCALE*x2);
        int sy2 = (int)(WINDOW_HEIGHT/2.0 - Y_SCALE*y2);
        SDL_RenderDrawLine(renderer, sx1, sy1, sx2, sy2);
    }

    SDL_RenderPresent(renderer);

    SDL_Delay(5000);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
