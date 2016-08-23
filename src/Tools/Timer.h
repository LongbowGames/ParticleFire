//High performance timer object.
//By Seumas McNally.


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

//
//Nov 98, now handles systems without perftimers properly, e.g. Cyrix.
//Downgrades to timeGetTime() and precision maxes at milliseconds.
//

#ifndef TIMER_H
#define TIMER_H

#define VC_EXTRALEAN
#define WIN32_EXTRA_LEAN
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

class Timer{
private:
	LARGE_INTEGER	tstart, tticks, tnow;
	int Started;
	int HighFreq;
public:
	Timer();
	~Timer();
	void Start();
	int Check(int FracSec);	//Enter the fractions of a second you'd like the result in, e.g. 1000 for ms.
};

#endif
