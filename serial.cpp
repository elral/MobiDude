
#include "include/serial.h"


bool getPorts(std::vector <std::string>* splsarray)
{
	
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


HANDLE openCOMport(const std::string use_port)
{
	DCB dcb;
	HANDLE hCom;
	BOOL fSuccess;

	// CAUTION!! filename for COM ports > 9 must be: "\\.\COM15"
	// this syntax works also for COM ports < 10
	std::string PortNo = "\\\\.\\" + use_port;

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
		return NULL;
	}

	//  Initialize the DCB structure.
	SecureZeroMemory(&dcb, sizeof(DCB));
	dcb.DCBlength = sizeof(DCB);

	//  Build on the current configuration by first retrieving all current settings.
	fSuccess = GetCommState(hCom, &dcb);
	if (!fSuccess)
	{
		MessageBox(NULL, "GetCommState failed with error", "Open COM port", MB_ICONINFORMATION | MB_OK);
		return NULL;
	}
	COMMTIMEOUTS timeouts = { 0 };
	timeouts.ReadIntervalTimeout = 50;
	timeouts.ReadTotalTimeoutConstant = 50;
	timeouts.ReadTotalTimeoutMultiplier = 10;
	timeouts.WriteTotalTimeoutConstant = 50;
	timeouts.WriteTotalTimeoutMultiplier = 10;
	fSuccess = SetCommTimeouts(hCom, &timeouts);
	if (!fSuccess)
	{
		MessageBox(NULL, "Setting Timeout with error", "Open COM port", MB_ICONINFORMATION | MB_OK);
		return NULL;
	}
	//  Fill in some DCB values and set the com state: 
	//  115200 bps, 8 data bits, no parity, and 1 stop bit.
	dcb.BaudRate = CBR_115200;		//  baud rate
	dcb.ByteSize = 8;				//  data size, xmit and rcv
	dcb.Parity = NOPARITY;			//  parity bit
	dcb.StopBits = ONESTOPBIT;		//  stop bit
	fSuccess = SetCommState(hCom, &dcb);
	if (!fSuccess)
	{
		MessageBox(NULL, "Setting COM state with error", "Open COM port", MB_ICONINFORMATION | MB_OK);
		return NULL;
	}
	return hCom;
}

int getAvailableCharactersFromCom(HANDLE hCom, char *SerialBuffer, int maxChar2Receive)
{
	char TempChar = 0x00;
	DWORD NoBytesRead;
	BOOL fSuccess;
	int i = 0;

	do {
		fSuccess = ReadFile(hCom,			//Handle of the Serial port
			&TempChar,					//Temporary character
			sizeof(TempChar),	//Size of TempChar
			&NoBytesRead,		//Number of bytes read
			NULL);
		if (TempChar > 31)
			SerialBuffer[i++] = TempChar;		// Store Tempchar into buffer
	} while (NoBytesRead > 0 && i < maxChar2Receive && fSuccess);

	return i;
}

int sendCharactersToCom(HANDLE hCom, const char* SerialBuffer, int numberChar2Send)
{
	DWORD dNoOfBytesWritten = 0;				// No of bytes written to the port
	BOOL fSuccess;

	fSuccess = WriteFile(hCom,				// Handle to the Serial port
		SerialBuffer,					// Data to be written to the port
		numberChar2Send,						//No of bytes to write
		&dNoOfBytesWritten,	//Bytes written
		NULL);

	return dNoOfBytesWritten;
}

int getCommandFromCom(HANDLE hCom, char* SerialBuffer)
{
	char TempChar = 0x00;
	char CommandChars[10] = { 0 };
	char ParamsChars[1024] = { 0 };
	DWORD NoBytesRead;
	BOOL fSuccess;
	int countChar = 0;

	do {
		fSuccess = ReadFile(hCom,&TempChar, sizeof(TempChar),&NoBytesRead,NULL);
		if (TempChar > 31)
			ParamsChars[countChar++] = TempChar;
	} while (TempChar != ';' && countChar < sizeof(ParamsChars));

	countChar = 0;
	do {
		CommandChars[countChar] = ParamsChars[countChar++];
	} while (ParamsChars[countChar] != ',' && ParamsChars[countChar] != ';' && countChar < sizeof(CommandChars));

	countChar++;
	int i = 0;
	do {
		SerialBuffer[i++] = ParamsChars[countChar++];
	} while (ParamsChars[countChar] != 0);

	return atoi(CommandChars);
}
