static char	sccs_id[] = " @(#)71 1.5  src/bos/usr/lib/nim/methods/m_alloc_pdir.c, cmdnim, bos411, 9428A410j  3/4/94  16:46:17";

/*
 *   COMPONENT_NAME: CMDNIM
 *
 *   FUNCTIONS: ./usr/lib/nim/methods/m_alloc_pdir.c
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
				cmd_what( valid_attrs );
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

/*---------------------------- mk_subdir         ------------------------------
*
* NAME: mk_subdir
*
* FUNCTION:
*		creates a subdirectory for the target in the specified pdir
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
*			target				= ptr to target's nim_object
*			pdir_server			= NIM name of pdir server
*			pdir					= pdir pathname
*			subdir				= ptr to char ptr
*		global:
*
* RETURNS: (int)
*		SUCCESS					= subdir created
*		FAILURE					= unable to create subdir
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
mk_subdir(	struct nim_object *target,
				char *pdir_server,
				char *pdir, 
				char **subdir )

{	int len;
	char *args[] = { MKDIR, "-p", NULL, NULL };
	int rc;
	FILE *c_stdout=NULL;

	/* we're going to create a subdirectory at the pdir location */
	/* form the absolute pathname now */
	len = strlen(pdir) + strlen(target->name) + 2;
	*subdir = nim_malloc( len );
	sprintf( *subdir, "%s/%s", pdir, target->name );

	/* create the target's subdir on the pdir server */
	args[2] = *subdir;
	if ( master_exec( pdir_server, &rc, &c_stdout, args) == FAILURE )
		ERROR( 0, NULL, NULL, NULL )
	else if ( rc > 0 )
		ERROR( ERR_METHOD, pdir_server, niminfo.errstr, NULL )

	return( SUCCESS );

} /* end of mk_subdir */
	
/*---------------------------- rm_subdir         ------------------------------
*
* NAME: rm_subdir
*
* FUNCTION:
*		removes the specified subdir
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
*			pdir_server			= NIM name of pdir server
*			subdir				= ptr to char ptr
*		global:
*
* RETURNS: (int)
*		SUCCESS					= subdir removed
*		FAILURE					= unable to create subdir
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
rm_subdir(	char *pdir_server,
				char *subdir )

{	int len;
	char *args[] = { RMDIR, subdir, NULL };
	int rc;
	FILE *c_stdout=NULL;

	/* remove the specified subdir */
	if ( master_exec( pdir_server, &rc, &c_stdout, args) == FAILURE )
		ERROR( 0, NULL, NULL, NULL )
	else if ( rc > 0 )
		ERROR( ERR_METHOD, pdir_server, niminfo.errstr, NULL )

	return( SUCCESS );

} /* end of rm_subdir */
	
/*------------------------- alloc_pdir            ----------------------------
*
* NAME: alloc_pdir
*
* FUNCTION:
*		allocates the specified pdir type resource by:
*			1) creating a subdirectory for the specified client
*			2) exporting that subdir for the client's use
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
*			backups
*
* RETURNS: (int)
*		SUCCESS					= resources allocated
*		FAILURE					= error encountered
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
alloc_pdir()

{	int res_type;
	NIM_OBJECT( target, minfo )
	NIM_OBJECT( res, rinfo )
	ODMQUERY
	struct nim_attr spot_server;
	struct nim_if nimif;
	struct nim_if pdir_nimif;
	int i,j,k,l;
	char *dir;
	char *loc;
	char *server;

	/* get the target object */
	if ( lag_object( 0, name, &target, &minfo ) == FAILURE )
		nim_error( 0, NULL, NULL, NULL );

	/* find the target's PIF */
	if ( (i = find_attr( target, NULL, NULL, 1, ATTR_IF )) == -1 )
		nim_error( ERR_OBJ_MISSING_ATTR, PIF, target->name, NULL );

	/* convert to nim_if */
	if ( s2nim_if( target->attrs[i].value, &nimif ) == FAILURE )
		nim_error( ERR_NET_INFO, PIF, target->name, NULL );

	/* for each pdir to be allocated */
	for (i=0; i < attr_ass.num; i++)
		if ( attr_in_subclass( attr_ass.list[i]->pdattr, ATTR_SUBCLASS_PDIR_TYPE))
		{	
			/* get resource object */
			if ( lag_object(	0, attr_ass.list[i]->value, &res, &rinfo ) ==FAILURE)
				nim_error( 0, NULL, NULL, NULL );

			VERBOSE("   verifying allocation of \"%s\"\n",res->name,NULL,NULL,NULL)

			/* find location & server */
			if ( (j = find_attr( res, NULL, NULL, 0, ATTR_LOCATION )) < 0 )
				nim_error( ERR_OBJ_MISSING_ATTR, ATTR_LOCATION_T, res->name, NULL );
			if ( (k = find_attr( res, NULL, NULL, 0, ATTR_SERVER )) < 0 )
				nim_error( ERR_OBJ_MISSING_ATTR, ATTR_SERVER_T, res->name, NULL );
			loc = res->attrs[j].value;
			server = res->attrs[k].value;

			/* if this is an ATTR_ROOT resource and the target already has a */
			/*		SPOT allocated, check to make sure that the root server is the */
			/*		same as the spot server */
			/* this is required in order to support the root sync methodology */
			if (	(res->type->attr == ATTR_ROOT) &&
					((j = find_attr( target, NULL, NULL, 0, ATTR_SPOT )) >= 0) )
			{
				/* find server of spot */
				sprintf( query, "id=%d and pdattr=%d", 
							get_id(target->attrs[j].value), ATTR_SERVER );
				if ( odm_get_first( nim_attr_CLASS, query, &spot_server ) < 0 )
					nim_error( ERR_OBJ_MISSING_ATTR, ATTR_SERVER_T, 
									target->attrs[j].value, NULL );

				/* root server and spot server must be the same */
				if ( strcmp( server, spot_server.value ) != 0 )
					nim_error( ERR_ROOT_SPOT_SERVER, NULL, NULL, NULL );
			}
	
			/* ok to allocate this resource? */
			if ( ok_to_allocate( target, nimif.network, res, &pdir_nimif)==FAILURE)
				nim_error( 0, NULL, NULL, NULL );
	
			VERBOSE("   creating subdirectory within \"%s\"\n",res->name,NULL,NULL,
						NULL)
	
			/* create a subdir for the target */
			if ( mk_subdir(	target, server, loc, &dir ) == FAILURE )
				nim_error( 0, NULL, NULL, NULL );
	
			/* finish allocation */
			if ( do_allocation( target, res, dir, &pdir_nimif, NULL ) == FAILURE )
			{
				/* undo what we've done */
				protect_errstr = TRUE;
				rm_subdir( server, dir );
				nim_error( 0, NULL, NULL, NULL );
			}
	
			/* release res object */
			uaf_object( res, &rinfo, TRUE );
		}

	return( SUCCESS );

} /* end of alloc_pdir */

/**************************       main         ********************************/

main( int argc, char *argv[] )

{
	/* initialize NIM environment */
	mstr_init( argv[0], ERR_POLICY_METHOD, NULL );

	if ( ! get_list_space( &attr_ass, DEFAULT_CHUNK_SIZE, TRUE ) )
		nim_error( 0, NULL, NULL, NULL );

	/* parse command line */
	parse_args( argc, argv );

	VERBOSE("m_alloc_pdir: allocate a pdir type resource to %s",name,NULL,NULL,
				NULL)

	/* allocate the specified pdir type resource */
	alloc_pdir();

	exit( 0 );

}
