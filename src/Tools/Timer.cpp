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

#include <Timer.h>

#include <mmsystem.h>

Timer::Timer(){
	HighFreq = QueryPerformanceFrequency(&tticks);
	Started = FALSE;
	Start();
}
Timer::~Timer(){
}
void Timer::Start(){
	if(HighFreq){
		QueryPerformanceCounter(&tstart);
	}else{
		tstart.LowPart = timeGetTime();
	}
	Started = TRUE;
}
int Timer::Check(int FracSec){
	if(!Started) return 0;
	if(HighFreq){
		QueryPerformanceCounter(&tnow);
		return (int)(((tnow.QuadPart - tstart.QuadPart) * FracSec) / tticks.QuadPart);
	}else{
		tnow.LowPart = timeGetTime();
		if(tnow.LowPart < tstart.LowPart){	//Oops, millisecond rollover.
			return (int)(((tnow.LowPart + (0xffffffff - tstart.LowPart)) * FracSec) / 1000);
		}else{
			return (int)(((tnow.LowPart - tstart.LowPart) * FracSec) / 1000);
		}
	}
}
