static char	sccs_id[] = " @(#)10 1.3  src/bos/usr/lib/nim/methods/m_chstate.c, cmdnim, bos411, 9428A410j  3/31/94  15:35:36";

/*
 *   COMPONENT_NAME: CMDNIM
 *
 *   FUNCTIONS: ./usr/lib/nim/methods/m_chstate.c
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
	{0,							NULL,							FALSE,	ch_pdattr_ass}
};

char *name=NULL;			/* NIM name of object to create */
int result = 0;			/* new Cresult value */
int state = 0;			/* new Mstate value */
	
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
	while ( (c = getopt(argc, argv, "a:qR:S:")) != -1 )
	{	switch (c)
		{	
			case 'a': /* attribute assignment */
				if ( result > 0 )
					syntax_err = TRUE;
				else if (! parse_attr_ass( &attr_ass, valid_attrs, optarg, TRUE ) )
					nim_error( 0, NULL, NULL, NULL );
			break;

			case 'q': /* display valid_attrs */
				cmd_what( valid_attrs );
				exit( 0 );
			break;

			case 'R': /* RESULT value */
				if ( state > 0 )
					syntax_err = TRUE;
				else if ( (result = RESULT_num( optarg )) == 0 )
					nim_error( ERR_VALUE, optarg, ATTR_CRESULT_T, NULL );
			break;

			case 'S': /* STATE value */
				if ( result > 0 )
					syntax_err = TRUE;
				else if ( (state = STATE_num( optarg )) == 0 )
					nim_error( ERR_VALUE, optarg, ATTR_MSTATE_T, NULL );
			break;

			case ':': /* option is missing a required argument */
				nim_error( ERR_MISSING_OPT_ARG, optopt, NULL, NULL );
			break;

			case '?': /* unknown option */
				nim_error( ERR_BAD_OPT, optopt,MSG_msg(MSG_GENERIC_CHSYNTAX),NULL);
			break;
		}

		if ( syntax_err )
			nim_error( ERR_SYNTAX, MSG_msg(MSG_GENERIC_CHSYNTAX), NULL, NULL );
	}

	/* check for errors */
	if ( optind == argc )
		nim_error( ERR_MISSING_OPERAND, MSG_msg(MSG_OBJ_NAME), NULL, NULL );
	else if ( optind < (argc-1) )
		nim_error( ERR_SYNTAX, MSG_msg(MSG_GENERIC_CHSYNTAX), NULL, NULL );

	name = argv[optind];

	/* return */
	return( SUCCESS );

} /* end of parse_args */
	
/**************************       main         ********************************/

main( int argc, char *argv[] )

{	long id;
	int Cstate;
	int Mstate;
	int i;

	/* initialize NIM environment */
	mstr_init( argv[0], ERR_POLICY_METHOD, NULL );
	if ( get_list_space( &attr_ass, DEFAULT_CHUNK_SIZE, TRUE ) == FAILURE )
		nim_error( 0, NULL, NULL, NULL );

	/* parse command line */
	parse_args( argc, argv );

	/* attempt to lock the object */
	/* NOTE that we cannot stop here if we cannot get the lock */
	/* this function is called by the nimclient command, which is executed */
	/*		from the client */
	/* in this environment, the client MUST update it's state, regardless of */
	/*		what's happening on the master */
	lock_object( 0, name );

	/* get the object's id */
	if ( (id = get_id( name )) <= 0 )
		nim_error( 0, NULL, NULL, NULL );

	/* get the current state info */
	Cstate = get_state( id, NULL, ATTR_CSTATE );
	Mstate = get_state( id, NULL, ATTR_MSTATE );

	/* result specified? */
	if ( result )
		Cresult( name, result );
	else
	{
		/* attrs may be provided when the caller proves it knows what's going */
		/*		on by providing the current Cstate */
		if ( attr_ass.num > 0 )
		{
			if ( state != Cstate )
				nim_error( ERR_STATE, name, NULL, NULL );
			for (i=0; i < attr_ass.num; i++)
				ch_attr( id, NULL, attr_ass.list[i]->value, attr_ass.list[i]->seqno,
							attr_ass.list[i]->pdattr, attr_ass.list[i]->name );
		}
		else if (	(state != STATE_SHUTDOWN) && 
						(state != STATE_BOOTING) &&
						(state != STATE_RUNNING) )
			nim_error( ERR_VALUE, optarg, ATTR_MSTATE_T, NULL );

		/* set the new Mstate */
		set_state( id, NULL, ATTR_MSTATE, state );
	}

	exit( 0 );
}
