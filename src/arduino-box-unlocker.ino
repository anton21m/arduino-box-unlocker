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

  pinMode(RED_RELAY_PIN, OUTPUT);
  digitalWrite(RED_RELAY_PIN, LOW); // Красный LED горит (ошибка по умолчанию)

  pinMode(MAGNET_RELAY_PIN, OUTPUT);
  digitalWrite(MAGNET_RELAY_PIN, HIGH); // Реле магнита ВЫКЛ (заблокировано)

  display.brightness(4);
  display.clear();
  
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
  // Уменьшаем общую задержку, чтобы система была более отзывчивой
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

  // Управление красным реле (индикатор ошибки/успех)
  // LOW = Красный LED горит (ошибка/не все карты)
  // HIGH = Зеленый LED горит (успех/все карты корректны)
  if (puzzleSolved) {
    digitalWrite(RED_RELAY_PIN, HIGH); // Зеленый LED горит (успех)
    Serial.println("Welcome! All tags correct.");
  } else {
    digitalWrite(RED_RELAY_PIN, LOW);  // Красный LED горит (ошибка)
    Serial.println("System needs " + String(NR_OF_READERS - currentTotalCorrectCards) + " more correct cards.");
  }

  // Управление магнитным реле (замок)
  if (puzzleSolved) {
    digitalWrite(MAGNET_RELAY_PIN, LOW);  // Магнитное реле ВКЛ (разблокировано)
    Serial.println("Door is now open.");
  } else {
    digitalWrite(MAGNET_RELAY_PIN, HIGH); // Магнитное реле ВЫКЛ (заблокировано)
  }

  Serial.print("Cards Present: ");
  Serial.print(currentTotalCardsPresent);
  Serial.print(" Cards Correct: ");
  Serial.println(currentTotalCorrectCards);

  updateDisplay(); // Обновление дисплея
}
