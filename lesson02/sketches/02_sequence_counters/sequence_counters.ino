/*
  Урок 2. Последовательности и счётчики (простой вариант)
  ─────────────────────────────────────────────────────────
  Что делает:
  • Каждые intervalMs миллисекунд включает следующий светодиод (из 3 штук).
  • Считает шаги и после N_STEPS_PER_CYCLE шагов считает "один цикл".
  • После каждого цикла меняет скорость: SLOW ↔ FAST.
  • Печатает полезные сообщения в Serial Monitor (115200 бод).

  Подключение (Arduino UNO):
    D2 — красный LED → резистор → GND
    D3 — жёлтый LED → резистор → GND
    D4 — зелёный LED → резистор → GND

  Важно: никаких delay() — только проверка времени через millis().
*/



/* ------------------------- Пины ------------------------- */
const byte LED_R = 2;
const byte LED_Y = 3;
const byte LED_G = 4;
const byte LEDS[3] = { LED_R, LED_Y, LED_G }; // массив пинов — удобно бегать по индексам 0..2

/* ----------------------- Тайминги ----------------------- */
const unsigned long INTERVAL_FAST = 120;  // быстрая скорость, мс
const unsigned long INTERVAL_SLOW = 400;  // медленная скорость, мс
unsigned long intervalMs = INTERVAL_SLOW; // начнём медленно

unsigned long lastStepMs = 0;             // когда в последний раз переключали шаг

/* -------------------- Счётчики шага/цикла --------------- */
byte  stepIndex    = 0;   // какой светодиод сейчас (0,1,2)
unsigned int stepCounter  = 0;   // сколько шагов сделали всего
byte  cycleCounter = 0;   // номер завершённого цикла

// Один "полный круг" по трём светодиодам — это 3 шага.
// Хотим 5 таких кругов на цикл → 3 * 5 = 15 шагов.
const unsigned int N_STEPS_PER_CYCLE = 3 * 5;

/* ------------------- Вспомогательные функции ------------ */

// Установить всем LED одинаковое состояние (LOW или HIGH)
void setAll(byte state) {
  for (byte i = 0; i < 3; i++) {
    digitalWrite(LEDS[i], state);
  }
}

// Показать один шаг последовательности:
// выключить все и включить один по индексу idx
void showStep(byte idx) {
  setAll(LOW);
  digitalWrite(LEDS[idx], HIGH);
}

/* ------------------------ Старт ------------------------- */
void setup() {
  Serial.begin(115200);

  // Настроим пины как выходы
  for (byte i = 0; i < 3; i++) {
    pinMode(LEDS[i], OUTPUT);
  }

  setAll(LOW);
  Serial.println("Sequence + counters (simple)");
}

/* ------------------------ Главный цикл ------------------ */
void loop() {
  unsigned long now = millis();

  // Неблокирующая проверка: "пришло ли время" сделать следующий шаг?
  if (now - lastStepMs >= intervalMs) {
    lastStepMs = now;         // запомнили момент переключения

    showStep(stepIndex);      // включили нужный светодиод
    stepIndex = (stepIndex + 1) % 3; // приготовили следующий индекс 0→1→2→0→…

    stepCounter++;            // считаем шаги

    // Каждые N_STEPS_PER_CYCLE шагов считаем, что завершился 1 цикл
    if (stepCounter % N_STEPS_PER_CYCLE == 0) {
      cycleCounter++;

      // Меняем скорость: если была медленная — станет быстрая, и наоборот
      if (intervalMs == INTERVAL_SLOW) {
        intervalMs = INTERVAL_FAST;
      } else {
        intervalMs = INTERVAL_SLOW;
      }

      // Сообщим в Serial — удобно видеть прогресс и текущую скорость
      Serial.print("Cycle #");
      Serial.print(cycleCounter);
      Serial.print(" | intervalMs = ");
      Serial.println(intervalMs);
    }
  }
}
