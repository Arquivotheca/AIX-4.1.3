/* aix_sccsid[] = "src/bos/usr/lib/sendmail/mailstats.h, cmdsend, bos411, 9428A410j AIX 6/15/90 23:24:11" */
/* 
 * COMPONENT_NAME: CMDSEND mailstats.h
 * 
 * FUNCTIONS: 
 *
 * ORIGINS: 10  26  27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
**  Sendmail
**  Copyright (c) 1983  Eric P. Allman
**  Berkeley, California
**
**  Copyright (c) 1983 Regents of the University of California.
**  All rights reserved.  The Berkeley software License Agreement
**  specifies the terms and conditions for redistribution.
**
**	"@(#)mailstats.h	5.1 (Berkeley) 5/2/86";
**
*/

/*
**  Statistics structure.
*/

struct statistics			/* file stats block */
{
	long	stat_itime;		/* file initialization time */
	short	stat_size;		/* size of this structure */
	long	msgsfrom[MAXMAILERS];	/* # msgs from each mailer */
	long	bytesfrom[MAXMAILERS];	/* kbytes from each mailer */
	long	msgsto[MAXMAILERS];	/* # msgs to each mailer */
	long	bytesto[MAXMAILERS];	/* kbytes to each mailer */
	char	stat_mname[MAXMAILERS][MAXNAME];
};
