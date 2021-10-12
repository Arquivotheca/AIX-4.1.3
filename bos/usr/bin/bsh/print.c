static char sccsid[] = "@(#)12	1.25  src/bos/usr/bin/bsh/print.c, cmdbsh, bos411, 9428A410j 9/1/93 17:35:25";
/*
 * COMPONENT_NAME: (CMDBSH) Bourne shell and related commands
 *
 * FUNCTIONS: prp prt prs prc prn itos stoi prl flushb  prc_buff prs_buff 
 *	      clear_buff prs_cntl prn_buff
 *
 * ORIGINS: 3, 26, 27, 71
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 * Copyright 1976, Bell Telephone Laboratories, Inc.
 *
 * 1.19  com/cmd/sh/sh/print.c, bos320, 9142320i 10/16/91 09:54:07	
 *
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 *
 * OSF/1 1.1
 */

#include	"defs.h"
#include	<time.h>
#include 	<errno.h>
#include	<sys/limits.h>

#define		BUFLEN		256

static uchar_t	buffer[BUFLEN];
static int	index = 0;
uchar_t		numbuf[17];	/*  changed from 12 to 14   */
                                /*  changed form 14 to 17   */

void	prc_buff();
void	prs_buff();
void	prn_buff();
void	prs_cntl();
void	prn_buff();
void	itos();
void	prs();

/*
 * printing and io conversion
 */
void
prp()
{
	if ((flags & prompt) == 0 && cmdadr)
	{
		prs_cntl(cmdadr);
		prs(colon);
	}
}

void
prs(as)
uchar_t	*as;
{
	register uchar_t	*s;

	if (s = as) {
		if (NLSisencoded(s))
			s = NLSndecode(s);
		write(output, s, length(s) - 1);
	}
}

void
prc(c)
uchar_t	c;
{
	if (c)
		write(output, &c, 1);
}

void
prt(t)
clock_t	t;
{
	register int hr, min, sec;

	t += 60 / 2;
	t /= 60;
	sec = t % 60;
	t /= 60;
	min = t % 60;

	if (hr = t / 60)
	{
		prn_buff(hr);
		prc_buff('h');
	}

	prn_buff(min);
	prc_buff('m');
	prn_buff(sec);
	prc_buff('s');
}

void
prn(n)
	int	n;
{
	itos(n);

	prs(numbuf);
}

void
itos(n)
	int n;
{
	register uchar_t *abuf;
	register unsigned a, i;
	int pr, d;

	abuf = numbuf;

	pr = FALSE;
	a = n;
	for (i = 1000000000; i != 1; i /= 10)
	{
		if ((pr |= (d = a / i)))
			*abuf++ = d + '0';
		a %= i;
	}
	*abuf++ = a + '0';
	*abuf++ = 0;
}

int
stoi(icp)
uchar_t	*icp;
{
	register uchar_t	*cp = icp;
	register int	r = 0;
	register uchar_t	c;

	if (NLSisencoded(icp)) 
		NLSdecode(icp);

	while ((c = *cp, digit(c)) && c && r >= 0)
	{
		r = r * 10 + c - '0';
		cp++;
	}
	if (r < 0 || cp == icp)
		failed(icp, MSGSTR(M_BADNUM,(char *)badnum));
	else
		return(r);
}

void
prl(n)
long n;
{
	int i;

	i = 11;
	while (n > 0 && --i >= 0)
	{
		numbuf[i] = n % 10 + '0';
		n /= 10;
	}
	numbuf[11] = '\0';
	prs_buff(&numbuf[i]);
}

#define FULL_FS( AAA ) if ( AAA < 0 && errno == ENOSPC ) { \
                       prs(MSGSTR(M_FULLFS, "error: File system is full")) ; \
                       newline() ; \
                       exitval |= 1 ; }
void
flushb()
{
	if (index)
	{
		buffer[index] = '\0';
		FULL_FS( write(1, buffer, length(buffer) - 1) );
		index = 0;
	}
}

void
prc_buff(c)
	uchar_t c;
{
	if (c)
	{
		if (index + 1 >= BUFLEN)
			flushb();

		buffer[index++] = c;
	}
	else
	{
		flushb();
		write(1, &c, 1);
	}
}

void
prs_buff(s)
	uchar_t *s;
{
	register int len;
	if (NLSisencoded(s))
		s = NLSndecode(s);
	len = length(s) - 1;

	if (index + len >= BUFLEN)
		flushb();

	if (len >= BUFLEN)
		write(1, s, len);
	else
	{
		movstr(s, &buffer[index]);
		index += len;
	}
}

void
clear_buff()
{
	index = 0;
}


void
prs_cntl(s)
	uchar_t *s;
{
	register uchar_t *ptr = buffer;
	register int c;
	register int mblength;

	if (NLSisencoded(s))
		s++;
/*
 * this line was changed to check the boundry of the buffer so
 * that it doesn't exceed BUFLEN, because when it does it trashes
 * another static variable "cwdname" in pwd.c and causes the 
 * internal pwd command to become broken when the input is 
 * >127 control characters, dewey coffman, 7-30-87
 */

	while((*s != '\0') && ((ptr - buffer) < (BUFLEN - 1)))
	{
		/* translate a control character into a printable sequence */
		c = *s;
		mblength = mblen(s, MBMAX);
		if (mblength > 1) {
			while (mblength--) {
				*ptr++ = *s++;
			}
			continue;
		}
		if ((c < '\040') && (!NLSfontshift (c))
			&& (c != '\n') && (c != '\t'))
		/* magic font shifts will be handled in prs() */
		{	/* assumes ASCII uchar_t */
			*ptr++ = '^';
			*ptr++ = (c + 0100);	/* assumes ASCII uchar_t */
		}
		else if (c == 0177)
		{	/* '\0177' does not work */
			*ptr++ = '^';
			*ptr++ = '?';
		}
		else 
		{	/* printable character */
			*ptr++ = c;
		}

		++s;
	}

	*ptr = '\0';
	prs(buffer);
}

void
prn_buff(n)
	int	n;
{
	itos(n);

	prs_buff(numbuf);
}
