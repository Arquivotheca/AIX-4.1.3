static char   sccs_id[] = " @(#)81 1.17  src/bos/usr/lib/nim/methods/m_cust.c, cmdnim, bos41J, 9519A_all  5/5/95  15:43:51";

/*
 *   COMPONENT_NAME: CMDNIM
 *
 *   FUNCTIONS: ./usr/lib/nim/methods/m_cust.c
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

extern int ch_pdattr_ass();

/*---------------------------- module globals    -----------------------------*/
ATTR_ASS_LIST attr_ass;

VALID_ATTR valid_attrs[] =
{	
	{ATTR_PULL_REQUEST,	ATTR_PULL_REQUEST_T,	FALSE,	ch_pdattr_ass},
	{ATTR_INSTALLP_FLAGS,	ATTR_INSTALLP_FLAGS_T,	FALSE,	ch_pdattr_ass},
	{ATTR_FIXES,		ATTR_FIXES_T,		FALSE,	ch_pdattr_ass},
	{ATTR_INSTALLP_OPTIONS,	ATTR_INSTALLP_OPTIONS_T,FALSE,	ch_pdattr_ass},
	{0,			NULL,			FALSE,	ch_pdattr_ass}
};

VALID_ATTR resources[] =
{		
	{ATTR_LPP_SOURCE,	ATTR_LPP_SOURCE_T,	FALSE,		NULL},
	{ATTR_INSTALLP_BUNDLE,	ATTR_INSTALLP_BUNDLE_T,	FALSE,		NULL},
	{ATTR_FIX_BUNDLE,	ATTR_FIX_BUNDLE_T,	FALSE,		NULL},
	{ATTR_SCRIPT,		ATTR_SCRIPT_T,		FALSE,		NULL},
	{0,			NULL, 			0,		NULL}
};

char *name=NULL;							/* NIM name of object to create */
NIM_OBJECT( client, cinfo )			/* object info */
int client_initiated=FALSE;			/* >0 if client initiated this processing */
char location[MAX_TMP];					/* pathname of nim_script */
char *installp_flags=NULL;

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

{	int i;

	for (i=0; i < attr_ass.num; i++)
		if ( attr_ass.list[i]->pdattr == ATTR_PULL_REQUEST )
			client_initiated = TRUE;
		else
		   if ( attr_ass.list[i]->pdattr == ATTR_INSTALLP_FLAGS )
			installp_flags = attr_ass.list[i]->value;

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
		exit( 0 );
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
*		backs out changes performed by customize
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

void
undo(	int errno,
		char *str1,
		char *str2,
		char *str3 )

{
	disable_err_sig();

	if ( errno > 0 )
		errstr( errno, str1, str2, str3 );

	protect_errstr = TRUE;

	/* NOTE that the scheduled transition from "cust" to "any" will */
	/*		remove any allocated resources */
	Cresult( client->name, RESULT_FAILURE );

	nim_error( 0, NULL, NULL, NULL );

} /* end of undo */
	
/*---------------------------- customize         ------------------------------
*
* NAME: customize
*
* FUNCTION:
*		performs software customization
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
*		global:
*
* RETURNS: (int)
*		SUCCESS					= customization performed
*		FAILURE					= error
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
customize()

{	int i,j;
	FILE *fp = NULL;
	char tmp[MAX_VALUE];
	int ch;
	int rc;
	FILE *c_stdout = NULL;
	char *args[] = { C_SCRIPT, tmp, NULL };
	char **ptr = NULL;
	int len = 0;
	int instp_preview;

	/* check the Cstate & Mstate */
	if ( (i = find_attr( client, NULL, NULL, 0, ATTR_CSTATE )) < 0 )
		nim_error( ERR_OBJ_MISSING_ATTR, ATTR_CSTATE_T, client->name, NULL );
	else if (! same_state( client->attrs[i].value, STATE_CREADY ) )
		nim_error( ERR_OP_NOT_NOW, ATTR_CUST_T, ATTR_CSTATE_T, 
						STATE_msg(client->attrs[i].value) );
	else if ( (j = find_attr( client, NULL, NULL, 0, ATTR_MSTATE )) < 0 )
		nim_error( ERR_OBJ_MISSING_ATTR, ATTR_MSTATE_T, client->name, NULL );
	else if (! same_state( client->attrs[j].value, STATE_RUNNING ) )
		nim_error( ERR_OP_NOT_NOW, ATTR_CUST_T, ATTR_MSTATE_T,
						STATE_msg((int) strtol(client->attrs[j].value, NULL, 0 ) ) );

	/* change the Cstate to "cust" */
	if ( set_state( client->id, NULL, ATTR_CSTATE, STATE_CUST ) == FAILURE )
		nim_error( 0, NULL, NULL, NULL );

	/* remove old state info */
	rm_attr( client->id, NULL, ATTR_MISSING, 0, NULL );
	rm_attr( client->id, NULL, ATTR_INFO, 0, NULL );
	rm_attr( client->id, NULL, ATTR_CRESULT, 0, NULL );
	rm_attr( client->id, NULL, ATTR_TRANS, 0, NULL );

	/* prepare for interrupts */
	undo_on_interrupt = undo;

        instp_preview = (strstr (installp_flags, "p") != NULL);

	/* Cstate change from "any" to "ready" -> deallocate all resources */
	/* note that ATTR_IGNORE_LOCK is used; this is required because */
	/*		this event will be executed in an environment in which the target */
	/*		will have already been locked by M_CHSTATE, so we must bypass the */
	/*		check in lock_object */
	/* ALSO: If doing an installp preview operation, we only want to */
	/*		deallocate the nim_script.  We don't want to     */ 
	/*		deallocate the rest of the user's resources. 	 */
	if ( client_initiated )
		if (instp_preview)
			sprintf(	tmp, "-o %s -F -a%s=yes -a%s=%s -a%s=yes %s", 
					ATTR_DEALLOCATE_T, ATTR_IGNORE_LOCK_T,
					ATTR_SCRIPT_T, ATTR_NIM_SCRIPT_T, ATTR_PULL_REQUEST_T, client->name );
		else
			sprintf(	tmp, "-o %s -F -a%s=yes -a%s=%s -a%s=yes %s", 
					ATTR_DEALLOCATE_T, ATTR_IGNORE_LOCK_T,
					ATTR_SUBCLASS_T, ATTR_ALL_T, ATTR_PULL_REQUEST_T, client->name );
	else
		if (instp_preview)
			sprintf(	tmp, "-o %s -F -a%s=yes -a%s=%s %s", 
					ATTR_DEALLOCATE_T, ATTR_IGNORE_LOCK_T,
					ATTR_SCRIPT_T, ATTR_NIM_SCRIPT_T, client->name );
		else
			sprintf(	tmp, "-o %s -F -a%s=yes -a%s=%s %s", 
					ATTR_DEALLOCATE_T, ATTR_IGNORE_LOCK_T,
					ATTR_SUBCLASS_T, ATTR_ALL_T, client->name );
   if ( add_trans_event(   client->id, NULL, ATTR_CSTATE,
                           STATE_ANY, STATE_CREADY, ATTR_MASTER_T,
                           NIM, tmp ) == FAILURE )
      undo( 0, NULL, NULL, NULL );

	/* since the Cstate is now "cust", add the ATTR_IGNORE_STATE attr so */
	/*		we can allocate stuff */
	if ( add_attr(	&attr_ass, ATTR_IGNORE_STATE, 
						ATTR_IGNORE_STATE_T, "yes" ) == FAILURE )
		undo( 0, NULL, NULL, NULL );

	/* this is not a bos_inst operation, so we don't want the */
	/*		bos.sysmgt.nim.client package installed */
	if ( add_attr(	&attr_ass, ATTR_NO_NIM_CLIENT,
						ATTR_NO_NIM_CLIENT_T, "yes" ) == FAILURE )
		undo( 0, NULL, NULL, NULL );

	/* is a client "pull"ing this customization? */
	if ( client_initiated )
	{
		/* the Cstate will be updated asynchronously */
		if ( add_attr_ass( &attr_ass, ATTR_ASYNC, ATTR_ASYNC_T, "yes",0)==FAILURE)
			undo( 0, NULL, NULL, NULL );
	}

	/* we want the access point written to stdout so that we can tell the */
	/*		client how to access the script we're going to create */
	if ( add_attr(	&attr_ass, ATTR_DISP_MNTPNT,
						ATTR_DISP_MNTPNT_T, "yes" ) == FAILURE )
		undo( 0, NULL, NULL, NULL );

	/* since the access point is being written to the method's stdout, we */
	/*		want to redirect it into a file so that we can process that info */
	/*		further, so, open a temporary file and use that FILE ptr when */
	/*		invoking the method */
	sprintf( tmp, "%s/ns.info", niminfo.tmps );
	if ( (fp = fopen( tmp, "w+" )) == NULL )
		undo( ERR_ERRNO, NULL, NULL, NULL );

	VERBOSE("   preparing a nim_script to perform the customization\n",NULL,
				NULL,NULL,NULL)

	/* allocate a nim_script for this client and pass the FILE ptr to the */
	/*		tmp file so we get that output trapped */
	if ( alloc_res( client, ATTR_NIM_SCRIPT_T, NULL, &fp ) == FAILURE )
		undo( 0, NULL, NULL, NULL );

	/* initialize first part of parameter string for C_SCRIPT */
	sprintf( tmp, "-a%s=", ATTR_LOCATION_T );
	rc = strlen(tmp);

	/* rewind tmp file */
	if ( fseek( fp, 0, SEEK_SET ) == -1 )
		undo( ERR_ERRNO, NULL, NULL, NULL );

	/* skip past '=' */
	while ( ((ch = fgetc( fp )) != EOF) && (ch != '=') );

	/* rest of the chars are script's access point - read it in */
	if ( fgets( tmp+rc, (MAX_VALUE-rc), fp ) == NULL )
		undo( ERR_ERRNO, NULL, NULL, NULL );

	/* remove the '\n' if it was returned as part of the script's location */
	len = strlen( tmp );
	if ( tmp[ len - 1 ] == '\n' )
		tmp[ --len ] = NULL_BYTE;

	/* since we set ATTR_DISP_MNTPNT before we called alloc_res, an attr */
	/*		assignment got returned on stdout */
	/* as a result, that assignment got added to the target's definition, */
	/*		however, we don't want this as part of the definition */
	/* so, delete the ATTR_NIM_SCRIPT which has the script's location as */
	/*		it's value */
	rm_attr( client->id, NULL, ATTR_NIM_SCRIPT, 0, tmp+rc );

	/* if this operation was initiated from the client, then write the command */
	/*		to be executed to stdout; otherwise, execute it now */
	if ( client_initiated )
	{
		disable_err_sig();
		undo_on_interrupt = NULL;
		for (ptr=args; *ptr != NULL; ptr++)
			printf( "%s ", *ptr );
		printf( "\n" );
	}
	else
	{
		VERBOSE("   executing the nim_script on %s\n",client->name,NULL,NULL,NULL)
		/* execute C_SCRIPT on the client.  NOTE: If requested to  */
		/*    perform a "preview" op for installp, send output to  */
		/*    the screen.  Otherwise, buffer and only report errs, */

		if ( master_exec( client->name, 
				  &rc, 
				  instp_preview ? NULL: &c_stdout, 
				  args ) == FAILURE )
			undo( 0, NULL, NULL, NULL );
		else if (rc > 0)
			undo( ERR_METHOD, client->name, niminfo.errstr, NULL );
		
		/* all done & successful except for admin stuff */
		/* an interrupt here would be deadly, so don't allow it */
		disable_err_sig();
		undo_on_interrupt = NULL;

		/* finish operation */
		/* NOTE that Cresult will remove the script as part of the state trans */
		Cresult( client->name, RESULT_SUCCESS );
	}

	/* success! */
	return( SUCCESS );

} /* end of customize */
	
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

	VERBOSE("m_cust: perform customization on the %s machine\n",name,NULL,NULL,
				NULL)

	/* check for missing attrs */
	ck_attrs();

	/* lock-and-get the object */
	if ( lag_object( 0, name, &client, &cinfo ) == FAILURE )
		nim_error( 0, NULL, NULL, NULL );

	/* perform customization */
	customize();

	exit( 0 );
}

