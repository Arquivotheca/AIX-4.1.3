static char sccsid[] = "@(#)27	1.15  src/bos/usr/bin/uucp/uucpd.c, cmduucp, bos411, 9428A410j 11/11/93 15:19:31";
/* 
 * COMPONENT_NAME: UUCP uucpd.c
 * 
 * FUNCTIONS: Muucpd, SCPYN, doit, dologin, dologout, dosrcpacket, 
 *            readline, wait3 
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

/*
 * AIX, 4.2BSD or 2.9BSD TCP/IP server for uucico
 * uucico's TCP channel causes this server to be run at the remote end.
 */

#define AIX
#define BSDINETD

#include "uucp.h"
#include <netdb.h>
#include <sys/syslog.h>
#ifdef BSD2_9
#include <sys/localopts.h>
#include <sys/file.h>
#endif BSD2_9
#include <signal.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <termio.h>

#ifdef AIX
#include <sys/ioctl.h>
#include <sys/wait.h>
#else
#include <sys/ioctl.h>
#endif AIX

#include <pwd.h>
#ifndef AIX
uucp/#include <lastlog.h>
#endif !AIX

#ifdef AIX
/*
 * The following declerations are for support of the System Resource    
 * Controller (SRC) - .   
 */
#include <spc.h>
static 	struct srcreq srcpacket;
int 	cont;
struct 	srchdr *srchdr;
char	progname[128];
#define SRC_FD 		13
#define SRCMSG     	(sizeof(srcpacket))
#endif AIX

#if !defined(AIX) && !defined(BSD2_9)
--- You must have either AIX or BSD2_9 defined for this to work
#endif !AIX && !BSD2_9
#if defined(AIX) && defined(BSD2_9)
--- You may not have both AIX and BSD2_9 defined for this to work
#endif	/* check for stupidity */

#ifndef AIX
char lastlog[] = "/usr/adm/lastlog";
#endif !AIX
struct	sockaddr_in hisctladdr;
int hisaddrlen = sizeof hisctladdr;
struct	sockaddr_in myctladdr;
int mypid;

static char Username[64];
/*
char *nenv[] = {
	Username,
	NULL,
};
extern char **environ;
*/

nl_catd catd;
main(argc, argv)
int argc;
char **argv;
{
#ifndef BSDINETD
	register int s, tcp_socket;
	struct servent *sp;
#endif !BSDINETD
	extern int errno;
	int dologout();
	void trace_handler(int);
	struct sigvec sv;     /* for SRC socket debug code */

	setlocale(LC_ALL,"");
	catd = catopen(MF_UUCP,NL_CAT_LOCALE);


#if 0
/*
 * SRC local variables and initial startup.  The program name is saved  
 * from the command line.  Stdin (fd 0) is checked to make sure that it
 * is the SRC socket descriptor.  Once verified the descriptor is duped
 * so that stdin can be used internally by uucpd - .
 */
	int rc, i, addrsz;
	struct sockaddr srcaddr;
	int src_exists = TRUE;

	strcpy(progname,argv[0]);
	addrsz = sizeof(srcaddr);
	if ((rc = getsockname(0, &srcaddr, &addrsz)) < 0) {
		fprintf(stderr,MSGSTR(MSG_UUCPD_6,
			"%s: ERROR: '%d' getsockname, SRC not found, continuing without SRC support\n"), progname,errno);
		 src_exists = FALSE;
	}
	if (src_exists)
		(void) dup2(0, SRC_FD);
#endif 0

/*
** set up the signal-handling for SRC to turn TRACE on and off.
*/

	bzero((char *)&sv, sizeof(sv));
	sv.sv_mask = sigmask(SIGUSR2);
	sv.sv_handler = trace_handler;
	sigvec(SIGUSR1, &sv, (struct sigvec *)0);
	sv.sv_mask = sigmask(SIGUSR1);
	sv.sv_handler = trace_handler;
	sigvec(SIGUSR2, &sv, (struct sigvec *)0);


/*
	environ = nenv;
*/

#ifdef BSDINETD
	close(1); close(2);
	dup(0); dup(0);
	hisaddrlen = sizeof (hisctladdr);
	if (getpeername(0, &hisctladdr, &hisaddrlen) < 0) {
		fprintf(stderr, "%s: ", argv[0]);
		perror("getpeername");
		_exit(1);
	}
	if (fork() == 0)
		doit(&hisctladdr);
	dologout();
	exit(1);
#else !BSDINETD
	sp = getservbyname("uucp", "tcp");
	if (sp == NULL){
		perror("uucpd: getservbyname");
		exit(1);
	}
/*
 * fork only if SRC exists.
 */
	if (!src_exists)
 		if (fork())
 			exit(0);
	if ((s=open("/dev/tty", 2)) >= 0){
#ifdef AIX
/*
 *		(void) setpgrp(1);
 */
	if (!src_exists)
 		(void) setsid();
#else
		ioctl(s, TIOCNOTTY, (char *)0);
#endif
		close(s);
	}

	bzero((char *)&myctladdr, sizeof (myctladdr));
	myctladdr.sin_family = AF_INET;
	myctladdr.sin_port = sp->s_port;
#ifdef AIX
	tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (tcp_socket < 0) {
		perror("uucpd: socket");
		exit(1);
	}
	if (bind(tcp_socket, (char *)&myctladdr, sizeof (myctladdr)) < 0) {
		perror("uucpd: bind");
		exit(1);
	}
	listen(tcp_socket, 3);	/* at most 3 simultaneuos uucp connections */
	signal(SIGCLD, (void(*)(int)) dologout);

	for(;;) {

		int ibits;
		int n;
	    	int onoff;

		cont=END;
		addrsz=sizeof(srcaddr);
		do {
			signal(SIGCLD, (void(*)(int)) dologout);
			ibits = 1 << tcp_socket;
			if (src_exists)
				ibits |= 1 << SRC_FD;
			n = select(20,&ibits,0,0,(struct timeval *)NULL);

			if (ibits & (1 << tcp_socket)) {
				s = accept(tcp_socket, &hisctladdr, &hisaddrlen);
				if (s < 0){
					if (errno == EINTR) 
						continue;
					perror("uucpd: accept");
					exit(1);
				}
				if (fork() == 0) {
					close(0); close(1); close(2);
					dup(s); dup(s); dup(s);
					close(tcp_socket); close(s);
					doit(&hisctladdr);
					exit(1);
				}
				close(s);
			}

			rc = -1;
			errno = EINTR;
			if (src_exists && (ibits & (1 << SRC_FD)))
				rc = recvfrom(SRC_FD,&srcpacket,SRCMSG,0,&srcaddr,&addrsz);
 		} while (rc == -1 && errno == EINTR);
		if (rc < 0) {
			fprintf(stderr, MSGSTR(MSG_UUCPD_7,
				"%s: ERROR: '%d' recvfrom\n"),progname,errno);
			exit(1);
		}

		if (src_exists) {
		    /* process the request packet */
		    switch(srcpacket.subreq.action) {
			case START:
			case REFRESH:
			case STATUS:
				dosrcpacket(SRC_SUBMSG,progname,MSGSTR(MSG_UUCPD_8,"ERROR: uucpd does not support this option.\n"),sizeof(struct srcrep));
				break;
			case STOP:
				if (srcpacket.subreq.object == SUBSYSTEM) {
					dosrcpacket(SRC_OK,argv[0],NULL,sizeof(struct srcrep));
					exit(0);
				} else
					dosrcpacket(SRC_SUBMSG,progname,MSGSTR(MSG_UUCPD_8,"ERROR: uucpd does not support this option.\n"),sizeof(struct srcrep));
				break;
			case TRACE:
				if (srcpacket.subreq.object == SUBSYSTEM) { 
					onoff = (srcpacket.subreq.parm2 == TRACEON) ? 1 : 0;
					if (setsockopt(tcp_socket,SOL_SOCKET,SO_DEBUG, &onoff, sizeof (onoff)) < 0) {
						fprintf(stderr, MSGSTR(MSG_UUCPD_9,"setsockopt SO_DEBUG: %m"));
						close(tcp_socket);
						exit(1);
					}
					dosrcpacket(SRC_OK,argv[0],NULL,sizeof(struct srcrep)); 
				} else
					dosrcpacket(SRC_SUBMSG,progname,MSGSTR(MSG_UUCPD_8,"ERROR: uucpd does not support this option.\n"),sizeof(struct srcrep));
				break;
			default:
				dosrcpacket(SRC_SUBICMD,argv[0],NULL,sizeof(struct srcrep));
				break;

		    }
		}
	}
#endif AIX


#ifdef BSD2_9
	for(;;) {
		signal(SIGCHLD, dologout);
		s = socket(SOCK_STREAM, 0,  &myctladdr,
			SO_ACCEPTCONN|SO_KEEPALIVE);
		if (s < 0) {
			perror("uucpd: socket");
			exit(1);
		}
		if (accept(s, &hisctladdr) < 0) {
			if (errno == EINTR) {
				close(s);
				continue;
			}
			perror("uucpd: accept");
			exit(1);
		}
		if (fork() == 0) {
			close(0); close(1); close(2);
			dup(s); dup(s); dup(s);
			close(s);
			doit(&hisctladdr);
			exit(1);
		}
	}
#endif BSD2_9
#endif	!BSDINETD
}

/*
 * SRC packet processing - .
 */
int dosrcpacket(msgno, subsys, txt, len)
	int msgno;
	char *subsys;
	char *txt;
	int len;
{
	struct srcrep reply;

	reply.svrreply.rtncode = msgno;
	strcpy(reply.svrreply.objname, subsys);
	strcpy(reply.svrreply.rtnmsg, txt);
	srchdr = srcrrqs(&srcpacket);
	srcsrpy(srchdr, &reply, len, cont);
}

doit(sinp)
struct sockaddr_in *sinp;
{
	char user[64], passwd[64];
	char *xpasswd, *crypt();
	struct passwd *pw;

	alarm(60);
	printf("login: "); fflush(stdout);
	if (readline(user, sizeof user) < 0) {
		fprintf(stderr, MSGSTR(MSG_UUCPD_1, "user read\n"));
		return;
	}
	/* truncate username to 8 characters */
	user[8] = '\0';
	pw = getpwnam(user);
	if (pw == NULL) {
		fprintf(stderr, MSGSTR(MSG_UUCPD_2, "user unknown\n"));
		return;
	}
  	if ((strcmp(pw->pw_shell, UUCICO)) &&
	    (strcmp(pw->pw_shell, UUCICO_ALT))) {
		fprintf(stderr, MSGSTR(MSG_UUCPD_3, "Login incorrect."));
		return;
	}
	if (pw->pw_passwd && *pw->pw_passwd != '\0') {
		printf("Password: "); fflush(stdout);
		if (readline(passwd, sizeof passwd) < 0) {
			fprintf(stderr, MSGSTR(MSG_UUCPD_4, "passwd read\n"));
			return;
		}
		xpasswd = crypt(passwd, pw->pw_passwd);
		if (strcmp(xpasswd, pw->pw_passwd)) {
		     fprintf(stderr, MSGSTR(MSG_UUCPD_5, "Login incorrect."));
		     return;
		}
	}
	alarm(0);
	sprintf(Username, "USER=%s", user);
	(void)putenv(Username);
	dologin(pw, sinp);
	setgid(pw->pw_gid);
#ifdef AIX
	initgroups(pw->pw_name, pw->pw_gid);
#endif AIX
	chdir(pw->pw_dir);
	setuid(pw->pw_uid);
#ifdef AIX
	execl(UUCICO, "uucico", (char *)0);
#endif AIX
#ifdef BSD2_9
	sprintf(passwd, "-h%s", inet_ntoa(sinp->sin_addr));
	execl(UUCICO, "uucico", passwd, (char *)0);
#endif BSD2_9
	perror("uucico server: execl");
}

readline(p, n)
register char *p;
register int n;
{
	char c;

	while (n-- > 0) {
		if (read(0, &c, 1) <= 0)
			return(-1);
		c &= 0177;
		if (c == '\n' || c == '\r') {
			*p = '\0';
			return(0);
		}
		*p++ = c;
	}
	return(-1);
}

#include <utmp.h>
#ifdef AIX
#include <fcntl.h>
#endif AIX

#ifdef BSD2_9
#define O_APPEND	0 /* kludge */
#define wait3(a,b,c)	wait2(a,b)
#endif BSD2_9

#define	SCPYN(a, b)	strncpy(a, b, sizeof (a))

struct	utmp utmp;

dologout()
{
	union wait status;
	int pid, wtmp;

#ifdef BSDINETD
	while ((pid=wait((int *) &status)) > 0) {
#else  !BSDINETD
	while ((pid=wait3(&status,WNOHANG,0)) > 0) {
#endif !BSDINETD
		wtmp = open("/usr/adm/wtmp", O_WRONLY|O_APPEND);
		if (wtmp >= 0) {
			sprintf(utmp.ut_line, "uucp%.4d", pid);
#ifdef AIX
			SCPYN(utmp.ut_user, "");
#else
			SCPYN(utmp.ut_name, "");
			SCPYN(utmp.ut_host, "");
#endif AIX
			(void) time(&utmp.ut_time);
#ifdef BSD2_9
			(void) lseek(wtmp, 0L, 2);
#endif BSD2_9
			(void) write(wtmp, (char *)&utmp, sizeof (utmp));
			(void) close(wtmp);
		}
	}
}

/*
 * Record login in wtmp file.
 */
dologin(pw, sin)
struct passwd *pw;
struct sockaddr_in *sin;
{
	char line[32];
	char remotehost[32];
	int wtmp, f;
	struct hostent *hp = gethostbyaddr(&sin->sin_addr,
		sizeof (struct in_addr), AF_INET);

	if (hp) {
		strncpy(remotehost, hp->h_name, sizeof (remotehost));
		endhostent();
	} else
		strncpy(remotehost, inet_ntoa(sin->sin_addr),
		    sizeof (remotehost));
	wtmp = open("/usr/adm/wtmp", O_WRONLY|O_APPEND);
	if (wtmp >= 0) {
		/* hack, but must be unique and no tty line */
		sprintf(line, "uucp%.4d", getpid());
		SCPYN(utmp.ut_line, line);
#ifdef AIX
		SCPYN(utmp.ut_user, pw->pw_name);
#else
		SCPYN(utmp.ut_name, pw->pw_name);
		SCPYN(utmp.ut_host, remotehost);
#endif AIX
		time(&utmp.ut_time);
#ifdef BSD2_9
		(void) lseek(wtmp, 0L, 2);
#endif BSD2_9
		(void) write(wtmp, (char *)&utmp, sizeof (utmp));
		(void) close(wtmp);
	}
#ifndef AIX
	if ((f = open(lastlog, 2)) >= 0) {
		struct lastlog ll;

		time(&ll.ll_time);
		lseek(f, (long)pw->pw_uid * sizeof(struct lastlog), 0);
		strcpy(line, remotehost);
		SCPYN(ll.ll_line, line);
		SCPYN(ll.ll_host, remotehost);
		(void) write(f, (char *) &ll, sizeof ll);
		(void) close(f);
	}
#endif !AIX
}
/*
** trace_handler - SRC TRACE ON/OFF signal handler
*/
void
trace_handler(int sig)
{
	int	onoff;

	onoff = (sig == SIGUSR1) ? 1 : 0;
	if (setsockopt(0, SOL_SOCKET, SO_DEBUG, &onoff, sizeof (onoff)) < 0)
		syslog(LOG_ERR, "setsockopt: %m");
}
