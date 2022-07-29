#pragma once

#include <vector>
#include <string>

struct arduinoboard{
	
    std::string board;
    std::string mcu;
    std::string ldr;
    std::string speed;
};

std::vector <arduinoboard> db_arduino = {
	{"Arduino Mega2560","atmega2560","wiring","115200"},
//	{"ProMicro","atmega32u4","avr109","57600"},		<- NOT working for now, first bootloader have to entered and then the new COM port has to be detected
	{"Uno","atmega328p","arduino","115200"},
	{"Nano","atmega328p","arduino","115200"},
	{"Nano old Bootloader","atmega328p","arduino","57600"},
	{"Mini","atmega328p","arduino","115200"},
	{"Pro/Mini","atmega328p","arduino","57600"},
};
