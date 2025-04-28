#include <ncurses.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

#define WIDTH 40
#define HEIGHT 20
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
    int invincible;
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


Snake snake1;
Fruit fruits[MAX_FRUITS];
Obstacle obstacles[MAX_OBSTACLES];

void resetSnake(Snake *snake, int startX, int startY) {
    snake->length = 5;
    snake->lives = 3;
    snake->score = 0;
    snake->dir = STOP;
    snake->speed_boost = 0;
    snake->invincible = 0;
    for (int i = 0; i < snake->length; i++) {
        snake->x[i] = startX - i;
        snake->y[i] = startY;
    }
}

void spawnFruit() {
    static int num_fruits = 0;
    if (num_fruits >= MAX_FRUITS) return;
    fruits[num_fruits].x = rand() % (WIDTH - 2) + 1;
    fruits[num_fruits].y = rand() % (HEIGHT - 2) + 1;
    fruits[num_fruits].type = rand() % 6;
    fruits[num_fruits].timer = 0;
    num_fruits++;
}

void spawnObstacle() {
    static int num_obstacles = 0;
    if (num_obstacles >= MAX_OBSTACLES) return;
    obstacles[num_obstacles].x = rand() % (WIDTH - 2) + 1;
    obstacles[num_obstacles].y = rand() % (HEIGHT - 2) + 1;
    obstacles[num_obstacles].active = 1;
    num_obstacles++;
}

void drawGame() {
    clear();
    for (int i = 0; i <= WIDTH; i++) {
        mvprintw(0, i, "-");
        mvprintw(HEIGHT, i, "-");
    }
    for (int i = 0; i <= HEIGHT; i++) {
        mvprintw(i, 0, "|");
        mvprintw(i, WIDTH, "|");
    }
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        if (obstacles[i].active) {
            mvprintw(obstacles[i].y, obstacles[i].x, "X");
        }
    }
    for (int i = 0; i < MAX_FRUITS; i++) {
        if (fruits[i].x != 0 && fruits[i].y != 0) {
            char c = 'F';
            switch (fruits[i].type) {
                case 1: c = 'R'; break;
                case 2: c = 'B'; break;
                case 3: c = 'G'; break;
                case 4: c = 'N'; break;
                case 5: c = 'Y'; break;
            }
            mvprintw(fruits[i].y, fruits[i].x, "%c", c);
        }
    }
    for (int i = 0; i < snake1.length; i++) {
        mvprintw(snake1.y[i], snake1.x[i], "O");
    }
    mvprintw(HEIGHT + 2, 0, "Player Score: %d Lives: %d", snake1.score, snake1.lives);
    refresh();
}

void moveSnake(Snake *snake) {
    for (int i = snake->length - 1; i > 0; i--) {
        snake->x[i] = snake->x[i - 1];
        snake->y[i] = snake->y[i - 1];
    }
    switch (snake->dir) {
        case LEFT: snake->x[0]--; break;
        case RIGHT: snake->x[0]++; break;
        case UP: snake->y[0]--; break;
        case DOWN: snake->y[0]++; break;
        default: break;
    }
}

int checkCollision(Snake *snake) {
    if (snake->x[0] <= 0 || snake->x[0] >= WIDTH || snake->y[0] <= 0 || snake->y[0] >= HEIGHT)
        return 1;
    for (int i = 1; i < snake->length; i++) {
        if (snake->x[0] == snake->x[i] && snake->y[0] == snake->y[i])
            return 1;
    }
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        if (obstacles[i].active && snake->x[0] == obstacles[i].x && snake->y[0] == obstacles[i].y)
            return 1;
    }
    return 0;
}

void applyFruitEffect(Snake *snake, int *fruits_eaten) {
    for (int i = 0; i < MAX_FRUITS; i++) {
        if (snake->x[0] == fruits[i].x && snake->y[0] == fruits[i].y) {
            switch (fruits[i].type) {
                case 0: snake->score++; snake->length++; break;
                case 1: snake->score += 2; snake->length += 2; break;
                case 2: snake->speed_boost = 10; break;
                case 3: snake->invincible = 10; break;
                case 5: snake->score -= 2; break;
            }
            fruits[i].x = fruits[i].y = 0;
            (*fruits_eaten)++;
            if ((*fruits_eaten) % 5 == 0) {
                for (int j = 0; j < MAX_OBSTACLES; j++) {
                    obstacles[j].active = 0;
                }
                for (int j = 0; j < 8; j++) {
                    spawnObstacle();
                }
            }
            spawnFruit();
        }
    }
}

void *inputHandler(void *arg) {
    int *game_over = (int *)arg;
    int ch;
    while (!(*game_over)) {
        ch = getch();
        switch (ch) {
            case 'w': if (snake1.dir != DOWN) snake1.dir = UP; break;
            case 's': if (snake1.dir != UP) snake1.dir = DOWN; break;
            case 'a': if (snake1.dir != RIGHT) snake1.dir = LEFT; break;
            case 'd': if (snake1.dir != LEFT) snake1.dir = RIGHT; break;
            case 'q': *game_over = 1; break;
        }
        usleep(10000);
    }
    return NULL;
}

void saveScore() {
    FILE *f = fopen("leaderboard.txt", "a");
    if (f) {
        fprintf(f, "Score: %d\n", snake1.score);
        fclose(f);
    }
}

void showMenu(int *speed_delay) {
    int choice;
    printf("Singleplayer Snake\n");
    printf("Select speed:\n1. Slow\n2. Normal\n3. Fast\nChoice: ");
    scanf("%d", &choice);
    if (choice == 1) *speed_delay = 300000;
    if (choice == 2) *speed_delay = 200000;
    if (choice == 3) *speed_delay = 100000;
}

int main() {
    int game_over = 0;
    int speed_delay;
    int fruits_eaten = 0;
    pthread_t input_thread;

    showMenu(&speed_delay);

    initscr();
    noecho();
    curs_set(FALSE);
    keypad(stdscr, TRUE);
    timeout(0);
    srand(time(NULL));

    resetSnake(&snake1, WIDTH / 4, HEIGHT / 2);

    spawnFruit();
    for (int i = 0; i < 3; i++) {
        spawnObstacle();
    }

    pthread_create(&input_thread, NULL, inputHandler, &game_over);

    while (!game_over) {
        drawGame();
        moveSnake(&snake1);

        if (!snake1.invincible && checkCollision(&snake1)) {
            snake1.lives--;
            resetSnake(&snake1, WIDTH/4, HEIGHT/2);
        }

        applyFruitEffect(&snake1, &fruits_eaten);

        if (snake1.lives <= 0)
            game_over = 1;

        usleep(speed_delay);
    }

    endwin();
    saveScore();
    return 0;
}

