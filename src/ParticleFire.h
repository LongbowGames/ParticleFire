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

#ifndef PARTICLE_FIRE_H
#define PARTICLE_FIRE_H

enum{
	ATTRACT_NONE = 0,
	ATTRACT_GRAVITY = 1,
	ATTRACT_ANGLE = 2
};

//struct PALETTEENTRY;

struct Particle{
	float x, y, dx, dy, lx, ly;	//Particle position, velocity, and last position.
	float ax, ay;	//Particle's attractor location.
	unsigned char attract;	//Flag to use attractor or not.
	unsigned char color;	//Particle color index.
	unsigned char red, green, blue;	//RGB color values for True Color fire mode.
public:
	//Will pull pe[color] into red, green, blue.
	void SetTrueColor(PALETTEENTRY pe[]) {
		red = pe[color].peRed;
		green = pe[color].peGreen;
		blue = pe[color].peBlue;
	}
};

//LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
//BOOL CALLBACK DlgProc(HWND dlgwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
void DoFrame();

#define NUMSCHEMES 6
const char ColorName[NUMSCHEMES][100] = {
	"Fiery Orange",
	"Skyish Teal",
	"Velvet Blue",
	"Slimy Green",
	"Burning Pink",
	"Flaming Metal"
};

enum {
	STYLE_NORMAL = 0,
	STYLE_STARFIELD,
	STYLE_EXPLOSIVE,
	STYLE_RINGS, 
	STYLE_SPIRALS,
	STYLE_POPCORN,
	STYLE_RAINBOWHOLE,
	STYLE_WORMS,
	STYLE_GALATIC_STORM,
	STYLE_PIXIE_DUST,
//	STYLE_GEOFF,
	NUMSTYLES,
};

// Temp to remove STYLE_GEOFF (test) without having to remove the code
enum {
	STYLE_GEOFF = 42000,
};

const char StyleName[NUMSTYLES][100] = {
	"Random",
	"StarField",
	"Explosive",
	"Rings",
	"Spirals",
	"Popcorn",
	"Rainbow Hole",
	"Worms",
	"Galactic Storm",
	"Pixie Dust"//,
//	"Geoff"
};

enum {
	STYLE_WALL_NORMAL = 0,
	STYLE_WALL_RAINBOW,
	STYLE_WALL_SMOKE,
	STYLE_WALL_NONE, 
	NUMSTYLEWALLS,
};

const char StyleWallName[NUMSTYLEWALLS][100] = {
	"Random",
	"Rainbow",
	"Smoke",
	"None"
};

#endif

