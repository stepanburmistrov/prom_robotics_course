// V1: последовательность без delay(), единый стиль
#define LED_R 2
#define LED_Y 3
#define LED_G 4

byte LEDS[3] = { LED_R, LED_Y, LED_G };

unsigned long tStep = 0;   // таймер шага
unsigned long STEP  = 200; // интервал в мс
byte idx = 0;              // текущий светодиод: 0..2

void setup() {
  for (byte i = 0; i < 3; i++) {
    pinMode(LEDS[i], OUTPUT);
    digitalWrite(LEDS[i], 0);   // погасить
  }
}

void loop() {
  unsigned long now = millis();
  if (now - tStep >= STEP) {
    tStep = now;

    // погасить все
    for (byte i = 0; i < 3; i++) digitalWrite(LEDS[i], 0);

    // зажечь текущий
    digitalWrite(LEDS[idx], 1);

    // следующий индекс по кругу: 0→1→2→0...
    idx = (idx + 1) % 3;
  }
}
