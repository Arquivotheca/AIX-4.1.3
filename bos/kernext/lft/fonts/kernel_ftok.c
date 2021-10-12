static char sccsid[] = "@(#)35	1.2  src/bos/kernext/lft/fonts/kernel_ftok.c, lftdd, bos411, 9428A410j 10/20/93 17:36:24";
/*
 *   COMPONENT_NAME: LFTDD
 *
 *   FUNCTIONS: kernel_ftok
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 *
 * Copyright (c) 1984 AT&T	
 * All Rights Reserved  
 *
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	
 * The copyright notice above does not evidence any   
 * actual or intended publication of such source code.
 *
 */

#include <lft.h>
#include <graphics/gs_trace.h>
#include <lft_debug.h>
#include <sys/stat.h>
#include <sys/fp_io.h>      /* not listed in manual ? */

/* #include <sys/sysmacros.h> */

/*
 * NAME:	kernel_ftok
 *                                                                    
 * FUNCTION:	generate a standard ipc key
 *                                                                    
 * NOTES:	Ftok generates a key based on 'path' and 'id'.  'Path'
 *		is the pathname of a currently existing file that is
 *		accessible to the process.  'Id' is a character that
 *		can cause different keys to be generated for the same
 *		'path'.
 *
 * RETURN VALUE DESCRIPTION:	-1 if the file does not exist or is
 *		not accessible to the process, -1 if 'id' is '\0',
 *		else the key
 *
 */  

/*---------------------------------------------------------------------------

       ftok is in libc.a library so kernel code can't link to it. 

       Because we want to create a shared memory segment which X server and
       Ped device driver can share,  the key used to create the shared
       memory segment (see shmget) has to be know by both.  It is agreed 
       upon that ftok() (same path and id) is used to create the key.  Since
       ftok() is in libc.a, kernel code can't like to it.  For this reason,
       we have to rewrite the original ftok() so that kernel code can 
       call it.  The list of changes are mentioned below:
 
          THE CHANGE IS THAT NOT TO USED STAT BUT FP_FSTAT TO ACHIEVE THE
          SAME EFFECT.  WE ALSO NEED TO USE FP_OPEN AND FP_CLOSE.
       
 --------------------------------------------------------------------------- */
key_t
kernel_ftok(path, id)
char *path;			/* pathname to base key on */
char id;			/* unique id per application */
{
	int rc;
	struct file * fp;
	struct stat st;
	key_t key;

	GS_ENTER_TRC0(HKWD_GS_LFT, kernel_ftok, 1, kernel_ftok);
	/* Leave keys 0 to (2^24-1) for SNA, other use */
	if(id == '\0') {
		GS_EXIT_TRC1(HKWD_GS_LFT, kernel_ftok, 1, kernel_ftok, -1);
		return((key_t)-1);
	}

	rc = fp_open(path,O_RDONLY,0,0,SYS_ADSPACE,&fp);	
	if (rc !=0) {
		GS_EXIT_TRC1(HKWD_GS_LFT, kernel_ftok, 1, kernel_ftok, -1);
		return((key_t)-1);
	}

	/*
	 * if the file is not accessible or doesn't exist, return -1.
	 * else use a combo of 'id', the minor device number of the
	 * device the file lives on, and the file's inode number to
	 * compute the key...
	 */

	if( fp_fstat(fp,&st,sizeof(st),SYS_ADSPACE) == 0)
        {
	    key = (key_t)(  (key_t)id << 24 | 
                            ((long)(unsigned)minor(st.st_dev)) << 16 |
		            (unsigned) st.st_ino ) ;
        } 
        else
        { 
           key = (key_t)-1;
        }

	fp_close(fp);

	GS_EXIT_TRC0(HKWD_GS_LFT, kernel_ftok, 1, kernel_ftok );
	return(key);
}
