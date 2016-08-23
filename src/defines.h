// Defines for Particle Fire

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

#define WM_INFINITE (WM_USER + 100)

#define PRELINES 5		// Huh?  Maybe for how many lines are visible at once when showing text?

//Remove this define to return to plain colored particles.
#define BURN

//Number of particles.
#define MAX_PART 10000
//Amount of lateral kick when bouncing.
#define KICK_STRENGTH 0.5f
//For ordinary particles, how many levels (of 64) to fade pixels per frame.
#define FADE_SPEED 4
//Strength of gravity.
#define GRAVITY 0.1f
//Amount of energy retained on bounce.
#define BOUNCE 0.95f

//					7 days
#define MaxISeconds (60 * 60 * 24 * 7)
//#define MaxBSeconds (60 * 60 * 24 * 7)
#define MaxBSeconds (60)		// test, 15 seconds
#define MinBSeconds 120
//Max time before nag.

//Number of pixels to not burn on edges of screen.
#define XOFF 5
#define YOFF 3
#define BXOFF 4
#define BX4OFF (BXOFF / 4)
#define BYOFF 2
//New for improved burn code, BXOFf must be multiple of 4, and XOFF must be greater still.
