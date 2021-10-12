static char sccsid[] = "@(#)35 1.5  src/bos/usr/bin/learn/makpipe.c, cmdlearn, bos41J, 9521A_all 5/23/95 15:35:53";
/*
 * COMPONENT_NAME: (CMDLEARN) provides computer-aided instruction courses
 *
 * FUNCTIONS: makpipe
 *
 * ORIGINS: 26, 27 
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#include "stdio.h"

#include "learn_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_LEARN,n,s)

makpipe()
{
	int f[2];

	pipe(f);
	if (fork()==0) {
		close(f[1]);
		close(0);
		dup(f[0]);
		close(f[0]);
		execl ("/usr/bin/bsh", "bsh", "-i", 0);
		perror("/usr/bin/bsh");
		exit(1);
	}
	close(f[0]);
	sleep(2);	/* so shell won't eat up too much input */
	return(f[1]);
}
