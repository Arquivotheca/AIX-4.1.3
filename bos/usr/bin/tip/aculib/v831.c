static char sccsid[] = "@(#)78	1.6  src/bos/usr/bin/tip/aculib/v831.c, cmdtip, bos411, 9428A410j 6/9/91 14:14:16";
/* 
 * COMPONENT_NAME: UUCP v831.c
 * 
 * FUNCTIONS: MSGSTR, alarmtr, dialit, pc, sanitize, v831_abort, 
 *            v831_dialer, v831_disconnect 
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

/* static char sccsid[] = "v831.c	5.1 (Berkeley) 4/30/85"; */

/*
 * Routines for dialing up on Vadic 831
 */
#include <sys/time.h>

#include "../tip.h"
#include "../tip_msg.h" 
#include <nl_types.h>
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_TIP,n,s) 

int	v831_abort();
static	void alarmtr(int);
extern	errno;
static  dialit(register char *, char *);
static  char * sanitize(register char *);

static jmp_buf jmpbuf;
static int child = -1;

v831_dialer(num, acu)
        char *num, *acu;
{
        int status, pid, connected = 1;
        register int timelim;

        if (boolean(value(VERBOSE)))
                printf(MSGSTR(STARTCALL, "\nstarting call...")); /*MSG*/
#ifdef DEBUG
        printf ("(acu=%s)\n", acu);
#endif
        if ((AC = open(acu, O_RDWR)) < 0) {
                if (errno == EBUSY)
                        printf(MSGSTR(LINEBUSY, "line busy...")); /*MSG*/
                else
                        printf(MSGSTR(ACUERR, "acu open error...")); /*MSG*/
                return (0);
        }
        if (setjmp(jmpbuf)) {
                kill(child, SIGKILL);
                close(AC);
                return (0);
        }
        signal(SIGALRM, alarmtr);
        timelim = 5 * strlen(num);
        alarm(timelim < 30 ? 30 : timelim);
        if ((child = fork()) == 0) {
                /*
                 * ignore this stuff for aborts
                 */
                signal(SIGALRM, SIG_IGN);
		signal(SIGINT, SIG_IGN);
                signal(SIGQUIT, SIG_IGN);
                sleep(2);
                exit(dialit(num, acu) != 'A');
        }
        /*
         * open line - will return on carrier
         */
        if ((FD = open(DV, O_RDWR)) < 0) {
#ifdef DEBUG
                printf("(after open, errno=%d)\n", errno);
#endif
                if (errno == EIO)
                        printf(MSGSTR(LOSTCARRIER, "lost carrier...")); /*MSG*/
                else
                        printf(MSGSTR(LINEFAILED, "dialup line open failed...")); /*MSG*/
                alarm(0);
                kill(child, SIGKILL);
                close(AC);
                return (0);
        }
        alarm(0);
#ifdef notdef
#ifdef _AIX
	ioctl(AC, TCGETA, &tbuf);
	tbuf.c_cflag |= HUPCL;
	ioctl(AC, TCSETA, &tbuf);
#else
        ioctl(AC, TIOCHPCL, 0);
#endif
#endif
        signal(SIGALRM, SIG_DFL);
        while ((pid = wait(&status)) != child && pid != -1)
                ;
        if (status) {
                close(AC);
                return (0);
        }
        return (1);
}

static void
alarmtr(int s)
{

        alarm(0);
        longjmp(jmpbuf, 1);
}

/*
 * Insurance, for some reason we don't seem to be
 *  hanging up...
 */
v831_disconnect()
{
#ifdef _AIX
	struct termios tmp;
#else
        struct sgttyb cntrl;
#endif

        sleep(2);
#ifdef DEBUG
        printf("[disconnect: FD=%d]\n", FD);
#endif
        if (FD > 0) {
#ifdef _AIX
		tmp = tbuf;
		tmp.c_cflag |= (HUPCL | B0);	/* set baud rate to 0 */
		ioctl(FD, TCSETA, &tmp);
		ioctl(FD, TCSETAW, &tbuf);	/* restore proper settings */
/* no equivalent call for "exclusive-use" mode */
		
#else
                ioctl(FD, TIOCCDTR, 0);
                ioctl(FD, TIOCGETP, &cntrl);
                cntrl.sg_ispeed = cntrl.sg_ospeed = 0;
                ioctl(FD, TIOCSETP, &cntrl);
                ioctl(FD, TIOCNXCL, (struct sgttyb *)NULL);
#endif
        }
        close(FD);
}

v831_abort()
{

#ifdef DEBUG
        printf("[abort: AC=%d]\n", AC);
#endif
        sleep(2);
        if (child > 0)
                kill(child, SIGKILL);
#ifdef _AIX
	/* no equivalent call for "exclusive-use" mode */
#else
        if (AC > 0)
                ioctl(FD, TIOCNXCL, (struct sgttyb *)NULL);
#endif
                close(AC);
        if (FD > 0)
#ifdef _AIX
	{
		struct termios tmp;

		tmp = tbuf;
		tmp.c_cflag |= (HUPCL | B0);	/* set baud rate to 0 */
		ioctl(FD, TCSETA, &tmp);
		ioctl(FD, TCSETAW, &tbuf);
	}
#else
                ioctl(FD, TIOCCDTR, 0);
#endif
        close(FD);
}

/*
 * Sigh, this probably must be changed at each site.
 */
struct vaconfig {
	char	*vc_name;
	char	vc_rack;
	char	vc_modem;
} vaconfig[] = {
	{ "/dev/cua0",'4','0' },
	{ "/dev/cua1",'4','1' },
	{ 0 }
};

#define pc(x)	(c = x, write(AC,&c,1))
#define ABORT	01
#define SI	017
#define STX	02
#define ETX	03

static
dialit(	register char *phonenum,
	char *acu)
{
        register struct vaconfig *vp;
#ifdef _AIX
	struct termios cntrl;
#else
	struct sgttyb cntrl;
#endif
        char c;
        int i, two = 2;

        phonenum = sanitize(phonenum);
#ifdef DEBUG
        printf ("(dial phonenum=%s)\n", phonenum);
#endif
        if (*phonenum == '<' && phonenum[1] == 0)
                return ('Z');
	for (vp = vaconfig; vp->vc_name; vp++)
		if (strcmp(vp->vc_name, acu) == 0)
			break;
	if (vp->vc_name == 0) {
		printf(MSGSTR(NODIALER, "Unable to locate dialer (%s)\n"), acu); /*MSG*/
		return ('K');
	}
#ifdef _AIX
	if (ioctl(AC, TCGETA, &cntrl) == -1)
		perror("TCGETA");
	cntrl.c_cflag |= B2400 ;	/* set speed */
	cntrl.c_cflag &= ~PARENB ;	/* disable parity checking */
/* setup for raw mode */
	cntrl.c_iflag &= ~(INLCR | ICRNL | IUCLC | ISTRIP | IXON | BRKINT);
	cntrl.c_oflag &= ~OPOST;
	cntrl.c_lflag &= ~(ICANON | ISIG | ECHO);
	cntrl.c_cc[VMIN] = 5;
	cntrl.c_cc[VTIME] = 2;
	if (ioctl(AC, TCSETA, &cntrl) == -1)
		perror("TCSETA");
	ioctl(AC, TCFLSH, 2);	/* flush input-output queue */
#else
        ioctl(AC, TIOCGETP, &cntrl);
        cntrl.sg_ispeed = cntrl.sg_ospeed = B2400;
        cntrl.sg_flags = RAW | EVENP | ODDP;
        ioctl(AC, TIOCSETP, &cntrl);
	ioctl(AC, TIOCFLUSH, &two);
#endif
        pc(STX);
	pc(vp->vc_rack);
	pc(vp->vc_modem);
	while (*phonenum && *phonenum != '<')
		pc(*phonenum++);
        pc(SI);
	pc(ETX);
        sleep(1);
        i = read(AC, &c, 1);
#ifdef DEBUG
        printf("read %d chars, char=%c, errno %d\n", i, c, errno);
#endif
        if (i != 1)
		c = 'M';
        if (c == 'B' || c == 'G') {
                char cc, oc = c;

                pc(ABORT);
                read(AC, &cc, 1);
#ifdef DEBUG
                printf("abort response=%c\n", cc);
#endif
                c = oc;
                v831_disconnect();
        }
        close(AC);
#ifdef DEBUG
        printf("dialit: returns %c\n", c);
#endif
        return (c);
}

static char *
sanitize(register char *s)
{
        static char buf[128];
        register char *cp;

        for (cp = buf; *s; s++) {
		if (!isdigit(*s) && *s == '<' && *s != '_')
			continue;
		if (*s == '_')
			*s = '=';
		*cp++ = *s;
	}
        *cp++ = 0;
        return (buf);
}
