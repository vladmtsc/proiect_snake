#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <time.h>

#define WIDTH  40
#define HEIGHT 20
#define MAX_LENGTH 100
#define NUM_LIVES 3
#define MAX_OBSTACLES 5

typedef struct {
    int x, y;
} Point;

typedef struct {
    Point body[MAX_LENGTH];
    int length;
    char direction;
    int lives;
    int score;
} Snake;

Snake snake1, snake2;
Point fruit, powerup, lifeup;
Point obstacles[MAX_OBSTACLES];
int num_obstacles = 0;
int speed = 100000;
int game_over = 0;
int multiplayer = 0;

WINDOW *game_win; 

void init_snake(Snake *snake, int start_x, int start_y, char dir) {
    snake->length = 3;
    snake->direction = dir;
    snake->lives = NUM_LIVES;
    snake->score = 0;
    for (int i = 0; i < snake->length; i++) {
        snake->body[i].x = start_x - i;
        snake->body[i].y = start_y;
    }
}

int main(void)
{
    return 0;
}
