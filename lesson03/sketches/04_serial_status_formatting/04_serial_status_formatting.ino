// V4: STATUS command – print states of LEDs (2,3,4). Demonstrate formatted output.
const int LEDS[3] = {2,3,4};

void setup() {
  for (int i=0;i<3;i++){ pinMode(LEDS[i], OUTPUT); digitalWrite(LEDS[i], LOW); }
  Serial.begin(115200);
  Serial.println("TYPE: STATUS");
}

void loop() {
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    if (cmd.length() == 0) return;

    if (cmd.equalsIgnoreCase("STATUS")) {
      char buf[64];
      // Build: "LED 2=0, 3=0, 4=0"
      int n = snprintf(buf, sizeof(buf), "LED %d=%d, %d=%d, %d=%d",
                       LEDS[0], digitalRead(LEDS[0])==HIGH?1:0,
                       LEDS[1], digitalRead(LEDS[1])==HIGH?1:0,
                       LEDS[2], digitalRead(LEDS[2])==HIGH?1:0);
      Serial.println(buf);
    } else {
      Serial.println("UNKNOWN");
    }
  }
}
