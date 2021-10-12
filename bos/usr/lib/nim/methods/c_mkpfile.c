/* @(#)48	1.1  src/bos/usr/lib/nim/methods/c_mkpfile.c, cmdnim, bos411, 9428A410j  8/22/93  12:29:11 */
/*
 *   COMPONENT_NAME: CMDNIM
 *
 *   FUNCTIONS: ./usr/lib/nim/methods/c_mkpfile.c
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

#include "cmdnim_cmd.h"
#include <fcntl.h>
#include <sys/mode.h>

#define PAGESIZE 4096			/* fundamental page size of machine */


extern int valid_pdattr_ass();

/*---------------------------- module globals    -----------------------------*/
ATTR_ASS_LIST attr_ass;

#define MAX_VALID_ATTRS				3
VALID_ATTR valid_attrs[] =
{	{ATTR_SIZE,					ATTR_SIZE_T,				TRUE,		NULL},
	{ATTR_LOCATION,			ATTR_LOCATION_T,			TRUE,		NULL},
	{0,							NULL, 						FALSE,	NULL}
};
	
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
	while ( (c = getopt(argc, argv, "a:")) != -1 )
	{	switch (c)
		{	
			case 'a': /* attribute assignment */
				if (! parse_attr_ass( &attr_ass, valid_attrs, optarg, FALSE ) )
					nim_error( 0, NULL, NULL, NULL );
				break;

			case ':': /* option is missing a required argument */
				nim_error( ERR_MISSING_OPT_ARG, optopt, NULL, NULL );
				break;

			case '?': /* unknown option */
				nim_error( ERR_BAD_OPT, optopt, MSG_msg(MSG_M_RMPDIR_SYNTAX), NULL);
				break;
		}
	}

	/* check for errors */
	if ( optind != argc )
		nim_error( ERR_SYNTAX, MSG_msg(MSG_M_RMPDIR_SYNTAX), NULL, NULL );

	/* return */
	return( SUCCESS );

} /* end of parse_args */

/*---------------------------- create_pfile      ------------------------------
*
* NAME: create_pfile
*
* FUNCTION:
*		creates a file of the specified size
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
*			pathname				= pathname of file
*			size					= size of file to create
*		global:
*
* RETURNS: (int)
*		SUCCESS					= file created
*		FAILURE					= system error?
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
create_pfile(	char *pathname,
					int size )

{	int fd;
	int i;
	char buf[PAGESIZE];		/* page size buffer to write from */

	/* create the file */
	if ( (fd = open(	pathname, 
							O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR)) < 0 )
		ERROR( ERR_ERRNO, NULL, NULL, NULL )
	
	/* fill it with to requested size */
	for (i = size * 1024 * 1024 / PAGESIZE; i > 0; i--)
		if (PAGESIZE != write(fd, buf, PAGESIZE))
			ERROR( ERR_ERRNO, NULL, NULL, NULL )

	return( SUCCESS );

} /* end of create_pfile */

/**************************       main         ********************************/

main( int argc, char *argv[] )

{	int i;
	char *pathname = NULL;
	int size = -1;

	/* initialize NIM environment */
	nim_init( argv[0], ERR_POLICY_METHOD, NULL, NULL );

	/* initialize the attr_ass LIST */
	if ( get_list_space( &attr_ass, 2, TRUE ) == FAILURE )
		nim_error( 0, NULL, NULL, NULL );

	/* parse command line */
	parse_args( argc, argv );

	/* find the file pathname & size */
	for (i=0; i < attr_ass.num; i++)
		if ( attr_ass.list[i]->pdattr == ATTR_LOCATION )
			pathname = attr_ass.list[i]->value;
		else if ( attr_ass.list[i]->pdattr == ATTR_SIZE )
			size = strtol( attr_ass.list[i]->value, NULL, 0 );

	/* make sure pathname & size were given */
	if ( pathname == NULL )
		nim_error( ERR_MISSING_ATTR, ATTR_LOCATION_T, NULL, NULL );
	else if ( size < 0 )
		nim_error( ERR_MISSING_ATTR, ATTR_SIZE_T, NULL, NULL );

	/* create the file */
	if ( create_pfile( pathname, size ) == FAILURE )
		nim_error( 0, NULL, NULL, NULL );

	exit( 0 );
}
