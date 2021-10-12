static char	sccs_id[] = " @(#)32 1.14  src/bos/usr/lib/nim/methods/m_allocate.c, cmdnim, bos411, 9428A410j  6/5/94  16:10:35";

/*
 *   COMPONENT_NAME: CMDNIM
 *
 *   FUNCTIONS: ./usr/lib/nim/methods/m_allocate.c
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
{	
	{0,							NULL,							FALSE,	valid_pdattr_ass}
};

char *name=NULL;									/* object name */
	
/*---------------------------- ck_attrs          ------------------------------
*
* NAME: ck_attrs
*
* FUNCTION:
*		verifies that the supplied attrs are either resource types or flags; no
*			other types of attrs are allowed
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
*			attr_ass
*
* RETURNS: (int)
*		SUCCESS					= all command line attrs are acceptable
*		FAILURE					= invalid attr specified
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
ck_attrs()

{	int i;
	NIM_PDATTR( pdattr, pinfo )

	for (i=0; i < attr_ass.num; i++)
	{
		/* get the predefined object for this attr */
		if ( get_pdattr(	&pdattr, &pinfo, 0, 0, 0, attr_ass.list[i]->pdattr,
								attr_ass.list[i]->name ) > 0 )
		{
			/* pdattr must be a resouce type -OR- an attr flags */
			if (	(pdattr->class == ATTR_CLASS_RESOURCES) &&
					(attr_in_list( ATTR_SUBCLASS_TYPE, pdattr->subclass )) )
				continue;
			else if ( pdattr->class == ATTR_CLASS_FLAGS )
				continue;

			/* if we get here, attr was specified which doesn't apply to the */
			/*		"allocate" operation */
			nim_error( ERR_INVALID_ATTR, attr_ass.list[i]->name, NULL, NULL );
		}
	}

	odm_free_list( pdattr, &pinfo );

	return( SUCCESS );

} /* end of ck_attrs */

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

/*---------------------------- res_alloc            ----------------------------
*
* NAME: res_alloc
*
* FUNCTION:
*		allocates the resources specified by command line attr assignments
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
*		SUCCESS					= resources allocated
*		FAILURE					= error encountered
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
res_alloc()

{	NIM_OBJECT( client, cinfo )
	struct nim_if nimif;
	int i;
	ODMQUERY
	struct nim_pdattr pdattr;
	FILE *c_stdout = NULL;
	FILE **outptr = NULL;

	/* get the client object */
	if ( lag_object( 0, name, &client, &cinfo ) == FAILURE )
		nim_error( 0, NULL, NULL, NULL );

	/* find the client's PIF */
	if ( (i = find_attr( client, NULL, NULL, 1, ATTR_IF )) == -1 )
		nim_error( ERR_OBJ_MISSING_ATTR, PIF, client->name, NULL );

	/* convert to nim_if */
	if ( s2nim_if( client->attrs[i].value, &nimif ) == FAILURE )
		nim_error( ERR_NET_INFO, PIF, client->name, NULL );

	/* trap stdout from allocation methods? */
	if (	(find_attr_ass( &attr_ass, ATTR_DISP_MNTPNT ) < 0) &&
			(find_attr_ass( &attr_ass, ATTR_MOUNT_CTRL ) < 0) )
		/* yes, trap it */
		outptr = &c_stdout;

	/* for each resource specified on the command line... */
	for (i=0; i < attr_ass.num; i++)
	{	
		/* get the nim_pdattr */
		sprintf( query, "attr=%d", attr_ass.list[i]->pdattr );
		if ( odm_get_first( nim_pdattr_CLASS, query, &pdattr ) <= 0 )
			nim_error( ERR_ODM, (char *)odmerrno, NULL, NULL );

		/* skip non-resource attrs */
		if ( pdattr.class != ATTR_CLASS_RESOURCES )
			/* some kind of flag being passed to the method */
			continue;

		/* allocate it to the client */
		if ( alloc_res(	client, attr_ass.list[i]->value, 
								&nimif, outptr ) == FAILURE )
			nim_error( 0, NULL, NULL, NULL );

		/* check for ATTR_MISSING for this type of resource */
		/* ATTR_MISSING is added by control operations for each required */
		/*		resource which hasn't been allocated yet */
		/* we want these "missing" attrs to go away once when allocated */
		rm_attr( client->id, NULL, ATTR_MISSING, 0, pdattr.name );
	}

	return( SUCCESS );

} /* end of res_alloc */

/**************************       main         ********************************/

main( int argc, char *argv[] )

{
	/* initialize NIM environment */
	mstr_init( argv[0], ERR_POLICY_METHOD, NULL );

	if ( ! get_list_space( &attr_ass, DEFAULT_CHUNK_SIZE, TRUE ) )
		nim_error( 0, NULL, NULL, NULL );

	/* parse command line */
	parse_args( argc, argv );

	VERBOSE("m_allocate: allocate a resource to %s\n",name,NULL,NULL,NULL)

	/* check command line */
	ck_attrs();

	/* allocate the specified resources */
	res_alloc();

	exit( 0 );

}
