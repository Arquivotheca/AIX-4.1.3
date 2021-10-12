/* @(#)25	1.6.1.1  src/bos/kernext/disp/ped/pedmacro/hw_typdefs.h, pedmacro, bos411, 9428A410j 3/17/93 20:07:34 */
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
/*	PEDERNALES HW MACRO PROGRAMMING INTERFACE		*/
/*								*/
/****************************************************************/


#ifndef _H_MID_HW_TYPEDEFS
#define _H_MID_HW_TYPEDEFS



/*************************************************************************
**************************************************************************
**									**
**	Definitions of basic FIFO string store structures		**
**	   ( useful for PCB operations as well )			**
**									**
**************************************************************************
**************************************************************************/




typedef volatile struct _MID_1param
{
	ulong		a;
} MID_1param,
  *pMID_1param;

typedef volatile struct _MID_2param
{
	ulong		a[2];
} MID_2param,
  *pMID_2param;

typedef volatile struct _MID_3param
{
	ulong		a[3];
} MID_3param,
  *pMID_3param;

typedef volatile struct _MID_4param
{
	ulong		a[4];
} MID_4param,
  *pMID_4param;

typedef volatile struct _MID_5param
{
	ulong		a[5];
} MID_5param,
  *pMID_5param;

typedef volatile struct _MID_6param
{
	ulong		a[6];
} MID_6param,
  *pMID_6param;

typedef volatile struct _MID_7param
{
	ulong		a[7];
} MID_7param,
  *pMID_7param;

typedef volatile struct _MID_8param
{
	ulong		a[8];
} MID_8param,
  *pMID_8param;

typedef volatile struct _MID_9param
{
	ulong		a[9];
} MID_9param,
  *pMID_9param;

typedef volatile struct _MID_10param
{
	ulong		a[10];
} MID_10param,
  *pMID_10param;

typedef volatile struct _MID_11param
{
	ulong		a[11];
} MID_11param,
  *pMID_11param;

typedef volatile struct _MID_12param
{
	ulong		a[12];
} MID_12param,
  *pMID_12param;

typedef volatile struct _MID_13param
{
	ulong		a[13];
} MID_13param,
  *pMID_13param;

typedef volatile struct _MID_14param
{
	ulong		a[14];
} MID_14param,
  *pMID_14param;

typedef volatile struct _MID_15param
{
	ulong		a[15];
} MID_15param,
  *pMID_15param;

typedef volatile struct _MID_16param
{
	ulong		a[16];
} MID_16param,
  *pMID_16param;

typedef union _MID_1600param
{
	MID_8param	data8[2000];
	MID_16param	data16[1000];
	ulong		a[16000];
} MID_16000param,
  *pMID_16000param;






/*************************************************************************
**************************************************************************
**									**
**	Definitions which aid in building FIFO command elements 	**
**									**
**************************************************************************
**************************************************************************/



/*
 *------------------------------------------------
 * Data types used in 2D environment
 *------------------------------------------------
 */

/*
 * Pedernales integer point structure
 */
typedef struct _MIDPoint
{
	short		x;		/* x coordinate */
	short		y;		/* y coordinate */
} MIDPoint, *pMIDPoint;


typedef struct _MIDRect
{
	short		ulx;		/* x coordinate (upper left) */
	short		uly;		/* y coordinate (upper left) */
	ushort		pw;		/* width in pixels */
	ushort		ph;		/* height in pixels */
} MIDRect, *pMIDRect;


typedef struct _MIDArc
{
	MIDRect 	rect;		/* enclosing rectangle */
	short		a1;		/* start angle 1/64 of degree */
	short		a2;		/* end angle 1/64 of degree */
} MIDarc, *pMIDArc;


typedef struct _MIDBox
{
	short		ulx;		/* x coordinate (upper left) */
	short		uly;		/* y coordinate (upper left) */
	short		lrx;		/* x coordinate (lower right) */
	short		lry;		/* y coordinate (lower right) */
} MIDBox, *pMIDBox;


/*
 *------------------------------------------------------------
 * Data types used in 3DM1 environment (some applies to 3DM1M)
 *------------------------------------------------------------
 */

typedef struct _MIDPoint2
{
	float		x;		/* x coordinate */
	float		y;		/* y coordinate */
} MIDPoint2, *pMIDPoint2,
  MIDVector2, *pMIDVector2,
  MIDCtrlPoint2, *pMIDCtrlPoint2;


typedef struct _MIDPoint3
{
	float		x;		/* x coordinate */
	float		y;		/* y coordinate */
	float		z;		/* y coordinate */
} MIDPoint3, *pMIDPoint3,
  MIDVector3, *pMIDVector3,
  MIDCtrlPoint3, *pMIDCtrlPoint3;


typedef struct _MIDWCtrlPoint2
{
	float		x;		/* x coordinate */
	float		y;		/* y coordinate */
	float		w;		/* weight of point */
} MIDWCtrlPoint2, *pMIDWCtrlPoint2;


typedef struct _MIDWCtrlPoint3
{
	float		x;		/* x coordinate */
	float		y;		/* y coordinate */
	float		z;		/* y coordinate */
	float		w;		/* weight of point */
} MIDWCtrlPoint3, *pMIDWCtrlPoint3,
  MIDVector4, *pMIDVector4,		/* for 3DM1M */
  MIDPosition, *pMIDPosition;		/* for 3DM1M */

typedef struct _MIDWCtrlPoint4
{
	int		x;		/* x coordinate */
	int		y;		/* y coordinate */
	int		z;		/* y coordinate */
	int		w;		/* weight of point */
}
  MIDVector4Int, *pMIDVector4Int,	/* for 3DM1M */
  MIDPositionInt, *pMIDPositionInt;	/* for 3DM1M */

typedef struct _MIDColor24
{
	char		reserved;
	char		red;		/* red value */
	char		green;		/* green value */
	char		blue;		/* blue value */
} MIDColor24, *pMIDColor24;

typedef struct _MIDColorIndex
{
	int		index;		/* color index */
} MIDColorIndex, *pMIDColorIndex;


typedef struct _MIDColorDirect
{
	float		r;		/* red component */
	float		g;		/* green component */
	float		b;		/* blue component */
} MIDColorDirect, *pMIDColorDirect,
  MIDColor3F, *pMIDColor3F;		/* for 3DM1M */


typedef struct _MIDRSLColorIndex
{
	float		type;		/* always 1 for indexed color */
	float		index;		/* red component */
	float		ign1;		/* ignored value */
	float		ign2;		/* ignored value */
} MIDRSLColorIndex, *pMIDRSLColorIndex;


typedef struct _MIDRSLColorDirect
{
	float		type;		/* always 2 for direct color */
	float		r;		/* red component */
	float		g;		/* green component */
	float		b;		/* blue component */
} MIDRSLColorDirect, *pMIDRSLColorDirect;


typedef struct _MIDVolume
{
	float		xmin;		/* x minimum */
	float		xmax;		/* x maximum */
	float		ymin;		/* y minimum */
	float		ymax;		/* y maximum */
	float		zmin;		/* z minimum */
	float		zmax;		/* z maximum */
} MIDVolume, *pMIDVolume;

typedef struct _MIDIpoint2
{
	MIDPoint2	point;
	float		knot;
} MIDIpoint2, *pMIDIpoint2;

typedef struct _MIDSpan2
{
	MIDPoint2	point;
	float		tessellation;
	float		knot;
} MIDSpan2, *pMIDSpan2;

typedef struct _MIDIpoint3
{
	MIDPoint3	point;
	float		knot;
} MIDIpoint3, *pMIDIpoint3;

typedef struct _MIDSpan3
{
	MIDPoint3	point;
	float		tessellation;
	float		knot;
} MIDSpan3, *pMIDSpan3;

typedef struct _MIDEdge
{
	MIDVector3	vect1;
	MIDVector3	vect2;
	MIDPoint3	point1;
	MIDPoint3	point2;
} MIDEdge, *pMIDEdge;

typedef struct _MIDPolylineAttr
{
	ulong			ltype;
	float			lwscale;
	MIDRSLColorIndex	lcolor;
} MIDPolylineAttr, *pMIDPolylineAttr;

typedef struct _MIDPolymarkerAttr
{
	float			mscale;
	MIDRSLColorIndex	mcolor;
} MIDPolymarkerAttr, *pMIDPolymarkerAttr;

typedef struct _MIDTextAttr
{
	ulong			tprec;
	float			charspac;
	MIDRSLColorIndex	tcolor;
} MIDTextAttr, *pMIDTextAttr;

typedef struct _MIDInteriorAttr
{
	ulong			istyle;
	MIDRSLColorIndex	icolor;
} MIDInteriorAttr, *pMIDInteriorAttr;

typedef struct _MIDEdgeAttr
{
	ulong			eflag;
	ulong			etype;
	float			escale;
	MIDRSLColorIndex	ecolor;
} MIDEdgeAttr, *pMIDEdgeAttr;


/*
 *------------------------------------------------
 * Data types used in 3DM1M environment
 *------------------------------------------------
 */


typedef struct _MIDMaterial
{
	struct _MIDMaterial	*next;
	ulong			name;
	MIDColor3F		emission;
	MIDColor3F		ambient;
	MIDColor3F		diffuse;
	MIDColor3F		specular;
	float			exponent;
} MIDMaterial, *pMIDMaterial;

typedef struct _MIDLight
{
	struct	_MIDLight	*next;
	ulong			name;
	ushort			lflag;
	ushort			lid;
	MIDColor3F		ambient;
	MIDColor3F		lcolor;
	MIDPosition		lposition;
	MIDVector3		spotdir;
	float			spotexp;
	float			spotspread;
} MIDLight, *pMIDLight;

typedef struct _MIDLightModel
{
	struct	_MIDLightModel	*next;
	ulong			name;
	MIDColor3F		ambient;
	ulong			viewer;
	float			fix_attenu;
	float			var_attenu;

} MIDLightModel, *pMIDLightModel;

typedef union _MIDLightsListHdr
{
	ulong	word;
	struct	_halfhdr
	{
		ushort numli;
		ushort mflag;
	} halfhdr;
} MIDLightsListHdr;

typedef struct _MIDLightsList
{
	MIDLightsListHdr	header;
	pMIDLight		plight;
} MIDLightsList, *pMIDLightsList;

typedef struct _MIDVertexM2
{
	char bitflag[4];
	MIDVector3		normal; 	/* */
	MIDColor3F		color;
	MIDMaterial		material;
	MIDLightsList		lights;
	MIDLightModel		lightmodel;
	MIDPosition		position;
} MIDVertexM2, *pMIDVertexM2;


/*
 *------------------------------------------------
 * Data types used in RMS environment
 *------------------------------------------------
 */

typedef struct _MID_map_duration_pair
{
	ushort	map;			/* colormap to be displayed	    */
	ushort	duration;		/* # of vert. retrace intervals     */
} MID_map_duration_pair, *pMID_map_duration_pair;

typedef struct _MID_id_pair
{
	ushort	w_id;			/* window id			    */
	ushort	cp_id;			/* color palette id		    */
} MID_id_pair, *pMID_id_pair;

#endif	/* _H_MID_HW_TYPEDEFS */
