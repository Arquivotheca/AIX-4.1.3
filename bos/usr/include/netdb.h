/* @(#)43       1.16  src/bos/usr/include/netdb.h, sockinc, bos411, 9438C411a 9/20/94 14:53:44 */
#ifndef _H_NETDB
#define _H_NETDB
#ifdef _POWER_PROLOG_
/*
 *   COMPONENT_NAME: SOCKINC
 *
 *   FUNCTIONS: 
 *
 *   ORIGINS: 26,27,71
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988,1992
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#endif /* _POWER_PROLOG_ */

/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 */



#ifdef _THREAD_SAFE
/*
 * Per thread h_errno is provided by the threads provider. Both the extern int
 * and the per thread value must be maintained by the threads library.
 */
#define h_errno   (*_h_errno())

#endif  /* _THREAD_SAFE */

#define _PATH_HEQUIV    "/etc/hosts.equiv"
#define _PATH_HOSTS     "/etc/hosts"
#define _PATH_NETWORKS  "/etc/networks"
#define _PATH_PROTOCOLS "/etc/protocols"
#define _PATH_SERVICES  "/etc/services"
#define _PATH_SERVCONF	"/etc/netsvc.conf"
#define _PATH_RPCDB	"/etc/rpc"

/*
 * Structures returned by network
 * data base library.  All addresses
 * are supplied in host order, and
 * returned in network order (suitable
 * for use in system calls).
 */
#include <stdio.h>
#include <netinet/in.h>

#define	_MAXSERVICES	255
#define _MAXALIASES	35
#define _MAXADDRS	35
#define _MAXLINELEN	1024
#define _HOSTBUFSIZE	(BUFSIZ + 1)

struct  hostent {
        char    *h_name;                        /* official name of host */
        char    **h_aliases;		         /* alias list */
        int     h_addrtype;                     /* host address type */
        int     h_length;                       /* length of address */
        char    **h_addr_list; 		        /* list of addresses from */
                                                /* name server */
#define	h_addr	h_addr_list[0]	/* address, for backward compatiblity */
};

/*
 * Trouble here... nlist.h and syms.h #define n_name.
 */
#ifdef n_name
#undef n_name
#endif

#ifdef _THREAD_SAFE
struct resolver_routines {
	int	version_num;
	int	(*gethostbyname)();
	int	(*gethostbyaddr)();
	int	(*gethostent)();
	int	(*sethostent)();
	int	(*endhostent)();
};
#else
struct resolver_routines {
	int		version_num;
	struct hostent	*(*gethostbyname)();
	struct hostent	*(*gethostbyaddr)();
	struct hostent	*(*gethostent)();
	int		(*sethostent)();
	int		(*endhostent)();
};
#endif

struct services {
	char	*name;
	int	auth;
	struct resolver_routines routine_addrs;
};

/*
 * Assumption here is that a network number
 * fits in 32 bits -- probably a poor one.
 */
struct	netent {
	char		*n_name;	/* official name of net */
	char		**n_aliases;	/* alias list */
	int		n_addrtype;	/* net address type */
	in_addr_t	n_net;		/* network # */
};

#ifdef  _THREAD_SAFE
struct  netent_data {		/* should be considered opaque */
	FILE 	*net_fp;
	char 	line[_MAXLINELEN];
	char	*net_aliases[_MAXALIASES];
	int	_net_stayopen;
	char	*current;
	int	currentlen;
	void	*_net_reserv1;		/* reserved for future use */
	void	*_net_reserv2;		/* reserved for future use */
};
#endif  /* _THREAD_SAFE */
#ifdef  _THREAD_SAFE
/*
 * After a successful call to gethostbyname_r()/gethostbyaddr_r(), the
 * structure hostent_data will contain the data to which pointers in
 * the hostent structure will point to.
 */
struct  hostent_data {
        struct  in_addr host_addr;       /* host address pointer */
        char    *h_addr_ptrs[_MAXADDRS + 1];    /* host address */
	char	hostaddr[_MAXADDRS];
        char    hostbuf[_HOSTBUFSIZE + 1];      /* host data */
	char	*host_aliases[_MAXALIASES];
	char	*host_addrs[2];
	FILE 	*hostf;
	int	stayopen;		/* AIX addon */
	ulong	host_addresses[_MAXADDRS];	/* As per defect 48367. */
	int	this_service;
	char 	domain[256];
	char 	*current;
	int	currentlen;
	void	*_host_reserv1;		/* reserved for future use */
	void	*_host_reserv2;		/* reserved for future use */
};						/*    Actual Addresses. */
#endif  /* _THREAD_SAFE */

struct	servent {
	char	*s_name;	/* official service name */
	char	**s_aliases;	/* alias list */
	int	s_port;		/* port # */
	char	*s_proto;	/* protocol to use */
};

#ifdef  _THREAD_SAFE
struct  servent_data {		/* should be considered opaque */
	FILE 	*serv_fp;
	char 	line[_MAXLINELEN];
	char 	*serv_aliases[_MAXALIASES];
	int 	_serv_stayopen;
	char	 *current;
	int 	currentlen;
	void	*_serv_reserv1;		/* reserved for future use */
	void	*_serv_reserv2;		/* reserved for future use */
};
#endif  /* _THREAD_SAFE */

struct	protoent {
	char	*p_name;	/* official protocol name */
	char	**p_aliases;	/* alias list */
	int	p_proto;	/* protocol # */
};

#ifdef  _THREAD_SAFE
struct  protoent_data {		/* should be considered opaque */
	FILE 	*proto_fp;
	int 	_proto_stayopen;
	char 	line[_MAXLINELEN];
	char 	*proto_aliases[_MAXALIASES];
	int 	currentlen;
	char 	*current;
	void	*_proto_reserv1;	/* reserved for future use */
	void	*_proto_reserv2;	/* reserved for future use */
};
#endif  /* _THREAD_SAFE */

struct rpcent {
	char	*r_name;	/* name of server for this rpc program */
	char	**r_aliases;	/* alias list */
	int	r_number;	/* rpc program number */
};

#ifdef  _THREAD_SAFE
struct  rpcent_data {		/* should be considered opaque */
	FILE 	*rpc_fp;
	int 	_rpc_stayopen;
	char 	line[_MAXLINELEN];
	char 	*rpc_aliases[_MAXALIASES];
	int 	currentlen;
	char 	*current;
	void	*_rpc_reserv1;	/* reserved for future use */
	void	*_rpc_reserv2;	/* reserved for future use */
};
#endif  /* _THREAD_SAFE */

#ifdef _THREAD_SAFE
struct innetgr_data {		/* should be considered opaque */
	char *name;
	char *machine;
	char *domain;
	char *list[200];
	char **listp;
};
#endif /* _THREAD_SAFE */

#ifdef _NO_PROTO

	struct hostent	*gethostbyname(), *gethostbyaddr(), *gethostent();
	struct netent	*getnetbyname(), *getnetbyaddr(), *getnetent();
	struct servent	*getservbyname(), *getservbyport(), *getservent();
	struct protoent	*getprotobyname(), *getprotobynumber(), *getprotoent();
	struct rpcent	*getrpcbyname(), *getrpcbynumber(), *getrpcent();

#else

#ifdef _XOPEN_EXTENDED_SOURCE
void		endhostent(void);
void		endnetent(void);
void		endprotoent(void);
void		endrpcent(void);
void		endservent(void);
struct hostent	*gethostbyaddr(const void *, size_t, int);
struct hostent	*gethostbyname(const char *);
struct hostent	*gethostent(void);
struct netent	*getnetbyaddr(in_addr_t, int);
struct netent	*getnetbyname(const char *);
struct netent	*getnetent(void);
struct protoent *getprotobyname(const char *);
struct protoent	*getprotobynumber(int);
struct protoent *getprotoent(void);
struct rpcent	*getrpcbyname(const char *);
struct rpcent	*getrpcbynumber(int);
struct rpcent	*getrpcent(void);
struct servent	*getservbyname(const char *, const char *);
struct servent	*getservbyport(int, const char *);
struct servent	*getservent(void);
void		sethostent(int);
void		setnetent(int);
void		setprotoent(int);
void		setrpcent(int);
void		setservent(int);
#else	/* _XOPEN_EXTENDED_SOURCE */
extern struct hostent *gethostbyname (const char *);
extern struct hostent *gethostbyaddr (const char *, int, int);
extern int sethostent (int);
extern void endhostent (void);

extern struct netent *getnetbyname (const char *);
extern struct netent *getnetbyaddr (long, int);
extern struct netent *getnetent (void);
extern int setnetent (int);
extern void endnetent (void);

extern struct servent *getservbyname (const char *, const char *);
extern struct servent *getservbyport (int, const char *);
extern struct servent *getservent (void);
extern int setservent (int);
extern void endservent (void);

extern struct protoent *getprotobyname (const char *);
extern struct protoent *getprotobynumber (int);
extern struct protoent *getprotoent (void);
extern int setprotoent (int);
extern void endprotoent (void);
#endif /* _XOPEN_EXTENDED_SOURCE */

#endif /* _NO_PROTO */

#ifdef _NO_PROTO

#ifdef _THREAD_SAFE
extern int gethostbyname_r();
extern int gethostbyaddr_r();
extern int gethostent_r();
extern int setservent_r();
extern void endservent_r();
extern int getservent_r();
extern int getservbyname_r();
extern int getservbyport_r();
extern int setnetent_r();
extern void endnetent_r();
extern int getnetbyaddr_r();
extern int getnetbyname_r();
extern int getnetent_r();
extern int setprotoent_r();
extern void endprotoent_r();
extern int getprotoent_r();
extern int getprotobyname_r();
extern int getprotobynumber_r();
extern int setrpcent_r();
extern void endrpcent_r();
extern int getrpcbyname_r();
extern int getrpcbynumber_r();
extern int getrpcent_r();

#endif /* _THREAD_SAFE */

#else /* _NO_PROTO */

#ifdef _THREAD_SAFE
extern int getnetbyaddr_r(int net, int type, struct netent *netptr, 
	struct netent_data *net_data);
extern int getnetbyname_r(const char *name, struct netent *netptr, 
	struct netent_data *net_data);
extern int setnetent_r(int f, struct netent_data *net_data);
extern void endnetent_r(struct netent_data *net_data);
extern int getnetent_r(struct netent *net, struct netent_data *net_data);
extern int gethostbyname_r(const char *name, struct hostent *htent, 
	struct hostent_data *ht_data);
extern int gethostbyaddr_r(char *addr, int len, int type, struct hostent *htent,
	struct hostent_data *ht_data);
extern int gethostent_r(struct hostent *htent, struct hostent_data *ht_data);
extern int setservent_r(int f, struct servent_data *serv_data);
extern void endservent_r(struct servent_data *serv_data);
extern int getservent_r(struct servent *serv, struct servent_data *serv_data);
extern int getservbyname_r(const char *name, const char *proto, 
	struct servent *servptr, struct servent_data *serv_data);
extern int getservbyport_r(int port, const char *proto, struct servent *servptr,
	struct servent_data *serv_data);
extern int setprotoent_r(int f, struct protoent_data *proto_data);
extern void endprotoent_r(struct protoent_data *proto_data);
extern int getprotoent_r(struct protoent *proto, 
	struct protoent_data *prot_data);
extern int getprotobyname_r(const char *name, struct protoent *protoptr,
	struct protoent_data *proto_data);
extern int getprotobynumber_r(int proto, struct protoent *protoptr,
	 struct protoent_data *proto_data);
extern int setrpcent_r(int f, struct rpcent_data *rpc_data);
extern void endrpcent_r(struct rpcent_data *rpc_data);
extern int getrpcent_r(struct rpcent *rpcent, struct rpcent_data *rpc_data);
extern int getrpcbyname_r(const char *name, struct rpcent *rpcent, 
			  struct rpcent_data *rpc_data);
extern int getrpcbynumber_r(int number, struct rpcent *rpcent,
			    struct rpcent_data *rpc_data);

#endif /* _THREAD_SAFE */

#endif /* _NO_PROTO */
/*
 * Error return codes from gethostbyname() and gethostbyaddr()
 * (left in extern int h_errno).
 */
extern	int h_errno;

#define	HOST_NOT_FOUND	1 /* Authoritative Answer Host not found */
#define	TRY_AGAIN	2 /* Non-Authoritive Host not found, or SERVERFAIL */
#define	NO_RECOVERY	3 /* Non recoverable errors, FORMERR, REFUSED, NOTIMP */
#define	NO_DATA		4 /* Valid name, no data record of requested type */
#define	NO_ADDRESS	NO_DATA		/* no address, look for MX record */
#define SERVICE_UNAVAILABLE 5

#endif /* _H_NETDB */
