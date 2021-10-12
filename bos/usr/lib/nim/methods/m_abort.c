static char	sccs_id[] = " @(#)89 1.12  src/bos/usr/lib/nim/methods/m_abort.c, cmdnim, bos411, 9428A410j  6/14/94  15:14:08";

/*
 *   COMPONENT_NAME: CMDNIM
 *
 *   FUNCTIONS: ./usr/lib/nim/methods/m_abort.c
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

char *name=NULL;									/* target object name */
NIM_OBJECT( obj, info )							/* target's object */
int dealloc_boot = FALSE;						/* TRUE if boot deallocated */
int dealloc_ns = FALSE;							/* TRUE if nim_script present */

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
	
/*---------------------------- unschedule        ------------------------------
*
* NAME: unschedule
*
* FUNCTION:
*		backs out a scheduled operation
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
*			obj					= ptr to nim_object struct
*		global:
*
* RETURNS: (int)
*		SUCCESS					= operation unscheduled
*		FAILURE					= ODM error?
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
unschedule(	struct nim_object *obj )

{	int rc;
	FILE *c_stdout = NULL;
	char *args[] = { AT, "-r", NULL, NULL };
	char *job = NULL;
	int i;

	/* get the cron job number */
	if ( (i = find_attr( obj, NULL, NULL, 0, ATTR_AT_JOB )) < 0 )
		ERROR( ERR_OBJ_MISSING_ATTR, ATTR_AT_JOB_T, obj->name, NULL )
	job = obj->attrs[i].value;

	/* remove the cron job */
	args[2] = job;
	if (	(client_exec( &rc, &c_stdout, args ) == FAILURE) ||
			(rc > 0) )
		warning( ERR_CMD, AT, niminfo.errstr, NULL );

	/* remove the ATTR_AT attribute */
	rm_attr( obj->id, NULL, ATTR_AT, 0, NULL );
	rm_attr( obj->id, NULL, ATTR_AT_ARG, 0, NULL );
	rm_attr( obj->id, NULL, ATTR_AT_JOB, 0, NULL );

	return( SUCCESS );

} /* end of unschedule */
	
/*---------------------------- undo              ------------------------------
*
* NAME: undo
*
* FUNCTION:
*		puts back the environment the way it was before this operation was
*			performed
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
*			errno					= error code
*			str1					= str1 of error msg
*			str2					= str2 of error msg
*			str3					= str3 of error msg
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

{	FILE *c_stdout = NULL;
	int tmp_err = 0;

	disable_err_sig();

	/* set the errstr */
	if ( errno > 0 )
		errstr( errno, str1, str2, str3 );
	tmp_err = nene( 0, NULL, NULL, NULL );

	/* re-get the target's object so that <obj> reflects current data */
	if ( lag_object( obj->id, NULL, &obj, &info ) == FAILURE )
		warning( 0, NULL, NULL, NULL );

	/* need to re-alloc the boot resource? */
	if (	(dealloc_boot) &&
			(alloc_res( obj, ATTR_BOOT_T, NULL, &c_stdout ) == FAILURE) )
		warning( 0, NULL, NULL, NULL );

	exit( tmp_err );

} /* end of undo */

/*---------------------------- abort             ------------------------------
*
* NAME: abort
*
* FUNCTION:
*		checks to make sure that the current control action may be aborted and,
*			if so, takes the specified machine back to the "ready" state
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
*		SUCCESS					= x
*		FAILURE					= x
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
abort()

{	int Cstate;
	int Mstate;
	FILE *c_stdout = NULL;

	VERBOSE("m_abort: abort NIM operation for %s\n",name,NULL,NULL,NULL)

	/* find Cstate & Mstate */
	if ( (Cstate = find_attr( obj, NULL, NULL, 0, ATTR_CSTATE )) < 0 )
		nim_error( ERR_OBJ_MISSING_ATTR, ATTR_CSTATE_T, obj->name, NULL );
	else if ( (Mstate = find_attr( obj, NULL, NULL, 0, ATTR_MSTATE )) < 0 )
		nim_error( ERR_OBJ_MISSING_ATTR, ATTR_MSTATE_T, obj->name, NULL );
	Cstate = (int) strtol( obj->attrs[Cstate].value, NULL, 0 );
	Mstate = (int) strtol( obj->attrs[Mstate].value, NULL, 0 );

	VERBOSE("   checking states: Cstate=%s; Mstate=%s\n",
				STATE_msg(Cstate),STATE_msg(Mstate),NULL,NULL)

	/* check for errors */
	if ( (Cstate == STATE_INCOMPLETE) || (Cstate == STATE_CREADY) )
		nim_error(	ERR_OP_NOT_NOW, ATTR_msg(ATTR_ABORT), 
						ATTR_msg(ATTR_CSTATE), STATE_msg(Cstate) );
	else if (	((Mstate == STATE_BOOTING) || (Mstate == STATE_RUNNING)) &&
					(! force) )
			nim_error(	ERR_OP_NOT_NOW, ATTR_msg(ATTR_ABORT), 
							ATTR_msg(ATTR_MSTATE), STATE_msg(Mstate) );

	/* boot and/or nim_script resources currently allocated? */
	dealloc_boot = (find_attr( obj, NULL, NULL, 0, ATTR_BOOT ) >= 0);
	dealloc_ns = (find_attr( obj, NULL, NULL, 0, ATTR_NIM_SCRIPT ) >= 0);

	/* deallocate network boot resource? */
	if (	(dealloc_boot) &&
			(dealloc_res( obj, ATTR_BOOT_T, &c_stdout ) == FAILURE) )
		nim_error( 0, NULL, NULL, NULL );

	/* deallocate nim_script resource? */
	if (	(dealloc_ns) &&
			(dealloc_res( obj, ATTR_NIM_SCRIPT_T, &c_stdout ) == FAILURE) )
		undo( 0, NULL, NULL, NULL );

	/* remove stale info */
	rm_attr( obj->id, NULL, ATTR_TRANS, 0, NULL );
	rm_attr( obj->id, NULL, ATTR_INFO, 0, NULL );

	/* change the Cstate */
	Cresult( obj->name, RESULT_ABORT );

	/* reset the state on any resource this machine serves */
	reset_server_res( obj );

	return( SUCCESS );

} /* end of abort */
	
/**************************       main         ********************************/

main( int argc, char *argv[] )

{
	/* initialize NIM environment */
	mstr_init( argv[0], ERR_POLICY_METHOD, NULL );
	if ( get_list_space( &attr_ass, DEFAULT_CHUNK_SIZE, TRUE ) == FAILURE )
		nim_error( 0, NULL, NULL, NULL );

	/* parse command line */
	parse_args( argc, argv );

	/* lock-and-get the object */
	if ( lag_object( 0, name, &obj, &info ) == FAILURE )
		nim_error( 0, NULL, NULL, NULL );

	/* abort the current control action */
	abort();

	exit( 0 );

}
