static char sccsid[] = "@(#)64	1.1  src/bos/usr/ccs/lib/lib300s/open.c, libt300s, bos411, 9428A410j 9/30/89 15:45:03";
/*
 * COMPONENT_NAME: libplot
 *
 * FUNCTIONS: openvt, openpl
 *
 * ORIGINS: 4,10,27
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
 */

#include <signal.h>
#include <termio.h>
/* gsi plotting output routines */
# define DOWN 012
# define UP 013
# define LEFT 010
# define RIGHT 040
# define BEL 007
# define ACK 006
# define CR 015
# define FF 014
# define VERTRESP 48
# define HORZRESP 60.
# define HORZRES 6.
# define VERTRES 8.
/* down is line feed, up is reverse line feed,
   left is backspace, right is space.  48 points per inch
   vertically, 60 horizontally */

int xnow, ynow;
struct termio ITTY, PTTY;
int OUTF;
float HEIGHT = 6.0, WIDTH = 6.0, OFFSET = 0.0;
int xscale, xoffset, yscale;
float botx = 0., boty = 0., obotx = 0., oboty = 0.;
float scalex = 1., scaley = 1.;

openpl ()
{
	int reset();
		xnow = ynow = 0;
		OUTF = 1;
		printf("\r");
		ioctl(OUTF, TCGETA, &ITTY);
		signal (SIGINT, reset);
		PTTY = ITTY;
		PTTY.c_oflag &= ~(ONLCR|OCRNL|ONOCR|ONLRET);
		PTTY.c_cflag |= CSTOPB;
		ioctl(OUTF, TCSETAW, &PTTY);
		/* initialize constants */
		xscale  = 4096./(HORZRESP * WIDTH);
		yscale = 4096 /(VERTRESP * HEIGHT);
		xoffset = OFFSET * HORZRESP;
		return;
}

openvt(){
openpl();
}
