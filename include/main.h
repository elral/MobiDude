#pragma once

#include <windows.h>
#include <windowsx.h>
#include <CommCtrl.h>

#include <vector>
#include <fstream>
#include <string>
#include <thread>
#include <iostream>

struct arduinoboard {

    std::string board;
    std::string mcu;
    std::string programmer;
    std::string ldr;
    std::string speed;
};

extern HINSTANCE appInstance;
extern HWND stc_filename;
extern HWND box_avrboard;
extern HWND box_avrprg;
extern HWND box_serspd;
extern HWND box_comport;
extern HWND btn_openfile;
extern HWND btn_flash;
extern HWND btn_terminal;
extern HWND btn_getInfo;
extern HWND btn_help;
extern HWND btn_canc;
extern HWND progbar_flash;
extern unsigned int sel_board;
extern unsigned int sel_com;
extern std::vector <std::string> serialPorts;
extern std::vector <std::string> serialPortsProMicro;

