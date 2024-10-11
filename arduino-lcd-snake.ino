#define LCD_W_CHARS 16
#define LCD_H_CHARS 2

#define BOARD_W_CHARS 4
#define BOARD_H_CHARS 2

#define BOARD_W (BOARD_W_CHARS * 5)
#define BOARD_H (BOARD_H_CHARS * 8)

#define PIN_LCD_RS 8
#define PIN_LCD_E 9
#define PIN_LCD_D4 4
#define PIN_LCD_D5 5
#define PIN_LCD_D6 6
#define PIN_LCD_D7 7

#define PIN_BTN_UP 12
#define PIN_BTN_DOWN 11
#define PIN_BTN_LEFT 10
#define PIN_BTN_RIGHT 13

#define PIN_BUZZ 3

#define INITIAL_SNAKE_LENGTH 3
#define SNAKE_SPEED 600

#include <Wire.h>
#include <LiquidCrystal.h>

LiquidCrystal lcd(PIN_LCD_RS, PIN_LCD_E, PIN_LCD_D4, PIN_LCD_D5, PIN_LCD_D6, PIN_LCD_D7);

static int snakeX[256], snakeY[256];
static int snakeLength = INITIAL_SNAKE_LENGTH;
static int foodX, foodY;
static int dirX = 1, dirY = 0;
static bool gameOver = false;

static uint8_t customChar[LCD_W_CHARS * LCD_H_CHARS][8] = { 0 };

void drawBorder() {
  for (int x = 0; x < BOARD_W_CHARS; x++) {
    for (int y = 0; y < BOARD_H_CHARS; y++) {
      int index = x + y * BOARD_W_CHARS;
      if (x == 0)                 for (int i = 0; i < 8; i++) customChar[index][i] |= 0x70;
      if (x == BOARD_W_CHARS - 1) for (int i = 0; i < 8; i++) customChar[index][i] |= 0x01;
      if (y == 0)                 customChar[index][0] |= 0xff;
      if (y == BOARD_H_CHARS - 1) customChar[index][7] |= 0xff;
      lcd.createChar(index, customChar[index]);
      lcd.setCursor(x, y);
      lcd.write(index);
    }
  }
}

void drawPixel(int x, int y, bool on) {
  int charX = x / 5;
  int charY = y / 8;
  int pixelX = x % 5;
  int pixelY = y % 8;
  
  if (charX < BOARD_W_CHARS && charY < BOARD_H_CHARS) {
    int charIndex = charX + charY * BOARD_W_CHARS;
    uint8_t currentState = customChar[charIndex][pixelY] & (1 << (4 - pixelX));
    
    if ((on && currentState) || (!on && !currentState)) return;
    
    if (on) customChar[charIndex][pixelY] |= (1 << (4 - pixelX));
    else customChar[charIndex][pixelY] &= ~(1 << (4 - pixelX));

    lcd.createChar(charIndex, customChar[charIndex]);
    lcd.setCursor(charX, charY);
    lcd.write(charIndex);
  }
}

void drawSnake() {
  drawPixel(snakeX[0], snakeY[0], true);
}

void drawFood() {
  drawPixel(foodX, foodY, true);
}

void buzz(int ms) {
  digitalWrite(PIN_BUZZ, HIGH);
  delay(ms);
  digitalWrite(PIN_BUZZ, LOW);
}

void moveSnake() {
  drawPixel(snakeX[snakeLength - 1], snakeY[snakeLength - 1], false);
  
  for (int i = snakeLength - 1; i > 0; i--) {
    snakeX[i] = snakeX[i - 1];
    snakeY[i] = snakeY[i - 1];
  }
  
  snakeX[0] += dirX;
  snakeY[0] += dirY;
  
  if (snakeX[0] == foodX && snakeY[0] == foodY) {
    snakeLength++;
    buzz(10);
    placeFood();
  }
}

void checkCollision() {
  if (snakeX[0] < 1 || snakeX[0] >= BOARD_W - 1 || snakeY[0] < 1 || snakeY[0] >= BOARD_H - 1) {
    gameOver = true;
  }

  for (int i = 1; i < snakeLength; i++) {
    if (snakeX[0] == snakeX[i] && snakeY[0] == snakeY[i]) {
      gameOver = true;
    }
  }
}

void checkButtons() {
  if (digitalRead(PIN_BTN_UP) == LOW && dirY != 1) {
    dirX = 0;
    dirY = -1;
  }
  if (digitalRead(PIN_BTN_DOWN) == LOW && dirY != -1) {
    dirX = 0;
    dirY = 1;
  }
  if (digitalRead(PIN_BTN_LEFT) == LOW && dirX != 1) {
    dirX = -1;
    dirY = 0;
  }
  if (digitalRead(PIN_BTN_RIGHT) == LOW && dirX != -1) {
    dirX = 1;
    dirY = 0;
  }
}

bool isSnakeAt(int x, int y) {
  for (int i = 0; i < snakeLength; i++) {
    if (snakeX[i] == x && snakeY[i] == y) return true;
  }
  return false;
}

void placeFood() {
  do {
    foodX = random(1, BOARD_W - 1);
    foodY = random(1, BOARD_H - 1);
  } while (isSnakeAt(foodX, foodY));
}

void startGame() {
  snakeX[0] = 10;
  snakeY[0] = 5;
  snakeLength = INITIAL_SNAKE_LENGTH;
  placeFood();
}

void setup() {
  pinMode(PIN_BTN_UP, INPUT_PULLUP);
  pinMode(PIN_BTN_DOWN, INPUT_PULLUP);
  pinMode(PIN_BTN_LEFT, INPUT_PULLUP);
  pinMode(PIN_BTN_RIGHT, INPUT_PULLUP);
  pinMode(PIN_BUZZ, OUTPUT);

  lcd.begin(LCD_W_CHARS, LCD_H_CHARS);
  lcd.print("Press any key");
  buzz(500);
  while (!(digitalRead(PIN_BTN_UP) == LOW || digitalRead(PIN_BTN_DOWN) == LOW || digitalRead(PIN_BTN_LEFT) == LOW || digitalRead(PIN_BTN_RIGHT) == LOW));
  lcd.clear();

  drawBorder();
  startGame();
}

void loop() {
  long start = millis();
  if (!gameOver) {
    while (millis() - start < SNAKE_SPEED) checkButtons();
    moveSnake();
    checkCollision();
    drawSnake();
    drawFood();
    lcd.setCursor(BOARD_W_CHARS, 0);
    lcd.print("Score: ");
    lcd.print(snakeLength - INITIAL_SNAKE_LENGTH);
    lcd.setCursor(BOARD_W_CHARS, 1);
    lcd.print("Time: ");
    lcd.print(millis() / 1000);
    lcd.print('s');
  } else {
    buzz(100);
    lcd.setCursor(BOARD_W_CHARS, 0);
    lcd.print("Game Over");
    lcd.setCursor(BOARD_W_CHARS, 1);
    lcd.print(snakeLength - INITIAL_SNAKE_LENGTH);
    lcd.print("pts in ");
    lcd.print(millis() / 1000);
    lcd.print('s');
    while (1);
  }
}
