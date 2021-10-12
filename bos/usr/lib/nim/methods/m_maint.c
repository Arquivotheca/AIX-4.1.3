static char   sccs_id[] = " @(#)96 1.6  src/bos/usr/lib/nim/methods/m_maint.c, cmdnim, bos41J, 9519A_all  5/5/95  15:43:22";
/*
 *   COMPONENT_NAME: CMDNIM
 *
 *   FUNCTIONS: ./usr/lib/nim/methods/m_maint.c
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
{	{ATTR_INSTALLP_FLAGS,	ATTR_INSTALLP_FLAGS_T,	TRUE,		valid_pdattr_ass},
	{ATTR_INSTALLP_OPTIONS,	ATTR_INSTALLP_OPTIONS_T,FALSE,	valid_pdattr_ass},
	{0,							NULL, 						FALSE,	NULL}
};

VALID_ATTR resources[] =
{
	{ATTR_INSTALLP_BUNDLE,	ATTR_INSTALLP_BUNDLE_T,	FALSE,	NULL},
	{0,							NULL, 						0,			NULL}
};

char *name=NULL;							/* NIM name of object to create */
NIM_OBJECT( target, tinfo )				/* object info */
char *flags = NULL;						/* ATTR_INSTALLP_FLAGS */
char *bundle = NULL;						/* ATTR_INSTALLP_BUNDLE */
char *options = NULL;					/* ATTR_INSTALLP_OPTIONS */
LIST alloc_list;							/* LIST of res_access structs */

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

	/* check command line attrs */
	for (i=0; i < attr_ass.num; i++)
		switch (attr_ass.list[i]->pdattr)
		{
			case ATTR_INSTALLP_FLAGS:
				flags = attr_ass.list[i]->value;
			break;

			case ATTR_INSTALLP_OPTIONS:
				options = attr_ass.list[i]->value;
			break;
		}

	if ( flags == NULL )
		nim_error( ERR_MISSING_ATTR, ATTR_INSTALLP_FLAGS_T, NULL, NULL );

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

	/* check for resource query */
	if ( ! strcmp( argv[1], "-q" )) {
		mstr_what(valid_attrs, resources);
		exit(0);
	}

	/* loop through all args */
	while ( (c = getopt(argc, argv, "a:")) != -1 )
	{	switch (c)
		{	
			case 'a': /* attribute assignment */
				if (! parse_attr_ass( &attr_ass, valid_attrs, optarg, FALSE ) )
					nim_error( 0, NULL, NULL, NULL );
				break;

			case ':': /* option is missing a required argument */
				nim_error( ERR_MISSING_OPT_ARG, optopt, NULL, NULL );
				break;

			case '?': /* unknown option */
				nim_error( ERR_BAD_OPT, optopt, MSG_msg(MSG_COP_SYNTAX), NULL );
				break;
		}

		if ( syntax_err )
			nim_error( ERR_SYNTAX, MSG_msg(MSG_COP_SYNTAX), NULL, NULL );
	}

	/* check for errors */
	if ( optind == argc )
		nim_error( ERR_MISSING_OPERAND, MSG_msg(MSG_OBJ_NAME), NULL, NULL );
	else if ( optind < (argc-1) )
		nim_error( ERR_SYNTAX, MSG_msg(MSG_COP_SYNTAX), NULL, NULL );

	name = argv[optind];

	/* return */
	return( SUCCESS );

} /* end of parse_args */
	
/*---------------------------- undo              ------------------------------
*
* NAME: undo
*
* FUNCTION:
*		backs out changes performed by maint
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*		call nim_error
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*			errno					= errno
*			str1					= str1 for error msg
*			str2					= str2 for error msg
*			str3					= str3 for error msg
*		global:
*
* RETURNS: (int)
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
undo(	int errno,
		char *str1,
		char *str2,
		char *str3 )

{	FILE *c_stdout = NULL;

	if ( errno > 0 )
		errstr( errno, str1, str2, str3 );

	protect_errstr = TRUE;

	/* NOTE that Cresult will remove the script during the state trans */
	Cresult( target->name, RESULT_FAILURE );

	/* deallocate installp_bundle? */
	if ( bundle != NULL )
		dealloc_res( target, bundle, &c_stdout );

	nim_error( 0, NULL, NULL, NULL );

} /* end of undo */
	
/*---------------------------- maint         ------------------------------
*
* NAME: maint
*
* FUNCTION:
*		performs software maintenance
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*		calls nim_error on failure
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*		global:
*
* RETURNS: (int)
*		SUCCESS					= software maintenance performed
*		FAILURE					= error
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
maint()

{	int i,j;
	int rc;
	FILE *c_stdout = NULL;
	char *args[] = { C_INSTALLP, NULL, NULL, NULL };
	char *installp_bundle = NULL;
	struct nim_if nimif;
	struct res_access *raccess;
	int instp_preview;

	/* check the Cstate & Mstate */
	if ( (i = find_attr( target, NULL, NULL, 0, ATTR_CSTATE )) < 0 )
		nim_error( ERR_OBJ_MISSING_ATTR, ATTR_CSTATE_T, target->name, NULL );

	else if ( ! same_state( target->attrs[i].value, STATE_CREADY ) )
		nim_error( ERR_OP_NOT_NOW, ATTR_MAINT_T, ATTR_CSTATE_T,
                  STATE_msg(target->attrs[i].value) );

	else if ( (j = find_attr( target, NULL, NULL, 0, ATTR_MSTATE )) < 0 )
		nim_error( ERR_OBJ_MISSING_ATTR, ATTR_MSTATE_T, target->name, NULL );

	else if ( ! same_state( target->attrs[j].value, STATE_RUNNING ) )
		nim_error( ERR_OP_NOT_NOW, ATTR_MAINT_T, ATTR_MSTATE_T,
                  STATE_msg(target->attrs[j].value) );

	/* use an installp_bundle? */
	if ( (i = find_attr( target, NULL, NULL, 0, ATTR_INSTALLP_BUNDLE )) >= 0 )
	{
		bundle = target->attrs[i].value;

		/* check for conflicts */
		if ( options != NULL )
			nim_error(  ERR_ATTR_CONFLICT, ATTR_INSTALLP_BUNDLE_T,
							ATTR_INSTALLP_OPTIONS_T, NULL);

		/* gen LIST of res_access structs */
		if ( LIST_res_access( target, &alloc_list ) == FAILURE )
			undo( 0, NULL, NULL, NULL );

		/* loop through the allocated resources... */
		for (i=0; i < alloc_list.num; i++)
		{
			raccess = (struct res_access *) alloc_list.list[i];
			if ( strcmp( raccess->name, bundle ) == 0 )
			{
				/* local or remote access? */
				if ( strcmp( raccess->server, target->name ) == 0 )
				{
					installp_bundle = nim_malloc( strlen(ATTR_INSTALLP_BUNDLE_T) +
															strlen(raccess->location) + 4 );
					sprintf( installp_bundle, "-a%s=%s", ATTR_INSTALLP_BUNDLE_T,
								raccess->location );
				}
				else
				{
					installp_bundle = nim_malloc( strlen(ATTR_INSTALLP_BUNDLE_T) +
															strlen(raccess->nimif.hostname) +
															strlen(raccess->location) + 5 );
					sprintf( installp_bundle, "-a%s=%s:%s", ATTR_INSTALLP_BUNDLE_T,
								raccess->nimif.hostname, raccess->location );
				}
			}
		}
	} /* if bundle */

	/* prepare for interrupts */
	undo_on_interrupt = undo;

	/* set the Cstate */
	if ( set_state( target->id, NULL, ATTR_CSTATE, STATE_MAINT ) == FAILURE )
		undo( 0, NULL, NULL, NULL );

	/* initialize the parameters for C_INSTALLP */


	/* installp_flags */
	args[1] = nim_malloc( strlen(ATTR_INSTALLP_FLAGS_T) + strlen(flags) + 4 );
	sprintf( args[1], "-a%s=%s", ATTR_INSTALLP_FLAGS_T, flags );

        instp_preview = (strstr (flags, "p") != NULL);

	/* installp_bundle? */
	if ( installp_bundle != NULL )
		args[2] = installp_bundle;
	else if ( options != NULL )
	{
		args[2] = nim_malloc( strlen(ATTR_INSTALLP_OPTIONS_T)+strlen(options)+4);
		sprintf( args[2], "-a%s=%s", ATTR_INSTALLP_OPTIONS_T, options );
	}
									
        /* NOTE: If requested to  perform a "preview" op for installp,  */
        /*      send output to the screen.  Otherwise, buffer and only  */
        /*      display errors.                                         */
	if ( master_exec(	target->name, 
				&rc, 
				instp_preview ? NULL: &c_stdout,
				args ) == FAILURE )
		undo( 0, NULL, NULL, NULL );
	else if ( rc > 0 )
		undo( ERR_METHOD, target->name, niminfo.errstr, NULL );

	/* all done */
	Cresult( target->name, RESULT_SUCCESS );

	/* deallocate installp_bundle? (don't if just did a preview) */
	if (	(bundle != NULL) &&  (! instp_preview) &&
			(dealloc_res( target, bundle, &c_stdout ) == FAILURE) )
		warning( 0, NULL, NULL, NULL );

	undo_on_interrupt = NULL;
	return( SUCCESS );

} /* end of maint */
	
/**************************       main         ********************************/

main(	int argc,
		char *argv[] )

{
	/* initialize NIM environment */
	mstr_init( argv[0], ERR_POLICY_METHOD, NULL );

	/* initialize the LISTs */
	if ( ! get_list_space( &attr_ass, DEFAULT_CHUNK_SIZE, TRUE ) )
		nim_error( 0, NULL, NULL, NULL );

	/* parse command line */
	parse_args( argc, argv );

	/* check for missing attrs */
	ck_attrs();

	/* lock-and-get the object */
	if ( lag_object( 0, name, &target, &tinfo ) == FAILURE )
		nim_error( 0, NULL, NULL, NULL );

	/* make sure this is a machine object */
	if ( target->class != ATTR_CLASS_MACHINES )
		nim_error( ERR_OBJ_CLASS, name, ATTR_CLASS_MACHINES_T, NULL, NULL );

	/* perform software maintenance */
	maint();

	exit( 0 );
}

