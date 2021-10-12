static char sccsid[] = "@(#)89  1.13  src/bos/usr/ccs/lib/libc/key_call.c, libcrpc, bos411, 9428A410j 2/26/94 15:28:36";
/*
 *   COMPONENT_NAME: LIBCRPC
 *
 *   FUNCTIONS: debug
 *		key_call
 *		key_decryptsession
 *		key_encryptsession
 *		key_gendes
 *		key_setsecret
 *		netname2user
 *		
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
 */


/* 
#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = 	"@(#)key_call.c	1.8 91/07/11 4.1NFSSRC Copyr 1990 Sun Micro";
#endif
 *
 * Copyright (c) 1988 by Sun Microsystems, Inc.
 * 1.11 88/02/08 
 */

/*
 * key_call.c, Interface to keyserver
 *
 * setsecretkey(key) - set your secret key
 * encryptsessionkey(agent, deskey) - encrypt a session key to talk to agent
 * decryptsessionkey(agent, deskey) - decrypt ditto
 * gendeskey(deskey) - generate a secure des key
 * netname2user(...) - get unix credential for given name (kernel only)
 */
#ifdef _KERNEL
#include <sys/param.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/user.h>
#include <sys/socket.h>
#include <rpc/rpc.h>
#include <rpc/key_prot.h>
#else
#include <sys/types.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <rpc/rpc.h>
#include <rpc/key_prot.h>
#endif

#define KEY_TIMEOUT	5	/* per-try timeout in seconds */
#define KEY_NRETRY	12	/* number of retries */

#define debug(msg)		/* turn off debugging */

static struct timeval trytimeout = { KEY_TIMEOUT, 0 };
#ifndef _KERNEL
static struct timeval tottimeout = { KEY_TIMEOUT * KEY_NRETRY, 0 };
#endif

static keycall();

#ifndef _KERNEL
key_setsecret(secretkey)
	char *secretkey;
{
	keystatus status;

	if (!key_call((u_long)KEY_SET, xdr_keybuf, secretkey, xdr_keystatus, 
		(char*)&status)) 
	{
		return (-1);
	}
	if (status != KEY_SUCCESS) {
		debug("set status is nonzero");
		return (-1);
	}
	return (0);
}
#endif


#ifdef _KERNEL
key_encryptsession(remotename, deskey, cred)
	char *remotename;
	des_block *deskey;
	struct ucred *cred;
#else
key_encryptsession(remotename, deskey)
	char *remotename;
	des_block *deskey;
#endif
{
	cryptkeyarg arg;
	cryptkeyres res;

	arg.remotename = remotename;
	arg.deskey = *deskey;
#ifdef _KERNEL
	if (!key_call((u_long)KEY_ENCRYPT, 
		xdr_cryptkeyarg, (char *)&arg, xdr_cryptkeyres, 
		      (char *)&res, cred))
#else
	if (!key_call((u_long)KEY_ENCRYPT, 
		xdr_cryptkeyarg, (char *)&arg, xdr_cryptkeyres, (char *)&res)) 
#endif
	{
		return (-1);
	}
	if (res.status != KEY_SUCCESS) {
		debug("encrypt status is nonzero");
		return (-1);
	}
	*deskey = res.cryptkeyres_u.deskey;
	return (0);
}


key_decryptsession(remotename, deskey)
	char *remotename;
	des_block *deskey;
{
	cryptkeyarg arg;
	cryptkeyres res;
#ifdef _KERNEL
	struct ucred *cred = crref();
#endif

	arg.remotename = remotename;
	arg.deskey = *deskey;

#ifdef _KERNEL	
	if (!key_call((u_long)KEY_DECRYPT, 
		xdr_cryptkeyarg, (char *)&arg, xdr_cryptkeyres, 
		      (char *)&res, cred))
	{
		crfree(cred);
		return (-1);
	}
	crfree(cred);
#else
	if (!key_call((u_long)KEY_DECRYPT, 
		xdr_cryptkeyarg, (char *)&arg, xdr_cryptkeyres, (char *)&res))  
#endif
	if (res.status != KEY_SUCCESS) {
		debug("decrypt status is nonzero");
		return (-1);
	}
	*deskey = res.cryptkeyres_u.deskey;
	return (0);
}

#ifdef _KERNEL
key_gendes(key, cred)
	des_block *key;
	struct ucred *cred;
{
	if (!key_call((u_long)KEY_GEN, xdr_void, (char *)NULL, xdr_des_block, 
		(char *)key, cred)) 
	{
		return (-1);
	}
#else
key_gendes(key)
	des_block *key;
{
	struct sockaddr_in sin;
	CLIENT *client;
	int socket;
	enum clnt_stat stat;

 
	sin.sin_family = AF_INET;
	sin.sin_port = 0;
	sin.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	bzero(sin.sin_zero, sizeof(sin.sin_zero));
	socket = RPC_ANYSOCK;
	client = clntudp_bufcreate(&sin, (u_long)KEY_PROG, (u_long)KEY_VERS,
		trytimeout, &socket, RPCSMALLMSGSIZE, RPCSMALLMSGSIZE);
	if (client == NULL) {
		return (-1);
	}
	stat = clnt_call(client, KEY_GEN, xdr_void, NULL,
		xdr_des_block, key, tottimeout);
	clnt_destroy(client);
	(void) close(socket);
	if (stat != RPC_SUCCESS) {
		return (-1);
	}
#endif
	return (0);
}
 

#ifdef _KERNEL
netname2user(name, uid, gid, len, groups)
	char *name;
	uid_t *uid;
	gid_t *gid;
	int *len;
	int *groups;
{
	struct getcredres res;
	struct ucred *cred = crref();

	res.getcredres_u.cred.gids.gids_val = groups;
	if (!key_call((u_long)KEY_GETCRED, xdr_netnamestr, (char *)&name, 
		xdr_getcredres, (char *)&res, cred)) 
	{
		debug("netname2user: timed out?");
		crfree(cred);
		return (0);
	}
	crfree(cred);
	if (res.status != KEY_SUCCESS) {
		return (0);
	}
	*uid = res.getcredres_u.cred.uid;
	*gid = res.getcredres_u.cred.gid;
	*len = res.getcredres_u.cred.gids.gids_len;
	return (1);
}
#endif

#ifdef _KERNEL
static
key_call(procn, xdr_args, args, xdr_rslt, rslt, cred)
	u_long procn;
	bool_t (*xdr_args)();	
	char *args;
	bool_t (*xdr_rslt)();
	char *rslt;
	struct ucred *cred;
{
	struct sockaddr_in sin;
	CLIENT *client;
	enum clnt_stat stat;

	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	bzero(sin.sin_zero, sizeof(sin.sin_zero));
	if (getport_loop(&sin, (u_long)KEY_PROG, (u_long)KEY_VERS, 
		(u_long)IPPROTO_UDP) != 0) 
	{
		debug("unable to get port number for keyserver");
		return (0);
	}
	client = clntkudp_create(&sin, (u_long)KEY_PROG, (u_long)KEY_VERS, 
		KEY_NRETRY, cred);

	if (client == NULL) {
		debug("could not create keyserver client");
		return (0);
	}
	/* for use by clntkudp_callit */
	client->cl_auth->ah_private = (caddr_t)cred;

	stat = clnt_call(client, procn, xdr_args, args, xdr_rslt, rslt, 
			 trytimeout);

	client->cl_auth->ah_private = (caddr_t)NULL;
	auth_destroy(client->cl_auth);
	clnt_destroy(client);
	if (stat != RPC_SUCCESS) {
		debug("keyserver clnt_call failed: ");
		debug(clnt_sperrno(stat));
		return (0);
	}
	return (1);
}

#else

#include <stdio.h>
#include <sys/wait.h>

 
static
key_call(proc, xdr_arg, arg, xdr_rslt, rslt)
	u_long proc;
	bool_t (*xdr_arg)();
	char *arg;
	bool_t (*xdr_rslt)();
	char *rslt;
{
	XDR xdrargs;
	XDR xdrrslt;
	FILE *fargs;
	FILE *frslt;
	int (*osigchild)();
	union wait status;
	int pid;
	int success;
	int ruid;
	int euid;
	static char *MESSENGER = "/usr/sbin/keyenvoy";

	success = 1;

	/*
	 * We are going to exec a set-uid program which makes our effective uid
	 * zero, and authenticates us with our real uid. We need to make the 
	 * effective uid be the real uid for the setuid program, and 
	 * the real uid be the effective uid so that we can change things back.
	 */
	euid = geteuid();
	ruid = getuid();
	(void) setreuid(euid, ruid);
	pid = _openchild(MESSENGER, &fargs, &frslt);
	(void) setreuid(ruid, euid);
	if (pid < 0) {
		debug("open_streams");
		return (0);
	}
	xdrstdio_create(&xdrargs, fargs, XDR_ENCODE);
	xdrstdio_create(&xdrrslt, frslt, XDR_DECODE);

	if (!xdr_u_long(&xdrargs, &proc) || !(*xdr_arg)(&xdrargs, arg)) {
		debug("xdr args");
		success = 0; 
	}
	(void) fclose(fargs);

	if (success && !(*xdr_rslt)(&xdrrslt, rslt)) {
		debug("xdr rslt");
		success = 0;
	}

	(void) fclose(frslt); 
	if (wait(&status.w_status) < 0 || status.w_retcode != 0) {
		debug("wait");
		success = 0; 
	}

	return (success);
}
#endif
