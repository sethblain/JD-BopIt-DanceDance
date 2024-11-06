// Include necessary libraries
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include "SoftwareSerial.h"


// mp3 stuff
# define Start_Byte 0x7E
# define Version_Byte 0xFF
# define Command_Length 0x06
# define End_Byte 0xEF
# define Acknowledge 0x00 //Returns info with command 0x41 [0x01: info, 0x00: no info]
# define ACTIVATED LOW

// state machine 
enum State {
  ON,
  ACTION_SEL,
  TURN_IT_UP,
  SPIN_IT,
  DANCE, 
  CHECK_IO,
  DETECT_ACTION,
  SUCCESS,
  FAIL
}

LiquidCrystal_I2C lcd(0x27, 20, 4);  
// mp3 pins
SoftwareSerial mySerial(0, 1);

// Track indexes:

const uint8_t TURN_IT_UP_SOUND = 1;
const uint8_t TURN_IT_DOWN_SOUND = 2;
const uint8_t SPIN_IT_SOUND = 3;
const uint8_t DANCE_SOUND = 4;
const uint8_t FAILED_SOUND = 5;


State curr_state = ON;
State next_state;

int random_func_int;
bool CORRECT_INPUT;

bool encoder_pressed = false;
bool IO_state_changed = false;

long unsigned time_interval = 4000;
unsigned int score = 0;

void setup() {
  randomSeed(A0);          // Seed random number generator
  lcd.init();              // Initialize the LCD
  lcd.backlight();         // Turn on the backlight

  mySerial.begin(9600);
  delay(1000);
  setVolume(15);

}

void loop() {

  // next state logic case
  switch(curr_state){
    case ON:
      if (encoder_pressed)
        next_state = ACTION_SEL;
      else
        next_state = ON;
    break;
    case ACTION_SEL:
      if (random_func_int == 0)
        next_state = TURN_IT_UP;
      else if (random_func_int == 1)
        next_state = SPIN_IT;
      else
        next_state = DANCE;
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
      if (IO_state_changed)
        next_state = DETECT_ACTION;
      else
        next_state = FAIL;
    break;
    case DETECT_ACTION:
      if (CORRECT_INPUT)
        next_state = SUCCESS;
      else
        next_state = FAIL;
    break;
    case SUCCESS:
      next_state = ACTION_SEL;
    break;
    case FAIL:
      next_state = ON;
    break;

    curr_state = next_state;

    // state execution logic:
    switch(curr_state){
    case ON:
      // call ON function
      // display start game
      time_interval = 4000;

    break;
    case ACTION_SEL:
      // set random pointer
      random_func_int = random(0, 3);
      break;
    case TURN_IT_UP:
      // call turn it up function
    break;
    case SPIN_IT:
      // call spin it func
    break;
    case DANCE:
      // call dance func
    break;
    case CHECK_IO:
      // checking all of the input pins / flags 
      // to see if there was a state change    
    break;
    case DETECT_ACTION:
      // check if correct action was taken based on random flag
      switch(random_func_int){
        case(0):
          // check if the slide was changed (old flag != new flag)
          // not the rotary encoder (old flag = new flag)
          // not the d-pad (old flag = new flag)
        break;
        case(1):
        break;
        case(2):
        break;
      }
    break;
    case SUCCESS:
      // increment score, display score, decrement time
      time_interval *= 0.99;
      score += 1;

      lcd.clear();
      lcd.setCursor(0,0);
      lcd.display("Success");
      lcd.setCursor(0,1);
      lcd.display(String(score));

    break;
    case FAIL:
      // show final score, play you failed, reset score to 0
      playSelected(FAILED_SOUND);
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.display("Game Over");
      lcd.setCursor(0,1);
      lcd.display("Score: " + String(score));
    break;
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
