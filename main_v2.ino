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

State curr_state = ON;
State next_state;

int random_func_int;
bool CORRECT_INPUT;

bool encoder_pressed = false;
bool IO_state_changed = false;

long unsigned time_interval = 4000;
unsigned int score = 0;

void setup() {
  randomSeed(A0);       // Seed random number generator


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
    break;
    case SPIN_IT:
    break;
    case DANCE:
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
