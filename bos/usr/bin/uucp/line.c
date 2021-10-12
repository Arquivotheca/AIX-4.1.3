static char sccsid[] = "@(#)04	1.9  src/bos/usr/bin/uucp/line.c, cmduucp, bos411, 9428A410j 6/17/93 14:21:16";
/* 
 * COMPONENT_NAME: CMDUUCP line.c
 * 
 * FUNCTIONS: fixline, genbrk, restline, savline, sethup, setline, 
 *            sytfix2line, sytfixline 
 *
 * ORIGINS: 10  27  3 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */



/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	uucp:line.c	1.4
*/
#include "uucp.h"
/* VERSION( line.c	5.2 -  -  ); */
#ifdef ATTSV
#include <termios.h>
#else
#include <termio.h>
#endif

extern nl_catd catd;
static struct sg_spds {
	int	sp_val,
		sp_name;
} spds[] = {
	{ 300,  B300},
	{ 600,  B600},
	{1200, B1200},
	{2400, B2400},
	{4800, B4800},
	{9600, B9600},
#ifdef EXTA
	{19200,	EXTA},
#endif
#ifdef B19200
	{19200,	B19200},
#endif
#ifdef B38400
	{38400,	B38400},
#endif
	{0,    0}
};

#define PACKSIZE	64
#define HEADERSIZE	6
#define SNDFILE	'S'
#define RCVFILE 'R'
#define RESET	'X'

int	linebaudrate = 0;	/* for speedup hook in pk (unused in ATTSV) */
static int Saved_line;          /* was savline() successful?    */

#ifdef ATTSV

static struct termios Savettyb;
/*
 * set speed/echo/mode...
 *	tty 	-> terminal name
 *	spwant 	-> speed
 *	type	-> type
 *
 *	if spwant == 0, speed is untouched
 *	type is unused, but needed for compatibility
 *
 * return:  
 *	none
 */
fixline(tty, spwant, type)
int	tty, spwant, type;
{
	register struct sg_spds	*ps;
	struct termios		ttbuf;
	int			speed = -1;

	DEBUG(6, "fixline(%d, ", tty);
	DEBUG(6, "%d)\n", spwant);
	if (tcgetattr(tty, &ttbuf) != 0)
		return;
	if (spwant > 0) {
		for (ps = spds; ps->sp_val; ps++)
			if (ps->sp_val == spwant) {
				speed = ps->sp_name;
				break;
			}
		ASSERT(speed >= 0, MSGSTR(MSG_LINEA1,"BAD SPEED"), "", speed);
		cfsetospeed(&ttbuf, speed);
		cfsetispeed(&ttbuf, speed);
	} else
		ttbuf.c_cflag &= CBAUD;
	ttbuf.c_iflag = ttbuf.c_oflag = ttbuf.c_lflag = (tcflag_t)0;

#ifdef NO_MODEM_CTRL
	/*   CLOCAL may cause problems on pdp11s with DHs */
	if (type == D_DIRECT) {
		DEBUG(4, "fixline - direct\n", "");
		ttbuf.c_cflag |= CLOCAL;
	} else
#endif NO_MODEM_CTRL

	ttbuf.c_cflag &= ~CLOCAL;
	ttbuf.c_cflag &= ~PARENB;
	ttbuf.c_cflag |= (CS8 | CREAD | (speed ? HUPCL : 0));
	ttbuf.c_cc[VMIN] = HEADERSIZE;
	ttbuf.c_cc[VTIME] = 1;
	
	ASSERT(tcsetattr(tty, TCSANOW, &ttbuf) >= 0,
	    MSGSTR(MSG_LINEA2,"RETURN FROM fixline tcsetattr()"), "", errno);
	return;
}

sethup(dcf)
int	dcf;
{
	struct termios ttbuf;

	if (tcgetattr(dcf, &ttbuf) != 0)
		return;
	if (!(ttbuf.c_cflag & HUPCL)) {
		ttbuf.c_cflag |= HUPCL;
		(void) tcsetattr(dcf, TCSANOW, &ttbuf);
	}
}

ttygenbrk(fn)
register int	fn;
{
	if (isatty(fn)) 
		(void) tcsendbreak(fn, 0);
}


/*
 * optimize line setting for sending or receiving files
 * return:
 *	none
 */
setline(type)
register char	type;
{
	static struct termios tbuf;
	
	if (tcgetattr(Ifn, &tbuf) != 0)
		return;
	DEBUG(2, "setline - %c\n", type);
	switch (type) {
	case RCVFILE:
		if (tbuf.c_cc[VMIN] != PACKSIZE) {
		    tbuf.c_cc[VMIN] = PACKSIZE;
		    (void) tcsetattr(Ifn, TCSADRAIN, &tbuf);
		}
		break;

	case SNDFILE:
	case RESET:
		if (tbuf.c_cc[VMIN] != 1) {
		    tbuf.c_cc[VMIN] = 1;
		    (void) tcsetattr(Ifn, TCSADRAIN, &tbuf);
		}
		break;
	}
}

savline()
{
	int ret;

	ret = tcgetattr(0, &Savettyb);
	if (ret == 0)
		Saved_line = FALSE;
	else {
		Saved_line = TRUE; 
		Savettyb.c_oflag |= OPOST;
		Savettyb.c_lflag |= (ISIG|ICANON|ECHO);
	}
	return(0);
}

/***
 *	sytfixline(tty, spwant)	set speed/echo/mode...
 *	int tty, spwant;
 *
 *	return codes:  none
 */

sytfixline(tty, spwant)
int tty, spwant;
{
	struct termios ttbuf;
	struct sg_spds *ps;
	int speed = -1;
	int ret;

	if (tcgetattr(tty, &ttbuf) != 0)
		return;
	for (ps = spds; ps->sp_val >= 0; ps++)
		if (ps->sp_val == spwant)
			speed = ps->sp_name;
	DEBUG(4, "sytfixline - speed= %d\n", speed);
	ASSERT(speed >= 0, MSGSTR(MSG_LINEA1,"BAD SPEED"), "", speed);
	(void) tcgetattr(tty, &ttbuf);
	ttbuf.c_iflag = (tcflag_t)0;
	ttbuf.c_oflag = (tcflag_t)0;
	ttbuf.c_lflag = (tcflag_t)0;
	cfsetospeed(&ttbuf, speed);
	cfsetispeed(&ttbuf, speed);
	ttbuf.c_cflag |= (CS8|CLOCAL);
	ttbuf.c_cc[VMIN] = 6;
	ttbuf.c_cc[VTIME] = 1;
	ret = tcsetattr(tty, TCSADRAIN, &ttbuf);
	ASSERT(ret >= 0, MSGSTR(MSG_LINEA4,"RETURN FROM sytfixline"), "", ret);
	return;
}

sytfix2line(tty)
int tty;
{
	struct termios ttbuf;
	int ret;

	if (tcgetattr(tty, &ttbuf) != 0)
		return;
	ttbuf.c_cflag &= ~CLOCAL;
	ttbuf.c_cflag |= CREAD|HUPCL;
	ret = tcsetattr(tty, TCSADRAIN, &ttbuf);
	ASSERT(ret >= 0, MSGSTR(MSG_LINEA5,"RETURN FROM sytfix2line"), "", ret);
	return;
}


restline()
{
        if ( Saved_line == TRUE )
		return(tcsetattr(0, TCSANOW, &Savettyb));
        return(0);
}

#else !ATTSV

static struct sgttyb Savettyb;

/***
 *	fixline(tty, spwant, type)	set speed/echo/mode...
 *	int tty, spwant;
 *
 *	if spwant == 0, speed is untouched
 *	type is unused, but needed for compatibility
 *
 *	return codes:  none
 */

/*ARGSUSED*/
fixline(tty, spwant, type)
int tty, spwant, type;
{
	struct sgttyb	ttbuf;
	struct sg_spds	*ps;
	int		 speed = -1;

	DEBUG(6, "fixline(%d, ", tty);
	DEBUG(6, "%d)\n", spwant);

	if (ioctl(tty, TIOCGETP, &ttbuf) != 0)
		return;
	if (spwant > 0) {
		for (ps = spds; ps->sp_val; ps++)
			if (ps->sp_val == spwant) {
				speed = ps->sp_name;
				break;
			}
		ASSERT(speed >= 0, MSGSTR(MSG_LINEA1,"BAD SPEED"), "", speed);
		ttbuf.sg_ispeed = ttbuf.sg_ospeed = speed;
	} else {
		for (ps = spds; ps->sp_val; ps++)
			if (ps->sp_name == ttbuf.sg_ispeed) {
				spwant = ps->sp_val;
				break;
			}
		ASSERT(spwant >= 0, MSGSTR(MSG_LINEA1,"BAD SPEED"), "", spwant);
	}
	ttbuf.sg_flags = (ANYP | RAW);
	(void) ioctl(tty, TIOCSETP, &ttbuf);
	(void) ioctl(tty, TIOCHPCL, STBNULL);
	(void) ioctl(tty, TIOCEXCL, STBNULL);
	linebaudrate = spwant;		/* for hacks in pk driver */
	return;
}

sethup(dcf)
int	dcf;
{
	if (isatty(dcf)) 
		(void) ioctl(dcf, TIOCHPCL, STBNULL);
}

/***
 *	genbrk		send a break
 *
 *	return codes;  none
 */

genbrk(fn)
{
	if (isatty(fn)) {
		(void) ioctl(fn, TIOCSBRK, 0);
		nap(HZ/10);				/* 0.1 second break */
		(void) ioctl(fn, TIOCCBRK, 0);
	}
	return;
}

/*
 * V7 and RT aren't smart enough for this -- linebaudrate is the best
 * they can do.
 */
/*ARGSUSED*/
setline(dummy) { }

savline()
{
        if (  ioctl(0, TIOCGETP, &Savettyb) != 0 ) {
                Saved_line = FALSE;
        else {
                Saved_line = TRUE;
                Savettyb.sg_flags |= ECHO;
                Savettyb.sg_flags &= ~RAW;
        }
        return(0);
}

restline()
{
	if ( Saved_line == TRUE )
		return(ioctl(0, TIOCSETP, &Savettyb));
	return(0);
}
#endif
