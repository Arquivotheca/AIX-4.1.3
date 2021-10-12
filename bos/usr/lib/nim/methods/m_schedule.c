static char	sccs_id[] = " @(#)15 1.3  src/bos/usr/lib/nim/methods/m_schedule.c, cmdnim, bos411, 9428A410j  2/11/94  14:55:32";

/*
 *   COMPONENT_NAME: CMDNIM
 *
 *   FUNCTIONS: ./usr/lib/nim/methods/m_schedule.c
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
 	{ATTR_FORCE,				ATTR_FORCE_T,				FALSE,	ch_pdattr_ass},
 	{ATTR_VERBOSE,				ATTR_VERBOSE_T,			FALSE,	ch_pdattr_ass},
	{0,							NULL,							FALSE,	valid_pdattr_ass}
};

char *name=NULL;									/* object name */
char *at = NULL;									/* time to execute the specified op */
NIM_OBJECT( obj, info )							/* object info */

/*---------------------------- unschedule        ------------------------------
*
* NAME: unschedule
*
* FUNCTION:
*		backs out changes made by the schedule function
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*			state_changed		= >0 if the object's state was changed
*		global:
*
* RETURNS: (int)
*		SUCCESS					= stuff undone
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
unschedule(	int state_changed )

{
	/* protect current errstr */
	protect_errstr = TRUE;

	/* remove the ATTR_AT */
	rm_attr( obj->id, NULL, ATTR_AT, 0, NULL );

	/* remove all ATTR_AT_ARGs */
	rm_attr( obj->id, NULL, ATTR_AT_ARG, 0, NULL );

	/* remove all ATTR_AT_JOBs */
	rm_attr( obj->id, NULL, ATTR_AT_JOB, 0, NULL );

	/* reset the state? */
	if ( state_changed )
		Cresult( name, RESULT_ABORT );

	return( SUCCESS );

} /* end of unschedule */

/*---------------------------- schedule        --------------------------------
*
* NAME: schedule
*
* FUNCTION:
*		schedules the specified operation
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
*			num					= number of <args>
*			args					= command & args to schedule
*		global:
*			obj
*
* RETURNS: (int)
*		SUCCESS					= operation scheduled
*		FAILURE					= ODM error?
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
schedule(	int num,
				char *args[] )

{	int rc;
	FILE *c_stdout = NULL;
	char when[MAX_TMP];
	char m_do_sop[MAX_TMP];
	char target[MAX_TMP];
	char *c_at[] = { C_AT, when, m_do_sop, target, NULL };
	int i;
	char tmperr[MAX_NIM_ERRSTR];
	char job[MAX_TMP];

	/* check the Cstate */
	if ( (i = find_attr( obj, NULL, NULL, 0, ATTR_CSTATE )) < 0 )
		nim_error( ERR_OBJ_MISSING_ATTR, ATTR_CSTATE_T, obj->name, NULL );
	else if ( ! same_state( obj->attrs[i].value, STATE_CREADY ) )
		nim_error( ERR_STATE, name, NULL, NULL );

	/* store the time to execute in the ATTR_AT attr */
	if ( mk_attr(	obj->id, NULL, args[1], 0, ATTR_AT, ATTR_AT_T ) == FAILURE )
		nim_error( 0, NULL, NULL, NULL );

	/* initialize the ATTR_AT_ARG attrs */

	/* add an ATTR_AT_ARG for each <args> */
	for( i=2; i < num; i++ )
		if ( mk_attr(	obj->id, NULL, args[i], 0, 
							ATTR_AT_ARG, ATTR_AT_ARG_T ) == FAILURE )
		{	unschedule( FALSE );
			nim_error( 0, NULL, NULL, NULL );
		}

	/* change the Cstate */
	if ( set_state( obj->id, NULL, ATTR_CSTATE, STATE_SCHEDULED ) == FAILURE)
	{	unschedule( FALSE );
		nim_error( 0, NULL, NULL, NULL );
	}

	/* schedule the cron job */
	sprintf( when, "%s=%s", ATTR_AT_T, args[1] );
	sprintf( m_do_sop, "%s=%s", ATTR_COMMAND_T, M_DO_SOP );
	sprintf( target, "%s=%s", ATTR_NAME_T, name );

	VERBOSE("   scheduling the operation for execution at \"%s\":\n",
				when,NULL,NULL,NULL)

	if (	(client_exec( &rc, &c_stdout, c_at ) == FAILURE) ||
			(rc > 0) )
	{	strcpy( tmperr, niminfo.errstr );
		errstr( ERR_COMMAND, tmperr, NULL, NULL );
		unschedule( TRUE );
		nim_error( 0, NULL, NULL, NULL );
	}

	/* add the "at" job number to the object's definition */
	/* note that we're ignoring the return code here because the ATTR_AT_JOB */
	/*		is not required in order for the operation to succeed */
	if ( fscanf( c_stdout, "%s", job ) > 0 )
		mk_attr( obj->id, NULL, job, 0, ATTR_AT_JOB, ATTR_AT_JOB_T );

	return( SUCCESS );

} /* end of schedule */
	
/**************************       main         ********************************/

main( int argc, char *argv[] )

{
	/* initialize NIM environment */
	mstr_init( argv[0], ERR_POLICY_METHOD, NULL );
	if ( get_list_space( &attr_ass, DEFAULT_CHUNK_SIZE, TRUE ) == FAILURE )
		nim_error( 0, NULL, NULL, NULL );

	/* expecting at least 3 args: */
	/*		#1   = time to execute command */
	/*		#2   = target to execute the command on */
	/*		#3   = command to execute */
	/*		#4-> = args (with last one being the object's name) */
	if ( argc < 5 )
		nim_error( ERR_SYNTAX, MSG_msg(MSG_SOP_SYNTAX), NULL, NULL );

	/* expecting first arg to be the time to execute the specified command */
	at = argv[1];

	/* expecting object name to be the last arg */
	name = argv[ argc - 1 ];

	VERBOSE("m_schedule: schedule a NIM operation for %s\n",name,NULL,NULL,NULL)

	/* lock-and-get the object */
	if ( lag_object( 0, name, &obj, &info ) == FAILURE )
		nim_error( 0, NULL, NULL, NULL );

	/* rest of the args are the method to invoke & its args */
	schedule( argc, argv );

	exit( 0 );

}
