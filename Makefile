# SPDX-FileCopyrightText: © 2022 Uri Shaked <uri@wokwi.com>
# SPDX-License-Identifier: MIT

SOURCE_DIR := src
# SOURCE_PROJECT теперь не используется напрямую для копирования, но остается для справки
# SOURCE_PROJECT := $(SOURCE_DIR)/arduino-box-unlocker.ino 

clean:
	echo "Cleaning up..."
	rm -rf build
	mkdir build
	chmod -R 777 build

compile-chip:
	echo "CHIP EXPORTED from git !!!"

compile-arduino:
	mkdir -p ./build/sketch
	echo "Compiling Arduino sketch..."
	# Copy all .ino and .h files from src/ to the sketch directory
	cp "$(SOURCE_DIR)"/*.ino ./build/sketch/
	cp "$(SOURCE_DIR)"/*.h ./build/sketch/
	# Rename the main .ino file to sketch.ino, which arduino-cli expects by default
	mv ./build/sketch/arduino-box-unlocker.ino ./build/sketch/sketch.ino

	arduino-cli lib install MFRC522
	arduino-cli lib install GyverTM1637
	arduino-cli compile --fqbn arduino:avr:uno ./build/sketch --output-dir build

compile-debug-arduino:
	mkdir -p ./build/sketch
	echo "Compiling Arduino sketch..."
	cp "$(SOURCE_DIR)"/*.ino ./build/sketch/
	cp "$(SOURCE_DIR)"/*.h ./build/sketch/
	mv ./build/sketch/arduino-box-unlocker.ino ./build/sketch/sketch.ino
	arduino-cli lib uninstall MFRC522
	pwd
	arduino-cli compile --fqbn arduino:avr:uno ./build/sketch --library ./lib/MFRC522 --output-dir build

all: clean compile-chip compile-arduino

create-release:
	echo "CHIP EXPORTED from git !!!"