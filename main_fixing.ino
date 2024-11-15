// Include necessary libraries
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

// SoftwareSerial mySerial(0, 1); // mp3 pins
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

// Track indexes:
const uint8_t TURN_IT_UP_SOUND = 2;
const uint8_t TURN_IT_DOWN_SOUND = 3;
const uint8_t SPIN_IT_SOUND = 4;
const uint8_t DANCE_SOUND = 5;
const uint8_t FAILED_SOUND = 1;

// prototypes
void setVolume(int);
void updateEncoder();
void checkTurns();
void d_pad_action();
void play(byte);
void slide_pot_action();
void checkButtonHold();
void check_io();
volatile bool checkCorrect(unsigned int);
// program resets after failure
void (* resetFunc)(void) = 0;


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

volatile int random_func_int;

// flags
volatile bool CORRECT_INPUT = false;
volatile bool GAME_STARTED = false;
volatile bool encoder_pressed = false;
volatile bool IO_state_changed = false;
// volatile bool slide_moved;
// volatile bool encoder_turned;
// volatile bool padA_pushed;
// volatile bool padB_pushed;
// volatile bool padC_pushed;
// volatile bool padD_pushed;
volatile int pad1;   // target input for d-pad 1
volatile int pad2;   // target input for d-pad 2
volatile int padA_prev;
volatile int padB_prev; 
volatile int padC_prev; 
volatile int padD_prev; 
volatile int padA_new;
volatile int padB_new; 
volatile int padC_new; 
volatile int padD_new;
volatile int slide_prev;
volatile int slide_new;
volatile int encoder_prev;
volatile int encoder_new;
int encoderPosition = 0;


const unsigned long holdTime = 3000;   // 3 seconds to hold button to start
unsigned int time_interval = 4000;
unsigned int score = 0;

void setup() {

  pinMode(D_PAD_IN_A, INPUT);
  pinMode(D_PAD_IN_B, INPUT);
  pinMode(D_PAD_IN_C, INPUT);
  pinMode(D_PAD_IN_D, INPUT);
  pinMode(D_PAD_OUT_A, OUTPUT);
  pinMode(D_PAD_OUT_B, OUTPUT);
  pinMode(D_PAD_OUT_C, OUTPUT);
  pinMode(D_PAD_OUT_D, OUTPUT);

  slide_prev = analogRead(SLIDE_POT_PIN);
  padA_prev = digitalRead(D_PAD_IN_A);
  padB_prev = digitalRead(D_PAD_IN_B);
  padC_prev = digitalRead(D_PAD_IN_C);
  padD_prev = digitalRead(D_PAD_IN_D);  

  encoder_prev = encoderPosition;

  randomSeed(A0);          // Seed random number generator
  lcd.init();              // Initialize the LCD
  lcd.backlight();         // Turn on the backlight

  Serial.begin(9600);
  delay(1000);
  init_mp3();
  setVolume(20);

  // Attach interrupts to the encoder pins
  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_A), updateEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_B), updateEncoder, CHANGE);

}


void loop(){

  // next state logic case
  switch(curr_state){

  case ON:
    // reset time interval
    time_interval = 4000;
    checkButtonHold();

    if (encoder_pressed)
      next_state = TURN_IT_UP;
    else
      next_state = ON;
    break;

  case ACTION_SEL:

    random_func_int = random(0,3);

    if (random_func_int == 0)
      next_state = TURN_IT_UP;
    else if (random_func_int == 1)
      next_state = DANCE;
    else 
      next_state = SPIN_IT;
    break;

  case TURN_IT_UP:
  
    slide_pot_action();
    next_state = CHECK_IO;
    break;

  case SPIN_IT:

    checkTurns();
    next_state = CHECK_IO;
    break;

  case DANCE:

    d_pad_action();
    next_state = CHECK_IO;
    break;

  case CHECK_IO:

    check_io();

    if (IO_state_changed){
      next_state = DETECT_ACTION;
      IO_state_changed = false; // Reset the flag
    } else next_state = FAIL;

    // reset LED's
    digitalWrite(D_PAD_OUT_A, LOW);
    digitalWrite(D_PAD_OUT_B, LOW);
    digitalWrite(D_PAD_OUT_C, LOW);
    digitalWrite(D_PAD_OUT_D, LOW);
    delay(10);

    break;

  case DETECT_ACTION:

    if (CORRECT_INPUT){
      next_state = SUCCESS;
      // lcd.clear();
      // lcd.setCursor(0,1);
      // lcd.print("corr input");
      // delay(1000);
      CORRECT_INPUT = false; // Reset the flag
    } else{
      next_state = FAIL;
      // lcd.clear();
      // lcd.setCursor(0,1);
      // lcd.print("wrong input");
      // delay(1000);
    } 
    break;
  case SUCCESS:
    // decrease time interval
    time_interval = time_interval * .9933;
    
    // increment score
    score += 1;

    if (score == 99){
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("YOU WIN");
      delay(2000);
      resetFunc();
    }

    next_state = ACTION_SEL;

    // update score to user
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("+1");
    lcd.setCursor(0,1);
    lcd.print("Score: " + String(score));
    delay(1000);
    break;

  case FAIL:

    next_state = ON;
    play(FAILED_SOUND);

    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Game Over");
    lcd.setCursor(0,1);
    lcd.print("Score: " + String(score));
    delay(1000);
    resetFunc();
    break;
  }

  curr_state = next_state;

}

void check_io(){

  IO_state_changed = false;
  CORRECT_INPUT = false;

  slide_prev = analogRead(SLIDE_POT_PIN);
  encoderPosition = 0;

  volatile unsigned long start_time = millis();
  while(abs(millis() - start_time) < time_interval && !IO_state_changed){ checkCorrect(); }
  return;
}

void checkTurns(){
  encoderPosition = 0;
  play(SPIN_IT_SOUND);

  // prompt
  lcd.setCursor(0,0);
  lcd.print("                ");
  lcd.setCursor(0,0);
  lcd.print("TURN");

  return;
}

void slide_pot_action() {

  slide_prev = analogRead(SLIDE_POT_PIN);
    
  lcd.setCursor(0,0);
  lcd.print("                ");
  lcd.setCursor(0,0);

  if (slide_prev < 100) {

    play(TURN_IT_UP_SOUND);
    lcd.print("V Up");

  } else {
    play(TURN_IT_DOWN_SOUND);     
    lcd.print("V Down");
  }

    return;
}

void d_pad_action() {

  play(DANCE_SOUND);

  lcd.setCursor(0,0);
  lcd.print("                ");
  lcd.setCursor(0,0);
  lcd.print("DANCE");

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
  }

  // turn on pad2 indicator LED
  switch (pad2) {
    case 0:
      // turn indicator LED for pad A on
      digitalWrite(D_PAD_OUT_A, HIGH);
      lcd.print("A");
      delay(10);
      break;
    case 1:
      // turn indicator LED for pad B on
      digitalWrite(D_PAD_OUT_B, HIGH);
      lcd.print("B");
      delay(10);
      break;
    case 2:
      // turn indicator LED for pad C on
      digitalWrite(D_PAD_OUT_C, HIGH);
      lcd.print("C");
      delay(10);
      break;
    case 3:
      // turn indicator LED for pad D on
      digitalWrite(D_PAD_OUT_D, HIGH);
      lcd.print("D");
      delay(10);
      break;
  }

  return;
}

// Function to check if button is held for 3 seconds to start the game
void checkButtonHold() {
    const unsigned long holdTime = 2000; // 2 seconds hold time
    unsigned long buttonPressStart = 0;
    bool buttonWasPressed = false;

    // Display start game message
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Start game");
    lcd.setCursor(0,1);
    lcd.print("Press DJ Deck");

    while(true) {
        if (digitalRead(ENCODER_BUTTON_PIN) == LOW) {  // Button is pressed when LOW
            if (!buttonWasPressed) {
                buttonWasPressed = true;
                buttonPressStart = millis();  // Start timer when button is first pressed
            }
            
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

volatile bool checkCorrect() {

  slide_new = analogRead(SLIDE_POT_PIN);
  int checkA = digitalRead(D_PAD_IN_A);
  delay(10);
  int checkB = digitalRead(D_PAD_IN_B);
  delay(10);
  int checkC = digitalRead(D_PAD_IN_C);
  delay(10);
  int checkD = digitalRead(D_PAD_IN_D);
  delay(10);

  IO_state_changed = false;
  CORRECT_INPUT = false;

  switch (random_func_int) {
    case 0:
      // check slide pot
      if ((slide_prev < 100 && slide_new >= 100) || (slide_prev > 100 && slide_new <= 100)){
        // lcd.setCursor(0, 1);
        // lcd.print("slide");
        // delay(1000);
        IO_state_changed = true;
        CORRECT_INPUT = true;
        break;
      }
      // check not dance
      if (checkA || checkB || checkC || checkD) {
        // lcd.setCursor(0, 1);
        // lcd.print("not dance");
        // delay(1000);
        IO_state_changed = true;
        break;
      }
      // check not encoder
      if (abs(encoderPosition) > 0) {
        // lcd.setCursor(0, 1);
        // lcd.print("not spin " + String(encoderPosition));
        // delay(1000);
        IO_state_changed = true;
      }
    break;
    case 1:
      // check dance
      if(pad1 == 0) {
        //int checkA = digitalRead(D_PAD_IN_A);
        //delay(10);
        if (checkA == HIGH){
          if (pad2 == 1) {
            //int checkB = digitalRead(D_PAD_IN_B);
            //delay(10);
            if (checkB == HIGH){
              IO_state_changed = true;
              CORRECT_INPUT = true;
              break;
            } else if (checkC == HIGH || checkD == HIGH) {
              IO_state_changed = true;
              break;
            }
          }
          else if (pad2 == 2) {
            //int checkC = digitalRead(D_PAD_IN_C);
            //delay(10);
            if (checkC == HIGH){
              IO_state_changed = true;
              CORRECT_INPUT = true;
              break;
            } else if (checkB == HIGH || checkD == HIGH) {
              IO_state_changed = true;
              break;
            }
          }
          else {
            //int checkD = digitalRead(D_PAD_IN_D);
            //delay(10);
            if (checkD == HIGH){
              IO_state_changed = true;
              CORRECT_INPUT = true;
              break;
            } else if (checkB == HIGH || checkC == HIGH) {
              IO_state_changed = true;
              break;
            }
          }
        } else if ((checkB == HIGH  && pad2 != 1) || (checkC == HIGH && pad2 != 2) || (checkD == HIGH && pad2 != 3)) {
          IO_state_changed = true;
          break;
        }
      }
      
      else if (pad1 == 1) {
        //int checkB = digitalRead(D_PAD_IN_B);
        //delay(10);
        if (checkB == HIGH){
          if (pad2 == 0) {
            //int checkA = digitalRead(D_PAD_IN_A);
            //delay(10);
            if (checkA == HIGH){
              IO_state_changed = true;
              CORRECT_INPUT = true;
              break;
            } else if (checkC == HIGH || checkD == HIGH) {
              IO_state_changed = true;
              break;
            }
          } 
          else if (pad2 == 2) {
            //int checkC = digitalRead(D_PAD_IN_C);
            //delay(10);
            if (checkC == HIGH){
              IO_state_changed = true;
              CORRECT_INPUT = true;
              break;
            } else if (checkA == HIGH || checkD == HIGH) {
              IO_state_changed = true;
              break;
            }
          }
          else {
            //int checkD = digitalRead(D_PAD_IN_D);
            //delay(10);
            if (checkD == HIGH){
              IO_state_changed = true;
              CORRECT_INPUT = true;
              break;
            } else if (checkA == HIGH || checkC == HIGH) {
              IO_state_changed = true;
              break;
            }
          } 
        } else if ((checkA == HIGH  && pad2 != 0) || (checkC == HIGH && pad2 != 2) || (checkD == HIGH && pad2 != 3)) {
          IO_state_changed = true;
          break;
        }
      }
      else if (pad1 == 2) {
        //int checkC = digitalRead(D_PAD_IN_C);
        //delay(10);
        if (checkC == HIGH){
          if (pad2 == 1) {
            //int checkB = digitalRead(D_PAD_IN_B);
            //delay(10);
            if (checkB == HIGH){
              IO_state_changed = true;
              CORRECT_INPUT = true;
              break;
            } else if (checkA == HIGH || checkD == HIGH) {
              IO_state_changed = true;
              break;
            }
          }
          else if (pad2 == 0) {
            //int checkA = digitalRead(D_PAD_IN_A);
            //delay(10);
            if (checkA == HIGH){
              IO_state_changed = true;
              CORRECT_INPUT = true;
              break;
            } else if (checkB == HIGH || checkD == HIGH) {
              IO_state_changed = true;
              break;
            }
          }
          else {
            int checkD = digitalRead(D_PAD_IN_D);
            delay(10);
            if (checkD == HIGH){
              IO_state_changed = true;
              CORRECT_INPUT = true;
              break;
            } else if (checkA == HIGH || checkB == HIGH) {
              IO_state_changed = true;
              break;
            }
          }
        } else if ((checkA == HIGH  && pad2 != 0) || (checkB == HIGH && pad2 != 1) || (checkD == HIGH && pad2 != 3)) {
          IO_state_changed = true;
          break;
        }
      }
      else {
        //int checkD = digitalRead(D_PAD_IN_D);
        //delay(10);
        if (checkD == HIGH){
          if (pad2 == 1) {
            //int checkB = digitalRead(D_PAD_IN_B);
            //delay(10);
            if (checkB == HIGH){
              IO_state_changed = true;
              CORRECT_INPUT = true;
              break;
            } else if (checkA == HIGH || checkC == HIGH) {
              IO_state_changed = true;
              break;
            }
          }
          else if (pad2 == 2) {
            //int checkC = digitalRead(D_PAD_IN_C);
            //delay(10);
            if (checkC == HIGH){
              IO_state_changed = true;
              CORRECT_INPUT = true;
              break;
            } else if (checkA == HIGH || checkC == HIGH) {
              IO_state_changed = true;
              break;
            }
          }
          else {
            int checkA = digitalRead(D_PAD_IN_A);
            delay(10);
            if (checkA == HIGH){
              IO_state_changed = true;
              CORRECT_INPUT = true;
              break;
            } else if (checkB == HIGH || checkC == HIGH) {
              IO_state_changed = true;
              break;
            }
          }
        } else if ((checkA == HIGH  && pad2 != 0) || (checkB == HIGH && pad2 != 1) || (checkC == HIGH && pad2 != 2)) {
          IO_state_changed = true;
          break;
        }
      }
      // check not slide pot
      if ((slide_prev < 100 && slide_new >= 100) || (slide_prev > 100 && slide_new <= 100)){
        // lcd.clear();
        // lcd.print("slide pot " + String(slide_new));
        // lcd.print(1000);
        IO_state_changed = true;
        break;
      }
      // check not encoder
      if (abs(encoderPosition) > 0) {
        // lcd.clear();
        // lcd.print("turn " + String(encoderPosition));
        // delay(1000);
        IO_state_changed = true;
      }
    break;
    case 2:
      lcd.setCursor(10,1);
      lcd.print(" ");
      // int enc_pos = encoderPosition;
      // delay(10);
      if (abs(encoderPosition) >= 8) {
        IO_state_changed = true;
        CORRECT_INPUT = true;
        break;
      }
      // check not slide pot
      if ((slide_prev < 100 && slide_new >= 100) || (slide_prev > 100 && slide_new <= 100)){
        IO_state_changed = true;
        break;
      }
      // check not dance
      if (checkA || checkB || checkC || checkD) {
        IO_state_changed = true;
      }
    break;
  }
}

// helpers
void updateEncoder() {

  int aState = digitalRead(ENCODER_PIN_A);
  int bState = digitalRead(ENCODER_PIN_B);

  // Increment or decrement position based on the direction of rotation
  if (aState != bState) {
    encoderPosition++;  // CW rotation
  } else {
    encoderPosition--;  // CCW rotation
  }
}
void init_mp3()
{
  execute_CMD(0x3F, 0, 0);
  delay(500);

}
void play(uint8_t track)
{
  execute_CMD(0x03,0,track); 
  delay(500);
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
    Serial.write( Command_line[k]);
  }
}
