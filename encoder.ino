// Pin Definitions
const int encoderPinA = 2;     // Encoder A pin (CLK)
const int encoderPinB = 4;     // Encoder B pin (DT)
const int buttonPin = 6;       // Button pin on the encoder (SW)
const int ledPin = 8;          // LED pin (encoder response LED)
const int startLedPin = 7;     // LED to indicate the game has started

volatile int encoderPosition = 0;  // Track encoder position

unsigned long previousTime = 0;
const unsigned long timeWindow = 1000;  // 1-second window
const unsigned long holdTime = 3000;   // 3 seconds to hold button to start

bool gameStarted = false;  // Flag to track if the game has started
unsigned long buttonPressStart = 0;  // To store when the button was pressed

void checkTurns(unsigned long currentTime);

void updateEncoder() {
  if(!gameStarted) return; //do not run code if game hasn't started

void updateEncoder() {
  int aState = digitalRead(encoderPinA);
  int bState = digitalRead(encoderPinB);

  // Increment or decrement position based on the direction of rotation
  if (aState != bState) {
    encoderPosition++;  // CW rotation
  } else {
    encoderPosition--;  // CCW rotation
  }
}

void setup() {
  pinMode(encoderPinA, INPUT);
  pinMode(encoderPinB, INPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(startLedPin, OUTPUT);

  // Attach interrupts to the encoder pins
  attachInterrupt(digitalPinToInterrupt(encoderPinA), updateEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(encoderPinB), updateEncoder, CHANGE);

  Serial.begin(9600);
}

void loop() {
  unsigned long currentTime = millis();
    // Check if game has started or button is being held to start the game
  if (!gameStarted) {
    checkButtonHold(currentTime);
  } else {
    checkTurns(currentTime);  // Run normal encoder check if game has started
  }
}

// Function to check if button is held for 3 seconds to start the game
void checkButtonHold(unsigned long currentTime) {
  if (digitalRead(buttonPin) == LOW) {  // Button is pressed when LOW
    if (buttonPressStart == 0) {
      buttonPressStart = currentTime;  // Start timer when button is first pressed
    }
    
    if (currentTime - buttonPressStart >= holdTime) {
      gameStarted = true;               // Set gameStarted flag to true
      digitalWrite(startLedPin, HIGH);   // Turn on start LED
      Serial.println("Game started!");
    }
  } else {
    buttonPressStart = 0;               // Reset start time if button is released
  }
}

void checkTurns(unsigned long currentTime){
  
  // Check if 1 second has passed
  if (currentTime - previousTime >= timeWindow) {
    previousTime = currentTime;

    // Check if exactly 4 clicks occurred in either direction
    if (abs(encoderPosition) >= 8) {
      digitalWrite(ledPin, HIGH);  // Light up LED
      Serial.println("4 clicks detected, LED ON");
    } else {
      digitalWrite(ledPin, LOW);   // Turn off LED
      Serial.println("Less than 4 clicks, LED OFF");
    }

    // Reset encoder position counter after the time window
    encoderPosition = 0;
  }
}
