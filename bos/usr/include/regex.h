/* @(#)21	1.2.1.7  src/bos/usr/include/regex.h, libcpat, bos41J, 9511A_all 2/22/95 19:37:11 */

/*
 * COMPONENT_NAME: (LIBCPAT) Standard C Library Regular Expression
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27,71
 *
 * (C) COPYRIGHT International Business Machines Corp. 1991, 1995
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */

#ifndef _H_REGEX
#define _H_REGEX

#ifndef _H_STANDARDS
#include <standards.h>
#endif /* _H_STANDARDS */

#ifndef _H_TYPES
#include <sys/types.h>
#endif /* _H_TYPES */

#define _REG_SUBEXP_MAX	23	/* Maximum # of subexpressions		*/

#ifdef _XOPEN_SOURCE

typedef	long	regoff_t;

/* regcomp() cflags */

#define	REG_EXTENDED	0x001	/* Use Extended RE syntax rules		*/
#define REG_ICASE	0x002	/* Ignore case in match			*/
#define REG_NEWLINE	0x004	/* Convert <backslash><n> to <newline>	*/
#define REG_NOSUB	0x008	/* regexec() not report subexpressions	*/


/* regexec() eflags */

#define	REG_NOTBOL	0x100	/* First character not start of line	*/
#define REG_NOTEOL	0x200	/* Last character not end of line	*/


/* Regular Expression error codes */

#define	REG_NOMATCH	1	/* RE pattern not found			*/
#define	REG_BADPAT	2	/* Invalid Regular Expression		*/
#define	REG_ECOLLATE	3	/* Invalid collating element		*/
#define	REG_ECTYPE	4	/* Invalid character class		*/
#define	REG_EESCAPE	5	/* Last character is \			*/
#define	REG_ESUBREG	6	/* Invalid number in \digit		*/
#define	REG_EBRACK	7	/* [] imbalance				*/
#define	REG_EPAREN	8	/* \( \) or () imbalance		*/
#define	REG_EBRACE	9	/* \{ \} or { } imbalance		*/
#define	REG_BADBR	10	/* Invalid \{ \} range exp		*/
#define	REG_ERANGE	11	/* Invalid range exp endpoint		*/
#define	REG_ESPACE	12	/* Out of memory			*/
#define	REG_BADRPT	13	/* ?*+ not preceded by valid RE		*/
#define REG_ECHAR	14	/* invalid multibyte character		*/
#define REG_EBOL	15	/* ^ anchor and not BOL			*/
#define REG_EEOL	16	/* $ anchor and not EOL			*/
#define	REG_ENOSYS	17	/* the function is not implemented	*/

/* NOTE:  The size of regex_t must not change.  The size of the members
 *	  __re_lsub + __re_esub + __re_map + __maxsub + __unsed must be 
 *	  exactly 336 bytes, so if _REG_SUBEXP_MAX above changes, you'll
 *	  have to modify __unsed accordingly.
 * ALSO:  See notes in __regexec_std.c regarding _REG_SUBEXP_MAX.
 */

typedef struct {		/* regcomp() data saved for regexec()	*/
	size_t	re_nsub;	/* # of subexpressions in RE pattern	*/
#ifdef _ALL_SOURCE
	void	*re_comp;	/* compiled RE; freed by regfree()	*/
	int	re_cflags;	/* saved cflags for regexec()		*/
	size_t	re_erroff;	/* RE pattern error offset		*/
	size_t	re_len;		/* # wchar_t chars in compiled pattern	*/
	wchar_t	re_ucoll[2];	/* min/max unique collating values	*/
	void	*re_lsub[_REG_SUBEXP_MAX+1]; /* start subexp		*/
	void	*re_esub[_REG_SUBEXP_MAX+1]; /* end subexp		*/
	uchar_t	*re_map;	/* map of valid pattern characters	*/
#else /* _ALL_SOURCE */
	void	*__re_comp;	/* compiled RE; freed by regfree()	*/
	int	__re_cflags;	/* saved cflags for regexec()		*/
	size_t	__re_erroff;	/* RE pattern error offset		*/
	size_t	__re_len;	/* # wchar_t chars in compiled pattern	*/
	wchar_t	__re_ucoll[2];	/* min/max unique collating values	*/
	void	*__re_lsub[_REG_SUBEXP_MAX+1]; /* start subexp		*/
	void	*__re_esub[_REG_SUBEXP_MAX+1]; /* end subexp		*/
	uchar_t	*__re_map;	/* map of valid pattern characters	*/
#endif /* _ALL_SOURCE */
	int	__maxsub;	/* maximum number of subs in pattern.   */
	void	*__unused[34];	/* Extra space if ever needed		*/
} regex_t;


typedef struct {		/* substring locations - from regexec()	*/
	regoff_t  rm_so;	/* Byte offset from start of string to	*/
				/*   start of substring			*/
	regoff_t  rm_eo;	/* Byte offset from start of string of	*/
				/*   first character after substring	*/
} regmatch_t;


/* Regular Expression function prototypes */

#ifdef _NO_PROTO
extern	int 	regcomp();
extern	int	regexec();
extern	size_t	regerror();
extern	void	regfree();
#else /* _NO_PROTO */
extern	int 	regcomp(regex_t *, const char *, int);
extern	int	regexec(const regex_t *, const char *, size_t, regmatch_t *, int);
extern	size_t	regerror(int, const regex_t *, char *, size_t);
extern	void	regfree(regex_t *);
#endif /* _NO_PROTO */

#ifdef _THREAD_SAFE

#define	ESIZE	512
#define	NBRA	9

typedef	struct regex_data {
	char	expbuf[ESIZE],
		*braslist[NBRA],
		*braelist[NBRA],
		circf;
} REGEXD;

#ifdef	_NO_PROTO
extern	char	*re_comp_r();
extern	int	re_exec_r();
#else /* _NO_PROTO */
extern	char	*re_comp_r(const char *sp, REGEXD *rd);
extern	int	re_exec_r(const char *p1, REGEXD *rd);
#endif	/* _NO_PROTO */
#endif /* _THREAD_SAFE */

#endif /* _XOPEN_SOURCE */

#ifdef _ALL_SOURCE
#ifdef _NO_PROTO
extern char	*re_comp();
extern int	re_exec();
#else /* _NO_PROTO */
extern char	*re_comp(const char *);
extern int	re_exec(const char *);
#endif /* _NO_PROTO */
#endif /* _ALL_SOURCE */

#ifdef _ALL_SOURCE

#ifndef _H_LIMITS
#include <limits.h>
#endif /* _H_LIMITS */

#ifdef	_NO_PROTO
extern	char	*regex();
extern	char	*regcmp();
#else /* _NO_PROTO */
extern	char	*regex(char *, char *, char *);
extern	char	*regcmp(int);
#endif	/* _NO_PROTO */

#endif /* _ALL_SOURCE */

#endif /* _H_REGEX */
