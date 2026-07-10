//Windows Registry wrapper class, by Seumas McNally, 1998.


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

//Usage:  Construct one object with Company Name and Product Name as parameters.
//Call OpenKey, write or read data, and CloseKey.  Or use Save and RestoreWindowPos.
//Note, they will open and close the key internally!
//Save/RestoreWindowPos will append the letters X, Y, W, and H onto the name given
//when writing the four size/position values.
//
//Actually, I modified it so the data reading/writing functions open and close the
//key internally themselves.  Call me paranoid, and a little inefficient.  But it
//seems to be fast enough, and Windows should cache registry accesses pretty damn
//heavily.

#ifndef REG_H
#define REG_H

#include <string>

#define VC_EXTRALEAN
#define WIN32_EXTRA_LEAN
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

class Registry{
private:
	std::wstring sKey;
	std::wstring sPrefix;
	HKEY hKey = nullptr;
	bool LocalMachine = false;
public:
	Registry(std::wstring_view company, std::wstring_view title, bool global = false);	//Set global to true to use HKEY_LOCAL_MACHINE instead of HKEY_CURRENT_USER.
	bool OpenKey();
	bool CloseKey();
	bool SetPrefix(std::wstring prefix);	//Sets a prefix which is prepended to all keys read or written, so you can easily create fake-subdirectories of keys.
	bool WriteDword(const std::wstring& name, unsigned long val);
	bool ReadDword(const std::wstring& name, unsigned long& val);
	bool ReadDword(const std::wstring& name, long& val) { unsigned long ulVal; auto rc = ReadDword(name, ulVal); val = ulVal; return rc; };
	bool ReadDword(const std::wstring& name, int& val) { unsigned long ulVal; auto rc = ReadDword(name, ulVal); val = ulVal; return rc; };
	bool ReadDword(const std::wstring& name, bool& val) { unsigned long ulVal; auto rc = ReadDword(name, ulVal); val = ulVal; return rc; };
	bool WriteFloat(const std::wstring& name, float val);
	bool ReadFloat(const std::wstring& name, float& val);
	bool WriteString(const std::wstring& name, const std::wstring& val);
	bool ReadString(const std::wstring& name, std::wstring& val);
	bool SaveWindowPos(HWND hwnd, const std::wstring& name, bool Size = false, bool Max = false);	//Restore and save Widht and Height too?  (Not for Dialogs!!!)
	bool RestoreWindowPos(HWND hwnd, const std::wstring& ame, bool Size = false, bool Max = false);	//Set Max to true to save/restore window's maximised state.
};

#endif
