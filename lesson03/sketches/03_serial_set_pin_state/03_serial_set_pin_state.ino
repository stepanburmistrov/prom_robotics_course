// V3: Read two numbers: <pin> <state>. Set LED. State must be 0 or 1.
void setup() {
  Serial.begin(115200);
}
void loop() {
  if (Serial.available()) {
    String line = Serial.readStringUntil('\n');
    line.trim();
    int sp = line.indexOf(' ');
    if (sp < 0) { Serial.println("ERR FORMAT"); return; }
    int pin = line.substring(0, sp).toInt();
    String s2 = line.substring(sp + 1);
    s2.trim();
    if (pin < 2 || pin > 13) { Serial.println("ERR PIN"); return; }
    if (s2 != "0" && s2 != "1") { Serial.println("ERR VAL"); return; }
    int val = (s2 == "1") ? HIGH : LOW;
    pinMode(pin, OUTPUT);
    digitalWrite(pin, val);
    Serial.print("SET "); Serial.print(pin); Serial.print(" = "); Serial.println(val == HIGH ? 1 : 0);
  }
}
