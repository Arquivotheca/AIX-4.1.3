static char	sccs_id[] = " @(#)53 1.3  src/bos/usr/lib/nim/methods/m_chres.c, cmdnim, bos411, 9428A410j  4/6/94  09:33:17";

/*
 *   COMPONENT_NAME: CMDNIM
 *
 *   FUNCTIONS: ch_res
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

extern 	int	valid_pdattr_ass();
extern 	int	ch_pdattr_ass();

/* --------------------------- module globals    ---------------------------- */
ATTR_ASS_LIST attr_ass;

#define MAX_VALID_ATTRS		2
VALID_ATTR valid_attrs[] =
{
	{ ATTR_COMMENTS, 		ATTR_COMMENTS_T,	FALSE, 	ch_pdattr_ass },
	{ ATTR_FORCE, 			ATTR_FORCE_T, 		FALSE, 	ch_pdattr_ass },
	{ ATTR_VERBOSE, 		ATTR_VERBOSE_T, 	FALSE, 	ch_pdattr_ass },
	{ 0, 						NULL, 				FALSE, 	NULL }
};

char	*name = NULL;


/* --------------------------- ch_res
 *
 * NAME: ch_res
 *
 * FUNCTION: changes attributes for a resource object
 *
 * NOTES:
 *	calls nim_error
 *
 * DATA STRUCTURES:
 *	parameters:
 *	global:
 *		name
 *
 * RETURNS: (int)
 *	SUCCESS	= attrs changed
 *	FAILURE	= error encountered
 * ------------------------------------------------------------------------- */

int
ch_res(char *res_name)

{
	int	rc = SUCCESS;
	NIM_OBJECT( robj, rinfo )
	int	current_cstate = 0;
	int	i;

	/* backup all info about current object */
	if ( lag_object( 0, res_name, &robj, &rinfo ) == FAILURE )
		nim_error( 0, NULL, NULL, NULL );

	/* change the specified attrs */
	for (i = 0; i < attr_ass.num; i++)
		if ( ch_attr(	robj->id, NULL, attr_ass.list[i]->value,
		    attr_ass.list[i]->seqno, attr_ass.list[i]->pdattr,
		    attr_ass.list[i]->name ) == FAILURE ) {
			rc = FAILURE;
			break;
		}
	if ( rc == FAILURE ) {
		protect_errstr = TRUE;
		rest_object( robj, &rinfo );
		nim_error( 0, NULL, NULL, NULL );
	}

	return( SUCCESS );
}


/* --------------------------- parse_args 
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
 *	parameters:
 *		argc = argc from main
 *		argv = argv from main
 *	global:
 *
 * RETURNS: (int)
 *	SUCCESS	= no syntax errors on command line
 *
 * OUTPUT:
 * -------------------------------------------------------------------------- */

int
parse_args(	int argc, 
char *argv[] )

{
	extern char	*optarg;
	extern int	optind, optopt;
	int	c;
	int	syntax_err = FALSE;

	/* loop through all args */
	while ( (c = getopt(argc, argv, "a:q")) != -1 ) {
		switch (c) {
		case 'a': /* attribute assignment */
			if (!parse_attr_ass( &attr_ass, valid_attrs, optarg, TRUE ) )
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
			nim_error(ERR_BAD_OPT, optopt, MSG_msg(MSG_GENERIC_CHSYNTAX), NULL);
		break;
		}

		if ( syntax_err )
			nim_error( ERR_SYNTAX, MSG_msg(MSG_GENERIC_CHSYNTAX), NULL, NULL );
	}

	/* only one operand and it had better NOT be master */
	if ( (optind != (argc - 1)) || (strcmp( argv[optind], ATTR_MASTER_T ) == 0) )
		nim_error( ERR_SYNTAX, MSG_msg(MSG_GENERIC_CHSYNTAX), NULL, NULL );

	name = argv[optind];

	/* return */
	return( SUCCESS );

} /* end of parse_args */


/**************************       main         ********************************/

main( int argc, char *argv[] )
{
	/* initialize NIM environment */
	mstr_init( argv[0], ERR_POLICY_METHOD, NULL );

	if ( !get_list_space( &attr_ass, MAX_VALID_ATTRS, TRUE ) )
		nim_error( 0, NULL, NULL, NULL );

	/* parse command line */
	parse_args( argc, argv );

	VERBOSE("m_chres: change attributes for the %s resource\n",name,NULL,NULL,
				NULL)

	ch_res(name);

	exit( 0 );
}


