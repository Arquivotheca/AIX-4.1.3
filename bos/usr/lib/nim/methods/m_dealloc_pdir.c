static char	sccs_id[] = " @(#)72 1.7  src/bos/usr/lib/nim/methods/m_dealloc_pdir.c, cmdnim, bos411, 9428A410j  3/31/94  15:36:37";

/*
 *   COMPONENT_NAME: CMDNIM
 *
 *   FUNCTIONS: ck_attrs
 *		dealloc_pdir
 *		main
 *		parse_args
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

#include "cmdnim_mstr.h"

extern int valid_pdattr_ass();

/*---------------------------- module globals    -----------------------------*/
ATTR_ASS_LIST attr_ass;

VALID_ATTR valid_attrs[] =
{
	{0,							NULL,							FALSE,	valid_pdattr_ass}
};

char *name=NULL;									/* object name */

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
				cmd_what( valid_attrs );
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

/*---------------------------- ck_attrs           ------------------------------
*
* NAME: ck_attrs
*
* FUNCTION:
*		checks command line attr assignments
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
	int res_type;
	ODMQUERY
	struct nim_pdattr pdattr;

	/* loop through supplied attrs */
	for (i=0; i < attr_ass.num; i++)
	{	
		/* get the nim_pdattr for this attr */
		sprintf( query, "attr=%d", attr_ass.list[i]->pdattr );
		if ( odm_get_first( nim_pdattr_CLASS, query, &pdattr ) <= 0 )
			nim_error( ERR_ATTR_NAME, attr_ass.list[i]->name );

		/* skip attrs which are just flags */
		if ( pdattr.class == ATTR_CLASS_FLAGS )
			continue; 

		/* get the object's type */
		if ( (res_type = get_type( 0, attr_ass.list[i]->value )) == 0 )
			nim_error(	ERR_BAD_OBJECT, 
							ATTR_msg( ATTR_CLASS_RESOURCES ), 
							attr_ass.list[i]->value, NULL );

		/* compare the expected type with this resource's type */
		if ( attr_ass.list[i]->pdattr != res_type )
			nim_error(	ERR_TYPE_CONFLICT, attr_ass.list[i]->name, 
							ATTR_msg(res_type), attr_ass.list[i]->value );

		/* make sure type is in ATTR_SUBCLASS_PDIR_TYPE */
		if (	(! attr_in_list( ATTR_SUBCLASS_PDIR_TYPE, pdattr.subclass )) )
			nim_error(	ERR_BAD_TYPE_FOR, attr_ass.list[i]->name,
							ATTR_SUBCLASS_PDIR_TYPE, NULL );
	}

	return( SUCCESS );

} /* end of ck_attrs */
	
/*---------------------------- dealloc_pdir            -------------------------
*
* NAME: dealloc_pdir
*
* FUNCTION:
*		deallocates pdir type resources
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
*			attr_ass
*
* RETURNS: (int)
*		SUCCESS					= resources deallocated
*		FAILURE					= error encountered
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
dealloc_pdir()

{	NIM_OBJECT( target, minfo )
	NIM_OBJECT( res, rinfo )
	int i,j,k;
	char *subdir;
	int rc;
	FILE *c_stdout=NULL;
	char *args[] = { RM, "-fr", NULL, NULL };
	ODMQUERY
	struct nim_pdattr pdattr;

	/* get the target object */
	if ( lag_object( 0, name, &target, &minfo ) == FAILURE )
		nim_error( 0, NULL, NULL, NULL );

	/* for each res specified */
	for (i=0; i < attr_ass.num; i++)
	{
		/* get the nim_pdattr for this attr */
		sprintf( query, "attr=%d", attr_ass.list[i]->pdattr );
		if ( odm_get_first( nim_pdattr_CLASS, query, &pdattr ) <= 0 )
			nim_error( ERR_ATTR_NAME, attr_ass.list[i]->name );

		/* skip attrs which are just flags */
		if ( pdattr.class == ATTR_CLASS_FLAGS )
			continue; 

		/* get the resource object & attr info */
		if ( lag_object( 0, attr_ass.list[i]->value, &res, &rinfo ) == FAILURE )
			nim_error( 0, NULL, NULL, NULL );

		/* ok to deallocate? */
		if ( ok_to_deallocate( target, res ) == FAILURE )
			nim_error( 0, NULL, NULL, NULL );

		/* find location & server */
		if (	((j = find_attr( res, NULL, NULL, 0, ATTR_LOCATION )) < 0) ||
				((k = find_attr( res, NULL, NULL, 0, ATTR_SERVER )) < 0) )
			nim_error( ERR_BAD_OBJECT, ATTR_CLASS_RESOURCES_T, res->name, NULL );

		/* perform pdir specific dealloc (ie, remove the target's subdir) */
		/* to do this, we need to form pathname of the target's subdir */
		subdir = nim_malloc( strlen(res->attrs[j].value) +
									strlen(target->name) + 2 );
		sprintf( subdir, "%s/%s", res->attrs[j].value, target->name );

		/* perform administrative deallocation stuff */
		if ( do_deallocation( target, res, subdir, &c_stdout ) == FAILURE ) 
			nim_error( 0, NULL, NULL, NULL );

		VERBOSE("   removing the %s:%s directory\n",res->attrs[k].value,
					res->attrs[j].value,NULL,NULL)

		/* now, actually remove the subdir */
		args[2] = subdir;
		if ( master_exec( res->attrs[k].value, &rc, &c_stdout, args ) == FAILURE )
			warning( 0, NULL, NULL, NULL );
		else if ( rc > 0 )
			warning( ERR_METHOD, res->attrs[k].value, niminfo.errstr, NULL );

		/* if this is a "root" resource, remove the ATTR_ROOT_INITIALIZED */
		if ( res->type->attr == ATTR_ROOT )
			rm_attr( target->id, NULL, ATTR_ROOT_INITIALIZED, 0, NULL );

		/* release the res object */
		uaf_object( res, &rinfo, FALSE );
	}

	return( SUCCESS );

} /* end of dealloc_pdir */

/**************************       main         ********************************/

main( int argc, char *argv[] )

{
	/* initialize NIM environment */
	mstr_init( argv[0], ERR_POLICY_METHOD, NULL );

	if ( ! get_list_space( &attr_ass, DEFAULT_CHUNK_SIZE, TRUE ) )
		nim_error( 0, NULL, NULL, NULL );

	/* parse command line */
	parse_args( argc, argv );

	VERBOSE("m_dealloc_pdir: deallocate a directory for %s\n",name,NULL,NULL,
				NULL)

	/* check command line attr assignments */
	ck_attrs();

	/* deallocate the nim_bundle */
	dealloc_pdir();

	exit( 0 );

}
