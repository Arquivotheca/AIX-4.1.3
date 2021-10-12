/* @(#)12       1.4.2.5  src/bos/kernext/disp/ped/pedmacro/hw_PCBrms.h, pedmacro, bos411, 9428A410j 2/28/94 20:22:50 */
/*
 *   COMPONENT_NAME: PEDMACRO
 *
 *   FUNCTIONS: 
 *		
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/************************************************************************/
/*									*/
/*	PEDERNALES HW MACRO PROGRAMMING	INTERFACE			*/
/*									*/
/************************************************************************/


/************************************************************************/
/************************************************************************/
/* NOTE:  Right	and left shifting by 2 designates a division by	4, not	*/
/*	  a shifting for masking purposes, unless otherwise specified.	*/
/************************************************************************/
/************************************************************************/


#ifndef	_H_MID_HW_PCB_RMS
#define	_H_MID_HW_PCB_RMS

#include <hw_se_types.h>

/************************************************************************/
/* GAI RMS PCB COMMAND ELEMENT TYPEDEFS					*/
/*									*/
/*	The names of the members of the	structures described below	*/
/*	are derived from the Pedernales	Software Interface		*/
/*	Specification v. 0.9						*/
/*									*/
/************************************************************************/

	/*----------------------------------------------*
	 * Check if the	header has already been	defined	*
	 *----------------------------------------------*/

#ifndef	_H_MID_PCB_HDR
#define	_H_MID_PCB_HDR

	/*----------------------------------------------*
	 * Definition for the header information of	*
	 * each	structure element			*
	 *----------------------------------------------*/

typedef	union	_MIDpcbhdr
{
	ulong	 word;
	struct	_PCBhalfword
	{
		ushort	length;		/* length of SE	in bytes */
		ushort	op;		/* opcode for SE */
	} halfword;
} MIDpcbhdr;

#endif	/* _H_MID_PCB_HDR */


	/*----------------------------------------------*
	 * The section below defines a PCB command	*
	 * element typedef for the set of PCB commands	*
	 * which are of	a fixed	length and whose fields	*
	 * are all regularly defined			*
	 *----------------------------------------------*/

	/*----------------------------------------------*
	 * Adapter Control Commands (fixed)		*
	 *----------------------------------------------*/

/*
 * PEDERNALES ScreenBlankingControl command packet structure
 */
typedef	struct _MIDScreenBlankingControl
{
	MIDpcbhdr		hdr;	/* SE header info (len/opcode)	   */
	ulong			blanking;
}	MIDScreenBlankingControl,
	*pMIDScreenBlankingControl;

/*
 * PEDERNALES RequestToDMAStructureElements command packet structure
 */
typedef	struct _MIDRequestToDMAStructureElements
{
	MIDpcbhdr		hdr;	/* SE header info (len/opcode) */
	ushort			corr , resv;
	ulong			haddr;
	ulong			hlength;
}	MIDRequestToDMAStructureElements,
	*pMIDRequestToDMAStructureElements;

/*
 * PEDERNALES CRTCControl command packet structure
 */
typedef	struct _MIDCRTCControl
{
	MIDpcbhdr		hdr;	/* SE header info (len/opcode) */
	ulong			lcir;
	ushort			hbet , hbst;
	ushort			hset , hsst;
	ushort			vbet , vbst;
	ushort			vset , vsst;
	ulong			flags;
}	MIDCRTCControl,
	*pMIDCRTCControl;

/*
 * PEDERNALES Diagnostics command packet structure
 */
typedef	struct _MIDDiagnostics
{
	MIDpcbhdr		hdr;	/* SE header info (len/opcode) */
	ulong			test;
}	MIDDiagnostics,
	*pMIDDiagnostics;

/*
 * PEDERNALES ContextStateUpdateComplete command packet	structure
 */
typedef	struct _MIDContextStateUpdateComplete
{
	MIDpcbhdr		hdr;	/* SE header info (len/opcode) */
}	MIDContextStateUpdateComplete,
	*pMIDContextStateUpdateComplete;

/*
 * PEDERNALES ContextMemoryPinned command packet structure
 */
typedef	struct _MIDContextMemoryPinned
{
	MIDpcbhdr		hdr;	/* SE header info (len/opcode) */
	ushort			corr, resv;
	ulong			haddr;
	ulong			hlength;
	ulong			ctxt_id;
}	MIDContextMemoryPinned,
	*pMIDContextMemoryPinned;


	/*----------------------------------------------*
	 * Clear Control Commands (fixed)		*
	 *----------------------------------------------*/

/*
 * PEDERNALES ClearControlPlanes command packet	structure
 */
typedef	struct _MIDClearControlPlanesPCB
{
	MIDpcbhdr		hdr;	/* SE header info (len/opcode) */
	ulong			cpvalue;
	ulong			cpmask;
	short			x , y;
	ushort			w , h;
	ushort			wcflag , wc_id;
}	MIDClearControlPlanesPCB,
	*pMIDClearControlPlanesPCB;


	/*----------------------------------------------*
	 * Cursor Control Commands (fixed)		*
	 *----------------------------------------------*/

/*
 * PEDERNALES DefineCursor command packet structure
 */
typedef	struct _MIDDefineCursor
{
	MIDpcbhdr		hdr;	/* SE header info (len/opcode) */
	ushort			c_id, resv;
}	MIDDefineCursor,
	*pMIDDefineCursor;

/*
 * PEDERNALES SetActiveCursor command packet structure
 */
typedef	struct _MIDSetActiveCursor
{
	MIDpcbhdr		hdr;	/* SE header info (len/opcode) */
	ushort			c_id , resv;
	short			icx , icy;
}	MIDSetActiveCursor,
	*pMIDSetActiveCursor;

/*
 * PEDERNALES MoveCursor command packet	structure
 */
typedef	struct _MIDMoveCursor
{
	MIDpcbhdr		hdr;	/* SE header info (len/opcode) */
	short			cx , cy;
}	MIDMoveCursor,
	*pMIDMoveCursor;

/*
 * PEDERNALES HideShowActiveCursor command packet structure
 */
typedef	struct _MIDHideShowActiveCursor
{
	MIDpcbhdr		hdr;	/* SE header info (len/opcode) */
	ulong			oper;
}	MIDHideShowActiveCursor,
	*pMIDHideShowActiveCursor;

/*
 * PEDERNALES BlinkCursorColor command packet structure
 */
typedef	struct _MIDBlinkCursorColor
{
	MIDpcbhdr		hdr;	/* SE header info (len/opcode) */
	ushort			c_id , resv;
	ulong			index;
	long			orig_time;
	long			alt_time;
	ulong			alt_ccolor;
}	MIDBlinkCursorColor,
	*pMIDBlinkCursorColor;


	/*----------------------------------------------*
	 * Font	Support	Commands (fixed)		*
	 *----------------------------------------------*/

/*
 * PEDERNALES FontPinnedUnpinned command packet	structure
 */
typedef	struct _MIDFontPinnedUnpinned
{
	MIDpcbhdr		hdr;	/* SE header info (len/opcode) */
	ushort			corr, resv;
	ulong			haddr;
	ulong			len;
	ulong			gfid_pinned;
	ulong			gfid_unpinned;
}	MIDFontPinnedUnpinned,
	*pMIDFontPinnedUnpinned;

/*
 * PEDERNALES FontMustBeUnpinned command packet	structure
 */
typedef	struct _MIDFontMustBeUnpinned
{
	MIDpcbhdr		hdr;	/* SE header info (len/opcode) */
	ulong			gfid;
}	MIDFontMustBeUnpinned,
	*pMIDFontMustBeUnpinned;

/*
 * PEDERNALES FontRequestReceived command packet structure
 */
typedef	struct _MIDFontRequestReceived
{
	MIDpcbhdr		hdr;	/* SE header info (len/opcode) */
}	MIDFontRequestReceived,
	*pMIDFontRequestReceived;


	/*----------------------------------------------*
	 * Color Processing Control Commands (fixed)	*
	 *----------------------------------------------*/

typedef	struct _MIDLoadFrameBufferColorTable
{
	MIDpcbhdr		hdr;	/* SE header info (len/opcode) */
	ushort			ct_id, resv;
	ulong			stindex;
	long			ctlen;
}	MIDLoadFrameBufferColorTable,
	*pMIDLoadFrameBufferColorTable;

typedef	struct _MIDBlinkFrameBufferColorTable
{
	MIDpcbhdr		hdr;	/* SE header info (len/opcode) */
	ushort			ct_id, resv;
	ulong			index;
	long			orig_time;
	long			alt_time;
	ulong			alt_fbcolor;
}	MIDBlinkFrameBufferColorTable,
	*pMIDBlinkFrameBufferColorTable;

typedef	struct _MIDLoadOverlayPlanesColorTable
{
	MIDpcbhdr		hdr;	/* SE header info (len/opcode) */
	ulong			opcolor1;
	ulong			opcolor2;
	ulong			opcolor3;
}	MIDLoadOverlayPlanesColorTable,
	*pMIDLoadOverlayPlanesColorTable;

typedef	struct _MIDBlinkOverlayPlanesColorTable
{
	MIDpcbhdr		hdr;	/* SE header info (len/opcode) */
	ulong			index;
	long			orig_time;
	long			alt_time;
	ulong			alt_opcolor;
}	MIDBlinkOverlayPlanesColorTable,
	*pMIDBlinkOverlayPlanesColorTable;

typedef	struct _MIDMultimap
{
	MIDpcbhdr		hdr;	/* SE header info (len/opcode) */
}	MIDMultimap,
	*pMIDMultimap;

typedef	struct _MIDOnemap
{
	MIDpcbhdr		hdr;	/* SE header info (len/opcode) */
}	MIDOnemap,
	*pMIDOnemap;

typedef	struct _MIDSetmap
{
	MIDpcbhdr		hdr;	/* SE header info (len/opcode) */
	ushort			resv, map;
}	MIDSetmap,
	*pMIDSetmap;


	/*----------------------------------------------*
	 * The section below defines a PCB command	*
	 * element typedef for the set of PCB commands	*
	 * which are variable in length	or which have	*
	 * a format that can vary based	on a value of	*
	 * one of the fields				*
	 *----------------------------------------------*/

	/*----------------------------------------------*
	 * Adapter Control Commands (variable)		*
	 *----------------------------------------------*/

/*
 * PEDERNALES SetWindowParametersPCB command packet structure
 */
/************************************************************************/
/* NOTE	1:  This is only the fixed portion of the command element.	*/
/*	    A variable number of CornerRectangles, which defines the	*/
/*	    visible area, follows this,	the fixed portion of the	*/
/*	    command.							*/
/*									*/
/* NOTE	2:  Uses include file "hw_se_types.h (to define	the rectangle	*/
/*	    structs).							*/
/************************************************************************/
typedef	struct _MIDSetWindowParametersPCB
{
	MIDpcbhdr		hdr;	/* SE header info (len/opcode) */
	ushort			w_id , woflags;
	MIDCornerRectangle	window_extents;
}	MIDSetWindowParametersPCB,
	*pMIDSetWindowParametersPCB;

/*
 * PEDERNALES AssocOneWindowWithColorPalette command packet structure
 */
typedef	struct _MIDAssocOneWindowWithColorPalette
{
	MIDpcbhdr		hdr;	/* SE header info (len/opcode) */
	ushort			w_id, cp_id;
}	MIDAssocOneWindowWithColorPalette,
	*pMIDAssocOneWindowWithColorPalette;


/*
 * PEDERNALES AssocWindowWithColorPalette command packet structure
 */
typedef	struct _MIDAssocWindowWithColorPalette
{
	MIDpcbhdr		hdr;	/* SE header info (len/opcode) */
}	MIDAssocWindowWithColorPalette,
	*pMIDAssocWindowWithColorPalette;


	/*----------------------------------------------*
	 * Color Processing Control Commands (variable)	*
	 *----------------------------------------------*/

/*
 * PEDERNALES CycleColorMaps command packet structure
 */
typedef	struct _MIDCycleColorMaps
{
	MIDpcbhdr		hdr;	/* SE header info (len/opcode) */
}	MIDCycleColorMaps,
	*pMIDCycleColorMaps;



/************************************************************************/
/* MACROS TO BUILD GAI RMS STRUCTURE ELEMENTS				*/
/*									*/
/*	The names of the members of the	structures described below	*/
/*	are derived from the Pedernales	Software Interface		*/
/*	Specification v. 0.9						*/
/*									*/
/************************************************************************/

	/*----------------------------------------------*
	 * The section below defines a set of macros	*
	 * to build PCB	command	elements		*
	 * which are of	a fixed	length and whose fields	*
	 * are all regularly defined			*
	 *----------------------------------------------*/

	/*----------------------------------------------*
	 * Adapter Control Commands (fixed)		*
	 *----------------------------------------------*/

#define	MID_SCREEN_BLANKING_OFF		0
#define	MID_SCREEN_BLANKING_ON		1

#define	MID_ScreenBlankingControl( blanking_param )			\
{									\
    MIDScreenBlankingControl	SE;					\
									\
    SE.hdr.halfword.length = ( ushort )					\
				sizeof(	MIDScreenBlankingControl );	\
    SE.blanking		   = ( ulong )	blanking_param;			\
									\
    MID_WR_PCB(	&SE , (	sizeof(	MIDScreenBlankingControl ) >> 2	) );	\
									\
}

#define	MID_RequestToDMAStructureElements( corr_param ,	haddr_param ,	\
						hlength_param )		\
{									\
    MIDRequestToDMAStructureElements	SE;				\
									\
    SE.hdr.halfword.length = ( ushort )					\
			sizeof(	MIDRequestToDMAStructureElements );	\
    SE.hdr.halfword.op	   = ( ushort )	OP_REQUEST_TO_DMA_STRUCTURE_ELEMENT ;\
    SE.corr		   = ( ushort )	corr_param;			\
    SE.resv		   = ( ushort )	0;				\
    SE.haddr		   = ( ulong )	haddr_param;			\
    SE.hlength		   = ( ulong )	hlength_param;			\
									\
    MID_WR_PCB(	&SE ,							\
		( sizeof( MIDRequestToDMAStructureElements ) >>	2 ) );	\
									\
}

#define	MID_CRTCControl( lcir_param , hbet_param , hbst_param ,		\
				hset_param , hsst_param	, vbet_param ,	\
				vbst_param , vset_param	, vsst_param ,	\
				flags_param )				\
{									\
    MIDCRTCControl	SE;						\
									\
    SE.hdr.halfword.length = ( ushort )	sizeof(	MIDCRTCControl );	\
    SE.hdr.halfword.op	   = ( ushort )	OP_CRTC_CONTROL ;		\
    SE.lcir		   = ( ulong )	lcir_param;			\
    SE.hbet		   = ( ushort )	hbet_param;			\
    SE.hbst		   = ( ushort )	hbst_param;			\
    SE.hset		   = ( ushort )	hset_param;			\
    SE.hsst		   = ( ushort )	hsst_param;			\
    SE.vbet                = ( ushort ) vbet_param;                     \
    SE.vbst		   = ( ushort )	vbst_param;			\
    SE.vset		   = ( ushort )	vset_param;			\
    SE.vsst		   = ( ushort )	vsst_param;			\
    SE.flags		   = ( ulong )	flags_param;			\
									\
    MID_WR_PCB(	&SE , (	sizeof(	MIDCRTCControl ) >> 2 )	);		\
									\
}

/*
 *  PEDERNALES CRTC values
 */

#define LCIR_PARAM	0x00000000

/*
 * PED and LEGA3 60 Hz.
 */
#define HBET_PARAM_60	0x0160
#define HBST_PARAM_60	0x0100
#define P_HSET_PARAM_60	0x012e
#define P_HSST_PARAM_60	0x0106
#define VBET_PARAM_60	0x0420
#define VBST_PARAM_60	0x0400
#define VSET_PARAM_60	0x0405
#define VSST_PARAM_60	0x0402
/*
 * LEGA and LEGA2 unique 60 Hz. 
 */
#define L_HSET_PARAM_60	0x012d
#define L_HSST_PARAM_60	0x0105
#define CRTC_FLAGS_60	0x00000000

/*
 * PED and LEGA3 77 Hz.
 */
#define HBET_PARAM_77	0x016c
#define HBST_PARAM_77	0x0100
#define P_HSET_PARAM_77	0x0135
#define P_HSST_PARAM_77	0x010c
#define VBET_PARAM_77	0x0420
#define VBST_PARAM_77	0x0400
#define VSET_PARAM_77	0x0405
#define VSST_PARAM_77	0x0402
/*
 * LEGA2 unique 77 Hz. 
 */
#define L_HSET_PARAM_77	0x0131
#define L_HSST_PARAM_77	0x0108
#define CRTC_FLAGS_77	0x00000001




#define	MID_Diagnostics( test_param )					\
{									\
    MIDDiagnostics	SE;						\
									\
    SE.hdr.halfword.length = ( ushort )	sizeof(	MIDDiagnostics );	\
    SE.hdr.halfword.op	   = ( ushort )	OP_DIAGNOSTICS ;		\
    SE.test		   = ( ulong ) test_param;			\
									\
    MID_WR_PCB(	&SE , (	sizeof(	MIDDiagnostics ) >> 2 )	);		\
									\
}

#define	MID_ContextStateUpdateComplete(	 )				\
{									\
    MIDContextStateUpdateComplete	SE;				\
									\
    SE.hdr.halfword.length = ( ushort )					\
			sizeof(	MIDContextStateUpdateComplete );	\
    SE.hdr.halfword.op	   = ( ushort )	OP_CONTEXT_STATE_UPDATE_COMPLETE ;\
									\
    MID_WR_PCB(	&SE ,							\
		( sizeof( MIDContextStateUpdateComplete	) >> 2 ) );	\
									\
}

#define	MID_ContextMemoryPinned( corr_param , haddr_param ,		\
					hlength_param ,	ctxt_id_param )	\
{									\
    MIDContextMemoryPinned	SE;					\
									\
    SE.hdr.halfword.length = ( ushort )					\
				sizeof(	MIDContextMemoryPinned );	\
    SE.hdr.halfword.op	   = ( ushort )	OP_CONTEXT_MEMORY_PINNED ;	\
    SE.corr		   = ( ushort )	corr_param;			\
    SE.resv		   = ( ushort )	0;				\
    SE.haddr		   = ( ulong )	haddr_param;			\
    SE.hlength		   = ( ulong )	hlength_param;			\
    SE.ctxt_id		   = ( ulong )	ctxt_id_param;			\
									\
    MID_WR_PCB(	&SE , (	sizeof(	MIDContextMemoryPinned ) >> 2 )	);	\
									\
}





	/*----------------------------------------------*
	 * Cursor Control Commands (fixed)		*
	 *----------------------------------------------*/

/************************************************************************/
/*									*/
/*  Macro Name:	 MID_DefineCursor					*/
/*									*/
/*  Function:								*/
/*    Defines a	cursor on the adapter.	This macro uses	a change mask	*/
/*    to determine which attributes of a hardware cursor have changed.	*/
/*    Each of the mask possiblities is checked.	 If set	the proper new	*/
/*    attribute	values are written into	the Cursor Image Command Block	*/
/*    memory on	the adapter.						*/
/*    All of the parameters are	optional except	"c_id_param" and	*/
/*    "changemask_param".						*/
/*									*/
/*  Input:								*/
/*    c_id_param       -- Which	of the hardware	cursors	to define.	*/
/*    cflags_param     -- Bit flags defining the type of cursor.	*/
/*    wx_param	       -- X coordinate of the cross hair cursor's	*/
/*			  window origin	(upper left corner).		*/
/*    wy_param	       -- Y coordinate of the cross hair cursor's	*/
/*			  window origin	(upper left corner).		*/
/*    ww_param	       -- Width	of the cross hair cursor's window.	*/
/*    wh_param	       -- Height of the	cross hair cursor's window.	*/
/*    ccolor?_param    -- The three colors to use for the cursor	*/
/*			  (color 0 is always transparency).		*/
/*    chsx_param       -- Hot Spot X coordinate	relative to the	cursor	*/
/*			  glyph	origin.					*/
/*    chsy_param       -- Hot Spot Y coordinate	relative to the	cursor	*/
/*			  glyph	origin.					*/
/*    cimage_param     -- The actual bit image of the glyph.		*/
/*    count_param      -- The number of	words in "cimage_param".	*/
/*    changemask_param -- Bit flags defining what attributes to		*/
/*			  download to adapter memory.			*/
/*									*/
/*  Output:								*/
/*    None								*/
/*									*/
/************************************************************************/

#define	MID_NO_UPDATE_CURSOR_IMAGE		( ( ushort ) (0) )
#define	MID_UPDATE_CURSOR_IMAGE			( ( ushort ) (1	<< 15) )

#define	MID_ENABLE_1_CURSOR_PLANES		( ( ushort ) (0) )
#define	MID_ENABLE_2_CURSOR_PLANES		( ( ushort ) (1	<< 14) )

#define	MID_NO_PATTERN_CURSOR_ENABLE		( ( ushort ) (0) )
#define	MID_PATTERN_CURSOR_ENABLE		( ( ushort ) (1	<< 6) )

#define	MID_NO_CROSS_HAIR_CURSOR_ENABLE		( ( ushort ) (0) )
#define	MID_CROSS_HAIR_CURSOR_ENABLE		( ( ushort ) (1	<< 5) )

#define	MID_XOR_CURSORS				( ( ushort ) (0) )
#define	MID_OR_CURSORS				( ( ushort ) (1	<< 4) )

#define	MID_CROSS_HAIR_CURSOR_TRANSPARENT	( ( ushort ) (0) )
#define	MID_CROSS_HAIR_CURSOR_CCOLOR1		( ( ushort ) (1	<< 2) )
#define	MID_CROSS_HAIR_CURSOR_CCOLOR2		( ( ushort ) (2	<< 2) )
#define	MID_CROSS_HAIR_CURSOR_CCOLOR3		( ( ushort ) (3	<< 2) )

#define	MID_CROSS_HAIR_CURSOR_THICKNESS1	( ( ushort ) (0) )
#define	MID_CROSS_HAIR_CURSOR_THICKNESS3	( ( ushort ) (1) )
#define	MID_CROSS_HAIR_CURSOR_THICKNESS5	( ( ushort ) (2) )
#define	MID_CROSS_HAIR_CURSOR_THICKNESS7	( ( ushort ) (3) )

#define	MID_CHANGE_CURSOR_FLAGS				( 1 << 0 )
#define	MID_CHANGE_CROSS_HAIR_WINDOW_COORDINATES	( 1 << 1 )
#define	MID_CHANGE_CROSS_HAIR_WINDOW_DIMENSIONS		( 1 << 2 )
#define	MID_CHANGE_CURSOR_COLORS			( 1 << 3 )
#define	MID_CHANGE_HOT_SPOT_COORDINATES			( 1 << 4 )
#define	MID_CHANGE_CURSOR_IMAGE				( 1 << 5 )


#ifdef MID_DD

#define	MID_DefineCursor( c_id_param , cflags_param , wx_param ,	\
				wy_param , ww_param , wh_param ,	\
				ccolor1_param ,	ccolor2_param ,		\
				ccolor3_param ,	chsx_param ,		\
				chsy_param , cimage_param ,		\
				count_param , changemask_param )	\
{									\
    MIDDefineCursor	SE;						\
									\
    MID_POLL_CICB_EQ_MASK_TO( MID_CICB_ID_FLAGS, 0x00ff0000, 0xffff0000, 200000 )	\
									\
    MID_WR_CICB_VALUE( MID_CICB_ID_FLAGS,				\
			 (((ulong) c_id_param) << 16 | cflags_param) )	\
									\
    if ( changemask_param & MID_CHANGE_CROSS_HAIR_WINDOW_COORDINATES ) {\
	MID_WR_CICB_VALUE( MID_CICB_CROSS_UPPER_LEFT_XY,		\
				(((ulong) wx_param) << 16 | wy_param) )	\
    }									\
									\
    if ( changemask_param & MID_CHANGE_CROSS_HAIR_WINDOW_DIMENSIONS ) {	\
	MID_WR_CICB_VALUE( MID_CICB_CROSS_WIDTH_HEIGHT,			\
				(((ulong) ww_param) << 16 | wh_param) )	\
    }									\
									\
    if ( changemask_param & MID_CHANGE_CURSOR_COLORS ) {		\
	MID_WR_CICB_VALUE( MID_CICB_COLOR_1, ccolor1_param )		\
	MID_WR_CICB_VALUE( MID_CICB_COLOR_2, ccolor2_param )		\
	MID_WR_CICB_VALUE( MID_CICB_COLOR_3, ccolor3_param )		\
    }									\
									\
    if ( changemask_param & MID_CHANGE_HOT_SPOT_COORDINATES ) {		\
	MID_WR_CICB_VALUE( MID_CICB_HOT_SPOT_XY,			\
			 (((ulong) chsx_param) << 16 | chsy_param) )	\
    }									\
									\
    if ( changemask_param & MID_CHANGE_CURSOR_IMAGE ) {			\
	MID_WR_CICB_ARRAY( MID_CICB_IMAGE_DATA,	cimage_param,		\
							count_param )	\
    }									\
									\
    SE.hdr.halfword.length = ( ushort )	sizeof(	MIDDefineCursor	);	\
    SE.hdr.halfword.op	   = ( ushort )	OP_DEFINE_CURSOR ;		\
    SE.c_id		   = ( ushort )	c_id_param;			\
    SE.resv		   = ( ushort )	0;				\
									\
    MID_WR_PCB(	&SE , (	sizeof(	MIDDefineCursor	) >> 2 ) );		\
									\
}
#else

#define	MID_DefineCursor( c_id_param , cflags_param , wx_param ,	\
				wy_param , ww_param , wh_param ,	\
				ccolor1_param ,	ccolor2_param ,		\
				ccolor3_param ,	chsx_param ,		\
				chsy_param , cimage_param ,		\
				count_param , changemask_param )	\
{									\
    MIDDefineCursor	SE;						\
									\
    MID_POLL_CICB_EQ_MASK( MID_CICB_ID_FLAGS, 0x00ff0000, 0xffff0000 )	\
									\
    MID_WR_CICB_VALUE( MID_CICB_ID_FLAGS,				\
			 (((ulong) c_id_param) << 16 | cflags_param) )	\
									\
    if ( changemask_param & MID_CHANGE_CROSS_HAIR_WINDOW_COORDINATES ) {\
	MID_WR_CICB_VALUE( MID_CICB_CROSS_UPPER_LEFT_XY,		\
				(((ulong) wx_param) << 16 | wy_param) )	\
    }									\
									\
    if ( changemask_param & MID_CHANGE_CROSS_HAIR_WINDOW_DIMENSIONS ) {	\
	MID_WR_CICB_VALUE( MID_CICB_CROSS_WIDTH_HEIGHT,			\
				(((ulong) ww_param) << 16 | wh_param) )	\
    }									\
									\
    if ( changemask_param & MID_CHANGE_CURSOR_COLORS ) {		\
	MID_WR_CICB_VALUE( MID_CICB_COLOR_1, ccolor1_param )		\
	MID_WR_CICB_VALUE( MID_CICB_COLOR_2, ccolor2_param )		\
	MID_WR_CICB_VALUE( MID_CICB_COLOR_3, ccolor3_param )		\
    }									\
									\
    if ( changemask_param & MID_CHANGE_HOT_SPOT_COORDINATES ) {		\
	MID_WR_CICB_VALUE( MID_CICB_HOT_SPOT_XY,			\
			 (((ulong) chsx_param) << 16 | chsy_param) )	\
    }									\
									\
    if ( changemask_param & MID_CHANGE_CURSOR_IMAGE ) {			\
	MID_WR_CICB_ARRAY( MID_CICB_IMAGE_DATA,	cimage_param,		\
							count_param )	\
    }									\
									\
    SE.hdr.halfword.length = ( ushort )	sizeof(	MIDDefineCursor	);	\
    SE.hdr.halfword.op	   = ( ushort )	OP_DEFINE_CURSOR ;		\
    SE.c_id		   = ( ushort )	c_id_param;			\
    SE.resv		   = ( ushort )	0;				\
									\
    MID_WR_PCB(	&SE , (	sizeof(	MIDDefineCursor	) >> 2 ) );		\
									\
}
#endif


#define	MID_SetActiveCursor( c_id_param	, icx_param , icy_param	)	\
{									\
    MIDSetActiveCursor	SE;						\
									\
    SE.hdr.halfword.length = ( ushort )	sizeof(	MIDSetActiveCursor );	\
    SE.hdr.halfword.op	   = ( ushort )	OP_SET_ACTIVE_CURSOR ;		\
    SE.c_id		   = ( ushort )	c_id_param;			\
    SE.resv		   = ( ushort )	0;				\
    SE.icx		   = ( short )	icx_param;			\
    SE.icy		   = ( short )	icy_param;			\
									\
    MID_WR_PCB(	&SE , (	sizeof(	MIDSetActiveCursor ) >>	2 ) );		\
									\
}

#define	MID_MoveCursor(	cx_param , cy_param )				\
{									\
    MIDMoveCursor	SE;						\
									\
    SE.hdr.halfword.length = ( ushort )	sizeof(	MIDMoveCursor );	\
    SE.hdr.halfword.op	   = ( ushort )	OP_MOVE_CURSOR ;		\
    SE.cx		   = ( short ) cx_param;			\
    SE.cy		   = ( short ) cy_param;			\
									\
    MID_WR_PCB(	&SE , (	sizeof(	MIDMoveCursor )	>> 2 ) );		\
									\
}

#define	MID_HIDE_ACTIVE_CURSOR	( ( ulong ) (0)	)
#define	MID_SHOW_ACTIVE_CURSOR	( ( ulong ) (1)	)

#define	MID_HideShowActiveCursor( oper_param )				\
{									\
    MIDHideShowActiveCursor	SE;					\
									\
    SE.hdr.halfword.length = ( ushort )					\
				sizeof(	MIDHideShowActiveCursor	);	\
    SE.hdr.halfword.op	   = ( ushort )	OP_HIDE_SHOW_ACTIVE_CURSOR ;	\
    SE.oper		   = ( ulong )	oper_param;			\
									\
    MID_WR_PCB(	&SE , (	sizeof(	MIDHideShowActiveCursor	) >> 2 ) );	\
									\
}

#define	MID_BlinkCursorColor( c_id_param , index_param ,		\
				orig_time_param	, alt_time_param ,	\
				alt_ccolor_param )			\
{									\
    MIDBlinkCursorColor		SE;					\
									\
    SE.hdr.halfword.length = ( ushort )	sizeof(	MIDBlinkCursorColor );	\
    SE.hdr.halfword.op	   = ( ushort )	OP_BLINK_CURSOR_COLORS ;	\
    SE.c_id		   = ( ushort )	c_id_param;			\
    SE.resv		   = ( ushort )	0;				\
    SE.index		   = ( ulong )	(index_param - 1);		\
    SE.orig_time	   = ( long )	orig_time_param;		\
    SE.alt_time		   = ( long )	alt_time_param;			\
    SE.alt_ccolor	   = ( ulong )	alt_ccolor_param;		\
									\
    MID_WR_PCB(	&SE , (	sizeof(	MIDBlinkCursorColor ) >> 2 ) );		\
									\
}




	/*----------------------------------------------*
	 * Font	Support	Commands (fixed)		*
	 *----------------------------------------------*/

#define	MID_FontPinnedUnpinned(	corr_param , haddr_param , len_param ,	\
				gfid_pinned_param ,			\
				gfid_unpinned_param )			\
{									\
    MIDFontPinnedUnpinned	SE;					\
									\
    SE.hdr.halfword.length = ( ushort )	sizeof(	MIDFontPinnedUnpinned );\
    SE.hdr.halfword.op	   = ( ushort )	OP_FONT_PINNED ;		\
    SE.corr		   = ( ushort )	corr_param;			\
    SE.resv		   = ( ushort )	0;				\
    SE.haddr		   = ( ulong )	haddr_param;			\
    SE.len		   = ( ulong )	len_param;			\
    SE.gfid_pinned	   = ( ulong )	gfid_pinned_param;		\
    SE.gfid_unpinned	   = ( ulong )	gfid_unpinned_param;		\
									\
    MID_WR_PCB(	&SE , (	sizeof(	MIDFontPinnedUnpinned )	>> 2 ) );	\
									\
}

#define	MID_FontMustBeUnpinned(	gfid_param )				\
{									\
    MIDFontMustBeUnpinned	SE;					\
									\
    SE.hdr.halfword.length = ( ushort )	sizeof(	MIDFontMustBeUnpinned );\
    SE.hdr.halfword.op	   = ( ushort )	OP_FONT_MUST_BE_UNPINNED ;	\
    SE.gfid		   = ( ulong ) gfid_param;			\
									\
    MID_WR_PCB(	&SE , (	sizeof(	MIDFontMustBeUnpinned )	>> 2 ) );	\
									\
}

#define	MID_FontRequestReceived(  )					\
{									\
    MIDFontRequestReceived	SE;					\
									\
    SE.hdr.halfword.length = ( ushort )	sizeof(MIDFontRequestReceived) ;\
    SE.hdr.halfword.op	   = ( ushort )	OP_FONT_REQUEST_RECEIVED ;	\
									\
    MID_WR_PCB(	&SE , (	sizeof(	MIDFontRequestReceived ) >> 2 )	);	\
									\
}




	/*----------------------------------------------*
	 * Color Processing Control Commands (fixed)	*
	 *----------------------------------------------*/

/************************************************************************/
/*									*/
/*  Macro Name:	 MID_LoadFrameBufferColorTable				*/
/*									*/
/*  Function:								*/
/*    Loads Frame Buffer Command Block memory on the adapter and	*/
/*    notifies the adapter that	updates	have been made.			*/
/*									*/
/*  Input:								*/
/*    ct_id_param   -- Which of	the hardware color tables to update.	*/
/*    stindex_param -- The first index in the color table to update.	*/
/*    ctlen_param   -- The number of color table entries to update	*/
/*		       (also the number	of words in "fbcolor_param".	*/
/*    fbcolor_param -- Colormap	array to get new colors	from.		*/
/*									*/
/*  Output:								*/
/*   None								*/
/*									*/
/************************************************************************/

#define	MID_LoadFrameBufferColorTable( ct_id_param , stindex_param ,	\
					ctlen_param , fbcolor_param )	\
{									\
    MIDLoadFrameBufferColorTable	SE;				\
									\
    MID_WR_CTCB_X_ARRAY( ct_id_param,					\
			(MID_CTCB_X_COLOR_TABLE + stindex_param),	\
			&fbcolor_param[stindex_param], ctlen_param )	\
									\
    SE.hdr.halfword.length = ( ushort )					\
				sizeof(	MIDLoadFrameBufferColorTable );	\
    SE.hdr.halfword.op	   = ( ushort )	OP_LOAD_FRAME_BUFFER_COLOR_TABLE ;\
    SE.ct_id		   = ( ushort )	ct_id_param;			\
    SE.resv		   = ( ushort )	0;				\
    SE.stindex		   = ( ulong )	stindex_param;			\
    SE.ctlen		   = ( long )	ctlen_param;			\
									\
    MID_WR_PCB(	&SE , (	sizeof(	MIDLoadFrameBufferColorTable ) >> 2 ) );\
									\
}

#define	MID_BlinkFrameBufferColorTable(	ct_id_param , index_param ,	\
					orig_time_param	,		\
					alt_time_param ,		\
					alt_fbcolor_param )		\
{									\
    MIDBlinkFrameBufferColorTable	SE;				\
									\
    SE.hdr.halfword.length = ( ushort )					\
			sizeof(	MIDBlinkFrameBufferColorTable );	\
    SE.hdr.halfword.op	   = ( ushort )	OP_BLINK_FRAME_BUFFER_COLOR_TABLE ;\
    SE.ct_id		   = ( ushort )	ct_id_param;			\
    SE.resv		   = ( ushort )	0;				\
    SE.index		   = ( ulong )	index_param;			\
    SE.orig_time	   = ( long )	orig_time_param;		\
    SE.alt_time		   = ( long )	alt_time_param;			\
    SE.alt_fbcolor	   = ( ulong )	alt_fbcolor_param;		\
									\
    MID_WR_PCB(	&SE ,							\
		( sizeof( MIDBlinkFrameBufferColorTable	) >> 2 ) );	\
									\
}

#define	MID_LoadOverlayPlanesColorTable( opcolor1_param	,		\
						opcolor2_param ,	\
						opcolor3_param )	\
{									\
    MIDLoadOverlayPlanesColorTable	SE;				\
									\
    SE.hdr.halfword.length = ( ushort )					\
			sizeof(	MIDLoadOverlayPlanesColorTable );	\
    SE.hdr.halfword.op	   = ( ushort )	OP_LOAD_OVERLAY_PLANES_COLOR_TABLE ;\
    SE.opcolor1		   = ( ulong )	opcolor1_param;			\
    SE.opcolor2		   = ( ulong )	opcolor2_param;			\
    SE.opcolor3		   = ( ulong )	opcolor3_param;			\
									\
    MID_WR_PCB(	&SE ,							\
		( sizeof( MIDLoadOverlayPlanesColorTable ) >> 2	) );	\
									\
}

#define	MID_BlinkOverlayPlanesColorTable( index_param ,			\
						orig_time_param	,	\
						alt_time_param ,	\
						alt_opcolor_param )	\
{									\
    MIDBlinkOverlayPlanesColorTable	SE;				\
									\
    SE.hdr.halfword.length = ( ushort )					\
			sizeof(	MIDBlinkOverlayPlanesColorTable	);	\
    SE.hdr.halfword.op	= ( ushort ) OP_BLINK_OVERLAY_PLANES_COLOR_TABLE ;\
    SE.index		   = ( ulong )	(index_param - 1);		\
    SE.orig_time	   = ( long )	orig_time_param;		\
    SE.alt_time		   = ( long )	alt_time_param;			\
    SE.alt_opcolor	   = ( ulong )	alt_opcolor_param;		\
									\
    MID_WR_PCB(	&SE ,							\
	( sizeof( MIDBlinkOverlayPlanesColorTable ) >> 2 ) );		\
									\
}

#define	MID_Multimap(  )						\
{									\
    MIDMultimap	SE;							\
									\
    SE.hdr.halfword.length = ( ushort )	sizeof(	MIDMultimap );		\
    SE.hdr.halfword.op	   = ( ushort )	OP_MULTIMAP ;			\
									\
    MID_WR_PCB(	&SE , (	sizeof(	MIDMultimap ) >> 2 ) );			\
									\
}

#define	MID_Onemap(  )							\
{									\
    MIDOnemap	SE;							\
									\
    SE.hdr.halfword.length = ( ushort )	sizeof(	MIDOnemap );		\
    SE.hdr.halfword.op	   = ( ushort )	OP_ONEMAP;				\
									\
    MID_WR_PCB(	&SE , (	sizeof(	MIDOnemap ) >> 2 ) );			\
									\
}

#define	MID_Setmap( map_param )						\
{									\
    MIDSetmap	SE;							\
									\
    SE.hdr.halfword.length = ( ushort )	sizeof(	MIDSetmap );		\
    SE.hdr.halfword.op	   = ( ushort )	OP_SETMAP ;			\
    SE.resv		   = ( ushort )	0;				\
    SE.map		   = ( ushort )	map_param;			\
									\
    MID_WR_PCB(	&SE , (	sizeof(	MIDSetmap ) >> 2 ) );			\
									\
}



 /* ---------------------------------------------------------------------
   Clear Control Planes macro  (used to write WID planes and other things)

    First, some special defines: 
  ---------------------------------------------------------------------- */

 /* ---------------------------------------------------------------------
     Mask used when only the window ID planes are to be written with the
     ClearControlPlanes macro.  (see ClearControlPlanes:  cp mask)
  ---------------------------------------------------------------------- */
#define CLEAR_CONTROL_PLANES_wid_only_MASK          0xFFFFFF0F

 /* ---------------------------------------------------------------------
     This constant is the amount to shift the window ID in order to have the
      current window be set to this value.  (see ClearControlPlanes: cp value)
  ---------------------------------------------------------------------- */
#define CLEAR_CONTROL_PLANES_wid_SHIFT              0x4

 /* ---------------------------------------------------------------------
     The control planes can be written under WID control; i.e. areas with
     a matching WID value (in the specifed rectangle) would be the only areas
     written .  This is normally not desireable when writing the WID planes
     themselves, so the following flag is used to disable the WID matching.
     (see ClearControlPlanes: wc flag )
  ---------------------------------------------------------------------- */
#define CLEAR_CONTROL_PLANES_WID_COMPARE_off        0x00000000



#ifdef 	MID_DD
/************************************************************************ 

   NOTE:  The macro(s) contained herein invoke the device driver (system)      
          trace function.  This interface requires an additional include    
          file (mid_dd_trace.h).  Since this file is part of a separate     
          component, it is not included here. 

 ************************************************************************/

#define MID_ClearControlPlanesPCB( cpvalue_param,cpmask_param,                \
		    rectangle_param, wcflag_param, wc_id_param)               \
{                                                                             \
	MIDClearControlPlanesPCB SE;                                          \
									      \
	BUGXDEF (trc_data_WID_planes_PCB) ;                                   \
									      \
	SE.hdr.halfword.length = (ushort) sizeof( MIDClearControlPlanesPCB ); \
	SE.hdr.halfword.op    = ( ushort ) OP_CLEAR_CONTROL_PLANES ;          \
	SE.cpvalue     = ( ulong ) cpvalue_param;                             \
	SE.cpmask      = ( ulong ) cpmask_param;                              \
	SE.x     = ( short ) rectangle_param->rulx;                           \
	SE.y     = ( short ) rectangle_param->ruly;                           \
	SE.w     = ( ushort ) rectangle_param->rwth;                          \
	SE.h     = ( ushort ) rectangle_param->rht;                           \
	SE.wcflag     = ( ushort ) wcflag_param;                              \
	SE.wc_id      = ( ushort ) wc_id_param;                               \
	                                                                      \
	MID_DD_TRACE_DATA (WID_planes_PCB,				      \
			   WRITE_WID_PLANES_PCB_SE, 		  	      \
	                   0, &SE,                                            \
	                   sizeof(SE) ) ;			 	      \
	                                                                      \
	MID_WR_PCB( &SE, ( sizeof(SE) >> 2 ) ); 			      \
}


#if  0  /* printf version */ 
	/*----------------------------------------------*
	   version with debug printfs      	 
	 *----------------------------------------------*/

#define MID_ClearControlPlanes( cpvalue_param,cpmask_param,                   \
		    rectangle_param, wcflag_param, wc_id_param)               \
{                                                                             \
	MIDClearControlPlanesPCB SE;                                          \
									      \
	SE.hdr.halfword.length = ( ushort ) sizeof( MIDClearControlPlanesPCB);\
	SE.hdr.halfword.op    = ( ushort ) OP_CLEAR_CONTROL_PLANES ;          \
	SE.cpvalue     = ( ulong ) cpvalue_param;                             \
	SE.cpmask      = ( ulong ) cpmask_param;                              \
	SE.x     = ( short ) rectangle_param->rulx;                           \
	SE.y     = ( short ) rectangle_param->ruly;                           \
	SE.w     = ( ushort ) rectangle_param->rwth;                          \
	SE.h     = ( ushort ) rectangle_param->rht;                           \
	SE.wcflag     = ( ushort ) wcflag_param;                              \
	SE.wc_id      = ( ushort ) wc_id_param;                               \
	                                                                      \
	                                                                      \
	printf("\n------------------------  Clear Control Planes"             \
	       " SE  -------------------- \n");                               \
	printf("leng, opcode:  %4X %4X\n", SE.hdr.halfword.length,            \
                  SE.hdr.halfword.op);   				      \
	printf("value:          %8X\n",SE.cpvalue    );                       \
	printf("mask:           %8X\n",SE.cpmask     );                       \
	printf("(x,y)          %4X %4X\n",SE.x, SE.y);                        \
	printf("width x height %4X %4X\n",SE.w, SE.h);                        \
	printf("flag, WID      %4X %4X\n",SE.wcflag, SE.wc_id);               \
	printf("------------------------  End of Clear Control Planes"        \
	       " SE  -------------------- \n\n");                             \
	                                                                      \
                                                                              \
	MID_WR_PCB( &SE, ( sizeof( MIDClearControlPlanesPCB ) >> 2 ) );       \     
}

#endif  /* printfs  */



#else 
#define	MID_ClearControlPlanes(	cpvalue_param ,	cpmask_param ,		\
				rectangle_param	, wcflag_param ,	\
				wc_id_param )				\
{									\
    MIDClearControlPlanesPCB	SE;					\
									\
    SE.hdr.halfword.length = ( ushort )	sizeof(	MIDClearControlPlanesPCB);\
    SE.hdr.halfword.op	   = ( ushort )	OP_CLEAR_CONTROL_PLANES;	\
    SE.cpvalue		   = ( ulong )	cpvalue_param;			\
    SE.cpmask		   = ( ulong )	cpmask_param;			\
    SE.x		   = ( short )	rectangle_param->rulx;		\
    SE.y		   = ( short )	rectangle_param->ruly;		\
    SE.w		   = ( ushort )	rectangle_param->rwth;		\
    SE.h		   = ( ushort )	rectangle_param->rht;		\
    SE.wcflag		   = ( ushort )	wcflag_param;			\
    SE.wc_id		   = ( ushort )	wc_id_param;			\
									\
									\
    MID_WR_PCB(	&SE, ( sizeof( MIDClearControlPlanesPCB ) >> 2 ) );	\
}

#endif 




	/*----------------------------------------------*
	 * The section below defines a set of macros	*
	 * to build PCB	command	elements which can be	*
	 * variable in length or which might have	*
	 * a format that can vary based	on a value of	*
	 * one of the fields				*
	 *----------------------------------------------*/

	/*----------------------------------------------*
	 * Adapter Control Commands (variable)		*
	 *----------------------------------------------*/

 /* ---------------------------------------------------------------------
   Set Window Parameters PCB Macro             
  ---------------------------------------------------------------------- */

#ifdef	 MID_DD 

/************************************************************************ 

   NOTE:  The macros contained herein invoke the device driver (system)      
          trace function.  This interface requires an additional include    
          file (mid_dd_trace.h).  Since this file is part of a separate     
          component, it is not included here. 

 ************************************************************************/

#define MID_SetWindowParametersPCB( w_id_param, woflags_param,                \
	window_extents_param, num_visual_rects_param, visual_rects_param)     \
{                                                                             \
	struct _SE                                                            \
	{       						 	      \
		MIDSetWindowParametersPCB s;                                  \
		MIDCornerRectangle        vr[MAX_VIS_RECT];  /*var portion */ \
	} SE ;     							      \
	        							      \
	int  num_vis, n ;                                                     \
	        							      \
	BUGXDEF (trc_data_SWP_PCB) ; 	                                      \
									      \
	num_vis = num_visual_rects_param ;                                    \
	if ( num_visual_rects_param > MAX_VIS_RECT )                          \
	{       							      \
	    num_vis = MAX_VIS_RECT ;                                          \
	}       							      \
	        							      \
   SE.s.hdr.halfword.length = (ushort) sizeof( MIDSetWindowParametersPCB)     \
		+ ( num_vis * sizeof(MIDCornerRectangle));       	      \
	        							      \
	SE.s.hdr.halfword.op      = ( ushort ) OP_SET_WINDOW_PARAMETERS_PCB ; \
	SE.s.w_id                 = ( ushort ) w_id_param;                    \
	SE.s.woflags              = ( ushort ) woflags_param;                 \
	SE.s.window_extents.rulx  = ( short ) window_extents_param->ul.x;     \
	SE.s.window_extents.ruly  = ( short ) window_extents_param->ul.y;     \
	SE.s.window_extents.rlrx  = ( short ) window_extents_param->lr.x;     \
	SE.s.window_extents.rlry  = ( short ) window_extents_param->lr.y;     \
	                                                                      \
       /***********************************************************/          \
       /* Now, for every exposure rectangle in the exposure list, */          \
       /* put the rectangle extents into the SEV array            */          \
       /***********************************************************/          \
	for (n=0;     n<num_vis;    n++)                                      \
	{                                                                     \
		SE.vr[n].rulx = visual_rects_param[n].ul.x;                   \
		SE.vr[n].ruly = visual_rects_param[n].ul.y;                   \
		SE.vr[n].rlrx = visual_rects_param[n].lr.x;                   \
		SE.vr[n].rlry = visual_rects_param[n].lr.y;                   \
	                                                                      \
	}                                                                     \
	                                                                      \
	/*------------------------------------------------------------------* \
	   Write the command to the PCB.  Since we have already moved the     \
	   visible rectangles into a single buffer, we just do a single write \
	   to the PCB.  If we didn't move the vis rects, we would probably    \
	   do two writes.                                                     \
	                                                                      \
	   Shift of 2 converts number of bytes to number of words             \
	 *-----------------------------------------------------------------*/ \
	                                                                      \
	MID_DD_TRACE_DATA (SWP_PCB,					      \
			   WRITE_WINDOW_PARMS_PCB_SE,			      \
	                   0, &SE,                                            \
                ((sizeof(SE) + (num_vis * sizeof(MIDCornerRectangle)) ) ) ) ; \
	                                                                      \
	                                                                      \
	MID_WR_PCB (&SE, ((sizeof(MIDSetWindowParametersPCB) +                \
	                  (num_vis * sizeof(MIDCornerRectangle)) ) >> 2));    \
                                                            		      \
}


#if 0   /* old printf version  */                                             

#define MID_SetWindowParametersPCB( w_id_param, woflags_param,                \
	window_extents_param, num_visual_rects_param, visual_rects_param)     \
{                                                                             \
	struct _SE                                                            \
	{       						 	      \
		MIDSetWindowParametersPCB s;                                  \
		MIDCornerRectangle        vr[MAX_VIS_RECT];  /*var portion */ \
	} SE ;     							      \
	        							      \
	int  num_vis, n ;                                                     \
	        							      \
	num_vis = num_visual_rects_param ;                                    \
	if ( num_visual_rects_param > MAX_VIS_RECT )                          \
	{       							      \
	    num_vis = MAX_VIS_RECT ;                                          \
	}       							      \
	        							      \
   SE.s.hdr.halfword.length = (ushort) sizeof( MIDSetWindowParametersPCB)     \
		+ ( num_vis * sizeof(MIDCornerRectangle));       	      \
	        							      \
	SE.s.hdr.halfword.op      = ( ushort ) OP_SET_WINDOW_PARAMETERS_PCB ; \
	SE.s.w_id                 = ( ushort ) w_id_param;                    \
	SE.s.woflags              = ( ushort ) woflags_param;                 \
	SE.s.window_extents.rulx  = ( short ) window_extents_param->ul.x;     \
	SE.s.window_extents.ruly  = ( short ) window_extents_param->ul.y;     \
	SE.s.window_extents.rlrx  = ( short ) window_extents_param->lr.x;     \
	SE.s.window_extents.rlry  = ( short ) window_extents_param->lr.y;     \
	                                                                      \
	printf("\n------------------------  Set Window Parms PCB"             \
	       " SE  -------------------- \n");                               \
	printf("leng, opcode:  %4X %4X\n", SE.s.hdr.halfword.length,          \
                  SE.s.hdr.halfword.op);   				      \
	printf("WID, flags:    %4X %4X\n", SE.s.w_id, SE.s.woflags);          \
	printf("Extents - ul   %4X %4X\n",                                    \
                            SE.s.window_extents.rulx,SE.s.window_extents.ruly)\
	printf("          lr   %4X %4X ________ END of FIXED portion of SE\n",\
                  SE.s.window_extents.rlrx, SE.s.window_extents.rlry);        \
	                                                                      \
       /***********************************************************/          \
       /* Now, for every exposure rectangle in the exposure list, */          \
       /* put the rectangle extents into the SEV array            */          \
       /***********************************************************/          \
	for (n=0;     n<num_vis;    n++)                                      \
	{                                                                     \
		SE.vr[n].rulx = visual_rects_param[n].ul.x;              
		SE.vr[n].ruly = visual_rects_param[n].ul.y;             \
		SE.vr[n].rlrx = visual_rects_param[n].lr.x;             \
		SE.vr[n].rlry = visual_rects_param[n].lr.y;             \
	                                                                      \
	   printf("vis rect %d     %4X %4X\n",n,SE.vr[n].rulx, SE.vr[n].ruly);\
	   printf("               %4X %4X\n", SE.vr[n].rlrx, SE.vr[n].rlry); \
	}                                                                     \
	printf("------------------------  END of Set Window Parms PCB"        \
	       " SE  -------------------- \n\n");                             \
	                                                                      \
	/*------------------------------------------------------------------* \
	   Write the command to the PCB.  Since we have already moved the     \
	   visible rectangles into a single buffer, we just do a single write \
	   to the PCB.  If we didn't move the vis rects, we would probably    \
	   do two writes.                                                     \
	                                                                      \
	   Shift of 2 converts number of bytes to number of words             \
	 *-----------------------------------------------------------------*/ \
	                                                                      \
	MID_WR_PCB (&SE, ((sizeof(MIDSetWindowParametersPCB) +           \
	                  (num_vis * sizeof(MIDCornerRectangle)) ) >> 2));    \
                                                            		      \
}

#endif  /* printfs  */                                             

#else  /* non-MID_DD */

#define	MID_SetWindowParametersPCB( w_id_param , woflags_param ,	\
					window_extents_param ,		\
					num_visual_rects_param ,	\
					visual_rects_param )		\
{									\
    struct _SE								\
    {									\
	MIDSetWindowParametersPCB s;					\
	MIDCornerRectangle	  vr[MAX_VIS_RECT];  /*	var portion */	\
    } SE ;								\
									\
    int	 num_vis, n ;							\
									\
    num_vis = num_visual_rects_param ;					\
    if ( num_visual_rects_param	> MAX_VIS_RECT )			\
    {									\
	num_vis	= MAX_VIS_RECT ;					\
    }									\
									\
    SE.s.hdr.halfword.length = ( ushort	)				\
			( sizeof( MIDSetWindowParametersPCB ) +		\
			(num_vis * sizeof( MIDCornerRectangle )) );	\
									\
    SE.s.hdr.halfword.op      = ( ushort ) OP_SET_WINDOW_PARAMETERS_PCB ; \
    SE.s.w_id		      =	( ushort ) w_id_param;			\
    SE.s.woflags	      =	( ushort ) woflags_param;		\
    SE.s.window_extents.rulx  =	( short	)  window_extents_param->ul.x;	\
    SE.s.window_extents.ruly  =	( short	)  window_extents_param->ul.y;	\
    SE.s.window_extents.rlrx  =	( short	)  window_extents_param->lr.x;	\
    SE.s.window_extents.rlry  =	( short	)  window_extents_param->lr.y;	\
									\
   /******************************************************************/	\
   /* Now, for every exposure rectangle	in the exposure	list, put    */	\
   /* the rectangle extents into the SEV array.			     */	\
   /******************************************************************/	\
    for	(n=0;	  n<num_vis;	n++)					\
    {									\
	SE.vr[n].rulx =	visual_rects_param[n].ul.x;			\
	SE.vr[n].ruly =	visual_rects_param[n].ul.y;			\
	SE.vr[n].rlrx =	visual_rects_param[n].lr.x;			\
	SE.vr[n].rlry =	visual_rects_param[n].lr.y;			\
    }									\
									\
   /******************************************************************/	\
   /* Write the	command	to the PCB.  Since we have already moved     */	\
   /* the visible rectangles into a single buffer, we just do a	     */	\
   /* single write to the PCB.	If we didn't move the vis rects, we  */	\
   /* would probably do	two writes.				     */	\
   /*								     */	\
   /* Shift of 2 converts number of bytes to number of words	     */	\
   /******************************************************************/	\
									\
    MID_WR_PCB (&SE, ((sizeof( MIDSetWindowParametersPCB ) +		\
		(num_vis * sizeof( MIDCornerRectangle ))) >> 2)	);	\
									\
}

#endif   /* non-MID_DD */




 /* ---------------------------------------------------------------------
   Associate One Window with Color Palette macro

   NOTE:  There are two macros: Assoc ONE Window with Color Palette and
          Associate Window with Color Palette (which maps multiple windows
          to their respective color palettes).   The first macro is simply      
          a special case of the more general. 
  ---------------------------------------------------------------------- */

#ifdef	MID_DD
/************************************************************************ 

   NOTE:  The macros contained herein invoke the device driver (system)      
          trace function.  This interface requires an additional include    
          file (mid_dd_trace.h).  Since this file is part of a separate     
          component, it is not included here. 

 ************************************************************************/

#define MID_AssocOneWindowWithColorPalette( w_id_param, cp_id_param )         \
{                                                                             \
	MIDAssocOneWindowWithColorPalette SE;                                 \
									      \
	BUGXDEF (trc_data_Associate_Color) ;	                              \
									      \
	                                                                      \
  SE.hdr.halfword.length = (ushort) sizeof(MIDAssocOneWindowWithColorPalette);\
  SE.hdr.halfword.op = (ushort) OP_ASSOC_WINDOW_WITH_COLOR_PALETTE ;          \
	SE.w_id                = (ushort) w_id_param;                         \
	SE.cp_id               = (ushort) cp_id_param;                        \
	                                                                      \
	MID_DD_TRACE_DATA (Associate_Color,				      \
			   ASSOCIATE_COLOR_SE,	 			      \
	                   0, &SE, sizeof(SE) ) ;	 		      \
	                                                                      \
	MID_WR_PCB(&SE,(sizeof(MIDAssocOneWindowWithColorPalette) >> 2 )) ;   \
}


#if 0   /* printf version  */
#define MID_AssocOneWindowWithColorPalette( w_id_param, cp_id_param )         \
{                                                                             \
	MIDAssocOneWindowWithColorPalette SE;                                 \
									      \
  SE.hdr.halfword.length = (ushort) sizeof(MIDAssocOneWindowWithColorPalette);\
  SE.hdr.halfword.op = (ushort) OP_ASSOC_WINDOW_WITH_COLOR_PALETTE ;          \
	SE.w_id                = (ushort) w_id_param;                         \
	SE.cp_id               = (ushort) cp_id_param;                        \
                                                                              \
	printf("\nMID_AssocOneWindowWithColorPalette:\n");                    \
	printf("  %4X %4X\n",SE.hdr.halfword.length,SE.hdr.halfword.op);      \
	printf("  %4X %4X\n",SE.w_id, SE.cp_id);                              \
	printf("MID_AssocOneWindowWithColorPalette:END of SE\n");             \
                                                                              \
	MID_WR_PCB(&SE,(sizeof(MIDAssocOneWindowWithColorPalette)>>2 )); \
}
#endif



#else 	/* non MID_DD */

#define	MID_AssocOneWindowWithColorPalette( w_id_param , cp_id_param )	\
{									\
	MIDAssocOneWindowWithColorPalette SE;				\
									\
  SE.hdr.halfword.length = ( ushort )					\
			sizeof(	MIDAssocOneWindowWithColorPalette );	\
					/* no 'One' here */		\
  	SE.hdr.halfword.op = (ushort) OP_ASSOC_WINDOW_WITH_COLOR_PALETTE ;\
	SE.w_id		 = ( ushort ) w_id_param;			\
	SE.cp_id	 = ( ushort ) cp_id_param;			\
									\
	MID_WR_PCB( &SE,						\
		(sizeof( MIDAssocOneWindowWithColorPalette ) >>	2 ) );	\
									\
}

#endif	/* MID_DD */


/************************************************************************/
/*									*/
/*  Macro Name:	 MID_AssocWindowWithColorPalette			*/
/*									*/
/*  Function:								*/
/*    Used to specify which of the 5 color palettes to use for each	*/
/*    hardware window.							*/
/*									*/
/*  Input:								*/
/*    id_pair_param -- array of	struct _MID_id_pair (defined in		*/
/*		       "hw_typdefs.h").					*/
/*									*/
/*    count_param -- number of "id_pair"s in array.			*/
/*									*/
/*  Output:								*/
/*    None								*/
/*									*/
/*  NOTE:  no special device driver version with traces, because we     */
/*          don't use it.     						*/
/*									*/
/************************************************************************/

#define	MID_AssocWindowWithColorPalette( id_pair_param , count_param )	\
{									\
    MIDAssocWindowWithColorPalette	SE;				\
									\
    SE.hdr.halfword.length = ( ushort )					\
			( sizeof( MIDAssocWindowWithColorPalette ) +	\
						( count_param << 2 ) );	\
    SE.hdr.halfword.op = (ushort) OP_ASSOC_WINDOW_WITH_COLOR_PALETTE ;  \
									\
    MID_WR_PCB_2_PARTS(	&SE ,						\
		( sizeof( MIDAssocWindowWithColorPalette ) >> 2	) ,	\
			id_pair_param ,	count_param );			\
}


	/*----------------------------------------------*
	 * Color Processing Control Commands (variable)	*
	 *----------------------------------------------*/

/************************************************************************/
/*									*/
/*  Macro Name:	 MID_CycleColorMaps					*/
/*									*/
/*  Function:								*/
/*    Cycles through the user defined array of colormaps and		*/
/*    times until the user re-issues the command with 0	or 1		*/
/*    "map_duration_pair"s specified.					*/
/*									*/
/*  Input:								*/
/*    map_duration_pair_param -- array of struct _MID_map_duration_pair	*/
/*				 (defined in "hw_typdefs.h").		*/
/*									*/
/*    count_param -- number of "map_duration_pair"s in array.		*/
/*									*/
/*  Output:								*/
/*    None								*/
/*									*/
/************************************************************************/

#define	MID_CycleColorMaps( map_duration_pair_param , count_param )	\
{									\
    MIDCycleColorMaps	SE;						\
									\
    SE.hdr.halfword.length = ( ushort )					\
		( sizeof( MIDCycleColorMaps ) +	( count_param << 2 ) );	\
    SE.hdr.halfword.op	   = ( ushort )	OP_CYCLE_COLOR_MAPS ;		\
									\
    MID_WR_PCB_2_PARTS(	&SE , (	sizeof(	MIDCycleColorMaps ) >> 2 ) ,	\
			map_duration_pair_param	, count_param );	\
									\
}


#endif /* _H_MID_HW_PCB_RMS */


