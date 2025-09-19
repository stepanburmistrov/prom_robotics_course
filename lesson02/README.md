# Урок 2. Кнопка, неблокирующий код, антидребезг и режимы

В этом уроке шаг за шагом строим «промышленную» логику на **Arduino**:

* неблокирующие последовательности на базе `millis()` (без `delay()`),
* **антидребезг** и детекция **короткого/длинного** нажатия кнопки (режим `INPUT_PULLUP`),
* переключение **режимов мигания** и чистая архитектура с функциями.

![Схема c одной кнопкой](assets/scheme-1btn.png)


---

## Ссылки на скетчи:

V1 — неблокирующая последовательность:
[lesson02/sketches/01_sequence/01_sequence.ino](sketches/01_sequence/01_sequence.ino)

V2 — короткое/длинное нажатие:
[lesson02/sketches/02_long_short_press/02_long_short_press.ino](sketches/02_long_short_press/02_long_short_press.ino)

V3 — режимы мигания:
[lesson02/sketches/03_modes/03_modes.ino](sketches/03_modes/03_modes.ino)

V4 — «про»-уровень:
[lesson02/sketches/04_modes_button_longpress/04_modes_button_longpress.ino](sketches/04_modes_button_longpress/04_modes_button_longpress.ino)


---

## Оборудование и подключение

* **Плата:** Arduino
* **Светодиоды:** 3 шт (красный D2, жёлтый D3, зелёный D4)
* **Резисторы** к светодиодам: 220–330 Ω к GND
* **Кнопка:** на пин D5 и **GND**, режим `INPUT_PULLUP` (внутренняя подтяжка к +5 В)

> При `INPUT_PULLUP`: *не нажато* = `HIGH (1)`, *нажато* = `LOW (0)`.
> У 4-контактной кнопки пары выводов соединены **вертикально**. Подключаайте провода к кнопке к контактам "по диагонали".
![Схема устройства кнопки](assets/button_img.png)
![Фото разобранной кнопки](assets/button_photo.png)

Подробнее про подключение кнопок:
https://stepik.org/lesson/651743/step/1?unit=648463

---

## Быстрый чек-лист перед стартом

* В Arduino IDE выбрана верная **Board** (UNO) и **Port**. А если вы используете Nano, то еще и правильный процессор

---

# V1 — Неблокирующая последовательность без `delay()`

**Идея:** циклически включать один из трёх светодиодов через равные интервалы, используя `millis()` и массив пинов.

Код:

```cpp
// V1: последовательность без delay()
// ───────────────────────────────────────────────────────────
// КЛЮЧЕВЫЕ ИДЕИ
// - Никаких delay(): используем millis() и проверку "пора ли".
// - Массив LEDS хранит пины; цикл for экономит строки.
// - (idx + 1) % 3 даёт круговой счётчик 0→1→2→0...
// ───────────────────────────────────────────────────────────

#define LED_R 2
#define LED_Y 3
#define LED_G 4

byte LEDS[3] = { LED_R, LED_Y, LED_G };

unsigned long tStep = 0;   // "время последнего шага"
unsigned long STEP  = 200; // интервал в миллисекундах
byte idx = 0;              // текущий светодиод: 0..2

void setup() {
  for (byte i = 0; i < 3; i++) {
    pinMode(LEDS[i], OUTPUT);
    digitalWrite(LEDS[i], 0);   // погасить все на старте
  }
}

void loop() {
  unsigned long now = millis();

  // Неблокирующий таймер: если прошло STEP мс — делаем шаг
  if (now - tStep >= STEP) {
    tStep = now;

    // 1) Погасить все
    for (byte i = 0; i < 3; i++) digitalWrite(LEDS[i], 0);

    // 2) Зажечь один
    digitalWrite(LEDS[idx], 1);

    // 3) Перейти к следующему индексу по кругу:
    //    (idx + 1) % 3 -> 0,1,2,0,1,2...
    idx = (idx + 1) % 3;
  }
}

```

### Объяснение

* `millis()` возвращает время от старта в миллисекундах. Сравнивая с `timer`, мы избегаем блокирующих `delay()` и код остаётся «живым».
* Массив `LEDS[]` избавляет от дублирования кода — можно изменить порядок/количество светодиодов за одну строку.
* Оператор % — остаток от деления. Остаток при делении на 3 всегда 0,1 или 2.
* Значит (idx + 1) % 3 «ходит по кругу»: 0→1→2→0→…

### Задания (V1)

1. **Измени скорость**: сделай `STEP = 80` и `STEP = 600`. Что меняется визуально? Где «комфортная» граница?
2. **Добавь 4-й светодиод** на D6, расширь массив и логику. Что нужно поменять?
3. **Обратная последовательность:** попробуй идти 2→1→0→2→… (подсказка: счётчик вниз).

---

# V2 — Кнопка: короткое/длинное нажатие и антидребезг

**Идея:** детектируем нажатие/отпускание, фильтруем дребезг, различаем **короткое** (переключение ON/OFF) и **длинное** (включить мигание всех).

Код:

```cpp
// V2: кнопка + антидребезг + короткое/длинное
// ───────────────────────────────────────────────────────────
// КЛЮЧЕВЫЕ ИДЕИ
// - INPUT_PULLUP: не нажато = 1, нажато = 0 (инвертированная логика).
// - Антидребезг: принимаем новое состояние, только если оно
//   стабильно дольше DEBOUNCE_MS.
// - Короткое событие обрабатываем по отпусканию (release), чтобы
//   оно не "догоняло" длинное.
// ───────────────────────────────────────────────────────────

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
unsigned long DEBOUNCE_MS = 20;
unsigned long LONG_MS     = 3000;
unsigned long BLINK_STEP  = 250;

// Таймеры и состояния
unsigned long tDebounce   = 0;
unsigned long tPressStart = 0;
unsigned long tBlink      = 0;
byte btnStable = 1;       // 1 = не нажато (INPUT_PULLUP)
byte btnPrevStable = 1;
byte longFired = 0;

void setAll(byte s) {
  for (byte i = 0; i < 3; i++) digitalWrite(LEDS[i], s);
}

void setup() {
  for (byte i = 0; i < 3; i++) { pinMode(LEDS[i], OUTPUT); digitalWrite(LEDS[i], 0); }
  pinMode(BTN, INPUT_PULLUP);
}

void loop() {
  unsigned long now = millis();

  // --- Чтение кнопки с антидребезгом (фронты) ---
  byte raw = digitalRead(BTN); // сырое: 1=не нажато, 0=нажато
  if (raw != btnStable and (now - tDebounce) > DEBOUNCE_MS) {
    btnPrevStable = btnStable;
    btnStable = raw;
    tDebounce = now;

    // Нажали (front: 1 -> 0)
    if (btnPrevStable == 1 and btnStable == 0) {
      tPressStart = now;
      longFired = 0;
    }

    // Отпустили (front: 0 -> 1)
    if (btnPrevStable == 0 and btnStable == 1) {
      if (!longFired) {
        // Короткое: OFF <-> ON (BLINK тоже сворачиваем в OFF)
        if (mode == MODE_OFF) mode = MODE_ON;
        else if (mode == MODE_ON) mode = MODE_OFF;
        else if (mode == MODE_BLINK) mode = MODE_OFF;
      }
    }
  }

  // --- Длинное удержание (пока держим) ---
  if (btnStable == 0 and !longFired and (now - tPressStart) >= LONG_MS) {
    longFired = 1;
    mode = MODE_BLINK;
  }

  // --- Режимы ---
  if (mode == MODE_OFF) {
    setAll(0);
  } else if (mode == MODE_ON) {
    setAll(1);
  } else {
    if (now - tBlink >= BLINK_STEP) {
      tBlink = now;
      static byte on = 0; on = !on;
      setAll(on ? 1 : 0);
    }
  }
}

```

### Объяснение

* Событие **короткого** клика срабатывает по **отпусканию**: это упрощает различение с длительным удержанием.
* Флаг `longFired` не даёт короткому «догнаться» за длинным.

### Задания (V2)

1. Сделай короткое действие **по нажатию**, а не по отпусканию. Что меняется? Где нужен дополнительный флаг?
2. Поменяй длительность длинного нажатия на **1500 мс** и на **5000 мс**. Удобно ли?
3. Пусть BLINK мигает **только средним** светодиодом. Как изменить `setAll()`/логику?

---

# V3 — Переключение режимов + функции (TinkerCad‑friendly)

**Идея:** масштабируем архитектуру: режимы **FAST → SLOW → ALL → FAST**, длинное удержание переводит в **OFF**. Добавляем функции `switchToMode()`, `setAll()`, аккуратно ведём индексы и тайминги.

Код:

```cpp
// V3: режимы + функции, чистая архитектура
// ───────────────────────────────────────────────────────────
// ФУНКЦИИ
// - setAll(s): включить/выключить все LED разом.
// - switchTo(m): аккуратно переключить режим (сбросить таймеры/индексы).
// - handleButton(): антидребезг + короткое/длинное.
// - doBlink(): исполнение текущего режима по таймеру.
// ───────────────────────────────────────────────────────────

#define LED_R 2
#define LED_Y 3
#define LED_G 4
#define BTN   5
byte LEDS[3] = { LED_R, LED_Y, LED_G };

// Режимы
#define MODE_FAST 0
#define MODE_SLOW 1
#define MODE_ALL  2
#define MODE_OFF  3
byte mode = MODE_FAST;

// Тайминги
unsigned long FAST_STEP = 150;
unsigned long SLOW_STEP = 600;
unsigned long ALL_STEP  = 400;

// Таймеры / состояния
unsigned long tStep = 0;
unsigned long tDebounce = 0;
unsigned long tPressStart = 0;
unsigned long DEBOUNCE_MS = 20;
unsigned long LONG_MS = 1500;

byte idx = 0;                   // индекс активного LED 0..2
byte btnStable = 1, btnPrevStable = 1, longFired = 0;

void setAll(byte s) {
  for (byte i = 0; i < 3; i++) digitalWrite(LEDS[i], s);
}

void switchTo(byte m) {
  mode = m;
  tStep = millis();
  idx = 0;
  if (mode == MODE_OFF) setAll(0);
}

void handleButton() {
  unsigned long now = millis();
  byte raw = digitalRead(BTN); // 1=не нажато, 0=нажато

  if (raw != btnStable and (now - tDebounce) > DEBOUNCE_MS) {
    btnPrevStable = btnStable;
    btnStable = raw;
    tDebounce = now;

    // press
    if (btnPrevStable == 1 and btnStable == 0) {
      tPressStart = now;
      longFired = 0;
    }

    // release
    if (btnPrevStable == 0 and btnStable == 1) {
      if (!longFired) {
        if (mode == MODE_FAST) switchTo(MODE_SLOW);
        else if (mode == MODE_SLOW) switchTo(MODE_ALL);
        else if (mode == MODE_ALL)  switchTo(MODE_FAST);
        else if (mode == MODE_OFF)  switchTo(MODE_FAST);
      }
    }
  }

  // long
  if (btnStable == 0 and !longFired and (now - tPressStart) >= LONG_MS) {
    longFired = 1;
    switchTo(MODE_OFF);
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
      idx = (idx + 1) % 3; // круговой индекс через "остаток от деления"
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

void loop() {
  handleButton();
  doBlink();
}

```



### Задания (V3)

1. Добавь **режим «Knight Rider»** (бегущий огонь туда‑сюда). Подсказка: хранить направление `dir=+1/-1`, менять на границах 0/2.
2. Сделай, чтобы **длинное удержание** из любого режима выключало и **сохраняло последний рабочий** режим, чтобы потом к нему вернуться (смотри идею в V4).
3. Добавь **режим «один медленный, два быстрых»** (комбинированные тайминги). Где лучше хранить набор таймингов?

---

# V4 — «Про»-уровень: мгновенная реакция, «залоченный OFF» и возврат к предыдущему

**Идея:** короткое **обрабатываем сразу при нажатии**, длинное срабатывает **во время удержания**, и режим **OFF залочен** до следующего нажатия; при выходе — возврат к **последнему рабочему** режиму.

Код:

```cpp
// V4: мгновенная реакция + OFF-lock + возврат к предыдущему
// ───────────────────────────────────────────────────────────
// НОВОЕ
// - Короткое действие обрабатываем при нажатии (press).
// - Длинное удержание (>= LONG_MS) → MODE_OFF и ставим offLocked=1.
// - Следующее нажатие снимает lock и возвращает lastWorkMode.
// ───────────────────────────────────────────────────────────

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

void setAll(byte s) {
  for (byte i = 0; i < 3; i++) digitalWrite(LEDS[i], s);
}

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

    // Нажали (короткое — сразу)
    if (btnPrevStable == 1 and btnStable == 0) {
      tPressStart = now;
      longFired = 0;

      if (!offLocked) {
        if (mode == MODE_FAST) switchTo(MODE_SLOW);
        else if (mode == MODE_SLOW) switchTo(MODE_ALL);
        else if (mode == MODE_ALL)  switchTo(MODE_FAST);
        else if (mode == MODE_OFF)  switchTo(lastWorkMode);
      } else {
        // Выход из залоченного OFF
        offLocked = 0;
        switchTo(lastWorkMode);
      }
    }
  }

  // Удержание: длинное
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

void loop() {
  handleButton();
  doBlink();
}

```

### Чем V4 отличается от V3

* Реакция на **короткое — сразу при нажатии**, а не при отпускании.
* Появился флаг `isLockedOff`: после длинного удержания OFF «залочен» до следующего клика.
* Хранится `lastWorkMode` для корректного возврата из OFF.

### Задания (V4)

1. Измени порядок циклических режимов на **FAST → ALL → SLOW → FAST**. Где менять?
2. Сделай **двойной клик**: два быстрых нажатия подряд (≤ 300 мс) включают ALL, иначе — обычная логика. Подсказка: второй таймер и счётчик кликов.
3. Добавь **индикацию режима** через короткую вспышку всех светодиодов при переключении. Как встроить, не ломая неблокирующую архитектуру?

---

---

## Контрольные вопросы

1. Почему `delay()` мешает обрабатывать кнопку и чем лучше `millis()`?
2. Что такое «антидребезг» и зачем сравнивать **предыдущее** и **текущее** состояние кнопки?
3. В `INPUT_PULLUP` значение пина при не нажатой кнопке — какое и почему?
4. Чем отличаются режимы `ALL` и `FAST/SLOW` по логике мигания?
5. Где и зачем хранится `lastWorkMode`?
6. Почему короткое действие удобнее делать по **отпусканию** (V2)? Когда лучше **по нажатию** (V4)?
7. Как добавить четвёртый светодиод, чтобы не переписывать много кода?
8. Как реализовать «бегущий огонь туда‑сюда»?
9. Как корректно подобрать `DEBOUNCE` и `LONG_MS` под реальное железо и под симулятор?
10. Какие плюсы у выделения функций `switchToMode()` и `doBlink()`?


---

## Приложение А — Мини-шаблоны (часто пригодятся)

**Неблокирующий шаг по таймеру:**

```cpp
unsigned long t=0; 
unsigned long STEP=200;
if (millis()-t >= STEP) { t = millis(); /* действие */ }
```

**Антидребезг на фронты:**

```cpp
byte stable = 1, prev = 1; unsigned long last = 0; unsigned long DEB = 20;
byte raw = digitalRead(BTN);
if (raw != stable and (millis() - last) > DEB) {
  prev = stable; stable = raw; last = millis(); /* обрабатываем press/release */
}
```

**Перебор массива пинов:**

```cpp
for (byte i=0;i<N;i++) digitalWrite(PINS[i], state);
```


