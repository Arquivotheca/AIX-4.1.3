
static char	sccs_id[] = " @(#)80 1.10  src/bos/usr/lib/nim/methods/m_chnet.c, cmdnim, bos41J, 9513A_all  3/27/95  09:59:25";

/*
 *   COMPONENT_NAME: CMDNIM
 *
 *   FUNCTIONS: ./usr/lib/nim/methods/m_chnet.c
 *		
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "cmdnim_mstr.h"

extern int	valid_pdattr_ass();
extern int	ch_pdattr_ass();

/*---------------------------- module globals    -----------------------------*/
ATTR_ASS_LIST attr_ass;

VALID_ATTR valid_attrs[] = 
{
	{ ATTR_NET_ADDR, 	ATTR_NET_ADDR_T,FALSE, valid_pdattr_ass },
	{ ATTR_SNM, 		ATTR_SNM_T, 	FALSE, valid_pdattr_ass },
	{ ATTR_ROUTING, 	ATTR_ROUTING_T, FALSE, ch_pdattr_ass },
	{ ATTR_OTHER_NET_TYPE,  ATTR_OTHER_NET_TYPE_T, FALSE, ch_pdattr_ass },
	{ ATTR_COMMENTS, 	ATTR_COMMENTS_T,FALSE, ch_pdattr_ass },
	{ ATTR_FORCE, 		ATTR_FORCE_T, 	FALSE, NULL },
	{ ATTR_VERBOSE, 	ATTR_VERBOSE_T, FALSE, NULL },
	{ 0, 			NULL, 		FALSE, NULL }
};


char	*name = NULL;
char	*type = NULL;

/* --------------------------- get_netattrs 
 *
 * NAME: get_netattrs
 *
 * FUNCTION: get any snm / net_addr update values.
 *
 * RETURNS: (int)
 *		SUCCESS	
 *		FAILURE
 *
 * ---------------------------------------------------- 
 */
void	
get_netattrs( struct nim_object *nobj )
{
	int	s, i = -1;
	long	l_addr;
	int	addr_ndx = -1;
	char	*netaddr = NULL;
	char	*snm = NULL;
	long	l_netaddr;
	long	l_snm;

	/* 
	 * get the address and/or netmask.
	 */
	for (i = 0; i < attr_ass.num; i++)
		if ( attr_ass.list[i]->pdattr == ATTR_NET_ADDR ) {
			netaddr = attr_ass.list[i]->value;
			addr_ndx = i;
		}
		else if ( attr_ass.list[i]->pdattr == ATTR_SNM )
			snm = attr_ass.list[i]->value;

	/* 
	 * If we do not have any changes no need to 
	 * continue...
	 */
	if ( (snm == NULL) && (netaddr == NULL) )
		return;

	/* 
	 * Are we changing snm ? -- if not get netobj snm and 
	 * use that for the calcs.
	 */
	if ( snm == NULL )
		if ( (s = find_attr(nobj, NULL, NULL, 0, ATTR_SNM) ) >= 0 )
			snm = nobj->attrs[s].value;
		else
			nim_error(ERR_BAD_OBJECT, ATTR_CLASS_NETWORKS_T, name, NULL);

	/* 
	 * Explicitly changing netaddr ? -- if not get netobj netaddr and 
	 * use that for the calcs.
	 */
	if ( netaddr == NULL )
		if ((addr_ndx=find_attr(nobj,NULL,NULL,0,ATTR_NET_ADDR)) >= 0 )
			netaddr = nobj->attrs[addr_ndx].value;
		else
			nim_error(ERR_BAD_OBJECT, ATTR_CLASS_NETWORKS_T, name, NULL);

	l_addr = inet_addr(netaddr);
	l_snm  = inet_addr(snm);
	l_netaddr = l_addr & l_snm;
	netaddr = inet_ntoa(l_netaddr);

	/* 
	 * check for existing network address object, we dont understand 
	 * two network objects with the same network address. (nimreg 
	 * cann't pick which netobj a machine may belong to..)
	 */
	if ( get_attr( NULL, NULL, 0, netaddr,  0, ATTR_NET_ADDR ) > 0 )
		nim_error(ERR_ONLY_ONE,MSG_msg(MSG_NET_DEFINITION),netaddr,
				NULL );

	/* 
	 * if we got the netaddr from the object then it doesn't exist in the 
	 * command line attr array, so add it. By adding it, the netaddr will 
	 * be updated in ch_attr further down the logic path ! 
	 */
	if ( addr_ndx != -1 ) {
		/* 
		 * Need to add to the cmd line attrs
		 */
		if (add_attr(&attr_ass,ATTR_NET_ADDR,ATTR_NET_ADDR_T,netaddr)!=SUCCESS)
			nim_error(0, NULL, NULL, NULL);
	}
	else {
		attr_ass.list[addr_ndx]->value=nim_malloc( strlen(netaddr)+1 );
		strcpy( attr_ass.list[addr_ndx]->value, netaddr );
	}

	return;
}


/* --------------------------- chroute 
 *
 * NAME: chroute
 *
 * FUNCTION:
 *		changes the specified network routing attribute
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *		sets errstr on failure
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *	parameters:
 *		id		= nim_object.id for target net
 *		name		= nim_object.name for target net
 *		routing_ch	= ptr to ATTR_ASS struct
 *	global:
 *		backups
 *
 * RETURNS: (int)
 *		SUCCESS		= routing changed
 *		FAILURE		= unable to make the change
 *
 * OUTPUT:
 *-----------------------------------------------------------------------------*/

#define ADD			1
#define CHANGE			2
#define REMOVE			3

int
chroute(	long id,
char *name,
ATTR_ASS *routing_ch )

{	
	int	op;
	int	value_specified;
	int	already_exists;
	NIM_ATTR( cur, curinfo )
	char	tmp[MAX_TMP];

	/* initialize flags */
	op = ADD;
	value_specified = ( routing_ch->value[0] != NULL_BYTE );

	/* does this attr already exist? */
	/* if seqno not given, we'll assume that it doesn't (if it does and user */
	/*		just didn't specify the seqno, then we'll catch it in mk_net_route */
	/*		because that function ensures only one route between any 2 nets */
	if (	(routing_ch->seqno == 0) || 
	    (get_attr(&cur, &curinfo, id, NULL, routing_ch->seqno, ATTR_ROUTING) <= 0) )
		already_exists = FALSE;
	else if ( curinfo.num > 0 )
		already_exists = TRUE;

	/* what operation shall we perform: add, change, or remove? */
	if ( already_exists ) {	
		if ( value_specified )
			op = CHANGE;
		else
			op = REMOVE;
	} else if ( !value_specified ) {	/* error - attr doesn't already exist and no value was given */
		if ( routing_ch->seqno ) {	
			sprintf( tmp, "name = %s%d", routing_ch->name, routing_ch->seqno );
			ERROR( ERR_ATTR_NOT_FOUND, tmp, NULL, NULL )
		} else
			ERROR( ERR_MISSING_VALUE, routing_ch->name, NULL, NULL );
	}

	/* perform the operation & return */
	switch (op) {
	case CHANGE	:	
		return( ch_net_route( cur, routing_ch->value ) );
		break;

	case REMOVE	:	
		return( rm_net_route( name, cur, &backups ) );
		break;

	default :	
		return( mk_net_route( id, name, routing_ch->value, 
		    routing_ch->seqno, &backups ) );
		break;
	}

} /* end of chroute */



/*---------------------------- ck_all_clients    ------------------------------
 *
 * NAME: ck_all_clients
 *
 * FUNCTION:
 *		ensures that all currently attached clients have a 4th field in their
 *			ATTR_IF
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
 *			nobj	= ptr to nim_object for network object
 *			ifs	= ptr to nim_attr structs for list of ATTR_IFs
 *			ifinfo	= ptr to listinfo struct for ifs
 *		global:
 *			backups
 *
 * RETURNS: (int)
 *		SUCCESS		= all clients have a 4th field
 *		FAILURE					= unable to update ATTR_IF for a client
 *
 * OUTPUT:
 *-----------------------------------------------------------------------------*/

int
ck_all_clients(	struct nim_object *nobj,
struct nim_attr *ifs,
struct listinfo *ifinfo )

{	
	int	i;
	char	*tmp;

	/* for each ATTR_IF which currently references this net... */
	for (i = 0; i < ifinfo->num; i++) {
		/* does it already have a 4th field? */
		if ( regexec(nimere[FOUR_FIELDS_ERE].reg, ifs[i].value, 0, NULL, 0) != 0 ) {
			/* no fourth field - need to add one */
			/* first, backup the object */
			if ( backup_object( &backups, ifs[i].id, NULL, NULL, NULL ) == FAILURE)
				return( FAILURE );

			/* add a 4th field */
			tmp = nim_malloc( strlen(ifs[i].value) + strlen(nobj->type->name) + 2);
			sprintf( tmp, "%s %s", ifs[i].value, nobj->type->name );
			if ( ch_attr(	ifs[i].id, NULL, tmp, ifs[i].seqno, 
			    ATTR_IF, ATTR_IF_T ) == FAILURE )
				return( FAILURE );
		}
	}

	return( SUCCESS );

} /* end of ck_all_clients */


/* --------------------------- ch_network             
 *
 * NAME: ch_network
 *
 * FUNCTION:
 *		changes network attributes
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *		calls nim_error
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *		parameters:
 *		global:
 *			name
 *			attr_ass
 *
 * RETURNS: (int)
 *		SUCCESS					= attrs changed
 *		FAILURE					= error encountered
 *
 * OUTPUT:
 * ----------------------------------------------------------------------------
 */

int
ch_network()

{	
	int	rc = SUCCESS;
	struct nim_object *nobj;
	struct listinfo *ninfo;
	int	i, j, k;
	int	routing_change = FALSE;
	ODMQUERY
	    NIM_ATTR( ifs, ifinfo )
	int	adding_net_type = FALSE;
	int	net_type;
	int	new_net_type;
	char	tmp[MAX_TMP];
	struct nim_if nimif;

	VERBOSE("m_chnet: net = %s\n", name, NULL, NULL, NULL)

	/* backup all info about current object */
	if ( backup_object( &backups, 0, name, &nobj, &ninfo ) == FAILURE )
		nim_error( 0, NULL, NULL, NULL );

	get_netattrs(nobj);

	/* remove any ATTR_MISSING & ATTR_INFO attrs */
	rm_attr( nobj->id, NULL, ATTR_MISSING, 0, NULL );
	rm_attr( nobj->id, NULL, ATTR_INFO, 0, NULL );

	/* get list of all currently attached clients */
	sprintf( query, "%s *", nobj->name );
	if ( get_attr( &ifs, &ifinfo, 0, query, 0, ATTR_IF ) == -1 )
		nim_error( 0, NULL, NULL, NULL );

	/* look for changes which require special processing */
	for (i = 0; i < attr_ass.num; i++) {
		switch (attr_ass.list[i]->pdattr) {
			/*
			 * check if subnet mask or network adress chnage.
			 * if so and no machines are attached then all is ok...
			 */
		case ATTR_SNM:
		case ATTR_NET_ADDR:
			if (ifinfo.num > 0)
				nim_error(ERR_NET_INUSE, name, NULL, NULL);
			break;

		case ATTR_ROUTING:
			VERBOSE("\trouting change request - checking activity\n", NULL, NULL,
			    NULL, NULL)

			/* change to routing - check to make sure this is ok to do right now */
			if ( !ok_to_chnet( nobj->id, NULL ) ) {
				/* something's going on that could result in failure if this net */
				/*		is changed */
				/* we'll allow the change, however, if this is a new route & */
				/*		ATTR_FORCE was supplied */
				if (	(attr_ass.list[i]->seqno > 0) && (!force) )
					nim_error( 0, NULL, NULL, NULL );
			}
			routing_change = TRUE;

			break;

		case ATTR_OTHER_NET_TYPE:
			/* if adding a new type... */
			sprintf( tmp, "%s%d", ATTR_OTHER_NET_TYPE_T, attr_ass.list[i]->seqno );
			if (	(attr_ass.list[i]->value != NULL) && 
			    (attr_ass.list[i]->value[0] != NULL_BYTE) ) {
				/* make sure it's a valid type */
				if ( (new_net_type = a2net_type( attr_ass.list[i]->value )) == -1 )
					nim_error(	ERR_BAD_TYPE_FOR, attr_ass.list[i]->value,
					    ATTR_CLASS_NETWORKS_T, NULL );

				/* make sure it's unique */
				if ( new_net_type == nobj->type->attr )
					nim_error( ERR_TYPE_CONFLICT, tmp, nobj->type->name, nobj->name);
				else if ( (k = find_attr(	nobj, NULL, attr_ass.list[i]->value,
				    0, ATTR_OTHER_NET_TYPE )) >= 0 ) {
					sprintf( tmp, "%s%d", ATTR_OTHER_NET_TYPE_T, 
					    nobj->attrs[k].seqno );
					nim_error( ERR_ONLY_ONE, ATTR_OTHER_NET_TYPE_T, tmp, NULL );
				} else
					adding_net_type = TRUE;
			} else if ( (k = find_attr(	nobj, NULL, NULL, attr_ass.list[i]->seqno,
			    ATTR_OTHER_NET_TYPE )) >= 0 ) {
				/* removing a net type - make sure no clients use that type */
				if ( (net_type = a2net_type( nobj->attrs[k].value )) == FAILURE )
					nim_error( 0, NULL, NULL, NULL );
				for (j = 0; j < ifinfo.num; j++) {
					if ( s2nim_if( ifs[j].value, &nimif ) == FAILURE )
						nim_error( 0, NULL, NULL, NULL );
					else if ( a2net_type( nimif.adapter) == net_type )
						nim_error( ERR_RM_NET_TYPE, tmp, nobj->name, NULL );
				}
			}
			break;
		}
	}

	/* for each attr change... */
	for (i = 0; i < attr_ass.num; i++) {	/* look for routing changes */
		if ( attr_ass.list[i]->pdattr == ATTR_ROUTING ) {
			VERBOSE("\tchanging routing\n", NULL, NULL, NULL, NULL)

			/* change the routing info */
			if ( chroute( nobj->id, nobj->name, attr_ass.list[i] ) == FAILURE )
				nim_error( 0, NULL, NULL, NULL );
		} else if ( ch_attr(	nobj->id, NULL, attr_ass.list[i]->value,
		    attr_ass.list[i]->seqno, attr_ass.list[i]->pdattr,
		    attr_ass.list[i]->name ) == FAILURE )
			nim_error( 0, NULL, NULL, NULL );
	}

	/* if adding a network type... */
	if ( adding_net_type ) {
		/* make sure all clients have 4th field in their ATTR_IF */
		if ( ck_all_clients( nobj, ifs, &ifinfo ) == FAILURE )
			nim_error( 0, NULL, NULL, NULL );
	}

	/* success! - clear backups */
	clear_backups( &backups );

	if ( routing_change ) {
		/* set all the ATTR_NSTATEs (network states) & connected machines */
		set_all_nstates();
	}

	return( SUCCESS );

} /* end of ch_network */


/* --------------------------- parse_args        
 *
 * NAME: parse_args
 *
 * FUNCTION:
 *		parses command line arguments
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *		calls nim_error
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *		parameters:
 *			argc			= argc from main
 *			argv			= argv from main
 *		global:
 *
 * RETURNS: (int)
 *		SUCCESS					= no syntax errors on command line
 *
 * OUTPUT:
 * ----------------------------------------------------------------------------
 */

int
parse_args(	int argc, 
char *argv[] )

{	
	extern char	*optarg;
	extern int	optind, optopt;
	int	c;
	int	syntax_err = FALSE;

	/* loop through all args */
	while ( (c = getopt(argc, argv, "a:q")) != -1 ) {	
		switch (c) {
		case 'a': /* attribute assignment */
			if (!parse_attr_ass( &attr_ass, valid_attrs, optarg, TRUE ) )
				nim_error( 0, NULL, NULL, NULL );
			break;

		case 'q': /* display valid_attrs */
			mstr_what( valid_attrs, NULL );
			exit( 0 );
			break;

		case ':': /* option is missing a required argument */
			nim_error( ERR_MISSING_OPT_ARG, optopt, NULL, NULL );
			break;

		case '?': /* unknown option */
			nim_error(ERR_BAD_OPT, optopt, MSG_msg(MSG_GENERIC_CHSYNTAX), NULL);
			break;
		}

		if ( syntax_err )
			nim_error( ERR_SYNTAX, MSG_msg(MSG_GENERIC_CHSYNTAX), NULL, NULL );
	}

	/* check for errors */
	if ( optind == argc )
		nim_error( ERR_MISSING_OPERAND, MSG_msg(MSG_OBJ_NAME), NULL, NULL );
	else if ( optind < (argc - 1) )
		nim_error( ERR_SYNTAX, MSG_msg(MSG_M_CHNET_SYNTAX), NULL, NULL );

	name = argv[optind];

	/* return */
	return( SUCCESS );

} /* end of parse_args */


/**************************       main         ********************************/

main( int argc, char *argv[] )
{
	/* initialize NIM environment */
	mstr_init( argv[0], ERR_POLICY_METHOD, NULL );

	if ( !get_list_space( &attr_ass, DEFAULT_CHUNK_SIZE, TRUE ) )
		nim_error( 0, NULL, NULL, NULL );

	/* parse command line */
	parse_args( argc, argv );


	/* change network info */
	ch_network();

	exit( 0 );

}


