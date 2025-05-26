#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "snake.h"

SDL_Renderer *gRenderer = NULL;

void handleInput(SDL_Event event) {
    if (event.type == SDL_QUIT) {
        game_over = 1;
    } else if (event.type == SDL_KEYDOWN) {
        SDL_Keycode ch = event.key.keysym.sym;
        switch (ch) {
            case SDLK_ESCAPE:
                if (showPauseMenu(gRenderer) == 1)
                    game_over = 1;
                break;
            case SDLK_w: if (snake1.dir != DOWN) snake1.dir = UP; break;
            case SDLK_s: if (snake1.dir != UP) snake1.dir = DOWN; break;
            case SDLK_a: if (snake1.dir != RIGHT) snake1.dir = LEFT; break;
            case SDLK_d: if (snake1.dir != LEFT) snake1.dir = RIGHT; break;
            case SDLK_UP: if (snake2.dir != DOWN) snake2.dir = UP; break;
            case SDLK_DOWN: if (snake2.dir != UP) snake2.dir = DOWN; break;
            case SDLK_LEFT: if (snake2.dir != RIGHT) snake2.dir = LEFT; break;
            case SDLK_RIGHT: if (snake2.dir != LEFT) snake2.dir = RIGHT; break;
        }
    }
}


int main(int argc, char *argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init Error: %s\n", SDL_GetError());
        return 1;
    }

    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        fprintf(stderr, "IMG_Init Error: %s\n", IMG_GetError());
        return 1;
    }

    if (TTF_Init() != 0) {
        fprintf(stderr, "TTF_Init Error: %s\n", TTF_GetError());
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("Snake SDL",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WIDTH * SCALE, HEIGHT * SCALE, SDL_WINDOW_SHOWN);
    gRenderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    if (!loadTextures(gRenderer)) {
        fprintf(stderr, "Failed to load textures!\n");
        return 1;
    }

    while (1) {
        showMainMenu(gRenderer);
        initGame();
        game_over = 0;
        SDL_Event event;

        while (!game_over) {
            while (SDL_PollEvent(&event)) {
                handleInput(event);
            }

            drawGame(gRenderer, window);
            updateGame();

            if (snake1.lives <= 0 || (multiplayer && snake2.lives <= 0))
                game_over = 1;

            usleep(speed_delay);
        }

        const char *winner = (!multiplayer || snake1.score > snake2.score) ? "Player 1" : "Player 2";
        saveScore(winner);
        showLeaderboard(gRenderer);
    }

    unloadTextures();
    SDL_DestroyRenderer(gRenderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
    return 0;
}