static char sccsid[] = "@(#)82	1.16  src/bos/usr/ccs/lib/libc/NLscanf.c, libcio, bos411, 9428A410j 2/26/91 13:40:51";
/*
 * COMPONENT_NAME: (LIBCIO) Standard C Library I/O Functions 
 *
 * FUNCTIONS: NLscanf, NLfscanf, NLsscanf 
 *
 * ORIGINS: 3, 27 
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1987, 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <stdarg.h>

/* 
 * EXTERNAL PROCEDURE CALLED
 */

extern int _doscan();

/*
 * NAME: NLscanf 
 *
 * FUNCTION: Reads character data including NLchars from stdin,    
 *           according to fmt and save data to va_alist. 
 *
 * NOTE:     extern _doscan() does all the scan and match.
 *
 * RETURN VALUE DESCRIPTION: Returns the number of matches found between
 *           fmt and va_alist, and EOF on the end of the input. 
 */

int
NLscanf(unsigned char *fmt, ...) 
{
        va_list ap;     /* the argument list */ 

	va_start(ap, fmt);	/* initialize ap */ 
	return(_doscan(stdin, fmt, ap));
}

/*
 * NAME: NLfscanf 
 *
 * FUNCTION: Reads character data including NLchars from stream iop,   
 *           according to fmt and save data to va_alist. 
 *
 * NOTE:     extern _doscan() does all the scan and match.
 *
 * RETURN VALUE DESCRIPTION: Returns the number of matches found between
 *           fmt and va_alist, and EOF on the end of the input. 
 */

int
NLfscanf(FILE *iop, unsigned char *fmt, ...)
/* FILE *iop		 an input file */
/* char *fmt       	 a conversion specifications */
{
        va_list ap;	/* argument list */

	va_start(ap, fmt);   /* initialize ap */
	return(_doscan(iop, fmt, ap));
}

/*
 * NAME: NLsscanf 
 *
 * FUNCTION: Read character data including NLchars from str according
 *           to fmt and save data to va_alist.  
 *
 * NOTE:     extern _doscan() does all the scan and match.
 *
 * RETURN VALUE DESCRIPTION: Returns the number of matches found between
 *           fmt and va_alist, and EOF on the end of the input. 
 */

int
NLsscanf(unsigned char *str, unsigned char *fmt, ...)
/* char *str	 	an input string */
/* char *fmt           the conversion specifications */  
{
	va_list ap;	/* argument list */
	FILE strbuf;    /* temporary file */

	va_start(ap, fmt);	/* initialize ap */

/*  Convert data type from char to FILE */ 
	strbuf._flag = (_IOREAD | _IONOFD);
	strbuf.__newbase = strbuf._ptr = strbuf._base = (unsigned char*)str;
	
	strbuf._cnt = strlen(str);
	strbuf._file = _NFILE;
	return(_doscan(&strbuf, fmt, ap));
}
