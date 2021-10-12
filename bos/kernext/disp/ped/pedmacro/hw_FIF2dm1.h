/* @(#)07	1.6.1.3  src/bos/kernext/disp/ped/pedmacro/hw_FIF2dm1.h, pedmacro, bos411, 9428A410j 3/17/93 19:43:36 */
/*
 *   COMPONENT_NAME: PEDMACRO
 *
 *   FUNCTIONS: 
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1990,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */



/****************************************************************/
/*								*/
/*	PEDERNALES HW MACRO PROGRAMMING INTERFACE 		*/
/*								*/
/****************************************************************/

#ifndef	_H_MID_HW_FIFO_2DM1
#define	_H_MID_HW_FIFO_2DM1

#include <hw_se_types.h>
#include <hw_seops.h>
#include <hw_2dm1con.h>

/************************************************************************/
/* GAI 2DM1 FIFO COMMAND ELEMENT TYPEDEFS				*/
/*									*/
/*	The names of the members of the structures described below	*/
/*	are derived from the Pedernales Software Interface		*/
/*	Specification v. 0.7 						*/
/*									*/
/*									*/
/************************************************************************/

        /*----------------------------------------------*
	* Check if the header has already been defined	*
	*-----------------------------------------------*/

#ifndef	_H_MID_SE_HDR
#define	_H_MID_SE_HDR

        /*----------------------------------------------*
	 * definition for the header information of	*
	 * each structure element			*
	*-----------------------------------------------*/

typedef union   _MIDfifohdr
{
        ulong    word;
        struct  _halfword
        {
                ushort  length;         /* length of SE in bytes */
                ushort  op;             /* opcode for SE */
        } halfword;
} MIDfifohdr;

#endif	/* _H_MID_SE_HDR */

        /*----------------------------------------------*
	 * the section below defines a FIFO command 	*
	 * element typedef for the set of FIFO commands	*
	 * which are of a fixed length and whose fields	*
	 * are all regularly defined			*
	*-----------------------------------------------*/


/*
 * PEDERNALES SetForegroundPixel command packet structure
 */
typedef struct _MIDSetForegroundPixel
{
	MIDfifohdr              hdr;           /* SE header info (len/opcode) */
	unsigned long           fgpixel;
}	MIDSetForegroundPixel,
	*pMIDSetForegroundPixel;

/*
 * PEDERNALES SetBackgroundPixel command packet structure
 */
typedef struct _MIDSetBackgroundPixel
{
	MIDfifohdr              hdr;           /* SE header info (len/opcode) */
	unsigned long           bgpixel;
}	MIDSetBackgroundPixel,
	*pMIDSetBackgroundPixel;

/*
 * PEDERNALES SetLineAttributes command packet structure
 */
typedef struct _MIDSetLineAttributes
{
	MIDfifohdr              hdr;           /* SE header info (len/opcode) */
	unsigned long           uflags;
	long                    line_width;
	long                    line_style;
	long                    cap_style;
}	MIDSetLineAttributes,
	*pMIDSetLineAttributes;

/*
 * PEDERNALES SetFillAttributes command packet structure
 */
typedef struct _MIDSetFillAttributes
{
	MIDfifohdr              hdr;           /* SE header info (len/opcode) */
	unsigned long           uflags;
	long                    fill_style;
	long                    fill_rule;
	long                    arc_mode;
}	MIDSetFillAttributes,
	*pMIDSetFillAttributes;

/*
 * PEDERNALES DefineLinePattern command packet structure
 */
typedef struct _MIDDefineLinePattern
{
	MIDfifohdr              hdr;           /* SE header info (len/opcode) */
	unsigned long           patdata;
	unsigned long           patrepl;
}       MIDDefineLinePattern,
	*pMIDDefineLinePattern;

/*
 * PEDERNALES SetActiveTile command packet structure
 */
typedef struct _MIDSetActiveTile
{
	MIDfifohdr              hdr;           /* SE header info (len/opcode) */
	unsigned short          resv;
	unsigned short          tl_id;
}	MIDSetActiveTile,
	*pMIDSetActiveTile;

/*
 * PEDERNALES SetActiveStipple command packet structure
 */
typedef struct _MIDSetActiveStipple
{
	MIDfifohdr              hdr;           /* SE header info (len/opcode) */
	unsigned short          resv;
	unsigned short          st_id;
}	MIDSetActiveStipple,
	*pMIDSetActiveStipple;

/*
 * PEDERNALES SetPatternOrigin command packet structure
 */
typedef struct _MIDSetPatternOrigin
{
	MIDfifohdr              hdr;           /* SE header info (len/opcode) */
	short                   originx;
	short                   originy;
}	MIDSetPatternOrigin,
	*pMIDSetPatternOrigin;

/*
 * PEDERNALES Set2DColorMode command packet structure
 */
typedef struct _MIDSet2DColorMode
{
	MIDfifohdr              hdr;           /* SE header info (len/opcode) */
	unsigned long           color_mode;
}       MIDSet2DColorMode,
	*pMIDSet2DColorMode;


        /*----------------------------------------------*
	 * the section below defines a FIFO command 	*
	 * element typedef for the set of FIFO commands	*
	 * which are variable in length                 *
	*-----------------------------------------------*/

/*
 * PEDERNALES DefineTile command packet structure
 */
typedef struct _MIDDefineTile
{
	MIDfifohdr              hdr;            /* SE header info (len/opcode)*/
	unsigned short          resv;
	unsigned short          tl_id;
	short                   theight;
	short                   twidth;
}	MIDDefineTile,
	*pMIDDefineTile;

/*
 * PEDERNALES DefineStipple command packet structure
 */
typedef struct _MIDDefineStipple
{
	MIDfifohdr              hdr;            /* SE header info (len/opcode)*/
	unsigned short          resv;
	unsigned short          st_id;
	short                   sheight;
	short                   swidth;
}	MIDDefineStipple,
	*pMIDDefineStipple;

/*
 * PEDERNALES Polypoint command packet structure
 */
typedef struct _MIDPolypoint
{
	MIDfifohdr              hdr;            /* SE header info (len/opcode)*/
	unsigned long           mode;
}	MIDPolypoint,
	*pMIDPolypoint;

/*
 * PEDERNALES Polylines command packet structure
 */
typedef struct _MIDPolylines
{
	MIDfifohdr              hdr;            /* SE header info (len/opcode)*/
	unsigned long           mode;
}       MIDPolylines,
	*pMIDPolylines;

/*
 * PEDERNALES Polysegment command packet structure
 */
typedef struct _MIDPolysegment
{
	MIDfifohdr              hdr;            /* SE header info (len/opcode)*/
}	MIDPolysegment,
	*pMIDPolysegment;

/*
 * PEDERNALES Polyrectangle command packet structure
 */
typedef struct _MIDPolyrectangle
{
	MIDfifohdr              hdr;            /* SE header info (len/opcode)*/
}	MIDPolyrectangle,
	*pMIDPolyrectangle;

/*
 * PEDERNALES Polyarc command packet structure
 */
typedef struct _MIDPolyarc
{
	MIDfifohdr              hdr;            /* SE header info (len/opcode)*/
}	MIDPolyarc,
	*pMIDPolyarc;

/*
 * PEDERNALES FillPolygon command packet structure
 */
typedef struct _MIDFillPolygon
{
	MIDfifohdr              hdr;            /* SE header info (len/opcode)*/
	unsigned long           shape;
	unsigned long           mode;
}	MIDFillPolygon,
	*pMIDFillPolygon;

/*
 * PEDERNALES FillPolyrectangle command packet structure
 */
typedef struct _MIDFillPolyrectangle
{
	MIDfifohdr              hdr;            /* SE header info (len/opcode)*/
}	MIDFillPolyrectangle,
	*pMIDFillPolyrectangle;

/*
 * PEDERNALES FillPolyarc command packet structure
 */
typedef struct _MIDFillPolyarc
{
	MIDfifohdr              hdr;            /* SE header info (len/opcode)*/
}	MIDFillPolyarc,
	*pMIDFillPolyarc;

/*
 * PEDERNALES Polytext8 command packet structure
 */
typedef struct _MIDPolytext8
{
	MIDfifohdr              hdr;            /* SE header info (len/opcode)*/
	short                   x;              /* starting x */
	short                   y;              /* starting y */
	long                    ccount;         /* number of characters */
}       MIDPolytext8,
	*pMIDPolytext8;

/*
 * PEDERNALES Polytext16 command packet structure
 */
typedef struct _MIDPolytext16
{
	MIDfifohdr              hdr;            /* SE header info (len/opcode)*/
	short                   x;              /* starting x */
	short                   y;              /* starting y */
	long                    ccount;         /* number of characters */
}       MIDPolytext16,
	*pMIDPolytext16;

/*
 * PEDERNALES Imagetext8 command packet structure
 */
typedef struct _MIDImagetext8
{
	MIDfifohdr              hdr;            /* SE header info (len/opcode)*/
	short                   x;              /* starting x */
	short                   y;              /* starting y */
	long                    ccount;         /* number of characters */
}       MIDImagetext8,
	*pMIDImagetext8;

/*
 * PEDERNALES Imagetext16 command packet structure
 */
typedef struct _MIDImagetext16
{
	MIDfifohdr              hdr;            /* SE header info (len/opcode)*/
	short                   x;              /* starting x */
	short                   y;              /* starting y */
	long                    ccount;         /* number of characters */
}       MIDImagetext16,
	*pMIDImagetext16;

/*
 * PEDERNALES PushPixels command packet structure
 */
typedef struct _MIDPushPixels
{
	MIDfifohdr              hdr;            /* SE header info (len/opcode)*/
	short                   height;
	short                   width;
	short                   xorg;
	short                   yorg;
}       MIDPushPixels,
	*pMIDPushPixels;

/*
 * PEDERNALES FillSpans command packet structure
 */
typedef struct _MIDFillSpans
{
	MIDfifohdr              hdr;            /* SE header info (len/opcode)*/
}       MIDFillSpans,
	*pMIDFillSpans;


/************************************************************************/
/* MACROS TO BUILD GAI 2D STRUCTURE ELEMENTS				*/
/*									*/
/*	The names of the members of the structures described below	*/
/*	are derived from the Pedernales Software Interface		*/
/*      Specification v. 0.7 and v. 0.8                                 */
/*									*/
/*									*/
/************************************************************************/

/************************************************************************/
/*                                                                      */
/*  Macro Name:  MID_SetForegroundPixel                                 */
/*                                                                      */
/*  Function:                                                           */
/*    Issues the Pedernales Set Foreground Pixel command element to set */
/*    the foreground color.  This command element is always written to  */
/*    the deferral buffer.                                              */
/*                                                                      */
/*  Input:                                                              */
/*    color (unsigned long) - foreground pixel value:  in 8 bit color   */
/*                            mode the low order 8 bits are used, and   */
/*                            in 24 bit color mode the low order 24     */
/*                            bits are used                             */
/*                                                                      */
/*  Output:                                                             */
/*    None                                                              */
/*                                                                      */
/************************************************************************/

#define MID_SetForegroundPixel( color )                                 \
{                                                                       \
    MIDSetForegroundPixel       SE;                                     \
									\
    SE.hdr.halfword.length = sizeof(MIDSetForegroundPixel);             \
    SE.hdr.halfword.op     = OP_SET_FOREGROUND_PIXEL;			\
    SE.fgpixel = color;                                                 \
									\
    MID_WR_DEFERBUF(&SE, MID_WORDS(sizeof(MIDSetForegroundPixel) >> 2));\
									\
}                       /* End MID_SetForegroundPixel                */

/************************************************************************/
/*                                                                      */
/*  Macro Name:  MID_SetBackgroundPixel                                 */
/*                                                                      */
/*  Function:                                                           */
/*    Issues the Pedernales Set Background Pixel command element to set */
/*    the background color.  This command element is always written to  */
/*    the deferral buffer.                                              */
/*                                                                      */
/*  Input:                                                              */
/*    color (unsigned long) - background pixel value:  in 8 bit color   */
/*                            mode the low order 8 bits are used, and   */
/*                            in 24 bit color mode the low order 24     */
/*                            bits are used                             */
/*                                                                      */
/*  Output:                                                             */
/*    None                                                              */
/*                                                                      */
/************************************************************************/

#define MID_SetBackgroundPixel( color )                                 \
{                                                                       \
    MIDSetBackgroundPixel       SE;                                     \
									\
    SE.hdr.halfword.length = sizeof(MIDSetBackgroundPixel);             \
    SE.hdr.halfword.op     = OP_SET_BACKGROUND_PIXEL;			\
    SE.bgpixel = color;                                                 \
									\
    MID_WR_DEFERBUF(&SE, MID_WORDS(sizeof(MIDSetBackgroundPixel) >> 2));\
									\
}                       /* End MID_SetBackgroundPixel                */

/************************************************************************/
/*                                                                      */
/*  Macro Name:  MID_SetLineAttributes                                  */
/*                                                                      */
/*  Function:                                                           */
/*    Issues the Pedernales Set Line Attributes command element to set  */
/*    the line width, line style, and/or cap style.  This command       */
/*    element is always written to the deferral buffer.                 */
/*                                                                      */
/*  Input:                                                              */
/*    flags (unsigned long) - update flags:                             */
/*                            bits 31-16: reserved (all bits must be 0) */
/*                            bit 15:  update line width (yes=1, no=0)  */
/*                            bit 14:  update line style (yes=1, no=0)  */
/*                            bit 13:  update cap style  (yes=1, no=0)  */
/*                            bits 12-0:  reserved (all bits must be 0) */
/*    lw (unsigned short)   - line width in pixels (must be 0 or 1 if   */
/*                            line width is to be updated)              */
/*    ls (int)              - line style (must have the value LineSolid,*/
/*                            LineOnOffDash, or LineDoubleDash if line  */
/*                            style is to be updated)                   */
/*    cs (int)              - cap style (must have the value CapNotLast,*/
/*                            CapButt, CapRound, or CapProjecting if    */
/*                            cap style is to be updated)               */
/*                                                                      */
/*  Output:                                                             */
/*    None                                                              */
/*                                                                      */
/************************************************************************/

#define MID_SetLineAttributes( flags , lw , ls , cs )                   \
{                                                                       \
    MIDSetLineAttributes        SE;                                     \
									\
    SE.hdr.halfword.length = sizeof(MIDSetLineAttributes);              \
    SE.hdr.halfword.op     = OP_SET_LINE_ATTRIBUTES;			\
    SE.uflags     = flags;                                              \
    SE.line_width = lw;                                                 \
    SE.line_style = ls;                                                 \
    SE.cap_style  = cs;                                                 \
									\
    MID_WR_DEFERBUF(&SE, MID_WORDS(sizeof(MIDSetLineAttributes) >> 2)); \
									\
}                       /* End MID_SetLineAttributes                 */

/************************************************************************/
/*                                                                      */
/*  Macro Name:  MID_SetFillAttributes                                  */
/*                                                                      */
/*  Function:                                                           */
/*    Issues the Pedernales Set Fill Attributes command element to set  */
/*    the fill style, fill rule, and/or arc mode.  This command element */
/*    is always written to the deferral buffer.                         */
/*                                                                      */
/*  Input:                                                              */
/*    flags (unsigned long) - update flags:                             */
/*                            bits 31-16: reserved (all bits must be 0) */
/*                            bit 15:  update fill style (yes=1, no=0)  */
/*                            bit 14:  update fill rule  (yes=1, no=0)  */
/*                            bit 13:  update arc mode   (yes=1, no=0)  */
/*                            bits 12-0: reserved  (all bits must be 0) */
/*    fs (int)              - fill style (must have the value FillSolid,*/
/*                            FillTiled, FillStippled, or               */
/*                            FillOpaqueStippled if fill style is to be */
/*                            updated)                                  */
/*    fr (int)              - fill rule (must have the value            */
/*                            EvenOddRule or WindingRule if fill rule   */
/*                            is to be updated)                         */
/*    am (int)              - arc mode (must have the value ArcChord or */
/*                            ArcPieSlice if arc mode is to be updated) */
/*                                                                      */
/*  Output:                                                             */
/*    None                                                              */
/*                                                                      */
/************************************************************************/

#define MID_SetFillAttributes( flags , fs , fr , am )                   \
{                                                                       \
    MIDSetFillAttributes        SE;                                     \
									\
    SE.hdr.halfword.length = sizeof(MIDSetFillAttributes);              \
    SE.hdr.halfword.op     = OP_SET_FILL_ATTRIBUTES;			\
    SE.uflags     = flags;                                              \
    SE.fill_style = fs;                                                 \
    SE.fill_rule  = fr;                                                 \
    SE.arc_mode   = am;                                                 \
									\
    MID_WR_DEFERBUF(&SE, MID_WORDS(sizeof(MIDSetFillAttributes) >> 2)); \
									\
}                       /* End MID_SetFillAttributes                 */

/************************************************************************/
/*                                                                      */
/*  Macro Name:  MID_DefineLinePattern                                  */
/*                                                                      */
/*  Function:                                                           */
/*    Issues the Pedernales Define Line Pattern command element to set  */
/*    the pattern to be used for dashed lines.  This command element is */
/*    always written to the deferral buffer.                            */
/*                                                                      */
/*  Input:                                                              */
/*    pattern_data (unsigned long) - pattern data                       */
/*    count (unsigned long)        - pattern replication count (must    */
/*                                   be <= 63)                          */
/*                                                                      */
/*  Output:                                                             */
/*    None                                                              */
/*                                                                      */
/************************************************************************/

#define MID_DefineLinePattern( pattern_data, count )                    \
{                                                                       \
    MIDDefineLinePattern        SE;                                     \
									\
    SE.hdr.halfword.length = sizeof(MIDDefineLinePattern);              \
    SE.hdr.halfword.op     = OP_DEFINE_LINE_PATTERN;			\
    SE.patdata = pattern_data;                                          \
    SE.patrepl = count;                                                 \
									\
    MID_WR_DEFERBUF(&SE, MID_WORDS(sizeof(MIDDefineLinePattern) >> 2)); \
									\
}                       /* End MID_DefineLinePattern                 */

/************************************************************************/
/*                                                                      */
/*  Macro Name:  MID_SetActiveTile                                      */
/*                                                                      */
/*  Function:                                                           */
/*    Issues the Pedernales Set Active Tile command element to make one */
/*    of the tiles defined to the adapter active.  This command element */
/*    is always written to the deferral buffer.                         */
/*                                                                      */
/*  Input:                                                              */
/*    tile_id (unsigned short) - the ID of the tile to make active      */
/*                               (must be 4, 5, 6, or 7)                */
/*                                                                      */
/*  Output:                                                             */
/*    None                                                              */
/*                                                                      */
/************************************************************************/

#define MID_SetActiveTile( tile_id )                                    \
{                                                                       \
    MIDSetActiveTile    SE;                                             \
									\
    SE.hdr.halfword.length = sizeof(MIDSetActiveTile);                  \
    SE.hdr.halfword.op     = OP_SET_ACTIVE_TILE;			\
    SE.resv  = 0;                                                       \
    SE.tl_id = tile_id;                                                 \
									\
    MID_WR_DEFERBUF(&SE, MID_WORDS(sizeof(MIDSetActiveTile) >> 2));     \
									\
}                       /* End MID_SetActiveTile                     */

/************************************************************************/
/*                                                                      */
/*  Macro Name:  MID_SetActiveStipple                                   */
/*                                                                      */
/*  Function:                                                           */
/*    Issues the Pedernales Set Active Stipple command element to make  */
/*    one of the stipples defined to the adapter active.  This command  */
/*    element is always written to the deferral buffer.                 */
/*                                                                      */
/*  Input:                                                              */
/*    stipple_id (unsigned short) - the ID of the stipple to make       */
/*                                  active (must be 0, 1, 2, or 3)      */
/*                                                                      */
/*  Output:                                                             */
/*    None                                                              */
/*                                                                      */
/************************************************************************/

#define MID_SetActiveStipple( stipple_id )                              \
{                                                                       \
    MIDSetActiveStipple SE;                                             \
									\
    SE.hdr.halfword.length = sizeof(MIDSetActiveStipple);               \
    SE.hdr.halfword.op     = OP_SET_ACTIVE_STIPPLE;			\
    SE.resv  = 0;                                                       \
    SE.st_id = stipple_id;                                              \
									\
    MID_WR_DEFERBUF(&SE, MID_WORDS(sizeof(MIDSetActiveStipple) >> 2));  \
									\
}                       /* End MID_SetActiveStipple                  */

/************************************************************************/
/*                                                                      */
/*  Macro Name:  MID_SetPatternOrigin                                   */
/*                                                                      */
/*  Function:                                                           */
/*    Issues the Pedernales Set Pattern Origin command element to set   */
/*    the tile or stipple origin.  This command element is always       */
/*    written to the deferral buffer.                                   */
/*                                                                      */
/*  Input:                                                              */
/*    pat_origin (DDXPointRec) - the origin of the tile or stipple,     */
/*                               relative to the window origin          */
/*                                                                      */
/*  Output:                                                             */
/*    None                                                              */
/*                                                                      */
/************************************************************************/

#define MID_SetPatternOrigin( pat_origin )                              \
{                                                                       \
    MIDSetPatternOrigin         SE;                                     \
									\
    SE.hdr.halfword.length = sizeof(MIDSetPatternOrigin);               \
    SE.hdr.halfword.op     = OP_SET_PATTERN_ORIGIN;			\
    SE.originx = pat_origin.x;                                          \
    SE.originy = pat_origin.y;                                          \
									\
    MID_WR_DEFERBUF(&SE, MID_WORDS(sizeof(MIDSetPatternOrigin) >> 2));  \
									\
}                       /* End MID_SetPatternOrigin                  */

/************************************************************************/
/*                                                                      */
/*  Macro Name:  MID_Set2DColorMode                                     */
/*                                                                      */
/*  Function:                                                           */
/*    Issues the Pedernales Set 2D Color Mode command element to set    */
/*    the color processing mode.  This command element is always        */
/*    written to the deferral buffer.                                   */
/*                                                                      */
/*  Input:                                                              */
/*    mode (unsigned long) - the color mode (must have the value        */
/*                           MID_MONO_SHADING_MODE or                   */
/*                           MID_TRUE_24BIT_MODE)                       */
/*  Output:                                                             */
/*    None                                                              */
/*                                                                      */
/************************************************************************/

#define MID_Set2DColorMode( mode )                                      \
{                                                                       \
    MIDSet2DColorMode           SE;                                     \
									\
    SE.hdr.halfword.length = sizeof(MIDSet2DColorMode);                 \
    SE.hdr.halfword.op     = OP_SET_2D_COLOR_MODE;			\
    SE.color_mode = mode;                                               \
									\
    MID_WR_DEFERBUF(&SE, MID_WORDS(sizeof(MIDSet2DColorMode) >> 2));    \
									\
}                       /* End MID_Set2DColorMode                    */


        /*----------------------------------------------*
	 * the section below defines a set of macros	*
	 * to build FIFO command elements which can be 	*
	 * variable in length                           *
	*-----------------------------------------------*/

/************************************************************************/
/*                                                                      */
/*  Macro Name:  MID_DefineTile                                         */
/*                                                                      */
/*  Function:                                                           */
/*    Issues the Pedernales Define Tile command element to define a     */
/*    tile and assign it an ID.  This command element is always written */
/*    to the deferral buffer.                                           */ /* ? */
/*                                                                      */
/*  Input:                                                              */
/*    tile_id (unsigned short) - ID to be assigned to the tile (must be */
/*                               4, 5, 6, or 7)                         */
/*    tile_height (unsigned short) - tile height in pixels (must        */
/*                               be <= 64 in 8 bit color mode and <= 32 */
/*                               in 24 bit color mode, and must always  */
/*                               be > 0)                                */
/*    tile_width (unsigned short)  - tile width in pixels (must         */
/*                               be <= 64 in 8 bit color mode and <= 32 */
/*                               in 24 bit color mode, and must always  */
/*                               be > 0)                                */
/*    bits_per_pixel (unsigned char) - number of bits each pixel        */
/*                               occupies in the tile pixmap (must be   */
/*                               8 in 8 bit color mode and 32 in 24 bit */
/*                               color mode)                            */
/*    pixmap_ptr (unsigned char *) - pointer to the tile pixmap data    */
/*                               (the pixmap must be stored in Z format */
/*                               and the scan line pad must be 32 bits) */
/*                                                                      */
/*  Output:                                                             */
/*    None                                                              */
/*                                                                      */
/************************************************************************/

#define MID_DefineTile( tile_id, tile_height, tile_width,               \
			bits_per_pixel, pixmap_ptr )                    \
{                                                                       \
    unsigned short      pixmap_len;                                     \
    MIDDefineTile       SE;                                             \
									\
    /* Determine the length, in bytes, of the tile pixmap */            \
									\
    pixmap_len = ((((tile_width * bits_per_pixel) + 31) >> 5) *         \
				  tile_height) << 2;                    \
    SE.hdr.halfword.length = sizeof(MIDDefineTile) + pixmap_len;        \
    SE.hdr.halfword.op     = OP_DEFINE_TILE;				\
    SE.resv    = 0;                                                     \
    SE.tl_id   = tile_id;                                               \
    SE.theight = tile_height;                                           \
    SE.twidth  = tile_width;                                            \
									\
    MID_WR_DEFERBUF(&SE , MID_WORDS(sizeof(MIDDefineTile) >> 2));       \
									\
    MID_WR_DEFERBUF( pixmap_ptr, MID_WORDS( pixmap_len >> 2 ) ); /* ? */\
									\
}                       /* End MID_DefineTile                        */

/************************************************************************/
/*                                                                      */
/*  Macro Name:  MID_DefineStipple                                      */
/*                                                                      */
/*  Function:                                                           */
/*    Issues the Pedernales Define Stipple command element to define a  */
/*    stipple and assign it an ID.  This command element is always      */
/*    written to the deferral buffer.                                   */
/*                                                                      */
/*  Input:                                                              */
/*    stipple_id (unsigned short) - ID to be assigned to the stipple    */
/*                                  (must be 0, 1, 2, or 3)             */
/*    stipple_height (unsigned short) - stipple height in pixels (must  */
/*                                  be >= 0 and <= 64)                  */
/*    stipple_width (unsigned short)  - stipple width in pixels (must   */
/*                                  be > 0 and <= 64)                   */
/*    pixmap_ptr (unsigned char *) - pointer to the stipple pixmap data */
/*                                  (the scan line pad must be 32 bits) */
/*                                                                      */
/*  Output:                                                             */
/*    None                                                              */
/*                                                                      */
/************************************************************************/

#define MID_DefineStipple( stipple_id, stipple_height,                  \
			   stipple_width, pixmap_ptr )                  \
{                                                                       \
    unsigned short      pixmap_len;                                     \
    MIDDefineStipple    SE;                                             \
									\
    /* Determine the length, in bytes, of the stipple pixmap */         \
									\
    pixmap_len = (((stipple_width + 31) >> 5) * stipple_height) << 2;   \
									\
    SE.hdr.halfword.length = sizeof(MIDDefineStipple) + pixmap_len;     \
    SE.hdr.halfword.op     = OP_DEFINE_STIPPLE;				\
    SE.resv    = 0;                                                     \
    SE.st_id   = stipple_id;                                            \
    SE.sheight = stipple_height;                                        \
    SE.swidth  = stipple_width;                                         \
									\
    MID_WR_DEFERBUF(&SE , MID_WORDS(sizeof(MIDDefineStipple) >> 2));    \
									\
    MID_WR_DEFERBUF( pixmap_ptr, MID_WORDS( pixmap_len >> 2 ) );        \
									\
}                       /* End MID_DefineStipple                     */

/************************************************************************/
/*                                                                      */
/*  Macro Name:  MID_Polypoint                                          */
/*                                                                      */
/*  Function:                                                           */
/*    Issues the Pedernales Polypoint command element.                  */
/*                                                                      */
/*    If all of the points specified do not fit into a single Polypoint */
/*    command element, this macro breaks up the array of points into    */
/*    smaller chunks and issues several Polypoint command elements to   */
/*    render all of the points.                                         */
/*                                                                      */
/*    Polypoint command elements are not deferred.  Any data in the     */
/*    deferral buffer is written to the FIFO (circular buffer) first,   */
/*    then the Polypoint command element(s) is written to the FIFO.     */
/*                                                                      */
/*  Input:                                                              */
/*    coord_mode (int)      - coordinate mode (must have the value      */
/*                            CoordModeOrigin or CoordModePrevious)     */
/*    num_points (int)      - number of points to be drawn              */
/*    points_ptr (xPoint *) - pointer to the array of points to be      */
/*                            drawn                                     */
/*                                                                      */
/*  Output:                                                             */
/*    None                                                              */
/*                                                                      */
/************************************************************************/

#define MID_Polypoint( coord_mode, num_points, points_ptr )             \
{                                                                       \
    MIDPolypoint        SE;                                             \
									\
    SE.hdr.halfword.op = OP_POLYPOINT;					\
    SE.mode = coord_mode;                                               \
									\
    /*****************************************************************  \
     *                                                               *  \
     * If all of the points specified fit into a single Polypoint    *  \
     * command element, then write the command element including all *  \
     * of the points to the adapter.                                 *  \
     *                                                               *  \
     ****************************************************************/  \
									\
    if (num_points <= MID_MAX_PERF_POLYPOINT_POINTS)                    \
    {                                                                   \
      SE.hdr.halfword.length = sizeof(MIDPolypoint) + (num_points << 2);\
									\
      /***************************************************************  \
       *                                                             *  \
       * The portion of the Polypoint command element that precedes  *  \
       * the points is written to the deferral buffer first.  Then   *  \
       * MID_WRITE_TO_FIFO is issued.  MID_WRITE_TO_FIFO flushes the *  \
       * deferral buffer then writes the rest of the Polypoint       *  \
       * command element to the FIFO as efficiently as possible.     *  \
       *                                                             *  \
       **************************************************************/  \
									\
      MID_WR_DEFERBUF(&SE , MID_WORDS(sizeof(MIDPolypoint) >> 2));      \
									\
      MID_WRITE_TO_FIFO( points_ptr, MID_WORDS( num_points ) );         \
    }                                                                   \
    else                                                                \
    {                                                                   \
      unsigned short    num_points_this_cmd;                            \
      xPoint            new;                                            \
      xPoint            *points_ptr_this_cmd = points_ptr;              \
      int               rem_points, j;                                  \
									\
      /***************************************************************  \
       *                                                             *  \
       * All of the points specified do not fit into a single        *  \
       * Polypoint command element.  So break up the input array of  *  \
       * points into smaller chunks and issue a Polypoint command    *  \
       * element to render each chunk.  Each chunk will consist of   *  \
       * MID_MAX_PERF_POLYPOINT_POINTS points, except possibly the   *  \
       * last chunk.                                                 *  \
       *                                                             *  \
       ***************************************************************/ \
									\
      for (rem_points = num_points; rem_points > 0;                     \
			   rem_points -= MID_MAX_PERF_POLYPOINT_POINTS) \
      {                                                                 \
	if (rem_points >= MID_MAX_PERF_POLYPOINT_POINTS)                \
	  num_points_this_cmd = MID_MAX_PERF_POLYPOINT_POINTS;          \
	else                                                            \
	  num_points_this_cmd = rem_points;                             \
									\
	/*************************************************************  \
	 *                                                           *  \
	 * Write the Polypoint command element including the chunk   *  \
	 * of points to the deferral buffer. It will be written to   *  \
	 * the FIFO when the deferral buffer fills up, or after the  *  \
	 * last Polypoint command element has been written to the    *  \
	 * deferral buffer.                                          *  \
	 *                                                           *  \
	 *************************************************************/ \
									\
	SE.hdr.halfword.length = sizeof(MIDPolypoint) +                 \
				 (num_points_this_cmd << 2);            \
									\
	MID_WR_DEFERBUF(&SE , MID_WORDS(sizeof(MIDPolypoint) >> 2));    \
									\
	MID_WR_DEFERBUF( points_ptr_this_cmd,                           \
			 MID_WORDS( num_points_this_cmd ) );            \
									\
	/*************************************************************  \
	 *                                                           *  \
	 * If the coordinate mode specified indicates that the       *  \
	 * points in the input array of points are each defined      *  \
	 * relative to the previous point (except for the first      *  \
	 * point which is defined relative to the window origin),    *  \
	 * and all of the points in the input array of points have   *  \
	 * not been processed yet, then update the first point in    *  \
	 * the chunk of points to be processed with the next         *  \
	 * Polypoint command element so that the first point in that *  \
	 * chunk is relative to the window origin and not to the     *  \
	 * previous point.                                           *  \
	 *                                                           *  \
	 ************************************************************/  \
									\
	if (coord_mode == CoordModePrevious &&                          \
	    rem_points > MID_MAX_PERF_POLYPOINT_POINTS)                 \
	{                                                               \
	  new.x = points_ptr_this_cmd->x;                               \
	  new.y = points_ptr_this_cmd->y;                               \
									\
	  for (j = 1; j <= MID_MAX_PERF_POLYPOINT_POINTS; j++)          \
	  {                                                             \
	    points_ptr_this_cmd++;                                      \
	    new.x += points_ptr_this_cmd->x;                            \
	    new.y += points_ptr_this_cmd->y;                            \
	  }                                                             \
									\
	  points_ptr_this_cmd->x = new.x;                               \
	  points_ptr_this_cmd->y = new.y;                               \
	}                                                               \
	else                                                            \
	{                                                               \
	  points_ptr_this_cmd += num_points_this_cmd;                   \
	}                                                               \
      }                 /* End for loop to break up the input array of  \
			   points into smaller chunks                */ \
									\
      /***************************************************************  \
       *                                                             *  \
       * Flush the deferral buffer so that all of the Polypoint      *  \
       * command elements, along with any other command elements     *  \
       * present at the front of the deferral buffer, are written to *  \
       * the FIFO now.  Note that each Polypoint command element was *  \
       * written to the deferral buffer instead of written directly  *  \
       * to the FIFO because it takes less time to copy each command *  \
       * element into the deferral buffer and then issue a single    *  \
       * DMA operation to write all of the command elements to the   *  \
       * FIFO than it takes to issue a single DMA operation or       *  \
       * multiple PIO operations to write each command element       *  \
       * directly to the FIFO.                                       *  \
       *                                                             *  \
       ***************************************************************/ \
									\
      MID_DEFERBUF_FLUSH;                                               \
    }                   /* End input array of points is too big to fit  \
			   into a single Polypoint command element   */ \
}                       /* End MID_Polypoint                         */

/************************************************************************/
/*                                                                      */
/*  Macro Name:  MID_Polylines                                          */
/*                                                                      */
/*  Function:                                                           */
/*    Issues the Pedernales Polylines command element.                  */
/*                                                                      */
/*    If all of the points specified do not fit into a single Polylines */
/*    command element, this macro breaks up the array of points into    */
/*    smaller chunks and issues several Polylines command elements to   */
/*    render all of the lines.                                          */
/*                                                                      */
/*    Note that if all of the points specified do not fit into a single */
/*    Polylines command element, this macro cannot be used if the line  */
/*    style is on off dash, or if the line style is double dash and the */
/*    fill style is solid or stippled.  This is because the dash        */
/*    pattern must start at the beginning of the first line and be      */
/*    continuous throughout all of the lines.  If this macro is used,   */
/*    the dash pattern will start over at the first line in each        */
/*    Polylines command element that is issued.                         */
/*                                                                      */
/*    Polylines command elements are not deferred.  Any data in the     */
/*    deferral buffer is written to the FIFO (circular buffer) first,   */
/*    then the Polylines command element(s) is written to the FIFO.     */
/*                                                                      */
/*  Input:                                                              */
/*    coord_mode (int)           - coordinate mode (must have the value */
/*                                 CoordModeOrigin or CoordModePrevious)*/
/*    num_points (int)           - number of points                     */
/*    points_ptr (DDXPointRec *) - pointer to an array of points        */
/*    cap_style (int)            - current cap style on the adapter     */
/*    logical_op (unsigned char) - current logical operation (must be   */
/*                                 one of the 16 Boolean functions      */
/*                                 defined by X)                        */
/*    dashed_lines (int)         - indicates whether or not dashed      */
/*                                 lines are to be drawn (0 = no)       */
/*  Output:                                                             */
/*    None                                                              */
/*                                                                      */
/************************************************************************/

#define MID_Polylines( coord_mode, num_points, points_ptr, cap_style,   \
		       logical_op, dashed_lines)                        \
{                                                                       \
    MIDPolylines SE;                                                    \
									\
    SE.hdr.halfword.op = OP_POLYLINES;					\
    SE.mode = coord_mode;                                               \
									\
    /*****************************************************************  \
     *                                                               *  \
     * If all of the points specified fit into a single Polylines    *  \
     * command element, then write the command element including all *  \
     * of the points to the adapter.                                 *  \
     *                                                               *  \
     ****************************************************************/  \
									\
    if (num_points <= MID_MAX_PERF_POLYLINES_POINTS || dashed_lines)    \
    {                                                                   \
      SE.hdr.halfword.length = sizeof(MIDPolylines) + (num_points << 2);\
									\
      /***************************************************************  \
       *                                                             *  \
       * The portion of the Polylines command element that precedes  *  \
       * the points is written to the deferral buffer first.  Then   *  \
       * MID_WRITE_TO_FIFO is issued.  MID_WRITE_TO_FIFO flushes the *  \
       * deferral buffer then writes the rest of the Polylines       *  \
       * command element to the FIFO as efficiently as possible.     *  \
       *                                                             *  \
       **************************************************************/  \
									\
      MID_WR_DEFERBUF(&SE , MID_WORDS(sizeof(MIDPolylines) >> 2));      \
									\
      MID_WRITE_TO_FIFO( points_ptr, MID_WORDS( num_points ) );         \
    }                                                                   \
    else                                                                \
    {                                                                   \
      DDXPointRec       new, last;                                      \
      DDXPointRec       *points_ptr_this_cmd = points_ptr;              \
      DDXPointRec       *temp_ptr;                                      \
      int               rem_points, j;                                  \
      unsigned char     point_can_be_drawn_twice = 0;                   \
									\
      /***************************************************************  \
       *                                                             *  \
       * All of the points specified do not fit into a single        *  \
       * Polylines command element.  So break up the input array of  *  \
       * points into smaller chunks and issue a Polylines command    *  \
       * element to render each chunk.  Each chunk will consist of   *  \
       * MID_MAX_PERF_POLYLINES_POINTS points, except possibly the   *  \
       * last chunk.  Note that the last point in each chunk will be *  \
       * repeated as the first point in the next chunk.              *  \
       *                                                             *  \
       ***************************************************************/ \
									\
      /***************************************************************  \
       *                                                             *  \
       * For some logical ops, it does not matter how many times a   *  \
       * pixel is drawn because the result is still the same.  For   *  \
       * those logical ops, set the point_can_be_drawn_twice flag.   *  \
       *                                                             *  \
       ***************************************************************/ \
									\
      if (logical_op == GXcopy  || logical_op == GXand ||               \
	  logical_op == GXor    || logical_op == GXset ||               \
	  logical_op == GXclear || logical_op == GXcopyInverted ||      \
	  logical_op == GXandInverted || logical_op == GXorInverted ||  \
	  logical_op == GXnoop)                                         \
	point_can_be_drawn_twice = 1;                                   \
									\
      /***************************************************************  \
       *                                                             *  \
       * Now, if a point cannot be drawn twice and the cap style is  *  \
       * NOT cap not last, set the cap style on the adapter to cap   *  \
       * not last.  This is done so that the last point specified in *  \
       * each Polylines command element will not be drawn.  This is  *  \
       * necessary since the last point specified in a Polylines     *  \
       * command element is also specified as the first point in the *  \
       * next Polylines command element, and a point cannot be drawn *  \
       * twice.                                                      *  \
       *                                                             *  \
       ***************************************************************/ \
									\
      if (!point_can_be_drawn_twice && cap_style != CapNotLast)         \
	MID_SetLineAttributes(MID_UPDATE_CAP_STYLE, 0, 0, CapNotLast);  \
									\
      /***************************************************************  \
       *                                                             *  \
       * Issue Polylines command elements to process all of the      *  \
       * points in the input array of points, except do not issue    *  \
       * the last Polylines command element to process the chunk of  *  \
       * points that contains the last point in the input array.     *  \
       *                                                             *  \
       ***************************************************************/ \
									\
      for (rem_points = num_points;                                     \
	   rem_points > MID_MAX_PERF_POLYLINES_POINTS;                  \
	   rem_points -= (MID_MAX_PERF_POLYLINES_POINTS - 1))           \
      {                                                                 \
	/*************************************************************  \
	 *                                                           *  \
	 * Write the Polylines command element including the chunk   *  \
	 * of points to the deferral buffer. It will be written to   *  \
	 * the FIFO when the deferral buffer fills up, or after the  *  \
	 * last Polylines command element has been written to the    *  \
	 * deferral buffer.                                          *  \
	 *                                                           *  \
	 *************************************************************/ \
									\
	SE.hdr.halfword.length = sizeof(MIDPolylines) +                 \
				 (MID_MAX_PERF_POLYLINES_POINTS << 2);  \
									\
	MID_WR_DEFERBUF(&SE , MID_WORDS(sizeof(MIDPolylines) >> 2));    \
									\
	MID_WR_DEFERBUF( points_ptr_this_cmd,                           \
			 MID_WORDS( MID_MAX_PERF_POLYLINES_POINTS ) );  \
									\
	/*************************************************************  \
	 *                                                           *  \
	 * If the coordinate mode specified indicates that the       *  \
	 * points in the input array of points are each defined      *  \
	 * relative to the previous point (except for the first      *  \
	 * point which is defined relative to the window origin),    *  \
	 * then update the first point in the chunk of points to be  *  \
	 * processed with the next Polylines command element so that *  \
	 * the first point in that chunk is relative to the window   *  \
	 * origin and not to the previous point.  Note that the      *  \
	 * first point to be processed with the next Polylines       *  \
	 * command element is the last point that was processed with *  \
	 * the Polylines command that was just written.              *  \
	 *                                                           *  \
	 ************************************************************/  \
									\
	if (coord_mode == CoordModePrevious)                            \
	{                                                               \
	  new.x = points_ptr_this_cmd->x;                               \
	  new.y = points_ptr_this_cmd->y;                               \
									\
	  for (j = 1; j < MID_MAX_PERF_POLYLINES_POINTS; j++)           \
	  {                                                             \
	    points_ptr_this_cmd++;                                      \
	    new.x += points_ptr_this_cmd->x;                            \
	    new.y += points_ptr_this_cmd->y;                            \
	  }                                                             \
									\
	  points_ptr_this_cmd->x = new.x;                               \
	  points_ptr_this_cmd->y = new.y;                               \
	}                                                               \
	else                                                            \
	{                                                               \
	  points_ptr_this_cmd += MID_MAX_PERF_POLYLINES_POINTS - 1;     \
	}                                                               \
      }                 /* End for loop to break up the input array of  \
			   points into smaller chunks                */ \
									\
      /***************************************************************  \
       *                                                             *  \
       * All of the points in the input array of points have already *  \
       * been processed except the last chunk of points that         *  \
       * contains the last point in the input array.  If a point     *  \
       * cannot be drawn twice and the original cap style is not cap *  \
       * not last, then the cap style on the adapter does not match  *  \
       * the original cap style.  So determine whether the first and *  \
       * last point specified in the input array of points are the   *  \
       * same in order to determine when to restore the cap style on *  \
       * the adapter to the original cap style.                      *  \
       *                                                             *  \
       ***************************************************************/ \
									\
      if (!point_can_be_drawn_twice && cap_style != CapNotLast)         \
      {                                                                 \
	/*************************************************************  \
	 *                                                           *  \
	 * If the last point in the input array was specified        *  \
	 * relative to the previous point, then determine what the   *  \
	 * last point is relative to the window origin.              *  \
	 *                                                           *  \
	 *************************************************************/ \
									\
	if (coord_mode == CoordModePrevious)                            \
	{                                                               \
	  temp_ptr = points_ptr_this_cmd;                               \
	  last.x = temp_ptr->x;                                         \
	  last.y = temp_ptr->y;                                         \
									\
	  for (j = 1; j < rem_points; j++)                              \
	  {                                                             \
	    temp_ptr++;                                                 \
	    last.x += temp_ptr->x;                                      \
	    last.y += temp_ptr->y;                                      \
	  }                                                             \
	}                                                               \
	else            /* The last point was specified relative to     \
			   the window origin                         */ \
	{                                                               \
	  last.x = (points_ptr_this_cmd + rem_points - 1)->x;           \
	  last.y = (points_ptr_this_cmd + rem_points - 1)->y;           \
	}                                                               \
									\
	/*************************************************************  \
	 *                                                           *  \
	 * If the first and last point specified in the input array  *  \
	 * of points are not the same, then restore the cap style on *  \
	 * the adapter to the original cap style.  This will cause   *  \
	 * the last point to be drawn when the final Polylines       *  \
	 * command element is issued.                                *  \
	 *                                                           *  \
	 *************************************************************/ \
									\
	if (points_ptr->x != last.x || points_ptr->y != last.y)         \
	{                                                               \
	  MID_SetLineAttributes(MID_UPDATE_CAP_STYLE, 0, 0, cap_style); \
	}                                                               \
      }                 /* End a point cannot be drawn twice         */ \
									\
      /***************************************************************  \
       *                                                             *  \
       * Write the final Polylines command element to render the     *  \
       * remaining lines.                                            *  \
       *                                                             *  \
       ***************************************************************/ \
									\
      SE.hdr.halfword.length = sizeof(MIDPolylines) + (rem_points << 2);\
									\
      MID_WR_DEFERBUF(&SE , MID_WORDS(sizeof(MIDPolylines) >> 2));      \
									\
      MID_WR_DEFERBUF( points_ptr_this_cmd, MID_WORDS( rem_points ) );  \
									\
      /***************************************************************  \
       *                                                             *  \
       * If a point cannot be drawn twice, the first and last point  *  \
       * specified in the input array of points are the same, and    *  \
       * the original cap style is not cap not last, then restore    *  \
       * the cap style on the adapter to the original cap style      *  \
       * because the cap style on the adapter was modified by this   *  \
       * macro.                                                      *  \
       *                                                             *  \
       ***************************************************************/ \
									\
      if (!point_can_be_drawn_twice && cap_style != CapNotLast &&       \
	  points_ptr->x == last.x && points_ptr->y == last.y)           \
      {                                                                 \
	MID_SetLineAttributes(MID_UPDATE_CAP_STYLE, 0, 0, cap_style);   \
      }                                                                 \
									\
      /***************************************************************  \
       *                                                             *  \
       * Flush the deferral buffer so that all of the Polylines      *  \
       * command elements, along with any other command elements     *  \
       * present in the deferral buffer, are written to the FIFO     *  \
       * now.  Note that each Polylines command element was written  *  \
       * to the deferral buffer instead of written directly to the   *  \
       * FIFO because it takes less time to copy each command        *  \
       * element into the deferral buffer and then issue a single    *  \
       * DMA operation to write all of the command elements to the   *  \
       * FIFO than it takes to issue a single DMA operation or       *  \
       * multiple PIO operations to write each command element       *  \
       * directly to the FIFO.                                       *  \
       *                                                             *  \
       ***************************************************************/ \
									\
      MID_DEFERBUF_FLUSH;                                               \
    }                   /* End input array of points is too big to fit  \
			   into a single Polylines command element   */ \
}                       /* End MID_Polylines                         */

/************************************************************************/
/*                                                                      */
/*  Macro Name:  MID_Polysegment                                        */
/*                                                                      */
/*  Function:                                                           */
/*    Issues the Pedernales Polysegment command element.                */
/*                                                                      */
/*    If all of the line segments specified do not fit into a single    */
/*    Polysegment command element, this macro breaks up the array of    */
/*    line segments into smaller chunks and issues several Polysegment  */
/*    command elements to render all of the line segments.              */
/*                                                                      */
/*    Polysegment command elements are not deferred.  Any data in the   */
/*    deferral buffer is written to the FIFO (circular buffer) first,   */
/*    then the Polysegment command element(s) is written to the FIFO.   */
/*                                                                      */
/*  Input:                                                              */
/*    num_segments (int)        - number of line segments to be drawn   */
/*    segments_ptr (xSegment *) - pointer to the array of line segments */
/*                                to be drawn                           */
/*                                                                      */
/*  Output:                                                             */
/*    None                                                              */
/*                                                                      */
/************************************************************************/

#define MID_Polysegment( num_segments, segments_ptr )                   \
{                                                                       \
    MIDPolysegment      SE;                                             \
									\
    SE.hdr.halfword.op = OP_POLYSEGMENT;				\
									\
    /*****************************************************************  \
     *                                                               *  \
     * If all of the line segments specified fit into a single       *  \
     * Polysegment command element, then write the command element   *  \
     * including all of the line segments to the adapter.            *  \
     *                                                               *  \
     ****************************************************************/  \
									\
    if (num_segments <= MID_MAX_PERF_POLYSEGMENT_SEGS)                  \
    {                                                                   \
      SE.hdr.halfword.length = sizeof(MIDPolysegment) +                 \
			       (num_segments << 3);                     \
									\
      /***************************************************************  \
       *                                                             *  \
       * The portion of the Polysegment command element that         *  \
       * precedes the line segments is written to the deferral       *  \
       * buffer first.  Then MID_WRITE_TO_FIFO is issued.            *  \
       * MID_WRITE_TO_FIFO flushes the deferral buffer then writes   *  \
       * the rest of the Polysegment command element to the FIFO as  *  \
       * efficiently as possible.                                    *  \
       *                                                             *  \
       **************************************************************/  \
									\
      MID_WR_DEFERBUF(&SE , MID_WORDS(sizeof(MIDPolysegment) >> 2));    \
									\
      MID_WRITE_TO_FIFO( segments_ptr, MID_WORDS( num_segments << 1 ) );\
    }                                                                   \
    else                                                                \
    {                                                                   \
      unsigned short    num_segments_this_cmd;                          \
      xSegment          *segments_ptr_this_cmd = segments_ptr;          \
      int               rem_segments;                                   \
									\
      /***************************************************************  \
       *                                                             *  \
       * All of the line segments specified do not fit into a single *  \
       * Polysegment command element.  So break up the input array   *  \
       * of line segments into smaller chunks and issue a Polysegment*  \
       * command element to render each chunk.  Each chunk will      *  \
       * consist of MID_MAX_PERF_POLYSEGMENT_SEGS segments, except   *  \
       * possibly the last chunk.                                    *  \
       *                                                             *  \
       ***************************************************************/ \
									\
      for (rem_segments = num_segments; rem_segments > 0;               \
	   rem_segments -= MID_MAX_PERF_POLYSEGMENT_SEGS)               \
      {                                                                 \
	if (rem_segments >= MID_MAX_PERF_POLYSEGMENT_SEGS)              \
	  num_segments_this_cmd = MID_MAX_PERF_POLYSEGMENT_SEGS;        \
	else                                                            \
	  num_segments_this_cmd = rem_segments;                         \
									\
	/*************************************************************  \
	 *                                                           *  \
	 * Write the Polysegment command element including the chunk *  \
	 * of line segments to the deferral buffer.  It will be      *  \
	 * written to the FIFO when the deferral buffer fills up, or *  \
	 * after the last Polysegment command element has been       *  \
	 * written to the deferral buffer.                           *  \
	 *                                                           *  \
	 *************************************************************/ \
									\
	SE.hdr.halfword.length = sizeof(MIDPolysegment) +               \
				 (num_segments_this_cmd << 3);          \
									\
	MID_WR_DEFERBUF(&SE , MID_WORDS(sizeof(MIDPolysegment) >> 2));  \
									\
	MID_WR_DEFERBUF( segments_ptr_this_cmd,                         \
			 MID_WORDS( num_segments_this_cmd << 1 ) );     \
									\
	segments_ptr_this_cmd += num_segments_this_cmd;                 \
      }                 /* End for loop to break up the input array of  \
			   line segments into smaller chunks         */ \
									\
      /***************************************************************  \
       *                                                             *  \
       * Flush the deferral buffer so that all of the Polysegment    *  \
       * command elements, along with any other command elements     *  \
       * present at the front of the deferral buffer, are written to *  \
       * the FIFO now.  Note that each Polysegment command element   *  \
       * was written to the deferral buffer instead of written       *  \
       * directly to the FIFO because it takes less time to copy     *  \
       * each command element into the deferral buffer and then      *  \
       * issue a single DMA operation to write all of the command    *  \
       * elements to the FIFO than it takes to issue a single DMA    *  \
       * operation or multiple PIO operations to write each command  *  \
       * element directly to the FIFO.                               *  \
       *                                                             *  \
       ***************************************************************/ \
									\
      MID_DEFERBUF_FLUSH;                                               \
    }                   /* End input array of line segments is too big  \
			   to fit into a single Polysegment command     \
			   element                                   */ \
}                       /* End MID_Polysegment                       */

/************************************************************************/
/*                                                                      */
/*  Macro Name:  MID_Polyrectangle                                      */
/*                                                                      */
/*  Function:                                                           */
/*    Issues the Pedernales Polyrectangle command element.              */
/*                                                                      */
/*    If all of the rectangles specified do not fit into a single       */
/*    Polyrectangle command element, this macro breaks up the array of  */
/*    rectangles into smaller chunks and issues several Polyrectangle   */
/*    command elements to render all of the rectangles.                 */
/*                                                                      */
/*    Polyrectangle command elements are not deferred.  Any data in the */
/*    deferral buffer is written to the FIFO (circular buffer) first,   */
/*    then the Polyrectangle command element(s) is written to the FIFO. */
/*                                                                      */
/*  Input:                                                              */
/*    num_rects (int)           - number of rectangles to be drawn      */
/*    rects_ptr (xRectangle *)  - pointer to the array of rectangles to */
/*                                be drawn                              */
/*                                                                      */
/*  Output:                                                             */
/*    None                                                              */
/*                                                                      */
/************************************************************************/

#define MID_Polyrectangle( num_rects, rects_ptr )                       \
{                                                                       \
    MIDPolyrectangle    SE;                                             \
									\
    SE.hdr.halfword.op = OP_POLYRECTANGLE;				\
									\
    /*****************************************************************  \
     *                                                               *  \
     * If all of the rectangles specified fit into a single          *  \
     * Polyrectangle command element, then write the command element *  \
     * including all of the rectangles to the adapter.               *  \
     *                                                               *  \
     ****************************************************************/  \
									\
    if (num_rects <= MID_MAX_PERF_POLYRECT_RECTS)                       \
    {                                                                   \
      SE.hdr.halfword.length = sizeof(MIDPolyrectangle) +               \
			       (num_rects << 3);                        \
									\
      /***************************************************************  \
       *                                                             *  \
       * The portion of the Polyrectangle command element that       *  \
       * precedes the rectangles is written to the deferral buffer   *  \
       * first.  Then MID_WRITE_TO_FIFO is issued. MID_WRITE_TO_FIFO *  \
       * flushes the deferral buffer then writes the rest of the     *  \
       * Polyrectangle command element to the FIFO as efficiently as *  \
       * possible.                                                   *  \
       *                                                             *  \
       **************************************************************/  \
									\
      MID_WR_DEFERBUF(&SE , MID_WORDS(sizeof(MIDPolyrectangle) >> 2));  \
									\
      MID_WRITE_TO_FIFO( rects_ptr, MID_WORDS( num_rects << 1 ) );      \
    }                                                                   \
    else                                                                \
    {                                                                   \
      unsigned short    num_rects_this_cmd;                             \
      xRectangle        *rects_ptr_this_cmd = rects_ptr;                \
      int               rem_rects;                                      \
									\
      /***************************************************************  \
       *                                                             *  \
       * All of the rectangles specified do not fit into a single    *  \
       * Polyrectangle command element.  So break up the input array *  \
       * of rectangles into smaller chunks and issue a Polyrectangle *  \
       * command element to render each chunk.  Each chunk will      *  \
       * consist of MID_MAX_PERF_POLYRECT_RECTS rectangles,  except  *  \
       * possibly the last chunk.                                    *  \
       *                                                             *  \
       ***************************************************************/ \
									\
      for (rem_rects = num_rects; rem_rects > 0;                        \
	   rem_rects -= MID_MAX_PERF_POLYRECT_RECTS)                    \
      {                                                                 \
	if (rem_rects >= MID_MAX_PERF_POLYRECT_RECTS)                   \
	  num_rects_this_cmd = MID_MAX_PERF_POLYRECT_RECTS;             \
	else                                                            \
	  num_rects_this_cmd = rem_rects;                               \
									\
	/*************************************************************  \
	 *                                                           *  \
	 * Write the Polyrectangle command element including the     *  \
	 * chunk of rectangles to the deferral buffer.  It will be   *  \
	 * written to the FIFO when the deferral buffer fills up, or *  \
	 * after the last Polyrectangle command element has been     *  \
	 * written to the deferral buffer.                           *  \
	 *                                                           *  \
	 *************************************************************/ \
									\
	SE.hdr.halfword.length = sizeof(MIDPolyrectangle) +             \
				 (num_rects_this_cmd << 3);             \
									\
	MID_WR_DEFERBUF(&SE , MID_WORDS(sizeof(MIDPolyrectangle) >> 2));\
									\
	MID_WR_DEFERBUF( rects_ptr_this_cmd,                            \
			 MID_WORDS( num_rects_this_cmd << 1 ) );        \
									\
	rects_ptr_this_cmd += num_rects_this_cmd;                       \
      }                 /* End for loop to break up the input array of  \
			   rectangles into smaller chunks            */ \
									\
      /***************************************************************  \
       *                                                             *  \
       * Flush the deferral buffer so that all of the Polyrectangle  *  \
       * command elements, along with any other command elements     *  \
       * present at the front of the deferral buffer, are written to *  \
       * the FIFO now.  Note that each Polyrectangle command element *  \
       * was written to the deferral buffer instead of written       *  \
       * directly to the FIFO because it takes less time to copy     *  \
       * each command element into the deferral buffer and then      *  \
       * issue a single DMA operation to write all of the command    *  \
       * elements to the FIFO than it takes to issue a single DMA    *  \
       * operation or multiple PIO operations to write each command  *  \
       * element directly to the FIFO.                               *  \
       *                                                             *  \
       ***************************************************************/ \
									\
      MID_DEFERBUF_FLUSH;                                               \
    }                   /* End input array of rectangles is too big to  \
			   fit into a single Polyrectangle command      \
			   element                                   */ \
}                       /* End MID_Polyrectangle                     */

/************************************************************************/
/*                                                                      */
/*  Macro Name:  MID_Polyarc                                            */
/*                                                                      */
/*  Function:                                                           */
/*    Issues the Pedernales Polyarc command element.                    */
/*                                                                      */
/*    If all of the arcs specified do not fit into a single Polyarc     */
/*    command element, this macro breaks up the array of arcs into      */
/*    smaller chunks and issues several Polyarc command elements to     */
/*    render all of the arcs.                                           */
/*                                                                      */
/*    Note that if all of the arcs specified do not fit into a single   */
/*    Polyarc command element, this macro cannot be used if the line    */
/*    style is on off dash, or if the line style is double dash and the */
/*    fill style is solid or stippled.  This is because the dash        */
/*    pattern may need to be continuous from one Polyarc command        */
/*    element to the next, and it will not be if this macro is used.    */
/*                                                                      */
/*    Polyarc command elements are not deferred.  Any data in the       */
/*    deferral buffer is written to the FIFO (circular buffer) first,   */
/*    then the Polyarc command element(s) is written to the FIFO.       */
/*                                                                      */
/*  Input:                                                              */
/*    num_arcs (int)     - number of arcs to be drawn                   */
/*    arcs_ptr (xArc *)  - pointer to the array of arcs to be drawn     */
/*    dashed_arcs (int)  - indicates whether or not dashed arcs are to  */
/*                         be drawn (0 = no)                            */
/*  Output:                                                             */
/*    None                                                              */
/*                                                                      */
/************************************************************************/

#define MID_Polyarc( num_arcs, arcs_ptr, dashed_arcs )                  \
{                                                                       \
    MIDPolyarc  SE;                                                     \
									\
    SE.hdr.halfword.op = OP_POLYARC;					\
									\
    /*****************************************************************  \
     *                                                               *  \
     * If all of the arcs specified fit into a single Polyarc        *  \
     * command element, then write the command element including all *  \
     * of the arcs to the adapter.                                   *  \
     *                                                               *  \
     ****************************************************************/  \
									\
    if (num_arcs <= MID_MAX_PERF_POLYARC_ARCS || dashed_arcs)           \
    {                                                                   \
      SE.hdr.halfword.length = sizeof(MIDPolyarc) + (num_arcs * 12);    \
									\
      /***************************************************************  \
       *                                                             *  \
       * The portion of the Polyarc command element that precedes    *  \
       * the arcs is written to the deferral buffer first.  Then     *  \
       * MID_WRITE_TO_FIFO is issued.  MID_WRITE_TO_FIFO flushes the *  \
       * deferral buffer then writes the rest of the Polyarc command *  \
       * element to the FIFO as efficiently as possible.             *  \
       *                                                             *  \
       **************************************************************/  \
									\
      MID_WR_DEFERBUF(&SE , MID_WORDS(sizeof(MIDPolyarc) >> 2));        \
									\
      MID_WRITE_TO_FIFO( arcs_ptr, MID_WORDS( num_arcs  * 3 ) );        \
    }                                                                   \
    else                                                                \
    {                                                                   \
      unsigned short    num_arcs_this_cmd;                              \
      xArc              *arcs_ptr_this_cmd = arcs_ptr;                  \
      int               rem_arcs;                                       \
									\
      /***************************************************************  \
       *                                                             *  \
       * All of the arcs specified do not fit into a single Polyarc  *  \
       * command element.  So break up the input array of arcs into  *  \
       * smaller chunks and issue a Polyarc command element to       *  \
       * render each chunk.  Each chunk will consist of              *  \
       * MID_MAX_PERF_POLYARC_ARCS arcs, except possibly the last    *  \
       * chunk.                                                      *  \
       *                                                             *  \
       ***************************************************************/ \
									\
      for (rem_arcs = num_arcs; rem_arcs > 0;                           \
	   rem_arcs -= MID_MAX_PERF_POLYARC_ARCS)                       \
      {                                                                 \
	if (rem_arcs >= MID_MAX_PERF_POLYARC_ARCS)                      \
	  num_arcs_this_cmd = MID_MAX_PERF_POLYARC_ARCS;                \
	else                                                            \
	  num_arcs_this_cmd = rem_arcs;                                 \
									\
	/*************************************************************  \
	 *                                                           *  \
	 * Write the Polyarc command element including the chunk of  *  \
	 * arcs to the deferral buffer.  It will be written to the   *  \
	 * FIFO when the deferral buffer fills up, or after the last *  \
	 * Polyarc command element has been written to the deferral  *  \
	 * buffer.                                                   *  \
	 *                                                           *  \
	 *************************************************************/ \
									\
	SE.hdr.halfword.length = sizeof(MIDPolyarc) +                   \
				 (num_arcs_this_cmd * 12);              \
									\
	MID_WR_DEFERBUF(&SE , MID_WORDS(sizeof(MIDPolyarc) >> 2));      \
									\
	MID_WR_DEFERBUF( arcs_ptr_this_cmd,                             \
			 MID_WORDS( num_arcs_this_cmd  * 3 ) );         \
									\
	arcs_ptr_this_cmd += num_arcs_this_cmd;                         \
      }                 /* End for loop to break up the input array of  \
			   arcs into smaller chunks                  */ \
									\
      /***************************************************************  \
       *                                                             *  \
       * Flush the deferral buffer so that all of the Polyarc        *  \
       * command elements, along with any other command elements     *  \
       * present at the front of the deferral buffer, are written to *  \
       * the FIFO now.  Note that each Polyarc command element was   *  \
       * written to the deferral buffer instead of written directly  *  \
       * to the FIFO because it takes less time to copy each command *  \
       * element into the deferral buffer and then issue a single    *  \
       * DMA operation to write all of the command elements to the   *  \
       * FIFO than it takes to issue a single DMA operation or       *  \
       * multiple PIO operations to write each command element       *  \
       * directly to the FIFO.                                       *  \
       *                                                             *  \
       ***************************************************************/ \
									\
      MID_DEFERBUF_FLUSH;                                               \
    }                   /* End input array of arcs is too big to fit    \
			   into a single Polyarc command element     */ \
}                       /* End MID_Polyarc                           */

/************************************************************************/
/*                                                                      */
/*  Macro Name:  MID_FillPolygon                                        */
/*                                                                      */
/*  Function:                                                           */
/*    Issues the Pedernales Fill Polygon command element.               */
/*                                                                      */
/*    If all of the points specified do not fit into a single Fill      */
/*    Polygon command element, this macro breaks up the array of points */
/*    into smaller chunks and issues several Fill Polygon command       */
/*    elements to render the entire polygon.                            */
/*                                                                      */
/*    Note that if all of the points specified do not fit into a single */
/*    Fill Polygon command element, this macro can only be used if the  */
/*    polygon is convex.                                                */
/*                                                                      */
/*    Fill Polygon command elements are not deferred.  Any data in the  */
/*    deferral buffer is written to the FIFO (circular buffer) first,   */
/*    then the Fill Polygon command element(s) is written to the FIFO.  */
/*                                                                      */
/*  Input:                                                              */
/*    poly_shape (int)          - polygon shape (must have the value    */
/*                                Complex, Nonconvex, or Convex)        */
/*    coord_mode (int)          - coordinate mode (must have the value  */
/*                                CoordModeOrigin or CoordModePrevious) */
/*    num_points (int)          - number of points (must be <= 65535,   */
/*                                where 65535 is the maximum request    */
/*                                length in words allowed by the X      */
/*                                Server)                               */
/*    points_ptr (DDXPointPtr)  - pointer to an array of points         */
/*                                                                      */
/*  Output:                                                             */
/*    None                                                              */
/*                                                                      */
/************************************************************************/

#define MID_FillPolygon( poly_shape, coord_mode, num_points, points_ptr ) \
{                                                                       \
    MIDFillPolygon      SE;                                             \
									\
    SE.hdr.halfword.op = OP_FILL_POLYGON;				\
    SE.shape = poly_shape;                                              \
    SE.mode  = coord_mode;                                              \
									\
    /*****************************************************************  \
     *                                                               *  \
     * If all of the points specified fit into a single Fill Polygon *  \
     * command element, then write the command element including all *  \
     * of the points to the adapter.                                 *  \
     *                                                               *  \
     *****************************************************************/ \
									\
    if (num_points <= MID_MAX_PERF_FILLPOLYGON_POINTS ||                \
	poly_shape != Convex)                                           \
    {                                                                   \
      SE.hdr.halfword.length = sizeof(MIDFillPolygon) +                 \
			       (num_points << 2);                       \
									\
      /***************************************************************  \
       *                                                             *  \
       * The portion of the Fill Polygon command element that        *  \
       * precedes the points is written to the deferral buffer       *  \
       * first.  Then MID_WRITE_TO_FIFO is issued. MID_WRITE_TO_FIFO *  \
       * flushes the deferral buffer then writes the rest of the     *  \
       * Fill Polygon command element to the FIFO as efficiently as  *  \
       * possible.                                                   *  \
       *                                                             *  \
       **************************************************************/  \
									\
      MID_WR_DEFERBUF(&SE , MID_WORDS(sizeof(MIDFillPolygon) >> 2));    \
									\
      MID_WRITE_TO_FIFO( points_ptr, MID_WORDS( num_points ) );         \
    }                                                                   \
    else                                                                \
    {                                                                   \
      unsigned short    num_points_this_cmd;                            \
      DDXPointRec       new;                                            \
      DDXPointPtr       points_ptr_this_cmd = points_ptr;               \
      int               rem_points, j;                                  \
      DDXPointRec       last_poly[(65535 /                              \
			   (MID_MAX_PERF_FILLPOLYGON_POINTS - 1)) + 3]; \
      short             num_last_poly_points;                           \
									\
      /***************************************************************  \
       *                                                             *  \
       * All of the points specified do not fit into a single Fill   *  \
       * Polygon command element.  So break up the input array of    *  \
       * points into smaller chunks and issue a Fill Polygon command *  \
       * element to render each chunk.  Each chunk will consist of   *  \
       * MID_MAX_PERF_FILLPOLYGON_POINTS points, except possibly the *  \
       * clast hunk.  Note that the last point in each chunk will be *  \
       * repeated as the first point in the next chunk.  Also note   *  \
       * that this algorithm will only work for convex polygons.     *  \
       *                                                             *  \
       ***************************************************************/ \
									\
      /***************************************************************  \
       *                                                             *  \
       * last_poly will contain the first point in the input array,  *  \
       * followed by the last point in each chunk.  Each point in    *  \
       * last_poly will be relative to the window origin.  This      *  \
       * polygon may have to be filled in order to complete the      *  \
       * original polygon.                                           *  \
       *                                                             *  \
       ***************************************************************/ \
									\
      last_poly[0].x = points_ptr->x;   /* Save the first point      */ \
      last_poly[0].y = points_ptr->y;                                   \
      num_last_poly_points = 1;                                         \
									\
      for (rem_points = num_points; rem_points > 2;                     \
	   rem_points -= (MID_MAX_PERF_FILLPOLYGON_POINTS - 1))         \
      {                                                                 \
	if (rem_points >= MID_MAX_PERF_FILLPOLYGON_POINTS)              \
	  num_points_this_cmd = MID_MAX_PERF_FILLPOLYGON_POINTS;        \
	else                                                            \
	  num_points_this_cmd = rem_points;                             \
									\
	/*************************************************************  \
	 *                                                           *  \
	 * Write the Fill Polygon command element including the      *  \
	 * chunk of points to the deferral buffer.  It will be       *  \
	 * written to the FIFO when the deferral buffer fills up, or *  \
	 * after the last Fill Polygon command element has been      *  \
	 * written to the deferral buffer.                           *  \
	 *                                                           *  \
	 *************************************************************/ \
									\
	SE.hdr.halfword.length = sizeof(MIDFillPolygon) +               \
				 (num_points_this_cmd << 2);            \
									\
	MID_WR_DEFERBUF(&SE , MID_WORDS(sizeof(MIDFillPolygon) >> 2));  \
									\
	MID_WR_DEFERBUF( points_ptr_this_cmd,                           \
			 MID_WORDS( num_points_this_cmd ) );            \
									\
	/*************************************************************  \
	 *                                                           *  \
	 * If the coordinate mode specified indicates that the       *  \
	 * points in the input array of points are each defined      *  \
	 * relative to the previous point (except for the first      *  \
	 * point which is defined relative to the window origin),    *  \
	 * then update the first point in the chunk of points to be  *  \
	 * processed with the next Fill Polygon command element so   *  \
	 * that the first point in that chunk is relative to the     *  \
	 * window origin and not to the previous point.              *  \
	 *                                                           *  \
	 *************************************************************/ \
									\
	if (coord_mode == CoordModePrevious)                            \
	{                                                               \
	  new.x = points_ptr_this_cmd->x;                               \
	  new.y = points_ptr_this_cmd->y;                               \
									\
	  for (j = 1; j < num_points_this_cmd; j++)                     \
	  {                                                             \
	    points_ptr_this_cmd++;                                      \
	    new.x += points_ptr_this_cmd->x;                            \
	    new.y += points_ptr_this_cmd->y;                            \
	  }                                                             \
									\
	  points_ptr_this_cmd->x = new.x;                               \
	  points_ptr_this_cmd->y = new.y;                               \
	}                                                               \
	else                                                            \
	{                                                               \
	  points_ptr_this_cmd += num_points_this_cmd - 1;               \
	}                                                               \
	/*************************************************************  \
	 *                                                           *  \
	 * Place the last point in the chunk of points that was just *  \
	 * processed with the last Fill Polygon command into the     *  \
	 * array of points that will be used to construct the last   *  \
	 * polygon to be filled.                                     *  \
	 *                                                           *  \
	 *************************************************************/ \
									\
	last_poly[num_last_poly_points].x = points_ptr_this_cmd->x;     \
	last_poly[num_last_poly_points].y = points_ptr_this_cmd->y;     \
	num_last_poly_points++;                                         \
      }                 /* End for loop to break up the input array of  \
			   points into smaller chunks                */ \
      /***************************************************************  \
       *                                                             *  \
       * If the last point in the input array of points has not been *  \
       * processed, then place that point in the array of points     *  \
       * that will be used to construct the last polygon to be       *  \
       * filled.  Update the point first if necessary so that it is  *  \
       * relative to the window origin.                              *  \
       *                                                             *  \
       ***************************************************************/ \
									\
      if (rem_points == 2)                                              \
      {                                                                 \
	if (coord_mode == CoordModePrevious)                            \
	{                                                               \
	  new.x = points_ptr_this_cmd->x;                               \
	  new.y = points_ptr_this_cmd->y;                               \
									\
	  points_ptr_this_cmd++;                                        \
									\
	  points_ptr_this_cmd->x = points_ptr_this_cmd->x + new.x;      \
	  points_ptr_this_cmd->y = points_ptr_this_cmd->y + new.y;      \
	}                                                               \
	else                                                            \
	{                                                               \
	  points_ptr_this_cmd++;                                        \
	}                                                               \
									\
	last_poly[num_last_poly_points].x = points_ptr_this_cmd->x;     \
	last_poly[num_last_poly_points].y = points_ptr_this_cmd->y;     \
	num_last_poly_points++;                                         \
      }                 /* End add last point to array of last          \
			   polygon points                            */ \
      /***************************************************************  \
       *                                                             *  \
       * Fill the last polygon in order to complete the original     *  \
       * polygon, unless the last polygon only consists of 3 points  *  \
       * and the first and last point are the same.                  *  \
       *                                                             *  \
       ***************************************************************/ \
									\
      if (num_last_poly_points > 3 ||                                   \
	  last_poly[0].x != last_poly[2].x ||                           \
	  last_poly[0].y != last_poly[2].y)                             \
      {                                                                 \
	SE.hdr.halfword.length = sizeof(MIDFillPolygon) +               \
				 (num_last_poly_points << 2);           \
	SE.shape = Convex;                                              \
	SE.mode  = CoordModeOrigin;                                     \
									\
	MID_WR_DEFERBUF(&SE , MID_WORDS(sizeof(MIDFillPolygon) >> 2));  \
									\
	MID_WR_DEFERBUF( last_poly, MID_WORDS (num_last_poly_points ) ); \
      }                                                                 \
									\
      /***************************************************************  \
       *                                                             *  \
       * Flush the deferral buffer so that all of the Fill Polygon   *  \
       * command elements, along with any other command elements     *  \
       * present at the front of the deferral buffer, are written to *  \
       * the FIFO now.  Note that each Fill Polygon command element  *  \
       * was written to the deferral buffer instead of written       *  \
       * directly to the FIFO because it takes less time to copy     *  \
       * each command element into the deferral buffer and then      *  \
       * issue a single DMA operation to write all of the command    *  \
       * elements to the FIFO than it takes to issue a single DMA    *  \
       * operation or multiple PIO operations to write each command  *  \
       * element directly to the FIFO.                               *  \
       *                                                             *  \
       ***************************************************************/ \
									\
      MID_DEFERBUF_FLUSH;                                               \
    }                   /* End input array of points is too big to fit  \
			   into a single Fill Polygon command element*/ \
}                       /* End MID_FillPolygon                       */

/************************************************************************/
/*                                                                      */
/*  Macro Name:  MID_FillPolyrectangle                                  */
/*                                                                      */
/*  Function:                                                           */
/*    Issues the Pedernales Fill Polyrectangle command element.         */
/*                                                                      */
/*    If all of the rectangles specified do not fit into a single Fill  */
/*    Polyrectangle command element, this macro breaks up the array of  */
/*    rectangles into smaller chunks and issues several Fill            */
/*    Polyrectangle command elements to render all of the rectangles.   */
/*                                                                      */
/*    Fill Polyrectangle command elements are not deferred.  Any data   */
/*    in the deferral buffer is written to the FIFO (circular buffer)   */
/*    first, then the Fill Polyrectangle command element(s) is written  */
/*    to the FIFO.                                                      */
/*                                                                      */
/*  Input:                                                              */
/*    num_rects (int)           - number of rectangles to be filled     */
/*    rects_ptr (xRectangle *)  - pointer to the array of rectangles to */
/*                                be filled                             */
/*                                                                      */
/*  Output:                                                             */
/*    None                                                              */
/*                                                                      */
/************************************************************************/

#define MID_FillPolyrectangle( num_rects, rects_ptr )                   \
{                                                                       \
    MIDFillPolyrectangle        SE;                                     \
									\
    SE.hdr.halfword.op = OP_FILL_POLYRECTANGLE;				\
									\
    /*****************************************************************  \
     *                                                               *  \
     * If all of the rectangles specified fit into a single Fill     *  \
     * Polyrectangle command element, then write the command element *  \
     * including all of the rectangles to the adapter.               *  \
     *                                                               *  \
     ****************************************************************/  \
									\
    if (num_rects <= MID_MAX_PERF_FILLPOLYRECT_RECTS)                   \
    {                                                                   \
      SE.hdr.halfword.length = sizeof(MIDFillPolyrectangle) +           \
			       (num_rects << 3);                        \
									\
      /***************************************************************  \
       *                                                             *  \
       * The portion of the Fill Polyrectangle command element that  *  \
       * precedes the rectangles is written to the deferral buffer   *  \
       * first.  Then MID_WRITE_TO_FIFO is issued. MID_WRITE_TO_FIFO *  \
       * flushes the deferral buffer then writes the rest of the     *  \
       * Fill Polyrectangle command element to the FIFO as           *  \
       * efficiently as possible.                                    *  \
       *                                                             *  \
       **************************************************************/  \
									\
      MID_WR_DEFERBUF(&SE , MID_WORDS(sizeof(MIDFillPolyrectangle) >> 2)); \
									\
      MID_WRITE_TO_FIFO( rects_ptr, MID_WORDS( num_rects << 1 ) );      \
    }                                                                   \
    else                                                                \
    {                                                                   \
      unsigned short    num_rects_this_cmd;                             \
      xRectangle        *rects_ptr_this_cmd = rects_ptr;                \
      int               rem_rects;                                      \
									\
      /***************************************************************  \
       *                                                             *  \
       * All of the rectangles specified do not fit into a single    *  \
       * Fill Polyrectangle command element.  So break up the input  *  \
       * array of rectangles into smaller chunks and issue a Fill    *  \
       * Polyrectangle command element to render each chunk.  Each   *  \
       * chunk will consist of MID_MAX_PERF_FILLPOLYRECT_RECTS       *  \
       * rectangles, except possibly the last chunk.                 *  \
       *                                                             *  \
       ***************************************************************/ \
									\
      for (rem_rects = num_rects; rem_rects > 0;                        \
	   rem_rects -= MID_MAX_PERF_FILLPOLYRECT_RECTS)                \
      {                                                                 \
	if (rem_rects >= MID_MAX_PERF_FILLPOLYRECT_RECTS)               \
	  num_rects_this_cmd = MID_MAX_PERF_FILLPOLYRECT_RECTS;         \
	else                                                            \
	  num_rects_this_cmd = rem_rects;                               \
									\
	/*************************************************************  \
	 *                                                           *  \
	 * Write the Fill Polyrectangle command element including    *  \
	 * the chunk of rectangles to the deferral buffer.  It will  *  \
	 * be written to the FIFO when the deferral buffer fills up, *  \
	 * or after the last Fill Polyrectangle command element has  *  \
	 * been written to the deferral buffer.                      *  \
	 *                                                           *  \
	 *************************************************************/ \
									\
	SE.hdr.halfword.length = sizeof(MIDFillPolyrectangle) +         \
				 (num_rects_this_cmd << 3);             \
									\
	MID_WR_DEFERBUF(&SE , MID_WORDS(sizeof(MIDFillPolyrectangle) >> 2)); \
									\
	MID_WR_DEFERBUF( rects_ptr_this_cmd,                            \
			 MID_WORDS( num_rects_this_cmd << 1 ) );        \
									\
	rects_ptr_this_cmd += num_rects_this_cmd;                       \
      }                 /* End for loop to break up the input array of  \
			   rectangles into smaller chunks            */ \
									\
      /***************************************************************  \
       *                                                             *  \
       * Flush the deferral buffer so that all of the Fill           *  \
       * Polyrectangle command elements, along with any other        *  \
       * command elements present at the front of the deferral       *  \
       * buffer, are written to the FIFO now.  Note that each Fill   *  \
       * Polyrectangle command element was written to the deferral   *  \
       * buffer instead of written directly to the FIFO because it   *  \
       * takes less time to copy each command element into the       *  \
       * deferral buffer and then issue a single DMA operation to    *  \
       * write all of the command elements to the FIFO than it takes *  \
       * to issue a single DMA operation or multiple PIO operations  *  \
       * to write each command element directly to the FIFO.         *  \
       *                                                             *  \
       ***************************************************************/ \
									\
      MID_DEFERBUF_FLUSH;                                               \
    }                   /* End input array of rectangles is too big to  \
			   fit into a single Fill Polyrectangle command \
			   element                                   */ \
}                       /* End MID_FillPolyrectangle                 */

/************************************************************************/
/*                                                                      */
/*  Macro Name:  MID_FillPolyarc                                        */
/*                                                                      */
/*  Function:                                                           */
/*    Issues the Pedernales Fill Polyarc command element.               */
/*                                                                      */
/*    If all of the arcs specified do not fit into a single Fill        */
/*    Polyarc command element, this macro breaks up the array of arcs   */
/*    into smaller chunks and issues several Fill Polyarc command       */
/*    elements to render all of the arcs.                               */
/*                                                                      */
/*    Fill Polyarc command elements are not deferred.  Any data in the  */
/*    deferral buffer is written to the FIFO (circular buffer) first,   */
/*    then the Fill Polyarc command element(s) is written to the FIFO.  */
/*                                                                      */
/*  Input:                                                              */
/*    num_arcs (int)     - number of arcs to be filled                  */
/*    arcs_ptr (xArc *)  - pointer to the array of arcs to be filled    */
/*                                                                      */
/*  Output:                                                             */
/*    None                                                              */
/*                                                                      */
/************************************************************************/

#define MID_FillPolyarc( num_arcs, arcs_ptr )                           \
{                                                                       \
    MIDFillPolyarc      SE;                                             \
									\
    SE.hdr.halfword.op = OP_FILL_POLYARC;				\
									\
    /*****************************************************************  \
     *                                                               *  \
     * If all of the arcs specified fit into a single Fill Polyarc   *  \
     * command element, then write the command element including all *  \
     * of the arcs to the adapter.                                   *  \
     *                                                               *  \
     ****************************************************************/  \
									\
    if (num_arcs <= MID_MAX_PERF_FILLPOLYARC_ARCS)                      \
    {                                                                   \
      SE.hdr.halfword.length = sizeof(MIDFillPolyarc) + (num_arcs * 12);\
									\
      /***************************************************************  \
       *                                                             *  \
       * The portion of the Fill Polyarc command element that        *  \
       * precedes the arcs is written to the deferral buffer first.  *  \
       * Then MID_WRITE_TO_FIFO is issued.  MID_WRITE_TO_FIFO        *  \
       * flushes the deferral buffer then writes the rest of the     *  \
       * Fill Polyarc command element to the FIFO as efficiently as  *  \
       * possible.                                                   *  \
       *                                                             *  \
       **************************************************************/  \
									\
      MID_WR_DEFERBUF(&SE , MID_WORDS(sizeof(MIDFillPolyarc) >> 2));    \
									\
      MID_WRITE_TO_FIFO( arcs_ptr, MID_WORDS( num_arcs * 3 ) );         \
    }                                                                   \
    else                                                                \
    {                                                                   \
      unsigned short    num_arcs_this_cmd;                              \
      xArc              *arcs_ptr_this_cmd = arcs_ptr;                  \
      int               rem_arcs;                                       \
									\
      /***************************************************************  \
       *                                                             *  \
       * All of the arcs specified do not fit into a single Fill     *  \
       * Polyarc command element.  So break up the input array of    *  \
       * arcs into smaller chunks and issue a Fill Polyarc command   *  \
       * element to render each chunk.  Each chunk will consist of   *  \
       * MID_MAX_PERF_FILLPOLYARC_ARCS arcs, except possibly the     *  \
       * last chunk.                                                 *  \
       *                                                             *  \
       ***************************************************************/ \
									\
      for (rem_arcs = num_arcs; rem_arcs > 0;                           \
	   rem_arcs -= MID_MAX_PERF_FILLPOLYARC_ARCS)                   \
      {                                                                 \
	if (rem_arcs >= MID_MAX_PERF_FILLPOLYARC_ARCS)                  \
	  num_arcs_this_cmd = MID_MAX_PERF_FILLPOLYARC_ARCS;            \
	else                                                            \
	  num_arcs_this_cmd = rem_arcs;                                 \
									\
	/*************************************************************  \
	 *                                                           *  \
	 * Write the Fill Polyarc command element including the      *  \
	 * chunk of arcs to the deferral buffer.  It will be written *  \
	 * to the FIFO when the deferral buffer fills up, or after   *  \
	 * the last Fill Polyarc command element has been written to *  \
	 * the deferral buffer.                                      *  \
	 *                                                           *  \
	 *************************************************************/ \
									\
	SE.hdr.halfword.length = sizeof(MIDFillPolyarc) +               \
				 (num_arcs_this_cmd * 12);              \
									\
	MID_WR_DEFERBUF(&SE , MID_WORDS(sizeof(MIDFillPolyarc) >> 2));  \
									\
	MID_WR_DEFERBUF( arcs_ptr_this_cmd,                             \
			 MID_WORDS( num_arcs_this_cmd * 3 ) );          \
									\
	arcs_ptr_this_cmd += num_arcs_this_cmd;                         \
      }                 /* End for loop to break up the input array of  \
			   arcs into smaller chunks                  */ \
									\
      /***************************************************************  \
       *                                                             *  \
       * Flush the deferral buffer so that all of the Fill Polyarc   *  \
       * command elements, along with any other command elements     *  \
       * present at the front of the deferral buffer, are written to *  \
       * the FIFO now.  Note that each Fill Polyarc command element  *  \
       * was written to the deferral buffer instead of written       *  \
       * directly to the FIFO because it takes less time to copy     *  \
       * each command element into the deferral buffer and then      *  \
       * issue a single DMA operation to write all of the command    *  \
       * elements to the FIFO than it takes to issue a single DMA    *  \
       * operation or multiple PIO operations to write each command  *  \
       * element directly to the FIFO.                               *  \
       *                                                             *  \
       ***************************************************************/ \
									\
      MID_DEFERBUF_FLUSH;                                               \
    }                   /* End input array of arcs is too big to fit    \
			   into a single Fill Polyarc command element*/ \
}                       /* End MID_FillPolyarc                       */

/************************************************************************/
/*                                                                      */
/*  Macro Name:  MID_Polytext8                                          */
/*                                                                      */
/*  Function:                                                           */
/*    Issues the Pedernales Polytext 8 command element.                 */
/*                                                                      */
/*    Polytext 8 command elements are not deferred.  Any data in the    */
/*    deferral buffer is written to the FIFO (circular buffer) first,   */
/*    then the Polytext 8 command element is written to the FIFO.       */
/*                                                                      */
/*  Input:                                                              */
/*    xstart (short)   - baseline starting position x coordinate,       */
/*                       relative to the origin of the window           */
/*    ystart (short)   - baseline starting position y coordinate,       */
/*                       relative to the origin of the window           */
/*    num_chars (int)  - number of characters to be drawn (must be <=   */
/*                       MID_MAX_PERF_POLYTEXT8_CHARS)                  */
/*    chars_ptr (unsigned char *) - pointer to the array of characters  */
/*                       to be drawn (each character is an 8-bit        */
/*                       index into the active font)                    */
/*                                                                      */
/*  Output:                                                             */
/*    None                                                              */
/*                                                                      */
/************************************************************************/

#define MID_Polytext8( xstart, ystart, num_chars, chars_ptr )           \
{                                                                       \
    unsigned short      char_array_len;                                 \
    MIDPolytext8        SE;                                             \
									\
    /*****************************************************************  \
     *                                                               *  \
     * All of the characters specified must fit into a single        *  \
     * Polytext 8 command element!                                   *  \
     *                                                               *  \
     *****************************************************************/ \
									\
    /*****************************************************************  \
     *                                                               *  \
     * Determine the length, in bytes, of the input array of         *  \
     * characters, rounded up to the nearest multiple of 4 bytes     *  \
     * (the length of the command element must be a multiple of 4    *  \
     * bytes).  If the input array does not contain a multiple of 4  *  \
     * characters, the last 1 - 3 bytes of the command element will  *  \
     * be meaningless.                                               *  \
     *                                                               *  \
     *****************************************************************/ \
									\
    char_array_len = ((num_chars + 3) >> 2) << 2;                       \
									\
    SE.hdr.halfword.length = sizeof(MIDPolytext8) + char_array_len;     \
    SE.hdr.halfword.op     = OP_POLYTEXT_8;				\
    SE.x      = xstart;                                                 \
    SE.y      = ystart;                                                 \
    SE.ccount = num_chars;                                              \
									\
    /*****************************************************************  \
     *                                                               *  \
     * The portion of the Polytext 8 command element that precedes   *  \
     * the characters is written to the deferral buffer first.  Then *  \
     * MID_WRITE_TO_FIFO is issued.  MID_WRITE_TO_FIFO flushes the   *  \
     * deferral buffer then writes the rest of the Polytext 8        *  \
     * command element to the FIFO as efficiently as possible.       *  \
     *                                                               *  \
     ****************************************************************/  \
									\
    MID_WR_DEFERBUF(&SE , MID_WORDS(sizeof(MIDPolytext8) >> 2));        \
									\
    MID_WRITE_TO_FIFO( chars_ptr, MID_WORDS( char_array_len >> 2 ) );   \
									\
}                       /* End MID_Polytext8                         */

/************************************************************************/
/*                                                                      */
/*  Macro Name:  MID_Polytext16                                         */
/*                                                                      */
/*  Function:                                                           */
/*    Issues the Pedernales Polytext 16 command element.                */
/*                                                                      */
/*    Polytext 16 command elements are not deferred.  Any data in the   */
/*    deferral buffer is written to the FIFO (circular buffer) first,   */
/*    then the Polytext 16 command element is written to the FIFO.      */
/*                                                                      */
/*  Input:                                                              */
/*    xstart (short)   - baseline starting position x coordinate,       */
/*                       relative to the origin of the window           */
/*    ystart (short)   - baseline starting position y coordinate,       */
/*                       relative to the origin of the window           */
/*    num_chars (int)  - number of characters to be drawn (must be <=   */
/*                       MID_MAX_PERF_POLYTEXT16_CHARS)                 */
/*    chars_ptr (unsigned short *) - pointer to the array of characters */
/*                       to be drawn (each character is a 16-bit        */
/*                       index into the active font)                    */
/*                                                                      */
/*  Output:                                                             */
/*    None                                                              */
/*                                                                      */
/************************************************************************/

#define MID_Polytext16( xstart, ystart, num_chars, chars_ptr )          \
{                                                                       \
    unsigned short      char_array_len;                                 \
    MIDPolytext16       SE;                                             \
									\
    /*****************************************************************  \
     *                                                               *  \
     * All of the characters specified must fit into a single        *  \
     * Polytext 16 command element!                                  *  \
     *                                                               *  \
     *****************************************************************/ \
									\
    /*****************************************************************  \
     *                                                               *  \
     * Determine the length, in bytes, of the input array of         *  \
     * characters, rounded up to the nearest multiple of 4 bytes     *  \
     * (the length of the command element must be a multiple of 4    *  \
     * bytes).  If the input array has an odd number of characters,  *  \
     * the last 2 bytes of the command element will be meaningless.  *  \
     *                                                               *  \
     *****************************************************************/ \
									\
    char_array_len = ((num_chars + 1) >> 1) << 2;                       \
									\
    SE.hdr.halfword.length = sizeof(MIDPolytext16) + char_array_len;    \
    SE.hdr.halfword.op     = OP_POLYTEXT_16;				\
    SE.x      = xstart;                                                 \
    SE.y      = ystart;                                                 \
    SE.ccount = num_chars;                                              \
									\
    /*****************************************************************  \
     *                                                               *  \
     * The portion of the Polytext 16 command element that precedes  *  \
     * the characters is written to the deferral buffer first.  Then *  \
     * MID_WRITE_TO_FIFO is issued.  MID_WRITE_TO_FIFO flushes the   *  \
     * deferral buffer then writes the rest of the Polytext 16       *  \
     * command element to the FIFO as efficiently as possible.       *  \
     *                                                               *  \
     ****************************************************************/  \
									\
    MID_WR_DEFERBUF(&SE , MID_WORDS(sizeof(MIDPolytext16) >> 2));       \
									\
    MID_WRITE_TO_FIFO( chars_ptr, MID_WORDS( char_array_len >> 2 ) );   \
									\
}                       /* End MID_Polytext16                        */

/************************************************************************/
/*                                                                      */
/*  Macro Name:  MID_Imagetext8                                         */
/*                                                                      */
/*  Function:                                                           */
/*    Issues the Pedernales Imagetext 8 command element.                */
/*                                                                      */
/*    Imagetext 8 command elements are not deferred.  Any data in the   */
/*    deferral buffer is written to the FIFO (circular buffer) first,   */
/*    then the Imagetext 8 command element is written to the FIFO.      */
/*                                                                      */
/*  Input:                                                              */
/*    xstart (short)   - baseline starting position x coordinate,       */
/*                       relative to the origin of the window           */
/*    ystart (short)   - baseline starting position y coordinate,       */
/*                       relative to the origin of the window           */
/*    num_chars (int)  - number of characters to be drawn (must be <=   */
/*                       MID_MAX_PERF_IMAGETEXT8_CHARS)                 */
/*    chars_ptr (unsigned char *) - pointer to the array of characters  */
/*                       to be drawn (each character is an 8-bit        */
/*                       index into the active font)                    */
/*                                                                      */
/*  Output:                                                             */
/*    None                                                              */
/*                                                                      */
/************************************************************************/

#define MID_Imagetext8( xstart, ystart, num_chars, chars_ptr )          \
{                                                                       \
    unsigned short      char_array_len;                                 \
    MIDImagetext8       SE;                                             \
									\
    /*****************************************************************  \
     *                                                               *  \
     * All of the characters specified must fit into a single        *  \
     * Imagetext 8 command element!                                  *  \
     *                                                               *  \
     *****************************************************************/ \
									\
    /*****************************************************************  \
     *                                                               *  \
     * Determine the length, in bytes, of the input array of         *  \
     * characters, rounded up to the nearest multiple of 4 bytes     *  \
     * (the length of the command element must be a multiple of 4    *  \
     * bytes).  If the input array does not contain a multiple of 4  *  \
     * characters, the last 1 - 3 bytes of the command element will  *  \
     * be meaningless.                                               *  \
     *                                                               *  \
     *****************************************************************/ \
									\
    char_array_len = ((num_chars + 3) >> 2) << 2;                       \
									\
    SE.hdr.halfword.length = sizeof(MIDImagetext8) + char_array_len;    \
    SE.hdr.halfword.op     = OP_IMAGETEXT_8;				\
    SE.x      = xstart;                                                 \
    SE.y      = ystart;                                                 \
    SE.ccount = num_chars;                                              \
									\
    /*****************************************************************  \
     *                                                               *  \
     * The portion of the Imagetext 8 command element that precedes  *  \
     * the characters is written to the deferral buffer first.  Then *  \
     * MID_WRITE_TO_FIFO is issued.  MID_WRITE_TO_FIFO flushes the   *  \
     * deferral buffer then writes the rest of the Imagetext 8       *  \
     * command element to the FIFO as efficiently as possible.       *  \
     *                                                               *  \
     ****************************************************************/  \
									\
    MID_WR_DEFERBUF(&SE , MID_WORDS(sizeof(MIDImagetext8) >> 2));       \
									\
    MID_WRITE_TO_FIFO( chars_ptr, MID_WORDS( char_array_len >> 2 ) );   \
									\
}                       /* End MID_Imagetext8                        */

/************************************************************************/
/*                                                                      */
/*  Macro Name:  MID_Imagetext16                                        */
/*                                                                      */
/*  Function:                                                           */
/*    Issues the Pedernales Imagetext 16 command element.               */
/*                                                                      */
/*    Imagetext 16 command elements are not deferred.  Any data in the  */
/*    deferral buffer is written to the FIFO (circular buffer) first,   */
/*    then the Imagetext 16 command element is written to the FIFO.     */
/*                                                                      */
/*  Input:                                                              */
/*    xstart (short)   - baseline starting position x coordinate,       */
/*                       relative to the origin of the window           */
/*    ystart (short)   - baseline starting position y coordinate,       */
/*                       relative to the origin of the window           */
/*    num_chars (int)  - number of characters to be drawn (must be <=   */
/*                       MID_MAX_PERF_IMAGETEXT16_CHARS)                */
/*    chars_ptr (unsigned short *) - pointer to the array of characters */
/*                       to be drawn (each character is a 16-bit        */
/*                       index into the active font)                    */
/*                                                                      */
/*  Output:                                                             */
/*    None                                                              */
/*                                                                      */
/************************************************************************/

#define MID_Imagetext16( xstart, ystart, num_chars, chars_ptr )         \
{                                                                       \
    unsigned short      char_array_len;                                 \
    MIDImagetext16      SE;                                             \
									\
    /*****************************************************************  \
     *                                                               *  \
     * All of the characters specified must fit into a single        *  \
     * Imagetext 16 command element!                                 *  \
     *                                                               *  \
     *****************************************************************/ \
									\
    /*****************************************************************  \
     *                                                               *  \
     * Determine the length, in bytes, of the input array of         *  \
     * characters, rounded up to the nearest multiple of 4 bytes     *  \
     * (the length of the command element must be a multiple of 4    *  \
     * bytes).  If the input array has an odd number of characters,  *  \
     * the last 2 bytes of the command element will be meaningless.  *  \
     *                                                               *  \
     *****************************************************************/ \
									\
    char_array_len = ((num_chars + 1) >> 1) << 2;                       \
									\
    SE.hdr.halfword.length = sizeof(MIDImagetext16) + char_array_len;   \
    SE.hdr.halfword.op     = OP_IMAGETEXT_16;				\
    SE.x      = xstart;                                                 \
    SE.y      = ystart;                                                 \
    SE.ccount = num_chars;                                              \
									\
    /*****************************************************************  \
     *                                                               *  \
     * The portion of the Imagetext 16 command element that precedes *  \
     * the characters is written to the deferral buffer first.  Then *  \
     * MID_WRITE_TO_FIFO is issued.  MID_WRITE_TO_FIFO flushes the   *  \
     * deferral buffer then writes the rest of the Imagetext 16      *  \
     * command element to the FIFO as efficiently as possible.       *  \
     *                                                               *  \
     ****************************************************************/  \
									\
    MID_WR_DEFERBUF(&SE , MID_WORDS(sizeof(MIDImagetext16) >> 2));      \
									\
    MID_WRITE_TO_FIFO( chars_ptr, MID_WORDS( char_array_len >> 2 ) );   \
									\
}                       /* End MID_Imagetext16                       */

/************************************************************************/
/*                                                                      */
/*  Macro Name:  MID_PushPixels                                         */
/*                                                                      */
/*  Function:                                                           */
/*    Issues the Pedernales Push Pixels command element.                */
/*                                                                      */
/*    If the portion of the bitmap data to be used does not fit into a  */
/*    single Push Pixels command element, this macro breaks up the      */
/*    bitmap into smaller pieces and issues several Push Pixels command */
/*    elements to fill the destination rectangle.                       */
/*                                                                      */
/*    Push Pixels command elements are not deferred.  Any data in the   */
/*    deferral buffer is written to the FIFO (circular buffer) first,   */
/*    then the Push Pixels command element(s) is written to the FIFO.   */
/*                                                                      */
/*  Input:                                                              */
/*    bitmap_width  (unsigned short)  - bitmap width in pixels          */
/*    bitmap_data_ptr (unsigned long *) - pointer to the bitmap data    */
/*                                      (the scan line pad must be 32   */
/*                                      bits)                           */
/*    dest_width  (unsigned short)    - width of the portion of the     */
/*                                      bitmap used (must be <= 65440)  */
/*    dest_height (unsigned short)    - height of the portion of the    */
/*                                      bitmap used                     */
/*    dest_x (short)                  - x coordinate of the upper left- */
/*                                      hand corner of the destination, */
/*                                      relative to the window origin   */
/*    dest_y (short)                  - y coordinate of the upper left- */
/*                                      hand corner of the destination, */
/*                                      relative to the window origin   */
/*                                                                      */
/*  Output:                                                             */
/*    None                                                              */
/*                                                                      */
/************************************************************************/

#define MID_PushPixels( bitmap_width, bitmap_data_ptr, dest_width,      \
			dest_height, dest_x, dest_y )                   \
{                                                                       \
    unsigned short      bitmap_len, row_len, total_row_len, rem_rows;   \
    unsigned long       *current_bitmap_ptr = bitmap_data_ptr;          \
    MIDPushPixels       SE;                                             \
									\
    SE.hdr.halfword.op = OP_PUSH_PIXELS;				\
									\
    /*****************************************************************  \
     *                                                               *  \
     * Determine how many words of bitmap data must be passed to the *  \
     * adapter.                                                      *  \
     *                                                               *  \
     ****************************************************************/  \
									\
    row_len    = (dest_width + 31) >> 5;   /* Num words in each row */  \
    bitmap_len = row_len * dest_height;                                 \
									\
    total_row_len = (bitmap_width + 31) >> 5;                           \
									\
    /*****************************************************************  \
     *                                                               *  \
     * If the bitmap data fits into a single Push Pixels command     *  \
     * element, then write the command element including the bitmap  *  \
     * data to the adapter.                                          *  \
     *                                                               *  \
     ****************************************************************/  \
									\
    if (bitmap_len <= MID_MAX_PERF_PUSHPIXELS_BITMAP_WORDS)             \
    {                                                                   \
      SE.hdr.halfword.length = sizeof(MIDPushPixels) + (bitmap_len << 2);\
      SE.height = dest_height;                                          \
      SE.width  = dest_width;                                           \
      SE.xorg   = dest_x;                                               \
      SE.yorg   = dest_y;                                               \
									\
      /***************************************************************  \
       *                                                             *  \
       * The Push Pixels command element is written to the deferral  *  \
       * buffer first since the input data must be formatted.        *  \
       *                                                             *  \
       **************************************************************/  \
									\
      MID_WR_DEFERBUF(&SE , MID_WORDS(sizeof(MIDPushPixels) >> 2));     \
									\
      for (rem_rows = dest_height; rem_rows > 0; rem_rows--)            \
      {                                                                 \
	MID_WR_DEFERBUF ( current_bitmap_ptr , MID_WORDS( row_len ) );  \
									\
	current_bitmap_ptr += total_row_len;                            \
      }                                                                 \
    }                                                                   \
    else                                                                \
    {                                                                   \
      int               total_rem_rows;                                 \
      unsigned short    num_rows_this_cmd, max_rows_per_cmd;            \
      short             yorg_this_cmd = dest_y;                         \
									\
      /***************************************************************  \
       *                                                             *  \
       * The bitmap data does not fit into a single Push Pixels      *  \
       * command element.  So break up the bitmap data into small    *  \
       * pieces and issue a Push Pixels command element for each     *  \
       * piece.  Each piece will consist of the maximum number of    *  \
       * complete rows that will fit into a Push Pixels command      *  \
       * element, except possibly the last piece.                    *  \
       *                                                             *  \
       ***************************************************************/ \
									\
      max_rows_per_cmd = MID_MAX_PERF_PUSHPIXELS_BITMAP_WORDS / row_len; \
									\
      for (total_rem_rows = dest_height; total_rem_rows > 0;            \
	   total_rem_rows -= max_rows_per_cmd)                          \
      {                                                                 \
	if (total_rem_rows >= max_rows_per_cmd)                         \
	  num_rows_this_cmd = max_rows_per_cmd;                         \
	else                                                            \
	  num_rows_this_cmd = total_rem_rows;                           \
									\
	/*************************************************************  \
	 *                                                           *  \
	 * Write the Push Pixels command element to the deferral     *  \
	 * buffer.  It will be written to the FIFO when the deferral *  \
	 * buffer fills up, or after the last Push Pixels command    *  \
	 * element has been written to the deferral buffer.  Note    *  \
	 * that each Push Pixels command element is written to the   *  \
	 * deferral buffer instead of written directly to the FIFO   *  \
	 * because the data must be formatted.  It turns out that    *  \
	 * this is more efficient anyway because a single DMA        *  \
	 * operation can be issued to write all of the command       *  \
	 * elements to the FIFO when we are done.                    *  \
	 *                                                           *  \
	 *************************************************************/ \
									\
	SE.hdr.halfword.length = sizeof(MIDPushPixels) +                \
				 ((num_rows_this_cmd * row_len) << 2);  \
	SE.height = num_rows_this_cmd;                                  \
	SE.width  = dest_width;                                         \
	SE.xorg   = dest_x;                                             \
	SE.yorg   = yorg_this_cmd;                                      \
									\
	yorg_this_cmd += num_rows_this_cmd;                             \
									\
	MID_WR_DEFERBUF(&SE , MID_WORDS(sizeof(MIDPushPixels) >> 2));   \
									\
	for (rem_rows = num_rows_this_cmd; rem_rows > 0; rem_rows--)    \
	{                                                               \
	  MID_WR_DEFERBUF ( current_bitmap_ptr , MID_WORDS( row_len ) ); \
									\
	  current_bitmap_ptr += total_row_len;                          \
	}                                                               \
      }                 /* End for loop to break up the bitmap into     \
			   small pieces                              */ \
    }                   /* End bitmap data does not fit into a single   \
			   Push Pixels command element               */ \
									\
    /*****************************************************************  \
     *                                                               *  \
     * Flush the deferral buffer so that the Push Pixels command     *  \
     * element(s), along with any other command elements present at  *  \
     * the front of the deferral buffer, is written to the FIFO now. *  \
     *                                                               *  \
     *****************************************************************/ \
									\
    MID_DEFERBUF_FLUSH;                                                 \
}                       /* End MID_PushPixels                        */

/************************************************************************/
/*                                                                      */
/*  Macro Name:  MID_FillSpans                                          */
/*                                                                      */
/*  Function:                                                           */
/*    Issues the Pedernales Fill Spans command element.                 */
/*                                                                      */
/*    If all of the spans specified do not fit into a single Fill Spans */
/*    command element, this macro breaks up the spans into small groups */
/*    and issues several Fill Spans command elements to render all of   */
/*    the spans.                                                        */
/*                                                                      */
/*    Fill Spans command elements are not deferred.  Any data in the    */
/*    deferral buffer is written to the FIFO (circular buffer) first,   */
/*    then the Fill Spans command element(s) is written to the FIFO.    */
/*                                                                      */
/*  Input:                                                              */
/*    num_spans (int)           - number of spans to be filled          */
/*    points_ptr (DDXPointPtr)  - pointer to an array of starting       */
/*                                points                                */
/*    widths_ptr (int *)        - pointer to an array of widths         */
/*                                                                      */
/*  Output:                                                             */
/*    None                                                              */
/*                                                                      */
/************************************************************************/

#define MID_FillSpans( num_spans, points_ptr, widths_ptr )              \
{                                                                       \
    int                 rem_spans;                                      \
    DDXPointPtr         current_point_ptr = points_ptr;                 \
    int                 *current_width_ptr = widths_ptr;                \
    MIDFillSpans        SE;                                             \
									\
    SE.hdr.halfword.op = OP_FILL_SPANS;					\
									\
    /*****************************************************************  \
     *                                                               *  \
     * If all of the spans specified fit into a single Fill Spans    *  \
     * command element, then write the command element including all *  \
     * of the spans to the adapter.                                  *  \
     *                                                               *  \
     ****************************************************************/  \
									\
    if (num_spans <= MID_MAX_PERF_FILLSPANS_SPANS)                      \
    {                                                                   \
      SE.hdr.halfword.length = sizeof(MIDFillSpans) + (num_spans << 3); \
									\
      /***************************************************************  \
       *                                                             *  \
       * The Fill Spans command element is written to the deferral   *  \
       * buffer first since the input data must be formatted.        *  \
       *                                                             *  \
       **************************************************************/  \
									\
      MID_WR_DEFERBUF(&SE , MID_WORDS(sizeof(MIDFillSpans) >> 2));      \
									\
      for (rem_spans = num_spans; rem_spans > 0; rem_spans--)           \
      {                                                                 \
	MID_WR_DEFERBUF ( current_point_ptr , MID_WORDS( 1 ) );         \
									\
	MID_WR_DEFERBUF ( current_width_ptr , MID_WORDS( 1 ) );         \
									\
	current_point_ptr++;						\
    	current_width_ptr++;						\
      }                                                                 \
    }                                                                   \
    else                                                                \
    {                                                                   \
      int               total_rem_spans;                                \
      unsigned short    num_spans_this_cmd;                             \
									\
      /***************************************************************  \
       *                                                             *  \
       * All of the spans specified do not fit into a single Fill    *  \
       * Spans command element.  So break up the spans into small    *  \
       * groups and issue a Fill Spans cmd element to render each    *  \
       * group.  Each group will consist of MID_MAX_PERF_FILLSPANS_  *  \
       * SPANS spans, except possibly the last group.                *  \
       *                                                             *  \
       ***************************************************************/ \
									\
      for (total_rem_spans = num_spans; total_rem_spans > 0;            \
	   total_rem_spans -= MID_MAX_PERF_FILLSPANS_SPANS)             \
      {                                                                 \
	if (total_rem_spans >= MID_MAX_PERF_FILLSPANS_SPANS)            \
	  num_spans_this_cmd = MID_MAX_PERF_FILLSPANS_SPANS;            \
	else                                                            \
	  num_spans_this_cmd = total_rem_spans;                         \
									\
	/*************************************************************  \
	 *                                                           *  \
	 * Write the Fill Spans command element including the group  *  \
	 * of spans to the deferral buffer.  It will be written to   *  \
	 * the FIFO when the deferral buffer fills up, or after the  *  \
	 * last Fill Spans command element has been written to the   *  \
	 * deferral buffer.  Note that each Fill Spans command       *  \
	 * element is written to the deferral buffer instead of      *  \
	 * written directly to the FIFO because the data must be     *  \
	 * formatted.  It turns out that this is more efficient      *  \
	 * anyway because a single DMA operation can be issued to    *  \
	 * write all of the command elements to the FIFO when we are *  \
	 * done.                                                     *  \
	 *                                                           *  \
	 *************************************************************/ \
									\
	SE.hdr.halfword.length = sizeof(MIDFillSpans) +                 \
				 (num_spans_this_cmd << 3);             \
									\
	MID_WR_DEFERBUF(&SE , MID_WORDS(sizeof(MIDFillSpans) >> 2));    \
									\
	for (rem_spans = num_spans_this_cmd; rem_spans > 0; rem_spans--)\
	{                                                               \
	  MID_WR_DEFERBUF ( current_point_ptr , MID_WORDS( 1 ) );       \
									\
	  MID_WR_DEFERBUF ( current_width_ptr , MID_WORDS( 1 ) );       \
 									\
   	  current_point_ptr++;						\
	  current_width_ptr++;						\
	}                                                               \
      }                 /* End for loop to break up the total number    \
			   of specified spans into small groups      */ \
    }                   /* End total number of spans do not fit into a  \
			   single Fill Spans command element         */ \
									\
    /*****************************************************************  \
     *                                                               *  \
     * Flush the deferral buffer so that the Fill Spans command      *  \
     * element(s), along with any other command elements present at  *  \
     * the front of the deferral buffer, is written to the FIFO now. *  \
     *                                                               *  \
     *****************************************************************/ \
									\
    MID_DEFERBUF_FLUSH;                                                 \
}                       /* End MID_FillSpans                         */

#endif /* _H_MID_HW_FIFO_2DM1 */

