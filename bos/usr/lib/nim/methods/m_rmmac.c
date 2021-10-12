static char	sccs_id[] = " @(#)01 1.13  src/bos/usr/lib/nim/methods/m_rmmac.c, cmdnim, bos411, 9428A410j  6/22/94  12:28:14";

/*
 *   COMPONENT_NAME: CMDNIM
 *
 *   FUNCTIONS: ./usr/lib/nim/methods/m_rmmac.c
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

/* --------------------------- module globals    ---------------------------- */
ATTR_ASS_LIST attr_ass;

VALID_ATTR valid_attrs[] =
{	
 	{ATTR_FORCE,				ATTR_FORCE_T,				FALSE,	ch_pdattr_ass},
 	{ATTR_VERBOSE,				ATTR_VERBOSE_T,			FALSE,	ch_pdattr_ass},
	{0,							NULL, 						FALSE, NULL}
};

char *name=NULL;
	
/*---------------------------- rmmac             ------------------------------
*
* NAME: rmmac
*
* FUNCTION:
*		removes a machine nim_object & its associated nim_attrs
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
*
* RETURNS: (int)
*		SUCCESS					= object created
*		FAILURE					= error encountered
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
rmmac()

{	NIM_OBJECT( mobj, minfo )
	long id;
	int i;
	int rc;
	FILE *c_stdout=NULL;
	char *args[] = { RM, NIM_ENV_FILE, NULL };
	int ignore_Cstate = FALSE;

	VERBOSE("   verifying that this machine may be removed at this time\n",
				NULL,NULL,NULL,NULL)

	/* lock-and-get the object */
	if ( lag_object( 0, name, &mobj, &minfo ) == FAILURE )
		nim_error( 0, NULL, NULL, NULL );

	/* check removal critiera: */
	/*	1) object must belong to the ATTR_CLASS_MACHINES class */
	if ( mobj->class != ATTR_CLASS_MACHINES )
		nim_error( ERR_OBJ_CLASS, name, ATTR_msg(ATTR_CLASS_MACHINES), NULL );

	/*	2) object cannot be the NIM master */
	if ( mobj->type->attr == ATTR_MASTER )
		nim_error( ERR_CONTEXT, ATTR_REMOVE_T, NULL, NULL );

	/* 3) Cstate must be "ready" or "incomplete" */
	if ( (i = find_attr( mobj, NULL, NULL, 0, ATTR_CSTATE )) < 0 )
	{
		/* internal error has occurred */
		/* if ATTR_FORCE supplied, go ahead and remove object */
		if ( force )
			ignore_Cstate = TRUE;
		else
			nim_error( ERR_BAD_OBJECT, ATTR_msg(ATTR_CLASS_MACHINES), name, NULL );
	}
	if ( ! ignore_Cstate )
	{
		if (	(! same_state( mobj->attrs[i].value, STATE_CREADY )) &&
				(! same_state( mobj->attrs[i].value, STATE_INCOMPLETE )) )
			nim_error( ERR_STATE, name, NULL, NULL );
	}

	/*	4) object cannot be serving any resources */
	if ( find_attr( mobj, NULL, NULL, 0, ATTR_SERVES ) >= 0 )
		nim_error( ERR_SERVING, name, NULL, NULL );

	/*	5) object cannot be using any resources */
	for (i=0; i < mobj->attrs_info->num; i++)
		if ( mobj->attrs[i].pdattr->class == ATTR_CLASS_RESOURCES )
			nim_error( ERR_USES, ATTR_msg(ATTR_REMOVE), name, NULL );

	/* ok to remove the object */

	/* what type of machine? */
	if ( mobj->type->attr == ATTR_STANDALONE )
	{
		/* if target is "running", try to remove its niminfo file */
		if (	((i = find_attr( mobj, NULL, NULL, 0, ATTR_MSTATE )) < 0) ||
				(!  same_state( mobj->attrs[i].value, STATE_RUNNING )) )
			warning( ERR_RM_NIMINFO, name, NULL, NULL );
		else
		{
			if ( (master_exec( name, &rc, &c_stdout, args) == FAILURE) || (rc > 0))
				warning( ERR_RM_NIMINFO, name, NULL, NULL );
		}
	}

	/* ok to remove the object - do it now */
	if (! rm_object( 0, name ) )
		nim_error( 0, NULL, NULL, NULL );

	return( SUCCESS );

} /* end of rmmac */

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
				nim_error( ERR_BAD_OPT, optopt, MSG_msg(MSG_M_RMMAC_SYNTAX), NULL );
			break;
		}

		if ( syntax_err )
			nim_error( ERR_SYNTAX, MSG_msg(MSG_M_RMMAC_SYNTAX), NULL, NULL );
	}

	/* object name (the only operand) is required */
	if ( optind != (argc-1) )
		nim_error( ERR_SYNTAX, MSG_msg(MSG_M_RMMAC_SYNTAX), NULL, NULL );

	name = argv[optind];

	/* return */
	return( SUCCESS );

} /* end of parse_args */

/**************************       main         ********************************/
main( int argc, char *argv[] )
{
	/* initialize NIM environment */
	mstr_init( argv[0], ERR_POLICY_METHOD, NULL );
	if ( get_list_space( &attr_ass, DEFAULT_CHUNK_SIZE, TRUE ) == FAILURE )
		nim_error( 0, NULL, NULL, NULL );

	/* parse command line */
	parse_args( argc, argv );

	VERBOSE("m_rmmac: remove the %s machine\n",name,NULL,NULL,NULL)

	/* remove the specified machine object */
	rmmac();

	exit( 0 );
}
