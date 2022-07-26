//  2022 maddsua | https://gitlab.com/maddsua
//	No warranties are given, etc...
//	This file is a component of the AVR Firmware Uploader

#include <vector>
#include <string>
#include "staticConfig.hpp"


#ifndef DUDEDEVDATA
#define DUDEDEVDATA

struct arduinoboard{
	
    std::string board;
    std::string mcu;
    std::string ldr;
    std::string speed;
};

std::vector <std::string> db_avrprog = {
	
	"2232HIO","4232h","89isp","abcmini","alf","arduino","arduino-ft232r","arduinoisp",
	"arduinoisporg","atisp","atmelice","atmelice_dw","atmelice_isp","atmelice_pdi",
	"atmelice_updi","avr109","avr910","avr911","avrftdi","avrisp","avrisp2","avrispmkII",
	"avrispv2","bascom","blaster","bsd","buspirate","buspirate_bb","butterfly","butterfly_mk",
	"bwmega","C232HM","c2n232i","dapa","dasa","dasa3","diecimila","dragon_dw","dragon_hvsp",
	"dragon_isp","dragon_jtag","dragon_pdi","dragon_pp","dt006","ehajo-isp","ere-isp-avr",
	"flip1","flip2","frank-stk200","ft232r","ft245r","futurlec","jtag1","jtag1slow","jtag2",
	"jtag2avr32","jtag2dw","jtag2fast","jtag2isp","jtag2pdi","jtag2slow","jtag2updi","jtag3",
	"jtag3dw","jtag3isp","jtag3pdi","jtagkey","jtagmkI","jtagmkII","jtagmkII_avr32","linuxspi",
	"lm3s811","mib510","mkbutterfly","nibobee","o-link","openmoko","pavr","pickit2","picoweb",
	"pony-stk200","ponyser","powerdebugger","powerdebugger_dw","powerdebugger_isp","powerdebugger_pdi",
	"powerdebugger_updi","siprog","sp12","stk200","stk500","stk500hvsp","stk500pp","stk500v1",
	"stk500v2",	"stk600","stk600hvsp","stk600pp","ttl232r","tumpa","UM232H","uncompatino","usbasp",
	"usbasp-clone","usbtiny","wiring","xi,","xplainedmini","xplainedmini_dw","xplainedmini_updi",
	"xplainedpro","xplainedpro_updi"
};

std::vector <std::string> db_avrmcu = {

	"at32uc3a0512","at90can128","at90can32","at90can64","at90pwm2","at90pwm216","at90pwm2b","at90pwm3",
	"at90pwm316","at90pwm3b","at90s1200","at90s2313","at90s2333","at90s2343","at90s4414","at90s4433",
	"at90s4434","at90s8515","at90s8535","at90usb1286","at90usb1287","at90usb162","at90usb646","at90usb647",
	"at90usb82","atmega103","atmega128","atmega1280","atmega1281","atmega1284","atmega1284p","atmega1284rfr2",
	"atmega128rfa1","atmega128rfr2","atmega16","atmega161","atmega162","atmega163","atmega164p","atmega168",
	"atmega168p","atmega168pb","atmega169","atmega16u2","atmega2560","atmega2561","atmega2564rfr2",
	"atmega256rfr2","atmega32","atmega3208","atmega3209","atmega324p","atmega324pa","atmega325","atmega3250",
	"atmega328","atmega328p","atmega328pb","atmega329","atmega3290","atmega3290p","atmega329p","atmega32m1",
	"atmega32u2","atmega32u4","atmega406","atmega48","atmega4808","atmega4809","atmega48p","atmega48pb",
	"atmega64","atmega640","atmega644","atmega644p","atmega644rfr2","atmega645","atmega6450","atmega649",
	"atmega6490","atmega64m1","atmega64rfr2","atmega8","atmega8515","atmega8535","atmega88","atmega88p",
	"atmega88pb","atmega8u2","attiny10","attiny11","attiny12","attiny13","attiny15","attiny1604","attiny1606",
	"attiny1607","attiny1614","attiny1616","attiny1617","attiny1634","attiny20","attiny202","attiny204",
	"attiny212","attiny214","attiny2313","attiny24","attiny25","attiny26","attiny261","attiny28","attiny3214",
	"attiny3216","attiny3217","attiny4","attiny40","attiny402","attiny404","attiny406","attiny412","attiny414",
	"attiny416","attiny417","attiny4313","attiny43u","attiny44","attiny441","attiny45","attiny461","attiny5",
	"attiny804","attiny806","attiny807","attiny814","attiny816","attiny817","attiny84","attiny841","attiny85",
	"attiny861","attiny88","attiny9","atxmega128a1","atxmega128a1revd","atxmega128a1u","atxmega128a3",
	"atxmega128a3u","atxmega128a4","atxmega128a4u","atxmega128b1","atxmega128b3","atxmega128c3","atxmega128d3",
	"atxmega128d4","atxmega16a4","atxmega16a4u","atxmega16c4","atxmega16d4","atxmega16e5","atxmega192a1",
	"atxmega192a3","atxmega192a3u","atxmega192c3","atxmega192d3","atxmega256a1","atxmega256a3","atxmega256a3b",
	"atxmega256a3bu","atxmega256a3u","atxmega256c3","atxmega256d3","atxmega32a4","atxmega32a4u","atxmega32c4",
	"atxmega32d4","atxmega32e5","atxmega384c3","atxmega384d3","atxmega64a1","atxmega64a1u","atxmega64a3",
	"atxmega64a3u","atxmega64a4","atxmega64a4u","atxmega64b1","atxmega64b3","atxmega64c3","atxmega64d3","atxmega64d4"
};

std::vector <std::string> db_speed = {

	"110","300","600","1200","2400","4800",
	"9600","14400","19200","38400","56000",
	"57600","115200","128000","256000"
};

std::vector <arduinoboard> db_arduino = {

	{"Pro/Mini, Fio, Nano, Duemilanove/Diecimila","atmega328p","arduino","57600"},
	{"Uno, Nano, Mini, Ethernet","atmega328p","arduino","115200"},
	{"Arduino Bluetooth","atmega328p","arduino","19200"},
	{"LilyPad, Nano, Mini, Pro/Mini, Duemilanove/Diecimila","atmega168","arduino","19200"},
	{"Yun/Mini, Micro, Leonardo, Industrial, Esplora, Linino","atmega32u4","avr109","57600"},
	{"Arduino Mega, Mega ADK","atmega2560","wiring","115200"},
	{"Arduino Mega","atmega1280","arduino","57600"},
	{"Arduino NG or older","atmegang","arduino","19200"},
	{"Arduino NG or older","atmega8","arduino","19200"},
	{"Arduino Gemma","attiny85","arduino","19200"}
};


#endif
