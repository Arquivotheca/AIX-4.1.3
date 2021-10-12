/* @(#)99	1.11  src/bos/usr/include/nlist.h, cmdld, bos411, 9428A410j 3/4/94 11:09:03 */
#ifndef	_H_NLIST 
#define _H_NLIST
/*
 * COMPONENT_NAME: (CMDLD) XCOFF object file format definition
 *
 * FUNCTIONS: nlist.h 
 *
 * ORIGINS: 3, 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* symbol table entry structure */

struct nlist
{
	union
	{
		char	*_n_name;	/* symbol name */
	} _n;
	long		n_value;	/* value of symbol */
	short		n_scnum;	/* section number */
	union
	{
		unsigned short	_n_type; /* type and derived type */
	} _n_tylc;
	char		n_sclass;	/* storage class */
	char		n_numaux;	/* number of aux. entries */
};

/* include file <syms.h> also defines n_name and n_type. */
#ifndef	n_name
#define	n_name		_n._n_name
#endif	/* n_name */

#ifndef	n_type
#define n_type		_n_tylc._n_type
#endif /* n_type */

#ifdef _NO_PROTO
extern int nlist();
#else /* _NO_PROTO */
extern int nlist(const char *, struct nlist *);
#endif /* _NO_PROTO */


#endif /* _H_NLIST */
