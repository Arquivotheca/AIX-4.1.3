static char     sccs_id[] = " @(#)75 1.51  src/bos/usr/lib/nim/lib/mstr_res.c, cmdnim, bos411, 9438C411a  9/22/94  20:05:12";

/*
 *   COMPONENT_NAME: CMDNIM
 *
 *   FUNCTIONS: ./usr/lib/nim/lib/mstr_res.c
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

#include "cmdnim.h"
#include "cmdnim_obj.h"
#include <sys/vmount.h>

/*$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
 $$$$$$$$$$$$$$$$ Control checking			                          $$$$$$$$$$$
 $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$*/


/*---------------------------- check_control     ------------------------------
*
* NAME: check_control
*
* FUNCTION:
*               determines whether the invoker of this command can perform an operation
*                       on the specified machines object
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*               calls nim_error on failure
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*               parameters:
*               global:
*                       obj
*
* RETURNS: (int)
*               SUCCESS = caller has permission to perform the operation
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
check_control(struct nim_object *obj )

{       int i;
        char *controller;
        int client_request = FALSE;

        /* does object have ATTR_CONTROL set? */
        if ( (i = find_attr( obj, NULL, NULL, 0, ATTR_CONTROL )) >= 0 )
        {
                /* check value of ATTR_CONTROL: */
                /*              first field will always contain the name of the machine object */
                /*                      which has control */
                /*              if there is a second field present, then this indicates that */
                /*                      control is persistent (ie, not just for one install op) */
                if ( two_fields( obj->attrs[i].value, &controller, NULL ) == FAILURE )
                        controller = obj->attrs[i].value;

                /* was this command invoked from the client? */
                /* if so, the ATTR_PULL_REQUEST will have been supplied */
                client_request = (find_attr_ass( &attr_ass, ATTR_PULL_REQUEST ) >= 0);

                /* does controller match requestor? */
                /* if this is a client request, then client must be in control */
                /* otherwise, the machine we're executing on must be in control */
                if (    ((client_request) && (strcmp( controller, obj->name ) != 0)) ||
                                ((! client_request) && (strcmp( controller, niminfo.nim_name )!=0)))
                        nim_error( ERR_CONTROL, obj->name, controller, NULL );
        }

        return( SUCCESS );

} /* end of check_control */

/*$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
 $$$$$$$$$$$$$$$$ resource object creation                          $$$$$$$$$$$
 $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$*/
	
/*---------------------------- ok_to_mk_robj           -------------------------
*
* NAME: ok_to_mk_robj
*
* FUNCTION:
*		checks to make sure that the information supplied by user is sufficient
*			to create a resource object
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
*			type					= type of resource object being defined
*			server				= name of machine which will serve the new resource
*			location				= location of new resource
*			stat_location		= >0 if the location is to be "stat"ed
*		global:
*
* RETURNS: (int)
*		SUCCESS					= nothing missing - ok to create a new object
*		FAILURE					= definition incomplete
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
ok_to_mk_robj(	char *res_type,
					char *server,
					char *location,
					int stat_location )

{	int i,j,k,l;
	NIM_OBJECT( obj, info )
	NIM_ATTR( attrs, ainfo )
	long id;
	int rc;
	char *new_pattern=NULL;
	regex_t new_re;
	char *existing_pattern=NULL;
	regex_t existing_re;

	VERBOSE2("   ok_to_mk_robj: server=%s; location=%s\n",server,location,
				NULL,NULL)

	/* location cannot be relative - ie, it must begin with "/" */
	if ( location[0] != '/' )
		ERROR( ERR_PATHNAME, location, NULL, NULL )

	/* location cannot contain any metacharacters */
	if ( regexec( nimere[BAD_LOCATION_ERE].reg, location, 0, NULL, 0 ) == 0 )
		ERROR( ERR_BAD_CHARS, location, NULL, NULL )

	/* lock-and-get the server object */
	if ( lag_object( 0, server, &obj, &info ) == FAILURE )
		return( FAILURE );

	/* check who has control of the server */
	if ( check_control( obj ) == FAILURE )
		return( FAILURE );

	/* can the specified machine be a server right now? */
	/* criteria are: */
	/*		type = standalone or master */
	/*		MSTATE = running */
	/*		CSTATE = ready */
	if (	(obj->type->attr != ATTR_STANDALONE) &&
			(obj->type->attr != ATTR_MASTER) )
		ERROR( ERR_SERVER_TYPE, server, ATTR_msg(obj->type->attr), NULL )
	if (	( (k = find_attr( obj, NULL, NULL, 0, ATTR_MSTATE )) < 0 ) ||
			( ! same_state( obj->attrs[k].value, STATE_RUNNING )) ||
			( (k = find_attr( obj, NULL, NULL, 0, ATTR_CSTATE )) < 0 ) ||
			( ! same_state( obj->attrs[k].value, STATE_CREADY )) )
		ERROR( ERR_STATE, server, NULL, NULL )

	VERBOSE2("      checking for location collisions\n",NULL,NULL,
				NULL,NULL)

	/* initialize the regular expression for new location */
	new_pattern = nim_malloc( strlen(location) + 5 );
	if ( location[1] == NULL_BYTE )
		strcpy( new_pattern, "^/*" );
	else
		sprintf( new_pattern, "^%s/.*", location );
	if ( (rc = regcomp( &new_re, new_pattern, REG_EXTENDED )) != 0 )
	{
		regerror( rc, &new_re, niminfo.errstr, MAX_NIM_ERRSTR );
		ERROR( ERR_SYS, niminfo.errstr, NULL, NULL )
	}

	/* for each resources this machines servers... */
	k = -1;
	while ( find_attr( obj, &k, NULL, 0, ATTR_SERVES ) >= 0 )
	{
		/* get this resource's location */
		if ( (id = get_id( obj->attrs[k].value )) <= 0 )
			ERROR( 0, NULL, NULL, NULL )
		if ( get_attr( &attrs, &ainfo, id, NULL, 0, ATTR_LOCATION ) <= 0 )
			ERROR(	ERR_OBJ_MISSING_ATTR, ATTR_LOCATION_T, obj->attrs[k].value,
						NULL)

		/* is new location the same as an existing resource? */
		if ( strcmp( location, attrs[0].value ) == 0 )
			ERROR(	ERR_RES_LOCATION, obj->name, attrs[0].value, location )

		/* is existing resource part of the new location? */
		if ( (rc = regexec( &new_re, attrs[0].value, 0, NULL, 0 )) != 0 )
		{
			if ( rc != REG_NOMATCH )
			{
				regerror( rc, &new_re, niminfo.errstr, MAX_NIM_ERRSTR );
				ERROR( ERR_SYS, niminfo.errstr, NULL, NULL )
			}
		}
		else
			ERROR(	ERR_RES_LOCATION, obj->name, attrs[0].value, location )

		/* is new location part of an existing resource? */
		existing_pattern = nim_malloc( strlen( attrs[0].value ) + 5 );
		if ( (attrs[0].value[0] == '/') && (attrs[0].value[1] == NULL_BYTE) )
			strcpy( existing_pattern, "^/*" );
		else
			sprintf( existing_pattern, "^%s/.*", attrs[0].value );
		if ( (rc = regcomp( &existing_re, existing_pattern, REG_EXTENDED )) != 0 )
		{
			regerror( rc, &existing_re, niminfo.errstr, MAX_NIM_ERRSTR );
			ERROR( ERR_SYS, niminfo.errstr, NULL, NULL )
		}
		if ( (rc = regexec( &existing_re, location, 0, NULL, 0 )) != 0 )
		{
			if ( rc != REG_NOMATCH )
			{
				regerror( rc, &existing_re, niminfo.errstr, MAX_NIM_ERRSTR );
				ERROR( ERR_SYS, niminfo.errstr, NULL, NULL )
			}
		}
		else
			ERROR(	ERR_RES_LOCATION, obj->name, attrs[0].value, location )

		/* free malloc'd space */
		free( existing_pattern );
		odm_free_list( attrs, &ainfo );
	}

	if ( stat_location )
		return( stat_file( NULL, NULL, res_type, server, location ) );

	return( SUCCESS );

} /* end of ok_to_mk_robj */
	
/*---------------------------- mk_robj             -----------------------------
*
* NAME: mk_robj
*
* FUNCTION:
*		creates a new resource object
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
*			name					= name of object
*			type					= type of object
*			server				= ptr to char ptr in order to return server name
*			stat_location		= >0 if the location is to be "stat"ed
*			attr_ass				= ptr to ATTR_ASS_LIST struct
*		global:
*
* RETURNS: (long)
*		>0							= id of new object
*		FAILURE					= error encountered; object not created
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

long
mk_robj(	char *name,
			char *type,
			char **server,
			int stat_location,
			ATTR_ASS_LIST *attr_ass )

{	long id;
	int i;
	int rc;
	char *location=NULL;

	VERBOSE3("      mk_robj: name=%s; type=%s\n",name,type,NULL,NULL)

	/* find the server & location */
	if ( (*server = attr_value( attr_ass, ATTR_SERVER )) == NULL )
		ERROR( ERR_MISSING_ATTR, ATTR_msg(ATTR_SERVER), NULL, NULL )
	else if ( (location = attr_value( attr_ass, ATTR_LOCATION )) == NULL )
		ERROR( ERR_MISSING_ATTR, ATTR_msg(ATTR_LOCATION), NULL, NULL )
	
	/* check for completeness of object definition */
	if ( ok_to_mk_robj( type, *server, location, stat_location ) == FAILURE )
		return( FAILURE );

	/* add ATTR_ALLOC_COUNT to the attr_ass LIST */
	add_attr_ass(	attr_ass, ATTR_ALLOC_COUNT, ATTR_ALLOC_COUNT_T,
						"0", 0 );

	/* create the object */
	if ( (id = mk_object( name, type, 0 )) <= 0 )
		return( FAILURE );

	/* add the nim_attrs */
	for (i=0; i < attr_ass->num; i++)
		if (! mk_attr( id, NULL, attr_ass->list[i]->value,
							attr_ass->list[i]->seqno, attr_ass->list[i]->pdattr,
							attr_ass->list[i]->name ) )
		{	/* error occured - object cannot be defined; remove it */
			protect_errstr = TRUE;
			rm_object( id, NULL );
			return( FAILURE );
		}

	/* set the RSTATE to unavailable - it's up to the caller to decide if */
	/*		the resource is ready to use or not */
	set_state( id, NULL, ATTR_RSTATE, STATE_UNAVAILABLE );

	return( id );

} /* end of mk_robj */
	
/*---------------------------- finish_robj       ------------------------------
*
* NAME: finish_robj
*
* FUNCTION:
*		completes the definition of a resource object by adding an ATTR_SERVES
*			attr for the server & setting the resource state to "available"
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
*			res					= name of resource
*			server				= name of server of <res>
*		global:
*
* RETURNS: (int)
*		SUCCESS					= definition complete - resource available
*		FAILURE					= error adding attr for server
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
finish_robj(	char *res,
					char *server )

{
	VERBOSE3("      finish_robj: res=%s; server=%s;\n",res,server,NULL,NULL)

	/* add an ATTR_SERVES for the server */
	if ( mk_attr( 0, server, res, 0, ATTR_SERVES, ATTR_SERVES_T ) == FAILURE )
	{	/* some kind of error occurred - things will be screwed up if they're */
		/*		left like this, so remove the new object */
		/* in order to avoid rm_object overwriting the current errstr, */
		/*		protect it now */
		protect_errstr = TRUE;
		rm_object( 0, res );
		return( FAILURE );
	}

	/* all object work done - ready to be used */
	set_state( 0, res, ATTR_RSTATE, STATE_AVAILABLE );

	return( SUCCESS );

} /* end of finish_robj */
	
	
/*---------------------------- unexport_res      ------------------------------
*
* NAME: unexport_res
*
* FUNCTION:
*		unexports the specified resource if NIM had previously exported it
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*		sets errstr on failure
*		this function ASSUMES that <robj> is retrieved using lag_object
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*			robj	= ptr to nim_object struct for the resource
*		global
*
* RETURNS: (int)
*		SUCCESS		= resource un-exported
*		FAILURE		= NFS error
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
unexport_res(	struct nim_object *robj )

{	int i,j;
	int rc;
	FILE *c_stdout = NULL;
	char *args[] = { RMNFSEXP, "-B", "-d",NULL, NULL };
	char *tmp = NULL;

	VERBOSE3("      unexport_res: res=%s;\n",robj->name,NULL,NULL,NULL)

	/* 
	 * if NIM had exported this resource, then there will be an ATTR_EXPORTED 
	 */
	if (	((i = find_attr( robj, NULL, NULL, 0, ATTR_EXPORTED )) < 0) ||
			((j = find_attr( robj, NULL, NULL, 0, ATTR_SERVER )) < 0) )
		/* no attr - NIM didn't export it */
		return( SUCCESS );

	/* 
	 * unexport it 
	 */
	args[3] = robj->attrs[i].value;
	if ( (master_exec( robj->attrs[j].value, &rc, &c_stdout, args )==FAILURE)||
			(rc > 0) ) {	
		/* 
		 * cache the RMNFSEXP error info 
		 */
		tmp = nim_malloc( strlen( niminfo.errstr ) + 1 );
		strcpy( tmp, niminfo.errstr );
		fclose( c_stdout );
		ERROR( ERR_NFSEXP, robj->attrs[i].value, robj->attrs[j].value, tmp )
	}
	/*
	 * remove the attr_exported attribute 
	 */ 
	rm_attr(robj->id, NULL, ATTR_EXPORTED, 0, NULL);
	fclose( c_stdout );
	return( SUCCESS );

} /* end of unexport_res */
	
/*$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
$$$$$$$$$$$$$$$$ resource object removal                           $$$$$$$$$$$$
$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$*/
	
/*---------------------------- ok_to_rm_robj     ------------------------------
*
* NAME: ok_to_rm_robj
*
* FUNCTION:
*		determines whether it's ok to remove a resource
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*		sets errstr on failure
*		this function ASSUMES that <robj> was retrieved using lag_object
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*			robj					= ptr to nim_object struct
*		global:
*
* RETURNS: (int)
*		SUCCESS					= ok to remove resource
*		FAILURE					= not ok to remove
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
ok_to_rm_robj(	struct nim_object *robj )

{	int i;

	VERBOSE3("      ok_to_rm_robj: res=%s;\n",robj->name,NULL,NULL,NULL)

	/* is this a "reserved" resource (ie., NIM won't allow user to remove) */
	if ( (i = find_attr( robj, NULL, NULL, 0, ATTR_RESERVED )) >= 0 )
		ERROR( ERR_RESERVED_OBJ, robj->name, NULL, NULL )

	/* is this resource currently allocated? */
	if ( (i = find_attr( robj, NULL, NULL, 0, ATTR_ALLOC_COUNT )) < 0 )
	{	/* if there's no ATTR_ALLOC_COUNT, something's messed up */
		/* since we're going to delete it anyway, just check to make sure */
		/*		no machines reference it */
		if ( get_attr( NULL, NULL, 0, robj->name, 0, robj->type->attr ) > 0 )
			ERROR( ERR_REFERENCED, robj->name, NULL, NULL )
	}
	else if ( strtol( robj->attrs[i].value, NULL, 0 ) > 0 )
		ERROR( ERR_RES_BUSY, robj->name, NULL, NULL )

	return( SUCCESS );

} /* end of ok_to_rm_robj */
	
/*---------------------------- rm_robj           ------------------------------
*
* NAME: rm_robj
*
* FUNCTION:
*		removes the specified resource object
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*		sets errstr on failure
*		this function ASSUMES that <robj> was retrieved using lag_object & that
*			ok_to_rm_robj has already been executed successfully
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*			robj					= ptr to nim_object struct
*		global:
*
* RETURNS: (int)
*		SUCCESS					= <robj> removed
*		FAILURE					= error
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
rm_robj(	struct nim_object *robj )

{	int i;
	char *server;

	VERBOSE3("      rm_robj: res=%s;\n",robj->name,NULL,NULL,NULL)

	/* find the server's name */
	if ( (i = find_attr( robj, NULL, NULL, 0, ATTR_SERVER )) < 0 )
		ERROR( ERR_OBJ_MISSING_ATTR, ATTR_SERVER_T, robj->name, NULL )
	else
		server = robj->attrs[i].value;

	/* lock the server's object */
	if ( lock_object( 0, server ) == FAILURE )
		return( FAILURE );

	/* remove the ATTR_SERVES attr, if not present we dont care */
	rm_attr( 0, server, ATTR_SERVES, 0, robj->name );

	/* unlock the server */
	unlock_object( 0, server, FALSE );

	/* remove <robj> */
	return( rm_object( robj->id, NULL ) );

} /* end of rm_robj */

/*---------------------------- rm_dir             ------------------------------
*
* NAME: rm_dir
*
* FUNCTION:
*		removes the specified directory
*		when <client_name> is specified, this function will append that name to
*			the given <dir>
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
*			server				= name of server of <dir>
*			dir					= directory name
*			client_name			= name of client subdirectory to remove
*		global:
*
* RETURNS: (int)
*		SUCCESS					= <dir> removed
*		FAILURE					= exec error?
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
rm_dir(	char *server,
			char *dir,
			char *client_name )

{	char tmp[MAX_TMP];
	int rc;
	FILE *c_stdout = NULL;
	char *args[] = { C_RMDIR, tmp, NULL };

	VERBOSE3("      rm_dir: server=%s; dir=%s; client=%s;\n",server,dir,
				client_name,NULL)

	/* initialize the parameters for C_RMDIR */
	if ( client_name == NULL )
		sprintf( tmp, "-a%s=%s", ATTR_LOCATION_T, dir );
	else
		sprintf( tmp, "-a%s=%s/%s", ATTR_LOCATION_T, dir, client_name );

	/* remove <dir> */
	if (	(master_exec( server, &rc, &c_stdout, args ) == FAILURE) ||
			(rc > 0) )
		return( FAILURE );

	return( SUCCESS );

} /* end of rm_dir */

/*$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
$$$$$$$$$$$$$$$$ resource access processing                        $$$$$$$$$$$$
$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$*/
	
/*---------------------------- new_res_access      ----------------------------
*
* NAME: new_res_access 
*
* FUNCTION:
*		adds another entry to the specified LIST of res_access structs
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
*			target_net			= name of target's primary network
*			res_name				= name of resource
*			res_type				= pdattr.attr for <res_name>
*			res_location		= location of <res_name>
*			res_server			= name of server of <res_name>
*			list					= ptr to LIST struct
*		global:
*
* RETURNS: (int)
*		SUCCESS					= new res_access struct added to <list>
*		FAILURE					= ODM problems?
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
new_res_access(	char *target_net,
						char *res_name,
						int res_type,
						char *res_location,
						char *res_server,
						LIST *list )

{	struct res_access *raccess=NULL;

	/* get space for a new LIST entry */
	if ( get_list_space( list, 1, FALSE ) == FAILURE )
		return( FAILURE );
	raccess = (struct res_access *) nim_malloc( RES_ACCESS_SIZE );

	/* copy in resource name & type */
	strncpy( raccess->name, res_name, MAX_NAME_BYTES );
	raccess->type = res_type;

	/* copy in server name */
	strncpy( raccess->server, res_server, MAX_NAME_BYTES );

	if ( res_location == NULL )
		raccess->location = NULL;
	else
	{	/* get space for the location & copy it in */
		raccess->location = nim_malloc( strlen(res_location) + 1 );
		strcpy( raccess->location, res_location );
	}

	/* determine what interface to use on the server */
	if ( net_to_mac(	target_net, res_server, &raccess->nimif ) == FAILURE )
		return( FAILURE );

	/* finish initialization */
	list->list[ list->num ] = (char *) raccess;
	list->num = list->num + 1;

	return( SUCCESS );

} /* end of new_res_access */
	
/*---------------------------- add_res_access_LIST ----------------------------
*
* NAME: add_res_access_LIST 
*
* FUNCTION:
*		adds res_access info to <list> for the specified resource
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
*			target				= name of target
*			target_net			= name of target's primary network
*			res					= name of resource
*			list					= ptr to LIST struct
*		global:
*
* RETURNS: (int)
*		SUCCESS					= entry added
*		FAILURE					= ODM error?
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
add_res_access_LIST(	char *target,
							char *target_net,
							char *res,
							LIST *list )

{	NIM_OBJECT( robj, rinfo )
	int j;
	char *server=NULL;
	char *location=NULL;

	/* get the resource object */
	if ( get_object( &robj, &rinfo, 0, res, 0, 0 ) <= 0 )
		ERROR(	ERR_BAD_OBJECT, ATTR_msg(ATTR_CLASS_RESOURCES), res, NULL )

	/* find the server & location */
	if ( (j = find_attr( robj, NULL, NULL, 0, ATTR_SERVER )) < 0 )
		return( FAILURE );
	server = robj->attrs[j].value;
	if ( (j = find_attr( robj, NULL, NULL, 0, ATTR_LOCATION )) < 0 )
		return( FAILURE );

	/* for PDIR type resources, need to append the client's name to location */
	if ( attr_in_list( ATTR_SUBCLASS_PDIR_TYPE, robj->type->subclass ) )
	{
		/* malloc enough space to construct correct pathname */
		location = nim_malloc( strlen(robj->attrs[j].value) + strlen(target) + 2);
		sprintf( location, "%s/%s", robj->attrs[j].value, target );
	}
	else
		location = robj->attrs[j].value;

	/* add a new entry in the list */
	if ( new_res_access(	target_net,
								robj->name,
								robj->type->attr,
								location,
								server,
								list ) == FAILURE )
		return( FAILURE );

	odm_free_list( robj, &rinfo );

	return( SUCCESS );

} /* end of add_res_access_LIST */
	
/*---------------------------- LIST_res_access   ------------------------------
*
* NAME: LIST_res_access
*
* FUNCTION:
*		generates a LIST of res_access structs for the specified machine
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*		sets errstr on failure
*		this function ASSUMES that the specified machine object was obtained using
*			lag_object (ie., that it is locked and retrieved to depth of 3)
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*			mac					= machine object to generate list for
*			list					= ptr to LIST struct
*		global:
*
* RETURNS: (int)
*		SUCCESS					= LIST of res_access structs generated
*		FAILURE					= ?
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
LIST_res_access(	struct nim_object *mac,
						LIST *list )

{	int i;
	struct nim_if pif;

	VERBOSE3("      LIST_res_access: mac=%s;\n",mac->name,NULL,NULL,NULL)

	/* initialize LIST */
	if ( get_list_space( list, DEFAULT_CHUNK_SIZE, TRUE ) == FAILURE )
		return( FAILURE );

	/* find the machine's primary network */
	if (	((i = find_attr( mac, NULL, NULL, 1, ATTR_IF )) < 0) ||
			(s2nim_if( mac->attrs[i].value, &pif ) == FAILURE) )
		return( FAILURE );

	/* for each attr <mac> has... */
	for (i=0; i < mac->attrs_info->num; i++)
	{	/* look for those which are resources */
		if (	(mac->attrs[i].pdattr->class == ATTR_CLASS_RESOURCES) &&
				(attr_in_list(ATTR_SUBCLASS_TYPE, mac->attrs[i].pdattr->subclass)) )
			if ( add_res_access_LIST(	mac->name, pif.network, 
												mac->attrs[i].value, list ) == FAILURE )
				return( FAILURE );
	}

	/* now add the master to the LIST (we always need access to the master) */
	if ( new_res_access(	pif.network,
								ATTR_MASTER_T,
								ATTR_MASTER,
								NULL,
								ATTR_MASTER_T,
								list ) == FAILURE )
		return( FAILURE );

	return( SUCCESS );

} /* end of LIST_res_access */
	
/*---------------------------- find_res_access   ------------------------------
*
* NAME: find_res_access
*
* FUNCTION:
*		returns the index in <list> for the specified resource <type>
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*		sets errstr on failure
*		this function ASSUMES that <list> was generated with LIST_res_access
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*			type					= type of resource being searched for
*			list					= ptr to LIST struct
*		global:
*
* RETURNS: (int)
*		>= 0						= index in <list> where resource <type> found
*		< 0						= <type> not found
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
find_res_access(	int type,
						LIST *list )

{	int i;
	struct res_access *raccess;

	for (i=0; i < list->num; i++)
	{	raccess = (struct res_access *) list->list[i];
		if ( raccess->type == type )
			return( i );
	}

	/* not found */
	return( -1 );

} /* end of find_res_access */

/*$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
$$$$$$$$$$$$$$$$ invokation of resource-specific method            $$$$$$$$$$$$
$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$*/
	
/*---------------------------- res_specific_method  ----------------------------
*
* NAME: res_specific_method
*
* FUNCTION:
*		invokes a resource specific method for the specified <res_name> if one
*			exists; if not, this function returns -1
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*		sets errstr on failure
*		this function ASSUMES that <client> has been obtained using lag_object
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*			client				= ptr to nim_object for machine 
*			res_name				= name of resource to allocate
*			operation			= operation to perform
*			c_stdout				= ptr to FILE ptr (for redirection of stdout)
*		global:
*			attr_ass
*
* RETURNS: (int)
*		1							= specific method exists & was successfully invoked
*		0							= method exists, but error encountered
*		-1							= specific method for <operation> does not exist
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
res_specific_method(	struct nim_object *client,
							char *res_name,
							int operation,
							FILE **c_stdout )

{	int rc = -1;
	struct nim_pdattr *type_pdattr;
	NIM_PDATTR( pdattr, pdinfo )
	char tmp[MAX_TMP];
	int i;
	LIST params;
	int mask;
	char *attrass;

	/* need resource's pdattr type */
	if ( (type_pdattr = get_type_pdattr( 0, res_name )) <= NULL )
	{
		errstr( ERR_DNE, res_name, NULL, NULL );
		return( -1 );
	}

	/* is there a resource-specific method for <operation>? */
	if ( get_pdattr(	&pdattr, &pdinfo, 0, 0, type_pdattr->attr, 
							operation, NULL ) > 0 )
	{
		/* yes, there is - initialize parameters for it */
		if (	(get_list_space( &params, DEFAULT_CHUNK_SIZE, TRUE ) == FAILURE) ||
				(add_to_LIST( &params, pdattr->value ) == FAILURE) )
			return( 0 );

		/* ok to pass the type? */
		mask = type_pdattr->mask & PDATTR_MASK_USER_ENTERABLE;
		if ( mask == PDATTR_MASK_USER_ENTERABLE )
		{
			sprintf( tmp, "-a%s=%s", type_pdattr->name, res_name );
			if ( add_to_LIST( &params, tmp ) == FAILURE )
				return( 0 );
		}

		/* pass force, verbose, or ignore_lock? */
		if ( force )
		{
			sprintf( tmp, "-a%s=yes", ATTR_FORCE_T );
			if ( add_to_LIST( &params, tmp ) == FAILURE )
				return( 0 );
		}
		if ( verbose )
		{
			sprintf( tmp, "-a%s=%d", ATTR_VERBOSE_T, verbose );
			if ( add_to_LIST( &params, tmp ) == FAILURE )
				return( 0 );
		}
		if ( ignore_lock )
		{
			sprintf( tmp, "-a%s=yes", ATTR_IGNORE_LOCK_T );
			if ( add_to_LIST( &params, tmp ) == FAILURE )
				return( 0 );
		}

		/* add attr flags */
		for (i=0; i < attr_ass.num; i++)
			if ( pdattr_class( attr_ass.list[i]->pdattr ) == ATTR_CLASS_FLAGS )
			{
				/* NOTE that we malloc space for the attr assignment because */
				/*		the attr value may be large (ie, greater than the */
				/*		length of tmp) */
				attrass = nim_malloc(	strlen( attr_ass.list[i]->name ) +
												strlen( attr_ass.list[i]->value ) + 4 );
				sprintf( attrass, "-a%s=%s", attr_ass.list[i]->name, 
							attr_ass.list[i]->value );
				if ( add_to_LIST( &params, attrass ) == FAILURE )
					return( 0 );
			}

		/* add client's name */
		if ( add_to_LIST( &params, client->name ) == FAILURE )
			return( 0 );

		VERBOSE3("      res_specific_method: invoking the %s method\n",
					params.list[0],NULL,NULL,NULL)

		/* invoke the method (NOTE that this is a "master method") */
		if ( client_exec( &rc, c_stdout, &params.list[0] ) == FAILURE )
			rc = 0;
		else
		{
			/* method failure? */
			if ( rc > 0 )
			{
				errstr( ERR_METHOD, niminfo.nim_name, niminfo.errstr, NULL );
				rc = 0;
			}
			else
				rc = 1;

			/* add attrs from client's stdout? */
			if ( c_stdout != NULL )
				attrs_from_FILE( client->id, *c_stdout );
		}
	
		odm_free_list( pdattr, &pdinfo );
	}

	return( rc );

} /* end of res_specific_method */

/*$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
$$$$$$$$$$$$$$$$ resource allocation                               $$$$$$$$$$$$
$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$*/
	
/*---------------------------- ok_to_allocate      -----------------------------
*
* NAME: ok_to_allocate
*
* FUNCTION:
*		performs all the checks necessary in order to ensure that <res> can
*			be allocated to <target>
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*		sets errstr on failure
*		this function ASSUMES that the objects involved have already been
*			retrieved using lag_object
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*			target				= machine which is going to use <res>
*			target_net			= name of network <target> is connected to; this is
*											passed in so that we don't have to convert
*											the PIF each time (more efficient for mult
*											allocation operations)
*			res					= resource to be used by <target>
*			nimif					= ptr to nim_if struct; this will be used to return
*										the server's interface info
*		global:
*			attr_ass
*
* RETURNS: (int)
*		SUCCESS					= ok to allocate the specified resource
*		FAILURE					= not ok to allocate
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
ok_to_allocate(	struct nim_object *target,
						char *target_net,
						struct nim_object *res,
						struct nim_if *nimif )

{	int j;

	VERBOSE3("      ok_to_allocate: target=%s; target_net=%s; res=%s\n",
				target->name,target_net,res->name,NULL)

	/* was ATTR_IGNORE_STATE flag used? */
	if ( attr_value( &attr_ass, ATTR_IGNORE_STATE ) <= NULL )
	{
		/* no - check the target's Cstate */
		if ( (j = find_attr( target, NULL, NULL, 0, ATTR_CSTATE )) == -1 )
			ERROR( ERR_OBJ_MISSING_ATTR, ATTR_CSTATE_T, target->name, NULL )
		if ( ! same_state( target->attrs[j].value, STATE_CREADY ) )
			ERROR( ERR_STATE, target->name, NULL, NULL );
	}

	/* resource is compatable with target type? */
	if ( attr_in_list( target->type->attr, res->type->type ) == FALSE )
		ERROR( ERR_TYPE_OF_RES, res->name, ATTR_msg(target->type->attr), NULL )

	/* if only one resource of this type allowed: */
	if ( attr_in_list( ATTR_SUBCLASS_ONLY_1, res->type->subclass ) )
	{	/* make sure target doesn't already have this type of resource */
		if ( find_attr( target, NULL, NULL, 0, res->type->attr ) >= 0 )
			ERROR( ERR_ALREADY_ALLOC, res->name, target->name, NULL )
	}

	/* resource is available? */
	if (	((j = find_attr( res, NULL, NULL, 0, ATTR_RSTATE )) < 0) ||
					(strtol(res->attrs[j].value,NULL,0) != STATE_AVAILABLE) )
		ERROR( ERR_STATE, res->name, NULL, NULL )

	/* target has route to server of resource? */
	if ( (j = find_attr( res, NULL, NULL, 0, ATTR_SERVER )) < 0 )
		ERROR( ERR_OBJ_MISSING_ATTR, ATTR_SERVER_T, res->name, NULL )
	else if ( net_to_mac( target_net, res->attrs[j].value, nimif ) == FAILURE )
	{
		if ( niminfo.errno != ERR_SERVER_IS_GATEWAY )
			ERROR( ERR_ROUTE_TO_RES, target->name, res->name, NULL )
		else
			/* use the error message which is already initialized */
			return( FAILURE );
	}

	/* if we get here, everything is ok */
	return( SUCCESS );

} /* end of ok_to_allocate */

/*---------------------------- do_allocation        ----------------------------
*
* NAME: do_allocation
*
* FUNCTION:
*		adds the required info to the target & resource objects required for
*			NIM resource allocation
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*		sets errstr on failure
*		this function ASSUMES that <res> was obtained using lag_object
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*			target_id			= id of machine to allocate to
*			res					= ptr to nim_object struct for resource
*			pathname				= pathname to export
*			nimif					= nim_if struct for server; used to understand how
*											to access the resource
*			c_stdout				= ptr to FILE ptr (for redirection of stdout)
*		global:
*			attr_ass
*
* RETURNS: (int)
*		SUCCESS					= resource allocated
*		FAILURE					= unable to add attrs
*
* OUTPUT:
*		writes mount info to stdout when the ATTR_DISP_MNTPNT or ATTR_MOUNT_CTRL
*			have been specified when the command is invoked
*-----------------------------------------------------------------------------*/

int
do_allocation(	struct nim_object *target,
					struct nim_object *res,
					char *pathname,
					struct nim_if *nimif,
					FILE **c_stdout )

{	int i;
	int rmattr = FALSE;
	char *ptr=NULL;
	int rc = SUCCESS;
	char *server;
	int local_res = FALSE;

	VERBOSE3("      do_allocation: target=%s; res=%s; pathname=%s\n",
				target->name,res->name,pathname,NULL)

	if ( (c_stdout == NULL) || (*c_stdout == NULL) )
		c_stdout = stdout;

	/* if pathname is NULL, use default of resource's location */
	if ( pathname == NULL )
	{
		/* find the ATTR_LOCATION */
		if ( (i = find_attr( res, NULL, NULL, 0, ATTR_LOCATION )) < 0 )
			ERROR( ERR_OBJ_MISSING_ATTR, ATTR_LOCATION_T, res->name, NULL )
		pathname = res->attrs[i].value;
	}

	/* find server of resource */
	if ( (i = find_attr( res, NULL, NULL, 0, ATTR_SERVER )) < 0 )
			ERROR( ERR_OBJ_MISSING_ATTR, ATTR_SERVER_T, res->name, NULL )
	server = res->attrs[i].value;

	/* is resource on the target or remote to it? */
	local_res = ( strcmp( target->name, server ) == 0 );

	/* don't need to NFS export if resource is on the target */
	if ( ! local_res )
	{
		if ( grant_access( target, res, pathname ) == FAILURE )
			return( FAILURE );
	}

	/* 
	 * add resource attr for target 
	 */
	if ( mk_attr( target->id,NULL,res->name,0,res->type->attr,NULL ) == SUCCESS )
	{
		rmattr = TRUE;

		/* Increment allocation count */
		if ( attr_count( res->id, NULL, ATTR_ALLOC_COUNT, 0, 1 ) == SUCCESS )
		{
			/* successful allocation! */

			/* need to write any info to stdout? */
			if ( (ptr = attr_value( &attr_ass, ATTR_DISP_MNTPNT )) != NULL )
			{
				/* display how resource can be accessed by client */
				if ( local_res )
					fprintf(	c_stdout, "%s=%s\n", res->type->name, pathname );
				else
					fprintf(	c_stdout, "%s=%s:%s\n", res->type->name, 
								nimif->hostname, pathname );
			}
			else if ( (ptr = attr_value( &attr_ass, ATTR_MOUNT_CTRL )) != NULL )
			{
				/* display the appropriate mount command to be executed */
				if ( local_res )
					fprintf( c_stdout, "%s %s %s\n", MOUNT, pathname, ptr );
				else
					fprintf( c_stdout, "%s %s:%s %s\n", MOUNT, nimif->hostname, 
								pathname, ptr );
			}

			/* if ATTR_CONTROL not already present, add it now */
			rc = SUCCESS;
			if ( get_attr( NULL, NULL, target->id, NULL, 0, ATTR_CONTROL ) <= 0 )
			{
				/* need to add an ATTR_CONTROL */
				/* if ATTR_PULL_REQUEST was specified, then it originates from */
				/*		the client, so put the client in control */
				/* otherwise, the machine we're executing on gets control */
				if ( find_attr_ass( &attr_ass, ATTR_PULL_REQUEST ) >= 0 )
					rc = mk_attr(	target->id, NULL, target->name, 0, 
										ATTR_CONTROL, ATTR_CONTROL_T );
				else
					rc = mk_attr(	target->id, NULL, niminfo.nim_name, 0, 
										ATTR_CONTROL, ATTR_CONTROL_T );
			}

			if ( rc == SUCCESS )
				return( rc );
		}
	}

	/* if we get here, FAILURE has occured */
	/* undo what we've done so far */
	protect_errstr = TRUE;
	if ( rmattr )
		rm_attr( target->id, NULL, res->type->attr, 0, NULL );
	revoke_access( target, res, pathname );

	return( FAILURE );

} /* end of do_allocation */
	
/*---------------------------- allocate             ----------------------------
*
* NAME: allocate
*
* FUNCTION:
*		performs resource allocation
*		the target (user of the resource) is specified by either:
*			<target_name> or
*			<target>
*		this is done so that this function may operate more efficiently in
*			situations where multiple resources will be allocated to the same
*			machine by the caller
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
*			target_name			= name of target if <target> not provided
*			target				= ptr to nim_object if target retrieved by caller
*			nimif					= ptr to nim_if struct if already filled in by caller
*			res_name				= name of resource to allocate
*			res_creation		= flag indicating that alloc will be used for
*										resource creation; >0 will cause some errors to
*										be ignored
*			c_stdout				= ptr to FILE ptr (for redirection of stdout)
*		global:
*
* RETURNS: (int)
*		SUCCESS					= resource allocated
*		FAILURE					= error encountered
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
allocate(	char *target_name,
				struct nim_object *target,
				struct nim_if *nimif,
				char *res_name,
				int res_creation,
				FILE **c_stdout )

{	int rc=SUCCESS;
	NIM_OBJECT( mobj, minfo )
	NIM_OBJECT( res, rinfo )
	NIM_PDATTR( pdattr, pdi )
	struct nim_if local_nimif;
	struct nim_if server_nimif;
	int i;


	/* init local pointers */
	mobj = NULL;
	res = NULL;

	/* target info already retrieved? */
	if ( target == NULL )
	{	/* nope - get the target */
		if ( lag_object( 0, target_name, &mobj, &minfo ) == FAILURE )
			ERROR( 0, NULL, NULL, NULL );

		target = mobj;
	}

	VERBOSE("   allocating the %s resource to %s\n",res_name,target->name,NULL,
				NULL)

	/* target nim_if provided? */
	if ( nimif == NULL )
	{	/* no - find the target's PIF */
		if ( (i = find_attr( target, NULL, NULL, 1, ATTR_IF )) == -1 )
		{	
			rc = FAILURE;
			errstr( ERR_OBJ_MISSING_ATTR, PIF, target->name, NULL );
		}
		else if ( s2nim_if( target->attrs[i].value, &local_nimif ) == FAILURE )
		{	
			rc = FAILURE;
			errstr( ERR_NET_INFO, PIF, target->name, NULL );
		}

		nimif = &local_nimif;
	}

	/* lock the resource object */
	if ( rc == SUCCESS )
		rc = lag_object( 0, res_name, &res, &rinfo );
		
	/* resource already allocated to target? */
	if ( rc == SUCCESS )
	{	
		/* ok to allocate? */
		if (	((rc = ok_to_allocate(	target, nimif->network,
												res, &server_nimif )) == SUCCESS) ||
				((res_creation) && (niminfo.errno == ERR_IF_NS_BY_SPOT)) )
		{	
			/* reset errstr if set in ok_to_allocate */
			niminfo.errno = 0;
			protect_errstr = FALSE;

			/* perform the allocation */
			rc = do_allocation( target, res, NULL, &server_nimif, c_stdout );
		}
		else if ( niminfo.errno == ERR_ALREADY_ALLOC )
		{	
			/* resource is already allocated - reset the error processing */
			rc = SUCCESS;
			niminfo.errno = 0;
			protect_errstr = FALSE;
		}
	}

	/* error encounterd? */
	if ( rc == FAILURE )
		protect_errstr = TRUE;

	/* free any retrieved objects */
	if ( mobj != NULL )
		uaf_object( mobj, &minfo, FALSE );
	if ( res != NULL )
		uaf_object( res, &rinfo, FALSE );

	return( rc );

} /* end of allocate */

/*---------------------------- alloc_res            ----------------------------
*
* NAME: alloc_res
*
* FUNCTION:
*		allocates the specified resource to the specified client by either:
*			1) invoking a resource specific allocation method (if it exists)
*			-or-
*			2) calling the generic allocation function
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*		sets errstr on failure
*		this function ASSUMES that <client> has been obtained using lag_object
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*			client				= ptr to nim_object for machine 
*			res_name				= name of resource to allocate
*			nimif					= optional; if non-NULL, ptr to client's nim_if
*			c_stdout				= ptr to FILE ptr (for redirection of stdout)
*		global:
*			attr_ass
*
* RETURNS: (int)
*		SUCCESS					= <res> allocated to <client>
*		FAILURE					= error encountered
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
alloc_res(	struct nim_object *client,
				char *res_name,
				struct nim_if *nimif,
				FILE **c_stdout )

{	int rc;
	struct nim_if local_nimif;
	int i;
	ODMQUERY

	/* resource already allocated to target? */
	sprintf( query, "^%s$", res_name );
	if ( find_attr( client, NULL, query, 0, get_type( 0, res_name ) ) >= 0 )
		return( SUCCESS );

	/* if there's a resource-specific allocation method, then invoke it */
	if ( (rc=res_specific_method(client, res_name, ATTR_ALLOCATE, c_stdout)) >=0)
	{
		/* method invoked - what was the result? */
		if ( rc == 1 )
			return( SUCCESS );
		else
			return( FAILURE );
	}

	/* if we get here, we'll use the generic allocation function */
	/* to do this, we need client's nim_if: was it supplied? */
	if ( nimif == NULL )
	{
		/* not supplied - get it now */
		if ( (i = find_attr( client, NULL, NULL, 1, ATTR_IF )) < 0 )
			ERROR( ERR_OBJ_MISSING_ATTR, PIF, client->name, NULL )
		if ( s2nim_if( client->attrs[i].value, &local_nimif ) == FAILURE )
			return( FAILURE );

		nimif = &local_nimif;
	}

	/* allocate the resource */
	return( allocate( NULL, client, nimif, res_name, FALSE, c_stdout ) );

} /* end of alloc_res */

/*$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
$$$$$$$$$$$$$$$$ resource deallocation                             $$$$$$$$$$$$
$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$*/
	
/*---------------------------- ok_to_deallocate      ---------------------------
*
* NAME: ok_to_deallocate
*
* FUNCTION:
*		performs all the checks necessary in order to ensure that <res> can
*			be deallocated from <client>
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*		sets errstr on failure
*		this function ASSUMES that the objects involved have already been
*			retrieved using lag_object
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*			client				= machine which is using <res>
*			res					= resource to be deallocated from <client>
*		global:
*			attr_ass
*
* RETURNS: (int)
*		SUCCESS					= ok to deallocate
*		FAILURE					= not ok to deallocate
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
ok_to_deallocate(	struct nim_object *client,
						struct nim_object *res )

{	int i;

	VERBOSE3("      ok_to_deallocate: client=%s; res=%s;\n",client->name,
				res->name,NULL,NULL)

	/* ignore the client's Cstate? */
	if ( attr_value( &attr_ass, ATTR_IGNORE_STATE ) <= NULL )
	{
		/* no - check it */
		if ( (i = find_attr( client, NULL, NULL, 0, ATTR_CSTATE )) < 0 )
			ERROR( ERR_OBJ_MISSING_ATTR, ATTR_CSTATE_T, client->name, NULL )
		if ( ! same_state( client->attrs[i].value, STATE_CREADY ) )
			ERROR( ERR_STATE, client->name, NULL, NULL )
	}

	/* ok to deallocate */
	return( SUCCESS );

} /* end of ok_to_deallocate */

/*---------------------------- do_deallocation        --------------------------
*
* NAME: do_deallocation
*
* FUNCTION:
*		adds the required info to the client & resource objects required for
*			NIM resource allocation
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*		sets errstr on failure
*		this function ASSUMES that <res> was obtained using lag_object
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*			client_id			= id of machine to allocate to
*			res					= ptr to nim_object struct for resource
*			pathname				= pathname to export
*			c_stdout				= ptr to FILE ptr (for redirection of stdout)
*		global:
*			attr_ass
*
* RETURNS: (int)
*		SUCCESS					= resource allocated
*		FAILURE					= unable to add attrs
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
do_deallocation(	struct nim_object *client,
						struct nim_object *res,
						char *pathname,
						FILE **c_stdout )

{	int i;
	char *ptr;
	int rc = SUCCESS;
	NIM_ATTR( attrs, ainfo )
	int rm_control = TRUE;

	VERBOSE3("      do_deallocation: client=%s; res=%s; pathname=%s;\n",
				client->name,res->name,pathname,NULL)

	if ( (c_stdout == NULL) || (*c_stdout == NULL) )
		c_stdout = stdout;

	/* if pathname is NULL, use default of resource's location */
	if ( pathname == NULL )
	{
		/* find the ATTR_LOCATION */
		if ( (i = find_attr( res, NULL, NULL, 0, ATTR_LOCATION )) < 0 )
			ERROR( ERR_OBJ_MISSING_ATTR, ATTR_LOCATION_T, res->name, NULL )
		pathname = res->attrs[i].value;
	}

	/* remove the resource attr, decre alloc count, and remove NFS access */
	if ( rm_attr( client->id, NULL, res->type->attr, 0, res->name ) == FAILURE )
		return( FAILURE );
	else if (	(attr_count( res->id, NULL, ATTR_ALLOC_COUNT, 0, -1 )==FAILURE)||
					(revoke_access( client, res, pathname ) == FAILURE) )
		rc = FAILURE;
	else if ( (ptr = attr_value( &attr_ass, ATTR_MOUNT_CTRL )) != NULL )
	{
		/* display the appropriate mount command to be executed */
		fprintf( c_stdout, "%s %s\n", UMOUNT, ptr );
	}

	/* do we need to remove the ATTR_CONTROL attribute? */
	/* we do under the following conditions: */
	/*		1) the attr is present */
	/*		2) value of ATTR_CONTROL has only 1 field (indicating tmp control) */
	/*		3) there are no other resources allocated */
	if ( (i = find_attr( client, NULL, NULL, 0, ATTR_CONTROL )) >= 0 )
	{
		/* one field or two? */
		if ( two_fields( client->attrs[i].value, NULL, NULL ) == FAILURE )
		{
			/* any resources still allocated? */
			if ( get_attr( &attrs, &ainfo, client->id, NULL, 0, 0 ) > 0 )
			{
				for (i=0; i < ainfo.num; i++)
					if ( attrs[i].pdattr->class == ATTR_CLASS_RESOURCES )
					{
						rm_control = FALSE;
						break;
					}

				odm_free_list( attrs, &ainfo );
			}

			/* remove ATTR_CONTROL? */
			if ( rm_control )
				rm_attr( client->id, NULL, ATTR_CONTROL, 0, NULL );
		}
	}

	return( rc );

} /* end of do_deallocation */
	
/*---------------------------- deallocate             --------------------------
*
* NAME: deallocate
*
* FUNCTION:
*		performs generic resource deallocation
*		the client (user of the resource) is specified by either:
*			<client_name> or
*			<client>
*		this is done so that this function may operate more efficiently in
*			situations where multiple resources will be deallocated to the same
*			machine by the caller
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
*			client_name			= name of client if <client> not provided
*			client				= ptr to nim_object if client retrieved by caller
*			res_name				= name of resource to deallocate
*			c_stdout				= ptr to FILE ptr (for redirection of stdout)
*		global:
*
* RETURNS: (int)
*		SUCCESS					= resource deallocated
*		FAILURE					= error encountered
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
deallocate(	char *client_name,
				struct nim_object *client,
				char *res_name,
				FILE **c_stdout )

{	int rc;
	NIM_OBJECT( mobj, minfo )
	NIM_OBJECT( res, rinfo )
	int i;
	char	*location; 

	/* make sure there's something to deallocate */
	if ( (res_name == NULL) || (*res_name == NULL_BYTE) )
		return( SUCCESS );

	/* init local pointers */
	mobj = NULL;

	/* client info already retrieved? */
	if ( client == NULL )
	{	/* nope - get the client */
		if ( lag_object( 0, client_name, &mobj, &minfo ) == FAILURE )
			return( FAILURE );

		client = mobj;
	}

	/* get the resource object */
	if ( lag_object( 0, res_name, &res, &rinfo ) == FAILURE )
		return( FAILURE );

	/* since this is generic deallocation, we'll use the location specified */
	/*		by the resource's ATTR_LOCATION attribute */
	if ( (i = find_attr( res, NULL, NULL, 0, ATTR_LOCATION )) < 0 )
		ERROR( ERR_OBJ_MISSING_ATTR, ATTR_LOCATION_T, res->name, NULL )
	location = res->attrs[i].value;

	VERBOSE("   deallocating the %s resource from %s\n",res->name,client->name,
				NULL,NULL)

	/* deallocate the resource */
	if (	((rc = ok_to_deallocate( client, res )) == SUCCESS) &&
			((rc = do_deallocation( client, res, location, c_stdout )) == SUCCESS))
		rc = SUCCESS;

	/* free any retrieved objects */
	if ( mobj != NULL )
		uaf_object( mobj, &minfo, FALSE );
	uaf_object( res, &rinfo, FALSE );

	return( rc );

} /* end of deallocate */
	
/*---------------------------- dealloc_res        ------------------------------
*
* NAME: dealloc_res
*
* FUNCTION:
*		deallocates the specified resource for the specified client by either:
*			1) invoking a resource specific deallocation method (if it exists)
*			-or-
*			2) calling the generic deallocation function
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*		sets errstr on failure
*		this function ASSUMES that <client> has been obtained using lag_object
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*			client				= ptr to client's nim_object struct
*			res_name				= name of res to deallocate
*			c_stdout				= ptr to FILE ptr (for redirection of stdout)
*		global:
*
* RETURNS: (int)
*		SUCCESS					= resource deallocated for specified <client>
*		FAILURE					= error
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
dealloc_res(	struct nim_object *client,
					char *res_name,
					FILE **c_stdout )

{	int rc;
	ODMQUERY

	/* if resource isn't currently allocated to the target, return success */
	sprintf( query, "^%s$", res_name );
	if ( find_attr( client, NULL, query, 0, get_type( 0, res_name ) ) < 0 )
		return( SUCCESS );

	/* if there's a resource-specific deallocation method, then invoke it */
	if ((rc=res_specific_method(client, res_name, ATTR_DEALLOCATE, c_stdout))>=0)
	{
		/* method invoked - what was the result? */
		if ( rc == 1 )
			return( SUCCESS );
		else
			return( FAILURE );
	}

   /* if we get here, we'll use the generic deallocation function */
	return( deallocate( NULL, client, res_name, c_stdout ) );

} /* end of dealloc_res */
	
/*---------------------------- dealloc_LIST      ------------------------------
*
* NAME: dealloc_LIST
*
* FUNCTION:
*		deallocates all of the resources specified in LIST
*		the client (user of the resource) is specified by either:
*			<client_name> or
*			<client>
*		this is done so that this function may operate more efficiently in
*			situations where multiple resources will be deallocated to the same
*			machine by the caller
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*		sets errstr on failure
*		this function will attempt to perform ALL deallocations specified in
*			<list>; ie, it will not stop when an error is encountered
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*			client_name			= name of client if <client> not provided
*			client				= ptr to nim_object if client retrieved by caller
*			list					= LIST of resource names to deallocate
*			c_stdout				= ptr to FILE ptr (for redirection of stdout)
*		global:
*
* RETURNS: (int)
*		SUCCESS					= all resources deallocated
*		FAILURE					= error in one or more deallocations
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
dealloc_LIST(	char *client_name,
					struct nim_object *client,
					LIST *list,
					FILE **c_stdout )

{	int rc=SUCCESS;
	NIM_OBJECT( mobj, minfo )
	int i;

	/* make sure there's something to deallocate */
	if ( list->num <= 0 )
		return( SUCCESS );

	/* init local pointers */
	mobj = NULL;

	/* client info already retrieved? */
	if ( client == NULL )
	{	/* nope - get the client */
		if ( lag_object( 0, client_name, &mobj, &minfo ) == FAILURE )
			ERROR( 0, NULL, NULL, NULL );

		client = mobj;
	}

	/* for each entry in LIST... */
	for (i=0; i < list->num; i++)
		/* deallocate it */
		if ( dealloc_res( client, list->list[i], c_stdout ) == FAILURE )
		{
			warning( 0, NULL, NULL, NULL );
			rc = FAILURE;
		}

	/* reset the allocation LIST */
	reset_LIST( list );

	/* free any retrieved objects */
	if ( mobj != NULL )
		uaf_object( mobj, &minfo, FALSE );

	return( rc );

} /* end of dealloc_LIST */

/*$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
$$$$$$$$$$$$$$$$ NFS access processing                             $$$$$$$$$$$$
$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$*/
	
/* -------------------------< grant_access 
 *
 * NAME: grant_access
 *
 * FUNCTION: Grant the client NFS ROOT access to the res resource.
 *
 *
 * DATA STRUCTURES:
 *	parameters:
 *
 * RETURNS: (int)
 *	FAILURE	  =  Something went wrong
 *	SUCCESS	  =  Everything was OK
 *
 * -------------------------------------------------------------------- */ 

int 
grant_access ( struct nim_object *client,
					struct nim_object *res,
					char *pathname )

{
	NIM_PDATTR( pdattr, pdi )
	char	*server; 
	char	location[MAX_TMP]; 	
	char	grant[MAX_TMP]; 	
	char	perms[MAX_TMP]; 	
	struct nim_if *nif; 
	
	long	id; 
	int	rc;
	FILE	*c_stdout = NULL; 
	int	svr_ndx; 

	char	*Args[] = { C_CH_NFSEXP, location, grant, perms, NULL }; 	

	VERBOSE3("      grant_access: client=%s; res=%s; pathname=%s;\n",
				client->name,res->name,pathname,NULL)

	/* see if the res has nfs perms, if so grant access */ 
	if ( get_pdattr( &pdattr, &pdi, ATTR_CLASS_RESOURCES, ATTR_SUBCLASS_INFO,
			res->type->attr, ATTR_NFS_PERMS, NULL ) <= 0 )
		return(SUCCESS);	
				
	/* find resource server */
	if ( (svr_ndx=find_attr(res, NULL, NULL, 0, ATTR_SERVER)) < 0  ) 
		ERROR( ERR_OBJ_MISSING_ATTR, ATTR_SERVER_T, res->name, NULL )
	server = res->attrs[svr_ndx].value;

	/* don't need to export if resource server is the client */
	if ( strcmp( client->name, server ) == 0 )
		return( SUCCESS );

	/* convert client's PIF into nimif struct */
	if ( (nif=getPif(client->name)) == NULL )
		return(FAILURE); 

	/* initialize params to C_CH_NFSEXP */
	sprintf(location, "-a%s=%s",ATTR_LOCATION_T, pathname );
	sprintf(grant, "-a%s=%s",ATTR_GRANT_T, nif->hostname);
	sprintf(perms, "-a%s=%s", ATTR_NFS_PERMS_T,  pdattr->value);

 	if ( master_exec(server, &rc, &c_stdout, Args) == FAILURE )
		return( FAILURE );
	else if ( rc > 0 )
	{
		errstr( ERR_METHOD, server, niminfo.errstr, NULL );
		rc = FAILURE;
	}
	else
		rc = SUCCESS;

	if ( rc == SUCCESS )
		attrs_from_FILE( client->id, c_stdout );

	fclose( c_stdout );

	return( rc );

} /* end of grant_access */

/* -------------------------< revoke_access 
 *
 * NAME: revoke_access
 *
 * FUNCTION:
 *
 *
 * DATA STRUCTURES:
 *	parameters:
 *
 * RETURNS: (int)
 *	FAILURE	  =  Something went wrong
 *	SUCCESS	  =  Everything was OK
 *
 * -------------------------------------------------------------------- */ 

int 
revoke_access( struct nim_object *client,
					struct nim_object *res,
					char *pathname )

{
	char	*server; 
	char	location[MAX_TMP]; 	
	char	revoke[MAX_TMP]; 	
	struct nim_if *nif; 
	
	long	id; 
	int	rc;
	FILE	*c_stdout = NULL; 
	int	svr_ndx; 

	char	*Args[] = { C_CH_NFSEXP, location, revoke, NULL }; 	

	VERBOSE3("      revoke_access: client=%s; res=%s; pathname=%s;\n",
				client->name,res->name,pathname,NULL)

	/* is resource exported to client? */
	if ( find_attr( client, NULL, pathname, 0, ATTR_EXPORTED ) < 0 )
		/* nope - wasn't explicitly exported; nothing else to do */
		return( SUCCESS );

	if ( (nif=getPif(client->name)) == NULL )
		return(FAILURE); 

	sprintf( location, "-a%s=%s", ATTR_LOCATION_T, pathname );
	sprintf( revoke, "-a%s=%s", ATTR_REVOKE_T, nif->hostname );

	if ( (svr_ndx=find_attr(res, NULL, NULL, 0, ATTR_SERVER)) < 0  ) 
		ERROR( ERR_OBJ_MISSING_ATTR, ATTR_SERVER_T, res->name, NULL )

	server = res->attrs[svr_ndx].value;

 	if ( master_exec(server, &rc, &c_stdout, Args) == FAILURE )
		return( FAILURE );
	else if ( rc > 0 )
	{
		errstr( ERR_METHOD, server, niminfo.errstr, NULL );
		rc = FAILURE;
	}
	else
		rc = SUCCESS;

	fclose( c_stdout );

	/* remove the ATTR_EXPORTED */
	/* NOTE that we're doing this even if the NFS unexport fails because, at */
	/*		this point, NIM considers this resource to be unexported */
	rm_attr( client->id, NULL, ATTR_EXPORTED, 0, pathname );

	return( rc );

} /* end of revoke_access */

/*$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
$$$$$$$$$$$$$$$$ common SPOT processing                            $$$$$$$$$$$$
$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$*/

/*---------------------------- ok_to_op_spot     ------------------------------
*
* NAME: ok_to_op_spot
*
* FUNCTION:
*		detemines if it is ok to perform an operation on a SPOT
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*		sets errstr on failure
*		ASSUMES <spot> has been retrieved using lag_object
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*			spot					= ptr to nim_object struct for SPOT
*		global:
*
* RETURNS: (int)
*		SUCCESS					= op may be performed
*		FAILURE					= state or alloc_count prevents op from occurring
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
ok_to_op_spot(	struct nim_object *spot )

{	int Rstate;
	int alcnt;

	VERBOSE("   verifying that SPOT may be operated on at this time\n",NULL,NULL,
				NULL,NULL)

	/* ATTR_FORCE used? */
	force = ( (force) || (find_attr_ass( &attr_ass, ATTR_IGNORE_STATE) >= 0));

	/* find the SPOT's Rstate, server & location */
	if ( (Rstate = find_attr( spot, NULL, NULL, 0, ATTR_RSTATE )) < 0 )
		ERROR( ERR_OBJ_MISSING_ATTR, ATTR_RSTATE_T, spot->name, NULL )
	else if ( (alcnt = find_attr( spot, NULL, NULL, 0, ATTR_ALLOC_COUNT )) < 0 )
		ERROR( ERR_OBJ_MISSING_ATTR, ATTR_ALLOC_COUNT_T, spot->name, NULL )
	Rstate = (int) strtol( spot->attrs[Rstate].value, NULL, 0 );
	alcnt = (int) strtol( spot->attrs[alcnt].value, NULL, 0 );

	/* check Rstate: need force flag if "unavailable" */
	if ( (Rstate == STATE_UNAVAILABLE) && (! force) )
		ERROR( ERR_STATE, spot->name, NULL, NULL )

	/* check allocation count */
	if ( (alcnt > 0) && (! force) )
		ERROR( ERR_RES_BUSY, spot->name, NULL, NULL )

	return( SUCCESS );

} /* end of ok_to_op_spot */
	
/*---------------------------- check_spot        ------------------------------
*
* NAME: check_spot
*
* FUNCTION:
*		invokes M_CKSPOT to check the specified spot
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
*			name					= name of SPOT to check
*			server_name			= name of SPOT server
*			alloc_list			= LIST of resources to be deallocated
*		global:
*
* RETURNS: (int)
*		SUCCESS					= SPOT is ok
*		FAILURE					= error encountered for SPOT not ok
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
check_spot(	char *name,
				char *server_name,
				LIST *alloc_list )

{	int rc;
	FILE *c_stdout=NULL;
	char lock[MAX_TMP];
	char ignoreState[MAX_TMP];
	char debug[MAX_TMP];
	char auto_expand[MAX_TMP];
	char installp_flags[MAX_TMP];
	char *flags=NULL;
	char *ckspot[] = { M_CKSPOT, lock, ignoreState, NULL, NULL, NULL, NULL, NULL };
	int i=2;
	int  xpand_idx; 

	disable_err_sig();

	/* dealloc any resources allocated to the SPOT server */
	dealloc_LIST( server_name, NULL, alloc_list, &c_stdout );

	/* prepare params for M_CKSPOT */
	/* ignore locks */
	sprintf( lock, "-a%s=yes", ATTR_IGNORE_LOCK_T );

	/* ignore state */
	sprintf( ignoreState, "-a%s=yes", ATTR_IGNORE_STATE_T );

	/* use ATTR_DEBUG? */
	if ( attr_value( &attr_ass, ATTR_DEBUG ) != NULL )
	{
		sprintf( debug, "-a%s=yes", ATTR_DEBUG_T );
		ckspot[ ++i ] = debug;
	}

	/* ATTR_AUTO_EXPAND specified? */
	if ( (xpand_idx=find_attr_ass( &attr_ass, ATTR_AUTO_EXPAND) ) >= 0 ) {
		sprintf( auto_expand, "-a%s=%s", ATTR_AUTO_EXPAND_T,
			attr_ass.list[xpand_idx]->value);
		ckspot[ ++i ] = auto_expand;
	}

	/* ATTR_INSTALLP_FLAGS specified? */
	/* was allow this to be passed to M_CKSPOT in case the flags have the */
	/*		auto expand flag ("X") */
	if ( (flags = attr_value( &attr_ass, ATTR_INSTALLP_FLAGS )) != NULL )
	{
		sprintf( installp_flags, "-a%s=%s", ATTR_INSTALLP_FLAGS_T, flags );
		ckspot[ ++i ] = installp_flags;
	}

	/* SPOT name */
	ckspot[ ++i ] = name;

	/* invoke M_CKSPOT method to check SPOT's status */
	if ( client_exec( &rc, &c_stdout, ckspot ) == FAILURE )
		ERROR( 0, NULL, NULL, NULL )
	else if ( rc > 0 )
		ERROR( ERR_METHOD, niminfo.nim_name, niminfo.errstr, NULL )

	return( SUCCESS );

} /* end of check_spot */

/*---------------------- sync_usr_spot_bos_inst_root  -------------------------
*
* NAME: sync_usr_spot_bos_inst_root
*
* FUNCTION:	performs a root_sync op on the /usr/lpp/bos/inst_root
*		directory for a /usr SPOT.
*		
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*		
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*			spot			= nim_object for the SPOT 
*						  to sync roots for
*			spot_server		= name of SPOT's server
*			installp_flags		= installp flags to use
*			installp_options	= installp options to use
*			bundle			= pathname of bundle file to use
*
* RETURNS: (int)
*		SUCCESS				= bos/inst_root sync'd 
*						  successfully
*		FAILURE				= one or more errors encountered
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
sync_usr_spot_bos_inst_root(	struct nim_object *spot,
				char *spot_server,
				char *installp_flags,
				char *installp_options,
				char *bundle )
{
	LIST params;
	int rc;
	int args;
	FILE *c_stdout = NULL;
	char **arg_list;

        /* initialize the parameter LIST */
        if ( ! get_list_space( &params, DEFAULT_CHUNK_SIZE, TRUE ) )
                nim_error( 0, NULL, NULL, NULL );

	/* build the c_sync_root command */
	args = 0;
	arg_list = params.list;
	params.list[ args++ ] = C_SYNC_ROOT;

	params.list[ args++ ] = "-s";
	
	/* SPOT location */
	params.list[ args ] = nim_malloc(	strlen(ATTR_LOCATION_T) + 8);
	sprintf( params.list[ args++ ], "-a%s=%s", ATTR_LOCATION_T, "/usr");

	/* installp flags */
	if ( installp_flags != NULL )
	{
		params.list[ args ] = nim_malloc(strlen(ATTR_INSTALLP_FLAGS_T) +
						strlen(installp_flags)+ 4 );
		sprintf( params.list[ args++ ], "-a%s=%s", 
						ATTR_INSTALLP_FLAGS_T, 
						installp_flags );
	}

	/* installp options */
	if ( installp_options != NULL )
	{
		params.list[ args ] = nim_malloc(strlen(ATTR_INSTALLP_OPTIONS_T) 						+ strlen(installp_options)+ 4 );
		sprintf( params.list[ args++ ], "-a%s=%s", 
					ATTR_INSTALLP_OPTIONS_T, 
					installp_options );
	}

	/* bundle file */
	if ( bundle != NULL )
	{
		params.list[ args ] = nim_malloc(	strlen(ATTR_BUNDLE_T) +
							strlen(bundle)+ 4 );
		sprintf( params.list[ args++ ], "-a%s=%s", ATTR_BUNDLE_T, 
							bundle );
	}
	params.list[ args++ ] = "/usr/lpp/bos/inst_root";

	/* execute the C_SYNC_ROOT script */
	if ( master_exec( spot_server, &rc, &c_stdout, arg_list ) == FAILURE )
		/*
		 * Quit for serious errors.
		 */
		nim_error (0, NULL, NULL, NULL);
	else
           if (rc > 0)
		/*
		 * Other failures will be reported as warnings by caller.
		 */
		return( FAILURE );

	return (SUCCESS);

} /* end of sync_usr_spot_bos_inst_root */

/*---------------------------- sync_roots        ------------------------------
*
* NAME: sync_roots
*
* FUNCTION:
*		synchronizes all diskless/dataless root parts which are 
*		associated with the specified SPOT.  
*		ALSO:  perform a sync op on the /usr/lpp/bos/inst_root
*		directory of a /usr SPOT.
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
*			spot					= nim_object for the SPOT to sync roots for
*			installp_flags		= installp flags to use
*			installp_options	= installp options to use
*			bundle				= pathname of bundle file to use
*		global:
*			attr_ass
*
* RETURNS: (int)
*		SUCCESS					= all client roots sync'd successfully
*		FAILURE					= one or more errors encountered
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

#define DEFAULT_ROOTS_IN_PARALLEL					5

int
sync_roots(	struct nim_object *spot,
				char *installp_flags,
				char *installp_options,
				char *bundle )

{	int rc;
	int result = SUCCESS;
	int i,j;
	int max_buf, len, name_len;
	char *spot_server;
	char *spot_path;
	char *num_parallel;
	int state, roots_in_parallel;
	int args;
	char **arg_list;
	LIST params;
	LIST roots;
	ODMQUERY
	NIM_PDATTR( pdattr, pinfo )
	NIM_ATTR( attr, ainfo )
	NIM_OBJECT( obj, oinfo )
	long loc_id;
	struct nim_attr location;
	char tmp[MAX_VALUE];
	FILE *fp;
	struct nim_pdattr *type;

	/* find SPOT pathname */
	if ( (i = find_attr( spot, NULL, NULL, 0, ATTR_LOCATION )) < 0 )
		ERROR( ERR_OBJ_MISSING_ATTR, ATTR_LOCATION_T, spot->name, NULL )
	spot_path = spot->attrs[i].value;

	/* find SPOT server */
	if ( (i = find_attr( spot, NULL, NULL, 0, ATTR_SERVER )) < 0 )
		ERROR( ERR_OBJ_MISSING_ATTR, ATTR_SERVER_T, spot->name, NULL )
	spot_server = spot->attrs[i].value;

	/*
	 * For /usr spots, sync the /usr/lpp/bos/inst_root directory. 
	 */
	if (strcmp (spot_path, "/usr") == 0)
		if (sync_usr_spot_bos_inst_root(spot, 
						spot_server,
						installp_flags, 
						installp_options, 
						bundle) == FAILURE)
			warning (0, NULL, NULL, NULL);

	/* get all objects which reference this spot */
	if ( get_attr( &attr, &ainfo, 0, spot->name, 0, ATTR_SPOT ) < 0 )
		return( FAILURE );
	else if ( ainfo.num == 0 )
		/* nothing to do - this SPOT isn't allocated to anybody */
		return( SUCCESS );

	/* verify that there are diskless/dataless clients which use this SPOT */
	for (i=0; i < ainfo.num; i++)
	{
		/* skip anything that's not ATTR_SPOT */
		if ( attr[i].pdattr->attr != ATTR_SPOT )
			continue;

		/* get this object's type */
		if ( (type = get_type_pdattr( attr[i].id, NULL )) != NULL )
		{
			/* looking for the first diskless or dataless machine */
			if ( (type->attr == ATTR_DISKLESS) || (type->attr == ATTR_DATALESS) )
				break;
		}
	}
	if ( i >= ainfo.num )
		/* nothing to do */
		return( SUCCESS );

	VERBOSE("   performing root sync operations on the following roots:",NULL,
				NULL,NULL,NULL)

	/* num roots in parallel specified as param? */
	if ( (num_parallel = 
						attr_value( &attr_ass, ATTR_NUM_PARALLEL_SYNCS )) == NULL )
	{
		/* parallel syncs in database? */
		if ( get_pdattr(	&pdattr, &pinfo, 0, 0, ATTR_SPOT, 
								ATTR_NUM_PARALLEL_SYNCS, 	
								ATTR_NUM_PARALLEL_SYNCS_T ) > 0 )
			num_parallel = pdattr->value;
	}
	roots_in_parallel = (int) strtol( num_parallel, NULL, 0 );
	if ( roots_in_parallel < 1 )
		roots_in_parallel = DEFAULT_ROOTS_IN_PARALLEL;

	/* initialize param LIST for C_SYNC_ROOT */
	if ( get_list_space( &params, (roots_in_parallel+6), TRUE ) == FAILURE )
		ERROR( 0, NULL, NULL, NULL )

	/* initialize LIST to cache client root pathnames */
	if ( get_list_space( &roots, DEFAULT_CHUNK_SIZE, TRUE ) == FAILURE )
		ERROR( 0, NULL, NULL, NULL )

	/* create a tmp file to hold the names of clients which fail the sync */
	sprintf( tmp, "%s/failed.sync", niminfo.tmps );
	if ( (fp = fopen( tmp, "w" )) == NULL )
		ERROR( ERR_ERRNO, NULL, NULL, NULL )

	/* for each reference... */
	for ( i=0; i < ainfo.num; i++ )
	{
		/* get this object */
		if ( lag_object( attr[i].id, NULL, &obj, &oinfo ) == FAILURE )
			return( FAILURE );

		/* is this a diskless or dataless machine? */
		if (	(obj->type->attr == ATTR_DISKLESS) || 
				(obj->type->attr == ATTR_DATALESS) )
		{
			/* do some checking and add it's root pathname to the LIST */
			/* NOTE that any error here and we'll write the client's name */
			/*		to the tmp file for display at the end of the operation */

			/* we only want to do a root sync operation if the client's root */
			/*		directory has actually been initialized (ie, it's been */
			/*		populated with the inst_root files) */
			/* we can't just check to see whether a root resource has been */
			/*		allocated to the client, because root allocation does not */
			/*		populate the client's directory */
			/* so, we're looking at both Cstate and ATTR_ROOT_INITIALIZED: */
			/*		for diskless machines, ATTR_ROOT_INITIALIZED is added when */
			/*			the dkls_init operation is performed */
			/*		for dataless machines, ATTR_ROOT_INITIALIZED is added AFTER */
			/*			the client's local paging/dump devices are made; that's */
			/*			why we're checking the Cstate also */
			if (	(find_attr( obj, NULL, NULL, 0, ATTR_ROOT_INITIALIZED) < 0) &&
					(get_state( obj->id, NULL, ATTR_CSTATE ) != STATE_DTLS_INIT) )
				continue;

			/* find the location of its root directory */
			if (	((j = find_attr( obj, NULL, NULL, 0, ATTR_ROOT )) < 0) ||
					((loc_id = get_id( obj->attrs[j].value )) == 0) )
			{
				fprintf( fp, "\t%s\n", obj->name );
				result = FAILURE;
				continue;
			}
			sprintf( query, "id=%d and pdattr=%d", loc_id, ATTR_LOCATION );
			if ( odm_get_first( nim_attr_CLASS, query, &location ) == NULL )
			{
				fprintf( fp, "\t%s\n", obj->name );
				result = FAILURE;
				continue;
			}
			sprintf( tmp, "%s/%s", location.value, obj->name );
			if ( add_to_LIST( &roots, tmp ) == FAILURE )
				return( FAILURE );
		}
		else
			/* free this object */
			uaf_object( obj, &oinfo, FALSE );
	}

	/* initialize common args for C_SYNC_ROOT: */
	/* method name */
	args = 0;
	arg_list = params.list;
	params.list[ args++ ] = C_SYNC_ROOT;

	/* SPOT location */
	params.list[ args ] = nim_malloc(	strlen(ATTR_LOCATION_T) + 
													strlen(spot_path)+ 4 );
	sprintf( params.list[ args++ ], "-a%s=%s", ATTR_LOCATION_T, spot_path );

	/* installp flags */
	if ( installp_flags != NULL )
	{
		params.list[ args ] = nim_malloc(	strlen(ATTR_INSTALLP_FLAGS_T) + 
														strlen(installp_flags)+ 4 );
		sprintf( params.list[ args++ ], "-a%s=%s", ATTR_INSTALLP_FLAGS_T, 
					installp_flags );
	}

	/* installp options */
	if ( installp_options != NULL )
	{
		params.list[ args ] = nim_malloc(	strlen(ATTR_INSTALLP_OPTIONS_T) + 
														strlen(installp_options)+ 4 );
		sprintf( params.list[ args++ ], "-a%s=%s", ATTR_INSTALLP_OPTIONS_T, 
					installp_options );
	}

	/* bundle file */
	if ( bundle != NULL )
	{
		params.list[ args ] = nim_malloc(	strlen(ATTR_BUNDLE_T) + 
														strlen(bundle)+ 4 );
		sprintf( params.list[ args++ ], "-a%s=%s", ATTR_BUNDLE_T, bundle );
	}

	/* modify the LIST struct so that the params above are never reset */
	params.max_num = roots_in_parallel + 1;
	params.list = params.list + args;

	/* sync the roots */
	i = 0;
	result = SUCCESS;
	while ( i < roots.num )
	{
		j = 0;
		reset_LIST( &params );

		/* add root pathnames to params LIST */
		while ( (i < roots.num) && (j < roots_in_parallel) )
		{
			VERBOSE("      %s\n",roots.list[i],NULL,NULL,NULL)

			if ( get_list_space( &params, 1, FALSE ) == FAILURE )
				return( FAILURE );
			params.list[ params.num++ ] = roots.list[i];

			j++;
			i++;
		}

		/* execute C_SYNC_ROOT for these roots */
		if ( master_exec( spot_server, &rc, &fp, arg_list ) == FAILURE )
			return( FAILURE );
		else if ( rc > 0 )
		{
			/* if the rc is something other than ERR_ROOT_SYNC, then there's */
			/*		some other kind of problem - stop now */
			if ( rc != ERR_ROOT_SYNC )
				ERROR( ERR_METHOD, spot_server, niminfo.errstr, NULL )

			result = FAILURE;
		}
	}

	/* close the tmp file */
	fclose( fp );

	/* any errors? */
	if ( result == FAILURE )
	{
		/* initialize the error message */
		errstr( ERR_ROOT_SYNC, NULL, NULL, NULL );
		len = strlen( niminfo.errstr );

		/* reserve room in the error msg for client names */
		niminfo.errstr = nim_realloc( niminfo.errstr, len, (MAX_VALUE+1) );
		max_buf = len + MAX_VALUE;

		/* open the tmp file again */
		sprintf( tmp, "%s/failed.sync", niminfo.tmps );
		fp = fopen( tmp, "r" );

		/* read from tmp file and copy into niminfo.errstr */
		while ( fgets( tmp, MAX_VALUE, fp ) != NULL )
		{
			name_len = strlen( tmp );
			len += name_len;
			if ( len >= max_buf )
			{
				/* alloc more space */
				niminfo.errstr = nim_realloc(niminfo.errstr,max_buf,(MAX_VALUE+1) );
				max_buf += MAX_VALUE;
			}
			strcat( niminfo.errstr, tmp );
		}

		/* close the tmp file */
		fclose( fp );

		return( FAILURE );
	}

	return( SUCCESS );

} /* end of sync_roots */

