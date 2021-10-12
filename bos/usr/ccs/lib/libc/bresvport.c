static char sccsid[] = "@(#)85  1.8  src/bos/usr/ccs/lib/libc/bresvport.c, libcnet, bos411, 9428A410j 11/22/93 09:17:39";
/*
 * COMPONENT_NAME: LIBCNET bresvport.c
 *
 * FUNCTIONS: bindresvport,  
 *           
 *
 * ORIGINS: 24  27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

/* Copyright (c) 1987 by Sun Microsystems, Inc. */

#include <sys/types.h>
#include <sys/errno.h>
#include <sys/socket.h>
#include <netinet/in.h>

/*
 * Bind a socket to a privileged IP port
 */
bindresvport(sd, sin)
        int sd;
        struct sockaddr_in *sin;
{
        int res;
#ifdef _THREAD_SAFE
	/**************
	  No more optimization, but this makes the
	  code thread_safe
	**************/
	short port = 0;
#else
        static short port=0;
#endif /* _THREAD_SAFE */
        struct sockaddr_in myaddr;
        int i;

#define STARTPORT 600
#define ENDPORT (IPPORT_RESERVED - 1)
#define NPORTS  (ENDPORT - STARTPORT + 1)

        if (sin == (struct sockaddr_in *)0) {
                sin = &myaddr;
                bzero(sin, sizeof (*sin));
                sin->sin_family = AF_INET;
        } else if (sin->sin_family != AF_INET) {
                errno = EPFNOSUPPORT;
                return (-1);
        }
        if (port == 0) {
                port = (getpid() % NPORTS) + STARTPORT;
        }

        res = -1;
        errno = EADDRINUSE;
        for (i = 0; i < NPORTS && res < 0 && errno == EADDRINUSE; i++) {
                while (alreadyreserved(port)) port++;
                sin->sin_port = htons(port++);
                if (port > ENDPORT) {
                        port = STARTPORT;
                }
                res = bind(sd, sin, sizeof(struct sockaddr_in));
        }
        return (res);
}

int alreadyreserved(int port)
{
/*
This static array comes from rfc1060.  When rfs1060 gets obsoleted,
this routine must change.  This was the least ugly solution we could
come up with. Sorry.

We don't want to allocate these since one of these guys may want to
start up and grab his port later.

We hardcoded it instead of asking NIS (the other obvious solution,) so
we could boot if the NIS server was down, since none of the servers who
want ports, like mountd for example, would come up.  If it happenned
that the system wasn't an NIS client, then we could just look in
/etc/services, but we can't assume that noone is an NIS client.
*/

/* OK, this array has now been updated for RFC1340 which obsoletes RFC1060
*/
        static int numinarray=0;
        int i;
        static int portarray[]=
		{600,   /* pcserver        */
                 607,   /* nqs             */
                 666,   /* mdqs            */
		 704,	/* elcs errlog copy/server daemon  		*/
		 740,	/* netcp NETscout Control Protocol   		*/
		 741,	/* netgw netGW     				*/
		 742,	/* netrcs Network based Rev. Cont.Sys.		*/
		 744,	/* flexlm Flexible License Manager    		*/
		 747,	/* fujitsu-dev Fujitsu Device Control  		*/
	 	 748,	/* ris-cm Russell Info Sci Calnder Mgr 		*/
		 749,	/* kerberos-adm kerberos administration		*/
                 750,   /* rfile           */
                 751,   /* pump            */
                 752,   /* qrh             */
                 753,   /* rrh             */
                 754,   /* tell            */
                 758,   /* nlogin          */
                 759,   /* con             */
                 760,   /* ns              */
                 761,   /* rxe             */
                 762,   /* quotad          */
                 763,   /* cycleserv       */
                 764,   /* omserv          */
                 765,   /* webster         */
                 767,   /* phonebook       */
                 769,   /* vid             */
		 770,	/* cadlock					*/
                 771,   /* rtip            */
                 772,   /* cycleserv2      */
                 773,   /* submit          */
                 774,   /* rpasswd         */
                 775,   /* entomb          */
                 776,   /* wpages          */
                 780,   /* wpgs            */
		 781,	/* hp-collector hp performance data collector 	*/
		 782,	/* hp-managed-node hp perf. data managed node 	*/
		 783,	/* hp-alarm-mgr hp perf. data alarm manager	*/
                 800,   /* mdbs_daemon     */
                 801,   /* device          */
		 996,	/* xtreelic XTREE License Server		*/
                 997,   /* maitrd          */
                 998,   /* busboy          */
                 999,	/* garcon          */
		 1000	/* cadlock/ock					*/
		};
        numinarray=sizeof(portarray)/sizeof(int);

        /* Is the port in question already reserved by the RFC?*/
        for (i=0; i<numinarray; i++)
                if (port==portarray[i]) 
                        return(1); 		/* return true */
        return(0);
}
