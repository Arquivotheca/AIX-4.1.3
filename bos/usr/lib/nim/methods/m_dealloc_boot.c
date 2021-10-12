static char	sccs_id[] = " @(#)86 1.6  src/bos/usr/lib/nim/methods/m_dealloc_boot.c, cmdnim, bos411, 9428A410j  7/5/94  15:20:55";

/*
 *   COMPONENT_NAME: CMDNIM
 *
 *   FUNCTIONS: ./usr/lib/nim/methods/m_dedealloc_boot.c
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

extern int ch_pdattr_ass();

/*---------------------------- module globals    -----------------------------*/
ATTR_ASS_LIST attr_ass;

VALID_ATTR valid_attrs[] =
{	
	{0,					NULL,				FALSE,		ch_pdattr_ass}
};

char *name=NULL;									/* target object name */

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

/*------------------------- dealloc_boot            ----------------------------
*
* NAME: dealloc_boot
*
* FUNCTION:
*		deallocates the network boot resource for the specified machine
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
*		SUCCESS					= boot resource deallocated
*		FAILURE					= error encountered
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
dealloc_boot()

{	NIM_OBJECT( target, minfo )
	NIM_OBJECT( boot, binfo )
	struct nim_if nimif;
	int i;
	LIST list;
	struct res_access *spot = NULL;
	char hostname[MAX_TMP];
	char *args[] = { C_DEALLOC_BOOT, hostname, NULL };
	int rc;
	FILE *c_stdout = NULL;

	/* get the target object */
	if ( lag_object( 0, name, &target, &minfo ) == FAILURE )
		nim_error( 0, NULL, NULL, NULL );

	/* get info about target's primary net */
	if ( (i = find_attr( target, NULL, NULL, 1, ATTR_IF )) == -1 )
		nim_error( ERR_OBJ_MISSING_ATTR, PIF, target->name, NULL );
	else if ( s2nim_if( target->attrs[i].value, &nimif ) == FAILURE )
		nim_error( ERR_NET_INFO, PIF, target->name, NULL );

	/* generate a LIST of res_access structs */
	if ( LIST_res_access( target, &list ) == FAILURE )
		nim_error( 0, NULL, NULL, NULL );

	/* find the SPOT in the LIST */
	for (i=0; i < list.num; i++)
	{	spot = (struct res_access *) list.list[i];
		if ( spot->type == ATTR_SPOT )
			break;
		spot = NULL;
	}
	if ( spot == NULL )
		nim_error( ERR_BAD_OBJECT, ATTR_msg(ATTR_CLASS_MACHINES), name, NULL );

	/* initialize parameter string for C_DEALLOC_BOOT */
	sprintf( hostname, "-a%s=%s", ATTR_HOSTNAME_T, nimif.hostname );

	/* get the boot resource */
	if ( lag_object( 0, ATTR_BOOT_T, &boot, &binfo ) == FAILURE )
		nim_error( 0, NULL, NULL, NULL );

	VERBOSE("   deallocating the boot resource which resides on %s\n",
				spot->server,NULL,NULL,NULL)

	/* remove the boot resource on the SPOT server */
	if ( master_exec( spot->server, &rc, &c_stdout, args ) == FAILURE )
		nim_error( 0, NULL, NULL, NULL );
	else if ( rc > 0 )
		nim_error( ERR_METHOD, spot->server, niminfo.errstr, NULL );

	/* generic administrative dealloc stuff */
	if ( do_deallocation( target, boot, NULL, &c_stdout ) == FAILURE )
		nim_error( 0, NULL, NULL, NULL );
	uaf_object( boot, &binfo, FALSE );

	/* remove old info */
	rm_attr( target->id, NULL, ATTR_BOOT_INFO, 0, NULL );

	return( SUCCESS );

} /* end of dealloc_boot */

/**************************       main         ********************************/

main( int argc, char *argv[] )

{
	/* initialize NIM environment */
	mstr_init( argv[0], ERR_POLICY_METHOD, NULL );
   if ( ! get_list_space( &attr_ass, DEFAULT_CHUNK_SIZE, TRUE ) )
      nim_error( 0, NULL, NULL, NULL );

	/* parse command line */
	parse_args( argc, argv );

	VERBOSE("m_dealloc_boot: deallocate the boot resource for %s\n",name,NULL,
				NULL,NULL)

	/* deallocate a network boot resource */
	dealloc_boot();

	exit( 0 );

}
