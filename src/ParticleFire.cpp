/*
	Burning particles demo, by Seumas McNally, Longbow Digital Arts, 1997.
	Demonstration of windowed/fullscreen BackBuffer display class.
	Original buffer-burning code thanks to Michael Welch.

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

1.0 - Initial version with colored particles and no fire.
1.1 - Buffer burning code added.
1.2 - Removed rand()s for per-pixel burning calculations, greatly enhancing
	speed with little visual degredation.  Added bounce factor, so particles
	eventually lose energy.  In windowed mode, app now updates even if it
	isn't the active window.  Added frand() function to generate floating
	point random numbers, making particle kick at bounce more natural.  Made
	explosions circular and of random intensity.  Added Comet effect when
	enter is pressed, which sends all particles off on top of each other.
	Added Emitter effect with backspace, dropping all particles from the
	center of the screen one after another (well, a few after another few).
	Added randomly occuring particle effects to liven things up a bit.
	(I know this would make a great screensaver...  :)
	Added multiple color schemes.
1.3 - June 3rd 1998:
	Giving it an overhaul.  Bresenham line drawer, blazingly faster buffer
	burner, 640x480 flies along.  Plan to add more types of particle control,
	such as swarming, 3D object points, fireworks, etc.

ParticleFire - November 12th 1998.
	Re-done as a Screen Saver!

2.0 - June 8th 2000:
	Converting to classes and into separate files for different styles for
	easier reading and augmenting.  -Geoff Howland

2.1.3 - May 11th, 2004
	Added support for multi-monitor systems. -Rob McConnell

2.1.4 - August 2016:
	Fixed a bug that causes the screen to blank on exit
	Removed resolution settings and screen mode changes for better compatability.
	Converted to freeware
*/

#include <math.h>
#include <time.h>
#include <stdio.h>
#include <windows.h>
#include <commctrl.h>
//#include <BackBuffer.h>
#include <Cdib.h>
#include "ParticleFire.h"
#include <Timer.h>
#include <Basis.h>
#include <Reg.h>
#include <scrnsave.h>
#include "resource.h"
#include <CStr.h>

// GH-Compiled list of defines
#include "defines.h"

HWND			hwnd, dwnd;
int Frames = 0;
float CycleT = 1.1f;

int				ActiveApp;
time_t			TimeStart, Time, LastTime;
int Hack8Bit = FALSE;
int CycleFrameDelay = 10;

// Make my Container Class - GH-CHANGE
#include "ParticleContainer.hpp"
ParticleContainer partFire;


//#include <ddraw.h>
//LPDIRECTDRAW m_lpDD = NULL;
HWND m_hWnd = NULL;

BOOL WINAPI RegisterDialogClasses(HANDLE hInst){
	return TRUE;
}

int Realized = FALSE;
int BlankedSecs = 0;
long int FirstUseTime = 0;
long int SecsStart = 0;

PALETTEENTRY tpe[256];
TEXTMETRIC tm;

//#define COUNTQUOTE

#ifdef COUNTQUOTE
int QuoteCount[1000];
#endif

// For testing out stuff or gathering errors
void error_print (char *buff)
{
	FILE *fp;
	fp = fopen ("C:\\error.txt", "w");
	fprintf (fp, "%s", buff);
	fclose (fp);
}


LRESULT CALLBACK ScreenSaverProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam){
	//
	m_hWnd = hwnd;
	//
//	PAINTSTRUCT ps;
	HDC hdc;
//	int ret = -1;	//Do DefScrProc
	switch(iMsg){
		case WM_CREATE :
			//
#ifdef COUNTQUOTE
			memset(QuoteCount, 0, sizeof(QuoteCount));
#endif
			//
			partFire.registry.LoadOpts();
			//
		//	InitStuff(hwnd);
		//	SetTimer(hwnd, 1, 50, NULL);
			SetTimer(hwnd, 1, 40, NULL);
		//	PostMessage(hwnd, WM_INFINITE, 0, 0);
			//
			return 0;
		//	ret = 0;
		//	break;
		case WM_ACTIVATE :
			return 0;
			break;
		case WM_INFINITE :
		//	PostMessage(hwnd, WM_INFINITE, 0, 0);
			//
		case WM_TIMER :
		//	if(Hack8Bit && !Realized){
		//		hdc = GetDC(NULL);
		//		SetSystemPaletteUse(hdc, SYSPAL_NOSTATIC);
		//		ReleaseDC(NULL, hdc);
		//	}
			if(Frames == 10) partFire.screen.InitScreen (hwnd);
			if(Frames < 10){
				Frames++;
				return 1;
			}
			//
			if(!Realized){
			//	Palette(partFire.screen.CustomScheme ? -1 : partFire.screen.ColorScheme);
				partFire.screen.Palette(partFire.screen.ColorScheme);
				partFire.screen.dib.RealizePalette();
				partFire.particle.Explode = TRUE;
			}
			//
			DoFrame();
			//
			if(!Realized){
				partFire.screen.Palette(partFire.screen.ColorScheme);
				partFire.screen.dib.RealizePalette();
				partFire.particle.Explode = TRUE;
			}
			//
			Frames++;
			if(partFire.screen.CycleColors && Frames % __max(1, (partFire.screen.UseTrueColor ? CycleFrameDelay / 4 : CycleFrameDelay)) == 0){
				CycleT += 0.01f;
				if(CycleT >= 1.0f){
					memcpy(partFire.cf, partFire.pe, sizeof(partFire.pe));	//Put current colors in from spot.
					//
					if(partFire.screen.RandomColor) partFire.screen.ColorScheme = rand() % (NUMSCHEMES + 1) - 1;
					else partFire.screen.ColorScheme++;
					if(partFire.screen.ColorScheme >= NUMSCHEMES) partFire.screen.ColorScheme = -1;
					//
					partFire.screen.Palette(partFire.screen.ColorScheme);//rand() % (NUMSCHEMES + 1) - 1);
					memcpy(partFire.ct, partFire.pe, sizeof(partFire.pe));	//Destination palette.
					CycleT = 0.0f;
				}
				float t = CycleT, it = 1.0f - t;
				for(int i = 1; i < 255; i++){
					partFire.pe[i].peRed = (int)((float)partFire.cf[i].peRed * it + (float)partFire.ct[i].peRed * t);
					partFire.pe[i].peGreen = (int)((float)partFire.cf[i].peGreen * it + (float)partFire.ct[i].peGreen * t);
					partFire.pe[i].peBlue = (int)((float)partFire.cf[i].peBlue * it + (float)partFire.ct[i].peBlue * t);
				}
				if(partFire.screen.UseTrueColor == 0){
					partFire.screen.dib.SetPalette(partFire.pe);
					partFire.screen.dib.RealizePalette();
				}
			}
			//
			Realized = TRUE;
		//	ret = 1;
		//	break;
			return TRUE;
		case WM_DESTROY :
			//
			BlankedSecs = time(NULL) - SecsStart;
			partFire.screen.TotalSecs += BlankedSecs;
// GH-CHANGED
			partFire.registry.RegistryWrite ("SecondsBlanked", partFire.screen.TotalSecs);
//			REG.WriteDword("SecondsBlanked", partFire.screen.TotalSecs);

			//
		//	if(Hack8Bit){
		//		hdc = GetDC(NULL);
		//		SetSystemPaletteUse(hdc, SYSPAL_STATIC);
		//		ReleaseDC(NULL, hdc);
		//		PostMessage(HWND_BROADCAST, WM_SYSCOLORCHANGE, 0, 0);
		//	}
			//
			partFire.screen.dib.DeleteHBitmap();
			KillTimer(hwnd, 1);
			//
			partFire.screen.UnsetupFont();
			//
#ifdef COUNTQUOTE
			FILE *f;
			if((f = fopen("C:\qcnt.txt", "w"))){
				for(int i = 0; i < 60; i++){
					fprintf(f, "%d\n", QuoteCount[i]);
				}
				fclose(f);
			}
#endif

			//
		//	PostQuitMessage(0);
			return 0;
			
	}//Switch
//	return DefWindowProc(hwnd, iMsg, wParam, lParam);
	//
//	MSG tm;
//	if(PeekMessage(&tm, hwnd, NULL, NULL, PM_NOREMOVE) == 0){
//		PostMessage(hwnd, WM_INFINITE, 0, 0);
//	}
//	if(ret != -1){
//		return ret;
//	}else{
//	}
//	if(IgnoreMouse){
//		return 0;
//		return DefWindowProc(hwnd, iMsg, wParam, lParam);
//	}else{
		return DefScreenSaverProc(hwnd, iMsg, wParam, lParam);
//	}
}//WndProc

bool BrowseForFile ()
{
	OPENFILENAME ofn;       // common dialog box structure
	char szFile[260];       // buffer for file name
	strcpy (szFile, "\0");
//	HWND hwnd;              // owner window
	HANDLE hf;              // file handle

	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = m_hWnd;//hwnd;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
//	ofn.lpstrFilter = "All\0*.*\0Text\0*.TXT\0";
	ofn.lpstrFilter = "Text\0*.TXT\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	// Display the Open dialog box. 

	int RESULT;
	RESULT = GetOpenFileName(&ofn);
	if (RESULT == TRUE)
		strcpy (partFire.QuoteFilename, ofn.lpstrFile);

/*	if (GetOpenFileName(&ofn)==TRUE) 
		hf = CreateFile(ofn.lpstrFile, GENERIC_READ,
			0, (LPSECURITY_ATTRIBUTES) NULL,
			OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
			(HANDLE) NULL);*/

	return true;
}

BOOL CALLBACK ScreenSaverConfigureDialog(HWND dlgwnd, UINT iMsg, WPARAM wParam, LPARAM lParam){
	BOOL error;
	HWND ctrl;
	LPDRAWITEMSTRUCT lpdis;
	HBRUSH brush;
//	POINT pt;
	RECT crc;
	int id, code, i;
	switch(iMsg){
		case WM_INITDIALOG :
			m_hWnd = dlgwnd;
			//
			partFire.registry.LoadOpts();
			SendDlgItemMessage(dlgwnd, IDC_CHECKRANDCOL, BM_SETCHECK, (partFire.screen.RandomColor ? BM_SETCHECK : FALSE), 0);

			for(i = 0; i < NUMSCHEMES + 1; i++){
				if(i == 0) SendDlgItemMessage(dlgwnd, IDC_COMBOCOLOR, CB_ADDSTRING, 0, (LONG)((char*)"Custom"));
				else SendDlgItemMessage(dlgwnd, IDC_COMBOCOLOR, CB_ADDSTRING, 0, (LONG)&ColorName[i - 1][0]);
			}
			SendDlgItemMessage(dlgwnd, IDC_COMBOCOLOR, CB_SETCURSEL, partFire.screen.CustomScheme ? 0 : partFire.screen.ColorScheme + 1, 0);
			EnableWindow(GetDlgItem(dlgwnd, IDC_COMBOCOLOR), !partFire.screen.RandomColor);
			//
//			SetDlgItemInt(dlgwnd, IDC_EDITBURNFADE, partFire.screen.BURNFADE, FALSE);
			SendDlgItemMessage(dlgwnd, IDC_CHECKCYCLE, BM_SETCHECK, (partFire.screen.CycleColors ? BM_SETCHECK : FALSE), 0);
			SendDlgItemMessage(dlgwnd, IDC_CHECKTEXT, BM_SETCHECK, (partFire.screen.DisableText ? BM_SETCHECK : FALSE), 0);
			//
			for(i = 0; i < NUMSTYLES; i++){
				SendDlgItemMessage(dlgwnd, IDC_COMBOSTYLE, CB_ADDSTRING, 0, (LONG)&StyleName[i][0]);
			}
			SendDlgItemMessage(dlgwnd, IDC_COMBOSTYLE, CB_SETCURSEL, partFire.particle.ParticleStyle, 0);
			//
			for(i = 0; i < NUMSTYLEWALLS; i++){
				SendDlgItemMessage(dlgwnd, IDC_COMBOSTYLE2, CB_ADDSTRING, 0, (LONG)&StyleWallName[i][0]);
			}
			SendDlgItemMessage(dlgwnd, IDC_COMBOSTYLE2, CB_SETCURSEL, partFire.particle.WallStyle, 0);
			//
			SendDlgItemMessage(dlgwnd, IDC_CHECKMULTI, BM_SETCHECK, (partFire.screen.UseTrueColor ? BM_SETCHECK : FALSE), 0);
			//
			SendDlgItemMessage(dlgwnd, IDC_QUOTEFILENAME, WM_SETTEXT, 0, (long) partFire.QuoteFilename);
			//
			//
			{	// Quote Text speed
			SendDlgItemMessage(dlgwnd, IDC_SLIDER_TEXT_SPEED, TBM_SETRANGE, TRUE, MAKELONG (1,20));
			int slider_position = partFire.screen.QuoteSecs / 2;
			SendDlgItemMessage(dlgwnd, IDC_SLIDER_TEXT_SPEED, TBM_SETPOS, TRUE, (long) slider_position);
			}
			{	// Burn Fade speed
			SendDlgItemMessage(dlgwnd, IDC_SLIDER_FADE_SPEED, TBM_SETRANGE, TRUE, MAKELONG (1,20));
			int slider_position = partFire.screen.BURNFADE;
			SendDlgItemMessage(dlgwnd, IDC_SLIDER_FADE_SPEED, TBM_SETPOS, TRUE, (long) slider_position);
			}
			{	// Number of particles
			SendDlgItemMessage(dlgwnd, IDC_SLIDER_PARTICLE_NUM, TBM_SETRANGE, TRUE, MAKELONG (1,20));
			int slider_position = partFire.particle.nParticles / 500;
			SendDlgItemMessage(dlgwnd, IDC_SLIDER_PARTICLE_NUM, TBM_SETPOS, TRUE, (long) slider_position);
			}
			{	// Event Change speed
			SendDlgItemMessage(dlgwnd, IDC_SLIDER_EVENT_SPEED, TBM_SETRANGE, TRUE, MAKELONG (1,20));
			int slider_position = partFire.particle.RANDEFFECT / 5;
			SendDlgItemMessage(dlgwnd, IDC_SLIDER_EVENT_SPEED, TBM_SETPOS, TRUE, (long) slider_position);
			}
			{	// Gravity Change speed
			SendDlgItemMessage(dlgwnd, IDC_SLIDER_GRAVITY_SPEED, TBM_SETRANGE, TRUE, MAKELONG (1,20));
			int slider_position = partFire.particle.GRAV_TIME / 10;
			SendDlgItemMessage(dlgwnd, IDC_SLIDER_GRAVITY_SPEED, TBM_SETPOS, TRUE, (long) slider_position);
			}
			return TRUE;
		case WM_HSCROLL:
			ctrl = (HWND) lParam;
			id = LOWORD(wParam);
			code = HIWORD(wParam);

			int slider_position;
			// Quote text speed
			slider_position = SendDlgItemMessage(dlgwnd, IDC_SLIDER_TEXT_SPEED, TBM_GETPOS, 0, 0);
			partFire.screen.QuoteSecs = slider_position * 2;
			// Fade out speed
			slider_position = SendDlgItemMessage(dlgwnd, IDC_SLIDER_FADE_SPEED, TBM_GETPOS, 0, 0);
			partFire.screen.BURNFADE = slider_position;
			// Number of particles
			slider_position = SendDlgItemMessage(dlgwnd, IDC_SLIDER_PARTICLE_NUM , TBM_GETPOS, 0, 0);
			partFire.particle.nParticles = slider_position * 500;
			// Event change speed
			slider_position = SendDlgItemMessage(dlgwnd, IDC_SLIDER_EVENT_SPEED , TBM_GETPOS, 0, 0);
			partFire.particle.RANDEFFECT = slider_position * 5;
			// Gravity change speed
			slider_position = SendDlgItemMessage(dlgwnd, IDC_SLIDER_GRAVITY_SPEED, TBM_GETPOS, 0, 0);
			partFire.particle.GRAV_TIME = slider_position * 10;
			return TRUE;
			break;
		case WM_COMMAND :
			ctrl = (HWND) lParam;
			id = LOWORD(wParam);
			code = HIWORD(wParam);
			if(code == BN_CLICKED){
				switch(id){	//Switch on ID when we've been clicked.
				case IDC_BUTTONCOLOR1 :
					partFire.screen.PickColor(dlgwnd, &partFire.screen.CustomPE1);
					InvalidateRect(ctrl, NULL, FALSE);
					return TRUE;
				case IDC_BUTTONCOLOR2 :
					partFire.screen.PickColor(dlgwnd, &partFire.screen.CustomPE2);
					InvalidateRect(ctrl, NULL, FALSE);
					return TRUE;
				case IDC_CHECKTEXT :
					partFire.screen.DisableText = (SendMessage(ctrl, BM_GETCHECK, 0, 0) == BST_CHECKED);
					return TRUE;
				case IDC_CHECKCYCLE :
					partFire.screen.CycleColors = (SendMessage(ctrl, BM_GETCHECK, 0, 0) == BST_CHECKED);
					return TRUE;
				case IDC_CHECKRANDCOL :
					partFire.screen.RandomColor = (SendMessage(ctrl, BM_GETCHECK, 0, 0) == BST_CHECKED);
					EnableWindow(GetDlgItem(dlgwnd, IDC_COMBOCOLOR), !partFire.screen.RandomColor);
					return TRUE;
				case IDC_CHECKMULTI :
					partFire.screen.UseTrueColor = (SendMessage(ctrl, BM_GETCHECK, 0, 0) == BST_CHECKED);
				//	EnableWindow(GetDlgItem(dlgwnd, IDC_COMBOCOLOR), !partFire.screen.RandomColor);
					return TRUE;
				case IDC_BUTTONLDA :
					ShellExecute(NULL, "open", "http://www.longbowgames.com/", NULL, NULL, NULL);
					break;
				case IDOK :
					partFire.registry.SaveOpts();
					EndDialog(dlgwnd, TRUE);
			//		DestroyWindow(dlgwnd);
					return TRUE;
				case IDCANCEL :
					EndDialog(dlgwnd, FALSE);
					return TRUE;
				case IDC_BUTTON_FIND_QUOTE_FILENAME:
//					strcpy (partFire.QuoteFilename, "Filename Here");
					BrowseForFile ();
					SendDlgItemMessage(dlgwnd, IDC_QUOTEFILENAME, WM_SETTEXT, 0, (long) partFire.QuoteFilename);
					return TRUE;
				}
			}
			else
			{
				switch(id){	//Switch on ID in all other cases.
				case IDC_COMBOCOLOR :
					if(code == CBN_SELCHANGE){
						i = SendDlgItemMessage(dlgwnd, IDC_COMBOCOLOR, CB_GETCURSEL, 0, 0);
						if(i > 0){
							partFire.screen.ColorScheme = i - 1;
							partFire.screen.CustomScheme = 0;
						}else{
							partFire.screen.CustomScheme = 1;
						}
					//	Palette(partFire.screen.ColorScheme);
					}
					return TRUE;
				case IDC_COMBOSTYLE :
					if(code == CBN_SELCHANGE){
						partFire.particle.ParticleStyle = SendDlgItemMessage(dlgwnd, IDC_COMBOSTYLE, CB_GETCURSEL, 0, 0);
					}
					return TRUE;
				case IDC_COMBOSTYLE2 :
					if(code == CBN_SELCHANGE){
						partFire.particle.WallStyle = SendDlgItemMessage(dlgwnd, IDC_COMBOSTYLE2, CB_GETCURSEL, 0, 0);
					}
					return TRUE;
				}
			}
			break;
		case WM_DRAWITEM :
			id = (UINT)wParam;
			lpdis = (LPDRAWITEMSTRUCT)lParam;
			GetClientRect(lpdis->hwndItem, &crc);
			switch(id){
				case IDC_BUTTONCOLOR1 :
					brush = CreateSolidBrush(RGB(partFire.screen.CustomPE1.peRed, partFire.screen.CustomPE1.peGreen, partFire.screen.CustomPE1.peBlue));
					FillRect(lpdis->hDC, &lpdis->rcItem, brush);
					DeleteObject(brush);
					DrawEdge(lpdis->hDC, &crc, EDGE_RAISED, BF_RECT);
{char buff[255];
sprintf (buff, "%x\n%x", *((ULONG*)&partFire.screen.CustomPE1), *((ULONG*)&partFire.screen.CustomPE2) );
//error_print (buff);
}
					return TRUE;
				case IDC_BUTTONCOLOR2 :
					brush = CreateSolidBrush(RGB(partFire.screen.CustomPE2.peRed, partFire.screen.CustomPE2.peGreen, partFire.screen.CustomPE2.peBlue));
					FillRect(lpdis->hDC, &lpdis->rcItem, brush);
					DeleteObject(brush);
					DrawEdge(lpdis->hDC, &crc, EDGE_RAISED, BF_RECT);
					return TRUE;
			}
			return TRUE;
		case WM_CLOSE :
			EndDialog(dlgwnd, FALSE);
			return TRUE;
	}
	return FALSE;
}

//Returns a random double between -range and range.
float frand(float range){
	return ((float)((rand() % 10000) - 5000) * range) / 5000.0f;
}
/*
void CubeSide(int firstparticle, int ppe,
			  float a, double b, double c,
			  float x1, double y1, double z1,
			  float x2, double y2, double z2,
			  float x3, double y3, double z3,
			  float x4, double y4, double z4){
	//TODO: Rotate points here.
	int i, j, pt = firstparticle;
	double jxoff = (x2 - x1) / ppe, jyoff = (y2 - y1) / ppe;
	double ixoff = (x3 - x1) / ppe, iyoff = (y3 - y1) / ppe;
	double xstart, ystart;
	for(i = 0; i < ppe; i++){
		xstart = x1 + i * ixoff;
		ystart = y1 + i * iyoff;
		for(j = 0; j < ppe; j++){
			p[pt].ax = xstart + jxoff * j;
			p[pt].ay = ystart + jyoff * j;
			pt++;
		}
	}
}
*/

// Include the Registration and Quotation text here, to make the file more readable
//#include "textStuff.h"

//int RegLine = 0;
//int LastTextTime = 0;


#define PRELINES 5

void DoFrame()
{
	// Handle particle container stuff
	partFire.Frame ();

}

