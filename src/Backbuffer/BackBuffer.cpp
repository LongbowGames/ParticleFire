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

#include "BackBuffer.h"

void TrueColorFormat::SetMasks(unsigned int bpp, unsigned int rm, unsigned int gm, unsigned int bm){
	BPP = bpp;
	RedMask = rm;
	RedMaskOff = RedMaskLen = 0;
	while((rm & 1) == 0 && rm){ RedMaskOff++; rm >>= 1; };
	while((rm & 1) != 0){ RedMaskLen++; rm >>= 1; };
	RedQuant = 8 - RedMaskLen;
	GreenMask = gm;
	GreenMaskOff = GreenMaskLen = 0;
	while((gm & 1) == 0 && gm){ GreenMaskOff++; gm >>= 1; };
	while((gm & 1) != 0){ GreenMaskLen++; gm >>= 1; };
	GreenQuant = 8 - GreenMaskLen;
	BlueMask = bm;
	BlueMaskOff = BlueMaskLen = 0;
	while((bm & 1) == 0 && bm){ BlueMaskOff++; bm >>= 1; };
	while((bm & 1) != 0){ BlueMaskLen++; bm >>= 1; };
	BlueQuant = 8 - BlueMaskLen;
}
int TrueColorFormat::MakeLookup(unsigned int *table, PALETTEENTRY *pe, int numcols){
	if(table && pe && numcols >= 0){
		for(int i = 0; i < numcols; i++){
			table[i] = (unsigned int)PackColor(pe[i].peRed, pe[i].peGreen, pe[i].peBlue);
		}
		return TRUE;
	}
	return FALSE;
}
int TrueColorFormat::MakeLookup(unsigned short *table, PALETTEENTRY *pe, int numcols){
	if(table && pe && numcols >= 0){
		for(int i = 0; i < numcols; i++){
			table[i] = (unsigned short)PackColor(pe[i].peRed, pe[i].peGreen, pe[i].peBlue);
		}
		return TRUE;
	}
	return FALSE;
}

BackBuffer::BackBuffer(){
	m_InitDIB = FALSE;	//Keep track of what's been initialized properly.
	m_InitDDRaw = FALSE;
	m_fullscreen2 = FALSE;	//True if DirectDraw SetDisplayMode is in effect.
	m_hwnd = NULL;
	for(int i = 0; i < 256; i++){
		m_pe[i].peRed = 0;
		m_pe[i].peGreen = 0;
		m_pe[i].peBlue = 0;
	}
	m_BufDesc.bytespixel = 0;
	m_BufDesc.width = 0;
	m_BufDesc.height = 0;
	m_BufDesc.pitch = 0;
	m_BufDesc.data = NULL;
	m_Locks = 0;
	m_w = m_h = m_x = m_y = m_realw = m_realh = 0;
	centering = TRUE;
	pointer = FALSE;
	ndirty = 0;
}
BackBuffer::~BackBuffer(){
	Destroy();
}
int BackBuffer::Centering(int flag){
	int ret = centering;
	centering = flag;
	return ret;
}
int BackBuffer::Pointer(int flag){
	int ret = pointer;
	pointer = flag;
	return ret;
}
int BackBuffer::InitWindow(int w, int h, const char *Name, HINSTANCE hInst, HWND *phWnd, LRESULT (CALLBACK *WProc)(HWND, UINT, WPARAM, LPARAM), DWORD Icon){
	if(m_Locks > 0) return FALSE;
	m_wc.cbSize			= sizeof(m_wc);
	m_wc.style			= CS_HREDRAW | CS_VREDRAW;
	m_wc.lpfnWndProc	= WProc;
	m_wc.cbClsExtra		= 0;
	m_wc.cbWndExtra		= 0;
	m_wc.hInstance		= hInst;
	m_wc.hCursor		= LoadCursor(NULL, IDC_ARROW);
	m_wc.hbrBackground	= NULL;//(HBRUSH)GetStockObject(WHITE_BRUSH);
	m_wc.lpszMenuName	= NULL;
	m_wc.lpszClassName	= Name;
	if(Icon == NULL){
		m_wc.hIcon			= LoadIcon(NULL, IDI_APPLICATION);
		m_wc.hIconSm		= LoadIcon(NULL, IDI_APPLICATION);
	}else{
		m_wc.hIcon			= LoadIcon(hInst, MAKEINTRESOURCE(Icon));
		m_wc.hIconSm		= LoadIcon(hInst, MAKEINTRESOURCE(Icon));
	}
	RegisterClassEx(&m_wc);

	*phWnd = m_hwnd = CreateWindowEx(0, Name, Name, WS_OVERLAPPEDWINDOW, 0, 0,
							  w, h, NULL, NULL, hInst, NULL);
	if(!m_hwnd) return Destroy();

	SetWindowSize(w, h);
	ShowWindow(m_hwnd, SW_SHOWNORMAL);
	SetFocus(m_hwnd);

	return TRUE;
}
int BackBuffer::SetBufferMode(int w, int h, int bpp, BOOL fscreen, TrueColorFormat *tcf){
	//For true-color, return a copy of the TrueColorFormat in tcf.
	if(m_Locks > 0 || m_hwnd == NULL || w <= 0 || w % 4 != 0 || h <= 0 || bpp <= 0) return FALSE;
	FreeDIB();
	if(fscreen){
		m_fullscreen2 = TRUE;
	}
	else
	{
		m_fullscreen2 = FALSE;
	}

	if(InitDIB(w, h, bpp)){
		if(tcf) *tcf = m_tcf;
		return TRUE;
	}

	return FALSE;
}
int BackBuffer::CooperateYaGit(){
	return FALSE;
}
int BackBuffer::CooperateNot(){
	return FALSE;
}

int BackBuffer::SetWindowPos(int x, int y){
	m_x = __max(x, 0);  m_y = __max(y, 0);
	if(!m_fullscreen2 && m_hwnd){
		ShowWindow(m_hwnd, SW_SHOWNORMAL);	//This is needed or else we can move/size the window while Maximised and Windows gets confoozed.
		MoveWindow(m_hwnd, m_x, m_y, m_realw, m_realh, TRUE);
		return TRUE;
	}
	return FALSE;
}
int BackBuffer::SetWindowSize(int w, int h){
	m_w = w;  m_h = h;
	if(m_hwnd)
	{
		if (m_fullscreen2)
		{
			ShowWindow(m_hwnd, SW_MAXIMIZE);
		}
		else
		{
			ShowWindow(m_hwnd, SW_SHOWNORMAL);
			//New, found AdjustWindowRectEx function!
			RECT rc;
			rc.left = rc.top = 0;
			rc.right = w;
			rc.bottom = h;
			AdjustWindowRectEx(&rc, WS_OVERLAPPEDWINDOW, 0, 0);
			m_realw = rc.right - rc.left;
			m_realh = rc.bottom - rc.top;
			//SetWindowPos will use the newly figured realw/h when it moves/sizes the window,
			//so it'll take care of it all.  SetWindowSize now automatically centers the window.
			if(centering){
				SetWindowPos((GetSystemMetrics(SM_CXSCREEN) - m_realw) / 2, (GetSystemMetrics(SM_CYSCREEN) - m_realh) / 2);
			}else{
				GetWindowRect(m_hwnd, &rc);
				SetWindowPos(rc.left, rc.top);
			}
		}
		return TRUE;
	}
	return FALSE;
}
int BackBuffer::SetPalette(PALETTEENTRY *pe){
	if(pe){
		for(int i = 0; i < 256; i++){
			m_pe[i].peRed = pe[i].peRed;
			m_pe[i].peGreen = pe[i].peGreen;
			m_pe[i].peBlue = pe[i].peBlue;
		}
	}

	m_DIB.SetPalette(m_pe);
	return TRUE;
}
int BackBuffer::GetPalette(PALETTEENTRY *pe){
	if(pe){
		for(int i = 0; i < 256; i++){
			pe[i].peRed = m_pe[i].peRed;
			pe[i].peGreen = m_pe[i].peGreen;
			pe[i].peBlue = m_pe[i].peBlue;
		}
		return TRUE;
	}
	return FALSE;
}
int BackBuffer::RealizePalette(){
	m_DIB.RealizePalette();

	return TRUE;
}

void BackBuffer::ClearDirtyRects(){
	ndirty = 0;
}
int BackBuffer::AddDirtyRect(DirtyRect *dr){
	if(dr) return AddDirtyRect(dr->x1, dr->y1, dr->x2 - dr->x1, dr->y2 - dr->y1);
	return FALSE;
}
int BackBuffer::AddDirtyRect(int x, int y, int w, int h){
	if(ndirty < MAX_DIRTY){
		int x2 = x + w;
		int y2 = y + h;
		int i;
		//See if it touches another rect, if so concatenate.
		//Does NOT concatenate properly if you overlap more than one...
		for(i = 0; i < ndirty; i++){
			if(dirty[i].x2 >= x && dirty[i].x1 <= x2 && dirty[i].y2 >= y && dirty[i].y1 <= y2){
				dirty[i].x1 = __min(dirty[i].x1, x);
				dirty[i].x2 = __max(dirty[i].x2, x2);
				dirty[i].y1 = __min(dirty[i].y1, y);
				dirty[i].y2 = __max(dirty[i].y2, y2);
				return TRUE;
			}
		}
		//No touch, so add it in.
		dirty[ndirty].x1 = x;
		dirty[ndirty].x2 = x2;
		dirty[ndirty].y1 = y;
		dirty[ndirty].y2 = y2;
		ndirty++;
		return TRUE;
	}
	return FALSE;	//Full-update will be done if dirty rects used up.
}
inline void DirtyToScreen(RECT *rcs, RECT *rcd, DirtyRect *dr, RECT *outs, RECT *outd){	//Thwacks dirty rectangle from rcs space into rcd space and plops it in out.
	outs->left = __max(0, dr->x1);
	outs->right = __min(rcs->right, dr->x2);
	outs->top = __max(0, dr->y1);
	outs->bottom = __min(rcs->bottom, dr->y2);
	//
	outd->left = (outs->left) * rcd->right / rcs->right;
	outd->right = (outs->right) * rcd->right / rcs->right;
	outd->top = (outs->top) * rcd->bottom / rcs->bottom;
	outd->bottom = (outs->bottom) * rcd->bottom / rcs->bottom;

	//outd->right = __min(rcd->right, outd->right);
	//outd->bottom = __min(rcd->bottom, outd->bottom);
}

int BackBuffer::UpdateFrontBuffer(int flags){

	if(m_Locks > 0) return FALSE;
	//If no dirty rects, add single all-encompassing dirty rect.
	if(ndirty <= 0 || (flags & UFB_DIRTY) == 0){
		ndirty = 1;
		dirty[0].x1 = dirty[0].y1 = 0;
		dirty[0].x2 = dirty[0].y2 = 10000;
	}
	int i;

	if(m_hwnd){
		if(m_DIB.Lock()){	//Still need this lock, as it's before a blit.
			RECT rcd, rcs;
			rcs.left = 0;
			rcs.top = 0;
			rcs.right = m_DIB.GetWidth();
			rcs.bottom = m_DIB.GetHeight();
			if((flags & UFB_STRETCH) == 0) CopyRect(&rcd, &rcs);
			else GetClientRect(m_hwnd, &rcd);
			RECT rcs2, rcd2;

			//m_DIB.StretchBlit(0, 0, rcd.right, rcd.bottom, 0, 0, rcs.right, rcs.bottom);

			for(i = 0; i < ndirty; i++){
				DirtyToScreen(&rcs, &rcd, &dirty[i], &rcs2, &rcd2);
				if(flags & UFB_STRETCH){
					m_DIB.StretchBlit(rcd2.left, rcd2.top, rcd2.right - rcd2.left, rcd2.bottom - rcd2.top,
						rcs2.left, rcs2.top, rcs2.right - rcs2.left, rcs2.bottom - rcs2.top);
				}else{
					m_DIB.Blit(rcd2.left, rcd2.top, rcs2.left, rcs2.top, rcs2.right - rcs2.left, rcs2.bottom - rcs2.top);
				}
			}
			m_DIB.Unlock();
			ndirty = 0;
			return TRUE;
		}
	}
	return FALSE;
}
int BackBuffer::Lock(BufferDesc *bufdesc){
	if(m_Locks < 0) m_Locks = 0;

	if(m_Locks < 1){
		GdiFlush();	//Now NOT locking dib in Lock(), as it's not needed, only for Dib blitting.
		if(m_BufDesc.data = m_DIB.Data()){//Lock()){
			m_BufDesc.width = m_DIB.GetWidth();
			m_BufDesc.height = m_DIB.GetHeight();
			m_BufDesc.pitch = m_DIB.GetPitch();
			m_BufDesc.bytespixel = m_DIB.BPP() >>3;
		}else{
			return FALSE;
		}
	}
	m_Locks++;
	*bufdesc = m_BufDesc;
	return TRUE;

	return FALSE;
}
int BackBuffer::Unlock(){
	if(m_Locks <= 0){
		m_Locks = 0;
		return FALSE;
	}
	m_Locks--;
	if(m_Locks == 0){	//Only unlock surface/DIB on last Unlock(), allows nested Locks/Unlocks.
		//DIB doesn't need to be locked/unlocked for pixel access.
	}
	return TRUE;
}
int BackBuffer::Destroy(int killwindow){
	FreeDIB();
	if(killwindow && m_hwnd) DestroyWindow(m_hwnd);
	m_hwnd = NULL;
	return FALSE;
}
int BackBuffer::InitDIB(int w, int h, int bpp){
	if (m_fullscreen2)
	{
		SetWindowLong(m_hwnd, GWL_STYLE, 0);
		SetWindowLong(m_hwnd, GWL_EXSTYLE, 0);
	}
	else
	{
		SetWindowLong(m_hwnd, GWL_STYLE, WS_OVERLAPPEDWINDOW);
		SetWindowLong(m_hwnd, GWL_EXSTYLE, 0);
	}
	//ShowWindow(m_hwnd, SW_SHOWNORMAL);
	SetFocus(m_hwnd);
	SetWindowSize(m_w, m_h);
	m_DIB.CreateHBitmap(m_hwnd, w, h, bpp);
	if(bpp == 8) m_tcf.SetMasks(8, 0, 0, 0);
	if(bpp == 16) m_tcf.SetMasks(16, 0x7C00, 0x3E0, 0x1F);	//Assumes 555 dib.
	if(bpp == 24 || bpp == 32) m_tcf.SetMasks(bpp, 0xFF0000, 0xFF00, 0xFF);
	return TRUE;
}
int BackBuffer::FreeDIB(){
	m_DIB.DeleteHBitmap();
	return FALSE;
}
