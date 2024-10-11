#include <Wire.h>
#include <LiquidCrystal.h>

// Macros for LCD and board dimensions
#define LCD_W 16
#define LCD_H 2
#define BOARD_W 80
#define BOARD_H 16

// Macros for pin assignments
#define PIN_LCD_RS 8
#define PIN_LCD_E 9
#define PIN_LCD_D4 4
#define PIN_LCD_D5 5
#define PIN_LCD_D6 6
#define PIN_LCD_D7 7

// Updated button pin assignments
#define BTN_UP 12
#define BTN_DOWN 11
#define BTN_LEFT 10
#define BTN_RIGHT 13

#define SNAKE_SPEED 800 // Snake speed (delay in ms between moves)

// Initialize the LCD
LiquidCrystal lcd(PIN_LCD_RS, PIN_LCD_E, PIN_LCD_D4, PIN_LCD_D5, PIN_LCD_D6, PIN_LCD_D7);

// Static variables for game state
static int snakeX[256], snakeY[256];  // Snake's body positions
static int snakeLength = 3;           // Starting length of the snake
static int foodX, foodY;              // Food position
static int dirX = 1, dirY = 0;        // Snake's direction
static bool gameOver = false;         // Game over flag

// Static array for custom characters
static uint8_t customChar[LCD_W * LCD_H][8];  // Custom characters for each LCD block

void setup() {
  lcd.begin(LCD_W, LCD_H);   // Initialize the LCD
  setupCustomChars();        // Initialize custom characters

  // Setup button pins
  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);
  pinMode(BTN_LEFT, INPUT_PULLUP);
  pinMode(BTN_RIGHT, INPUT_PULLUP);

  startGame();
}

void loop() {
  if (!gameOver) {
    checkButtons();
    moveSnake();
    checkCollision();
    drawSnake();
    drawFood();
    delay(SNAKE_SPEED);  // Adjust speed of snake
  } else {
    lcd.clear();
    lcd.setCursor(4, 0);
    lcd.print("Game Over");
    while (1);  // Stop game when over
  }
}

// Initialize custom characters to all empty
void setupCustomChars() {
  for (int i = 0; i < LCD_W * LCD_H; i++) {
    uint8_t empty[8] = {0, 0, 0, 0, 0, 0, 0, 0};  
    updateCustomChar(i, empty);
  }
}

// Update custom character at index with pixel data (8x5)
void updateCustomChar(int index, uint8_t pixelData[8]) {
  for (int i = 0; i < 8; i++) {
    customChar[index][i] = pixelData[i];
  }
  lcd.createChar(index, customChar[index]);
}

// Set or clear a pixel on the 80x16 grid
void setPixel(int x, int y, bool on) {
  int charX = x / 5;    // Each character is 5 pixels wide
  int charY = y / 8;    // Each character is 8 pixels tall
  int pixelX = x % 5;   // Pixel position within the character
  int pixelY = y % 8;   // Row position within the character

  // Ensure pixel is within the LCD's bounds
  if (charX < LCD_W && charY < LCD_H) {
    int charIndex = charY * LCD_W + charX;  // Character index
    uint8_t currentState = customChar[charIndex][pixelY] & (1 << (4 - pixelX));

    // Check if the pixel is already in the desired state
    if ((on && currentState) || (!on && !currentState)) {
      return;  // Return early if the pixel is already in the same state
    }

    // Set or clear the pixel
    if (on) {
      customChar[charIndex][pixelY] |= (1 << (4 - pixelX));  // Set pixel
    } else {
      customChar[charIndex][pixelY] &= ~(1 << (4 - pixelX)); // Clear pixel
    }

    lcd.createChar(charIndex, customChar[charIndex]);  // Update LCD character
    lcd.setCursor(charX, charY); // Move cursor
    lcd.write(charIndex);        // Write character to the screen
  }
}

// Draw the snake on the screen
void drawSnake() {
  for (int i = 0; i < snakeLength; i++) {
    setPixel(snakeX[i], snakeY[i], true);
  }
}

// Place the food on the screen
void drawFood() {
  setPixel(foodX, foodY, true);
}

// Move the snake
void moveSnake() {
  setPixel(snakeX[snakeLength - 1], snakeY[snakeLength - 1], false);
  
  for (int i = snakeLength - 1; i > 0; i--) {
    snakeX[i] = snakeX[i - 1];
    snakeY[i] = snakeY[i - 1];
  }

  // Move the snake's head
  snakeX[0] += dirX;
  snakeY[0] += dirY;

  // Check if the snake eats the food
  if (snakeX[0] == foodX && snakeY[0] == foodY) {
    snakeLength++;  // Grow snake
    placeFood();    // Place new food
  }
}

// Check for collisions with walls or the snake itself
void checkCollision() {
  // Check wall collision
  if (snakeX[0] < 0 || snakeX[0] >= BOARD_W || snakeY[0] < 0 || snakeY[0] >= BOARD_H) {
    gameOver = true;
  }
  
  // Check self-collision
  for (int i = 1; i < snakeLength; i++) {
    if (snakeX[0] == snakeX[i] && snakeY[0] == snakeY[i]) {
      gameOver = true;
    }
  }
}

// Handle button inputs
void checkButtons() {
  if (digitalRead(BTN_UP)    == LOW && dirY !=  1) { dirX =  0; dirY = -1; }
  if (digitalRead(BTN_DOWN)  == LOW && dirY != -1) { dirX =  0; dirY =  1; }
  if (digitalRead(BTN_LEFT)  == LOW && dirX !=  1) { dirX = -1; dirY =  0; }
  if (digitalRead(BTN_RIGHT) == LOW && dirX != -1) { dirX =  1; dirY =  0; }
}

// Initialize the game state
void startGame() {
  snakeX[0] = 10;
  snakeY[0] = 5;      // Snake's starting position
  snakeLength = 3;
  placeFood();         // Place the initial food
}

// Place food at a random position
void placeFood() {
  foodX = random(0, BOARD_W / 2);
  foodY = random(0, BOARD_H / 2);
}
