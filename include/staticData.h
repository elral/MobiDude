#pragma once

#include <vector>
#include <string>

struct arduinoboard{
	
    std::string board;
    std::string mcu;
	std::string programmer;
    std::string ldr;
    std::string speed;
};

std::vector <arduinoboard> db_arduino = {
	{"Arduino Mega2560", "atmega2560", "AVRDude", "wiring", "115200"},
	{"ProMicro", "atmega32u4", "AVRDude", "avr109", "57600"},
	{"Uno", "atmega328p", "AVRDude", "arduino", "115200"},
	{"Nano", "atmega328p", "AVRDude", "arduino", "115200"},
	{"Nano old Bootloader", "atmega328p", "AVRDude", "arduino", "57600"},
	{"Mini", "atmega328p", "AVRDude", "arduino", "115200"},
	{"Pro/Mini", "atmega328p", "AVRDude", "arduino", "57600"},
	{"------", "", "none", "", ""},
	{"ESP32-S2-mini", "ESP32-S2", "ESP32tool", "", ""}
};
