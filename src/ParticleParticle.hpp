// Particle Fire Particle class - header

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

#ifndef PARTICLE_PARTICLE_HPP
#define PARTICLE_PARTICLE_HPP

#include "ParticleScreen.hpp"
#include "ParticleRegistry.hpp"

// Defines
#define MAX_PART 10000

class ParticleContainer;

class ParticleParticle 
{
public:
	ParticleParticle ();
	~ParticleParticle ();

	void Init ();

	void Frame ();

	void Frame_Starfield ();			// Handle a frame of starfield based particles (moving towards the camera)
	void Frame_Gravity();				// Handle a frame of gravity based particles (moving up or down)

	void Handle_Gravity ();				// Handle all the options of gravity

	void Do_Popcorn ();
	void Do_InnerRing ();
	void Do_ShakeUp ();
	void Do_Freeze ();
	void Do_Explode ();
	void Do_Comet ();
	void Do_Emit ();
	void Do_AttractFollow ();

	// New
	void Do_RainbowHole (int initNow = 0);					
	void Do_SquigglyWiggly (int init=0);
	void Do_GalacticStorm (int init=0);
	void Do_PixieDust (int init=0);
	void Do_Geoff (int init=0);			// Test pattern

	void ClearMode (int mode=0);
	void SetMode (int mode=0);

public:
	ParticleContainer *parent;

	// Local vars
	float IdentityAngle;

	// Particle style info
	int ParticleStyle, WallStyle;
	int ZMoveSpeed;

	// Particle style info
	int nParticles;
	int GRAV_TIME;
	int RANDEFFECT;
	int AltColor;

	// Particle
	BOOL			NoiseBurn, Follow, MultipleFollow, UseGravity;
	BOOL			fullscreen, Stretch, Explode, Cube3D, Attract;
	BOOL			SwitchMode, ShakeUp, Freeze, UseRandom;
	BOOL			Comet, Emit, FollowMouse, IsMinimized;
	int				EmitCount, ExplodeX, ExplodeY;//, MouseX, MouseY;
	int InnerRing, Popcorn;
	int EmitRotate;
	float			xgrav, ygrav;
	int BurnDown;

	// GH-New vars
	int MagnetX, MagnetY;

	int RainbowHole, SquigglyWiggly, GalacticStorm, PixieDust;

};

#endif

