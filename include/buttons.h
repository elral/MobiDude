#pragma once

struct drawArea {
    int x;
    int y;
    int w;
    int h;
};


HWND createButton(char* className, char* Name, int style, drawArea area, HWND hwnd, HMENU menue);
void createAllButtons(HWND hwnd);
void pressButtonOpen(HWND hwnd, char* filepath, char* filename);
bool pressButtonFlash(HWND hwnd, char* filepath, char* filename, const char* programmerSelected, const char* boardSelected, const char* mcuSelected, const char* serialport, bool* inProgress, bool* programerStarted, DWORD* dudeStat);
void pressButtonGetInfo();

