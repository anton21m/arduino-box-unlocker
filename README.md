# Описание проекта 
Есть 6 rfid меток и 6 или 12 светодиодов с синим и зеленым свечением, также есть сервопривод.
Когда все rfid метки поставлены правильно запускается привод на 5 секунд в одну сторону, и держится пока стоят все rfid метки как только 1 убирается, серво привод на 5 секунд вращается в другую сторону

Метки uid устанавливаются в 'src/config.ino' 

# компоненты 
- 6 или 12 светодиодов rgb включенную в общий pin (Будут подключатьться отдельно через 2 реле)
- 6 rfid меток 
- 1 магнит (будет подключаться через реле)

// я подключил так и работает
// aduino 
// gnd черный -> 8
// 3.3 v белый -> 6
// 13 желтый -> 2
// 12 зеленый -> 4
// 11 синий -> 3
// 10 розовый -> 7
// SDA (2-7) -> 1 (0-1 это служебные от arduino RX/TX)


# заметки по сборке 

```
alias arduino-cli=./bin/arduino-cli
```

Install arduino uno
```
arduino-cli config init
arduino-cli core update-index
arduino-cli core install arduino:avr
arduino-cli compile --fqbn arduino:avr:uno arduino-bomba.ino --output-dir build
```

# Установка новых библиотек
arduino-cli lib install GyverTM1637
arduino-cli lib install "MFRC522"
