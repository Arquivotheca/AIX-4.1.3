static char sccsid[] = "@(#)66 1.10 src/bos/usr/bin/sccs/lib/fmterr.c, cmdsccs, bos411, 9428A410j 2/6/94 10:34:58";
/*
 * COMPONENT_NAME: CMDSCCS      Source Code Control System (sccs)
 *
 * FUNCTIONS: Fmterr
 *
 * ORIGINS: 3, 10, 27, 71
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
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

Fmterr(pkt, cfile, line)
char *cfile;
register struct packet *pkt;
{
	char ErrMsg[512];

	fclose(pkt->p_iop);
	fprintf(stderr, "Fmterr: file=%s, line=%d\n", cfile, line);
	sprintf(ErrMsg,MSGCO(FRMTERR, "There is a format error at line %u \n\
\tRestore the backup copy. (co4)\n"),pkt->p_slnno);  /* MSG */
	fatal(ErrMsg);
}
