/* @(#)85	1.5  src/bos/kernext/rcm/inc/rcmdds.h, rcm, bos41J, 9517B_all 4/25/95 13:43:21 */

/*
 * COMPONENT_NAME: (rcm) AIX Rendering Context Manager DDS struct definitions
 *
 * FUNCTIONS: none
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989-1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/types.h>
#include <sys/cfgdb.h>


/* -----------------------------------------------------------------
		DDS Structure for RCM

	This is the minimum size (one display).  It may be extended
	indefinitely for additional displays.
   -------------------------------------------------------------- */
typedef struct {
	int open_count;
	int number_of_displays;
	int unused[2];

	struct {
		struct file *fp;
		dev_t devno;
		int   unused[2];
	} lft_info;

	struct {
		dev_t devno;
		char lname[NAMESIZE];
		ulong flags;
#define  RCM_DEVNO_IN_USE	0x1
#define  RCM_DEVNO_MAKE_GP	0x2
#define  RCM_DRIVER_NOTOK	0x80000000
		pid_t pid;
		ulong handle;
		struct phys_displays *pd;
		tid_t tid;
		int   unused[1];
	} disp_info[1];

} rcm_dds, *rcm_ddsptr;
