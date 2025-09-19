#include <SPI.h>
#include "config.h" // Включаем наш новый заголовочный файл
// Forward declarations for functions defined in functions.ino
void InitializePuzzleState();
void updateDisplay();
void dump_byte_array(byte * buffer, byte bufferSize);

void setup() {
  Serial.begin(9600);
  //while (!Serial);

  SPI.begin();
  Serial.println(F("MFRC522 Access Control Initialize."));

  pinMode(ACCESS_RELAY_PIN, OUTPUT);

  // Начальное состояние реле: LOW выключает реле.
  // Согласно схеме: ACCESS_RELAY_PIN LOW -> реле ВЫКЛ -> COM на NC -> lock-magnet (красный) горит.
  // Это соответствует состоянию "заблокировано" при старте.
  digitalWrite(ACCESS_RELAY_PIN, LOW); // По умолчанию "заблокировано" (красный LED ON) при старте

  display.clear();
  display.brightness(3); // Уменьшаем яркость дисплея
  
  // Инициализация массива readerInitialized и других начальных состояний
  InitializePuzzleState(); // Вызывается один раз в setup для начальной настройки

  for (uint8_t reader = 0; reader < NR_OF_READERS; reader++) {
    mfrc522[reader].PCD_Init(ssPins[reader], RST_PIN);
    Serial.print(F("Reader "));
    Serial.print(reader);
    Serial.print(F(": "));
    byte version = mfrc522[reader].PCD_ReadRegister(MFRC522::VersionReg);
    if (version == 0x00 || version == 0xFF) {
      Serial.println(F("WARNING: Communication failure, is the MFRC522 properly connected?"));
      readerInitialized[reader] = false; // Отметить как неинициализированный
    } else {
      mfrc522[reader].PCD_DumpVersionToSerial();
      initializedReadersCount++;
      readerInitialized[reader] = true; // Отметить как инициализированный
    }
    delay(20);
  }

  Serial.print(F("Initialized "));
  Serial.print(initializedReadersCount);
  Serial.print(F(" / "));
  Serial.print(NR_OF_READERS);
  Serial.println(F(" readers."));
  
  updateDisplay(); // Обновить дисплей после инициализации

  // Уменьшаем общую задержку, чтобы система была более отзывчивой
  delay(50);
}

void loop() {
  // Фиксированная задержка между полными циклами сканирования.
  // Обратите внимание: эта задержка замедляет опрос считывателей.
  delay(200);

  // Сбрасываем состояние головоломки для каждой итерации
  InitializePuzzleState();

  // Сканирование всех считывателей
  for (uint8_t reader_idx = 0; reader_idx < NR_OF_READERS; reader_idx++) {
    // Включаем антенну только для текущего считывателя
    mfrc522[reader_idx].PCD_AntennaOn();
    delay(10); // Короткая пауза для стабилизации поля

    // Сразу проверяем наличие карты и читаем её UID
    if (mfrc522[reader_idx].PICC_IsNewCardPresent() && mfrc522[reader_idx].PICC_ReadCardSerial()) {
      readerHasCard[reader_idx] = true;
      Serial.print("Card on Reader ");
      Serial.print(reader_idx);
      Serial.print(F(": UID:"));
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
