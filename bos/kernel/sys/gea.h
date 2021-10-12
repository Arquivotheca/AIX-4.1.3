/* @(#)15	1.23  src/bos/kernel/sys/gea.h, sysxdispsab, bos411, 9428A410j 3/7/91 16:51:18 */

/*
 * COMPONENT_NAME: (HIPRF3D) HIgh PeRFormance 3D color graphics processor
 *
 * FUNCTIONS: NONE
 *
 * ORIGINS: 27, 21
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *		 Copyright (C) 1987, Silicon Graphics, Inc.
 *
 *  These coded instructions, statements, and computer programs  contain
 *  unpublished  proprietary  information of Silicon Graphics, Inc., and
 *  are protected by Federal copyright law.  They  may	not be disclosed
 *  to	third  parties	or copied or duplicated in any form, in whole or
 *  in part, without the prior written consent of Silicon Graphics, Inc.
 *
 */


#ifndef _GEA_H_
#define _GEA_H_

typedef struct gea_make_gp {
	char *bus_addr; 	/* bus address without segment reg */
	int   hwconfig; 	/* hardware configuration flags */
#define GEA_8BIT	0x8
#define GEA_ZBUF	0x10
	ulong geaversion;	/* microcode version */
	ulong screen_width_mm;
	ulong screen_height_mm;
} gea_make_gp_t;

typedef struct gea_create_rcx {
	int raster_op;
	int raster_op_type;
	int pattern;
	int fg_color;
	int writemask;
	int pixel_type;
} gea_create_rcx_t;

/*
 * XMAP2 operation argument structures
 */

/*	-----------------------------------------------------------------
 *
 *	aixgsc(gsc_handle, DEV_DEP_FUN, &arg)
 *	GSC_HANDLE gsc_handle - return from ioctl(fd, HFHANDLE, &gsc_handle)
 *	struct dev_dep_fun arg;
 *	struct gea_xmapmodelo ddi;
 *
 *		arg.cmd = GEA_XMAPMODELO;
 *		arg.ddi = &ddi;
 *		arg.ddi_len = sizeof (struct gea_xmapmodelo);
 */

#	define GEA_XMAPMODELO	    202

typedef struct gea_xmapmodelo {
	WA_Handle	wa;

	unsigned char	mask;			/* selects attribute(s) */
#	define GEA_COLMODE		0x1	/* colmode should be updated */
#	define GEA_BUFSEL		0x2	/* bufsel should be updated */
#	define GEA_OVERLAY		0x4	/* overlay should be updated */
#	define GEA_UNDERLAY		0x8	/* underlay should be updated*/

	unsigned char colmode;
#	define GEA_XMAP_8CI		0x0	/* 8-bit color index */
#	define GEA_XMAP_8DCI		0x1	/* 8-bit Float64 bf'd col inx */
#	define GEA_XMAP_24DCI		0x2	/* 24-bit Float64 bf'd col inx */
#	define GEA_XMAP_24RGB		0x4	/* 24-bit RGB */
#	define GEA_XMAP_24DRGB		0x5	/* 24-bit Float64 bf'd RGB */

	unsigned char bufsel;
#	define GEA_SEL_BACK_BUF 	1
#	define GEA_SEL_FRONT_BUF	0

	unsigned char overlay;			/* bitmask for the planes to be
						   enabled (2 - base (0x3),
						   4 - enhanced (0xf)) */
#	define GEA_OVERLAY_ON		0xf	/* all planes enabled */
#	define GEA_OVERLAY_OFF		0	/* all planes disabled */

	unsigned char underlay;
#	define GEA_UNDERLAY_ON		1
#	define GEA_UNDERLAY_OFF 	0

} gea_xmapmodelo_t;

/*	-----------------------------------------------------------------
 *
 *	aixgsc(gsc_handle, DEV_DEP_FUN, &arg)
 *	GSC_HANDLE gsc_handle - return from ioctl(fd, HFHANDLE, &gsc_handle)
 *	struct dev_dep_fun arg;
 *	struct gea_xmapmodehi ddi;
 *
 *		arg.cmd = GEA_XMAPMODEHI;
 *		arg.ddi = &ddi;
 *		arg.ddi_len = sizeof (struct gea_xmapmodehi);
 */

#	define GEA_XMAPMODEHI		203

typedef struct gea_xmapmodehi {
	WA_Handle	wa;
	unsigned char	multimap;
#	define	GEA_MULTIMAP_ON 	1
#	define	GEA_MULTIMAP_OFF	0

	unsigned char	mapsel; 		/* 0-15 allowable */

} gea_xmapmodehi_t;

/*	-----------------------------------------------------------------
 *
 *	aixgsc(gsc_handle, DEV_DEP_FUN, &arg)
 *	GSC_HANDLE gsc_handle - return from ioctl(fd, HFHANDLE, &gsc_handle)
 *	struct dev_dep_fun arg;
 *	struct gea_mapcolor ddi;
 *
 *		arg.cmd = GEA_MAPCOLOR;
 *		arg.ddi = &ddi;
 *		arg.ddi_len = sizeof (struct gea_mapcolor);
 */

#	define GEA_MAPCOLOR	    204

typedef struct gea_mapcolor {
	unsigned long low;		/* starting color index */
	unsigned long high;		/* ending color index */
	struct color_ary {		/* ptr to array of colors */
		unsigned char reserved;
		unsigned char red;
		unsigned char green;
		unsigned char blue;
	} *color;
} gea_mapcolor_t;

/*	-----------------------------------------------------------------
 *
 *	aixgsc(gsc_handle, DEV_DEP_FUN, &arg)
 *	GSC_HANDLE gsc_handle - return from ioctl(fd, HFHANDLE, &gsc_handle)
 *	struct dev_dep_fun arg;
 *	struct gea_getmcolor ddi;
 *
 *		arg.cmd = GEA_GETMCOLOR;
 *		arg.ddi = &ddi;
 *		arg.ddi_len = sizeof (struct gea_getmcolor);
 */

#	define GEA_GETMCOLOR		    223

typedef struct gea_getmcolor {
	short index;
/*XXX shorts as in GL doc or chars? */
	unsigned char red;
	unsigned char green;
	unsigned char blue;
} gea_getmcolor_t;

/*	-----------------------------------------------------------------
 *
 *	aixgsc(gsc_handle, DEV_DEP_FUN, &arg)
 *	GSC_HANDLE gsc_handle - return from ioctl(fd, HFHANDLE, &gsc_handle)
 *	struct dev_dep_fun arg;
 *	struct gea_mapauxcolor ddi;
 *
 *		arg.cmd = GEA_MAPAUXCOLOR;
 *		arg.ddi = &ddi;
 *		arg.ddi_len = sizeof (struct gea_mapauxcolor);
 */

#	define GEA_MAPAUXCOLOR	    205

typedef struct gea_mapauxcolor {
	unsigned long low;		/* starting color index */
	unsigned long high;		/* ending color index */
	struct color_ary *color;	/* ptr to array of colors */
} gea_mapauxcolor_t;

/*	-----------------------------------------------------------------
 *
 *	aixgsc(gsc_handle, DEV_DEP_FUN, &arg)
 *	GSC_HANDLE gsc_handle - return from ioctl(fd, HFHANDLE, &gsc_handle)
 *	struct dev_dep_fun arg;
 *
 *		arg.cmd = GEA_SWAPBUFFERS;
 *		arg.ddi = &ddi;
 *		arg.ddi_len = sizeof (struct gea_swapbuffers);
 */

#	define GEA_SWAPBUFFERS	    206

typedef struct gea_swapbuffers {
	int buffer;
} gea_swapbuffers_t;

/*	-----------------------------------------------------------------
 *
 *	aixgsc(gsc_handle, DEV_DEP_FUN, &arg)
 *	GSC_HANDLE gsc_handle - return from ioctl(fd, HFHANDLE, &gsc_handle)
 *	struct dev_dep_fun arg;
 *	struct gea_swapinterval ddi;
 *
 *		arg.cmd = GEA_SWAPINTERVAL;
 *		arg.ddi = &ddi;
 *		arg.ddi_len = sizeof (struct gea_swapinterval);
 */

#	define GEA_SWAPINTERVAL     207

typedef struct gea_swapinterval {
	short frames;
} gea_swapinterval_t;

/*	-----------------------------------------------------------------
 *
 *	aixgsc(gsc_handle, DEV_DEP_FUN, &arg)
 *	GSC_HANDLE gsc_handle - return from ioctl(fd, HFHANDLE, &gsc_handle)
 *	struct dev_dep_fun arg;
 *	struct gea_blink ddi;
 *
 *		arg.cmd = GEA_BLINK;
 *		arg.ddi = &ddi;
 *		arg.ddi_len = sizeof (struct gea_blink);
 */

#	define GEA_BLINK	    208

typedef struct gea_blink {
	unsigned short rate;
	unsigned short index;
	struct color_ary color;
} gea_blink_t;

/*	-----------------------------------------------------------------
 *
 *	aixgsc(gsc_handle, DEV_DEP_FUN, &arg)
 *	GSC_HANDLE gsc_handle - return from ioctl(fd, HFHANDLE, &gsc_handle)
 *	struct dev_dep_fun arg;
 *	struct gea_cyclemap ddi;
 *
 *		arg.cmd = GEA_CYCLEMAP;
 *		arg.ddi = &ddi;
 *		arg.ddi_len = sizeof (struct gea_cyclemap);
 */

#	define GEA_CYCLEMAP	    209


/* Cyclemap struct (for retrace module) */
typedef struct gea_cyclemap {
	WA_Handle      wa;
	unsigned short map;
	unsigned short nextmap;
	unsigned short duration;
} gea_cyclemap_t;

/*	----------------------------------------------------------------- */

/*
 * DAC operation argument structures
 */

/*	-----------------------------------------------------------------
 *
 *	aixgsc(gsc_handle, DEV_DEP_FUN, &arg)
 *	GSC_HANDLE gsc_handle - return from ioctl(fd, HFHANDLE, &gsc_handle)
 *	struct dev_dep_fun arg;
 *	struct gea_gammaramp ddi;
 *
 *		arg.cmd = GEA_GAMMARAMP;
 *		arg.ddi = &ddi;
 *		arg.ddi_len = sizeof (struct gea_gammaramp);
 */

#	define GEA_GAMMARAMP		    210

typedef struct gea_gammaramp {
	unsigned long low;		/* starting color index */
	unsigned long high;		/* ending color index */
	struct color_ary *color;	/* ptr to array of colors */
} gea_gammaramp_t;


/*	----------------------------------------------------------------- */

/*
 * Cursor operation argument structures
 */

/*	-----------------------------------------------------------------
 *
 *	aixgsc(gsc_handle, DEV_DEP_FUN, &arg)
 *	GSC_HANDLE gsc_handle - return from ioctl(fd, HFHANDLE, &gsc_handle)
 *	struct dev_dep_fun arg;
 *	struct gea_setcursor ddi;
 *
 *		arg.cmd = GEA_SETCURSOR;
 *		arg.ddi = &ddi;
 *		arg.ddi_len = sizeof (struct gea_setcursor);
 */

#	define GEA_SETCURSOR		    211

typedef struct gea_setcursor {

	unsigned char *glyph[2];	/* glyph is 64 x 64 bits */

	char type;			/* cursor type */

/* cursor types */
#define CURS_BLANK	0x00		/* turn off cursor */
#define CURS_BLOCK	0x40		/* block cursor */
#define CURS_CROSS	0x20		/* Cross hair cursor */
#define CURS_FMT	0x10		/* OR overlapping pixels if both
					   block and cross hair are enabled */

	char cross_hair_type;		/* used only when crosshair
					   cursor is selected above */
/* crosshair cursor types */
#define CURS_1THICK	0x00		/* Cross hair is 1 bit thick */
#define CURS_3THICK	0x01		/* Cross hair is 3 bits thick */
#define CURS_5THICK	0x02		/* Cross hair is 5 bits thick */
#define CURS_7THICK	0x03		/* Cross hair is 7 bits thick */

	char xorigin;
	char yorigin;

	ulong	changes;		/* change mask */
#define CHANGE_CURS_TYPE	0x00000008
#define CHANGE_CURS_ORIGIN	0x00000001
#define CHANGE_CURS_GLYPH	0x00000002
#define CHANGE_CURS_ALL 	0xffffffff
} gea_setcursor_t;

/*	-----------------------------------------------------------------
 *
 *	aixgsc(gsc_handle, DEV_DEP_FUN, &arg)
 *	GSC_HANDLE gsc_handle - return from ioctl(fd, HFHANDLE, &gsc_handle)
 *	struct dev_dep_fun arg;
 *	struct gea_cursorposition ddi;
 *
 *		arg.cmd = GEA_CURSORPOSITION;
 *		arg.ddi = &ddi;
 *		arg.ddi_len = sizeof (struct gea_cursorposition);
 */

#define GEA_CURSORPOSITION	     212

typedef struct gea_cursorposition {
	unsigned short x;
	unsigned short y;
} gea_cursorposition_t;

/*	-----------------------------------------------------------------
 *
 *	aixgsc(gsc_handle, DEV_DEP_FUN, &arg)
 *	GSC_HANDLE gsc_handle - return from ioctl(fd, HFHANDLE, &gsc_handle)
 *	struct dev_dep_fun arg;
 *	struct gea_mapcurscolor ddi;
 *
 *		arg.cmd = GEA_MAPCURSCOLOR;
 *		arg.ddi = &ddi;
 *		arg.ddi_len = sizeof (struct gea_mapcurscolor);
 */

#	define GEA_MAPCURSCOLOR     213

typedef struct gea_mapcurscolor {

	char index;		/* color index */
	unsigned char red;
	unsigned char green;
	unsigned char blue;
} gea_mapcurscolor_t;

/*	----------------------------------------------------------------- */

/*
 * Display Register operation argument structures
 */

/*	-----------------------------------------------------------------
 *
 *	aixgsc(gsc_handle, DEV_DEP_FUN, &arg)
 *	GSC_HANDLE gsc_handle - return from ioctl(fd, HFHANDLE, &gsc_handle)
 *	struct dev_dep_fun arg;
 *	struct gea_blankscreen ddi;
 *
 *		arg.cmd = GEA_BLANKSCREEN;
 *		arg.ddi = &ddi;
 *		arg.ddi_len = sizeof (struct gea_blankscreen);
 */

#	define GEA_BLANKSCREEN	    215

typedef struct gea_blankscreen {
	char blankscreen;
} gea_blankscreen_t;

/*	-----------------------------------------------------------------
 *
 *	aixgsc(gsc_handle, DEV_DEP_FUN, &arg)
 *	GSC_HANDLE gsc_handle - return from ioctl(fd, HFHANDLE, &gsc_handle)
 *	struct dev_dep_fun arg;
 *	struct gea_setmonitor ddi;
 *
 *		arg.cmd = GEA_SETMONITOR;
 *		arg.ddi = &ddi;
 *		arg.ddi_len = sizeof (struct gea_setmonitor);
 */

#	define GEA_SETMONITOR	    216

typedef struct gea_setmonitor {
	unsigned char monitortype;
	unsigned char flag;
	long xmaxscreen;
	long ymaxscreen;
	long xcursmagic;
	long ycursmagic;
} gea_setmonitor_t;

/*	-----------------------------------------------------------------
 *
 *	aixgsc(gsc_handle, DEV_DEP_FUN, &arg)
 *	GSC_HANDLE gsc_handle - return from ioctl(fd, HFHANDLE, &gsc_handle)
 *	struct dev_dep_fun arg;
 *	struct gea_getmonitor ddi;
 *
 *		arg.cmd = GEA_GETMONITOR;
 *		arg.ddi = &ddi;
 *		arg.ddi_len = sizeof (struct gea_getmonitor);
 */

#	define GEA_GETMONITOR	222

typedef struct gea_getmonitor {
	unsigned char monitortype;
	unsigned char flag;
} gea_getmonitor_t;

/* Display control flags: */
#	define GEA_SYNCGRN	0x01	/* Composite sync on green */
#	define GEA_BLANK	0x02	/* Screen is currently off */
#	define GEA_STEREO	0x04	/* Stereo optic display enabled */
#	define GEA_EXTCLK	0x08	/* Timing locked to external clock
					   (ie in genlock mode */


/*	-----------------------------------------------------------------
 *
 *	aixgsc(gsc_handle, DEV_DEP_FUN, &arg)
 *	GSC_HANDLE gsc_handle - return from ioctl(fd, HFHANDLE, &gsc_handle)
 *	struct dev_dep_fun arg;
 *	struct gea_getwin_org ddi;
 *
 *		arg.cmd = GEA_GETWIN_ORG;
 *		arg.ddi = &ddi;
 *		arg.ddi_len = sizeof (struct gea_win_org);
 */

#	define GEA_GETWIN_ORG  224

typedef struct gea_getwin_org {
	ulong wa_handle;
	struct _gWinGeomAttributes wg;
} gea_getwin_org_t;
/*	----------------------------------------------------------------- */



/*	-----------------------------------------------------------------
 *
 *	aixgsc(gsc_handle, DEV_DEP_FUN, &arg)
 *	GSC_HANDLE gsc_handle - return from ioctl(fd, HFHANDLE, &gsc_handle)
 *	struct dev_dep_fun arg;
 *	struct gea_perf_trace ddi;
 *
 *		arg.cmd = GEA_PERF_TRACE;
 *		arg.ddi = &ddi;
 *		arg.ddi_len = sizeof (struct gea_perf_trace);
 */

#	define GEA_PERF_TRACE  225

typedef struct gea_perf_trace {
	int	type;		/* type of operation */
#	define	GEA_PERF_TRACE_ON	1
#	define	GEA_PERF_TRACE_OFF	0
	/*
	 * The following fields are returned if type=GEA_PERF_TRACE_OFF:
	 */
	int	gthf_secs;	/* seconds FIFO > than half full */
	int	gthf_nsec;	/* nanoseconds FIFO > than half full */
	int	tot_secs;	/* total seconds in measurement interval */
	int	tot_nsec;	/* total nanosecs in measurement interval */
} gea_perf_trace_t;
/*	----------------------------------------------------------------- */

/*
 * Raster Engine operation argument structures
 */

/*	-----------------------------------------------------------------
 *
 *	aixgsc(gsc_handle, DMA_SERVICE, &arg)
 *	GSC_HANDLE gsc_handle - return from ioctl(fd, HFHANDLE, &gsc_handle)
 *	struct _gscdma arg;
 *	struct gea_dma ddi;
 *
 *		arg.dma_cmd = &ddi;
 *		arg.cmd_length = sizeof(struct gea_dma);  OR
 *		arg.cmd_length = sizeof(struct gea_manual_dma);
 */

#	define GEA_DMA		1


typedef struct gea_dma {
	ulong flags;
#	define GEA_DMA_LNBYLN	   0x04 /* Line by line DMA */

/* The following are not currently supported: */
#	define GEA_DMA_SINGLE	   0x08 /* Single line DMA (special case) */
#	define GEA_DMA_INTR	   0x10 /* Ucode should interrupt when done */

/*	Don't use these bits	*/
#	define GEA_DMA_RESERVED_1	0x100
#	define GEA_DMA_RESERVED_2	0x200
	ulong x_start;			/* Rectangular coordinate in pixels */
	ulong x_len;
	ulong y_start;
	ulong y_len;
	ulong pixsize;			/* Pixel size (1, 2, 4 bytes) */
	ulong upacmode; 		/* Packing factor */
#	define GEA_DMA_NOPACK	0	/* No packing */
#	define GEA_DMA_PACK	3

} gea_dma_t;

/*----------
  The graphics entry level display adapter (gea) offers monitor
  mode users the ability to manually send down the fifo tokens
  for the DMA_SERVICE aixgsc call.  The application writer fills
  the data structure gea_manual_dma, shown below, and points the
  dma_cmd field in the gscdma structure to it before calling
  aixgsc.
  ----------*/

#define MAX_GEA_DATA_TOKENS		50

typedef struct gea_manual_dma {
	ulong	flags;

/* flag for gea_manual_dma */
#define MANUAL_DMA	0x100		/* set this flag for manual dma */
/*	Don't use these bits
#define RESERVED	0x200
*/

	ulong	fifo_entry_point_token;
	ulong	num_of_data_tokens;
	ulong	data_token[MAX_GEA_DATA_TOKENS];
} gea_manual_dma_t;

/*	----------------------------------------------------------------- */

#define GEA_NOMEMORY	1

/*	----------------------------------------------------------------- */

/*
 *	Diagnostics
 */

#define GEA_INTRS	8	/* number of interrupt types */

struct geaqdiagr {		/* query diagnostics response */
	char hf_intro[HFINTROSZ];	/* esc [ x  sizeof(struct hfqdiagr)-3
						 HFQDIAGRH HFQDIAGRL */
					/* Next 3 fields ignored */
	char hf_sublen; 		/* sub header length = 3 */
	char hf_subtype;		/* sub header type = 1 */
	char hf_align;			/* alignment byte */
	ulong hf_result[GEA_INTRS];	/* interrupt counters */
};

/*
 *	Structure defining counters used to return values in hf_result[]:
 */

	struct gea_intr_count {
		ulong x[GEA_INTRS];	/* one counter per interrupt type */
	};

/*
 *	Values used as cmd parameters to ioctl (2nd parm):
 */

#	define GEA_START_DIAG 1
#	define GEA_QUERY_DIAG 2
#	define GEA_STOP_DIAG  3

/*
 *	Return codes
 */
#define GEA_PIN_FAIL		1

#define PICKEVENT    0x01000000
#define ENDPICK      0x02000000
#define GSYNC	     0x03000000

/*
 *	VPD Configuration Interface
 *
 *	Structure returned by the device configuration call.
 */

struct gea_vpd {

	ulong	microcode_version;		/* version microcode loaded */

	ulong	configuration;			/* cards configured */
	/* Note: MGE, MRV always present; one of MDE or MEV always present */
#	define	GEA_VPD_MGE	0x01		/* host i/f & graphics engine*/
#	define	GEA_VPD_MRV	0x02		/* screen buf/display subsys */
#	define	GEA_VPD_MDE	0x04		/* video DACs & XPCs */
#	define	GEA_VPD_MEV	0x08		/* ext screen buffer & XMAPs */
#	define	GEA_VPD_MZB	0x10		/* Z-buffer */

#	define	MAX_VPD_LEN	256		/* bytes per VPD */
	uchar	vpd[4 * MAX_VPD_LEN];		/* 3 or 4 VPDs configured */
};

#endif /* _GEA_H */
