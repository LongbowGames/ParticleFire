// Particle Fire Screen class - source

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

#include "windows.h"
//#include "multimon.h"

#include "ParticleScreen.hpp"
#include "ParticleContainer.hpp"

#include "image.h"		// ToolsLib

// For parent-> vars
#include "ParticleParticle.hpp"

#include "textStuff.h"			// Register and random text info

#include "defines.h"

// External functions
extern float frand(float range);
extern void error_print (char *buff);


ParticleScreen::ParticleScreen ()
{
	Init ();
}

ParticleScreen::~ParticleScreen ()
{
	if (RealQuotes != NULL && RealQuotes != (char*) Quotes)	
		free (RealQuotes);
}

void ParticleScreen::Init ()
{
//	sz;											// Not init'd
//	lf;											// Not init'd
//	hfont; holdfont;							// Not init'd
//	fontdc;										// Not init'd
//	holdbitmap;									// Not init'd
//	WIDTH;	HEIGHT;	// Not init'd
//	BurnFlags;		// Not init'd
//	dib;			// Not init'd
//	tmr;
//	Timer UseTmr;
//	fade[256]; half[256];		// Not init'd

	Burns = 0; BurnTimes = 0; KindleTimes = 0; ParticleTimes = 0; BlitTimes = 0;

	UseTrueColor = 1;			// Set multi colored to the default

	//Each pixel is this many units in Noise space.
	FlameSpeed = 0.03f;

//Amount subtracted from each averaged pixel each time through the burn.  Fade out speed.
	BURNFADE = 3;//6;

	CustomPE1.peRed = 220;	CustomPE1.peGreen = 220;	CustomPE1.peBlue = 255;	CustomPE1.peFlags = 0;
	CustomPE2.peRed = 255;	CustomPE2.peGreen = 255;CustomPE2.peBlue = 255;	CustomPE2.peFlags = 0;

	DisableFire = 0;

	FirstUseTime = 0;
	SecsStart = 0;
	TotalSecs = 0;

	LastTextTime = 0;
	RegLine = 0;

	// RegText and Quotes init'd in "textStuff.h"

	QuoteSecs = 26;//10;

	DisableText = 1;//0;		// Dont have random text by default

	// Had to shift it into a pointer to make it work inside the class
	basis = new Basis (0);

	Preview = FALSE;

	CustomScheme = 0;
//Color palette.
	ColorScheme = 0;
	RandomColor = TRUE;

	CycleColors = 1;
	
	RealQuotes = NULL;			// Pointer to memory for quotations
	LastQuote = -1;				// Last quote we displayed

	iDstX = iDstY = 0;
	WIDTH = HEIGHT = 0;
	m_hWnd = NULL;
}

void ParticleScreen::InitScreen (HWND hwnd)
{
	m_hWnd = hwnd;

	RECT rc;
	GetWindowRect(hwnd, &rc);
	if((rc.right - rc.left) < GetSystemMetrics(SM_CXSCREEN) - 32){
		Preview = TRUE;
		parent->particle.nParticles /= 10;
	}else{
		Preview = FALSE;
	}
	//
	SecsStart = time(NULL);
	//
	srand((unsigned)time(NULL));
	//
	GetWindowRect(hwnd, &rc);
	
	//WIDTH = __min(rc.right - rc.left, GetSystemMetrics(SM_CXSCREEN));
	
	WIDTH = __min(rc.right - rc.left, GetSystemMetrics(SM_CXVIRTUALSCREEN));
	HEIGHT = __min(rc.bottom - rc.top, GetSystemMetrics(SM_CYVIRTUALSCREEN));
		
	dib.CreateHBitmap( hwnd, WIDTH, HEIGHT, UseTrueColor ? 32 : 8);

	if( Preview )
	{
		iDstX = iDstY = 0;
	}
	else
	{
		iDstX = rc.left;
		iDstY = rc.top;
	}

	//
	if(CustomScheme) ColorScheme = -1;
	if(RandomColor) ColorScheme = rand() % (NUMSCHEMES + 1) - 1;
	//
	int i;
	//Create fading lookup table.
	for(i = 0; i < 256; i++){
		half[i] = i >>1;//__max(i & ~63, i & ~63 + (int)((i & 63) / 1.5));
	}
	//Initialize particles.
	for(i = 0; i < MAX_PART; i++){
		parent->p[i].x = parent->p[i].lx = (float)(XOFF + rand() % (WIDTH - XOFF * 2));
		parent->p[i].y = parent->p[i].ly = (float)(YOFF + rand() % (HEIGHT - YOFF * 2));
		parent->p[i].dx = parent->p[i].dy = 0.0f;
		parent->p[i].color = rand() % 127 + 127;
	}
	parent->TimeStart = parent->LastTime = parent->Time = time(NULL);
	//
	parent->particle.Follow = rand() & 1;
	parent->particle.MultipleFollow = rand() & 1;
	parent->particle.NoiseBurn = rand() & 1;
	parent->particle.UseGravity = rand() & 1;
	//
	SetupFont((35 * WIDTH) / 640);

	// Load text
	this->LoadText ();
	LastQuotePrintTime = NULL;
}

void ParticleScreen::Draw ()
{
	//Lock buffer.
	if(dib.Data()){
		//Draw particles.
		unsigned char *data, *data2;
		int pitch;
		int d, e, dir, x, y, dx, dy;
		data = (unsigned char*)dib.Data();//bdesc.data;
		pitch = dib.Pitch();//bdesc.pitch;
		unsigned char *bdescdata = dib.Data();
		int bdescpitch = dib.Pitch();
		int bdescwidth = dib.Width();
		int bdescheight = dib.Height();
		float adx, ady, dist, dist2, ang, angd;

		int pitchBit = dib.Pitch() / dib.Width();	// Use for bit depth
	
		tmr.Start();

		// GH-Draw the Particles
		DrawParticles ();

		ParticleTimes += tmr.Check(10000);

		//Kindle bottom row for main fire.
		tmr.Start();

		// Seed the Wall of Fire!
		SeedWall ();

		KindleTimes += tmr.Check(10000);

		tmr.Start();
		Burns++;
//#if 1
		//New burn code.
#define BLONG unsigned long
#define BLONGS sizeof(BLONG)
#define UCP (unsigned char*)
#define BURNIT(n) temp = (*(UCP lptr + n - pitch) + *(UCP lptr + n - 1) + *(UCP lptr + n) + *(UCP lptr + n + 1) - BURNFADE) >>2; \
	*(UCP lptr + n - pitch) = (unsigned char)(temp < 0 ? 0 : temp);
#define BURNIT2(n) temp = (*(UCP lptr + n + pitch) + *(UCP lptr + n - 1) + *(UCP lptr + n) + *(UCP lptr + n + 1) - BURNFADE) >>2; \
	*(UCP lptr + n + pitch) = (unsigned char)(temp < 0 ? 0 : temp);
		//
#define TCBURNIT(n) temp = (*(UCP lptr + n - pitch) + *(UCP lptr + n - 4) + *(UCP lptr + n) + *(UCP lptr + n + 4) - BURNFADE) >>2; \
	*(UCP lptr + n - pitch) = (unsigned char)(temp < 0 ? 0 : temp);
#define TCBURNIT2(n) temp = (*(UCP lptr + n + pitch) + *(UCP lptr + n - 4) + *(UCP lptr + n) + *(UCP lptr + n + 4) - BURNFADE) >>2; \
	*(UCP lptr + n + pitch) = (unsigned char)(temp < 0 ? 0 : temp);
		//
		BLONG *lptr = (BLONG*)bdescdata;
		int lpitch = bdescpitch / BLONGS;
		int lwidth = bdescwidth / BLONGS;
		int xlwidth = lwidth - BX4OFF;
		int bWIDTH = WIDTH;
		int temp;//, x2;
		unsigned char* bf;
		//
		if(UseTrueColor){
			bWIDTH *= 4;
			xlwidth *= 4;
		}
		//

		// If True Color
		if(UseTrueColor)
		{
			// If Burn Down
			if(parent->particle.BurnDown){
				for(y = HEIGHT - BYOFF - 2; y > BYOFF - 1; y--){
					lptr = (BLONG*)bdescdata + (lpitch * y) + BX4OFF;
					for(x = BX4OFF; x < xlwidth; x++){
						TCBURNIT2(0) TCBURNIT2(1) TCBURNIT2(2);// TCBURNIT2(3);
						lptr++;
					}
				}
			} // End, If Burn Down
			// Else, Burn Up
			else
			{
				for(y = BYOFF + 1; y < HEIGHT - BYOFF + 1; y++){
					lptr = (BLONG*)bdescdata + (lpitch * y) + BX4OFF;
					for(x = BX4OFF; x < xlwidth; x++){
						TCBURNIT(0) TCBURNIT(1) TCBURNIT(2);// TCBURNIT(3);
						lptr++;
					}
				}
			} // End, Else, Burn Up
		} // End, If True Color
		// Else, no True Color
		else
		{
			// If Burn Down
			if(parent->particle.BurnDown)
			{
				for(y = HEIGHT - BYOFF - 2; y > BYOFF - 1; y--){
					lptr = (BLONG*)bdescdata + (lpitch * y) + BX4OFF;
					bf = &BurnFlags[BX4OFF];
					for(x = BX4OFF; x < xlwidth; x++){
						if(*(lptr)){
							BURNIT2(0) BURNIT2(1) BURNIT2(2) BURNIT2(3);
							BurnFlags[x] = 1;
						}else{
							if(BurnFlags[x]){
								BURNIT2(0) BURNIT2(1) BURNIT2(2) BURNIT2(3);
							}
							BurnFlags[x] = 0;
						}
						lptr++;
					}
					if(y & 1){
						parent->tdata = (unsigned char*)bdescdata + y * bdescpitch + BXOFF;
						if(y > BYOFF && y < HEIGHT - BYOFF - 1){	//Keeps boost splatter off edges.
							parent->tdata2 = parent->tdata + 1 + (rand() % (bWIDTH - BXOFF * 2 - 2));
							if(*parent->tdata2 > 32 && *parent->tdata2 < 128){
								*(parent->tdata2 + 1) = *(parent->tdata2 - 1) =
								*(parent->tdata2 + bdescpitch) = *(parent->tdata2 - bdescpitch) = 
								__min(*parent->tdata2 << 1, 255);
							}
						}
					}
				}
			} // End, If Burn Down
			// Else, Burn Up
			else
			{
				for(y = BYOFF + 1; y < HEIGHT - BYOFF + 1; y++){
					lptr = (BLONG*)bdescdata + (lpitch * y) + BX4OFF;
					bf = &BurnFlags[BX4OFF];
					for(x = BX4OFF; x < xlwidth; x++){
						if(*(lptr)){
							BURNIT(0) BURNIT(1) BURNIT(2) BURNIT(3);
							BurnFlags[x] = 1;
						}else{
							if(BurnFlags[x]){
								BURNIT(0) BURNIT(1) BURNIT(2) BURNIT(3);
							}
							BurnFlags[x] = 0;
						}
						lptr++;
					}
					if(y & 1){
						parent->tdata = (unsigned char*)bdescdata + y * bdescpitch + BXOFF;
						if(y > BYOFF && y < HEIGHT - BYOFF - 1){	//Keeps boost splatter off edges.
							parent->tdata2 = parent->tdata + 1 + (rand() % (bWIDTH - BXOFF * 2 - 2));
							if(*parent->tdata2 > 32 && *parent->tdata2 < 128){
								*(parent->tdata2 + 1) = *(parent->tdata2 - 1) =
								*(parent->tdata2 + bdescpitch) = *(parent->tdata2 - bdescpitch) = 
								__min(*parent->tdata2 << 1, 255);
							}
						}
					}
				}
			} // End, Else, Burn Up
		} // End, Else, no True Color

		// Setup for fade-zoom
		int UseFadeZoom = 1;

		//Try fade-zoom here.
	//	{
/*		static Frames = 0;
		Frames++;
		static Bitmap TBM;
		if(UseFadeZoom){
			if(TBM.Width() != bdescwidth || TBM.Height() != bdescheight || TBM.Data() == 0){
				TBM.Init(bdescwidth, bdescheight, 8);
			}
			if(TBM.Width() == bdescwidth && TBM.Height() == bdescheight && TBM.Data()){
				TBM.Suck(bdescdata, bdescwidth, bdescheight, bdescpitch, dib.BPP() );//8);
				int Blast = 0;
			//	if((Time / 1) % 10 == 0) Blast = 1;
				if((Frames / 10) % 10 == 0) Blast = 1;
				int xshrink = WIDTH / 50, yshrink = HEIGHT / 50;
			//	int xshrink = WIDTH / 10, yshrink = HEIGHT / 10;
				if(Blast){
					xshrink = WIDTH / 40; yshrink = HEIGHT / 40;
			//		xshrink = WIDTH / 10; yshrink = HEIGHT / 10;
				}
				int xerr, yerr, xeadd, yeadd;
				int srcy, srcx;
				unsigned char *src, *dst;
				int x, y;
				int w = WIDTH - BXOFF * 2;
				int h = (HEIGHT - BYOFF * 2) - 1;
				yerr = h / 2;
				yeadd = yshrink;
				xeadd = xshrink;
				srcy = yshrink >>1;
				//
				unsigned char clamptab[512];
				for(int i = 0; i < 512; i++) clamptab[i] = __min(i, 255);
				//
				for(y = 0; y < h; y++){
					srcx = (xshrink >>1) + BXOFF;
				//	src = bdescdata + srcx + srcy * bdescpitch;
					src = TBM.Data() + srcx + (srcy + BYOFF) * TBM.Pitch();
					dst = bdescdata + BXOFF + (y + BYOFF) * bdescpitch;
					xerr = w / 2;
					if(Blast){
						for(x = w; x; x--){
							unsigned char tc = *src >>1;
						//	int t = (*dst >>1) + (tc >>2) + (tc);//(((*src <<7)) >>8);
						//	if(t > 255) *dst = 255;
						//	else *dst = (unsigned char)t;
							*dst = clamptab[(*dst >>1) + (tc >>2) + (tc)];//(((*src <<7)) >>8);
							//
							xerr += xeadd;
							if(xerr >= w) xerr -= w;
							else src++;
							dst++;
						}
					}else{
						for(x = w; x; x--){
							*dst = (*dst >>1) + (*src >>1);
						//	int t = (*dst >>1) + (*src >>1) - 2;
						//	if(t < 0) *dst = 0;
						//	else *dst = (unsigned char)t;
							dst++;
							xerr += xeadd;
							if(xerr >= w) xerr -= w;
							else src++;
						}
					}
					//
					yerr += yeadd;
					if(yerr >= h){
					//	srcy++;
						yerr -= h;
					}else{
						srcy++;
					}
				//	srcy++;
				}
			}
		}
*/		// End Fade Zoom

		// End timer for burning
		BurnTimes += tmr.Check(10000);

	}
	//Blit backbuffer to window or to primary surface as needed.
	tmr.Start();
	dib.Lock();

	if( Preview ) dib.Blit(0, 0, 0, 0, dib.Width(), dib.Height());
	else
	{
		RECT	rc;
		GetWindowRect(m_hWnd, &rc);
		
		iDstX = rc.left;
		iDstY = rc.top;

		dib.Blit(iDstX, iDstY, 0, 0, dib.Width(), dib.Height());
	}
	
	dib.Unlock();
	BlitTimes += tmr.Check(10000);

}

int ParticleScreen::SetScreenMode(int w, int h, int bpp)
{
	int		iRetVal = 0;

	if(w > 0 && h > 0 && bpp > 0){
#if 1
		DEVMODE			dm;
		DISPLAY_DEVICE	dd;
		int				iDevice = 0;

		dd.cb = sizeof(dd);

		while( EnumDisplayDevices(NULL, iDevice, &dd, 0) != 0 )
		{
			while(1)
			{
				dm.dmSize = sizeof(dm);
				dm.dmBitsPerPel = bpp;
				dm.dmPelsWidth = w;
				dm.dmPelsHeight = h;
				dm.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | (bpp > 0 ? DM_BITSPERPEL : 0);

				if(DISP_CHANGE_SUCCESSFUL == ChangeDisplaySettingsEx(dd.DeviceName, &dm, NULL, CDS_FULLSCREEN, NULL)) 
				{
					iRetVal = 1;
					break;
				}
				if(bpp > 0) bpp = 0;
				else 
				{
					iRetVal = 0;
					break;
				}
			}

			iDevice++;
		}
#else
		HRESULT ddreturn;
		m_lpDD = NULL;
		if(DD_OK != DirectDrawCreate(NULL, &m_lpDD, NULL)) return 0;
		if(DD_OK != (ddreturn = m_lpDD->SetCooperativeLevel(m_hWnd, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN))) return 0;
		if(DD_OK != m_lpDD->SetDisplayMode(w, h, bpp)){
				return 0;
		//	}
		}
#endif
	}
	return iRetVal;
}

void ParticleScreen::ResetScreenmode ()
{
#if 1

	DISPLAY_DEVICE	dd;
	int				iDevice = 0;

	dd.cb = sizeof(dd);

	while( EnumDisplayDevices(NULL, iDevice, &dd, 0) != 0 )
	{
		ChangeDisplaySettingsEx(dd.DeviceName, NULL, NULL, 0, NULL);

		iDevice++;
	}

	//ChangeDisplaySettings(NULL, CDS_FULLSCREEN);
#else
	if(m_lpDD){
		m_lpDD->RestoreDisplayMode();
		m_lpDD->SetCooperativeLevel(m_hWnd, DDSCL_NORMAL);
		m_lpDD->Release();
		m_lpDD = NULL;
	}
#endif
}

int ParticleScreen::PickColor(HWND dlg, PALETTEENTRY *pe)
{
	static COLORREF crCustCol[16];
	static int initialized = 0;
	if(!initialized){
		memset(crCustCol, 0, sizeof(crCustCol));
		initialized = 1;
	}
	if(pe){
		CHOOSECOLOR cc;
		cc.lStructSize = sizeof(cc);
		cc.hwndOwner = dlg;
		cc.hInstance = NULL;
		cc.lpCustColors = crCustCol;
		cc.Flags = CC_RGBINIT | CC_FULLOPEN;
		cc.lCustData = NULL;
		cc.lpfnHook = NULL;
		cc.lpTemplateName = NULL;
		cc.rgbResult = RGB(pe->peRed, pe->peGreen, pe->peBlue);
		ChooseColor(&cc);
		pe->peRed = GetRValue(cc.rgbResult);
		pe->peGreen = GetGValue(cc.rgbResult);
		pe->peBlue = GetBValue(cc.rgbResult);
		return TRUE;
	}
	return FALSE;
}

void ParticleScreen::SpreadPal(PALETTEENTRY *pe1, PALETTEENTRY *pe2, PALETTEENTRY *dst)
{
	if(pe1 && pe2 && dst){
		for(int i = 0; i < 256; i++){
			if(i < 128){
				float t = ((float)i) / 127.0f;
				dst[i].peRed = (int)(((float)pe1->peRed) * t);
				dst[i].peGreen = (int)(((float)pe1->peGreen) * t);
				dst[i].peBlue = (int)(((float)pe1->peBlue) * t);
			}else{
				float t = (float)(i - 127) / 128.0f;
				float it = 1.0f - t;
				dst[i].peRed = (int)((float)pe1->peRed * it + (float)pe2->peRed * t);
				dst[i].peGreen = (int)((float)pe1->peGreen * it + (float)pe2->peGreen * t);
				dst[i].peBlue = (int)((float)pe1->peBlue * it + (float)pe2->peBlue * t);
			}
		}
	}
}

void ParticleScreen::Palette(int ColorScheme)
{
	//Create palette.
	PALETTEENTRY p1, p2;
	switch(ColorScheme){
	case -1 :
		p1 = CustomPE1;
		p2 = CustomPE2;
		break;
	case 0 :
		p1.peRed = 255; p1.peGreen = 128; p1.peBlue = 0;
		p2.peRed = 255; p2.peGreen = 255; p2.peBlue = 0;
		break;
	case 1 :
		p1.peRed = 0; p1.peGreen = 128; p1.peBlue = 255;
		p2.peRed = 0; p2.peGreen = 255; p2.peBlue = 255;
		break;
	case 2 :
		p1.peRed = 64; p1.peGreen = 64; p1.peBlue = 128;
		p2.peRed = 192; p2.peGreen = 192; p2.peBlue = 255;
		break;
	case 3 :
		p1.peRed = 32; p1.peGreen = 128; p1.peBlue = 32;
		p2.peRed = 160; p2.peGreen = 255; p2.peBlue = 160;
		break;
	case 4 :
		p1.peRed = 255; p1.peGreen = 64; p1.peBlue = 64;
		p2.peRed = 255; p2.peGreen = 192; p2.peBlue = 192;
		break;
	case 5 :
		//2687104 (0x00290080)
		//8454143 (0x0080ffff)
		*((ULONG*) (&p1)) = 2687104;
		*((ULONG*) (&p2)) = 8454143;
//		p1.peRed = 128; p1.peGreen = 0; p1.peBlue = 41;
//		p2.peRed = 255; p2.peGreen = 192; p2.peBlue = 128;
		break;
	}
	SpreadPal(&p1, &p2, parent->pe);
	parent->pe[255].peRed = parent->pe[255].peGreen = parent->pe[255].peBlue = 255;
	dib.SetPalette(parent->pe);
	dib.RealizePalette();
}

#include <stdio.h>

// Load the Text from a file
void ParticleScreen::LoadText ()
{
	// If there is a filename
	if (strcmp (parent->QuoteFilename, "\0") )
	{
		FILE *fp;
		fp = fopen (parent->QuoteFilename, "r+");
		if (fp != NULL)
		{
			char buff[512];
			RealQuoteLines = 0;

			// Count the number of lines
			while (!feof (fp)) {
				fgets (buff, 512, fp);
				RealQuoteLines++;
			}
			fseek (fp, NULL, 0);
			this->RealQuotes = (char*) malloc (sizeof (char) * 512 * RealQuoteLines);

			int numLine = 0;
			while (!feof (fp)) {
				fgets (buff, 512, fp);

				for (int i = 0; i < strlen (buff); i++) {
					if (buff[i] == '\n') buff[i] = '\0';
				}

				strcpy (&this->RealQuotes[512*numLine], buff);
				numLine++;
			}

			fclose (fp);
		}
		else
			strcpy (parent->QuoteFilename, "\0");
	}

	// If the Quote filename is blank
	if (!strcmp (parent->QuoteFilename, "\0") )
	{
		// Set to pre-made quotes
		this->RealQuotes = (char*) Quotes;
		this->RealQuoteLines = NUMQUOTES;
	}

}

int ParticleScreen::SetupFont(int height)
{
	memset(&lf, 0, sizeof(lf));
	lf.lfHeight = -(int)height;
	lf.lfWidth = 0;//(int)xsize;
	lf.lfWeight = 550;//(flags & FONT_BOLD ? 700 : 400);
	lf.lfItalic = 0;//(flags & FONT_ITALIC) != 0;
	lf.lfUnderline = 0;//(flags & FONT_UNDERLINE) != 0;
	lf.lfStrikeOut = 0;//(flags & FONT_STRIKEOUT) != 0;
	strcpy(lf.lfFaceName, "Arial");
	if(hfont = CreateFontIndirect(&lf)){	//Make font once...
		return TRUE;
	}
	return FALSE;
}

void ParticleScreen::UnsetupFont()
{
	DeleteObject(hfont);
}

void ParticleScreen::HandleText ()
{
	//
	int iseconds = time(NULL) - FirstUseTime;	//Installed seconds.
	int iminutes = iseconds / 60;
	int ihours = iminutes / 60;
	int idays = ihours / 24;
	int sseconds = (time(NULL) - SecsStart);	//Session Seconds.
	int bseconds = sseconds + TotalSecs;	//Blanked seconds.
	int bminutes = bseconds / 60;
	int bhours = bminutes / 60;

	int newQuote = 0;

	if(parent->Time > parent->LastTime)
	{
#ifdef COUNTQUOTE
		while((rand() >>2) % QuoteSecs != 0);
		int q = (rand() >>8) % RealQuoteLines;
		DrawFont(&RealQuotes[q*512]);
		QuoteCount[q]++;
#else
		if((rand() >>2) % QuoteSecs == 0 && !DisableText)
		{
			this->LastQuote = ((rand() >>8) % RealQuoteLines);
			this->LastQuotePrintTime = 0;

			DrawFont(&RealQuotes[this->LastQuote*512]);
			newQuote = 1;
		}
#endif
	}

	// If we didnt just add a new quote
	if (!newQuote && this->LastQuote != -1) 
	{
		if (this->LastQuotePrintTime < 3 && this->LastQuotePrintTime != 1)
		{
			DrawFont(&RealQuotes[this->LastQuote*512]);
			this->LastQuotePrintTime++;	// Increment to reduce times it will happen
		}
	}
}

int ParticleScreen::DrawFont(const char *text, int mode)
{
	int lineLen = 37;
	int numLine = 0;

	// Cut up the text
	int len, curChar = 0;	char buff[512];
	len = strlen (text);

	static int col_r, col_g, col_b;
	// If normal random mode
	if (mode == 0)
	{
		col_r += rand() % 25;	col_r %= 255;
		col_g += rand() % 25;	col_g %= 255;
		col_b += rand() % 25;	col_b %= 255;
	}
	// Else, if random mode but lighter colors
	else if (mode == 1)
	{
		col_r += rand() % 25;	col_r %= 124;	col_r += 128;
		col_g += rand() % 25;	col_g %= 124;	col_g += 128;
		col_b += rand() % 25;	col_b %= 124;	col_b += 128;
	}

	// If the text is too wide
	if (len > lineLen) 
	{
		// Cycle through the characters
		while (curChar < len) 
		{
			int lastSpace = 0;

			// Find the last space in this line
			for (int i=0; i < lineLen; i++)
			{
				if (text[curChar + i] == ' ')
					lastSpace = i;	// Mark the space point
				if (text[curChar + i] == '\0') {
					lastSpace = i;
					i = lineLen+1;	// Exit loop gracefully
				}
			}
			// If there were no spaces
			if (lastSpace == 0)
				lastSpace = lineLen;

			// Copy over these letters to be drawn
			strncpy (buff, &text[curChar], lastSpace+1);
			buff[lastSpace+1] = 0;
			curChar += lastSpace+1;

			DrawCenteredFont (buff, len/lineLen, numLine, col_r, col_g, col_b);
			numLine++;	// Increment the number of lines so far
		}
	}
	else
		DrawCenteredFont (text, 1, 0, col_r, col_g, col_b);

	return TRUE;
}

int ParticleScreen::DrawCenteredFont(const char *text, int lines, int lineNum, int col_r, int col_g, int col_b)
{
	int x, y;

	// Calc Y line modifier
	int yMod = 0;
	yMod += lines/2 * -30;
	yMod += lineNum * 30;

	if(text && (fontdc = CreateCompatibleDC(NULL))){
		holdfont = (HFONT)SelectObject(fontdc, hfont);
		holdbitmap = (HBITMAP)SelectObject(fontdc, dib.GetHBitmap());
		//Draw text!
		SetTextAlign(fontdc, TA_TOP);
		SetBkMode(fontdc, TRANSPARENT);
//		SetTextColor(fontdc, RGB(255, 255, 255));
//		SetTextColor(fontdc, RGB((text[5]+120)%255, (text[12]+120)%255, (text[15]+120)%255) );
		SetTextColor(fontdc, RGB(col_r, col_g, col_b) );
		GetTextExtentPoint32(fontdc, text, strlen(text), &sz);

		if(1) x = (WIDTH - sz.cx) / 2;
		//	else x = -(int)(xoff * xsize);
//		if(1) y = (HEIGHT - sz.cy) / 2;//ysize) / 2;
		//	else y = -(int)(yoff * ysize);
		if(1) y = (HEIGHT - sz.cy) / 2 + yMod;//ysize) / 2;

		TextOut(fontdc, x, y, text, strlen(text));
		GdiFlush();
		SelectObject(fontdc, holdfont);
		SelectObject(fontdc, holdbitmap);
		DeleteDC(fontdc);
		return TRUE;
	}
	return FALSE;
}

int ParticleScreen::DrawXYFont(const char *text, int x, int y, int col_r, int col_g, int col_b)
{
	if(text && (fontdc = CreateCompatibleDC(NULL))){
		holdfont = (HFONT)SelectObject(fontdc, hfont);
		holdbitmap = (HBITMAP)SelectObject(fontdc, dib.GetHBitmap());
		//Draw text!
		SetTextAlign(fontdc, TA_TOP);
		SetBkMode(fontdc, TRANSPARENT);

		SetTextColor(fontdc, RGB(col_r, col_g, col_b) );
		GetTextExtentPoint32(fontdc, text, strlen(text), &sz);

		// Bound the words
		if (x > (WIDTH - sz.cx))			x = WIDTH - sz.cx - 10;
		if (y > (HEIGHT - sz.cy))			y = WIDTH - sz.cy - 10;

		TextOut(fontdc, x, y, text, strlen(text));
		GdiFlush();
		SelectObject(fontdc, holdfont);
		SelectObject(fontdc, holdbitmap);
		DeleteDC(fontdc);
		return TRUE;
	}
	return FALSE;
}

void ParticleScreen::DrawParticles ()
{
	//Draw particles.
	unsigned char *data, *data2;
	int pitch;
	int d, e, dir, x, y, dx, dy;
	data = (unsigned char*)dib.Data();//bdesc.data;
	pitch = dib.Pitch();//bdesc.pitch;
	unsigned char *bdescdata = dib.Data();
	int bdescpitch = dib.Pitch();
	int bdescwidth = dib.Width();
	int bdescheight = dib.Height();
	float adx, ady, dist, dist2, ang, angd;

//

	for(int i = 0; i < parent->particle.nParticles; i++)
	{
		//
		if(parent->particle.AltColor)
		{
			parent->p[i].color -= 1;
			if(parent->p[i].color <= 128) parent->p[i].color += 32;
		}
		//
		parent->p[i].lx = parent->p[i].x;
		parent->p[i].ly = parent->p[i].y;
		if(parent->p[i].attract & ATTRACT_GRAVITY)
		{
			parent->p[i].dx += parent->particle.xgrav;
			parent->p[i].dy += parent->particle.ygrav;
		}
		if(parent->p[i].attract & ATTRACT_ANGLE)
		{
			adx = parent->p[i].ax - parent->p[i].x;
			ady = parent->p[i].ay - parent->p[i].y;
			dist = sqrt(adx * adx + ady * ady);
			dist2 = sqrt(parent->p[i].dx * parent->p[i].dx + parent->p[i].dy * parent->p[i].dy);
			if(dist == 0.0f) dist = 2.0f;
			if(dist2 == 0.0f) dist2 = 2.0f;

			if(parent->p[i].dx == 0.0f && parent->p[i].dy == 0.0f)
			{
				ang = parent->particle.IdentityAngle;
			}
			else
			{
				ang = atan2(parent->p[i].dy, parent->p[i].dx);
			}
			//
			if(adx == 0.0f && ady == 0.0f)
			{
				angd = parent->particle.IdentityAngle - ang;
			}
			else
			{
				angd = atan2(ady, adx) - ang;
			}
			//
			if(angd > 3.14159f) angd -= 3.14159f * 2.0f;
			if(angd < -3.14159f) angd += 3.14159f * 2.0f;
			ang += angd * 0.05f;

			parent->p[i].dx = cos(ang) * dist2;
			parent->p[i].dy = sin(ang) * dist2;
		}
		parent->p[i].x += parent->p[i].dx;
		parent->p[i].y += parent->p[i].dy;
		//Edge collisions.
		if(parent->p[i].x < XOFF)
		{
			parent->p[i].x = (float)XOFF;
			parent->p[i].dx = fabs(parent->p[i].dx) * BOUNCE;
			parent->p[i].dy += frand(KICK_STRENGTH);
			if(parent->particle.AltColor) parent->p[i].color = __min(254, parent->p[i].color + 32);

		}
		if(parent->p[i].x >= WIDTH - XOFF)
		{
			parent->p[i].x = (float)(WIDTH - XOFF - 1);
			parent->p[i].dx = -fabs(parent->p[i].dx) * BOUNCE;
			parent->p[i].dy += frand(KICK_STRENGTH);
			if(parent->particle.AltColor) parent->p[i].color = __min(254, parent->p[i].color + 32);
		}
		if(parent->p[i].y < YOFF)
		{
			parent->p[i].y = (float)YOFF;
			parent->p[i].dy = fabs(parent->p[i].dy) * BOUNCE;
			parent->p[i].dx += frand(KICK_STRENGTH);
			if(parent->particle.AltColor) parent->p[i].color = __min(254, parent->p[i].color + 32);
		}
		if(parent->p[i].y >= HEIGHT - YOFF)
		{
			parent->p[i].y = (float)(HEIGHT - YOFF - 1);
			parent->p[i].dy = -fabs(parent->p[i].dy) * BOUNCE;
			parent->p[i].dx += frand(KICK_STRENGTH);
			if(parent->particle.AltColor) parent->p[i].color = __min(254, parent->p[i].color + 32);
		}
		x = (int)parent->p[i].lx;
		y = (int)parent->p[i].ly;
		dx = (int)parent->p[i].x - x;
		dy = (int)parent->p[i].y - y;
		//Make sure LAST coords are inside bounds too.
		if(x >= XOFF && y >= YOFF && x < WIDTH - XOFF && y < HEIGHT - YOFF)
		{
			//Bresenham style line drawer.
			//X major.
			if(abs(dx) > abs(dy))
			{
				if(dx < 0)
				{
					x += dx; y += dy;
					dx = -dx; dy = -dy;
				}

				int j;
				d = j = -dx; e = abs(dy);

				if(UseTrueColor)
				{
					data2 = data + (x <<2) + y * pitch;// + (i % 3);
				}
				else
				{
					data2 = data + x + y * pitch;
				}

				if(dy < 0) dir = -pitch; else dir = pitch;
				unsigned char c = parent->p[i].color, hc = c >> 1;
				const unsigned char tc[4] = {parent->p[i].blue, parent->p[i].green, parent->p[i].red, 0};
				//Draw start and end pixels.
				while(dx >= 0)
				{					
					if(UseTrueColor)
					{
						for(int px = 0; px < 3; px++)
						{
							*(data2 - pitch + px) = __max(*(data2 - pitch + px), tc[px] >>1);
							*(data2 + px) = __max(*(data2 + px), tc[px]);
							*(data2 + pitch + px) = __max(*(data2 + pitch + px), tc[px] >>1);
						}
					}
					else
					{
						*(data2 - pitch) = __max(*(data2 - pitch), hc);
						*(data2) = __max(*(data2), c);
						*(data2 + pitch) = __max(*(data2 + pitch), hc);
					}
					d += e;
					if(d >= 0){
						d += j;
						data2 += dir;
					}
					if(UseTrueColor) data2 += 4;
					else data2++;
					dx--;
				}
			}
			else
			{	//Y major.
				if(dy < 0)
				{
					x += dx; y += dy;
					dx = -dx; dy = -dy;
				}
// GH-CHANGE
				int j;
				d = j = -dy; e = abs(dx);
				if(UseTrueColor)
				{
					data2 = data + (x <<2) + y * pitch;// + (i % 3);
				}
				else
				{
					data2 = data + x + y * pitch;
				}
				if(dx < 0)
				{
					if(UseTrueColor) dir = -4;
					else dir = -1;
				}
				else
				{
					if(UseTrueColor) dir = 4;
					else dir = 1;
				}
				unsigned char c = parent->p[i].color, hc = c >> 1;
				const unsigned char tc[4] = {parent->p[i].blue, parent->p[i].green, parent->p[i].red, 0};
				//Draw start and end pixels.
				while(dy >= 0)
				{
					if(UseTrueColor)
					{
						for(int px = 0; px < 3; px++)
						{
							*(data2 - dir + px) = __max(*(data2 - dir + px), tc[px] >>1);
							*(data2 + px) = __max(*(data2 + px), tc[px]);
							*(data2 + dir + px) = __max(*(data2 + dir + px), tc[px] >>1);
						}
					}
					else
					{
						*(data2 - dir) = __max(*(data2 - dir), hc);
						*(data2) = __max(*(data2), c);
						*(data2 + dir) = __max(*(data2 + dir), hc);
					}
					d += e;
					if(d >= 0)
					{
						d += j;
						data2 += dir;
					}
					data2 += pitch;
					dy--;
				}
			}
		}
	}
}

void ParticleScreen::DrawWall ()
{
}

void ParticleScreen::SeedWall ()
{
	//Draw particles.
	int x, y;
	unsigned char *bdescdata = dib.Data();
	int bdescpitch = dib.Pitch();
	int bdescwidth = dib.Width();
	int bdescheight = dib.Height();

	int pitchBit = dib.Pitch() / dib.Width();

//

	if(parent->particle.BurnDown)
	{
		parent->tdata = (unsigned char*)bdescdata + (BYOFF) * bdescpitch;
		memset((unsigned char*)bdescdata + (HEIGHT - BYOFF) * bdescpitch, 0, bdescwidth * pitchBit);
	}
	else
	{
		parent->tdata = (unsigned char*)bdescdata + (HEIGHT - BYOFF) * bdescpitch;
		memset((unsigned char*)bdescdata + (BYOFF) * bdescpitch, 0, bdescwidth * pitchBit);
	}
	
	// If the Fire isnt Disabled
	if(!DisableFire)
	{
		static float flamenoisey = 0.0f;
		if((parent->particle.WallStyle == STYLE_WALL_NORMAL && parent->particle.NoiseBurn == 1) || parent->particle.WallStyle == STYLE_WALL_SMOKE)
		{
			// Smoke wall
			flamenoisey += FlameSpeed;
			for(x = BXOFF*pitchBit; x < WIDTH*pitchBit - BXOFF*pitchBit; x++)
			{
				*(parent->tdata + x) = 64 + (unsigned char)(basis->Noise((float)x * FlameSpeed, flamenoisey, 2) * 191.0);
			}
		}
		else if((parent->particle.WallStyle == STYLE_WALL_NORMAL && parent->particle.NoiseBurn == -1) || parent->particle.WallStyle == STYLE_WALL_NONE)
		{
			// No wall
			for(x = BXOFF*pitchBit; x < WIDTH*pitchBit - BXOFF*pitchBit; x++)
			{
				*(parent->tdata + x) = 0;
			}
		}
		else if (parent->particle.WallStyle != STYLE_WALL_NONE)
		{
			// Rainbow wall
			y = rand() & 255;
			for(x = BXOFF*pitchBit; x < WIDTH*pitchBit - BXOFF*pitchBit; x++)
			{
				if(*(parent->tdata + x))
				{
					y = *(parent->tdata + x);
				}
				y += rand() % 65 - 32;
				if(y < 0) y = 0;
				if(y > 254) y = 254;
				*(parent->tdata + x) = y;
			}
		}
	} // End, If the Fire isnt Disabled
}


