#include <windows.h>


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
	ofn.lpstrFilter = ("Intel HEX;bin files\0*.hex;*bin\0Intel HEX\0*.hex\0bin files\0*.bin\0All files\0*.*\0");
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
