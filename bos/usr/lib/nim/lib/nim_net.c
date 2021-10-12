static char	sccs_id[] = " @(#)98 1.13  src/bos/usr/lib/nim/lib/nim_net.c, cmdnim, bos411, 9428A410j  6/22/94  16:23:32";
/*
 *   COMPONENT_NAME: CMDNIM
 *
 *   FUNCTIONS: getsocket
 *		nim_connect
 *		nimappend
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/******************************************************************************
 *
 *                             NIM function library
 *
 * this file contains functions which are common across ALL NIM commands
 * NONE of these files should reference the NIM database; any function which
 *		does belongs in the libmstr.c file
 ******************************************************************************/

#include "cmdnim.h"
#include "cmdnim_ip.h"
#include <varargs.h>
#include <sys/wait.h>
#include <sys/cfgodm.h> 
#include <netdb.h>
#include <net/if.h> 
#include <net/if_dl.h> 

#define max(a, b) 	(a > b ? a : b)
#define size(p) 	max((p).sa_len, sizeof(p))

#define ADDRESS_TOKEN   "*NA" 


/*---------------------------- verify_hostname           -----------------------
 *
 * NAME: verify_hostname
 *
 * FUNCTION:
 *		resolves <name> to IP addr
 *		returns full hostname and/or IP address when <hostname> & <ip> are > NULL
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *		sets errstr on failure
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *		parameters:
 *			name					= name to resolve
 *			hostname				= if >NULL, ptr to char ptr
 *			ip						= if >NULL, ptr to char ptr
 *		global:
 *
 * RETURNS: (int)
 *		SUCCESS					= <name> can be resolved
 *		FAILURE					= unable to resolve <name>
 *
 * OUTPUT:
 *-----------------------------------------------------------------------------*/

int
verify_hostname(	char *name,
						char **hostname,
						char **ip )

{	
	struct hostent *hostptr;
	struct in_addr *addrPtr;
	char *ptr;

	VERBOSE4("      verify_hostname: name=%s;\n",name,NULL,NULL,NULL)

	if ( regexec( nimere[HOSTNAME_ERE].reg, name, 0, NULL, 0 ) != 0 )
		ERROR( ERR_VALUE, name, MSG_msg(MSG_HOSTNAME), NULL )

	if ( (hostptr = gethostbyname( name )) == NULL )
		ERROR( ERR_IP_RESOLVE, name, MSG_msg(MSG_NET_ADDR), NULL )

	if ( hostname != NULL ) {	/* return the full hostname */
		*hostname = nim_malloc( strlen( hostptr->h_name ) + 1 );
		strcpy( *hostname, hostptr->h_name );
	}

	if ( ip != NULL ) {
		addrPtr = (struct in_addr *) *(hostptr->h_addr_list);
		if ( (ptr = inet_ntoa( addrPtr->s_addr )) == NULL )
			ERROR( ERR_IP_RESOLVE, name, MSG_msg(MSG_NET_ADDR), NULL )

		/* inet_ntoa points to a staic struct, so we must malloc our own */
		*ip = nim_malloc( strlen(ptr) + 1 );
		strcpy( *ip, ptr );
	}

	return( SUCCESS );

} /* end of verify_hostname */
	
/*---------------------------- verify_net_addr       ---------------------------
*
* NAME: verify_net_addr
*
* FUNCTION:
*		verifies that the specified network address (specified as either a
*			hostname or IP address) is a valid address
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*		sets errstr on failure
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*			addr					= network address (either hostname or IP addr)
*		global:
*
* RETURNS: (int)
*		SUCCESS					= <name> can be resolved
*		FAILURE					= unable to resolve <name>
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
verify_net_addr(	char *addr )

{	struct hostent *hostptr;
	regmatch_t hname[ERE_HOSTNAME_NUM];

	VERBOSE4("      verify_net_addr: addr=%s;\n",addr,NULL,NULL,NULL)

	/* hostname or IP addr? */
	if (	(regexec(	nimere[HOSTNAME_ERE].reg, addr, ERE_HOSTNAME_NUM,
							hname, 0 ) == 0) &&
			(hname[1].rm_so >= 0) )
	{	/* resolve as hostname */
		if ( verify_hostname( addr, NULL, NULL ) == NULL )
			ERROR( ERR_VALUE, addr, MSG_msg(MSG_NET_ADDR), NULL )
	}
	else
	{	/* assume it's an IP addr - verify it as such */
		/* initialize the sockaddr_in struct - convert to inet addr */
		if (	inet_addr( addr ) == -1 )
			ERROR( ERR_VALUE, addr, MSG_msg(MSG_NET_ADDR), NULL )
	}
		
	return( SUCCESS );

} /* end of verify_net_addr */
	
/*---------------------------- verify_snm        ------------------------------
*
* NAME: verify_snm
*
* FUNCTION:
*		verifies a subnetmask
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*		sets errstr on failure
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
			snm					= subnetmask to verify
*		global:
*
* RETURNS: (int)
*		SUCCESS					= <snm> is ok
*		FAILURE					= not ok
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
verify_snm(	char *snm )

{	int first;
	int second;
	int third;
	int fourth;

	VERBOSE4("      verify_snm: snm=%s;\n",snm,NULL,NULL,NULL)

	/* subnetmask should be: */
	/*		- 4 octets */
	/*		- each octest <= 255 */
	if (	((sscanf( snm, "%3d.%3d.%3d.%3d", &first, 
						&second, &third, &fourth )) != 4) ||
			(first > 255) ||
			(second > 255) ||
			(third > 255) ||
			(fourth > 255) )
		ERROR( ERR_VALUE, snm, MSG_msg(MSG_SNM), NULL )

	return( SUCCESS );

} /* end of verify_snm */

/*---------------------------- verify_ring_speed ------------------------------
*
* NAME: verify_ring_speed
*
* FUNCTION:
*		verifies the value of a ring_speed attr assignment
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*		sets errstr on failure
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*			speed					= ring_speed value
*		global:
*
* RETURNS: (int)
*		SUCCESS					= <speed> ok
*		FAILURE					= invalid ring speed
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
verify_ring_speed(	char *speed )

{	int ring_speed;

	VERBOSE4("      verify_ring_speed: speed=%s;\n",speed,NULL,NULL,NULL)

	ring_speed = (int) strtol( speed, NULL, 0 );

	if ( (ring_speed != 4) && (ring_speed != 16) )
		ERROR( ERR_VALUE, speed, ATTR_RING_SPEED_T, NULL )

	return( SUCCESS );

} /* end of verify_ring_speed */
	

/*---------------------------- verify_cable_type ------------------------------
*
* NAME: verify_cable_type
*
* FUNCTION:
*		verifies the ethernet cable type
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*		sets errstr on failure
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*			ct						= cable_type
*		global:
*
* RETURNS: (int)
*		SUCCESS					= cable type ok
*		FAILURE					= invalid cable type
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

#define BNC		"bnc"
#define DIX		"dix"
#define N_A		"N/A"

int
verify_cable_type(	char *ct )

{

	VERBOSE4("      verify_cable_type: type=%s;\n",ct,NULL,NULL,NULL)

	if (	(strcmp( ct, BNC ) == 0) ||
			(strcmp( ct, DIX ) == 0) ||
			(strcmp( ct, N_A ) == 0) )
		return( SUCCESS);

	ERROR( ERR_VALUE, ct, ATTR_CABLE_TYPE_T, NULL )

} /* end of verify_cable_type */
	
/* ---------------------------- nimappend 
 *
 * NAME:	 nimappend
 *
 * FUNCTION:	nimappend takes care of addeding In_packet to the
 *		end of the Buffer. Termination is cause by zero 
 *		newline or linefeed.
 *
 * DATA STRUCTURES:
 *		parameters:
 *			Buffer		- A pointer for a destination buffer. 
 *			In_packet	- A pointer to the input.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:	This function does not check the upper bound of Buffer.
 *
 * -------------------------------------------------------------------------*/

void
nimappend(	char	*Buffer, 
				char	*In_packet )

{
	char	*dest=Buffer; 
	char	*src=In_packet;

	/* 
	 * Find the end of buffer to tack in_packet on to
	 */
	while (*dest)
		dest++;
	/* 
	 * copy the src to the dest until end of string (a null) or 
	 * we hit a newline
	 */ 
	 while (*src != '\0' && *src!='\n' && *src!='\r')
		*dest++=*src++;
	*dest='\0';

} /* end of nimappend */

/* ---------------------------- nim_connect      ------------------------------
 *
 * NAME: nim_connect
 *
 * FUNCTION:
 *		Establishes a connection to a remote NIM process.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:	The socket information is returned in the struct pointed to 
 *				by Sockinfo.
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *		parameters:
 *			Sockinfo		- A pointer to the socket information needed.
 *		global:
 *
 * RETURNS:
 *	SUCCESS		= A valid file pointer to the connection
 *	FAILURE		= NULL
 *
 * EXITS:
 *
 * OUTPUT:
 * --------------------------------------------------------------------------*/

int
nim_connect(	NIM_SOCKINFO *Sockinfo )

{	

	int	addrSize = sizeof(Sockinfo->addr);
	int	cState = -1; 
	int	attempts=10; 

	VERBOSE4("      nim_connect\n",NULL,NULL,NULL,NULL)

	if ( (getsocket(Sockinfo)) != SUCCESS )
		return(FAILURE);
	
	while ( ( (cState=connect( Sockinfo->fd,
		         (struct sockaddr *) &(Sockinfo->addr),addrSize)) == -1) && 
		         attempts) { 
		/* 
		 * If we're hit by an interrupted system call then try again
		 */
		if (errno == EINTR)  
			continue;
		/*
		 * If the connection was refused then the socket descriptor is 
		 * no longer valid, so close it and re-open then attempt the 
		 * connection again. 
		 */
		if (errno == ECONNREFUSED) { 
			if ( (getsocket(Sockinfo)) != SUCCESS )  
				return(FAILURE);
			sleep(1);
			attempts--; 
			continue;
		}
		nim_error(ERR_ERRNO_LOG, "nim_connect:", "connect", NULL);
		close(Sockinfo->fd); 
		return(FAILURE);
	}
	if (cState != 0) {
		close(Sockinfo->fd);
		return(FAILURE);
	}
	if ( (Sockinfo->FP=fdopen(Sockinfo->fd, "wr")) == NULL ) { 
		close(Sockinfo->fd);
		return(FAILURE);
	}
	return(SUCCESS);

} /* end of nim_connect */


/* ----------------------------- getsocket
 *
 * NAME: getsocket
 *
 * FUNCTION:
 *		This function will obtain a socket for the caller, if 
 * fd has a value it is assumed a socket is already opened so a close 
 * is done and a reopen performed. 
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *		parameters:
 *			Sockinfo		- A pointer to the struct in which we will return the 
 *							  Socket information.
 *		global:
 *
 * RETURNS:
 *	SUCCESS  -  It worked ok.		
 *	FAILURE  -  It did not work.	
 *
 * EXITS:
 *
 * OUTPUT:
 * --------------------------------------------------------------------------*/

int
getsocket(	NIM_SOCKINFO *Sockinfo )

{
	int start_port; 

	VERBOSE4("      getsocket\n",NULL,NULL,NULL,NULL)

	/* 
	 * The socket fd may be in use, so close it then re-open
	 */
	if (Sockinfo->fd > 0)
		close(Sockinfo->fd);
	/*
	 *  Allocate a socket depending on the state of useRes. If flagged 
	 * we'll use a reserved port else we'll use from the pool ! (splash)
	 */
	if ( Sockinfo->useRes ) { 
		start_port = IPPORT_RESERVED-1; 
		if ( (Sockinfo->fd=rresvport(&start_port)) < 0 ) {
			/* 
		 	 * We are unable to open a socket
		 	 */
			nim_error(ERR_ERRNO_LOG, "getsocket:", "rresvport", NULL);
			return(FAILURE);
		}
	}
	else { 
		if ((Sockinfo->fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
			/* 
		 	 * We are unable to open a socket
		 	 */
			nim_error(ERR_ERRNO_LOG, "getsocket:", "socket", NULL);
			return(FAILURE);
		}
	}
	return(SUCCESS);

} /* end of getsocket */


/* ----------------------------- get_adpt_addr
 *
 * NAME: get_adpt_addr
 *
 * FUNCTION: This function will obtain an adapter address for the caller. The  
 * address (if found) will be placed into the current attribute assingment list.
 *
 * DATA STRUCTURES:
 *	parameters:
 *		sd - socket descriptor.		
 *	  attr_ass - ptr to attr assignments					
 *	  net_name - the name of the network
 *
 * RETURNS:
 *	SUCCESS  -  It worked ok.		
 *	FAILURE  -  It did not work.	
 *
 * --------------------------------------------------------------------------*/

int
get_adpt_addr( 	int 	 sd, 
		ATTR_ASS *attr_ass, 
		char 	 *net_name,
	struct  sockaddr_dl **ll_addr )
{
	int 	bufsize;
	int	addrSize;
	char	*cp, *acp; 
	char	adpt_addr[20];
	caddr_t max_ifreqs;

struct  ifconf 	ifconf_req;
struct  ifreq 	*ifrp;

	VERBOSE4("      get_adpt_addr: net_name=%s;\n",net_name,NULL,NULL,NULL)

	/* 
	 * Hunting for the network adapter hardware address. 
	 * Guestimate that 16 interfaces will cover most situations
	 */
	bufsize = 16 * sizeof(struct ifreq)+1;
	
	for (; ; ) {
		ifconf_req.ifc_req = (struct ifreq *) malloc((unsigned) bufsize);
		if (!ifconf_req.ifc_req) 
			ERROR(ERR_ERRNO_LOG, "get_adpt_addr","malloc:", NULL);
		
		ifconf_req.ifc_len = bufsize;

		if (ioctl(sd, SIOCGIFCONF, (char *)&ifconf_req) < 0) 
			ERROR(ERR_ERRNO_LOG, "get_adpt_addr", "ioctl SIOCGIFCONF:", NULL);
		/*
       		 * if spare buffer space for at least one more interface all found
         	 */
		if ((bufsize - ifconf_req.ifc_len) > sizeof(struct ifreq )) {
			break;
		}
		/* 
		 * Not all interfaces found so double buffer size and retry
		 */
		free((char *) ifconf_req.ifc_req);
		if (bufsize > 64 * sizeof(struct ifreq )) {
			errno=E2BIG;
			ERROR(ERR_ERRNO_LOG, "get_adpt_addr","> 63 if", NULL);
		}
		bufsize *= 2;
	}

	/* 
	 * Now that we have all the interfaces romp through looking 
	 * for the ifr_name we're intrested in... 
	 */

	max_ifreqs = (caddr_t) ifconf_req.ifc_req + ifconf_req.ifc_len;

	for (ifrp = ifconf_req.ifc_req; 
		(caddr_t) ifrp < max_ifreqs; 
	    	ifrp = (struct ifreq *) ((caddr_t)ifrp + 
		max(sizeof(struct ifreq ),(sizeof(ifrp->ifr_name)+size(ifrp->ifr_addr))))) {

		if ( ( strcmp(ifrp->ifr_name, net_name ) == 0 ) && 
			(ifrp->ifr_addr.sa_family == AF_LINK) ) {
			/* 
			 * Got the af_link for our interface now check its
			 * present (supported) ..
			 */
			*ll_addr = (struct sockaddr_dl *) &(ifrp->ifr_addr);
			if ( (*ll_addr)->sdl_alen == 0 ) {
				errno=ENOSYS;
				ERROR(ERR_ERRNO_LOG, MSG_msg (MSG_GET_ADAPT_ADDR),
												MSG_msg (MSG_CANT_GET_ADAPT_ADDR),  NULL);
			}
			cp  = adpt_addr; 
			acp = (char *)LLADDR(*ll_addr);
			for ( 	addrSize = (*ll_addr)->sdl_alen; addrSize > 0 ; 
				addrSize--, cp+=(sizeof(char)*2) )
				sprintf(cp, "%02X", *acp++);
			add_attr(attr_ass, ATTR_ADPT_ADDR, ATTR_ADPT_ADDR_T, adpt_addr);
			return(SUCCESS);
			break; 	
		}
	}
	errno=ENXIO;
	ERROR(ERR_ERRNO_LOG, MSG_msg (MSG_GET_ADAPT_ADDR), 
										MSG_msg (MSG_ADAPT_NOT_FOUND), NULL);
}

/* ----------------------------- addr_to_str
 *
 * NAME: addr_to_str
 *
 * FUNCTION: Convert the hardware address to a string.
 *
 * RETURNS:
 *	string ptr  -  It worked ok.		
 *	NULL  	    -  It did not work.	
 *
 * --------------------------------------------------------------------------*/

char *
addr_to_str(unsigned char *addr)
{
static char digits[] = "0123456789ABCDEF";
	int	i;
	char 	*buf;
	char 	*ptr;
unsigned char	*addrPtr;

	if ( (buf=malloc(14)) == NULL )
		return(NULL);
	addrPtr=addr; 
	ptr=buf; 
	for (i = 0; i < 6; i++) {
		*ptr++ = digits[ *addrPtr >> 4 ];
		*ptr++ = digits[ *addrPtr++ & 0xf];
	}
	*ptr = 0;
	return (buf);
}

/* ----------------------------- memstr
 *
 * NAME: memstr
 *
 * FUNCTION: Find s in memory pointed to by m upto a length of 
 *	sz. 
 *
 * RETURNS:
 *	string ptr  -  It worked ok.		
 *	NULL  	    -  It did not work.	
 *
 * --------------------------------------------------------------------------*/

char *
memstr( char *m, int sz, char *s ) 
{ 
	char  *sptr; 
	char  *stop;
	if ( (m == NULL) || (s==NULL) || (sz < 1) )
		return(NULL);
	sptr=s; 
	stop=m+sz;
	for ( ; m < stop ; sptr=s )
		for ( ; *m++==*sptr++ ; )
			if ( !*sptr )
				return(m);
	return(NULL);
}

/* ----------------------------- getnet
 *
 * NAME: getnet
 *
 * FUNCTION: This function will obtain an network information about "net_name". 
 * The information is saved in the current attribute assignment list.
 *
 * DATA STRUCTURES:
 *	parameters:
 *	  attr_ass - ptr to attr assignments					
 *	  net_name - the name of the network
 *
 * RETURNS:
 *	SUCCESS  -  It worked ok.		
 *	FAILURE  -  It did not work.	
 *
 * --------------------------------------------------------------------------*/

int
getnet(ATTR_ASS *attr_ass, char *net_name )
{

struct  ifreq 	ifrq;
struct  sockaddr_dl *ll_addr;

struct  sockaddr_in *addrPtr ;
struct 	Class *CuVPD_CLASS; 
struct  listinfo   Info;
struct  CuVPD	*cipid; 

	char	*adpt_addr=NULL;
	char	*vpd_adpt_addr=NULL;
	char	*Query=NULL;
	char	*ptr; 
	
	int	sd;
	int	loop;
	int	found_it; 

	VERBOSE4("      getnet: net_name=%s;\n",net_name,NULL,NULL,NULL)

	strncpy(ifrq.ifr_name, net_name, sizeof(ifrq.ifr_name));

	sd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sd < 0)
		ERROR(ERR_ERRNO_LOG, "getnet", "socket", NULL);

	/* 
	 * Get the interface flags and test if its up, if it 
	 * is down then get outa here... 
	 */
	if (ioctl(sd, SIOCGIFFLAGS, &ifrq) < 0)
		ERROR(ERR_ERRNO_LOG, "getnet", "ioctl flags", NULL);

	if ( !(ifrq.ifr_flags & IFF_UP) ) {
		errno=ENETDOWN;
		ERROR(ERR_ERRNO, "getnet", NULL, NULL);
	}

	/* 
	 * Interface is up so its ok to carry on and get the ip address 
	 */
	if (ioctl(sd, SIOCGIFADDR, &ifrq) < 0)
		ERROR(ERR_ERRNO_LOG, "getnet", "ioctl ifaddr", NULL);

	addrPtr = (struct sockaddr_in *) & ifrq.ifr_addr;
	add_attr( attr_ass, ATTR_NET_ADDR, ATTR_NET_ADDR_T, inet_ntoa(addrPtr->sin_addr) );

	/* 
	 * Now get the netmask 
	 */
	if (ioctl(sd, SIOCGIFNETMASK, &ifrq) < 0)
		ERROR(ERR_ERRNO_LOG, "getnet", "ioctl netmask", NULL);

	add_attr( attr_ass, ATTR_SNM, ATTR_SNM_T, inet_ntoa(addrPtr->sin_addr) );

	/* 
	 * User has the option of inputting this info from the cmd line, if 
	 * so do not probe about to find it. 
	 */
	if ( (adpt_addr=attr_value(attr_ass, ATTR_ADPT_ADDR)) == NULL ) {
		if (get_adpt_addr(sd, attr_ass, net_name, &ll_addr) != SUCCESS)
			return(FAILURE);
		if ( (adpt_addr=addr_to_str(LLADDR(ll_addr))) == NULL )
			ERROR(ERR_ERRNO_LOG, "getnet", "adpt addr", NULL);
	}
	else { 
		/*
		 * Make sure its all in upper case.
		 */
		for ( loop=0; adpt_addr[loop]; loop++ )
			adpt_addr[loop]=toupper(adpt_addr[loop]);
	}

	/* 
	 * Now hunt through the CuVPD odm "database" for the device 
	 * name. Matching on the hardware adddress. 
	 */
	
	odm_initialize();

	if ( (CuVPD_CLASS = odm_mount_class("CuVPD")) == -1   || 
	     (cipid=odm_get_list(CuVPD_CLASS, Query, &Info, 100, 3 )) == -1) { 
		if (odm_err_msg( odmerrno, &ptr ) < 0 )
			sprintf(niminfo.errstr," ODM Error %d\n", odmerrno);
		else
			sprintf(niminfo.errstr," %s\n", ptr);
		return(FAILURE);
	}

	found_it=0;	
	for (loop=0; loop != Info.num; loop++) {
		if ( (ptr=memstr(cipid->vpd, sizeof(cipid->vpd), ADDRESS_TOKEN)) != NULL ) { 
			ptr++;
			if ( (vpd_adpt_addr=addr_to_str(ptr)) == NULL )
				ERROR(ERR_ERRNO_LOG, "getnet", "vpd adpt addr", NULL);
			if (strcmp( adpt_addr, vpd_adpt_addr ) == 0 ) {
				/* 
				 * Nasty old intergrated e-net is in the sio vpd so if this 
				 * is sio name, change it to ent0... Yes its a kludge
				 */
				if (strncmp(cipid->name, "sio", 3) == 0 )
					ptr="ent0"; 
				else 
					ptr=cipid->name; 
				add_attr(attr_ass, ATTR_ADPT_NAME, ATTR_ADPT_NAME_T, ptr );
				found_it++;
				break;
			}
		}
		cipid++;
	}
	if ( !found_it ) {	
		errno=ENODEV;
		ERROR(ERR_ERRNO_LOG, attr_value (attr_ass, ATTR_ADPT_ADDR), 
											MSG_msg (MSG_NO_ADPT_DEVICE_NAME), NULL);
	}
	return(SUCCESS);
}
