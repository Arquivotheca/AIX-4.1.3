/* @(#)96	1.10  src/bos/usr/include/IN/DRdefs.h, libIN, bos411, 9428A410j 3/22/93 11:32:22 */
/*
 * COMPONENT_NAME: LIBIN
 *
 * FUNCTIONS:
 *
 * ORIGINS: 9,27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#ifndef _H_DRDEFS
#define _H_DRDEFS

#include <sys/limits.h>
#include <sys/dir.h>
#ifndef NAME_MAX
#define NAME_MAX 255
#endif


/*
 * Directory entry as returned from DRget
 */

struct  dir_ent {
	ino_t   dir_ino;
	char    dir_name[NAME_MAX+1];
};

/*
 * Work area for DRget
 */

struct dir_data {
	char    dir_flags;
	DIR     *dir_p;
	long    dir_pos;
};

#define DIR_LDOT   1    /* return entries starting with '.' */
#define DIR_CLOSE  2    /* keep directory closed between calls */
#define DIR_NOSTAT 0x40 /* don't check to see if file is really a directory */

extern int DRstart();
extern void DRend();
extern struct dir_ent *DRget();

#endif /* _H_DRDEFS */
