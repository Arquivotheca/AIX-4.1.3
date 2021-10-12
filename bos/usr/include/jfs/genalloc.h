/* @(#)43	1.2  src/bos/usr/include/jfs/genalloc.h, syspfs, bos411, 9428A410j 12/9/92 08:13:36 */
/*
 * COMPONENT_NAME: (SYSPFS) Physical File System
 *
 * FUNCTIONS: genalloc.h
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_JFS_GENALLOC
#define _H_JFS_GENALLOC

#include <sys/types.h>

/* Common table management for large tables, genalloc structure should 
 * be kept to # of words that will divide evenly into PAGESIZE.
 */
struct genalloc
{	char	a_handle[8];		/* Object id			*/
	caddr_t	a_head;			/* Head of the free list	*/
	caddr_t	a_hiwater;		/* Current high water mark	*/
	caddr_t	a_table;		/* Static table			*/
	caddr_t	a_end;			/* Last+1 table element		*/
	int	a_osz;			/* Object size in bytes		*/
	int	a_froff;		/* Saved offset of free pointer */
};

typedef	struct genalloc genalloc_t;

#define objsize(x)      ((x)->a_osz)
#define freeobj(x)      ((x)->a_head)
#define eotbl(x)      	((x)->a_end)

#endif /* _H_JFS_GENALLOC */
