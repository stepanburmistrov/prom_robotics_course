int ledPin = 2;
unsigned long timer = 0;
int STEP = 300;
void setup() {
  pinMode(ledPin, OUTPUT);
}
void loop() {
  if (millis() - timer >= STEP) {
    timer = millis();
    digitalWrite(ledPin, !digitalRead(ledPin));
  }
}