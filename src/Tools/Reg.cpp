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

#include <cstdlib>
#include <string>

Registry::Registry(std::wstring_view company, std::wstring_view title, bool global){
	sKey = L"Software";
	if(!company.empty()){
		sKey += L'\\';
		sKey += company;
	}
	if(!title.empty()){
		sKey += L'\\';
		sKey += title;
	}
}

bool Registry::OpenKey(){
	DWORD t;
	CloseKey();
	if(ERROR_SUCCESS == RegCreateKeyEx((LocalMachine ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER), sKey.c_str(), NULL, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &t)) {
		return true;
	}
	hKey = nullptr;
	return false;
}

bool Registry::CloseKey(){
	if(hKey && ERROR_SUCCESS == RegCloseKey(hKey)){
		hKey = nullptr;
		return true;
	}
	return false;
}

bool Registry::SetPrefix(std::wstring prefix){
	sPrefix = std::move(prefix);
	return !sPrefix.empty();
}

bool Registry::WriteDword(const std::wstring& name, unsigned long val){
	if (OpenKey()){
		if (ERROR_SUCCESS == RegSetValueEx(hKey, (sPrefix + name).c_str(), NULL, REG_DWORD, (BYTE*)&val, sizeof(val))) {
			return CloseKey();
		}
	}
	CloseKey();
	return false;
}

bool Registry::ReadDword(const std::wstring& name, unsigned long& val){
	if (OpenKey()) {
		DWORD len = sizeof(unsigned long);
		if (ERROR_SUCCESS == RegGetValue(hKey, nullptr, (sPrefix + name).c_str(), RRF_RT_REG_DWORD, nullptr, &val, &len)) {
			return CloseKey();
		}
	}
	CloseKey();
	return false;
}

bool Registry::WriteFloat(const std::wstring& name, float val){
	if (OpenKey()) {
		if (ERROR_SUCCESS == RegSetValueEx(hKey, (sPrefix + name).c_str(), NULL, REG_QWORD, (BYTE*)&val, sizeof(val))) {
			return CloseKey();
		}
	}
	CloseKey();
	return false;
}

bool Registry::ReadFloat(const std::wstring& name, float& val){
	if (OpenKey()) {
		DWORD len = sizeof(float);
		if (ERROR_SUCCESS == RegGetValue(hKey, nullptr, (sPrefix + name).c_str(), RRF_RT_REG_QWORD, nullptr, &val, &len)) {
			return CloseKey();
		}
	}
	CloseKey();
	return false;
}

bool Registry::WriteString(const std::wstring& name, const std::wstring& val){
	if (OpenKey()) {
		if (ERROR_SUCCESS == RegSetValueEx(hKey, (sPrefix + name).c_str(), NULL, REG_SZ, (BYTE*)val.c_str(), DWORD(sizeof(wchar_t)*(val.length()+1)))) {
			return CloseKey();
		}
	}
	CloseKey();
	return false;
}

bool Registry::ReadString(const std::wstring& name, std::wstring& str){
	if(OpenKey()) {
		DWORD len = 0;
		if (ERROR_SUCCESS == RegGetValue(hKey, nullptr, (sPrefix + name).c_str(), RRF_RT_REG_SZ, NULL, NULL, &len)) {
			str.resize(len/sizeof(wchar_t));
			if (ERROR_SUCCESS == RegGetValue(hKey, nullptr, (sPrefix + name).c_str(), RRF_RT_REG_SZ, NULL, str.data(), &len)) {
				return CloseKey();
			}
		}
	}
	CloseKey();
	return false;
}

bool Registry::SaveWindowPos(HWND hwnd, const std::wstring& name, bool Size, bool Max){
	std::wstring buf;
	if(OpenKey()){
		RECT rc;
		GetWindowRect(hwnd, &rc);
		WriteDword(name + L'X', rc.left);
		WriteDword(name + L'Y', rc.top);
		if(Size){
			WriteDword(name + L'W', rc.right - rc.left);
			WriteDword(name + L'H', rc.bottom - rc.top);
		}
		if(Max){	//Save Maximised state.
			WriteDword(name + L'S', IsZoomed(hwnd)? 1 : 0);
		}
		return CloseKey();
	}
	return false;
}

bool Registry::RestoreWindowPos(HWND hwnd, const std::wstring& name, bool Size, bool Max){
	if(hwnd && OpenKey()){
		RECT rc, wrc;
		bool PosOK = false, SizeOK = false, StateOK = false, state;
		if(ReadDword(name + L'X', rc.left)) {
			if (ReadDword(name + L'Y', rc.top)) {
				PosOK = true;
			}
		}
		if(Size){
			if (ReadDword(name + L'W', rc.right)) {
				if (ReadDword(name + L'H', rc.bottom)) {
					SizeOK = true;
				}
			}
		}
		if(Max){
			if(ReadDword(name+L'S', (DWORD&)state))
				StateOK = true;
		}
		if(PosOK){
			if(SizeOK == false){	//Don't want size read anyway, or size not present.
				GetWindowRect(hwnd, &wrc);
				rc.right = wrc.right - wrc.left;	//Compute current width and height of window and throw those into rc for MoveWindow call.
				rc.bottom = wrc.bottom - wrc.top;
			}
			MoveWindow(hwnd, std::min(std::max(LONG(-8), rc.left), GetSystemMetrics(SM_CXSCREEN) - rc.right / 2),
				std::min(std::max(LONG(-8), rc.top), GetSystemMetrics(SM_CYSCREEN) - rc.bottom / 2),
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
			return true;
		}
	}
	return false;
}
