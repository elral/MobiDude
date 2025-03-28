#include <windows.h>
#include <windowsx.h>
#include <CommCtrl.h>

#include "include/buttons.h"


HWND drawButton(char* className, char* Name, int style, drawArea area, HWND hwnd, HMENU menue) {
	CreateWindow(className, Name, style, area.x, area.y, area.w, area.w, hwnd, menue, NULL, NULL);
}