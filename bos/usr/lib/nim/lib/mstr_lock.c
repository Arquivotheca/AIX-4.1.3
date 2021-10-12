static char sccs_id[] = " @(#)87 1.9.1.1  src/bos/usr/lib/nim/lib/mstr_lock.c, cmdnim, bos411, 9430C411a  7/22/94  11:33:34";
/*
 *   COMPONENT_NAME: CMDNIM
 *
 *   FUNCTIONS:
 *		is_object_locked
 *		lag_object
 *		lock_object
 *		rm_all_obj_locks
 *		unlock_object
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/******************************************************************************
 *
 *                        NIM master function library
 *
 * this library contains NIM database manipulation routines, which should only
 *		be accessed by the NIM master
 ******************************************************************************/

#include "cmdnim.h"
#include "cmdnim_obj.h"
#include <varargs.h> 
#include "cmdnim_ip.h" 
#include <sys/wait.h> 

/*$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
$$$$$$$$$$$$$$$$ object locking                                    $$$$$$$$$$$$
$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$*/
	
/*---------------------------- is_object_locked  ------------------------------
*
* NAME: is_object_locked
*
* FUNCTION:
*		looks for a ATTR_LOCKED attribute for the specified object
*		the nim_object may be specified by either its <id> or <name>
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
*			id				= nim_object.id
*			name			= nim_object.name
*		global:
*
* RETURNS: (int)
*		NO_LOCK			= no lock currently held
*		I_LOCK			= "I" (this process) owns the lock
*		O_LOCK			= some "O"ther process owns the lock
*		FAILURE			= ODM errrors
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
is_object_locked(	long id, 
						char *name )

{	ODMQUERY
 	int	rc;
	struct listinfo info;
	struct nim_attr attr;
	pid_t lock_id;

	/* get the nim_object */
	if ( id <= 0 )
	{	if ( (id = get_id( name )) <= 0 )
			return( FAILURE );
	}

	VERBOSE4("         is_object_locked: id=%d; name=%s;\n",id,name,NULL,NULL)

	/* does the specified object have an ATTR_LOCKED attribute? */
	sprintf( query, "id=%d and pdattr=%d", id, ATTR_LOCKED );
	rc = (int) odm_get_first( nim_attr_CLASS, query, &attr );
	if ( rc == -1) 	
		ERROR( ERR_ODM, (char *) odmerrno, NULL, NULL )

	if ( rc != NULL)
	{	/* attr exists - object is locked */
		/* value of attr will be the process group id of the owner */
		lock_id = (pid_t) strtol( attr.value, NULL, 0 );

		VERBOSE4("            lock_id=%d;\n",lock_id,NULL,NULL,NULL)

		/* does current process belong to the process group of the owner */
		/*		of this lock? (true if this process owns the lock or is a child */
		/*		of the owner) */
		if ( niminfo.pgrp == lock_id )
		{
			VERBOSE4("            returning I_LOCK\n",NULL,NULL,NULL,NULL)

			/* effectively: "I" own the lock */
			return( I_LOCK );
		}

		/* if we get here, the object is locked but this process is not */
		/*		allowed to access the object */
		/* "O"ther owns the lock */
		VERBOSE4("            returning O_LOCK\n",NULL,NULL,NULL,NULL)
		return( O_LOCK );
	}

	/* "NO"body owns the lock */
	VERBOSE4("            returning NO_LOCK\n",NULL,NULL,NULL,NULL)
	return( NO_LOCK );

} /* end of is_object_locked */ 
	
/*---------------------------- lock_object       ------------------------------
*
* NAME: lock_object
*
* FUNCTION:
*		if the specified object is not locked, this function will lock the
*			specified object by adding a "lock" attribute; otherwise, it will
*			return FAILURE
*		the nim_object may be specified by either its <id> or <name>
*
* EXECUTION ENVIRONMENT:
*		if the NIM_GLOCK has not already been obtained, this function will
*			get it before doing any processing and will release the lock before
*			returning
*
* NOTES:
*		sets errstr on failure
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*			id					= nim_object.id
*			name				= nim_object.name
*		global:
*			attr_ass			= attribute assignments from the command line
*
* RETURNS: (int)
*		SUCCESS				= object locked successfully
*		FAILURE				= unable to lock the object
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
lock_object(	long id, 
					char *name )

{	int rc=SUCCESS;
	struct nim_attr attr;
	int release_glock=FALSE;
	char tmp[MAX_TMP];
	int lock_obtained=FALSE;
	int retry=0;

	VERBOSE4("         lock_object: id=%d; name=%s;\n",id,name,NULL,NULL)

	/* was ATTR_IGNORE_LOCK specified? */
	if ( ignore_lock )
		return( SUCCESS );

	while ( (! lock_obtained) && (retry++ < LOCK_RETRY) )
	{
		/* 
		 * NIM_GLOCK already held? 
		 */
		if ( !niminfo.glock_held)
		{	/* 
			 * no - get it before continuing 
			 */
			if ( (rc = get_glock(0,0)) == FAILURE )
				return( FAILURE );
			else if ( rc == SUCCESS )	
				/* just obtained the lock - flag for release before returning */
				release_glock = TRUE;
		}
	
		/* 
		 * is object already locked? 
		 */
		if ( (rc = is_object_locked( id, name )) == FAILURE )
			return( FAILURE );
	
		if ( rc == I_LOCK )
			/* we already own the lock */
			lock_obtained = TRUE;
	 	else if ( rc == NO_LOCK )
		{
			/* no lock currently exists - lock the object now */
			/* NOTE: id used to lock an object is the process group id */
			sprintf( tmp, "%d", niminfo.pgrp );
	
			/* add the lock attr */
			rc = mk_attr( id, name, tmp, 0, ATTR_LOCKED, ATTR_msg(ATTR_LOCKED) );

			lock_obtained = TRUE;
		}
	
		/* 
		 * release NIM_GLOCK if obtained upon entry 
		 */
		if ( release_glock )
			rm_glock( FALSE );

		/* delay before retrying if we didn't get the lock */
		if ( ! lock_obtained )
			sleep( LOCK_RETRY_DELAY );
	}

	if ( lock_obtained )
		rc = SUCCESS;
	else
	{	
		/* object is locked by somebody else and we were unable to obtain it */
		if ( name == NULL )
			name = get_name( id );
		errstr( ERR_ALREADY_LOCKED, name, NULL, NULL );
		rc = FAILURE;
	}

	return( rc );

} /* end of lock_object */ 
	
/*---------------------------- lag_object        ------------------------------
*
* NAME: lag_object
*
* FUNCTION:
*		"locks -and- gets" (lag) the specified object
*		the object may be specified by either its <id> or <name>
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*			id					= id of nim_object
*			name				= name of the object to get
*			obj				= ptr to nim_object struct ptr
*			info				= ptr to listinfo struct
*		global:
*
* RETURNS: (int)
*		SUCCESS				= object locked & retrieved
*		FAILURE				= couldn't lock -or- couldn't get (errstr set)
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
lag_object(	long id, 
				char *name,
				struct nim_object **obj,
				struct listinfo *info )

{	
	VERBOSE4("         lag_object: id=%d; name=%s;\n",id,name,NULL,NULL)

	/* lock the object */
	if ( lock_object( id, name ) == SUCCESS )
		if ( get_object( obj, info, id, name, 0, 0 ) > 0 )
			return( SUCCESS );

	return( FAILURE );

} /* end of lag_object */
	
/*---------------------------- unlock_object    ------------------------------
*
* NAME: unlock_object
*
* FUNCTION:
*		removes the specified object lock if this process is the current owner
*			-or- if the force option is used
*		the owner may be specified by either its <id> or <name>
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*		if the NIM_GLOCK has not already been obtained, this function will
*			get it before doing any processing and will release the lock before
*			returning
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*			id						= nim_object.id
*			name					= nim_object.name
*			force					= TRUE if lock is to be removed unconditionally
*		global:
*			attr_ass				= attribute assignments from the command line
*
* RETURNS: (int)
*		SUCCESS					= lock removed
*		FAILURE					= unable to remove lock
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
unlock_object(	long id, 
					char *name, 
					int force )

{	int rc=SUCCESS;
	char criteria[MAX_CRITELEM_LEN];
	int release_glock=FALSE;

	VERBOSE4("         unlock_object: id=%d; name=%s; force=%d;\n",id,name,
				force,NULL)

	/* was ATTR_IGNORE_LOCK specified? */
	if ( ignore_lock )
		return( SUCCESS );

	/* 
	 * NIM_GLOCK already held? 
	 */
	if ( !niminfo.glock_held) {
		/* 
		 * no - get it before continuing 
		 */
		rc = get_glock(0,0);
		if ( rc == FAILURE )
			return( FAILURE );
		/* 
		 * If lock obtained NOW - flag it for release later
		 */
		if ( rc == SUCCESS )	
			release_glock = TRUE;
	}

	/* 
	 * is object locked ? 
	 */
	rc = is_object_locked( id, name );
	if ( (rc != FAILURE) && (rc != NO_LOCK) && ((rc==I_LOCK) || (force)) )
		rc = rm_attr( id, name, ATTR_LOCKED, 0, NULL );
	else if ( rc == O_LOCK )
		rc = FAILURE;

	/* 
	 * release NIM_GLOCK if obtained upon entry 
	 */
	if ( release_glock )
		rm_glock( FALSE );

	return( rc );

} /* unlock_object */
	
/*---------------------------- uaf_object        ------------------------------
*
* NAME: uaf_object ("u"nlock "a"nd "f"ree object)
*
* FUNCTION:
*		unlock and frees the specified object
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*			obj					= ptr to nim_object struct
*			info					= ptr to listinfo struct
*			force					= TRUE if lock is to be removed unconditionally
*		global:
*
* RETURNS: (int)
*		SUCCESS					= object released
*		FAILURE					= ODM error?
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
uaf_object(	struct nim_object *obj,
				struct listinfo *info,
				int force )

{	int id;

	VERBOSE4("         uaf_object: name=%s; force=%d;\n",obj->name,force,NULL,
				NULL)

	id = obj->id;
	odm_free_list( obj, info );

	return( unlock_object( id, NULL, force ) );

} /* end of uaf_object */
	
/*---------------------------- rm_all_obj_locks  ------------------------------
*
* NAME: rm_all_obj_locks
*
* FUNCTION:
*		removes all object locks held by the current process
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*		if the NIM_GLOCK has not already been obtained, this function will
*			get it before doing any processing and will release the lock before
*			returning
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*		global:
*
* RETURNS: (int)
*		SUCCESS					= all locks removed
*		FAILURE					= unable to remove one or more locks
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
rm_all_obj_locks()

{	int rc=SUCCESS;
	char criteria[MAX_CRITELEM_LEN];
	int release_glock=FALSE;

	VERBOSE4("         rm_all_obj_locks\n",NULL,NULL,NULL,NULL)

	/* NIM_GLOCK already held? */
	if (! niminfo.glock_held)
	{	/* no - get it before continuing */
		if ( get_glock(0,0) )
			release_glock = TRUE;
		else
			/* unable to get the NIM_GLOCK */
			return( FAILURE );
	}

	/* initialize the criteria string to this process's lock id */
	sprintf( criteria, "pdattr=%d and value=%d", ATTR_LOCKED, niminfo.pgrp );

	/* remove all machine locks */
	if ( odm_rm_obj( nim_attr_CLASS, criteria ) == -1 )
		rc = FAILURE;

	/* release NIM_GLOCK is obtained upon entry */
	if ( release_glock )
		rm_glock( FALSE );

	/* return */
	return( rc );

} /* end of rm_all_obj_locks */

