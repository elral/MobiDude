//  2022 maddsua | https://gitlab.com/maddsua
//	No warranties are given, etc...
//	This file is a component of the AVR Firmware Uploader

#include <windows.h>
#include <windowsx.h>
#include <CommCtrl.h>

#include <vector>
#include <fstream>
#include <string>
#include <thread>

#include "avrfmu_private.h"
#include "include/serial.h"
#include "include/staticData.h"
#include "include/serial.h"
#include "include/shell.h"


bool openfile(char* filepath, char* filename);
bool cutFilePath(const char* filepath, char* filename);

LRESULT CALLBACK WndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);
HINSTANCE appInstance;

std::string openFileAtStartup;


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	
	//	parse start arguments
	if(strlen(lpCmdLine) > 0){
		
		std::string cmdline = lpCmdLine;
		if(cmdline.find(":\\") != std::string::npos && cmdline.find(".hex") != std::string::npos){
			
			openFileAtStartup = cmdline.substr(cmdline.find(":\\") - 1, cmdline.find(".hex") + 3);
		}
	}
	
	
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
	
	//	load banner
	HWND banner;
	HBITMAP	banner_bmp = LoadBitmap(hInstance, MAKEINTRESOURCE(IMG_BANNER));
	
	//	show banner
	if(banner_bmp != NULL){
		banner = CreateWindow("STATIC", NULL, SS_BITMAP | WS_CHILD | WS_VISIBLE, 50, 10, 230, 50, hwnd, NULL, NULL, NULL);	
		SendMessage(banner, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)banner_bmp);
	}
	
		
	while(GetMessage(&msg, NULL, 0, 0) > 0) {
		
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	
	return msg.wParam;
}


LRESULT CALLBACK WndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam){
	
	static std::thread worker;
	static std::vector <std::string> serialPorts;
	
	static HWND stc_filename;
	static HWND stc_boardname;
	static HWND stc_boardicon;
	static HWND box_avrmcu;
	static HWND box_avrprg;
	static HWND box_serspd;
	static HWND box_comport;
	static HWND btn_openfile;
	static HWND btn_flash;
	static HWND btn_savecfg;
	static HWND btn_help;
	static HWND btn_canc;
	static HWND progbar_flash;
	
	static unsigned int sel_mcu;
	static unsigned int sel_prg;
	static unsigned int sel_spd;
	static unsigned int sel_com;
	
	static char filepath[MAX_PATH];
	static char filename[MAX_PATH];
	
	static unsigned int waitToFlash = 0;
	static bool inProgress = false;
	static DWORD dudeStat = 0;
	
	static bool updateboard = false;
	
	switch(Message) {

		case WM_CREATE: {
			
			//	abort if there is no serial ports
			if(!getPorts(&serialPorts)){
				
				MessageBox(NULL, msg_nocomport,"No COM ports", MB_ICONINFORMATION | MB_OK);
				PostQuitMessage(0);
			}
			
			//	set defaults
			sel_mcu = 56;
			sel_prg = 5;
			sel_spd = 11;
			sel_com = serialPorts.size() - 1;
			
		//	draw static
			//	file name string
			stc_filename = CreateWindow("STATIC", msg_nofilesel, WS_VISIBLE | WS_CHILD | SS_LEFT | WS_BORDER | SS_NOTIFY, 25, 73, 265, 20, hwnd, (HMENU)GUI_STAT_FILE, NULL, NULL);
			
			//	board name string
			stc_boardname = CreateWindow("STATIC", "does not match with any board", WS_VISIBLE | WS_CHILD | SS_LEFT | SS_SIMPLE, 55, 101, 265, 18, hwnd, (HMENU)GUI_STAT_BOARD, NULL, NULL);
			
			//	board icon
			stc_boardicon = CreateWindow("STATIC", NULL, SS_BITMAP | WS_CHILD | WS_VISIBLE, 25, 95, 24, 24, hwnd, NULL, NULL, NULL);
				HBITMAP	board_bmp = LoadBitmap(appInstance, MAKEINTRESOURCE(IMG_BOARD));
				if(board_bmp != NULL){
					SendMessage(stc_boardicon, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)board_bmp);
				}
				
				// tips
				CreateWindow("STATIC", "AVR device (MCU)", WS_VISIBLE | WS_CHILD | SS_SIMPLE, 40, 155, 130, 16, hwnd, (HMENU)GUI_STAT_TIP_MCU, NULL, NULL);
				CreateWindow("STATIC", "AVR programmer", WS_VISIBLE | WS_CHILD | SS_SIMPLE, 175, 155, 130, 16, hwnd, (HMENU)GUI_STAT_TIP_PROG, NULL, NULL);
				CreateWindow("STATIC", "Serial speed", WS_VISIBLE | WS_CHILD | SS_SIMPLE, 40, 200, 140, 16, hwnd, (HMENU)GUI_STAT_TIP_SPEED, NULL, NULL);
				CreateWindow("STATIC", "Serial port", WS_VISIBLE | WS_CHILD | SS_SIMPLE, 175, 200, 140, 16, hwnd, (HMENU)GUI_STAT_TIP_COM, NULL, NULL);
				
				// group
				CreateWindow("Button", "Main programmer config", WS_CHILD | WS_VISIBLE | BS_GROUPBOX, 25, 125, 300, 130, hwnd, (HMENU)GUI_GROUP_MAIN, NULL, NULL);
				
			
		//	draw dropboxes	
			//	mcu dropbox
			box_avrmcu = CreateWindow(WC_COMBOBOX, NULL, WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | CBS_SIMPLE | WS_VSCROLL, 40, 170, 130, 380, hwnd, (HMENU)GUI_BOX_MCU, NULL, NULL);
				for(int i = 0; i < db_avrmcu.size(); i++){ 
					SendMessage(box_avrmcu, CB_ADDSTRING, 0, (LPARAM) db_avrmcu[i].c_str());
				}
				SendMessage(box_avrmcu, CB_SETCURSEL , sel_mcu, (LPARAM)NULL);
				
			//	programmer dropbox
			box_avrprg = CreateWindow(WC_COMBOBOX, NULL, WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | CBS_SIMPLE | WS_VSCROLL, 175, 170, 130, 200, hwnd, (HMENU)GUI_BOX_PRG, NULL, NULL);
				for(int i = 0; i < db_avrprog.size(); i++){ 
					SendMessage(box_avrprg, CB_ADDSTRING, 0, (LPARAM) db_avrprog[i].c_str());
				}
				SendMessage(box_avrprg, CB_SETCURSEL , sel_prg, (LPARAM)NULL);
			
			//	serial speed dropbox
			box_serspd = CreateWindow(WC_COMBOBOX, NULL, WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | CBS_SIMPLE | WS_VSCROLL, 40, 215, 130, 200, hwnd, (HMENU)GUI_BOX_SPD, NULL, NULL);
				for(int i = 0; i < db_speed.size(); i++){ 
					
					std::string tempSpeedStr = db_speed[i] + " baud";
					SendMessage(box_serspd, CB_ADDSTRING, 0, (LPARAM) tempSpeedStr.c_str());
				}
				SendMessage(box_serspd, CB_SETCURSEL , sel_spd, (LPARAM)NULL);
				
			//	com port dropbox
			box_comport = CreateWindow(WC_COMBOBOX, NULL, WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | CBS_SIMPLE | WS_VSCROLL, 175, 215, 130, 200, hwnd, (HMENU)GUI_BOX_COM, NULL, NULL);
				for(int i = 0; i < serialPorts.size(); i++){ 
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
			btn_flash = CreateWindow("BUTTON", "Upload", WS_VISIBLE | WS_CHILD, 85, 270, 80, 25, hwnd, (HMENU)GUI_BTN_FLASH, NULL, NULL);
			
			//	save config button
			btn_savecfg = CreateWindow("BUTTON", "Save config", WS_VISIBLE | WS_CHILD, 180, 270, 80, 25, hwnd, (HMENU)GUI_BTN_SVCFG, NULL, NULL);
			
			//	help button
			btn_help = CreateWindow("STATIC", NULL, SS_BITMAP | WS_CHILD | WS_VISIBLE | SS_NOTIFY, 310, 10, 24, 24, hwnd, (HMENU)GUI_BTN_HELP, NULL, NULL);
				HBITMAP	question_bmp = LoadBitmap(appInstance, MAKEINTRESOURCE(IMG_HELP));
				if(question_bmp != NULL){
					SendMessage(btn_help, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)question_bmp);
				}
				
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
			for(int i = GUI_GROUP_MAIN; i <= GUI_BTN_SVCFG; i++){
				SendDlgItemMessage(hwnd, i, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE,0));
			}
			
			//	grey out inactive buttons
			EnableWindow(GetDlgItem(hwnd, GUI_BTN_FLASH), false);
			EnableWindow(GetDlgItem(hwnd, GUI_BTN_SVCFG), false);
			
			//	hide in-process controls
			ShowWindow(btn_canc, false);
			ShowWindow(progbar_flash, false);
			ShowWindow(stc_boardname, false);
			ShowWindow(stc_boardicon, false);
			
			
		//	load startup file
			if(openFileAtStartup.size() > 8 && openFileAtStartup.size() < MAX_PATH){
				
				strcpy(filepath, openFileAtStartup.c_str());
				cutFilePath(openFileAtStartup.c_str(), filename);
				
				//	black magic trick
				goto loadbinfile;
			}
					
			break;
		}
	
		case WM_COMMAND: {
			
			switch(HIWORD(wParam)){
			
				//	dropboxes
				case LBN_SELCHANGE:{
					
					switch(LOWORD(wParam)){
						
						case GUI_BOX_MCU:{
							
							sel_mcu = (int) SendMessage(box_avrmcu, CB_GETCURSEL, 0, 0);
							updateboard = true;
							
							break;
						}
						case GUI_BOX_PRG:{
							
							sel_prg = (int) SendMessage(box_avrprg, CB_GETCURSEL, 0, 0);
							updateboard = true;
							
							break;
						}
						case GUI_BOX_SPD:{
							
							sel_spd = (int) SendMessage(box_serspd, CB_GETCURSEL, 0, 0);
							updateboard = true;
							
							break;
						}
						case GUI_BOX_COM:{
							
							sel_com = (int) SendMessage(box_comport, CB_GETCURSEL, 0, 0);
							updateboard = true;
							
							break;
						}
					}
					
					break;
				}
				
				//	buttons
				case BN_CLICKED:{
					
					switch(LOWORD(wParam)){
						
						case GUI_BTN_OPEN:{
						openbinfile:	
						
							if(openfile(filepath, filename)){
								
							loadbinfile:
								
								//	display file name
								char showfilename[binfilenamelen];
									strcpy(showfilename, "File: ");
									strcpy(showfilename, filename);
									
									
								//	try to load config file
								std::string configFilePath = filepath;
								if(configFilePath.find_last_of(".") != std::string::npos){
									
									//	cut extension
									short wipefrom = configFilePath.find_last_of(".");
									configFilePath.erase(wipefrom, configFilePath.size() - 1);
									
									//	add new extension
									configFilePath += ".avrfw";
									
									//	try to open
									std::ifstream configFile(configFilePath, std::ios::in);
									
									if(configFile.is_open()){
										
										std::string configFileContents;
										
										while(!configFile.eof()){
											
											//	read string
											std::string txttempstr;
											configFile >> txttempstr;
											
											//	append
											configFileContents += txttempstr + "\n";
										}
											
										if(configFileContents.size() > strlen(configFileHeader)){
											
											//	compare mcu
											std::string search_pattern_mcu = "MCU=";
											for(int i = 0; i < db_avrmcu.size(); i++){
											
												if(configFileContents.find(search_pattern_mcu + db_avrmcu[i] + "\n") != std::string::npos){
													
													sel_mcu = i;
													SendMessage(box_avrmcu, CB_SETCURSEL , sel_mcu, (LPARAM)NULL);
													break;
												}
											}
											//	compare prorammer
											std::string search_pattern_loader = "LDR=";
											for(int i = 0; i < db_avrprog.size(); i++){
											
												if(configFileContents.find(search_pattern_loader + db_avrprog[i] + "\n") != std::string::npos){
													
													sel_prg = i;
													SendMessage(box_avrprg, CB_SETCURSEL , sel_prg, (LPARAM)NULL);
													break;
												}
											}
											//	compare speed
											std::string search_pattern_speed = "SPD=";
											for(int i = 0; i < db_speed.size(); i++){
											
												if(configFileContents.find(search_pattern_speed + db_speed[i] + "\n") != std::string::npos){
													
													sel_spd = i;
													SendMessage(box_serspd, CB_SETCURSEL , sel_spd, (LPARAM)NULL);
													break;
												}
											}
											
										}
										configFile.close();
									}
								}
								
								updateboard = true;
								
								//	show buttons
								SetWindowText(stc_filename, showfilename);
								EnableWindow(GetDlgItem(hwnd, GUI_BTN_FLASH), true);
								EnableWindow(GetDlgItem(hwnd, GUI_BTN_SVCFG), true);
							}
							else{
								
								if(strlen(filename) < 3){
									EnableWindow(GetDlgItem(hwnd, GUI_BTN_FLASH), false);
									EnableWindow(GetDlgItem(hwnd, GUI_BTN_SVCFG), false);
								}
							}
							
							break;
						}
						
						case GUI_STAT_FILE:{
							
							//	black magic trick again
							goto openbinfile;
							break;
						}
						
						case GUI_BTN_FLASH:{
							
							//	zero times							
							waitToFlash = 0;
							
							//	switch controls
							ShowWindow(btn_flash, false);
							ShowWindow(btn_savecfg, false);
							ShowWindow(progbar_flash, true);
							ShowWindow(btn_canc, true);
							
							//	step progress bar
							SendMessage(progbar_flash, PBM_SETPOS, 0, 0);
							SetTimer(hwnd, ID_TIMER, progbar_timer_step, NULL);
							
							//	start routine
							inProgress = true;
							dudeStat = 0;
							worker = std::thread(launcher, db_avrmcu[sel_mcu].c_str(), db_avrprog[sel_prg].c_str(), db_speed[sel_spd].c_str(), serialPorts[sel_com].c_str(), filepath, &inProgress, &dudeStat);
							
							break;
						}
						
						case GUI_BTN_SVCFG:{
										
							std::string configFilePath = filepath;
							
								if(configFilePath.find_last_of(".") != std::string::npos){
									
									//	cut extension
									short wipefrom = configFilePath.find_last_of(".");
									configFilePath.erase(wipefrom, configFilePath.size() - 1);
									
									//	add new extension
									configFilePath += ".avrfw";
									
									//	format contents
									std::string configFileContents = configFileHeader;
												configFileContents += filename;
												configFileContents += "\nMCU=" + db_avrmcu[sel_mcu] + "\nLDR=" + db_avrprog[sel_prg] + "\nSPD=" + db_speed[sel_spd] + "\n";
											
									//	write file	
									std::ofstream configFile(configFilePath, std::ios::out);
    								
    								if(configFile.is_open()){
    									
    									//	write and close
    									configFile << configFileContents;
    									configFile.close();
    									
    									//	report
    									MessageBox(NULL, "Config saved","Config saved", MB_OK);
									}
									else{
										MessageBox(NULL, "Unable to save config","Error!", MB_ICONEXCLAMATION | MB_OK);
									}
								}
							
							break;
						}
						
						case GUI_BTN_HELP:{
							
							char msgabout[512];
								sprintf(msgabout, "%s v%i.%i.%i\nA graphical interface for avrdude\n\n%s\n%s", PRODUCT_NAME, VER_MAJOR, VER_MINOR, VER_RELEASE, VER_AUTHSTAMP, LEGAL_COPYRIGHT);
							
							MessageBox(NULL, msgabout, "About...", 0);
							break;
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
				ShowWindow(btn_savecfg, true);
        		
        		//	show messages
        		if(dudeStat == 0){
        			SendMessage(progbar_flash, PBM_SETPOS, progbar_steps, 0);
        			MessageBox(NULL, "Firmware successfully uploaded","avrdude done", MB_ICONINFORMATION | MB_OK);
				}
				else{
					SendMessage(progbar_flash, PBM_SETPOS, 0, 0);
					
					switch(dudeStat){
						
						case EC_DUDE_MAIN:{
							MessageBox(NULL, "Port or device is inaccessible","avrdude error", MB_ICONEXCLAMATION | MB_OK);
							break;
						}
						
						case EC_DUDE_TIMEOUT:{
							MessageBox(NULL, "Connection timed out","avrdude error", MB_ICONERROR | MB_OK);
							break;
						}
						
						case EC_DUDE_NOEXEC:{
							MessageBox(NULL, "Unable to start avrdude","avrdude error", MB_ICONERROR | MB_OK);
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
	
	//	update board name
	if(updateboard){
		
		updateboard = false;
		
		bool matched = false;
			ShowWindow(stc_boardname, false);
			ShowWindow(stc_boardicon, false);
		
		for(short i = 0; i < db_arduino.size(); i++){
			
			if(db_arduino[i].mcu == db_avrmcu[sel_mcu] && db_arduino[i].ldr == db_avrprog[sel_prg] && db_arduino[i].speed == db_speed[sel_spd]){
				
				SetWindowText(stc_boardname, db_arduino[i].board.c_str());
				matched = true;
				
				//MessageBox(NULL, "we are here","avrdude error", MB_OK);
				break;
			}
		}
		
		if(matched){
			
			ShowWindow(stc_boardname, true);
			ShowWindow(stc_boardicon, true);
		}
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
		ofn.lpstrFilter = ("Intel HEX\0*.hex\0All files\0*.*\0"); 
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

