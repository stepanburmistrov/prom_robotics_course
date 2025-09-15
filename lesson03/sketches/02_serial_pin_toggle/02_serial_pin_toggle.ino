// V2: Serial — выбор целевого пина (2/3/4) и переключение его.
// Кнопка BTN (D5) переключает тот же выбранный пин.

#define LED_R 2
#define LED_Y 3
#define LED_G 4
#define BTN   5

int targetPin = LED_R;
int pressed = HIGH, prev = HIGH;
unsigned long last = 0;
const unsigned long DEBOUNCE = 20;

void setup() {
  pinMode(BTN, INPUT_PULLUP);

  pinMode(LED_R, OUTPUT);
  pinMode(LED_Y, OUTPUT);
  pinMode(LED_G, OUTPUT);

  digitalWrite(LED_R, LOW);
  digitalWrite(LED_Y, LOW);
  digitalWrite(LED_G, LOW);

  Serial.begin(115200);
  Serial.setTimeout(100);
  Serial.println("READY");
  Serial.print("TARGET="); Serial.println(targetPin);
}

void loop() {
  unsigned long now = millis();

  int raw = digitalRead(BTN);
  if (raw != pressed && (now - last) > DEBOUNCE) {
    prev = pressed; pressed = raw; last = now;
    if (prev == HIGH && pressed == LOW) {
      digitalWrite(targetPin, !digitalRead(targetPin));
      Serial.print("BTN TOGGLE PIN "); Serial.println(targetPin);
    }
  }

  if (Serial.available()) {
    int p = Serial.parseInt();
    if (p == LED_R || p == LED_Y || p == LED_G) {
      targetPin = p;
      digitalWrite(targetPin, !digitalRead(targetPin));
      Serial.print("SER TOGGLE PIN "); Serial.println(targetPin);
    } else {
      Serial.println("ERR PIN");
    }
  }
}
