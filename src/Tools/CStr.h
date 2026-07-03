/*

CStr.h - Header for Generic String Class.
Copyright 1998 by Seumas McNally.

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

//This was my first real class, with most of the code taken from
//C++ in Plain English, so it's a little ratty in places as I've
//tried to keep updating it with my growing knowledge.  I've got
//a number of convenience functions that mimic Amos' string
//functionality, and a few new really handy file path/extension
//parsing functions.

#ifndef CSTR_H
#define CSTR_H

#include <algorithm>

#define TRUE 1
#define FALSE 0

inline int Range(int a, int b, int c) {return a < b ? b : ( a > c ? c : a );}
inline int Max(int a, int b) {return a < b ? b : a;}
inline int Min(int a, int b) {return a > b ? b : a;}

class CStr{
	wchar_t	*pData;
	size_t	nLength;
	const static wchar_t Dummy;
	static wchar_t Playground[];
	void InitialInit();
	//For pointing to if mallocs fail.
	//Leave pData NULL when they fail, but return pointer to Dummy when
	//someone asks for a const char *.
public:
	CStr();
	CStr(const wchar_t *s);
	CStr(const CStr &str);
	CStr(const wchar_t *s, const wchar_t *ins, int pos);
	~CStr();

	const wchar_t	*get() const {return pData ? pData : &Dummy;}
	size_t	len() const {return pData ? nLength : 0;}
	size_t	alloc(int size);	//Allocates size _uninitialized_ bytes not counting ending null, ready to be filled.  Err, how, there's no non-const way to get at pData...  Sigh.
	void	cpy(const wchar_t  *s);
	void	cat(const wchar_t *s);
	int		cmp(const wchar_t *s) const;

	wchar_t	chr(unsigned int pos) const {
		if(pos < (unsigned int)nLength) return pData[pos];
		return 0;
	};
	wchar_t &operator[](size_t n){
		if((unsigned int)n < (unsigned int)nLength) return pData[n];
		Playground[0] = '\0';
		return Playground[0];	//This will give any writes a safe place to play.
	};
/*
	friend int operator==(const CStr &str1, const CStr &str2)
	{
		return str1.cmp(str2.get());
	}
	friend int operator==(const CStr &str1, const char *s)
	{
		return str1.cmp(s);
	}
	friend int operator==(const char *s, const CStr &str2)
	{
		return str2.cmp(s);
	}
	friend int operator!=(const CStr &str1, const CStr &str2)
	{
		return !str1.cmp(str2.get());
	}
	friend int operator!=(const CStr &str1, const char *s)
	{
		return !str1.cmp(s);
	}
	friend int operator!=(const char *s, const CStr &str2)
	{
		return !str2.cmp(s);
	}
*/
//	friend CStr operator+(const CStr &str1, const CStr &str2);
//	friend CStr operator+(const CStr &str, const char *s);
//	friend CStr operator+(const char *s, const CStr &str);

	CStr& operator=(const CStr &source) {
		if(&source != this) cpy(source.get());	//Handle identity copy.
		return *this;
	}
	CStr& operator=(const wchar_t *s) {
		if(s != pData) cpy(s);
		return *this;
	}

	operator const wchar_t*() const {return get();}

//	char operator[](int idx) const;
	//Implement later.
};

CStr operator+(const CStr &str1, const CStr &str2);
CStr operator+(const CStr &str, const wchar_t *s);
CStr operator+(const wchar_t *s, const CStr &str);

inline bool operator==(const CStr &str1, const CStr &str2){
	return str1.cmp(str2.get());
};
inline bool operator==(const CStr &str1, const wchar_t *s){
	return str1.cmp(s);
};
inline bool operator==(const wchar_t *s, const CStr &str2){
	return str2.cmp(s);
};
inline bool operator!=(const CStr &str1, const CStr &str2){
	return !str1.cmp(str2.get());
};
inline bool operator!=(const CStr &str1, const wchar_t *s){
	return !str1.cmp(s);
};
inline bool operator!=(const wchar_t *s, const CStr &str2){
	return !str2.cmp(s);
};

//Seperate CStr manipulation functions.

CStr Mid(const CStr &str, size_t pos, size_t num = -1);
inline CStr Left(const CStr &str, size_t num) {return Mid(str, 0, num);}
inline CStr Right(const CStr &str, size_t num) {return Mid(str, std::max(str.len() - num, size_t(0)));}
int Instr(const CStr &str, const CStr &fnd, int pos = 0);
inline CStr Insert(const CStr &str, const CStr &ins, int pos) {CStr nstr(str, ins, pos); return nstr;}
CStr Lower(const wchar_t *str);
CStr Upper(const wchar_t *str);
CStr String(const double a);
CStr String(const int a);
CStr String(const char c);

CStr FileExtension(const wchar_t *n);	//Returns only aspects after period ".".
CStr FileNoExtension(const wchar_t *n);	//Returns file path/name with .foo extension removed.
CStr FilePathOnly(const wchar_t *n);	//Returns path (with trailing path char) without name.
CStr FileNameOnly(const wchar_t *n);	//Returns name without any path.
int FileInPath(const wchar_t *n, const wchar_t *path);	//Returns true if string n starts with string path.  Only works with fully qualified drive:\dir pathnames.
CStr FileMinusPath(const wchar_t *n, const wchar_t *path);	//Removes path string from name, or if no match returns FileNameOnly().
CStr FileNameSafe(const wchar_t *n);	//Kills path chars and anything not a letter, number, space, or underscore.

int CmpLower(const wchar_t *s1, const wchar_t *s2);

CStr PadString(const wchar_t *str, int padlen, wchar_t padchar = L' ', bool clip = TRUE);

#endif
