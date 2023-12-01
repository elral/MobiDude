
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


bool openfile(char* filepath, char* filename);
bool cutFilePath(const char* filepath, char* filename);

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
	
	static std::thread worker;
	static std::vector <std::string> serialPorts;
	static std::vector <std::string> serialPortsProMicro;
	
	static HWND stc_filename;
	static HWND box_avrboard;
	static HWND box_avrprg;
	static HWND box_serspd;
	static HWND box_comport;
	static HWND btn_openfile;
	static HWND btn_flash;
	static HWND btn_help;
	static HWND btn_canc;
	static HWND progbar_flash;
	
	static unsigned int sel_board;
	static unsigned int sel_com;
	
	static char filepath[MAX_PATH];
	static char filename[MAX_PATH];
	
	static unsigned int waitToFlash = 0;
	static bool inProgress = false;
	static DWORD dudeStat = 0;
	
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
				
				MessageBox(NULL, "No serial ports was found on this PC","No COM ports", MB_ICONINFORMATION | MB_OK);
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
			btn_flash = CreateWindow("BUTTON", "Upload", WS_VISIBLE | WS_CHILD, 140, 270, 80, 25, hwnd, (HMENU)GUI_BTN_FLASH, NULL, NULL);
				
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
			for(int i = GUI_GROUP_MAIN; i <= GUI_BTN_FLASH; i++){
				SendDlgItemMessage(hwnd, i, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE,0));
			}
			
			//	grey out inactive buttons
			EnableWindow(GetDlgItem(hwnd, GUI_BTN_FLASH), false);
			
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
								strcpy_s(showfilename, "File: ");
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
							//	zero times							
							waitToFlash = 0;
							
							//	switch controls
							ShowWindow(btn_flash, false);
							ShowWindow(progbar_flash, true);
							ShowWindow(btn_canc, true);
							
							//	step progress bar
							SendMessage(progbar_flash, PBM_SETPOS, 0, 0);
							SetTimer(hwnd, ID_TIMER, progbar_timer_step, NULL);
							
							//	start routine
							inProgress = true;
							dudeStat = 0;
							const char* serialport;
							serialport = serialPorts[sel_com].c_str();
							
							// it's an ProMicro or ESP32-S2, entering bootloader will change the serial port
							if (!strcmp(db_arduino[sel_board].mcu.c_str(), "atmega32u4") || !strcmp(db_arduino[sel_board].programmer.c_str(), "ESP32tool")) {
								// open serial port with 1200 Baud to enter bootloader
								DCB dcb;
								HANDLE hCom;
								BOOL fSuccess;
								
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

								if (!strcmp(db_arduino[sel_board].mcu.c_str(), "atmega32u4")) {
									if (!fSuccess)
									{
										MessageBox(NULL, "SetCommState failed with error", "Open COM port", MB_ICONINFORMATION | MB_OK);
									}

									//  Get the comm config again.
									fSuccess = GetCommState(hCom, &dcb);

									if (!fSuccess)
									{
										MessageBox(NULL, "GetCommState failed with error", "Open COM port", MB_ICONINFORMATION | MB_OK);
										break;
									}

									// wait to appear new COM port
									Sleep(1500);

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
								else if (!strcmp(db_arduino[sel_board].programmer.c_str(), "ESP32tool")) {
									// why does the ESP32 gets an error message for open the port??
									// but it seems that the bootloader gets activates!??
									/*
									if (!fSuccess)
									{
										MessageBox(NULL, "SetCommState failed with error", "Open COM port", MB_ICONINFORMATION | MB_OK);
									}
									*/
									Sleep(1000);
								}
							}

							if (!strcmp(db_arduino[sel_board].programmer.c_str(), "AVRDude") || !strcmp(db_arduino[sel_board].programmer.c_str(), "ESP32tool")) {
								worker = std::thread(launchProgrammer, FinalPath, db_arduino[sel_board].programmer.c_str(), db_arduino[sel_board].mcu.c_str(), db_arduino[sel_board].ldr.c_str(), db_arduino[sel_board].speed.c_str(), serialport, filepath, &inProgress, &dudeStat);
								break;
							} else {
								MessageBox(NULL, "Error! Wrong processor!", "About...", 0);
							}
						}
						
						case GUI_BTN_CANCEL:{
							
							//	cancel upload
							killProcessByName("avrdude.exe");
        					waitToFlash = 0;
							
							break;
						}

					}
					
					break;
				}
			}
			
			break;
		}
			
        case WM_TIMER:{
        	
        	if(!inProgress){
        		
        		//	stop routine
        		worker.join();
        			KillTimer(hwnd, ID_TIMER);
        			waitToFlash = 0;
        			inProgress = false;
        		
        		//	switch controls
        		ShowWindow(btn_canc, false);
				ShowWindow(progbar_flash, false);
				ShowWindow(btn_flash, true);
        		
        		//	show messages
        		if(dudeStat == 0){
        			SendMessage(progbar_flash, PBM_SETPOS, progbar_steps, 0);
        			MessageBox(NULL, "Firmware successfully uploaded","Programmer done", MB_ICONINFORMATION | MB_OK);
					PostQuitMessage(0);
				}
				else{
					SendMessage(progbar_flash, PBM_SETPOS, 0, 0);
					
					switch(dudeStat){
						
						case EC_DUDE_MAIN:{
							MessageBox(NULL, "Port or device is inaccessible","Programmer error", MB_ICONEXCLAMATION | MB_OK);
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
			else{
				
        		//	step pbar forward
        		SendMessage(progbar_flash, PBM_STEPIT, 0, 0);
        		waitToFlash++;
			}
        	
			break;
		}

		case WM_DESTROY: {
			
			//	kill avrdude if its running and exit
			killProcessByName("avrdude.exe");
			PostQuitMessage(0);
			break;
		}
		
		default:
			return DefWindowProc(hwnd, Message, wParam, lParam);
	}

	return 0;
}


bool cutFilePath(const char* filepath, char* filename){
	
	if(strchr(filepath, '\\') != NULL){
		
		const char *slcp = strrchr(filepath, '\\');
		int slcpos = (int)(slcp - filepath);
		slcpos++;
		
		strcpy(filename, filepath + slcpos);
	}
	else{
		return true;
	}
		
	return false;
}


bool openfile(char* filepath, char* filename){
	
	char file[MAX_PATH] = {0};
	OPENFILENAME ofn = {0}; 

		ofn.lStructSize = sizeof(ofn); 
		ofn.hwndOwner = NULL; 
		ofn.lpstrFile = file; 
		ofn.nMaxFile = MAX_PATH; 
		ofn.lpstrFilter = ("Intel HEX\0*.hex\0bin files\0*.bin\0All files\0*.*\0"); 
		ofn.nFilterIndex = 1; 
		ofn.lpstrFileTitle = NULL; 
		ofn.nMaxFileTitle = 0; 
		ofn.lpstrInitialDir = NULL;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
		
	if(GetOpenFileName(&ofn)){

		strcpy(filepath, file);
		
		cutFilePath(filepath, filename);
		
		return true;
	}

	return false;
}
