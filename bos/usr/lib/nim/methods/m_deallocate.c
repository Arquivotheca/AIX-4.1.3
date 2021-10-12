static char sccs_id[] = " @(#)08 1.9  src/bos/usr/lib/nim/methods/m_deallocate.c, cmdnim, bos411, 9428A410j  5/25/94  13:10:42";
/*
 *   COMPONENT_NAME: CMDNIM
 *
 *   FUNCTIONS: ./usr/lib/nim/methods/m_deallocate.c
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
	{0,							NULL,							FALSE,	valid_pdattr_ass }
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

/*---------------------------- res_dealloc            --------------------------
*
* NAME: res_dealloc
*
* FUNCTION:
*		deallocates the resources specified in attr_ass
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
res_dealloc()

{	NIM_OBJECT( target, minfo )
	int i;
	ODMQUERY
	struct nim_pdattr pdattr;
	struct nim_attr attr;
	FILE *c_stdout = NULL;
	FILE **outptr = NULL;
	int subclass = -1;

	/* get the target object */
	if ( lag_object( 0, name, &target, &minfo ) == FAILURE )
		nim_error( 0, NULL, NULL, NULL );

	/* trap stdout from deallocation methods? */
	if (	(find_attr_ass( &attr_ass, ATTR_DISP_MNTPNT ) < 0) &&
			(find_attr_ass( &attr_ass, ATTR_MOUNT_CTRL ) < 0) )
		/* yes, trap the output */
		outptr = &c_stdout;

	/* if ATTR_SUBCLASS specified, then ... */
	if ( (i = find_attr_ass( &attr_ass, ATTR_SUBCLASS )) >= 0 )
	{
		/* was "all" specified? */
		if ( strcmp( attr_ass.list[i]->value, ATTR_ALL_T ) == 0 )
			subclass = 0;
		else
		{
			/* get the nim_pdattr for this subclass */
			sprintf( query, "class=%d and value=%s", ATTR_SUBCLASS,
						attr_ass.list[i]->value );
			if ( odm_get_first( nim_pdattr_CLASS, query, &pdattr ) <= 0 )
				nim_error(	ERR_VALUE, attr_ass.list[i]->value, 
								ATTR_SUBCLASS_T, NULL );
			subclass = pdattr.attr;
		}

		VERBOSE("   deallocating all resources of the %s subclass\n",
					attr_ass.list[i]->value,NULL,NULL,NULL)

		/* deallocate all resources which belong to the specified subclass */
		for (i=0; i < target->attrs_info->num; i++)
			if (	(target->attrs[i].pdattr->class == ATTR_CLASS_RESOURCES) &&
					((subclass == 0) || 
					 (attr_in_list( subclass, target->attrs[i].pdattr->subclass))))
			{
				/* if the attr really exists still in the object class dealloc  */
				sprintf(query, "id=%d and pdattr=%s and value='%s'", target->id, 
					target->attrs[i].pdattr_Lvalue, target->attrs[i].value); 
				if (odm_get_first( nim_attr_CLASS, query, &attr) > 0 )
					if ( dealloc_res( target, target->attrs[i].value, 
										outptr ) == FAILURE )
						warning( 0, NULL, NULL, NULL );
			}

	}
	else
	{
		/* deallocate each specified resource */
		for (i=0; i < attr_ass.num; i++)
		{
			/* is this attr a resource? */
			sprintf( query, "attr=%d", attr_ass.list[i]->pdattr );
			if ( odm_get_first( nim_pdattr_CLASS, query, &pdattr ) <= 0 )
				nim_error( ERR_ODM, (char *) odmerrno, NULL, NULL );
			else if ( pdattr.class != ATTR_CLASS_RESOURCES )
				continue;

			if ( dealloc_res( target, attr_ass.list[i]->value,
									outptr ) == FAILURE )
				warning( 0, NULL, NULL, NULL );
		}
	}

	return( SUCCESS );

} /* end of res_dealloc */

/**************************       main         ********************************/

main( int argc, char *argv[] )

{
	/* initialize NIM environment */
	mstr_init( argv[0], ERR_POLICY_METHOD, NULL );

	if ( ! get_list_space( &attr_ass, DEFAULT_CHUNK_SIZE, TRUE ) )
		nim_error( 0, NULL, NULL, NULL );

	/* parse command line */
	parse_args( argc, argv );

	VERBOSE("m_deallocate: deallocate resources for %s\n",name,NULL,NULL,NULL)

	/* deallocate the specified resources */
	res_dealloc();

	exit( 0 );

}
