#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ncurses.h>

#define WIDTH 30
#define HEIGHT 15
#define HIGH_SCORE_FILE "highscore.txt"
#define BASKET_MOVE_STEP 3  // The basket moves 3 spaces per tick
#define BASKET_WIDTH 5      // The basket's holding range is now 5

typedef struct {
    int x, y;
    int speed; // Speed in milliseconds
} Fruit;

void draw(Fruit fruit, int basketX, int score, int highScore);
void updateFruit(Fruit *fruit, int basketX, int *score, int *gameOver);
int loadHighScore(const char *filename);
void saveHighScore(const char *filename, int highScore);

int main() {
    Fruit fruit;
    int basketX = WIDTH / 2;  // Basket starts at the center
    int score = 0;
    int gameOver = 0;
    int highScore = loadHighScore(HIGH_SCORE_FILE);
    char input;
//heheh
    srand(time(0));          // Initialize random seed
    initscr();               // Start ncurses mode
    noecho();                // Disable input echoing
    curs_set(0);             // Hide the cursor
    keypad(stdscr, TRUE);    // Enable arrow keys

    // Initialize the fruit
    fruit.x = rand() % WIDTH;
    fruit.y = 0;
    fruit.speed = 200 + rand() % 300; // Random speed between 200-500 ms

    while (!gameOver) {
        clear();             // Clear the screen
        draw(fruit, basketX, score, highScore);

        // Handle input
        timeout(fruit.speed);  // Set the input timeout to the fruit's speed
        input = getch();
        if (input == 'a' || input == KEY_LEFT) {
            if (basketX > 0) basketX -= BASKET_MOVE_STEP;
            if (basketX < 0) basketX = 0; // Prevent moving out of bounds
        } else if (input == 'd' || input == KEY_RIGHT) {
            if (basketX < WIDTH - BASKET_WIDTH) basketX += BASKET_MOVE_STEP;
            if (basketX > WIDTH - BASKET_WIDTH) basketX = WIDTH - BASKET_WIDTH; // Prevent moving out of bounds
        } else if (input == 'q') {
            break;          // Quit the game
        }

        // Update fruit and game state
        updateFruit(&fruit, basketX, &score, &gameOver);
    }

    // Check and save the high score
    if (score > highScore) {
        highScore = score;
        saveHighScore(HIGH_SCORE_FILE, highScore);
    }

    // End ncurses mode
    endwin();
    printf("Game Over! Your final score: %d\n", score);
    printf("High Score: %d\n", highScore);
    return 0;
}

void draw(Fruit fruit, int basketX, int score, int highScore) {
    // Draw the fruit
    mvprintw(fruit.y, fruit.x, "([*])");

    // Draw the basket
    for (int i = 0; i < BASKET_WIDTH; i++) {
        mvprintw(HEIGHT - 1, basketX + i, "V");
    }

    // Draw the score and high score
    mvprintw(HEIGHT, 0, "Score: %d", score);
    mvprintw(HEIGHT + 1, 0, "High Score: %d", highScore);
    mvprintw(HEIGHT + 2, 0, "Controls: 'a'/'LEFT' (left), 'd'/'RIGHT' (right), 'q' (quit)");
}

void updateFruit(Fruit *fruit, int basketX, int *score, int *gameOver) {
    if (fruit->y == HEIGHT - 1) {  // Check if fruit reached the basket level
        if (fruit->x >= basketX && fruit->x < basketX + BASKET_WIDTH) {
            (*score)++;  // Increase score for catching the fruit
        } else {
            *gameOver = 1;  // Missed the fruit, game over
        }
        // Reset fruit position and randomize speed
        fruit->y = 0;
        fruit->x = rand() % WIDTH;
        fruit->speed = 200 + rand() % 300; // Random speed between 200-500 ms
    } else {
        fruit->y++;  // Move fruit down
    }
}

int loadHighScore(const char *filename) {
    FILE *file = fopen(filename, "r");
    int highScore = 0;

    if (file) {
        fscanf(file, "%d", &highScore);
        fclose(file);
    }

    return highScore;
}

void saveHighScore(const char *filename, int highScore) {
    FILE *file = fopen(filename, "w");

    if (file) {
        fprintf(file, "%d", highScore);
        fclose(file);
    }
}
