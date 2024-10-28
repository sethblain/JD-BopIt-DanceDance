void setup() {
  // put your setup code here, to run once:
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);
  pinMode(8, OUTPUT);
}

// Lucas Connell Touched This File @JD TA's
// Lizza Novikova was here...

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(6, HIGH);
    // turn the LED on (HIGH is the voltage level)
  delay(1000);                      // wait for a second
  digitalWrite(6, LOW); 
  digitalWrite(7, HIGH); 
  delay(1000);
  digitalWrite(7, LOW);
  digitalWrite(8, HIGH);
  delay(1000); 
  digitalWrite(8, LOW); 
}
