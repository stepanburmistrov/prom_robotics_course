// V4: мгновенная реакция + OFF-lock
#define LED_R 2
#define LED_Y 3
#define LED_G 4
#define BTN   5
byte LEDS[3] = { LED_R, LED_Y, LED_G };

#define MODE_FAST 0
#define MODE_SLOW 1
#define MODE_ALL  2
#define MODE_OFF  3
byte mode = MODE_FAST;
byte lastWorkMode = MODE_FAST;

unsigned long FAST_STEP = 120;
unsigned long SLOW_STEP = 500;
unsigned long ALL_STEP  = 250;

unsigned long tStep = 0;
unsigned long tDebounce = 0;
unsigned long tPressStart = 0;

unsigned long DEBOUNCE_MS = 20;
unsigned long LONG_MS = 3000;

byte idx = 0;
byte btnStable = 1, btnPrevStable = 1, longFired = 0;
byte offLocked = 0;

void setAll(byte s) { for (byte i = 0; i < 3; i++) digitalWrite(LEDS[i], s); }

void switchTo(byte m) {
  if (m != MODE_OFF) lastWorkMode = m;
  mode = m;
  tStep = millis();
  idx = 0;
  if (mode == MODE_OFF) setAll(0);
}

void handleButton() {
  unsigned long now = millis();
  byte raw = digitalRead(BTN);

  if (raw != btnStable and (now - tDebounce) > DEBOUNCE_MS) {
    btnPrevStable = btnStable;
    btnStable = raw;
    tDebounce = now;

    // нажали → короткое действие сразу
    if (btnPrevStable == 1 and btnStable == 0) {
      tPressStart = now;
      longFired = 0;

      if (!offLocked) {
        if (mode == MODE_FAST) switchTo(MODE_SLOW);
        else if (mode == MODE_SLOW) switchTo(MODE_ALL);
        else if (mode == MODE_ALL)  switchTo(MODE_FAST);
        else if (mode == MODE_OFF)  switchTo(lastWorkMode);
      } else {
        offLocked = 0;               // выход из залоченного OFF
        switchTo(lastWorkMode);
      }
    }
  }

  // удержание
  if (btnStable == 0) {
    unsigned long held = now - tPressStart;
    if (!longFired and held >= LONG_MS) {
      longFired = 1;
      offLocked = 1;
      switchTo(MODE_OFF);
    }
  }
}

void doBlink() {
  if (mode == MODE_OFF) return;
  unsigned long now = millis();

  if (mode == MODE_FAST or mode == MODE_SLOW) {
    unsigned long step = (mode == MODE_FAST) ? FAST_STEP : SLOW_STEP;
    if (now - tStep >= step) {
      tStep = now;
      setAll(0);
      digitalWrite(LEDS[idx], 1);
      idx = (idx + 1) % 3;
    }
  } else if (mode == MODE_ALL) {
    if (now - tStep >= ALL_STEP) {
      tStep = now;
      static byte on = 0; on = !on;
      setAll(on ? 1 : 0);
    }
  }
}

void setup() {
  for (byte i = 0; i < 3; i++) { pinMode(LEDS[i], OUTPUT); digitalWrite(LEDS[i], 0); }
  pinMode(BTN, INPUT_PULLUP);
  switchTo(mode);
}

void loop() { handleButton(); doBlink(); }
