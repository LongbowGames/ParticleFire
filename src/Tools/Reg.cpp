// Windows Registry wrapper class, by Seumas McNally, 1998.


// This file is part of Particle Fire.
// 
// Particle Fire is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// Particle Fire is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with Particle Fire.  If not, see <http://www.gnu.org/licenses/>.

#include "Reg.h"

#include <CStr.h>

#include <cstdlib>

Registry::Registry(const char *company, const char *title, int global){
	strcpy(sKey, "Software");
	if(company){
		strcat(sKey, "\\");
		strcat(sKey, company);
	}
	if(title){
		strcat(sKey, "\\");
		strcat(sKey, title);
	}
	hKey = NULL;
	sPrefix[0] = '\0';
	//
	LocalMachine = global;
}
Registry::~Registry(){
}
int Registry::OpenKey(){
	DWORD t;
	CloseKey();
	if(ERROR_SUCCESS == RegCreateKeyEx((LocalMachine ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER), sKey, NULL, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &t)){
		return TRUE;
	}
	hKey = NULL;
	return FALSE;
}
int Registry::CloseKey(){
	if(hKey && ERROR_SUCCESS == RegCloseKey(hKey)){
		hKey = NULL;
		return TRUE;
	}
	return FALSE;
}
int Registry::SetPrefix(const char *prefix){
	if(prefix && strlen(prefix) < 1000){
		strcpy(sPrefix, prefix);
		return 1;
	}else{
		sPrefix[0] = '\0';
		return 0;
	}
}
int Registry::WriteDword(const char *name, unsigned long val){
	if(name && OpenKey()){
		if(ERROR_SUCCESS == RegSetValueEx(hKey, CStr(sPrefix) + name, NULL, REG_DWORD, (BYTE*)&val, sizeof(val))){
			return CloseKey();
		}
	}
	CloseKey();
	return FALSE;
}
int Registry::ReadDword(const char *name, unsigned long *val){
	DWORD tsize = sizeof(*val), ttype = REG_DWORD, temp;
	if(name && val && OpenKey()){
		if(ERROR_SUCCESS == RegQueryValueEx(hKey, CStr(sPrefix) + name, NULL, &ttype, (BYTE*)&temp, &tsize)){
			*val = temp;
			return CloseKey();
		}
	}
	CloseKey();
	return FALSE;
}
int Registry::WriteFloat(const char *name, float val){
	return WriteDword(name, *((unsigned long*)&val));
}
int Registry::ReadFloat(const char *name, float *val){
	unsigned long t;
	if(ReadDword(name, &t)){
		*val = *((float*)&t);
		return 1;
	}
	return 0;
}

int Registry::WriteString(const char *name, const char *val){
	if(name && val && OpenKey()){
		if(ERROR_SUCCESS == RegSetValueEx(hKey, CStr(sPrefix) + name, NULL, REG_SZ, (BYTE*)val, strlen(val) + 1)){
			return CloseKey();
		}
	}
	CloseKey();
	return FALSE;
}
int Registry::ReadString(const char *name, char *val, int maxlen){
	DWORD ttype = REG_SZ;
	if(name && val && maxlen > 0 && OpenKey()){
		if(ERROR_SUCCESS == RegQueryValueEx(hKey, CStr(sPrefix) + name, NULL, &ttype, (BYTE*)val, (DWORD*)&maxlen)){
			val[maxlen - 1] = '\0';	//Add safety null.
			return CloseKey();
		}
	}
	CloseKey();
	return FALSE;
}
int Registry::ReadString(const char *name, CStr *str){
	char b[1024];
	if(str && ReadString(name, b, 1024)){
		*str = b;
		return TRUE;
	}
	return FALSE;
}

int Registry::SaveWindowPos(HWND hwnd, const char *name, int Size, int Max){
	char buf[256];
	if(hwnd && name && OpenKey()){
		RECT rc;
		GetWindowRect(hwnd, &rc);
		strcpy(buf, name); strcat(buf, "X"); WriteDword(buf, rc.left);
		strcpy(buf, name); strcat(buf, "Y"); WriteDword(buf, rc.top);
		if(Size){
			strcpy(buf, name); strcat(buf, "W"); WriteDword(buf, abs(rc.right - rc.left));
			strcpy(buf, name); strcat(buf, "H"); WriteDword(buf, abs(rc.bottom - rc.top));
		}
		if(Max){	//Save Maximised state.
			strcpy(buf, name); strcat(buf, "S"); WriteDword(buf, IsZoomed(hwnd) ? 1 : 0);
		}
		CloseKey();
		return TRUE;
	}
	return FALSE;
}
int Registry::RestoreWindowPos(HWND hwnd, const char *name, int Size, int Max){
	char buf[256];
	if(hwnd && name && OpenKey()){
		RECT rc, wrc;
		int PosOK = FALSE, SizeOK = FALSE, StateOK = FALSE, state;
		strcpy(buf, name); strcat(buf, "X");
		if(ReadDword(buf, (DWORD*)&rc.left)){
			strcpy(buf, name); strcat(buf, "Y");
			if(ReadDword(buf, (DWORD*)&rc.top)){
				PosOK = TRUE;
			}
		}
		if(Size){
			strcpy(buf, name); strcat(buf, "W");
			if(ReadDword(buf, (DWORD*)&rc.right)){
				strcpy(buf, name); strcat(buf, "H");
				if(ReadDword(buf, (DWORD*)&rc.bottom)){
					SizeOK = TRUE;
				}
			}
		}
		if(Max){
			strcpy(buf, name); strcat(buf, "S");
			if(ReadDword(buf, (DWORD*)&state)) StateOK = TRUE;
		}
		if(PosOK){
			if(SizeOK == FALSE){	//Don't want size read anyway, or size not present.
				GetWindowRect(hwnd, &wrc);
				rc.right = wrc.right - wrc.left;	//Compute current width and height of window and throw those into rc for MoveWindow call.
				rc.bottom = wrc.bottom - wrc.top;
			}
			MoveWindow(hwnd, __min(__max(-8, rc.left), GetSystemMetrics(SM_CXSCREEN) - rc.right / 2),
				__min(__max(-8, rc.top), GetSystemMetrics(SM_CYSCREEN) - rc.bottom / 2),
				rc.right, rc.bottom, TRUE);
		}
		if(StateOK){
			if(state == 0) ShowWindow(hwnd, SW_SHOWNORMAL);
			if(state == 1) ShowWindow(hwnd, SW_MAXIMIZE);
		}else{	//If we want state restored, but there is none, do show normal.
			if(Max) ShowWindow(hwnd, SW_SHOWNORMAL);
		}
		CloseKey();
		if(PosOK || SizeOK || StateOK){	//Only return true if data found in reg.
			return TRUE;
		}
	}
	return FALSE;
}
