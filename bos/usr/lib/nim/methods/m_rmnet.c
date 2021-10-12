static char	sccs_id[] = " @(#)86 1.4  src/bos/usr/lib/nim/methods/m_rmnet.c, cmdnim, bos411, 9428A410j  4/6/94  17:22:51";

/*
 *   COMPONENT_NAME: CMDNIM
 *
 *   FUNCTIONS: ./usr/lib/nim/methods/m_rmnet.c
 *		
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure nettricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "cmdnim_mstr.h"

extern int valid_pdattr_ass();
extern int ch_pdattr_ass();

/* --------------------------- module globals    ---------------------------- */
ATTR_ASS_LIST attr_ass;

VALID_ATTR valid_attrs[] =
{	
 	{ATTR_FORCE,				ATTR_FORCE_T,				FALSE,	ch_pdattr_ass},
 	{ATTR_VERBOSE,				ATTR_VERBOSE_T,			FALSE,	ch_pdattr_ass},
	{0,							NULL, 						FALSE, NULL}
};

char *name=NULL;
	
/*---------------------------- rmnet             ------------------------------
*
* NAME: rmnet
*
* FUNCTION:
*		removes a network nim_object & its associated nim_attrs
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
*		SUCCESS					= object removed
*		FAILURE					= error encountered
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
rmnet()

{	struct nim_object *obj;
	struct listinfo *info;
	ODMQUERY
	int i;

	VERBOSE("   verifying that this network may be removed at this time\n",
				NULL,NULL,NULL,NULL)

	/* get & backup this object */
	if ( backup_object( &backups, 0, name, &obj, &info ) == FAILURE )
		nim_error( 0, NULL, NULL, NULL );

	/* make sure that no machines are connected to this net; ie, make sure */
	/*		that this net is NOT referenced in any ATTR_IF */
	sprintf( query, "%s *", name );
	if ( get_attr( NULL, NULL, 0, query, 0, ATTR_IF ) > 0 )
		nim_error( ERR_REFERENCED, name, NULL, NULL );

	/* remove any routes this network had */
	for (i=0; i < obj->attrs_info->num; i++)
		if ( obj->attrs[i].pdattr->attr == ATTR_ROUTING )
		{	if ( rm_net_route( name, (obj->attrs + i), &backups ) == FAILURE )
				nim_error( 0, NULL, NULL, NULL );
		}

	/* remove the object */
	if ( rm_object( 0, name ) == FAILURE )
		nim_error( 0, NULL, NULL, NULL );

	/* success! - clear backups */
	clear_backups( &backups );

	return( SUCCESS );

} /* end of rmnet */

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
	int syntax_err=FALSE;

	/* loop through all args */
	while ( (c = getopt(argc, argv, "q")) != -1 )
	{	switch (c)
		{	
			case 'q': /* display valid_attrs */
				mstr_what( valid_attrs, NULL );
				exit( 0 );
			break;

			case ':': /* option is missing a required argument */
				nim_error( ERR_MISSING_OPT_ARG, optopt, NULL, NULL );
			break;

			case '?': /* unknown option */
				nim_error( ERR_BAD_OPT, optopt, MSG_msg(MSG_M_RMNET_SYNTAX), NULL );
			break;
		}

		if ( syntax_err )
			nim_error( ERR_SYNTAX, MSG_msg(MSG_M_RMNET_SYNTAX), NULL, NULL );
	}

	/* object name (the only operand) is required */
	if ( optind != (argc-1) )
		nim_error( ERR_SYNTAX, MSG_msg(MSG_M_RMNET_SYNTAX), NULL, NULL );

	name = argv[optind];

	/* return */
	return( SUCCESS );

} /* end of parse_args */

/**************************       main         ********************************/

main( int argc, char *argv[] )
{
	/* initialize NIM environment */
	mstr_init( argv[0], ERR_POLICY_METHOD, NULL );
	if ( get_list_space( &attr_ass, DEFAULT_CHUNK_SIZE, TRUE ) == FAILURE )
		nim_error( 0, NULL, NULL, NULL );

	/* parse command line */
	parse_args( argc, argv );

	VERBOSE("m_rmnet: remove the %s network\n",name,NULL,NULL,NULL)

	/* remove the specified network object */
	rmnet();

	exit( 0 );

} /* end of main */
