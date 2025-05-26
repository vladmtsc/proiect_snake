#ifndef SNAKE_H
#define SNAKE_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <time.h>

#define WIDTH 40
#define HEIGHT 20
#define SCALE 32
#define MAX_LENGTH 100
#define MAX_OBSTACLES 20
#define MAX_FRUITS 5

typedef enum { STOP = 0, LEFT, RIGHT, UP, DOWN } Direction;

typedef struct {
    int x[MAX_LENGTH];
    int y[MAX_LENGTH];
    int length;
    int lives;
    int score;
    Direction dir;
    int speed_boost;
    time_t invincible_until;
} Snake;

typedef struct {
    int x, y;
    int type;
    int timer;
} Fruit;

typedef struct {
    int x, y;
    int active;
} Obstacle;

extern Snake snake1, snake2;
extern Fruit fruits[MAX_FRUITS];
extern Obstacle obstacles[MAX_OBSTACLES];
extern int num_fruits, num_obstacles, game_over, multiplayer, speed_delay;

extern SDL_Texture *tex_snake_head, *tex_snake_body;
extern SDL_Texture *tex_apple, *tex_pear, *tex_cherry;
extern SDL_Texture *tex_watermelon, *tex_blue_apricot;
extern SDL_Texture *tex_cactus, *info_background;
extern TTF_Font *game_font;

void resetSnake(Snake *snake, int startX, int startY);
void spawnFruit(int count);
void spawnObstacle();
void initGame();
void moveSnake(Snake *snake);
int checkCollision(Snake *snake);
void applyFruitEffect(Snake *snake, Fruit *fruit);
void updateGame();
void drawGame(SDL_Renderer *renderer, SDL_Window *window);
void saveScore(const char *winner);

int loadTextures(SDL_Renderer *renderer);
void unloadTextures();
void drawTexture(SDL_Renderer *renderer, SDL_Texture *tex, int grid_x, int grid_y);

int showMainMenu(SDL_Renderer *renderer);
int showPauseMenu(SDL_Renderer *renderer);
void showInfoMenu(SDL_Renderer *renderer);
void showLeaderboard(SDL_Renderer *renderer);
SDL_Texture* renderText(SDL_Renderer *renderer, const char *text, SDL_Color color);


#endif