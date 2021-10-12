static char sccsid[] = "@(#)66	1.10  src/bos/usr/ccs/bin/size/process.c, cmdaout, bos411, 9428A410j 5/14/91 09:17:45";

/*
 * COMPONENT_NAME: CMDAOUT (size command)
 *
 * FUNCTIONS: process
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
#include	<ar.h>

/* COMMON SGS HEADERS */
#include	"filehdr.h"
#include	"scnhdr.h"
#include	"ldfcn.h"

/* SIZE HEADERS */
#include	"defs.h"
#include	"process.h"

#include	<nl_types.h>
#include	"size_msg.h"
extern nl_catd	catd;
#define		MSGSTR(Num, Str) catgets(catd, MS_SIZE, Num, Str)

/* STATIC VARIABLE USED */
#ifndef UNIX
static char	*prtotal[3] = {
			PRTOTAL_0,
			PRTOTAL_1,
			PRTOTAL_2
		};
#endif


    /*  process(filename)
     *
     *  prints out the sum total of section sizes along with the size
     *  information (size, physical and virtual addresses) for each section
     *  in the object file
     *  uses static format strings to do the printing (see process.h).
     *
     *  calls:
     *      - findtotal( ) to find the sum total of section sizes
     *      - error(file, string) if the object file is somehow messed up
     *
     *  simply returns
     */


process(filename)

char	*filename;

{
    /* UNIX FUNCTIONS CALLED */
/*    extern		printf( );
*/

    /* COMMON OBJECT FILE ACCESS ROUTINE CALLED */
    extern int		ldshread( );

    /* SIZE FUNCTIONS CALLED */
#ifndef UNIX
    extern long		findtotal( );
#endif /* UNIX */
    extern		error( );

    /* EXTERNAL VARIABLES USED */

    extern int		numbase;
    extern LDFILE	*ldptr;
    extern int		Fflag;

    /* LOCAL VARIABLES */
    long		size;
    unsigned short	section;
    SCNHDR		secthead;
/*  following are presently unused:  
    ARCHDR		arhead;
    extern int		ldahread( );
    long		ssize[10];
    char		*sname[10];
 */

#ifndef UNIX
    if ((size = findtotal( )) == TROUBLE) {
	error(filename, MSGSTR(TOTAL_ERROR_MSG, TOTAL_ERR));
	return;
    }

    if (numbase == 0)
    	printf(MSGSTR(PRTOTAL_0_MSG, PRTOTAL_0), filename, size);
    else if (numbase == 1)
    	printf(MSGSTR(PRTOTAL_1_MSG, PRTOTAL_1), filename, size);
    else if (numbase == 2)
    	printf(MSGSTR(PRTOTAL_2_MSG, PRTOTAL_2), filename, size);
    printf(prhead);
#endif

    size = 0L;
    for (section = 1; section <= HEADER(ldptr).f_nscns; ++section) {
	if (ldshread(ldptr, section, &secthead) != SUCCESS) {
	    error(filename, MSGSTR(SCN_HDR_ERROR_MSG, SCN_HDR_ERROR));
	    return;
	}
#ifdef UNIX
	printf(prusect[numbase], secthead.s_size);
	if (Fflag)
	    printf(PAREN_STRING, secthead.s_name);
	if (HEADER(ldptr).f_nscns != section)
	    printf(PLUS_SIGN);
	size += secthead.s_size;
#else
	printf(prsect[numbase], secthead.s_name, secthead.s_size,
	       secthead.s_paddr, secthead.s_vaddr);
#endif

	}
#ifdef UNIX
	printf(prusum[numbase], size);
    return;
#endif
}
