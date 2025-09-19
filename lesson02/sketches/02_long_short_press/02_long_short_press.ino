// V2: кнопка + короткое/длинное, единый стиль
#define LED_R 2
#define LED_Y 3
#define LED_G 4
#define BTN   5

byte LEDS[3] = { LED_R, LED_Y, LED_G };

// Режимы
#define MODE_OFF   0
#define MODE_ON    1
#define MODE_BLINK 2
byte mode = MODE_OFF;

// Тайминги
unsigned long tDebounce  = 0;   // для антидребезга
unsigned long tPressStart= 0;   // момент нажатия
unsigned long tBlink     = 0;   // для мигания в BLINK

unsigned long DEBOUNCE_MS = 20;   // устойчивое срабатывание
unsigned long LONG_MS     = 3000; // порог длинного
unsigned long BLINK_STEP  = 250;  // период мигания

// Состояния кнопки (стабильные после антидребезга)
byte btnStable    = 1;  // 1 = не нажато (INPUT_PULLUP)
byte btnPrevStable= 1;
byte longFired    = 0;

void setAll(byte s) {
  for (byte i = 0; i < 3; i++) digitalWrite(LEDS[i], s);
}

void setup() {
  for (byte i = 0; i < 3; i++) { pinMode(LEDS[i], OUTPUT); digitalWrite(LEDS[i], 0); }
  pinMode(BTN, INPUT_PULLUP);
}

void loop() {
  unsigned long now = millis();

  // --- антидребезг на фронты ---
  byte raw = digitalRead(BTN); // 1 = не нажато, 0 = нажато
  if (raw != btnStable and (now - tDebounce) > DEBOUNCE_MS) {
    btnPrevStable = btnStable;
    btnStable = raw;
    tDebounce = now;

    // фронт "нажатия"
    if (btnPrevStable == 1 and btnStable == 0) {
      tPressStart = now;
      longFired = 0;
    }

    // фронт "отпускания"
    if (btnPrevStable == 0 and btnStable == 1) {
      if (!longFired) {
        // короткое нажатие: OFF <-> ON
        if (mode == MODE_OFF) mode = MODE_ON;
        else if (mode == MODE_ON) mode = MODE_OFF;
        else if (mode == MODE_BLINK) mode = MODE_OFF;
      }
    }
  }

  // длинное удержание (пока нажато)
  if (btnStable == 0 and !longFired and (now - tPressStart) >= LONG_MS) {
    longFired = 1;
    mode = MODE_BLINK;
  }

  // --- логика режимов ---
  if (mode == MODE_OFF) {
    setAll(0);
  } else if (mode == MODE_ON) {
    setAll(1);
  } else {
    if (now - tBlink >= BLINK_STEP) {
      tBlink = now;
      static byte on = 0;
      on = !on;
      setAll(on ? 1 : 0);
    }
  }
}
