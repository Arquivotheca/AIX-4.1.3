static char sccsid[] = "@(#)25  1.3  src/bos/usr/bin/ckwinsiz/ckwinsiz.c, rcs, bos411, 9428A410j 11/21/93 15:22:57";
/*
 *   COMPONENT_NAME: RCS
 *
 *   FUNCTIONS: main
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* the only function and prototype */
int main(void);

#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <locale.h>
#include <nl_types.h>
/*              include file for message texts          */
#include <ckwinsiz_msg.h> 

#define FAIL 255
#define SUCCESS 0

nl_catd  scmc_catd;   /* Cat descriptor for scmc conversion */

int main(void)
{
	struct winsize wsize;

	(void) setlocale(LC_ALL,"");

	scmc_catd = catopen("ckwinsiz.cat",NL_CAT_LOCALE);
 
	if (ioctl(STDIN_FILENO, TIOCGWINSZ, (char *) &wsize) < 0)
	{
		fprintf(stderr, catgets(scmc_catd, MS_ckwinsiz, M_MSG_1,
				"Error retrieving window size.\n") );
		catclose(scmc_catd);
		exit(FAIL);
	}

	if (wsize.ws_row < 24 || wsize.ws_col < 80)	
	{
		if (!(wsize.ws_row == 0 && wsize.ws_col == 0))
		{
			fprintf(stderr, catgets(scmc_catd, MS_ckwinsiz, 
				M_MSG_2, "\nA minimum window size of 80 "
				"by 24 is required to run this program.\n"
				"Please check your terminal type or window "
				"size and try again.\n") );
			exit(FAIL);
		}
	}

	exit(SUCCESS);

}

