// Particle Fire Container class - source 

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

#include <time.h>
#include <math.h>
#include <windows.h>
#include <stdio.h>

#include <CStr.h>
#include "ParticleFire.h"
#include <Timer.h>
#include <Basis.h>
#include <Reg.h>

#include "defines.h"

#include "ParticleContainer.hpp"

// External functions
extern void error_print (char *buff);

ParticleContainer::ParticleContainer ()
{
	Init ();
}

ParticleContainer::~ParticleContainer ()
{

}
	
void ParticleContainer::Init ()
{
	strcpy (QuoteFilename, "\0");

	XMouse = 0;
	YMouse = 0;

//	TimeStart; Time; LastTime;		// Not init'd

//	p;				// Not init'd
//	pe; cf; ct;		// Not init'd

	// Clear before setting
	tdata = NULL;
	tdata2 = NULL;

	// Save parent pointers for sub-classes
	this->screen.parent = this;
	this->particle.parent = this;
	this->registry.parent = this;
}

void ParticleContainer::Frame ()
{
	this->HandleParticleStyle ();
	this->HandleScreen ();
}

void ParticleContainer::HandleParticleStyle ()
{
	particle.Frame ();
}

void ParticleContainer::HandleScreen ()
{
	screen.HandleText ();
	screen.Draw ();
}
