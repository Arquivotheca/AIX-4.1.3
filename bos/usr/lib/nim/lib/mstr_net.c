static char sccs_id[] = " @(#)92 1.25.1.7  src/bos/usr/lib/nim/lib/mstr_net.c, cmdnim, bos41J, 9518A_all  5/2/95  16:26:23";
/*
 *   COMPONENT_NAME: CMDNIM
 *
 *   FUNCTIONS: connectivity
 *		find_net
 *		find_net_attr
 *		get_net_info
 *		get_nim_if
 *		net_connection
 *		s2nim_if
 *		stonim_route
 *		verify_if
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
*                        NIM master function library
*
* this library contains NIM database manipulation routines, which should only
*		be accessed by the NIM master
******************************************************************************/

#include "cmdnim.h"
#include "cmdnim_obj.h"
#include <varargs.h> 
#include "cmdnim_ip.h" 
#include <sys/wait.h> 

/*$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
$$$$$$$$$$$$$$$$ network stanza manipulation                       $$$$$$$$$$$$
$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$*/
	
/*---------------------------- s2nim_if         ------------------------------
*
* NAME: s2nim_if
*
* FUNCTION:
*		converts a "s"tanza "to" a "nim_if" struct
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
*			stanza				= ptr to string containing a valid nim_if
*			nimif					= ptr to nim_if struct to initialize
*		global:
*
* RETURNS: (int)
*		SUCCESS					= nimif initialized
*		FAILURE					= unable to initialize nimif
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
s2nim_if(	char *stanza, 
				struct nim_if *nimif )

{	int rc=SUCCESS;
	regmatch_t pmatch[ERE_IF_STANZA_NUM];
	regmatch_t hmatch[ERE_HARD_ADDR_NUM];
	char *name = NULL;
	char *host = NULL;
	char *haddr = NULL;
	char *adapter = NULL;
	NIM_OBJECT( net, ninfo )
	int net_type=-1;
	int other_net_type=-1;
	int i;

	VERBOSE4("         s2nim_if: stanza=%s;\n",stanza,NULL,NULL,NULL)

	/* init for later checking */
	ninfo.num = 0;

	/* valid interface stanza looks like this: */
	/*		"<network name> <hostname> <hardware addr> <boot adapter name>" */
	/* where <boot adapter name> is optional */

	/* does <stanza> match this format? */
	if ( (regexec( nimere[IF_STANZA_ERE].reg, stanza, 
						ERE_IF_STANZA_NUM, pmatch, 0) != 0) ||
			(pmatch[1].rm_so < 0) ||
			(pmatch[2].rm_so <= 0) ||
			(pmatch[3].rm_so <= 0) )
		ERROR( ERR_VALUE, stanza, MSG_msg(MSG_NIM_IF_STANZA), NULL )

	/* matches format - separate strings */
	stanza[ pmatch[1].rm_eo ] = NULL_BYTE;
	stanza[ pmatch[2].rm_eo ] = NULL_BYTE;
	stanza[ pmatch[3].rm_eo ] = NULL_BYTE;
	name = stanza;
	host = stanza + pmatch[2].rm_so;
	haddr = stanza + pmatch[3].rm_so;

	/* adapter name specified? */
	if ( pmatch[4].rm_so > 0 )
		/* adding 1 to the offset because the 4th field of the IF_STANZA_ERE */
		/*		includes the blank (" ") which separates field 3 from field 4 */
		adapter = stanza + pmatch[4].rm_so + 1;

	/* valid hardware addr? */
	if ( regexec( nimere[HARD_ADDR_ERE].reg, haddr, 
						ERE_HARD_ADDR_NUM, hmatch, 0) != 0 )
	{	rc = FAILURE;
		errstr( ERR_VALUE, haddr, MSG_msg(MSG_NET_HARD_ADDR), NULL );
	}

	/* valid network name? */
	else if ( get_object( &net, &ninfo, 0, name, ATTR_CLASS_NETWORKS, 0 ) <= 0 )
	{	rc = FAILURE;
		errstr( ERR_VALUE, name, MSG_msg(MSG_NETNAME), NULL );
	}

	/* valid hostname? */
	else if ( verify_hostname( host, &nimif->hostname, &nimif->ip ) == FAILURE )
		rc = FAILURE;

	/* valid adapter info? */
	else 
	{
		if ( adapter == NULL )
		{
			if ( find_attr( net, NULL, NULL, 0, ATTR_OTHER_NET_TYPE ) >= 0 )
			{
				rc = FAILURE;
				errstr( ERR_ADAPTER_REQ, stanza, net->name, NULL );
			}
		}
		else if ( (net_type = a2net_type( adapter )) == -1 )
			rc = FAILURE;
		else
		{
			/* does specific network support the specified adapter type? */
			i = -1;
			while ( find_attr( net, &i, NULL, 0, ATTR_OTHER_NET_TYPE ) >= 0 )
			{
				other_net_type = a2net_type( net->attrs[i].value );
				if ( net_type == other_net_type )
					break;
			}
			if ( (i == -1) && (net_type != net->type->attr) )
			{
				rc = FAILURE;
				errstr( ERR_TYPE_CONFLICT, adapter, net->type->name,net->name,NULL);
			}
		}
	}

	/* if all checks have passed... */
	if ( rc == SUCCESS )
	{	/* initialize nimif struct */

		/* network object name */
		strncpy( nimif->network, name, MAX_NAME_BYTES );

		/* hardware addr */
		strncpy( nimif->hard_addr, haddr, MAX_NET_HARD_ADDR);

		/* adapter name */
		/* use the network's type if adapter not supplied */
		if ( adapter == NULL )
			strncpy( nimif->adapter, net->type->name, MAX_ADAPTER_NAME );
		else
			strncpy( nimif->adapter, adapter, MAX_ADAPTER_NAME );
	}
		
	/* restore original stanza */
	stanza[ pmatch[1].rm_eo ] = ' ';
	stanza[ pmatch[2].rm_eo ] = ' ';
	if ( pmatch[4].rm_so > 0 )
		stanza[ pmatch[3].rm_eo ] = ' ';

	if ( ninfo.num > 0 )
		odm_free_list( net, &ninfo );

	return( rc );

} /* end of s2nim_if */
	
/*---------------------------- verify_if         ------------------------------
 *
 * NAME: verify_if
 *
 * FUNCTION:
 *	verifies the value of <if_stanza> by:
 *	1) checking the format
 *	2) verifying that <net obj name> is a currently defined NIM net object
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *		sets err str via ERROR macro
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *	parameters:
 *		mac_id	   = id of machine using the specified interface
 *		if_seqno   = seqno of the ATTR_IF being checked
 *		if_stanza  = ptr to interface stanza
 *		nimif	   = ptr to nim_if struct; 
 *			 	NULL indicates that no info to be returned
 *	global:
 *
 * RETURNS: (int)
 *		SUCCESS		= stanza is correctly formatted
 *		FAILURE		= invalid stanza
 *
 * OUTPUT:
 *----------------------------------------------------------------------------*/

int
verify_if(	long mac_id,
				int if_seqno,
				char *if_stanza, 
				struct nim_if *nimif )

{	struct nim_if local;
	NIM_ATTR( ifs, ifinfo )
	NIM_OBJECT( net, netinfo )
	char pattern[MAX_TMP];
	regex_t ere;
	regmatch_t pmatch[2];
	int rc;
	int i;
	char *zp = NULL;
	
	unsigned long	net_mask, net_addr, mac_addr;

	VERBOSE4("         verify_if: attr=%s%d; if_stanza=\"%s\"\n",ATTR_IF_T,
				if_seqno,if_stanza,NULL)

	/* 
	 * return nim_if? 
	 */
	if ( nimif == NULL )
		/* use local version */
		nimif = &local;

	/* 
	 * does <if_stanza> match required format? 
	 */
	if ( s2nim_if( if_stanza, nimif ) == FAILURE )
		return( FAILURE );

	/* 
	 * does network object exist? 
	 */
	if (get_object( &net, &netinfo,0,nimif->network,ATTR_CLASS_NETWORKS,0 )<= 0)
		ERROR( ERR_DNE, nimif->network, NULL, NULL )

	/* 
	 * Given the networks netmask, does this hostname really belong 
	 * to this logical network ? 
	 */ 
	if ( (i=find_attr(net, NULL, NULL, 0, ATTR_SNM)) < 0 )
		ERROR(ERR_BAD_OBJECT, ATTR_CLASS_NETWORKS_T, nimif->network, NULL )
	net_mask = inet_addr( net->attrs[i].value );		

	if ( (i=find_attr(net, NULL, NULL, 0, ATTR_NET_ADDR)) < 0 )
		ERROR(ERR_BAD_OBJECT, ATTR_CLASS_NETWORKS_T, nimif->network, NULL )
	net_addr = inet_addr( net->attrs[i].value );		

	mac_addr = inet_addr( nimif->ip );
	mac_addr = mac_addr&net_mask;	
	if ( mac_addr != net_addr )
		ERROR(ERR_WRONG_NETWORK, nimif->ip, net->attrs[i].value, net->name);
	/*
	 * If the hardware address is all zeros then the user has told us 
	 * that they are not using hardware addresses, so skip the next 
	 * check and return success.
	 */ 
	for (zp=nimif->hard_addr;
	      	(zp != NULL) && (*zp != '\0') && (*zp == '0'); zp++);
	if ((zp != nimif->hard_addr) && (*zp == '\0'))
		return( SUCCESS );

	/* 
	 * any existing ATTR_IF with the specified network hardware address? 
	 */
	if ( get_attr( &ifs, &ifinfo, 0, NULL, 0, ATTR_IF ) > 0 ) {
		VERBOSE4("                    looking for hardware addr collisions \n",
					NULL,NULL,NULL,NULL)
		sprintf( pattern, "[^ 	]+ [^ 	]+ %s", nimif->hard_addr );
		if ((rc=regcomp( &ere, pattern, REG_EXTENDED|REG_ICASE|REG_NOSUB ))!=0)
			ERROR( ERR_SYS, regerror(rc, &ere, pattern, MAX_TMP), NULL, NULL )

		for (i=0; i < ifinfo.num; i++) {
			if ( (rc = regexec( &ere, ifs[i].value, 2, pmatch, 0 )) == 0 ) {
				/* 
				 * ATTR_IF already exists with this hardware addr 
				 * check to see if the existing ATTR_IF is the one specified by 
				 * the caller (this will happen for change operations) 
				 */
				if ( (ifs[i].id != mac_id) || (ifs[i].seqno != if_seqno) )
					ERROR(	ERR_HARD_ADDR, nimif->hard_addr, 
								get_name( ifs[i].id ), NULL)
			}
		}

		odm_free_list( ifs, &ifinfo );
	}

	return( SUCCESS );

} /* end of verify_if */
	
/*---------------------------- getPif            ------------------------------
*
* NAME: getPif
*
* FUNCTION:
*		given a machine's name, this function returns its PIF as a nim_if
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
*			name					= machine's name
*		global:
*
* RETURNS: (struct nim_if *)
*		>NULL						= ptr to nim_if struct for <name>
*		FAILURE					= unable to get or convert pif
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

struct nim_if *
getPif(	char *name )

{	long id;
	ODMQUERY
	struct nim_attr pif;
	struct nim_if *nimif;

	VERBOSE4("         getPif: name=%s;\n",name,NULL,NULL,NULL)

	/* get machine's id */
	if ( (id = get_id( name )) <= 0 )
		return( NULL );

	/* get machine's pif */
	sprintf( query, "id=%d and seqno=1 and pdattr=%d", id, ATTR_IF );
	if ( odm_get_first( nim_attr_CLASS, query, &pif ) <= 0 )
	{
		errstr( ERR_ODM, (char *) odmerrno, NULL, NULL );
		return( NULL );
	}

	/* malloc space for the nim_if struct */
	nimif = nim_malloc( sizeof( struct nim_if ) );

	/* convert PIF into a nim_if struct */
	if ( s2nim_if( pif.value, nimif ) == FAILURE )
		return( NULL );

	return( nimif );

} /* end of getPif */
	
/*---------------------------- a2net_type        ------------------------------
*
* NAME: a2net_type
*
* FUNCTION:
*		translates the given network adapter logical name into a network interface
*			type
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
*			adapter				= logical device name of network adapter
*		global:
*
* RETURNS: (int)
*		>=0						= network interface type
*		-1							= invalid adapter name
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
a2net_type(	char *adapter )

{	int net_type=-1;
	regmatch_t lname[ERE_ATTR_NAME_NUM];
	char tmp=NULL_BYTE;
	NIM_PDATTR( pdattr, pdinfo )

	/* does specified string match expected pattern for logical device name? */
	if ( (regexec( nimere[ATTR_NAME_ERE].reg, adapter, 
						ERE_ATTR_NAME_NUM, lname, 0) == 0) && (lname[1].rm_so >= 0) )
	{
		/* ignore seqno if present */
		if ( adapter[ lname[1].rm_eo ] != NULL_BYTE )
		{
			tmp = adapter[ lname[1].rm_eo ];
			adapter[ lname[1].rm_eo ] = NULL_BYTE;
		}

		/* is specified network adapter type supported by NIM? */
		if ( get_pdattr( &pdattr, &pdinfo, ATTR_CLASS_NETWORKS, 0, 0, 0,
								adapter ) > 0 )
		{
			net_type = pdattr->attr;
			odm_free_list( pdattr, &pdinfo );
		}

		/* put seqno back? */
		if ( tmp != NULL_BYTE )
			adapter[ lname[1].rm_eo ] = tmp;
	}

	if ( net_type == -1 )
		errstr( ERR_INVALID_DEVICE, adapter, NULL, NULL );

	return( net_type );

} /* end of a2net_type */

/*---------------------------- s2nim_route      -----------------------------
*
* NAME: s2nim_route
*
* FUNCTION:
*		converts the value of an ATTR_ROUTING attr into a nim_route struct
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
*			stanza				= ptr to value of ATTR_ROUTING attr
*			route_struct		= ptr to nim_route struct
*		global:
*
* RETURNS: (int)
*		SUCCESS					= valid ATTR_ROUTING stanza
*		FAILURE					= unable to convert
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
s2nim_route (	char *stanza,
					struct nim_route *route_struct )

{	int rc=SUCCESS;
	regmatch_t format[ERE_TWO_FIELDS_NUM];
	regmatch_t ip_addr[ERE_IP_ADDR_NUM];
	char *target_gw;
	char *dest_gw;
	char tmp[MAX_IP_ADDR];
	struct nim_object *dest_obj;
	struct listinfo info;
	int i,j;
	struct nim_route tmp_route;

	VERBOSE4("         s2nim_route: stanza=%s;\n",stanza,NULL,NULL,NULL)

	/* nim_route struct provided? */
	if ( route_struct == NULL )
		route_struct = &tmp_route;

	/* valid routing stanza looks like this: */
	/*		<target net name> <gateway to target> */
	/* does this one match this format? */
	if ( (regexec( nimere[TWO_FIELDS_ERE].reg, stanza, 
						ERE_TWO_FIELDS_NUM, format, 0) != 0) ||
			(format[1].rm_so < 0) ||
			(format[2].rm_so < 0) )
		ERROR( ERR_VALUE, stanza, MSG_msg(MSG_NIM_ROUTING_STANZA), NULL)

	/* matches format - check the name - must be a current NIM network object */
	stanza[ format[1].rm_eo ] = NULL_BYTE;
	if ( get_object( &dest_obj, &info, 0, stanza, 0, 0 ) <= 0 )
	{	errstr( ERR_VALUE, stanza, MSG_msg(MSG_NETNAME), NULL );
		stanza[ format[1].rm_eo ] = ' ';
		return( FAILURE );
	}
	stanza[ format[1].rm_eo ] = ' ';

	/* check target gateway (2nd field) */
	/* allow either IP address or resolvable hostname */
	target_gw = stanza + format[2].rm_so;
	if (	(regexec(	nimere[IP_ADDR_ERE].reg, target_gw, 
							ERE_IP_ADDR_NUM, ip_addr, 0) == 0) ||
			(rc = verify_hostname( target_gw, NULL, &target_gw )) == SUCCESS )
	{	
		/* fill out a nim_route struct */
		/* find dest net addr */
		if ( (i = find_attr( dest_obj, NULL, NULL, 0, ATTR_NET_ADDR )) < 0 )
		{
			rc = FAILURE;
			errstr( ERR_OBJ_MISSING_ATTR, ATTR_NET_ADDR_T, dest_obj->name, NULL );
		}
		else if ( (j = find_attr( dest_obj, NULL, NULL, 0, ATTR_SNM )) < 0 )
		{
			rc = FAILURE;
			errstr( ERR_OBJ_MISSING_ATTR, ATTR_SNM_T, dest_obj->name, NULL );
		}
		else
		{
			strncpy( route_struct->dest, dest_obj->attrs[i].value, MAX_IP_ADDR );
			strncpy( route_struct->snm, dest_obj->attrs[j].value, MAX_IP_ADDR );
			strncpy( route_struct->gateway, target_gw, MAX_IP_ADDR );
		}
	}

	odm_free_list( dest_obj, &info );

	return( rc );

} /* end of s2nim_route */
	
/*---------------------------- ck_routing_ass      -----------------------------
*
* NAME: ck_routing_ass
*
* FUNCTION:
*		verifies the value of an ATTR_ROUTING assignment by:
*			1) checking the format, which must be:
*				<target net name> <gateway to target> <target's gatway back>
*			2) verifying that <net obj name> is a currently defined NIM net object
*		returns this info if <rass> is > NULL
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*		sets errstr on failure
*		NOTE that the format of an ATTR_ROUTING assignment is different from the 
*			actual value which will be added to the database
*		this occurs because 2 attrs are actually added for each ATTR_ROUTING ass:
*			one for the target and one for the destination net
*		look in m_mknet for more details
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*			routing				= ptr to NIM routing stanza
*			rass					= if >NULL, ptr to routing_ass struct
*		global:
*
* RETURNS: (int)
*		SUCCESS					= valid ATTR_ROUTING stanza
*		FAILURE					= unable to convert
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
ck_routing_ass (	char *routing,
						struct routing_ass *rass )

{	int rc=SUCCESS;
	regmatch_t format[ERE_THREE_FIELDS_NUM];
	regmatch_t ip_addr[ERE_IP_ADDR_NUM];
	struct nim_route nr;
	char *dest;
	char *gateway;
	char *dest_gw;
	char tmp[MAX_IP_ADDR];

	VERBOSE4("         ck_routing_ass: routing=%s\n",routing,NULL,NULL,NULL)

	/* valid routing stanza looks like this: */
	/*		<target net name> <gateway to target> <target's gatway back> */
	/* does <routing> match this format? */
	if ( (regexec( nimere[THREE_FIELDS_ERE].reg, routing, 
						ERE_THREE_FIELDS_NUM, format, 0) != 0) ||
			(format[1].rm_so < 0) ||
			(format[2].rm_so < 0) ||
			(format[3].rm_so < 0) )
		ERROR( ERR_VALUE, routing, MSG_msg(MSG_NIM_ROUTING_STANZA), NULL)

	/* validate the first 2 fields */
	routing[ format[2].rm_eo ] = NULL_BYTE;
	dest = routing;
	if ( s2nim_route( dest, &nr ) == SUCCESS )
	{	
		/* now validate the third field, which is the dest net's gateway */
		/* allow either IP address or resolvable hostname */
		dest_gw = routing + format[3].rm_so;
		if (	(regexec(	nimere[IP_ADDR_ERE].reg, dest_gw, 
								ERE_IP_ADDR_NUM, ip_addr, 0) == 0) ||
				(verify_hostname( dest_gw, NULL, NULL ) == SUCCESS) )
		{	
			/* convert to routing_ass struct? */
			if ( rass != NULL )
			{	routing[ format[1].rm_eo ] = NULL_BYTE;
				gateway = routing + format[2].rm_so;

				rass->dest = nim_malloc( strlen( dest ) + 1 );
				rass->gateway = nim_malloc( strlen( gateway ) + 1 );
				rass->dest_gateway = nim_malloc( strlen( dest_gw ) + 1 );

				strcpy( rass->dest, routing );
				strcpy( rass->gateway, gateway );
				strcpy( rass->dest_gateway, dest_gw );
			}
		}
		else
			rc = FAILURE;
	}
	else
		rc = FAILURE;

	routing[ format[1].rm_eo ] = ' ';
	routing[ format[2].rm_eo ] = ' ';

	return( rc );

} /* end of ck_routing_ass */

/*---------------------------- LIST_ifs          ------------------------------
*
* NAME: LIST_ifs
*
* FUNCTION:
*		generates a LIST of nim_if structs for the specified machine
*		the machine may be specified by either its <id> or <name>
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
*			id						= id of machine
*			name					= name of machine
*			list					= ptr to uninitialized LIST struct
*		global:
*
* RETURNS: (int)
*		SUCCESS					= LIST constructed
*		FAILURE					= no elements added to LIST
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
LIST_ifs(	long id,
				char *name,
				LIST *list )

{	NIM_ATTR( ifs, ifinfo )
	struct nim_if *nimif;
	int i;

	VERBOSE4("         LIST_ifs: id=%d; name=%s;\n",id,name,NULL,NULL)

	list->num = -1;

	/* id given? */
	if ( (id <= 0) && ((id = get_id(name)) == 0) )
		return( FAILURE );

	/* has the list been initialized yet? */
	if ( (list->num == -1) && (get_list_space( list, 5, TRUE ) == FAILURE) )
		return( FAILURE );

	/* get all network interface info */
	get_attr( &ifs, &ifinfo, id, NULL, 0, ATTR_IF );

	/* for each interface */
	for (i=0; i < ifinfo.num; i++)
	{	
		/* malloc space for a new nim_if struct */
		nimif = (struct nim_if *) nim_malloc( sizeof( struct nim_if ) );

		/* convert the attr value to a nim_if */
		s2nim_if( ifs[i].value, nimif );

		/* add it to the LIST */
		if ( get_list_space( list, 1, FALSE ) == FAILURE )
			return( FAILURE );
		list->list[ list->num ] = (char *) nimif;
		list->num = list->num + 1;
	}

	/* free the objects */
	odm_free_list( ifs, &ifinfo );

	return( SUCCESS );

} /* end of LIST_ifs */
	
/*---------------------------- LIST_routes       ------------------------------
*
* NAME: LIST_routes
*
* FUNCTION:
*		constructs a LIST of nim_route structs for the specified network
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*		sets errstr on failure
*		this function ASSUMES that <net> has been retrieved using lag_object
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*			net					= ptr to nim_object for desired network
*			list					= ptr to LIST struct
*		global:
*
* RETURNS: (int)
*		SUCCESS					= LIST constructed
*		FAILURE					= ODM err?
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
LIST_routes(	struct nim_object *net,
					LIST *list )

{	int i;

	VERBOSE4("         LIST_routes: net=%s;\n",net,NULL,NULL,NULL)

	/* allocate space for the LIST */
	if ( get_list_space( list, DEFAULT_CHUNK_SIZE, TRUE ) == FAILURE )
		return( FAILURE );

	/* for each route... */
	i = -1;
	while ( find_attr( net, &i, NULL, 0, ATTR_ROUTING ) >= 0 )
	{	/* get LIST space */
		if ( get_list_space( list, 1, FALSE ) == FAILURE )
			return( FAILURE );

		/* allocate space for another nim_route */
		list->list[ list->num ] = nim_malloc( sizeof(struct nim_route) );

		/* initialize it */
		if ( s2nim_route( net->attrs[i].value, list->list[ list->num ] )==FAILURE)
			return( FAILURE );

		list->num = list->num + 1;
	}

	return( SUCCESS );

} /* end of LIST_routes */

/*---------------------------- write_nim_routes    -----------------------------
*
* NAME: write_nim_routes
*
* FUNCTION:
*		initializes the NIM_ROUTES .info variable for the specified machine
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
*			mac					= target's nim_object
*			fp						= FILE ptr to .info file
*		global:
*
* RETURNS: (int)
*		SUCCESS					= NIM_ROUTES added
*		FAILURE					= unable to add NIM_ROUTES
*
* OUTPUT:
*		writes the NIM_ROUTES assignment to <fp>
*-----------------------------------------------------------------------------*/

int
write_nim_routes(	struct nim_object *mac,
						FILE *fp )

{	int i,j,k;
	struct nim_if nimif;
	NIM_OBJECT( net, ninfo )
	LIST routes;
	struct nim_route *route;

	VERBOSE4("         write_nim_routes: name=%s;\n",mac->name,NULL,NULL,NULL)

	/* convert PIF into nim_if struct */
	if ( (i = find_attr( mac, NULL, NULL, 1, ATTR_IF )) < 0 )
		ERROR( ERR_OBJ_MISSING_ATTR, PIF, mac->name, NULL )
	if ( s2nim_if( mac->attrs[i].value, &nimif ) == FAILURE )
		return( FAILURE );

	/* get the target's primary network object */
	if ( lag_object( 0, nimif.network, &net, &ninfo ) == FAILURE )
		return( FAILURE );
	unlock_object( net->id, NULL, FALSE );

	/* generate LIST of nim_route structs */
	if ( LIST_routes( net, &routes ) == FAILURE )
		return( FAILURE ); 

	/* add each route to the NIM_ROUTES variable */
	/* format of each stanza is: */
	/*		<dest net IP>:<dest net snm>:<gateway IP> */
	if ( routes.num > 0 )
	{
		fprintf( fp, "%s=\"", NIM_ROUTES );
		for (i=0; i < routes.num; i++)
		{
			route = (struct nim_route *) routes.list[i];

			/* make sure that this route doesn't use one of the target's */
			/*		interfaces as the gateway; if it does, then the target is */
			/*		acting as the gateway between NIM networks and we don't */
			/*		want to add the route (because we would be adding a route */
			/*		essentially through the target itself) */
			j = -1;
			while ( (k = find_attr( mac, &j, NULL, 0, ATTR_IF )) >= 0 )
			{
				/* convert to nim_if struct */
				if ( s2nim_if( mac->attrs[k].value, &nimif ) == FAILURE )
					return( FAILURE );

				/* is target's interface used as the gateway for this route? */
				if ( strcmp( nimif.ip, route->gateway ) == 0 )
					/* yes, target being used as gateway; skip this route */
					break;
			}

			/* add this route to the list if the target's interface is not the */
			/*		gateway for this route */
			if ( k < 0 )
				fprintf( fp, " %s:%s:%s ", route->dest, route->snm, route->gateway);
		}
		fprintf( fp, "\"\n" );
	}

	/* release network obj */
	odm_free_list( net, &ninfo );

	return(SUCCESS);

} /* end of write_nim_routes */

/*$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
$$$$$$$$$$$$$$$$ network routing manipulation                      $$$$$$$$$$$$
$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$*/

/*---------------------------- mk_net_route   ------------------------------
*
* NAME: mk_net_route
*
* FUNCTION:
*		adds a route between two NIM network objects
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*		sets errstr on failure
*		it is ASSUMED that the caller has locked the target net
*
* RECOVERY OPERATION:
*		it is ASSUMED that the caller implements recovery for the target net; this
*			function will backup the dest net
*
* DATA STRUCTURES:
*		parameters:
*			id						= id of target network
*			target				= name of target network
*			stanza				= NIM routing stanza
*			seqno					= seqno of ROUTING attr
*			backups				= ptr to OBJ_BACKUP LIST
*		global:
*
* RETURNS: (int)
*		SUCCESS					= NIM route added between target & dest net
*		FAILURE					= unable to add routing attrs
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
mk_net_route(	long id,
					char *target,
					char *stanza,
					int seqno,
					LIST *backups )

{	struct routing_ass rass;
	char tmp[MAX_TMP];

	VERBOSE4("         mk_net_route: id=%s; target=%s; stanza=%s;\n",id,target,
				stanza,NULL)

	/* convert <stanza> to routing_ass struct */
	if ( ck_routing_ass( stanza, &rass ) == FAILURE )
		return( FAILURE );

	/* make sure that the specified destination network is NOT the target */
	/*		network (ie, doesn't make sense to add a route to yourself!) */
	if ( strcmp( target, rass.dest ) == 0 )
	{
		if ( seqno > 0 )
			sprintf( tmp, "%s%d", ATTR_ROUTING_T, seqno );
		else
			sprintf( tmp, "%s", ATTR_ROUTING_T );
		ERROR( ERR_ROUTE_TO_SELF, target, tmp, NULL )
	}

	/* each ATTR_ROUTING attr references the destination network and the */
	/*		gateway to use to get there */
	/* however, we CANNOT allow more than one NIM route to exist between any */
	/*		2 networks because that messes us up */
	/* so, check now to ensure there isn't a route between these nets already */
	sprintf( tmp, "%s *", rass.dest );
	if ( get_attr( NULL, NULL, id, tmp, 0, ATTR_ROUTING ) > 0 )
		ERROR( ERR_ROUTE_ALREADY, target, rass.dest, NULL )

	/* NIM routing between networks involves adding an ATTR_ROUTING attr for */
	/*		both the target & dest networks */
	/* it is ASSUMED that the target has already been backed up, so backup the */
	/*		dest now */
	if ( backup_object( backups, 0, rass.dest, NULL, NULL ) == FAILURE )
		return( FAILURE );

	/* initialize routing stanza for target & add attr */
	sprintf( tmp, "%s %s", rass.dest, rass.gateway );
	if ( mk_attr( id, NULL, tmp, seqno, ATTR_ROUTING, ATTR_ROUTING_T )==FAILURE)
		return( FAILURE );

	/* initialize routing stanza for <net2> & add attr */
	sprintf( tmp, "%s %s", target, rass.dest_gateway );
	if ( mk_attr( 0, rass.dest, tmp, 0, ATTR_ROUTING, ATTR_ROUTING_T )== FAILURE)
		return( FAILURE );

	/* successful if we get here */
	return( SUCCESS );

} /* end of mk_net_route */
	
/*---------------------------- ch_net_route      ------------------------------
*
* NAME: ch_net_route
*
* FUNCTION:
*		changes the gateway field of the specified NIM routing attribute
*		NOTE that this is the only field which may be changed for an ATTR_ROUTING
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*		sets errstr on failure
*		it is ASSUMED that the caller has locked both network objects
*
* RECOVERY OPERATION:
*		it is ASSUMED that the caller implements recovery
*
* DATA STRUCTURES:
*		parameters:
*			old					= ptr to nim_attr which is the old routing value
*			new_stanza			= new routing attr value
*		global:
*
* RETURNS: (int)
*		SUCCESS					= attr changed
*		FAILURE					= error
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
ch_net_route(	struct nim_attr *old,
					char *new_value )

{	struct nim_route new_route;
	char *new_dest;
	char *old_dest;
	char tmp[MAX_TMP];

	VERBOSE4("         ch_net_route: old=%s; new_value=%s;\n",old->value,
				new_value,NULL,NULL)

	/* verify the info in <new_value> */
	if ( s2nim_route( new_value, &new_route ) == FAILURE )
		return( FAILURE );

	/* get destination network specified in new value */
	if ( two_fields( new_value, &new_dest, NULL ) == FAILURE )
     	ERROR( ERR_VALUE, new_value, MSG_msg(MSG_NIM_ROUTING_STANZA), NULL)

	/* get destination network specified in old value */
	if ( two_fields( old->value, &old_dest, NULL ) == FAILURE )
     	ERROR( ERR_VALUE, old->value, MSG_msg(MSG_NIM_ROUTING_STANZA), NULL)

	/* NIM does not allow the user to change the destination network - only */
	/*		the gateway field */
	/* so, destination network specified in old and new values must match */
	/* is the dest net the same in the old and new values? */
	if ( strcmp( new_dest, old_dest ) != 0 )
	{	if ( old->seqno )
			sprintf( tmp, "%s%d", ATTR_ROUTING_T, old->seqno );
		else
			sprintf( tmp, "%s", ATTR_ROUTING_T );
		ERROR( ERR_DEST_MATCH, tmp, NULL, NULL );
	}

	/* change the attr value */
	return( ch_attr(	old->id, NULL, new_value, old->seqno, 
							ATTR_ROUTING, ATTR_ROUTING_T ) );

} /* end of ch_net_route */
	
/*---------------------------- rm_net_route    ------------------------------
*
* NAME: rm_net_route
*
* FUNCTION:
*		removes a NIM route between two NIM networks
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*		sets errstr on failure
*		it is ASSUMED that the caller has locked the target network
*
* RECOVERY OPERATION:
*		it is ASSUMED that the caller has backed up the target; this function
*			will backup the dest
*
* DATA STRUCTURES:
*		parameters:
*			target				= name of target network
*			rattr					= ptr to nim_attr which is the ATTR_ROUTING to remove
*			backups				= ptr to OBJ_BACKUP LIST
*		global:
*
* RETURNS: (int)
*		SUCCESS					= routing removed
*		FAILURE					= unable to remove one or both routes
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
rm_net_route(	char *target,
					struct nim_attr *rattr,
					LIST *backups )

{	char *dest;
	struct nim_object *dobj;
	struct listinfo *dinfo;
	ODMQUERY

	VERBOSE4("         rm_net_route: target=%s; rattr=%s;\n",target,rattr->value,
				NULL,NULL)

	/* separate current stanza so we can get to dest net name */
	if ( two_fields( rattr->value, &dest, NULL ) == FAILURE )
     	ERROR( ERR_VALUE, rattr->value, MSG_msg(MSG_NIM_ROUTING_STANZA), NULL)

	/* it is ASSUMED that the target has already been backed up, so backup the */
	/*		dest now */
	if ( backup_object( backups, 0, dest, &dobj, &dinfo ) == FAILURE )
		return( FAILURE );

	/* remove the attr for the owner of the attr */
	if ( rm_attr( rattr->id, NULL, ATTR_ROUTING, rattr->seqno, NULL ) == FAILURE)
		return( FAILURE );

	/* construct the query to remove the ATTR_ROUTING for dest net */
	sprintf( query, "%s *", target );

	/* remove routing for dest net */
	if ( rm_attr( dobj->id, NULL, ATTR_ROUTING, 0, query ) == FAILURE )
		return( FAILURE );

	/* successful removal if we get here */
	return( SUCCESS );

} /* end of rm_net_route */

/*$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
$$$$$$$$$$$$$$$$ connectivity algorithms                           $$$$$$$$$$$$
$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$*/
	
/*---------------------------- net_to_net        ------------------------------
*
* NAME: net_to_net
*
* FUNCTION:
*		determines if <net1> has route to <net2>
*		optionally, the route from <net1> to <net2> may be returned
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*			net1					= one network object
*			net2					= another network object
*			nimroute				= ptr to nim_route struct if info is to be returned
*		global:
*
* RETURNS: (int)
*		SUCCESS					= nets can communicate
*		FAILURE					= no mutual routing
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
net_to_net (	char *net1,
					char *net2,
					struct nim_route *nimroute )

{	int rc=SUCCESS;
	NIM_OBJECT( obj, info )
	ODMQUERY
	int i;

	VERBOSE4("         net_to_net: net1=%s; net2=%s;\n",net1,net2,NULL,NULL)

	/* get <net1>'s info */
	if ( get_object( &obj, &info, 0, net1, 0, 0 ) <= 0 )
		/* no object - internal error? */
		return( FAILURE );

	/* looking for ATTR_ROUTING which references <net2> */
	sprintf( query, "^%s *", net2 );
	if ( (i = find_attr( obj, NULL, query, 0, ATTR_ROUTING)) < 0 )
		rc = FAILURE;
	else if ( nimroute != NULL )
		rc = s2nim_route( obj->attrs[i].value, nimroute );

	odm_free_list( obj, &info );

	return( rc );

} /* end of net_to_net */

/*---------------------------- net_to_mac   ------------------------------
*
* NAME: net_to_mac
*
* FUNCTION:
*		determines if there's sufficient routing for any machine on <net> to
*			communicate with <mac>
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*			net					= name of network object
*			mac					= name of machine object
*			nimif					= ptr to nim_if struct if caller wants info back
*		global:
*
* RETURNS: (int)
*		SUCCESS					= <net> can communicate with <mac>
*		FAILURE					= no connection -OR- not both ways
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
net_to_mac (	char *net,
					char *mac,
					struct nim_if *nimif )

{	int rc=SUCCESS;
	ODMQUERY
	NIM_OBJECT( server, sinfo )
	char *ptr;
	int i;
	regmatch_t pmatch[ERE_FIRST_FIELD_NUM];
	int attr;
	struct nim_if local_nif;
	struct nim_route nimroute;
	int server_is_gateway = FALSE;

	VERBOSE4("         net_to_mac: net=%s; mac=%s;\n",net,mac,NULL,NULL)

	/* use local struct if caller doesn't want info back */
	if ( nimif == NULL )
		nimif = &local_nif;

	/* first, check to see if <mac> is directly connected to <net> */
	if ( get_object( &server, &sinfo, 0, mac, 0, 0 ) <= 0 )
		return( FAILURE );
	sprintf( query, "^%s *", net );
	if ( (i = find_attr( server, NULL, query, 0, ATTR_IF )) >= 0 )
	{
		/* client is on same network as server - use this server interface */
		rc = s2nim_if( server->attrs[i].value, nimif );
	}
	else
	{	
		/* server not on same network as client - check all server interfaces */
		i = -1;
		rc = FAILURE;
		while ( (i = find_attr( server, &i, NULL, 0, ATTR_IF )) >= 0 )
		{		
			/* separate <net name> from other info */
			if ( (rc = s2nim_if( server->attrs[i].value, nimif )) == SUCCESS )
			{	
				/* is there a NIM route between these 2 networks? */
				if ( (rc = net_to_net( nimif->network, net, &nimroute )) == SUCCESS)
				{
					/* is IP address of server's interface the same as the gateway */
					/*		which would be used by the client? */
					if ( strcmp( nimif->ip, nimroute.gateway ) == 0 )
					{
						/* this interface for the server should NOT be used by the */
						/*		client because it may lead to network boot failures */
						/* flag that we've encountered this condition in case we */
						/*		don't find any other interfaces to use */
						server_is_gateway = TRUE;
						rc = FAILURE;
					}
					else
						/* everything is OK - the client may use this interface of */
						/*		the server without any problems (assuming, of course, */
						/*		that the physical network routing is configured */
						/*		correctly) */
						break;
				}
			}
			else
				/* unable to convert ATTR_IF into valid nim_if struct */
				break;

		} /* while */
	}

	odm_free_list( server, &sinfo );

	if ( rc == FAILURE )
	{
		/* then NIM doesn't allow client to access this server */
		if ( server_is_gateway )
			ERROR( ERR_SERVER_IS_GATEWAY, mac, nimif->network, net )
		else
			ERROR( ERR_NO_ROUTE, net, mac, NULL )
	}

	return( SUCCESS );

} /* end of net_to_mac */
	
/*$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
$$$$$$$$$$$$$$$$ network state manipulation                       $$$$$$$$$$$$
$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$*/
	
/*---------------------------- net_is_active     ------------------------------
*
* NAME: net_is_active
*
* FUNCTION:
*		checks machine activity on the specified network (may be specified by
*			either its <id> or <name>)
*		a network will be considered "active" if:
*			1) there are any machines with a Cstate other than "ready" or
*					"incomplete"
*			2) there are any machines which have been allocated a resource
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*		Any objects which are locked by this function will NOT be released upon
*			exit.  This is done intentionally so that the caller can retain
*			control over those objects until the desired change has been completed.
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*			id						= id of network to check
*			name					= name of network to check
*		global:
*
* RETURNS: (int)
*		TRUE						= a machine attached to the specified net is "active"
*		FALSE						= no "active" machines here
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
net_is_active(	long id,
					char *name )

{	int rc=FALSE;
	NIM_ATTR( netref, refinfo )
	NIM_OBJECT( mac, minfo )
	ODMQUERY
	int i,j;
	int cstate;
	long	master_id; 

	VERBOSE4("         net_is_active: id=%d; name=%s;\n",id,name,NULL,NULL)

	/* lock the network object */
	if ( lock_object( id, name ) == FAILURE )
		return( TRUE );

	/* if name not given, get it */
	if ( name == NULL )
		name = get_name( id );

	/* get the masters id */
	if ( (master_id = get_id( ATTR_MASTER_T )) == 0 )
		ERROR(ERR_DNE, ATTR_MASTER_T, NULL, NULL)

	/* now, get all the references to this network */
	sprintf( query, "%s *", name );
	if ( get_attr( &netref, &refinfo, 0, query, 0, ATTR_IF ) > 0 )
	{	/* check each reference */
		for (i=0; i < refinfo.num; i++)
		{	
			/* skip master interfaces */
			if ( netref[i].id == master_id )
				continue;

			/* lock this object to prevent anything else from happening */
			/*		before we're done */
			if ( lag_object( netref[i].id, NULL, &mac, &minfo ) == SUCCESS )
			{	
				/* loop through this machine's attrs */
				for (j=0; j < mac->attrs_info->num; j++)
				{	/* looking for the Cstate & any allocated resource */
					if ( mac->attrs[j].pdattr->attr == ATTR_CSTATE )
					{	/* if Cstate is anything other than "ready" or "incomplete" */
						/*		we'll consider it "active" */
						cstate = (int) strtol( mac->attrs[j].value, NULL, 0 );
						if ((cstate != STATE_CREADY) && (cstate != STATE_INCOMPLETE))
						{	
							errstr( ERR_CHNET, mac->name, NULL, NULL );
							rc = TRUE;
							break;
						}
					}
					else if ( mac->attrs[j].pdattr->class == ATTR_CLASS_RESOURCES )
					{	
						/* resource has been allocated to this machine */
						errstr( ERR_CHNET, mac->name, NULL, NULL );
						rc = TRUE;
						break;
					}
				}
			}

			/* free the memory from odm_get_list, but leave the object LOCKED! */
			/* the caller needs to effect the desired change BEFORE this object */
			/*		can be released */
			odm_free_list( mac, &minfo );

			if ( rc == TRUE )
				break;
		}
	}

	odm_free_list( netref, &refinfo );

	return( rc );

} /* end of net_is_active */

/*---------------------------- ok_to_chnet       ------------------------------
*
* NAME: ok_to_chnet
*
* FUNCTION:
*		determines if a network's Nstate or routing may be changed
*		this will be allowed when:
*			1) the specified network is NOT active -AND-
*			2) any network connected to the specified network is NOT active
*		if either condition if FALSE, then the change cannot be allowed
*		the network may be specified with either its <id> or <name>
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*		sets errstr on failure
*		Any objects which are locked by this function will NOT be released upon
*			exit.  This is done intentionally so that the caller can retain
*			control over those objects until the desired change has been completed.
*
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*			id						= id of network
*			name					= name of network
*		global:
*
* RETURNS: (int)
*		SUCCESS					= network object may be changed without screwing up
*											any active NIM operations
*		FAILURE					= a machine on this network is busy doing some NIM
*											operation: it's either installing, booting, or
*											has an allocated resource
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
ok_to_chnet(	long id,
					char *name )

{	NIM_OBJECT( net, ninfo )
	NIM_ATTR( netref, refinfo )
	ODMQUERY
	int i;
	char *target;

	VERBOSE4("         ok_to_chnet: id=%d; name=%s;\n",id,name,NULL,NULL)

	/* make sure we've got the network's name for message purposes */
	if ( name == NULL )
		target = get_name( id );
	else
		target = name;

	/* first, check activity on this network */
	if ( net_is_active( id, name ) )
		return( FAILURE );

	/* now, get all the references to this network */
	/* this will include ATTR_IF and ATTR_ROUTING references */
	sprintf( query, "%s *", target );
	if ( get_attr( &netref, &refinfo, 0, query, 0, 0 ) > 0 )
	{	for (i=0; i < refinfo.num; i++)
		{	/* skip any references that aren't ATTR_ROUTING attrs */
			if ( netref[i].pdattr->attr != ATTR_ROUTING )
				continue;

			/* check activity on the connected network */
			if ( net_is_active( netref[i].id, NULL ) )
				return( FAILURE );
		}
	}

	/* if we get here, there is no NIM activity which would conflict with */
	/*		changing info about the specified network -AND- all of the effected */
	/*		objects have been locked in preparation for that change */
	return( SUCCESS );

} /* end of ok_to_chnet */

/*---------------------------- set_nstate     ------------------------------
*
* NAME: set_nstate
*
* FUNCTION:
*		sets the ATTR_NSTATE for a network
*		the criteria for a network being "ready" is that it have a route to the
*			 master
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*		ASSUMES that network object is already locked
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*			net					= name of network
*		global:
*
* RETURNS: (int)
*		SUCCESS					= network is "ready"
*		FAILURE					= no path to master
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
set_nstate(	char *net )

{	int rc;
	NIM_ATTR( macifs, ifinfo )
	ODMQUERY
	int i;
	long	master_id; 

	VERBOSE4("        set_nstate: net=%s;\n",net,NULL,NULL,NULL)

	/* first, delete in ATTR_MISSING that were left from previous attempts */
	rm_attr( 0, net, ATTR_MISSING, 0, NULL );
	
	/* get the masters id */
	if ( (master_id = get_id( ATTR_MASTER_T)) == 0 )
		ERROR( ERR_DNE, ATTR_MASTER_T, NULL, NULL )

	/* does <net> have route to master? */
	/* ignore FAILURE when error is returned because the master is functioning */
	/*		as a gateway */
	if (	((rc = net_to_mac( net, ATTR_MASTER_T, NULL )) == SUCCESS) ||
			(niminfo.errno == ERR_SERVER_IS_GATEWAY) )
	{
		set_state( 0, net, ATTR_NSTATE, STATE_NREADY );
		if ( niminfo.errno == ERR_SERVER_IS_GATEWAY )
			rc = SUCCESS;
	}
	else
	{	/* missing route to master */
		mk_attr( 0, net, MSG_msg(MSG_NO_ROUTE_TO_MASTER), 0, ATTR_MISSING, NULL );
		set_state( 0, net, ATTR_NSTATE, STATE_INCOMPLETE );
	}

	return( rc );

} /* end of set_nstate */
	
/*---------------------------- set_all_nstates       ---------------------------
*
* NAME: set_all_nstates
*
* FUNCTION:
*		sets the ATTR_NSTATE attr for all networks
*		this algorithm is run anytime an interface is changed for the "master"
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
*		global:
*
* RETURNS: (int)
*		SUCCESS					= all network states set
*		FAILURE					= ?
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
set_all_nstates()

{	ODMQUERY
	NIM_OBJECT( nets, ninfo )
	int i;

	VERBOSE3("         set_all_nstates\n",NULL,NULL,NULL,NULL)

	/* lock the MASTER's object if it's not already */
	if ( lock_object( 0, ATTR_MASTER_T ) == FAILURE )
		return( FAILURE );

	/* get all the network objects */
	/* not using get_object here because we don't want a depth of 3 */
	sprintf( query, "class=%d", ATTR_CLASS_NETWORKS );
	if ( (nets = (struct nim_object *)
						odm_get_list( nim_object_CLASS, query, &ninfo, 10, 1 )) == -1)
		ERROR( ERR_ODM, (char *) odmerrno, NULL, NULL )

	/* lock all the network objects */
	for (i=0; i < ninfo.num; i++)
		if ( lock_object( nets[i].id, NULL ) == FAILURE )
			return( FAILURE );

	/* now, set all the ATTR_NSTATEs */
	for (i=0; i < ninfo.num; i++)
		set_nstate( nets[i].name );

	odm_free_list( nets, &ninfo );

	return( SUCCESS );

} /* end of set_all_nstates */

