#pragma once

#include <vector>
#include <string>

struct arduinoboard{
	
    std::string board;
	std::string programmer;
    std::string mcu;
    std::string ldr;
    std::string speed;
};

std::vector <arduinoboard> db_arduino = {
	{"Arduino Mega2560","AVRDude", "atmega2560","wiring","115200"},
	{"ProMicro","AVRDude","atmega32u4","avr109","57600"},
	{"Uno","AVRDude","atmega328p","arduino","115200"},
	{"Nano","AVRDude","atmega328p","arduino","115200"},
	{"Nano old Bootloader","AVRDude","atmega328p","arduino","57600"},
	{"Mini","AVRDude","atmega328p","arduino","115200"},
	{"Pro/Mini","AVRDude","atmega328p","arduino","57600"},
	{"ESP32-S2-mini","ESP32tool","ESP32-S2","",""}
};
