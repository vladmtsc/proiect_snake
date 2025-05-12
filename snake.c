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

static Snake snake1, snake2;
static Fruit fruits[MAX_FRUITS];
static Obstacle obstacles[MAX_OBSTACLES];
static int num_fruits = 0;
static int num_obstacles = 0;
static int game_over = 0;
static int multiplayer = 0;
static int speed_delay = 200000;
static int fruits_eaten = 0;
static pthread_mutex_t input_mutex;

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
    while (num_fruits < 3) {
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

void initGame() {
    initscr(); noecho(); curs_set(FALSE); keypad(stdscr, TRUE); timeout(0);
    srand(time(NULL));

    resetSnake(&snake1, WIDTH / 4, HEIGHT / 2);
    if (multiplayer) resetSnake(&snake2, 3 * WIDTH / 4, HEIGHT / 2);

    num_fruits = 0;
    num_obstacles = 0;
    spawnFruit(3);
    for (int i = 0; i < 5; i++) spawnObstacle();
}


void drawGame() {
    clear();

    
    mvprintw(0, 0, "┌");
    mvprintw(0, WIDTH, "┐");
    mvprintw(HEIGHT, 0, "└");
    mvprintw(HEIGHT, WIDTH, "┘");

    for (int i = 1; i < WIDTH; i++) {
        mvprintw(0, i, "-");
        mvprintw(HEIGHT, i, "-");
    }

    for (int i = 1; i < HEIGHT; i++) {
        mvprintw(i, 0, "|");
        mvprintw(i, WIDTH, "|");
    }


    for (int i = 0; i < num_obstacles; i++)
        if (obstacles[i].active)
            mvprintw(obstacles[i].y, obstacles[i].x, "X");

 
    for (int i = 0; i < num_fruits; i++) {
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

   
    for (int i = 0; i < snake1.length; i++) {
        if (i == 0)
            mvprintw(snake1.y[i], snake1.x[i], "@");  
        else
            mvprintw(snake1.y[i], snake1.x[i], "0");  
    }

   
    if (multiplayer) {
        for (int i = 0; i < snake2.length; i++) {
            if (i == 0)
                mvprintw(snake2.y[i], snake2.x[i], "#");
            else
                mvprintw(snake2.y[i], snake2.x[i], "+");
        }
    }


    mvprintw(HEIGHT + 2, 0, "Player 1 Score: %d Lives: %d", snake1.score, snake1.lives);
    if (multiplayer)
        mvprintw(HEIGHT + 3, 0, "Player 2 Score: %d Lives: %d", snake2.score, snake2.lives);

    refresh();
}
void moveSnake(Snake *snake) {
    int next_x = snake->x[0];
    int next_y = snake->y[0];
    switch (snake->dir) {
        case LEFT:  next_x--; break;
        case RIGHT: next_x++; break;
        case UP:    next_y--; break;
        case DOWN:  next_y++; break;
        default: break;
    }

    int is_invincible = time(NULL) < snake->invincible_until;

    if (is_invincible) {
        if (next_x <= 0 || next_x >= WIDTH || next_y <= 0 || next_y >= HEIGHT) {
            snake->dir = RIGHT;
            return;
        }
        for (int i = 0; i < num_obstacles; i++) {
            if (obstacles[i].active && next_x == obstacles[i].x && next_y == obstacles[i].y) {
                snake->dir = RIGHT;
                return;
            }
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
        case 0: snake->score++; snake->length++; break;
        case 1: snake->score += 10; snake->length += 4; break;
        case 2: snake->speed_boost = 40; break;
        case 3: snake->invincible_until = time(NULL) + 10; break;
        case 4: if (multiplayer) snake2.speed_boost = -10; break;
        case 5: snake->score -= 2; break;
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

    if (time(NULL) >= snake1.invincible_until && checkCollision(&snake1)) {
        snake1.lives--;
        resetSnake(&snake1, WIDTH/4, HEIGHT/2);
    }
    if (multiplayer && time(NULL) >= snake2.invincible_until && checkCollision(&snake2)) {
        snake2.lives--;
        resetSnake(&snake2, 3*WIDTH/4, HEIGHT/2);
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
    if(num_fruits==1)
    spawnFruit(2);
}

void *inputHandler(void *arg) {
    int ch;
    while (!game_over) {
        pthread_mutex_lock(&input_mutex);
        ch = getch();
        switch (ch) {
            case 'w': if (snake1.dir != DOWN) snake1.dir = UP; break;
            case 's': if (snake1.dir != UP) snake1.dir = DOWN; break;
            case 'a': if (snake1.dir != RIGHT) snake1.dir = LEFT; break;
            case 'd': if (snake1.dir != LEFT) snake1.dir = RIGHT; break;
            case KEY_UP: if (snake2.dir != DOWN) snake2.dir = UP; break;
            case KEY_DOWN: if (snake2.dir != UP) snake2.dir = DOWN; break;
            case KEY_LEFT: if (snake2.dir != RIGHT) snake2.dir = LEFT; break;
            case KEY_RIGHT: if (snake2.dir != LEFT) snake2.dir = RIGHT; break;
            case 'q': game_over = 1; break;
        }
        pthread_mutex_unlock(&input_mutex);
        usleep(10000);
    }
    return NULL;
}

void saveScore(const char *winner) {
    FILE *f = fopen("leaderboard.txt", "a");
    if (f) {
        fprintf(f, "Winner: %s, Score: %d\n", winner, (strcmp(winner, "Player 1") == 0) ? snake1.score : snake2.score);
        fclose(f);
    }
}

void showMenu() {
    int choice;
    printf("1. Singleplayer\n2. Multiplayer\n3. Exit\nChoice: ");
    scanf("%d", &choice);
    if (choice == 1) multiplayer = 0;
    else if (choice == 2) multiplayer = 1;
    else exit(0);

    printf("Select speed:\n1. Slow\n2. Normal\n3. Fast\nChoice: ");
    scanf("%d", &choice);
    if (choice == 1) speed_delay = 300000;
    if (choice == 2) speed_delay = 200000;
    if (choice == 3) speed_delay = 100000;
}

int main() {
    pthread_t input_thread;
    pthread_mutex_init(&input_mutex, NULL);

    showMenu();
    initGame();
    pthread_create(&input_thread, NULL, inputHandler, NULL);

    while (!game_over) {
        drawGame();
        updateGame();
        if (snake1.lives <= 0 || (multiplayer && snake2.lives <= 0))
            game_over = 1;
        usleep(speed_delay);
    }

    endwin();
    pthread_mutex_destroy(&input_mutex);

    printf("\nGame Over!\n");
    if (!multiplayer || snake1.score > snake2.score) {
        printf("Winner: Player 1\n");
        saveScore("Player 1");
    } else {
        printf("Winner: Player 2\n");
        saveScore("Player 2");
    }
    return 0;
}

