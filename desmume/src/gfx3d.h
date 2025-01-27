/*
	Copyright (C) 2006 yopyop
	Copyright (C) 2008-2023 DeSmuME team

	This file is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 2 of the License, or
	(at your option) any later version.

	This file is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with the this software.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _GFX3D_H_
#define _GFX3D_H_

#include <iosfwd>
#include <ostream>
#include <istream>

#include "types.h"
#include "matrix.h"
#include "GPU.h"

class EMUFILE;

//geometry engine command numbers
#define GFX3D_NOP 0x00
#define GFX3D_MTX_MODE 0x10
#define GFX3D_MTX_PUSH 0x11
#define GFX3D_MTX_POP 0x12
#define GFX3D_MTX_STORE 0x13
#define GFX3D_MTX_RESTORE 0x14
#define GFX3D_MTX_IDENTITY 0x15
#define GFX3D_MTX_LOAD_4x4 0x16
#define GFX3D_MTX_LOAD_4x3 0x17
#define GFX3D_MTX_MULT_4x4 0x18
#define GFX3D_MTX_MULT_4x3 0x19
#define GFX3D_MTX_MULT_3x3 0x1A
#define GFX3D_MTX_SCALE 0x1B
#define GFX3D_MTX_TRANS 0x1C
#define GFX3D_COLOR 0x20
#define GFX3D_NORMAL 0x21
#define GFX3D_TEXCOORD 0x22
#define GFX3D_VTX_16 0x23
#define GFX3D_VTX_10 0x24
#define GFX3D_XY 0x25
#define GFX3D_XZ 0x26
#define GFX3D_YZ 0x27
#define GFX3D_DIFF 0x28
#define GFX3D_POLYGON_ATTR 0x29
#define GFX3D_TEXIMAGE_PARAM 0x2A
#define GFX3D_PLTT_BASE 0x2B
#define GFX3D_DIF_AMB 0x30
#define GFX3D_SPE_EMI 0x31
#define GFX3D_LIGHT_VECTOR 0x32
#define GFX3D_LIGHT_COLOR 0x33
#define GFX3D_SHININESS 0x34
#define GFX3D_BEGIN_VTXS 0x40
#define GFX3D_END_VTXS 0x41
#define GFX3D_SWAP_BUFFERS 0x50
#define GFX3D_VIEWPORT 0x60
#define GFX3D_BOX_TEST 0x70
#define GFX3D_POS_TEST 0x71
#define GFX3D_VEC_TEST 0x72
#define GFX3D_NOP_NOARG_HACK 0xDD

#define GFX3D_5TO6(x) ((x)?(((x)<<1)+1):0)
#define GFX3D_5TO6_LOOKUP(x) (material_5bit_to_6bit[(x)])

// 15-bit to 24-bit depth formula from http://nocash.emubase.de/gbatek.htm#ds3drearplane
extern CACHE_ALIGN u32 dsDepthExtend_15bit_to_24bit[32768];
#define DS_DEPTH15TO24(depth) ( dsDepthExtend_15bit_to_24bit[(depth) & 0x7FFF] )

extern CACHE_ALIGN NDSMatrixStack1  mtxStackProjection;
extern CACHE_ALIGN NDSMatrixStack32 mtxStackPosition;
extern CACHE_ALIGN NDSMatrixStack32 mtxStackPositionVector;
extern CACHE_ALIGN NDSMatrixStack1  mtxStackTexture;

extern u32 mtxStackIndex[4];

// POLYGON PRIMITIVE TYPES
enum PolygonPrimitiveType
{
	GFX3D_TRIANGLES				= 0,
	GFX3D_QUADS					   = 1,
	GFX3D_TRIANGLE_STRIP		   = 2,
	GFX3D_QUAD_STRIP			   = 3,
	GFX3D_TRIANGLES_LINE		   = 4,
	GFX3D_QUADS_LINE			   = 5,
	GFX3D_TRIANGLE_STRIP_LINE	= 6,
	GFX3D_QUAD_STRIP_LINE		= 7
};

// POLYGON MODES
enum PolygonMode
{
	POLYGON_MODE_MODULATE		= 0,
	POLYGON_MODE_DECAL			= 1,
	POLYGON_MODE_TOONHIGHLIGHT	= 2,
	POLYGON_MODE_SHADOW			= 3
};

// POLYGON TYPES
enum PolygonType
{
	POLYGON_TYPE_TRIANGLE	= 3,
	POLYGON_TYPE_QUAD		   = 4
};

// TEXTURE PARAMETERS - FORMAT ID
enum NDSTextureFormat
{
	TEXMODE_NONE								= 0,
	TEXMODE_A3I5								= 1,
	TEXMODE_I2									= 2,
	TEXMODE_I4									= 3,
	TEXMODE_I8									= 4,
	TEXMODE_4X4									= 5,
	TEXMODE_A5I3								= 6,
	TEXMODE_16BPP								= 7
};

enum TextureTransformationMode
{
	TextureTransformationMode_None				= 0,
	TextureTransformationMode_TexCoordSource	= 1,
	TextureTransformationMode_NormalSource		= 2,
	TextureTransformationMode_VertexSource		= 3
};

enum PolygonShadingMode
{
	PolygonShadingMode_Toon						= 0,
	PolygonShadingMode_Highlight				= 1
};

void gfx3d_init();
void gfx3d_deinit();
void gfx3d_reset();

typedef union
{
	u16 value;
	
	struct
	{
#ifndef MSB_FIRST
		u8 XOffset;
		u8 YOffset;
#else
		u8 YOffset;
		u8 XOffset;
#endif
	};
} IOREG_CLRIMAGE_OFFSET;

typedef union
{
	u8 cmd[4];								//  0- 7: Unpacked command OR packed command #1
											//  8-15: Packed command #2
											// 16-23: Packed command #3
											// 24-31: Packed command #4
	
	u32 command;							// 8-bit unpacked command
	u32 param;								// Parameter(s) for previous command(s)
	
} IOREG_GXFIFO;								// 0x04000400: Geometry command/parameter sent to FIFO

typedef union
{
#ifndef MSB_FIRST
		u8 MtxMode:2;						//  0- 1: Set matrix mode;
											//        0=Projection
											//        1=Position
											//        2=Position+Vector
											//        3=Texture
		u8 :6;								//  2- 7: Unused bits
		
		u32 :24;							//  8-31: Unused bits
#else
		u8 :6;								//  2- 7: Unused bits
		u8 MtxMode:2;						//  0- 1: Set matrix mode;
											//        0=Projection
											//        1=Position
											//        2=Position+Vector
											//        3=Texture

		u32 :24;							//  8-31: Unused bits
#endif
} IOREG_MTX_MODE;							// 0x04000440: MTX_MODE command port

typedef union
{
	u32 value;
	
	struct
	{
#ifndef MSB_FIRST
		u8 Light0:1;						//     0: Light 0; 0=Disable, 1=Enable
		u8 Light1:1;						//     1: Light 1; 0=Disable, 1=Enable
		u8 Light2:1;						//     2: Light 2; 0=Disable, 1=Enable
		u8 Light3:1;						//     3: Light 3; 0=Disable, 1=Enable
		u8 Mode:2;							//  4- 5: Polygon mode;
											//        0=Modulate
											//        1=Decal
											//        2=Toon/Highlight
											//        3=Shadow
		u8 BackSurface:1;					//     6: Back surface; 0=Hide, 1=Render
		u8 FrontSurface:1;					//     7: Front surface; 0=Hide, 1=Render
		
		u8 :3;								//  8-10: Unused bits
		u8 TranslucentDepthWrite_Enable:1;	//    11: Translucent depth write; 0=Keep 1=Replace
		u8 FarPlaneIntersect_Enable:1;		//    12: Far-plane intersecting polygons; 0=Hide, 1=Render/clipped
		u8 OneDotPolygons_Enable:1;			//    13: One-dot polygons; 0=Hide, 1=Render
		u8 DepthEqualTest_Enable:1;			//    14: Depth test mode; 0=Less, 1=Equal
		u8 Fog_Enable:1;					//    15: Fog; 0=Disable, 1=Enable
		
		u8 Alpha:5;							// 16-20: Alpha value
		u8 :3;								// 21-23: Unused bits
		
		u8 PolygonID:6;						// 24-29: Polygon ID
		u8 :2;								// 30-31: Unused bits
#else
		u8 :2;								// 30-31: Unused bits
		u8 PolygonID:6;						// 24-29: Polygon ID
		
		u8 :3;								// 21-23: Unused bits
		u8 Alpha:5;							// 16-20: Alpha value
		
		u8 Fog_Enable:1;					//    15: Fog; 0=Disable, 1=Enable
		u8 DepthEqualTest_Enable:1;			//    14: Depth test mode; 0=Less, 1=Equal
		u8 OneDotPolygons_Enable:1;			//    13: One-dot polygons; 0=Hide, 1=Render
		u8 FarPlaneIntersect_Enable:1;		//    12: Far-plane intersecting polygons; 0=Hide, 1=Render/clipped
		u8 TranslucentDepthWrite_Enable:1;	//    11: Translucent depth write; 0=Keep 1=Replace
		u8 :3;								//  8-10: Unused bits
		
		u8 FrontSurface:1;					//     7: Front surface; 0=Hide, 1=Render
		u8 BackSurface:1;					//     6: Back surface; 0=Hide, 1=Render
		u8 Mode:2;							//  4- 5: Polygon mode;
											//        0=Modulate
											//        1=Decal
											//        2=Toon/Highlight
											//        3=Shadow
		u8 Light3:1;						//     3: Light 3; 0=Disable, 1=Enable
		u8 Light2:1;						//     2: Light 2; 0=Disable, 1=Enable
		u8 Light1:1;						//     1: Light 1; 0=Disable, 1=Enable
		u8 Light0:1;						//     0: Light 0; 0=Disable, 1=Enable
#endif
	};
	
	struct
	{
#ifndef MSB_FIRST
		u8 LightMask:4;						//  0- 3: Light enable mask
		u8 :2;
		u8 SurfaceCullingMode:2;			//  6- 7: Surface culling mode;
											//        0=Cull front and back
											//        1=Cull front
											//        2=Cull back
											//        3=No culling
		u8 :8;
		u8 :8;
		u8 :8;
#else
		u8 :8;
		u8 :8;
		u8 :8;
		
		u8 SurfaceCullingMode:2;			//  6- 7: Surface culling mode;
											//        0=Cull front and back
											//        1=Cull front
											//        2=Cull back
											//        3=No culling
		u8 :2;
		u8 LightMask:4;						//  0- 3: Light enable mask
#endif
	};
} POLYGON_ATTR;								// 0x040004A4: POLYGON_ATTR command port

typedef union
{
	u32 value;
	
	struct
	{
#ifndef MSB_FIRST
		u16 VRAMOffset:16;					//  0-15: VRAM offset address
		
		u16 RepeatS_Enable:1;				//    16: Repeat for S-coordinate; 0=Clamp 1=Repeat
		u16 RepeatT_Enable:1;				//    17: Repeat for T-coordinate; 0=Clamp 1=Repeat
		u16 MirroredRepeatS_Enable:1;		//    18: Mirrored repeat for S-coordinate, interacts with bit 16; 0=Disable 1=Enable
		u16 MirroredRepeatT_Enable:1;		//    19: Mirrored repeat for T-coordinate, interacts with bit 17; 0=Disable 1=Enable
		u16 SizeShiftS:3;					// 20-22: Texel size shift for S-coordinate; 0...7, where the actual texel size is (8 << N)
		u16 SizeShiftT:3;					// 23-25: Texel size shift for T-coordinate; 0...7, where the actual texel size is (8 << N)
		u16 PackedFormat:3;					// 26-28: Packed texture format;
											//        0=None
											//        1=A3I5, 5-bit indexed color (32-color palette) with 3-bit alpha (0...7, where 0=Fully Transparent and 7=Opaque)
											//        2=I2, 2-bit indexed color (4-color palette)
											//        3=I4, 4-bit indexed color (16-color palette)
											//        4=I8, 8-bit indexed color (256-color palette)
											//        5=4x4-texel compressed
											//        6=A5I3, 3-bit indexed color (8-color palette) with 5-bit alpha (0...31, where 0=Fully Transparent and 31=Opaque)
											//        7=Direct 16-bit color
		u16 KeyColor0_Enable:1;				//    29: Use palette color 0 as transparent; 0=Displayed 1=Transparent
		u16 TexCoordTransformMode:2;		// 30-31: Texture coordinate transformation mode;
											//        0=No transformation
											//        1=TexCoord source
											//        2=Normal source
											//        3=Vertex source
#else
		u16 TexCoordTransformMode:2;		// 30-31: Texture coordinate transformation mode;
											//        0=No transformation
											//        1=TexCoord source
											//        2=Normal source
											//        3=Vertex source
		u16 KeyColor0_Enable:1;				//    29: Use palette color 0 as transparent; 0=Displayed 1=Transparent
		u16 PackedFormat:3;					// 26-28: Packed texture format;
											//        0=None
											//        1=A3I5, 5-bit indexed color (32-color palette) with 3-bit alpha (0...7, where 0=Fully Transparent and 7=Opaque)
											//        2=I2, 2-bit indexed color (4-color palette)
											//        3=I4, 4-bit indexed color (16-color palette)
											//        4=I8, 8-bit indexed color (256-color palette)
											//        5=4x4-texel compressed
											//        6=A5I3, 3-bit indexed color (8-color palette) with 5-bit alpha (0...31, where 0=Fully Transparent and 31=Opaque)
											//        7=Direct 16-bit color
		u16 SizeShiftT:3;					// 23-25: Texel size shift for T-coordinate; 0...7, where the actual texel size is (8 << N)
		u16 SizeShiftS:3;					// 20-22: Texel size shift for S-coordinate; 0...7, where the actual texel size is (8 << N)
		u16 MirroredRepeatT_Enable:1;		//    19: Mirrored repeat for T-coordinate, interacts with bit 17; 0=Disable 1=Enable
		u16 MirroredRepeatS_Enable:1;		//    18: Mirrored repeat for S-coordinate, interacts with bit 16; 0=Disable 1=Enable
		u16 RepeatT_Enable:1;				//    17: Repeat for T-coordinate; 0=Clamp 1=Repeat
		u16 RepeatS_Enable:1;				//    16: Repeat for S-coordinate; 0=Clamp 1=Repeat
		
		u16 VRAMOffset:16;					//  0-15: VRAM offset address
#endif
	};
	
	struct
	{
#ifndef MSB_FIRST
		u16 :16;
		u16 TextureWrapMode:4;				// 16-19: Texture wrap mode for repeat and mirrored repeat
		u16 :12;
#else
		u16 :12;
		u16 TextureWrapMode:4;				// 16-19: Texture wrap mode for repeat and mirrored repeat
		u16 :16;
#endif
	};
} TEXIMAGE_PARAM;							// 0x040004A8: TEXIMAGE_PARAM command port

typedef union
{
	u32 value;
	
	struct
	{
#ifndef MSB_FIRST
		u8 YSortMode:1;						//     0: Translucent polygon Y-sorting mode; 0=Auto-sort, 1=Manual-sort
		u8 DepthMode:1;						//     1: Depth buffering select; 0=Z 1=W
		u8 :6;								//  2- 7: Unused bits
		
		u32 :24;							//  8-31: Unused bits
#else
		u8 :6;								//  2- 7: Unused bits
		u8 DepthMode:1;						//     1: Depth buffering select; 0=Z 1=W
		u8 YSortMode:1;						//     0: Translucent polygon Y-sorting mode; 0=Auto-sort, 1=Manual-sort
		
		u32 :24;							//  8-31: Unused bits
#endif
	};
} IOREG_SWAP_BUFFERS;						// 0x04000540: SWAP_BUFFERS command port

typedef union
{
	u32 value;
	
	struct
	{
		// Coordinate (0,0) represents the bottom-left of the screen.
		// Coordinate (255,191) represents the top-right of the screen.
		u8 X1;								//  0- 7: First X-coordinate; 0...255
		u8 Y1;								//  8-15: First Y-coordinate; 0...191
		u8 X2;								// 16-23: Second X-coordinate; 0...255
		u8 Y2;								// 24-31: Second Y-coordinate; 0...191
	};
} IOREG_VIEWPORT;							// 0x04000580: VIEWPORT command port

typedef union
{
	u32 value;
	
	struct
	{
		u8 TestBusy:1;
		u8 BoxTestResult:1;
		u8 :6;
		
		u8 PosVecMtxStackLevel:5;
		u8 ProjMtxStackLevel:1;
		u8 MtxStackBusy:1;
		u8 AckMtxStackError:1;
		
		u16 CommandListCount:9;
		u8 CommandListLessThanHalf:1;
		u8 CommandListEmpty:1;
		u8 EngineBusy:1;
		u8 :2;
		u8 CommandListIRQ:2;
	};
	
} IOREG_GXSTAT;								// 0x04000600: Geometry engine status

typedef union
{
	u32 value;
	
	struct
	{
		u16 PolygonCount;					//  0-15: Number of polygons currently stored in polygon list RAM; 0...2048
		u16 VertexCount;					// 16-31: Number of vertices currently stored in vertex RAM; 0...6144
	};
} IOREG_RAM_COUNT;							// 0x04000604: Polygon list and vertex RAM count

struct GFX3D_IOREG
{
	u8 RDLINES_COUNT;						// 0x04000320
	u8 __unused1[15];
	u16 EDGE_COLOR[8];						// 0x04000330
	u8 ALPHA_TEST_REF;						// 0x04000340
	u8 __unused2[15];
	u32 CLEAR_COLOR;						// 0x04000350
	u16 CLEAR_DEPTH;						// 0x04000354
	IOREG_CLRIMAGE_OFFSET CLRIMAGE_OFFSET;	// 0x04000356
	u32 FOG_COLOR;							// 0x04000358
	u16 FOG_OFFSET;							// 0x0400035C
	u8 __unused3[2];
	u8 FOG_TABLE[32];						// 0x04000360
	u16 TOON_TABLE[32];						// 0x04000380
	u8 __unused4[64];
	
	IOREG_GXFIFO GXFIFO;					// 0x04000400
	u8 __unused5[60];
	
	// Geometry command ports
	u32 MTX_MODE;							// 0x04000440
	u32 MTX_PUSH;							// 0x04000444
	u32 MTX_POP;							// 0x04000448
	u32 MTX_STORE;							// 0x0400044C
	u32 MTX_RESTORE;						// 0x04000450
	u32 MTX_IDENTITY;						// 0x04000454
	u32 MTX_LOAD_4x4;						// 0x04000458
	u32 MTX_LOAD_4x3;						// 0x0400045C
	u32 MTX_MULT_4x4;						// 0x04000460
	u32 MTX_MULT_4x3;						// 0x04000464
	u32 MTX_MULT_3x3;						// 0x04000468
	u32 MTX_SCALE;							// 0x0400046C
	u32 MTX_TRANS;							// 0x04000470
	u8 __unused6[12];
	u32 COLOR;								// 0x04000480
	u32 NORMAL;								// 0x04000484
	u32 TEXCOORD;							// 0x04000488
	u32 VTX_16;								// 0x0400048C
	u32 VTX_10;								// 0x04000490
	u32 VTX_XY;								// 0x04000494
	u32 VTX_XZ;								// 0x04000498
	u32 VTX_YZ;								// 0x0400049C
	u32 VTX_DIFF;							// 0x040004A0
	u32 POLYGON_ATTR;						// 0x040004A4
	u32 TEXIMAGE_PARAM;						// 0x040004A8
	u32 PLTT_BASE;							// 0x040004AC
	u8 __unused7[16];
	u32 DIF_AMB;							// 0x040004C0
	u32 SPE_EMI;							// 0x040004C4
	u32 LIGHT_VECTOR;						// 0x040004C8
	u32 LIGHT_COLOR;						// 0x040004CC
	u32 SHININESS;							// 0x040004D0
	u8 __unused8[44];
	u32 BEGIN_VTXS;							// 0x04000500
	u32 END_VTXS;							// 0x04000504
	u8 __unused9[56];
	IOREG_SWAP_BUFFERS SWAP_BUFFERS;		// 0x04000540
	u8 __unused10[60];
	IOREG_VIEWPORT VIEWPORT;				// 0x04000580
	u8 __unused11[60];
	u32 BOX_TEST;							// 0x040005C0
	u32 POS_TEST;							// 0x040005C4
	u32 VEC_TEST;							// 0x040005C8
	u8 __unused12[52];
	
	IOREG_GXSTAT GXSTAT;					// 0x04000600
	IOREG_RAM_COUNT RAM_COUNT;				// 0x04000604
	u8 __unused13[8];
	u16 DISP_1DOT_DEPTH;					// 0x04000610
	u8 __unused14[14];
	u32 POS_RESULT[4];						// 0x04000620
	u16 VEC_RESULT[3];						// 0x04000630
	u8 __unused15[10];
	u8 CLIPMTX_RESULT[64];					// 0x04000640
	u8 VECMTX_RESULT[36];					// 0x04000680
};
typedef struct GFX3D_IOREG GFX3D_IOREG; // 0x04000320 - 0x040006A4

union GFX3D_Viewport
{
	u64 value;
	
	struct
	{
		s16 x;
		s16 y;
		u16 width;
		u16 height;
	};
};
typedef union GFX3D_Viewport GFX3D_Viewport;

struct POLY
{
	PolygonType type; //tri or quad
	PolygonPrimitiveType vtxFormat;
	u16 vertIndexes[4]; //up to four verts can be referenced by this poly
	
	POLYGON_ATTR attribute;
	TEXIMAGE_PARAM texParam;
	u32 texPalette; //the hardware rendering params
	GFX3D_Viewport viewport;
	IOREG_VIEWPORT viewportLegacySave; // Exists for save state compatibility.
	
	float miny;
	float maxy;
};
typedef struct POLY POLY;

// TODO: Handle these polygon utility functions in a class rather than as standalone functions.
// Most likely, the class will be some kind of polygon processing class, such as a polygon list
// handler or a polygon clipping handler. But before such a class is designed, simply handle
// these function here so that the POLY struct can remain as a POD struct.
bool GFX3D_IsPolyWireframe(const POLY &p);
bool GFX3D_IsPolyOpaque(const POLY &p);
bool GFX3D_IsPolyTranslucent(const POLY &p);

#define POLYLIST_SIZE 20000
#define VERTLIST_SIZE (POLYLIST_SIZE * 4)
#define INDEXLIST_SIZE (POLYLIST_SIZE * 4)

#include "PACKED.h"

// This struct is padded in such a way so that each component can be accessed with a 16-byte alignment.
struct VERT
{
	union
	{
		float coord[4];
		struct
		{
			float x, y, z, w;
		};
	};
	
	union
	{
		float texcoord[4];
		struct
		{
			float u, v, tcPad2, tcPad3;
		};
	};
	
	union
	{
		float fcolor[4];
		struct
		{
			float rf, gf, bf, af; // The alpha value is unused and only exists for padding purposes.
		};
	};
	
	union
	{
		u32 color32;
		u8 color[4];
		
		struct
		{
			u8 r, g, b, a; // The alpha value is unused and only exists for padding purposes.
		};
	};
	
	u8 padFinal[12]; // Final padding to bring the struct to exactly 64 bytes.
};
typedef struct VERT VERT;

#include "PACKED_END.h"

union VtxCoord32
{
	s32 coord[4];
	struct
	{
		s32 x, y, z, w;
	};
};
typedef union VtxCoord32 VtxCoord32;

union VtxTexCoord16
{
	s16 coord[2];
	struct
	{
		s16 u, v;
	};
};
typedef union VtxTexCoord16 VtxTexCoord16;

//ok, imagine the plane that cuts diagonally across a cube such that it clips
//out to be a hexagon. within that plane, draw a quad such that it cuts off
//four corners of the hexagon, and you will observe a decagon
#define MAX_CLIPPED_VERTS 10

enum ClipperMode
{
	ClipperMode_DetermineClipOnly = 0,		// Retains only the pointer to the original polygon info. All other information in CPoly is considered undefined.
	ClipperMode_Full = 1,					// Retains all of the modified polygon's info in CPoly, including the clipped vertex info.
	ClipperMode_FullColorInterpolate = 2	// Same as ClipperMode_Full, but the vertex color attribute is better interpolated.
};

struct CPoly
{
	u16 index; // The index number of this polygon in the full polygon list.
	PolygonType type; //otherwise known as "count" of verts
	POLY *poly;
	VERT clipVerts[MAX_CLIPPED_VERTS];
};
typedef struct CPoly CPoly;

class GFX3D_Clipper
{
protected:
	size_t _clippedPolyCounter;
	CPoly *_clippedPolyList; // The output of clipping operations goes into here. Be sure you init it before clipping!
	
public:
	GFX3D_Clipper();
	
	const CPoly* GetClippedPolyBufferPtr();
	void SetClippedPolyBufferPtr(CPoly *bufferPtr);
	
	const CPoly& GetClippedPolyByIndex(size_t index) const;
	size_t GetPolyCount() const;
	
	void Reset();
	template<ClipperMode CLIPPERMODE> bool ClipPoly(const u16 polyIndex, const POLY &poly, const VERT **verts); // the entry point for poly clipping
};

//used to communicate state to the renderer
struct GFX3D_State
{
	IOREG_DISP3DCNT DISP3DCNT;
	u8 fogShift;
	
	u8 alphaTestRef;
	u32 clearColor; // Not an RGBA8888 color. This uses its own packed format.
	u32 clearDepth;
	IOREG_CLRIMAGE_OFFSET clearImageOffset;
	u32 fogColor; // Not an RGBA8888 color. This uses its own packed format.
	u16 fogOffset;
	
	IOREG_SWAP_BUFFERS SWAP_BUFFERS;
	
	CACHE_ALIGN u16 edgeMarkColorTable[8];
	CACHE_ALIGN u8 fogDensityTable[32];
	CACHE_ALIGN u16 toonTable16[32];
	CACHE_ALIGN u8 shininessTable[128];
};
typedef struct GFX3D_State GFX3D_State;

struct GFX3D_GeometryList
{
	PAGE_ALIGN VERT vertList[VERTLIST_SIZE];
	PAGE_ALIGN POLY polyList[POLYLIST_SIZE];
	PAGE_ALIGN CPoly clippedPolyList[POLYLIST_SIZE];
	
	size_t vertListCount;
	size_t polyCount;
	size_t polyOpaqueCount;
	size_t clippedPolyCount;
	size_t clippedPolyOpaqueCount;
};
typedef struct GFX3D_GeometryList GFX3D_GeometryList;

struct LegacyGFX3DStateSFormat
{
	u32 enableTexturing;
	u32 enableAlphaTest;
	u32 enableAlphaBlending;
	u32 enableAntialiasing;
	u32 enableEdgeMarking;
	u32 enableClearImage;
	u32 enableFog;
	u32 enableFogAlphaOnly;
	
	u32 fogShift;
	
	u32 toonShadingMode;
	u32 enableWDepth;
	u32 polygonTransparentSortMode;
	u32 alphaTestRef;
	
	u32 clearColor;
	u32 clearDepth;
	
	u32 fogColor[4]; //for savestate compatibility as of 26-jul-09
	u32 fogOffset;
	
	u16 toonTable16[32];
	u8 shininessTable[128];
	
	u32 activeFlushCommand;
	u32 pendingFlushCommand;
};
typedef struct LegacyGFX3D_StateSFormat LegacyGFX3D_StateSFormat;

struct Viewer3D_State
{
	int frameNumber;
	GFX3D_State state;
	GFX3D_GeometryList gList;
	int indexList[INDEXLIST_SIZE];
};
typedef struct Viewer3D_State Viewer3D_State;

extern Viewer3D_State viewer3D;

struct GFX3D
{
	GFX3D_Viewport viewport;
	
	GFX3D_State pendingState;
	GFX3D_State appliedState;
	GFX3D_GeometryList gList[2];
	
	u8 pendingListIndex;
	u8 appliedListIndex;
	u32 render3DFrameCount; // Increments when gfx3d_doFlush() is called. Resets every 60 video frames.
	
	// Working lists for rendering.
	PAGE_ALIGN int polyWorkingIndexList[INDEXLIST_SIZE];
	
	// Everything below is for save state compatibility.
	IOREG_VIEWPORT viewportLegacySave; // Historically, the viewport was stored as its raw register value.
	float PTcoordsLegacySave[4]; // Historically, PTcoords were stored as floating point values, not as integers.
	PAGE_ALIGN FragmentColor framebufferNativeSave[GPU_FRAMEBUFFER_NATIVE_WIDTH * GPU_FRAMEBUFFER_NATIVE_HEIGHT]; // Rendered 3D framebuffer that is saved in RGBA8888 color format at the native size.
};
typedef struct GFX3D GFX3D;

extern GFX3D gfx3d;

//---------------------

extern CACHE_ALIGN u32 dsDepthExtend_15bit_to_24bit[32768];

extern u32 isSwapBuffers;

void gfx3d_glFlush(u32 v);
// end GE commands

void gfx3d_glFogColor(u32 v);
void gfx3d_glFogOffset (u32 v);
template<typename T, size_t ADDROFFSET> void gfx3d_glClearDepth(const T val);
template<typename T, size_t ADDROFFSET> void gfx3d_glClearImageOffset(const T val);
void gfx3d_glSwapScreen(u32 screen);
u32 gfx3d_GetNumPolys();
u32 gfx3d_GetNumVertex();
template<typename T> void gfx3d_UpdateEdgeMarkColorTable(const u8 offset, const T val);
template<typename T> void gfx3d_UpdateFogTable(const u8 offset, const T val);
template<typename T> void gfx3d_UpdateToonTable(const u8 offset, const T val);
s32 gfx3d_GetClipMatrix (const u32 index);
s32 gfx3d_GetDirectionalMatrix(const u32 index);
void gfx3d_glAlphaFunc(u32 v);
u32 gfx3d_glGetPosRes(const size_t index);
u16 gfx3d_glGetVecRes(const size_t index);
void gfx3d_VBlankSignal();
void gfx3d_VBlankEndSignal(bool skipFrame);
void gfx3d_execute3D();
void gfx3d_sendCommandToFIFO(u32 val);
void gfx3d_sendCommand(u32 cmd, u32 param);

//other misc stuff
template<MatrixMode MODE> void gfx3d_glGetMatrix(const int index, float (&dst)[16]);
void gfx3d_glGetLightDirection(const size_t index, u32 &dst);
void gfx3d_glGetLightColor(const size_t index, u32 &dst);

struct SFORMAT;
extern SFORMAT SF_GFX3D[];
void gfx3d_PrepareSaveStateBufferWrite();
void gfx3d_savestate(EMUFILE &os);
bool gfx3d_loadstate(EMUFILE &is, int size);
void gfx3d_FinishLoadStateBufferRead();

void gfx3d_ClearStack();

void gfx3d_parseCurrentDISP3DCNT();
const GFX3D_IOREG& GFX3D_GetIORegisterMap();
void ParseReg_DISP3DCNT();

#endif //_GFX3D_H_
