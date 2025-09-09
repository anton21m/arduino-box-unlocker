#include <Arduino.h> // Для String, digitalWrite и Serial.print
#include "config.h"  // Включаем наш новый заголовочный файл

/**
   Helper routine to dump a byte array as hex values to Serial.
*/
void dump_byte_array(byte * buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

/**
 * Initializes the puzzle state.
 * Called only once in setup().
 * Сбрасывает только состояние карт, но не флаг puzzleSolved.
 */
void InitializePuzzleState() {
  currentTotalCardsPresent = 0;
  currentTotalCorrectCards = 0;
  for (uint8_t i = 0; i < NR_OF_READERS; i++) {
    readerHasCard[i] = false;
    readerHasCorrectCard[i] = false;
    // puzzleSolved НЕ сбрасывается здесь
  }
  Serial.println("Puzzle state reset (card data cleared).");
}

/**
 * Updates the 7-segment display with the current status.
 */
void updateDisplay() {
  uint8_t data[4];

  // Digit 0: MRFC Status (инициализация каждого считывателя)
  uint8_t initSegments = 0;
  // Digit 1: Cards Present (наличие карт на каждом считывателе)
  uint8_t cardsPresentSegments = 0;
  // Digit 2: Cards Correct (правильность карт на каждом считывателе)
  uint8_t cardsCorrectSegments = 0;

  for (uint8_t i = 0; i < NR_OF_READERS; i++) {
    if (readerInitialized[i]) {
      initSegments |= READER_SEGMENT_MAPPING[i];
    }
    if (readerHasCard[i]) {
      cardsPresentSegments |= READER_SEGMENT_MAPPING[i];
    }
    if (readerHasCorrectCard[i]) {
      cardsCorrectSegments |= READER_SEGMENT_MAPPING[i];
    }
  }
  data[0] = initSegments;
  data[1] = cardsPresentSegments;
  data[2] = cardsCorrectSegments;
  

  // Digit 3: Relay States
  // Segment A: Red Relay (error) status (активен, если LOW -> Красный LED горит)
  // Segment D: Magnet Relay (lock) status (активен, если LOW - разблокирован)
  uint8_t relayStatus = 0;
  // RED_RELAY_PIN == LOW означает "красный LED ON" (ошибка)
  if (digitalRead(RED_RELAY_PIN) == LOW) { // Если RED_RELAY_PIN НИЗКИЙ (красный LED активен)
    relayStatus |= SEG_A; // Активировать сегмент A для индикации ошибки
  }
  if (digitalRead(MAGNET_RELAY_PIN) == LOW) { // Если Magnet Relay активен (LOW - разблокирован)
    relayStatus |= SEG_D; // Активировать сегмент D для индикации UNLOCKED
  }
  data[3] = relayStatus;

  display.displayByte(data);
}








