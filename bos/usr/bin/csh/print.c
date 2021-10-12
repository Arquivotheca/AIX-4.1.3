static char sccsid[] = "@(#)33	1.17  src/bos/usr/bin/csh/print.c, cmdcsh, bos411, 9428A410j 5/4/94 14:19:03";
/*
 * COMPONENT_NAME: CMDCSH  c shell(csh)
 *
 * FUNCTIONS: p60ths psecs p2dig displaychar draino flush plist
 *
 * ORIGINS:  10,26,27,18,71
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985,1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 * 
 * OSF/1 1.1
 */

#include <sys/ioctl.h>
#include "sh.h"


void
p60ths(long l)
{
	l += 3;
	printf("%d.%d", (int) (l / 60), (int) ((l % 60) / 6));
}

void
psecs(long l)
{
	register int i;

	i = l / 3600;
	if (i) {
		printf("%d:", i);
		i = l % 3600;
		p2dig(i / 60);
		goto minsec;
	}
	i = l;
	printf("%d", i / 60);
minsec:
	i %= 60;
	display_char(':');
	p2dig(i);
}

void
p2dig(register int i)
{
	printf("%d%d", i / 10, i % 10);
}

uchar_t linbuf[128];
uchar_t *linp = linbuf;
static int quoted = 0;

/*
 * If c is NLQUOTE, then the next "character" must be reconstructed
 * to determine how many bytes are actually quoted.  The only way
 * to solve this is 1) follow NLQUOTE with the number of quoted bytes
 * or 2) convert everything to process codes.
 *
 * If the last few characters are preceeded by an invalid character,
 * they will be lost when draino() is called because there is no way
 * to determine whether a character is invalid vs. insufficient text
 * to complete. The multibyte version of flush() will empty the
 * remaining ilsbuf[] bytes.
 */

static uchar_t ilsbuf[MB_LEN_MAX+1];
static uchar_t *pils = NULL;

void
display_char(register int c)
{
	if (!quoted) {
		if (c != NLQUOTE)
			put_one_char(c);
		else {
			quoted++;
			pils = ilsbuf;
		}
	} else if (mb_cur_max == 1) {
		put_one_char(c);
		quoted = 0;
	} else {
		register int n;

		*pils++ = c;
		n = mblen((char *)ilsbuf, pils - ilsbuf);
		if (n > 0) {
			pils = ilsbuf;
			do
				put_one_char(*pils++);
			while (--n > 0);
			quoted = 0;
		} else if (pils - ilsbuf >= mb_cur_max) {
			uchar_t *ptmp;

			ptmp = ilsbuf;
			put_one_char(*ptmp++);
			n = pils - ptmp;
			quoted = 0;
			while (n-- > 0)
				display_char (*ptmp++);
		}
	}
	return;
}

void
put_one_char(register int c)
{
	if (!quoted && (c < ' ' && c != '\t' && c != '\n' && c != '\b')) {	
		*linp++ = '^';
		c |= 'A' - 1;
	}
	*linp++ = c;
	if (c == '\n' || linp > &linbuf[sizeof linbuf - MB_LEN_MAX])
		flush_now();
}

void
draino(void)
{
	quoted = 0;
	linp = linbuf;
}

#define FULL_FS( AAA ) if ( AAA < 0 && errno == ENOSPC && ! haderr ) \
                       bferr(MSGSTR(M_FULLFS, "error: File system is full"))

/*
 * Empty ilsbuf[] before output - avoid infinite recursion loop by
 * always eliminating one byte in ilsbuf[] each time flush() is called
 *
 * Assume first ilsbuf[] byte is invalid multibyte character and process
 * remaining ilsbuf[] through display_char() again
 */
void
flush(void)
{
	if (quoted && pils > ilsbuf) {
		register int n;
		register uchar_t *ptmp;
 
		ptmp = ilsbuf;
		put_one_char(*ptmp++);
		n = pils - ptmp;
		quoted = 0;
		while (n-- > 0)
			display_char (*ptmp++);
		flush();
	}
	flush_now();
	quoted = 0;
}

void
flush_now()
{
	register int unit;
	int lmode = 0;

	if (linp == linbuf)
		return;
	if (haderr)
		unit = didfds ? 2 : SHDIAG;
	else
		unit = didfds ? 1 : SHOUT;

#ifdef BSD_LINE_DISC
        if (didfds==0 && ioctl(unit, TIOCLGET, &lmode)==0 && lmode & LFLUSHO) {
                lmode = LFLUSHO;
                IOCTL(unit, TIOCLBIC, &lmode, "43");
                write(unit, "\n", 1);
        }
#else
        if (didfds==0 )  {
                struct termios tty;

                if (ioctl(unit, TCGETS, (char *)&tty) == 0 &&
                    tty.c_lflag & FLUSHO ) {
                        tty.c_lflag &= ~FLUSHO;
                        IOCTL(unit, TCSETS, (char *)&tty, "43");
                }
        }
#endif

	FULL_FS( write(unit, linbuf, linp - linbuf) );
	linp = linbuf;
}

void
plist(register struct varent *vp)
{

	if (setintr)
		sigrelse(SIGINT);
	for (vp = vp->link; vp != 0; vp = vp->link) {
		int len = blklen(vp->vec);

		printf((char *)vp->name);
		printf("\t");
		if (len != 1)
			display_char('(');
		blkpr(vp->vec);
		if (len != 1)
			display_char(')');
		printf("\n");
	}
	if (setintr)
		sigrelse(SIGINT);
}
