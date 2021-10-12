static char sccsid[] = "@(#)06  1.6  src/bos/usr/ccs/lib/libtli/terror.c, libtli, bos411, 9434A411a 8/19/94 14:07:21";
/*
 *   COMPONENT_NAME: LIBTLI
 *
 *   FUNCTIONS: t_error, terrmsg
 *
 *   ORIGINS: 18 27 63
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991, 1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.2
 */

/** Copyright (c) 1989  Mentat Inc.
 ** terror.c 1.3, last change 12/20/89
 **/

#include "common.h"
#include <stdio.h>
#include <nl_types.h>
#include <locale.h>
#include "libtli_msg.h"

char	* t_errlist[] = {
	"no error specified",				/* 0 no error	*/
	"incorrect addr format",			/* 1 TBADADDR	*/
	"incorrect option format",			/* 2 TBADOPT	*/
	"incorrect permissions",			/* 3 TACCESS	*/
	"illegal transport fd",				/* 4 TBADF	*/
	"couldn't allocate addr",			/* 5 TNOADDR	*/
	"out of state",					/* 6 TOUTSTATE	*/
	"bad call sequence number",			/* 7 TBADSEQ	*/
	"system error",					/* 8 TSYSERR	*/
	"event requires attention",			/* 9 TLOOK	*/
	"illegal amount of data",			/* 10 TBADDATA	*/
	"buffer not large enough",			/* 11 TBUFOVFLOW*/
	"flow control",					/* 12 TFLOW	*/
	"no data",					/* 13 TNODATA	*/
	"discon_ind not found on queue",		/* 14 TNODIS	*/
	"unitdata error not found",			/* 15 TNOUDERR	*/
	"bad flags",					/* 16 TBADFLAG	*/
	"no ord rel found on queue",			/* 17 TNOREL	*/
	"primitive/action not supported",		/* 18 TNOTSUPPORT */
	"state is in process of changing",		/* 19 TSTATECHNG*/
	"unsupported struct-type requested",		/* 20 TNOSTRUCTYPE */
	"invalid transport provider name",		/* 21 TBADNAME	*/
	"qlen is zero",					/* 22 TBADQLEN	*/
	"address in use"				/* 23 TADDRBUSY	*/
#ifdef XTI
	,
	"outstanding connection indications",		/* 24 TINDOUT	*/
	"transport provider mismatch",			/* 25 TPROVMISMATCH */
	"resfd specified to accept w/qlen >0",		/* 26 TRESQLEN	*/
	"resfd not bound to same addr as fd",		/* 27 TRESADDR	*/
	"incoming connection queue full",		/* 28 TQFULL	*/
	"XTI protocol error"				/* 29 TPROTO	*/
#endif
};


int	t_nerr = sizeof(t_errlist) / sizeof(t_errlist[0]);

#if defined(_NO_PROTO)
extern	char	*terrmsg(); 
#else
extern	char	*terrmsg( int ); 
#endif /* defined(_NO_PROTO) */

extern 	int 	sys_nerr;
extern 	char 	*sys_errlist[];
extern	int	errno;

#ifdef	XTI
int
#else
void
#endif
t_error (msg)
	char	* msg;
{
	char	*c;
	nl_catd	catd;
	int	err;

	setlocale (LC_ALL,"");    /* Designates native locale */
	catd = catopen(MF_LIBTLI, NL_CAT_LOCALE);

	if (!msg)
		msg = catgets(catd, MS_LIBTLI, TERROR_MSG, "t_error");

	if ((err = t_errno) > 0  &&  err < t_nerr) {

		c = catgets(catd, MS_LIBTLI, err, t_errlist[err]);

		if (err == TSYSERR) {
			if (msg == NULL || *msg == 0) 
				fprintf(stderr, "%s, %s\n", c, 
					terrmsg(errno));
			else
				fprintf(stderr, "%s: %s, %s\n", msg, c, 
						terrmsg(errno));
		} else if (msg == NULL || *msg == 0)
			fprintf(stderr, "%s\n", c);
		else
			fprintf(stderr, "%s: %s\n", msg, c);
	} else 
		fprintf(stderr, catgets(catd, MS_LIBTLI, TERROR_ERR,
			"%s: Error %d occured\n"), msg, err);

	catclose(catd);

#ifdef	XTI
	return 0;
#else
	return;
#endif
}



/*
 * terrmsg()	
 *
 *	Map the value of errno to a text string error message
 *
 * Inputs:
 *	e	= Current value of errno
 *
 * Outputs:
 *	None.
 *
 * Returns:
 *	Pointer to null terminated character string error
 *	message.
 */

static char buf[256];

char *terrmsg( e )
int e;
{
	/*
	 * Clear buffer prior to usage
	 */
	bzero(buf, sizeof(buf));

	/*
	 * Check range of error value
	 */
	if ( (e >= 0) && (e < sys_nerr) )
	{
		sprintf(buf, "%s", sys_errlist[e]);
	}
	else
	{
		sprintf(buf, "Error %d occurred.", e);
	}

	/*
	 * Return pointer to statically defined 
	 * buffer contaning error string.
	 */
	return(buf);
}


