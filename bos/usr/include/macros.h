/* @(#)18	1.7  src/bos/usr/include/macros.h, incstd, bos411, 9428A410j 6/24/94 13:35:11 */
/* macros.h	5.1 - 86/12/09 - 06:04:58 */
#ifndef _H_MACROS
#define _H_MACROS

/*
 * COMPONENT NAME: (INC)  System header files
 *
 * FUNCTIONS: none
 *
 * ORIGINS:  27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1986, 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
	numeric() is useful in while's, if's, etc., but don't use *p++
	max() and min() depend on the types of the operands
	abs() is absolute value
*/

# define numeric(c)		((c) >= '0' && (c) <= '9')
# define max(a,b)		((a)<(b) ? (b) : (a))
# define min(a,b)		((a)>(b) ? (b) : (a))
# define abs(x)			((x)>=0 ? (x) : -(x))

# define copy(srce,dest)	cat(dest,srce,0)
# define compare(str1,str2)	strcmp(str1,str2)
# define equal(str1,str2)	!strcmp(str1,str2)
# define length(str)		strlen(str)
# define size(str)		(strlen(str) + 1)

/*
	The global variable Statbuf is available for use as a stat(II)
	structure.  Note that "stat.h" is included here and should
	not be included elsewhere.
	Exists(file) returns 0 if the file does not exist;
	the flags word if it does (the flags word is always non-zero).
*/
# include <sys/stat.h>
extern struct stat Statbuf;
# define exists(file)		(stat(file,&Statbuf)<0 ? 0:Statbuf.st_mode)

extern long itol();
/*
	libS.a interface for xopen() and xcreat()
*/
# define xfopen(file,mode)	fdfopen(xopen(file,mode),mode)
# define xfcreat(file,mode)	fdfopen(xcreat(file,mode),1)

# define remove(file)		xunlink(file)

/*
	SAVE() and RSTR() use local data in nested blocks.
	Make sure that they nest cleanly.
*/
# define SAVE(name,place)	{ int place = name;
# define RSTR(name,place)	name = place;}

/*
	Use: DEBUG(sum,d) which becomes fprintf(stderr,"sum = %d\n",sum)
*/
# ifndef _KERNSYS
# define DEBUG(variable,type)	fprintf(stderr,"variable = %type\n",variable)
#endif


/*
	Use of ERRABORT() will cause libS.a internal
	errors to cause aborts
*/
# define ERRABORT()	_error() { abort(); }

/*
	Use of USXALLOC() is required to force all calls to alloc()
	(e.g., from libS.a) to call xalloc().
*/
# define USXALLOC() \
		char *alloc(n) {return((char *)xalloc((unsigned)n));} \
		free(n) char *n; {xfree(n);} \
		char *malloc(n) unsigned n; {int p; p=xalloc(n); \
			return((char *)(p != -1?p:0));}

# define NONBLANK(p)		while (*(p)==' ' || *(p)=='\t') (p)++

/*
	A global null string.
*/
extern char	Null[1];

/*
	A global error message string.
*/
extern char	Error[128];

#endif /* _H_MACROS */
