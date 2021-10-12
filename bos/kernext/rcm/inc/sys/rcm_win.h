/* @(#)16	1.5.2.7  src/bos/kernext/rcm/inc/sys/rcm_win.h, rcm, bos41J, 9519B_all 5/10/95 16:35:59 */
/*
 * COMPONENT_NAME: (rcm) Rendering Context Manager Win Geom Defs
 *
 * FUNCTIONS: none
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1991-1995
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_RCM_WIN
#define _H_RCM_WIN


typedef char *gHandle;

typedef struct _gPoint {               /* ---  exactly like X11 server  ---*/
    short               x, y;
} gPoint, *gPointPtr;

#define gVector		gPoint	       /* A vector connects two points     */
#define gVectorPtr	gPointPtr      /* A vector connects two points     */

typedef struct _gBox {                 /* ---- exactly like X11 server ----*/
    gPoint              ul;            /* upper left coordinates           */
    gPoint              lr;            /* lower right coordinates          */
} gBox, *gBoxPtr;


/*****************************************************************************
 *		X11 R4 Region Structure support 
 *****************************************************************************/

typedef struct _gRegData {             /* --- exactly like X11R4 server ---*/
    long    		size;          /*                                  */
    long                numRects;      /* number of clipping boxes         */
/*  gBox		rects[size]; in memory but not explicitly declared */
} gRegData, *gRegDataPtr;

typedef struct _gRegionR4 {            /* --- exactly like X11R4 server ---*/
    gBox		extents;       /* smallest box bounding region     */
    gRegDataPtr		data;
} gRegionR4, *gRegionR4Ptr;

/*****************************************************************************
 *		X11 R3 Region Structure support
 *****************************************************************************/

typedef struct _gRegion {            /* --- exactly like X11R3 server ---*/
    long    	        size;          /*                                  */
    long                numBoxes;      /* number of clipping boxes         */
    gBoxPtr             pBox;          /* pointer to clipping boxes        */
    gBox                extents;       /* smallest box bounding region     */
} gRegionR3, *gRegionR3Ptr;


/*****************************************************************************
 *		Defined symbols to switch between R4 and R3
 *		(R4 is recommended!)
 *****************************************************************************/

#ifndef USE_R3_REGIONS
	typedef gRegionR4 gRegion, *gRegionPtr;
#else
	typedef gRegionR3 gRegion, *gRegionPtr;
#endif

/*****************************************************************************
 *		Region Macros which obey the R4/R3 defined symbol switch
 *****************************************************************************/

#ifndef _REGION_MACROS
#define _REGION_MACROS

#ifndef USE_R3_REGIONS

   /* ---- R4 (recommended case) --------------------- */
#  define gREGION_NIL(reg) ((reg)->data && !(reg)->data->numRects)
#  define gREGION_NUM_RECTS(reg) ((reg)->data ? (reg)->data->numRects : 1)
#  define gREGION_SIZE(reg) ((reg)->data ? (reg)->data->size : 0)
#  define gREGION_RECTS(reg) ((reg)->data ? (gBoxPtr)((reg)->data + 1) \
					: &(reg)->extents)
#  define gREGION_BOXPTR(reg) ((gBoxPtr)((reg)->data + 1))
#  define gREGION_BOX(reg,i) (&gREGION_BOXPTR(reg)[i])
#  define gREGION_TOP(reg) gREGION_BOX(reg, (reg)->data->numRects)
#  define gREGION_END(reg) gREGION_BOX(reg, (reg)->data->numRects - 1)
#  define gREGION_SZOF(n) (sizeof(gRegData) + ((n) * sizeof(gBox)))

#else

   /* ---- R3 case ----------------------------------- */
#  define gREGION_NIL(reg) (!(reg)->numBoxes)
#  define gREGION_NUM_RECTS(reg) ((reg)->numBoxes)
#  define gREGION_SIZE(reg) ((reg)->size)
#  define gREGION_RECTS(reg) ((reg)->pBox)
#  define gREGION_BOXPTR(reg) ((gBoxPtr) ((reg)->pBox))
#  define gREGION_BOX(reg,i) (&gREGION_BOXPTR(reg)[i])
#  define gREGION_TOP(reg) gREGION_BOX(reg, (reg)->numBoxes)
#  define gREGION_END(reg) gREGION_BOX(reg, (reg)->numBoxes - 1)
#  define gREGION_SZOF(n) ((n) * sizeof(gBox))

#endif /* USE_R3_REGIONS */

#endif /* _REGION_MACROS */


/*****************************************************************************
 *		Typedefs Shared between Kernel Extension and GAI
 *****************************************************************************/

typedef union _gPixmapFormat {         /* two ways to access               */
    struct {
	uchar       depth;             /* number of planes per pixel       */
	uchar       bitsPerPixel;      /* system memory bits per pel       */
	uchar       scanlinePad;       /* 8, 16, or 32 bits                */
	uchar       format;            /* XY, Z,                           */
    }           info;
    unsigned    id;                    /* treate composite as ID           */
} gPixmapFormat;

	     /* for Pixmapformat info.format, there is a flag to           */
	     /* "or" in that gives hints on coordinate orientation         */
	     /* of the pixmap (upper-left or lower-left)                   */
#define gPixmapOrgLL    0x80           /* if UL, then leave bit as 0       */

typedef struct _gPixmap {
    gPixmapFormat fmt;                 /* pixmap format                    */
    ushort        width;               /* width of pixmap in pixels        */
    ushort        height;              /* height of pixmap in pixels       */
    uchar         *pData;              /* pointer to pixmap data           */
} gPixmap, *gPixmapPtr;

typedef struct _gWindowAttributes {
   gHandle              wa_handle;      /* For use by Server and RCM only */
   ushort               numGroups;      /* number of group resources      */
   struct _gWinGroup    *group;         /* array of group resources       */
   struct _gWinGroup    *pActiveGroup;  /* group resource being used      */
   gPoint               maskOrg;        /* bitmask upper left w/in window */
   gPixmapPtr           pMask;          /* bitmask used to clip pixels    */
   gRegionPtr           pRegion;        /* window clipping regions        */
   ushort		RegOrigin;	/* ul or ll region origin	  */
   unsigned             StateCount;     /* count of changes to WinAttr    */
} gWindowAttributes, *gWindowAttrPtr;

/*
 * RegOrigin flags
 */
#define REG_ORG_LL	1
#define REG_ORG_UL	2

/* permitted values for (gsc_MultDispBuff_t) x.buffer in user space */
enum gsc_MultBuff_values {
	gsc_MBX_SWAP_PENDING = 0,
	gsc_MBX_SWAPPED      = 1 };

/* For driver to update user space on swapbuffers */
typedef struct _gscMultDispBuff {
   enum gsc_MultBuff_values flag;
} gscMultDispBuff_t, *pgscMultDispBuff_t;

/* permitted values for (gsc_CurrDispBuff_t) x.buffer in user space */
enum gsc_DispBuff_values {
	gsc_DispBuff_init = 0,
	gsc_DispBuff_A = 1,
	gsc_DispBuff_B = 2,
	gsc_DispBuff_C = 3,
	gsc_DispBuff_D = 4,
	gsc_DispBuff_E = 5,
	gsc_DispBuff_F = 6,
	gsc_DispBuff_G = 7,
	gsc_DispBuff_H = 8,
	gsc_DispBuff_I = 9,
	gsc_DispBuff_J = 10,
	gsc_DispBuff_K = 11,
	gsc_DispBuff_L = 12,
	gsc_DispBuff_M = 13,
	gsc_DispBuff_N = 14,
	gsc_DispBuff_O = 15,
	gsc_DispBuff_P = 16  };

/* For driver to update user space on swapbuffers */
typedef struct _gscCurrDispBuff {
   enum gsc_DispBuff_values buffer;
} gscCurrDispBuff_t, *pgscCurrDispBuff_t;

typedef struct _gWinGeomAttributes {
   gHandle              wg_handle;      /* For use by Server and RCM only */
   gPoint               winOrg;         /* origin in screen coordinates   */
   ushort               width;          /* width in pixels                */
   ushort               height;         /* height in pixels               */
   unsigned		correctColor:1; /* window has installed colormap  */
   unsigned		mustRealize:1;  /* must realize colormap          */
   unsigned		transparent:1;  /* for Overlay Extension	  */
   gRegionPtr           pClip;          /* screen clipping regions        */
   gHandle              cm_handle;      /* color map index, focus         */
   unsigned char	depth;		/* depth in bits		  */
   unsigned char	colorClass;	/* pixel data interpretation	  */
   long			layer;		/* for Overlay Extension	  */
   gRegionPtr		visibilityList;	/* screen visibility regions	  */
   long			cmapID;		/* hardware color table index	  */
   pgscCurrDispBuff_t	pCurrDispBuff;	/* pointer to user spc to update  */
} gWinGeomAttributes, *gWinGeomAttrPtr;

/*
 * For colorClass values, see "palette class" defines in gX.h
 */


#endif /* _H_RCM_WIN */

