
#include <windows.h>
#include <windowsx.h>
#include <CommCtrl.h>

#include <vector>
#include <fstream>
#include <string>
#include <thread>
#include <iostream>

#include "include/staticConfig.h"
#include "include/staticData.h"
#include "include/serial.h"
#include "include/shell.h"
#include "include/FileHandling.h"


LRESULT CALLBACK WndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);
HINSTANCE appInstance;

std::string openFileAtStartup;
TCHAR fullPath[MAX_PATH] = {0};
TCHAR driveLetter[3];
TCHAR directory[MAX_PATH];
TCHAR FinalPath[MAX_PATH];

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	
	WNDCLASSEX wc = {0};
	MSG msg;

	wc.cbSize		 = sizeof(WNDCLASSEX);
	wc.lpfnWndProc	 = WndProc;
	wc.hInstance	 = hInstance;
	wc.hCursor		 = LoadCursor(NULL, IDC_ARROW);
	
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszClassName = "WindowClass";
	wc.hIcon		 = LoadIcon(hInstance, "APPICON");
	wc.hIconSm		 = LoadIcon(hInstance, "APPICON");
	
	if(!RegisterClassEx(&wc)) {
		
		MessageBox(NULL, "Window Registration Failed!","Error!", MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}
	
	//	export instance to global space to use in WM_CREATE
	appInstance = hInstance;
	
	//	calc window position (1/8 left; 1/10 top)
    int winPosx = (GetSystemMetrics(SM_CXSCREEN) / 2) - (windowSizeX);
    int winPosy = (GetSystemMetrics(SM_CYSCREEN) / 2) - (windowSizeY / 1.2);

	HWND hwnd = CreateWindowEx(WS_EX_CLIENTEDGE, "WindowClass" , PRODUCT_NAME, WS_VISIBLE | WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
				winPosx, winPosy, windowSizeX, windowSizeY,	NULL, NULL, hInstance, NULL);

	if(hwnd == NULL) {
		
		MessageBox(NULL, "Window Creation Failed!","Error!", MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}
/*
	//	load banner
	HWND banner;
	HBITMAP	banner_bmp = LoadBitmap(hInstance, MAKEINTRESOURCE(IMG_BANNER));
	
	//	show banner
	if(banner_bmp != NULL){
		banner = CreateWindow("STATIC", NULL, SS_BITMAP | WS_CHILD | WS_VISIBLE, 50, 10, 230, 50, hwnd, NULL, NULL, NULL);	
		SendMessage(banner, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)banner_bmp);
	}
*/

	while(GetMessage(&msg, NULL, 0, 0) > 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	
	return msg.wParam;
}


LRESULT CALLBACK WndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam){
	
	static std::thread workerProgramer;
	static std::thread workerTerminal;
	static std::vector <std::string> serialPorts;
	static std::vector <std::string> serialPortsProMicro;
	
	static HWND stc_filename;
	static HWND box_avrboard;
	static HWND box_avrprg;
	static HWND box_serspd;
	static HWND box_comport;
	static HWND btn_openfile;
	static HWND btn_flash;
	static HWND btn_terminal;
	static HWND btn_getInfo;
	static HWND btn_help;
	static HWND btn_canc;
	static HWND progbar_flash;
	
	static unsigned int sel_board;
	static unsigned int sel_com;
	
	static char filepath[MAX_PATH];
	static char filename[MAX_PATH];
	
	static unsigned int waitToFlash = 0;
	static bool inProgress = false;
	static bool programerStarted = false;
	static DWORD dudeStat = 0;
	static DWORD terminalStat = 0;
	static bool terminalRunning = false;
	static bool terminalStarted = false;
	static bool getInfoRunning = false;
	static bool getInfoStarted = false;
	
	//GetfullPath(1024, fullPath);
	GetModuleFileName(NULL, fullPath, MAX_PATH);
	_splitpath(fullPath, driveLetter, directory, NULL, NULL);
	sprintf(FinalPath, "%s%s", driveLetter, directory);
	
	//std::wstring::size_type pos = std::wstring(fullPath).find_last_of(L"\\/");
	//std::wstring(fullPath).substr(0, pos);


	switch(Message) {

		case WM_CREATE: {
			
			//	abort if there is no serial ports
			if(!getPorts(&serialPorts)){
				
				MessageBox(NULL, "No serial port was found on this PC","No COM ports", MB_ICONINFORMATION | MB_OK);
				PostQuitMessage(0);
			}
			
			//	set defaults
			sel_board = 0;
			sel_com = serialPorts.size() - 1;
			
		//	draw static
			//	file name string
			stc_filename = CreateWindow("STATIC", "No binary file selected", WS_VISIBLE | WS_CHILD | SS_LEFT | WS_BORDER | SS_NOTIFY, 25, 73, 265, 20, hwnd, (HMENU)GUI_STAT_FILE, NULL, NULL);

			// tips
			CreateWindow("STATIC", "Board", WS_VISIBLE | WS_CHILD | SS_SIMPLE, 40, 155, 130, 16, hwnd, (HMENU)GUI_STAT_TIP_MCU, NULL, NULL);
			CreateWindow("STATIC", "Serial port", WS_VISIBLE | WS_CHILD | SS_SIMPLE, 40, 205, 140, 16, hwnd, (HMENU)GUI_STAT_TIP_COM, NULL, NULL);
				
			// group
			CreateWindow("Button", "Select", WS_CHILD | WS_VISIBLE | BS_GROUPBOX, 25, 125, 300, 130, hwnd, (HMENU)GUI_GROUP_MAIN, NULL, NULL);
				
		//	draw dropboxes	
			//	Arduino Board dropbox
			box_avrboard = CreateWindow(WC_COMBOBOX, NULL, WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | CBS_SIMPLE | WS_VSCROLL, 40, 170, 200, 380, hwnd, (HMENU)GUI_BOX_MCU, NULL, NULL);
				for (u_int i = 0; i < db_arduino.size(); i++) {
					SendMessage(box_avrboard, CB_ADDSTRING, 0, (LPARAM)db_arduino[i].board.c_str());
				}
				SendMessage(box_avrboard, CB_SETCURSEL , sel_board, (LPARAM)NULL);
		
			//	com port dropbox
			box_comport = CreateWindow(WC_COMBOBOX, NULL, WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | CBS_SIMPLE | WS_VSCROLL, 40, 220, 200, 200, hwnd, (HMENU)GUI_BOX_COM, NULL, NULL);
				for(u_int i = 0; i < serialPorts.size(); i++){ 
					SendMessage(box_comport, CB_ADDSTRING, 0, (LPARAM) serialPorts[i].c_str());
				}
				SendMessage(box_comport, CB_SETCURSEL , sel_com, (LPARAM)NULL);
			
			
		//	draw buttons	
			//	open file button
			btn_openfile = CreateWindow("BUTTON", NULL, WS_VISIBLE | WS_CHILD | BS_BITMAP, 300, 70, 24, 24, hwnd, (HMENU)GUI_BTN_OPEN, NULL, NULL);
				HBITMAP	folder_bmp = LoadBitmap(appInstance, MAKEINTRESOURCE(IMG_FOLDER));
				if(folder_bmp != NULL){
					SendMessage(btn_openfile, BM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)folder_bmp);
				}
			
			//	upload button
			btn_flash = CreateWindow("BUTTON", "Upload", WS_VISIBLE | WS_CHILD, 30, 270, 80, 25, hwnd, (HMENU)GUI_BTN_FLASH, NULL, NULL);

			//	terminal button
			btn_terminal = CreateWindow("BUTTON", "Terminal", WS_VISIBLE | WS_CHILD, 240, 270, 80, 25, hwnd, (HMENU)GUI_BTN_TERMINAL, NULL, NULL);

			//	getInfo button
			btn_getInfo = CreateWindow("BUTTON", "Get Info", WS_VISIBLE | WS_CHILD, 130, 270, 80, 25, hwnd, (HMENU)GUI_BTN_GETINFO, NULL, NULL);
				
			//	cancel button
			btn_canc = CreateWindow("STATIC", NULL, SS_BITMAP | WS_CHILD | WS_VISIBLE | SS_NOTIFY, 300, 270, 24, 24, hwnd, (HMENU)GUI_BTN_CANCEL, NULL, NULL);
				HBITMAP	abort_bmp = LoadBitmap(appInstance, MAKEINTRESOURCE(IMG_CANCEL));
				if(abort_bmp != NULL){
					SendMessage(btn_canc, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)abort_bmp);
				}
			
		//	draw progress bar	
			//	progress bar
			progbar_flash = CreateWindowEx(0, PROGRESS_CLASS, NULL, WS_CHILD | WS_VISIBLE | PBS_SMOOTH, 25, 270, 270, 25, hwnd, NULL, NULL, NULL);   
				SendMessage(progbar_flash, PBM_SETRANGE, 0, MAKELPARAM(0, 1000));
				SendMessage(progbar_flash, PBM_SETSTEP, 1, 0);
			
		//	settings	
			//	set fonts
			for(int i = GUI_GROUP_MAIN; i < GUI_GROUP_LAST; i++){
				SendDlgItemMessage(hwnd, i, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE,0));
			}
			
			//	grey out inactive buttons
		//	EnableWindow(GetDlgItem(hwnd, GUI_BTN_FLASH), false);
			
			//	hide in-process controls
			ShowWindow(btn_canc, false);
			ShowWindow(progbar_flash, false);

			break;
		}
	
		case WM_COMMAND: {
			
			switch(HIWORD(wParam)){
			
				//	dropboxes
				case LBN_SELCHANGE:{
					switch(LOWORD(wParam)){
						case GUI_BOX_MCU:{
							sel_board = (int) SendMessage(box_avrboard, CB_GETCURSEL, 0, 0);
							break;
						}
						case GUI_BOX_COM:{
							sel_com = (int) SendMessage(box_comport, CB_GETCURSEL, 0, 0);
							break;
						}
					}

					break;
				}
				
				//	buttons
				case BN_CLICKED:{
					switch(LOWORD(wParam)){
						case GUI_BTN_OPEN:{
							if(openfile(filepath, filename)){
								//	display file name
								char showfilename[binfilenamelen];
								strcpy_s(showfilename, filename);
								//	show buttons
								SetWindowText(stc_filename, showfilename);
								EnableWindow(GetDlgItem(hwnd, GUI_BTN_FLASH), true);
							}
							else{
								
								if(strlen(filename) < 3){
									EnableWindow(GetDlgItem(hwnd, GUI_BTN_FLASH), false);
								}
							}

							break;
						}
						case GUI_BTN_FLASH:{
							if (!strcmp(db_arduino[sel_board].programmer.c_str(), "none"))
							{
								break;
							}
							if (strlen(filename) == 0 && strcmp(db_arduino[sel_board].board.c_str(),"Raspberry Pico"))
							{
								MessageBox(NULL, "Please choose a Filename", "Programmer error", MB_ICONEXCLAMATION | MB_OK);
								break;
							}
							if (!strcmp(db_arduino[sel_board].mcu.c_str(), "ESP32-S2"))
							{
								std::string checkFileName = filename;
								if (checkFileName.find("merged") > strlen(filename)) {
									MessageBox(NULL, "Please choose a merged .bin File!", "Programmer error", MB_ICONEXCLAMATION | MB_OK);
									break;
								}
							}

							//	zero times							
							waitToFlash = 0;
							
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
							inProgress = true;
							programerStarted = true;
							dudeStat = 0;
							const char* serialport;
							serialport = serialPorts[sel_com].c_str();
							
							// it's an ProMicro or ESP32-S2, entering bootloader will change the serial port
							// or it's a Pico where the flash drive will appear
							if (!strcmp(db_arduino[sel_board].mcu.c_str(), "atmega32u4") || !strcmp(db_arduino[sel_board].programmer.c_str(), "ESP32tool") || !strcmp(db_arduino[sel_board].programmer.c_str(), "Picotool")) {
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
									MessageBox(NULL, "Open COM port failed with error","Open COM port", MB_ICONINFORMATION | MB_OK);
									break;
								}

								//  Initialize the DCB structure.
								SecureZeroMemory(&dcb, sizeof(DCB));
								dcb.DCBlength = sizeof(DCB);

								//  Build on the current configuration by first retrieving all current
								//  settings.
								fSuccess = GetCommState(hCom, &dcb);

								if (!fSuccess)
								{
									MessageBox(NULL, "GetCommState failed with error","Open COM port", MB_ICONINFORMATION | MB_OK);
									break;
								}

								//  Fill in some DCB values and set the com state: 
								//  1200 bps, 8 data bits, no parity, and 1 stop bit.
								dcb.BaudRate = CBR_1200;		//  baud rate
								dcb.ByteSize = 8;				//  data size, xmit and rcv
								dcb.Parity = NOPARITY;			//  parity bit
								dcb.StopBits = ONESTOPBIT;		//  stop bit

								fSuccess = SetCommState(hCom, &dcb);

								// wait to appear new COM port
								Sleep(1500);
								
								if (!strcmp(db_arduino[sel_board].board.c_str(), "Raspberry Pico"))
								{
									DWORD newDrives = GetCurrentDrives();
									// Check old and new drives to see what has changed
									DWORD addedDrives = newDrives & ~oldDrives;  // Drives which were added
									if (checkBootDriveAndCopy(newDrives, oldDrives, filepath, filename)) {
										dudeStat = 0;
									}
									else {
										dudeStat = EC_DUDE_MAIN;
									}
									CloseHandle(hCom);
									//Sleep(1500);
									inProgress = false;
									break;
								} else {
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
									CloseHandle(hCom);
								}
							}

							workerProgramer = std::thread(launchProgrammer, FinalPath, db_arduino[sel_board].programmer.c_str(), db_arduino[sel_board].mcu.c_str(), db_arduino[sel_board].ldr.c_str(), db_arduino[sel_board].speed.c_str(), serialport, filepath, &inProgress, &dudeStat);
							break;

						}

						case GUI_BTN_TERMINAL: {
							SetTimer(hwnd, ID_TIMER_TERMINAL, progbar_timer_step, NULL);
							EnableWindow(GetDlgItem(hwnd, GUI_BTN_TERMINAL), false);
							EnableWindow(GetDlgItem(hwnd, GUI_BTN_FLASH), false);
							EnableWindow(GetDlgItem(hwnd, GUI_BTN_GETINFO), false);
							terminalRunning = true;
							terminalStarted = true;
							workerTerminal = std::thread(launchTerminal, FinalPath, serialPorts[sel_com].c_str(), &terminalStat, &terminalRunning);
							break;
						}
						
						case GUI_BTN_GETINFO: {
							// open serial port
							BOOL fSuccess;
							HANDLE hCom = openCOMport(serialPorts[sel_com]);
							Sleep(2000);

							char *SerialBuffer;
							char *NameBuffer;
							char *TypeBuffer;
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

							break;
						}

						case GUI_BTN_CANCEL:{
							
							//	cancel upload
							if (!strcmp(db_arduino[sel_board].programmer.c_str(), "AVRDude")) {
								killProcessByName("avrdude.exe");
							}
							else if (!strcmp(db_arduino[sel_board].programmer.c_str(), "ESP32tool")) {
								killProcessByName("python.exe");
							}
							break;
						}

					}
					
					break;
				}
			}
			
			break;
		}
			
        case WM_TIMER:{
			if (!terminalRunning && terminalStarted) {
				workerTerminal.join();
				KillTimer(hwnd, ID_TIMER_TERMINAL);
				EnableWindow(GetDlgItem(hwnd, GUI_BTN_TERMINAL), true);
				EnableWindow(GetDlgItem(hwnd, GUI_BTN_GETINFO), true);
				if (strlen(filename) > 2) {
					EnableWindow(GetDlgItem(hwnd, GUI_BTN_FLASH), true);
				}
				terminalStarted = false;
			}

        	if(!inProgress && programerStarted){
        		//	stop routine
				if (strcmp(db_arduino[sel_board].board.c_str(), "Raspberry Pico")) {
					workerProgramer.join();
				}
        		KillTimer(hwnd, ID_TIMER_AVRDUDE);
        		waitToFlash = 0;
        		inProgress = false;
				programerStarted = false;
        		
        		//	switch controls
        		ShowWindow(btn_canc, false);
				ShowWindow(progbar_flash, false);
				ShowWindow(btn_flash, true);
				ShowWindow(btn_terminal, true);
				ShowWindow(btn_getInfo, true);
        		
        		//	show messages
        		if(dudeStat == 0){
        			SendMessage(progbar_flash, PBM_SETPOS, progbar_steps, 0);
        			MessageBox(NULL, "Firmware successfully uploaded","Programmer done", MB_ICONINFORMATION | MB_OK);
				}
				else{
					SendMessage(progbar_flash, PBM_SETPOS, 0, 0);
					
					switch(dudeStat){
						
						case EC_DUDE_MAIN:{
							if (!strcmp(db_arduino[sel_board].programmer.c_str(), "AVRDude")) {
								MessageBox(NULL, "Port or device is inaccessible","Programmer error", MB_ICONEXCLAMATION | MB_OK);
							}
							else if (!strcmp(db_arduino[sel_board].programmer.c_str(), "ESP32tool")) {
								MessageBox(NULL, "Can't leaving Bootloader\nPlease do a manuel reset", "Programmer Info", MB_ICONEXCLAMATION | MB_OK);
							}
							else if (!strcmp(db_arduino[sel_board].programmer.c_str(), "Raspberry Pico")) {
								MessageBox(NULL, "Failure while copying uf2 file", "Programmer Info", MB_ICONEXCLAMATION | MB_OK);
							}
							break;
						}
						
						case EC_DUDE_TIMEOUT:{
							MessageBox(NULL, "Connection timed out","Programmer error", MB_ICONERROR | MB_OK);
							break;
						}
						
						case EC_DUDE_NOEXEC:{
							MessageBox(NULL, "Unable to start Programmer","Programmer error", MB_ICONERROR | MB_OK);
							break;
						}
					}
				}
			}
			else {

        		//	step pbar forward
        		SendMessage(progbar_flash, PBM_STEPIT, 0, 0);
        		waitToFlash++;
			}
        	
			break;
		}

		case WM_DESTROY: {
			
			//	kill avrdude, esptool or Putty if its running and exit
			killProcessByName("avrdude.exe");
			killProcessByName("python.exe");
			killProcessByName("PuTTYPortable.exe");
			PostQuitMessage(0);
			break;
		}
		
		default:
			return DefWindowProc(hwnd, Message, wParam, lParam);
	}

	return 0;
}
