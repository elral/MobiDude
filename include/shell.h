#pragma once

#include <stdio.h>
#include <windows.h>
#include <Tlhelp32.h>

#include "staticConfig.h"


void launcherAVR(const char* use_mcu, const char* use_prog, const char* use_speed, const char* use_port, const char* filepath, bool* running, DWORD* exitcode);
void launcherESP32_S2(const char* use_mcu, const char* use_prog, const char* use_speed, const char* use_port, const char* filepath, bool* running, DWORD* exitcode);
void killProcessByName(const char *filename);
