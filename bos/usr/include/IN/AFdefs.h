/* @(#)80	1.10  src/bos/usr/include/IN/AFdefs.h, libIN, bos411, 9428A410j 6/7/91 09:38:47 */
/*
 * COMPONENT_NAME: LIBIN
 *
 * FUNCTIONS:
 *
 * ORIGINS: 9,10,27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#ifndef _H_AFDEFS 
#define _H_AFDEFS 

/*
 * Definitions to be included by programs that use "Attribute Files".
 * stdio.h must be included ahead of this include file.
 */

struct ATTR
{       char *  AT_name;
	char *  AT_value;
};

typedef struct ATTR * ATTR_t;

struct AFILE
{       FILE *  AF_iop;
	int     AF_rsiz;
	int     AF_natr;
	char *  AF_cbuf;
	char *  AF_dbuf;
	ATTR_t  AF_catr;
	ATTR_t  AF_datr;
};

typedef struct AFILE * AFILE_t;

#ifdef _NO_PROTO

extern AFILE_t AFopen();
extern AFclose();
extern AFrewind();
extern ATTR_t AFnxtrec();
extern ATTR_t AFgetrec();
extern ATTR_t AFfndrec();
extern char * AFgetatr();

#else /* ~ _NO_PROTO */

extern AFILE_t AFopen(char *filename, int maxrecsiz, int maxnumatr);
extern AFclose(register AFILE_t af);
extern AFrewind(AFILE_t af);
extern ATTR_t AFnxtrec(register AFILE_t af);
extern ATTR_t AFgetrec(register AFILE_t af, char *name);
extern ATTR_t AFfndrec(register AFILE_t af, ATTR_t pattern);
extern char * AFgetatr(register ATTR_t at, char *name);

#endif /* _NO_PROTO */
#endif /* _H_AFDEFS */
