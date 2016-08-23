// Particle Fire Registry class - header

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

#ifndef PARTICLE_REGISTRY_HPP
#define PARTICLE_REGISTRY_HPP

class ParticleContainer;

class ParticleRegistry
{
public:
	ParticleRegistry ();
	~ParticleRegistry ();

	void Init ();

	void LoadOpts();
	void SaveOpts();

	void RegistryWrite (char *buff, int num);

public:
	ParticleContainer *parent;

};

#endif
