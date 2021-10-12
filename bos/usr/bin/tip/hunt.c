static char sccsid[] = "@(#)37	1.10  src/bos/usr/bin/tip/hunt.c, cmdtip, bos411, 9428A410j 3/11/94 09:00:26";
/* 
 * COMPONENT_NAME: UUCP hunt.c
 * 
 * FUNCTIONS: MSGSTR, dead, hunt 
 *
 * ORIGINS: 10  26  27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

/* static char sccsid[] = "hunt.c	5.1 (Berkeley) 4/30/85"; */

#include "tip.h"
#include <nl_types.h>
#include "tip_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_TIP,n,s) 

extern char *getremote();
extern char *rindex();

static	jmp_buf deadline;
static	int deadfl;

dead(int s)
{

	deadfl = 1;
	longjmp(deadline, 1);
}

hunt(name)
	char *name;
{
	register char *cp;	/* Name of tty to open */
	void (*f)(int);

	f = signal(SIGALRM, (void(*)(int)) dead);
	deadfl = 0;
	while (cp = getremote(name)) {
		uucplock = rindex(cp, '/')+1;
		if (ttylock(uucplock) < 0) {
			ttyunlock(uucplock);
			continue;
		}
		/*
		 * Straight through call units, such as the BIZCOMP,
		 * VADIC and the DF, must indicate they're hardwired in
		 *  order to get an open file descriptor placed in FD.
		 * Otherwise, as for a DN-11, the open will have to
		 *  be done in the "open" routine.
		 */
		if (!HW)
			break;
		if (setjmp(deadline) == 0) {
			alarm(10);
			FD = open(cp, (O_RDWR|O_NDELAY));
		}
		alarm(0);
		if (FD < 0) {
			perror(cp);
			deadfl = 1;
		}
		if (!deadfl) {
#ifdef _AIX
			tcgetattr(FD,&FDbuf);
			FDbufsave = FDbuf;
#  ifdef DUNZEL
			lockf(FD, F_LOCK, 0);
#  endif
			/* hang up on last close, turn DCD ctrl 
			 * off till connected, don't echo stuff back to
			 * remote
			 */
			FDbuf.c_cflag |= (HUPCL|CLOCAL);
			FDbuf.c_iflag &= ~ICRNL;
			FDbuf.c_lflag &= (~ECHO & ~ECHONL & ~ECHOE & ~ECHOK & \
					  ~ECHOCTL & ~ECHOK & ~ECHOPRT & \
					  ~ISIG & ~ICANON);
			tcsetattr(FD,TCSAFLUSH,&FDbuf);
			fcntl(FD, F_SETFL, (fcntl(FD,F_GETFL,0) & ~O_NDELAY));
#else
			ioctl(FD, TIOCEXCL, 0);
			ioctl(FD, TIOCHPCL, 0);
#endif
			signal(SIGALRM, SIG_DFL);
			return ((int)cp);
		}
		ttyunlock(uucplock);
	}
	signal(SIGALRM,(void(*)(int)) f);
	return (deadfl ? -1 : (int)cp);
}
