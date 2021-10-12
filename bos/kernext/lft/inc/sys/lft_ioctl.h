/* @(#)92	1.1  src/bos/kernext/lft/inc/sys/lft_ioctl.h, lftdd, bos411, 9428A410j 10/15/93 14:40:31 */
/*
 *   COMPONENT_NAME: LFTDD
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/cfgdb.h>

#define LFTIOC            ('L'<<8)
#define	LFT_SET_DFLT_DISP 	(LFTIOC | 1)    /* set the default display */
#define	LFT_SET_DIAG_OWNER	(LFTIOC | 2)    /* set diag owner of display */
#define	LFT_QUERY_LFT		(LFTIOC | 3)    /* query the lft */
#define	LFT_QUERY_DISP		(LFTIOC | 4)    /* query the lft */
#define LFT_ACQ_DISP		(LFTIOC | 5)    /* acquire a display */
#define LFT_REL_DISP		(LFTIOC | 6)    /* release a display */

/* ---------------------------------------------------------------------- *
 * The lft_query ioctl passes back to the user information about the      *
 * lft, kbd and the displays- NAMESIZE is currently set to 16 and is	  *
 * defined int cfgdb.h							  *
 * ---------------------------------------------------------------------- */


typedef struct {
	dev_t	disp_devno;
	char	disp_name[NAMESIZE];
	ushort	disp_enable;		/* TRUE if lft can use display    */
	ushort	flags;			/* Ex: APP_IS_DIAG		  */
}lft_disp_info_t;
 
typedef struct {
	int	num_of_disps;		/* Get info for this many displays */
	lft_disp_info_t	*lft_disp;	/* Ptr to lft_disp_info_t structs */
}lft_disp_query_t;

typedef struct {
	dev_t	lft_devno;		/* Majoro.minor # of the lft	  */
	dev_t	kbd_devno;		/* Major/minor # of the kbd       */
	int  	number_of_displays;	/* Total # of AVAILABLE displays  */
	int	default_display;	/* Index of the default display   */
	char	swkbd_file[256];	/* Name of swkbd file		  */
	char	keyboard_name[NAMESIZE];/* e.g. en_US			  */
	uint	fkproc_started;		/* TRUE is fkproc has been started*/
} lft_query_t;
