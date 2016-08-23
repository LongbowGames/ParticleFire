// Particle Fire Registry class - source

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

#include <windows.h>
#include <time.h>
#include <stdio.h>

#include "Reg.h"
#include "ParticleFire.h"
#include "ParticleRegistry.hpp"

// For parent-> vars
#include "ParticleContainer.hpp"
#include "ParticleScreen.hpp"
#include "ParticleParticle.hpp"

#include "defines.h"

// External functions
extern void error_print (char *buff);

//#define MAX_PART 10000

// ? GH-CHANGE
Registry REG("Longbow Digital Arts", "Particle Fire 2");


ParticleRegistry::ParticleRegistry ()
{
	Init ();
}

ParticleRegistry::~ParticleRegistry ()
{

}

void ParticleRegistry::Init ()
{

}

void ParticleRegistry::LoadOpts()
{
	char buff[255];
	sprintf (buff, "%x\n%x", *((ULONG*)&parent->screen.CustomPE1), *((ULONG*)&parent->screen.CustomPE2) );
//	error_print (buff);

	REG.ReadDword("RandomTime", (ULONG*)&parent->particle.RANDEFFECT);
	REG.ReadDword("GravityTime", (ULONG*)&parent->particle.GRAV_TIME);
	REG.ReadDword("RandomColor", (ULONG*)&parent->screen.RandomColor);
	REG.ReadDword("ColorScheme", (ULONG*)&parent->screen.ColorScheme);
	REG.ReadDword("Particles", (ULONG*)&parent->particle.nParticles);
	REG.ReadDword("CustomScheme", (ULONG*)&parent->screen.CustomScheme);
	REG.ReadDword("CustomColor1", (ULONG*)&parent->screen.CustomPE1);
	REG.ReadDword("CustomColor2", (ULONG*)&parent->screen.CustomPE2);
	REG.ReadDword("ParticleStyle", (ULONG*)&parent->particle.ParticleStyle);
	REG.ReadDword("WallStyle", (ULONG*)&parent->particle.WallStyle);
	REG.ReadDword("BurnFade", (ULONG*)&parent->screen.BURNFADE);
	REG.ReadDword("DisableText", (ULONG*)&parent->screen.DisableText);
	REG.ReadDword("DisableFire", (ULONG*)&parent->screen.DisableFire);	// Using WallStyle now to do this
	REG.ReadDword("CycleColors", (ULONG*)&parent->screen.CycleColors);
	REG.ReadDword("UseTrueColor", (ULONG*)&parent->screen.UseTrueColor);
	REG.ReadDword("QuoteTextSpeed", (ULONG*)&parent->screen.QuoteSecs);

	sprintf (buff, "%x\n%x", *((ULONG*)&parent->screen.CustomPE1), *((ULONG*)&parent->screen.CustomPE2) );
//	error_print (buff);

	// Get the Quote String
	CStr string;	REG.ReadString("QuoteFileName", &string);
	strcpy (parent->QuoteFilename, string.get() );
	
	parent->particle.RANDEFFECT = __max(parent->particle.RANDEFFECT, 1);
	parent->particle.GRAV_TIME = __max(parent->particle.GRAV_TIME, 1);
	parent->screen.ColorScheme = __min(parent->screen.ColorScheme, NUMSCHEMES);
	parent->particle.nParticles = __min(parent->particle.nParticles, MAX_PART);

	// Bound the Wall Styles
	parent->particle.WallStyle = __min (parent->particle.WallStyle, NUMSTYLEWALLS);
	parent->particle.WallStyle = __max (parent->particle.WallStyle, 0);
	//
	// First Time Use
	parent->screen.FirstUseTime = 0;
	if(REG.ReadDword("Time", (ULONG*)&parent->screen.FirstUseTime) == FALSE && parent->screen.FirstUseTime <= 0){	//Write first-use-time.
		parent->screen.FirstUseTime = time(NULL);
		REG.WriteDword("Time", parent->screen.FirstUseTime);
	}
	parent->screen.TotalSecs = 0;
	REG.ReadDword("SecondsBlanked", (ULONG*)&parent->screen.TotalSecs);	//Total seconds of use.
}

void ParticleRegistry::SaveOpts()
{
/*	{char buff[255];
//	sprintf (buff, "%x\n%x", *((ULONG*)&parent->screen.CustomPE1), *((ULONG*)&parent->screen.CustomPE2) );
//	error_print (buff);
	}*/

	REG.WriteDword("RandomTime", parent->particle.RANDEFFECT);
	REG.WriteDword("GravityTime", parent->particle.GRAV_TIME);
	REG.WriteDword("RandomColor", parent->screen.RandomColor);
	REG.WriteDword("ColorScheme", parent->screen.ColorScheme);
	REG.WriteDword("Particles", parent->particle.nParticles);
	REG.WriteDword("CustomScheme", parent->screen.CustomScheme);
	REG.WriteDword("CustomColor1", *((ULONG*)&parent->screen.CustomPE1));
	REG.WriteDword("CustomColor2", *((ULONG*)&parent->screen.CustomPE2));
	REG.WriteDword("ParticleStyle", parent->particle.ParticleStyle);
	REG.WriteDword("WallStyle", parent->particle.WallStyle);
	REG.WriteDword("BurnFade", parent->screen.BURNFADE);
	REG.WriteDword("DisableText", parent->screen.DisableText);
	REG.WriteDword("DisableFire", parent->screen.DisableFire);	// Using WallStyle now to disable Wall Fire
	REG.WriteDword("CycleColors", parent->screen.CycleColors);
	REG.WriteDword("UseTrueColor", parent->screen.UseTrueColor);
	REG.WriteDword("QuoteTextSpeed", parent->screen.QuoteSecs);

	REG.WriteString("QuoteFileName", parent->QuoteFilename);

//	char buff[255];
//	sprintf (buff, "%x\n%x", *((ULONG*)&parent->screen.CustomPE1), *((ULONG*)&parent->screen.CustomPE2) );
//	error_print (buff);
}

void ParticleRegistry::RegistryWrite (char *buff, int num)
{
	REG.WriteDword(buff, num);
}

