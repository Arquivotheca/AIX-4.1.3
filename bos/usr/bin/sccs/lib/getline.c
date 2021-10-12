static char sccsid[] = "@(#)72 1.8 src/bos/usr/bin/sccs/lib/getline.c, cmdsccs, bos411, 9428A410j 2/1/94 17:05:57";
/*
 * COMPONENT_NAME: CMDSCCS      Source Code Control System (sccs)
 *
 * FUNCTIONS: getline
 *
 * ORIGINS: 3, 10, 27, 71
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
 *
 *
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 * OSF/1 1.1
 */

# include       "defines.h"

/*
	Routine to read a line into the packet.  The main reason for
	it is to make sure that pkt->p_wrttn gets turned off,
	and to increment pkt->p_slnno.
*/

char *
getline(pkt)
register struct packet *pkt;
{
	char *n;
	register char *p;

	if(pkt->p_wrttn==0)
		putline(pkt,(char *) 0);
	if ((n = fgets(pkt->p_line,sizeof(pkt->p_line),pkt->p_iop)) != NULL) {
		pkt->p_slnno++;
		pkt->p_wrttn = 0;
		for (p = pkt->p_line; *p; ) {
			pkt->p_7bchash += (signed char)*p;
			pkt->p_chash += *p++;
		}
	}
	else {
		if (!pkt->p_chkeof)
			fatal(MSGCO(PRMTREOF,"\nThe end of the file was premature.\n\
\tMake sure that the last line of the file ends with a newline character or\n\
\tuse local problem reporting procedures. (co5)\n"));
		/* If we are looking for chksum, try the new hashing algorithm 
		calculated in p_7bchash, if this does not match, try old 
		algorithm (p_chash) if this also fail, we got an 
		errorneous file */
		if ( (pkt->do_chksum)
			&& (pkt->p_7bchash ^ pkt->p_ihash)&0xFFFF
			&& (pkt->p_chash ^ pkt->p_ihash)&0xFFFF ) {
			fatal(MSGCO(CORRUPT,"\nThe file is damaged.\n\
\tUse local problem reporting procedures. (co6)\n"));
		}
		if (pkt->p_reopen) {
			fseek(pkt->p_iop,0L,0);
			pkt->p_reopen = 0;
			pkt->p_slnno = 0;
			pkt->p_ihash = 0;
			pkt->p_chash = 0;
			pkt->p_nhash = 0;
			pkt->p_keep = 0;
			pkt->do_chksum = 0;
		}
		else {
			fclose(pkt->p_iop);
			pkt->p_iop = 0;
		}
	}
	return(n);
}
