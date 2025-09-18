// V4: STATUS + управление светодиодами (2,3,4) через Serial.
// Команды:
//   STATUS            -> вывести состояния D2/D3/D4
//   <pin>             -> переключить (toggle) указанный пин (2/3/4)
//   <pin> <0|1>       -> установить состояние пина (0=LOW, 1=HIGH)
// Ошибки: ERR FORMAT / ERR PIN / ERR VAL

const int LEDS[3] = { 2, 3, 4 };

bool isLedPin(int p) {
  return (p == LEDS[0] || p == LEDS[1] || p == LEDS[2]);
}

void setup() {
  for (int i = 0; i < 3; i++) {
    pinMode(LEDS[i], OUTPUT);
    digitalWrite(LEDS[i], LOW);
  }
  Serial.begin(115200);
  Serial.println("TYPE: STATUS | <pin> | <pin> <0|1>");
}

void loop() {
  if (!Serial.available()) return;

  String cmd = Serial.readStringUntil('\n');
  cmd.trim();
  if (cmd.length() == 0) return;

  if (cmd.equalsIgnoreCase("STATUS")) {
    char buf[64];
    snprintf(buf, sizeof(buf), "LED %d=%d, %d=%d, %d=%d",
             LEDS[0], (digitalRead(LEDS[0])),
             LEDS[1], (digitalRead(LEDS[1])),
             LEDS[2], (digitalRead(LEDS[2])));
    Serial.println(buf);
    return;
  }

  int sp = cmd.indexOf(' ');

  if (sp < 0) {
    // Один токен: ожидаем номер пина -> TOGGLE
    int p = cmd.toInt();
    if (!isLedPin(p)) {
      Serial.println("ERR PIN");
      return;
    }
    digitalWrite(p, !digitalRead(p));
    Serial.print("SER TOGGLE PIN ");
    Serial.println(p);
    return;
  }

  // Два токена: "<pin> <state>"
  String s1 = cmd.substring(0, sp);
  s1.trim();
  String s2 = cmd.substring(sp + 1);
  s2.trim();

  int p = s1.toInt();
  if (!isLedPin(p)) {
    Serial.println("ERR PIN");
    return;
  }

  if (s2 != "0" && s2 != "1") {
    Serial.println("ERR VAL");
    return;
  }
  int val = (s2 == "1") ? HIGH : LOW;

  digitalWrite(p, val);
  Serial.print("SET ");
  Serial.print(p);
  Serial.print(" = ");
  Serial.println(val);
}
