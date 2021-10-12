/* @(#)97	1.2  src/bos/usr/include/glob.h, libcpat, bos411, 9428A410j 1/12/93 16:59:26 */

/*
 * COMPONENT_NAME: (LIBCPAT) Standard C Library Pattern Matching
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1991, 1992
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_GLOB
#define _H_GLOB

#ifndef _H_STANDARDS
#include <standards.h>
#endif

#ifndef _H_TYPES
#include <sys/types.h>		/* size_t */
#endif

/* glob() flags */

#ifdef _XOPEN_SOURCE

#define GLOB_APPEND	0x01	/* append to end of gl_pathv		*/
#define GLOB_DOOFFS	0x02	/* first allocate gl_offs NULL ptrs	*/
#define GLOB_ERR	0x04	/* return on error			*/
#define GLOB_MARK	0x08	/* add / to end of directory name	*/
#define GLOB_NOCHECK	0x10	/* return pattern if no matches		*/
#define GLOB_NOSORT	0x20	/* do not sort matched filenames	*/
#define GLOB_NOESCAPE	0x80	/* disable backslash escaping		*/

/* Pathname Matching error codes - large so not confused with errno.h	*/

#define GLOB_ABORTED	0x1000	/* error detected			*/
#define GLOB_NOSPACE	0x2000	/* memory allocation failure		*/
#define GLOB_NOMATCH	0x4000	/* pattern doesn't match any pathname	*/
#define GLOB_NOSYS	0x8000	/* this function is not supported	*/

typedef struct {
	size_t	gl_pathc;	/* matched pathname count (not gl_offs)	*/
	char	**gl_pathv;	/* ptr to list of matched pathnames	*/
	size_t	gl_offs;	/* # of gl_pathv reserved slots		*/
	void	*gl_padr;	/* ptr to pathname address structure	*/
	void	*gl_ptx;	/* ptr to first pathname text buffer	*/
} glob_t;

/* Pathname Matching function prototypes */

#ifdef _NO_PROTO
extern	int	glob();
extern	void	globfree();
#else
extern	int	glob(const char *, int, int (*)(), glob_t *);
extern	void	globfree(glob_t *);
#endif /* _NO_PROTO */
#endif /* _XOPEN_SOURCE */

#ifdef _ALL_SOURCE
#define GLOB_QUOTE	0x40	/* <backslash> protects next character	*/
#endif /* _ALL_SOURCE */

#endif /* _H_GLOB */
