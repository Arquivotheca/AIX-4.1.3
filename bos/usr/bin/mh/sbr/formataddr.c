static char sccsid[] = "@(#)53	1.7  src/bos/usr/bin/mh/sbr/formataddr.c, cmdmh, bos41J, 9515B_all 4/3/95 15:59:36";
/* 
 * COMPONENT_NAME: CMDMH formataddr.c
 * 
 * FUNCTIONS: CHECKMEM, CPY, MSGSTR, formataddr 
 *
 * ORIGINS: 10  26  27  28  35 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/* static char sccsid[] = "formataddr.c	7.1 87/10/13 17:05:38"; */

/* formataddr.c - format an address field (from formatsbr) */

#include "mh.h"
#include "addrsbr.h"
#include "formatsbr.h"
#include <ctype.h>
#include <stdio.h>

#include "mh_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MH,n,s) 

static char *buf;		/* our current working buffer */
static char *bufend;		/* end of working buffer */
static char *last_dst;		/* buf ptr at end of last call */
static unsigned int bufsiz;	/* current size of buf */

#define BUFINCR 512		/* how much to expand buf when if fills */

#define CPY(s) { cp = (s); while (*dst++ = *cp++) ; --dst; }

/* check if there's enough room in buf for str.  add more mem if needed */
#define CHECKMEM(str) \
	    if ((len = strlen (str)) >= bufend - dst) {\
		bufsiz += ((dst + len - bufend) / BUFINCR + 1) * BUFINCR;\
		last_dst = dst - buf;\
		buf = (char *)realloc (buf, bufsiz);\
		if (! buf)\
		    adios (NULLCP, MSGSTR(NOBSP, "formataddr: couldn't get buffer space"));\
		dst = buf + (int)last_dst;\
		bufend = buf + bufsiz;\
	    } /*MSG*/


/* fmtscan will call this routine if the user includes the function
 * "(formataddr {component})" in a format string.  "orig" is the
 * original contents of the string register.  "str" is the address
 * string to be formatted and concatenated onto orig.  This routine
 * returns a pointer to the concatenated address string.
 *
 * We try to not do a lot of malloc/copy/free's (which is why we
 * don't call "getcpy") but still place no upper limit on the
 * length of the result string.
 *
 * This routine is placed in a separate library so it can be
 * overridden by particular programs (e.g., "replsbr").
 */
char *formataddr (orig, str)
    char *orig;
    char *str;
{
    register int  len;
    register int  isgroup;
    register char  *dst;
    register char  *cp;
    register char  *sp;
    register struct mailname *mp = NULL;

    /* if we don't have a buffer yet, get one */
    if (bufsiz == 0) {
	buf = (char *)malloc (BUFINCR);
	if (! buf)
	    adios (NULLCP, MSGSTR(NOABSP, "formataddr: couldn't allocate buffer space")); /*MSG*/
	bufsiz = BUFINCR - 6;  /* leave some slop */
	bufend = buf + bufsiz;
    }
    /*
     * If "orig" points to our buffer we can just pick up where we
     * left off.  Otherwise we have to copy orig into our buffer.
     */
    if (orig == buf)
	dst = last_dst;
    else if (!orig || !*orig) {
	dst = buf;
	*dst = '\0';
    } else {
	CHECKMEM (orig);
	CPY (orig);
    }

    /* concatenate all the new addresses onto 'buf' */
    for (isgroup = 0; cp = getname (str); ) {
	if ((mp = getm (cp, NULLCP, 0, fmt_norm, NULLCP)) == NULL)
	    continue;

	if (isgroup && (mp->m_gname || !mp->m_ingrp)) {
	    *dst++ = ';';
	    isgroup = 0;
	}
	/* if we get here we're going to add an address */
	if (dst != buf) {
	    *dst++ = ',';
	    *dst++ = ' ';
	}
	if (mp->m_gname) {
	    CHECKMEM (mp->m_gname);
	    CPY (mp->m_gname);
	    isgroup++;
	}
	sp = adrformat (mp);
	CHECKMEM (sp);
	CPY (sp);
	mnfree (mp);
    }

    if (isgroup)
	*dst++ = ';';

    *dst = '\0';
    last_dst = dst;
    return (buf);
}
