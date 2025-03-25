#pragma once

#define MAX_PATH_LENGTH 260

bool openfile(char* filepath, char* filename);
DWORD GetCurrentDrives();
bool CopyFileToDrive(const char* sourceFilePath, const char* sourceFileName, const char* targetDrive);
bool checkBootDriveAndCopy(DWORD newDrives, DWORD oldDrives, const char* sourceFilePath, const char* sourceFileName);
