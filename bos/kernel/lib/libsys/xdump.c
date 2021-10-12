static char sccsid[] = "@(#)44	1.4  src/bos/kernel/lib/libsys/xdump.c, libsys, bos411, 9428A410j 6/5/91 12:57:36";
/*
 * COMPONENT_NAME (libsys)
 *
 * ORIGIN: 25
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989,1991
 * Unpublished Work
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   


void 	printf(char *,...);
static void x_scpy (char *,char *);
static int x_scmp (char *,char *);
/*
 * xdump -- hex dump routine to facilitate debugging.
 */

void xdump (char *saddr, int count)

{

#define LINESIZE     60
#define ASCIISTRT    40
#define HEXEND       36

    int i, j, k, hexdigit;
    register int c;
    char *hexchar;
    char linebuf[LINESIZE+1];
    char prevbuf[LINESIZE+1];
    char *linestart;
    int asciistart;
    char asterisk = ' ';


    hexchar = "0123456789ABCDEF";
    prevbuf[0] = '\0';
    i = (int) saddr % 4;
    if (i != 0)
	saddr = saddr - i;

    for (i = 0; i < count;) {
	for (j = 0; j < LINESIZE; j++)
	    linebuf[j] = ' ';

	linestart = saddr;
	asciistart = ASCIISTRT;
	for (j = 0; j < HEXEND;) {
	    for (k = 0; k < 4; k++) {
		c = *(saddr++) & 0xFF;
		if ((c >= 0x20) && (c <= 0x7e))
		    linebuf[asciistart++] = (char) c;
		else
		    linebuf[asciistart++] = '.';
		hexdigit = c >> 4;
		linebuf[j++] = hexchar[hexdigit];
		hexdigit = c & 0x0f;
		linebuf[j++] = hexchar[hexdigit];
		i++;
	    }
	    if (i >= count)
		break;
	    linebuf[j++] = ' ';
	}
	linebuf[LINESIZE] = '\0';
	if (((j = x_scmp (linebuf, prevbuf)) == 0) && (i < count)) {
	    if (asterisk == ' ') {
		asterisk = '*';
#ifdef DEBUG
		(void) printf ("    *\n");
#endif DEBUG
	    }
	}
	else {
#ifdef DEBUG
	    (void) printf ("    %x  %s\n",linestart, linebuf);
#endif DEBUG
	    asterisk = ' ';
	    x_scpy (prevbuf, linebuf);
	}
    }

    return;
}

static int x_scmp(register char *s1,register char *s2)
{
    while ((*s1) && (*s1 == *s2)) {
	s1++;
	s2++;
    }
    if (*s1 || *s2)
	return(-1);
    else
	return(0);
}

static void x_scpy(register char *s1, register char *s2)
{
    while ((*s1 = *s2) != '\0') {
	s1++;
	s2++;
    }
}
