static char sccsid[] = "@(#)88	1.11  src/bos/usr/bin/uucp/culine.c, cmduucp, bos411, 9428A410j 7/13/94 10:49:15";
/* 
 * COMPONENT_NAME: CMDUUCP culine.c
 * 
 * FUNCTIONS: fixline, genbrk, restline, savline, sethup, setline 
 *
 * ORIGINS: 10  27  3 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*	culine.c --differs from uucp
	line.c	2.4	1/28/84 00:56:32
	only in fixline() where termios line parameters
	for cu are set before remote connection is made.
*/
#include "uucp.h"
/* VERSION( culine.c	5.2 -  -  ); */
#ifdef ATTSV
#include <termios.h>
#else
#include <termio.h>
#endif
#include <sys/termiox.h>

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
extern int	Modemctrl;	/* for soft carrier option */

extern Oddflag, Evenflag, Duplex, Terminal, Sstop;	/*for cu options*/
extern char *P_PARITY;

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
	struct termios		lv;
	struct termiox		tiox;
	int			speed = -1;
	char                    msgb[NL_TEXTMAX];

	sprintf( msgb, MSGSTR(MSG_CULINECD1, "fixline(%d, %d)\n"),
		tty, spwant);
	CDEBUG(6, "%s", msgb);

	if (tcgetattr(tty, &lv) != 0)
		return;

/* set line attributes associated with -h, -t, -e, and -o options */

	lv.c_iflag = lv.c_oflag = lv.c_lflag = (tcflag_t)0;
	lv.c_cc[VEOF] = '\1';

        /* RTS/CTS is the preferred method of input flow control, */
        /* but only use it if the user has already configured */
        /* the port is such a way . . . */

	bzero(&tiox, sizeof(tiox));
	ioctl(tty, TCGETX, &tiox);

        if (tiox.x_hflag & RTSXOFF) {
                lv.c_iflag = (IGNPAR | IGNBRK);
                lv.c_iflag &= ~(IXON | IXOFF);
                Sstop = 0;
        }
	else {
                lv.c_iflag = (IGNPAR | IGNBRK | IXON | IXOFF);
        }

	if (spwant > 0) {
		for (ps = spds; ps->sp_val; ps++)
			if (ps->sp_val == spwant) {
				speed = ps->sp_name;
				break;
			}
		ASSERT(speed >= 0, MSGSTR(MSG_CULINEA1,"BAD SPEED"), "", speed);
		lv.c_cflag = (tcflag_t)0;
		cfsetospeed(&lv, speed);
		cfsetispeed(&lv, speed);
	} else
		lv.c_cflag &= CBAUD;

	lv.c_cflag |= ( CREAD | (speed ? HUPCL : 0));
	
	if(Evenflag) {				/*even parity -e */
		if(lv.c_cflag & PARENB) {
			VERBOSE(P_PARITY, 0);
			exit(1);
		}else
			lv.c_cflag |= (PARENB | CS7);
	}
	else if(Oddflag) {			/*odd parity -o */
		if(lv.c_cflag & PARENB) {
			VERBOSE(P_PARITY, 0);
			exit(1);
		}else {
			lv.c_cflag |= PARODD;
			lv.c_cflag |= (PARENB | CS7);
		}
	}
	else 					/*no parity */
		lv.c_cflag |= CS8;

	if(!Duplex)				/*half duplex -h */
		lv.c_iflag &= ~(IXON | IXOFF);
	if(Terminal)				/* -t */
		lv.c_oflag |= (OPOST | ONLCR);

	/*   CLOCAL may cause problems on pdp11s with DHs */
	if ((Modemctrl == 1) && (type == D_DIRECT)) {
		CDEBUG(4, MSGSTR(MSG_CULINECD2,"fixline - direct\n"), "");
		lv.c_cflag |= CLOCAL;
	} else
		lv.c_cflag &= ~CLOCAL;
	
	ASSERT(tcsetattr(tty, TCSANOW, &lv) >= 0,
	    MSGSTR(MSG_CULINEA2, "RETURN FROM fixline ioctl"), "", errno);
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
		if (tbuf.c_cc[VMIN] != HEADERSIZE) {
		    tbuf.c_cc[VMIN] = HEADERSIZE;
		    (void) tcsetattr(Ifn, TCSADRAIN, &tbuf);
		}
		break;
	}
}

savline()
{
	int ret;

	ret = tcgetattr(0, &Savettyb);
	Savettyb.c_oflag |= OPOST;
	Savettyb.c_lflag |= (ISIG|ICANON|ECHO);
	return(ret);
}

restline()
{
	return(tcsetattr(0, TCSANOW, &Savettyb));
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
	char            msgb[NL_TEXTMAX];

	sprintf( msgb, MSGSTR(MSG_CULINECD1, "fixline(%d, %d)\n"),
		tty, spwant);
	CDEBUG(6, "%s", msgb);

	if (ioctl(tty, TIOCGETP, &ttbuf) != 0)
		return;
	if (spwant > 0) {
		for (ps = spds; ps->sp_val; ps++)
			if (ps->sp_val == spwant) {
				speed = ps->sp_name;
				break;
			}
		ASSERT(speed >= 0, MSGSTR(MSG_CULINEA1,"BAD SPEED"), "", speed);
		ttbuf.sg_ispeed = ttbuf.sg_ospeed = speed;
	} else {
		for (ps = spds; ps->sp_val; ps++)
			if (ps->sp_name == ttbuf.sg_ispeed) {
				spwant = ps->sp_val;
				break;
			}
		ASSERT(spwant >= 0, MSGSTR(MSG_CULINEA1,"BAD SPEED"),
			 "", spwant);
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

ttygenbrk(fn)
{
	if (isatty(fn)) {
		(void) ioctl(fn, TIOCSBRK, 0);
		nap(6);				/* 0.1 second break */
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
	int	ret;

	ret = ioctl(0, TIOCGETP, &Savettyb);
	Savettyb.sg_flags |= ECHO;
	Savettyb.sg_flags &= ~RAW;
	return(ret);
}

restline()
{
	return(ioctl(0, TIOCSETP, &Savettyb));
}
#endif
