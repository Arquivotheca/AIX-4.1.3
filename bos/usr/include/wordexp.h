/* @(#)03	1.2  src/bos/usr/include/wordexp.h, libcpat, bos411, 9428A410j 1/12/93 17:03:53 */

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

#ifndef _H_WORDEXP
#define _H_WORDEXP

#ifndef _H_TYPES
#include <sys/types.h>
#endif

/* wordexp() flags */

#define WRDE_APPEND	0x01	/* append to end of we_wordv		*/
#define WRDE_DOOFFS	0x02	/* first allocate we_offs NULL ptrs	*/
#define WRDE_NOCMD	0x04	/* error on command substitution	*/
#define WRDE_REUSE	0x08	/* first call wordfree()		*/
#define WRDE_SHOWERR	0x10	/* do not redirect stderr to /dev/null	*/
#define WRDE_UNDEF	0x20	/* undefined shell variable is error	*/

/* Word Expansion error codes */

#define WRDE_BADCHAR	1	/* unquoted special character		*/
#define WRDE_BADVAL	2	/* undefined shell variable		*/
#define WRDE_CMDSUB	3	/* command substitution not permitted	*/
#define WRDE_NOSPACE	4	/* memory allocation failure		*/
#define WRDE_SYNTAX	5	/* shell syntax error			*/
#define WRDE_EPOPEN	6	/* popen() failure			*/
#define WRDE_ESHELL	7	/* error reading psh data		*/
#define WRDE_NOSYS	8	/* the function is not supported	*/

typedef struct {
	size_t	we_wordc;	/* expanded word count (not we_offs)	*/
	char	**we_wordv;	/* ptr to list of expanded words	*/
	size_t	we_offs;	/* # of we_wordv reserved slots		*/
	int	we_sflags;	/* saved flags for wordfree()		*/
	int	we_soffs;	/* saved we_offs for wordfree()		*/
} wordexp_t;

/* Word Expansion function prototypes */

#ifdef _NO_PROTO
extern	int	wordexp();
extern	void	wordfree();
#else
extern	int	wordexp(const char *, wordexp_t *, int);
extern	void	wordfree(wordexp_t *);
#endif /* _NO_PROTO */

#endif /* _H_WORDEXP */
