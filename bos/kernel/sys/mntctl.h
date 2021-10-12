/* @(#)73	1.8  src/bos/kernel/sys/mntctl.h, syslfs, bos411, 9428A410j 12/9/92 08:14:14 */

#ifndef _H_MNTCTL
#define _H_MNTCTL

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

#include <sys/vmount.h>
#include <sys/types.h>

/*
 * mntctl.h
 */

/* this is the list of possible mount control commands                  */
#define MC_MOUNTS       0       /* get mount status information         */
#define MC_UMOUNTS	1	/* force unmout of particular mount point
				 * and all mount points below it.
				 */

/* This is the structure of the mount information that is returned
   from the mntctl() system call.  An array of these structures
   immediately follows the buffer's header (of struct bheader type).
   When the mntctl() system call returns successfully, the number of
   elements in the array is contained in the buffer header field size.  */

struct  minfo
{
        char         *m_object; /* path of the mounted object           */
        char         *m_stub;   /* path of the mounted-over object      */            
        unsigned int m_flag;    /* flag indicating whether a vmount is
                                   read-only, removable, device, or
                                   remote                               */
        time_t       m_date;    /* date that vfs was created            */
};

/* the mntctl buffer must have this structure as its header             */
struct bheader
{
        int           reserved; /* for internal use of system call      */
        unsigned int  size;     /* # of minfo structures in buffer OR
                                   required size of buffer              */
        struct minfo  m[1];     /* start of minfo descriptions          */
} ;

/* The search info struct is used internally by mntctl as a handy place
 * to stick things that we need to keep track of during the search of the
 * VFS tree.
 */
struct srch_info {		
	int	s_mcount;	/* number of mounts */
	int	s_bcount;	/* number of bytes we need to hold the object */
				/* and stub path names.			      */
	struct minfo *s_mptr;	/* pointer to the minfo structs in bheader    */
	char	*s_strptr;	/* pointer to string space in buf	      */
};

/* Flags used by the search routine */
#define	COUNT		0x01		/* Count the nodes */
#define	F_UNMOUNT	0x02		/* Force unmount all nodes */

#endif  /* _H_MNTCTL */
