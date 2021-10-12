/* @(#)84	1.3  src/bos/kernext/lft/inc/lft_dds.h, lftdd, bos41J, 9516B_all 4/18/95 10:05:59 */
/*
 *   COMPONENT_NAME: LFTDD
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* ------------------------------------------------------------------------ *
 * The lft_dev structure contains information about the lft device	    *
 * NAMESIZE is currently set to 16 and is defined in <sys/cfgdb.h>	    *
 * ------------------------------------------------------------------------ */
#include <sys/cfgdb.h>
#include <kks.h>

typedef struct {
	dev_t		devno;		/* Major/minor number of lft	    */
	char		devname[NAMESIZE]; /* lft device name		    */
} lft_dev_t;

/* ------------------------------------------------------------------------ *
 * The lft_kbd structure contains information about the keyboard	    *
 * ------------------------------------------------------------------------ */

typedef struct {
	dev_t		devno;		/* Major/minor number of the kbd    */
	char		devname[NAMESIZE]; /* Keyboard - device name	    */
	struct file	*fp;		/* File descriptor 		    */
	struct diacritic *diac;		/* Pointer to the diacritic table   */
	uint		kbd_type;	/* Type of keyboard 		    */
} lft_kbd_t;

/* ------------------------------------------------------------------------ */
/* Each display has a lft_disp structure.  This structure contains info-    */
/* rmation about the display device.					    */
/* ------------------------------------------------------------------------ */

typedef struct {
	dev_t		devno;		/* Major/minor number of display    */
	char		devname[NAMESIZE]; /* Logical name of display	    */
	int		font_index;	/* -1 -> use the 'best' font	    */
	struct file	*fp;		/* display - file descriptor	    */
	ushort		fp_valid;	/* Boolean: TRUE -> can use display */
	ushort		flags;		/* state flags 			    */	
	struct vtmstruc	*vtm_ptr;	/* Ptr to the virtual term. struct  */
} lft_disp_t;

/* ------------------------------------------------------------------------ */
/* The lft_dds structure is the device dependent structure.  Most of the    */
/* elements in this data structure are initialized by the build_dds         */
/* build_dds function in the lft configure method.                          */
/* ------------------------------------------------------------------------ */


typedef struct {
        lft_dev_t       lft;            /* lft devno and devname            */
        lft_kbd_t       kbd;            /* kbd devno, devname and file ptr  */
        int             number_of_displays;
        int             default_disp_index;
        char            *swkbd_file;	/* Ptr to name of swkbd file    */
        char            *font_file_name; /* Ptr to font file names	*/
        int             number_of_fonts;
        uint            start_fkproc;
	int		pwr_mgr_time[3]; /* array of Power Mgr timeouts */
	int		enable_dpms;     /* flag to enable/disable DPMS */
        lft_disp_t      displays[1]; 	/* array of disp_info structs*/

} lft_dds_t;

