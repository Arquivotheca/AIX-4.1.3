static char	sccs_id[] = " @(#)57 1.4  src/bos/usr/lib/nim/methods/m_do_sop.c, cmdnim, bos411, 9428A410j  2/19/94  18:01:34";

/*
 *   COMPONENT_NAME: CMDNIM
 *
 *   FUNCTIONS: ./usr/lib/nim/methods/m_do_sop.c
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
{ {0,							NULL,							FALSE,	valid_pdattr_ass}
};

char *name=NULL;									/* object name */

/*---------------------------- exec_method       ------------------------------
*
* NAME: exec_method
*
* FUNCTION:
*		does some schedule specific processing then invokes the specified method
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
*
* RETURNS: (int)
*		SUCCESS					= method invoked
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
exec_method()

{	NIM_OBJECT( obj, info )
	NIM_ATTR( meth_args, minfo )
	int rc;
	int i,j;
	LIST args;
	char *ignore = NULL;

	/* first, we need to ignore locks here because, if we don't, we'll end */
	/*		up having a lock with cron's PID */
	ignore_lock = TRUE;

	/* lag-and-get the object */
	if ( lag_object( 0, name, &obj, &info ) == FAILURE )
		nim_error( 0, NULL, NULL, NULL );

	/* for machines, reset the state */
	if ( obj->class == ATTR_CLASS_MACHINES )
		Cresult( name, RESULT_SUCCESS );

	/* get all the ATTR_AT_ARGs */
	if ( get_attr( &meth_args, &minfo, obj->id, NULL, 0, ATTR_AT_ARG ) <= 0 )
		nim_error( ERR_MISSING_AT, ATTR_msg(ATTR_AT_ARG), name, NULL );

	/* remove the ATTR_AT, ATTR_AT_ARG, ATTR_AT_JOB attrs */
	/* NOTE that we're not checking the return code here because we need to */
	/*		continue processing regardless of what happens */
	rm_attr( obj->id, NULL, ATTR_AT, 0, NULL );
	rm_attr( obj->id, NULL, ATTR_AT_ARG, 0, NULL );
	rm_attr( obj->id, NULL, ATTR_AT_JOB, 0, NULL );

	/* construct a LIST of method arguments */
	if ( get_list_space( &args, (minfo.num + 1), TRUE ) == FAILURE )
		nim_error( 0, NULL, NULL, NULL );
	for (i=0; i < minfo.num; i++)
	{	/* make sure that <args> is constructed in the correct order */
		for (j=0; j < minfo.num; j++)
			if ( meth_args[j].seqno == i )
			{	if ( add_to_LIST( &args, meth_args[j].value ) == FAILURE )
					nim_error( 0, NULL, NULL, NULL );
			}
	}

	/* ignore locking (again, because we'll get cron's PID if we don't) */
	ignore = nim_malloc( strlen(ATTR_IGNORE_LOCK_T) + 7 );
	sprintf( ignore, "-a%s=yes", ATTR_IGNORE_LOCK_T );
	if ( add_to_LIST( &args, ignore ) == FAILURE )
		nim_error( 0, NULL, NULL, NULL );

	/* add the object name */
	if ( add_to_LIST( &args, name ) == FAILURE )
		nim_error( 0, NULL, NULL, NULL );

	/* execute the method */
	if (	(master_exec( args.list[0], &rc, NULL, &args.list[1] ) == FAILURE) ||
			(rc > 0) )
	{	niminfo.errno = rc;
		nim_error( 0, NULL, NULL, NULL );
	}
	else
		/* display warnings */
		display_errstr();

	return( SUCCESS );

} /* end of exec_method */
	
/**************************       main         ********************************/

main( int argc, char *argv[] )

{
	/* initialize NIM environment */
	mstr_init( argv[0], ERR_POLICY_METHOD, NULL );
	if ( get_list_space( &attr_ass, DEFAULT_CHUNK_SIZE, TRUE ) == FAILURE )
		nim_error( 0, NULL, NULL, NULL );

	/* expecting argv[1] to be the object name */
	name = argv[1];

	VERBOSE("m_do_sop: execute a scheduled operation for %s\n",name,NULL,NULL,
				NULL)

	/* rest of the args are the method to invoke & its args */
	exec_method();

	exit( 0 );

}
