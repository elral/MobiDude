
#include "include/main.h"
#include "include/staticConfig.h"
#include "include/staticData.h"
#include "include/serial.h"
#include "include/shell.h"
#include "include/FileHandling.h"
#include "include/buttons.h"


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

HWND stc_filename;
HWND box_avrboard;
HWND box_avrprg;
HWND box_serspd;
HWND box_comport;
HWND btn_openfile;
HWND btn_flash;
HWND btn_terminal;
HWND btn_getInfo;
HWND btn_help;
HWND btn_canc;
HWND progbar_flash;
unsigned int sel_board;
unsigned int sel_com;
std::vector <std::string> serialPorts;
std::vector <std::string> serialPortsProMicro;

LRESULT CALLBACK WndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam){
	
	static std::thread workerProgramer;
	static std::thread workerTerminal;
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
			createAllButtons(hwnd);
			for (u_int i = 0; i < db_arduino.size(); i++) {
				SendMessage(box_avrboard, CB_ADDSTRING, 0, (LPARAM)db_arduino[i].board.c_str());
			}
			SendMessage(box_avrboard, CB_SETCURSEL, sel_board, (LPARAM)NULL);
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
							pressButtonOpen(hwnd, filepath, filename);
							break;
						}
						case GUI_BTN_FLASH: {
							//	zero times							
							waitToFlash = 0;
							const char* serialport;
							if (pressButtonFlash(hwnd, filepath, filename, db_arduino[sel_board].programmer.c_str(), db_arduino[sel_board].board.c_str(), db_arduino[sel_board].mcu.c_str(), serialport, &inProgress, &programerStarted, &dudeStat)) {
								workerProgramer = std::thread(launchProgrammer, FinalPath, db_arduino[sel_board].programmer.c_str(), db_arduino[sel_board].mcu.c_str(), db_arduino[sel_board].ldr.c_str(), db_arduino[sel_board].speed.c_str(), serialport, filepath, &inProgress, &dudeStat);
							}
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
							void pressButtonGetInfo();
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
