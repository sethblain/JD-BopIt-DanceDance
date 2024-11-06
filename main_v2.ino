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
};

State curr_state = ON;
State next_state;

// I/O Pins
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

int random_func_int;
bool CORRECT_INPUT;
bool GAME_STARTED = false;

bool encoder_pressed = false;
bool IO_state_changed = false;
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
int slide_prev;
int slide_new;
int encoder_prev;
int encoder_new;
volatile int encoderPosition = 0;
const unsigned long holdTime = 3000;   // 3 seconds to hold button to start

long unsigned time_interval = 4000;
unsigned int score = 0;

void setup() {
  randomSeed(A0);       // Seed random number generator

  // Attach interrupts to the encoder pins
  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_A), updateEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_B), updateEncoder, CHANGE);
  
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
    }
    
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
      slide_pot_action();
      break;
    case SPIN_IT:
      // call spin in func
      checkTurns();
      break;
    case DANCE:
      // call dance func
      d_pad_action();
      break;
    case CHECK_IO:
      // checking all of the input pins / flags 
      // to see if there was a state change

      while (!IO_state_changed) {
      // check slide change
      slide_new = analogRead(SLIDE_POT_PIN);
      if (slide_new != slide_prev) {
        IO_state_changed = true;
        slide_moved = true;
      } else slide_moved = false;

      // check encoder change
      encoder_new = encoderPosition;
      if (encoder_new != encoder_prev) {
        IO_state_changed = true;
        encoder_turned = true;
      } else encoder_turned = false;

      // check pads change
      padA_new = digitalRead(D_PAD_IN_A);
      padB_new = digitalRead(D_PAD_IN_B);
      padC_new = digitalRead(D_PAD_IN_C);
      padD_new = digitalRead(D_PAD_IN_D);

      if (padA_new != padA_prev) {
        IO_state_changed = true;
        padA_pushed = true;
      } else padA_pushed = false;

      if (padB_new != padB_prev) {
        IO_state_changed = true;
        padB_pushed = true;
      } else padB_pushed = false;

      if (padC_new != padC_prev) {
        IO_state_changed = true;
        padC_pushed = true;
      } else padC_pushed = false;

      if (padD_new != padD_prev) {
        IO_state_changed = true;
        padD_pushed = true;
      } else padD_pushed = false;
    }

    break;
    case DETECT_ACTION:
      // check if correct action was taken based on random flag
    break;
    case SUCCESS:
      // increment score, display score
      time_interval *= 0.99;
      score += 1;

    break;
    case FAIL:
      // show final score, play you failed, reset score to 0
    break;


  }

}

void checkTurns(){
  encoder_prev = 0;

  // prompt
    lcd.setCursor(0,0);
    lcd.print("TURN");

    return;
}

void slide_pot_action() {
    slide_prev = analogRead(SLIDE_POT_PIN);

    if (slide_prev < 100) {
        lcd.setCursor(0,0);
        lcd.print("V Up");
    } else {
        lcd.setCursor(0,0);      
        lcd.print("V Down");
    }

    return;
}

void d_pad_action() {
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