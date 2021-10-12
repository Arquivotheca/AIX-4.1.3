static char sccsid[] = "@(#)50	1.3  src/bos/usr/bin/pagesize/pagesize.c, cmdstat, bos411, 9428A410j 4/25/91 17:46:08";
/*
 * COMPONENT_NAME: (CMDSTAT) Displays the System Page Size
 *
 * FUNCTIONS: pagesize
 *
 * ORIGINS: 26, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

main()
{
	printf("%d\n", getpagesize());
	exit (0);
}
