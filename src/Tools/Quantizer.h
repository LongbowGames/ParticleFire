/*

Color quantization class.  Compresses multiple 256 color palettes into one.

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

#ifndef QUANTIZER_H
#define QUANTIZER_H

#define VC_EXTRALEAN
#define WIN32_EXTRA_LEAN
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

class InversePal{
private:
	int r_center = 0, g_center = 0, b_center = 0;
	int r_skip1 = 0, r_skip2 = 0, g_skip1 = 0, g_skip2 = 0, b_skip1 = 0, b_skip2 = 0;
	unsigned char *inv_pal = NULL;
	int red_max = 0, green_max = 0, blue_max = 0;
	int gb_pow2 = 0, red_pow2 = 0, green_pow2 = 0, blue_pow2 = 0;
	int r_quant = 0, g_quant = 0, b_quant = 0;
	unsigned char cur_color = 0;
	void red_init();
	void green_init();
	void blue_init();
	bool red_loop(int *dbuf, unsigned char *cbuf, int center, int dist);
	bool green_loop(int *dbuf, unsigned char *cbuf, int center, int dist);
	bool blue_loop(int *dbuf, unsigned char *cbuf, int center, int dist);
public:
	PALETTEENTRY pe[256];
	InversePal();
	InversePal(int rbits, int gbits, int bbits);
	~InversePal();
	bool Init(int rbits, int gbits, int bbits);
	bool Make(PALETTEENTRY *pe, int ncols = 256);
	void Free();
	unsigned char Lookup(unsigned char r, unsigned char g, unsigned char b) const{
		if(inv_pal){
			return *(inv_pal + ((r >>r_quant) <<gb_pow2) + ((g >>g_quant) <<blue_pow2) + (b >>b_quant));
		}
		return 0;
	};
};


//Storage and access abstraction for color mixing table.
#define MIX25 3
#define MIX50 2
#define MIX75 1
#define MIX100 0
//#define INVPALBITS 4
//#define INVPALSIZE (1 << INVPALBITS)
//#define INVPALSHIFT (8 - INVPALBITS)
//#define INVPALSHIFT2 (INVPALBITS - INVPALSHIFT)
//Shift 8bit value down this much to get inverse palette index.
#define LIGHT_SHADES 16
class MixTable{
friend class RenderEngine;
friend class PolyRender;
private:
	UCHAR Mix4[257][256][4];
	UCHAR Add1[257][256];
	UCHAR Sub1[257][256];
	UCHAR Light1[LIGHT_SHADES][256];
//	UCHAR InvPal[INVPALSIZE][INVPALSIZE][INVPALSIZE];
	InversePal InvPal;
public:
	int DifferentColors;
	int CachedColors;
public:
	unsigned char Mix25(unsigned char a, unsigned char b){ return Mix4[a][b][MIX25]; };
	unsigned char Mix50(unsigned char a, unsigned char b){ return Mix4[a][b][MIX50]; };
	unsigned char Mix75(unsigned char a, unsigned char b){ return Mix4[a][b][MIX75]; };
	unsigned char Mix(unsigned char a, unsigned char b, unsigned char c){
		return Mix4[a][b][c]; };
	unsigned char Add(unsigned char src, unsigned char dst){ return Add1[src][dst]; };	//Saturation arithmetic addition and subtraction.
	unsigned char Sub(unsigned char src, unsigned char dst){ return Sub1[src][dst]; };
	unsigned char Light(unsigned char src, unsigned char shade){ return Light1[shade][src]; };	//Assumes you will usually light a whole area to one shade, better cache coherency.
	bool MakeLookup(PALETTEENTRY *pe, bool disk = false);
	void BestColorInit(PALETTEENTRY *pe);
	UCHAR BestColor(PALETTEENTRY *pe, UCHAR r, UCHAR g, UCHAR b);
		//Set disk to TRUE to load in old table (if palette matches) and save out table.
};

//Note because of static variable, only one Reduce(int) or GetPalette() call can be going on at a time, anywhere!
//Ok, for sucking out the palette after reduction, you iterate through and copy leaves'
//colors (RtGtBt / Count) to next pep stop, and divide RtGtBt by Count and set it to 1.
//Then (after external sorting, say) to reassociate the octree to the sorted palette
//entries, feed them back in...
//This is silly!  Sort and remap AFTER making the 256 color image!  Ack!  Ok, so just
//store the index in the node (and all sub-nodes) as the palette is being pulled out.
class ColorOctree{
private:
	static int ColorCount, ColorIndex;
	static PALETTEENTRY *pep;
	ColorOctree *Prev;
	ColorOctree *Next[8];
	PALETTEENTRY Split;	//The splitting plane for this node.  Root is 128,128,128.
	int Size;	//The size of this node's children.  Root should have a Size of 128, the final level should have a break of 1.  Break / 2 is how much to add to the node's center when spawning shildren.
	int Rt, Gt, Bt;	//Color value totals for this node, combination of all children that were reduced into this node.
	int Count;	//Count of children concatenated, or 1 for a first time leaf node.  If 0, we are not a leaf.
	int Index;	//Color table index for inverse palette use.
	int Octant(PALETTEENTRY col);
	PALETTEENTRY SplitPlane(int octant);
	void Reduce(int size, int cols);	//Recursively reduces only those nodes with sizes smaller than size.
	void GetPalette(int index);
	void Clear();
public:
	ColorOctree();	//Creates a root node, 128,128,128.
	ColorOctree(ColorOctree *prev, PALETTEENTRY split, int size);	//Creates a child node with splitting plane split, and child size of size.
	~ColorOctree();
	void DeleteTree();	//Deletes all children recursively, DOES NOT TOUCH this node.
	ColorOctree *Add(PALETTEENTRY col);	//Adds color col to tree.  If split and col match, becomes leaf node, otherwise passes on to children or makes new kids.
	int CountLeaves();	//Recursively counts the color leaves in the tree, NOT all the total nodes.
	int Reduce(int cols);	//Only call on root node, reduces tree to only cols leaves.
	bool GetPalette(PALETTEENTRY *pe, int cols = 256);
	int LookupIndex(PALETTEENTRY pe);	//You can only look up exact colors that were put in the tree previously!
};

//#define MAXCOLS 10240

class Quantizer{
private:
//	PALETTEENTRY realpal1[MAXCOLS];
//	PALETTEENTRY realpal2[MAXCOLS];
//	PALETTEENTRY *pal1;
//	PALETTEENTRY *pal2;
//	int npal1, npal2;
	ColorOctree octree;
public:
	Quantizer();
	~Quantizer();
	bool ClearPalette();
	bool AddPalette(PALETTEENTRY *pe, int NumCols, int NumShades = 0);
	bool GetCompressedPalette(PALETTEENTRY *pe, int NumCols);
};

#endif
