//  2022 maddsua | https://gitlab.com/maddsua
//	No warranties are given, etc...
//	This file is a component of the AVR Firmware Uploader

#include "include/serial.h"


bool getPorts(std::vector <std::string>* splsarray){
	
	bool portsAvail = false;
	
	//	clear old list
	while(splsarray->size() > 0){
		splsarray->pop_back();
	}
	
	for(int i = 1; i < scanPorts; i++){
		
		//	generate port name
		char thisPortName[comPortNameSize];
			sprintf_s(thisPortName, "COM%i", i);
		char thisPortPath[comPortNameSize];
			sprintf_s(thisPortPath, "\\\\.\\%s", thisPortName);
		
		//	try to open port
		HANDLE Port  = CreateFile(thisPortPath, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
		
		//	add port to list
		if(Port != INVALID_HANDLE_VALUE){
			
			splsarray->push_back(thisPortName);
			portsAvail = true;
		}
		else{
			
			DWORD lerr = GetLastError();
			
			//	still add port if it is busy but print its name in brackets
			if(lerr == ERROR_ACCESS_DENIED){
				
				std::string tempname = thisPortName;
							tempname += " [BUSY]";
				
				splsarray->push_back(tempname);
				portsAvail = true;
			}
		}
		
		CloseHandle(Port);
	}
	
	return portsAvail;
}
