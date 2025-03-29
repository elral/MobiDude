#include <windows.h>
#include <windowsx.h>
#include <CommCtrl.h>

#include "include/main.h"
#include "include/staticConfig.h"
#include "include/buttons.h"
#include "include/FileHandling.h"
#include "include/serial.h"

drawArea boxAvrboardArea = { 40, 170, 200, 380 };
drawArea boxComportArea = { 40, 220, 200, 200 };
drawArea btnOpenfileArea = { 300, 70, 24, 24 };
drawArea btnFlashArea = { 30, 270, 80, 25 };
drawArea btnTerminalArea = { 240, 270, 80, 25 };
drawArea btnGetinfoArea = { 130, 270, 80, 25 };
drawArea btnCancelArea = { 300, 270, 24, 24 };
drawArea progbarFlashArea = { 25, 270, 270, 25 };
drawArea stcFilenameArea = { 25, 73, 265, 20 };
drawArea tipBoardArea = { 40, 155, 130, 16 };
drawArea tipSerialArea = { 40, 205, 140, 16 };
drawArea groupButtonArea = { 25, 125, 300, 130 };

HWND createButton(char* className, char* Name, int style, drawArea area, HWND hwnd, HMENU menue) {
	return CreateWindow(className, Name, style, area.x, area.y, area.w, area.w, hwnd, menue, NULL, NULL);
}

void createAllButtons(HWND hwnd) {
//	draw static
	//	file name string
	stc_filename = createButton("STATIC", "No binary file selected", WS_VISIBLE | WS_CHILD | SS_LEFT | WS_BORDER | SS_NOTIFY, stcFilenameArea, hwnd, (HMENU)GUI_STAT_FILE);
	// tips
	createButton("STATIC", "Board", WS_VISIBLE | WS_CHILD | SS_SIMPLE, tipBoardArea, hwnd, (HMENU)GUI_STAT_TIP_MCU);
	createButton("STATIC", "Serial port", WS_VISIBLE | WS_CHILD | SS_SIMPLE, tipSerialArea, hwnd, (HMENU)GUI_STAT_TIP_COM);
	// group
	createButton("Button", "Select", WS_CHILD | WS_VISIBLE | BS_GROUPBOX, groupButtonArea, hwnd, (HMENU)GUI_GROUP_MAIN);

//	draw dropboxes
	//	Arduino Board dropbox
	box_avrboard = createButton(WC_COMBOBOX, NULL, WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | CBS_SIMPLE | WS_VSCROLL, boxAvrboardArea, hwnd, (HMENU)GUI_BOX_MCU);
	//	com port dropbox
	box_comport = createButton(WC_COMBOBOX, NULL, WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | CBS_SIMPLE | WS_VSCROLL, boxComportArea, hwnd, (HMENU)GUI_BOX_COM);
	for (u_int i = 0; i < serialPorts.size(); i++) {
		SendMessage(box_comport, CB_ADDSTRING, 0, (LPARAM)serialPorts[i].c_str());
	}
	SendMessage(box_comport, CB_SETCURSEL, sel_com, (LPARAM)NULL);

//	draw buttons	
	//	open file button
	btn_openfile = createButton("Button", NULL, WS_VISIBLE | WS_CHILD | BS_BITMAP, btnOpenfileArea, hwnd, (HMENU)GUI_BTN_OPEN);
	HBITMAP	folder_bmp = LoadBitmap(appInstance, MAKEINTRESOURCE(IMG_FOLDER));
	if (folder_bmp != NULL) {
		SendMessage(btn_openfile, BM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)folder_bmp);
	}
	//	upload button
	btn_flash = createButton("Button", "Upload", WS_VISIBLE | WS_CHILD, btnFlashArea, hwnd, (HMENU)GUI_BTN_FLASH);
	//	terminal button
	btn_terminal = createButton("Button", "Terminal", WS_VISIBLE | WS_CHILD, btnTerminalArea, hwnd, (HMENU)GUI_BTN_TERMINAL);
	//	getInfo button
	btn_getInfo = createButton("Button", "Get Info", WS_VISIBLE | WS_CHILD, btnGetinfoArea, hwnd, (HMENU)GUI_BTN_GETINFO);
	//	cancel button
	btn_canc = createButton("STATIC", NULL, SS_BITMAP | WS_CHILD | WS_VISIBLE | SS_NOTIFY, btnCancelArea, hwnd, (HMENU)GUI_BTN_CANCEL);
	HBITMAP	abort_bmp = LoadBitmap(appInstance, MAKEINTRESOURCE(IMG_CANCEL));
	if (abort_bmp != NULL) {
		SendMessage(btn_canc, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)abort_bmp);
	}
	
//	draw progress bar	
	//	progress bar
	progbar_flash = createButton(0, PROGRESS_CLASS, WS_CHILD | WS_VISIBLE | PBS_SMOOTH, progbarFlashArea, hwnd, NULL);
	SendMessage(progbar_flash, PBM_SETRANGE, 0, MAKELPARAM(0, 1000));
	SendMessage(progbar_flash, PBM_SETSTEP, 1, 0);

//	settings	
	//	set fonts
	for (int i = GUI_GROUP_MAIN; i < GUI_GROUP_LAST; i++) {
		SendDlgItemMessage(hwnd, i, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));
	}

//	grey out inactive buttons
	//	EnableWindow(GetDlgItem(hwnd, GUI_BTN_FLASH), false);

	//	hide in-process controls
	ShowWindow(btn_canc, false);
	ShowWindow(progbar_flash, false);
}

void pressButtonOpen(HWND hwnd, char* filepath, char* filename) {
	if (openfile(filepath, filename)) {
		//	display file name
		char showfilename[binfilenamelen];
		strcpy_s(showfilename, filename);
		//	show buttons
		SetWindowText(stc_filename, showfilename);
		EnableWindow(GetDlgItem(hwnd, GUI_BTN_FLASH), true);
	}
	else {
		if (strlen(filename) < 3) {
			EnableWindow(GetDlgItem(hwnd, GUI_BTN_FLASH), false);
		}
	}
}

bool pressButtonFlash(HWND hwnd, char* filepath, char* filename, const char* programmerSelected, const char* boardSelected, const char* mcuSelected,
					  const char* serialport, bool* inProgress, bool* programerStarted, DWORD* dudeStat) {

	if (!strcmp(programmerSelected, "none"))
	{
		return false;
	}
	if (strlen(filename) == 0)
	{
		MessageBox(NULL, "Please choose a Filename", "Programmer error", MB_ICONEXCLAMATION | MB_OK);
		return false;
	}
	if (!strcmp(mcuSelected, "ESP32-S2"))
	{
		std::string checkFileName = filename;
		if (checkFileName.find("merged") > strlen(filename)) {
			MessageBox(NULL, "Please choose a merged .bin File!", "Programmer error", MB_ICONEXCLAMATION | MB_OK);
			return false;
		}
	}

	//	switch controls
	ShowWindow(btn_flash, false);
	ShowWindow(progbar_flash, true);
	ShowWindow(btn_canc, true);
	ShowWindow(btn_terminal, false);
	ShowWindow(btn_getInfo, false);

	//	step progress bar
	SendMessage(progbar_flash, PBM_SETPOS, 0, 0);
	SetTimer(hwnd, ID_TIMER_AVRDUDE, progbar_timer_step, NULL);

	//	start routine
	*inProgress = true;
	*programerStarted = true;
	*dudeStat = 0;
	serialport = serialPorts[sel_com].c_str();

	//MessageBox(NULL, serialport, "Open COM port", MB_ICONINFORMATION | MB_OK);

	// it's an ProMicro or ESP32-S2, entering bootloader will change the serial port
	// or it's a Pico where the flash drive will appear
	if (!strcmp(mcuSelected, "atmega32u4") || !strcmp(programmerSelected, "ESP32tool") || !strcmp(programmerSelected, "Picotool")) {
		// open serial port with 1200 Baud to enter bootloader
		DCB dcb;
		HANDLE hCom;
		BOOL fSuccess;

		DWORD oldDrives = GetCurrentDrives();

		// CAUTION!! filename for COM ports > 9 must be: "\\.\COM15"
		// this syntax works also for COM ports < 10
		std::string PortNo = "\\\\.\\" + serialPorts[sel_com];

		//  Open a handle to the specified com port.
		hCom = CreateFile(PortNo.c_str(),
			GENERIC_READ | GENERIC_WRITE,
			0,				//  must be opened with exclusive-access
			NULL,			//  default security attributes
			OPEN_EXISTING,	//  must use OPEN_EXISTING
			0,				//  not overlapped I/O
			NULL);			//  hTemplate must be NULL for comm devices

		if (hCom == INVALID_HANDLE_VALUE)
		{
			MessageBox(NULL, "Open COM port failed with error", "Open COM port", MB_ICONINFORMATION | MB_OK);
			return false;
		}

		//  Initialize the DCB structure.
		SecureZeroMemory(&dcb, sizeof(DCB));
		dcb.DCBlength = sizeof(DCB);

		//  Build on the current configuration by first retrieving all current
		//  settings.
		fSuccess = GetCommState(hCom, &dcb);

		if (!fSuccess)
		{
			MessageBox(NULL, "GetCommState failed with error", "Open COM port", MB_ICONINFORMATION | MB_OK);
			return false;
		}

		//  Fill in some DCB values and set the com state: 
		//  1200 bps, 8 data bits, no parity, and 1 stop bit.
		dcb.BaudRate = CBR_1200;		//  baud rate
		dcb.ByteSize = 8;				//  data size, xmit and rcv
		dcb.Parity = NOPARITY;			//  parity bit
		dcb.StopBits = ONESTOPBIT;		//  stop bit

		fSuccess = SetCommState(hCom, &dcb);

		// wait to appear new COM port for ProMicro
		// or to enter bootloader for ESP32
		Sleep(1500);
		CloseHandle(hCom);

		if (!strcmp(boardSelected, "Raspberry Pico"))
		{
			DWORD newDrives = GetCurrentDrives();
			// Check old and new drives to see what has changed
			DWORD addedDrives = newDrives & ~oldDrives;  // Drives which were added
			if (checkBootDriveAndCopy(newDrives, oldDrives, filepath, filename)) {
				*dudeStat = 0;
			}
			else {
				*dudeStat = EC_DUDE_MAIN;
			}
			inProgress = false;
			return false;
		}
		else if (!strcmp(mcuSelected, "atmega32u4") || !strcmp(programmerSelected, "ESP32tool")) {
			// and get the new COM port
			getPorts(&serialPortsProMicro);
			u_int i = 0;
			// check which is the new one
			while (!strcmp(serialPorts[i].c_str(), serialPortsProMicro[i].c_str()) && i < serialPortsProMicro.size())
			{
				i++;
				if (i == serialPortsProMicro.size()) break;
			}
			if (i < serialPortsProMicro.size())		// COM port has changed, so ProMicro is NOT already in bootloader mode
			{
				serialport = serialPortsProMicro[i].c_str();
			}
		}
	}
	return true;
}

void pressButtonGetInfo() {
	// open serial port
	BOOL fSuccess;
	HANDLE hCom = openCOMport(serialPorts[sel_com]);
	Sleep(2000);

	char* SerialBuffer;
	char* NameBuffer;
	char* TypeBuffer;
	char* VersionFW;
	char* VersionCore;
	char MessageBuffer[2048] = { 0 };
	char ConfigBuffer[1024] = { 0 };

	// send "9;" to get Board Info
	sendCharactersToCom(hCom, "9;", 2);
	Sleep(100);
	if (getCommandFromCom(hCom, MessageBuffer) == 17)
		getCommandFromCom(hCom, MessageBuffer);
	NameBuffer = strtok(MessageBuffer, ",");
	TypeBuffer = strtok(NULL, ",");
	SerialBuffer = strtok(NULL, ",");
	VersionFW = strtok(NULL, ",");
	VersionCore = strtok(NULL, ";");

	// send "12;" to get Config
	sendCharactersToCom(hCom, "12;", 3);
	Sleep(100);
	getCommandFromCom(hCom, ConfigBuffer);

	CloseHandle(hCom);

	sprintf(MessageBuffer, "%s\r%s\r%s\r%s\r%s\r\r%s", NameBuffer, TypeBuffer, SerialBuffer, VersionFW, VersionCore, ConfigBuffer);

	MessageBox(NULL, MessageBuffer, "Get Board Info", MB_ICONINFORMATION | MB_OK);
}