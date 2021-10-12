/* @(#)93	1.2  src/bos/usr/lib/nim/methods/m_chattr.c, cmdnim, bos411, 9428A410j  1/20/94  14:59:12 */
/*
 *   COMPONENT_NAME: CMDNIM
 *
 *   FUNCTIONS: ./usr/lib/nim/methods/m_chattr.c
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

extern int	valid_pdattr_ass();

/* --------------------------- module globals    ---------------------------- */
char	*name = NULL;
NIM_OBJECT( obj, info )
ATTR_ASS_LIST attr_ass;
int just_add = FALSE;

/* --------------------------- parse_args        ------------------------------
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
 *		SUCCESS	= no syntax errors on command line
 *
 * OUTPUT:
 * ----------------------------------------------------------------------------*/

int
parse_args(	int argc, 
				char *argv[] )

{
	extern char	*optarg;
	extern int	optind, optopt;
	int	c;

	/* 
	 * loop through all args 
	 */
	while ( (c = getopt(argc, argv, "a:A")) != -1 )
	{	switch (c)
		{
			case 'a':
				if (!parse_attr_ass( &attr_ass, NULL, optarg, TRUE ) )
					nim_error( 0, NULL, NULL, NULL );
			break;

			case 'A':
				just_add = TRUE;
			break;

			case ':': 
				nim_error( ERR_MISSING_OPT_ARG, optopt, NULL, NULL );
			break;

			case '?': 
				nim_error( ERR_BAD_OPT, optopt, MSG_msg(MSG_M_CHNET_SYNTAX), NULL );
			break;
		}
	}

	/* check for errors */
	if ( optind == argc )
		nim_error( ERR_MISSING_OPERAND, MSG_msg(MSG_OBJ_NAME), NULL, NULL );
	else if ( optind < (argc - 1) )
		nim_error( ERR_SYNTAX, MSG_msg(MSG_M_CHNET_SYNTAX), NULL, NULL );

	name = argv[optind];

	if ( get_object( &obj, &info, 0, name, 0, 0 ) <= 0 )
		nim_error( 0, NULL, NULL, NULL );

	/* return */
	return( SUCCESS );

} /* end of parse_args */

/* --------------------------- main              ---------------------------- */

main( int argc, char *argv[] )

{	int i;
	ODMQUERY
	struct nim_pdattr pdattr;
	struct nim_pdattr state;
	int type;
	char tmp[MAX_TMP];
	int rc;

	mstr_init( argv[0], ERR_POLICY_METHOD, NULL );

	if ( !get_list_space( &attr_ass, DEFAULT_CHUNK_SIZE, TRUE ) )
		nim_error( 0, NULL, NULL, NULL );

	parse_args( argc, argv );

	for (i=0; i < attr_ass.num; i++)
	{
		/* get the nim_pdattr info */
		sprintf( query, "name=%s", attr_ass.list[i]->name );
		if ( odm_get_first( nim_pdattr_CLASS, query, &pdattr ) <= 0 )
		{
			warning( ERR_ATTR_NOT_FOUND, query, NULL, NULL );
			continue;
		}

		if ( pdattr.mask & PDATTR_MASK_VAL_IS_STATE )
		{
			if ( strcmp( attr_ass.list[i]->value, STATE_CREADY_T ) == 0 )
			{
				/* machines or networks object? */
				if ( obj->type->class == ATTR_CLASS_MACHINES )
					sprintf( tmp, "%d", STATE_CREADY );
				else if ( obj->type->class == ATTR_CLASS_NETWORKS )
					sprintf( tmp, "%d", STATE_NREADY );
				else
				{
					warning( ERR_INVALID_ATTR, attr_ass.list[i]->value, NULL, NULL );
					continue;
				}
				attr_ass.list[i]->value = tmp;
			}
			else
			{
				/* get the nim_pdattr for the new state */
				sprintf( query, "class=%d and value=%s",
							ATTR_CLASS_STATES, attr_ass.list[i]->value );
				if ( odm_get_first( nim_pdattr_CLASS, query, &state ) <= 0 )
				{
					warning( ERR_ATTR_NOT_FOUND, query, NULL, NULL );
					continue;
				}
				attr_ass.list[i]->value = state.type;
			}
		}	

		if ( just_add )
			rc = mk_attr(	obj->id, NULL, attr_ass.list[i]->value, 
								attr_ass.list[i]->seqno,
								pdattr.attr,
								attr_ass.list[i]->name );
		else
			rc = ch_attr(	obj->id, NULL, attr_ass.list[i]->value, 
								attr_ass.list[i]->seqno,
								pdattr.attr,
								attr_ass.list[i]->name );

		if ( rc == FAILURE )
			warning( 0, NULL, NULL, NULL );
	}

	exit( 0 );
}
