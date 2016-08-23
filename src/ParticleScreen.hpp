// Particle Fire Screen class - header

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

#ifndef PARTICLE_SCREEN_HPP
#define PARTICLE_SCREEN_HPP

#include <windows.h>
#include <math.h>
#include <time.h>

#include <CDib.h>
#include "Timer.h"
#include "ParticleFire.h"
#include "Basis.h"
#include "CStr.h"

#define MAX_PART 10000

class ParticleContainer;

class ParticleScreen 
{
public:
	ParticleScreen  ();
	~ParticleScreen  ();

	void Init ();
	void InitScreen (HWND hwnd);

	int SetScreenMode (int w, int h, int bpp);
	void ResetScreenmode ();

	void Draw ();						// Main drawing routine
	void DrawParticles ();				// Draw and update the particles
	void SeedWall ();					// Seed the Wall of Fire
	void DrawWall ();					// Draw the Wall of Fire

	void HandleText ();					// Handle the Text drawing
	void LoadText ();					// Load the Text from a file
	
	int PickColor(HWND dlg, PALETTEENTRY *pe);
	void SpreadPal(PALETTEENTRY *pe1, PALETTEENTRY *pe2, PALETTEENTRY *dst);
	void Palette(int ColorScheme);

	int SetupFont(int height);
	void UnsetupFont();
	int DrawFont(const char *text, int mode=0);
	int DrawCenteredFont(const char *text, int lines=1, int lineNum=0, int col_r=255, int col_g=255, int col_b=255);
	int DrawXYFont(const char *text, int x, int y, int col_r=255, int col_g=255, int col_b=255);

public:
	ParticleContainer *parent;

	// Local vars
	// Font stuff
	SIZE sz;
	LOGFONT lf;
	HFONT hfont, holdfont;
	HDC fontdc;
	HBITMAP holdbitmap;

	// Particles
	int Burns, BurnTimes, KindleTimes, ParticleTimes, BlitTimes;
	unsigned char BurnFlags[2048];

	// CDib
	HWND m_hWnd;
	CreateDib dib;
	int UseTrueColor;

	// System
	Timer tmr;

	// Particle style info
	float FlameSpeed;

	// Particle style info
//	unsigned char BURNFADE;
	int BURNFADE;
	PALETTEENTRY CustomPE1, CustomPE2;
	int DisableFire;

	// Screen
	int WIDTH, HEIGHT;
	int iDstX, iDstY;

	// Useage time/registration
	long int FirstUseTime;
	long int SecsStart;
	int TotalSecs;
	int LastTextTime;
	int RegLine;

	// Registration text
//#define REGLINES 34
//	char RegText[REGLINES][100];

	// Text
	char *RealQuotes;
	int RealQuoteLines;

	int QuoteSecs;
	int DisableText;

	time_t LastQuotePrintTime;
	int LastQuote;


	// Unknown
	Basis *basis;

	// Preview mode?
	int Preview;

	// Particle style info
	int CustomScheme;
	int ColorScheme;
	int RandomColor;
	unsigned char	fade[256], half[256];

	int CycleColors;

};

#endif

