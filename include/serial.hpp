//  2022 maddsua | https://gitlab.com/maddsua
//	No warranties are given, etc...
//	This file is a component of the AVR Firmware Uploader

#ifndef _MADDSUA_WINAPI_SERIAL
#define _MADDSUA_WINAPI_SERIAL

#include <stdio.h>
#include <windows.h>

#include <vector>
#include <string>

#include "staticConfig.hpp"


	bool getPorts(std::vector <std::string>* splsarray);

#endif
