#ifndef CONFIG_H
#define CONFIG_H

#include <MFRC522.h>     // Включаем здесь, чтобы типы MFRC522 были известны для extern

class WrapMFRC522 {
    private:
        MFRC522 mfrc522;

    public:
        // Пустой конструктор для массивов
        WrapMFRC522() : mfrc522(0, 0) {}
        
        // Конструктор с параметрами
        WrapMFRC522(byte ssPin, byte rstPin) : mfrc522(ssPin, rstPin) {}

        // Инициализация
        void PCD_Init() {
            mfrc522.PCD_Init();
        }

        // Инициализация с переназначением пинов (нужна для вашего hardReset)
        void PCD_Init(byte ssPin, byte rstPin) {
            mfrc522.PCD_Init(ssPin, rstPin);
        }

        // Антенна
        void PCD_AntennaOn()  { mfrc522.PCD_AntennaOn(); }
        void PCD_AntennaOff() { mfrc522.PCD_AntennaOff(); }

        // Чтение регистра (принимает просто число)
        byte PCD_ReadRegister(byte reg) {
            return mfrc522.PCD_ReadRegister((MFRC522::PCD_Register)reg);
        }

        // ОСТАВЛЯЕМ ТОЛЬКО ОДИН VersionReg
        // 0x37 - это адрес регистра версии для чипа RC522
        static const byte VersionReg = (0x37 << 1);

        // Работа с картами
        bool PICC_IsNewCardPresent() {
            return mfrc522.PICC_IsNewCardPresent();
        }

        bool PICC_ReadCardSerial() {
            return mfrc522.PICC_ReadCardSerial();
        }

        // Дамп и остановка
        void PCD_DumpVersionToSerial() {
            mfrc522.PCD_DumpVersionToSerial();
        }

        void PICC_HaltA() {
            mfrc522.PICC_HaltA();
        }

        void PCD_StopCrypto1() {
            mfrc522.PCD_StopCrypto1();
        }

        // Ссылка на UID для совместимости со старым кодом
        MFRC522::Uid &uid = mfrc522.uid;

        MFRC522& getRaw() {
            return mfrc522;
        }
};

#include <GyverTM1637.h> // Включаем здесь, чтобы тип GyverTM1637 был известен для extern

// PIN Numbers : RESET + SDAs
#define RST_PIN         9 // подтвердил
#define SS_1_PIN        2
#define SS_2_PIN        3
#define SS_3_PIN        4
#define SS_4_PIN        5
#define SS_5_PIN        6
#define SS_6_PIN        7

// TM1637 Display Pins
#define CLK_DISPLAY     A4
#define DIO_DISPLAY     A5

// Relay Pins
#define ACCESS_RELAY_PIN A0 // Реле для управления замком и индикацией состояния (зеленый/красный LED)
#define NR_OF_READERS   6

// Define segment bit patterns for clarity
#define SEG_A 0b00000001
#define SEG_B 0b00000010
#define SEG_C 0b00000100
#define SEG_D 0b00001000
#define SEG_E 0b00010000
#define SEG_F 0b00100000
#define SEG_G 0b01000000 // Middle segment (not used for these specific counts)

// External declarations for global variables and objects
extern int initializedReadersCount;
extern bool readerInitialized[NR_OF_READERS]; // Состояние инициализации каждого считывателя
extern bool readerHasCard[NR_OF_READERS];
extern bool readerHasCorrectCard[NR_OF_READERS];
extern int currentTotalCardsPresent;
extern int currentTotalCorrectCards;
extern byte ssPins[NR_OF_READERS];
extern const byte tagarray[NR_OF_READERS][4];

extern WrapMFRC522 mfrc522[NR_OF_READERS];
extern GyverTM1637 display;

extern const uint8_t READER_SEGMENT_MAPPING[NR_OF_READERS];
extern bool puzzleSolved; // Статус решенной головоломки

#endif // CONFIG_H
