static char sccsid[] = "@(#)10	1.1  src/bos/usr/ccs/lib/libc/clnt_perror.c, libcrpc, bos411, 9428A410j 10/25/93 20:52:28";
/*
 *   COMPONENT_NAME: LIBCRPC
 *
 *   FUNCTIONS: _buf
 *		auth_errmsg
 *		clnt_pcreateerror
 *		clnt_perrno
 *		clnt_perror
 *		clnt_spcreateerror
 *		clnt_sperrno
 *		clnt_sperror
 *
 *   ORIGINS: 24,27
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1988 by Sun Microsystems, Inc.
 *  1.16 88/02/08
 */

#ifdef _KERNEL
#include <rpc/types.h>
#include <rpc/auth.h>
#include <rpc/clnt.h>
#endif

#ifndef _KERNEL
/*
 * clnt_perror.c
 */
#include <stdio.h>
#include <rpc/types.h>
#include <rpc/auth.h>
#include <rpc/clnt.h>
#include <libc_msg.h>

extern char *sys_errlist[];
static char *auth_errmsg();

extern char *strcpy();

static char *buf;

static char *
_buf()
{

	if (buf == 0)
		buf = (char *)malloc(256);
	return (buf);
}

/*
 * Print reply error info
 */
char *
clnt_sperror(rpch, s)
	CLIENT *rpch;
	char *s;
{
	struct rpc_err e;
	void clnt_perrno();
	char *err;
	char *str = _buf();
 char *strstart = str;

	if (str == NULL)
		return (NULL);
	CLNT_GETERR(rpch, &e);

	(void) sprintf(str, "%s: ", s);  
	str += strlen(str);

	(void) strcpy(str, clnt_sperrno(e.re_status));  
	str += strlen(str);

	switch (e.re_status) {
	case RPC_SUCCESS:
	case RPC_CANTENCODEARGS:
	case RPC_CANTDECODERES:
	case RPC_TIMEDOUT:     
	case RPC_PROGUNAVAIL:
	case RPC_PROCUNAVAIL:
	case RPC_CANTDECODEARGS:
	case RPC_SYSTEMERROR:
	case RPC_UNKNOWNHOST:
	case RPC_UNKNOWNPROTO:
	case RPC_PMAPFAILURE:
	case RPC_PROGNOTREGISTERED:
	case RPC_FAILED:
		break;

	case RPC_CANTSEND:
	case RPC_CANTRECV:
		(void) sprintf(str, "; errno = %s",(char *)oncmsg(MS_LIBC,
				e.re_errno, sys_errlist[e.re_errno])); 
		str += strlen(str);
		break;

	case RPC_VERSMISMATCH:
		(void) sprintf(str, (char *)oncmsg(LIBCRPC,RPC70,
			"; low version = %lu, high version = %lu"),
			e.re_vers.low, e.re_vers.high);
		str += strlen(str);
		break;

	case RPC_AUTHERROR:
		err = auth_errmsg(e.re_why);
		(void) sprintf(str,"; why = ");
		str += strlen(str);
		if (err != NULL) {
			(void) sprintf(str, "%s",err);
		} else {
			(void) sprintf(str,(char *)oncmsg(LIBCRPC,RPC69,
				"(unknown authentication error - %d)"),
				(int) e.re_why);
		}
		str += strlen(str);
		break;

	case RPC_PROGVERSMISMATCH:
		(void) sprintf(str,(char *)oncmsg(LIBCRPC,RPC70, 
			"; low version = %lu, high version = %lu"), 
			e.re_vers.low, e.re_vers.high);
		str += strlen(str);
		break;

	default:	/* unknown */
		(void) sprintf(str, 
			"; s1 = %lu, s2 = %lu", 
			e.re_lb.s1, e.re_lb.s2);
		str += strlen(str);
		break;
	}
	(void) sprintf(str, "\n");
	return(strstart) ;
}

void
clnt_perror(rpch, s)
	CLIENT *rpch;
	char *s;
{
	(void) fprintf(stderr,"%s",clnt_sperror(rpch,s));
}

struct rpc_errtab {
	enum clnt_stat status;
	long	set_no;
	long	msg_no;
	char *message;
};

static struct rpc_errtab  rpc_errlist[] = {
	{ RPC_SUCCESS, LIBCRPC, RPC3,
		"RPC: Success" }, 
	{ RPC_CANTENCODEARGS, LIBCRPC, RPC4, 
		"RPC: Can't encode arguments" },
	{ RPC_CANTDECODERES, LIBCRPC, RPC5, 
		"RPC: Can't decode result" },
	{ RPC_CANTSEND, LIBCRPC, RPC6,
		"RPC: Unable to send" },
	{ RPC_CANTRECV, LIBCRPC, RPC7,
		"RPC: Unable to receive" },
	{ RPC_TIMEDOUT, LIBCRPC, RPC8,
		"RPC: Timed out" },
	{ RPC_VERSMISMATCH, LIBCRPC, RPC9,
		"RPC: Incompatible versions of RPC" },
	{ RPC_AUTHERROR, LIBCRPC, RPC10,
		"RPC: Authentication error" },
	{ RPC_PROGUNAVAIL, LIBCRPC, RPC11,
		"RPC: Program unavailable" },
	{ RPC_PROGVERSMISMATCH, LIBCRPC, RPC12,
		"RPC: Program/version mismatch" },
	{ RPC_PROCUNAVAIL, LIBCRPC, RPC13,
		"RPC: Procedure unavailable" },
	{ RPC_CANTDECODEARGS, LIBCRPC, RPC14,
		"RPC: Server can't decode arguments" },
	{ RPC_SYSTEMERROR, LIBCRPC, RPC15,
		"RPC: Remote system error" },
	{ RPC_UNKNOWNHOST, LIBCRPC, RPC16,
		"RPC: Unknown host" },
	{ RPC_UNKNOWNPROTO, LIBCRPC, RPC17,
		"RPC: Unknown protocol" },
	{ RPC_PMAPFAILURE, LIBCRPC, RPC18,
		"RPC: Port mapper failure" },
	{ RPC_PROGNOTREGISTERED, LIBCRPC, RPC19,
		"RPC: Program not registered"},
	{ RPC_FAILED, LIBCRPC, RPC20,
		"RPC: Failed (unspecified error)"}
};


/*
 * This interface for use by clntrpc
 */
char *
clnt_sperrno(stat)
	enum clnt_stat stat;
{
	int i;

	for (i = 0; i < sizeof(rpc_errlist)/sizeof(struct rpc_errtab); i++) {
		if (rpc_errlist[i].status == stat) {
			return ((char *)oncmsg(rpc_errlist[i].set_no,
					rpc_errlist[i].msg_no,
					rpc_errlist[i].message));
		}
	}
	return ((char *)oncmsg(LIBCRPC,RPC71,"RPC: (unknown error code)"));
}

void
clnt_perrno(num)
	enum clnt_stat num;
{
	(void) fprintf(stderr,"%s",clnt_sperrno(num));
}


char *
clnt_spcreateerror(s)
	char *s;
{
	extern int sys_nerr;
	extern char *sys_errlist[];
	char *str = _buf();

	if (str == NULL)
		return(NULL);
	(void) sprintf(str, "%s: ", s);
	(void) strcat(str, clnt_sperrno(rpc_createerr.cf_stat));
	switch (rpc_createerr.cf_stat) {
	case RPC_PMAPFAILURE:
		(void) strcat(str, " - ");
		(void) strcat(str,
		    clnt_sperrno(rpc_createerr.cf_error.re_status));
		break;

	case RPC_SYSTEMERROR:
		(void) strcat(str, " - ");
		if (rpc_createerr.cf_error.re_errno > 0
		    && rpc_createerr.cf_error.re_errno < sys_nerr) {
			(void) strcat(str,(char *)oncmsg(MS_LIBC, 
			    rpc_createerr.cf_error.re_errno,
			    sys_errlist[rpc_createerr.cf_error.re_errno]));
		}
		else
			(void) sprintf(&str[strlen(str)],(char *)oncmsg(LIBCRPC,RPC72,
				"Error %d"), rpc_createerr.cf_error.re_errno);
		break;
	}
	(void) strcat(str, "\n");
	return (str);
}

void
clnt_pcreateerror(s)
	char *s;
{
	(void) fprintf(stderr,"%s",clnt_spcreateerror(s));
}

struct auth_errtab {
	enum auth_stat status;	
	long	set_no;
	long	msg_no;
	char *message;
};

static struct auth_errtab auth_errlist[] = {
	{ AUTH_OK, LIBCRPC, RPC21,
		"Authentication OK" },
	{ AUTH_BADCRED, LIBCRPC, RPC22,
		"Invalid client credential" },
	{ AUTH_REJECTEDCRED, LIBCRPC, RPC23,
		"Server rejected credential" },
	{ AUTH_BADVERF, LIBCRPC, RPC24,
		"Invalid client verifier" },
	{ AUTH_REJECTEDVERF, LIBCRPC, RPC25,
		"Server rejected verifier" },
	{ AUTH_TOOWEAK, LIBCRPC, RPC26,
		"Client credential too weak" },
	{ AUTH_INVALIDRESP, LIBCRPC, RPC27,
		"Invalid server verifier" },
	{ AUTH_FAILED, LIBCRPC, RPC28,
		"Failed (unspecified error)" },
};

static char *
auth_errmsg(stat)
	enum auth_stat stat;
{
	int i;

	for (i = 0; i < sizeof(auth_errlist)/sizeof(struct auth_errtab); i++) {
		if (auth_errlist[i].status == stat) {
			return((char *)oncmsg(auth_errlist[i].set_no,
				auth_errlist[i].msg_no,
				auth_errlist[i].message));
		}
	}
	return(NULL);
}
#endif /* !_KERNEL */

#ifdef _KERNEL
struct rpc_errtab {
	enum clnt_stat status;
	char *message;
};

static struct rpc_errtab  rpc_errlist[] = {
	{ RPC_SUCCESS, 
		"RPC: Success" }, 
	{ RPC_CANTENCODEARGS, 
		"RPC: Can't encode arguments" },
	{ RPC_CANTDECODERES, 
		"RPC: Can't decode result" },
	{ RPC_CANTSEND, 
		"RPC: Unable to send" },
	{ RPC_CANTRECV, 
		"RPC: Unable to receive" },
	{ RPC_TIMEDOUT, 
		"RPC: Timed out" },
	{ RPC_VERSMISMATCH, 
		"RPC: Incompatible versions of RPC" },
	{ RPC_AUTHERROR, 
		"RPC: Authentication error" },
	{ RPC_PROGUNAVAIL, 
		"RPC: Program unavailable" },
	{ RPC_PROGVERSMISMATCH, 
		"RPC: Program/version mismatch" },
	{ RPC_PROCUNAVAIL, 
		"RPC: Procedure unavailable" },
	{ RPC_CANTDECODEARGS, 
		"RPC: Server can't decode arguments" },
	{ RPC_SYSTEMERROR, 
		"RPC: Remote system error" },
	{ RPC_UNKNOWNHOST, 
		"RPC: Unknown host" },
	{ RPC_UNKNOWNPROTO,
		"RPC: Unknown protocol" },
	{ RPC_PMAPFAILURE, 
		"RPC: Port mapper failure" },
	{ RPC_PROGNOTREGISTERED, 
		"RPC: Program not registered"},
	{ RPC_FAILED, 
		"RPC: Failed (unspecified error)"}
};


/*
 * This interface for use by callrpc() and clnt_broadcast()
 */
char *
clnt_sperrno(stat)
	enum clnt_stat stat;
{
	int i;

	for (i = 0; i < sizeof(rpc_errlist)/sizeof(struct rpc_errtab); i++) {
		if (rpc_errlist[i].status == stat) {
			return (rpc_errlist[i].message);
		}
	}
	return ("RPC: (unknown error code)");
}
#endif /* _KERNEL */
