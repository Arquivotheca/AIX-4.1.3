static char   sccs_id[] = " @(#)45 1.11  src/bos/usr/lib/nim/methods/m_spotmaint.c, cmdnim, bos41J, 9511A_all  3/3/95  15:03:30";

/*
 *   COMPONENT_NAME: CMDNIM
 *
 *   FUNCTIONS: ./usr/lib/nim/methods/m_spotmaint.c
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
extern int ch_pdattr_ass();

/*---------------------------- module globals    -----------------------------*/
ATTR_ASS_LIST attr_ass;

VALID_ATTR valid_attrs[] =
{	
 	{ATTR_INSTALLP_FLAGS,	ATTR_INSTALLP_FLAGS_T,	TRUE,		valid_pdattr_ass},
	{ATTR_INSTALLP_BUNDLE,	ATTR_INSTALLP_BUNDLE_T,	FALSE,	valid_pdattr_ass},
 	{ATTR_INSTALLP_OPTIONS,	ATTR_INSTALLP_OPTIONS_T,FALSE,	valid_pdattr_ass},
{ATTR_NUM_PARALLEL_SYNCS,ATTR_NUM_PARALLEL_SYNCS_T,FALSE,	valid_pdattr_ass},
	{0,							NULL, 						FALSE,	ch_pdattr_ass}
};

char *name=NULL;								/* NIM name of object to create */
char *location=NULL;							/* location of SPOT */
char *installp_bundle=NULL;				/* name of installp_bundle to use */
char *lpp_source=NULL;						/* name of lpp_source to use */
char *options=NULL;							/* options to install */
char *flags=NULL;								/* installp flags to use */
LIST res_access_list;						/* LIST of res_access structs */
LIST params_list;								/* LIST of parameters for methods */
LIST alloc_list;								/* LIST of alloc'd resources */
char params[MAX_VALUE];						/* tmp storage for parameter str */
NIM_OBJECT( spot, sinfo )					/* the new SPOT's NIM definition */
char *server_name=NULL;						/* NIM name of server */
int maint_errno=0;

/*---------------------------- ck_attrs           -------------------------
*
* NAME: ck_attrs
*
* FUNCTION:
*		checks to make sure that the information supplied by user is sufficient
*			to complete object definition
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
*			valid_attrs
*			attr_ass
*
* RETURNS: (int)
*		SUCCESS					= nothing missing
*		FAILURE					= definition incomplete
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
ck_attrs ()

{	int i,j;
	char *path;
	int found;
	ODMQUERY
	struct nim_pdattr pdattr;

	/* loop through the attrs */
	for (i=0; i < attr_ass.num; i++)
		switch (attr_ass.list[i]->pdattr)
		{	
			case ATTR_INSTALLP_BUNDLE:
				/* have options already been specified? */
				if ( options != NULL )
					nim_error(	ERR_ATTR_CONFLICT, ATTR_INSTALLP_BUNDLE_T,
									ATTR_INSTALLP_OPTIONS_T, NULL);
				else if ( installp_bundle != NULL )
					nim_error(	ERR_ONLY_ONE_ATTR, ATTR_INSTALLP_BUNDLE_T,NULL,NULL);

				installp_bundle = attr_ass.list[i]->value;
			break;

			case ATTR_INSTALLP_OPTIONS:
				/* has a installp_bundle already been specified? */
				if ( installp_bundle != NULL )
					nim_error(	ERR_ATTR_CONFLICT, ATTR_INSTALLP_BUNDLE_T,
									ATTR_INSTALLP_OPTIONS_T, NULL);
				else if ( options != NULL )
					nim_error( ERR_ONLY_ONE_ATTR, ATTR_INSTALLP_OPTIONS_T,NULL,NULL);

				options = attr_ass.list[i]->value;
			break;

			case ATTR_INSTALLP_FLAGS:
				if ( flags != NULL )
					nim_error( ERR_ONLY_ONE_ATTR, ATTR_INSTALLP_FLAGS_T, NULL, NULL);

				flags = attr_ass.list[i]->value;
			break;

			case ATTR_IGNORE_STATE:
			case ATTR_FORCE:
				force = TRUE;
			break;
		}

	/* anything missing? */
	if ( flags == NULL )
		nim_error( ERR_MISSING_ATTR, ATTR_INSTALLP_FLAGS_T, NULL, NULL );

	if ( force )
		rm_attr_ass( &attr_ass, ATTR_FORCE );

	return( SUCCESS );

} /* end of ck_attrs */
	
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
	int syntax_err=FALSE;

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
				nim_error( ERR_BAD_OPT, optopt, MSG_msg(MSG_GENERIC_MKSYNTAX),NULL);
			break;
		}

		if ( syntax_err )
			nim_error( ERR_SYNTAX, MSG_msg(MSG_GENERIC_MKSYNTAX), NULL, NULL );
	}

	/* check for errors */
	if ( optind == argc )
		nim_error( ERR_MISSING_OPERAND, MSG_msg(MSG_OBJ_NAME), NULL, NULL );
	else if ( optind < (argc-1) )
		nim_error( ERR_SYNTAX, MSG_msg(MSG_GENERIC_MKSYNTAX), NULL, NULL );

	name = argv[optind];

	/* return */
	return( SUCCESS );

} /* end of parse_args */
	
/* --------------------------- undo_ckspot -------------------------------------
 *
 * NAME: undo_ckspot
 *
 * FUNCTION:
 *		sets the SPOT's Rstate when error has occurred (probably interrupt)
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *		calls nim_error
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *	parameters:
 *		errno	= error code
 *		str1	= str1 for error msg
 *		str2	= str2 for error msg
 *		str3	= str3 for error msg
 *	global:
 *
 * RETURNS: (void)
 *
 * OUTPUT:
 * -------------------------------------------------------------------------- */

void
undo_ckspot(	int errno,
					char *str1,
					char *str2,
					char *str3 )

{
	errno = nene( errno, str1, str2, str3 );

	/* check the SPOT */
	if ( check_spot( name, server_name, &alloc_list ) == FAILURE )
		warning( 0, NULL, NULL, NULL );

	exit( errno );

} /* end of undo_ckspot */
	
/*---------------------------- maint_spot          -----------------------------
*
* NAME: maint_spot
*
* FUNCTION:
*		performs installp maintenance operations within a SPOT
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*		global:
*			name					= NIM name of SPOT
*			spot					= ptr to nim_object for SPOT
*
* RETURNS: (int)
*		SUCCESS					= SPOT installed
*		FAILURE					= internal error
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
maint_spot()

{	int i,j;
	struct nim_if nimif;
	NIM_OBJECT( server, serveri )
	int rc=0;
	int do_root_sync;
	int doing_preview=0;
	FILE *c_stdout=NULL;
	struct res_access *raccess=NULL;
	char *bundle=NULL;
	char *bun_path=NULL;
	char *root_sync_errmsg=NULL;

	/* find the location and server of this SPOT */
	if ( (i = find_attr( spot, NULL, NULL, 0, ATTR_LOCATION )) < 0 )
		ERROR( ERR_OBJ_MISSING_ATTR, ATTR_LOCATION_T, name, NULL );
	location = spot->attrs[i].value;
	if ( (i = find_attr( spot, NULL, NULL, 0, ATTR_SERVER )) < 0 )
		ERROR( ERR_OBJ_MISSING_ATTR, ATTR_SERVER_T, name, NULL );
	server_name = spot->attrs[i].value;

	/* was a installp_bundle specified? */
	if ( installp_bundle != NULL )
	{	
		/* allocate the installp_bundle to the SPOT server */

		/* get the server's object because we need to do some stuff with it */
		if ( lag_object( 0, server_name, &server, &serveri ) == FAILURE )
			ERROR( 0, NULL, NULL, NULL )

		/* get the server's PIF info */
		if ( (i = find_attr( server, NULL, NULL, 1, ATTR_IF )) < 0 )
			ERROR( ERR_OBJ_MISSING_ATTR, PIF, server->name, NULL )
		if ( s2nim_if( server->attrs[i].value, &nimif ) == FAILURE )
			ERROR( ERR_NET_INFO, PIF, server->name, NULL )

		/* allocate the source to the server */
		if (	(alloc_res( server, installp_bundle, &nimif, &c_stdout )==FAILURE)||
				(add_to_LIST( &alloc_list, installp_bundle ) == FAILURE) )
			ERROR( 0, NULL, NULL, NULL )

		/* resources allocated, which means new attrs were added, so re-get it */
		odm_free_list( server, &serveri );
		if ( lag_object( 0, server_name, &server, &serveri ) == FAILURE )
			ERROR( 0, NULL, NULL, NULL )

		/* generate res_access LIST - tells us what interface to use */
		/*		on the server in order to access resources */
		if ( LIST_res_access( server, &res_access_list ) == FAILURE )
			ERROR( 0, NULL, NULL, NULL )

		/* server may have had other resources allocated, so loop through and */
		/*		find the installp_bundle */
		for (i=0; i < res_access_list.num; i++)
		{	
			raccess = (struct res_access *) res_access_list.list[i];
			if ( strcmp( installp_bundle, raccess->name ) == 0 )
			{	
				/* local or remote access? */
				if ( strcmp( raccess->server, server_name ) == 0 )
				{
					bundle = nim_malloc( strlen(ATTR_INSTALLP_BUNDLE_T) +
												strlen(raccess->location) + 4 );
					sprintf( bundle, "-a%s=%s", ATTR_INSTALLP_BUNDLE_T, 
									raccess->location );
					bun_path = nim_malloc( strlen(raccess->location) + 1 );
					sprintf( bun_path, "%s", raccess->location );
				}
				else
				{
					bundle = nim_malloc( strlen(ATTR_INSTALLP_BUNDLE_T) +
												strlen(raccess->nimif.hostname) +
												strlen(raccess->location) + 5 );
					sprintf( bundle, "-a%s=%s:%s", ATTR_INSTALLP_BUNDLE_T, 
									raccess->nimif.hostname, raccess->location );
					bun_path = nim_malloc(	strlen(raccess->nimif.hostname) + 
													strlen(raccess->location) + 2 );
					sprintf( bun_path, "%s:%s", raccess->nimif.hostname,
								raccess->location );
				}

				break;
			}
		}
	} /* if installp_bundle specified */

	/* initialize the parameter string for C_INSTSPOT */

	/* method pathname */
	if ( add_to_LIST( &params_list, C_INSTSPOT ) == FAILURE )
		ERROR( 0, NULL, NULL, NULL );

	/* SPOT location (the complete pathname) */
	sprintf( params, "-a%s=%s", ATTR_LOCATION_T, location );
	if ( add_to_LIST( &params_list, params ) == FAILURE )
		ERROR( 0, NULL, NULL, NULL );

	/* SPOT name */
	sprintf( params, "-a%s=%s", ATTR_NAME_T, name );
	if ( add_to_LIST( &params_list, params ) == FAILURE )
		ERROR( 0, NULL, NULL, NULL );

	/* installp_bundle or installp_options? */
	if ( bundle != NULL )
	{
		if ( add_to_LIST( &params_list, bundle ) == FAILURE )
			ERROR( 0, NULL, NULL, NULL );
	}
	else if ( options != NULL )
	{
		sprintf( params, "-a%s=%s", ATTR_INSTALLP_OPTIONS_T, options );
		if ( add_to_LIST( &params_list, params ) == FAILURE )
			ERROR( 0, NULL, NULL, NULL );
	}

	/* if user has specified installp flags, then use them */
	if ( flags != NULL )
	{	
		sprintf( params, "-a%s=%s", ATTR_INSTALLP_FLAGS_T, flags );
		if ( add_to_LIST( &params_list, params ) == FAILURE )
			ERROR( 0, NULL, NULL, NULL );
	}

	/* Perform a root sync on diskless/dataless clients which use this   */
	/* SPOT.  Do it now for all installp maint ops except commit and     */
	/* cleanup. For these two ops, the usr part should be done           */
	/* before the root part, so we'll do the root sync later.  For the   */
	/* remaining ops, essentially reject and deinstall, deinstall info   */
	/* is required from the SPOT to perform the root part operation,     */
        /* therefore we don't remove from the SPOT before the roots.         */
	/* If doing a preview, skip the root_sync, since SPOT maint op is    */
        /*    independent of what goes on with roots.		             */

	do_root_sync = TRUE;
	if (!(doing_preview = (strstr (flags, "p") != NULL)))
	{	
		if ( 	(strchr (flags, 'c') == NULL) && 
			(strchr (flags, 'C') == NULL))
		{
			/* any errors here are considered to be non-fatal */
			if ( sync_roots( spot, flags, options, bun_path ) == FAILURE )
			{
				/* cache the current error msg */
				root_sync_errmsg = niminfo.errstr;
				niminfo.errstr = NULL;
			}
	 		do_root_sync = FALSE;
		}
	}
	/* if any interrupts occur during execution of this method, we want to */
	/*		check the SPOT and sets its Rstate appropriately */
	undo_on_interrupt = undo_ckspot;

	VERBOSE("   going to perform an installp operation in the %s:%s SPOT\n",
				server_name,location,NULL,NULL)
	VERBOSE("   this may take some time - please wait\n",NULL,NULL,NULL,NULL)

	/* all ready, now do it! 					*/
        /* NOTE: If requested to  perform a "preview" op for installp,  */
        /*      send output to the screen.  Otherwise, buffer and only  */
        /*      display errors.                                         */
	if ( master_exec(	server_name, 
				&rc, 
                                doing_preview ? NULL: &c_stdout,
				&params_list.list[0] ) == FAILURE )
		maint_errno = nene( 0, NULL, NULL, NULL );
	else if ( rc > 0 )
		maint_errno = nene( ERR_METHOD, server_name, niminfo.errstr, NULL );

	/* do the root_sync if we didn't do it earlier (and not previewing) */
	if ((do_root_sync) && (! doing_preview) &&
	    (sync_roots( spot, flags, options, bun_path ) == FAILURE ))
	{
		/* any errors here are considered to be non-fatal */
		root_sync_errmsg = niminfo.errstr;
		niminfo.errstr = NULL;
	}

	/* is there a root sync error msg? */
	if ( root_sync_errmsg != NULL )
	{
		/* display it now */
		niminfo.errstr = root_sync_errmsg;
		warning( 0, NULL, NULL, NULL );
	}

	/* add any attrs which were written to stdout */
	attrs_from_FILE( spot->id, c_stdout );

	/* deallocate the resources allocated for this operation */
	dealloc_LIST( NULL, server, &alloc_list, &c_stdout );

	/* wow - SPOT is installed! */
	return( SUCCESS );

} /* end of maint_spot */
	
/**************************       main         ********************************/

main( int argc, char *argv[] )

{	int rc;
	char *tmp = NULL;

	/* initialize NIM environment */
	mstr_init( argv[0], ERR_POLICY_METHOD, NULL );

	/* initialize the attr_ass LIST */
	if ( ! get_list_space( &attr_ass, DEFAULT_CHUNK_SIZE, TRUE ) )
		nim_error( 0, NULL, NULL, NULL );

	/* initialize the parameter LIST */
	if ( ! get_list_space( &params_list, DEFAULT_CHUNK_SIZE, TRUE ) )
		nim_error( 0, NULL, NULL, NULL );

	/* initialize the allocation LIST */
	if ( ! get_list_space( &alloc_list, DEFAULT_CHUNK_SIZE, TRUE ) )
		nim_error( 0, NULL, NULL, NULL );

	/* parse command line */
	parse_args( argc, argv );

	VERBOSE("m_maintspot: perform installp maintenance on the %s SPOT\n",name,
				NULL,NULL,NULL)

	/* get the SPOT's object */
	if ( lag_object( 0, name, &spot, &sinfo ) == FAILURE )
		nim_error( 0, NULL, NULL, NULL );
	if ( spot->type->attr != ATTR_SPOT )
		nim_error(	ERR_OP_NOT_ALLOWED, ATTR_CUST_T, 
						ATTR_msg(spot->type->attr), NULL );

	/* check for missing attrs */
	ck_attrs();

   /* check the SPOT's state */
	if ( ok_to_op_spot( spot ) == FAILURE )
		nim_error( 0, NULL, NULL, NULL );

	if ( maint_spot() == FAILURE )
		maint_errno = nene( 0, NULL, NULL, NULL );

	if ( check_spot( name, server_name, &alloc_list ) == FAILURE )
		warning( 0, NULL, NULL, NULL );

	/* all done - bye bye */
	exit( maint_errno );
}

