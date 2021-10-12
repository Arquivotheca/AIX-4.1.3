/* @(#)10       1.12.1.6  src/bos/kernext/disp/ped/pedmacro/hw_FIFrms.h, pedmacro, bos411, 9428A410j 3/23/94 17:22:15 */

/*
 * COMPONENT_NAME: PEDMACRO
 *
 * FUNCTIONS:
 *
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991,1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *
 */



/************************************************************************/
/*                                                                      */
/*      PEDERNALES HW MACRO PROGRAMMING INTERFACE                       */
/*                                                                      */
/************************************************************************/


/************************************************************************/
/************************************************************************/
/* NOTE:  Right and left shifting by 2 designates a division by 4, not  */
/*        a shifting for masking purposes, unless otherwise specified.  */
/************************************************************************/
/************************************************************************/


#ifndef _H_MID_HW_FIFO_RMS
#define _H_MID_HW_FIFO_RMS

#include <sys/errno.h>
#include <sys/rcm_win.h>
#include <mid/hw_se_types.h>
#include <mid/hw_seops.h>
#include <mid/hw_rmscon.h>
#include <mid/hw_macros.h>

#ifdef  MID_DD
#include <sys/sleep.h>
#endif

/************************************************************************/
/* GAIRMS FIFO COMMAND ELEMENT TYPEDEFS                                 */
/*                                                                      */
/*      The names of the members of the structures described below      */
/*      are derived from the Pedernales Software Interface              */
/*      Specification v. 0.9                                            */
/*                                                                      */
/************************************************************************/

	/*----------------------------------------------*
	 * Check if the header has already been defined *
	 *----------------------------------------------*/

#ifndef _H_MID_SE_HDR
#define _H_MID_SE_HDR

	/*----------------------------------------------*
	 * Definition for the header information of     *
	 * each structure element                       *
	 *----------------------------------------------*/

typedef union   _MIDfifohdr
{
	ulong    word;
	struct  _FIFhalfword
	{
	        ushort  length;         /* length of SE in bytes */
	        ushort  op;             /* opcode for SE */
	} halfword;
} MIDfifohdr;

#endif  /* _H_MID_SE_HDR */


	/*----------------------------------------------*
	 * The section below defines a FIFO command     *
	 * element typedef for the set of FIFO commands *
	 * which are of a fixed length and whose fields *
	 * are all regularly defined                    *
	 *----------------------------------------------*/

	/*----------------------------------------------*
	 * Font Support Commands (fixed)                *
	 *----------------------------------------------*/

/*
 * PEDERNALES SetActiveFont command packet structure
 */
typedef struct _MIDSetActiveFont
{
	MIDfifohdr              hdr;    /* SE header info (len/opcode) */
	ulong                   bgfid;
	ulong                   agfid;
}       MIDSetActiveFont,
	*pMIDSetActiveFont;

/*
 * PEDERNALES RemoveActiveFont command packet structure
 */
typedef struct _MIDRemoveActiveFont
{
	MIDfifohdr              hdr;    /* SE header info (len/opcode) */
	ulong                   gfid;
}       MIDRemoveActiveFont,
	*pMIDRemoveActiveFont;


	/*----------------------------------------------*
	 * Bit Plane Control Commands (fixed)           *
	 *----------------------------------------------*/

/*
 * PEDERNALES SelectDrawingBitPlanes command packet structure
 */
typedef struct _MIDSelectDrawingBitPlanes
{
	MIDfifohdr              hdr;    /* SE header info (len/opcode) */
	ulong                   dbpset;
}       MIDSelectDrawingBitPlanes,
	*pMIDSelectDrawingBitPlanes;

/*
 * PEDERNALES SetBitPlanesWriteMasks command packet structure
 */
typedef struct _MIDSetBitPlanesWriteMasks
{
	MIDfifohdr              hdr;    /* SE header info (len/opcode) */
	ulong                   uflags;
	ulong                   fbwmask;
	ulong                   wogwmask;
}       MIDSetBitPlanesWriteMasks,
	*pMIDSetBitPlanesWriteMasks;

/*
 * PEDERNALES SetBitPlanesDisplayMasks command packet structure
 */
typedef struct _MIDSetBitPlanesDisplayMasks
{
	MIDfifohdr              hdr;    /* SE header info (len/opcode) */
	ulong                   uflags;
	ulong                   fbdmask;
	ulong                   odmask;
}       MIDSetBitPlanesDisplayMasks,
	*pMIDSetBitPlanesDisplayMasks;

/*
 * PEDERNALES FrameBufferControl command packet structure
 */
typedef struct _MIDFrameBufferControl
{
	MIDfifohdr              hdr;    /* SE header info (len/opcode) */
	ushort                  corr , uflags;
	ulong                   fbcflags;
}       MIDFrameBufferControl,
	*pMIDFrameBufferControl;

/*
 * PEDERNALES SetFrameBufferSwapInterval command packet structure
 */
typedef struct _MIDSetFrameBufferSwapInterval
{
	MIDfifohdr              hdr;    /* SE header info (len/opcode) */
	ulong                   fbsintvl;
}       MIDSetFrameBufferSwapInterval,
	*pMIDSetFrameBufferSwapInterval;

/*
 * PEDERNALES ZBufferControl command packet structure
 */
typedef struct _MIDZBufferControl
{
	MIDfifohdr              hdr;    /* SE header info (len/opcode) */
	ulong                   zftcond;
}       MIDZBufferControl,
	*pMIDZBufferControl;

/*
 * PEDERNALES ClearBitPlanes command packet structure
 */
typedef struct _MIDClearBitPlanes
{
	MIDfifohdr              hdr;    /* SE header info (len/opcode) */
	ulong                   fflags;
	short                   x , y;
	ushort                  w , h;
	ulong                   fbfill;
	ulong                   ogfill;
	ulong                   zbfill;
}       MIDClearBitPlanes,
	*pMIDClearBitPlanes;

/*
 * PEDERNALES ClearControlPlanes command packet structure
 */
typedef struct _MIDClearControlPlanesFIF
{
	MIDfifohdr              hdr;    /* SE header info (len/opcode) */
	ushort                  correlator, reserved; /* correlator    */
	ulong                   cpvalue;
	ulong                   cpmask;
	short                   x , y;
	ushort                  w , h;
	ushort                  wcflag , wc_id;
}       MIDClearControlPlanesFIF, *pMIDClearControlPlanesFIF;


/*
 * PEDERNALES SetGPMCompareValue command packet structure
 */
typedef struct _MIDSetGPMCompareValue
{
	MIDfifohdr              hdr;    /* SE header info (len/opcode) */
	ulong                   efcvalue;
}       MIDSetGPMCompareValue,
	*pMIDSetGPMCompareValue;


/*
 * PEDERNALES Set M1 Client clip Region command packet structure
 */
typedef struct _MIDSetM1ClientClipRegion
{
	MIDfifohdr              hdr;    /* SE header info (len/opcode) */
	ulong                   ccflags ;
	gBox                    rect ;  /* specified as ll (x,y) then ur (x,y) */

}       MIDSetM1ClientClipRegion,
	*pMIDSetM1ClientClipRegion;

#define MID_CLIENT_CLIP_REGION_ENABLE   0x00008000
#define MID_CLIENT_CLIP_REGION_DISABLE  0

/*
 * PEDERNALES SetLogicalOperation command packet structure
 */
typedef struct _MIDSetLogicalOperation
{
	MIDfifohdr              hdr;    /* SE header info (len/opcode) */
	ulong                   uflags;
	ushort                  bg_op , fg_op;
}       MIDSetLogicalOperation,
	*pMIDSetLogicalOperation;


	/*----------------------------------------------*
	 * Blit Support Commands (fixed)                *
	 *----------------------------------------------*/

/*
 * PEDERNALES FIFPixelBlit command packet structure
 */
typedef struct _MIDFIFPixelBlit
{
	MIDfifohdr              hdr;    /* SE header info (len/opcode) */
	ushort                  resv;
	ushort                  bflags;
	ulong                   sourcebp;
	short                   sx;
	short                   sy;
	ushort                  sw;
	ushort                  sh;
	ulong                   destbp;
	short                   dx;
	short                   dy;
	ushort                  sbdir;
	ushort                  bdir;
	ulong                   pb_color;
	ushort                  resv1;
	ushort                  pb_op;
}       MIDFIFPixelBlit,
	*pMIDFIFPixelBlit;

/*
 * PEDERNALES FIFZoomScreen command packet structure
 */
typedef struct _MIDFIFZoomScreen
{
	MIDfifohdr              hdr;    /* SE header info (len/opcode) */
	ushort                  resv, bflags;
	ushort                  xrepl, yrepl;
	ulong                   source;
	short                   sx, sy;
	ushort                  sw, sh;
	ulong                   destbp;
	short                   dx, dy;
}       MIDFIFZoomScreen,
	*pMIDFIFZoomScreen;


	/*----------------------------------------------*
	 * Color Processing Control Commands (fixed)    *
	 *----------------------------------------------*/

/*
 * PEDERNALES TransparencyAndColorCompare command packet structure
 */
typedef struct _MIDTransparencyAndColorCompare
{
	MIDfifohdr              hdr;    /* SE header info (len/opcode) */
	ulong                   flags;
	ulong                   t_cond_value;
	ulong                   cc_cond_value;
}       MIDTransparencyAndColorCompare,
	*pMIDTransparencyAndColorCompare;


	/*----------------------------------------------*
	 * Miscellaneous Commands (fixed)               *
	 *----------------------------------------------*/

/*
 * PEDERNALES SynchronizeOnVerticalRetrace command packet structure
 */
typedef struct _MIDSynchronizeOnVerticalRetrace
{
	MIDfifohdr              hdr;    /* SE header info (len/opcode) */
	ushort                  resv, vrcnt;
}       MIDSynchronizeOnVerticalRetrace,
	*pMIDSynchronizeOnVerticalRetrace;

/*
 * PEDERNALES ResetLinePattern command packet structure
 */
typedef struct _MIDResetLinePattern
{
	MIDfifohdr              hdr;    /* SE header info (len/opcode) */
}       MIDResetLinePattern,
	*pMIDResetLinePattern;

/*
 * PEDERNALES Synchronization command packet structure
 */
typedef struct _MIDSynchronization
{
	MIDfifohdr              hdr;    /* SE header info (len/opcode) */
	ushort                  scorr , resv;
}       MIDSynchronization,
	*pMIDSynchronization;


/*
 * PEDERNALES Associate One Window with Color Palette command packet structure
 */
typedef struct _MIDAssocOneWindowWithColorPaletteFIF
{
	MIDfifohdr      hdr;            /* SE header info (len/opcode) */
	ushort          w_id;           /* Window ID */
	ushort          cp_id;          /* Color Palette ID to assoc w/ WID */
} MIDAssocOneWindowWithColorPaletteFIF,
	        *pMIDAssocOneWindowWithColorPaletteFIF ;


	/*----------------------------------------------*
	 * The section below defines a FIFO command     *
	 * element typedef for the set of FIFO commands *
	 * which are variable in length or which have   *
	 * a format that can vary based on a value of   *
	 * one of the fields                            *
	 *----------------------------------------------*/

	/*----------------------------------------------*
	 * Adapter Control Commands (variable)          *
	 *----------------------------------------------*/

/*****************************************************************************
      Associate Multiple Windows with (multiple) Color Palettes

   NOTE 1:  This is only the fixed portion of the command element.
	    A variable number of WID / color palettes pairs may be specified
	    (following the header).
	
 *****************************************************************************/
typedef struct _MIDAssocWindowWithColorPaletteFIF
{
	MIDfifohdr      hdr;            /* SE header info (len/opcode) */
} MIDAssocWindowWithColorPaletteFIF,
  *pMIDAssocWindowWithColorPaletteFIF ;



/*
 * PEDERNALES SetWindowParametersFIF command packet structure
 */

/************************************************************************/
/* NOTE 1:  This is only the fixed portion of the command element.      */
/*          A variable number of CornerRectangles, which defines the    */
/*          visible area, follows this, the fixed portion of the        */
/*          command.                                                    */
/*                                                                      */
/* NOTE 2:  Uses include file "hw_se_types.h (to define the rectangle   */
/*          structs).                                                   */
/************************************************************************/
typedef struct _MIDSetWindowParametersFIF
{
	MIDfifohdr              hdr;    /* SE header info (len/opcode) */
	ushort                  correlator , reserved;
	ushort                  w_id , woflags;
	MIDCornerRectangle      window_extents;
}       MIDSetWindowParametersFIF,
	*pMIDSetWindowParametersFIF;


	/*----------------------------------------------*
	 * Blit Support Commands (variable)             *
	 *----------------------------------------------*/

/*
 * PEDERNALES FIFWritePixelData command packet structure
 */
typedef struct _MIDFIFWritePixelData
{
	MIDfifohdr              hdr;    /* SE header info (len/opcode) */
	ushort                  corr;
	ushort                  bflags;
	ulong                   haddr;
	ushort                  resv;
	ushort                  hstride;
	ulong                   destbp;
	short                   dx;
	short                   dy;
	ushort                  dw;
	ushort                  dh;
	float                   mcx;
	float                   mcy;
	float                   mcz;
	ulong                   fbit;
	ushort                  xrepl;
	ushort                  yrepl;
	ulong                   fgcolor;
	ulong                   bgcolor;
	ushort                  bg_op;
	ushort                  fg_op;
}       MIDFIFWritePixelData,
	*pMIDFIFWritePixelData;


/************************************************************************/
/*                                                                      */
/* MISCELLANEOUS MACROS                                                 */
/*                                                                      */
/************************************************************************/

/************************************************************************/
/*                                                                      */
/*  Macro Name:  MID_TRANSLATE_LOG_OP                                   */
/*                                                                      */
/*  Function:                                                           */
/*    Translates an X, graPHIGS, or GL logical operation to a           */
/*    Pedernales logical operation.                                     */
/*                                                                      */
/*  Input:                                                              */
/*    input_logical_op (int)  - X, graPHIGS, or GL logical operation    */
/*                                                                      */
/*  Output:                                                             */
/*    mid_logical_op (unsigned short)  - Pedernales logical operation   */
/*                                                                      */
/************************************************************************/

static ushort   logical_op_table[22] = {19, 27, 18, 26, 23, 31, 22, 30,
	                                17, 25, 16, 24, 21, 29, 20, 28,
	                                 1,  0,  5,  3,  4,  2};

#define MID_TRANSLATE_LOG_OP( input_logical_op, mid_logical_op )        \
{                                                                       \
    mid_logical_op = logical_op_table[input_logical_op];                \
	                                                                \
}                       /* End MID_TRANSLATE_LOG_OP                  */


/************************************************************************/
/* MACROS TO BUILD GAI RMS STRUCTURE ELEMENTS                           */
/*                                                                      */
/*      The names of the members of the structures described below      */
/*      are derived from the Pedernales Software Interface              */
/*      Specification v. 0.9                                            */
/*                                                                      */
/************************************************************************/

	/*----------------------------------------------*
	 * The section below defines a set of macros    *
	 * to build FIFO command elements               *
	 * which are of a fixed length and whose fields *
	 * are all regularly defined                    *
	 *----------------------------------------------*/

	/*----------------------------------------------*
	 * Font Support Commands (fixed)                *
	 *----------------------------------------------*/

#define MID_SetActiveFont( bgfid_param , agfid_param )                  \
{                                                                       \
    MIDSetActiveFont    SE;                                             \
	                                                                \
    SE.hdr.halfword.length = ( ushort ) sizeof( MIDSetActiveFont );     \
    SE.hdr.halfword.op     = ( ushort ) OP_SET_ACTIVE_FONT;             \
    SE.bgfid               = ( ulong )  bgfid_param;                    \
    SE.agfid               = ( ulong )  agfid_param;                    \
	                                                                \
    MID_WR_DEFERBUF( &SE , ( sizeof( MIDSetActiveFont ) >> 2 ) );       \
	                                                                \
}

#define MID_RemoveActiveFont( gfid_param )                              \
{                                                                       \
    MIDRemoveActiveFont SE;                                             \
	                                                                \
    SE.hdr.halfword.length = ( ushort ) sizeof( MIDRemoveActiveFont );  \
    SE.hdr.halfword.op     = ( ushort ) OP_REMOVE_ACTIVE_FONT;          \
    SE.gfid                = ( ulong )  gfid_param;                     \
	                                                                \
    MID_WRITE_TO_FIFO( &SE ,( sizeof( MIDRemoveActiveFont ) >> 2) );    \
	                                                                \
}


	/*----------------------------------------------*
	 * Bit Plane Control Commands (fixed)           *
	 *----------------------------------------------*/

#define MID_SelectDrawingBitPlanes( dbpset_param )                      \
{                                                                       \
    MIDSelectDrawingBitPlanes   SE;                                     \
	                                                                \
    SE.hdr.halfword.length = ( ushort )                                 \
	                        sizeof( MIDSelectDrawingBitPlanes );    \
    SE.hdr.halfword.op     = ( ushort ) OP_SELECT_DRAWING_BIT_PLANES;   \
    SE.dbpset              = ( ulong )  dbpset_param;                   \
	                                                                \
    MID_WR_DEFERBUF( &SE ,                                              \
	                ( sizeof( MIDSelectDrawingBitPlanes ) >> 2 ) ); \
	                                                                \
}

#define MID_SetBitPlanesWriteMasks( uflags_param , fbwmask_param ,      \
	                                wogwmask_param )                \
{                                                                       \
    MIDSetBitPlanesWriteMasks   SE;                                     \
	                                                                \
    SE.hdr.halfword.length = ( ushort )                                 \
	                        sizeof( MIDSetBitPlanesWriteMasks );    \
    SE.hdr.halfword.op     = ( ushort ) OP_SET_BIT_PLANES_WRITE_MASKS;  \
    SE.uflags              = ( ulong )  uflags_param;                   \
    SE.fbwmask             = ( ulong )  fbwmask_param;                  \
    SE.wogwmask            = ( ulong )  wogwmask_param;                 \
	                                                                \
    MID_WR_DEFERBUF( &SE ,                                              \
	                ( sizeof( MIDSetBitPlanesWriteMasks ) >> 2 ) ); \
	                                                                \
}

#define MID_SetBitPlanesDisplayMasks( uflags_param , fbdmask_param ,    \
	                                                odmask_param )  \
{                                                                       \
    MIDSetBitPlanesDisplayMasks SE;                                     \
	                                                                \
    SE.hdr.halfword.length = ( ushort )                                 \
	                        sizeof( MIDSetBitPlanesDisplayMasks );  \
    SE.hdr.halfword.op     = ( ushort ) OP_SET_BIT_PLANES_DISPLAY_MASKS;\
    SE.uflags              = ( ulong )  uflags_param;                   \
    SE.fbdmask             = ( ulong )  fbdmask_param;                  \
    SE.odmask              = ( ulong )  odmask_param;                   \
	                                                                \
    MID_WR_DEFERBUF( &SE ,                                              \
	        ( sizeof( MIDSetBitPlanesDisplayMasks ) >> 2 ) );       \
	                                                                \
}

#define MID_FrameBufferControl( corr_param , uflags_param ,             \
	                                        fbcflags_param  )       \
{                                                                       \
    MIDFrameBufferControl       SE;                                     \
	                                                                \
    SE.hdr.halfword.length = ( ushort ) sizeof( MIDFrameBufferControl );\
    SE.hdr.halfword.op     = ( ushort ) OP_FRAME_BUFFER_CONTROL;        \
    SE.corr                = ( ushort ) corr_param;                     \
    SE.uflags              = ( ushort ) uflags_param;                   \
    SE.fbcflags            = ( ulong )  fbcflags_param;                 \
	                                                                \
    MID_WRITE_TO_FIFO( &SE , ( sizeof( MIDFrameBufferControl ) >> 2 ) );\
	                                                                \
}

#define MID_SetFrameBufferSwapInterval( fbsintvl_param )                \
{                                                                       \
    MIDSetFrameBufferSwapInterval       SE;                             \
	                                                                \
    SE.hdr.halfword.length = ( ushort )                                 \
	                sizeof( MIDSetFrameBufferSwapInterval );        \
    SE.hdr.halfword.op     = ( ushort ) OP_SET_FRAME_BUFFER_SWAP_INTERVAL;\
    SE.fbsintvl            = ( ulong )  fbsintvl_param;                 \
	                                                                \
    MID_WR_DEFERBUF( &SE ,                                              \
	        ( sizeof( MIDSetFrameBufferSwapInterval ) >> 2 ) );     \
	                                                                \
}

#define MID_ZBufferControl( zftcond_param )                             \
{                                                                       \
    MIDZBufferControl   SE;                                             \
	                                                                \
    SE.hdr.halfword.length = ( ushort ) sizeof( MIDZBufferControl );    \
    SE.hdr.halfword.op     = ( ushort ) OP_Z_BUFFER_CONTROL;            \
    SE.zftcond             = ( ulong )  zftcond_param;                  \
	                                                                \
    MID_WR_DEFERBUF( &SE , ( sizeof( MIDZBufferControl ) >> 2 ) );      \
	                                                                \
}

#define MID_ZBufferControl_NoDefer( zftcond_param )                     \
{                                                                       \
    MIDZBufferControl   SE;                                             \
	                                                                \
    SE.hdr.halfword.length = ( ushort ) sizeof( MIDZBufferControl );    \
    SE.hdr.halfword.op     = ( ushort ) OP_Z_BUFFER_CONTROL;            \
    SE.zftcond             = ( ulong )  zftcond_param;                  \
	                                                                \
    MID_WR_FIFO( &SE , ( sizeof( MIDZBufferControl ) >> 2 ) );          \
	                                                                \
}

#define MID_ClearBitPlanes( fflags_param , x_param , y_param ,          \
	                        w_param , h_param , fbfill_param ,      \
	                        ogfill_param , zbfill_param )           \
{                                                                       \
    MIDClearBitPlanes   SE;                                             \
	                                                                \
    SE.hdr.halfword.length = ( ushort ) sizeof( MIDClearBitPlanes );    \
    SE.hdr.halfword.op     = ( ushort ) OP_CLEAR_BIT_PLANES ;           \
    SE.fflags              = ( ulong )  fflags_param;                   \
    SE.x                   = ( short )  x_param;                        \
    SE.y                   = ( short )  y_param;                        \
    SE.w                   = ( ushort ) w_param;                        \
    SE.h                   = ( ushort ) h_param;                        \
    SE.fbfill              = ( ulong )  fbfill_param;                   \
    SE.ogfill              = ( ulong )  ogfill_param;                   \
    SE.zbfill              = ( ulong )  zbfill_param;                   \
	                                                                \
    MID_WRITE_TO_FIFO( &SE , ( sizeof( MIDClearBitPlanes ) >> 2 ) );    \
	                                                                \
}



 /* ---------------------------------------------------------------------
     Mask used when only the window ID planes are to be written with the
     ClearControlPlanes macro.
 * (see ClearControlPlanes:  cp mask)
  ---------------------------------------------------------------------- */
#define CLEAR_CONTROL_PLANES_wid_only_MASK          0xFFFFFF0F

 /* ---------------------------------------------------------------------
     This constant is the amount to shift the window ID by in order to have
     the current window be set to this value.
 * (see ClearControlPlanes: cp value)
  ---------------------------------------------------------------------- */
#define CLEAR_CONTROL_PLANES_wid_SHIFT              0x4

 /* ---------------------------------------------------------------------
     The control planes can be written under WID control; i.e. areas with
     a matching WID value (in the specifed rectangle) would be the only areas
     written .  This is normally not desireable when writing the WID planes
     themselves, so the following flag is used to disable the WID matching.
 * (see ClearControlPlanes: wc flag )
  ---------------------------------------------------------------------- */
#define CLEAR_CONTROL_PLANES_WID_COMPARE_off        0x00000000

#ifndef MID_DD

#define MID_ClearControlPlanesFIF( cpvalue_param, cpmask_param ,        \
	                        rectangle_param, wcflag_param ,         \
	                        wc_id_param, correlator_param )         \
{                                                                       \
    MIDClearControlPlanesFIF    SE;                                     \
	                                                                \
    SE.hdr.halfword.length = ( ushort ) sizeof( MIDClearControlPlanesFIF );\
    SE.hdr.halfword.op     = ( ushort ) OP_CLEAR_CONTROL_PLANES_FIFO;   \
    SE.correlator          = ( ushort ) correlator_param;               \
    SE.reserved            = ( ushort ) 0x0000;                         \
    SE.cpvalue             = ( ulong )  cpvalue_param;                  \
    SE.cpmask              = ( ulong )  cpmask_param;                   \
    SE.x                   = ( short )  (rectangle_param)->x1;          \
    SE.y                   = ( short )  (rectangle_param)->y1;          \
    SE.w                   = ( ushort ) (rectangle_param)->x2 -         \
	                                (rectangle_param)->x1;          \
    SE.h                   = ( ushort ) (rectangle_param)->y2 -         \
	                                (rectangle_param)->y1;          \
    SE.wcflag              = ( ushort ) wcflag_param;                   \
    SE.wc_id               = ( ushort ) wc_id_param;                    \
	                                                                \
    MID_WRITE_TO_FIFO( &SE, ( sizeof( SE ) >> 2 ) );                    \
}

#else  /* MID_DD */

#define MID_ClearControlPlanesFIF(cpvalue_param,cpmask_param,                 \
	            rectangle_param, wcflag_param, wc_id_param,               \
	            correlator_param)                                         \
{                                                                             \
	MIDClearControlPlanesFIF SE;                                          \
	                                                                      \
	SE.hdr.halfword.length = ( ushort ) sizeof( MIDClearControlPlanesFIF);\
	SE.hdr.halfword.op    = ( ushort ) OP_CLEAR_CONTROL_PLANES_FIFO ;     \
	SE.correlator  = (ushort) correlator_param;                           \
	SE.reserved    = (ushort) 0x0000;                                     \
	SE.cpvalue     = ( ulong ) cpvalue_param;                             \
	SE.cpmask      = ( ulong ) cpmask_param;                              \
	SE.x     = ( short ) rectangle_param->rulx;                           \
	SE.y     = ( short ) rectangle_param->ruly;                           \
	SE.w     = ( ushort ) rectangle_param->rwth;                          \
	SE.h     = ( ushort ) rectangle_param->rht;                           \
	SE.wcflag     = ( ushort ) wcflag_param;                              \
	SE.wc_id      = ( ushort ) wc_id_param;                               \
	                                                                      \
	MID_DD_TRACE_DATA (WID_planes_FIFO,                            \
	                   WRITE_WID_PLANES_FIFO_SE,                          \
	                   0, &SE,                                            \
	                   sizeof(SE) ) ;                                     \
	                                                                      \
	MID_WR_FIFO( &SE, ( sizeof(SE) >> 2 ) );                       \
	/*------------------------------------------------------------------* \
	   Shift of 2 converts number of bytes to number of words             \
	 *-----------------------------------------------------------------*/ \
	                                                                      \
}

#endif /* MID_DD */



#define MID_SetGPMCompareValue( efcvalue_param )                        \
{                                                                       \
    MIDSetGPMCompareValue       SE;                                     \
	                                                                \
    SE.hdr.halfword.length = ( ushort ) sizeof( MIDSetGPMCompareValue );\
    SE.hdr.halfword.op     = ( ushort ) OP_SET_GPM_COMPARE ;            \
    SE.efcvalue            = ( ulong )  efcvalue_param;                 \
	                                                                \
    MID_WR_DEFERBUF( &SE , ( sizeof( MIDSetGPMCompareValue ) >> 2 ) );  \
	                                                                \
}


#define MID_SetM1ClientClipRegion(box, ccflags_value)                   \
{                                                                       \
    MIDSetM1ClientClipRegion    SE;                                     \
	                                                                \
    SE.hdr.halfword.length = (ushort) sizeof(MIDSetM1ClientClipRegion); \
    SE.hdr.halfword.op     = ( ushort ) OP_SET_M1_CLIENT_CLIP_REGION ;  \
    SE.ccflags             = ( ulong )  ccflags_value;                  \
    SE.rect                = box ;                                      \
	                                                                \
    MID_WR_DEFERBUF( &SE , (sizeof(MIDSetM1ClientClipRegion) >> 2 ));   \
	                                                                \
}

#define MID_SetLogicalOperation( uflags_param , bg_op_param ,           \
	                                fg_op_param )                   \
{                                                                       \
    MIDSetLogicalOperation      SE;                                     \
	                                                                \
    SE.hdr.halfword.length = ( ushort )                                 \
	                        sizeof( MIDSetLogicalOperation );       \
    SE.hdr.halfword.op     = ( ushort ) OP_SET_LOGICAL_OPERATION ;      \
    SE.uflags              = ( ulong )  uflags_param;                   \
	                                                                \
    MID_TRANSLATE_LOG_OP(bg_op_param, SE.bg_op);                        \
    MID_TRANSLATE_LOG_OP(fg_op_param, SE.fg_op);                        \
	                                                                \
    MID_WR_DEFERBUF( &SE , ( sizeof( MIDSetLogicalOperation ) >> 2 ) ); \
	                                                                \
}


	/*----------------------------------------------*
	 * Blit Support Commands (fixed)                *
	 *----------------------------------------------*/

/************************************************************************/
/*                                                                      */
/*  Macro Name: MID_FIFReadPixelData                                    */
/*                                                                      */
/*  Function:                                                           */
/*    Issues the Pedernales Read Pixel Data command element to read     */
/*    pixel data from the adapter.                                      */
/*                                                                      */
/*    Read Pixel Data command elements are not deferred.  Any data in   */
/*    the deferral buffer is written to the FIFO first, then the Read   */
/*    Pixel Data command element is written to the FIFO.  Note that     */
/*    DMA is always used to write this command element to the FIFO.     */
/*                                                                      */
/*  Input:                                                              */
/*    cmdflags (unsigned short) - bit 15:  8 or 24 bit operation (must  */
/*                                  have the value MID_8_BIT_BLIT or    */
/*                                  MID_24_BIT_BLIT - MID_24_BIT_BLIT   */
/*                                  can only be specified if the 24 bit */
/*                                  card is installed and the source of */
/*                                  the blit is a frame buffer)         */
/*                                bit 14:  Indicates whether source_x   */
/*                                  and source_y are window or screen   */
/*                                  coordinates (must have the value    */
/*                                  MID_RPD_WINDOW_COORDS or            */
/*                                  MID_RPD_SCREEN_COORDS)              */
/*                                bits 13-12: reserved (bits must be 0) */
/*                                bits 11-10: color component to be     */
/*                                  read (must have the value MID_READ_ */
/*                                  ALL_COLORS, MID_READ_RED_ONLY,      */
/*                                  MID_READ_GREEN_ONLY, or MID_READ_   */
/*                                  BLUE_ONLY if the 24 bit card is     */
/*                                  installed and the source of the     */
/*                                  blit is a frame buffer, otherwise   */
/*                                  this field is reserved and must be  */
/*                                  0)                                  */
/*                                bits 9-8:  blit direction for the y   */
/*                                  coordinate (must have the value     */
/*                                  MID_BLIT_INCREASING_Y, or           */
/*                                  MID_BLIT_DECREASING_Y)              */
/*                                bits 7-0:  location of each color     */
/*                                  component in the output word - this */
/*                                  field is ignored unless the 24 bit  */
/*                                  card is installed and the source of */
/*                                  the blit is a frame buffer and all  */
/*                                  color components are to be read, in */
/*                                  which case bits 7-6 are reserved    */
/*                                  and must be 0, bits 5-4 specify the */
/*                                  red location, bits 3-2 specify the  */
/*                                  green location, and bits 1-0        */
/*                                  specify the blue location (bit      */
/*                                  pairs 5-4, 3-2, and 1-0 must each   */
/*                                  have a value of 0 to indicate       */
/*                                  bits 7-0, 1 to indicate bits 15-8,  */
/*                                  2 to indicate bits 23-16, or 3 to   */
/*                                  indicate bits 31-24)                */
/*    dest_addr (unsigned long *) - starting address of where the pixel */
/*                                data read is to be placed             */
/*    byte_stride (unsigned short) - number of bytes to be skipped      */
/*                                between rows when each row of pixel   */
/*                                data read is written to the           */
/*                                destination                           */
/*    source (unsigned long)    - source of the blit (must have the     */
/*                                value MID_READ_FRAME_BUF_A, MID_READ_ */
/*                                FRAME_BUF_B, MID_READ_WOG_PLANES,     */
/*                                MID_READ_Z_BUF_HIGH, MID_READ_Z_BUF_  */
/*                                LOW, MID_READ_Z_BUF_MID, MID_READ_Z_  */
/*                                BUF_ALL, MID_READ_DISP_FRAME_BUF, or  */
/*                                MID_READ_DRAW_FRAME_BUF)              */
/*    source_x (short)          - x coordinate of either the upper left */
/*                                corner of the pixel data to be read   */
/*                                if the window origin is the upper     */
/*                                left corner of the window, or the     */
/*                                lower left corner of the pixel data   */
/*                                to be read if the window origin is    */
/*                                the lower left corner of the window   */
/*    source_y (short)          - y coordinate of either the upper left */
/*                                corner of the pixel data to be read   */
/*                                if the window origin is the upper     */
/*                                left corner of the window, or the     */
/*                                lower left corner of the pixel data   */
/*                                to be read if the window origin is    */
/*                                the lower left corner of the window   */
/*    source_width (unsigned short) - the width in pixels of each row   */
/*                                of pixel data to be read              */
/*    source_height (unsigned short) - the number of rows of pixel data */
/*                                to be read                            */
/*    gsc_handle (gHandle)      - graphics system call handle           */
/*                                                                      */
/*  Output:                                                             */
/*    None                                                              */
/*                                                                      */
/************************************************************************/

#define MID_MAX_DMA_FAILURES    4       /* Maximum number of DMA
	                                   failures allowed          */
#define MID_FIFReadPixelData( cmdflags, dest_addr, byte_stride, source, \
	                      source_x, source_y, source_width,         \
	                      source_height, gsc_handle )               \
{                                                                       \
    int                 buf_length;     /* Minimum length of the        \
	                                   destination buffer        */ \
    int                 rc;             /* Returned by aixgsc        */ \
    gscdma              dma_read_struct; /* Device independent DMA      \
	                                   data structure            */ \
    mid_dma_t           mid_dma_struct; /* Device dependent DMA data    \
	                                   structure for Pedernales  */ \
    mid_dma_read_t      mid_dma_read_SE;/* Read Pixel Data command      \
	                                   element structure         */ \
	                                                                \
    /*****************************************************************/ \
    /*                                                               */ \
    /* If there is data in the deferral buffer, flush it now.        */ \
    /*                                                               */ \
    /*****************************************************************/ \
	                                                                \
    if (MID_DEFERBUF_LEN != 0)                                          \
      MID_DEFERBUF_FLUSH;                                               \
	                                                                \
    /*****************************************************************/ \
    /*                                                               */ \
    /* Compute the amount of space, in bytes, needed to hold the     */ \
    /* data that will be read.  All stride bytes must be included in */ \
    /* this computation since the computed value is used to indicate */ \
    /* how much memory to pin for the DMA operation.                 */ \
    /*                                                               */ \
    /*****************************************************************/ \
	                                                                \
    if ((cmdflags & MID_24_BIT_BLIT) == 0 &&                            \
	source != MID_READ_Z_BUF_ALL)   /* 8 bits/pixel to be read   */ \
      buf_length = (source_width + byte_stride) * source_height;        \
    else                                /* 24 bits/pixel to be read  */ \
      buf_length = ((source_width << 2) + byte_stride) * source_height; \
	                                                                \
    /*****************************************************************/ \
    /*                                                               */ \
    /* Set up the portion of the device independent DMA data         */ \
    /* structure that will not change regardless of how many DMA     */ \
    /* operations it will take to complete this read request.        */ \
    /*                                                               */ \
    /*****************************************************************/ \
	                                                                \
    dma_read_struct.flags      = DMA_WAIT | DMA_READ; /* Do a DMA read  \
	                                   operation and wait until it  \
	                                   is complete               */ \
    dma_read_struct.dma_cmd    = &mid_dma_struct;                       \
    dma_read_struct.cmd_length = sizeof(mid_dma_t);                     \
    dma_read_struct.num_sw     = 1;     /* There is 1 buffer to read    \
	                                   into                      */ \
    dma_read_struct.subwindow[0].sw_flag = 0; /* Nothing weird       */ \
	                                                                \
    /*****************************************************************/ \
    /*                                                               */ \
    /* Set up the device dependent DMA structure for Pedernales.     */ \
    /*                                                               */ \
    /*****************************************************************/ \
	                                                                \
    mid_dma_struct.flags   = MID_DMA_READ; /* Do a DMA read operation   \
	                                   by issuing the Pedernales    \
	                                   Read Pixel Data command      \
	                                   element to the FIFO       */ \
    mid_dma_struct.se_size = sizeof(mid_dma_read_t) >> 2;               \
    mid_dma_struct.se_data = &mid_dma_read_SE;                          \
	                                                                \
    /*****************************************************************/ \
    /*                                                               */ \
    /* Set up the portion of the Pedernales Read Pixel Data command  */ \
    /* element that will not change regardless of how many DMA       */ \
    /* operations it will take to complete this read request.        */ \
    /*                                                               */ \
    /*****************************************************************/ \
	                                                                \
    mid_dma_read_SE.length   = sizeof(mid_dma_read_t);                  \
    mid_dma_read_SE.opcode   = OP_READ_PIXEL_DATA ;                     \
    mid_dma_read_SE.corr     = 0;       /* Correlator - used by DMA     \
	                                   services                  */ \
    mid_dma_read_SE.bflags   = cmdflags;                                \
    mid_dma_read_SE.stride   = byte_stride;                             \
    mid_dma_read_SE.sourcebp = source;                                  \
    mid_dma_read_SE.sourcex  = source_x;                                \
    mid_dma_read_SE.width    = source_width;                            \
	                                                                \
    if (buf_length <= MAX_SINGLE_DMA)                                   \
    {                                                                   \
      /***************************************************************/ \
      /*                                                             */ \
      /* The amount of data to be read can be transferred in one     */ \
      /* DMA operation.  Complete the set up of the DMA structures.  */ \
      /*                                                             */ \
      /***************************************************************/ \
	                                                                \
      dma_read_struct.subwindow[0].sw_addr   = dest_addr; /* Starting   \
	                                   address of where to place    \
	                                   the data read             */ \
      dma_read_struct.subwindow[0].sw_length = buf_length;              \
	                                                                \
      mid_dma_read_SE.hostaddr = dest_addr;                             \
      mid_dma_read_SE.sourcey  = source_y;                              \
      mid_dma_read_SE.height   = source_height;                         \
	                                                                \
      /***************************************************************/ \
      /*                                                             */ \
      /* Do the DMA.                                                 */ \
      /*                                                             */ \
      /***************************************************************/ \
	                                                                \
      rc = aixgsc(gsc_handle, DMA_SERVICE, &dma_read_struct);           \
	                                                                \
      if (rc && (errno != ENOMEM))                                      \
      {                                                                 \
      /* XXX TRACE ??? */                                               \
      }                 /* End bad return code from aixgsc           */ \
    }                   /* End data can be transferred with one DMA  */ \
	                                                                \
    if (buf_length > MAX_SINGLE_DMA ||                                  \
	(rc && (errno == ENOMEM)))                                      \
    {                                                                   \
      /***************************************************************/ \
      /*                                                             */ \
      /* The amount of data to be read could not be transferred in   */ \
      /* one DMA operation because either there is too much data, or */ \
      /* the amount of memory requested could not be pinned.         */ \
      /*                                                             */ \
      /***************************************************************/ \
	                                                                \
      int     rem_rows = source_height; /* Number of rows of data       \
	                                   remaining to be read      */ \
      int     max_rows;                 /* Maximum number of rows of    \
	                                   data to be read in a single  \
	                                   DMA operation             */ \
      int     total_source_width;       /* Total width, in bytes, of    \
	                                   each row of data to be read, \
	                                   including the stride bytes*/ \
      int     failures = 0;             /* Number of times a DMA        \
	                                   operation has failed      */ \
	                                                                \
      /***************************************************************/ \
      /*                                                             */ \
      /* Determine the total width, in bytes, of each row of source  */ \
      /* pixel data to be read, and include the stride bytes.        */ \
      /*                                                             */ \
      /***************************************************************/ \
	                                                                \
      if ((cmdflags & MID_24_BIT_BLIT) == 0 &&                          \
	  source != MID_READ_Z_BUF_ALL) /* 8 bits/pixel to be read   */ \
	total_source_width = source_width + byte_stride;                \
      else                              /* 24 bits/pixel to be read  */ \
	total_source_width = (source_width << 2) + byte_stride;         \
	                                                                \
      /***************************************************************/ \
      /*                                                             */ \
      /* Determine the maximum number of rows of data to read in     */ \
      /* each of the following DMA operations.                       */ \
      /*                                                             */ \
      /***************************************************************/ \
	                                                                \
      if (buf_length > MAX_SINGLE_DMA)                                  \
	max_rows = MAX_SINGLE_DMA / total_source_width;                 \
      else                                                              \
      {                                                                 \
	/*************************************************************/ \
	/*                                                           */ \
	/* An attempt was made above to read all of the requested    */ \
	/* data at once, but the attempt failed because not enough   */ \
	/* memory could be pinned.  So, break the data in half and   */ \
	/* try again.                                                */ \
	/*                                                           */ \
	/*************************************************************/ \
	                                                                \
	max_rows = source_height >> 1;                                  \
	failures++;                                                     \
      }                                                                 \
	                                                                \
      dma_read_struct.subwindow[0].sw_addr = dest_addr; /* Starting     \
	                                   address of where to place    \
	                                   the data read             */ \
      mid_dma_read_SE.hostaddr = dest_addr;                             \
      mid_dma_read_SE.sourcey  = source_y;                              \
	                                                                \
      /***************************************************************/ \
      /*                                                             */ \
      /* Break up the data to be read into smaller chunks and issue  */ \
      /* several DMA operations to read it.                          */ \
      /*                                                             */ \
      /***************************************************************/ \
	                                                                \
      while (rem_rows != 0)                                             \
      {                                                                 \
	/*************************************************************/ \
	/*                                                           */ \
	/* Determine how many rows of data to read with the next DMA */ \
	/* operation.                                                */ \
	/*                                                           */ \
	/*************************************************************/ \
	                                                                \
	if (rem_rows > max_rows)                                        \
	  mid_dma_read_SE.height = max_rows;                            \
	else                                                            \
	  mid_dma_read_SE.height = rem_rows;                            \
	                                                                \
	/*************************************************************/ \
	/*                                                           */ \
	/* Determine how much memory to pin.                         */ \
	/*                                                           */ \
	/*************************************************************/ \
	                                                                \
	dma_read_struct.subwindow[0].sw_length =                        \
	          total_source_width * mid_dma_read_SE.height;          \
	                                                                \
	/*************************************************************/ \
	/*                                                           */ \
	/* Do the DMA.                                               */ \
	/*                                                           */ \
	/*************************************************************/ \
	                                                                \
	rc = aixgsc(gsc_handle, DMA_SERVICE, &dma_read_struct);         \
	                                                                \
	if (!rc)                                                        \
	{                                                               \
	  /***********************************************************/ \
	  /*                                                         */ \
	  /* The DMA operation was successful.                       */ \
	  /*                                                         */ \
	  /***********************************************************/ \
	                                                                \
	  rem_rows                -= mid_dma_read_SE.height;            \
	  mid_dma_read_SE.sourcey += mid_dma_read_SE.height;            \
	  mid_dma_read_SE.hostaddr =                                    \
	            ((unsigned char *) (mid_dma_read_SE.hostaddr)) +    \
	            dma_read_struct.subwindow[0].sw_length;             \
	  dma_read_struct.subwindow[0].sw_addr = mid_dma_read_SE.hostaddr; \
	}                                                               \
	else                                                            \
	{                                                               \
	  /***********************************************************/ \
	  /*                                                         */ \
	  /* The DMA operation failed.                               */ \
	  /*                                                         */ \
	  /***********************************************************/ \
	                                                                \
	  if (errno == ENOMEM)                                          \
	  {                                                             \
	    /*********************************************************/ \
	    /*                                                       */ \
	    /* The DMA operation failed because not enough memory    */ \
	    /* could be pinned.  So, break the data in half again    */ \
	    /* and try again if the retry limit has not been reached.*/ \
	    /*                                                       */ \
	    /*********************************************************/ \
	                                                                \
	    max_rows >>= 1;                                             \
	                                                                \
	    failures++;                                                 \
	    if (failures == MID_MAX_DMA_FAILURES || max_rows == 0)      \
	    {                                                           \
	      rem_rows = 0;             /* Set to exit loop          */ \
	      /* XXX TRACE ??? */                                       \
	    }                                                           \
	  }                                                             \
	  else                                                          \
	  {                                                             \
	    /*********************************************************/ \
	    /*                                                       */ \
	    /* The error is not recoverable.  Set rem_rows to 0 to   */ \
	    /* exit the loop.                                        */ \
	    /*                                                       */ \
	    /*********************************************************/ \
	                                                                \
	    rem_rows = 0;                                               \
	    /* XXX TRACE ??? */                                         \
	  }                                                             \
	}               /* End DMA operation failed                  */ \
      }                 /* End loop                                  */ \
    }                                                                   \
}                       /* End MID_FIFReadPixelData                  */


/************************************************************************/
/*                                                                      */
/*  Macro Name: MID_FIFPixelBlit                                        */
/*                                                                      */
/*  Function:                                                           */
/*    Issues the Pedernales Pixel Blit command element to copy a        */
/*    rectangular region of pixel data from one location on the adapter */
/*    to another.                                                       */
/*                                                                      */
/*    Pixel Blit command elements are not deferred.  Any data in the    */
/*    deferral buffer is written to the FIFO first, then the Pixel Blit */
/*    command element is written to the FIFO.                           */
/*                                                                      */
/*  Input:                                                              */
/*    flags (unsigned short)    - bit 15:  8 or 24 bit operation (must  */
/*                                  have the value MID_8_BIT_BLIT or    */
/*                                  MID_24_BIT_BLIT - MID_24_BIT_BLIT   */
/*                                  can only be specified if the 24 bit */
/*                                  card is installed and the           */
/*                                  destination of the blit is a frame  */
/*                                  buffer and the source of the blit   */
/*                                  is either a frame buffer or a       */
/*                                  color)                              */
/*                                bit 14:  Indicates whether source_x,  */
/*                                  source_y, dest_x, and dest_y are    */
/*                                  window or screen coordinates (must  */
/*                                  have the value MID_PB_WINDOW_COORDS */
/*                                  or MID_PB_SCREEN_COORDS)            */
/*                                bits 13-11: reserved (bits must be 0) */
/*                                bit 10:  execution flag (must have    */
/*                                  the value MID_BLIT_EXEC_IMMED or    */
/*                                  MID_BLIT_EXEC_NEXT_VERT_RETRACE)    */
/*                                bits 9-0:  reserved (bits must be 0)  */
/*    source (unsigned long)    - source of the blit (must have the     */
/*                                value MID_READ_FRAME_BUF_A, MID_READ_ */
/*                                FRAME_BUF_B, MID_READ_WOG_PLANES,     */
/*                                MID_READ_Z_BUF_HIGH, MID_READ_Z_BUF_  */
/*                                LOW, MID_READ_Z_BUF_MID, MID_READ_    */
/*                                PIXEL_BLIT_COLOR, MID_READ_DISP_      */
/*                                FRAME_BUF, or MID_READ_DRAW_FRAME_BUF)*/
/*    source_x (short)          - x coordinate of either the upper left */
/*                                corner of the source pixel data if    */
/*                                the window origin is the upper left   */
/*                                corner of the window, or the lower    */
/*                                left corner of the source pixel data  */
/*                                if the window origin is the lower     */
/*                                left corner of the window (this param */
/*                                is ignored if the source of the blit  */
/*                                is MID_READ_PIXEL_BLIT_COLOR)         */
/*    source_y (short)          - y coordinate of either the upper left */
/*                                corner of the source pixel data if    */
/*                                the window origin is the upper left   */
/*                                corner of the window, or the lower    */
/*                                left corner of the source pixel data  */
/*                                if the window origin is the lower     */
/*                                left corner of the window (this param */
/*                                is ignored if the source of the blit  */
/*                                is MID_READ_PIXEL_BLIT_COLOR)         */
/*    source_width (unsigned short) - width in pixels of each row of    */
/*                                pixel data to be blitted              */
/*    source_height (unsigned short) - number of rows of pixel data to  */
/*                                be blitted                            */
/*    dest (unsigned long)      - destination of the blit (must have    */
/*                                the value MID_WRITE_FRAME_BUF_A,      */
/*                                MID_WRITE_FRAME_BUF_B, MID_WRITE_     */
/*                                BOTH_FRAME_BUFS, MID_WRITE_WOG_PLANES,*/
/*                                MID_WRITE_Z_BUF_HIGH, MID_WRITE_Z_    */
/*                                BUF_LOW, MID_WRITE_Z_BUF_MID, MID_    */
/*                                WRITE_Z_BUF_HIGH_MID, MID_WRITE_Z_    */
/*                                BUF_HIGH_LOW, MID_WRITE_Z_BUF_MID_LOW,*/
/*                                MID_WRITE_Z_BUF_ALL_SAME, MID_WRITE_  */
/*                                DISP_FRAME_BUF, or MID_WRITE_DRAW_    */
/*                                FRAME_BUF)                            */
/*    dest_x (short)            - x coordinate of either the upper left */
/*                                corner of the location where the      */
/*                                pixel data is to be blitted if the    */
/*                                window origin is the upper left       */
/*                                corner of the window, or the lower    */
/*                                left corner of the location where the */
/*                                pixel data is to be blitted if the    */
/*                                window origin is the lower left       */
/*                                corner of the window                  */
/*    dest_y (short)            - y coordinate of either the upper left */
/*                                corner of the location where the      */
/*                                pixel data is to be blitted if the    */
/*                                window origin is the upper left       */
/*                                corner of the window, or the lower    */
/*                                left corner of the location where the */
/*                                pixel data is to be blitted if the    */
/*                                window origin is the lower left       */
/*                                corner of the window                  */
/*    blit_dir_source (unsigned short) - source of the blit direction   */
/*                                information (must have the value      */
/*                                MID_BLIT_DIR_USER_SUPPLIED or         */
/*                                MID_BLIT_DIR_ADAPTER_SUPPLIED)        */
/*    blit_dir (unsigned short) - blit direction (must have the value   */
/*                                MID_BLIT_INCR_X_Y, MID_BLIT_DECR_X_Y, */
/*                                MID_BLIT_DECR_X_INCR_Y, or            */
/*                                MID_BLIT_INCR_X_DECR_Y if blit_dir_   */
/*                                source is MID_BLIT_DIR_USER_SUPPLIED, */
/*                                otherwise this parameter is ignored)  */
/*    blit_color (unsigned long) - color to be blitted - in 8 bit color */
/*                                mode the low order 8 bits are used,   */
/*                                and in 24 bit color mode the low      */
/*                                order 24 bits are used (this param    */
/*                                is ignored if the source of the blit  */
/*                                is not MID_READ_PIXEL_BLIT_COLOR)     */
/*    logic_op (unsigned char)  - logical operation to be used (must be */
/*                                an X, graPHIGS, or GL logical         */
/*                                operation value)                      */
/*                                                                      */
/*  Output:                                                             */
/*    None                                                              */
/*                                                                      */
/************************************************************************/

#define MID_FIFPixelBlit( flags, source, source_x, source_y,            \
	                  source_width, source_height, dest, dest_x,    \
	                  dest_y, blit_dir_source, blit_dir,            \
	                  blit_color, logic_op )                        \
{                                                                       \
    MIDFIFPixelBlit     SE;                                             \
	                                                                \
    SE.hdr.halfword.length = sizeof(MIDFIFPixelBlit);                   \
    SE.hdr.halfword.op     = OP_PIXEL_BLIT ;                            \
    SE.resv     = 0;                                                    \
    SE.bflags   = flags;                                                \
    SE.sourcebp = source;                                               \
    SE.sx       = source_x;                                             \
    SE.sy       = source_y;                                             \
    SE.sw       = source_width;                                         \
    SE.sh       = source_height;                                        \
    SE.destbp   = dest;                                                 \
    SE.dx       = dest_x;                                               \
    SE.dy       = dest_y;                                               \
    SE.sbdir    = blit_dir_source;                                      \
    SE.bdir     = blit_dir;                                             \
    SE.pb_color = blit_color;                                           \
    SE.resv1    = 0;                                                    \
	                                                                \
    MID_TRANSLATE_LOG_OP(logic_op, SE.pb_op);                           \
	                                                                \
    /*****************************************************************/ \
    /*                                                               */ \
    /* MID_WRITE_TO_FIFO will flush any data in the deferral buffer  */ \
    /* then write the Pixel Blit command element to the FIFO as      */ \
    /* efficiently as possible.                                      */ \
    /*                                                               */ \
    /*****************************************************************/ \
	                                                                \
    MID_WRITE_TO_FIFO( &SE , (sizeof(MIDFIFPixelBlit) >> 2) );          \
	                                                                \
}

#define MID_FIFZoomScreen( bflags_param , xrepl_param , yrepl_param ,   \
	                        source_param , sw_param , sh_param ,    \
	                        sx_param , sy_param , destbp_param ,    \
	                        dx_param , dy_param )                   \
{                                                                       \
    MIDFIFZoomScreen    SE;                                             \
	                                                                \
    SE.hdr.halfword.length = ( ushort ) sizeof( MIDFIFZoomScreen );     \
    SE.hdr.halfword.op     = ( ushort ) OP_ZOOM ;                       \
    SE.resv                = ( ushort ) 0;                              \
    SE.bflags              = ( ushort ) bflags_param;                   \
    SE.xrepl               = ( ushort ) xrepl_param;                    \
    SE.yrepl               = ( ushort ) yrepl_param;                    \
    SE.source              = ( ulong )  source_param;                   \
    SE.sx                  = ( short )  sx_param;                       \
    SE.sy                  = ( short )  sy_param;                       \
    SE.sw                  = ( ushort ) sw_param;                       \
    SE.sh                  = ( ushort ) sh_param;                       \
    SE.destbp              = ( ulong )  destbp_param;                   \
    SE.dx                  = ( short )  dx_param;                       \
    SE.dy                  = ( short )  dy_param;                       \
	                                                                \
    MID_WRITE_TO_FIFO( &SE , ( sizeof( MIDFIFZoomScreen ) >> 2 ) );     \
	                                                                \
}


	/*----------------------------------------------*
	 * Color Processing Control Commands (fixed)    *
	 *----------------------------------------------*/


#define MID_TransparencyAndColorCompare( flags_param ,                  \
	                                        t_cond_value_param ,    \
	                                        cc_cond_value_param )   \
{                                                                       \
    MIDTransparencyAndColorCompare      SE;                             \
	                                                                \
    SE.hdr.halfword.length = ( ushort )                                 \
	                sizeof( MIDTransparencyAndColorCompare );       \
    SE.hdr.halfword.op     = ( ushort ) OP_TRANSPARENCY_AND_COLOR_COMPARE ;\
    SE.flags               = ( ulong )  flags_param;                    \
    SE.t_cond_value        = ( ulong )  t_cond_value_param;             \
    SE.cc_cond_value       = ( ulong )  cc_cond_value_param;            \
	                                                                \
    MID_WR_DEFERBUF( &SE ,                                              \
	        ( sizeof( MIDTransparencyAndColorCompare ) >> 2 ) );    \
	                                                                \
}

	/*----------------------------------------------*
	 * Miscellaneous Commands (fixed)               *
	 *----------------------------------------------*/

#define MID_SynchronizeOnVerticalRetrace( vrcnt_param )                 \
{                                                                       \
    MIDSynchronizeOnVerticalRetrace     SE;                             \
	                                                                \
    SE.hdr.halfword.length = ( ushort )                                 \
	                sizeof( MIDSynchronizeOnVerticalRetrace.);      \
    SE.hdr.halfword.op     = ( ushort ) OP_SYNCHRONIZE_ON_VERTICAL_RETRACE ;\
    SE.resv                = ( ushort ) 0;                              \
    SE.vrcnt               = ( ushort ) vrcnt_param;                    \
	                                                                \
    MID_WRITE_TO_FIFO( &SE ,                                            \
	        ( sizeof( MIDSynchronizeOnVerticalRetrace ) >> 2 ) );   \
	                                                                \
}

#define MID_ResetLinePattern                                            \
{                                                                       \
    MIDResetLinePattern SE;                                             \
	                                                                \
    SE.hdr.halfword.length = ( ushort ) sizeof( MIDResetLinePattern );  \
    SE.hdr.halfword.op     = ( ushort ) OP_RESET_LINE_PATTERN;          \
	                                                                \
    MID_WR_DEFERBUF( &SE , ( sizeof( MIDResetLinePattern ) >> 2 ) );    \
	                                                                \
}

#define MID_Synchronization( scorr_param )                              \
{                                                                       \
    MIDSynchronization  SE;                                             \
	                                                                \
    SE.hdr.halfword.length = ( ushort ) sizeof( MIDSynchronization );   \
    SE.hdr.halfword.op     = ( ushort ) OP_SYNCHRONIZATION ;            \
    SE.scorr               = ( ushort ) scorr_param;                    \
    SE.resv                = ( ushort ) 0;                              \
	                                                                \
    MID_WRITE_TO_FIFO( &SE , ( sizeof( MIDSynchronization ) >> 2 ) );   \
	                                                                \
}


 /* ---------------------------------------------------------------------
   Associate One Window with Color Palette macro

   NOTE:  There are two macros: Assoc ONE Window with Color Palette and
	  Associate Window with Color Palette (which maps multiple windows
	  to their respective color palettes).   The first macro is simply
	  a special case of the more general.
  ---------------------------------------------------------------------- */

#define ASSOC_FRAME_BUFFER_NULL 0x0
#define ASSOC_FRAME_BUFFER_A    0x20
#define ASSOC_FRAME_BUFFER_B    0x30


#ifndef MID_DD

#define MID_AssocOneWindowWithColorPaletteFIF( wid, pal_buffer )              \
{                                                                             \
	MIDAssocOneWindowWithColorPaletteFIF SE;                              \
	                                                                      \
	SE.hdr.halfword.length =                                              \
	        (ushort) sizeof(MIDAssocOneWindowWithColorPaletteFIF) ;       \
	SE.hdr.halfword.op      = (ushort)                                    \
	                        OP_ASSOC_WINDOW_WITH_COLOR_PAL_FIFO;          \
	SE.w_id                 = (ushort) wid ;                              \
	SE.cp_id                = (ushort) (pal_buffer) ;                     \
	                                                                      \
	MID_WRITE_TO_FIFO                                                     \
	        (&SE,(sizeof(MIDAssocOneWindowWithColorPaletteFIF) >> 2 ));   \
}

#else  /* MID_DD */

#define MID_AssocOneWindowWithColorPaletteFIF( wid, pal_buffer )             \
{                                                                            \
	MIDAssocOneWindowWithColorPaletteFIF SE;                             \
	                                                                     \
	BUGXDEF (trc_data_Associate_Color) ;                                 \
	                                                                     \
	SE.hdr.halfword.length =                                             \
	        (ushort) sizeof(MIDAssocOneWindowWithColorPaletteFIF) ;      \
	SE.hdr.halfword.op      = (ushort)                                   \
	                          OP_ASSOC_WINDOW_WITH_COLOR_PAL_FIFO ;      \
	SE.w_id                 = (ushort) wid ;                             \
	SE.cp_id                = (ushort) (pal_buffer) ;                    \
	                                                                     \
	                                                                     \
	MID_DD_TRACE_DATA (Associate_Color,                                  \
	                   ASSOCIATE_COLOR_SE,                               \
	                   0, &SE, sizeof(SE) ) ;                            \
	                                                                     \
	MID_WR_FIFO                                                   \
	        (&SE,(sizeof(MIDAssocOneWindowWithColorPaletteFIF) >> 2 ));  \
}

#if 0
	printf("\n--------------------  Assoc Window with CP, Buffer"        \
	       " --------------------\n" )  ;                                \
	printf("leng, opcode  %4X %4X\n",SE.hdr.halfword.length,             \
	                     SE.hdr.halfword.op);                            \
	printf("WID, flags:   %4X %4X\n\n",SE.w_id, SE.cp_id);               \
	                                                                     \

#endif

#endif  /* MID_DD */







	/*----------------------------------------------*
	 * The section below defines a set of macros    *
	 * to build FIFO command elements which can be  *
	 * variable in length or which might have       *
	 * a format that can vary based on a value of   *
	 * one of the fields                            *
	 *----------------------------------------------*/

	/*----------------------------------------------*
	 * Adapter Control Commands (variable)          *
	 *----------------------------------------------*/


 /* ---------------------------------------------------------------------
   Associate Window with Color Palette macro

   NOTE:  See note preceding the previous macro.

   Function:  Maps a window ID to a (one of 5) color palettes.  Can be used
	      to map several windows with a single command.
	
   Input:      id_pair_param -- array of struct _MID_id_pair
	        (defined in "hw_typdefs.h").

	        count_param -- number of "id_pair"s in array.
	
  ---------------------------------------------------------------------- */
#ifndef MID_DD

#define MID_AssocWindowWithColorPaletteFIF( id_pair_param , count_param )      \
{                                                                              \
    MIDAssocWindowWithColorPaletteFIF   SE;                                    \
	                                                                       \
    SE.hdr.halfword.length = ( ushort )                                        \
	                ( sizeof( MIDAssocWindowWithColorPaletteFIF ) +        \
	                                        ( count_param << 2 ) );        \
    SE.hdr.halfword.op = (ushort) OP_ASSOC_WINDOW_WITH_COLOR_PAL_FIFO;         \
	                                                                       \
    MID_WRITE_TO_FIFO (&SE, (sizeof (MIDAssocWindowWithColorPaletteFIF) >> 2); \
    MID_WRITE_TO_FIFO (id_pair_param, count_param) ;                           \
}

#else /* MID_DD */

#define MID_AssocWindowWithColorPaletteFIF(id_pair_param, count_param)        \
{                                                                             \
    MIDAssocWindowWithColorPaletteFIF   SE;                                   \
	                                                                      \
    SE.hdr.halfword.length = ( ushort )                                       \
	                ( sizeof( MIDAssocWindowWithColorPaletteFIF ) +       \
	                                        ( count_param << 2 ) );       \
   SE.hdr.halfword.op   = (ushort) OP_ASSOC_WINDOW_WITH_COLOR_PAL_FIFO ;      \
	                                                                      \
    MID_WR_FIFO (&SE, (sizeof (MIDAssocWindowWithColorPaletteFIF) >> 2);      \
    MID_WR_FIFO (id_pair_param, count_param) ;                                \
}

#endif /* MID_DD */


/*-----------------------------------------------------------------------------
   Set Window Parameters (FIFO) macro
 -----------------------------------------------------------------------------*/

/* Define the maximum number of visible rects that can be given to the  */
/* SetWindowParameters SE. NOTE: If there is only rect given, then that */
/* rect is used as the clipping bounding box. The extents always define */
/* the origin, width, and height; and also the clipping area IF there   */
/* are more than 1 visible rect. If there is only 1 visible rect then   */
/* the origin is disjoint from the clipping area.                       */

#define MAX_VIS_RECT    4


#ifdef MID_DD

#define MID_SetWindowParametersFIF( w_id_param, woflags_param,        \
	correlator_param,  window_extents_param,                              \
	num_visual_rects_param, visual_rects_param)                           \
{                                                                             \
	struct _SE                                                            \
	{                                                                     \
	        MIDSetWindowParametersFIF s;                                  \
	        MIDCornerRectangle        vr[MAX_VIS_RECT];  /*var portion */ \
	} SE ;                                                                \
	                                                                      \
	int vis_rect;                                                         \
	int  num_vis ;                                                        \
	                                                                      \
	BUGXDEF (trc_data_SWP_FIFO) ;                                         \
	                                                                      \
	num_vis = num_visual_rects_param ;                                    \
	if ( num_visual_rects_param > MAX_VIS_RECT )                          \
	{                                                                     \
	    num_vis = MAX_VIS_RECT ;                                          \
	}                                                                     \
	                                                                      \
   SE.s.hdr.halfword.length = (ushort) sizeof( MIDSetWindowParametersFIF)     \
	        + ( num_vis * sizeof(MIDCornerRectangle));                    \
	                                                                      \
	SE.s.hdr.halfword.op      = ( ushort ) OP_SET_WINDOW_PARAMETERS ;     \
	SE.s.correlator           = ( ushort ) correlator_param;              \
	SE.s.reserved             = ( ushort ) 0x0000;                        \
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
	for (vis_rect=0;     vis_rect<num_vis;    vis_rect++)                 \
	{                                                                     \
	        SE.vr[vis_rect].rulx = visual_rects_param[vis_rect].ul.x;     \
	        SE.vr[vis_rect].ruly = visual_rects_param[vis_rect].ul.y;     \
	        SE.vr[vis_rect].rlrx = visual_rects_param[vis_rect].lr.x;     \
	        SE.vr[vis_rect].rlry = visual_rects_param[vis_rect].lr.y;     \
	                                                                      \
	}                                                                     \
	                                                                      \
	/*------------------------------------------------------------------* \
	   Write the command to the FIFO.  Since we have already moved the    \
	   visible rectangles into a single buffer, we just do a single write \
	   to the FIFO.  If we didn't move the vis rects, we would probably   \
	   do two writes.                                                     \
	                                                                      \
	   Shift of 2 converts number of bytes to number of words             \
	 *-----------------------------------------------------------------*/ \
	                                                                      \
	MID_DD_TRACE_DATA (SWP_FIFO,                                          \
	                   WRITE_WINDOW_PARMS_FIFO_SE,                        \
	                   0, &SE,                                            \
	                   ((sizeof(MIDSetWindowParametersFIF) +              \
	                   (num_vis * sizeof(MIDCornerRectangle)) ) ) ) ;     \
	                                                                      \
	MID_WR_FIFO (&SE, ((sizeof(MIDSetWindowParametersFIF) +               \
	                   (num_vis * sizeof(MIDCornerRectangle)) ) >> 2));\
	                                                                      \
}


#if   0
/******************************************************************************
   old printfs
 ******************************************************************************/
	                                                                      \
	printf("\n--------------------  Set Window Parms FIFO SE"             \
	       " --------------------\n" )  ;                                 \
	printf("leng, opcode  %4X %4X\n",SE.s.hdr.halfword.length,            \
	                     SE.s.hdr.halfword.op);                           \
	printf("correlator:   %4X %4X\n",SE.s.correlator,SE.s.reserved);      \
	printf("WID, flags:   %4X %4X\n",SE.s.w_id, SE.s.woflags);            \
	printf("extents - ul  %4X %4X\n",SE.s.window_extents.rulx,            \
	                     SE.s.window_extents.ruly);                       \
       printf("        - lr  %4X %4X _____________ END of FIXED part of SE\n",\
	                     SE.s.window_extents.rlrx,                        \
	                     SE.s.window_extents.rlry);                       \
	                                                                      \
	                                                                      \
	    printf("vis rect %d    %4X %4X\n", vis_rect,                      \
	            SE.vr[vis_rect].rulx, SE.vr[vis_rect].ruly);              \
	    printf("              %4X %4X\n", SE.vr[vis_rect].rlrx,           \
	            SE.vr[vis_rect].rlry);                                    \
	}                                                                     \
	printf("--------------------- END of Set Window Parms FIFO SE"        \
	       " --------------------\n"  ) ;                                 \
	


#endif

#else  /* not MID_DD */

#define MID_SetWindowParametersFIF( w_id_param , woflags_param ,        \
	                                correlator_param ,              \
	                                window_x_origin_param ,         \
	                                window_y_origin_param ,         \
	                                window_width_param ,            \
	                                window_height_param ,           \
	                                num_visual_rects_param ,        \
	                                visual_rects_param )            \
{                                                                       \
    struct _SE                                                          \
    {                                                                   \
	MIDSetWindowParametersFIF s;                                    \
	MIDCornerRectangle        vr[MAX_VIS_RECT];  /* var portion */  \
    } SE ;                                                              \
    int vis_rect;                                                       \
    int num_vis;                                                        \
    unsigned short      *ptr;                                           \
	                                                                \
    num_vis = num_visual_rects_param ;                                  \
    if ( num_visual_rects_param > MAX_VIS_RECT )                        \
    {                                                                   \
	num_vis = MAX_VIS_RECT ;                                        \
    }                                                                   \
	                                                                \
    if(num_vis == 0)                                                    \
      SE.s.hdr.halfword.length = ( ushort       )                       \
	                ( sizeof( MIDSetWindowParametersFIF ) +         \
	                (sizeof( MIDCornerRectangle )) );               \
    else                                                                \
      SE.s.hdr.halfword.length = ( ushort       )                       \
	                ( sizeof( MIDSetWindowParametersFIF ) +         \
	                (num_vis * sizeof( MIDCornerRectangle )) );     \
	                                                                \
    SE.s.hdr.halfword.op      = ( ushort ) OP_SET_WINDOW_PARAMETERS;    \
    SE.s.correlator           = ( ushort ) correlator_param;            \
    SE.s.reserved             = ( ushort ) 0x0000;                      \
    SE.s.w_id                 = ( ushort ) w_id_param;                  \
    SE.s.woflags              = ( ushort ) woflags_param;               \
    SE.s.window_extents.rulx  = ( short )  window_x_origin_param;       \
    SE.s.window_extents.ruly  = ( short )  window_y_origin_param;       \
    SE.s.window_extents.rlrx  = ( short )  window_x_origin_param +      \
	                                   window_width_param - 1;      \
    SE.s.window_extents.rlry  = ( short )  window_y_origin_param +      \
	                                   window_height_param - 1;     \
	                                                                \
    if(SE.s.window_extents.rlrx < SE.s.window_extents.rulx)             \
	SE.s.window_extents.rlrx = SE.s.window_extents.rulx;            \
	                                                                \
    if(SE.s.window_extents.rlry < SE.s.window_extents.ruly)             \
	SE.s.window_extents.rlry = SE.s.window_extents.ruly;            \
	                                                                \
   /******************************************************************/ \
   /* If there num_vis == 0, then we send only 1 vis_rect which is   */ \
   /* the same as the extents.                                       */ \
   /******************************************************************/ \
	                                                                \
   if(num_vis == 0){                                                    \
	SE.vr[0].rulx = SE.s.window_extents.rulx;                       \
	SE.vr[0].ruly = SE.s.window_extents.ruly;                       \
	SE.vr[0].rlrx = SE.s.window_extents.rlrx;                       \
	SE.vr[0].rlry = SE.s.window_extents.rlry;                       \
	                                                                \
   }else{                                                               \
   /******************************************************************/ \
   /* Now, for every exposure rectangle in the exposure list, put    */ \
   /* the rectangle extents into the SEV array.                      */ \
   /******************************************************************/ \
    ptr = (unsigned short *) visual_rects_param;                        \
    for (vis_rect=0;     vis_rect<num_vis;    vis_rect++)               \
    {                                                                   \
	SE.vr[vis_rect].rulx = *ptr++;                                  \
	SE.vr[vis_rect].ruly = *ptr++;                                  \
	SE.vr[vis_rect].rlrx = (*ptr++) - 1;                            \
	SE.vr[vis_rect].rlry = (*ptr++) - 1;                            \
    }                                                                   \
   }                                                                    \
	                                                                \
   /******************************************************************/ \
   /* Write the command to the FIFO.  Since we have already moved    */ \
   /* the visible rectangles into a single buffer, we just do a      */ \
   /* single write to the FIFO.  If we didn't move the vis rects, we */ \
   /* would probably do two writes.                                  */ \
   /*                                                                */ \
   /* Shift of 2 converts number of bytes to number of words         */ \
   /******************************************************************/ \
	                                                                \
   MID_WRITE_TO_FIFO( &SE, ((SE.s.hdr.halfword.length) >> 2));          \
}

#endif  /* NON MID_DD */


	/*----------------------------------------------*
	 * Blit Support Commands (variable)             *
	 *----------------------------------------------*/

/************************************************************************/
/*                                                                      */
/*  Macro Name: MID_FIFWritePixelData                                   */
/*                                                                      */
/*  Function:                                                           */
/*    Issues the Pedernales Write Pixel Data command element to write   */
/*    pixel data to the adapter.  This macro will choose the fastest    */
/*    method (PIO or DMA) of transferring the data to the adapter.      */
/*                                                                      */
/*    Write Pixel Data command elements are not deferred.  Any data in  */
/*    the deferral buffer is written to the FIFO first, then the Write  */
/*    Pixel Data command element is written to the FIFO.                */
/*                                                                      */
/*  Input:                                                              */
/*    cmdflags (unsigned short) - bit 15:  8 or 24 bit operation (must  */
/*                                  have the value MID_8_BIT_BLIT or    */
/*                                  MID_24_BIT_BLIT - MID_24_BIT_BLIT   */
/*                                  can only be specified if the 24 bit */
/*                                  card is installed and the           */
/*                                  destination of the blit is a frame  */
/*                                  buffer)                             */
/*                                bits 14-13: indicates whether window, */
/*                                  screen, or modelling coordinates    */
/*                                  are specified in this command (must */
/*                                  have the value MID_WPD_WINDOW_      */
/*                                  COORDS, MID_WPD_SCREEN_COORDS, or   */
/*                                  MID_WPD_MODELLING_COORDS)           */
/*                                bit 12:  color expand enable flag     */
/*                                  (must have the value                */
/*                                  MID_WRITE_NO_COLOR_EXPAND or        */
/*                                  MID_WRITE_COLOR_EXPAND)             */
/*                                bit 11:  data transfer method (this   */
/*                                  flag is set by this macro)          */
/*                                bit 10:  execution flag (must have    */
/*                                  the value MID_BLIT_EXEC_IMMED or    */
/*                                  MID_BLIT_EXEC_NEXT_VERT_RETRACE)    */
/*                                bits 9-8:  blit direction for the y   */
/*                                  coordinate (must have the value     */
/*                                  MID_BLIT_INCREASING_Y, or           */
/*                                  MID_BLIT_DECREASING_Y)              */
/*                                bits 7-0:  location of each color     */
/*                                  component in the input word - this  */
/*                                  field is ignored unless the 24 bit  */
/*                                  card is installed and the           */
/*                                  destination of the blit is a frame  */
/*                                  buffer, in which case bits 7-6 are  */
/*                                  reserved and must be 0, bits 5-4    */
/*                                  specify the red location, bits 3-2  */
/*                                  specify the green location, and     */
/*                                  bits 1-0 specify the blue location  */
/*                                  (bit pairs 5-4, 3-2, and 1-0 must   */
/*                                  each have a value of 0 to indicate  */
/*                                  bits 7-0, 1 to indicate bits 15-8,  */
/*                                  2 to indicate bits 23-16, or 3 to   */
/*                                  indicate bits 31-24)                */
/*    data_ptr (unsigned long *) - pointer to the first word of source  */
/*                                pixel data to be written              */
/*    bits_per_pixel (short)    - number of bits per pixel in the       */
/*                                source data (must be 1, 8, or 32)     */
/*    byte_stride (unsigned short - number of bytes to be skipped       */
/*                                between rows of the source pixel data */
/*    dest (unsigned long)      - destination of the blit (must have    */
/*                                the value MID_WRITE_FRAME_BUF_A,      */
/*                                MID_WRITE_FRAME_BUF_B, MID_WRITE_     */
/*                                BOTH_FRAME_BUFS, MID_WRITE_WOG_PLANES,*/
/*                                MID_WRITE_Z_BUF_HIGH, MID_WRITE_Z_    */
/*                                BUF_LOW, MID_WRITE_Z_BUF_MID, MID_    */
/*                                WRITE_Z_BUF_HIGH_MID, MID_WRITE_Z_    */
/*                                BUF_HIGH_LOW, MID_WRITE_Z_BUF_MID_LOW,*/
/*                                MID_WRITE_Z_BUF_ALL_SAME, MID_WRITE_  */
/*                                Z_BUF_ALL, MID_WRITE_DISP_FRAME_BUF,  */
/*                                or MID_WRITE_DRAW_FRAME_BUF)          */
/*    dest_x (short)            - x coordinate of either the upper left */
/*                                corner of the location where the      */
/*                                pixel data is to be written if the    */
/*                                window origin is the upper left       */
/*                                corner of the window, or the lower    */
/*                                left corner of the location where the */
/*                                pixel data is to be written if the    */
/*                                window origin is the lower left       */
/*                                corner of the window (this parameter  */
/*                                is ignored if modelling coordinates   */
/*                                are used)                             */
/*    dest_y (short)            - y coordinate of either the upper left */
/*                                corner of the location where the      */
/*                                pixel data is to be written if the    */
/*                                window origin is the upper left       */
/*                                corner of the window, or the lower    */
/*                                left corner of the location where the */
/*                                pixel data is to be written if the    */
/*                                window origin is the lower left       */
/*                                corner of the window (this parameter  */
/*                                is ignored if modelling coordinates   */
/*                                are used)                             */
/*    src_width (unsigned short) - width in pixels of each row of       */
/*                                source pixel data to be written       */
/*    src_height (unsigned short) - number of rows of source pixel data */
/*                                to be written                         */
/*    mod_x (float)               x coordinate of the upper left corner */
/*                                of the pixel rectangle (this          */
/*                                parameter is ignored if window or     */
/*                                screen coordinates are used)          */
/*    mod_y (float)               y coordinate of the upper left corner */
/*                                of the pixel rectangle (this          */
/*                                parameter is ignored if window or     */
/*                                screen coordinates are used)          */
/*    mod_z (float)               z coordinate of the upper left corner */
/*                                of the pixel rectangle (this          */
/*                                parameter is ignored if window or     */
/*                                screen coordinates are used)          */
/*    first_bit (unsigned long) - number of bits preceding the first    */
/*                                valid bit in the first byte of pixel  */
/*                                data to be written (must be >= 0 and  */
/*                                <= 7 if color expansion is specified, */
/*                                otherwise this field must be 0)       */
/*    x_rep (unsigned short)    - number of times each pixel is to be   */
/*                                written in the x direction (must be 1 */
/*                                if color expansion is specified)      */
/*    y_rep (unsigned short)    - number of times each pixel is to be   */
/*                                written in the y direction (must be 1 */
/*                                if color expansion is specified)      */
/*    fg_color (unsigned long)  - foreground pixel value - in 8 bit     */
/*                                color mode the low order 8 bits are   */
/*                                used, and in 24 bit color mode the    */
/*                                low order 24 bits are used (this      */
/*                                parameter is ignored if color         */
/*                                expansion is not specified)           */
/*    bg_color (unsigned long)  - background pixel value - in 8 bit     */
/*                                color mode the low order 8 bits are   */
/*                                used, and in 24 bit color mode the    */
/*                                low order 24 bits are used (this      */
/*                                parameter is ignored if color         */
/*                                expansion is not specified)           */
/*    fg_logic_op (unsigned char) - foreground logical operation (must  */
/*                                be an X, graPHIGS, or GL logical      */
/*                                operation value if color expansion is */
/*                                specified, otherwise this parameter   */
/*                                is ignored)                           */
/*    bg_logic_op (unsigned char) - background logical operation (must  */
/*                                be an X, graPHIGS, or GL logical      */
/*                                operation value if color expansion is */
/*                                specified, otherwise this parameter   */
/*                                is ignored)                           */
/*    gsc_handle (gHandle)      - graphics system call handle           */
/*                                                                      */
/*  Output:                                                             */
/*    None                                                              */
/*                                                                      */
/************************************************************************/

#define MID_WPD_DMA_THRESH      1050    /* PIO/DMA threshold + header */

#define MID_FIFWritePixelData( cmdflags, data_ptr, bits_per_pixel,      \
	                       byte_stride, dest, dest_x, dest_y,       \
	                       src_width, src_height, mod_x, mod_y,     \
	                       mod_z, first_bit, x_rep, y_rep,          \
	                       fg_color, bg_color, fg_logic_op,         \
	                       bg_logic_op, gsc_handle )                \
{                                                                       \
    unsigned int        words_to_xfer;                                  \
    unsigned int        bytes_per_row;                                  \
    unsigned short      stride_needed;                                  \
    int                 total_src_width;                                \
    int                 rem_rows = src_height;                          \
    int                 max_rows;                                       \
	                                                                \
    /*****************************************************************/ \
    /*                                                               */ \
    /* Compute the number of bytes of pixel data that will be        */ \
    /* written if PIO is used.                                       */ \
    /*                                                               */ \
    /*****************************************************************/ \
	                                                                \
    switch (bits_per_pixel)                                             \
    {                                                                   \
      case 1:                                                           \
	                                                                \
	/*************************************************************/ \
	/*                                                           */ \
	/* Each pixel is a bit.                                      */ \
	/*                                                           */ \
	/*************************************************************/ \
	                                                                \
	/*************************************************************/ \
	/*                                                           */ \
	/* Determine how many stride bytes are needed to cause each  */ \
	/* row of data that will be transferred to be a multiple of  */ \
	/* words in length.                                          */ \
	/*                                                           */ \
	/*************************************************************/ \
	                                                                \
	bytes_per_row = (first_bit + src_width + 7) >> 3;               \
	                                                                \
	if ((bytes_per_row & 3) == 0)                                   \
	  stride_needed = 0;                                            \
	else                                                            \
	  stride_needed = 4 - (bytes_per_row & 3);                      \
	                                                                \
	if (byte_stride >= stride_needed)                               \
	{                                                               \
	  /***********************************************************/ \
	  /*                                                         */ \
	  /* Enough stride bytes are present.  If PIO is used, the   */ \
	  /* pixel data will be transferred to the adapter with the  */ \
	  /* minimum number of stride bytes at the end of each row   */ \
	  /* to cause each row to be a multiple of words in length.  */ \
	  /*                                                         */ \
	  /***********************************************************/ \
	                                                                \
	  total_src_width = bytes_per_row + stride_needed;              \
	  words_to_xfer   = (total_src_width * src_height) >> 2;        \
	}                                                               \
	else                                                            \
	{                                                               \
	  /***********************************************************/ \
	  /*                                                         */ \
	  /* Not enough stride bytes are present.  If PIO is used,   */ \
	  /* all of the pixel data and the stride bytes present will */ \
	  /* be transferred to the adapter at once.  An additional   */ \
	  /* 0 - 3 bytes will be included in the transfer because    */ \
	  /* the total amount of data transferred must be a multiple */ \
	  /* of 4 bytes.                                             */ \
	  /*                                                         */ \
	  /***********************************************************/ \
	                                                                \
	  total_src_width = bytes_per_row + byte_stride;                \
	  words_to_xfer   = ((total_src_width * src_height) + 3) >> 2;  \
	}                                                               \
	break;                                                          \
	                                                                \
      case 8:                                                           \
	                                                                \
	/*************************************************************/ \
	/*                                                           */ \
	/* Each pixel is a byte.  Determine how many stride bytes    */ \
	/* are needed to cause each row of data that will be         */ \
	/* transferred to be a multiple of words in length.          */ \
	/*                                                           */ \
	/*************************************************************/ \
	                                                                \
	if ((src_width & 3) == 0)                                       \
	  stride_needed = 0;                                            \
	else                                                            \
	  stride_needed = 4 - (src_width & 3);                          \
	                                                                \
	if (byte_stride >= stride_needed)                               \
	{                                                               \
	  /***********************************************************/ \
	  /*                                                         */ \
	  /* Enough stride bytes are present.  If PIO is used, the   */ \
	  /* pixel data will be transferred to the adapter with the  */ \
	  /* minimum number of stride bytes at the end of each row   */ \
	  /* to cause each row to be a multiple of words in length.  */ \
	  /*                                                         */ \
	  /***********************************************************/ \
	                                                                \
	  total_src_width = src_width + stride_needed;                  \
	  words_to_xfer   = (total_src_width * src_height) >> 2;        \
	}                                                               \
	else                                                            \
	{                                                               \
	  /***********************************************************/ \
	  /*                                                         */ \
	  /* Not enough stride bytes are present.  If PIO is used,   */ \
	  /* all of the pixel data and the stride bytes present will */ \
	  /* be transferred to the adapter at once.  An additional   */ \
	  /* 0 - 3 bytes will be included in the transfer because    */ \
	  /* the total amount of data transferred must be a multiple */ \
	  /* of 4 bytes.                                             */ \
	  /*                                                         */ \
	  /***********************************************************/ \
	                                                                \
	  total_src_width = src_width + byte_stride;                    \
	  words_to_xfer   = ((total_src_width * src_height) + 3) >> 2;  \
	}                                                               \
	break;                                                          \
	                                                                \
      case 32:                                                          \
	                                                                \
	/*************************************************************/ \
	/*                                                           */ \
	/* Each pixel is a word.  Therefore,  if PIO is used no      */ \
	/* stride bytes will be transferred to the adapter.          */ \
	/*                                                           */ \
	/*************************************************************/ \
	                                                                \
	stride_needed   = 0;                                            \
	total_src_width = src_width << 2;                               \
	words_to_xfer   = src_width * src_height;                       \
	                                                                \
	break;                                                          \
    } /* end switch */                                                  \
	                                                                \
    /*****************************************************************/ \
    /*                                                               */ \
    /* Determine which data transfer method to use (PIO or DMA)      */ \
    /* based on the amount of data there is to transfer.  Note that  */ \
    /* DMA will be used if a complete row of source data, including  */ \
    /* the minimum number of stride bytes needed to make the row a   */ \
    /* multiple of words in length, will not fit into a Write Pixel  */ \
    /* Data command element.  DMA will also be used if all of the    */ \
    /* source data will not fit into a Write Pixel Data command      */ \
    /* element and modelling coordinates are specified.              */ \
    /*                                                               */ \
    /*****************************************************************/ \
	                                                                \
    if (words_to_xfer <                                                 \
	(MID_WPD_DMA_THRESH - ((sizeof(MIDFIFWritePixelData) + 3) >> 2)) && \
	total_src_width <= (MID_MAX_PERF_WPD_WORDS << 2) &&             \
	(words_to_xfer <= MID_MAX_PERF_WPD_WORDS ||                     \
	 !(cmdflags & MID_WPD_MODELLING_COORDS)))                       \
    {                                                                   \
      /***************************************************************/ \
      /*                                                             */ \
      /* Use PIO.                                                    */ \
      /*                                                             */ \
      /***************************************************************/ \
	                                                                \
      unsigned char             *curr_row_ptr = (unsigned char *) data_ptr; \
      unsigned int              total_bytes_to_write, i;                \
      unsigned int              one_row_at_a_time = 0;                  \
      unsigned int              xfer_last_word    = 0;                  \
      MIDFIFWritePixelData      SE;                                     \
	                                                                \
      /***************************************************************/ \
      /*                                                             */ \
      /* Build the portion of the fixed part of the Pedernales Write */ \
      /* Pixel Data command element that will be the same in all of  */ \
      /* the command elements built to complete this write request.  */ \
      /*                                                             */ \
      /***************************************************************/ \
	                                                                \
      SE.hdr.halfword.op = OP_WRITE_PIXEL_DATA ;                        \
      SE.corr    = 0;           /* Ignored when PIO is used          */ \
      SE.bflags  = cmdflags | MID_WRITE_PIO;                            \
      SE.haddr   = 0;           /* The first byte of data is valid   */ \
      SE.resv    = 0;                                                   \
      SE.destbp  = dest;                                                \
      SE.dx      = dest_x;                                              \
      SE.dy      = dest_y;      /* Note:  this field will not be the    \
	                                  same in all cmd elements   */ \
      SE.dw      = src_width;                                           \
      SE.mcx     = mod_x;                                               \
      SE.mcy     = mod_y;                                               \
      SE.mcz     = mod_z;                                               \
      SE.fbit    = first_bit;                                           \
      SE.xrepl   = x_rep;                                               \
      SE.yrepl   = y_rep;                                               \
      SE.fgcolor = fg_color;                                            \
      SE.bgcolor = bg_color;                                            \
	                                                                \
      MID_TRANSLATE_LOG_OP(bg_logic_op, SE.bg_op);                      \
      MID_TRANSLATE_LOG_OP(fg_logic_op, SE.fg_op);                      \
	                                                                \
      /***************************************************************/ \
      /*                                                             */ \
      /* Determine how the data will be transferred to the adapter.  */ \
      /* Data may be transferred all at once, a row at a time, with  */ \
      /* or without all of the stride bytes present, etc.            */ \
      /*                                                             */ \
      /***************************************************************/ \
	                                                                \
      if (byte_stride == stride_needed)                                 \
      {                                                                 \
	/*************************************************************/ \
	/*                                                           */ \
	/* The right number of stride bytes are present to cause     */ \
	/* each row of data that will be transferred to the adapter  */ \
	/* to be a multiple of words in length.  This is the case    */ \
	/* that will have the best performance.  The pixel data and  */ \
	/* the stride bytes will be transferred all at once.         */ \
	/*                                                           */ \
	/*************************************************************/ \
	                                                                \
	SE.hstride = byte_stride;                                       \
      }                                                                 \
      else if (byte_stride > stride_needed)                             \
      {                                                                 \
	/*************************************************************/ \
	/*                                                           */ \
	/* Too many stride bytes are present.  Indicate that the     */ \
	/* pixel data should be transferred to the adapter a row at  */ \
	/* a time so that only the stride bytes needed will be       */ \
	/* transferred.                                              */ \
	/*                                                           */ \
	/*************************************************************/ \
	                                                                \
	one_row_at_a_time = 1;                                          \
	SE.hstride        = stride_needed;                              \
      }                                                                 \
      else              /* byte_stride < stride_needed               */ \
      {                                                                 \
	/*************************************************************/ \
	/*                                                           */ \
	/* Not enough stride bytes are present.  The pixel data and  */ \
	/* all of the stride bytes that are present will be          */ \
	/* transferred to the adapter.                               */ \
	/*                                                           */ \
	/*************************************************************/ \
	                                                                \
	SE.hstride = byte_stride;                                       \
	                                                                \
      }                 /* End not enough stride bytes present       */ \
	                                                                \
      /***************************************************************/ \
      /*                                                             */ \
      /* Determine the maximum number of rows of source data that    */ \
      /* will fit into one Write Pixel Data command element.         */ \
      /*                                                             */ \
      /***************************************************************/ \
	                                                                \
      max_rows = (MID_MAX_PERF_WPD_WORDS << 2) / total_src_width;       \
	                                                                \
      /***************************************************************/ \
      /*                                                             */ \
      /* Now issue as many Write Pixel Data command elements to the  */ \
      /* FIFO as necessary to write all of the data.                 */ \
      /*                                                             */ \
      /***************************************************************/ \
	                                                                \
      while (rem_rows != 0)                                             \
      {                                                                 \
	if (rem_rows <= max_rows)                                       \
	{                                                               \
	  SE.dh         = rem_rows;                                     \
	  words_to_xfer = ((total_src_width * rem_rows) + 3) >> 2;      \
	  if (byte_stride < stride_needed)                              \
	  {                                                             \
	    /*********************************************************/ \
	    /*                                                       */ \
	    /* Determine if the total amount of data to be           */ \
	    /* transferred to the adapter this time is a multiple of */ \
	    /* words.                                                */ \
	    /*                                                       */ \
	    /*********************************************************/ \
	                                                                \
	    total_bytes_to_write = total_src_width * rem_rows;          \
	                                                                \
	    if ((total_bytes_to_write & 3) != 0)                        \
	    {                                                           \
	      /*******************************************************/ \
	      /*                                                     */ \
	      /* The total amount of data to be transferred is not a */ \
	      /* multiple of words.  Therefore, care must be taken   */ \
	      /* when the end of the data is reached so that data    */ \
	      /* beyond the end is not accessed.  So, the data will  */ \
	      /* be transferred to the adapter in 2 pieces.  The     */ \
	      /* first piece will consist of all of the data except  */ \
	      /* the last partial word.  The second piece will       */ \
	      /* consist of the last partial word padded to be a     */ \
	      /* full word.                                          */ \
	      /*                                                     */ \
	      /*******************************************************/ \
	                                                                \
	      xfer_last_word = 1;                                       \
	      words_to_xfer--;                                          \
	    }                                                           \
	  }                                                             \
	}                                                               \
	else                                                            \
	{                                                               \
	  SE.dh         = max_rows;                                     \
	  words_to_xfer = ((total_src_width * max_rows) + 3) >> 2;      \
	}                                                               \
	                                                                \
	SE.hdr.halfword.length = sizeof(MIDFIFWritePixelData) +         \
	                         (words_to_xfer << 2);                  \
	if(xfer_last_word)                                              \
	        SE.hdr.halfword.length += 4;                            \
	                                                                \
	/*************************************************************/ \
	/*                                                           */ \
	/* Write the fixed portion of the Write Pixel Data command   */ \
	/* element to the FIFO.                                      */ \
	/*                                                           */ \
	/*************************************************************/ \
	                                                                \
	MID_WRITE_TO_FIFO(&SE, MID_WORDS(sizeof(MIDFIFWritePixelData) >> 2)); \
	                                                                \
	if (!one_row_at_a_time)                                         \
	{                                                               \
	  /***********************************************************/ \
	  /*                                                         */ \
	  /* Now write all of the pixel data and the stride bytes    */ \
	  /* that are present to the FIFO.                           */ \
	  /*                                                         */ \
	  /***********************************************************/ \
	                                                                \
	  MID_WRITE_TO_FIFO(curr_row_ptr, MID_WORDS(words_to_xfer));    \
	                                                                \
	  curr_row_ptr += total_src_width * SE.dh;                      \
	                                                                \
	  if (xfer_last_word)                                           \
	  {                                                             \
	    /*********************************************************/ \
	    /*                                                       */ \
	    /* The total amount of pixel data and stride bytes to be */ \
	    /* transferred was not a multiple of words.  Therefore,  */ \
	    /* the last partial word of data was not written to the  */ \
	    /* FIFO.  So, build the last word of data to be written  */ \
	    /* to the FIFO by padding the last partial word to make  */ \
	    /* it a full word, then write that word to the FIFO.     */ \
	    /*                                                       */ \
	    /*********************************************************/ \
	                                                                \
	    unsigned char       last_word[4];                           \
	    unsigned int        bytes_left;                             \
	                                                                \
	    bytes_left    = total_bytes_to_write & 3;                   \
	    curr_row_ptr -= bytes_left;                                 \
	                                                                \
	    for (i = 0; i < bytes_left; i++)                            \
	    {                                                           \
	      last_word[i] = *curr_row_ptr;                             \
	      curr_row_ptr++;                                           \
	    }                                                           \
	                                                                \
	    MID_WRITE_TO_FIFO(last_word, MID_WORDS(1));                 \
	  }                                                             \
	}                                                               \
	else                                                            \
	{                                                               \
	  /***********************************************************/ \
	  /*                                                         */ \
	  /* There are more stride bytes present than are needed to  */ \
	  /* cause each row of data to be transferred to the adapter */ \
	  /* to be a multiple of words in length.  So write the      */ \
	  /* pixel data to the adapter a row at a time so that only  */ \
	  /* the minimum number of stride bytes necessary to cause   */ \
	  /* each row to be a multiple of words are written.         */ \
	  /*                                                         */ \
	  /***********************************************************/ \
	                                                                \
	  for (i = 1; i <= SE.dh; i++)                                  \
	  {                                                             \
	    MID_WRITE_TO_FIFO(curr_row_ptr, MID_WORDS(total_src_width >> 2)); \
	    curr_row_ptr += total_src_width + byte_stride - stride_needed;   \
	  }                                                             \
	}                                                               \
	                                                                \
	rem_rows -= SE.dh;                                              \
	SE.dy    += SE.dh * y_rep;                                      \
	                                                                \
      }                 /* End loop                                  */ \
    }                   /* End use PIO                               */ \
    else                                                                \
    {                                                                   \
      /***************************************************************/ \
      /*                                                             */ \
      /* Use DMA.                                                    */ \
      /*                                                             */ \
      /***************************************************************/ \
	                                                                \
      int               buf_length;     /* Length of the source data */ \
      int               rc;             /* Returned by aixgsc        */ \
      gscdma            dma_write_struct; /* Device independent DMA     \
	                                   data structure            */ \
      mid_dma_t         mid_dma_struct; /* Device dependent DMA data    \
	                                   structure for Pedernales  */ \
      mid_dma_write_t   mid_dma_write_SE; /* Write Pixel Data command   \
	                                   element structure         */ \
	                                                                \
      /***************************************************************/ \
      /*                                                             */ \
      /* If there is data in the deferral buffer, flush it now.      */ \
      /*                                                             */ \
      /***************************************************************/ \
	                                                                \
      if (MID_DEFERBUF_LEN != 0)                                        \
	MID_DEFERBUF_FLUSH;                                             \
	                                                                \
      /***************************************************************/ \
      /*                                                             */ \
      /* Compute the amount of source data present.  All stride      */ \
      /* bytes must be included in this computation since the        */ \
      /* computed value is used to indicate how much memory to pin   */ \
      /* for the DMA operation.                                      */ \
      /*                                                             */ \
      /***************************************************************/ \
	                                                                \
      switch (bits_per_pixel)                                           \
      {                                                                 \
	case 1:                                                         \
	                                                                \
	  total_src_width = bytes_per_row + byte_stride;                \
	  break;                                                        \
	                                                                \
	case 8:                                                         \
	                                                                \
	  total_src_width = src_width + byte_stride;                    \
	  break;                                                        \
	                                                                \
	case 32:                                                        \
	                                                                \
	  total_src_width = (src_width << 2) + byte_stride;             \
	  break;                                                        \
	                                                                \
      } /* end switch */                                                \
	                                                                \
      buf_length = total_src_width * src_height;                        \
	                                                                \
      /***************************************************************/ \
      /*                                                             */ \
      /* Set up the portion of the device independent DMA data       */ \
      /* structure that will not change regardless of how many DMA   */ \
      /* operations it will take to complete this write request.     */ \
      /*                                                             */ \
      /***************************************************************/ \
	                                                                \
      dma_write_struct.flags      = DMA_WAIT | DMA_WRITE; /* Do a DMA   \
	                                   write operation and wait     \
	                                   until it is complete      */ \
      dma_write_struct.dma_cmd    = &mid_dma_struct;                    \
      dma_write_struct.cmd_length = sizeof(mid_dma_t);                  \
      dma_write_struct.num_sw     = 1;  /* There is 1 buffer to write   \
	                                   from                      */ \
      dma_write_struct.subwindow[0].sw_flag = 0; /* Nothing weird    */ \
	                                                                \
      /***************************************************************/ \
      /*                                                             */ \
      /* Set up the device dependent DMA structure for Pedernales.   */ \
      /*                                                             */ \
      /***************************************************************/ \
	                                                                \
      mid_dma_struct.flags   = MID_DMA_WRITE; /* Do a DMA write         \
	                                   operation by issuing the Ped \
	                                   Write Pixel Data command     \
	                                   element to the FIFO       */ \
      mid_dma_struct.se_size = sizeof(mid_dma_write_t) >> 2;            \
      mid_dma_struct.se_data = &mid_dma_write_SE;                       \
	                                                                \
      /***************************************************************/ \
      /*                                                             */ \
      /* Set up the portion of the Pedernales Write Pixel Data       */ \
      /* command element that will not change regardless of how many */ \
      /* DMA operations it will take to complete this write request. */ \
      /*                                                             */ \
      /***************************************************************/ \
	                                                                \
      mid_dma_write_SE.length   = sizeof(mid_dma_write_t);              \
      mid_dma_write_SE.opcode   = OP_WRITE_PIXEL_DATA ;                 \
      mid_dma_write_SE.corr     = 0;    /* Correlator - used by DMA     \
	                                   services                  */ \
      mid_dma_write_SE.bflags   = cmdflags & 0xF7FF;                    \
      mid_dma_write_SE.stride   = byte_stride;                          \
      mid_dma_write_SE.destinbp = dest;                                 \
      mid_dma_write_SE.destinx  = dest_x;                               \
      mid_dma_write_SE.width    = src_width;                            \
      mid_dma_write_SE.mcx      = mod_x;                                \
      mid_dma_write_SE.mcy      = mod_y;                                \
      mid_dma_write_SE.mcz      = mod_z;                                \
      mid_dma_write_SE.dbpos    = first_bit;                            \
      mid_dma_write_SE.xrepl    = x_rep;                                \
      mid_dma_write_SE.yrepl    = y_rep;                                \
      mid_dma_write_SE.fgcolor  = fg_color;                             \
      mid_dma_write_SE.bgcolor  = bg_color;                             \
	                                                                \
      MID_TRANSLATE_LOG_OP(bg_logic_op, mid_dma_write_SE.bgop);         \
      MID_TRANSLATE_LOG_OP(fg_logic_op, mid_dma_write_SE.fgop);         \
	                                                                \
      if (buf_length <= MAX_SINGLE_DMA)                                 \
      {                                                                 \
	/*************************************************************/ \
	/*                                                           */ \
	/* The amount of data to be written can be transferred in    */ \
	/* one DMA operation.  Complete the set up of the DMA        */ \
	/* structures.                                               */ \
	/*                                                           */ \
	/*************************************************************/ \
	                                                                \
	dma_write_struct.subwindow[0].sw_addr   = data_ptr; /* Starting \
	                                   address of where to get the  \
	                                   data to write             */ \
	dma_write_struct.subwindow[0].sw_length = buf_length;           \
	                                                                \
	mid_dma_write_SE.hostaddr = data_ptr;                           \
	mid_dma_write_SE.destiny  = dest_y;                             \
	mid_dma_write_SE.height   = src_height;                         \
	                                                                \
	/*************************************************************/ \
	/*                                                           */ \
	/* Do the DMA.                                               */ \
	/*                                                           */ \
	/*************************************************************/ \
	                                                                \
	rc = aixgsc(gsc_handle, DMA_SERVICE, &dma_write_struct);        \
	                                                                \
	if (rc &&                                                       \
	    (errno != ENOMEM || (cmdflags & MID_WPD_MODELLING_COORDS))) \
	{                                                               \
	/* XXX TRACE ??? */                                             \
	}               /* End bad return code from aixgsc           */ \
      }                 /* End data can be transferred with one DMA  */ \
	                                                                \
      if ((buf_length > MAX_SINGLE_DMA ||                               \
	   (rc && (errno == ENOMEM))) &&                                \
	  !(cmdflags & MID_WPD_MODELLING_COORDS))                       \
      {                                                                 \
	/*************************************************************/ \
	/*                                                           */ \
	/* The data to be written could not all be transferred in    */ \
	/* one DMA operation because either there is too much data,  */ \
	/* or the memory containing the data could not be pinned.    */ \
	/*                                                           */ \
	/*************************************************************/ \
	                                                                \
	int     failures = 0;           /* Number of times a DMA        \
	                                   operation has failed      */ \
	                                                                \
	/*************************************************************/ \
	/*                                                           */ \
	/* Determine the maximum number of rows of data to write in  */ \
	/* each of the following DMA operations.                     */ \
	/*                                                           */ \
	/*************************************************************/ \
	                                                                \
	if (buf_length > MAX_SINGLE_DMA)                                \
	  max_rows = MAX_SINGLE_DMA / total_src_width;                  \
	else                                                            \
	{                                                               \
	  /***********************************************************/ \
	  /*                                                         */ \
	  /* An attempt was made above to write all of the source    */ \
	  /* data at once, but the attempt failed because the data   */ \
	  /* could not be pinned in memory.  So, break the data in   */ \
	  /* half and try again.                                     */ \
	  /*                                                         */ \
	  /***********************************************************/ \
	                                                                \
	  max_rows = src_height >> 1;                                   \
	  failures++;                                                   \
	}                                                               \
	                                                                \
	dma_write_struct.subwindow[0].sw_addr = data_ptr; /* Starting   \
	                                   address of where to get the  \
	                                   data to write             */ \
	mid_dma_write_SE.hostaddr = data_ptr;                           \
	mid_dma_write_SE.destiny  = dest_y;                             \
	                                                                \
	/*************************************************************/ \
	/*                                                           */ \
	/* Break up the source data into smaller chunks and issue    */ \
	/* several DMA operations to write it.                       */ \
	/*                                                           */ \
	/*************************************************************/ \
	                                                                \
	while (rem_rows != 0)                                           \
	{                                                               \
	  /***********************************************************/ \
	  /*                                                         */ \
	  /* Determine how many rows of data to write with the next  */ \
	  /* DMA operation.                                          */ \
	  /*                                                         */ \
	  /***********************************************************/ \
	                                                                \
	  if (rem_rows > max_rows)                                      \
	    mid_dma_write_SE.height = max_rows;                         \
	  else                                                          \
	    mid_dma_write_SE.height = rem_rows;                         \
	                                                                \
	  /***********************************************************/ \
	  /*                                                         */ \
	  /* Determine how much memory to pin.                       */ \
	  /*                                                         */ \
	  /***********************************************************/ \
	                                                                \
	  dma_write_struct.subwindow[0].sw_length =                     \
	            total_src_width * mid_dma_write_SE.height;          \
	                                                                \
	  /***********************************************************/ \
	  /*                                                         */ \
	  /* Do the DMA.                                             */ \
	  /*                                                         */ \
	  /***********************************************************/ \
	                                                                \
	  rc = aixgsc(gsc_handle, DMA_SERVICE, &dma_write_struct);      \
	                                                                \
	  if (!rc)                                                      \
	  {                                                             \
	    /*********************************************************/ \
	    /*                                                       */ \
	    /* The DMA operation was successful.                     */ \
	    /*                                                       */ \
	    /*********************************************************/ \
	                                                                \
	    rem_rows                 -= mid_dma_write_SE.height;        \
	    mid_dma_write_SE.destiny += mid_dma_write_SE.height * y_rep;\
	    mid_dma_write_SE.hostaddr =                                 \
	              ((unsigned char *) (mid_dma_write_SE.hostaddr)) + \
	              dma_write_struct.subwindow[0].sw_length;          \
	    dma_write_struct.subwindow[0].sw_addr =                     \
	              mid_dma_write_SE.hostaddr;                        \
	  }                                                             \
	  else                                                          \
	  {                                                             \
	    /*********************************************************/ \
	    /*                                                       */ \
	    /* The DMA operation failed.                             */ \
	    /*                                                       */ \
	    /*********************************************************/ \
	                                                                \
	    if (errno == ENOMEM)                                        \
	    {                                                           \
	      /*******************************************************/ \
	      /*                                                     */ \
	      /* The DMA operation failed because not enough memory  */ \
	      /* could be pinned.  So, break the data in half again  */ \
	      /* and try again if the retry limit has not been       */ \
	      /* reached.                                            */ \
	      /*                                                     */ \
	      /*******************************************************/ \
	                                                                \
	      max_rows >>= 1;                                           \
	                                                                \
	      failures++;                                               \
	      if (failures == MID_MAX_DMA_FAILURES || max_rows == 0)    \
	      {                                                         \
	        rem_rows = 0;             /* Set to exit loop        */ \
	        /* XXX TRACE ??? */                                     \
	      }                                                         \
	    }                                                           \
	    else                                                        \
	    {                                                           \
	      /*******************************************************/ \
	      /*                                                     */ \
	      /* The error is not recoverable.  Set rem_rows to 0 to */ \
	      /* exit the loop.                                      */ \
	      /*                                                     */ \
	      /*******************************************************/ \
	                                                                \
	      rem_rows = 0;                                             \
	      /* XXX TRACE ??? */                                       \
	    }                                                           \
	  }             /* End DMA operation failed                  */ \
	}               /* End loop                                  */ \
      }                                                                 \
    }                   /* End use DMA                               */ \
}                       /* End MID_FIFWritePixelData                 */


/* ---------------------------------------------------------------------*
	The following SEs are out of date.
 * ---------------------------------------------------------------------*/

#if 0

/*
 * PEDERNALES SetColorProcessingMode command packet structure
 */
typedef struct _MIDSetColorProcessingMode
{
	MIDfifohdr              hdr;    /* SE header info (len/opcode) */
	ulong                   cpm;
	ulong                   Rbl;
	ulong                   Gbl;
	ulong                   Bbl;
	ulong                   pad;
}       MIDSetColorProcessingMode,
	*pMIDSetColorProcessingMode;

#define MID_SetColorProcessingMode( cpm_param , Rbl_param , Gbl_param , \
	                                Bbl_param , pad_param )         \
{                                                                       \
    MIDSetColorProcessingMode   SE;                                     \
	                                                                \
    SE.hdr.halfword.length = ( ushort )                                 \
	                        sizeof( MIDSetColorProcessingMode );    \
    SE.hdr.halfword.op     = ( ushort ) OP_SET_2D_COLOR_MODE ;          \
    SE.cpm                 = ( ulong )  cpm_param;                      \
    SE.Rbl                 = ( ulong )  Rbl_param;                      \
    SE.Gbl                 = ( ulong )  Gbl_param;                      \
    SE.Bbl                 = ( ulong )  Bbl_param;                      \
    SE.pad                 = ( ulong )  pad_param;                      \
	                                                                \
    MID_WR_DEFERBUF( &SE ,                                              \
	                ( sizeof( MIDSetColorProcessingMode ) >> 2 ) ); \
	                                                                \
}



#define MID_SetColorProcessingMode_NoDefer( cpm_param , Rbl_param ,     \
	                                        Gbl_param , Bbl_param , \
	                                        pad_param )             \
{                                                                       \
    MIDSetColorProcessingMode   SE;                                     \
	                                                                \
    SE.hdr.halfword.length = ( ushort )                                 \
	                        sizeof( MIDSetColorProcessingMode );    \
    SE.hdr.halfword.op     = ( ushort ) OP_DELETED  ;                   \
    SE.cpm                 = ( ulong )  cpm_param;                      \
    SE.Rbl                 = ( ulong )  Rbl_param;                      \
    SE.Gbl                 = ( ulong )  Gbl_param;                      \
    SE.Bbl                 = ( ulong )  Bbl_param;                      \
    SE.pad                 = ( ulong )  pad_param;                      \
	                                                                \
    MID_WR_FIFO( &SE ,                                                  \
	                ( sizeof( MIDSetColorProcessingMode ) >> 2 ) ); \
	                                                                \
}



/*
 * PEDERNALES SetRGBTransform command packet structure
 */
typedef struct _MIDSetRGBTransform
{
	MIDfifohdr              hdr;    /* SE header info (len/opcode) */
	long                    rrange;
	long                    grange;
	long                    brange;
	long                    roffset;
	long                    goffset;
	long                    boffset;
}       MIDSetRGBTransform,
	*pMIDSetRGBTransform;


#define MID_SetRGBTransform( rrange_param , grange_param ,              \
	                        brange_param , roffset_param ,          \
	                        goffset_param , boffset_param )         \
{                                                                       \
    MIDSetRGBTransform  SE;                                             \
	                                                                \
    SE.hdr.halfword.length = ( ushort ) sizeof( MIDSetRCBTransform );   \
    SE.hdr.halfword.op     = ( ushort ) OP_SET_RGB_TRANSFORM;           \
    SE.rrange              = ( long )   rrange_param;                   \
    SE.grange              = ( long )   grange_param;                   \
    SE.brange              = ( long )   brange_param;                   \
    SE.roffset             = ( long )   roffset_param;                  \
    SE.goffset             = ( long )   goffset_param;                  \
    SE.boffset             = ( long )   boffset_param;                  \
	                                                                \
    MID_WR_DEFERBUF( &SE , ( sizeof( MIDSetRCBTransform ) >> 2 ) );     \
	                                                                \
}


#endif


#endif  /* _H_MID_HW_FIFO_RMS */


