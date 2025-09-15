// V5: Analog read A0, print value for Serial Plotter
int POT = A0;
void setup() {
  Serial.begin(115200);
}
void loop() {
  int raw = analogRead(POT);       // 0..1023
  Serial.println(raw);             // Plotter: one series
  delay(10);
}
