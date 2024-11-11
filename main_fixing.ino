// Include necessary libraries
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include "SoftwareSerial.h"
SoftwareSerial mySerial(0, 1); // mp3 pins
LiquidCrystal_I2C lcd(0x27, 20, 4);  

// state machine 
enum State {
  ON, //0
  ACTION_SEL, //1
  TURN_IT_UP, //2
  SPIN_IT, //3
  DANCE, //4
  CHECK_IO, //5
  DETECT_ACTION, //6
  SUCCESS, //7
  FAIL //8
};

// mp3 stuff
# define Start_Byte 0x7E
# define Version_Byte 0xFF
# define Command_Length 0x06
# define End_Byte 0xEF
# define Acknowledge 0x00 //Returns info with command 0x41 [0x01: info, 0x00: no info]
# define ACTIVATED LOW

void setVolume(int);
void updateEncoder();
void checkTurns();
void d_pad_action();
void playSelected(byte);
void slide_pot_action();
void checkButtonHold();

// Track indexes:
const uint8_t TURN_IT_UP_SOUND = 1;
const uint8_t TURN_IT_DOWN_SOUND = 2;
const uint8_t SPIN_IT_SOUND = 3;
const uint8_t DANCE_SOUND = 4;
const uint8_t FAILED_SOUND = 5;


// IO

// d-pad
const int D_PAD_IN_A = 5;
const int D_PAD_IN_B = 16;  // analog as digital
const int D_PAD_IN_C = 9;
const int D_PAD_IN_D = 15;  // analog as digital
const int D_PAD_OUT_A = 7;
const int D_PAD_OUT_B = 8;
const int D_PAD_OUT_C = 3;
const int D_PAD_OUT_D = 10;

// encoder
const int ENCODER_PIN_A = 2;     // Encoder A pin (CLK)
const int ENCODER_PIN_B = 4;     // Encoder B pin (DT)
const int ENCODER_BUTTON_PIN = 6;       // Button pin on the encoder (SW)

// slide pot
int SLIDE_POT_PIN = 0;

State curr_state = ON;
State next_state;

int random_func_int = 0;

// flags
bool CORRECT_INPUT;
bool GAME_STARTED = false;
volatile bool encoder_pressed = false;
bool IO_state_changed;
bool slide_moved;
bool encoder_turned;
bool padA_pushed;
bool padB_pushed;
bool padC_pushed;
bool padD_pushed;

// I/O states
int pad1;   // target input for d-pad 1
int pad2;   // target input for d-pad 2
int padA_prev;
int padB_prev; 
int padC_prev; 
int padD_prev; 
int padA_new;
int padB_new; 
int padC_new; 
int padD_new;
volatile int slide_prev;
volatile int slide_new;
int encoder_prev;
int encoder_new;
volatile int encoderPosition = 0;
const unsigned long holdTime = 3000;   // 3 seconds to hold button to start



unsigned int time_interval = 4000;
unsigned int score = 0;

void setup() {

  slide_prev = analogRead(SLIDE_POT_PIN);
  padA_prev = digitalRead(D_PAD_IN_A);
  padB_prev = digitalRead(D_PAD_IN_B);
  padC_prev = digitalRead(D_PAD_IN_C);
  padD_prev = digitalRead(D_PAD_IN_D);  
  encoder_prev = encoderPosition;

  randomSeed(A0);          // Seed random number generator
  lcd.init();              // Initialize the LCD
  lcd.backlight();         // Turn on the backlight

  mySerial.begin(9600);
  delay(1000);
  setVolume(15);

  // Attach interrupts to the encoder pins
  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_A), updateEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_B), updateEncoder, CHANGE);

}

void loop() {

  // next state logic case
  switch(curr_state){
  case ON:
    if (encoder_pressed)
      next_state = TURN_IT_UP;
    else
      next_state = ON;
    break;
  case ACTION_SEL:
    next_state = TURN_IT_UP;
    break;
  case TURN_IT_UP:
    next_state = CHECK_IO;
    break;
  case SPIN_IT:
    next_state = CHECK_IO;
    break;
  case DANCE:
    next_state = CHECK_IO;
    break;
  case CHECK_IO:
    if (IO_state_changed){
      next_state = DETECT_ACTION;
      IO_state_changed = false; // Reset the flag
    } else next_state = FAIL;
    break;
  case DETECT_ACTION:
    if (CORRECT_INPUT){
      next_state = SUCCESS;
      CORRECT_INPUT = false; // Reset the flag
    } else next_state = FAIL;
    break;
  case SUCCESS:
    next_state = ACTION_SEL;
    break;
  default:
    next_state = ON;
    break;
  }

  // Store previous state before updating
  int prev_state = curr_state;
  curr_state = next_state;

  // Debug print only when state changes
  // if (prev_state != curr_state) {
  //   lcd.clear();
  //   lcd.setCursor(0,0);
  //   lcd.print("State: " + String(curr_state));
  //   lcd.setCursor(0,1);
    
  //   // Print state name for clarity
  //   switch(curr_state) {
  //     case ON: lcd.print("ON"); break;
  //     case ACTION_SEL: lcd.print("ACTION_SEL"); break;
  //     case TURN_IT_UP: lcd.print("TURN_IT_UP"); break;
  //     case SPIN_IT: lcd.print("SPIN_IT"); break;
  //     case DANCE: lcd.print("DANCE"); break;
  //     case CHECK_IO: lcd.print("CHECK_IO"); break;
  //     case DETECT_ACTION: lcd.print("DETECT_ACTION"); break;
  //     case SUCCESS: lcd.print("SUCCESS"); break;
  //     case FAIL: lcd.print("FAIL"); break;
  //   }
  //   delay(2000);
  // }

  // state execution logic:
  switch(curr_state){
  case ON:
    time_interval = 4000;
    checkButtonHold();
    break;
  case ACTION_SEL:
    break;
  case TURN_IT_UP:
    // call turn it up function
    slide_pot_action();
    // delay(1000);
    break;
  case SPIN_IT:
    // call spin it func
    checkTurns();
    // delay(1000);
    break;
  case DANCE:
    // call dance func
    d_pad_action();
    // delay(1000);
    break;
  case CHECK_IO:
    slide_new = analogRead(SLIDE_POT_PIN);
    unsigned long start_time = millis();
    IO_state_changed = false;
    slide_moved = false;
    CORRECT_INPUT = false;
    while (abs(millis() - start_time) < time_interval){
      if ((slide_prev < 100 && slide_new >= 100) || (slide_prev > 100 && slide_new <= 100)){
        IO_state_changed = true;
        slide_moved = true;
        CORRECT_INPUT = true;
        break;
      }
      slide_new = analogRead(SLIDE_POT_PIN);  
    }
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("seth");
    delay(2000);
      
    break;
  case DETECT_ACTION:
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("DETECT");
    delay(2000);
    break;
  case SUCCESS:
    // increment score, display score, decrement time by .5%
    // time_interval *= 0.995;
    // score += 1;

    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Success");
    // lcd.setCursor(0,1);
    // lcd.print(String(score));
    // delay(2000);

    break;
  default:
    // show final score, play you failed, reset score to 0
    // playSelected(FAILED_SOUND);
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Game Over");
    // // lcd.setCursor(0,1);
    // // lcd.print("Score: ");
    delay(2000);
    break;
  }
  // delay(1000);
 
}

void checkTurns(){
  encoder_prev = 0;
  // playSelected(SPIN_IT_SOUND);

  // prompt
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("TURN");

  return;
}

void slide_pot_action() {

    slide_prev = analogRead(SLIDE_POT_PIN);
    lcd.clear();
    lcd.setCursor(0,0);
    if (slide_prev < 100) {
        // playSelected(TURN_IT_UP_SOUND);

        lcd.print("V Up");

    } else {
        // playSelected(TURN_IT_DOWN_SOUND);
      
        lcd.print("V Down");

    }
    return;
}

void d_pad_action() {
  // playSelected(DANCE_SOUND);

  // read state before  anyinput
  padA_prev = digitalRead(D_PAD_IN_A);
  padB_prev = digitalRead(D_PAD_IN_B);
  padC_prev = digitalRead(D_PAD_IN_C);
  padD_prev = digitalRead(D_PAD_IN_D);

  pad1 = random(0, 4);  // randomly choose d-pad number for pad1 (choose 0, 1, 2, 3 in full implementation)
  pad2 = random(0, 4);  // randomly choose d-pad number for pad2
  while (pad2 == pad1) { pad2 = random(0, 4); } // if pad2 == pad1, choose again

  // turn on pad1 indicator LED
  switch (pad1) {
    case 0:
      // turn indicator LED for pad A on
      digitalWrite(D_PAD_OUT_A, HIGH);
      lcd.setCursor(0,0); 
      lcd.print("A + ");
      break;
    case 1:
      // turn indicator LED for pad B on
      digitalWrite(D_PAD_OUT_B, HIGH);
      lcd.setCursor(0,0); 
      lcd.print("B + ");
      break;
    case 2:
      // turn indicator LED for pad C on
      digitalWrite(D_PAD_OUT_C, HIGH);
      lcd.setCursor(0,0); 
      lcd.print("C + ");
      break;
    case 3:
      // turn indicator LED for pad D on
      digitalWrite(D_PAD_OUT_D, HIGH);
      lcd.setCursor(0,0); 
      lcd.print("D + ");
      break;
    default:
      // default case, should only go here if for some reason a pad # was not selected
      CORRECT_INPUT = false;
      return;
      break;
  }

  // turn on pad2 indicator LED
  switch (pad2) {
    case 0:
      // turn indicator LED for pad A on
      digitalWrite(D_PAD_OUT_A, HIGH);
      //lcd.setCursor(0,4); 
      lcd.print("A");
      break;
    case 1:
      // turn indicator LED for pad B on
      digitalWrite(D_PAD_OUT_B, HIGH);
      //lcd.setCursor(0,4); 
      lcd.print("B");
      break;
    case 2:
      // turn indicator LED for pad C on
      digitalWrite(D_PAD_OUT_C, HIGH);
      //lcd.setCursor(0,4); 
      lcd.print("C");
      break;
    case 3:
      // turn indicator LED for pad D on
      digitalWrite(D_PAD_OUT_D, HIGH);
      //lcd.setCursor(0,4); 
      lcd.print("D");
      break;
    default:
      // default case, should only go here if for some reason a pad # was not selected
      // do nothing i guess
      CORRECT_INPUT = false;
      return;
      break;
  }

  return;
}

// Function to check if button is held for 3 seconds to start the game
void checkButtonHold() {
    const unsigned long holdTime = 2000; // 3 seconds hold time
    unsigned long buttonPressStart = 0;
    bool buttonWasPressed = false;

    // Display start game message
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Start game");
    lcd.setCursor(0,1);
    lcd.print("Push down deck");

    while(true) {
        if (digitalRead(ENCODER_BUTTON_PIN) == LOW) {  // Button is pressed when LOW
            if (!buttonWasPressed) {
                buttonWasPressed = true;
                buttonPressStart = millis();  // Start timer when button is first pressed
            }

            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Button pressed");
            
            // Check if button has been held for the hold time
            if (abs(millis() - buttonPressStart) >= holdTime) {
                encoder_pressed = true;
                lcd.clear();
                lcd.setCursor(0,0);
                lcd.print("Starting...");
                delay(1000);
                break;
            }
        } else {
            // Button was released before the hold time was reached
            buttonWasPressed = false;
        }
    }
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

void playSelected(byte track)
{
  execute_CMD(0x3F, 0, track);
}

void setVolume(int volume)
{
  execute_CMD(0x06, 0, volume); // Set the volume (0x00~0x30)
  delay(2000);
}

void execute_CMD(byte CMD, byte Par1, byte Par2)
// Excecute the command and parameters
{
  // Calculate the checksum (2 bytes)
  word checksum = -(Version_Byte + Command_Length + CMD + Acknowledge + Par1 + Par2);
  // Build the command line
  byte Command_line[10] = { Start_Byte, Version_Byte, Command_Length, CMD, Acknowledge,
  Par1, Par2, highByte(checksum), lowByte(checksum), End_Byte};
  //Send the command line to the module
  for (byte k=0; k<10; k++){
    mySerial.write( Command_line[k]);
  }
}
