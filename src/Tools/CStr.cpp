/*

CStr.cpp - Generic String Class by Seumas McNally, based on code in
"C++ In Plain English" by Brian Overland.

This file is part of Particle Fire.

Particle Fire is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Particle Fire is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Particle Fire.  If not, see <http://www.gnu.org/licenses/>.

*/

//June 98:  Yick, this needs some serious fixing up.  This is showing its age.

//#include <string.h>
//#include <stdlib.h>
//#include <stdio.h>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cctype>

#include "CStr.h"

const wchar_t CStr::Dummy = L'\0';

wchar_t CStr::Playground[] = L"\0\0";

#define GRANULARITY 32
//Minimum memory alloc.

#define GRAN(a) (((a) + (GRANULARITY - 1)) & (~(GRANULARITY - 1)))

void CStr::InitialInit(){
	pData = (wchar_t*) malloc(GRANULARITY*sizeof(wchar_t));
	nLength = 0;
	if(pData) pData[0] = '\0';
	return;
}
CStr::CStr(){
//	pData = (char*) malloc(1);
//	if(!pData){
//		nLength = 0;
//		return;
//	}
//	*pData = '\0';
//	nLength = 0;
	InitialInit();
}
CStr::CStr(const wchar_t *s){
	if(!s){
		InitialInit();
		return;
	}
	nLength = wcslen(s);
	pData = (wchar_t*) malloc(GRAN(nLength + 1) * sizeof(wchar_t));
	if(!pData){
		nLength = 0;
		return;
	}
	wcscpy(pData, s);
//	nLength = strlen(s);
}
CStr::CStr(const CStr &s){
	nLength = s.len();
	pData = (wchar_t*) malloc(GRAN(nLength + 1) * sizeof(wchar_t));
	if(!pData){
		nLength = 0;
		return;
	}
	wcscpy(pData, s.get());
}
CStr::CStr(const wchar_t *s, const wchar_t *ins, int pos){
	if(!s || !ins){
		InitialInit();
		return;
	}
	nLength = wcslen(s) + wcslen(ins);
	pData = (wchar_t*) malloc(GRAN(nLength + 1) * sizeof(wchar_t));
	if(!pData){
		nLength = 0;
		return;
	}
	pos = Range(pos, 0, int(wcslen(s)));
	if(pos > 0) memcpy(pData, s, pos);
	wcscpy(pData + pos, ins);
	wcscpy(pData + pos + wcslen(ins), s + pos);
}

CStr::~CStr(){
	if(pData) free(pData);
	pData = NULL;
}

size_t CStr::alloc(int size){
	if(size > 0){
		if(GRAN(nLength + 1) != GRAN(size + 1) || pData == NULL){
			if(pData) free(pData);
			pData = (wchar_t*)malloc(GRAN(size + 1) * sizeof(wchar_t));
		}
		if(pData){
			pData[size] = 0;	//Set null, to be safe.
			nLength = size;
			return nLength;
		}else{
			nLength = 0;
		}
	}
	return 0;
}

void CStr::cpy(const wchar_t *s){
	if(s && s != pData){
		size_t n = wcslen(s);
		if(GRAN(nLength + 1) != GRAN(n + 1)){
			if(pData)
				free(pData);
			pData = (wchar_t*) malloc(GRAN(n + 1) * sizeof(wchar_t));
			if(!pData){
				nLength = 0;
				return;
			}
			nLength = n;
		}
		wcscpy(pData, s);
		nLength = n;
	}
}

void CStr::cat(const wchar_t *s){
	if(s){
		size_t n = wcslen(s);
		if(n <= 0) return;
		if(GRAN(nLength + 1) != GRAN(nLength + n + 1)){
			wchar_t *pTemp;
			pTemp = (wchar_t*) malloc(GRAN(n + nLength + 1) * sizeof(wchar_t));
			if(!pTemp) return;
			if(pData) wcscpy(pTemp, pData);
			wcscat(pTemp, s);
			if(pData) free(pData);
			pData = pTemp;
		}else{
			wcscat(pData, s);
		}
		nLength += n;
	}
}

int CStr::cmp(const wchar_t *s) const{
	if(!pData || !s) return FALSE;	//NULL pData means there's been a malloc problem
//	if((int)strlen(s) != nLength) return FALSE;	//Strict checking here
//	if(0 == memcmp(pData, s, nLength)) return TRUE;
	size_t n = 0;
	while(n < nLength && s[n] == pData[n] && s[n]) n++;
	if(n == nLength && s[n] == 0) return TRUE;
	return FALSE;
}

CStr operator+(const CStr &str1, const CStr &str2){
	CStr new_string(str1);
	new_string.cat(str2.get());
	return new_string;
}

CStr operator+(const CStr &str, const wchar_t *s){
	CStr new_string(str);
	new_string.cat(s);
	return new_string;
}

CStr operator+(const wchar_t *s, const CStr &str){
	CStr new_string(s);
	new_string.cat(str.get());
	return new_string;
}

#define LOWER(a) ( ((a) >= 'A' && (a) <= 'Z') ? (a) + ('a' - 'A') : (a))
#define UPPER(a) ( ((a) >= 'a' && (a) <= 'z') ? (a) - ('a' - 'A') : (a))

CStr Mid(const CStr &str, size_t pos, size_t num){
	CStr nstr;
	if(num == 0) return nstr;
	wchar_t *tmp;
	size_t L = str.len();
	if(L == 0 || pos >= L || pos < 0)
		return nstr;
	if(num < 0) num = L;
	num = Range(int(num), 0, int(L - pos));
	tmp = (wchar_t*) malloc((num + 1) * sizeof(wchar_t));
	if(!tmp)
		return nstr;
	memcpy(tmp, str.get() + pos, num);
	tmp[num] = '\0';
	nstr.cpy(tmp);
	free(tmp);
	return nstr;
}

//Returns position of fnd in str starting at 0, searching from pos, or -1 if
//string isn't found.
int Instr(const CStr &str, const CStr &fnd, int pos){
	const wchar_t *pstr = str.get();
	const wchar_t *pfnd = fnd.get();
	const wchar_t *result;
	size_t lstr = str.len();
	size_t lfnd = fnd.len();
	pos = Range(pos, 0, int(lstr));
	if(!pstr || !pfnd || !lstr || !lfnd || pos + lfnd > lstr) return -1;
	result = wcsstr(pstr + pos, pfnd);
	if(result != NULL) return (int)(result - pstr);
	return -1;
}

CStr Lower(const wchar_t *str){
	if(str){
		CStr out = str;
		for(size_t i = 0; i < out.len(); i++){
			wchar_t c = out[i];
			out[i] = LOWER(c);
		}
		return out;
	}
	return CStr();
}
CStr Upper(const wchar_t *str){
	if(str){
		CStr out = str;
		for(size_t i = 0; i < out.len(); i++){
			wchar_t c = out[i];
			out[i] = UPPER(c);
		}
		return out;
	}
	return CStr();
}

//Returns a CStr with the input number in human readable ASCII string form.
CStr String(const double a){
	wchar_t Buf[100];
	swprintf(Buf, 100, L"%f", a);
	CStr str(Buf);
	return str;
}
//Returns a CStr with the input number in human readable ASCII string form.
CStr String(const int a){
	wchar_t Buf[100];
	swprintf(Buf, 100, L"%i", a);
	CStr str(Buf);
	return str;
}

//Returns a CStr containing the input single char and a terminating null.
CStr String(const char c){
	wchar_t Buf[2];
	Buf[0] = c;
	Buf[1] = '\0';
	CStr str(&Buf[0]);
	return str;
}

CStr FileExtension(const wchar_t *n){
	CStr str;
	if(n){
		for(size_t i = wcslen(n) - 1; i >= 0; i--){
			if(n[i] == '.'){ str.cpy(&n[i + 1]); break; }
			if(n[i] == '/' || n[i] == '\\' || n[i] == ':') break;
		}
	}
	return str;
}
CStr FileNoExtension(const wchar_t *n){
	CStr str(n);
	if(n){
		for(size_t i = wcslen(n) - 1; i >= 0; i--){
			if(n[i] == '.'){ str = Left(str, i); break; }
			if(n[i] == '/' || n[i] == '\\' || n[i] == ':') break;
		}
	}
	return str;
}
CStr FilePathOnly(const wchar_t *n){
	CStr str(n);
	if(n){
		for(size_t i = wcslen(n) - 1; i >= 0; i--){
			if(n[i] == '/' || n[i] == '\\' || n[i] == ':'){ str = Left(str, i + 1); break; }
			if(i == 0) str = L"";
		}
	}
	return str;
}
CStr FileNameOnly(const wchar_t *n){
	CStr str(n);
	if(n){
		for(size_t i = wcslen(n) - 1; i >= 0; i--){
			if(n[i] == '/' || n[i] == '\\' || n[i] == ':'){ str.cpy(&n[i + 1]); break; }
		}
	}
	return str;
}
int FileInPath(const wchar_t *n, const wchar_t *path){
	if(n && path && wcslen(path) < wcslen(n)){
		for(int i = 0; i < (int)wcslen(path); i++){
			if(tolower(n[i]) == tolower(path[i]) ||
				(n[i] == '/' && path[i] == '\\') ||
				(n[i] == '\\' && path[i] == '/')){	//Handle mixed up slashes, and mixed case.
				//We're OK.
				continue;
			}else{
				return FALSE;
			}
		}
		return TRUE;
	}
	return FALSE;
}
CStr FileMinusPath(const wchar_t *n, const wchar_t *path){
	CStr str;
	if(FileInPath(n, path)){
		str = Mid(n, int(wcslen(path)));
	}
	return str;
}

CStr FileNameSafe(const wchar_t *n){	//Kills path chars and anything not a letter, number, space, or underscore.
	CStr str;
	wchar_t buf[1024];
	if(n){
		wchar_t c;
		int i = 0;
		while(i < 1024 && (c = n[i]) != 0){
			if((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') ||
				c == ' ' || c == '_' || (c == '.' && i > 0)){	//Dots after initial char are ok.
				buf[i] = c;
			}else{
				buf[i] = 'x';	//Replacement for bad char.
			}
			i++;
		}
		buf[__min(1023, i)] = 0;
		str.cpy(buf);
	}
	return str;
}

int CmpLower(const wchar_t *s1, const wchar_t *s2){
	if(s1 && s2){
		int n = 0;
		while(s1[n] && s2[n] && LOWER(s1[n]) == LOWER(s2[n])) n++;
		if((s1[n] | s2[n]) == 0) return TRUE;
		return FALSE;
	//	if(_stricmp(s1, s2) == 0) return TRUE;
	}
	return FALSE;
}

CStr PadString(const wchar_t *str, int padlen, wchar_t padchar, bool clip){
	static wchar_t padbuf[1024];
	if(str && padlen > 0){
		size_t len = wcslen(str);
		len = __min(1023, len);
		padlen = __min(1023, padlen);
		memset(padbuf, padchar, padlen);
		memcpy(padbuf, str, len);
		if(clip) padbuf[padlen] = 0;
		else padbuf[len] = 0;
		return padbuf;
	}
	return L"";
}

