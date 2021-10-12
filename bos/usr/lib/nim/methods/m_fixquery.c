static char   sccs_id[] = " @(#)24 1.1  src/bos/usr/lib/nim/methods/m_fixquery.c, cmdnim, bos41J, 9511A_all  3/2/95  16:07:04";

/*
 *   COMPONENT_NAME: CMDNIM
 *
 *   FUNCTIONS: ./usr/lib/nim/methods/m_fixquery.c
 *		
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "cmdnim_mstr.h"
#include <swvpd.h>

extern int valid_pdattr_ass();
extern int ch_pdattr_ass();

/*---------------------------- module globals    -----------------------------*/
ATTR_ASS_LIST attr_ass;

VALID_ATTR valid_attrs[] =
{  
	{ATTR_FIXES,   		ATTR_FIXES_T,  		FALSE,   valid_pdattr_ass},
	{ATTR_FIX_BUNDLE,   	ATTR_FIX_BUNDLE_T,  	FALSE,   valid_pdattr_ass},
	{ATTR_FIXQUERY_FLAGS,   ATTR_FIXQUERY_FLAGS_T,  FALSE,   valid_pdattr_ass},
	{0,			NULL,			FALSE,   NULL}
};


char *name=NULL;			/* fix_query target */
char *fix_bundle=NULL;			/* bundle of fix keywords */
char *fixes=NULL;			/* list of fix keywords */
char *flags;				/* list of instfix flags */
LIST alloc_list;			/* list of allocated resources */
	
/*---------------------------- ck_attrs        ------------------------------
*
* NAME: ck_attrs
*
* FUNCTION:
*		check for validity of attributes specfied to fix_query
*		operation
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*		calls nim_error
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		global:
*			fixes			= list of keywords
*			fix_bundle		= keyword file
*			flags 			= instfix flags
*
* RETURNS: (int)
*		SUCCESS if no problems with attrs, else errors off.
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
ck_attrs()
{	int i;
        NIM_OBJECT( bun, binfo )

	/* loop through the attrs */
	for (i=0; i < attr_ass.num; i++)
		switch (attr_ass.list[i]->pdattr)
		{	
			case ATTR_FIXQUERY_FLAGS:
				/* has flags already been specified? */
				if ( flags != NULL )
					nim_error(	ERR_ONLY_ONE_ATTR, 
							ATTR_FIXQUERY_FLAGS_T, 
							NULL, NULL);
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

		}

        /* Check for attribute conflicts. */
	if ( (fixes != NULL)  && (fix_bundle != NULL) )
                nim_error(      ERR_ATTR_CONFLICT, 
				ATTR_FIXES_T, 
				ATTR_FIX_BUNDLE_T, NULL);

	return (SUCCESS);

} /* end ck_attrs */
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
*		SUCCESS				= no syntax errors on cmd line
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
            			if (! parse_attr_ass( 	&attr_ass, 	
							valid_attrs, 
							optarg, FALSE ) )
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
				nim_error( ERR_BAD_OPT, optopt, MSG_msg(MSG_GENERIC_RMSYNTAX),NULL);
			break;
		}

	}

	/* object name (the only operand) is required */
	if ( optind != (argc-1) )
		nim_error( ERR_SYNTAX, MSG_msg(MSG_GENERIC_RMSYNTAX), NULL, NULL );

	name = argv[optind];

	/* return */
	return( SUCCESS );

} /* end of parse_args */

/*---------------------------- fix_query             --------------------------
*
* NAME: fix_query
*
* FUNCTION:
*		lists installed fix information on the target 
*					(standalone machine or SPOT)
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*		Calls nim_error
*		Calls C_CKSPOT to do the real work which ultimately calls
*		 	the instfix command.
* 			We use C_CKSPOT regardless of the target being a SPOT or
* 			client for convenience.  If it's a client, we simply 
*			treate the /usr filesystem of the client as if it were
* 			a SPOT on a remote server.
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*		global:
*			name				= target name
*			fixes				= fix keyword list
*			fix_bundle			= keyword file
*			flags				= optional instfix flags
*
* RETURNS: (int)
*		SUCCESS					= info displayed
*		FAILURE					= error encountered
*
* OUTPUT:
*		fix info written to stdout
*-----------------------------------------------------------------------------*/

int
fixquery()

{	NIM_OBJECT( obj, info )
	LIST res_access_list;
	int i;
	char *location;
	char *server;
	int rc;
	FILE *c_stdout=NULL;
	char st_applied[MAX_TMP];
	char st_committed[MAX_TMP];
	char *pathname=NULL;
	char objname[MAX_TMP];
	char *args[] = { C_CKSPOT, "-l", "-f", st_applied, st_committed, NULL, 
			 NULL, NULL, NULL, NULL, NULL};
        struct nim_if nimif;
	char line[MAX_VALUE];
        struct res_access *raccess=NULL;
        struct res_access *bundle=NULL;
	int argcnt = 4;

	/* get info target object's info */
	if ( lag_object( 0, name, &obj, &info ) == FAILURE )
		nim_error( 0, NULL, NULL, NULL );

	if ( (obj->type->attr == ATTR_STANDALONE)  &&

	/* If neither bundles nor fixes were specified on the command line */
	/* 	see if there's one allocated that we can work with.  This  */
	/* 	allows bundles to be specified or allocated.  If there's   */
	/*	one allocated and fixes or fix_bundle is specified on the  */
	/*	comman line, then the command line wins out.		   */

	     (fixes == NULL)                       &&
	     (fix_bundle == NULL) )
	{
		/* generate res_access LIST */
		if ( LIST_res_access( obj, &res_access_list ) == FAILURE )
			ERROR( 0, NULL, NULL, NULL )

		/* See if there's a fix_bundle allocated. */
		for (i=0; i < res_access_list.num; i++)
		{
			raccess = (struct res_access *) res_access_list.list[i];
			if ( raccess->type == ATTR_FIX_BUNDLE )
				fix_bundle = raccess->name;
		}
	}
		
	/* Set up the command line to C_CKSPOT. */

	/* The following parameters are required by C_CKSPOT but not used*/
	/* for fix listing purposes:					 */
	sprintf( st_applied, "-a%s=%d", ATTR_ST_APPLIED_T, ST_APPLIED );
	sprintf( st_committed, "-a%s=%d", ATTR_ST_COMMITTED_T, ST_COMMITTED );
	args[ ++argcnt ] = (char *) nim_malloc( strlen(ATTR_PLATFORM_T) + 7 );
	sprintf( args[ argcnt ], "-a%s=yes", ATTR_PLATFORM_T );

	/* object's name */
	sprintf( objname, "-a%s=%s", ATTR_NAME_T, name );
	args[ ++argcnt ] = objname;

	/* Set up flags and keyword list if necessary */
	if (flags != NULL)
	{
		args[ ++argcnt ] = (char *) nim_malloc( 
					strlen(ATTR_FIXQUERY_FLAGS_T) + 
					strlen(flags) + 4 );
		sprintf(args[argcnt], "-a%s=%s", ATTR_FIXQUERY_FLAGS_T, flags);
	}
	if (fixes != NULL)
	{
		args[ ++argcnt ] = (char *) nim_malloc( 
					strlen(ATTR_FIXES_T) + 
					strlen(fixes) + 4 );
		sprintf(args[argcnt], "-a%s=%s", ATTR_FIXES_T, fixes);
	}

	/* Set up pathname to SPOT (remember, we'll treat standalone   */
	/* targets like /usr SPOTs).                                   */
	if ( obj->type->attr == ATTR_SPOT )
	{	
		/* get SPOT's location */
   		if ( (i = find_attr( obj, NULL, NULL, 0, ATTR_LOCATION )) < 0 )
			nim_error( 	ERR_OBJ_MISSING_ATTR, 
					ATTR_LOCATION_T, name, NULL );
		location = obj->attrs[i].value;
		pathname = (char *) nim_malloc( strlen(ATTR_LOCATION_T) 
					+ strlen(location) + 4 );
		sprintf( pathname, "-a%s=%s", ATTR_LOCATION_T, location );

		/* get the SPOT server's name */
   		if ( (i = find_attr( obj, NULL, NULL, 0, ATTR_SERVER )) < 0 )
			nim_error( 	ERR_OBJ_MISSING_ATTR, 
					ATTR_SERVER_T, name, NULL );
		server = obj->attrs[i].value;

	}
	else if (	(obj->type->attr == ATTR_STANDALONE) ||
					(obj->type->attr == ATTR_MASTER) )
	{
		/* location is always /usr */
		pathname = (char *)nim_malloc( strlen(ATTR_LOCATION_T) + 8 );
		sprintf( pathname, "-a%s=/usr", ATTR_LOCATION_T );

		/* server is always the machine itself */
		server = name;

	}
	else
		nim_error(	ERR_OP_NOT_ALLOWED, 
				ATTR_FIXQUERY_T, 
				ATTR_msg(obj->type->attr),
				NULL );
	args[ ++argcnt ] = pathname;

	/* Allocate a fix_bundle if one was specified. */
	if (fix_bundle != NULL)
	{
		/* Get the server's object (if this is a spot target 	*/
		/* we don't have that object yet -- we have the spot).	*/
		if ( lag_object( 0, server, &obj, &info ) == FAILURE )
			ERROR( 0, NULL, NULL, NULL )

        	/* get the server's PIF info */
        	if ( (i = find_attr( obj, NULL, NULL, 1, ATTR_IF )) < 0 )
                	ERROR( ERR_OBJ_MISSING_ATTR, PIF, obj->name, NULL )
        	if ( s2nim_if( obj->attrs[i].value, &nimif ) == FAILURE )
                	ERROR( ERR_NET_INFO, PIF, obj->name, NULL )

		/* allocate the bundle. */
                if (    (alloc_res( obj, fix_bundle, &nimif, &c_stdout )
==FAILURE)||
                                (add_to_LIST( &alloc_list, fix_bundle ) ==
FAILURE) )
                        ERROR( 0, NULL, NULL, NULL )

		/* resources allocated, which means new attrs were added, 
		   so re-get object */
		odm_free_list( obj, &info );
		if ( lag_object( 0, server, &obj, &info ) == FAILURE )
			ERROR( 0, NULL, NULL, NULL )

		/* generate res_access LIST - tells us what interface to use */
		/*              on the server in order to access resources */
		if ( LIST_res_access( obj, &res_access_list ) == FAILURE )
			ERROR( 0, NULL, NULL, NULL )


		/* Get the access structure for the bundle. */
		for (i=0; i < res_access_list.num; i++)
		{
			raccess = (struct res_access *) res_access_list.list[i];
			if (strcmp( fix_bundle, raccess->name) == 0) 
				bundle = (struct res_access *) res_access_list.list[i];
		}

		/* establish the path name to the bundle, depending upon */
		/* whether or not it's local or remote.			 */
		if ( strcmp( bundle->server, server ) == 0 )
		{
			args[++argcnt] = (char *)nim_malloc(  
						strlen (ATTR_FIX_BUNDLE_T) + 
						strlen( bundle->location ) + 4 );
			sprintf( args[argcnt], "-a%s=%s", ATTR_FIX_BUNDLE_T,
						bundle->location );
		}
		else
		{
			args[++argcnt] = (char *)nim_malloc(  
						strlen (ATTR_FIX_BUNDLE_T) + 
						strlen( bundle->nimif.hostname ) +
						strlen( bundle->location ) + 5 );
			sprintf( args[argcnt], "-a%s=%s:%s", ATTR_FIX_BUNDLE_T,
				bundle->nimif.hostname, bundle->location );
		}
	}
	
	/* execute C_CKSPOT */
	if (  (master_exec(  server, &rc, NULL, &args[0] ) == FAILURE) ||
		(rc > 0) )
		warning( ERR_METHOD, server, niminfo.errstr, NULL );

        /* deallocate the resources allocated for this operation */
        dealloc_LIST( NULL, obj, &alloc_list, &c_stdout );

	return ( SUCCESS );

} /* end of fixquery */

/**************************       main         ********************************/

main( int argc, char *argv[] )
{
	/* initialize NIM environment */
	mstr_init( argv[0], ERR_POLICY_METHOD, NULL );

	/* initialize the attr_ass LIST */
	if ( ! get_list_space( &attr_ass, DEFAULT_CHUNK_SIZE, TRUE ) )
		nim_error( 0, NULL, NULL, NULL );

	/* initialize the alloc_list LIST */
	if ( ! get_list_space( &alloc_list, DEFAULT_CHUNK_SIZE, TRUE ) )
		nim_error( 0, NULL, NULL, NULL );

	/* parse command line */
	parse_args( argc, argv );

	ck_attrs();

	/* display the fix info */
	fixquery();

	exit (0);
}
