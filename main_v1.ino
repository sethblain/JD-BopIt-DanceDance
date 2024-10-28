/* 
CODE REFERENCE
https://forum.arduino.cc/t/randomize-the-function-called/426967 - random function selector ref
*/
// Include necessary libraries
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

// slide pot
int SLIDE_POT_PIN = 0;

// encoder
const int ENCODER_PIN_A = 2;     // Encoder A pin (CLK)
const int ENCODER_PIN_B = 4;     // Encoder B pin (DT)
const int ENCODER_BUTTON_PIN = 6;       // Button pin on the encoder (SW)
volatile int encoderPosition = 0;  // Track encoder position
const unsigned long holdTime = 3000;   // 3 seconds to hold button to start

// Initialize the LCD object, set the LCD I2C address to 0x27 for a 20x4 display
LiquidCrystal_I2C lcd(0x27, 20, 4);  

// MODIFY THIS FILE INSTEAD OF RETURNING BOOL
volatile bool CORRECT_INPUT;
volatile bool GAME_STARTED = false;  // Flag to track if the game has started

int score = 0;
unsigned long time_interval = 4000;


// prototypes
void slide_pot_action(unsigned long timeInterval);
void checkTurns(unsigned long currentTime);
void checkButtonHold();
void updateEncoder();

// function array. add your function to this array
void (*functionPtr[])(unsigned long time_interval) = { slide_pot_action, checkTurns };


void setup() {
  // put your setup code here, to run once:
  pinMode(ENCODER_PIN_A, INPUT);
  pinMode(ENCODER_PIN_B, INPUT);
  pinMode(ENCODER_BUTTON_PIN, INPUT_PULLUP);
  pinMode(SLIDE_POT_PIN, INPUT);

  // Attach interrupts to the encoder pins
  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_A), updateEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_B), updateEncoder, CHANGE);
  
  lcd.init();              // Initialize the LCD
  lcd.backlight();         // Turn on the backlight

  randomSeed(A0);       // Seed random number generator
}

void loop() {
  if (!GAME_STARTED){
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("START GAME");
  }
  // start sequence
  while(!GAME_STARTED){
    checkButtonHold();
  }

  lcd.setCursor(0,0);
  lcd.print("                "); // clear the row
  long rnd = random(0, 2); // replace second number with number of functions in functionPtr
  functionPtr[rnd](time_interval);

  if (CORRECT_INPUT){
    score += 1;    
  } else{ // fail sequence
    score = 0; 
    GAME_STARTED = false;
    lcd.setCursor(0,0);
    lcd.print("                "); // clear the row        
    lcd.setCursor(0,0);
    lcd.print("FAIL");
    delay(1000);

  }

  // display score
  lcd.setCursor(1,0);
  lcd.print("                "); // clear the row        
  lcd.setCursor(1,0);
  lcd.print(String(score));
}

// Function to check if button is held for 3 seconds to start the game
void checkButtonHold() {
  unsigned long buttonPressStart = 0;
  bool game_started = false;

  if (digitalRead(ENCODER_BUTTON_PIN) == LOW) {  // Button is pressed when LOW
    buttonPressStart = millis();  // Start timer when button is first pressed
  }

  unsigned long currentMillis = millis();
  while(digitalRead(ENCODER_BUTTON_PIN) == LOW && currentMillis - buttonPressStart < holdTime){
    currentMillis = millis();
  }
  if (abs(currentMillis - buttonPressStart) >= holdTime) {
    game_started = true;
    lcd.setCursor(0,0);
    lcd.print("                "); // clear the row
    lcd.setCursor(0,0);
    lcd.print("Game started!");
    delay(1000);
    lcd.clear();

  }
  GAME_STARTED = game_started;
}

void checkTurns(unsigned long time_interval){

  // prompt
    lcd.setCursor(0,0);
    lcd.print("TURN");

  unsigned long previousTime = millis();
  unsigned long currentTime = millis();
  bool correctFlag;

  // Check if time_interval has passed
  while (currentTime - previousTime < time_interval) {
    currentTime = millis();

    // Check if exactly 4 clicks occurred in either direction
    if (abs(encoderPosition) >= 8) {
      correctFlag = true;
      break;
    } else {
      correctFlag = false;
    }
  }
    // Reset encoder position counter after the time window
    encoderPosition = 0;
    CORRECT_INPUT = correctFlag;
    return;
}

void slide_pot_action(unsigned long timeInterval) {
    unsigned long startMillis = millis();
    int initialSensorValue = analogRead(SLIDE_POT_PIN);
    int currentSensorValue;
    bool correctInput = false;

    if (initialSensorValue < 100) {
        lcd.setCursor(0,0);
        lcd.print("V Up");
    } else {
        lcd.setCursor(0,0);      
        lcd.print("V Down");
    }

    while (millis() - startMillis < timeInterval) {  
        
        currentSensorValue = analogRead(SLIDE_POT_PIN);

        if (initialSensorValue < 100 && currentSensorValue > 100) {
            correctInput = true;
            break;
        } else if (initialSensorValue >= 100 && currentSensorValue < 100) {
            correctInput = true;
            break;
        }
    }

    CORRECT_INPUT = correctInput;
    return;
}

// helpers
void updateEncoder() {
  if(!GAME_STARTED) return; //do not run code if game hasn't started
  int aState = digitalRead(ENCODER_PIN_A);
  int bState = digitalRead(ENCODER_PIN_B);

  // Increment or decrement position based on the direction of rotation
  if (aState != bState) {
    encoderPosition++;  // CW rotation
  } else {
    encoderPosition--;  // CCW rotation
  }
}
