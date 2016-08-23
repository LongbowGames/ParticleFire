/*

Image & Bitmap - Tool classes to load 256 color bitmaps (Windows BMPs right now)
into raw data and palette buffers.
Extension of RawBMP class.
Adding multiple-image support and mip-map generation.

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

*/

#include "Image.h"
//#include <stdio>//.h>
#include <Quantizer.h>
#include <Basis.h>

extern "C" {

#include <neuquant.h>
//Neural network quantizer.

};

inline int WriteLong(int n, FILE *f){ return fwrite(&n, sizeof(int), 1, f); }
inline int ReadLong(FILE *f){ int n = 0; fread(&n, sizeof(int), 1, f); return n; }

int BGRAfromPE(ARGB *argb, PALETTEENTRY *pe, unsigned char alpha){
	if(argb && pe){
		for(int i = 0; i < 256; i++){
			argb[i].argb = (unsigned long)((pe[i].peRed <<16) | (pe[i].peGreen <<8) | pe[i].peBlue | (alpha <<24));
		}
		return TRUE;
	}
	return FALSE;
}
int RGBAfromPE(ARGB *argb, PALETTEENTRY *pe, unsigned char alpha){
	if(argb && pe){
		for(int i = 0; i < 256; i++){
			argb[i].argb = (unsigned long)((pe[i].peRed) | (pe[i].peGreen <<8) | (pe[i].peBlue <<16) | (alpha <<24));
		}
		return TRUE;
	}
	return FALSE;
}

int MakeRemapTable(unsigned char *Remap, const PALETTEENTRY *oldpe, const PALETTEENTRY *pe){
	if(Remap == NULL || oldpe == NULL || pe == NULL) return FALSE;

	int r, g, b, c, k, diff, t;
	UCHAR col;

	for(c = 0; c < 256; c++){	//First create remap table from the two palettes.
		r = oldpe[c].peRed;
		g = oldpe[c].peGreen;
		b = oldpe[c].peBlue;
		col = 0;	//Last color reg with lowest difference found.
		diff = 2048;
		for(k = 0; k < 256; k++){
		//	t = (abs(pe[k].peRed - r) << 1) + (abs(pe[k].peGreen - g) << 2) + abs(pe[k].peBlue - b);
			t = (abs(pe[k].peRed - r)) + (abs(pe[k].peGreen - g)) + abs(pe[k].peBlue - b);
			if(t < diff){
				diff = t;
				col = k;
			}
		}
		Remap[c] = col;
	}
	return TRUE;
}
int MakeRemapTable(unsigned char *Remap, const PALETTEENTRY *oldpe, const InversePal *inv){
	if(Remap == NULL || oldpe == NULL || inv == NULL) return FALSE;
	int c;
	for(c = 0; c < 256; c++){
		Remap[c] = inv->Lookup(oldpe[c].peRed, oldpe[c].peGreen, oldpe[c].peBlue);
	}
	return TRUE;
}

//Load and save functions now work if pe pointer is NULL.

int LoadBMP(const char *name, Bitmap *bmp, PALETTEENTRY *pe){
	FILE *f;
	if(f = fopen(name, "rb")){
		int t = LoadBMP(f, bmp, pe);
		fclose(f);
		return t;
	}
	return 0;
}
int LoadBMP(FILE *file, Bitmap *bmp, PALETTEENTRY *pe){
	BITMAPFILEHEADER    BMPFileHead;
	BITMAPINFOHEADER    BMPFileInfo;
	RGBQUAD             Palette[256];
	int NumCols, BPP;

	if(file == NULL || bmp == NULL) return FALSE;

	long startpos = ftell(file);

	// Read the BMP header and info structures
	if(NULL == fread(&BMPFileHead, sizeof(BMPFileHead), 1, file)) return FALSE;
	if(NULL == fread(&BMPFileInfo, sizeof(BMPFileInfo), 1, file)) return FALSE;
	
	//Make sure the BMP is 8-bit color
	//Nope, can handle 24 and 32 bits now...
	memset(Palette, 0, sizeof(Palette));
	if((BPP = BMPFileInfo.biBitCount) == 8){// return FALSE;
		NumCols = BMPFileInfo.biClrUsed;	//Check the actual number of palette entries.
		if(NumCols == 0) NumCols = 256;
	
		//get the BMP palette
		if(NULL == fread(Palette, sizeof(RGBQUAD), NumCols, file)) return FALSE;
	}
	//Allocate memory for image data
	if(!bmp->Init(BMPFileInfo.biWidth, abs(BMPFileInfo.biHeight), BPP)) return FALSE;

	//read BMP into memory
	fseek(file, startpos, SEEK_SET);
	fseek(file, BMPFileHead.bfOffBits, SEEK_CUR);
	for(int y = bmp->Height() - 1; y >= 0; y--){
		fread(bmp->Data() + y * bmp->Pitch(), sizeof(unsigned char), bmp->Pitch(), file);
	}

	//Copy palette from file.
	if(pe){
		for(int i = 0; i < 256; i++){
			pe[i].peRed = Palette[i].rgbRed;
			pe[i].peGreen = Palette[i].rgbGreen;
			pe[i].peBlue = Palette[i].rgbBlue;
		}
	}
	return TRUE;
}
int LoadPackedBMP(void *bmi, Bitmap *bmp, PALETTEENTRY *pe){
//	BITMAPFILEHEADER    BMPFileHead;
	BITMAPINFOHEADER	*BMPFileInfo;
	RGBQUAD             Palette[256];
	int NumCols, BPP;

	if(bmi == NULL || bmp == NULL) return FALSE;

	// Read the BMP header and info structures
	BMPFileInfo = (BITMAPINFOHEADER*)bmi;
	
	//Make sure the BMP is 8-bit color
	//Nope, can handle 24 and 32 bits now...
	NumCols = 0;
	memset(Palette, 0, sizeof(Palette));
	if((BPP = BMPFileInfo->biBitCount) == 8){// return FALSE;
		NumCols = BMPFileInfo->biClrUsed;	//Check the actual number of palette entries.
		if(NumCols == 0) NumCols = 256;
		//get the BMP palette
		memcpy(Palette, (char*)bmi + BMPFileInfo->biSize, sizeof(RGBQUAD) * NumCols);
	}
	//Allocate memory for image data
	if(!bmp->Init(BMPFileInfo->biWidth, abs(BMPFileInfo->biHeight), BPP)) return FALSE;

	//read BMP into memory
	int sy = 0;
	for(int y = bmp->Height() - 1; y >= 0; y--){
		memcpy(bmp->Data() + y * bmp->Pitch(), (char*)bmi + BMPFileInfo->biSize + NumCols * sizeof(RGBQUAD) +
			sy++ * bmp->Pitch(), bmp->Pitch());
	}

	//Copy palette from file.
	if(pe){
		for(int i = 0; i < 256; i++){
			pe[i].peRed = Palette[i].rgbRed;
			pe[i].peGreen = Palette[i].rgbGreen;
			pe[i].peBlue = Palette[i].rgbBlue;
		}
	}
	return TRUE;
}
int SaveBMP(const char *name, Bitmap *bmp, PALETTEENTRY *pe, int noflip){
	FILE *f;
	if(f = fopen(name, "wb")){
		int t = SaveBMP(f, bmp, pe, noflip);
		fclose(f);
		return t;
	}
	return 0;
}
int SaveBMP(FILE *file, Bitmap *bmp, PALETTEENTRY *pe, int noflip){
	BITMAPFILEHEADER    BMPFileHead;
	BITMAPINFOHEADER    BMPFileInfo;
	RGBQUAD             Palette[256];

	if(file == NULL || bmp == NULL || bmp->Data() == NULL ||
		bmp->Width() <= 0 || bmp->Height() <= 0 || bmp->Pitch() <= 0){
		return FALSE;
	}

	BMPFileHead.bfType = 'B' | ('M' << 8);
	BMPFileHead.bfReserved1 = 0;
	BMPFileHead.bfReserved2 = 0;
	if(bmp->BPP() == 8){
		BMPFileHead.bfSize = sizeof(BMPFileHead) + sizeof(BMPFileInfo) + sizeof(RGBQUAD) * 256 + bmp->Height() * bmp->Pitch();
		BMPFileHead.bfOffBits = sizeof(BMPFileHead) + sizeof(BMPFileInfo) + sizeof(RGBQUAD) * 256;
	}else{
		BMPFileHead.bfSize = sizeof(BMPFileHead) + sizeof(BMPFileInfo) + bmp->Height() * bmp->Pitch();
		BMPFileHead.bfOffBits = sizeof(BMPFileHead) + sizeof(BMPFileInfo);
	}

	BMPFileInfo.biSize = sizeof(BITMAPINFOHEADER);
	BMPFileInfo.biWidth = bmp->Width();
	BMPFileInfo.biHeight = bmp->Height();
	BMPFileInfo.biPlanes = 1;
	BMPFileInfo.biBitCount = bmp->BPP();
	BMPFileInfo.biCompression = BI_RGB;
	BMPFileInfo.biSizeImage = 0;
	BMPFileInfo.biXPelsPerMeter = 0;
	BMPFileInfo.biYPelsPerMeter = 0;
	BMPFileInfo.biClrUsed = 0;
	BMPFileInfo.biClrImportant = 0;

	if(NULL == fwrite(&BMPFileHead, sizeof(BMPFileHead), 1, file)) return FALSE;
	if(NULL == fwrite(&BMPFileInfo, sizeof(BMPFileInfo), 1, file)) return FALSE;
	
	if(bmp->BPP() == 8){
		for(int i = 0; i < 256; i++){
			if(pe){
				Palette[i].rgbRed = pe[i].peRed;
				Palette[i].rgbGreen = pe[i].peGreen;
				Palette[i].rgbBlue = pe[i].peBlue;
			}else{
				Palette[i].rgbRed = Palette[i].rgbGreen = Palette[i].rgbBlue = 0;
			}
		}
		if(NULL == fwrite(Palette, sizeof(Palette), 1, file)) return FALSE;
	}
	
	if(noflip){
		for(int y = 0; y < bmp->Height(); y++){
			fwrite(bmp->Data() + y * bmp->Pitch(), sizeof(unsigned char), bmp->Pitch(), file);
		}
	}else{
		for(int y = bmp->Height() - 1; y >= 0; y--){
			fwrite(bmp->Data() + y * bmp->Pitch(), sizeof(unsigned char), bmp->Pitch(), file);
		}
	}
	return TRUE;
}

Bitmap::Bitmap() : data(NULL), LineLeft(NULL), LineRight(NULL), ppe(NULL), premap(NULL),
					width(0), height(0), pitch(0), bpp(0), xhot(0), yhot(0), id(0), flags(0), nLines(0) {
//	data = NULL;
//	LineLeft = LineRight = NULL;
//	width = height = pitch = xhot = yhot = nLines = 0;
}
Bitmap::Bitmap(const Bitmap &bmp) : data(NULL), LineLeft(NULL), LineRight(NULL), ppe(NULL), premap(NULL),
					width(0), height(0), pitch(0), bpp(0), xhot(0), yhot(0), id(0), flags(0), nLines(0) {
//	data = NULL;
//	LineLeft = LineRight = NULL;
//	width = height = pitch = xhot = yhot = nLines = 0;
	if(bmp.data){
		if(Init(bmp.width, bmp.height, bmp.bpp)){
			Suck(bmp.data, bmp.width, bmp.height, bmp.pitch);
			if(bmp.nLines > 0){
				if(InitAnalyze()){
					memcpy(LineLeft, bmp.LineLeft, sizeof(int) * nLines);
					memcpy(LineRight, bmp.LineRight, sizeof(int) * nLines);
				}
			}
		}
	}
}
Bitmap::~Bitmap(){
	Free();
}
Bitmap& Bitmap::operator=(const Bitmap &bmp){
	if(&bmp == this) return *this;	//Identity copy check.  (x = x;)
	Free();
	if(bmp.data){
		if(Init(bmp.width, bmp.height, bmp.bpp)){
			Suck(bmp.data, bmp.width, bmp.height, bmp.pitch);
		//	ppe = bmp.ppe;	//Don't do that, only copy image data.
		//	premap = bmp.premap;
			if(bmp.nLines > 0){
				if(InitAnalyze()){
					memcpy(LineLeft, bmp.LineLeft, sizeof(int) * nLines);
					memcpy(LineRight, bmp.LineRight, sizeof(int) * nLines);
				}
			}
		}
	}
	return *this;
}
/*
void Bitmap::MakeMasks(){
	xshift = HiBit(width);
	xmask = (1 << xshift) - 1;
	yshift = HiBit(height);
	ymask = (1 << yshift) - 1;
}
*/
int Bitmap::Init(int w, int h, int b){	//Creates an empty bitmap.
	Free();
	if(w > 0 && h > 0 && (b == 8 || b == 16 || b == 24 || b == 32)){
		width = w;
		height = h;
	//	bpp = b;
	//	pixelpitch = bpp / 8;
	//	alphaskip = pixelpitch - 3;
	//	colorbytes = (pixelpitch > 3 ? 3 : pixelpitch);
		SetBPP(b);
		pitch = CalcPitch(w, b);//(w + 3) & (~3);
		if(NULL == (data = (unsigned char*)malloc(CalcLength(w, h, b)))){
			Free();
			return 0;
		}
		memset(data, 0, CalcLength(w, h, b));
	//	MakeMasks();
//		memset(pe, 0, sizeof(pe));
		return 1;
	}
	return 0;
}
void Bitmap::Free(){
	FreeAnalyze();
	if(data) free(data);
	data = NULL;
	LineLeft = LineRight = NULL;
	width = height = pitch = bpp = xhot = yhot = nLines = 0;
	pixelpitch = colorbytes = alphaskip = 0;
	id = flags = 0;
//	xmask = ymask = xshift = yshift = 0;
}
int Bitmap::Clear(unsigned char color){
	if(data && width > 0 && height > 0){
		for(int y = 0; y < height; y++){
			memset(data + y * pitch, color, width * (bpp >>3));
		}
		flags |= BFLAG_CLEARED;
		return TRUE;
	}
	return FALSE;
}
int Bitmap::ColorCorrect(float rgain, float ggain, float bgain, float again,
					float rbias, float gbias, float bbias, float abias,
					float rscale, float gscale, float bscale, float ascale,
					float rshift, float gshift, float bshift, float ashift){
	const float i255 = 1.0f / 255.0f;
	int t;
	if(bpp == 8 && ppe){
		for(int i = 0; i < 256; i++){
			t = (int)((Bias(rbias, Gain(rgain, (float)ppe[i].peRed * i255)) * rscale + rshift) * 255.0f);
			ppe[i].peRed = __min(__max(t, 0), 255);
			t = (int)((Bias(gbias, Gain(ggain, (float)ppe[i].peGreen * i255)) * gscale + gshift) * 255.0f);
			ppe[i].peGreen = __min(__max(t, 0), 255);
			t = (int)((Bias(bbias, Gain(bgain, (float)ppe[i].peBlue * i255)) * bscale + bshift) * 255.0f);
			ppe[i].peBlue = __min(__max(t, 0), 255);
		}
		flags |= BFLAG_COLORCORRECTED;
		return 1;
	}else if((bpp == 24 || bpp == 32) && data && width > 0 && height > 0){
		unsigned char tab[4][256];
		for(int i = 0; i < 256; i++){
			t = (int)((Bias(bbias, Gain(bgain, (float)i * i255)) * bscale + bshift) * 255.0f);
			tab[0][i] = __min(__max(t, 0), 255);
			t = (int)((Bias(gbias, Gain(ggain, (float)i * i255)) * gscale + gshift) * 255.0f);
			tab[1][i] = __min(__max(t, 0), 255);
			t = (int)((Bias(rbias, Gain(rgain, (float)i * i255)) * rscale + rshift) * 255.0f);
			tab[2][i] = __min(__max(t, 0), 255);
			t = (int)((Bias(abias, Gain(again, (float)i * i255)) * ascale + ashift) * 255.0f);
			tab[3][i] = __min(__max(t, 0), 255);
		}
		for(int y = 0; y < height; y++){
			unsigned char *p = data + pitch * y;
			if(bpp == 24){
				for(int x = width; x; x--){
					p[0] = tab[0][p[0]];
					p[1] = tab[1][p[1]];
					p[2] = tab[2][p[2]];
					p += 3;
				}
			}else{
				for(int x = width; x; x--){
					p[0] = tab[0][p[0]];
					p[1] = tab[1][p[1]];
					p[2] = tab[2][p[2]];
					p[3] = tab[3][p[3]];
					p += 4;
				}
			}
		}
		flags |= BFLAG_COLORCORRECTED;
		return 1;
	}
	return 0;
}

int Bitmap::To32Bit(PALETTEENTRY *pe){
	if(!pe) pe = ppe;	//Null param, try stored palette pointer.
	if(bpp == 32) return 1;
	if(data && width > 0 && height > 0 && (bpp == 8 || bpp == 24)){
		int NewPitch = CalcPitch(width, 32);
		unsigned char *newdata, *tdata, *tnewdata;
		if(newdata = (unsigned char*)malloc(CalcLength(width, height, 32))){
			for(int y = 0; y < height; y++){
				tdata = (data + y * pitch);
				tnewdata = (newdata + y * NewPitch);
				for(int x = 0; x < width; x++){
					if(bpp == 8){	//8bit original.
						if(pe){
							*tnewdata++ = pe[*tdata].peBlue;
							*tnewdata++ = pe[*tdata].peGreen;
							*tnewdata++ = pe[*tdata].peRed;
						}else{	//Assume grayscale source.
							*tnewdata++ = *tdata;
							*tnewdata++ = *tdata;
							*tnewdata++ = *tdata;
						}
						*tnewdata++ = 0;
						tdata++;
					}else{	//24bit original.
						*tnewdata++ = *tdata++;
						*tnewdata++ = *tdata++;
						*tnewdata++ = *tdata++;
						*tnewdata++ = 0;
					}
				}
			}
			free(data);
			data = newdata;
			pitch = NewPitch;
			SetBPP(32);
			return 1;
		}
	}
	return 0;
}

int Bitmap::RotateRight90(){	//NOTE:  Function only works with 8 BPP Bitmaps!
	if(data && width > 0 && height > 0 && bpp == 8){
		int NewWidth = height, NewHeight = width;
		int NewPitch = (NewWidth + 3) & (~3);
	//	if(NewWidth % 4 != 0) NewPitch = NewWidth + 4 - NewWidth % 4;
	//		else NewPitch = NewWidth;
		unsigned char *newdata;
		if(newdata = (unsigned char*)malloc(NewPitch * NewHeight)){
			for(int y = 0; y < NewHeight; y++){
				for(int x = 0; x < NewWidth; x++){
					*(newdata + x + y * NewPitch) = *(data + y + pitch * (height - 1) - x * pitch);
				}
			}
			free(data);
			data = newdata;
			width = NewWidth;
			height = NewHeight;
			pitch = NewPitch;
			flags |= BFLAG_ROTATED;
			return 1;
		}
	}
	return 0;
}
int Bitmap::RotateLeft90(){	//NOTE:  Function only works with 8 BPP Bitmaps!
	if(data && width > 0 && height > 0 && bpp == 8){
		int NewWidth = height, NewHeight = width;
		int NewPitch = (NewWidth + 3) & (~3);
	//	if(NewWidth % 4 != 0) NewPitch = NewWidth + 4 - NewWidth % 4;
	//		else NewPitch = NewWidth;
		unsigned char *newdata;
		if(newdata = (unsigned char*)malloc(NewPitch * NewHeight)){
			for(int y = 0; y < NewHeight; y++){
				for(int x = 0; x < NewWidth; x++){
					*(newdata + x + y * NewPitch) = *(data + (width - 1) - y + x * pitch);
				}
			}
			free(data);
			data = newdata;
			width = NewWidth;
			height = NewHeight;
			pitch = NewPitch;
			flags |= BFLAG_ROTATED;
			return 1;
		}
	}
	return 0;
}
int Bitmap::ScaleToPow2(){
	if(data && width > 0 && height > 0){
		int NewWidth = 1 << HiBit(width);
		int NewHeight = 1 << HiBit(height);
		//(NewWidth * height) / width;
		if(NewWidth == width && NewHeight == height) return TRUE;	//Already correct.
		return Scale(NewWidth, NewHeight);
	}
	return FALSE;
}
int Bitmap::Scale(int NewWidth, int NewHeight, int lerp){
	if(data && width > 0 && height > 0){// && bpp == 8){
		int NewPitch = CalcPitch(NewWidth, bpp);//(NewWidth + 3) & (~3);
		unsigned char *newdata;
		if(NewWidth > 0 && NewHeight > 0 && NewPitch > 0 && (newdata = (unsigned char*)malloc(CalcLength(NewWidth, NewHeight, bpp)))){//NewPitch * NewHeight))){
			int Dx = (width <<16) / NewWidth;
			int Dy = (height <<16) / NewHeight;
			int SrcX = 0, SrcY = 0;
			if(lerp && bpp >= 24){	//Right now, this will only lerp when down-sizing...
				for(int y = 0; y < NewHeight; y++){
					SrcX = 0;
					unsigned char *td = newdata + y * NewPitch;
					unsigned char *ts;
					int bts = bpp / 8;
					int h2 = ((SrcY + Dy) >>16) - (SrcY >>16);
					h2 = __min(h2, height - (SrcY >>16));
					for(int x = 0; x < NewWidth; x++){
						int w2 = ((SrcX + Dx) >>16) - (SrcX >>16);
						w2 = __min(w2, width - (SrcX >>16));
						int pxls = w2 * h2;
						if(pxls > 0){
							int accum[4] = {0, 0, 0, 0};	//Color accumulators.
							unsigned char *ts1 = data + (SrcX >>16) * bts + (SrcY >>16) * pitch;
							for(int y2 = h2; y2; y2--){
								unsigned char *ts2 = ts1;
								for(int x2 = w2; x2; x2--){
									accum[0] += *ts2++;
									accum[1] += *ts2++;
									accum[2] += *ts2++;
									if(bts > 3) accum[3] += *ts2++;
								}
								ts1 += pitch;
							}
							for(int z = 0; z < bts; z++){
								*td++ = accum[z] / pxls;
							}
						}
					//	ts = data + (SrcX >>16) * bts + (SrcY >>16) * pitch;
					//	for(int z = bts; z; z--){
					//		*td++ = *ts++;
					//	}
					//	*(newdata + x + y * NewPitch) = *(data + (SrcX >>16) + (SrcY >>16) * pitch);
						SrcX += Dx;
					}
					SrcY += Dy;
				}
			}else{
				for(int y = 0; y < NewHeight; y++){
					SrcX = 0;
					unsigned char *td = newdata + y * NewPitch;
					unsigned char *ts;
					int bts = bpp / 8;
					for(int x = 0; x < NewWidth; x++){
						ts = data + (SrcX >>16) * bts + (SrcY >>16) * pitch;
						for(int z = bts; z; z--){
							*td++ = *ts++;
						}
					//	*(newdata + x + y * NewPitch) = *(data + (SrcX >>16) + (SrcY >>16) * pitch);
						SrcX += Dx;
					}
					SrcY += Dy;
				}
			}
			free(data);
			data = newdata;
			width = NewWidth;
			height = NewHeight;
			pitch = NewPitch;
			flags |= BFLAG_SCALED;
			return TRUE;
		}
	}
	return FALSE;
}
unsigned int Bitmap::GetPixel(int x, int y){
	if(data && x >= 0 && x < width && y >= 0 && y < height){
		if(bpp == 8) return (unsigned int)*(data + y * pitch + x);
		if(bpp == 16) return (unsigned int)*((unsigned short*)(data + y * pitch + x));
		if(bpp == 24){
			unsigned char *tp = data + y * pitch + x * 3;
			return (unsigned int)tp[0] | ((unsigned int)tp[1] <<8) | ((unsigned int)tp[2] <<16);
		}
		if(bpp == 32) return *((unsigned int*)(data + y * pitch + (x <<2)));
	}
	return 0;
}
int Bitmap::PutPixel(int x, int y, unsigned int pixel){
	if(data && x >= 0 && x < width && y >= 0 && y < height){
		switch(bpp){
		case 8 : *(data + y * pitch + x) = (unsigned char)pixel; break;
		case 16 : *((unsigned short*)(data + y * pitch + x)) = (unsigned short)pixel; break;
		case 24 :
			unsigned char *tp; tp = data + y * pitch + x * 3;
			tp[0] = pixel & 255; tp[1] = (pixel >>8) & 255; tp[2] = (pixel >>16) & 255;
			break;
		case 32 : *((unsigned int*)(data + y * pitch + (x <<2))) = pixel; break;
		}
		return 1;
	}
	return 0;
}

int Bitmap::Suck(void *vsrc, int sw, int sh, int spitch, int sbpp){	//Works with 32bit, with proper pitches in bytes and widths in pixels.
	unsigned char *src = (unsigned char*)vsrc;
	if(sbpp <= 0) sbpp = bpp;	//Not entering value assumes identical bpps.
	if(data && src && (sbpp == bpp || (sbpp >= 24 && bpp >= 24) || (bpp == 8 && sbpp >= 24))){	//Must be the same bpp, or both true color.  No mixing true and paletteded.
		sw = __min(sw, width);
		sh = __min(sh, height);
		if(sw > 0 && sh > 0){
			for(int y = 0; y < sh; y++){
				if(sbpp == bpp){	//Bits per pixel match, so we just memcpy.
					memcpy(data + y * pitch, src + y * spitch, sw * (bpp >>3));
				}else{	//Time for the true-color mambo.  If one is 8 bit we're sunk!
					int skips = (sbpp == 24 ? 0 : 1);
					int skipd = (bpp == 24 ? 0 : 1);	//Alpha skipping?
					unsigned char *sp = src + y * spitch, *dp = data + y * pitch;
					if(bpp == 8){	//Do the grayscale convert.
						for(int x = 0; x < sw; x++){
							*dp++ = ((sp[0] + sp[1] + sp[2]) / 3);
							sp += 3 + skips;
						}
					}else{	//Do the color copy.
						for(int x = 0; x < sw; x++){
							*dp++ = *sp++;
							*dp++ = *sp++;
							*dp++ = *sp++;
							dp += skipd;
							sp += skips;
						}
					}
				}
			}
			return TRUE;
		}
	}
	return FALSE;
}
int Bitmap::SuckARGB(Bitmap *bmp, ARGB *argb){
	if(bmp && argb && bpp >= 16 && bmp->BPP() == 8 && data && bmp->Data()){
		int sw = __min(bmp->Width(), width);
		int sh = __min(bmp->Height(), height);
		for(int y = 0; y < sh; y++){
			unsigned char *dp = data + y * pitch, *sp = bmp->Data() + y * bmp->Pitch();
			unsigned char *ta;
			int bytes = bpp >>3;
			for(int x = 0; x < sw; x++){
				ta = (unsigned char*)&argb[*sp++];
				for(int b = bytes; b > 0; b--) *dp++ = *ta++;
			}
		}
		return TRUE;
	}
	return FALSE;
}

//Clipped blitter, assumes entire Bitmap as source.
//Uses Bitmap's HotSpot.
//This SHOULD WORK with true color bitmaps, though transparency isn't supported!
int Bitmap::Blit(void *vdest, int dw, int dh, int dp, int dx, int dy, int trans){
	return Blit(vdest, dw, dh, dp, dx, dy, 0, 0, width, height, trans);
}
int Bitmap::Blit(void *vdest, int dw, int dh, int dp, int dx, int dy,
				 int sx, int sy, int sw, int sh, int trans){
	unsigned char *dest = (unsigned char*)vdest;
	if(data && dest && dw > 0 && dh > 0){
		int t = 0;
		//Initial values for rectangle to blit from Bitmap.
		int inx = sx;//__min(__max(sx, 0), width);
		int iny = sy;//__min(__max(sy, 0), height);
		int w = __min(__max(sw, 0), width - inx);//width;
		int h = __min(__max(sh, 0), height - iny);//height;
		dx -= xhot;
		dy -= yhot;
		//Lets get clipping!
		//
		if(inx < 0){	//Source off left edge.
			dx -= inx;	//Push dest x right by that amount.
			w += inx;
			inx = 0;
		}
		if(iny < 0){
			dy -= iny;
			h += iny;
			iny = 0;
		}
		//
		if(dx < 0){	//Off left edge.
			inx -= dx;
			w += dx;
			dx = 0;
		}
		if(dx + w > dw){	//Off right edge.
			w -= dx + w - dw;
		}
		if(dy < 0){	//Off top edge.
			iny -= dy;
			h += dy;
			dy = 0;
		}
		if(dy + h > dh){	//Off bottom edge.
			h -= dy + h - dh;
		}
		if(w > 0 && h > 0 && dx < dw && dy < dh){
			BlitRaw(dest + dx * pixelpitch + dy * dp, dp, inx, iny, w, h, trans);
		}
		return TRUE;
	}
	return FALSE;
}
//Unclipped blitter, allows partial area of Bitmap as source.
int Bitmap::BlitRaw(void *vdest, int dpitch, int insetx, int insety, int w, int h, int trans){
	unsigned char *dest = (unsigned char*)vdest;
//	if(data && dest && w > 0 && h > 0 && insetx >= 0 && insety >= 0 && w + insetx <= width && h + insety <= height){
	if(data && dest && w > 0 && h > 0 && insetx >= 0 && insety >= 0){
		if(w <= 0) w = width;
		if(h <= 0) h = height;
		w = __min(w, width - insetx);
		h = __min(h, height - insety);
		int x, y;
	//	unsigned long *lsource, *ltsrc, *ltdst;
		unsigned char *source, *tsrc, *tdst;
		if(trans && (bpp == 8 || bpp == 16 || bpp == 32)){	//Transblit not supported on 24bit.
			source = data + insetx * pixelpitch + insety * pitch;
			for(y = 0; y < h; y++){
				tdst = dest;
				tsrc = source;
				if(bpp == 8){
					for(x = 0; x < w; x++){
						if(tsrc[x]) tdst[x] = tsrc[x];
					//	if(*tsrc) *tdst = *tsrc;
					//	tdst++; tsrc++;
					}
				}else if(bpp == 16){
					for(x = 0; x < w; x++){
						if(((unsigned short*)tsrc)[x]) ((unsigned short*)tdst)[x] = ((unsigned short*)tsrc)[x];
					//	tdst += 2; tsrc += 2;
					}
				}else{
					for(x = 0; x < w; x++){
						if(((unsigned long*)tsrc)[x]) ((unsigned long*)tdst)[x] = ((unsigned long*)tsrc)[x];
					}
				}
				dest += dpitch;
				source += pitch;
			}
		}else{
			source = data + insetx * pixelpitch + insety * pitch;
			for(y = 0; y < h; y++){
				memcpy(dest, source, w * pixelpitch);
				dest += dpitch;
				source += pitch;
			}
		}
		return TRUE;
	}
	return FALSE;
}
int Bitmap::BlitRaw8to32(unsigned long *dest, int dpitch, int insetx, int insety, int w, int h, int trans, ARGB *argb){
	ARGB argb2[256];
	if(!argb && ppe){
		BGRAfromPE(argb2, ppe);
		argb = argb2;
	}
	if(argb && bpp == 8 && data && dest && w > 0 && h > 0 && insetx >= 0 && insety >= 0 && w + insetx <= width && h + insety <= height){
		int x, y;
		unsigned char *source, *tsrc;
		unsigned long *tdst;
	//	ARGB argb[256];
	//	ARGBfromPE(argb, pe);
		source = data + insetx * (bpp >>3) + insety * pitch;
		for(y = 0; y < h; y++){
			tdst = dest;
			tsrc = source;
			for(x = 0; x < w; x++){
				*tdst = argb[*tsrc].argb;
				tdst++;
				tsrc++;
			}
			dest += (dpitch >>2);	//Flinking pointer arithmetic.
			source += pitch;
		}
		return TRUE;
	}
	return FALSE;
}
int Bitmap::Quantize32to8HighQuality(Bitmap *destb, PALETTEENTRY *pe){
	if(!destb) return 0;
	if(!pe) pe = destb->ppe;
	if((bpp == 24 || bpp == 32) && data && width > 0 && height > 0 && destb && pe){
		if(destb->Width() < width || destb->Height() < height || destb->Data() == NULL || destb->BPP() != 8){
			destb->Init(width, height, 8);	//Make sure dest bitmap is compatible with our output.
		}
		if(destb->Data() && destb->BPP() == 8){
			unsigned char *pic = (unsigned char*)malloc(width * height * 3);
			if(!pic) return 0;
			unsigned char *tp = pic;
			//
			int y, x, i;
			for(y = 0; y < height; y++){
				unsigned char *sl = data + y * pitch;
				for(x = 0; x < width; x++){
					*tp++ = *sl++;
					*tp++ = *sl++;
					*tp++ = *sl++;
					if(bpp > 24) sl++;
				}
			}
//   	pic = (unsigned char*) malloc(3*width*height);
			initnet(pic,3 * width * height, 1);
			learn();
			unbiasnet();
		//	[write output image header, using writecolourmap(f)]
			for(i = 0; i < 256; i++){
				pe[i].peRed = getcolourcomp(i, 0);
				pe[i].peGreen = getcolourcomp(i, 1);
				pe[i].peBlue = getcolourcomp(i, 2);
			}
			inxbuild();
		//	write output image using inxsearch(b,g,r)		*/
			for(y = 0; y < height; y++){
				unsigned char *sl = data + y * pitch;
				unsigned char *dl = destb->Data() + y * destb->Pitch();
				for(x = 0; x < width; x++){
					*dl++ = inxsearch(sl[0], sl[1], sl[2]);
					if(bpp > 24) sl += 4;
					else sl += 3;
				}
			}
			//
			free(pic);
			//
			flags |= BFLAG_QUANTIZED;
			return TRUE;
		}
	}
	return FALSE;
}
int Bitmap::Quantize32to8(Bitmap *destb, PALETTEENTRY *pe, int cols, int prequantize){
	if(!destb) return 0;
	if(!pe) pe = destb->ppe;
	if((bpp == 24 || bpp == 32) && data && width > 0 && height > 0 && destb && pe && cols > 0 && cols <= 256){
		if(destb->Width() < width || destb->Height() < height || destb->Data() == NULL || destb->BPP() != 8){
			destb->Init(width, height, 8);	//Make sure dest bitmap is compatible with our output.
		}
		if(destb->Data() && destb->BPP() == 8){
			int x, y, w, h;
			PALETTEENTRY col;
			ColorOctree oct;
			unsigned char *sp, *dp;
			int skip = (bpp == 24 ? 0 : 1);	//Bytes to skip over (alpha channel).
			w = __min(width, destb->Width());
			h = __min(height, destb->Height());
			//
			unsigned char pqm = 0xff;
			for(int i = 0; i < __min(prequantize, 8); i++) pqm = (pqm <<1) & 0xfe;
			//First pass, collect colors.
			for(y = 0; y < h; y++){
				sp = data + y * pitch;
				if(flags & BFLAG_BYTEORDER_RGBA){
					for(x = 0; x < w; x++){
						col.peRed = *sp++ & pqm;
						col.peGreen = *sp++ & pqm;	//Load bits into col PE for transfer.
						col.peBlue = *sp++ & pqm;
						sp += skip;	//Skip alpha if present.
						oct.Add(col);	//Add color to octree.
					}
				}else{
					for(x = 0; x < w; x++){
						col.peBlue = *sp++ & pqm;
						col.peGreen = *sp++ & pqm;	//Load bits into col PE for transfer.
						col.peRed = *sp++ & pqm;
						sp += skip;	//Skip alpha if present.
						oct.Add(col);	//Add color to octree.
					}
				}
			}
			//Now we reduce palette.
			oct.Reduce(cols);
			oct.GetPalette(pe);
			//And now we produce 8bit output.
			for(y = 0; y < h; y++){
				sp = data + y * pitch;
				dp = destb->Data() + y * destb->Pitch();
				if(flags & BFLAG_BYTEORDER_RGBA){
					for(x = 0; x < w; x++){
						col.peRed = *sp++ & pqm;
						col.peGreen = *sp++ & pqm;	//Load bits into col PE for transfer.
						col.peBlue = *sp++ & pqm;
						sp += skip;	//Skip alpha if present.
						*dp++ = oct.LookupIndex(col);	//Write color register to 8bit bitmap.
					}
				}else{
					for(x = 0; x < w; x++){
						col.peBlue = *sp++ & pqm;
						col.peGreen = *sp++ & pqm;	//Load bits into col PE for transfer.
						col.peRed = *sp++ & pqm;
						sp += skip;	//Skip alpha if present.
						*dp++ = oct.LookupIndex(col);	//Write color register to 8bit bitmap.
					}
				}
			}
			flags |= BFLAG_QUANTIZED;
			return TRUE;
		}
	}
	return FALSE;
}

int Bitmap::MakeMipMap(Bitmap *sbm, MixTable *Mix, int mixmode, int trans){	//8bit ONLY!
	if(sbm && sbm->data && Mix && sbm->bpp == 8){
		if(Init(__max(sbm->width / 2, 1), __max(sbm->height / 2, 1), sbm->bpp)){
			flags |= BFLAG_MIPMAP;
			if(sbm->width <= 1 || sbm->height <= 1) return TRUE;	//Just abort on miniscule mipmaps for now.  Code below assumes at least 2x2 source bitmap.
			unsigned char ul, ur, dl, dr, u, d, *ts, *td;
			for(int y = 0; y < height; y++){
				ts = sbm->data + (y <<1) * sbm->pitch;
				td = data + y * pitch;
				for(int x = 0; x < width; x++){
					if(trans){
						if(*ts == 0){//*(sbm->data + (x <<1) + (y <<1) * sbm->pitch) == 0){
						//	*(data + x + y * pitch) = 0;
							*td = 0;
						}else{
							ul = *ts;
							ur = *(ts + 1);
							if(ul == 0) ul = ur;
							if(ur == 0) ur = ul;
							u = Mix->Mix(ul, ur, mixmode);
							dl = *(ts + sbm->pitch);
							dr = *(ts + 1 + sbm->pitch);
							if(dl == 0) dl = dr;
							if(dr == 0) dr = dl;
							d = Mix->Mix(dl, dr, mixmode);
							if(u == 0) u = d;
							if(d == 0) d = u;
							*td = Mix->Mix(u, d, mixmode);
						}
					}else{
						*td = Mix->Mix( Mix->Mix(*ts, *(ts + 1), mixmode),
							Mix->Mix(*(ts + sbm->pitch), *(ts + 1 + sbm->pitch), mixmode), mixmode);

						//
					//	*(data + x + y * pitch) = Mix->Mix(
					//		Mix->Mix( *(sbm->data + (x <<1) + (y <<1) * sbm->pitch),
					//					*(sbm->data + (x <<1) + 1 + (y <<1) * sbm->pitch), mixmode ),
					//		Mix->Mix( *(sbm->data + (x <<1) + ((y <<1) + 1) * sbm->pitch),
					//					*(sbm->data + (x <<1) + 1 + ((y <<1) + 1) * sbm->pitch), mixmode ), mixmode );
					}
					ts += 2;
					td++;
				}
			}
			return TRUE;
		}
	}
	return FALSE;
}

int Bitmap::Remap(unsigned char *Remap){	//8bit ONLY!
	if(!Remap) Remap = premap;
	if(Remap && data && width > 0 && height > 0 && bpp == 8){
		unsigned char *pdata;
		for(int y = 0; y < height; y++){	//Now remap pixel data using remap table.
			for(int x = 0; x < width; x++){
				pdata = data + x + y * pitch;
				*pdata = Remap[*pdata];
			}
		}
		flags |= BFLAG_REMAPPED;
		return TRUE;
	}
	return FALSE;
}

int Bitmap::InitAnalyze(){
	if(nLines != height){
		FreeAnalyze();
		nLines = height;
		LineLeft = new int[nLines];
		LineRight = new int[nLines];
		if(LineLeft != NULL && LineRight != NULL) return 1;
		FreeAnalyze();
		return 0;
	}
	if(nLines > 0) return 1;
	return 0;
}
int Bitmap::AnalyzeLines(){	//8bit ONLY at the moment!
	int x, y;
	if(data && height > 0 && InitAnalyze() && bpp == 8){
		for(y = 0; y < nLines; y++){
			x = 0;
			while(x < width && *((unsigned char*)data + x + y * pitch) == 0) x++;
			LineLeft[y] = x;
			x = 0;
			while(x < width && *((unsigned char*)data + width - 1 - x + y * pitch) == 0) x++;
			LineRight[y] = x;
		}
		return 1;
	}
	return 0;
}
void Bitmap::FreeAnalyze(){
	nLines = 0;
	if(LineLeft) delete [] LineLeft; LineLeft = NULL;
	if(LineRight) delete [] LineRight; LineRight = NULL;
}

int ImageSet::LoadSet(const char *n){
	FILE *f;
	if((f = fopen(n, "rb"))){
		int ret = LoadSet(f);
		fclose(f);
		return ret;
	}
	return 0;
}
#define IMAGESETVERSION 1
int ImageSet::LoadSet(FILE *f){
	char buf[1024];
	memset(buf, 0, sizeof(buf));
	if(f){
		fread(buf, 4, 1, f);
		if(strcmp("IMST", buf) == 0){
			int Version = ReadLong(f);
			if(Version > IMAGESETVERSION){
				OutputDebugString("Bad ImageSet version!\n");
				return 0;
			}
			int nImg = ReadLong(f);
			if(!InitSet(nImg)) return FALSE;
			int Cols = ReadLong(f);
			PALETTEENTRY gpe[256];
			memset(gpe, 0, sizeof(gpe));
			int i, j, pos, w, h, y;
			Image *ip;
			for(i = 0; i < Cols; i++){
				gpe[i].peRed = fgetc(f);
				gpe[i].peGreen = fgetc(f);
				gpe[i].peBlue = fgetc(f);
				fgetc(f);
			}
			for(i = 0; i < nImg; i++){
				pos = ReadLong(f);	//Size of name.
				fread(buf, __min(sizeof(buf) - 1, pos), 1, f);
				if(names && i < nImages) names[i] = buf;	//Set the name.
				ip = &((*this)[i]);
				w = ReadLong(f);
				h = ReadLong(f);
				ReadLong(f);
				ReadLong(f);
				pos = ReadLong(f);	//Optional local palette colors.
				if(ip->Init(w, h, 8)){
					if(pos > 0){	//Read local palette.
						for(j = 0; j < pos; j++){
							ip->pe[j].peRed = fgetc(f);
							ip->pe[j].peGreen = fgetc(f);
							ip->pe[j].peBlue = fgetc(f);
							ip->remap[j] = fgetc(f);
						}
					}else{	//Just use global palette.
						ip->SetPalette(gpe);
					}
					for(y = 0; y < h; y++){
						fread(ip->Data() + y * ip->Pitch(), w, 1, f);
					}
				}
			}
			return TRUE;
		}
	}
	return 0;
}
