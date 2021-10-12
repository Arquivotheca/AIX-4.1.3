static char sccsid[] = "@(#)89	1.9  src/bos/usr/ccs/lib/libc/setbuffer.c, libcio, bos411, 9428A410j 4/20/94 17:52:59";

#ifdef _POWER_PROLOG_
/*
 *   COMPONENT_NAME: LIBCIO
 *
 *   FUNCTIONS: setbuffer
 *		setlinebuf
 *
 *   ORIGINS:26, 27,71
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1985,1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#endif /* _POWER_PROLOG_ */

/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/* setbuffer.c,v $ $Revision: 2.8.2.3 $ (OSF) */
/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 */

#include <stdio.h>
#include <malloc.h>
#include "ts_supp.h"
#include "push_pop.h"

#ifdef	_THREAD_SAFE
#include "stdio_lock.h"
#endif	/* _THREAD_SAFE */


/*                                                                    
 * FUNCTION: Assigns buffering to a stream.
 *
 * RETURN VALUE DESCRIPTION: None.
 *
 * setbuffer - setup stdio stream buffering after opening, but before
 *	it is read or written.
 */
void
setbuffer(FILE *stream, char *buf, size_t size)
{
	int typ;
	/* if buf parameter is null, i/o is unbuffered - p70635 */
	if (buf == NULL)
		typ = _IONBF;
	else
		typ = _IOFBF;

	/* we just call the sysV setvbuf(3) */
	(void)setvbuf( stream, buf, typ, size );
	/* BSD returns indeterminate value */
}


#ifdef _THREAD_SAFE
#define	FFLUSH	fflush_unlocked
#define	SETVBUF	setvbuf_unlocked
#else
#define	FFLUSH	fflush
#define	SETVBUF	setvbuf
#endif	/* _THREAD_SAFE */

/*
 * setlinebuf - change stdio stream buffering from block or unbuffered to
 *	line buffered, may be used even after reading or writting.
 */
void setlinebuf(FILE *stream)
{
	TS_FDECLARELOCK(filelock)

	TS_FLOCK(filelock, stream);

	TS_PUSH_CLNUP(filelock);
	FFLUSH(stream);		      /* force out all output */
	TS_POP_CLNUP(0);

	SETVBUF(stream,(char *)NULL,_IONBF,(size_t)0);/* close down buffering */
	(void)SETVBUF(stream, (char *)NULL, _IOLBF, (size_t)BUFSIZ);
	/* say this buffer belongs to stdio */
	stream->_flag |= _IOMYBUF;

	TS_FUNLOCK(filelock);

	/* BSD returns indeterminate value */
}
