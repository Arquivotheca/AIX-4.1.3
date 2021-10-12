static char sccs_id[] = " @(#)55 1.3  src/bos/usr/lib/nim/methods/c_stat.c, cmdnim, bos411, 9428A410j  3/14/94  13:28:22";
/*
 *   COMPONENT_NAME: CMDNIM
 *
 *   FUNCTIONS: ./usr/lib/nim/methods/c_stat.c
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
#include <sys/vmount.h>

/* --------------------------- module globals    ---------------------------- */
ATTR_ASS_LIST attr_ass;

VALID_ATTR valid_attrs[] =
{	
	{ATTR_LOCATION,			ATTR_LOCATION_T,			TRUE,		NULL},
	{ATTR_MODE,					ATTR_MODE_T,				FALSE,	NULL},
	{ATTR_VFSTYPE,				ATTR_VFSTYPE_T,			FALSE,	NULL},
	{ATTR_VERBOSE,				ATTR_VERBOSE_T,			FALSE,	NULL},
	{0,							NULL, 						FALSE,	NULL}
};

/* --------------------------- parse_args        -------------------------------
 *
 * NAME: parse_args
 *
 * FUNCTION:
 *		parses command line arguments
 *
 * NOTES:
 *		calls nim_error
 *
 * DATA STRUCTURES:
 *	parameters:
 *		argc	= argc from main
 *		argv	= argv from main
 *	global:
 *
 * RETURNS: (int)
 *	SUCCESS		= no syntax errors on command line
 *
 * -------------------------------------------------------------------------- */

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
				nim_error( ERR_BAD_OPT, optopt,MSG_msg(MSG_GENERIC_MKSYNTAX), NULL);
			break;
		}
	}

	/* check for errors */
	if ( optind != argc )
		nim_error( ERR_SYNTAX, MSG_msg(MSG_GENERIC_MKSYNTAX), NULL, NULL );

	/* return */
	return( SUCCESS );

} /* end of parse_args */
	
/**************************       main         ********************************/

main( int argc, char *argv[] )

{	char *pathname;
	char *mode;
	mode_t mode_mask;
	mode_t file_type;
	mode_t perms;
	char *vfstypes;
	int vfstype;
	struct stat buf;
	char *ptr;
	char tmp[MAX_TMP];
	char tmp1[MAX_TMP];
	
	/* initialize NIM environment */
	nim_init( argv[0], ERR_POLICY_METHOD, NULL, NULL );
	if ( ! get_list_space( &attr_ass, DEFAULT_CHUNK_SIZE, TRUE ) )
		nim_error( 0, NULL, NULL, NULL );

	/* parse command line assignments */
	parse_args( argc, argv );

	/* make sure ATTR_LOCATION specified */
	if ( (pathname = attr_value( &attr_ass, ATTR_LOCATION )) == NULL )
		nim_error( ERR_MISSING_ATTR, ATTR_LOCATION_T, NULL, NULL );
	VERBOSE("c_stat: stat the pathname \"%s\"\n",pathname,NULL,NULL,NULL)

	/* look for ATTR_MODE; if not given, use "directory" by default */
	if ( (mode = attr_value( &attr_ass, ATTR_MODE )) == NULL )
		mode_mask = (mode_t) S_IFDIR;
	else
		mode_mask = (mode_t) strtol( mode, NULL, 0 );

	/* look for ATTR_VFSTYPE */
	if ( (vfstypes = attr_value( &attr_ass, ATTR_VFSTYPE )) == NULL )
	{
		vfstypes = nim_malloc( MAX_TMP );
		sprintf( vfstypes, "%d", MNT_JFS );
	}

	/* stat the pathname */
	if ( stat( pathname, &buf ) == -1 )
	{
		if ( errno == ENOENT )
			nim_error( ERR_ENOENT, pathname, NULL, NULL );
		else
			nim_error( ERR_ERRNO, NULL, NULL, NULL );
	}

	VERBOSE("   mode_mask    = %8o; vfstypes        = \"%s\"\n",mode_mask,
				vfstypes,NULL,NULL)
	VERBOSE("   stat.st_mode = %8o; stat.st_vfstype = %d\n",buf.st_mode,
				buf.st_vfstype,NULL,NULL)

	/* does stat.st_vfstype match any of the acceptable values? */
	ptr = strtok( vfstypes, " " );
	while ( ptr != NULL )
	{
		vfstype = (int) (*ptr - '0');
		if ( buf.st_vfstype == vfstype )
			break;

		ptr = strtok( NULL, " " );
	}
	if ( ptr == NULL )
	{
		sprintf( tmp, "%d", buf.st_vfstype );
		nim_error( ERR_VFSTYPE, pathname, tmp, vfstypes );
	}

	/* check both the "file type" and the "file user/group perms" */
	file_type = (mode_mask & S_IFMT);
	perms = (mode_mask & ~ S_IFMT);
	VERBOSE("   file_type    = %8o\n",file_type,NULL,NULL,NULL)
	VERBOSE("   perms        = %8o\n",perms,NULL,NULL,NULL)
	if ( (buf.st_mode & S_IFMT) != file_type )
	{
		sprintf( tmp, "0%o", (buf.st_mode & S_IFMT) );
		sprintf( tmp1, "0%o", file_type );
		nim_error( ERR_FILE_TYPE, pathname, tmp, tmp1 );
	}
	else if ( (perms > 0) && ((buf.st_mode & perms) == 0) )
	{
		sprintf( tmp, "0%o", (buf.st_mode & ~S_IFMT) );
		sprintf( tmp1, "0%o", perms );
		nim_error( ERR_FILE_PERMS, pathname, tmp, tmp1 );
	}

	exit( 0 );
}

