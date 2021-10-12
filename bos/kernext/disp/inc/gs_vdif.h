/* @(#)23	1.1  src/bos/kernext/disp/inc/gs_vdif.h, sysxdisp, bos411, 9433B411a 8/15/94 14:58:50 */

#ifndef _H_GS_VDIF
#define _H_GS_VDIF

/*
 * COMPONENT_NAME: (sysxdisp) Display Sub-System
 *
 * FUNCTIONS:  VDIF structure from the VESA standards group
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1994
 * All Rights Reserved
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/******************************************************************************
* 									      *
*	Misc Definitions						      *
* 									      *
******************************************************************************/

#define GS_VDIF_MONITOR_MONOCHROME		0
#define GS_VDIF_MONITOR_COLOR			1

#define GS_VDIF_VIDEO_TTL			0
#define GS_VDIF_VIDEO_ANALOG			1
#define GS_VDIF_VIDEO_ECL			2
#define GS_VDIF_VIDEO_DECL			3
#define GS_VDIF_VIDEO_OTHER			4

#define GS_VDIF_SYNC_SEPARATE			0
#define GS_VDIF_SYNC_C				1
#define GS_VDIF_SYNC_CP				2
#define GS_VDIF_SYNC_G				3
#define GS_VDIF_SYNC_GP				4
#define GS_VDIF_SYNC_OTHER			5

#define GS_VDIF_SCAN_NONINTERLACED		0
#define GS_VDIF_SCAN_INTERLACED			1
#define GS_VDIF_SCAN_OTHER			2

#define GS_VDIF_POLARITY_NEGATIVE		0
#define GS_VDIF_POLARITY_POSITIVE		1


/******************************************************************************
* 									      *
*	VDIF Main structure						      *
* 									      *
******************************************************************************/

typedef struct _GS_VDIF {			/* Monitor Description	     */
	unsigned char	VDIFId[4];		/* always "VDIF"	     */
	unsigned long	FileLength;		/* length of the whole file  */
	unsigned long	Checksum;		/* sum of all the bytes in   */
						/* the file after this field */
	unsigned short	VDIFVersion;		/* structure version number  */
	unsigned short	VDIFRevision;		/* structure revision number */
	unsigned short	Date[3];		/* file date Year/Month/Day  */
	unsigned short	DateManufactured[3];	/* date Year/Month/Day	     */
	unsigned long	FileRevision;		/* file revision string	     */
	unsigned long	Manufacturer;		/* ASCII ID of the	     */
						/* manufacturer		     */
	unsigned long	ModelNumber;		/* ASCII ID of the model     */
	unsigned long	MinVDIFIndex;		/* ASCII ID of Minimum VDIF  */
						/* index		     */
	unsigned long	Version;		/* ASCII ID of the model     */
						/* version		     */
	unsigned long	SerialNumber;		/* ASCII ID of the serial    */
						/* number		     */
	unsigned char	MonitorType;		/* Monochrome or Color	     */
	unsigned char	CRTSize;		/* inches		     */
	unsigned char	BorderRed;		/* percent		     */
	unsigned char	BorderGreen;		/* percent		     */
	unsigned char	BorderBlue;		/* percent		     */
	unsigned char	Reserved1;		/* padding		     */
	unsigned short	Reserved2;		/* padding		     */
	unsigned long	RedPhosphorDecay;	/* microseconds		     */
	unsigned long	GreenPhosphorDecay;	/* microseconds		     */
	unsigned long	BluePhosphorDecay;	/* microseconds		     */
	unsigned short	WhitePoint_x;		/* WhitePoint in CIExyY	     */
	unsigned short	WhitePoint_y;		/* (scale 1000)		     */
	unsigned short	WhitePoint_Y;
	unsigned short	RedChromaticity_x;	/* Red chromaticity in x,y   */
	unsigned short	RedChromaticity_y;
	unsigned short	GreenChromaticity_x;	/* Green chromaticity in x,y */
	unsigned short	GreenChromaticity_y;
	unsigned short	BlueChromaticity_x;	/* Blue chromaticity in x,y  */
	unsigned short	BlueChromaticity_y;
	unsigned short	RedGamma;		/* Gamma curve exponent	     */
	unsigned short	GreenGamma;		/* (scale 1000)		     */
	unsigned short	BlueGamma;
	unsigned long	NumberOperationalLimits;
	unsigned long	OffsetOperationalLimits;
	unsigned long	NumberOptions;		/* optional sections	     */
	unsigned long	OffsetOptions;		/* (gamma table)	     */
	unsigned long	OffsetStringTable;
} GS_VDIFRec, *GS_VDIFPtr;


/******************************************************************************
* 									      *
*	Header structure and definitions				      *
* 									      *
******************************************************************************/

typedef enum {					/* Tags for section	     */
	GS_VDIF_OPERATIONAL_LIMITS_TAG = 1,	/* identification	     */
	GS_VDIF_PREADJUSTED_TIMING_TAG,
	GS_VDIF_GAMMA_TABLE_TAG
} GS_VDIFScnTag;

typedef struct _GS_VDIFScnHdr {			/* Generic Section Header    */
	unsigned long	ScnLength;		/* length of section	     */
	unsigned long	ScnTag;			/* tag for section id	     */
} GS_VDIFScnHdrRec, *GS_VDIFScnHdrPtr;


/******************************************************************************
* 									      *
*	Limits structure						      *
* 									      *
******************************************************************************/

typedef struct _GS_VDIFLimits {			/* Operational Limits	     */
	GS_VDIFScnHdrRec  Header;		/* common section info	     */
	unsigned short	MaxHorPixel;		/* pixels		     */
	unsigned short	MaxVerPixel;		/* lines		     */
	unsigned short	MaxHorActiveLength;	/* millimeters		     */
	unsigned short	MaxVerActiveLength;	/* millimeters	  	     */
	unsigned char	VideoType;		/* TTL / Analog / ECL / DECL */
	unsigned char	SyncType;		/* TTL / Analog / ECL / DECL */
	unsigned char	SyncConfiguration;	/* separate / composite /    */
						/* other		     */
	unsigned char	Reserved1;		/* padding		     */
	unsigned short	Reserved2;		/* padding		     */
	unsigned short	TerminationResistance;
	unsigned short	WhiteLevel;		/* millivolts		     */
	unsigned short	BlackLevel;		/* millivolts		     */
	unsigned short	SyncLevel;		/* millivolts		     */
	unsigned long	MaxPixelClock;		/* Khz			     */
	unsigned long	MinHorFrequency;	/* Hz			     */
	unsigned long	MaxHorFrequency;	/* Hz			     */
	unsigned long	MinVerFrequency;	/* millihertz		     */
	unsigned long	MaxVerFrequency;	/* millihertz		     */
	unsigned short	MinHorRetrace;		/* ns			     */
	unsigned short	MinVerRetrace;		/* us			     */
	unsigned long	NumberPreadjustedTimings;
	unsigned long	OffsetNextLimits;
} GS_VDIFLimitsRec, *GS_VDIFLimitsPtr;


/******************************************************************************
* 									      *
*	Timing structure						      *
* 									      *
******************************************************************************/

typedef struct _GS_VDIFTiming {			/* Preadjusted Timing	     */
	GS_VDIFScnHdrRec  Header;		/* common section info	     */
	unsigned long	PreadjustedTimingName;	/* SVGA/SVPMI mode number    */
	unsigned short	HorPixel;		/* pixels		     */
	unsigned short	VerPixel;		/* lines	  	     */
	unsigned short	HorAddrLength;		/* millimeters		     */
	unsigned short	VerAddrLenth;		/* millimeters		     */
	unsigned char	PixelWidthRatio;	/* gives H:V		     */
	unsigned char 	PixelHeightRatio;
	unsigned char	Reserved1;		/* padding		     */
	unsigned char	ScanType;		/* noninterlaced /	     */
						/* interlaced / other	     */
	unsigned char	HorSyncPolarity;	/* negative / positive	     */
	unsigned char	VerSyncPolarity;	/* negative / positive	     */
	unsigned short	CharacterWidth;		/* pixels		     */
	unsigned long	PixelClock;		/* Khz			     */
	unsigned long	HorFrequency;		/* Hz			     */
	unsigned long	VerFrequency;		/* millihertz		     */
	unsigned long	HorTotalTime;		/* ns			     */
	unsigned long	VerTotalTime;		/* us			     */
	unsigned short	HorAddrTime;		/* ns			     */
	unsigned short	HorBlankStart;		/* ns			     */
	unsigned short	HorBlankTime;		/* ns			     */
	unsigned short	HorSyncStart;		/* ns			     */
	unsigned short	HorSyncTime;		/* ns			     */
	unsigned short	VerAddrTime;		/* us			     */
	unsigned short	VerBlankStart;		/* us			     */
	unsigned short	VerBlankTime;		/* us			     */
	unsigned short	VerSyncStart;		/* us			     */
	unsigned short	VerSyncTime;		/* us			     */
} GS_VDIFTimingRec, *GS_VDIFTimingPtr;


/******************************************************************************
* 									      *
*	Gamma structure							      *
* 									      *
******************************************************************************/

typedef struct _GS_VDIFGamma {			/* Gamma Table		    */
	GS_VDIFScnHdrRec  Header;		/* common section info	    */
	unsigned short	GammaTableEntries;	/* count of grays or 	    */
	unsigned short	Unused1;		/* RGB 3-tuples	 	    */
} GS_VDIFGammaRec, *GS_VDIFGammaPtr;


/******************************************************************************
* 									      *
*	Access macros							      *
* 									      *
******************************************************************************/

#define GS_VDIF_OPERATIONAL_LIMITS(vdif)				\
	((GS_VDIFLimitsPtr)((char *)(vdif) + (vdif)->OffsetOperationalLimits))

#define GS_VDIF_NEXT_OPERATIONAL_LIMITS(limits)				\
	((GS_VDIFLimitsPtr)((char *)(limits) + (limits)->OffsetNextLimits))

#define GS_VDIF_PREADJUSTED_TIMING(limits)				\
	((GS_VDIFTimingPtr)((char *)(limits) + (limits)->Header.ScnLength))

#define GS_VDIF_NEXT_PREADJUSTED_TIMING(timing)				\
	((GS_VDIFTimingPtr)((char *)(timing) + (timing)->Header.ScnLength))

#define GS_VDIF_NEXT_OPTIONS(options)					\
	((GS_VDIFScnHdrPtr)((char *)(options) + (options)->ScnLength))

#define GS_VDIF_STRING(vdif, string)					\
	((char *)((char *)(vdif) + (vdif)->OffsetStringTable + (string)))


#endif /* _H_GS_VDIF */

