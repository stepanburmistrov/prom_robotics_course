int greenPin  = 4;  // Пин зелёного светодиода 
int yellowPin = 3;  // Пин жёлтого светодиода
int redPin    = 2;  // Пин красного светодиода
int buttonPin = 5;  // Пин кнопки (кнопка между D5 и GND, включим INPUT_PULLUP)

// Таймеры для каждого независимого процесса мигания/дребезга.
long timerRed   = 0;
long timerYellow= 0;
long timerGreen = 0;

// Переменные для логики кнопки
int button = 0;          // Текущее состояние кнопки после инверсии (!digitalRead)
int buttonPressed = 0;   // Флаг «кнопка находится в нажатом состоянии» (машина состояний: 0→1→0)


void setup()
{
  pinMode(redPin,    OUTPUT);
  pinMode(yellowPin, OUTPUT);
  pinMode(greenPin,  OUTPUT);

  pinMode(buttonPin, INPUT_PULLUP); // НЕ нажата: HIGH (1). Нажата: LOW (0).
}


void loop() 
{

  if (millis() - timerRed > 100) {
    digitalWrite(redPin, !digitalRead(redPin));
    timerRed = millis();
  }

  if (millis() - timerYellow > 235) {
    digitalWrite(yellowPin, !digitalRead(yellowPin));
    timerYellow = millis();
  }

  button = !digitalRead(buttonPin);

  if (millis() - timerGreen > 5 and button == 1 and buttonPressed == 0) {
    digitalWrite(greenPin, !digitalRead(greenPin)); // Однократное переключение при нажатии
    timerGreen = millis();  // Сбрасываем таймер антидребезга
    buttonPressed = 1;      // Фиксируем, что мы уже обработали «нажатое» состояние
  }

  if (millis() - timerGreen > 5 and button == 0 and buttonPressed == 1) {
    timerGreen = millis();  // Снова сброс таймера для стабильности
    buttonPressed = 0;      // Разрешаем обработку следующего нажатия
  }
}
