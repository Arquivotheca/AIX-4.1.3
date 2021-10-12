static char sccsid[] = "@(#)58  1.22  src/bos/usr/ccs/lib/libc/rcmd.c, libcnet, bos411, 9432A411a 8/5/94 09:43:01";
/*
 * COMPONENT_NAME: LIBCNET rcmd.c
 *
 * FUNCTIONS: rcmd, rresvport, ruserok, _validuser, _checklist
 *
 * ORIGINS: 24 26  27  71
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 */

/*
#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)rcmd.c	5.11 (Berkeley) 5/6/86";
#endif LIBC_SCCS and not lint
*/

#include <sys/types.h>
#include <arpa/nameser.h>
#define MAXHOSTNAMELEN	MAXDNAME
#include <sys/ioctl.h>		/* for SIOCSPGRP */
#include <stdio.h>
#include <ctype.h>
#include <pwd.h>
#include <sys/param.h>
#include <sys/file.h>
#include <sys/signal.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/id.h>

#include <netinet/in.h>

#include <netdb.h>
#include <errno.h>
#include <nl_types.h>
#include "libc_msg.h"
#include "ts_supp.h"

#ifdef _THREAD_SAFE
#include "rec_mutex.h"
extern struct rec_mutex _domain_rmutex;
extern struct rec_mutex _ypresolv_rmutex;
#endif /* _THREAD_SAFE */

#ifdef	TCP_DEBUG
#include <sys/syslog.h>
#	define	DPRINTF(args)	dprintf args

	dprintf (args)
	char	*args;
	{
	char	**argv;
	char	buf[BUFSIZ];
	static	char	prefix[] = "rcmd: ";

		argv = &args;
		vsprintf (buf, argv[0], &argv[1]);
		syslog (LOG_DEBUG, "%s %s\n", prefix, buf);
	}
#else
#	define	DPRINTF(args)
#endif

char	*index();

#ifdef _THREAD_SAFE
rcmd_r(char **ahost, u_short rport, char *locuser, char *remuser, 
       char *cmd, int *fd2p, struct hostent_data *host_data)
#else
rcmd(char **ahost, u_short rport, char *locuser, char *remuser, 
     char *cmd, int *fd2p)
#endif /* _THREAD_SAFE */
{
	int s, timo = 1, pid, oldmask;
	struct sockaddr_in sin, sin2, from;
	char c;
	int lport = IPPORT_RESERVED - 1;
	struct hostent *hp;
	nl_catd catd;

#ifdef _THREAD_SAFE
	struct hostent host;
	hp = &host;
#endif /* _THREAD_SAFE */

	DPRINTF (("rcmd(%s, %d, %s, %s, %s, %d)", *ahost, rport, locuser, remuser, cmd, fd2p));

	pid = getpid();
#ifdef _THREAD_SAFE
	bzero(&host_data, sizeof(host_data));
	if (gethostbyname_r(*ahost, &host, host_data) < 0) {
#else
	hp = gethostbyname(*ahost);
	if (hp == 0) {
#endif /* _THREAD_SAFE */
                catd = catopen(MF_LIBC,NL_CAT_LOCALE);
		fprintf(stderr, catgets(catd,LIBCNET,NET22,
					"%s: unknown host\n"), *ahost);
		(void)catclose(catd);
		return (-1);
	}
	*ahost = hp->h_name;
#ifndef _THREAD_SAFE
	/*
	 * For thread safe case we do not mess with the signal
	 * settings - note default SIGURG is to ignore it.
	 */
	oldmask = sigblock(sigmask(SIGURG));
#endif /* _THREAD_SAFE */
	for (;;) {
		s = rresvport(&lport);
		if (s < 0) {
			if (errno == EAGAIN) {
                		catd = catopen(MF_LIBC,NL_CAT_LOCALE);
				fprintf(stderr, catgets(catd,LIBCNET,NET23,
						"socket: All ports in use\n"));
				(void)catclose(catd);
			} else
				perror("rcmd: socket");
#ifndef _THREAD_SAFE
			sigsetmask(oldmask);
#endif /* _THREAD_SAFE */
			return (-1);
		}
#ifndef aiws
		fcntl(s, F_SETOWN, pid);
#else
		/* since AIX has no F_SETOWN, we just do the ioctl */
		(void)ioctl(s, SIOCSPGRP, &pid );
#endif
		sin.sin_family = hp->h_addrtype;
		bcopy(hp->h_addr_list[0], (caddr_t)&sin.sin_addr, hp->h_length);
		sin.sin_port = rport;
		if (connect(s, (caddr_t)&sin, sizeof (sin)) >= 0)
			break;
		(void) close(s);
		if (errno == EADDRINUSE) {
			lport--;
			continue;
		}
		if (errno == ECONNREFUSED && timo <= 16) {
			sleep(timo);
			timo *= 2;
			continue;
		}
		if (hp->h_addr_list[1] != NULL) {
			int oerrno = errno;

                	catd = catopen(MF_LIBC,NL_CAT_LOCALE);
			fprintf(stderr, catgets(catd,LIBCNET,NET24,
			   "connect to address %s: "),inet_ntoa(sin.sin_addr));
			errno = oerrno;
			perror((char *)0);
			hp->h_addr_list++;
			bcopy(hp->h_addr_list[0], (caddr_t)&sin.sin_addr,
			    hp->h_length);
			fprintf(stderr, catgets(catd,LIBCNET,NET25,
				"Trying %s...\n"), inet_ntoa(sin.sin_addr));
			(void)catclose(catd);
			continue;
		}
		perror(hp->h_name);
#ifndef _THREAD_SAFE
		sigsetmask(oldmask);
#endif /* _THREAD_SAFE */
		return (-1);
	}
	lport--;
	if (fd2p == 0) {
		write(s, "", 1);
		lport = 0;
	} else {
		char num[8];
		int s2 = rresvport(&lport), s3;
		int len = sizeof (from);

		if (s2 < 0)
			goto bad;
		listen(s2, 1);
		(void) sprintf(num, "%d", lport);
		if (write(s, num, strlen(num)+1) != strlen(num)+1) {
                	catd = catopen(MF_LIBC,NL_CAT_LOCALE);
			perror(catgets(catd,LIBCNET,NET26,
					"write: setting up stderr"));
			(void)catclose(catd);
			(void) close(s2);
			goto bad;
		}
		s3 = accept(s2, &from, &len);
		(void) close(s2);
		if (s3 < 0) {
			perror("accept");
			lport = 0;
			goto bad;
		}
		*fd2p = s3;
		from.sin_port = ntohs((u_short)from.sin_port);
		if (from.sin_family != AF_INET ||
		    from.sin_port >= IPPORT_RESERVED) {
                	catd = catopen(MF_LIBC,NL_CAT_LOCALE);
			fprintf(stderr, catgets(catd,LIBCNET,NET27,
			    "socket: protocol failure in circuit setup.\n"));
			(void)catclose(catd);
			goto bad2;
		}
	}
	(void) write(s, locuser, strlen(locuser)+1);
	(void) write(s, remuser, strlen(remuser)+1);
	(void) write(s, cmd, strlen(cmd)+1);
	if (read(s, &c, 1) != 1) {
		perror(*ahost);
		goto bad2;
	}
	if (c != 0) {
		while (read(s, &c, 1) == 1) {
			(void) write(2, &c, 1);
			if (c == '\n')
				break;
		}
		goto bad2;
	}
#ifndef _THREAD_SAFE
	sigsetmask(oldmask);
#endif /* _THREAD_SAFE */
	return (s);
bad2:
	if (lport)
		(void) close(*fd2p);
bad:
	(void) close(s);
#ifndef _THREAD_SAFE
	sigsetmask(oldmask);
#endif /* _THREAD_SAFE */
	return (-1);
}

rresvport(alport)
	int *alport;
{
	struct sockaddr_in sin;
	int s;

	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	s = socket(AF_INET, SOCK_STREAM, 0);
	if (s < 0)
		return (-1);
	for (;;) {
		sin.sin_port = htons((u_short)*alport);
		if (bind(s, (caddr_t)&sin, sizeof (sin)) >= 0)
			return (s);
		if (errno != EADDRINUSE) {
			(void) close(s);
			return (-1);
		}
		(*alport)--;
		if (*alport == IPPORT_RESERVED/2) {
			(void) close(s);
			errno = EAGAIN;		/* close */
			return (-1);
		}
	}
}

int _check_rhosts_file = 1;

#ifdef _THREAD_SAFE 
struct innetgr_data innet_data;
#define INNETGR(w, x, y, z)	innetgr_r(w, x, y, z, &innet_data)
#else
#define INNETGR(w, x, y, z)	innetgr(w, x, y, z)
#endif /* _THREAD_SAFE */


ruserok(rhost, superuser, ruser, luser)
	char *rhost;
	int superuser;
	char *ruser, *luser;
{
	FILE *hostf;
	int first = 1, i;
        uid_t effuid = -1;
	register char *cp;
	int baselen = -1;
	nl_catd catd;


	DPRINTF (("ruserok(%s, %d, %s, %s)", rhost, superuser, ruser, luser));

	/* mark position of domain name, if any */
	for (cp = rhost; *cp; ++cp) {
		if (*cp == '.') {
			baselen = cp - rhost;
			break;
		}
	}
	hostf = superuser ? (FILE *)0 : fopen(_PATH_HEQUIV, "r");
again:
	if (hostf) {

		if (i = _validuser(hostf, rhost, luser, ruser, baselen)) {
			(void) fclose(hostf);
			if (effuid != -1) 
				(void) setuidx(ID_REAL|ID_EFFECTIVE,effuid);
			return(i == 1 ? 0 : i);
		}
		(void) fclose(hostf);
	}
	if (first == 1 && (_check_rhosts_file || superuser)) {
		struct stat sbuf;
		struct passwd *pwd;
		char pbuf[MAXPATHLEN];
#ifdef _THREAD_SAFE
#define	PWD_LEN	1024
		char pwd_entry[PWD_LEN];
		struct passwd paswd;
		pwd = &paswd;
		
		if (getpwnam_r(luser, pwd, pwd_entry, PWD_LEN) < 0)
#else
		if ((pwd = getpwnam(luser)) == NULL)
#endif /* _THREAD_SAFE	*/
			return(-1);

		first = 0;
		(void)strcpy(pbuf, pwd->pw_dir);

/* Read .rhosts as the local user since root may not be able to read it.  
   If the .rhosts is not on this machine, but is nfs mounted from elsewhere,
   we may not be able to read it as root since root is usually netnoone on the
   wire.  1/31/91. 
*/ 		effuid = getuidx(ID_EFFECTIVE);
		(void) setuidx(ID_REAL|ID_EFFECTIVE,pwd->pw_uid);

		(void) strcat(pbuf, "/.rhosts");
		if ((hostf = fopen(pbuf, "r")) == NULL) {
			(void) setuidx(ID_REAL|ID_EFFECTIVE,effuid);
			return(-1);
		}
		if (fstat(fileno(hostf), &sbuf) ||
		    sbuf.st_uid && sbuf.st_uid != pwd->pw_uid ||
		    sbuf.st_mode&022) {
			fclose(hostf);
			(void) setuidx(ID_REAL|ID_EFFECTIVE,effuid);
			return(-1);
		}
		goto again;
	}
	(void) setuidx(ID_REAL|ID_EFFECTIVE,effuid);
	return (-1);
}

void truncate_domain(char *hostname, char *newhost);

/* don't make static, used by lpd(8) */
/* returns -1 if ruser@rhost is denied access, 1 if granted, 0 if not found */
_validuser(hostf, rhost, luser, ruser, baselen)
FILE *hostf;
char *rhost, *luser, *ruser;
int baselen;
{
	char *user;
	char ahost[MAXHOSTNAMELEN * 2];
	register char *p;
	int hostmatch, usermatch;
	char domain[MAXDNAME], *dp;
        char hostname_nodom[MAXHOSTNAMELEN + 1];

	DPRINTF (("_validuser(%d, %s, %s, %s, %d)",
	    hostf, rhost, luser, ruser, baselen));

	dp = NULL;
	TS_LOCK(&_ypresolv_rmutex);
	if (getdomainname(domain, sizeof(domain)) == 0)
		dp = domain;
	TS_UNLOCK(&_ypresolv_rmutex);

        truncate_domain(rhost, hostname_nodom);

	while (fgets(ahost, (int)sizeof(ahost), hostf)) {
		hostmatch = usermatch = 0;
		p = ahost;
		if (*p == '#' || *p == '\n')  /* ignore comments and blanks */
			continue;
		while (*p != '\n' && *p != ' ' && *p != '\t' && *p != '\0')
			p++;
		if (*p == ' ' || *p == '\t') {
			*p++ = '\0';
			while (*p == ' ' || *p == '\t')
				p++;
			user = p;
			while (*p != '\n' && *p != ' ' && *p != '\t' &&
			    *p != '\0')
				p++;
		} else
			user = p;
		*p = '\0';

		DPRINTF (("user = %s, ruser = %s, luser = %s",
		    user, ruser, luser));

		/*
		 *  + - anything goes
		 *  +@<name> - group <name> allowed
		 *  -@<name> - group <name> disallowed
		 *  -<name> - host <name> disallowed
		 */
		if (ahost[0] == '+' && ahost[1] == 0)
			hostmatch = 1;
                /* This code was changed to allow the use of      */
                /* unqualified hostnames in the netgroups.        */
                else if (ahost[0] == '+' && ahost[1] == '@') {
                        /* Check for fully qualified hostname */
			hostmatch = INNETGR(ahost + 2, rhost, NULL, dp);
                        /* Check for hostname without domain */
                        if (!hostmatch && hostname_nodom[0])
                                hostmatch = INNETGR(ahost + 2, hostname_nodom, NULL,dp);
		}
		else if (ahost[0] == '-' && ahost[1] == '@') {
                        /* Check for fully qualified hostname */
			if (INNETGR(ahost + 2, rhost, NULL, dp))
				return(-1);
                        /* Check for hostname without domain */
                        else if (hostname_nodom[0]) {
                                if (INNETGR(ahost + 2, hostname_nodom, NULL,dp))
                                        return(-1);
                        }
		}
		else if (ahost[0] == '-') {
			if (_checkhost(rhost, ahost+1, baselen))
				return(-1);
		}
		else
			hostmatch = _checkhost(rhost, ahost, baselen);

		if (user[0]) {
			if (user[0] == '+' && user[1] == 0)
				usermatch = 1;
			else if (user[0] == '+' && user[1] == '@')
				usermatch = INNETGR(user+2, NULL, ruser, dp);
			else if (user[0] == '-' && user[1] == '@') {
				if (hostmatch && INNETGR(user+2, NULL,
				    ruser, dp))
					return(-1);
			}
			else if (user[0] == '-') {
				if (hostmatch && !strcmp(user+1, ruser))
					return(-1);
			}
			else
				usermatch = !strcmp(user, ruser);
		}
		else
			usermatch = !strcmp(ruser, luser);
		if (hostmatch && usermatch)
			return(1);
	}
	return (0);
}

_checkhost(rhost, lhost, len)
char *rhost, *lhost;
int len;
{
	struct hostent *hp;
	static char ldomain[MAXHOSTNAMELEN + 1];
	static char *domainp = NULL;
	static int nodomain = 0;
	register char *cp;
	long addr;
	int ret_val;

#ifdef _THREAD_SAFE
	int rc;
	struct hostent host;
	struct hostent_data host_data;
	hp = &host;
	bzero(&host_data, sizeof(host_data));
#define GETHOSTBYADDR(a, s, t, h)	((rc=gethostbyaddr_r(a, s, t, h, &host_data)) != TS_FAILURE)
#else
#define GETHOSTBYADDR(a, s, t, h)	(h = gethostbyaddr(a, s, t))
#endif /* _THREAD_SAFE */
	/*
	 * check for ip address and do a lookup to convert to hostname
	 */
	if (isinet_addr(lhost) && (addr = inet_addr(lhost)) != -1 &&
	    GETHOSTBYADDR(&addr, sizeof(addr), AF_INET, hp))
		lhost = hp->h_name;

	/*
	 * note that this is optimized for speed, since it is iterated upon
	 * many times; we therefore try to take care of the most common cases
	 * first.
	 */
	if (len == -1) {  /* see if hostname from file has a domain name */
		for (cp = lhost; *cp; ++cp) {
			if (*cp == '.') {
				len = cp - lhost;
				break;
			}
		}
		if (len == -1)
			return(!strcasecmp(rhost, lhost));
	}
	if (strncasecmp(rhost, lhost, len))
		return(0);
	if (!strcasecmp(rhost, lhost))
		return(1);
	if (*(lhost + len) != '\0' && *(rhost + len) != '\0')
		return(0);

	TS_LOCK(&_domain_rmutex);
	if (nodomain) {
		TS_UNLOCK(&_domain_rmutex);
		return(0);
	}
	if (!domainp) {
		if (gethostname(ldomain, sizeof(ldomain)) == -1) {
			nodomain = 1;
			TS_UNLOCK(&_domain_rmutex);
			return(0);
		}
		ldomain[MAXHOSTNAMELEN] = '\0';
		if ((domainp = index(ldomain, '.')) == (char *)NULL) {
			nodomain = 1;
			TS_UNLOCK(&_domain_rmutex);
			return(0);
		}
	}
	ret_val = strcasecmp(domainp, *(rhost + len)?rhost+len:lhost+len);
	TS_UNLOCK(&_domain_rmutex);
	return(!ret_val);
}

#include <resolv.h>

/*
 *      take 'hostname', chop off the tcp/ip domain if it matches
 *      the local machine's, then copy it into 'newhost'
 */
void
truncate_domain(char *hostname, char *newhost)
{
        char *dom;
        static char mydomain[MAXDNAME + 1];
        static char myhostname[MAXHOSTNAMELEN + 1];


        /*
         * get the domain name unless we already have it
         */
        if (!mydomain[0]) {
                /*
                 * get the domainname from /etc/resolv.conf
                 */
                if (!(_res.options & RES_INIT))
                        res_init();

                /*
                 * if we found a domain, use it
                 */
                if (_res.defdname[0])
                        strcpy(mydomain, _res.defdname);
                else {
                        /*
                         * else try our hostname for one
                         */
                        if (!myhostname[0])
                                gethostname(myhostname, sizeof(myhostname));
                        if ((dom = strchr(myhostname, '.')) != NULL)
                                strcpy(mydomain, ++dom);
                        }
                }

        /*
         * does hostname have a domain that matches ours?
         */
        if ((dom = strchr(hostname, '.')) != NULL &&
             mydomain[0] && strcmp(dom+1, mydomain) == 0)
                (void) sprintf(newhost, "%.*s", dom - hostname,hostname);
        else
                *newhost = '\0';
}

