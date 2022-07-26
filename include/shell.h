//  2022 maddsua | https://gitlab.com/maddsua
//	No warranties are given, etc...
//	This file is a component of the AVR Firmware Uploader

#ifndef _MADDSUA_AVRFMU_WINSHELL
#define _MADDSUA_AVRFMU_WINSHELL

#include <stdio.h>
#include <windows.h>
#include <Tlhelp32.h>

#include "staticConfig.hpp"


	void launcher(const char* use_mcu, const char* use_prog, const char* use_speed, const char* use_port, const char* filepath, bool* running, DWORD* exitcode);
	void killProcessByName(const char *filename);

#endif
