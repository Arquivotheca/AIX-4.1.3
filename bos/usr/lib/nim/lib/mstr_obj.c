static char sccsid[] = "@(#)91  1.37  src/bos/usr/lib/nim/lib/mstr_obj.c, cmdnim, bos411, 9439C411f 9/30/94 21:07:48";
/*
 *   COMPONENT_NAME: CMDNIM
 *
 *   FUNCTIONS: attr_in_list
 *		class_s2i
 *		find_attr
 *		get_attr
 *		get_class
 *		get_id
 *		get_name
 *		get_object
 *		get_pdattr
 *		mk_attr
 *		mk_object
 *		pdattr_s2i
 *		rm_attr
 *		rm_object
 *		subclass_s2i
 *		type_s2i
 *		valid_pdattr_ass
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
$$$$$$$$$$$$$$$$ object manipulation                               $$$$$$$$$$$$
$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$*/

/*---------------------------- get_pdattr        ------------------------------
*
* NAME: get_pdattr
*
* FUNCTION:
*		returns the number of nim_pdattrs found for the specified attr
*		the attr may be specified by any combination of the "class",
*			"subclasss", "type", "attr", and/or "name" nim_pdattr fields
*		the list of pd_attrs is also returned when the "pdattr" and "info" 
*			parameters are non-NULL; when either is NULL, 0 or 1 will be returned
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*			pdattr				= ptr to nim_pdattr struct ptr
*			info					= ptr to listinfo struct
*			class					= nim_pdattr.class
*			subclass				= nim_pdattr.subclass
*			type					= nim_pdattr.type
*			attr					= nim_pdattr.attr
*			name					= nim_pdattr.name
*		global:
*
* RETURNS: (number of nim_pdattrs )
*		>0							= at least one nim_pdattr found
*		0							= no attr found
*		-1							= ODM error
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
get_pdattr(	struct nim_pdattr **pdattr, 
				struct listinfo *info, 
				int class, 
				int subclass, 
				int type, 
				int attr, 
				char *name )

{	char query[MAX_CRITELEM_LEN];
	char tmp[MAX_CRITELEM_LEN];
	struct nim_pdattr lattr;
	int existance_query;

	VERBOSE5("         get_pdattr: class=%d; subclass=%d; type=%d; attr=%d;\n",
				class,subclass,type,attr)

	/* caller wants list? */
	existance_query = ( (pdattr == NULL) || (info == NULL) ) ? TRUE : FALSE;

	/* form the query */
	query[0] = NULL_BYTE;
	if ( class > 0 )
		sprintf( query, "class=%d", class );

	if ( subclass > 0 )
	{	if ( query[0] != NULL_BYTE )
			sprintf( tmp, " and subclass like '*,%d,*'", subclass );
		else
			sprintf( tmp, "subclass like '*,%d,*'", subclass );
		strncat( query, tmp, MAX_CRITELEM_LEN );
	}

	if ( type > 0 )
	{	if ( query[0] != NULL_BYTE )
			sprintf( tmp, " and type like '*,%d,*'", type );
		else
			sprintf( tmp, "type like '*,%d,*'", type );
		strncat( query, tmp, MAX_CRITELEM_LEN );
	}

	if ( attr > 0 )
	{	if ( query[0] != NULL_BYTE )
			sprintf( tmp, " and attr=%d", attr );
		else
			sprintf( tmp, "attr=%d", attr );
		strncat( query, tmp, MAX_CRITELEM_LEN );
	}

	if ( name != NULL )
	{	if ( query[0] != NULL_BYTE )
			sprintf( tmp, " and name='%s'", name );
		else
			sprintf( tmp, "name='%s'", name );
		strncat( query, tmp, MAX_CRITELEM_LEN );
	}

	if ( existance_query )
	{	if ( odm_get_first( nim_pdattr_CLASS, query, &lattr ) > 0 )
			return( 1 );
	}
	else
	{	if ( (*pdattr = odm_get_list( nim_pdattr_CLASS, 
												query, info, 1, 1 )) == -1 )
		{	errstr( ERR_ODM, (char *) odmerrno, NULL, NULL );
			return( -1 );
		}
		else if ( info->num > 0 )
			return( info->num );
	}

	/* no objects found */
	return( 0 );

} /* end of get_pdattr */

/*---------------------------- get_attr        ------------------------------
*
* NAME: get_attr
*
* FUNCTION:
*		returns the number of nim_attrs found for the specified attr
*		the desired attr may be specified by any combination of the "id",
*			"value", "seqno", and/or "pdattr" nim_attr fields
*		the list of attrs is also returned when the "attr" and "info" parameters
*			are non-NULL; when either is NULL, 0 or 1 will be returned
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*		when the list is obtained with odm_get_list, a level of 2 is used (this
*			retrieves the nim_attr & its corresponding nim_pdattr)
*		read WARNINGS below
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*			attr					= when non-NULL, ptr to nim_attr struct ptr
*			info					= when non-NULL, ptr to listinfo struct
*			id						= nim_attr.id
*			value					= nim_attr.value
*			seqno					= nim_attr.seqno
*			pdattr				= nim_attr.pdattr
*		global:
*
* RETURNS: (number of nim_attrs )
*		>0							= at least one nim_attr found
*		0							= no attr found
*		-1							= ODM error
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

/* WARNINGS:
 *		1) when value > NULL, make sure it's initialized correctly: this function
 *				will use the value in an ODM "like" expression
 */

int
get_attr (	struct nim_attr **attr, 
				struct listinfo *info, 
				long id,
				char *value, 
				int seqno, 
				int pdattr )

{	char query[MAX_CRITELEM_LEN];
	char tmp[MAX_CRITELEM_LEN];
	struct nim_attr lattr;
	int existance_query;

	VERBOSE5("         get_attr: id=%d; value=%s; seqno=%d; pdattr=%d;\n",
				id,value,seqno,pdattr);

	/* caller wants list? */
	existance_query = ( (attr == NULL) || (info == NULL) ) ? TRUE : FALSE;

	/* form the query */
	query[0] = NULL_BYTE;
	if ( id > 0 )
		sprintf( query, "id=%d", id );

	if ( value != NULL )
	{	if ( *query != NULL_BYTE )
			sprintf( tmp, " and value like '%s'", value );
		else
			sprintf( tmp, "value like '%s'", value );
		strncat( query, tmp, MAX_CRITELEM_LEN );
	}

	if ( seqno > 0 )
	{	if ( *query != NULL_BYTE )
			sprintf( tmp, " and seqno=%d", seqno );
		else
			sprintf( tmp, "seqno=%d", seqno );
		strncat( query, tmp, MAX_CRITELEM_LEN );
	}

	if ( pdattr > 0 )
	{	if ( *query != NULL_BYTE )
			sprintf( tmp, " and pdattr=%d", pdattr );
		else
			sprintf( tmp, "pdattr=%d", pdattr );
		strncat( query, tmp, MAX_CRITELEM_LEN );
	}

	/* caller just querying for existance or wants the list? */
	if ( existance_query )
	{	if ( odm_get_first( nim_attr_CLASS, query, &lattr ) > 0 )
			return( 1 );
	}
	else
	{	/* return list */
		if ( (*attr = odm_get_list( nim_attr_CLASS, query, info, 1, 2 )) == -1 )
		{	errstr( ERR_ODM, (char *) odmerrno, NULL, NULL );
			return( -1 );
		}
		else if ( info->num > 0 )
			return( info->num );
	}

	/* no objects found */
	return( 0 );

} /* end of get_attr */

/*---------------------------- get_object           ----------------------------
*
* NAME: get_object
*
* FUNCTION:
*		returns the number of objects found for the specified query
*		objects may be specified by any combination of the "id", "name", "class",
*			and/or "type" nim_object fields
*		a list of nim_objects matching the query is returned when the "object"
*			and "info" parameters are specified; otherwise, only 0 or 1 will
*			be returned
*		the list of objects is also returned when the "object" and "info" 
*			parameters are non-NULL; when either is NULL, 0 or 1 will be returned
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*		when the list is obtained with odm_get_list, a level of 3 is used
*		this results in getting the nim_object, all of its nim_attrs and their
*			corresponding nim_pdattr
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*			object			= ptr to nim_object ptr
*			info				= ptr to listinfo struct
*			id					= nim_object.id
*			name				= nim_object.name
*			class				= nim_object.class
*			type				= nim_object.type
*		global:
*
* RETURNS: (number of objects found)
*		>0							= object found
*		0							= no objects found
*		-1							= ODM error encountered
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
get_object(	struct nim_object **obj, 
				struct listinfo *info, 
				long id, 
				char *name, 
				int class, 
				int type )

{	char query[MAX_CRITELEM_LEN];
	char tmp[MAX_CRITELEM_LEN];
	struct nim_object lobj;
	int existance_query;

	VERBOSE5("         get_object: id=%d; name=%s; class=%d; type=%d;\n",
				id,name,class,type)

	/* return list of objects or just whether object exists? */
	existance_query = ( (obj == NULL) || (info == NULL) ) ? TRUE : FALSE;

	/* form the query */
	query[0] = NULL_BYTE;
	if ( id > 0 )
		sprintf( query, "id=%d", id );

	if ( name != NULL )
	{	if ( *query != NULL_BYTE )
			sprintf( tmp, " and name='%s'", name );
		else
			sprintf( tmp, "name='%s'", name );
		strncat( query, tmp, MAX_CRITELEM_LEN );
	}

	if ( class > 0 )
	{	if ( *query != NULL_BYTE )
			sprintf( tmp, " and class=%d", class );
		else
			sprintf( tmp, "class=%d", class );
		strncat( query, tmp, MAX_CRITELEM_LEN );
	}

	if ( type > 0 )
	{	if ( *query != NULL_BYTE )
			sprintf( tmp, " and type=%d", type );
		else
			sprintf( tmp, "type=%d", type );
		strncat( query, tmp, MAX_CRITELEM_LEN );
	}

	/* caller just querying for existance or wants the list? */
	if ( existance_query )
	{	if ( odm_get_first( nim_object_CLASS, query, &lobj ) > 0 )
			return( 1 );
	}
	else
	{	/* return list */
		if ( (*obj = odm_get_list( nim_object_CLASS, query, info, 10, 3 )) == -1 )
		{	errstr( ERR_ODM, (char *) odmerrno, NULL, NULL );
			return( -1 );
		}
		else if ( info->num > 0 )
			return( info->num );
	}

	/* no objects found */
	return( 0 );

} /* end of get_object */
	
/*---------------------------- get_name          ------------------------------
*
* NAME: get_name
*
* FUNCTION:
*		returns the name of a nim_object corresponding to the specified id
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*		this routine caches the nim_object name by malloc'ing space for it
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*			id						= id of nim_object
*		global:
*
* RETURNS: (char *)
*		>0							= ptr to the nim_object name
*		NULL						= no nim_object found with specified id
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

char *
get_name(	long id )

{	char *ptr=NULL;
	struct nim_object obj;
	char query[MAX_CRITELEM_LEN];

	VERBOSE5("         get_name: id=%d;\n",id,NULL,NULL,NULL)

	sprintf( query, "id=%d", id );
	if ( odm_get_first( nim_object_CLASS, query, &obj ) > 0 )
	{	/* malloc space for the name */
		if ( (ptr = malloc( strlen(obj.name) + 1 )) > 0 )
			strcpy( ptr, obj.name );
	}

	return( ptr );

} /* end of get_name */
	
/*---------------------------- get_id            ------------------------------
*
* NAME: get_id
*
* FUNCTION:
*		returns the id of the specified object
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*			name					= name of nim_object
*		global:
*
* RETURNS: (long)
*		>0							= nim_object.id of <name>
*		0							= ODM error? or couldn't find <name>
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

long
get_id(	char *name )

{	char query[MAX_CRITELEM_LEN];
	struct nim_object obj;

	VERBOSE5("         get_id: name=%s;\n",name,NULL,NULL,NULL)

	sprintf( query, "name='%s'", name );
	if ( odm_get_first( nim_object_CLASS, query, &obj ) > 0 )
		return( obj.id );

	errstr( ERR_DNE, name, NULL, NULL );

	return( 0 );

} /* end of get_id */

/*---------------------------- get_class         ------------------------------
*
* NAME: get_class
*
* FUNCTION:
*		returns the class of the specified nim_object
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
*			id						= id of nim_object
*			name					= name of nim_object
*		global:
*
* RETURNS: (int)
*		>0							= nim_object class
*		NULL						= no nim_object found
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
get_class(	long id, char *name )

{	struct nim_object obj;
	char query[MAX_CRITELEM_LEN];

	VERBOSE5("         get_class: id=%d; name=%s;\n",id,name,NULL,NULL)

	/* id or name given? */
	if ( id > 0 )
		sprintf( query, "id=%d", id );
	else if ( name == NULL )
		return( 0 );
	else
		sprintf( query, "name='%s'", name );

	if ( odm_get_first( nim_object_CLASS, query, &obj ) > 0 )
		return( obj.class );

	return( 0 );

} /* end of get_class */

/*---------------------------- get_type         ------------------------------
*
* NAME: get_type
*
* FUNCTION:
*		returns the type of the specified nim_object
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
*			id						= id of nim_object
*			name					= name of nim_object
*		global:
*
* RETURNS: (int)
*		>0							= nim_object class
*		0							= no nim_object found
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
get_type(	long id, char *name )

{	struct nim_object obj;
	ODMQUERY

	VERBOSE5("         get_type: id=%d; name=%s;\n",id,name,NULL,NULL)

	/* id or name given? */
	if ( id > 0 )
		sprintf( query, "id=%d", id );
	else if ( name == NULL )
		return( 0 );
	else
		sprintf( query, "name='%s'", name );

	if ( odm_get_first( nim_object_CLASS, query, &obj ) > 0 )
		return( strtol( obj.type_Lvalue, NULL, 0 ) );

	return( 0 );

} /* end of get_type */
	
/*---------------------------- get_type_pdattr  ------------------------------
*
* NAME: get_type_pdattr
*
* FUNCTION:
*		returns the pdattr corresponding to the type of the specified nim_object
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
*			id						= id of nim_object
*			name					= name of nim_object
*		global:
*
* RETURNS: (struct nim_pdattr *)
*		>NULL						= ptr to nim_pdattr struct for object's type
*		NULL						= no nim_object found
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

struct nim_pdattr *
get_type_pdattr(	long id, 	
						char *name )

{	int type;
	ODMQUERY
	struct nim_pdattr *ptr;

	VERBOSE5("         get_type_pdattr: id=%d; name=%s;\n",id,name,NULL,NULL)

	/* get the object's type */
	if ( (type = get_type( id, name )) > 0 )
	{
		/* malloc space for nim_pdattr */
		ptr = (struct nim_pdattr *) nim_malloc( sizeof( struct nim_pdattr ) );

		/* initialize query to retrieve the type pdattr */
		sprintf( query, "attr=%d", type );
		if ( odm_get_first( nim_pdattr_CLASS, query, ptr ) > 0 )
			return( ptr );

		free( ptr );
	}

	/* error */
	return( NULL );

} /* end of get_type_pdattr */
	
/*---------------------------- get_state         ------------------------------
*
* NAME: get_state
*
* FUNCTION:
*		returns the value of the specified state attr for the specified object
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
*			id						= id of nim_object
*			name					= name of nim_object
*			state_attr			= integer rep of attr for desired state
*		global:
*
* RETURNS: (int)
*		>0							= current state
*		0							= couldn't get the object/state
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
get_state(	long id,
				char *name,
				int state_attr )

{	ODMQUERY
	struct nim_attr state;

	VERBOSE5("         get_state: id=%d; name=%s; state_attr=%d\n",id,name,
				state_attr,NULL)

	/* we must have the id - get it if it wasn't passed in */
	if (id <= 0)
		if ( (id = get_id( name )) <= 0 ) 
			return( FAILURE );

	/* get this object's state */
	sprintf( query, "id=%d and pdattr=%d", id, state_attr );
	if ( odm_get_first( nim_attr_CLASS, query, &state ) > 0 )
			return( (int) strtol( state.value, NULL, 0 ) );

	return( 0 );

} /* end of get_state */
	
/*---------------------------- pdattr_class        -----------------------------
*
* NAME: pdattr_class
*
* FUNCTION:
*		returns the pdattr.class of the specified <attr>
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*			attr					= attr to get class for
*		global:
*
* RETURNS: (int)
*		>0							= pdattr.class for <attr>
*		0							= <attr> not found
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
pdattr_class(	int attr )

{	ODMQUERY
	struct nim_pdattr pdattr;

	VERBOSE5("         pdattr_class: attr=%d;\n",attr,NULL,NULL,NULL)

	sprintf( query, "attr=%d", attr );
	if ( odm_get_first( nim_pdattr_CLASS, query, &pdattr ) > 0 )
		return( pdattr.class );

	return( 0 );

} /* end of pdattr_class */
	
/*---------------------------- pdattr_s2i        ------------------------------
*
* NAME: pdattr_s2i
*
* FUNCTION:
*		translates the name of a nim_pdattr into its corresponding integer
*			representation
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*			name					= name of nim_pdattr
*		global:
*
* RETURNS: (int)
*		>0							= nim_pdattr.attr value
*		0							= specified type doesn't exist
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
pdattr_s2i( char *name )

{	char query[MAX_CRITELEM_LEN];
	struct nim_pdattr pdattr;

	VERBOSE5("         pdattr_s2i: name=%s;\n",name,NULL,NULL,NULL)

	if (name != NULL )
	{	sprintf( query, "name='%s'", name );
		if ( odm_get_first( nim_pdattr_CLASS, query, &pdattr ) > 0 )
			return( pdattr.attr );
	}

	return( 0 );

} /* end of pdattr_s2i */

/*---------------------------- class_s2i         ------------------------------
*
* NAME: class_s2i
*
* FUNCTION:
*		translates a class name into its corresponding integer representation
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*			class					= name of nim_object class
*		global:
*
* RETURNS: (int)
*		>0							= class number
*		0							= class not found
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
class_s2i( char *class )

{	char query[MAX_CRITELEM_LEN];
	struct nim_pdattr pdattr;

	VERBOSE5("         class_s2i: class=%s;\n",class,NULL,NULL,NULL)

	if (class != NULL )
	{	sprintf( query, "name='%s'", class );
		if ( odm_get_first( nim_pdattr_CLASS, query, &pdattr ) > 0 )
			return( pdattr.attr );
	}

	return( 0 );

} /* end of class_s2i */

/*---------------------------- subclass_s2i      ------------------------------
*
* NAME: subclass_s2i
*
* FUNCTION:
*		translates a subclass name into its corresponding integer representation
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*			subclass				= name of nim_object subclass
*		global:
*
* RETURNS: (int)
*		>0							= class number
*		0							= class not found
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
subclass_s2i( char *subclass )

{	char query[MAX_CRITELEM_LEN];
	struct nim_pdattr pdattr;

	VERBOSE5("         subclass_s2i: subclass=%s;\n",subclass,NULL,NULL,NULL)

	if (subclass != NULL )
	{	sprintf( query, "subclass like '*,%d,*' and name='%s'", 
					ATTR_SUBCLASS, subclass );
		if ( odm_get_first( nim_pdattr_CLASS, query, &pdattr ) > 0 )
			return( pdattr.attr );
	}

	return( 0 );

} /* end of subclass_s2i */

/*---------------------------- type_s2i          ------------------------------
*
* NAME: type_s2i
*
* FUNCTION:
*		translates the name of a nim_object type into its corresponding integer
*			representation
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*			type					= name of nim_object type
*		global:
*
* RETURNS: (int)
*		>0							= nim_object type
*		0							= specified type doesn't exist
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
type_s2i( char *type )

{	char query[MAX_CRITELEM_LEN];
	struct nim_pdattr pdattr;

	VERBOSE5("         type_s2i: type=%s;\n",type,NULL,NULL,NULL)

	if (type != NULL )
	{	sprintf( query, "subclass like '*,%d,*' and name='%s'", 
					ATTR_SUBCLASS_TYPE, type );
		if ( odm_get_first( nim_pdattr_CLASS, query, &pdattr ) > 0 )
			return( pdattr.attr );
	}

	return( 0 );

} /* end of type_s2i */
	
/*---------------------------- attr_in_list      ------------------------------
*
* NAME: attr_in_list
*
* FUNCTION:
*		checks to see if the specified attr is a member of the specified list
*		this function assumes that the list is a comma separated list of 
*			numbers and it should always be either the pdattr.subclass or
*			pdattr.type field
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*			attr					= attr to look for
*			list					= list to search in
*		global:
*
* RETURNS: (int)
*		TRUE						= attr is in the list
*		FALSE						= attr not in the list
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
attr_in_list(	int attr,
					char *list )

{	char query[MAX_TMP];

	VERBOSE5("         attr_in_list: attr=%d;\n",attr,NULL,NULL,NULL)

	sprintf( query, ",%d,", attr );
	if ( strstr( list, query ) != NULL )
		return( TRUE );

	return( FALSE );

} /* end of attr_in_list */
	
/*---------------------------- attr_in_subclass  ------------------------------
*
* NAME: attr_in_subclass
*
* FUNCTION:
*		returns TRUE if the specified <attr> belongs to the specified <subclass>
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*			attr					= attr to examine
*			subclass				= subclass to query on
*		global:
*
* RETURNS: (int)
*		TRUE						= <attr> is a member of the <subclass> subclass
*		FALSE						= <attr> isn't a member
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
attr_in_subclass(	int attr,
						int subclass )

{	ODMQUERY
	struct nim_pdattr pdattr;

	VERBOSE5("         attr_in_subclass: attr=%d; subclass=%d;\n",attr,subclass,
				NULL,NULL)

	/* get the predefined attr */
	sprintf( query, "attr=%d", attr );
	if ( odm_get_first( nim_pdattr_CLASS, query, &pdattr ) > 0 )
	{
		/* is <attr> in <subclass>? */
		return( attr_in_list( subclass, pdattr.subclass ) );
	}

	return( FALSE );

} /* end of attr_in_subclass */
	
/*---------------------------- nimattr           ------------------------------
*
* NAME: nimattr
*
* FUNCTION:
*		returns the specified nim_attr for the specified object
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
*			id						= id of nim_object
*			name					= name of nim_object
*			attr					= nim_attr.pdattr->attr to get info for
*		global:
*
* RETURNS: (struct nim_attr *)
*		>NULL						= ptr to nim_attr struct for <attr>
*		NULL						= <attr> not found or error encountered
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

struct nim_attr *
nimattr(	long id,
			char *name,
			int attr )

{	ODMQUERY
	struct nim_attr *ptr;

	VERBOSE5("         nimattr: id=%d; name=%s; attr=%d;\n",id,name,attr,NULL)

	/* get id if not given */
	if ( (id > 0) || ((id = get_id( name )) > 0) )
	{
		/* malloc space for nim_attr struct */
		ptr = (struct nim_attr *) nim_malloc( sizeof( struct nim_attr ) );

		/* form the query which will retrieve the desired attr */
		sprintf( query, "id=%d and pdattr=%d", id, attr );

		/* get the nim_attr */
		if ( odm_get_first( nim_attr_CLASS, query, ptr ) > 0 )
			return( ptr );

		free( ptr );
	}

	return( NULL );

} /* end of nimattr */

/*---------------------------- mk_attr            ------------------------------
*
* NAME: mk_attr
*
* FUNCTION:
*		add the specified nim_attr; the owner, which is a nim_object, may be
*			specified by either its "id" or its "name"
*		note that some attributes must have a sequence number; in these cases,
*			if a seqno is not supplied to mk_attr, mk_attr will generate one
*			automatically
*		note that some attributes must be unique; for these types, mk_attr will
*			verify that no attr of that type currently exists before adding it
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*		calls nim_error
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*			id						= nim_object id
*			name					= nim_object name
*			value					= value of nim_attr
*			seqno					= the seqno of the nim_attr
*			attr					= pdattr.attr
*			attr_name			= name of pdattr
*		global:
*
* RETURNS: (int)
*		SUCCESS					= specified nim_attr was added
*		FAILURE					= unable to add the specified nim_attr
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
mk_attr( long id, 
			char *name, 
			char *value, 
			int seqno, 
			int attr,
			char *attr_name )

{	char query[MAX_CRITELEM_LEN];
	NIM_PDATTR( pdattr, pdinfo )
	struct nim_attr new_attr;
	int i;
	NIM_ATTR( curattr, ainfo )
	char tmp[MAX_TMP];
	char tmp1[MAX_TMP];

	/* <attr> or <attr_name> must be supplied */
	if ( (attr <= 0) && ((attr_name == NULL) || (attr_name[0] == NULL_BYTE)) )
		ERROR( ERR_ATTR_NAME, attr_name, NULL, NULL )

	/* get the pdattr info */
	if ( get_pdattr( &pdattr, &pdinfo, 0, 0, 0, attr, attr_name ) <= 0 )
		ERROR( ERR_ATTR_NAME, attr_name, NULL, NULL )

	/* attr in the flags class? */
	if ( pdattr->class == ATTR_CLASS_FLAGS )
		/* this attr is just used to pass flags to NIM methods and does not
		/*		belong with any object's definition */
		/* don't add this attr - just return */
		return( SUCCESS );

	/* <value> must be supplied */
	if ( (value == NULL) || (value[0] == NULL_BYTE) )
		ERROR( ERR_MISSING_VALUE, pdattr->name, NULL, NULL )

	/* determine nim_object id */
	if ( id > 0 )
		new_attr.id = id;
	else if ( (new_attr.id = get_id( name )) <= 0 )
		return( FAILURE );
	if (	((name == NULL) || (*name == NULL_BYTE)) &&
			((name = get_name( new_attr.id )) == NULL) )
		return( FAILURE );

	VERBOSE5("         mk_attr: name=%s; value=%s; seqno=%d; attr=%d;\n",
				name,value,seqno,attr)

	/* need to do more verification? */
	if (	(pdattr->mask & PDATTR_MASK_ONLY_ONE) ||
			(pdattr->mask & PDATTR_MASK_SEQNO_REQ) )
	{
		/* get all attrs of this type which belong to <id> */
		if ( get_attr( &curattr, &ainfo, new_attr.id, NULL, 0, pdattr->attr )<0)
			return( FAILURE );

		/* check for errors */
		if ( (pdattr->mask & PDATTR_MASK_ONLY_ONE) && (ainfo.num > 0) )
				ERROR( ERR_ATTR_MK, pdattr->name, name, NULL )
		if ( pdattr->mask & PDATTR_MASK_SEQNO_REQ )
		{
			if ( ainfo.num > 0 )
			{
				/* check for collision errors */
				for (i=0; i < ainfo.num; i++)
				{
					if (	(pdattr->mask & PDATTR_MASK_ONLY_ONE) &&
							(strcmp( value, curattr[i].value ) == 0) )
					{
						/* collision on attribute values */
						sprintf( tmp1, "%s%d", pdattr->name, curattr[i].seqno );
						if ( seqno > 0 )
						{
							sprintf( tmp, "%s%d", pdattr->name, seqno );
							ERROR( ERR_ATTR_MK_VALUE, tmp, tmp1, NULL )
						}
						else
							ERROR( ERR_ATTR_MK_VALUE, pdattr->name, tmp1, NULL )
					}
					else if ( seqno == curattr[i].seqno )
						/* collision on seqno */
						ERROR( ERR_ATTR_SAME_SEQNO, (char *) seqno, pdattr->name,NULL)
				}

				if ( seqno == 0 )
				{
					/* generate a new sequence number */
					seqno = 1;
					while ( seqno < MAX_SEQNO )
					{
						for (i=0; i < ainfo.num; i++)
							if ( curattr[i].seqno == seqno )
								break;
						if ( i == ainfo.num )
							break;
						seqno++;
					}
	
					if ( seqno >= MAX_SEQNO )
						ERROR( ERR_GEN_SEQNO, pdattr->name, NULL, NULL )
				}
			}
			else if ( seqno <= 0 )
				seqno = 1;

		}	/* if ( pdattr->mask & PDATTR_MASK_SEQNO_REQ ) */

		/* free the attr list */
		odm_free_list( curattr, &ainfo );

	}

	/* add a new nim_attr */
	new_attr.value = value;
	new_attr.seqno = seqno;
	sprintf( new_attr.pdattr_Lvalue, "%d", pdattr->attr );
	if ( odm_add_obj( nim_attr_CLASS, &new_attr ) == -1 )
		ERROR( ERR_ODM, (char *) odmerrno, NULL, NULL )

	return( SUCCESS );

} /* end of mk_attr */

/*---------------------------- rm_attr            ------------------------------
*
* NAME: rm_attr
*
* FUNCTION:
*		removes the specified nim_attr; the owner of the attribute can be
*			specified with either its "id" or its "name"
*		the <attr> parameter is required; all others are optional
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*		calls errstr on failure
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*			id						= nim_object id
*			name					= nim_object name
*			attr					= nim_attr.pdattr->attr
*			seqno					= nim_attr.seqno
*			value					= nim_attr.value
*		global:
*
* RETURNS: (int)
*		SUCCESS					= specified nim_attr removed
*		FAILURE					= unable to remove the specified nim_attr
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
rm_attr( long id, 
			char *name, 
			int attr, 
			int seqno,
			char *value )

{	char query[MAX_CRITELEM_LEN];
	char tmp[MAX_TMP];
	int num;

	/* was id or name given? */
	if (id <= 0)
	{	if ( name != NULL )
			if ( (id = get_id( name )) <= 0 ) 
				return( FAILURE );
	}

	VERBOSE5("         rm_attr: id=%d; attr=%d; seqno=%d; value=%s;\n",
				id,attr,seqno,value)

/*???? danger! danger! danger! will robinson - by not supplying an id */
/*????		could delete ALL attrs of a particular type */
/*???? we want to do this in some cases (look at m_mknet), but... */

	/* form the query */
	query[0] = NULL_BYTE;
	sprintf( query, "pdattr=%d", attr );

	if ( id > 0 )
	{	sprintf( tmp, " and id=%d", id );
		strncat( query, tmp, MAX_CRITELEM_LEN );
	}

	if ( seqno > 0 )
	{	sprintf( tmp, " and seqno=%d", seqno );
		strncat( query, tmp, MAX_CRITELEM_LEN );
	}

	if ( value != NULL )
	{	sprintf( tmp, " and value like '%s'", value );
		strncat( query, tmp, MAX_CRITELEM_LEN );
	}

	/* remove the specified nim_attr */
	if ( (num = odm_rm_obj( nim_attr_CLASS, query )) == -1 )
		ERROR( ERR_ODM, (char *) odmerrno, NULL, NULL )
	else if ( num == 0 )
		ERROR( ERR_ATTR_NOT_FOUND, query, NULL, NULL ) 

	return( SUCCESS );

} /* end of rm_attr */

/*---------------------------- ch_attr           ------------------------------
*
* NAME: ch_attr
*
* FUNCTION:
*		changes the value of the specified attr if it already exists; otherwise,
*			it just adds it
*		owner of attr may be specified by either its <id> or <name>
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*			id						= nim_object id
*			name					= nim_object name
*			value					= new value for the specified nim_attr
*			seqno					= the seqno of the nim_attr
*			attr					= pdattr.attr
*			attr_name			= name of pdattr
*		global:
*
* RETURNS: (int)
*		SUCCESS					= specified nim_attr was added
*		FAILURE					= unable to add the specified nim_attr
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
ch_attr( long id, 
			char *name, 
			char *value, 
			int seqno, 
			int attr,
			char *attr_name )

{	NIM_ATTR( curval, curinfo )
	NIM_PDATTR( pdattr, pdinfo )
	int i;
	int null_value = ( (value == NULL) || (*value == NULL_BYTE) );
	int remove_attr = FALSE;
	int seqno_required = 0;
	int attr_removed = FALSE;
	char tmp[MAX_TMP];

	/* must have the id */
	if ( (id <= 0) && ((id = get_id( name )) <= 0) )
			return( FAILURE );
	if (	((name == NULL) || (*name == NULL_BYTE)) &&
			((name = get_name( id )) == NULL) )
		return( FAILURE );

	VERBOSE5("         ch_attr: id=%d; value=%s; seqno=%d; attr=%d;\n",
				id,value,seqno,attr)

	/* is this a change to an existing attr or an addition? */
	/* to find out, we need 2 pieces of info: */
	/*		1) nim_pdattr for this attr */
	/*		2) nim_attrs of this type */

	/* get the predefined info */
	if ( get_pdattr( &pdattr, &pdinfo, 0, 0, 0, attr, NULL ) <= 0 )
		return( FAILURE );
	seqno_required = (pdattr->mask & PDATTR_MASK_SEQNO_REQ);

	/* get the customized info */
	if ( get_attr( &curval, &curinfo, id, NULL, seqno, attr ) < 0 )
		return( FAILURE );
				
	/* check for errors */
	if ( seqno > 0 )
		sprintf( tmp, "%s%d", attr_name, seqno );
	if ( (null_value) && (curinfo.num == 0) )
		ERROR( ERR_ATTR_RM_DNE, tmp, name, NULL )
	else if ( (null_value) && (seqno_required) && (seqno <= 0) )
		ERROR( ERR_MISSING_SEQNO, attr_name, NULL, NULL )

	/* remove current value? */
	if ( curinfo.num > 0 )
	{
		if ( seqno_required )
			remove_attr = ( (null_value) || (seqno > 0) );
		else
			remove_attr = TRUE;
	}
	else
		remove_attr = FALSE;

	if ( remove_attr )
	{	
		/* delete the current version */
		if ( rm_attr( id, name, attr, seqno, NULL ) == FAILURE )
			return( FAILURE );
		attr_removed = TRUE;
	}

	/* add new attr? */
	if (	(null_value) ||
			(mk_attr( id, name, value, seqno, attr, attr_name ) == SUCCESS ) )
		return( SUCCESS );

	/* couldn't add new attr - put original attr back */
	if ( attr_removed )
	{
		protect_errstr = TRUE;
		for (i=0; i < curinfo.num; i++)
			mk_attr( curval[i].id, NULL, curval[i].value, curval[i].seqno, 
						curval[i].pdattr->attr, curval[i].pdattr->name );
	}

	return( FAILURE );

} /* end of ch_attr */
	
/*---------------------------- attr_count        ------------------------------
*
* NAME: attr_count
*
* FUNCTION:
*		adds the specified <value> to the specified <attr>
*		owner of <attr> may be specified by either its <id> or <name>
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*		sets errstr on failure
*		this function is used to incre/decre allocation counts
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*			id						= nim_object id
*			name					= nim_object name
*			attr					= pdattr.attr
*			seqno					= pdattr.seqno when applicable
*			delta					= value to be added
*		global:
*
* RETURNS: (int)
*		SUCCESS					= specified nim_attr was added
*		FAILURE					= unable to add the specified nim_attr
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
attr_count(	long id,
				char *name,
				int attr,
				int seqno,
				int delta )

{	NIM_ATTR( curval, curinfo )
	int value=0;
	char tmp[MAX_TMP];

	/* must have the id */
	if ( id <= 0 )
	{	if ( (id = get_id( name )) <= 0 )
			return( FAILURE );
	}

	VERBOSE5("         attr_count: id=%d; attr=%d; seqno=%d; delta=%d;\n",
				id,attr,seqno,delta)

	/* get the current value */
	if ( get_attr( &curval, &curinfo, id, NULL, seqno, attr ) < 0 )
		return( FAILURE );
	if ( curinfo.num > 0 )
		value = (int) strtol( curval->value, NULL, 0 );

	/* add delta */
	value = value + delta;
	if ( value < 0 )
		value = 0;

	/* replace attr with new value */
	sprintf( tmp, "%d", value );

	return( ch_attr( id, NULL, tmp, seqno, attr, ATTR_msg(attr) ));

} /* end of attr_count */

/*---------------------------- find_attr         ------------------------------
*
* NAME: find_attr
*
* FUNCTION:
*		searches a nim_object structure for the specified attribute; if found,
*			it returns the nim_object.attrs index
*		this function may be called repeadly to do a sequence search through the
*			attribute list; this is done by passing the address (via <index>) of
*			a variable which find_attr will use as the index - when this is done,
*			find_attr will start with the current value of <index> instead of
*			starting with zero
*		attributes may be searched for by any combination of the "value",
*			"seqno", and/or "pdattr" nim_attr fields
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*		read WARNING below
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*			nim_object			= ptr to nim_object
*			index					= optional ptr to index variable
*			pattern				= pattern to match with the nim_attr.value field
*			seqno					= nim_attr.seqno
*			pdattr				= nim_attr.pdattr
*		global:
*
* RETURNS: (int)
*		>=0						= attribute found; index of attr returned
*		-1							= unable to find the attr
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

/* WARNINGS:
 *		1) when using the repeatitive search (ie, index > NULL), make sure
 *				index = -1 the first time
 *		2) when pattern is specified, it will be used as an extendend regular
 *				expression, so it must be formed properly
 */

int
find_attr(	struct nim_object *obj, 
				int *index, 
				char *pattern, 
				int seqno,
				int pdattr )

{	int i=0;
	int *ptr;
	int matches_required;
	int matches;
	regex_t preg;

	VERBOSE5("         find_attr: obj=%s; pattern=%s; seqno=%d; pdattr=%d;\n",
				obj->name,pattern,seqno,pdattr)

	/* initialize index ptr */
	if ( index == NULL )
		/* don't care about subsequent searches - use local index */
		ptr = &i;
	else
	{	/* use the supplied index */
		ptr = index;
		if ( *ptr < 0 )
			/* assuming that this is the first in a series of calls to find_attr */
			*ptr = 0;
		else
			/* assuming that index was set by previous call to find_attr & that */
			/*		it currently points to the location of the previous find */
			(*ptr)++;
	}

	/* initialize the match variable */
	matches_required=0;
	if ( pattern != NULL )
	{	matches_required++;

		/* compile the extended regular expression */
		if ( regcomp( &preg, pattern, REG_EXTENDED ) != 0 )
			return( -1 );
	}
	if ( seqno > 0 )
		matches_required++;
	if ( pdattr > 0 )
		matches_required++;

	/* search for the specified attribute */
	for (; *ptr < obj[0].attrs_info->num; (*ptr)++ )
	{	matches = 0;
		if (	(pattern != NULL) && 
				(regexec( &preg, obj[0].attrs[*ptr].value, 0, NULL, 0 ) == 0) )
				matches++;
		if ( (seqno > 0) && (obj[0].attrs[*ptr].seqno == seqno) )
				matches++;
		if ( (pdattr > 0) && (obj[0].attrs[*ptr].pdattr->attr == pdattr) )
				matches++;

		if ( matches == matches_required )
			return( *ptr );
	}

	/* no attrs found if we get here */
	*ptr = -1;

	return( -1 );

} /* end of find_attr */

/*---------------------------- valid_name        ------------------------------
*
* NAME: valid_name
*
* FUNCTION:
*		verifies that the specified string is an acceptable name for a NIM object
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
*			name					= string to verify
*		global:
*
* RETURNS: (int)
*		TRUE						= name ok
*		FALSE						= invalid name
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
valid_name(	char *name )

{	int i;
	int	rc;
	regex_t ere;
	char	tmp[MAX_TMP];

	VERBOSE5("         valid_name: name=%s;\n",NULL,NULL,NULL,NULL)

	/* any invalid chars in name? */
	if ( (rc = regexec( nimere[BAD_NAME_ERE].reg, name, 0, NULL, 0 )) == 0 )
		/* pattern matched - invalid name! */
		ERROR( ERR_BAD_CHARS, name, NULL, NULL )

	if ( rc != REG_NOMATCH )
	{	/* regexec error */
		regerror( rc, nimere[BAD_NAME_ERE].reg, tmp, MAX_TMP );
		nene( ERR_SYS, "regexec", tmp, NULL );
	}

	if ( strlen(name) > MAX_NAME_CHARS )
		ERROR( ERR_MAX_VALUE, name, (char *) MAX_NAME_CHARS, NULL )

	/* check reserved words */
	if ( ! force )
	{
		for (i=0; reserved_words[i] != NULL; i++)
			if ( ! strcmp( reserved_words[i], name ) )
				ERROR( ERR_RESERVED_WORD, name, NULL, NULL )
	}

	/* return */
	return( TRUE );

} /* end of valid_name */

/*---------------------------- mk_object             ---------------------------
*
* NAME: mk_object
*
* FUNCTION:
*		creates a new nim_object
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*			name					= name of new nim_object
*			type					= name of nim_object type
*			id						= (optional) id to use for the object
*		global:
*
* RETURNS: (long)
*		>0							= id of new object
*		0							= failure
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

long
mk_object(	char *name, 
				char *type,
				long id )

{	struct nim_pdattr *pdtype;
	struct listinfo pdtinfo;
	struct nim_pdattr *pdclass;
	struct listinfo pdcinfo;
	char query[MAX_CRITELEM_LEN];
	struct listinfo info;
	struct nim_object obj;
	struct nim_object *exst;
	int retries;

	VERBOSE5("         mk_object: name=%s; type=%s; id=%d\n",name,type,id,NULL)

	/* verify object type */
	if ( get_pdattr( &pdtype, &pdtinfo, 0, ATTR_SUBCLASS_TYPE, 0, 0, type ) <= 0)
		ERROR( ERR_OBJ_TYPE, type, NULL, NULL )

	/* check name */
	if (! valid_name( name ) )
		return( FAILURE );

	/* verify that name is unique */
	sprintf( query, "name='%s'", name );
	if ( odm_get_list( nim_object_CLASS, query, &info, 1, 1 ) != NULL )
		ERROR( ERR_ONLY_ONE, MSG_msg(MSG_OBJ_NAME), name, NULL );

	/* if id is supplied... */
	if ( id > 0 )
	{
		/* use the one which is supplied and verify that id is unique */
		sprintf( query, "id=%d", id );
		if ( odm_get_first( nim_object_CLASS, query, &obj ) != NULL )
			ERROR( ERR_CANT_ID, NULL, NULL, NULL )
	}
	else
	{
		/* lock database & generate a new, unique id */
		if (! get_glock(1,1) )
			ERROR( ERR_CANT_GLOCK, NULL, NULL, NULL )
		retries =  5;
		while ( retries-- > 0 )
		{	/* id = current system time */
			id = time( NULL );

			/* make sure id is unique */
			sprintf( query, "id=%d", id );
			if ((exst=odm_get_list( nim_object_CLASS, query, &info,1,1)) == -1 )
				ERROR( ERR_ODM, (char *) odmerrno, NULL, NULL )
			else if ( exst == NULL )
				break;
		 	sleep( 1 ); 
		}
		rm_glock( FALSE );
		if ( retries == 0 )
			ERROR( ERR_CANT_ID, NULL, NULL, NULL );
	}

	/* ready to add a new object */
	obj.name = name;
	obj.class = pdtype->class;
	obj.id = id;
	sprintf( obj.type_Lvalue, "%d", pdtype->attr );
	sprintf( obj.attrs_Lvalue, "%d", id );
	if ( odm_add_obj( nim_object_CLASS, &obj ) == -1 )
		ERROR( ERR_ODM, (char *) odmerrno, NULL, NULL );

	/* lock the new object */
	lock_object( obj.id, NULL );

	return( obj.id );

} /* end of mk_object */

/*---------------------------- rm_object             ---------------------------
*
* NAME: rm_object
*
* FUNCTION:
*		removes the specified nim_object
*		the nim_object may be specified by either its <id> or <name>
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*		ASSUMES that the specified object has already been locked
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*			id						= id of the nim_object
*			name					= name of nim_object
*		global:
*
* RETURNS: (int)
*		SUCCESS					= object removed
*		FAILURE					= unable to remove specified nim_object
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
rm_object(	long id, 
				char *name )

{	int rc=SUCCESS;
	ODMQUERY

	if ( (id <= 0) && ((id = get_id( name )) == FAILURE) )
		return( FAILURE );

	VERBOSE5("         rm_object: id=%d; name=%s\n",id,name,NULL,NULL)

	/* initialize the query */
	sprintf( query, "id=%d", id );

	/* disable interrupts - can't be interrupted here or we'd be left with */
	/*		invalid objects */
	disable_err_sig();

	/* remove the nim_object and it's corresponding nim_attrs */
	/* note that we're removing the attrs first so that, if an error occurs */
	/*		there, the nim_object will remain */
	if (	(odm_rm_obj( nim_attr_CLASS, query ) < 0 ) ||
			(odm_rm_obj( nim_object_CLASS, query ) < 0) )
	{	rc = FAILURE;
		errstr( ERR_ODM, (char *) odmerrno, NULL, NULL );
	}

	enable_err_sig();

	return( rc );

} /* end of rm_object */
	
/*---------------------------- order_attrs       ------------------------------
*
* NAME: order_attrs
*
* FUNCTION:
*		reorders the specified nim_attr list so that they appear in ascending
*			seqno order
*		NOTE that a seqno of 0 means that ordering is not important and thus
*			these attrs are put at the end of the list
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*		ASSUMES that the <attrs> were obtained using odm_get_list
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*			attrs					= ptr to list of nim_attr structs
*			info					= listinfo returned by odm_get_list
*		global:
*
* RETURNS: (int)
*		SUCCESS					= list ordered on seqno
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
order_attrs(	struct nim_attr *attrs,
					struct listinfo *info )

{	int i,j;
	struct nim_attr tmp;

	VERBOSE5("         order_attrs\n",NULL,NULL,NULL,NULL)

	for (i=0; i < (info->num - 1); i++)
		for (j=i+1; j < info->num; j++)
			if (	(attrs[j].seqno > 0) && 
					((attrs[i].seqno == 0) || (attrs[j].seqno < attrs[i].seqno)) )
			{
				/* swap entries */
				memcpy( &tmp, (attrs+i), sizeof(struct nim_attr) );
				memcpy( (attrs+i), (attrs+j), sizeof(struct nim_attr) );
				memcpy( (attrs+j), &tmp, sizeof(struct nim_attr) );
			}

	return( SUCCESS );

} /* end of order_attrs */

/*$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
$$$$$$$$$$$$$$$$ object backup & recovery                          $$$$$$$$$$$$
$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$*/
	
/*---------------------------- rest_object       ------------------------------
*
* NAME: rest_object
*
* FUNCTION:
*		restores info about the specified object by replacing any current info
*			with the specified info
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
*		global:
*
* RETURNS: (int)
*		SUCCESS					= info restored
*		FAILURE					= ODM error?
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
rest_object(	struct nim_object *obj,
					struct listinfo *info )

{	int i;

	VERBOSE5("         rest_object: obj=%s\n",obj->name,NULL,NULL,NULL)

	/* first, make sure info is somewhat valid */
	if ( (obj->id <= 0) || (info->num <= 0) )
		return( FAILURE );

	/* remove all current info about <obj> */
	rm_object( obj->id, NULL );

	/* add in the old info */
	if ( odm_add_obj( nim_object_CLASS, obj ) == -1 )
		ERROR( ERR_ODM, (char *) odmerrno, NULL, NULL );
	for (i=0; i < obj->attrs_info->num; i++)
		if ( odm_add_obj( nim_attr_CLASS, (obj->attrs + i) ) == -1 )
			ERROR( ERR_ODM, (char *) odmerrno, NULL, NULL )

	return( SUCCESS );

} /* end of rest_object */
	
/*---------------------------- add_backup_LIST   ------------------------------
*
* NAME: add_backup_LIST
*
* FUNCTION:
*		adds the specified obj to the list of backups to be restored upon failure
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
*			backups				= ptr to LIST for storage of OBJ_BACKUP nodes
*			new_node				= ptr to OBJ_BACKUP struct
*		global:
*
* RETURNS: (int)
*		SUCCESS					= obj added to backup LIST
*		FAILURE					= unable to add to LIST
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
add_backup_LIST(	LIST *backups,
						OBJ_BACKUP *new_node )

{
	VERBOSE5("         add_backup_LIST\n",NULL,NULL,NULL,NULL)

   /* reserve space */
   if ( get_list_space( backups, 1, FALSE ) == FAILURE )
      return( FAILURE );

	/* initialize the new LIST entry */
	backups->list[ backups->num ] = (char *) new_node;

   /* incre the count */
   backups->num = backups->num + 1;

   return( SUCCESS );

} /* end of add_backup_LIST */

/*---------------------------- backup_object      ------------------------------
*
* NAME: backup_object
*
* FUNCTION:
*		locks, gets, and backs up the specified object
*		the object may be specified by either its <id> or <name>
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*		sets errstr on failure
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*     parameters:
*			backups			= ptr to LIST for storage of OBJ_BACKUP nodes
*        id             = id of nim_object
*        name           = name of the object to get
*        obj            = ptr to nim_object struct ptr if caller wants obj back
*        info           = ptr to listinfo struct if caller wants obj back
*     global:
*
* RETURNS: (int)
*     SUCCESS           = object locked, retrieved, and backed up
*     FAILURE           = couldn't lock -or- couldn't get (errstr set)
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
backup_object(	LIST *backups,
					long id,
            	char *name,
            	struct nim_object **obj,
            	struct listinfo **info )

{	int i;
	OBJ_BACKUP *tmp;

	/* get id if not specified */
	if ( id <= 0 )
	{	if ( (id = get_id(name)) <= 0 )
			return( FAILURE );
	}

	VERBOSE5("         backup_object: id=%d; name=%s;\n",id,name,NULL,NULL)

	/* is the object already backed up? */
	for (i=0; i < backups->num; i++)
	{	if (	((tmp = (OBJ_BACKUP *) backups->list[i]) == NULL) ||
				(tmp->obj == NULL) )
			continue;

		if ( tmp->obj->id == id )
			break;
	}

	if ( i >= backups->num )
	{	/* not backed up yet - create a new OBJ_BACKUP node */

		/* malloc space for obj info */
		tmp = (OBJ_BACKUP *) nim_malloc( OBJ_BACKUP_SIZE );

   	/* lock-and-get the object */
   	if ( lag_object( id, name, &(tmp->obj), &(tmp->info) ) == FAILURE )
   		return( FAILURE );

		/* add to backup LIST */
		if ( add_backup_LIST( backups, tmp ) == FAILURE )
   		return( FAILURE );
	}

	/* return obj ptrs */
	if ( (obj != NULL) && (info != NULL) )
	{	*obj = (struct nim_object *) tmp->obj;
		*info = (struct listinfo *) &(tmp->info);
	}

	return( SUCCESS );

} /* end of backup_object */
	
/*---------------------------- clear_backups     ------------------------------
*
* NAME: clear_backups  
*
* FUNCTION:
*		clears the backups LIST by throwing away all cached objects
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
*			backups				= ptr to LIST of OBJ_BACKUP structs
*		global:
*			backups
*
* RETURNS: (int)
*		SUCCESS
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
clear_backups(	LIST *backups )

{	int i;
	OBJ_BACKUP *tmp;

	VERBOSE5("         clear_backups\n",NULL,NULL,NULL,NULL)

	/* no interruptions allowed */
	disable_err_sig();

	/* for each node in the LIST... */
	for (i=0; i < backups->num; i++)
	{	if ( (tmp = (OBJ_BACKUP *) backups->list[i]) == NULL )
			continue;

		/* free memory malloc'd by ODM */
		odm_free_list( tmp->obj, &(tmp->info) );

		/* free the OBJ_BACKUP node */
		free( tmp );
	}

	/* reset number of backups */
	backups->num = 0;

	/* restore errsigs */
	enable_err_sig();

	return( SUCCESS );

} /* end of clear_backups */

/*---------------------------- restore_backups   ------------------------------
*
* NAME: restore_backups
*
* FUNCTION:
*		restores the objects specified in the LIST of OBJ_BACKUP structs
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*		sets errstr on failure
*		this routine attempts a rest_object for each entry in <backups> - ie, it
*			will NOT return if one of the rest_object calls fails
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*			backups				= ptr to LIST of OBJ_BACKUP structs
*		global:
*
* RETURNS: (int)
*		SUCCESS					= all objects restored
*		FAILURE					= failure on one or more objects
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
restore_backups(	LIST *backups )

{	int i;
	OBJ_BACKUP *tmp;

	VERBOSE5("         restore_backups\n",NULL,NULL,NULL,NULL)

	/* no interruptions allowed */
	disable_err_sig();

	/* protect the errstr */
	protect_errstr = TRUE;

	/* for each node in the LIST... */
	for (i=0; i < backups->num; i++)
	{	if ( (tmp = (OBJ_BACKUP *) backups->list[i]) == NULL )
			continue;

		/* restore this object - NOTE that we're NOT going to return if */
		/*		a failure occurs */
		if ( rest_object( tmp->obj, &(tmp->info) ) == FAILURE )
			warning( 0, NULL, NULL, NULL );
	}

	/* clear the <backups> LIST */
	clear_backups( backups );

	/* restore errsigs */
	enable_err_sig();

	return( SUCCESS );

} /* end of restore_backups */
	
/*$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
$$$$$$$$$$$$$$$$ attribute processing                              $$$$$$$$$$$$
$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$*/
	
/*---------------------------- valid_pdattr_ass --------------------------------
*
* NAME: valid_pdattr_ass
*
* FUNCTION:
*		validates attribute assignments involving nim_pdattrs
*		creates a new entry in the ATTR_ASS_LIST when the assignments are valid
*		if this routine is called from a "change" method, then the <change_op>
*			flag should be used so that:
*				1) NULL <value>s are accepted
*				2)	seqno check is performed
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
*			list					= ptr to ATTR_ASS_LIST
*			name					= pdattr.name
*			value					= value for the pdattr
*			seqno					= seqno for the pdattr
*			change_op			= TRUE if NULL is ok for <value>
*		global:
*
* RETURNS: (int)
*		SUCCESS					= valid assignment; entry added to <list>
*		FAILURE					= invalid assignment
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
valid_pdattr_ass(	ATTR_ASS_LIST *list,
						char *name,
						char *value,
						int seqno,
						int change_op )

{	char query[MAX_CRITELEM_LEN];
	struct nim_pdattr pdattr;
	struct nim_pdattr *ros_support;
	struct listinfo rinfo;
	int i;
	char boot_if[MAX_TMP];
	char tmp[MAX_TMP];

	VERBOSE5("         valid_pdattr_ass:name=%s;value=%s; seqno=%s; change=%d\n",
				name,value,seqno,change_op)

	/* get the pdattr info for <name> */
	sprintf( query, "name='%s'", name );
	if ( odm_get_first( nim_pdattr_CLASS, query, &pdattr ) <= 0 )
		ERROR( ERR_ATTR_NAME, name, NULL, NULL )

	/* skip normal checking if attr in the flags class */
	if ( pdattr.class != ATTR_CLASS_FLAGS )
	{
		/* check the pdattr.mask */
		if (	(pdattr.mask == PDATTR_MASK_NOTHING) ||
				((pdattr.mask & PDATTR_MASK_INFO) == 0) )
			ERROR( ERR_NO_CH_ATTR, name, NULL, NULL )
		else if ( (seqno > 0) && ((pdattr.mask & PDATTR_MASK_SEQNO_REQ) == 0) )
			ERROR( ERR_NO_SEQNO, name, NULL, NULL )
		else if (	(change_op) && (seqno == 0) && 
						(pdattr.mask & PDATTR_MASK_SEQNO_REQ) )
			ERROR( ERR_MISSING_SEQNO, name, NULL, NULL )
	
		/* waited until here to look at value because this routine is called */
		/*		by ch_pdattr_ass, which wants the <attr> part of the assignement */
		/*		verified even though the <value> part may be NULL */
		/* this occurs because "change" methods will allow the resetting of an */
		/*		attr value using the following syntax: <attr>="" */
		if ( (value != NULL) && (value[0] != NULL_BYTE) )
		{	/* look for attrs which need specific verification */
			switch (pdattr.attr)
			{
				case ATTR_NET_ADDR:
					if ( verify_net_addr( value ) == FAILURE )
						return( FAILURE );
				break;

				case ATTR_SNM:
					if ( verify_snm( value ) == FAILURE )
						return( FAILURE );
				break;

				case ATTR_RING_SPEED:
					if ( verify_ring_speed( value ) == FAILURE )
						return( FAILURE );
				break;

				case ATTR_CABLE_TYPE:
					if ( verify_cable_type( value ) == FAILURE )
						return( FAILURE );
				break;
			}
		}
		else if ( ! change_op )
			ERROR( ERR_MISSING_VALUE, name, NULL, NULL );
	}

	return( add_attr_ass( list, pdattr.attr, name, value, seqno ) );

} /* end of valid_pdattr_ass */
	
/*---------------------------- ch_pdattr_ass    --------------------------------
*
* NAME: ch_pdattr_ass 
*
* FUNCTION:
*		validates attribute assignments involving nim_pdattrs
*		creates a new entry in the ATTR_ASS_LIST when the assignments are valid
*		unlike valid_pdattr_ass, this function will accept NULL <value>s
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
*			list					= ptr to ATTR_ASS_LIST
*			name					= pdattr.name
*			value					= value for the pdattr
*			seqno					= seqno for the pdattr
*			dummy					= this arg is ignored & only included so that param
*											list will match what's passed from
*											parse_attr_ass
*		global:
*
* RETURNS: (int)
*		SUCCESS					= valid assignment; entry added to <list>
*		FAILURE					= invalid assignment
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
ch_pdattr_ass(	ATTR_ASS_LIST *list,
					char *name,
					char *value,
					int seqno,
					int dummy )

{	
	/* this routine will accept NULL <value>s */
	return( valid_pdattr_ass( list, name, value, seqno, TRUE ) );

} /* end of ch_pdattr_ass */
	
/*---------------------------- attrs_from_FILE   ------------------------------
*
* NAME: attrs_from_FILE
*
* FUNCTION:
*		processes attribute assignments from the specified FILE and adds them
*			to the specified object's definition
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*		ignores errors
*		this function ASSUMES that <obj> was retrieved using lag_object
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*			id						= id of nim_object
*			fp						= FILE ptr
*		global:
*
* RETURNS: (int)
*		SUCCESS					= attrs added
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
attrs_from_FILE (	long id,
						FILE *fp )

{	char attr_ass[MAX_VALUE];
	int len;
	ATTR_ASS_LIST list;
	ATTR_ASS *ass;
	ODMQUERY
	struct nim_pdattr pdattr;

	VERBOSE5("         attrs_from_FILE: id=%d;\n",id,NULL,NULL,NULL)

	/* initialize the LIST */
	if ( get_list_space( &list, DEFAULT_CHUNK_SIZE, TRUE ) == FAILURE )
		return( FAILURE );

	/* for each line in FILE */
	while ( fgets( attr_ass, MAX_VALUE, fp ) != NULL )
	{
		/* remove the newline if it's at the end of the string */
		len = strlen( attr_ass ) - 1;
		if ( attr_ass[len] == '\012' )
			attr_ass[len] = NULL_BYTE;

		/* separate fields & always use same LIST entry */
		list.num = 0;
		if ( parse_attr_ass( &list, NULL, attr_ass, TRUE ) == FAILURE )
		{	warning( 0, NULL, NULL, NULL );
			continue;
		}

		/* get the pdattr info for this assignment */
		ass = list.list[ 0 ];
		sprintf( query, "name='%s'", ass->name );
		if ( odm_get_first( nim_pdattr_CLASS, query, &pdattr ) <= 0 )
		{	
			warning( ERR_ATTR_NAME, ass->name, NULL, NULL );
			continue;
		}

		/* add the attr to the object's definition */
		if ( mk_attr(	id, NULL, ass->value, ass->seqno, pdattr.attr,
							pdattr.name ) == FAILURE )
			warning( 0, NULL, NULL, NULL );
	}

	return( SUCCESS );

} /* end of attrs_from_FILE */

