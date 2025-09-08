/*
  Урок 2. Переключение режимов.
  ─────────────────────────────────────────────────────────
  Что должно уметь:
  1) Короткое нажатие (реакция СРАЗУ) — переключаем режимы по кругу:
     FAST → SLOW → ALL → FAST.
  2) Длинное нажатие (≥ 3 секунды, срабатывание по таймеру во время удержания)
     — выключаем всё (OFF). Любое следующее нажатие возвращает назад в работу.
  3) Код неблокирующий: НИКАКИХ delay(), только проверки времени через millis().
  4) Отладка через Serial Monitor (115200 бод).

  Подключение (Arduino UNO):
    D2 — красный светодиод → резистор → GND
    D3 — жёлтый светодиод → резистор → GND
    D4 — зелёный светодиод → резистор → GND
    D5 — кнопка ↔ GND (режим INPUT_PULLUP в коде)

  Почему INPUT_PULLUP?
    Включаем внутренний подтягивающий резистор к +5 В.
    Логика получается инвертированной:
      не нажато = HIGH (1), нажато = LOW (0).
*/


/* ------------------------ Пины ------------------------ */
const byte LED_R = 2;
const byte LED_Y = 3;
const byte LED_G = 4;
const byte BTN1  = 5;

// Список “в какие порты подключены наши три светодиода”.
// Так удобнее по очереди включать их по индексу 0..2.
const byte LEDS[3] = { LED_R, LED_Y, LED_G };

/* ---------------------- Режимы ------------------------ */

// 0 — быстрое последовательное мигание,
// 1 — медленное последовательное,
// 2 — все вместе,
// 3 — выкл.
const byte MODE_FAST = 0;
const byte MODE_SLOW = 1;
const byte MODE_ALL  = 2;
const byte MODE_OFF  = 3;

byte currentMode  = MODE_FAST;   // какой режим сейчас
byte lastWorkMode = MODE_FAST;   // последний рабочий (не OFF), чтобы вернуться после OFF

/* ---------------------- Тайминги ---------------------- */
unsigned long lastStepMs = 0;        // когда в последний раз что-то мига́ли
const unsigned long FAST_STEP = 100; // мс между переключениями в FAST
const unsigned long SLOW_STEP = 400; // мс между переключениями в SLOW
const unsigned long ALL_STEP  = 250; // мс между переключениями в ALL

byte idx = 0;                        // индекс текущего светодиода в последовательности 0..2

/* --------------- Кнопка + антидребезг ---------------- */
bool          btnStable       = HIGH;   // “стабильное” состояние (после фильтра)
bool          btnPrevStable   = HIGH;   // предыдущее стабильное
unsigned long btnLastChangeMs = 0;      // когда оно менялось в последний раз
const unsigned long DEBOUNCE  = 15;     // фильтр дребезга кнопки, мс

bool          isLockedOff     = false;  // OFF “залочен” длинным нажатием
unsigned long pressStartMs    = 0;      // когда нажали (начали удерживать)
bool          longTriggered   = false;  // длинное уже сработало (чтобы не повторялось)

/* ----------------- Вспомогательные вещи --------------- */

// Погасить/зажечь сразу все светодиоды одним значением (LOW или HIGH)
void setAll(byte state) {
  for (byte i = 0; i < 3; i++) {
    digitalWrite(LEDS[i], state);
  }
}

// Название режима (для печати в Serial Monitor)
const char* modeName(byte m) {
  if (m == MODE_FAST) return "FAST (sequential)";
  if (m == MODE_SLOW) return "SLOW (sequential)";
  if (m == MODE_ALL)  return "ALL (sync blink)";
  if (m == MODE_OFF)  return "OFF";
  return "?";
}

// Переключить режим (и красиво всё обнулить)
void switchToMode(byte newMode) {
  if (newMode != MODE_OFF) {
    lastWorkMode = newMode;  // запомним последний “рабочий” режим
  }

  currentMode = newMode;

  // Сообщим в монитор порта, что происходит — так удобнее отлаживать
  Serial.print("Mode => ");
  Serial.println(modeName(currentMode));

  // Сбросим “таймер шага” и индекс последовательности
  lastStepMs = millis();
  idx = 0;

  // Если OFF — сразу гасим все светодиоды
  if (currentMode == MODE_OFF) {
    setAll(LOW);
  }
}

/* ----------------- Обработка кнопки ------------------- */
void handleButton() {
  // “Сырое” чтение пина
  bool raw = digitalRead(BTN1);
  unsigned long now = millis();

  // Антидребезг: состояние считается изменившимся, только если прошло DEBOUNCE мс
  if (raw != btnStable && (now - btnLastChangeMs) > DEBOUNCE) {
    btnPrevStable   = btnStable;
    btnStable       = raw;
    btnLastChangeMs = now;

    // Поймали момент НАЖАТИЯ (HIGH -> LOW) — реагируем СРАЗУ
    if (btnPrevStable == HIGH && btnStable == LOW) {
      pressStartMs  = now;   // запомнили, когда начали удерживать
      longTriggered = false; // длинное ещё не отработало

      if (!isLockedOff) {
        // Короткое нажатие: переключаем режимы по кругу
        if      (currentMode == MODE_FAST) switchToMode(MODE_SLOW);
        else if (currentMode == MODE_SLOW) switchToMode(MODE_ALL);
        else if (currentMode == MODE_ALL)  switchToMode(MODE_FAST);
        else if (currentMode == MODE_OFF)  switchToMode(lastWorkMode);
      } else {
        // Из “залоченного OFF” выходим любым нажатием
        isLockedOff = false;
        switchToMode(lastWorkMode);
      }
    }
  }

  // Длинное нажатие: если кнопку держат НИЖНЕЙ (LOW) ≥ 3000 мс, срабатываем
  if (btnStable == LOW) {
    unsigned long held = now - pressStartMs;
    if (!longTriggered && held >= 3000) {
      longTriggered = true;   // чтобы не сработало дважды, пока держим
      isLockedOff   = true;   // OFF теперь “залочен”
      switchToMode(MODE_OFF);
      Serial.println("Long press >= 3s → MODE_OFF");
    }
  }
}

/* ---------------- Логика мигания светодиодов ---------- */
void doBlinkLogic() {
  if (currentMode == MODE_OFF) return;  // в OFF ничего не делаем

  unsigned long now = millis();

  if (currentMode == MODE_FAST || currentMode == MODE_SLOW) {
    // Для последовательности берём нужный интервал
    unsigned long step = (currentMode == MODE_FAST) ? FAST_STEP : SLOW_STEP;

    // “Пришло ли время?” — главный неблокирующий приём
    if (now - lastStepMs >= step) {
      lastStepMs = now;

      // Гасим всё и включаем один нужный светодиод по индексу
      setAll(LOW);
      digitalWrite(LEDS[idx], HIGH);

      // Следующий индекс (0→1→2→0→…)
      idx = (idx + 1) % 3;
    }
  }
  else if (currentMode == MODE_ALL) {
    // Все вместе моргают с периодом ALL_STEP
    if (now - lastStepMs >= ALL_STEP) {
      lastStepMs = now;
      static bool on = false;  // “состояние мигания”: был OFF — станет ON и наоборот
      on = !on;

      for (byte i = 0; i < 3; i++) {
        digitalWrite(LEDS[i], on ? HIGH : LOW);
      }
    }
  }
}

/* -------------------- Стандартные setup/loop ---------- */
void setup() {
  Serial.begin(115200);  // открой Serial Monitor на той же скорости

  // Настроим пины
  pinMode(LED_R, OUTPUT);
  pinMode(LED_Y, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(BTN1, INPUT_PULLUP);  // включили внутреннюю подтяжку

  setAll(LOW);                  // на старте — всё погашено
  switchToMode(currentMode);    // включаем стартовый режим
  Serial.println("Lesson 2: simple version started");
}

void loop() {
  handleButton();   // читаем кнопку (антидребезг, короткое/длинное)
  doBlinkLogic();   // мигаем в зависимости от режима
}
