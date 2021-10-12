/* @(#)23       1.5  src/bos/kernel/sys/acpa.h, sysxacpa, bos411, 9428A410j 6/26/91 13:04:00 */

#ifndef _H_ACPA
#define _H_ACPA

/*
 * COMPONENT_NAME: SYSXACPA - Multimedia Audio Capture and Playback Adapter
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* These are the available tracks and their definitions. */
#define ACPA_TRACK1     0x01            /* first ACPA track was specified */
#define ACPA_TRACK2     0x02            /* second ACPA track was specified */

/* These are the available control files and their definitions. */
#define ACPA_CTL        0x100           /* the ctl bit */
#define ACPA_CTL1       ACPA_CTL | 1    /* first ctl file was specified */
#define ACPA_CTL2       ACPA_CTL | 2    /* second ctl file was specified */

/* This is the definition for information about a track's settings. */
typedef struct _track_info
{
	unsigned short master_volume;   /* the master volume setting */
	unsigned short dither_percent;  /* 0 - 100 are valid percentages */
	unsigned short reserved[3];     /* reserved fields */
} track_info;

#endif /* _H_ACPA */
