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

//#include <string.h>
//#include <stdlib.h>
//#include <stdio.h>

//using namespace std;

#define TRUE 1
#define FALSE 0

inline int Range(int a, int b, int c) {return a < b ? b : ( a > c ? c : a );}
inline int Max(int a, int b) {return a < b ? b : a;}
inline int Min(int a, int b) {return a > b ? b : a;}

class CStr{
	char	*pData;
	int		nLength;
	const static char Dummy;
	static char Playground[];
	void InitialInit();
	//For pointing to if mallocs fail.
	//Leave pData NULL when they fail, but return pointer to Dummy when
	//someone asks for a const char *.
public:
	CStr();
	CStr(const char *s);
	CStr(const CStr &str);
	CStr(const char *s, const char *ins, int pos);
	~CStr();

	const char	*get() const {return pData ? pData : &Dummy;}
	int		len() const {return pData ? nLength : 0;}
	int		alloc(int size);	//Allocates size _uninitialized_ bytes not counting ending null, ready to be filled.  Err, how, there's no non-const way to get at pData...  Sigh.
	void	cpy(const char *s);
	void	cat(const char *s);
	int		cmp(const char *s) const;

	char	chr(unsigned int pos) const {
		if(pos < (unsigned int)nLength) return pData[pos];
		return 0;
	};
	char &operator[](int n){
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
	CStr& operator=(const char *s) {
		if(s != pData) cpy(s);
		return *this;
	}

	operator const char*() const {return get();}

//	char operator[](int idx) const;
	//Implement later.
};

CStr operator+(const CStr &str1, const CStr &str2);
CStr operator+(const CStr &str, const char *s);
CStr operator+(const char *s, const CStr &str);

inline int operator==(const CStr &str1, const CStr &str2){
	return str1.cmp(str2.get());
};
inline int operator==(const CStr &str1, const char *s){
	return str1.cmp(s);
};
inline int operator==(const char *s, const CStr &str2){
	return str2.cmp(s);
};
inline int operator!=(const CStr &str1, const CStr &str2){
	return !str1.cmp(str2.get());
};
inline int operator!=(const CStr &str1, const char *s){
	return !str1.cmp(s);
};
inline int operator!=(const char *s, const CStr &str2){
	return !str2.cmp(s);
};

//Seperate CStr manipulation functions.

CStr Mid(const CStr &str, int pos, int num = -1);
inline CStr Left(const CStr &str, int num) {return Mid(str, 0, num);}
inline CStr Right(const CStr &str, int num) {return Mid(str, Max(str.len() - num, 0));}
int Instr(const CStr &str, const CStr &fnd, int pos = 0);
inline CStr Insert(const CStr &str, const CStr &ins, int pos) {CStr nstr(str, ins, pos); return nstr;}
CStr Lower(const char *str);
CStr Upper(const char *str);
CStr String(const double a);
CStr String(const int a);
CStr String(const char c);

CStr FileExtension(const char *n);	//Returns only aspects after period ".".
CStr FileNoExtension(const char *n);	//Returns file path/name with .foo extension removed.
CStr FilePathOnly(const char *n);	//Returns path (with trailing path char) without name.
CStr FileNameOnly(const char *n);	//Returns name without any path.
int FileInPath(const char *n, const char *path);	//Returns true if string n starts with string path.  Only works with fully qualified drive:\dir pathnames.
CStr FileMinusPath(const char *n, const char *path);	//Removes path string from name, or if no match returns FileNameOnly().
CStr FileNameSafe(const char *n);	//Kills path chars and anything not a letter, number, space, or underscore.

int CmpLower(const char *s1, const char *s2);

CStr PadString(const char *str, int padlen, char padchar = ' ', char clip = TRUE);

#endif
