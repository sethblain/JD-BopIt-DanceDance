// Include necessary libraries
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

// constants
int SLIDE_POT_PIN = 23;

// Initialize the LCD object, set the LCD I2C address to 0x27 for a 20x4 display
LiquidCrystal_I2C lcd(0x27, 20, 4);  

volatile bool turnUpFlag;
volatile bool inputReceived;

// the setup routine runs once when you press reset:
void setup() {

  pinMode(SLIDE_POT_PIN, INPUT);
  lcd.init();              // Initialize the LCD
  lcd.backlight();         // Turn on the backlight
  // pinMode(8, OUTPUT);

}


// the loop routine runs over and over again forever:
void loop() {

  unsigned long timeInterval = 4000;

  bool correctResult = slide_pot_action(timeInterval);

  lcd.clear();

  if (correctResult){
    if (turnUpFlag){
      lcd.setCursor(0,0);
      lcd.print("success, V up");
    } else{
      lcd.setCursor(0,0);
      lcd.print("success, V down");
    }
    
  } else{
    if (turnUpFlag){
      lcd.setCursor(0,0);
      lcd.print("failure, V up");
    } else{
      lcd.setCursor(0,0);
      lcd.print("failure, V down");
    }
  }

  delay(1000);

}

bool slide_pot_action(unsigned long timeInterval) {
    unsigned long startMillis = millis();
    int initialSensorValue = analogRead(SLIDE_POT_PIN);
    int currentSensorValue;
    bool correctInput = false;

    if (initialSensorValue < 100) {
        turnUpFlag = true;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("V Up");
    } else {
        turnUpFlag = false;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("V Down");
    }

    while (millis() - startMillis < timeInterval) {  
        currentSensorValue = analogRead(SLIDE_POT_PIN);

        if (initialSensorValue < 100 && currentSensorValue > 100) {
            correctInput = true;
            turnUpFlag = false;
            break;
        } else if (initialSensorValue >= 100 && currentSensorValue < 100) {
            correctInput = true;
            turnUpFlag = true;
            break;
        }
    }

    return correctInput;
}
