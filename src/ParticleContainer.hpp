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

#ifndef PARTICLE_CONTAINER_HPP
#define PARTICLE_CONTAINER_HPP

#include "ParticleScreen.hpp"
#include "ParticleRegistry.hpp"
#include "ParticleParticle.hpp"

class ParticleContainer 
{
public:
	ParticleContainer ();
	~ParticleContainer ();
	
	void Init ();

	void Frame ();

	void HandleParticleStyle ();		// Hanlde the particle effect styles
	void HandleScreen ();				// Handle the screen related things

public:
	ParticleScreen screen;
	ParticleRegistry registry;
	ParticleParticle particle;

	// Local vars
	char QuoteFilename[255];			// Quotes filename

	// System
	int XMouse;
	int YMouse;

	// Useage time/registration
	time_t			TimeStart, Time, LastTime;

	// Particles
	Particle		p[MAX_PART];
	PALETTEENTRY	pe[256], cf[256], ct[256];

	// Global shared between Particle and Drawing
	unsigned char *tdata, *tdata2;
};

#endif
