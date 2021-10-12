static char sccsid[] = "@(#)81	1.12  src/bos/usr/ccs/lib/libc/termdef.c, libctty, bos411, 9428A410j 3/2/94 16:04:00";
/*
 * COMPONENT_NAME: LIBCTTY terminal control routines
 *
 * FUNCTIONS: termdef
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include <standards.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/lft_ioctl.h>

/*#include <sys/hft.h>*/
#if 0
static char cbuf[HFINTROSZ] = {
    HFINTROESC,
    HFINTROLBR,
    HFINTROEX,
    0,
    0,
    0,
    6,
    HFQHFTCH,
    HFQHFTCL,
};

static char rbuf[sizeof(struct hfqhftr)+1];	/* plus 1 is a bug in the hft */

static struct hfquery qbuf = {
    cbuf,
    sizeof(cbuf),
    rbuf,
    sizeof(rbuf),
};

static int hfquery(fd, c)
{
    struct hfqhftr *resp = (struct hfqhftr *)rbuf;
    int i;

    if (ioctl(fd, HFQUERY, &qbuf) < 0)
	return 0;
    switch (c) {
    case 'c':
	return resp->hf_phcol;

    case 'l':
	return resp->hf_phrow;

    default:
	return (resp->hf_phdevid >> 16) & 0xff;
    }
}
#endif
char *termdef(int fd, char c)
{
    static char cbuf[16];
    static char lbuf[16];
    struct winsize win;
    lft_query_t lq;	
    char *s;
    int i;

    switch (c) {
    case 'c':				/* columns */
	/*if ((!ioctl(fd, TIOCGWINSZ, &win) && (i = win.ws_col)) ||
	    (i = hfquery(fd, c))) {*/
	if ((!ioctl(fd, TIOCGWINSZ, &win) && (i = win.ws_col))) {
	    sprintf(cbuf, "%d", i);
	    return cbuf;
	}
	if (s = getenv("COLUMNS"))
	    return s;
	return "";

    case 'l':				/* lines */
	if ((!ioctl(fd, TIOCGWINSZ, &win) && (i = win.ws_row))) {
	    sprintf(lbuf, "%d", i);
	    return lbuf;
	}
	if (s = getenv("LINES"))
	    return s;
	return "";
	
    default:				/* term type */
	/*if ((i = hfquery(fd, c)) > 0)
	    return "hft";*/
	i = ioctl(fd, LFT_QUERY_LFT, &lq);
	if (i != -1)
	{
		return "lft";
	}
	if (s = getenv("TERM"))
	    return s;
	return "dumb";
    }
}
