/* @(#)28	1.14  src/bos/usr/include/malloc.h, libcgen, bos411, 9428A410j 3/4/94 11:07:20 */
/*
 * COMPONENT_NAME: (LIBCGEN) Standard C Library General Functions
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#ifndef _H_MALLOC
#define _H_MALLOC

/*
	Constants defining mallopt operations
*/
#define M_MXFAST	1	/* set size of blocks to be fast */
#define M_NLBLKS	2	/* set number of block in a holding block */
#define M_GRAIN		3	/* set number of sizes mapped to one, for
				   small blocks */
#define M_KEEP		4	/* retain contents of block after a free until
				   another allocation */
#define	M_DISCLAIM	5	/* disclaim free'd memory */
/*
	structure filled by mallinfo
*/
struct mallinfo  {
	int arena;	      /* total space in arena */
	int ordblks;	      /* number of ordinary blocks */
	int smblks;	      /* number of small blocks */
	int hblks;	      /* number of holding blocks */
	int hblkhd;	      /* space in holding block headers */
	int usmblks;	      /* space in small blocks in use */
	int fsmblks;	      /* space in free small blocks */
	int uordblks;	      /* space in ordinary blocks in use */
	int fordblks;	      /* space in free ordinary blocks */
	int keepcost;	      /* cost of enabling keep option */
#ifdef SUNINFO
	int mxfast;	      /* max size of small block */
	int nblks;	      /* number of small blocks in holding block */
	int grain;	      /* small block rounding factor */
	int uordbytes;	      /* space allocated in ordinary blocks */
	int allocated;	      /* number of ordinary blocks allocated */
	int treeoverhead;     /* bytes used in maintaining in free tree */
#endif
};	

#ifdef   _NO_PROTO
extern int mallopt();
extern struct mallinfo mallinfo();
#else  /*_NO_PROTO */
extern int mallopt(int, int);
extern struct mallinfo mallinfo(void);
#endif /*_NO_PROTO */
#endif /* _H_MALLOC */
