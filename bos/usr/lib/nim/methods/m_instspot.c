static char   sccs_id[] = " @(#)22 1.24.1.6  src/bos/usr/lib/nim/methods/m_instspot.c, cmdnim, bos41J, 9511A_all  3/3/95  15:02:50";

/*
 *   COMPONENT_NAME: CMDNIM
 *
 *   FUNCTIONS: ./usr/lib/nim/methods/m_instspot.c
 *		
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993, 1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "cmdnim_mstr.h"

extern int valid_pdattr_ass();
extern int ch_pdattr_ass();
extern int ok_to_op_spot();
/*---------------------------- module globals    -----------------------------*/
ATTR_ASS_LIST attr_ass;

VALID_ATTR valid_attrs[] =
{	
 	{ATTR_LPP_SOURCE,	ATTR_LPP_SOURCE_T,	TRUE,	valid_pdattr_ass},
 	{ATTR_INSTALLP_FLAGS,	ATTR_INSTALLP_FLAGS_T,	FALSE,	valid_pdattr_ass},
 	{ATTR_FIXES,		ATTR_FIXES_T,		FALSE,	valid_pdattr_ass},
 	{ATTR_INSTALLP_OPTIONS,	ATTR_INSTALLP_OPTIONS_T,FALSE,	valid_pdattr_ass},
 	{ATTR_FIX_BUNDLE,	ATTR_FIX_BUNDLE_T,	FALSE,	valid_pdattr_ass},
 	{ATTR_INSTALLP_BUNDLE,	ATTR_INSTALLP_BUNDLE_T,	FALSE,	valid_pdattr_ass},
	{ATTR_AUTO_EXPAND,	ATTR_AUTO_EXPAND_T,	FALSE,	ch_pdattr_ass},
	{ATTR_FORCE,		ATTR_FORCE_T,		FALSE,	ch_pdattr_ass},
	{ATTR_IGNORE_STATE,	ATTR_IGNORE_STATE_T,	FALSE,	ch_pdattr_ass},
	{ATTR_DEBUG,		ATTR_DEBUG_T,		FALSE,	ch_pdattr_ass},
{ATTR_NUM_PARALLEL_SYNCS,ATTR_NUM_PARALLEL_SYNCS_T,FALSE,	valid_pdattr_ass},
	{0,			NULL, 			FALSE,	ch_pdattr_ass}
};

char *name=NULL;								/* NIM name of object to create */
char *location=NULL;							/* location of SPOT */
char *lpp_source=NULL;						/* name of lpp_source to use */
char *flags=NULL;								/* installp flags to use */
char *options=NULL;							/* options to install */
char *fixes=NULL;							/* fixes to install */
char *installp_bundle=NULL;					/* name of installp_bundle to use */
char *fix_bundle=NULL;						/* name of instfix_bundle to use */
LIST res_access_list;						/* LIST of res_access structs */
LIST params_list;								/* LIST of parameters for methods */
LIST alloc_list;								/* LIST of alloc'd resources */
char params[MAX_VALUE];						/* tmp storage for parameter str */
NIM_OBJECT( spot, sinfo )					/* the new SPOT's NIM definition */
char *server_name=NULL;						/* NIM name of server */
int warn_clients = FALSE;					/* >0 if SPOT alloc'd & force used */
int lpp_source_is_device=FALSE;			/* >0 if lpp_source begins with "/dev" */
int tmp_errno=0;
char *auto_expand="yes";					/* reflect ATTR_AUTO_EXPAND value */

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
 *		ALWAYS , no matter what, ALWAYS check the spot!!!!
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
*		calls undo_ckspot
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
	char *Xfl, *path;
	int found;
	ODMQUERY
	struct nim_pdattr pdattr;
	regmatch_t device[ERE_DEVICE_NUM];
	NIM_OBJECT( lpps, linfo )
	NIM_OBJECT( bun, binfo )

	/* loop through the attrs */
	for (i=0; i < attr_ass.num; i++)
		switch (attr_ass.list[i]->pdattr)
		{	
			case ATTR_IGNORE_STATE:
			case ATTR_FORCE:
				force = TRUE;
			break;

			case ATTR_AUTO_EXPAND:
				auto_expand = attr_ass.list[i]->value;
				if ( (strcmp(auto_expand, "yes") != 0) &&
                                     (strcmp(auto_expand, "no" ) != 0) )  
                                     nim_error( ERR_ATTR_YES_NO, ATTR_AUTO_EXPAND_T, NULL, NULL);

			break;

			case ATTR_LPP_SOURCE:
				/* lpp_source already been specified? */
				if ( lpp_source != NULL )
					nim_error( ERR_ONLY_ONE_ATTR, ATTR_LPP_SOURCE_T, NULL, NULL);

				lpp_source = attr_ass.list[i]->value;
			break;

			case ATTR_INSTALLP_FLAGS:
				if ( flags != NULL )
					nim_error(	ERR_ONLY_ONE_ATTR, ATTR_INSTALLP_FLAGS_T, NULL, 
										NULL);

				flags = attr_ass.list[i]->value;
			break;

			case ATTR_FIXES:
				/* has fixes already been specified? */
				if ( fixes != NULL )
					nim_error(	ERR_ONLY_ONE_ATTR, 
							ATTR_FIXES_T,
							NULL, NULL );

				fixes = attr_ass.list[i]->value;
			break;

			case ATTR_INSTALLP_OPTIONS:
				/* has options already been specified? */
				if ( options != NULL )
					nim_error(	ERR_ONLY_ONE_ATTR, 
							ATTR_INSTALLP_OPTIONS_T,
							NULL, NULL );

				options = attr_ass.list[i]->value;
			break;

			case ATTR_FIX_BUNDLE:
				/* has bundle already been specified? */
				if ( fix_bundle != NULL )
					nim_error(	ERR_ONLY_ONE_ATTR, 
							ATTR_FIX_BUNDLE_T, 
							NULL, NULL );

				fix_bundle = attr_ass.list[i]->value;

				/* must be a valid NIM fix_bundle resource */
				if ( lag_object( 0, fix_bundle, &bun, &binfo ) == FAILURE )
					nim_error( 0, NULL, NULL, NULL );
				else if ( bun->type->attr != ATTR_FIX_BUNDLE )
					nim_error(	ERR_TYPE_CONFLICT, 
							ATTR_FIX_BUNDLE_T,
							bun->type->name, 
							fix_bundle );
			break;

			case ATTR_INSTALLP_BUNDLE:
				/* has bundle already been specified? */
				if ( installp_bundle != NULL )
					nim_error(	ERR_ONLY_ONE_ATTR, 
							ATTR_INSTALLP_BUNDLE_T, 
							NULL, NULL );

				installp_bundle = attr_ass.list[i]->value;

				/* must be a valid NIM installp_bundle resource */
				if ( lag_object( 0, installp_bundle, &bun, &binfo ) == FAILURE )
					nim_error( 0, NULL, NULL, NULL );
				else if ( bun->type->attr != ATTR_INSTALLP_BUNDLE )
					nim_error(	ERR_TYPE_CONFLICT, 
							ATTR_INSTALLP_BUNDLE_T,
							bun->type->name, 
							installp_bundle );
			break;
		}

	/* Check for attribute conflicts. */
	if ( (options != NULL) && (installp_bundle != NULL )) 
		nim_error(	ERR_ATTR_CONFLICT, ATTR_INSTALLP_OPTIONS_T, ATTR_INSTALLP_BUNDLE_T, NULL);

	if ( (options != NULL) && (fix_bundle != NULL )) 
		nim_error(	ERR_ATTR_CONFLICT, ATTR_INSTALLP_OPTIONS_T, ATTR_FIX_BUNDLE_T, NULL);

	if ( (options != NULL) && (fixes != NULL )) 
		nim_error(	ERR_ATTR_CONFLICT, ATTR_INSTALLP_OPTIONS_T, ATTR_FIXES_T, NULL);

	if ( (fixes != NULL) && (installp_bundle != NULL )) 
		nim_error(	ERR_ATTR_CONFLICT, ATTR_FIXES_T, ATTR_INSTALLP_BUNDLE_T, NULL);

	if ( (fixes != NULL) && (fix_bundle != NULL )) 
		nim_error(	ERR_ATTR_CONFLICT, ATTR_FIXES_T, ATTR_FIX_BUNDLE_T, NULL);

	if ( (installp_bundle != NULL) && (fix_bundle != NULL )) 
		nim_error(	ERR_ATTR_CONFLICT, ATTR_INSTALLP_BUNDLE_T, ATTR_FIX_BUNDLE_T, NULL);

	if ( force )
		rm_attr_ass( &attr_ass, ATTR_FORCE );

	if ( lpp_source == NULL )
		nim_error( ERR_MISSING_ATTR, ATTR_LPP_SOURCE_T, NULL, NULL );
	else
	{
		/* is the lpp_source a device? */
		if ( regexec(	nimere[DEVICE_ERE].reg, lpp_source,
							ERE_DEVICE_NUM, device, 0 ) == 0 )
		{
			/* ASSUMING that it is local to the server of the SPOT & that */
			/*		it has "simages" */
			lpp_source_is_device = TRUE;
		}
		else
		{ 
			/* must be a valid NIM lpp_source resource */
			if ( lag_object( 0, lpp_source, &lpps, &linfo ) == FAILURE )
				nim_error( 0, NULL, NULL, NULL );
			else if ( lpps->type->attr != ATTR_LPP_SOURCE )
					nim_error(	ERR_TYPE_CONFLICT, ATTR_LPP_SOURCE_T,
										lpps->type->name, lpp_source );

			if ( strcmp( location, "/usr" ) != 0 )
			{
				/* install operation will be performed on a non-/usr SPOT */
				/* in this case, we must prevent use of a CDROM as the lpp_source*/
				/*		because, to use CDROM for non-/usr SPOT installs, we */
				/*		would have to mount it inside the inst_root of the SPOT, */
				/*		which could require unmounting the CDROM from its current */
				/*		mount point, remember where it was, mounting it inside the */
				/*		SPOT, do the operation, remount the CDROM at the original */
				/*		mount point.  also, we'd have to deal with issues like */
				/*		the possibilty of the CDROM being exported for use, etc. */
				/* so, is this lpp_source a CDROM? */
				if ( get_attr( NULL, NULL, lpps->id, NULL, 0, ATTR_CDROM ) > 0 )
					nim_error( ERR_CDROM_USE, lpp_source, NULL, NULL );
			}
		}
	}
	/* 
	 * If auto_expand == no and installp_flags sez expand error out ! 
	 */ 
	if ( ( strcmp(auto_expand, "no") == 0 ) && 
		  ( strrchr(flags, 'X') != NULL )    )  
					nim_error( ERR_NO_X, NULL, NULL, NULL );
		
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
*		calls undo_ckspot
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
	
/*---------------------------- inst_spot          ------------------------------
*
* NAME: inst_spot
*
* FUNCTION:
*		performs installp operations within a SPOT
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
inst_spot()

{	int i,j;
	struct nim_if nimif;
	NIM_OBJECT( server, serveri )
	NIM_PDATTR( pdattr, pinfo )
	int rc=0;
	int doing_preview=0;
	FILE *c_stdout=NULL;
	struct res_access *raccess=NULL;
	struct res_access *bundle=NULL;
	char *images=NULL;
	int ignore_lock = TRUE;
	char *bun_path=NULL;

	/* find the server of this SPOT */
	if ( (i = find_attr( spot, NULL, NULL, 0, ATTR_SERVER )) < 0 )
		ERROR( ERR_OBJ_MISSING_ATTR, ATTR_SERVER_T, name, NULL );
	server_name = spot->attrs[i].value;

	/* allocate the resources needed to complete the install */
	/* to do that, we need some info about the server */

	/* get the server's object because we need to do some stuff with it */
	if ( lag_object( 0, server_name, &server, &serveri ) == FAILURE )
		ERROR( 0, NULL, NULL, NULL )

	/* get the server's PIF info */
	if ( (i = find_attr( server, NULL, NULL, 1, ATTR_IF )) < 0 )
		ERROR( ERR_OBJ_MISSING_ATTR, PIF, server->name, NULL )
	if ( s2nim_if( server->attrs[i].value, &nimif ) == FAILURE )
		ERROR( ERR_NET_INFO, PIF, server->name, NULL )

	/* where do the installable images come from? */
	if ( lpp_source_is_device )
	{
		images = nim_malloc( strlen(ATTR_LPP_SOURCE_T) + strlen(lpp_source) + 4 );
		sprintf( images, "-a%s=%s", ATTR_LPP_SOURCE_T, lpp_source );
	}
	else
	{
		/* allocate the lpp_source to the server */
		if (	(alloc_res( server, lpp_source, &nimif, &c_stdout ) == FAILURE) ||
				(add_to_LIST( &alloc_list, lpp_source ) == FAILURE) )
			ERROR( 0, NULL, NULL, NULL )
	}

	/* was installp_bundle or fix_bundle specified? */
	if ( installp_bundle != NULL )
	{
		if (	(alloc_res( server, installp_bundle, &nimif, &c_stdout )==FAILURE)||
				(add_to_LIST( &alloc_list, installp_bundle ) == FAILURE) )
			ERROR( 0, NULL, NULL, NULL )
	}
	else if ( fix_bundle != NULL )
		if (	(alloc_res( server, fix_bundle, &nimif, &c_stdout )==FAILURE)||
				(add_to_LIST( &alloc_list, fix_bundle ) == FAILURE) )
			ERROR( 0, NULL, NULL, NULL )

	/* resources allocated, which means new attrs were added, so re-get it */
	odm_free_list( server, &serveri );
	if ( lag_object( 0, server_name, &server, &serveri ) == FAILURE )
		ERROR( 0, NULL, NULL, NULL )

	/* generate res_access LIST - tells us what interface to use */
	/*		on the server in order to access resources */
	if ( LIST_res_access( server, &res_access_list ) == FAILURE )
		ERROR( 0, NULL, NULL, NULL )

	/* loop through the allocated resources... */
	for (i=0; i < res_access_list.num; i++)
	{	
		raccess = (struct res_access *) res_access_list.list[i];

		/* lpp_source, installp_bundle, or fix_bundle? */
		if ( strcmp( lpp_source, raccess->name ) == 0 )
		{
			/* lpp_source: local or remote? */
			if ( strcmp( raccess->server, server_name ) == 0 )
			{
				/* local resource */
				images = nim_malloc( strlen(ATTR_LPP_SOURCE_T) + 
											strlen(raccess->location) + 4 );
				sprintf( images, "-a%s=%s", ATTR_LPP_SOURCE_T, raccess->location );
			}
			else
			{
				/* remote resource */
				images = nim_malloc( strlen(ATTR_LPP_SOURCE_T) + 
											strlen(raccess->nimif.hostname) +
											strlen(raccess->location) + 5 );
				sprintf( images, "-a%s=%s:%s", ATTR_LPP_SOURCE_T,
							raccess->nimif.hostname, raccess->location );
			}
		}
		else if (	((installp_bundle != NULL) && 
						(strcmp( installp_bundle, raccess->name) == 0))
						||
				((fix_bundle != NULL) && 
						(strcmp( fix_bundle, raccess->name) == 0) ) )
			bundle = (struct res_access *) res_access_list.list[i];
	}
	if ( images == NULL )
		ERROR( ERR_INCOMPLETE, NULL, NULL, NULL );

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

	/* lpp_source */
	if ( add_to_LIST( &params_list, images ) == FAILURE )
		ERROR( 0, NULL, NULL, NULL );

	/* installp_bundle, installp_options, fixes, or fix_bundle? */
	if ( bundle != NULL )
	{
		if ( strcmp( bundle->server, server_name ) == 0 )
		{
			if (installp_bundle != NULL)
				sprintf( params, "-a%s=%s", ATTR_INSTALLP_BUNDLE_T, bundle->location );
			else
				sprintf( params, "-a%s=%s", ATTR_FIX_BUNDLE_T, bundle->location );
			bun_path = nim_malloc( strlen( bundle->location ) + 1 );
			strcpy( bun_path, bundle->location );
		}
		else
		{
			if (installp_bundle != NULL)
				sprintf( params, "-a%s=%s:%s", 
					ATTR_INSTALLP_BUNDLE_T,
					bundle->nimif.hostname, 
					bundle->location );
			else
				sprintf( params, "-a%s=%s:%s", 
					ATTR_FIX_BUNDLE_T,
					bundle->nimif.hostname, 
					bundle->location );
			bun_path = nim_malloc(	
					strlen( bundle->nimif.hostname ) +
					strlen( bundle->location ) + 2 );
			sprintf( bun_path, "%s:%s", 
					bundle->nimif.hostname, 
					bundle->location );
		}
		if ( add_to_LIST( &params_list, params ) == FAILURE )
			ERROR( 0, NULL, NULL, NULL );
	}
	else if ( options != NULL )
	{
		sprintf( params, "-a%s=%s", ATTR_INSTALLP_OPTIONS_T, options );
		if ( add_to_LIST( &params_list, params ) == FAILURE )
			ERROR( 0, NULL, NULL, NULL );
	}
	else if ( fixes != NULL )
	{
		sprintf( params, "-a%s=%s", ATTR_FIXES_T, fixes );
		if ( add_to_LIST( &params_list, params ) == FAILURE )
			ERROR( 0, NULL, NULL, NULL );
	}

	/* if user has specified installp flags, then use them */
	/* otherwise, let the client method use its default installp flag settings */
	if ( flags != NULL )
	{	
		sprintf( params, "-a%s=%s", ATTR_INSTALLP_FLAGS_T, flags );
		if ( add_to_LIST( &params_list, params ) == FAILURE )
			ERROR( 0, NULL, NULL, NULL );
	}

	/* was ATTR_AUTO_EXPAND used? */
	if ( auto_expand != NULL ) {	
		sprintf( params, "-a%s=%s", ATTR_AUTO_EXPAND_T, auto_expand );
		if ( add_to_LIST( &params_list, params ) == FAILURE )
			ERROR( 0, NULL, NULL, NULL );
	}

	VERBOSE("   going to perform an installp operation in the %s:%s SPOT\n",
				server_name,location,NULL,NULL)
	VERBOSE("   this may take some time - please wait\n",NULL,NULL,NULL,NULL)

	/* all ready, now do it! 					*/
	/* NOTE: If requested to  perform a "preview" op for installp, 	*/
	/* 	send output to the screen.  Otherwise, buffer and only 	*/
	/* 	display errors.                                      	*/
	doing_preview = (strstr (flags, "p") != NULL);
	if ( master_exec(	server_name, 
				&rc, 
				doing_preview ? NULL: &c_stdout,
				&params_list.list[0] ) == FAILURE )
		warning( 0, NULL, NULL, NULL );
	else if ( rc > 0 )
		warning( ERR_METHOD, server_name, niminfo.errstr, NULL );

	/* add any attrs which were written to stdout */
	attrs_from_FILE( spot->id, c_stdout );

	/* Perform root sync on any diskless/dataless client which uses this 
	   SPOT.  If fixes or fix_bundle was specified initially, just
	   do "install all" on the root parts -- since it doesn't make
	   sense to install by APAR on the root parts.  

	   If doing preview operation, 
 		don't do root_sync previews, since results will 
           	depend largely upon result of SPOT install.        */
        if (! doing_preview)
	{
		if ((fixes != NULL) || (fix_bundle != NULL))
		{
			options="all";
			bun_path=NULL;
		}
		if ( sync_roots( spot, flags, options, bun_path ) == FAILURE )
			warning( 0, NULL, NULL, NULL );
	}

	/* deallocate the resources allocated for this operation */
	dealloc_LIST( NULL, server, &alloc_list, &c_stdout );

	/* wow - SPOT is installed! */
	return( SUCCESS );

} /* end of inst_spot */
	
/**************************       main         ********************************/

main( int argc, char *argv[] )

{	int rc;
	int i;
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

	VERBOSE("m_instspot: perform install on the %s SPOT\n",name,NULL,NULL,NULL)

	/* get the SPOT's object */
	if ( lag_object( 0, name, &spot, &sinfo ) == FAILURE )
		nim_error( 0, NULL, NULL, NULL );
	if ( spot->type->attr != ATTR_SPOT )
		nim_error(	ERR_OP_NOT_ALLOWED, ATTR_CUST_T, 
						ATTR_msg(spot->type->attr), NULL );

	/* find the location */
	if ( (i = find_attr( spot, NULL, NULL, 0, ATTR_LOCATION )) < 0 )
		nim_error( ERR_OBJ_MISSING_ATTR, ATTR_LOCATION_T, name, NULL );
	location = spot->attrs[i].value;

	/* check for missing attrs */
	ck_attrs();

	/* check the SPOT's state */
	if ( ok_to_op_spot( spot ) == FAILURE )
		nim_error( 0, NULL, NULL, NULL );

	/* if any interrupts occur during execution of this method, we want to */
	/*		check the SPOT and sets its Rstate appropriately */
	undo_on_interrupt = undo_ckspot;

	/* install the required options into the SPOT */
	if ( inst_spot() == FAILURE )
		warning( 0, NULL, NULL, NULL );

	/* check out the SPOT - is it viable? */
	if ( check_spot( name, server_name, &alloc_list ) == FAILURE )
		nim_error( 0, NULL, NULL, NULL );

	/* all done - bye bye */
	exit( 0 );
}

