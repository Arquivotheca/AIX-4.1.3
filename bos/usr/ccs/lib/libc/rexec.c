static char sccsid[] = "@(#)65  1.17  src/bos/usr/ccs/lib/libc/rexec.c, libcnet, bos411, 9432A411a 8/5/94 09:44:22";
/* 
 * COMPONENT_NAME: LIBCNET rexec.c
 * 
 * FUNCTIONS: CONNECTION_WRITE, USE_EFFECTIVE_UID, USE_REAL_UID, 
 *            make_in_addr, rexec, tcpip_auditlog 
 *
 * ORIGINS: 26  27  71
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 */

#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>

#include <stdio.h>
#include <netdb.h>
#include <errno.h>
#include <nl_types.h>
#include <arpa/nameser.h>
#include "libc_msg.h"
#include "ts_supp.h"
#include "push_pop.h"

#ifdef _THREAD_SAFE
#include "rec_mutex.h"
extern struct rec_mutex _netrc_rmutex;
#endif /* _THREAD_SAFE */

#ifdef _CSECURITY
#include <arpa/inet.h>

#define TCPIP_MAX_TAIL_FIELDS		8
#define	CONNECTION			0000010
#define	EVENT_CONNECTION		"connection"
#define CONNECTION_WRITE(INETADDR, PORT, OPERATION, RESULT, ERRNO) \
	audit_tail[1] = inet_ntoa(INETADDR); \
	audit_tail[2] = PORT; \
	audit_tail[3] = OPERATION; \
	audit_tail[4] = RESULT; \
	tcpip_auditlog(CONNECTION, audit_tail, ERRNO);
#endif /* _CSECURITY */

char	*index();
int	rexecoptions;
char	*getpass(), *getlogin();

#ifdef _THREAD_SAFE
rexec_r(char **ahost, int rport, char *name, char *pass, char *cmd, 
	int *fd2p, struct hostent_data *host_data)
#else
rexec(char **ahost, int rport, char *name, char *pass, char *cmd, int *fd2p)
#endif /* _THREAD_SAFE */
{
	int s, timo = 1, s3;
	struct sockaddr_in sin, from;
	char c;
	int port;
	struct hostent *hp;
#ifdef _CSECURITY
	char *audit_tail[TCPIP_MAX_TAIL_FIELDS];
#endif /* _CSECURITY */
	nl_catd catd;
	long addr;

#ifdef _THREAD_SAFE
	int tmp_res;
	struct hostent	host;
	hp = &host;
#endif /* _THREAD_SAFE */

	if (isinet_addr(*ahost) && (addr = inet_addr(*ahost)) != -1) {
#ifdef _THREAD_SAFE
		bzero(&host_data, sizeof(host_data));
		tmp_res = gethostbyaddr_r(&addr, sizeof(addr), AF_INET, hp, host_data);
	} else {
		bzero(&host_data, sizeof(host_data));
		tmp_res = gethostbyname_r(*ahost, hp, host_data);
	}
	if (tmp_res < 0) { 
#else
		hp = gethostbyaddr(&addr, sizeof(addr), AF_INET);
	} else {
		hp = gethostbyname(*ahost);
	}
	if (hp == 0) {
#endif /* _THREAD_SAFE */
                catd = catopen(MF_LIBC,NL_CAT_LOCALE);
		fprintf(stderr, catgets(catd,LIBCNET,NET22,
					"%s: unknown host\n"), *ahost);
		return (-1);
	}
	sin.sin_family = hp->h_addrtype;
	sin.sin_port = rport;
	bcopy(hp->h_addr, (caddr_t)&sin.sin_addr, hp->h_length);

	/*
	 * check for host specified by user, as well as the result
	 * of the hostname/address lookup above.
	 */
	check_netrc(*ahost, &name, &pass);
	if (!name && !pass)
		check_netrc(hp->h_name, &name, &pass);
	if (!name || !pass) {  /* have ruserpass() prompt for them */
		extern int netrc_restricted;
		int i;
		TS_LOCK(&_netrc_rmutex);
		i = netrc_restricted;
		netrc_restricted = 1;  /* don't check .netrc again */

		TS_PUSH_CLNUP(&_netrc_rmutex);
		ruserpass(hp->h_name, &name, &pass);
		TS_POP_CLNUP(0);

		netrc_restricted = i;
		TS_UNLOCK(&_netrc_rmutex);
	}
	*ahost = hp->h_name;

retry:
	s = socket(AF_INET, SOCK_STREAM, 0);
	if (s < 0) {
		perror("rexec: socket");
		return (-1);
	}
	if (connect(s, &sin, sizeof(sin)) < 0) {
		if (errno == ECONNREFUSED && timo <= 16) {
			(void) close(s);
			sleep(timo);
			timo *= 2;
			goto retry;
		}
		perror(hp->h_name);
		return (-1);
	}
#ifdef _CSECURITY
		CONNECTION_WRITE(sin.sin_addr.s_addr, "exec/tcp", "open", "",0);
#endif /* _CSECURITY */
	if (fd2p == 0) {
		(void) write(s, "", 1);
		port = 0;
	} else {
		char num[8];
		int s2;
		
		port = IPPORT_USERRESERVED;
		s2 = rresvport(&port);
		if (s2 < 0) {
			/* fake errno if rresvport went below IPPORT_RESERVED */
			if (errno == EACCES)
				errno = EAGAIN;
			perror("rexec: bind");
			(void) close(s);
			return (-1);
		}
		listen(s2, 1);
		(void) sprintf(num, "%d", port);
		(void) write(s, num, strlen(num)+1);
		{ int len = sizeof (from);
		  s3 = accept(s2, &from, &len);
		  close(s2);
		  if (s3 < 0) {
			perror("accept");
			port = 0;
			goto bad;
		  }
#ifdef _CSECURITY
		  CONNECTION_WRITE(from.sin_addr.s_addr, "exec/tcp",
			"open", "", 0);
#endif /* _CSECURITY */
		}
		*fd2p = s3;
	}
	(void) write(s, name, strlen(name) + 1);
	/* should public key encypt the password here */
	(void) write(s, pass, strlen(pass) + 1);
	(void) write(s, cmd, strlen(cmd) + 1);
	if (read(s, &c, 1) != 1) {
		perror(*ahost);
		goto bad;
	}
	if (c != 0) {
		while (read(s, &c, 1) == 1) {
			(void) write(2, &c, 1);
			if (c == '\n')
				break;
		}
		goto bad;
	}
	return (s);
bad:
	if (port)
		(void) close(*fd2p);
	(void) close(s);
	return (-1);
}

#ifdef _CSECURITY 		/* !!! This code pulled from TCPIP LPP !!! */
#include <sys/signal.h>
#include <sys/syslog.h>

#include <sys/audit.h>
#include <arpa/inet.h>

#define	PROTOCOL 	"TCP/IP"		/* TCP/IP, SNA, UUCP, etc..*/
/*
 * TCP/IP application auditlog cases.
*/
#define	CHANGE_HOST_ID			0000001
#define	EVENT_CHANGE_HOST_ID		"change_host_id"

#define	CHANGE_CONFIGURATION		0000002
#define	EVENT_CHANGE_CONFIGURATION	"change_config"

#define	CHANGE_MANUALLY_SET_ROUTE	0000004
#define	EVENT_CHANGE_MANUALLY_SET_ROUTE	"change_route"
/*
#define	CONNECTION			0000010
#define	EVENT_CONNECTION		"connection"
*/

#define	IMPORT_DATA			0000020
#define	EVENT_IMPORT_DATA		"import_data"

#define	EXPORT_DATA			0000040
#define	EVENT_EXPORT_DATA		"export_data"

#define	NET_ACCESS			0000100
#define	EVENT_NET_ACCESS		"net_access"

#define	SET_NETWORK_TIME		0000200
#define	EVENT_SET_NETWORK_TIME		"set_network_tim"

#define	MAIL_CONFIGURATION		0000400
#define	EVENT_MAIL_CONFIGURATION	"sendmail_config"

#define	MAIL_TO_A_FILE			0001000
#define	EVENT_MAIL_TO_A_FILE		"sendmail_to_fil"


/* globals and externs */



/* Macros */

#define	USE_EFFECTIVE_UID(euid) \
	if ((seteuid(euid)) == -1) syslog(LOG_ALERT, "setuid: %m");

#define	USE_REAL_UID(uid) \
	if ((seteuid(uid)) == -1) syslog(LOG_ALERT, "setuid: %m");


int Real_Uid, Effective_Uid;

tcpip_auditlog(audit_type, audit_tail, audit_result)
int audit_type;
char *audit_tail[];
int audit_result;
{
	int signal_mask;
	static int first_trip = 1;
	static int audit_status;

	if (first_trip) {
		Real_Uid = getuid();
		Effective_Uid = geteuid();

		audit_tail[0] = PROTOCOL;

		signal_mask = sigblock(0xffff);
		USE_EFFECTIVE_UID(Effective_Uid);
		audit_status = auditproc((char *)0, AUDIT_QSTATUS,(char *)0, 0);
		USE_REAL_UID(Real_Uid);
		(void) sigsetmask(signal_mask);

		first_trip--;
	}

	if (audit_status) {
		signal_mask = sigblock(0xffff);
		USE_EFFECTIVE_UID(Effective_Uid);
		switch (audit_type) {
			case CHANGE_HOST_ID: {
				auditwrite(EVENT_CHANGE_HOST_ID, audit_result,
					audit_tail[0], strlen(audit_tail[0])+1,
					audit_tail[1], strlen(audit_tail[1])+1,
					audit_tail[2], strlen(audit_tail[2])+1,
					audit_tail[3], strlen(audit_tail[3])+1,
					(char *) 0);
				break;
			}
			case CHANGE_CONFIGURATION: {
				auditwrite(EVENT_CHANGE_CONFIGURATION, audit_result,
					audit_tail[0], strlen(audit_tail[0])+1,
					audit_tail[1], strlen(audit_tail[1])+1,
					audit_tail[2], strlen(audit_tail[2])+1,
					audit_tail[3], strlen(audit_tail[3])+1,
					audit_tail[4], strlen(audit_tail[4])+1,
					(char *) 0);
				break;
			}
			case CHANGE_MANUALLY_SET_ROUTE: {
				auditwrite(EVENT_CHANGE_MANUALLY_SET_ROUTE, audit_result,
					audit_tail[0], strlen(audit_tail[0])+1,
					audit_tail[1], strlen(audit_tail[1])+1,
					audit_tail[2], strlen(audit_tail[2])+1,
					audit_tail[3], strlen(audit_tail[3])+1,
					audit_tail[4], strlen(audit_tail[4])+1,
					(char *) 0);
				break;
			}
			case CONNECTION: {
				auditwrite(EVENT_CONNECTION, audit_result,
					audit_tail[0], strlen(audit_tail[0])+1,
					audit_tail[1], strlen(audit_tail[1])+1,
					audit_tail[2], strlen(audit_tail[2])+1,
					audit_tail[3], strlen(audit_tail[3])+1,
					audit_tail[4], strlen(audit_tail[4])+1,
					(char *) 0);
				break;
			}
			case IMPORT_DATA: {
				auditwrite(EVENT_IMPORT_DATA, audit_result,
					audit_tail[0], strlen(audit_tail[0])+1,
					audit_tail[1], strlen(audit_tail[1])+1,
					audit_tail[2], strlen(audit_tail[2])+1,
					audit_tail[3], strlen(audit_tail[3])+1,
					audit_tail[4], strlen(audit_tail[4])+1,
					(char *) 0);
				break;
			}
			case EXPORT_DATA: {
				auditwrite(EVENT_EXPORT_DATA, audit_result,
					audit_tail[0], strlen(audit_tail[0])+1,
					audit_tail[1], strlen(audit_tail[1])+1,
					audit_tail[2], strlen(audit_tail[2])+1,
					audit_tail[3], strlen(audit_tail[3])+1,
					audit_tail[4], strlen(audit_tail[4])+1,
					(char *) 0);
				break;
			}
			case NET_ACCESS: {
				auditwrite(EVENT_NET_ACCESS, audit_result,
					audit_tail[0], strlen(audit_tail[0])+1,
					audit_tail[1], strlen(audit_tail[1])+1,
					audit_tail[2], strlen(audit_tail[2])+1,
					audit_tail[3], strlen(audit_tail[3])+1,
					audit_tail[4], strlen(audit_tail[4])+1,
					(char *) 0);
				break;
			}
			case SET_NETWORK_TIME: {
				auditwrite(EVENT_SET_NETWORK_TIME, audit_result,
					audit_tail[0], strlen(audit_tail[0])+1,
					audit_tail[1], strlen(audit_tail[1])+1,
					audit_tail[2], strlen(audit_tail[2])+1,
					audit_tail[3], strlen(audit_tail[3])+1,
					(char *) 0);
				break;
			}
			case MAIL_CONFIGURATION: {
				auditwrite(EVENT_MAIL_CONFIGURATION, audit_result,
					audit_tail[0], strlen(audit_tail[0])+1,
					audit_tail[1], strlen(audit_tail[1])+1,
					(char *) 0);
				break;
			}
			case MAIL_TO_A_FILE: {
				auditwrite(EVENT_MAIL_TO_A_FILE, audit_result,
					audit_tail[0], strlen(audit_tail[0])+1,
					audit_tail[1], strlen(audit_tail[1])+1,
					audit_tail[2], strlen(audit_tail[2])+1,
					(char *) 0);
				break;
			}
			default: {
				syslog(LOG_ALERT, "Unknown audit event!!!");
				exit(1);
			}
			
		}
		USE_REAL_UID(Real_Uid);
		(void) sigsetmask(signal_mask);
	}
}

unsigned long
make_in_addr(p)
char *p;
{
	unsigned long inetaddr;
	bcopy(p, &inetaddr, sizeof(inetaddr));
	return(inetaddr);
}

#endif /* _CSECURITY */ 
