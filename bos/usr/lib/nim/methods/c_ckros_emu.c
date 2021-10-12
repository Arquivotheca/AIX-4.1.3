char	sccs_id[] = " @(#)08 1.5  src/bos/usr/lib/nim/methods/c_ckros_emu.c, cmdnim, bos411, 9428A410j  5/16/94  10:02:18";

/*
 *   COMPONENT_NAME: CMDNIM
 *
 *   FUNCTIONS: check_input
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

#include "cmdnim_cmd.h"
#include <sys/bootrecord.h> 

#define	EMULATION_FLAG		1

/*---------------------------- module globals    -----------------------------*/
ATTR_ASS_LIST attr_ass;

VALID_ATTR valid_attrs[] = {
	{ ATTR_IPLROM_EMU,	ATTR_IPLROM_EMU_T,	TRUE,   NULL },
	{ 0,                    NULL,                   FALSE,   NULL }
};


/* --------------------------- parse_args 
 *
 * NAME: parse_args
 *
 * FUNCTION:
 *     parses command line arguments
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *     calls nim_error
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *     parameters:
 *        argc        = argc from main
 *        argv        = argv from main
 *     global:
 *
 * RETURNS: (int)
 *     SUCCESS              = no syntax errors on command line
 *
 * OUTPUT:
 * ----------------------------------------------------------------------------*/

int
parse_args( int argc, 
char *argv[] )

{
	extern char	*optarg;
	extern int	optind, optopt;
	int	c;
	int	syntax_err = FALSE;

	while ( (c = getopt(argc, argv, "a:")) != -1 ) {
		switch (c) {
		case 'a': /* attribute assignment */
			if (!parse_attr_ass( &attr_ass, valid_attrs, optarg, FALSE ) )
				nim_error( 0, NULL, NULL, NULL );
			break;

		case ':': /* option is missing a required argument */
			nim_error( ERR_MISSING_OPT_ARG, optopt, NULL, NULL );
			break;

		case '?': /* unknown option */
			nim_error( ERR_BAD_OPT, optopt, MSG_msg(MSG_C_CKROS_SYNTAX), NULL );
			break;
		}

		if ( syntax_err )
			nim_error( ERR_SYNTAX, MSG_msg(MSG_C_CKROS_SYNTAX), NULL, NULL );
	}

	return( SUCCESS );

} /* end of parse_args */


/* ---------------------------- check_input
 *
 * NAME: check_input
 *
 * FUNCTION:
 * checks to make sure that the information supplied by user is sufficient
 * to complete operation
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *    calls nim_error
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *    global:
 *       valid_attrs
 *       attr_ass
 *
 * RETURNS: (int)
 *    SUCCESS  = nothing missing
 *    FAILURE  = definition incomplete
 *
 * OUTPUT:
 * -----------------------------------------------------------------------------
 */

int
check_input()

{
	int	va_indx, attr_indx;
	VALID_ATTR   * va_ptr;
	ATTR_ASS     * aa_ptr;

	/* 
    * check for required attrs 
	 */
	for (va_indx = 0; valid_attrs[va_indx].pdattr; va_indx++) {
		va_ptr = &valid_attrs[va_indx];
		if ( va_ptr->required ) {
			for (attr_indx = 0; attr_indx < attr_ass.num; attr_indx++) {
				aa_ptr = attr_ass.list[attr_indx];
				if ( aa_ptr->pdattr == va_ptr->pdattr )
					break;
			}
			/* 
 		 	 * is the required attr present? 
 		 	 */
			if ( attr_indx == attr_ass.num )
				nim_error( ERR_MISSING_ATTR, va_ptr->name, NULL, NULL );
		}
	}
}


main(	int argc, 
char *argv[])

{

	IPL_REC	br;
	char	*pathname;
	FILE		 * fp;

	/* 
	 * Do the common nim init stuff. 
	 */
	nim_init( argv[0], ERR_POLICY_FOREGROUND, NULL, NULL );

	/* 
	 * we are expecting only one attribute (ros_emu)
	 */
	parse_args( argc, argv );
	check_input();

	pathname = attr_ass.list[0]->value;
	/* 
	 * As we only have one attribute and we came back from parse_args
	 * ( it exits on an error ) no need to search through attr_ass for 
	 * it.  Now attempt to open and read the boot record.
	 */
	if ( (fp = fopen(pathname, "r")) == NULL )
		nim_error(ERR_ERRNO, "", NULL, NULL);

	if (fread(&br, sizeof(br), 1, fp ) != 1)
		nim_error(ERR_ERRNO, "", NULL, NULL);

	/* 
	 * Got some bytes off the media, now see if this is really a 
	 * boot record by looking for a magic number.
	 */
	if ( br.IPL_record_id != IPLRECID ) 
		nim_error(ERR_NOT_IPLEMU, pathname, NULL, NULL );
	

	if ( br.boot_emulation == EMULATION_FLAG ) 
		exit(0);

	nim_error(ERR_NOT_IPLEMU, pathname, NULL, NULL );
}
