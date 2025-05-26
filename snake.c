#include "snake.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <SDL2/SDL_image.h>
#include <time.h>

// Variabile globale și efecte temporare
Snake snake1, snake2;
Fruit fruits[MAX_FRUITS];
Obstacle obstacles[MAX_OBSTACLES];
int num_fruits = 0, num_obstacles = 0;
int game_over = 0, multiplayer = 0, speed_delay = 200000;
static int fruits_eaten = 0;
static time_t watermelon_effect_until = 0;

SDL_Texture *tex_snake_head = NULL;
SDL_Texture *tex_snake_body = NULL;
SDL_Texture *tex_apple = NULL;
SDL_Texture *tex_pear = NULL;
SDL_Texture *tex_cherry = NULL;
SDL_Texture *tex_watermelon = NULL;
SDL_Texture *tex_blue_apricot = NULL;
SDL_Texture *tex_cactus = NULL;
SDL_Texture *info_background = NULL;
TTF_Font *game_font = NULL;

SDL_Texture* loadImage(SDL_Renderer *renderer, const char *path) {
    SDL_Surface *surf = IMG_Load(path);
    if (!surf) {
        printf("IMG_Load failed: %s\n", IMG_GetError());
        return NULL;
    }
    SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_FreeSurface(surf);
    return tex;
}

SDL_Texture* renderText(SDL_Renderer *renderer, const char *text, SDL_Color color) {
    SDL_Surface *surf = TTF_RenderText_Blended(game_font, text, color);
    if (!surf) return NULL;
    SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_FreeSurface(surf);
    return tex;
}

int loadTextures(SDL_Renderer *renderer) {
    tex_snake_head = loadImage(renderer, "assets/snake_head.png");
    tex_snake_body = loadImage(renderer, "assets/snake_body.png");
    tex_apple      = loadImage(renderer, "assets/fruit_apple.png");
    tex_pear       = loadImage(renderer, "assets/fruit_pear.png");
    tex_cherry     = loadImage(renderer, "assets/fruit_cherry.png");
    tex_watermelon = loadImage(renderer, "assets/watermelon.png");
    tex_blue_apricot = loadImage(renderer, "assets/fruit.png");
    tex_cactus     = loadImage(renderer, "assets/obstacle_cactus.png");
    info_background= loadImage(renderer, "assets/fruit_info.png");

    game_font = TTF_OpenFont("assets/OpenSans-Regular.ttf", 18);
    if (!game_font) {
        printf("Font load failed: %s\n", TTF_GetError());
        return 0;
    }

    return tex_snake_head && tex_snake_body && tex_apple &&
           tex_pear && tex_cherry && tex_cactus && info_background &&
           tex_watermelon && tex_blue_apricot;
}

void unloadTextures() {
    SDL_DestroyTexture(tex_snake_head);
    SDL_DestroyTexture(tex_snake_body);
    SDL_DestroyTexture(tex_apple);
    SDL_DestroyTexture(tex_pear);
    SDL_DestroyTexture(tex_cherry);
    SDL_DestroyTexture(tex_watermelon);
    SDL_DestroyTexture(tex_blue_apricot);
    SDL_DestroyTexture(tex_cactus);
    SDL_DestroyTexture(info_background);

    if (game_font) TTF_CloseFont(game_font);
    TTF_Quit();
}

void drawTexture(SDL_Renderer *renderer, SDL_Texture *tex, int grid_x, int grid_y) {
    SDL_Rect dest = { grid_x * SCALE, grid_y * SCALE, SCALE, SCALE };
    SDL_RenderCopy(renderer, tex, NULL, &dest);
}

void drawGame(SDL_Renderer *renderer, SDL_Window *window) {
    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
    SDL_RenderClear(renderer);

    
    SDL_SetRenderDrawColor(renderer, 80, 80, 80, 255);
    SDL_Rect b;
    b = (SDL_Rect){0, 0, WIDTH * SCALE, SCALE}; SDL_RenderFillRect(renderer, &b); // top
    b.y = (HEIGHT - 1) * SCALE; SDL_RenderFillRect(renderer, &b); // bottom
    b = (SDL_Rect){0, 0, SCALE, HEIGHT * SCALE}; SDL_RenderFillRect(renderer, &b); // left
    b.x = (WIDTH - 1) * SCALE; SDL_RenderFillRect(renderer, &b); // right

    for (int i = 0; i < num_obstacles; i++)
        if (obstacles[i].active)
            drawTexture(renderer, tex_cactus, obstacles[i].x, obstacles[i].y);

    for (int i = 0; i < num_fruits; i++) {
        SDL_Texture *fruitTex = tex_apple;
        if (fruits[i].type == 1) fruitTex = tex_pear;
        else if (fruits[i].type == 2 || fruits[i].type == 5) fruitTex = tex_cherry;
        else if (fruits[i].type == 3) fruitTex = tex_watermelon;
        else if (fruits[i].type == 4) fruitTex = tex_blue_apricot;
        drawTexture(renderer, fruitTex, fruits[i].x, fruits[i].y);
    }

    for (int i = 0; i < snake1.length; i++)
        drawTexture(renderer, i == 0 ? tex_snake_head : tex_snake_body, snake1.x[i], snake1.y[i]);

    if (multiplayer)
        for (int i = 0; i < snake2.length; i++)
            drawTexture(renderer, i == 0 ? tex_snake_head : tex_snake_body, snake2.x[i], snake2.y[i]);

    SDL_RenderPresent(renderer);
}

void showInfoMenu(SDL_Renderer *renderer) {
    int running = 1;
    SDL_Event e;

    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                game_over = 1;
                return;
            }
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
                running = 0;
            }
        }

        SDL_SetRenderDrawColor(renderer, 240, 240, 240, 255);
        SDL_RenderClear(renderer);
        SDL_Rect dest = { (WIDTH * SCALE - 480) / 2, (HEIGHT * SCALE - 300) / 2, 480, 300 };
        SDL_RenderCopy(renderer, info_background, NULL, &dest);
        SDL_RenderPresent(renderer);
    }
}

void initGame() {
    srand(time(NULL));
    resetSnake(&snake1, WIDTH / 4, HEIGHT / 2);
    if (multiplayer)
        resetSnake(&snake2, 3 * WIDTH / 4, HEIGHT / 2);

    num_fruits = 0;
    num_obstacles = 0;
    spawnFruit(3);
    for (int i = 0; i < 5; i++) spawnObstacle();
}

void resetSnake(Snake *snake, int startX, int startY) {
    snake->length = 5;
    snake->lives = 3;
    snake->score = 0;
    snake->dir = STOP;
    snake->speed_boost = 0;
    snake->invincible_until = 0;
    for (int i = 0; i < snake->length; i++) {
        snake->x[i] = startX - i;
        snake->y[i] = startY;
    }
}

void spawnFruit(int count) {
    while (num_fruits < 3 && count > 0) {
        fruits[num_fruits].x = rand() % (WIDTH - 2) + 1;
        fruits[num_fruits].y = rand() % (HEIGHT - 2) + 1;
        fruits[num_fruits].type = rand() % 6;
        fruits[num_fruits].timer = 0;
        num_fruits++;
        count--;
    }
}

void spawnObstacle() {
    if (num_obstacles >= MAX_OBSTACLES) return;
    obstacles[num_obstacles].x = rand() % (WIDTH - 2) + 1;
    obstacles[num_obstacles].y = rand() % (HEIGHT - 2) + 1;
    obstacles[num_obstacles].active = 1;
    num_obstacles++;
}

void moveSnake(Snake *snake) {
    int next_x = snake->x[0];
    int next_y = snake->y[0];
    int is_invincible = time(NULL) < snake->invincible_until;

    switch (snake->dir) {
        case LEFT:  next_x--; break;
        case RIGHT: next_x++; break;
        case UP:    next_y--; break;
        case DOWN:  next_y++; break;
        default: return;
    }

    if (is_invincible) {
        int collision = 0;
        if (next_x <= 0 || next_x >= WIDTH || next_y <= 0 || next_y >= HEIGHT)
            collision = 1;
        for (int i = 0; i < num_obstacles; i++)
            if (obstacles[i].active && next_x == obstacles[i].x && next_y == obstacles[i].y)
                collision = 1;
        if (collision) {
            snake->dir = LEFT;
            return;
        }
    }

    for (int i = snake->length - 1; i > 0; i--) {
        snake->x[i] = snake->x[i - 1];
        snake->y[i] = snake->y[i - 1];
    }

    snake->x[0] = next_x;
    snake->y[0] = next_y;
}

int checkCollision(Snake *snake) {
    if (snake->x[0] <= 0 || snake->x[0] >= WIDTH || snake->y[0] <= 0 || snake->y[0] >= HEIGHT)
        return 1;
    for (int i = 1; i < snake->length; i++)
        if (snake->x[0] == snake->x[i] && snake->y[0] == snake->y[i])
            return 1;
    for (int i = 0; i < num_obstacles; i++)
        if (obstacles[i].active && snake->x[0] == obstacles[i].x && snake->y[0] == obstacles[i].y)
            return 1;
    return 0;
}

void applyFruitEffect(Snake *snake, Fruit *fruit) {
    switch (fruit->type) {
        case 0: // Apple
            snake->score++;
            snake->length++;
            break;
        case 1: // Pear
            snake->score += 10;
            snake->length += 4;
            break;
        case 2: // Cherry - invincibility
            snake->invincible_until = time(NULL) + 10;
            break;
        case 3: // Watermelon - speed x5 + double obstacles
            snake->length += 2;
            speed_delay = speed_delay / 3;
            watermelon_effect_until = time(NULL) + 10;
            for (int i = 0; i < num_obstacles && num_obstacles < MAX_OBSTACLES; i++) {
                if (obstacles[i].active) {
                    obstacles[num_obstacles].x = obstacles[i].x + 1;
                    obstacles[num_obstacles].y = obstacles[i].y + 1;
                    obstacles[num_obstacles].active = 1;
                    num_obstacles++;
                }
            }
            break;
        case 4: //  apricot - slows and repositions
            speed_delay *= 2;
            for (int i = 0; i < num_obstacles; i++) {
                obstacles[i].x = rand() % (WIDTH - 2) + 1;
                obstacles[i].y = rand() % (HEIGHT - 2) + 1;
            }
            break;
        case 5: 
            snake->score -= 2;
            break;
    }

    fruits_eaten++;
    if (fruits_eaten == 3) {
        for (int i = 0; i < num_obstacles; i++) obstacles[i].active = 0;
        num_obstacles = 0;
        for (int i = 0; i < 7; i++) spawnObstacle();
    }
}

void updateGame() {
    moveSnake(&snake1);
    if (multiplayer) moveSnake(&snake2);

    // expiră efectul pepene?
    if (time(NULL) > watermelon_effect_until && watermelon_effect_until != 0) {
        speed_delay = 180000;
        watermelon_effect_until = 0;
    }

    if (time(NULL) >= snake1.invincible_until && checkCollision(&snake1)) {
        snake1.lives--;
        resetSnake(&snake1, WIDTH / 4, HEIGHT / 2);
    }
    if (multiplayer && time(NULL) >= snake2.invincible_until && checkCollision(&snake2)) {
        snake2.lives--;
        resetSnake(&snake2, 3 * WIDTH / 4, HEIGHT / 2);
    }

    for (int i = 0; i < num_fruits;) {
        int eaten = 0;
        if (snake1.x[0] == fruits[i].x && snake1.y[0] == fruits[i].y) {
            applyFruitEffect(&snake1, &fruits[i]);
            eaten = 1;
        } else if (multiplayer && snake2.x[0] == fruits[i].x && snake2.y[0] == fruits[i].y) {
            applyFruitEffect(&snake2, &fruits[i]);
            eaten = 1;
        }
        if (eaten) {
            fruits[i] = fruits[num_fruits - 1];
            num_fruits--;
        } else {
            i++;
        }
    }

    if (num_fruits <= 1)
        spawnFruit(2);
}

void saveScore(const char *winner) {
    FILE *f = fopen("leaderboard.txt", "a");
    if (f) {
        fprintf(f, "Winner: %s, Score: %d\n", winner,
                (strcmp(winner, "Player 1") == 0) ? snake1.score : snake2.score);
        fclose(f);
    }
}

void showLeaderboard(SDL_Renderer *renderer) {
    FILE *f = fopen("leaderboard.txt", "r");
    if (!f) return;

    SDL_Event e;
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);

    char line[256];
    int y = 30;
    SDL_Color black = {0, 0, 0};

    while (fgets(line, sizeof(line), f) && y < HEIGHT * SCALE - 40) {
        SDL_Texture *text = renderText(renderer, line, black);
        if (text) {
            SDL_Rect dest = {30, y, 0, 0};
            SDL_QueryTexture(text, NULL, NULL, &dest.w, &dest.h);
            SDL_RenderCopy(renderer, text, NULL, &dest);
            SDL_DestroyTexture(text);
            y += dest.h + 5;
        }
    }

    fclose(f);
    SDL_RenderPresent(renderer);

    while (SDL_WaitEvent(&e)) {
        if (e.type == SDL_QUIT) {
            game_over = 1;
            return;
        }
        if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_RETURN)
            break;
    }
}

int showMainMenu(SDL_Renderer *renderer) {
    const char *options[] = {"Singleplayer", "Multiplayer", "Exit"};
    int selected = 0;
    SDL_Event e;
    SDL_Color white = {255, 255, 255}, black = {0, 0, 0};

    while (1) {
        SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
        SDL_RenderClear(renderer);

        for (int i = 0; i < 3; i++) {
            SDL_Color color = (i == selected) ? white : black;
            SDL_Texture *text = renderText(renderer, options[i], color);
            SDL_Rect dest = {WIDTH * SCALE / 2 - 100, 100 + i * 60, 0, 0};
            SDL_QueryTexture(text, NULL, NULL, &dest.w, &dest.h);
            SDL_RenderCopy(renderer, text, NULL, &dest);
            SDL_DestroyTexture(text);
        }

        SDL_RenderPresent(renderer);

        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                game_over = 1;
                exit(0);
            }
            if (e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.sym) {
                    case SDLK_UP: selected = (selected + 2) % 3; break;
                    case SDLK_DOWN: selected = (selected + 1) % 3; break;
                    case SDLK_RETURN:
                        if (selected == 0) multiplayer = 0;
                        else if (selected == 1) multiplayer = 1;
                        else exit(0);

                        // Selectare viteză implicită
                        speed_delay = 180000;
                        return 0;
                }
            }
        }
    }
}

int showPauseMenu(SDL_Renderer *renderer) {
    const char *options[] = {"Resume", "Exit to Main Menu", "Info"};
    int current = 0;
    int running = 1;
    SDL_Event e;
    SDL_Color white = {255, 255, 255}, black = {0, 0, 0};

    while (running) {
        SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
        SDL_RenderClear(renderer);

        for (int i = 0; i < 3; i++) {
            SDL_Color color = (i == current) ? black : white;
            SDL_Texture *text = renderText(renderer, options[i], color);
            SDL_Rect dest = {WIDTH * SCALE / 2 - 100, 100 + i * 50, 0, 0};
            SDL_QueryTexture(text, NULL, NULL, &dest.w, &dest.h);
            SDL_RenderCopy(renderer, text, NULL, &dest);
            SDL_DestroyTexture(text);
        }

        SDL_RenderPresent(renderer);

        while (SDL_WaitEvent(&e)) {
            if (e.type == SDL_QUIT) {
                game_over = 1;
                return -1;
            }
            if (e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.sym) {
                    case SDLK_UP: current = (current + 2) % 3; break;
                    case SDLK_DOWN: current = (current + 1) % 3; break;
                    case SDLK_RETURN:
                        if (current == 0) return 0; // Resume
                        if (current == 1) return 1; // Exit
                        if (current == 2) showInfoMenu(renderer); break;
                    case SDLK_ESCAPE: return 0;
                }
                break;
            }
        }
    }

    return 0;
}
