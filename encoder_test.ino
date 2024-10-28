// Pin Definitions
const int encoderPinA = 2;  // Encoder A pin (CLK)
const int encoderPinB = 4;  // Encoder B pin (DT)
const int ledPin = 8;       // LED pin

volatile int encoderPosition = 0;  // Track encoder position

unsigned long previousTime = 0;
const unsigned long timeWindow = 1000;  // 1-second window

void checkTurns(unsigned long currentTime);

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

  // Attach interrupts to the encoder pins
  attachInterrupt(digitalPinToInterrupt(encoderPinA), updateEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(encoderPinB), updateEncoder, CHANGE);

  Serial.begin(9600);
}

void loop() {
  unsigned long currentTime = millis();
  checkTurns(currentTime);
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
