#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ncurses/ncurses.h>
#include <unistd.h>

#define WIDTH 30
#define HEIGHT 15
#define HIGH_SCORE_FILE "highscore.txt"
#define LEADERBOARD_FILE "leaderboard.txt"
#define BASKET_MOVE_STEP 3  // The basket moves 3 spaces per tick
#define BASKET_WIDTH 5      // The basket's holding range is now 5
#define MAX_NAME_LENGTH 20
#define MAX_LEADERBOARD_ENTRIES 10

typedef struct {
    int x, y;
    int speed; // Speed in milliseconds
} Fruit;

typedef struct {
    char name[MAX_NAME_LENGTH];
    int score;
} LeaderboardEntry;

void draw(Fruit fruit, int basketX, int score, int highScore);
void updateFruit(Fruit *fruit, int basketX, int *score, int *gameOver);
int loadHighScore(const char *filename);
void saveHighScore(const char *filename, int highScore);
void displayMenu();
void displayGameOver(int score, int highScore, int qualifiesForLeaderboard);
void inputName(char *name, int maxLength);
void updateLeaderboard(const char *filename, const char *name, int score);
void displayLeaderboard(const char *filename);

int main() {
    Fruit fruit;
    int basketX = WIDTH / 2;  // Basket starts at the center
    int score = 0;
    int gameOver = 0;
    int highScore = loadHighScore(HIGH_SCORE_FILE);
    int input;

    srand(time(0));          // Initialize random seed
    initscr();               // Start ncurses mode
    noecho();                // Disable input echoing
    curs_set(0);             // Hide the cursor
    keypad(stdscr, TRUE);    // Enable arrow keys

    // Display the menu screen
    displayMenu();

    // Initialize the fruit
    fruit.x = rand() % WIDTH;
    fruit.y = 0;
    fruit.speed = 200 + rand() % 300; // Random speed between 200-500 ms

    while (1) {
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
                endwin();
                return 0;          // Quit the game
            }

            // Update fruit and game state
            updateFruit(&fruit, basketX, &score, &gameOver);
        }

        // Check if the score qualifies for leaderboard
        LeaderboardEntry entries[MAX_LEADERBOARD_ENTRIES];
        int count = 0;

        FILE *file = fopen(LEADERBOARD_FILE, "r");
        if (file) {
            while (fscanf(file, "%s %d", entries[count].name, &entries[count].score) == 2) {
                count++;
                if (count >= MAX_LEADERBOARD_ENTRIES) break;
            }
            fclose(file);
        }

        int qualifiesForLeaderboard = (count < MAX_LEADERBOARD_ENTRIES || score > entries[count - 1].score);

        // Display game over screen
        displayGameOver(score, highScore, qualifiesForLeaderboard);

        if (qualifiesForLeaderboard) {
            char name[MAX_NAME_LENGTH];
            inputName(name, MAX_NAME_LENGTH);
            updateLeaderboard(LEADERBOARD_FILE, name, score);
            clear();
            displayLeaderboard(LEADERBOARD_FILE);
        }

        // Ask the player if they want to try again or quit
        mvprintw(HEIGHT / 2 + 5, (WIDTH - 25) / 2, "Press 'p' to play again or 'q' to quit.");
        refresh();

        while (1) {
            input = getch();
            if (input == 'p') {
                // Reset game variables
                score = 0;
                gameOver = 0;
                basketX = WIDTH / 2;
                fruit.x = rand() % WIDTH;
                fruit.y = 0;
                fruit.speed = 200 + rand() % 300;
                break;
            } else if (input == 'q') {
                endwin();
                return 0;
            }
        }
    }

    endwin();
    return 0;
}



void displayMenu() {
    clear();
    mvprintw(HEIGHT / 2 - 5, (WIDTH - 5) / 2, "CATCH");
    mvprintw(HEIGHT / 2 - 3, (WIDTH - 3) / 2, "THE");
    mvprintw(HEIGHT / 2 - 1, (WIDTH - 5) / 2, "FRUIT");
    mvprintw(HEIGHT / 2 + 3, (WIDTH - 21) / 2, "Press any key to enter");
    refresh();
    getch();
}

void draw(Fruit fruit, int basketX, int score, int highScore) {
    // Draw the fruit
    mvprintw(fruit.y, fruit.x, "*");

    // Draw the basket
    for (int i = 0; i < BASKET_WIDTH; i++) {
        mvprintw(HEIGHT - 1, basketX + i, "_");
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

void displayGameOver(int score, int highScore, int qualifiesForLeaderboard) {
    clear();
    mvprintw(HEIGHT / 2 - 2, (WIDTH - 9) / 2, "GAME OVER");
    mvprintw(HEIGHT / 2, (WIDTH - 19) / 2, "Your final score: %d", score);
    mvprintw(HEIGHT / 2 + 1, (WIDTH - 13) / 2, "High Score: %d", highScore);

    if (qualifiesForLeaderboard) {
        mvprintw(HEIGHT / 2 + 2, (WIDTH - 25) / 2, "Congratulations! You're in the top 10!");
        mvprintw(HEIGHT / 2 + 4, (WIDTH - 40) / 2, "Enter your name and press Enter: ");
    } else {
        mvprintw(HEIGHT / 2 + 3, (WIDTH - 25) / 2, "Press 'p' to play again or 'q' to quit.");
    }

    mvprintw(HEIGHT + 3, 0, "Credits:");
    mvprintw(HEIGHT + 4, 0, "Jecer Egagamao");
    mvprintw(HEIGHT + 5, 0, "Maxwell Morales");
    mvprintw(HEIGHT + 6, 0, "Gian De La Cruz");

    refresh();
}

void inputName(char *name, int maxLength) {
    echo();  // Enable input echoing
    curs_set(1);  // Show the cursor

    mvprintw(HEIGHT / 2 + 2, (WIDTH - 25) / 2, "Enter your name: ");
    getnstr(name, maxLength - 1);

    noecho();  // Disable input echoing
    curs_set(0);  // Hide the cursor
}

void updateLeaderboard(const char *filename, const char *name, int score) {
    LeaderboardEntry entries[MAX_LEADERBOARD_ENTRIES];
    int count = 0;

    // Load existing leaderboard

    FILE *file = fopen(filename, "r");
    if (file) {
        while (fscanf(file, "%s %d", entries[count].name, &entries[count].score) == 2) {
            count++;
            if (count >= MAX_LEADERBOARD_ENTRIES) break;
        }
        fclose(file);
    }

    // Add the new score to the leaderboard
    if (count < MAX_LEADERBOARD_ENTRIES || score > entries[count - 1].score) {
        strncpy(entries[count].name, name, MAX_NAME_LENGTH - 1);
        entries[count].name[MAX_NAME_LENGTH - 1] = '\0'; // Ensure null termination
        entries[count].score = score;
        count++;
    }

    // Sort the leaderboard by score (descending)
    for (int i = 0; i < count - 1; i++) {
        for (int j = 0; j < count - i - 1; j++) {
            if (entries[j].score < entries[j + 1].score) {
                LeaderboardEntry temp = entries[j];
                entries[j] = entries[j + 1];
                entries[j + 1] = temp;
            }
        }
    }

    // Write the updated leaderboard back to the file
    file = fopen(filename, "w");
    if (file) {
        for (int i = 0; i < count && i < MAX_LEADERBOARD_ENTRIES; i++) {
            fprintf(file, "%s %d\n", entries[i].name, entries[i].score);
        }
        fclose(file);
    }
}

void displayLeaderboard(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        mvprintw(2, (WIDTH - 25) / 2, "No leaderboard data available.");
        return;
    }

    LeaderboardEntry entry;
    int row = 2;

    while (fscanf(file, "%s %d", entry.name, &entry.score) == 2) {
        mvprintw(row++, (WIDTH - 25) / 2, "%s - %d", entry.name, entry.score);
    }

    fclose(file);
}
