// Particle Fire Particle class - source

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
#include <math.h>

#include <stdio.h>

#include "ParticleFire.h"

#include "ParticleParticle.hpp"

// For parent-> vars
#include "ParticleContainer.hpp"
#include "ParticleScreen.hpp"

#include "defines.h"

// External functions
extern float frand(float range);
extern void error_print (char *buff);


ParticleParticle::ParticleParticle ()
{
	Init ();
}

ParticleParticle::~ParticleParticle ()
{

}

void ParticleParticle::Init ()
{

	IdentityAngle = 3.0f;
	ParticleStyle = 0;
	WallStyle = 0;
	ZMoveSpeed = 2;
//Number of particles.
	nParticles = 2000;//500;
//Number of seconds between gravity direction shifts.
	GRAV_TIME = 50;//60;
//One in X chance of an effect happening every second.
	RANDEFFECT = 75;//8;
	AltColor = FALSE;

	NoiseBurn = FALSE; Follow = TRUE; MultipleFollow = TRUE; UseGravity = TRUE;
	fullscreen = FALSE; Stretch = FALSE; Explode = FALSE; Cube3D = TRUE; Attract = TRUE;
	SwitchMode = FALSE; ShakeUp = FALSE; Freeze = FALSE; UseRandom = TRUE;
	Comet = FALSE; Emit = FALSE; FollowMouse = FALSE; IsMinimized =  FALSE;
	EmitCount = 0; ExplodeX = 0; ExplodeY = 0;//, MouseX, MouseY;
	InnerRing = FALSE, Popcorn = FALSE;

	EmitRotate = 0;					// GH-Wasnt set
	xgrav = 0.0f; ygrav = 0.0f;		// GH-Wasnt set
	BurnDown = FALSE;

	SquigglyWiggly = FALSE; GalacticStorm = FALSE; PixieDust = FALSE;

}

void ParticleParticle::Frame ()
{
//	double kick;
//	//Edge collision variance for this frame.
//	kick = ((rand() % 2000) * KICK_STRENGTH) / 1000.0 - KICK_STRENGTH;

	parent->LastTime = parent->Time;
	parent->Time = time(NULL);

	if(ParticleStyle == STYLE_STARFIELD)
		this->Frame_Starfield ();
	else
		this->Frame_Gravity ();

}

// Clear the modes
void ParticleParticle::ClearMode (int mode)
{
	switch (mode)
	{
	case 0:	// Clear everything
		PixieDust = RainbowHole = SquigglyWiggly = GalacticStorm = Explode = InnerRing = Emit = Popcorn = FALSE;
		break;
	}
}

// Set a mode
void ParticleParticle::SetMode (int mode)
{
	// Clear all of the modes to start with
	ClearMode ();

	switch (mode)
	{
	case STYLE_EXPLOSIVE:		Explode = TRUE;			break;
	case STYLE_RINGS:			InnerRing = TRUE;		break;
	case STYLE_SPIRALS:			Emit = TRUE;	EmitRotate = (rand() % 3) - 1;
														break;
	case STYLE_POPCORN:			Popcorn = TRUE;			break;
	case STYLE_RAINBOWHOLE:		RainbowHole = TRUE;		break;
	case STYLE_WORMS:			SquigglyWiggly = TRUE;	break;
	case STYLE_GALATIC_STORM:	GalacticStorm = TRUE;	break;
	case STYLE_PIXIE_DUST:		PixieDust = TRUE;		break;
	case STYLE_GEOFF:									break;
	}
}


void ParticleParticle::Frame_Starfield ()
{
	// Turn off the Wall of Fire for the starfield
	NoiseBurn = -1;

	static int starsinit = 0;

	float halfw = (float)(parent->screen.WIDTH / 2), halfh = (FLOAT)(parent->screen.HEIGHT / 2);
	float i128 = 1.0f / (256.0f / 2.0f);
	for(int i = 0; i < nParticles; i++)
	{
		parent->p[i].attract = ATTRACT_NONE;
		if(!starsinit)
		{
			parent->p[i].color = rand() % 255;	//Color is Z coord.
			parent->p[i].SetTrueColor(parent->pe);
			//
			parent->p[i].ax = frand(1.0f);
			parent->p[i].ay = frand(1.0f);
		//	parent->p[i].lx = parent->p[i].ly = 0.0f;
		}
		float lx, ly;
		float iz = 1.0f / (2.0f - (float)parent->p[i].color * i128);
		lx = parent->p[i].ax * iz;// (2.0f - (float)parent->p[i].color / 128.0f);
		ly = parent->p[i].ay * iz;/// (2.0f - (float)parent->p[i].color / 128.0f);
		parent->p[i].color += ZMoveSpeed;
		parent->p[i].SetTrueColor(parent->pe);
		//
		if(parent->p[i].color > 255)
		{// || fabs(lx) > 1.0f || fabs(ly) > 1.0f){
			parent->p[i].color = __max(0, parent->p[i].color - 255);
			parent->p[i].SetTrueColor(parent->pe);
			//
			parent->p[i].ax = frand(1.0f);
			parent->p[i].ay = frand(1.0f);
		}
		float x, y;
		iz = 1.0f / (2.0f - (float)parent->p[i].color * i128);
		x = parent->p[i].ax * iz;/// (2.0f - (float)parent->p[i].color / 128.0f);
		y = parent->p[i].ay * iz;/// (2.0f - (float)parent->p[i].color / 128.0f);
		if(fabs(x) > 1.0f || fabs(y) > 1.0f)
		{
			parent->p[i].color = 0;
			parent->p[i].SetTrueColor(parent->pe);
			//
			parent->p[i].ax = frand(1.0f);
			parent->p[i].ay = frand(1.0f);
		}
		parent->p[i].dx = (x - lx) * halfw;//(WIDTH >> 1);
		parent->p[i].dy = (y - ly) * halfh;//(HEIGHT >> 1);
		parent->p[i].x = lx * halfw + halfw;//(float)(WIDTH >> 1) + (WIDTH >> 1);
		parent->p[i].y = ly * halfh + halfh;//(float)(HEIGHT >> 1) + (HEIGHT >> 1);
		//
	}

	starsinit = 1;
}

void ParticleParticle::Frame_Gravity ()
{
	// Gravity direction, based on time.
	switch(((parent->Time - parent->TimeStart) / GRAV_TIME) % 4)
	{
		case 0 :  xgrav = 0.0f;  ygrav = GRAVITY;  BurnDown = FALSE;  break;
		case 1 :  xgrav = -GRAVITY;  ygrav = 0.0f;  break;
		case 2 :  xgrav = 0.0f;  ygrav = -GRAVITY;  BurnDown = TRUE;  break;
		case 3 :  xgrav = GRAVITY;  ygrav = 0.0f;  break;
	}

	// Randomly cause particle effects to happen.
	if(parent->Time > parent->LastTime && UseRandom)
	{
		// Randomly change things
		if((rand() ) % RANDEFFECT == 0)
		{
			// Randomly modify the effects and styles
//			switch((rand() >>3) % 17)
			switch(rand() % 7)
			{
				case 0 : ShakeUp = TRUE;					break;	// Modifier
				case 1 : Freeze = TRUE;						break;	// Modifier
				case 2 : Comet = TRUE;						break;
				case 3 : Follow = !Follow;					break;	// Modifier
				case 4 : MultipleFollow = !MultipleFollow;	break;	// Modifier
				case 5 : NoiseBurn = rand()%3 - 1;			break;	// Modifier
				case 6 : UseGravity = !UseGravity;			break;	// Modifier
			}

		} // End, Randomly change things
	}

	// Set states based on the Particle Style
	if (ParticleStyle == STYLE_NORMAL) {
		// Randomly change things
		if((rand() ) % (RANDEFFECT*16) == 0)
		{
			// Randomly cycle through the modes
			switch(rand() % 8)
			{
				case 0 : SetMode (STYLE_SPIRALS);		break;
				case 1 : SetMode (STYLE_EXPLOSIVE);		break;
				case 2 : SetMode (STYLE_RINGS);			break;
				case 3 : SetMode (STYLE_POPCORN);		break;
				case 4 : SetMode (STYLE_RAINBOWHOLE);	Do_RainbowHole (1);	break;
				case 5 : SetMode (STYLE_WORMS);			break;
				case 6 : SetMode (STYLE_GALATIC_STORM);	break;
				case 7 : SetMode (STYLE_PIXIE_DUST);	break;
			}
		}
	}
	else if(ParticleStyle == STYLE_POPCORN)
	{
		// Set the mode 
		SetMode (STYLE_POPCORN);
	}
	else if(ParticleStyle == STYLE_SPIRALS)
	{
		// Set the mode 
		SetMode (STYLE_SPIRALS);
	}
	else if(ParticleStyle == STYLE_RINGS)
	{
		// Set the mode 
		SetMode (STYLE_RINGS);
	}
	else if(ParticleStyle == STYLE_EXPLOSIVE)
	{
		// Set the mode 
		SetMode (STYLE_EXPLOSIVE);
	}
	else if(ParticleStyle == STYLE_RAINBOWHOLE)
	{
		// Set the mode 
		SetMode (STYLE_RAINBOWHOLE);
	}
	else if(ParticleStyle == STYLE_WORMS)
	{
		// Set the mode 
		SetMode (STYLE_WORMS);
	}
	else if(ParticleStyle == STYLE_GALATIC_STORM)
	{
		// Set the mode 
		SetMode (STYLE_GALATIC_STORM);
	}
	else if(ParticleStyle == STYLE_PIXIE_DUST)
	{
		// Set the mode 
		SetMode (STYLE_PIXIE_DUST);
	}
	else if(ParticleStyle == STYLE_GEOFF)
	{
		// Set the mode 
		SetMode (STYLE_GEOFF);
	}

	// Check Following the mouse
	if(Follow == FALSE && (rand() % 4) == 0){
		FollowMouse = TRUE;
		parent->XMouse = rand() % parent->screen.WIDTH;
		parent->YMouse = rand() % parent->screen.HEIGHT;
	}
	else{
		FollowMouse = FALSE;
	}

	// Handle the Gravity stuff
	this->Handle_Gravity ();

}

void ParticleParticle::Handle_Gravity ()
{
//	int i;

	if(Popcorn){
		Do_Popcorn ();
	}
	if(InnerRing){
		Do_InnerRing ();
	}
	//Shake particles up when left button pressed.
	if(ShakeUp){
		Do_ShakeUp ();
	}
	//Freeze particles when right button pressed.
	if(Freeze){
		Do_Freeze ();
	}
	//Explode particles when space bar pressed.
	if(Explode){
		Do_Explode ();
	}
	//Create a comet particle when enter is pressed.
	if(Comet){
		Do_Comet ();
	}
	//Drop particles from center of screen when backspace pressed.
	if (Emit) {
		Do_Emit ();
	}

	// GH-New Modes
	if (RainbowHole) {
		Do_RainbowHole ();
	}
	if (SquigglyWiggly) {
		Do_SquigglyWiggly ();
	}
	if (GalacticStorm) {
		Do_GalacticStorm ();
	}
	if (PixieDust) {
		Do_PixieDust ();
	}

//	char buff[255];
//	sprintf (buff, "%d %d %d %d %d %d %d %d %d %d %d", Popcorn, InnerRing, ShakeUp, Freeze, Explode, Comet, Emit, RainbowHole, SquigglyWiggly, GalacticStorm, PixieDust);
//	error_print (buff);

	// Test Particle
	if (ParticleStyle == STYLE_GEOFF) {
		Do_Geoff ();
	}

//	static float cubex, cubey, cubez, cubesize = 100;
//	static int cppe = 10;
//	if(Cube3D){
//	}


	// Attract or Follow the Mouse (left over from Particle Toy?)
	if(Attract || FollowMouse)
	{
		Do_AttractFollow ();
	}

	// Purpose of this?  To swing particles around in a circular pattern?
	IdentityAngle += 0.01f;
	if(IdentityAngle > 3.14159f) IdentityAngle -= 3.14159f * 2.0f;

}

void ParticleParticle::Do_Popcorn ()
{
	static int firstInit = 0;

	if (ParticleStyle != STYLE_POPCORN || firstInit == 0 || (ParticleStyle == STYLE_POPCORN && rand()%50 == 0) )
	{
		firstInit = 1;	// Do this at least once

		for(int n = rand() % 15 + 5; n; n--)
		{
			float velocity = fabs(frand(6.0f)) + 1.0f;
			float ex, ey;
			int sunburst = rand() & 1;
			int pnt = rand() % nParticles;
			//
			ex = parent->p[pnt].x;
			ey = parent->p[pnt].y;
			//
			float pvel, angle;

			for(int ii = nParticles / 20; ii; ii--)
			{
				int i = rand() % nParticles;
				parent->p[i].x = ex;
				parent->p[i].y = ey;
				if(sunburst)
				{
					pvel = velocity * 0.5f;
				}
				else
				{
					pvel = fabs(frand(velocity));
				}
				angle = frand(3.14159f);
				parent->p[i].dx = cos(angle) * pvel;
				parent->p[i].dy = sin(angle) * pvel;
				if(AltColor) parent->p[i].color = rand() % 84 + 170;
				//
				parent->p[i].SetTrueColor(parent->pe);
			}
		}
		Popcorn = FALSE;
	}
}

void ParticleParticle::Do_InnerRing ()
{
	static int firstInit = 0;

	if (ParticleStyle != STYLE_RINGS || firstInit == 0 || (ParticleStyle == STYLE_RINGS && rand()%1000 == 0) )
	{
		firstInit = 1;	// Do this at least once

		float velocity = fabs(frand(10.0f)) + 2.0f;
		float pvel, angle;
		int exprob = (rand() % 3) + 1;
		float size = fabs(frand(0.5f)) + 0.4f;
		float w2 = (float)parent->screen.WIDTH * size * 0.5f;
		float h2 = (float)parent->screen.HEIGHT * size * 0.5f;
		int sunburst = rand() & 1;
		int rot = rand() % 4;
		float rx = (float)parent->screen.WIDTH / 2.0f + frand((float)parent->screen.WIDTH / 2.0f - w2);
		float ry = (float)parent->screen.HEIGHT / 2.0f + frand((float)parent->screen.HEIGHT / 2.0f - h2);
		for(int i = 0; i < nParticles; i++)
		{
			if(i % exprob == 0)
			{
				angle = frand(3.14159f);
				parent->p[i].x = rx + sin(angle) * h2;
				parent->p[i].y = ry - cos(angle) * h2;
				if(sunburst)
				{
					pvel = velocity * 0.5f;
				}
				else
				{
					pvel = fabs(frand(velocity));
				}

				if(rot == 1) pvel = 0.0f;	//Frozen ring.
				if(rot == 2) angle += 3.14159f * 0.5f;	//0 produces no rotation.
				if(rot == 3) angle -= 3.14159f * 0.5f;
				parent->p[i].dx = -sin(angle) * pvel;
				parent->p[i].dy = cos(angle) * pvel;
				if(AltColor) parent->p[i].color = rand() % 84 + 170;
				//
				parent->p[i].SetTrueColor(parent->pe);
			}
		}
		InnerRing = FALSE;
	}
}

void ParticleParticle::Do_ShakeUp ()
{
	for(int i = 0; i < nParticles; i++)
	{
		parent->p[i].dx -= xgrav * 40;
		parent->p[i].dy -= ygrav * 40;
	}
	ShakeUp = FALSE;
}

void ParticleParticle::Do_Freeze ()
{
	for(int i = 0; i < nParticles; i++)
	{
		parent->p[i].dx = 0.0f;
		parent->p[i].dy = 0.0f;
	}
	Freeze = FALSE;
}

void ParticleParticle::Do_Explode ()
{
	static int firstInit = 0;

	if (ParticleStyle != STYLE_EXPLOSIVE || firstInit == 0 || (ParticleStyle == STYLE_EXPLOSIVE && rand()%50 == 0) )
	{
		firstInit = 1;	// Do this at least once

		float velocity = fabs(frand(9.0f)) + 3.0f;
		float ex, ey;
		int sunburst = rand() & 1;
		if(ExplodeX == 0 || ExplodeY == 0)
		{
			ex = (float)(XOFF + rand() % (parent->screen.WIDTH - XOFF * 2));
			ey = (float)(YOFF + rand() % (parent->screen.HEIGHT - YOFF * 2));
		}
		else
		{
			ex = (float)__min(__max(ExplodeX, XOFF), parent->screen.WIDTH - XOFF - 1);
			ey = (float)__min(__max(ExplodeY, YOFF), parent->screen.HEIGHT - YOFF - 1);
		}
		ExplodeX = ExplodeY = 0;
		float pvel, angle;
		int exprob = (rand() % 3) + 1;
		for(int i = 0; i < nParticles; i++)
		{
			if(i % exprob == 0)
			{
				parent->p[i].x = ex;
				parent->p[i].y = ey;
				if(sunburst)
				{
					pvel = velocity * 0.5f;
				}
				else
				{
					pvel = fabs(frand(velocity));
				}
				angle = frand(3.14159f);
				parent->p[i].dx = cos(angle) * pvel;
				parent->p[i].dy = sin(angle) * pvel;
				if(AltColor) parent->p[i].color = rand() % 84 + 170;
				//
				parent->p[i].SetTrueColor(parent->pe);
			}
		}
		Explode = FALSE;
	}
}

void ParticleParticle::Do_Comet ()
{
	float velocity = fabs(frand(8.0f));
	float ex = (float)(XOFF + rand() % (parent->screen.WIDTH - XOFF * 2));
	float ey = (float)(YOFF + rand() % (parent->screen.HEIGHT - YOFF * 2));
	float angle = frand(3.14159f);
	for(int i = 0; i < nParticles; i++)
	{
		parent->p[i].x = ex + frand(1.0f);
		parent->p[i].y = ey + frand(1.0f);
		parent->p[i].dx = cos(angle) * velocity;
		parent->p[i].dy = sin(angle) * velocity;
		if(AltColor) parent->p[i].color = rand() % 84 + 170;
		//
		parent->p[i].SetTrueColor(parent->pe);
	}
	Comet = FALSE;
}

void ParticleParticle::Do_Emit ()
{
	static int firstInit = 0;

	if (ParticleStyle != STYLE_SPIRALS || firstInit == 0 || (ParticleStyle == STYLE_SPIRALS && rand()%500 == 0) )
	{
		firstInit = 1;	// Do this at least once
		int i;

		EmitCount = nParticles;

		if(EmitCount > 0)
		{
			float angscale = (3.14159f * 2.0f) / ((float)nParticles / 4.0f);
			if(EmitRotate < 0) angscale = -angscale;
			i = EmitCount;
			if (!firstInit)
				EmitCount = __max(EmitCount - __max(nParticles / 100, 1), 0);

			while(i-- > EmitCount)
			{
				parent->p[i].x = (float)(parent->screen.WIDTH / 2);
				parent->p[i].y = (float)(parent->screen.HEIGHT / 2);
				if(EmitRotate != 0)
				{
					parent->p[i].dx = sin((float)i * angscale) * 4.0f;
					parent->p[i].dy = -cos((float)i * angscale) * 4.0f;
				}
				else
				{
					parent->p[i].dx = 0.0f;
					parent->p[i].dy = 0.0f;
				}
				if (AltColor) parent->p[i].color = rand() % 84 + 170;
				//
				parent->p[i].SetTrueColor(parent->pe);
			}
			Emit = FALSE;
		}

	}
}

void ParticleParticle::Do_AttractFollow ()
{
	int i;

	int tt = (MultipleFollow ? 63 : 0x7fff);

	for(i = 0; i < nParticles; i++)
	{
		if(Follow && i & tt)
		{
			if(FollowMouse && ((i & tt) == 0))
			{
				parent->p[i].ax = (float)parent->XMouse;
				parent->p[i].ay = (float)parent->YMouse;
			}
			else
			{
				parent->p[i].ax = parent->p[i & ~tt].x;
				parent->p[i].ay = parent->p[i & ~tt].y;
			}
			parent->p[i].attract = ATTRACT_ANGLE | (UseGravity ? ATTRACT_GRAVITY : 0);
		}
		else
		{
			if(FollowMouse)
			{
				parent->p[i].ax = (float)parent->XMouse;
				parent->p[i].ay = (float)parent->YMouse;
				parent->p[i].attract = ATTRACT_ANGLE | (UseGravity ? ATTRACT_GRAVITY : 0);
			}
			else
			{
				parent->p[i].attract = ATTRACT_NONE | (UseGravity ? ATTRACT_GRAVITY : 0);
			}
			parent->p[i].SetTrueColor(parent->pe);	//Make leaders constantly change true color.
		}
	}

}

float Distance (float x1, float y1, float x2, float y2)
{
	float dist;
	float tx, ty;			// Temp
	
	// Check Special Cases //
	if (x1 == x2)
		return((float) fabs (y1 - y2) );
	if (y1 == y2)
		return((float) fabs (x1 - x2) );

	tx = x1 - x2;	// Set initial x distances //
	ty = y1 - y2;	// Dont need to abs() them since their going to be squared //

	dist = (float) sqrt ((double) ((tx * tx) + (ty * ty)));	// Dist. Formula //

	return (dist);
} /// vtMath::Distance()

// Rainbow hole
void ParticleParticle::Do_RainbowHole (int initNow)
{
	static int init = 0;		// Has this style been init'd?
	static float velocity;
		
	// Randomly re-init the pepper
//	if (rand()%175 == 0 || !init)
	if (!init || initNow || (ParticleStyle == STYLE_RAINBOWHOLE && rand()%1000 == 0) )
	{
		// Create velocity
		velocity = fabs(frand(6.0f));
		velocity = __max (velocity, 1.0f);

		// Randomly pick new magnet point
		MagnetX = parent->screen.WIDTH/2 + rand()%100 - 50;
		// If we are burning up
		if (!this->BurnDown)
			MagnetY = __min (parent->screen.HEIGHT/2 + rand()%80 - 40 + (parent->screen.HEIGHT/4), parent->screen.HEIGHT-20);
		else
			MagnetY = __max (parent->screen.HEIGHT/2 + rand()%80 - 40 - (parent->screen.HEIGHT/4), 20);

		// Cycle through the particles
		for(int i = 0; i < nParticles; i++)
		{
			// Randomly place the particles on the screen
			parent->p[i].x = rand() % parent->screen.WIDTH;
			parent->p[i].y = rand() % parent->screen.HEIGHT;

			// Point to the movement direction at the Magnet point
			float dist = Distance (parent->p[i].x, parent->p[i].y, MagnetX, MagnetY);
			parent->p[i].dx = ( (float) (MagnetX - parent->p[i].x) / dist) * velocity;
			parent->p[i].dy = ( (float) (MagnetY - parent->p[i].y) / dist) * velocity;

			// Randomly set color
			if(AltColor) parent->p[i].color = rand() % 84 + 170;
			//
			parent->p[i].SetTrueColor(parent->pe);
		}

		init = 1;		// Make sure we've init'd the style
	}
	else 
	{
		if (rand()%10 == 0)
		{
			// Move the magnet to the right or left
			MagnetX += rand()%10 - 5;
			if (MagnetX < 30) MagnetX = 30;
			else if (MagnetX > parent->screen.WIDTH-30) MagnetX = parent->screen.WIDTH-30;
//			MagnetX = parent->screen.WIDTH/2 + rand()%100 - 50;

			// Cycle through the particles
			for(int i = 0; i < nParticles; i++)
			{
				// Point to the movement direction at the Magnet point
				float dist = Distance (parent->p[i].x, parent->p[i].y, MagnetX, MagnetY);
				parent->p[i].dx = ( (float) (MagnetX - parent->p[i].x) / dist) * velocity;
				parent->p[i].dy = ( (float) (MagnetY - parent->p[i].y) / dist) * velocity;

				// Randomly set color
//				if(AltColor) parent->p[i].color = rand() % 254;
				if(AltColor) parent->p[i].color = rand() % 84 + 170;
				//
				parent->p[i].SetTrueColor(parent->pe);
			}
		}
	}

}

// Squiggly wiggly
void ParticleParticle::Do_SquigglyWiggly (int init)
{
	static int firstInit = 0;
	float velocity, angle;

	// If we're initing the routine
	if (init || !firstInit || rand() % 1500 == 0)
	{
		// Cycle through the particles
		for (int i=0; i < nParticles; i++)
		{
			// Random position of the particle
			parent->p[i].x = rand () % (parent->screen.WIDTH-20) + 10;
			parent->p[i].y = rand () % (parent->screen.HEIGHT-20) + 10;
			//
			if(AltColor) parent->p[i].color = rand() % 84 + 170;
			//
			parent->p[i].SetTrueColor(parent->pe);
		}
	}

	// Create velocity
	velocity = fabs(frand(6.0f));
	velocity = __max (velocity, 1.0f);

	for (int i=0; i < nParticles; i++) {
		// If this is a leader particle
		if (i % 5 == 0) 
		{
			// Randomly change direction
			if (rand() % 100) 
			{
				parent->p[i].dx = frand (1.0f) * velocity;
				parent->p[i].dy = frand (1.0f) * velocity;
			}
		}
		// Follower particle
		else 
		{
			int leader = i - i%5;	// Save the leader particle

			// Move this particle automatically the DX/DY of the leader so that it keeps pace.  This way the particles movement will all be about circling
			parent->p[i].x += parent->p[i].dx;
			parent->p[i].y += parent->p[i].dy;

			// Circle around the leader, twice the velocity?
//			int lx = parent->p[leader].x, ly = parent->p[leader].y;

			// Set circular delta
			angle = frand(3.14159f);
			parent->p[i].dx = cos(angle) * velocity;
			parent->p[i].dy = sin(angle) * velocity;
			//
			if(AltColor) parent->p[i].color = rand() % 84 + 170;
			//
			parent->p[i].SetTrueColor(parent->pe);

		}
	}
}


// Galatic Storm
void ParticleParticle::Do_GalacticStorm (int init)
{
	static int firstInit = 0;
	float velocity, angle;

	int leaderPartition = 25;		// Partition between leaders (number of followers to leaders)

	// Create velocity
	velocity = fabs(frand(6.0f));
	velocity = __max (velocity, 1.0f);

	// Init the particles, either the first time we start the routine, or when we are told to
	if (init || !firstInit || rand() % 1500 == 0)
	{
		// Cycle through particles
		for (int i=0; i < nParticles; i++) 
		{
			// If this is a leader particle
			if (i % leaderPartition == 0) 
			{
				// Random position of the particle
				parent->p[i].x = rand () % (parent->screen.WIDTH-20) + 10;
				parent->p[i].y = rand () % (parent->screen.HEIGHT-20) + 10;

				// Random directions
				parent->p[i].dx = frand (1.0f) * velocity;
				parent->p[i].dy = frand (1.0f) * velocity;
				//
				if(AltColor) parent->p[i].color = rand() % 84 + 170;
				//
				parent->p[i].SetTrueColor(parent->pe);
			}
			// Dont need to do anything for the non-leaders, as they are updated every frame
		}
	}

	// Cycle through particles
	for (int i=0; i < nParticles; i++) {
		// If this is a leader particle
		if (i % leaderPartition == 0) 
		{
			// Randomly change direction
			if (rand() % 10000) 
			{
				// Random directions
				parent->p[i].dx = frand (1.0f) * velocity;
				parent->p[i].dy = frand (1.0f) * velocity;
			}
		}
		// Follower particle
		else 
		{
			// Save the leader particle
			int leader = i - (i % leaderPartition);

			// Move this particle automatically the DX/DY of the leader so that it keeps pace.  This way the particles movement will all be about circling
			parent->p[i].x += parent->p[i].dx;
			parent->p[i].y += parent->p[i].dy;

			// Circle around the leader, twice the velocity?
//			int lx = parent->p[leader].x, ly = parent->p[leader].y;

			// Set circular delta
			angle = frand(3.14159f);
//			parent->p[i].dx = cos(angle) * velocity;
			parent->p[i].dx = sin(angle) * velocity;
			parent->p[i].dy = sin(angle) * velocity;
			//
			if(AltColor) parent->p[i].color = rand() % 84 + 170;
			//
			parent->p[i].SetTrueColor(parent->pe);
		}
	}
}

// Pixie Dust
void ParticleParticle::Do_PixieDust (int init)
{
	static int firstInit = 0;
	float velocity, angle;

	int leaderPartition = 5;		// Partition between leaders (number of followers to leaders)
	float randDist = 5.0f;			// Random distance from the leader particle

	int lastLeader;
	float buffer, bufferRand;

	// Create velocity
	velocity = fabs(frand(10.0f));
	velocity = __max (velocity, 1.0f);

	// Init the particles, either the first time we start the routine, or when we are told to
	if (init || !firstInit || rand() % 1500 == 0)
	{
		// Cycle through particles
		for (int i=0; i < nParticles; i++) 
		{
			// If this is a leader particle
			if (i % leaderPartition == 0) 
			{
				lastLeader = i;

				// Random position of the particle
				parent->p[i].x = rand () % (parent->screen.WIDTH-20) + 10;
				parent->p[i].y = rand () % (parent->screen.HEIGHT-20) + 10;

				// Random directions
				parent->p[i].dx = frand (1.0f) * velocity;
				parent->p[i].dy = frand (1.0f) * velocity;
				//
				if(AltColor) parent->p[i].color = rand() % 84 + 170;
				//
				parent->p[i].SetTrueColor(parent->pe);
			}
			// Followers
/*			else
			{
				// Save the leader particle
//				int leader = i - (i % leaderPartition);
				int leader = lastLeader;

				// Set this particle to the position of its leader, with an offset
				float buffer_x, buffer_y;
				buffer_x = frand (randDist) - randDist/2.0f;
				buffer_y = frand (randDist) - randDist/2.0f;

				parent->p[i].x = parent->p[leader].x + buffer_x;
				parent->p[i].y = parent->p[leader].y + buffer_y;
			}*/
		}

		// Make sure the first init has been marked
		firstInit = 1;
	}

	// Cycle through particles
	for (int i=0; i < nParticles; i++) {
		// If this is a leader particle
		if (i % leaderPartition == 0) 
		{
			lastLeader = i;

			// Randomly change direction
			if (rand() % 50 == 0) 
			{
				// Random directions
				parent->p[i].dx = frand (1.0f) * velocity;
				parent->p[i].dy = frand (1.0f) * velocity;
				//
				if(AltColor) parent->p[i].color = rand() % 84 + 170;
				//
				parent->p[i].SetTrueColor(parent->pe);
			}
		}
		// Follower particle
		else 
		{
			// Save the leader particle
			int leader = lastLeader;

			// Move this particle automatically the DX/DY of the leader so that it keeps pace.  This way the particles movement will all be about circling
			bufferRand = frand (randDist);	buffer = bufferRand - (randDist/2.0f);
			if (buffer < 0.0f && fabs (buffer) < randDist/4.0f)	buffer = -randDist/4.0f;
			else if (buffer > 0.0f && fabs (buffer) < randDist/4.0f)	buffer = randDist/4.0f;
			//
//			parent->p[i].x = parent->p[leader].x + buffer + parent->p[leader].dx;
			parent->p[i].x = parent->p[leader].x + buffer;
			//
			bufferRand = frand (randDist);	buffer = bufferRand - (randDist/2.0f);
			if (buffer < 0.0f && fabs (buffer) < randDist/4.0f)	buffer = -randDist/4.0f;
			else if (buffer > 0.0f && fabs (buffer) < randDist/4.0f)	buffer = randDist/4.0f;
			//
//			parent->p[i].y = parent->p[leader].y + buffer + parent->p[leader].dx;
			parent->p[i].y = parent->p[leader].y + buffer;

			// Set circular delta
			angle = frand(3.14159f);
			parent->p[i].dx = parent->p[leader].dx / 3.0f;//0;
			parent->p[i].dy = parent->p[leader].dy / 3.0f;//0;
/*			parent->p[i].dx = 0;
			parent->p[i].dy = 0;*/

			//
			if(AltColor) parent->p[i].color = rand() % 84 + 170;
			//
			parent->p[i].SetTrueColor(parent->pe);
		}
	}
}



// Atomic particles
void ParticleParticle::Do_Geoff (int init)
{
	static int firstInit = 0;
	float velocity, angle;

	int leaderPartition = 5;		// Partition between leaders (number of followers to leaders)
	float randDist = 5.0f;			// Random distance from the leader particle

	int lastLeader;
	float buffer, bufferRand;

	// Create velocity
	velocity = fabs(frand(10.0f));
	velocity = __max (velocity, 1.0f);

	// Init the particles, either the first time we start the routine, or when we are told to
	if (init || !firstInit || rand() % 1500 == 0)
	{
		// Cycle through particles
		for (int i=0; i < nParticles; i++) 
		{
			// If this is a leader particle
			if (i % leaderPartition == 0) 
			{
				lastLeader = i;

				// Random position of the particle
				parent->p[i].x = rand () % (parent->screen.WIDTH-20) + 10;
				parent->p[i].y = rand () % (parent->screen.HEIGHT-20) + 10;

				// Random directions
				parent->p[i].dx = frand (1.0f) * velocity;
				parent->p[i].dy = frand (1.0f) * velocity;
				//
				if(AltColor) parent->p[i].color = rand() % 84 + 170;
				//
				parent->p[i].SetTrueColor(parent->pe);
			}
			// Followers
/*			else
			{
				// Save the leader particle
//				int leader = i - (i % leaderPartition);
				int leader = lastLeader;

				// Set this particle to the position of its leader, with an offset
				float buffer_x, buffer_y;
				buffer_x = frand (randDist) - randDist/2.0f;
				buffer_y = frand (randDist) - randDist/2.0f;

				parent->p[i].x = parent->p[leader].x + buffer_x;
				parent->p[i].y = parent->p[leader].y + buffer_y;
			}*/
		}

		// Make sure the first init has been marked
		firstInit = 1;
	}

	// Cycle through particles
	for (int i=0; i < nParticles; i++) {
		// If this is a leader particle
		if (i % leaderPartition == 0) 
		{
			lastLeader = i;

			// Randomly change direction
			if (rand() % 50 == 0) 
			{
				// Random directions
				parent->p[i].dx = frand (1.0f) * velocity;
				parent->p[i].dy = frand (1.0f) * velocity;
				//
				if(AltColor) parent->p[i].color = rand() % 84 + 170;
				//
				parent->p[i].SetTrueColor(parent->pe);
			}
		}
		// Follower particle
		else 
		{
			// Save the leader particle
			int leader = lastLeader;

			// Move this particle automatically the DX/DY of the leader so that it keeps pace.  This way the particles movement will all be about circling
			bufferRand = frand (randDist);	buffer = bufferRand - (randDist/2.0f);
			if (buffer < 0.0f && fabs (buffer) < randDist/4.0f)	buffer = -randDist/4.0f;
			else if (buffer > 0.0f && fabs (buffer) < randDist/4.0f)	buffer = randDist/4.0f;
			//
//			parent->p[i].x = parent->p[leader].x + buffer + parent->p[leader].dx;
			parent->p[i].x = parent->p[leader].x + buffer;
			//
			bufferRand = frand (randDist);	buffer = bufferRand - (randDist/2.0f);
			if (buffer < 0.0f && fabs (buffer) < randDist/4.0f)	buffer = -randDist/4.0f;
			else if (buffer > 0.0f && fabs (buffer) < randDist/4.0f)	buffer = randDist/4.0f;
			//
//			parent->p[i].y = parent->p[leader].y + buffer + parent->p[leader].dx;
			parent->p[i].y = parent->p[leader].y + buffer;

			// Set circular delta
			angle = frand(3.14159f);
			parent->p[i].dx = parent->p[leader].dx / 3.0f;//0;
			parent->p[i].dy = parent->p[leader].dy / 3.0f;//0;
/*			parent->p[i].dx = 0;
			parent->p[i].dy = 0;*/

			//
			if(AltColor) parent->p[i].color = rand() % 84 + 170;
			//
			parent->p[i].SetTrueColor(parent->pe);
		}
	}
}


