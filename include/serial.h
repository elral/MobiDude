#pragma once

#include <stdio.h>
#include <windows.h>

#include <vector>
#include <string>

#include "staticConfig.h"


bool getPorts(std::vector <std::string>* splsarray);
HANDLE openCOMport(const std::string use_port);
int getAvailableCharactersFromCom(HANDLE hCom, char* SerialBuffer, int maxChar2Receive);
int sendCharactersToCom(HANDLE hCom, const char* SerialBuffer, int numberChar2Send);
int getCommandFromCom(HANDLE hCom, char* SerialBuffer);
