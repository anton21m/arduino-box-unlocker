#include <SPI.h>
#include "config.h" // Включаем наш новый заголовочный файл
// Forward declarations for functions defined in functions.ino
void InitializePuzzleState();
void updateDisplay();
void dump_byte_array(byte * buffer, byte bufferSize);

int blinkRedCycles = 0;
int prevTotalCorrectCards = 0;


void addBlinkRedLedCount() {
  blinkRedCycles = 10;
}

void setLedStates(bool red_on, bool blue_on) {

  // Common Anode 
  //digitalWrite(A3, red_on ? LOW : HIGH);  // A3 is red LED (LOW = ON, HIGH = OFF)
  //digitalWrite(A2, blue_on ? LOW : HIGH); // A2 is blue LED (LOW = ON, HIGH = OFF)

  // Common Cathode
  digitalWrite(A3, !red_on ? LOW : HIGH);  // A3 is red LED (LOW = ON, HIGH = OFF)
  digitalWrite(A2, !blue_on ? LOW : HIGH); // A2 is blue LED (LOW = ON, HIGH = OFF)
}

// Initialize LED pins and turn them off
void initLed() {
  pinMode(A3, OUTPUT); // red leds
  pinMode(A2, OUTPUT); // blue led
  setLedStates(false, false); // Ensure both are off initially
}

void setup() {
  //delay(1000);
  initLed();
  pinMode(RST_PIN, OUTPUT); // Убедимся, что пин настроен как выход
  digitalWrite(RST_PIN, LOW); // Импульс: сначала LOW
  delay(3000);                   // Краткая пауза
  digitalWrite(RST_PIN, HIGH); // затем HIGH для выхода из сброса
  delay(1000);                  // Даём время чипам ожить


  Serial.begin(9600);
  SPI.begin();

  digitalWrite(RST_PIN, HIGH); 
  delay(500); // Даем чипам полсекунды, чтобы прийти в себя после сброса

  //SPI.setClockDivider(SPI_CLOCK_DIV8); // не работает
  
  Serial.println(F("MFRC522 Access Control Initialize."));
  
  // Инициализация реле и дисплея...
  pinMode(ACCESS_RELAY_PIN, OUTPUT);
  digitalWrite(ACCESS_RELAY_PIN, LOW); // разблокировано пока все датчики не иницилизированы

  display.clear();
  display.brightness(3); // Уменьшаем яркость дисплея

  InitializePuzzleState();
  
  // ------------------------------------------------------------------
  //  ИЗМЕНЕННЫЙ БЛОК: Повторная инициализация
  // ------------------------------------------------------------------
  for (uint8_t reader = 0; reader < NR_OF_READERS; reader++) {
    bool success = false;
    byte version = 0x00;
    int attempt = 0;
    
    // Пытаемся инициализировать чип до XX раз
    while (!success && attempt < 99999999999999) { 
      mfrc522[reader].PCD_Init(ssPins[reader], RST_PIN);
      version = mfrc522[reader].PCD_ReadRegister(MFRC522::VersionReg);
      
      if (version == 0x92) {
        success = true;
      } else {
        
        attempt++;
        Serial.print(F("Reader "));
        Serial.print(reader);
        Serial.print(F(": Attempt "));
        Serial.print(attempt);
        Serial.print(F(" failed (Code "));
        Serial.print(version);
        Serial.println(F("). Retrying..."));
        
        // Даём чипу время восстановиться перед следующей попыткой
        digitalWrite(ssPins[reader], HIGH);
        delay(1000); 
      }
    }
    
    // Вывод результата
    if (success) {
      Serial.print(F("Reader "));
      Serial.print(reader);
      Serial.print(F(": "));
      mfrc522[reader].PCD_DumpVersionToSerial();
      initializedReadersCount++;
      readerInitialized[reader] = true; // Отметить как инициализированный
    } else {
      Serial.print(F("Reader "));
      Serial.print(reader);
      Serial.println(F(": FAILED after 5 attempts."));
      readerInitialized[reader] = false;
    }
    delay(50); // Короткая пауза перед следующим чипом
  }
  // ------------------------------------------------------------------

  // Начальное состояние реле: LOW выключает реле.
  // Согласно схеме: ACCESS_RELAY_PIN LOW -> реле ВЫКЛ -> COM на NC -> lock-magnet (красный) горит.
  // Это соответствует состоянию "заблокировано" при старте.
  digitalWrite(ACCESS_RELAY_PIN, LOW); // По умолчанию "заблокировано" (красный LED ON) при старте
  
  Serial.print(F("Initialized "));
  Serial.print(initializedReadersCount);
  Serial.print(F(" / "));
  Serial.print(NR_OF_READERS);
  Serial.println(F(" readers."));
  
  updateDisplay(); // Обновить дисплей после инициализации

  // Уменьшаем общую задержку, чтобы система была более отзывчивой
  delay(100);
}

void loop() {

  // Делаем моргание если есть счетчик
  if (blinkRedCycles) {
    setLedStates(true, puzzleSolved);
  }
  

  // Фиксированная задержка между полными циклами сканирования.
  // Обратите внимание: эта задержка замедляет опрос считывателей.
  delay(1000);
  
  // Делаем моргание если есть счетчик
  if (blinkRedCycles) {
    setLedStates(false, puzzleSolved);
    blinkRedCycles -= 1;
  }

  if (prevTotalCorrectCards != currentTotalCorrectCards) {
    addBlinkRedLedCount();
  }

  prevTotalCorrectCards = currentTotalCorrectCards;

  
  // Сбрасываем состояние головоломки для каждой итерации
  InitializePuzzleState();

  // делаем сброс циклически для борьбы с багом циклического зависания (fix от alexgyver)
  digitalWrite(RST_PIN, HIGH);          // Сбрасываем все модули
  delayMicroseconds(2);                 // Ждем 2 мкс
  digitalWrite(RST_PIN, LOW);           // Отпускаем сброс на всех модулях
  delay(50);

  // Сканирование всех считывателей
  for (uint8_t reader_idx = 0; reader_idx < NR_OF_READERS; reader_idx++) {

    mfrc522[reader_idx].PCD_Init(ssPins[reader_idx], RST_PIN); // Инициализируем заново (fix от alexgyver)

    // Включаем антенну только для текущего считывателя
    mfrc522[reader_idx].PCD_AntennaOn();
    delay(30); // Короткая пауза для стабилизации поля

    // Проверка связи со считывателем в каждом цикле
    byte version = mfrc522[reader_idx].PCD_ReadRegister(MFRC522::VersionReg);
    if (version != 0x00 && version != 0xFF) {
      readerInitialized[reader_idx] = true;
      
    } else {
      Serial.print(":ERR Version: on id: ");
      Serial.print(reader_idx);
      Serial.print("-> ");
      Serial.println(version);
      readerInitialized[reader_idx] = false;

      digitalWrite(ssPins[reader_idx], HIGH);
      delay(100);
    }

    // Включаем антенну только для текущего считывателя
    mfrc522[reader_idx].PCD_AntennaOn();
    delay(30); // Короткая пауза для стабилизации поля

    // Сразу проверяем наличие карты и читаем её UID
    if (mfrc522[reader_idx].PICC_IsNewCardPresent() && mfrc522[reader_idx].PICC_ReadCardSerial()) {
      readerHasCard[reader_idx] = true;
      Serial.print("Card on Reader ");
      Serial.print(reader_idx);
      Serial.print(" ");
      Serial.print(version);
      Serial.print(F(" : UID:"));
      dump_byte_array(mfrc522[reader_idx].uid.uidByte, mfrc522[reader_idx].uid.size);
      Serial.println();

      bool uidMatch = true;
      for (int i = 0; i < mfrc522[reader_idx].uid.size; i++) {
        if (mfrc522[reader_idx].uid.uidByte[i] != tagarray[reader_idx][i]) {
            uidMatch = false;
            break;
          }
      }

      if (uidMatch) {
        readerHasCorrectCard[reader_idx] = true;
      } else {
        Serial.println("Unknown Tag on Reader " + String(reader_idx));
      }

      mfrc522[reader_idx].PICC_HaltA();
      mfrc522[reader_idx].PCD_StopCrypto1();
    }

    // Выключаем антенну, чтобы она не мешала другим считывателям
    mfrc522[reader_idx].PCD_AntennaOff();
    }

  // Агрегация данных после сканирования всех считывателей
  for (uint8_t i = 0; i < NR_OF_READERS; i++) {
    if (readerHasCard[i]) {
      currentTotalCardsPresent++;
    }
    if (readerHasCorrectCard[i]) {
      currentTotalCorrectCards++;
    }
  }

  // Обновляем флаг puzzleSolved
  puzzleSolved = (currentTotalCorrectCards == NR_OF_READERS);

  // Логика управления единственным реле (ACCESS_RELAY_PIN)
  // Согласно схеме и желаемой логике:
  // ACCESS_RELAY_PIN HIGH (реле ВКЛ) -> COM на NO -> led-open (зеленый) горит (ДВЕРЬ РАЗБЛОКИРОВАНА)
  // ACCESS_RELAY_PIN LOW (реле ВЫКЛ) -> COM на NC -> lock-magnet (красный) горит (ДВЕРЬ ЗАБЛОКИРОВАНА)
  if (puzzleSolved) {
    // Если головоломка решена, ДВЕРЬ РАЗБЛОКИРОВАНА (зеленый LED ON)
    digitalWrite(ACCESS_RELAY_PIN, HIGH); // Реле ВКЛ -> РАЗБЛОКИРОВАНО
    Serial.println("Welcome! All tags correct. Door is now UNLOCKED.");
  } else {
    // Если головоломка НЕ решена, ДВЕРЬ ЗАБЛОКИРОВАНА (красный LED ON)
    digitalWrite(ACCESS_RELAY_PIN, LOW); // Реле ВЫКЛ -> ЗАБЛОКИРОВАНО
    Serial.println("System needs " + String(NR_OF_READERS - currentTotalCorrectCards) + " more correct cards. Door is LOCKED.");
  }

  Serial.print("Cards Present: ");
  Serial.print(currentTotalCardsPresent);
  Serial.print(" Cards Correct: ");
  Serial.println(currentTotalCorrectCards);

  updateDisplay(); // Обновление дисплея
}
