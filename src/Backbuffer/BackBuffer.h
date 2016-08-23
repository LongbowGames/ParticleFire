/****************************************************************************

Generic full screen and windowed linear frame buffer abstraction class,
with support for simple dirty rectangle updating and true color or
paletted display modes.

This file is part of Particle Fire.

Particle Fire is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Particle Fire is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Particle Fire.  If not, see <http://www.gnu.org/licenses/>.

****************************************************************************/


#ifndef BACKBUFFER_H
#define BACKBUFFER_H

#include <windows.h>
#include <ddraw.h>
#include "cdib.h"

class TrueColorFormat{
public:
	void SetMasks(unsigned int bpp, unsigned int rm, unsigned int gm, unsigned int bm);
		//Sets all values based on supplied masks.
	int MakeLookup(unsigned int *table, PALETTEENTRY *pe, int numcols = 256);
		//Creates a lookup table of true color pixels for a palette.
	int MakeLookup(unsigned short *table, PALETTEENTRY *pe, int numcols = 256);
		//Creates a lookup table of true color pixels for a palette.
	inline unsigned int PackColor(unsigned char r, unsigned char g, unsigned char b){
		return ((r >> RedQuant) << RedMaskOff) |
			((g >> GreenQuant) << GreenMaskOff) |
			((b >> BlueQuant) << BlueMaskOff);
	};	//Returns a single true color pixel for an 888 rgb val.
	inline void UnpackColor(unsigned int c, unsigned char *r, unsigned char *g, unsigned char *b){
		*r = ((c & RedMask) >> RedMaskOff) << RedQuant;
		*g = ((c & GreenMask) >> GreenMaskOff) << GreenQuant;
		*b = ((c & BlueMask) >> BlueMaskOff) << BlueQuant;
	};	//Unpacks a true color pixel into its respective components.
public:
	unsigned int BPP;			//Bits per pixel.
	unsigned int RedMask;		//Bit-mask for red component.
	unsigned int RedMaskLen;	//Bit length of red component.
	unsigned int RedMaskOff;	//Bits to shift red component up.
	unsigned int RedQuant;		//Bits to shift 8-bit value down to get proper length.
	unsigned int GreenMask;
	unsigned int GreenMaskLen;
	unsigned int GreenMaskOff;
	unsigned int GreenQuant;
	unsigned int BlueMask;
	unsigned int BlueMaskLen;
	unsigned int BlueMaskOff;
	unsigned int BlueQuant;
};

struct BufferDesc{
	int bytespixel;
	int width, height, pitch;
	void *data;
};

#define MAX_DIRTY 100

struct DirtyRect{
	int x1, y1, x2, y2;
};

#define UFB_STRETCH	0x01
	//Stretches buffer to fit window or visible surface.
#define UFB_DIRTY	0x02
	//Uses accumulated dirty rectangles to only update changed regions.

class BackBuffer{
public:
	BackBuffer();
	~BackBuffer();
	int InitWindow(int w, int h, const char *Name, HINSTANCE hInst, HWND *phWnd, LRESULT (CALLBACK *WProc)(HWND, UINT, WPARAM, LPARAM), DWORD Icon = NULL);
		//Creates the main window and the DirectDraw object.  Requires a width, height,
		//name, hInstance, and a pointer to a WindowProcedure function.
		//Set icon to NULL to use standard, or to a resource identifier to load that
		//icon from your project.
	int SetBufferMode(int w, int h, int bpp, BOOL fscreen, TrueColorFormat *tcf);
		//Call this function at any time OUTSIDE of a Lock/Unlock pair to set or
		//reset the buffer to a new display mode.  Only 8 and 16 bpp is supported,
		//and in windowed mode a 16bit buffer will first be converted to 24bit for
		//display through Windows.
	int SetWindowPos(int x, int y);
		//In a windowed mode, sets the top/left position of the window, does nothing
		//in a fullscreen mode.
	int SetWindowSize(int w, int h);
		//In a windowed mode, sets the display size of the window (including the
		//title bar at the top, in Win32), does nothing in full screen mode.
		//
		//Now automatically centers the window on the desktop, calling SetWindowPos.
		//To set custom size and position, first size, then move.
		//
	int SetPalette(PALETTEENTRY *pe);
		//Sets the BackBuffer to the entered 256 color palette.  MUST be done for
		//both Windowed and FullScreen BackBuffer objects.  Passing a NULL pointer
		//will use the last-set palette internal to the BackBuffer object.
	int GetPalette(PALETTEENTRY *pe);
		//Fills the passed in array with the buffer's current palette.
	int RealizePalette();
		//Only required for Windowed modes, but harmless otherwise, this function
		//causes the Windows palette manager to accept and realize your palette.
		//Call after SetPalette and in the appropriate Windows palette messages.
	int AddDirtyRect(int x, int y, int w, int h);
	int AddDirtyRect(DirtyRect *dr);
		//Adds a dirty rectangle to the internal list of rectangles to be updated.
	void ClearDirtyRects();
		//Nullifies all previously added dirty rectangles.
	int UpdateFrontBuffer(int flags = NULL);
		//In fullscreen mode, blits the backbuffer data to the physical display
		//screen, perhaps copying first to an offscreen VRAM surface and page-
		//flipping.  In windowed mode, asks Windows to copy the backbuffer to the
		//main window, stretching data to fit.  Possibly the backbuffer data will
		//first be manually converted to 24bit true color first, if the backbuffer
		//is in 16 bits per pixel mode.
		//Set flags to UFB_STRETCH to stretch data to fit window size.
	int Lock(BufferDesc *bufdesc);
		//Locks the backbuffer pixel data and returns a BufferDesc structure with
		//info on properly accessing the data directly.
	int Unlock();
		//Unlocks the backbuffer data.  Any previously acquired BufferDesc structures
		//are invalid after this call.
	int Destroy(int killwindow = FALSE);
		//Destroys all data and resources allocated by the BackBuffer object,
		//including the DirectDraw object, the Window, and all GDI and DDraw buffers.
		//Only call when your program is about to exit, and even then it is best to
		//just delete the object, which will call Destroy to clean up.
	int Centering(int flag);
		//Enables or disables automatic window centering on mode switches etc.
	int Pointer(int flag);
		//Enables mouse pointer in full screen (normally disabled).
	int CooperateYaGit();
		//If fullscreen, forces DDSCL_NORMAL cooperation mode.
	int CooperateNot();
		//Goes back to DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN if in fullscreen.
private:	//Helper functions.
	int InitDIB(int w, int h, int bpp);
	int FreeDIB();
private:	//Private data members.
	BOOL	m_InitInitial;
	BOOL	m_InitDIB;	//Keep track of what's been initialized properly.
	BOOL	m_InitDDRaw;
	BOOL	m_fullscreen2;	//True if DirectDraw SetDisplayMode is in effect.
	WNDCLASSEX			m_wc;
	HWND				m_hwnd;
	CreateDib			m_DIB;
	PALETTEENTRY		m_pe[256];
	TrueColorFormat		m_tcf;
	BufferDesc			m_BufDesc;
	int					m_Locks;	//Number of concurrent Lock()s.
	int m_x, m_y, m_w, m_h;		//Window position and size variables.
	int m_realw, m_realh;		//The width and height needed to generate a w,h client area.
	DDSURFACEDESC	ddsd;	//Temporary member variables.
	HRESULT			ddreturn;
	int centering, pointer;
	DirtyRect dirty[MAX_DIRTY];
	int ndirty;
};

#endif
