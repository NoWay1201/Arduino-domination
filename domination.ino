#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27,  16, 2);

const int BLUE_LED_PIN = 8;
const int RED_LED_PIN = 7;

const int RED_BUTTON_PIN = 5;
const int BLUE_BUTTON_PIN = 4;
const int BUZZER_PIN = 9; //buzzer to arduino pin 9
const unsigned long MAX_HOLD_TIME_DEFAULT = 5000;  // Maximum hold time (10 seconds)
const int PROGRESS_BAR_LENGTH = 16; // Length of the progress bar
const unsigned long BUTTON_DEBOUNCE_DELAY = 300;
const unsigned long GAME_TIME_DEFAULT = 900000;

int controllingTeam = 0; // 0 - no control, 1 - blue, 2 - red

unsigned long redButtonPressStart = 0; // Track when the RED button was pressed
unsigned long blueButtonPressStart = 0; // Track when the BLUE button was pressed
unsigned long lastIncrementTime = 0; // Track time for incrementing controlled seconds
unsigned long maxHoldTime = MAX_HOLD_TIME_DEFAULT;
unsigned long gameTime = GAME_TIME_DEFAULT;
unsigned long lastButtonPress = 0; // Prevent accidental double presses

int menuState = 0; // Tracks the current menu option
int gameType = 0;
int redTeamControlLeft = 0;
int blueTeamControlLeft = 0;

bool startScreenPlayed = false;
bool inMenu = true;
bool gameFinished = false;

enum GameState {
  MENU,
  GAME
};

GameState currentGameState = MENU;

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(BLUE_LED_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(RED_BUTTON_PIN, INPUT);
  pinMode(BLUE_BUTTON_PIN, INPUT);

  pinMode(BUZZER_PIN, OUTPUT);

  digitalWrite(RED_BUTTON_PIN, HIGH);
  digitalWrite(BLUE_BUTTON_PIN, HIGH);

  //initialize lcd screen
  lcd.init();
  // turn on the backlight
  lcd.backlight();

  Serial.begin(9600);
}

// the loop function runs over and over again forever
void loop() {
  if (gameFinished) {
    tone(BUZZER_PIN, 2000, 1000);
    digitalWrite(RED_LED_PIN, HIGH);
    digitalWrite(BLUE_LED_PIN, LOW);
    delay(2000);
    tone(BUZZER_PIN, 2000, 1000);
    digitalWrite(RED_LED_PIN, LOW);
    digitalWrite(BLUE_LED_PIN, HIGH);
    delay(2000);
    return;
  }

  if (!startScreenPlayed) {
    startScreenPlayed = true;

    lcd.setCursor(0,0);
    lcd.print("DOMINATION  GAME");
    lcd.setCursor(0,1);
    lcd.print("    BY NOWAY    ");

    delay(3000);

    lcd.setCursor(0,0);
    lcd.print("                ");
    lcd.setCursor(0,1);
    lcd.print("                ");
  }

  if (currentGameState == MENU) {
    displayMenu();
    handleMenuInput();
  } else if (currentGameState == GAME) {
    handleGameLogic();
  }
}

void displayMenu() {
  lcd.setCursor(0, 0);

  if (menuState == 0) {
    lcd.print("-> START GAME   ");
  } else if (menuState == 1) {
    lcd.print("START GAME      ");
  } else {
    lcd.print("HOLD TIME       ");
  }

  lcd.setCursor(0, 1);

  if (menuState == 0) {
    lcd.print("HOLD TIME       ");
  } else if (menuState == 1) {
    lcd.print("-> HOLD TIME    ");
  } else {
    lcd.print("-> GAME TIME    ");
  }
}

void handleMenuInput() {
  unsigned long currentMillis = millis();

  // Handle Blue Button (Navigation)
  if (digitalRead(BLUE_BUTTON_PIN) == LOW && currentMillis - lastButtonPress > BUTTON_DEBOUNCE_DELAY) {
    lastButtonPress = currentMillis;
    menuState = (menuState + 1) % 3; // Cycle through 3 menu options (0, 1, 2)
    lcd.clear();
  }

  // Handle Red Button (Selection)
  if (digitalRead(RED_BUTTON_PIN) == LOW && currentMillis - lastButtonPress > BUTTON_DEBOUNCE_DELAY) {
    lastButtonPress = currentMillis;
    lcd.clear();
    switch (menuState) {
      case 0: // Start Game
        currentGameState = GAME;
        redTeamControlLeft = gameTime / 1000;
        blueTeamControlLeft = gameTime / 1000;
        lcd.setCursor(0, 0);
        lcd.print("  STARTING GAME  ");
        lcd.setCursor(0, 1);
        lcd.print("                 ");
        delay(2000);
        break;

      case 1:
        adjustHoldTime();
        break;

      case 2:
        adjustGameTime();
        break;
    }
  }
}

void adjustHoldTime() {
  lcd.setCursor(0, 0);
  lcd.print("ADJUST HOLD TIME");
  lcd.setCursor(0, 1);
  lcd.print("TIME: ");
  lcd.print(maxHoldTime / 1000);
  lcd.print("S");

  while (true) {
    if (digitalRead(BLUE_BUTTON_PIN) == LOW && millis() - lastButtonPress > BUTTON_DEBOUNCE_DELAY) {
      lastButtonPress = millis();

      if (maxHoldTime == 30000) {
        maxHoldTime = 1000; // set back to 1s
      } else {
        maxHoldTime += 1000; // Increment by 1 second
      }
      lcd.setCursor(6, 1);
      lcd.print(maxHoldTime / 1000);
      lcd.print("S  "); // Clear trailing characters
    }

    if (digitalRead(RED_BUTTON_PIN) == LOW && millis() - lastButtonPress > BUTTON_DEBOUNCE_DELAY) {
      lastButtonPress = millis();
      lcd.clear();
      break; // Exit adjustment
    }
  }
}

void adjustGameTime() {
  lcd.setCursor(0, 0);
  lcd.print("ADJUST GAME TIME");
  lcd.setCursor(0, 1);
  lcd.print("TIME: ");
  int seconds = gameTime / 1000;
  int minutes = seconds / 60;
  lcd.print(minutes / 10);
  lcd.print(minutes % 10);
  lcd.print(":");
  lcd.print("00");

  while (true) {
    if (digitalRead(BLUE_BUTTON_PIN) == LOW && millis() - lastButtonPress > BUTTON_DEBOUNCE_DELAY) {
      lastButtonPress = millis();

      if (gameTime == 3600000) { // 1h
        gameTime = 60000; // set back to 1 minute
      } else {
        gameTime += 60000; // Increment by 1 minute
      }
      lcd.setCursor(6, 1);
      int seconds = gameTime / 1000;
      int minutes = seconds / 60;
      lcd.print(minutes / 10);
      lcd.print(minutes % 10);
      lcd.print(":");
      lcd.print("00");
    }

    if (digitalRead(RED_BUTTON_PIN) == LOW && millis() - lastButtonPress > BUTTON_DEBOUNCE_DELAY) {
      lastButtonPress = millis();
      lcd.clear();
      break; // Exit adjustment
    }
  }
}

void printProgressBar(int progress, int totalLength) {
  for (int i = 0; i < totalLength; i++) {
    if (i < progress) {
      lcd.print("#"); // Print progress character
    } else {
      lcd.print(" "); // Print space for the remaining part
    }
  }
}

void printTime(int totalSeconds) {
  int minutes = totalSeconds / 60;
  int seconds = totalSeconds % 60;
  lcd.print(minutes / 10);
  lcd.print(minutes % 10);
  lcd.print(":");
  lcd.print(seconds / 10);
  lcd.print(seconds % 10);
}

void handleGameLogic() {
  unsigned long currentMillis = millis();
  if (currentMillis - lastIncrementTime >= 1000 && redButtonPressStart == 0 && blueButtonPressStart == 0) { // Increment every second
    if (controllingTeam == 1) {
      blueTeamControlLeft--;
    } else if (controllingTeam == 2) {
      redTeamControlLeft--;
    }
    lastIncrementTime = currentMillis;
  }

  if (blueTeamControlLeft == 0 || redTeamControlLeft == 0) {
    gameFinished = true;
  }

  if (digitalRead(RED_BUTTON_PIN) == LOW && controllingTeam != 2) {
    if (redButtonPressStart == 0) { // Button just pressed
      redButtonPressStart = millis();
    }

    unsigned long elapsed = millis() - redButtonPressStart;
    int progress = min((elapsed * PROGRESS_BAR_LENGTH) / maxHoldTime, PROGRESS_BAR_LENGTH);

    digitalWrite(RED_LED_PIN, HIGH);
    digitalWrite(BLUE_LED_PIN, LOW);
    tone(BUZZER_PIN, 2000);
    lcd.setCursor(0,0);
    lcd.print("RED TAKING OVER ");
    lcd.setCursor(0,1);
    printProgressBar(progress, PROGRESS_BAR_LENGTH);

    // Check if 10 seconds have passed
    if (elapsed >= maxHoldTime) {
      controllingTeam = 2; // Red team takes control
      redButtonPressStart = 0; // Reset the timer
      noTone(BUZZER_PIN);
    }
  } else {
    redButtonPressStart = 0; // Reset if the button is released
  }

  if (digitalRead(BLUE_BUTTON_PIN) == LOW && controllingTeam != 1) {
    if (blueButtonPressStart == 0) { // Button just pressed
      blueButtonPressStart = millis();
    }

    unsigned long elapsed = millis() - blueButtonPressStart;
    int progress = min((elapsed * PROGRESS_BAR_LENGTH) / maxHoldTime, PROGRESS_BAR_LENGTH);

    digitalWrite(BLUE_LED_PIN, HIGH);
    digitalWrite(RED_LED_PIN, LOW);
    tone(BUZZER_PIN, 2000);
    lcd.setCursor(0,0);
    lcd.print("BLUE TAKING OVER");
    lcd.setCursor(0,1);
    printProgressBar(progress, PROGRESS_BAR_LENGTH);

    // Check if 10 seconds have passed
    if (elapsed >= maxHoldTime) {
      controllingTeam = 1; // Blue team takes control
      blueButtonPressStart = 0; // Reset the timer
      noTone(BUZZER_PIN);
    }
  } else {
    blueButtonPressStart = 0; // Reset if the button is released
  }

  if (
    (digitalRead(BLUE_BUTTON_PIN) == HIGH || controllingTeam == 1) && 
    (digitalRead(RED_BUTTON_PIN) == HIGH || controllingTeam == 2)
  ) {
    noTone(BUZZER_PIN);
    lcd.setCursor(0, 0);
    lcd.print("RED        ");
    printTime(redTeamControlLeft);
    lcd.setCursor(0, 1);
    lcd.print("BLUE       ");
    printTime(blueTeamControlLeft);

    if (controllingTeam == 0) { // no control
      digitalWrite(RED_LED_PIN, HIGH);
      digitalWrite(BLUE_LED_PIN, HIGH);
    } else if (controllingTeam == 1) {
      digitalWrite(RED_LED_PIN, LOW);
      digitalWrite(BLUE_LED_PIN, HIGH);
    } else if (controllingTeam == 2) {
      digitalWrite(RED_LED_PIN, HIGH);
      digitalWrite(BLUE_LED_PIN, LOW);
    }
  }
}
