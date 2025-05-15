// Arduino IDE version: 1.0.0
// Board: Arduino Uno
// Author: Anton Shumeyko
// Libraries: Keypad, EEPROM
// This code is a simple bomb defusal simulation using an Arduino.
// Demo code: https://www.tinkercad.com/things/bFFO5Djb8Vd-bomb-imitation

// Digital Input: D2, D3, D4, D5, D6, D7, D8, D9, D10
// Analog Input: A4, A5

#include <Keypad.h>
#include <EEPROM.h>
#include "GyverTM1637.h"

#define BUZZER_PIN 10
#define CODE_LENGTH 4
#define TIMER_DURATION 180 // 3 минуты

#define CLK A5
#define DIO A4

GyverTM1637 display(CLK, DIO);

String secretSequence = "*4#58#"; // Секретная последовательность для смены кода
String storedCode = "1357"; // начальный код

// Клавиатура
const byte numRows = 4;
const byte numCols = 3;
char keymap[numRows][numCols] = {
  {'1', '2', '3', },
  {'4', '5', '6', },
  {'7', '8', '9', },
  {'*', '0', '#', }
};
byte rowPins[numRows] = {9, 8, 7, 6};
byte colPins[numCols] = {5, 4, 3};

Keypad keypad = Keypad(makeKeymap(keymap), rowPins, colPins, numRows, numCols);

// Переменные состояния
String inputCode = "";
String sequenceBuffer = "";
int remainingSeconds = TIMER_DURATION;
unsigned long lastMillis = 0;
bool codeAccepted = false;
bool colonState = false;

// EEPROM
const int EEPROM_ADDR = 0;

enum State { WAIT_CODE, TIMER_RUNNING, CHANGE_CODE };
State currentState = WAIT_CODE;

void setup() {
  display.brightness(7);  // яркость, 0 - 7 (минимум - максимум)

  pinMode(BUZZER_PIN, OUTPUT);
  Serial.begin(9600);
  loadStoredCode(); // Загрузка сохраненного кода из EEPROM
  //showCodeDots();   // Отображение точек на дисплее
}

void loadStoredCode() {
  // Загрузка сохраненного кода из EEPROM
  char codeBuf[CODE_LENGTH + 1] = {0};
  for (int i = 0; i < CODE_LENGTH; i++) {
    codeBuf[i] = EEPROM.read(EEPROM_ADDR + i);
  }
  if (isDigit(codeBuf[0])) {
    Serial.println("Stored code found in EEPROM:");
    Serial.println(codeBuf);
    storedCode = String(codeBuf);
  }
}

void loop() {
  // Основной цикл программы
  char key = keypad.getKey();
  if (key != NO_KEY) {
   handleKeyPress(key); // Обработка нажатия клавиши
  }
  if (codeAccepted) {
   updateTimer(); // Обновление таймера
  }
}

void handleKeyPress(char key) {
  // Обработка нажатия клавиши
  tone(BUZZER_PIN, 2000, 50);
  sequenceBuffer += key;
  if (sequenceBuffer.length() > secretSequence.length()) {
    sequenceBuffer = sequenceBuffer.substring(sequenceBuffer.length() - secretSequence.length());
  }
  if (sequenceBuffer == secretSequence) {
    enterChangeCodeMode(); // Вход в режим смены кода
    return;
  }
  if (currentState == WAIT_CODE) {
    handleCodeInput(key); // Обработка ввода кода
  } else if (currentState == CHANGE_CODE) {
    handleChangeCodeInput(key); // Обработка ввода нового кода
  }
}

void enterChangeCodeMode() {
  // Вход в режим смены кода
  currentState = CHANGE_CODE;
  inputCode = "";
  showCodeDots();
  Serial.println("Entering new code");
  tone(BUZZER_PIN, 1200, 100);
}

void handleCodeInput(char key) {
  // Обработка ввода кода
  if (isdigit(key)) {
    inputCode += key;
    showCodeDigits(); // Отображение введенных цифр
    if (inputCode.length() == CODE_LENGTH) {
    Serial.print("Input code: ");
    Serial.println(inputCode);
    Serial.print("Stored code: ");
    Serial.println(storedCode);

      if (inputCode == storedCode) {
        onCodeAccepted(); // Код принят
      } else {
        onCodeRejected(); // Код отклонен
      }
    }
  }
}

void handleChangeCodeInput(char key) {
  // Обработка ввода нового кода
  if (isdigit(key)) {
    inputCode += key;
    showCodeDigits(); // Отображение введенных цифр
    if (inputCode.length() == CODE_LENGTH) {
      saveNewCode(); // Сохранение нового кода
    }
  } else {
    tone(BUZZER_PIN, 400, 100);
  }
}

void onCodeAccepted() {
  // Действия при принятии кода
  tone(BUZZER_PIN, 1000, 200);
  codeAccepted = true;
  remainingSeconds = TIMER_DURATION;
  lastMillis = millis();
}

void onCodeRejected() {
  // Действия при отклонении кода
  errorBeep();       // Звуковой сигнал ошибки
  flashDisplay(3);   // Моргание дисплея
  inputCode = "";
}

void saveNewCode() {
  // Сохранение нового кода в EEPROM
  for (int i = 0; i < CODE_LENGTH; i++) {
    EEPROM.write(EEPROM_ADDR + i, inputCode.charAt(i));
  }
  storedCode = inputCode;
  Serial.println("New code saved!");
  tone(BUZZER_PIN, 1500, 300);
  delay(500);
  resetToWaitCodeState(); // Сброс в режим ожидания кода
}

void resetToWaitCodeState() {
  // Сброс в режим ожидания кода
  inputCode = "";
  sequenceBuffer = "";
  currentState = WAIT_CODE;
  showCodeDots();
}

void updateTimer() {
  // Обновление таймера
  unsigned long currentMillis = millis();
  if (currentMillis - lastMillis >= 1000 && remainingSeconds > 0) {
    lastMillis = currentMillis;
    remainingSeconds--;
    colonState = !colonState;
    displayTime(remainingSeconds); // Отображение оставшегося времени
    if (remainingSeconds == 0) {
      onTimerEnd(); // Таймер завершен
    }
  }
}

void onTimerEnd() {
  // Действия при завершении таймера
  tone(BUZZER_PIN, 800, 1000);
  delay(1000);
  resetState(); // Сброс состояния
}

void displayTime(int totalSeconds) {
  // Отображение оставшегося времени на дисплее
  int minutes = totalSeconds / 60;
  int seconds = totalSeconds % 60;
  display.displayClock(minutes, seconds);	
  //display.print(minutes * 100 + seconds);
  display.point(colonState);
  //display.writeDisplay();
}

void showCodeDots() {
  // Отображение точек на дисплее
  display.clear();
  //display.displayInt(1234);
  //for (int i = 0; i < inputCode.length() && i < CODE_LENGTH; i++) {
  display.displayInt(inputCode.toInt());
  //}
  //display.point(false);
  //display.writeDisplay();
}

void showCodeDigits() {
  // Отображение введенных цифр на дисплее
  //display.clear();
  display.displayInt(inputCode.toInt());
  //for (int i = 0; i < inputCode.length() && i < CODE_LENGTH; i++) {
    //display.display(i < 2 ? i : i + 1, inputCode.charAt(i) - '0');
  //}
  //display.writeDisplay();
}

void errorBeep() {
  // Звуковой сигнал ошибки
  for (int i = 0; i < 2; i++) {
    tone(BUZZER_PIN, 400, 150);
    delay(200);
  }
}

void flashDisplay(int times) {
  // Моргание дисплея
  for (int i = 0; i < times; i++) {
    display.clear();
    //display.writeDisplay();
    delay(200);
    showCodeDots();
    delay(200);
  }
  // Очистка экрана после моргания
  display.clear();
  //display.writeDisplay();
}

void resetState() {
  // Сброс состояния
  inputCode = "";
  codeAccepted = false;
  display.clear();
  //display.writeDisplay();
}
