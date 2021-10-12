static char	sccs_id[] = " @(#)88 1.10.1.2  src/bos/usr/lib/nim/methods/m_alloc_spot.c, cmdnim, bos411, 9434B411a  8/24/94  15:09:25";

/*
 *   COMPONENT_NAME: CMDNIM
 *
 *   FUNCTIONS: ./usr/lib/nim/methods/m_alloc_spot.c
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

extern int valid_pdattr_ass();

/*---------------------------- module globals    -----------------------------*/
ATTR_ASS_LIST attr_ass;

VALID_ATTR valid_attrs[] =
{	
	{0,							NULL,							FALSE,	valid_pdattr_ass}
};

char *name=NULL;									/* target object name */

/*---------------------------- parse_args        ------------------------------
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
*-----------------------------------------------------------------------------*/

int
parse_args(	int argc, 
				char *argv[] )

{	extern char *optarg;
	extern int optind, optopt;
	int c;

	/* loop through all args */
	while ( (c = getopt(argc, argv, "a:q")) != -1 )
	{	switch (c)
		{	
			case 'a': /* attribute assignment */
				if (! parse_attr_ass( &attr_ass, valid_attrs, optarg, FALSE ) )
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
				nim_error(	ERR_BAD_OPT, optopt, 
								MSG_msg(MSG_GENERIC_ALSYNTAX), NULL );
			break;
		}
	}

	/* check for errors */
	if ( optind == argc )
		nim_error( ERR_MISSING_OPERAND, MSG_msg(MSG_OBJ_NAME), NULL, NULL );
	else if ( optind < (argc-1) )
		nim_error( ERR_SYNTAX, MSG_msg(MSG_GENERIC_ALSYNTAX), NULL, NULL );

	name = argv[optind];

	/* return */
	return( SUCCESS );

} /* end of parse_args */
	
/*---------------------------- ok_to_spot        ------------------------------
*
* NAME: ok_to_spot
*
* FUNCTION:
*		performs SPOT specific allocation checking:
*			1) verify that the SPOT supports the target's interface type
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
*			target				= machine which is going to use <res>
*			nimif					= nim_if struct for target's PIF
*			res					= resource to be used by <target>
*		global:
*
* RETURNS: (int)
*		SUCCESS					= ok to allocate the nim_bundle
*		FAILURE					= not ok
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
ok_to_spot(	struct nim_object *target,
				struct nim_if *nimif,
				struct nim_object *res )

{	int rc=FAILURE;
	ODMQUERY
	struct nim_attr root_server;
	int i;
	int platform = -1;
	int net_type;
	char *net_type_str;

	/* SPOTs can be allocated as the source for SPOT creation operations */
	/* in these cases, we don't care about the interfaces that a SPOT supports */
	if ( find_attr_ass( &attr_ass, ATTR_IGNORE_STATE ) >= 0 )
		return( SUCCESS );

	/* convert client's network adapter name into a network type */
	if ( (net_type = a2net_type( nimif->adapter )) == -1 )
		return( FAILURE );
	net_type_str = ATTR_msg( net_type );

	/* construct the if_supported pattern we're expecting, which is: */
	/*		"<platform type> <network type>" */
	if ( (platform = find_attr( target, NULL, NULL, 0, ATTR_PLATFORM )) < 0 )
		ERROR( ERR_OBJ_MISSING_ATTR, ATTR_PLATFORM_T, target->name, NULL )
	sprintf( query, "%s %s", target->attrs[platform].value, net_type_str );

	/* does SPOT support this platform/network combo? */
	if ( find_attr( res, 0, query, 0, ATTR_IF_SUPPORTED ) == -1 )
		ERROR( ERR_IF_NS_BY_SPOT, res->name, target->name, NULL )

	/* if client has root resource allocated - make sure the server of the */
	/*		root is the same as the SPOT server */
	/* this requirement is enforced in order to support the root sync */
	/*		methodology */
	if ( (i = find_attr( target, NULL, NULL, 0, ATTR_ROOT )) >= 0 )
	{
		/* find the server of the root resource */
		sprintf( query, "id=%d and pdattr=%d", get_id(target->attrs[i].value),
					ATTR_SERVER );
		if ( odm_get_first( nim_attr_CLASS, query, &root_server ) < 0 )
			ERROR(	ERR_OBJ_MISSING_ATTR, ATTR_SERVER_T, target->attrs[i].value,
						NULL )

		/* is root server the same as the spot server? */
		if ( (i = find_attr( res, NULL, NULL, 0, ATTR_SERVER )) < 0 )
			ERROR( ERR_OBJ_MISSING_ATTR, ATTR_SERVER_T, res->name, NULL )
		else if ( strcmp( root_server.value, res->attrs[i].value ) != 0 )
			ERROR( ERR_ROOT_SPOT_SERVER, NULL, NULL, NULL )
	}

	return( SUCCESS );

} /* end of ok_to_spot */
	
/*------------------------- alloc_spot            ----------------------------
*
* NAME: alloc_spot
*
* FUNCTION:
*		allocates the specified SPOT
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
*		SUCCESS					= resources allocated
*		FAILURE					= error encountered
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
alloc_spot()

{	int res_type;
	NIM_OBJECT( target, minfo )
	NIM_OBJECT( res, rinfo )
	struct nim_if nimif;
	struct nim_if spot_nimif;
	int i;

	/* get the target object */
	if ( lag_object( 0, name, &target, &minfo ) == FAILURE )
		nim_error( 0, NULL, NULL, NULL );

	/* find the target's PIF */
	if ( (i = find_attr( target, NULL, NULL, 1, ATTR_IF )) == -1 )
		nim_error( ERR_OBJ_MISSING_ATTR, PIF, target->name, NULL );

	/* convert to nim_if */
	if ( s2nim_if( target->attrs[i].value, &nimif ) == FAILURE )
		nim_error( ERR_NET_INFO, PIF, target->name, NULL );

	/* for each SPOT to be allocated */
	for (i=0; i < attr_ass.num; i++)
	{	
		/* make sure attr specifies a SPOT */
		if ( attr_ass.list[i]->pdattr != ATTR_SPOT )
			continue;

		/* lock-and-get the resource object */
		if ( lag_object( 0, attr_ass.list[i]->value, &res, &rinfo ) == FAILURE )
			nim_error( 0, NULL, NULL, NULL );

		VERBOSE("   performing allocation for the %s SPOT\n",res->name,NULL,NULL,
					NULL)

		/* perform pre-allocation checks, then allocate the SPOT */
		if (	(ok_to_allocate(target, nimif.network, res, &spot_nimif)==FAILURE)||
				(ok_to_spot(target, &nimif, res) == FAILURE) ||
				(do_allocation( target, res, NULL, &spot_nimif, NULL ) == FAILURE) )
				nim_error( 0, NULL, NULL, NULL );

		/* free memory */
		uaf_object( res, &rinfo, FALSE );
	}

	return( SUCCESS );

} /* end of alloc_spot */

/**************************       main         ********************************/

main( int argc, char *argv[] )

{
	/* initialize NIM environment */
	mstr_init( argv[0], ERR_POLICY_METHOD, NULL );

	if ( ! get_list_space( &attr_ass, DEFAULT_CHUNK_SIZE, TRUE ) )
		nim_error( 0, NULL, NULL, NULL );

	/* parse command line */
	parse_args( argc, argv );

	VERBOSE("m_alloc_spot: allocate a SPOT to %s\n",name,NULL,NULL,NULL)

	/* allocate the specified SPOT */
	alloc_spot();

	exit( 0 );

}
