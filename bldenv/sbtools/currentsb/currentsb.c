static char sccsid[] = "@(#)19  1.2  src/bldenv/sbtools/currentsb/currentsb.c, bldprocess, bos412, GOLDA411a 9/19/94 21:19:33";
/*
 * Copyright (c) 1990, 1991, 1992  
 * Open Software Foundation, Inc. 
 *  
 * Permission is hereby granted to use, copy, modify and freely distribute 
 * the software in this file and its documentation for any purpose without 
 * fee, provided that the above copyright notice appears in all copies and 
 * that both the copyright notice and this permission notice appear in 
 * supporting documentation.  Further, provided that the name of Open 
 * Software Foundation, Inc. ("OSF") not be used in advertising or 
 * publicity pertaining to distribution of the software without prior 
 * written permission from OSF.  OSF makes no representations about the 
 * suitability of this software for any purpose.  It is provided "as is" 
 * without express or implied warranty. 
 */
/*
 * ODE 2.1.1
 */
/******************************************************************************
**                          Open Software Foundation                         **
**                              Cambridge, MA                                **
**                              Randy Barbano                                **
**                                May 1990                                   **
*******************************************************************************
**
**    Description:
**	This program prints out the information on the current sandbox
**	and current set including base dir, set dir, and backing tree.
**
**  Functions:
**    main (int, ** char) int
**      cmdline_syntax (** char, ** char) int
**      fill_data (** char, ** char, ** char, ** char, ** char, ** char) int
**      print_output (* char, * char, * char, * char, * char) int
**        cd_to_sbdir (* char) int
**    print_usage () int
**    print_revision () int
**
 */

static char * rcsid =
 "$RCSfile: currentsb.c,v $ $Revision: 1.8.2.2 $ $Date: 92/03/25 22:45:45 $";

#  include <ode/odedefs.h>

#  define  MAX_ARGS	7
#  define  SBRC_OP	"-sbrc"
#  define  INCLUDE	"include"

    UIINIT init [ ] = {                     /* initialize the user interface */
	   	{ SB_OP,     1, OVERWRITE, 0, 0, "" },
	   	{ SET_OP,    1, OVERWRITE, 0, 0, "" },
	   	{ DIR_OP,    1, OVERWRITE, 0, 0, "" },
	   	{ SETDIR_OP, 1, OVERWRITE, 0, 0, "" },
	   	{ SBRC_OP,   1, OVERWRITE, 0, 0, "" },
	   	{ BACK_OP,   1, OVERWRITE, 0, 0, "" },
	   	{ ALL_OP,    1, OVERWRITE, 0, 0, "" }
    };

    char 	* progname = "currentsb";

main ( argc, argv )

  	/* This function checks the command line arguments and makes
	   sure they are syntactically correct.  This is done using
	   the library function interface.  If this is correct, the
	   dependencies are checked.  Errors lead to usage messages.
	   If all is okay, the primary procedure is called. */

    int         argc;                /* the number of command line arugments */
    char     ** argv;                          /* strings with each argument */

{
    char        * sb = NULL,                                 /* sandbox name */
                * sbbase = NULL,                          /* path to sandbox */
                * sb_rcfile,                         /* sandbox rc file name */
                * rc_file,                                   /* user rc file */
                * setname = NULL,                        /* full name of set */
                * setdir = NULL;                    /* path of set directory */

  ui_init ( argc, argv, MAX_ARGS, init );
  cmdline_syntax ( &sb_rcfile, &rc_file );
  fill_data ( &sb, &sbbase, &sb_rcfile, &rc_file, &setname, &setdir );
  print_output ( sb, sbbase, sb_rcfile, setname, setdir );
  return ( OK );
}                                                                    /* main */



cmdline_syntax ( sb_rcfile, rc_file )

	/* This procedure checks for relationships between the
	   command line arguments.  It assumes the syntax is
	   already correct.  */

    char       ** sb_rcfile,                              /* sandbox rc file */
               ** rc_file;                                   /* user rc file */

{
  *rc_file = ui_arg_value ( RC_OP, 1, 1 );
  *sb_rcfile = ui_arg_value ( SB_RC_OP, 1, 1 );

  if ( ui_is_set ( ALL_OP )) {
    ui_load ( SB_OP, 0 );
    ui_load ( SET_OP, 0 );
    ui_load ( DIR_OP, 0 );
    ui_load ( SETDIR_OP, 0 );
    ui_load ( SBRC_OP, 0 );
    ui_load ( BACK_OP, 0 );
  } /* if */

  else if (( ! ui_is_set ( SB_OP )) &&
	   ( ! ui_is_set ( SET_OP )) &&
	   ( ! ui_is_set ( DIR_OP )) &&
	   ( ! ui_is_set ( SETDIR_OP )) &&
           ( ! ui_is_set ( SBRC_OP )) &&
	   ( ! ui_is_set ( BACK_OP ))) {
    ui_load ( SB_OP, 0 );
    ui_load ( SET_OP, 0 );
  } /* else if */
}                                                          /* cmdline syntax */



fill_data ( sb, sbbase, sb_rcfile, rc_file, setname, setdir )

	/* This procedure fills in the values of the strings
	   which then can be printed out according to the 
	   command line instructions. */

    char       ** sb,                                        /* sandbox name */
               ** sbbase,                                 /* path to sandbox */
               ** sb_rcfile,                              /* sandbox rc file */
               ** rc_file,                                   /* user rc file */
               ** setname,                                   /* full setname */
               ** setdir;                           /* path of set directory */

{
  if ( current_sb ( sb, sbbase, sb_rcfile, rc_file) == ERROR) {
    ui_print ( VWARN, "--EMPTY --EMPTY\n" );
    exit ( OK );
  } /* if */

                          /* Update to print meaningful message */ 
                          /* and allow user to continue if sets */
                          /* are not defined.                   */
  if ( current_set (  setname,  setdir,  sb, rc_file ) == ERROR ) 
    {
    ui_print ( VWARN, "Sandbox exists. No sets defined.  Proceeding.\n " );
    } /* if */

 /* code had been this prior to above update. */
 /*
  if ( current_set (  setname,  setdir,  sb, rc_file ) == ERROR ) {
    ui_print ( VWARN, "%s --EMPTY\n", sb );
    exit ( OK );
  }  if 
 */

}                                                               /* fill data */



print_output ( sb, sbbase, sb_rcfile, setname, setdir )

	/* This procedure prints out each type of info if it
	   is set.  It also does a change directory and prints
	   out the current directory to get the backing tree. */


    char        * sb,                                        /* sandbox name */
                * sbbase,                                 /* path to sandbox */
                * sb_rcfile,                              /* sandbox rc file */
                * setname,                                   /* full setname */
                * setdir;                           /* path of set directory */

{
    char        link [ PATH_LEN ],                     /* holds path to link */
                defsbrc [ PATH_LEN ],                  /* default sb rc file */
                currentdir [ PATH_LEN ];                      /* misc string */

  if ( ui_is_set ( SB_OP ))
    ui_print ( VALWAYS, "%s ", sb );

                       /* added NOSET as placeholder if no set defined.*/ 
  if ( ui_is_set ( SET_OP ))
    {
    if ( setdir != NULL )
      ui_print ( VALWAYS, "%s ", setname );
    else
      ui_print ( VALWAYS, "NOSET " );
    }


  if ( ui_is_set ( DIR_OP ))
    ui_print ( VALWAYS, "%s ", sbbase );

                       /* added NOSETDIR as placeholder if no set defined.*/ 
  if ( ui_is_set ( SETDIR_OP ))
    {
    if ( setdir != NULL )
      ui_print ( VALWAYS, "%s ", setdir );
    else
      ui_print ( VALWAYS, "NOSETDIR " );
    }

  if ( ui_is_set ( SBRC_OP ))
    ui_print ( VALWAYS, "%s ", sb_rcfile );

  if ( ui_is_set ( BACK_OP )) {
    concat ( defsbrc, PATH_LEN, sbbase, "/", sb, "/", LOCAL_RC, NULL );

    if ( streq ( sb_rcfile, defsbrc )) {
      concat ( link, PATH_LEN, sbbase, "/", sb, "/", LINK_DIR, NULL );

      if ( chdir ( link ) == ERROR )
	uquit ( ERROR, FALSE, "could not change directories to %s\n%s", link,
		"\t\t  to get backing tree.\n" );
    } /* if */

    else
      cd_to_sbdir ( sb_rcfile );

    if ( getwd ( currentdir ) == NUL )
      uquit ( ERROR, FALSE, "could not get current directories\n%s",
	      "\t\t  to get backing tree.\n" );

    ui_print ( VALWAYS, "%s ", currentdir );
  } /* if */

  ui_print ( VALWAYS, "\n" );
}                                                            /* print output */



cd_to_sbdir ( sb_rcfile )

	/*
	 *  Finds the directory indicated in the sb_rcfile and changes
	 *  to it.
	 */

    char        * sb_rcfile;                              /* sandbox rc file */

{
    FILE        * sbptr;                                     /* file to read */
    char          line [ PATH_LEN ],                          /* misc string */
                * lineptr,                         /* allows parsing of line */
                * token;                             /* piece of parsed line */
    BOOLEAN       looking = TRUE;                            /* misc boolean */

  if (( sbptr = fopen ( sb_rcfile, READ )) == NULL )
    uquit ( ERROR, FALSE, "could not open sb rc file: %s,\n%s",
	      sb_rcfile, "\t\t  to read to get backing tree.\n" );

  while ( looking && ( lineptr = fgets ( line, PATH_LEN, sbptr )) != NULL ) {
    token = nxtarg ( &lineptr, WHITESPACE );

    if ( streq ( token, INCLUDE ))
      looking = FALSE;
  } /* while */

  if ( looking )
    uquit ( ERROR, FALSE, "could find %s line in sb rc file: %s,\n%s",
	    INCLUDE, sb_rcfile, "\t\t  to read to get backing tree.\n" );

  if (( token = rindex ( sb_rcfile, SLASH )) != NULL )
    *token = NUL; 

  if ( chdir ( sb_rcfile ) == ERROR )
    uquit ( ERROR, FALSE, "could not change directories to %s\n%s", sb_rcfile,
	    "\t\t  to get backing tree.\n" );

  if (( token = rindex ( lineptr, SLASH )) != NULL ) {
    *token = NUL; 

    if (( token = rindex ( lineptr, SLASH )) != NULL )
      *token = NUL; 
  } /* if */

  if ( chdir ( lineptr ) == ERROR )
    uquit ( ERROR, FALSE, "could not change directories to %s\n%s", lineptr,
	    "\t\t  to get backing tree.\n" );
}                                                             /* cd to sbdir */



print_usage ()

	/* This procedure prints the usages for currentsb. */

{
  printf ( "USAGE:\n" );
  printf ( "%s [-sb -set -dir -setdir -back -sbrc] [-all]\n", progname );
  printf ( "\t  -sb : list current sandbox\n" );
  printf ( "\t  -set : list current set\n" );
  printf ( "\t  -dir : list base directory of current sandbox\n" );
  printf ( "\t  -setdir : list default set directory of current set\n" );
  printf ( "\t  -sbrc : list sandbox rc file for current directory\n" );
  printf ( "\t  -back : list backing tree of current sandbox\n" );
  printf ( "\t  -all : list all the current information\n" );
  printf ( "\t    defaults to -sb and -set\n" );
  printf ( "\t    prints --EMPTY if no value was found for sandbox or set\n" );
  printf ( "\t  sb_opts:\n" );
  printf ( "\t    -rc <user rc file>, -sb_rc <sb rc file>\n" );
  printf ( "%s -usage | -version\n", progname );
}                                                             /* print usage */
