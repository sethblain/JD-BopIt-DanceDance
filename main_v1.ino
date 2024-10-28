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

// d-pad
const int D_PAD_IN_A = 5;
const int D_PAD_IN_B = 16;  // analog as digital
const int D_PAD_IN_C = 9;
const int D_PAD_IN_D = 15;  // analog as digital

const int D_PAD_OUT_A = 7;
const int D_PAD_OUT_B = 8;
const int D_PAD_OUT_C = 3;
const int D_PAD_OUT_D = 10;

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
void d_pad_action(unsigned long time_interval);
void checkButtonHold();
void updateEncoder();

// function array. add your function to this array
void (*functionPtr[])(unsigned long time_interval) = { slide_pot_action, checkTurns, d_pad_action };


void setup() {
  // put your setup code here, to run once:
  pinMode(ENCODER_PIN_A, INPUT);
  pinMode(ENCODER_PIN_B, INPUT);
  pinMode(ENCODER_BUTTON_PIN, INPUT_PULLUP);
  pinMode(SLIDE_POT_PIN, INPUT);
  pinMode(D_PAD_IN_A, INPUT);
  pinMode(D_PAD_IN_B, INPUT);
  pinMode(D_PAD_IN_C, INPUT);
  pinMode(D_PAD_IN_D, INPUT);
  pinMode(D_PAD_OUT_A, OUTPUT);
  pinMode(D_PAD_OUT_B, OUTPUT);
  pinMode(D_PAD_OUT_C, OUTPUT);
  pinMode(D_PAD_OUT_D, OUTPUT);

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

void d_pad_action(unsigned long timeInterval) {
  //set up timing for pin delay
  unsigned long setup_start = millis();   // i don't really remember why i have these but maybe i will remember eventually
  unsigned long pad1_on_time = millis();  // ^^
  unsigned long pad2_on_time = millis();  // otherwise i will get rid of them

  long pad1 = random(0, 4);  // randomly choose d-pad number for pad1 (choose 0, 1, 2, 3 in full implementation)
  long pad2 = random(0, 4);  // randomly choose d-pad number for pad2
  while (pad2 == pad1) { pad2 = random(0, 4); } // if pad2 == pad1, choose again

  // set up flags
  bool padA = false;  // state of pad A
  bool padB = false;  // state of pad B
  bool padC = false;  // state of pad C
  bool padD = false;  // state of pad D
  bool inputReceived = false;
  bool correctInput = false;

  // turn on pad1 indicator LED
  switch (pad1) {
    case 0:
      // turn indicator LED for pad A on
      digitalWrite(D_PAD_OUT_A, HIGH);
      delay(1);
      break;
    case 1:
      // turn indicator LED for pad B on
      digitalWrite(D_PAD_OUT_B, HIGH);
      delay(1);
      break;
    case 2:
      // turn indicator LED for pad C on
      digitalWrite(D_PAD_OUT_C, HIGH);
      break;
    case 3:
      // turn indicator LED for pad D on
      digitalWrite(D_PAD_OUT_D, HIGH);
      break;
    default:
      // default case, should only go here if for some reason a pad # was not selected
        digitalWrite(D_PAD_OUT_A, HIGH);
        delay(250);
        digitalWrite(D_PAD_OUT_A, LOW);
        delay(1);
        digitalWrite(D_PAD_OUT_B, HIGH);
        delay(250);
        digitalWrite(D_PAD_OUT_B, LOW);
        delay(1);
      break;
  }

  // turn on pad2 indicator LED
  switch (pad2) {
    case 0:
      // turn indicator LED for pad A on
      digitalWrite(D_PAD_OUT_A, HIGH);
      break;
    case 1:
      // turn indicator LED for pad B on
      digitalWrite(D_PAD_OUT_B, HIGH);
      break;
    case 2:
      // turn indicator LED for pad C on
      digitalWrite(D_PAD_OUT_C, HIGH);
      break;
    case 3:
      // turn indicator LED for pad D on
      digitalWrite(D_PAD_OUT_D, HIGH);
      break;
    default:
      // default case, should only go here if for some reason a pad # was not selected
      // do nothing i guess
      break;
  }

  // start timer
  unsigned long start_time = millis();
  unsigned long curr_time = millis();

  // wait for user input
  while (!inputReceived) {
    // may have to add slight delay here before reading pin values

    // read all pins
    if (digitalRead(D_PAD_IN_A) == HIGH) {
      // pad A activated, input recieved
      padA = true;
      inputReceived = true;
    }

    if (digitalRead(D_PAD_IN_B) == HIGH) {
      // pad B activated, input recieved
      padB = true;
      if (!inputReceived) inputReceived = true;
    }

    if (digitalRead(D_PAD_IN_C) == HIGH) {
      // pad C activated, input recieved
      padC = true;
      if (!inputReceived) inputReceived = true;
    }

    if (digitalRead(D_PAD_IN_D) == HIGH) {
      //pad D activated, input recieved
      padD = true;
      if (!inputReceived) inputReceived = true;
    }

    // check if time limit has been exceeded
    curr_time = millis();  // read current millis
    if (curr_time - start_time > time_interval) {
      return false;   // failure via time limit exceeded, return false
    }
  }

  // check if user input is correct
  if (pad1 == 0 && pad2 == 1) {   // user was instructed to hit Pad A and Pad B
    // did user hit pad A and pad B?
    if (!padA || !padB) correctInput = false;  // NO
    else correctInput = true;   // YES
  }
  else if (pad1 == 0 && pad2 == 2) {  // user was instruced to hit pad A and Pad C
    // did user hit pad A and pad C?
    if (!padA || !padC) correctInput = false;   // NO
    else correctInput = true;   // YES
  }
  else if (pad1 == 0 && pad2 == 3) {  // user was instructed to hit Pad A and Pad D
    // did user hit pad A and pad D?
    if (!padA || !padD) correctInput = false;   // NO
    else correctInput = true;   // YES
  }
  else if (pad1 == 1 && pad2 == 2) {  // user was instructed to hit Pad B and Pad C
    // did user hit pad B and pad C?
    if (!padB || !padC) correctInput = false;   // NO
    else correctInput = true;   // YES
  }
  else if (pad1 == 1 and pad2 == 3) {  // user was instructed to hit Pad B and Pad D
    // did user hit pad B and pad D?
    if (!padB || !padD) correctInput = false;   // NO
    else correctInput = true;   // YES
  }
  else if (pad1 == 2 and pad2 == 3) {  // user was instructed to hit Pad C and Pad D
    // did user hit pad C and pad D?
    if (!padC || !padD) correctInput = false;   // NO
    else correctInput = true;   // YES
  }

  // turn off indicators LEDs
  digitalWrite(D_PAD_OUT_A, HIGH);
  delay(1);
  digitalWrite(D_PAD_OUT_B, HIGH);
  delay(1);
  digitalWrite(D_PAD_OUT_C, HIGH);
  delay(1);
  digitalWrite(D_PAD_OUT_D, HIGH);
  delay(1);
  digitalWrite(D_PAD_OUT_A, LOW);
  delay(1);
  digitalWrite(D_PAD_OUT_B, LOW);
  delay(1);
  digitalWrite(D_PAD_OUT_C, LOW);
  delay(1);
  digitalWrite(D_PAD_OUT_D, LOW);
  delay(1);


  // update CORRECT_INPUT
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
