static char sccsid[] = "@(#)80	1.3.1.1  src/bos/usr/lpp/jls/dictutil/kjprmsg.c, cmdKJI, bos411, 9428A410j 7/23/92 00:58:01";
/*
 * COMPONENT_NAME: User Dictionary Utility
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * COPYRIGHT:           5756-030 COPYRIGHT IBM CORP 1991
 *                      LICENSED MATERIAL-PROGRAM PROPERTY OF IBM
 *                      REFER TO COPYRIGHT INSTRUCTIONS FORM NO.G120-2083
 *
 * STATUS:              User Dictionary Maintenance for AIX 3.2
 *
 * CLASSIFICATION:    OCO Source Material - IBM Confidential.
 *                    (IBM Confidential-Restricted when aggregated)
 *
 * FUNCTION:
 *
 * NOTES:               NA.
 *
 */


#if defined(PARMADD)
#include <stdio.h>

#include "kje.h"
#define	EUCBUF	256
extern  int     cnvflg;         /* Conversion Type Code */

int kjprmsg( prpos, ptr, msgcode )

int     prpos;                  /* Paramater Error Position         (i) */
char    *ptr;                   /* Data Parameter (PC code)         (i) */
int     msgcode;                /* Error Message ID                 (i) */
{
	char eucbuf[EUCBUF];
	char	*pbuf;
	size_t	pclen, euclen;

	if ( cnvflg == U_SJIS ) {
	    pbuf = ptr;
	}
	else {
	    pclen = strlen( ptr );
	    euclen = EUCBUF;
	    kjcnvste( ptr, &pclen, eucbuf, &euclen);
	    eucbuf[EUCBUF - euclen] = '\0';
	    pbuf = eucbuf;
	}

	switch ( msgcode ) {
	    case U_DUPPRD :
		(void)printf(CU_MSGDPD);
		printf("\n");
		break;

	    case U_DUPPRP :
		(void)printf(CU_MSGDPP);
		printf("\n");
		break;

	    case U_NOTYDT :
		(void)printf(CU_MSGNYM);
		printf("%s \n",pbuf);
		break;

	    case U_NOTGDT :
		(void)printf(CU_MSGNGK);
		printf("%s \n",pbuf);
		break;

	    case U_YNOLDB :
		(void)printf(CU_MSGIDY,pbuf);
		printf("\n");
		break;

	    case U_GNOLDB :
		(void)printf(CU_MSGIDG,pbuf);
		printf("\n");
		break;

	    case U_IGPTRD :
		(void)printf(CU_MSGIGP);
		printf("%s \n",pbuf);
		break;

	    case U_IGPTRY :
		(void)printf(CU_MSGNXY);
		printf("%s \n",pbuf);
		break;

	    case U_IGPTRG :
		(void)printf(CU_MSGNXG);
		printf("%s \n",pbuf);
		break;

	    case U_IGDATA :
		(void)printf(CU_MSGIGP);
		printf("%s \n",pbuf);
		break;

	    case U_NOTYMI :
		(void)printf(CU_MSGNFY);
		printf("%s \n",pbuf);
		break;

	    case U_NOTGKU :
		(void)printf(CU_MSGNFG);
		printf("%s \n",pbuf);
		break;
	};

	return( 0 );

}
#endif
