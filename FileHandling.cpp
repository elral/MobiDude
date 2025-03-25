#include <windows.h>
#include <vector>
#include <string>
#include "include/FileHandling.h"

bool cutFilePath(const char* filepath, char* filename) {

	if (strchr(filepath, '\\') != NULL) {

		const char* slcp = strrchr(filepath, '\\');
		int slcpos = (int)(slcp - filepath);
		slcpos++;

		strcpy(filename, filepath + slcpos);
	}
	else {
		return true;
	}

	return false;
}


bool openfile(char* filepath, char* filename) {

	char file[MAX_PATH] = { 0 };
	OPENFILENAME ofn = { 0 };

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL;
	ofn.lpstrFile = file;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFilter = ("Intel HEX;bin files\0*.hex;*bin\0Intel HEX\0*.hex\0bin files\0*.bin\0Pico files\0*.uf2\0All files\0*.*\0");
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	if (GetOpenFileName(&ofn)) {

		strcpy(filepath, file);

		cutFilePath(filepath, filename);

		return true;
	}

	return false;
}


DWORD GetCurrentDrives() {
	return GetLogicalDrives();  // Gibt die Bitmaske der aktuellen Laufwerke zurück
}


bool CopyFileToDrive(const char* sourceFilePath, const char* sourceFileName, const char* targetDrive) {
	char targetPath[MAX_PATH_LENGTH];
	snprintf(targetPath, sizeof(targetPath), "%s%s", targetDrive, sourceFileName);
	if (CopyFile(sourceFilePath, targetPath, FALSE))
		return true;
	else
		return false;

}

void checkBootDrive(DWORD newDrives, DWORD oldDrives, const char* sourceFilePath, const char* sourceFileName)
{
	DWORD addedDrives = newDrives & ~oldDrives;  // Drives which were added
	if (addedDrives) {
		// A new drive was added
		for (char letter = 'A'; letter <= 'Z'; letter++) {
			if (addedDrives & 1) {
				char drive[4] = { letter, ':', '\\', '\0' };
				// Copy file to new drive
				if (CopyFileToDrive(sourceFilePath, sourceFileName, drive)) {
					char msg[200];
					snprintf(msg, sizeof(msg), "FW successfully copied to %s", drive);
					MessageBox(NULL, msg, "Success", MB_OK | MB_ICONINFORMATION);
					return;
				}
				else {
					 char msg[200];
					 snprintf(msg, sizeof(msg), "Failure while copying FW to %s", sourceFilePath);
					 MessageBox(NULL, msg, "Failure", MB_OK | MB_ICONERROR);
				}
			}
			addedDrives >>= 1;
		}
	}
}