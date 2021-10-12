static char	sccs_id[] = " @(#)71 1.3  src/bos/usr/lib/nim/methods/m_dealloc_spot.c, cmdnim, bos411, 9428A410j  5/25/94  13:10:19";

/*
 *   COMPONENT_NAME: CMDNIM
 *
 *   FUNCTIONS: /src/bos/usr/lib/nim/methods/m_dealloc_spot.c
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

/*---------------------------- dealloc_spot            -------------------------
*
* NAME: dealloc_spot
*
* FUNCTION:
*		deallocates a spot type resource
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
dealloc_spot()

{	NIM_OBJECT( target, minfo )
	NIM_OBJECT( res, rinfo )
	int i,j,k;
	char *subdir;
	int rc;
	FILE *c_stdout=NULL;
	char *args[] = { RM, "-fr", NULL, NULL };
	ODMQUERY
	struct nim_pdattr pdattr;
	int boot_res = -1;

	/* get the target object */
	if ( lag_object( 0, name, &target, &minfo ) == FAILURE )
		nim_error( 0, NULL, NULL, NULL );

	/* does target still have a boot resource allocated? */
	/* if so, we can't allow the spot to be deallocated yet because the boot */
	/*		deallocation method relies on knowing where the SPOT is */
	boot_res = find_attr( target, NULL, NULL, 0, ATTR_BOOT );

	/* for each res specified */
	for (i=0; i < attr_ass.num; i++)
	{
		/* skip anything that's not a SPOT */
		if ( attr_ass.list[i]->pdattr != ATTR_SPOT )
			continue;

		/* get the resource object & attr info */
		if ( lag_object( 0, attr_ass.list[i]->value, &res, &rinfo ) == FAILURE )
			nim_error( 0, NULL, NULL, NULL );

		/* ok to deallocate? */
		if ( boot_res >= 0 )
			nim_error( ERR_DEALLOC_DEPEND, attr_ass.list[i]->value,
							target->attrs[boot_res].value, NULL );
		else if ( ok_to_deallocate( target, res ) == FAILURE )
			nim_error( 0, NULL, NULL, NULL );

		/* find res location */
		if ( (j = find_attr( res, NULL, NULL, 0, ATTR_LOCATION )) < 0 )
			nim_error( ERR_OBJ_MISSING_ATTR, ATTR_LOCATION, res->name, NULL );

		/* perform administrative deallocation stuff */
		if ( do_deallocation(	target, res, 
										res->attrs[j].value, &c_stdout ) == FAILURE ) 
			nim_error( 0, NULL, NULL, NULL );

		/* release the res object */
		uaf_object( res, &rinfo, FALSE );
	}

	return( SUCCESS );

} /* end of dealloc_spot */

/**************************       main         ********************************/

main( int argc, char *argv[] )

{
	/* initialize NIM environment */
	mstr_init( argv[0], ERR_POLICY_METHOD, NULL );

	if ( ! get_list_space( &attr_ass, DEFAULT_CHUNK_SIZE, TRUE ) )
		nim_error( 0, NULL, NULL, NULL );

	/* parse command line */
	parse_args( argc, argv );

	VERBOSE("m_dealloc_spot: deallocate a SPOT for %s\n",name,NULL,NULL,NULL)

	/* deallocate a SPOT */
	dealloc_spot();

	exit( 0 );

}
