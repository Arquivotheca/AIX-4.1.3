static char	sccs_id[] = " @(#)95 1.8  src/bos/usr/lib/nim/methods/m_diag.c, cmdnim, bos411, 9434A411a  8/19/94  08:03:44";

/*
 *   COMPONENT_NAME: CMDNIM
 *
 *   FUNCTIONS: ./usr/lib/nim/methods/m_diag.c
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

extern int ch_pdattr_ass();

/*---------------------------- module globals    -----------------------------*/
ATTR_ASS_LIST attr_ass;

VALID_ATTR valid_attrs[] =
{	
	{ATTR_FORCE,				ATTR_FORCE_T,				FALSE,	ch_pdattr_ass},
	{ATTR_VERBOSE,				ATTR_VERBOSE_T,			FALSE,	ch_pdattr_ass},
	{ATTR_PULL_REQUEST,		ATTR_PULL_REQUEST_T,		FALSE,	ch_pdattr_ass},
	{0,							NULL, 						FALSE,	ch_pdattr_ass}
};

VALID_ATTR resources[] =
{	{ATTR_SPOT,					ATTR_SPOT_T,				TRUE,		NULL},
	{0,							NULL, 						0,			NULL}
};

char *name=NULL;					/* NIM name of object to create */
NIM_OBJECT( mobj, minfo )		/* object being operated on */
LIST allocd;						/* res_access LIST for alloc'd res */

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

	/* if "-q" used, it must be the first arg */
	if (! strcmp( argv[1], "-q" ) ) {
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
	
/*---------------------------- ok_to_diag    ------------------------------
*
* NAME: ok_to_diag
*
* FUNCTION:
*		determines whether it's ok to boot the machine in diagnostic mode
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
*			mobj
*			allocd
*			res_served
*
* RETURNS: (int)
*		SUCCESS					= ok to boot diag
*		FAILURE					= not ok - something missing or wrong state or ...
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
ok_to_diag()

{	int i;
	struct res_access *raccess;
	int rc = SUCCESS;

	VERBOSE("   verifying that this operation may be performed at this time\n",
				NULL,NULL,NULL,NULL)

	/* check the Cstate - it must be "ready" */
	if ( (i = find_attr( mobj, NULL, NULL, 0, ATTR_CSTATE )) < 0 )
		ERROR( ERR_BAD_OBJECT, ATTR_msg(ATTR_CLASS_MACHINES), mobj->name, NULL )
	if ( ! same_state( mobj->attrs[i].value, STATE_CREADY ) )
		ERROR( ERR_STATE, mobj->name, NULL, NULL )

	/* remove any missing attrs laying around - ignore the return code because */
	/*		it doesn't really matter whether they get deleted or not */
	rm_attr( mobj->id, NULL, ATTR_MISSING, 0, NULL );

	/* we need to check the allocated resources, so generate the list now */
	if ( LIST_res_access( mobj, &allocd ) == FAILURE )
		return( FAILURE );

	/* verify that all the required resources have been allocated */
	for (i=0; resources[i].name != NULL; i++)
		if (	(resources[i].required) &&
				(find_res_access( resources[i].pdattr, &allocd ) < 0) )
		{	if ( rc == SUCCESS )
			{	/* first error - print message */
				nene( ERR_MISSING_RES, mobj->name, NULL, NULL );
				protect_errstr = TRUE;
			}
			rc = FAILURE;
			mk_attr(	mobj->id, NULL, resources[i].name, 0, 
						ATTR_MISSING, ATTR_MISSING_T);
			fprintf( stderr, "\t%s\n", resources[i].name );
		}

	return( rc );

} /* end of ok_to_diag */

/*---------------------------- diag          ------------------------------
*
* NAME: diag
*
* FUNCTION:
*		initializes the NIM environment to boot a machine in diagnostic mode
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
*			mobj
*			allocd
*
* RETURNS: (int)
*		SUCCESS					= diag resources setup
*		FAILURE					= not ok - something missing or wrong state or ...
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
diag()

{	FILE *c_stdout = NULL;

	/* set the Cstate to diag */
	if ( set_state( mobj->id, NULL, ATTR_CSTATE, STATE_DIAG ) == FAILURE )
		nim_error( 0, NULL, NULL, NULL );

	VERBOSE("   allocating a diagnostic, network boot resource\n",NULL,NULL,
				NULL,NULL)

	/* allocate the network boot resource */
	if ( alloc_res( mobj, ATTR_BOOT_T, NULL, &c_stdout ) == FAILURE )
	{	
		protect_errstr = TRUE;
		Cresult( mobj->name, RESULT_FAILURE );
		nim_error( 0, NULL, NULL, NULL );
	}

	/* all done & EVERYTHING worked! amazing but true */
	return( SUCCESS );

} /* end of diag */

/**************************       main         ********************************/

main(	int argc,
		char *argv[] )

{
	/* initialize NIM environment */
	mstr_init( argv[0], ERR_POLICY_METHOD, NULL );

	/* initialize the LISTs */
	if ( ! get_list_space( &attr_ass, DEFAULT_CHUNK_SIZE, TRUE ) )
		nim_error( 0, NULL, NULL, NULL );
	if ( ! get_list_space( &allocd, DEFAULT_CHUNK_SIZE, TRUE ) )
		nim_error( 0, NULL, NULL, NULL );

	/* parse command line */
	parse_args( argc, argv );

	VERBOSE("m_diag: enable diagnostic network boot for %s\n",name,NULL,NULL,
				NULL)

	/* lock-and-get the object */
	if ( lag_object( 0, name, &mobj, &minfo ) == FAILURE )
		nim_error( 0, NULL, NULL, NULL );

	/* ok to boot diagnostics on this machine? */
	if ( ok_to_diag() == FAILURE )
		nim_error( 0, NULL, NULL, NULL );

	/* do it */
	diag();

	exit( 0 );
}

