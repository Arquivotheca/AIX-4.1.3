/* "@(#)50	1.5  src/bos/usr/ccs/lib/libc/printf_macros.c, libcprnt, bos411, 9428A410j 4/20/94 17:50:42" */
/*
 * COMPONENT_NAME: (LIBCPRNT) Standard C Library Print Functions 
 *
 * FUNCTIONS: printf 
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

/*LINTLIBRARY*/

#ifdef _THREAD_SAFE
# ifndef _STDIO_UNLOCK_CHAR_IO
# define _STDIO_UNLOCK_CHAR_IO
# endif
#include "stdio_lock.h"
#endif /* _THREAD_SAFE */

#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <values.h>
#include "ts_supp.h"
#include "push_pop.h"

extern int _doprnt();
extern int _doprnt128();

/* 
 * This file builds versions of the following print functions:
 *
 * printf
 * fprintf
 * sprintf
 * vfprintf
 * vprintf
 * vsprintf
 * vwsprintf
 * wsprintf
 *
 * This file is not intended to be compiled directly.  Rather,
 * the following symbols are defined to provide the names
 * of the functions to be built.  Another file will define
 * the appropriate symbols and then #include this file.
 *
 * PRINTF
 * FPRINTF
 * SPRINTF
 * VFPRINTF
 * VPRINTF
 * VSPRINTF
 * VWSPRINTF
 * WSPRINTF
 *
 * The symbol DOPRNT must be defined to what version of _doprnt
 * should be used, either _doprnt or _doprnt128.
 */

/*                                                                    
 * FUNCTION: writes output to stdout under control of the string pointed
 *           to by format that specifies how subsequent argumnts are
 *           converted for output.
 *
 * PARAMETERS: format - format used to print arguments
 *	       ...    -   arguments to be printed
 *
 * RETURN VALUE DESCRIPTIONS:
 * 	      If successful, returns number of characters printed
 *	      Otherwise returns negative value
 */

int	
PRINTF(const char *format, ...) 
{
	int count;
	va_list ap;
	register int rc;
	TS_FDECLARELOCK(filelock);

	va_start(ap,format);

	TS_FLOCK(filelock, stdout);

	TS_PUSH_CLNUP(filelock);

	count = DOPRNT(format, ap, stdout);

	TS_POP_CLNUP(0);

	va_end(ap);
	rc = ferror(stdout)? EOF: count;

	TS_FUNLOCK(filelock);

	return(rc);
}

/*                                                                    
 * FUNCTION: Writes output to to the stream pointed to by stream, under
 *           control of the string pointed to by format, that specifies
 *           how subsequent argumnts are converted for output.
 *
 * PARAMETERS: stream - stream to be printed to
 *             format - format used to print arguments
 *	       ...    -   arguments to be printed
 *
 * RETURN VALUE DESCRIPTIONS:
 * 	      If successful, returns number of characters printed
 *	      Otherwise returns negative value
 */

int	
FPRINTF(FILE *stream, const char *format, ...) 
{
	int count;
	va_list ap;
	register int rc;
	TS_FDECLARELOCK(filelock);

	va_start(ap,format);

	TS_FLOCK(filelock, stream);

	if (!(stream->_flag & _IOWRT)) {
		/* if no write flag */
		if (stream->_flag & _IORW) {
			/* if ok, cause read-write */
			stream->_flag |= _IOWRT;
		} else {
			/* else error */
			TS_FUNLOCK(filelock);
			errno=EBADF;
			return EOF;
		}
	}

	TS_PUSH_CLNUP(filelock);
	count = DOPRNT(format, ap, stream);
	TS_POP_CLNUP(0);

	va_end(ap);
	rc = ferror(stream)? EOF: count;
	TS_FUNLOCK(filelock);
	return(rc);
}

/*                                                                    
 * FUNCTION: Writes output to the array specified by s, under
 *           control of the string pointed to by format, that specifies
 *           how subsequent argumnts are converted for output.
 *
 * PARAMETERS: s      - array to be printed to
 *             format - format used to print arguments
 *	       ...    -   arguments to be printed
 *
 * RETURN VALUE DESCRIPTIONS:
 * 	      Returns number of characters printed
 */                                                                   

int	
SPRINTF(char *s, const char *format, ...) 
{
	int count;
	FILE siop;
	va_list ap;

#ifdef _THREAD_SAFE
	siop._lock = NULL;
#endif /* _THREAD_SAFE */

	siop._cnt = INT_MAX;
	siop._base = siop._ptr = (unsigned char *)s;
	siop._flag = (_IOWRT|_IONOFD);
	va_start(ap,format);
	count = DOPRNT(format, ap, &siop);
	va_end(ap);
	*siop._ptr = '\0'; /* plant terminating null character */
	return(count);
}

/*                                                                    
 * FUNCTION: Writes output to to the stream pointed to by stream, under
 *           control of the string pointed to by format, that specifies
 *           how subsequent argumnts are converted for output.
 *
 * PARAMETERS: stream - stream to be printed to
 *             format - format used to print arguments
 *	       arg   -   arguments to be printed
 *
 * RETURN VALUE DESCRIPTIONS:
 * 	      If successful, returns number of characters printed
 *	      Otherwise returns negative value
 */

int	
VFPRINTF(FILE *stream, const char *format, va_list arg)
{       
	int count;
	register int rc;
	TS_FDECLARELOCK(filelock);

	TS_FLOCK(filelock, stream);

	TS_PUSH_CLNUP(filelock);
	count = DOPRNT (format, arg, stream);
	TS_POP_CLNUP(0);

	rc = ferror(stream)? EOF: count;
	TS_FUNLOCK(filelock);
	return(rc);
}

/*                                                                    
 * FUNCTION: Writes output to stdout under control of the string pointed
 *           to by format that specifies how subsequent argumnts are
 *           converted for output.
 *
 * PARAMETERS: format - format used to print arguments
 *	       arg   - arguments to be printed
 *
 * RETURN VALUE DESCRIPTIONS:
 * 	      If successful, returns number of characters printed
 *	      Otherwise returns negative value
 */                                                                   

int	
VPRINTF(const char *format, va_list arg) 
{       
	int count;
	register int rc;
	TS_FDECLARELOCK(filelock)

	TS_FLOCK(filelock, stdout);


	TS_PUSH_CLNUP(filelock);
	count = DOPRNT (format, arg, stdout);
	TS_POP_CLNUP(0);

	rc = ferror(stdout)? EOF: count;
	TS_FUNLOCK(filelock);
	return(rc);
}
/*                                                                    
 * FUNCTION: Writes output to to the array pointed to by s, under
 *           control of the string pointed to by format, that specifies
 *           how subsequent argumnts are converted for output.
 *
 * PARAMETERS: s      - array to be printed to
 *             format - format used to print arguments
 *	       arg   - arguments to be printed
 *
 * RETURN VALUE DESCRIPTIONS:
 * 	      If successful, returns number of characters printed
 *	      Otherwise returns negative value
 */                                                                   

int	
VSPRINTF(char *s, const char *format, va_list arg)
{
	FILE siop;
	int rc;

#ifdef _THREAD_SAFE 
	siop._lock = NULL;
#endif /* _THREAD_SAFE */

	siop._cnt = INT_MAX;
	siop._base = siop._ptr = (unsigned char *)s;
	siop._flag = (_IOWRT|_IONOFD);
	rc = DOPRNT(format, arg, &siop);
	*siop._ptr = '\0';
	return (rc);
}

/*                                                                    
 * FUNCTION: Writes output to to the array pointed to by wcs, under
 *           control of the string pointed to by format, that specifies
 *           how subsequent argumnts are converted for output.
 *
 * PARAMETERS: wcs    - array to be printed to
 *             format - format used to print arguments
 *	       arg   - arguments to be printed
 *
 * RETURN VALUE DESCRIPTIONS:
 * 	      If successful, returns number of wide characters printed
 *	      Otherwise returns negative value
 */                                                                   

int	
VWSPRINTF(wchar_t *wcs, const char *format, va_list arg)
{
	FILE siop;
	int count;
	wchar_t* buf;

#ifdef _THREAD_SAFE
	siop._lock = NULL;
#endif /* _THREAD_SAFE */

	siop._cnt = INT_MAX;
	siop._base = siop._ptr = (unsigned char *)wcs;
	siop._flag = (_IOWRT|_IONOFD);
	DOPRNT(format, arg, &siop);
	*siop._ptr = '\0';	/* plant terminating null character */
        count = strlen((char*) wcs) + 1;
        buf = (wchar_t*) malloc(sizeof(wchar_t) * count);
	/* check result of malloc - d079359 */
	if (buf == NULL) {
	    errno = ENOMEM;
	    return (-1);
	}
        count = mbstowcs(buf, (char*) wcs, count);
        wstrcpy (wcs, buf);
        free (buf);
        return(count);
}

/*                                                                    
 * FUNCTION: Writes output to the wide character array specified by ws, under
 *           control of the string pointed to by format, that specifies
 *           how subsequent argumnts are converted for output.
 *
 * PARAMETERS: ws     - wide character array to be printed to
 *             format - format used to print arguments
 *	       ...    -   arguments to be printed
 *
 * RETURN VALUE DESCRIPTIONS:
 * 	      Returns number of characters printed
 */                                                                   

int	
WSPRINTF(wchar_t *wcs, const char *format, ...) 
{
	int count;
	FILE siop;
	va_list ap;
	wchar_t* buf;

#ifdef _THREAD_SAFE
	siop._lock = NULL;
#endif /* _THREAD_SAFE */

	siop._cnt = INT_MAX;
	siop._base = siop._ptr = (unsigned char *)wcs;
	siop._flag = (_IOWRT|_IONOFD);
	va_start(ap,format);
	DOPRNT(format, ap, &siop);
	va_end(ap);
	*siop._ptr = '\0'; 	/* plant terminating null character */
	count = strlen((char*) wcs) + 1;
	buf = (wchar_t*) malloc((count)*sizeof(wchar_t));
	/* check result of malloc - d079358 */
	if (buf == NULL) {
	    errno = ENOMEM;
	    return (-1);
	}
	count = mbstowcs(buf, (char*) wcs, count);
	wstrcpy (wcs, buf); 
	free (buf);
	return(count);
}
