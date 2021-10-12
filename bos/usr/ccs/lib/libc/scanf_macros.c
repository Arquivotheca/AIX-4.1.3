/* "@(#)56	1.4  src/bos/usr/ccs/lib/libc/scanf_macros.c, libcio, bos411, 9428A410j 4/20/94 17:52:40" */
/*
 * COMPONENT_NAME: (LIBCIO) Standard C Library I/O Functions 
 *
 * FUNCTIONS: scanf, fscanf, sscanf, wsscanf
 *
 * ORIGINS: 3, 27 
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <stdarg.h>
#include "ts_supp.h"
#include "push_pop.h"

#ifdef _THREAD_SAFE
#include "stdio_lock.h"
#endif /* _THREAD_SAFE */

extern int _doscan();
extern int _doscan128();

/* 
 * This file builds versions of the following scan functions:
 *
 * scanf
 * fscanf
 * sscanf
 * wsscanf
 *
 * This file is not intended to be compiled directly.  Rather,
 * the following symbol are defined to provide the names
 * of the functions to be built.  Another file will define
 * the appropriate symbol and then #include this file.
 *
 * SCANF
 * FSCANF
 * SSCANF
 * WSSCANF
 *
 * The symbol DOSCAN must be defined to what version of _doscan
 * should be used, either _doscan or _doscan128.
 */

/*
 * FUNCTION:	The scanf function reads input from stdin, under control
 *		of the string pointed to by format that specifies the 
 *		admissible input sequences and how they are to be converted
 *		for assignment.
 *
 * RETURN VALUE DESCRIPTION:	
 *		returns the value of EOF if an input failure occurs before
 *		any conversion.  Otherwise, the fscanf function returns
 *		the number of input items assigned.
 *
 */                                                                   
/* the following are supplied so users can create

 * The *64 and *128 functions allow a user to create a mixed
 * 64-bit / 128-bit long double application.
 */

int	
SCANF(const char *format, ...) 
{
	va_list ap;
	register int rc;
	TS_FDECLARELOCK(filelock);

	va_start(ap,format);
	TS_FLOCK(filelock,stdin);

	TS_PUSH_CLNUP(filelock);
	rc = DOSCAN(stdin, format, ap);
	TS_POP_CLNUP(0);

	TS_FUNLOCK(filelock);
	return(rc);
}


int	
FSCANF(FILE *stream, const char *format, ...)
{
	va_list ap;
	register int rc;
	TS_FDECLARELOCK(filelock);

	va_start(ap,format);
	TS_FLOCK(filelock,stream);

	TS_PUSH_CLNUP(filelock);
	rc = DOSCAN(stream, format, ap);
	TS_POP_CLNUP(0);

	TS_FUNLOCK(filelock);
	return(rc);
}

int	
SSCANF(const char *s, const char *format, ...) 
{
	va_list ap;
	FILE strbuf;

	va_start(ap,format);
	strbuf._flag = (_IOREAD|_IONOFD);
	strbuf._ptr = strbuf.__newbase = strbuf._base = (unsigned char*)s;
	strbuf._cnt = strlen(s);
#ifdef _THREAD_SAFE
	strbuf._lock = NULL;
#endif /* _THREAD_SAFE */
	return(DOSCAN(&strbuf, format, ap));
}

#include <stddef.h>

int	
WSSCANF(const wchar_t *s, const char *format, ...) 
{
	va_list ap;
	FILE strbuf;
	unsigned char *buf;
	int len;

	len = wstrlen (s);
	buf = malloc (len * sizeof(wchar_t)+1);
	if (buf == NULL)
	    return (EOF);

	wstrtos (buf, s);

	va_start(ap,format);
	strbuf._flag = (_IOREAD|_IONOFD);
	strbuf.__newbase = strbuf._ptr = strbuf._base = buf;
	strbuf._cnt = strlen(buf);

#ifdef _THREAD_SAFE
	strbuf._lock = NULL;
#endif /* _THREAD_SAFE */
	len = DOSCAN(&strbuf, format, ap);

	free (buf);
	return (len);
}

