/* @(#)21	1.11  src/bos/kernel/sys/gpai.h, syslfs, bos411, 9428A410j 3/23/94 10:31:05 */

#ifndef _H_GPAI
#define _H_GPAI

/*
 * COMPONENT_NAME: SYSLFS - Logical File System
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "sys/types.h"
#include "sys/lock_def.h"

struct galloc
{      
	/* The fields down to a_freeobj are set up with static */
	/* initialization before the call to gpai_init.        */
	ushort      a_osz;	    /* object size             */
	ushort      a_pas;          /* primary allocation size */
	/* The following fields are used for lock allocation   */
	/* and initialization.                                 */
	int         a_lkoffset;     /* offset in struct to per-struct lock *
				     * (a value of zero indicates no lock) */
	int         a_lktype;       /* lock type: 0=simple, 1=complex      */
	short       a_lkclass;      /* lock class for lock_alloc           */
	short       a_lkoccur;      /* lock occurrence / allocation count  */
	/* The remaining fields are used at runtime.           */
	caddr_t     a_freeobj;      /* pointer to first free obj */
	caddr_t     a_sat;          /* allocation pool ptr */
	Simple_lock a_lock;         /* serialization lockword */
	ulong       a_inuse;        /* number of objects currently in use  */
};

#define objsize(x)      ((ulong)((x)->a_osz)+sizeof(caddr_t))
#define freeobj(x)      ((x)->a_freeobj)

/* Lock type defines */
#define GPA_SIMPLE	0
#define GPA_COMPLEX	1

/* gpai function prototypes */
void     gpai_init(struct galloc *);  /* pointer to allocation structure */

caddr_t *gpai_alloc(struct galloc *); /* pointer to allocation structure */

void     gpai_free(struct galloc *,   /* pointer to allocation structure */
		   caddr_t *);	      /* pointer to object to be freed   */

char    *gpai_srch(struct galloc *,   /* pointer to pool anchor */
		 int,                 /* specific type of object in list */
		 int,                 /* offset in bytes to type in object */
		 int (*)(),           /* pointer to caller's search routine */
		 caddr_t);            /* pointer to search routine args */


#endif /* _H_GPAI */
