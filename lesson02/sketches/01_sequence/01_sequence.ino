byte LEDS[3] = {2, 3, 4};
unsigned long timer = 0;
unsigned long STEP = 200;
byte idx = 0;

void setup() {
  for (byte i=0;i<3;i++){ pinMode(LEDS[i], OUTPUT); digitalWrite(LEDS[i], LOW); }
  Serial.begin(115200);
  Serial.println("V1: sequence without delay()");
}

void loop() {

  if (millis() - timer >= STEP) {
    timer = millis();
    for (byte i=0;i<3;i++) digitalWrite(LEDS[i], 0);
    digitalWrite(LEDS[idx], 1);
    idx = (idx + 1) % 3;
  }
}
