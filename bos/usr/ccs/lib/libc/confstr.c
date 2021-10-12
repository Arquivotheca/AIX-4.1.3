static char sccsid[] = "@(#)80	1.3  src/bos/usr/ccs/lib/libc/confstr.c, libcgen, bos411, 9428A410j 1/12/93 11:13:00";
/*
 * COMPONENT_NAME: (LIBCGEN) Standard C Library General Functions
 *
 * FUNCTIONS: confstr
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989,1992
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <unistd.h>
#include <errno.h>

size_t
confstr(int name, char *buf, size_t len)
{ 
	size_t return_len;
	switch (name)
	{
		case _CS_PATH:
#ifndef _CSPATH
			/*  If _CSPATH is not defined, return zero */
			return(0);
#else
			return_len = strlen(_CSPATH) + 1;
		       /* 
                        *  If the buffer is large enough to hold the _CSPATH
                        *  value + NULL, copy _CSPATH into buffer and return
			*  the size of the buffer to indicate success.
                        */

			if (len >= return_len)  {
				strcpy(buf, _CSPATH);
				return(return_len);
			}

		       /*  
                        *  If the buffer is not large enough, truncate
			*  the _CSPATH value to len - 1 and copy into buffer.
			*  Return shortened length to indicate error.
                        */

			if ((len != 0) && (buf != (char *)0)) {
				strncpy(buf, _CSPATH, --len);
				buf += len;
				*buf = '\0';
				return(return_len);
			}

			/*
                         *  If len is zero and buffer is NULL
                         *  return the size buffer needed.
                         */
			return(return_len);
#endif

		/* If name is not valid, set errno and exit */
		default:
			errno = EINVAL;
			return(0);
	}

}  
