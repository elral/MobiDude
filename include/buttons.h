#pragma once

struct drawArea {
    int x;
    int y;
    int w;
    int h;
};


HWND drawButton(char* className, char* Name, int style, drawArea area, HWND hwnd, HMENU menue);
