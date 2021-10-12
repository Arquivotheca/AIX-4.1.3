static char sccsid[] = "@(#)65	1.6  src/bos/usr/ccs/bin/size/fcns.c, cmdaout, bos411, 9428A410j 6/15/90 20:08:21";

/*
 * COMPONENT_NAME: CMDAOUT (size command)
 *
 * FUNCTIONS: error, findtotal
 *
 * ORIGINS: 27, 3
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* UNIX HEADER */
#include	<stdio.h>

/* COMMON SGS HEADERS */
#include	"filehdr.h"
#include	"scnhdr.h"
#include	"ldfcn.h"

/* SIZE HEADER */
#include	"defs.h"

/* EXTERNAL VARIABLES USED */
extern int Exit_code;


/*  error(file, string)
 *
 *  simply prints the error message string
 *  sets Exit_code variable
 *  simply returns
 */


error(file, string)

char	*file;
char	*string;

{
	/* UNIX FUNCTIONS CALLED */
/*	extern	fprintf( ),
		fflush( );
*/

	fflush(stdout);
	fprintf(stderr, ERROR_STRING, SGS, file, string);

	Exit_code = -1;
	return;
}




#ifndef UNIX
/*  findtotal( )
 *
 *  computes and returns the sum total of all section sizes
 *  returns TROUBLE if the object file is messed up
 */


long
findtotal( )

{
	/* COMMON OBJECT FILE ACCESS ROUTINE CALLED */
	extern int		ldshread( );

	/* EXTERNAL VARIABLE USED */
	extern LDFILE		*ldptr;

	/* LOCAL VARIABLES */
	long			size;
	unsigned short		section;
	SCNHDR			secthead;

	size = 0L;
	for (section = 1; section <= HEADER(ldptr).f_nscns; ++section)
	{
		if (ldshread(ldptr, section, &secthead) != SUCCESS)
		{
			return(TROUBLE);
		}
		size += secthead.s_size;
	}

	return(size);
}
#endif	/*UNIX*/
