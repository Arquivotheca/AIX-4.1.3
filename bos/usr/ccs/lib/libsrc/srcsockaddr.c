static char sccsid[] = "@(#)25	1.9  src/bos/usr/ccs/lib/libsrc/srcsockaddr.c, libsrc, bos411, 9428A410j 3/22/91 17:00:35";
/*
 * COMPONENT_NAME: (cmdsrc) System Resource Controler
 *
 * FUNCTIONS:
 *	srcsockaddr
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregate modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989,1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/*
** IDENTIFICATION:
**    Name:	srcsockaddr
**    Title:	Get SRC Socket Address
** PURPOSE:
**	To get the address of SRC's Socket regardless of which machine
**	the specified SRC is running on.
** 
** SYNTAX:
**    srcsockaddr(sin,hostname)
**    Parameters:
**	o struct sockaddr_in *sin - address of SRC socket
**	i char *hostname - host to get address for
**
** INPUT/OUTPUT SECTION:
**
** PROCESSING:
**
** PROGRAMS/FUNCTIONS CALLED:
**	gethostbyname
**	getservbyname
**
** OTHER:
**
** RETURNS:
**	int TRUE on success or error code
**
**/
#include	"src.h"
#include	"srcsocket.h"
#include	<stdio.h>
#include	<netinet/in.h> 
#include	<arpa/inet.h> 
#include	<netdb.h> 

int srcsockaddr();
int srcgetport();
void srcafunixsockaddr();
void src_get_sun_path();

int srcsockaddr(sin,hostname)
struct sockaddr_in *sin;
char *hostname;
{
	struct hostent *gethostbyname();
	struct servent *getservbyname();
	struct hostent *host;


	if(hostname!=0 && *hostname!='\0')
	{
		/* init return socket address */
		bzero(sin,sizeof(struct sockaddr_in));

		/* get the hosts address */
		host = gethostbyname(hostname);
	
		/* was the host found? */
		if(host != (struct hostent *)0)
		{
			/* use hosts address */
			sin->sin_family = host->h_addrtype;
			memcpy((void *)&sin->sin_addr,(void *)host->h_addr,(size_t)host->h_length);
		}
		else
		{
			/* was not a hostname. is it an internet address in
			** standard dot notation?
			**/
			sin->sin_addr.s_addr = inet_addr(hostname);
	
			/* was not an internet address? */
			if(sin->sin_addr.s_addr == -1)
				return(SRC_UHOST);

			sin->sin_family = AF_INET;
		}
		return(srcgetport(sin));
	}
	else
	{
		srcafunixsockaddr(sin,0);
		return(TRUE);
	}
}

/*
** IDENTIFICATION:
**    Name:	srcafunixsockaddr
**    Title:	Get SRC AF_UNIX Socket Address
** PURPOSE:
**	To create an address for an SRC component to communicate on.
** 
** SYNTAX:
**    void srcsockaddr(sockaddr,key)
**    Parameters:
**	o struct sockaddr *sockaddr - local AF_UNIX socket address for SRC
**	i int key - sockaddr key - should be the pid of the process calling
**		execpt for srcmstr which sould be zero.
**
**/
void srcafunixsockaddr(sockaddr,key)
struct sockaddr_un *sockaddr;
int key;
{
	sockaddr->sun_family=AF_UNIX;
	src_get_sun_path(sockaddr->sun_path,key);
	sockaddr->sun_len = src_what_sockaddr_size(sockaddr);
}

/*
** IDENTIFICATION:
**    Name:	srcgetport
**    Title:	Get SRC port number 
** PURPOSE:
**	To get the src inet port number.
** 
** SYNTAX:
**    void srcgetport(sin)
**    Parameters:
**	o struct sockaddr_in *sin - socket address for SRC
**/
int srcgetport(sin)
struct sockaddr_in *sin;
{

	static short port=0;

	/* we only need to get the port number once
	**	assuming that port 0 will not be used
	**/
	if(port == 0)
	{
		struct servent *sp=(struct servent *) 0;

		/* get the src port number */
			sp = getservbyname("src","udp");
		if(sp == (struct servent *)0)
			return(SRC_UDP);
		port = sp->s_port;
	}

	sin->sin_port = port;

	return(TRUE);
}
/*
** IDENTIFICATION:
**    Name:	src_get_sa_data
**    Title:	Get SRC sa_data
** PURPOSE:
**	Fills in the AF_UNIX sockets sa_data field.
** 
** SYNTAX:
**    void src_get_sa_data(sa_data,key)
**    Parameters:
**	o char *sa_data - pointer to sockaddr->sa_data to be filed in
**		with the local address.
**	i int key - sockaddr key - should be the nonzero 
**		execpt for srcmstr which sould be zero.
**/
void src_get_sun_path(sun_path,key)
char *sun_path;
int key;
{

	if(key)
	{
		strcpy(sun_path,SRC_BASE_AF_UNIX);
		mktemp(sun_path);
	}
	else
		strcpy(sun_path,SRC_MASTER_AF_UNIX);
}
