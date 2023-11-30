#pragma once

#include <vector>
#include <string>

struct arduinoboard{
	
    std::string board;
	std::string architecture;
    std::string mcu;
    std::string ldr;
    std::string speed;
};

std::vector <arduinoboard> db_arduino = {
	{"Arduino Mega2560","AVR", "atmega2560","wiring","115200"},
	{"ProMicro","AVR","atmega32u4","avr109","57600"},
	{"Uno","AVR","atmega328p","arduino","115200"},
	{"Nano","AVR","atmega328p","arduino","115200"},
	{"Nano old Bootloader","AVR","atmega328p","arduino","57600"},
	{"Mini","AVR","atmega328p","arduino","115200"},
	{"Pro/Mini","AVR","atmega328p","arduino","57600"},
	{"ESP32-S2-mini","ESP32","ESP32-S2","",""}
};
