#include "config.h" // Включаем наш новый заголовочный файл

// Для теста в wokwi
// const byte tagarray[NR_OF_READERS][4] = { // Убедитесь, что здесь 6 UID
//   {0x50, 0x9D, 0x39, 0x23},
//   {0x50, 0x9D, 0x39, 0x23},
//   {0x50, 0x9D, 0x39, 0x23},
//   {0x50, 0x9D, 0x39, 0x23},
//   {0x50, 0x9D, 0x39, 0x23},
//   {0x50, 0x9D, 0x39, 0x23},
// };


// List of Tags UIDs that are allowed to open the puzzle
const byte tagarray[NR_OF_READERS][4] = { // Убедитесь, что здесь 6 UID
  {0x89, 0xC2, 0x48, 0x05}, // 1
  {0x77, 0x18, 0x40, 0x05}, // 2
  {0x04, 0x2D, 0x3E, 0x05}, // 3
  {0x6B, 0x8E, 0x3F, 0x05}, // 4
  {0x96, 0x6E, 0x41, 0x05}, // 5
  {0xAD, 0x25, 0x3B, 0x05}, // 6
};


// State tracking for puzzle logic
int initializedReadersCount = 0;
bool readerInitialized[NR_OF_READERS]; // Состояние инициализации каждого считывателя
bool readerHasCard[NR_OF_READERS];
bool readerHasCorrectCard[NR_OF_READERS];
int currentTotalCardsPresent = 0;
int currentTotalCorrectCards = 0;

byte ssPins[NR_OF_READERS] = {SS_1_PIN, SS_2_PIN, SS_3_PIN, SS_4_PIN, SS_5_PIN, SS_6_PIN};

// Create an MFRC522 instance :
MFRC522 mfrc522[NR_OF_READERS];
GyverTM1637 display(CLK_DISPLAY, DIO_DISPLAY);

// Этот массив определяет, какой сегмент соответствует какому reader_idx
// Например: Reader 0 -> SEG_A, Reader 1 -> SEG_B, ..., Reader 5 -> SEG_F
const uint8_t READER_SEGMENT_MAPPING[NR_OF_READERS] = {
  SEG_A,
  SEG_B,
  SEG_C,
  SEG_D,
  SEG_E,
  SEG_F
};

// Статус головоломки
bool puzzleSolved = false;




