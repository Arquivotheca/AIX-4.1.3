static char sccsid[] = "@(#)42  1.1  src/bldenv/sbtools/libsb/interface.c, bldprocess, bos412, GOLDA411a 4/29/93 12:21:33";
/*
 *  Copyright (c) 1990, 1991, 1992  
 *  Open Software Foundation, Inc. 
 *   
 *  Permission is hereby granted to use, copy, modify and freely distribute 
 *  the software in this file and its documentation for any purpose without 
 *  fee, provided that the above copyright notice appears in all copies and 
 *  that both the copyright notice and this permission notice appear in 
 *  supporting documentation.  Further, provided that the name of Open 
 *  Software Foundation, Inc. ("OSF") not be used in advertising or 
 *  publicity pertaining to distribution of the software without prior 
 *  written permission from OSF.  OSF makes no representations about the 
 *  suitability of this software for any purpose.  It is provided "as is" 
 *  without express or implied warranty. 
 */
/*
 * ODE 2.1.1
 */
/******************************************************************************
**                          Open Software Foundation                         **
**                              Cambridge, MA                                **
**                Randy Barbano, Damon Poole, Martha Hester                  **
**                              December 1990                                **
*******************************************************************************
**
**  Description:  These functions provide a common user interface for the
**	ODE tools.  They parse the command line, provide an interface to
**	the information, and provide a common way of printing out messages.
**
**  functions:
**    ui_init (int, ** char, int, * struct) int
**      ui_set_progname (* char) int
**      init_htable ( int, ** char) int
**        hvalue (* char) unsigned
**        init_one (* struct, * char, int) void
**      load_htable (int, * struct) void
**        test_conflicts (* struct) void
**        match_pattern (* char) * struct
**          hvalue 
**        hvalue 
**      rc_file_options (int, ** char, * char, * int, *** char) BOOLEAN
**        get_local_rcfile (* char, * char, * char) BOOLEAN
**        ui_print (int, * char, va_list) void
**        get_dotsbx_options (* char, * char, * int, *** char) BOOLEAN
**          ui_print 
**          get_options (* struct, * char) * char
**          next_token (** char, * char) BOOLEAN
**            skip_white_space (** char) void
**            ui_print 
**      load_options (int, ** char, int, * struct) int
**        built_ins (* char) * char
**        special_case (* char) BOOLEAN
**          ui_unset (* char) int
**            match_pattern 
**          ui_print 
**        entry_pattern (* char, * struct, int) * char
**        match_pattern 
**        load_entry (* struct, * struct, ** char, * char, int, * int) void
**          load_min_args (* struct, ** char, ** struct, int, * int, * int) void
**            match_arg_list (* char, * char) BOOLEAN
**            in_arg_list (* struct, * char) BOOLEAN
**          load_max_args (* struct, ** char, ** struct, int, * int, * int) void
**            match_arg_list 
**            in_arg_list 
**        ui_unset 
**        set_duplicates (* struct, ** struct, * char) void
**          match_entry (* struct, * char, ** struct) void
**      print_htable () void
**        ui_print 
**        print_internals (* struct, * struct, * struct, int) void
**          ui_print 
**      ui_restore_progname () int
**    ui_load (* char, int, va_list) int
**      built_ins 
**      special_case 
**      match_pattern 
**      ui_print 
**      load_entry 
**      ui_unset 
**      set_duplicates 
**    ui_is_set (* char) BOOLEAN
**      match_pattern 
**      ui_print 
**    ui_entry_cnt (* char) int
**      match_pattern 
**      ui_print 
**    ui_arg_cnt (* char, int) int
**      ui_print 
**      match_pattern 
**    ui_entry_value (* char, int) * char
**      ui_print 
**      match_pattern 
**    ui_arg_value (* char, int, int) * char
**      ui_print 
**      match_pattern 
**      arg_value (* struct, int, int) * char
**        ui_print 
**    ui_entries_to_argv (* char, *** char) int
**      match_pattern 
**      ui_print 
**    ui_args_to_argv (* char, int, *** char) int
**      ui_print 
**      match_pattern 
**    ui_ver_switch () * char
**      ui_print 
**    ui_is_auto () BOOLEAN
**    ui_is_info () BOOLEAN
**    ui_ver_level () int
**
*/

#  include  "ui.h"


int	ui_init ( argc, argv, initct, init )

	/*
     	 * Initialize ui with the argv passed in as well as any switches
	 * appearing in the rc file. This procedure must be called before
	 * any of the other library procedures are called. This replaces
	 * the old call to parse_cmd_line.
     	 * Returns ERROR if not successful, OK otherwise.
     	 */

    int         argc;                /* the number of command line arugments */
    char     ** argv;                          /* strings with each argument */
    int         initct;                      /* count of lines to initialize */
    UIINIT    * init;                          /* initialization information */

{
    char     ** arglist;                        /* list of rc file arguments */
    int         argct;                                  /* rc file arg count */

  from_ui_load = FALSE;
  ui_set_progname ( "ui_init" );
  init_htable ( argc, argv );
  load_htable ( initct, init );

  if ( rc_file_options ( argc, argv, &argct, &arglist ))
    load_options ( argct, arglist, initct, init );

  load_options ( argc - 1, argv + 1, initct, init );
  print_htable ();
  ui_restore_progname ();
  return ( OK );
}                                                                 /* ui init */



ui_set_progname ( newname )

	/*
	 * Put the newname into progname after having saved the
	 * initial value.
	 */

    char        * newname;                           /* new name of progname */

{
  save_progname = progname;
  progname = newname;
}                                                         /* ui set_progname */



ui_restore_progname ()

	/*
	 * Puts previous progname back into progname.
	 */

{
  progname = save_progname;
}                                                     /* ui restore progname */



static	init_htable ( argc, argv )

	/*
     	 * Initializes the entire hash table with default values.
	 * Also loads the know options.
	 */

    int         argc;                /* the number of command line arugments */
    char     ** argv;                          /* strings with each argument */

{
    HTABLE      * hptr;                               /* points to one entry */
    int           ct;                                        /* misc counter */

  auto_pos = hvalue ( AUTO_OP );
  info_pos = hvalue ( INFO_OP );

  if (( argc > 1 ) && streq ( argv[1], DEBUG_OP ))
    verbosity = VDEBUG;    	       /* a way to get inside interface info */
  else if (( argc > 1 ) && streq ( argv[1], VERBOSE_OP ))
    verbosity = VDETAIL;
  else 
    verbosity = VNORMAL;

  if (( htbl = (HTABLE *) malloc (sizeof ( HTABLE ) * HTABLESIZE)) == NULL )
    uquit ( ERROR, FALSE, "sys: cannot create space for user interface.\n" );

  for ( ct = 0; ct < HTABLESIZE; ct++ ) {
    hptr = htbl + ct;
    hptr->pattern = salloc ( EMPTY_STRING );
    hptr->entry = NULL;
    hptr->arg_list = NULL;
    hptr->entry_list = NULL;
    hptr->next_pattern = NULL;
    hptr->num_entries = hptr->num_args = 0;
  } /* for */

  init_one ( htbl + auto_pos, AUTO_OP, 0 );
  init_one ( htbl + info_pos, INFO_OP, 0 );
  init_one ( htbl + hvalue ( RC_OP), RC_OP, 1 );
  init_one ( htbl + hvalue ( SB_RC_OP), SB_RC_OP, 1 );
}                                                             /* init htable */



static	unsigned hvalue ( string )

	/*
	 * Calculates the hash value of the string.
	 */

    char        * string;                          /* string to get value of */

{
    unsigned      hashval;                               /* cumulative value */

  for ( hashval = 0; *string != NUL; string++ )
    hashval = *string + 31 * hashval;

  return ( hashval % HTABLESIZE );
}                                                                  /* hvalue */



static	void init_one ( hptr, pattern, argct )

	/*
	 * Initializes a single hash table entry.
	 */

    HTABLE      * hptr;                               /* points to one entry */
    char        * pattern;                               /* pattern to match */
    int           argct;                           /* args allowed or needed */

{
  hptr->pattern = salloc ( pattern );
  hptr->duplicates = OVERWRITE;
  hptr->max_entries = 1;
  hptr->min_args = hptr->max_args = argct;

  if ( argct == 0 )
    hptr->legal_args = salloc ( EMPTY_STRING );
  else
    hptr->legal_args = salloc ( ARGS_OP );
}                                                                /* init one */



static	void load_htable ( initct, init )

	/*
	 * loads the initial values into the hash table.  The
	 * build-in values are already loaded.
	 */

    int           initct;                    /* count of lines to initialize */
    UIINIT      * init;                        /* initialization information */

{
    HTABLE      * hptr;                               /* points to one entry */
    UIINIT      * iptr;              /* points to initialization information */
    int           ct;                                        /* misc counter */

  for ( ct = 0; ct < initct; ct++ ) {
    iptr = init + ct;

    test_conflicts ( iptr );

    if (( hptr = match_pattern ( iptr->pattern )) != NULL )
      uquit ( ERROR, FALSE, 
	      "internal error: program has duplicate patterns: %s.\n",
	       iptr->pattern );

    hptr = htbl + hvalue ( iptr->pattern );

    if ( ! streq ( hptr->pattern, EMPTY_STRING )) {
      while ( hptr->next_pattern != NULL )
	hptr = hptr->next_pattern;
      
      hptr->next_pattern = ( HTABLE * ) malloc ( sizeof ( HTABLE ));
      hptr = hptr->next_pattern;
      hptr->entry = NULL;
      hptr->entry_list = NULL;
      hptr->arg_list = NULL;
      hptr->next_pattern = NULL;
      hptr->num_entries = hptr->num_args = 0;
    } /* if */

    hptr->pattern = iptr->pattern;
    hptr->duplicates = iptr->duplicates;
    hptr->max_entries = iptr->max_entries;
    hptr->min_args = iptr->min_args;
    hptr->max_args = iptr->max_args;
    hptr->legal_args = iptr->legal_args;
  } /* for */
}                                                             /* load htable */



static	void test_conflicts ( iptr )

	/*
	 * Tests user entered intialization array to make sure it is
	 * legal.
	 */

    UIINIT      * iptr;              /* points to initialization information */

{
  if ( *(iptr->pattern) == NUL )
    uquit ( ERROR, FALSE, "internal error: UIINIT pattern set to NULL.\n" );

  if ( iptr->max_entries < 1 && iptr->max_entries != UNLIMITED )
    uquit ( ERROR, FALSE, 
            "internal error: UIINIT %s max_enties < 1.\n", iptr->pattern );

  if ( iptr->duplicates < TOGGLE || iptr->duplicates > ACCUM )
    uquit ( ERROR, FALSE, 
       "internal error: UIINIT %s duplicates out of range.\n", iptr->pattern );

  if ( iptr->min_args < 0 )
    uquit ( ERROR, FALSE, 
            "internal error: UIINIT %s min_args < 0.\n", iptr->pattern );

  if ( iptr->max_args < UNLIMITED )
    uquit ( ERROR, FALSE, 
            "internal error: UIINIT %s max_args < 0.\n", iptr->pattern );

  if ( iptr->min_args > iptr->max_args && iptr-> max_args != UNLIMITED )
    uquit ( ERROR, FALSE, 
           "internal error: UIINIT %s max_args < min_args.\n", iptr->pattern );

  if ( iptr->legal_args == NULL )
    uquit ( ERROR, FALSE,
        "internal error: UIINIT %s legal_args set to NULL.\n", iptr->pattern );

  if ( iptr->max_args == 0 && iptr->duplicates == ACCUM )
    uquit ( ERROR, FALSE, 
            "internal error: UIINIT %s max_args = 0 && duplicates = ACCUM.\n",
             iptr->pattern );

  if ( iptr->max_entries != 1 && iptr->duplicates == TOGGLE )
    uquit ( ERROR, FALSE, 
        "internal error: UIINIT %s max_entires != 1 && duplicates = TOGGLE.\n",
         iptr->pattern );

  if ( iptr->max_entries > 1 && *(iptr->pattern) != '!' &&
      ( strpbrk ( iptr->pattern, "*?[" ) == NULL ))
    uquit ( ERROR, FALSE, 
        "internal error: UIINIT unique pattern %s allows max_entires > 1.\n",
         iptr->pattern );
}                                                          /* test conflicts */



static HTABLE *  match_pattern ( pattern )

	/*
	 * Finds the match to the pattern in the hash table.
	 * Returns a pointer to the table entry; NULL if none found.
	 */

    char        * pattern;                      /* pattern to match in table */

{
    HTABLE      * hptr;                               /* points to one entry */

  hptr = htbl + hvalue ( pattern );

  while ( hptr != NULL ) {
    if ( streq ( hptr->pattern, pattern ))
      break;
    hptr = hptr->next_pattern;
  } /* while */

  return ( hptr );
}                                                           /* match pattern */



static BOOLEAN	rc_file_options ( argc, argv, rc_argc, rc_argv )

	/*
 	 * Determine the rc file to parse for default options.
 	 */

    int           argc;              /* the number of command line arugments */
    char       ** argv;                            /* command line arguments */
    int         * rc_argc;                  /* the number of rc file options */
    char      *** rc_argv;                     /* strings with each argument */

{
    char       ** argv_ptr,                                 /* agrv traveler */
                * rc_file_name,                           /* name of rc file */
                * nameonly;                  /* in case more than name given */
    int           argn;                                      /* misc counter */
    BOOLEAN       alt_rc = FALSE;                            /* misc boolean */

  	/* Check to see if a different .sandboxrc file should be used.  */
  argv_ptr = argv + 1;

  if ( ( nameonly = strrchr ( *argv, SLASH )) != NULL )
    nameonly++;
  else
    nameonly = *argv;

  for ( argn = 1; argn < argc; argn++ ) {
    if ( streq ( *argv_ptr, RC_OP )) {
      alt_rc = TRUE;
      break;
    } /* if */

    argv_ptr++;
  } /* end for */

  if ( alt_rc ) {
    if ( access ( *(++argv_ptr), R_OK ) == OK )
      rc_file_name = *argv_ptr;

    else if ( ! get_local_rcfile ( *argv_ptr, &rc_file_name, EMPTY_STRING )) {
      ui_print ( VDEBUG, "Could not find rc file : %s\n", *argv_ptr );
      return ( FALSE );
    } /* else if */
  } /* if */

  else if ( ! get_local_rcfile ( nameonly, &rc_file_name, RCEXT )) {
    if ( ! get_default_usr_rcfile ( &rc_file_name, FALSE ))
      return ( FALSE );
  } /* else if */

  ui_print ( VDETAIL, "Reading rc file : %s\n", rc_file_name );
  return ( get_dotsbx_options ( rc_file_name, nameonly, rc_argc, rc_argv ));
}                                                         /* rc file options */



static BOOLEAN get_local_rcfile ( cname, rcfile, ext )

	/*
	 * Builds path and searches for existence of rc file with same
	 * name as program calling routine.
	 * Returns TRUE if local name is found; FALSE otherwise.
	 */

    char        * cname,                                     /* command name */
               ** rcfile,                     /* string to hold name of file */
                * ext;                      /* extension to end of file name */
{
    char          trcfile [ PATH_LEN ],  /* temporary holder */
		* env_input;                     /* holds values from getenv */

  if (( env_input = getenv ( "HOME" )) == NULL )
    return ( FALSE );

  if ( streq ( ext, EMPTY_STRING ))
    concat ( trcfile, PATH_LEN, env_input, "/", cname, ext, NULL );
  else
    concat ( trcfile, PATH_LEN, env_input, "/.", cname, ext, NULL );

  if ( access ( trcfile, R_OK ) != OK ) {
    *rcfile = NULL;
    return ( FALSE );
  } /* if */

  *rcfile = salloc ( trcfile );
  return ( TRUE );
}                                                        /* get local rcfile */



void 	ui_print ( va_alist )

	/*
	 * This procedure takes the in va_alist and parses out the
	 * pieces it needs then passes the rest to a regular print
	 * routine.  It provides a standard printing routine for
	 * the ODE commands.
	 */

    va_dcl

{
                va_list   arglist;               /* variable length arg list */
                int       msg_verbosity;         /* print on verbosity level */
                char    * msg_format;                   /* printf formatting */

    static      char    * outfmt[] = {                  /* leader for output */
      "",                                                         /* VALWAYS */
      ">> FATAL ERROR",                                            /* VFATAL */
      "",                                                          /* VQUIET */
      ">> WARNING",                                                 /* VWARN */
      "",                                                         /* VNORMAL */
      ">> DIAGNOSTIC",                                              /* VDIAG */
      ">  ",                                                      /* VDETAIL */
      ">> DEBUG INFO"                                              /* VDEBUG */
    };

    static      FILE    * outfile[] = {               /* placement of output */
      stdout,                                                     /* VALWAYS */
      stderr,                                                      /* VFATAL */
      stdout,                                                      /* VQUIET */
      stderr,                                                       /* VWARN */
      stdout,                                                     /* VNORMAL */
      stderr,                                                       /* VDIAG */
      stdout,                                                     /* VDETAIL */
      stderr                                                       /* VDEBUG */
    };

    static      int       last_ver = VNORMAL;      /* former verbosity level */

  fflush ( stdout );
  va_start ( arglist );
  msg_verbosity = va_arg ( arglist, int );

  if (( msg_verbosity <= verbosity ) || 
      ( msg_verbosity == VCONT && last_ver <= verbosity )) {
    msg_format  = va_arg ( arglist, char * );

    if ( msg_verbosity == VFATAL  || msg_verbosity == VWARN ||
	 msg_verbosity == VDIAG || msg_verbosity == VDEBUG )
      fprintf ( outfile[msg_verbosity], "%s in %s:\n>> ",
		outfmt[msg_verbosity], progname );
   
    if ( msg_verbosity == VDETAIL )
      fprintf ( outfile[msg_verbosity], "%s", outfmt[msg_verbosity] );

    if ( msg_verbosity == VCONT ) {
      if ( last_ver == VFATAL  || last_ver == VWARN ||
	   last_ver == VDIAG || last_ver == VDEBUG )
	fprintf ( outfile[last_ver], ">> " );
      else if ( last_ver == VDETAIL )
	fprintf ( outfile[last_ver], ">  " );

      vfprintf ( outfile[last_ver], msg_format, arglist );
    } /* if */

    else
      vfprintf ( outfile[msg_verbosity], msg_format, arglist );
  } /* if */

  if ( msg_verbosity != VCONT )
    last_ver = msg_verbosity;

  va_end ( arglist );
}



static 	BOOLEAN  get_dotsbx_options ( rc_file_name, command, rcargc, rcargv )

	/*
 	 * Reads the options from the .sandboxrc file
	 * Returns TRUE if file was read.
 	 */

    char        * rc_file_name;                  /* name of rc file to parse */
    char        * command;                    /* name of command to look for */
    int         * rcargc;                                    /* option count */
    char      *** rcargv;                                 /* list of options */

{
    FILE        * rcfile;                                  /* ptr to rc file */
    char        * options,               /* list of options found in sandbox */
                  token [ TOKEN_LENGTH ];               /* holds each option */
    int           alloc_arg = 10,                     /* available arguments */
                  ct;                                        /* misc counter */

  if (( rcfile = fopen ( rc_file_name, READ )) == NULL ) {
    ui_print ( VWARN,  "could not open %s.\n", rc_file_name );
    return ( FALSE );
  } /* if */
 
  options = get_options ( rcfile, command );

  if ( options == NULL || *options == NUL )
    return ( FALSE );              /* return immediately if options is empty */

  if (( *rcargv = (char **) calloc ( 10, sizeof (char *))) == NULL ) {
    ui_print ( VWARN,  "could not calloc space for rc file options.\n" );
    return ( FALSE );
  } /* if */

  *rcargc = 0;            /* Allocate 10 args, realloc later if more needed. */

  do {
    if ( ! next_token ( &options, token ))
      return ( FALSE );

    if ( *token == NUL || *token == SPACE )
      break;

    rm_newline ( token );

    if ( *rcargc >= alloc_arg ) {
      alloc_arg += 10;

      if (( *rcargv = (char **) realloc
		      ( *rcargv, sizeof (char *) * alloc_arg )) == NULL )
	return ( FALSE );
    } /* if */

    if (( (*rcargv)[*rcargc] = salloc ( token )) == NULL )
      return ( FALSE );

    (*rcargc)++;
  } while ( TRUE );

  ui_print ( VDEBUG, "Tokens are:\n" );
 
  for ( ct = 0; ct < *rcargc; ct++ )
    ui_print ( VCONT, "%s\n", (*rcargv)[ct] );

  fclose ( rcfile );
  return ( TRUE );
}                                                      /* get dotsbx options */



static char 	* get_options ( rc_file, command )

	/*
 	 * Generic procedure to find the options associated with a
	 * command in a given file.
 	 */

    FILE        * rc_file;                              /* file to read from */
    char        * command;                          /* command to search for */

{
    char        * ptr,                          /* misc pointer to each line */
		* field,
                  buf [ PATH_LEN ],                           /* misc string */
		  options [ PATH_LEN ],
    		  options2 [ PATH_LEN ];

  *options = NUL;

  while ( fgets ( buf, PATH_LEN, rc_file ) != NULL ) {
    if (( ptr = index ( buf, '#' )) != NULL )          /* Check for comment. */
      *ptr = NUL;

    ptr = buf;
    field = nxtarg ( &ptr, WHITESPACE ); 

    if ( *field == NUL )
      continue;

   	/* For every line which matches the desired command,
    	 * add the list of options to the end of the options string.  */
    if ( gmatch ( command, field )) {
      concat ( options2, PATH_LEN, options, " ", ptr, NULL );
      strcpy ( options, options2 );
    } /* if */
  } /* while */

  ptr = options + strlen ( options ) - 1;
  return ( salloc ( options ));
}                                                             /* get options */



static BOOLEAN	next_token ( str, token )

	/* 
	 * This function extracts the next token from the string.
	 * Returns TRUE if there is not an illegal token.
	 */

  char         ** str;                                    /* string to parse */
  char            token[];                                  /* token to fill */

{
  char 		* env;
  int             ct,                                        /* misc counter */
      		  expand,
		  braces;

  if ( *str != NULL )
    skip_white_space ( str );

  if ( *str == NULL || **str == NUL || **str == NEWLINE ) {
    *token = NUL;
    return ( TRUE );
  } /* if */

 	/* If a '$' is encountered, the remainder of the token is an
  	 *  environment variable and needs to be expanded. Strip off
  	 *  the '$' and record the fact that it needs to be expanded.  */
  expand = FALSE;
  braces = FALSE;

  if ( **str == '$' ) {
    expand = TRUE;
    (*str)++;

    if ( **str == '{' ) {
      braces = TRUE;
      (*str)++;
    } /* if */
  } /* if */

 	/* A token is either all of the characters within quotes or all of
  	 *  the characters between two areas of whitespace.  */
  ct = 0;

  if ( **str == '"' ) {
    (*str)++;

    while ( **str != NUL && **str != '"' ) {
      if ( ct >= TOKEN_LENGTH ) {
        ui_print (  VWARN, "token too long: %s.\n", *str );
        return ( FALSE );
      } /* if */

      token[ct++] = *(*str)++;
    } /* while */

    if ( **str == '"' )
      (*str)++;
  } /* if */
  
  else if ( braces ) {
    (*str)++;

    while ( **str != NUL && **str != '}' ) {
      if( ct >= TOKEN_LENGTH ) {
        ui_print (  VWARN, "token too long: %s.\n", *str );
        return ( FALSE );
      } /* if */

      token[ct++] = *(*str)++;
    } /* while */

    if(**str == '}')
      (*str)++;
  } /* else if */
  
  else {
    while ( ! WHITESPACEFN ( *str ) && **str != NUL ) {
      if( ct >= TOKEN_LENGTH ) {
        ui_print (  VWARN, "token too long: %s.\n", *str );
        return ( FALSE );
      } /* if */

      token[ct++] = *(*str)++;
    } /* while */
  } /* else */

  if ( ct < TOKEN_LENGTH )
    token[ct] = NUL;

 	/* If a '$' was encountered earlier, expand the token.  */
  if ( expand ) {
    env = getenv ( token );
    if ( strlen ( env ) > TOKEN_LENGTH ) {
      ui_print ( VWARN, "Environment variable too long: %s\n", env );
      return ( FALSE );
    } /* if */

    else
      strcpy ( token, env );

    if ( token == NULL )
      token = salloc ( EMPTY_STRING );
  } /* endif */

  return ( TRUE );
}                                                              /* next token */



static 	void skip_white_space ( str )

	/* advance to none white space */

    char       ** str;

{
  while ( WHITESPACEFN (*str) && **str != NUL )
    (*str)++;
}                                                        /* skip white space */



static	int load_options ( usrct, usr_list, initct, init )

	/*
 	 * Enters the arg_list into the ui information table.  First
	 * the build in's and special cases are checked, then the user
	 * entries.  Then load them according to type: first time,
	 * toggles, repeats of overwrite and accums.
	 * Returns ERROR if there is a failure; OK otherwise.
 	 */

    int           usrct;                     /* number of arguments to parse */
    char       ** usr_list;                             /* list of arguments */
    int           initct;                    /* count of lines to initialize */
    UIINIT      * init;                        /* initialization information */

{
    HTABLE      * hptr;                       /* points to one table pattern */
    ENTRY_SET   * eptr = NULL;                            /* points to entry */
    char        * opt,                                   /* points to option */
                * pattern;                        /* pattern to match option */
    int           ct = 0;                  /* count of arg list being parsed */

  while ( ct < usrct ) {
    opt = *(usr_list + ct);

    if (( pattern = built_ins ( opt )) == NULL ) {
      if ( special_case ( opt )) { /* try build in, special case before user */
	ct++;
	continue;
      } /* if */

      if  (( pattern = entry_pattern ( opt, init, initct )) == NULL )
	uiquit ( ERROR, TRUE, "No such option: %s.\n", opt );
    } /* if */
      
    if (( hptr = match_pattern ( pattern )) == NULL )
      uquit ( ERROR, FALSE, "Program error in ui function load_options.\n" );

    if ( hptr->num_entries == 0 )
      load_entry ( hptr, NULL, usr_list, opt, usrct, &ct );

    else if ( hptr->duplicates == TOGGLE )
      ui_unset ( pattern );

    else {
      set_duplicates ( hptr, &eptr, opt );
      load_entry ( hptr, eptr, usr_list, opt, usrct, &ct );
    } /* else */

    ct++;
  } /* while */
}                                                            /* load options */



static char *	built_ins ( opt )

	/*
	 * Checks to see if the opt matches one of the built-in
	 * patterns.
	 * Returns pointer to pattern if found; else NULL.
	 */

    char        * opt;                                    /* option to check */

{
  if ( streq ( opt, AUTO_OP ))
    return ( salloc ( AUTO_OP ));

  if ( streq ( opt, INFO_OP ))
    return ( salloc ( INFO_OP ));

  if ( streq ( opt, RC_OP ))
    return ( salloc ( RC_OP ));

  if ( streq ( opt, SB_RC_OP ))
    return ( salloc ( SB_RC_OP ));

  return ( NULL );
}                                                               /* built ins */



static char *	entry_pattern ( entry, patlist, patct )

	/*
	 * Finds pattern to match entry in pattern list.  Purpose is to
	 * search patlist in same order as program provided it.
	 * Returns pointer to pattern if found; else NULL.
	 */

    char        * entry;                                   /* entry to match */
    UIINIT      * patlist;                 /* pattern list to search through */
    int           patct;                       /* number of patterns in list */

{
    UIINIT      * pptr;                            /* points to pattern list */
    int           ct;                                        /* misc counter */

  for ( ct = 0; ct < patct; ct++ ) {
    pptr = patlist + ct;

    if ( gmatch ( entry, pptr->pattern ))
      return ( pptr->pattern );
  } /* for */

  return ( NULL );
}                                                           /* entry pattern */



static	BOOLEAN special_case ( opt )

	/*
	 * Checks option against internal, special values.  If there
	 * is a match, it carries out the appropriate action.
	 * Returns TRUE if opt matches a special case.
	 */

    char        * opt;                                    /* option to check */

{
  if ( streq ( opt, QUIET_OP ))
    verbosity = VQUIET;
  else if ( streq ( opt, NORMAL_OP ))
    verbosity = VNORMAL;
  else if ( streq ( opt, VERBOSE_OP ))
    verbosity = VDETAIL;
  else if ( streq ( opt, DEBUG_OP ))
    verbosity = VDEBUG;

  else if ( streq ( opt, NOAUTO_OP )) {
    if ( ui_unset ( AUTO_OP ) != OK )
      ui_print ( VWARN, "Error in program; unable to unset %s.\n", AUTO_OP );
  } /* else if */

  else if ( streq ( opt, USAGE_OP )) {
    ui_restore_progname ();
    print_usage ();
    exit ( OK );
  } /* else if */

  else if ( streq ( opt, REV_OP ) || streq ( opt, VERSION_OP ) ) {
    ui_restore_progname ();
    ui_print_revision ();
    exit ( OK );
  } /* else if */

  else
    return ( FALSE );

  return ( TRUE );
}                                                            /* special case */



int set_ver ( opt )

	/*
	 * Allows setting of verbosity.  This is a temporary function.
	 * Returns OK if verbosity level is legal, else ERROR.
	 */

    char        * opt;                                    /* option to check */

{
  if ( streq ( opt, QUIET_OP ))
    verbosity = VQUIET;
  else if ( streq ( opt, NORMAL_OP ))
    verbosity = VNORMAL;
  else if ( streq ( opt, VERBOSE_OP ))
    verbosity = VDETAIL;
  else if ( streq ( opt, DEBUG_OP ))
    verbosity = VDEBUG;
  else
    return ( ERROR );

  return ( OK );
}  								  /* set ver */



int	ui_unset ( pattern )

	/*
	 * Unsets the ui information for pattern.  Sets the pattern
	 * back to its initialized state.
	 * Returns ERROR if there is no such pattern; OK otherwise.
 	 */

    char      * pattern;                        /* pattern to match in table */

{
    HTABLE      * hptr;                       /* points to one table pattern */
  
  if (( hptr = match_pattern ( pattern )) == NULL )
    return ( ERROR );

  hptr->entry = NULL;
  hptr->arg_list = NULL;
  hptr->entry_list = NULL;
  hptr->next_pattern = NULL;
  hptr->num_entries = hptr->num_args = 0;
  return ( OK );
}                                                                /* ui unset */



static 	int load_entry ( hptr, eptr, usrlist, entry, usrct, pct )

	/*
	 * Puts entry into place indicated by hptr and eptr.
	 */

    HTABLE      * hptr;                   /* points to table pattern to fill */
    ENTRY_SET   * eptr;                           /* points to entry to load */
    char       ** usrlist;                /* list of user supplied arguments */
    char        * entry;                    /* points to user supplied entry */
    int           usrct,                /* number of user supplied arguments */
                * pct;                      /* pointer to count of user list */

{
  (hptr->num_entries)++;

  if ( hptr->num_entries > hptr->max_entries &&
       hptr->max_entries != UNLIMITED ) {
    uiquit ( ERROR, TRUE, "More than %d entr%s for option: %s.\n",
      hptr->max_entries, hptr->max_entries > 1 ? "ies" : "y", hptr->pattern );
    return ( ERROR );
  } /* if */

  if ( eptr == NULL ) {
    hptr->entry = entry;

    if ( load_min_args ( hptr, usrlist, &(hptr->arg_list), usrct,
		         &(hptr->num_args), pct ) == ERROR )
      return ( ERROR );

    load_max_args ( hptr, usrlist, &(hptr->arg_list), usrct,
		    &(hptr->num_args), pct );
  } /* if */

  else {
    eptr->entry = entry;

    if ( load_min_args ( hptr, usrlist, &(eptr->arg_list), usrct,
		         &(eptr->num_args), pct ) == ERROR )
      return ( ERROR );

    load_max_args ( hptr, usrlist, &(eptr->arg_list), usrct,
		    &(eptr->num_args), pct );
      return ( ERROR );
  } /* else */

  return ( OK );
}                                                              /* load entry */



uiquit ( va_alist )

	/* This procedure takes a variable length argument list and
	   prints out the name of the function that failed, the error
	   message, usage if asked for, and then, if it is not a from
	   ui_load error, exits with the code entered.  This program
	   is actually uquit with a check for ui_load added. */

va_dcl

{
    va_list     args;           /* see vprintf(3) and varargs(5) for details */
    int         status,                               /* status to exit with */
                usage;                                 /* do you print usage */
    char      * fmt;                                        /* format string */

  fflush ( stdout );
  va_start ( args );
  status = va_arg ( args, int );         /* gets the first argument and type */
  usage = va_arg ( args, int );         /* gets the second argument and type */
  fmt = va_arg ( args, char * );

  if ( from_ui_load )
    fprintf ( stderr, "ERROR: " );

  else {
    ui_restore_progname ();
    fprintf ( stderr, ">> FATAL ERROR in %s:\n   ", progname );
  } /* if */

  vfprintf ( stderr, fmt, args );                 /* print out error message */

  if ( usage )
    print_usage ();

  va_end ( args );

  if ( ! from_ui_load )
    exit ( status );
}                                                                  /* uiquit */



static	int load_min_args ( hptr, usrlist, arglist, usrct, argct, pcount )

	/* 
	 * This function attempts to load the minimum number of required
	 * arguments into the entry.
	 */

    HTABLE      * hptr;                       /* points to one table pattern */
    char       ** usrlist;                /* list of user supplied arguments */
    ARG_SET    ** arglist;                     /* place to put argument list */
    int           usrct,                     /* the number of user arugments */
                * argct,             /* running total of arguments for entry */
                * pcount;                          /* counts current usrlist */

{
    ARG_SET     * argptr;                        /* points to additonal args */

  if ( *arglist != NULL ) {
    argptr = *arglist;

    while ( argptr->next_arg != NULL )     /* bring argptr to end of arglist */
      argptr = argptr->next_arg;
  } /* if */

  while ( *argct < hptr->min_args ) {               /* get min number fields */
    if ( *pcount + 1 < usrct )
      (*pcount)++;

    else {
      uiquit ( ERROR, TRUE, "option %s requires %d argument%s.\n",
              hptr->pattern, hptr->min_args, hptr->min_args == 1 ? "" : "s" );
      return ( ERROR );
    } /* else */

    if ( ! match_arg_list ( *(usrlist + *pcount), hptr->legal_args )) {
      uiquit ( ERROR, TRUE,
	"argument: %s, not in %s's list of legal arguments.  List is:\n\t%s\n",
         *(usrlist + *pcount), hptr->pattern, hptr->legal_args );
      return ( ERROR );
    } /* if */

    if ( in_arg_list ( *arglist, *(usrlist + *pcount)))
      continue;

    if ( *arglist == NULL ) {
      *arglist = ( ARG_SET * ) malloc ( sizeof ( ARG_SET ));
      argptr = *arglist;
    } /* if */

    else {
      argptr->next_arg = ( ARG_SET * ) malloc ( sizeof ( ARG_SET ));
      argptr = argptr->next_arg;
    } /* else */

    argptr->arg = *(usrlist + *pcount);
    argptr->next_arg = NULL;
    (*argct)++;
  } /* while */

  return ( OK );
}                                                           /* load min args */



static	BOOLEAN match_arg_list ( arg, list )

	/*
	 * Tests the argument to see if it is in the legal list.
	 * Returns TRUE if it is; FALSE otherwise.
	 */

    char        * arg,                                  /* argument to check */
                * list;             /* space separated list to check against */

{
    char          copy [ PATH_LEN ],            /* don't touch original list */
                * listptr,                         /* points to copy of list */
                * token;                       /* holds each match from list */

  strcpy ( copy, list );
  listptr = copy;

  while ( (*(token = nxtarg ( &listptr, WHITESPACE ))) != NUL ) {
    if ( gmatch ( arg, token ))
      return ( TRUE );
  } /* while */

  return ( FALSE );
}                                                          /* match arg list */



static	BOOLEAN in_arg_list ( list, arg )

	/*
	 * Checks to see if arg is already in list.
	 * Returns TRUE if arg is in list.
	 */

    ARG_SET     * list;                             /* list of args to check */
    char        * arg;                              /* argument to check for */

{
  while ( list != NULL ) {
    if ( streq ( list->arg, arg ))
      return ( TRUE );
    list = list->next_arg;
  } /* while */

  return ( FALSE );
}                                                             /* in arg list */



static	void load_max_args ( hptr, usrlist, arglist, usrct, argct, pcount )

	/* 
	 * This function attempts to load up to the maximum number of
	 * required arguments into the entry.
	 */

    HTABLE      * hptr;                       /* points to one table pattern */
    char       ** usrlist;                /* list of user supplied arguments */
    ARG_SET    ** arglist;                     /* place to put argument list */
    int           usrct,                     /* the number of user arugments */
                * argct,             /* running total of arguments for entry */
                * pcount;                          /* counts current usrlist */

{
    ARG_SET     * argptr;                        /* points to additonal args */
    BOOLEAN       in = TRUE;                                 /* misc boolean */

  if ( *arglist != NULL ) {
    argptr = *arglist;

    while ( argptr->next_arg != NULL )     /* bring argptr to end of arglist */
      argptr = argptr->next_arg;
  } /* if */

  while ((( hptr->max_args == UNLIMITED ) || ( *argct < hptr->max_args )) &&
	    in ) {
    if ( *pcount + 1 < usrct ) {
      (*pcount)++;

      if (! (in = match_arg_list (*(usrlist + *pcount), hptr->legal_args )))
        (*pcount)--;

      else {
        if ( in_arg_list ( *arglist, *(usrlist + *pcount)))
          continue;

        if ( *arglist == NULL ) {
          *arglist = ( ARG_SET * ) malloc ( sizeof ( ARG_SET ));
          argptr = *arglist;
        } /* if */

        else {
          argptr->next_arg = ( ARG_SET * ) malloc ( sizeof ( ARG_SET ));
          argptr = argptr->next_arg;
        } /* else */

        argptr->arg = *(usrlist + *pcount);
        argptr->next_arg = NULL;
        (*argct)++;
      } /* else */
    } /* if */

    else
      in = FALSE;
  } /* while */
}                                                           /* load max args */



static	int set_duplicates ( hptr, eeptr, entry )

	/*
	 * Sets counts and pointers appropriately for handling OVERWRITE
	 * and ACCUM duplicates.
	 */

    HTABLE      * hptr;                       /* points to one table pattern */
    ENTRY_SET  ** eeptr;                              /* points to entry ptr */
    char        * entry;                                  /* points to entry */

{
  if ( hptr->max_entries == 1 ) {
    if ( ! streq ( hptr->entry, entry )) {
      if ( streq ( ARGS_OP, hptr->pattern ))
	uiquit ( ERROR, TRUE, "Only 1 argument allowed.\n" );
      else
	uiquit ( ERROR, TRUE, "More than 1 entry for option: %s.\n",
			       hptr->pattern );
      return ( ERROR );
    } /* if */

    hptr->num_entries = 0;

    if ( hptr->duplicates != ACCUM ) {           /* overwrite existing entry */
      hptr->num_args = 0;
      hptr->arg_list = NULL;
    } /* else */

    else if ( ! streq ( hptr->entry, entry )) {
      hptr->num_args = 0;                     /* can't accum different entry */
      hptr->arg_list = NULL;
    } /* else if */
  } /* if */

  else {
    match_entry ( hptr, entry, eeptr );
  
    if ( hptr->duplicates == OVERWRITE ) {
      if ( *eeptr == NULL ) {
	hptr->num_args = 0;
	hptr->arg_list = NULL;
      } /* if */

      else {
	(*eeptr)->num_args = 0;
	(*eeptr)->arg_list = NULL;
      } /* else */
    } /* if */
  } /* else */

  return ( OK );
}                                                          /* set duplicates */



static	void match_entry ( hptr, entry, eeptr )

	/* 
	 * Finds matching entry and set the eptr to it.  If there is
	 * no entry yet or the new entry matches the first entry in
	 * the apattern, return eeptr as NULL.  If the new entry matches
	 * an existing entry, return a pointer to that entry.  Finally,
	 * if the entry does not exist, create space for it.
	 */

    HTABLE      * hptr;                       /* points to one table pattern */
    char        * entry;                         /* option to actually match */
    ENTRY_SET  ** eeptr;                                  /* points to entry */

{
  *eeptr = NULL;

  if ( hptr->entry == NULL )
    return;
  
  if ( streq ( hptr->entry, entry )) {             /* entry already in place */
    (hptr->num_entries)--;       /* subtract entry as it will be added later */
    return;
  } /* if */

  if ( hptr->entry_list == NULL ) {
    hptr->entry_list = ( ENTRY_SET * ) malloc ( sizeof ( ENTRY_SET ));
    *eeptr = hptr->entry_list;
  } /* if */

  else {
    *eeptr = hptr->entry_list;

    if ( streq ( (*eeptr)->entry, entry )) {
      (hptr->num_entries)--;
      return;
    } /* if */

    while ( (*eeptr)->next_entry != NULL ) {
      *eeptr = (*eeptr)->next_entry;

      if ( streq ( (*eeptr)->entry, entry )) {
	(hptr->num_entries)--;
	return;
      } /* if */
    } /* while */

    (*eeptr)->next_entry = ( ENTRY_SET * ) malloc ( sizeof ( ENTRY_SET ));
    *eeptr = (*eeptr)->next_entry;
  } /* else */

  (*eeptr)->num_args = 0;
  (*eeptr)->arg_list = NULL;
  (*eeptr)->next_entry = NULL;
}                                                             /* match entry */



static	void print_htable ()

	/*
	 * Prints out the initialized portion of the hash table.
	 */

{
    HTABLE      * hptr;                        /* points to each table entry */
    ARG_SET     * aptr;                                    /* points to args */
    ENTRY_SET   * eptr;                                 /* points to entries */
    int           ct;                                        /* misc counter */

  ui_print ( VDEBUG, "Contents of interface table:\n" );

  for ( ct = 0; ct < HTABLESIZE; ct++ ) {
    hptr = htbl + ct;
    aptr = hptr->arg_list;
    eptr = hptr->entry_list;

    if ( *(hptr->pattern) != NUL ) {
      print_internals ( hptr, aptr, eptr, ct );
      hptr = hptr->next_pattern;

      while ( hptr != NULL ) {
	aptr = hptr->arg_list;
        eptr = hptr->entry_list;
	print_internals ( hptr, aptr, eptr, -1 );
	hptr = hptr->next_pattern;
      } /* while */
    } /* if */
  } /* for */

  ui_print ( VCONT, "vl=%d; ap=%d; ip=%d.\n", verbosity, auto_pos, info_pos );
}                                                            /* print htable */



static	void print_internals ( hptr, aptr, eptr, ct )

	/*
	 * Prints out internals of a single hash table entry.
	 */

    HTABLE      * hptr;                             /* points to table entry */
    ARG_SET     * aptr;                                    /* points to args */
    ENTRY_SET   * eptr;                                 /* points to entries */
    int           ct;                                         /* table entry */

{
  if ( ct == -1 )
    ui_print ( VCONT, "    %s: maxe=%d; dup=%d; mina=%d; maxa=%d; a=%s.\n",
		       hptr->pattern, hptr->max_entries, hptr->duplicates,
		       hptr->min_args, hptr->max_args, hptr->legal_args  );
  else
    ui_print ( VCONT, "%-2d: %s: maxe=%d; dup=%d; mina=%d; maxa=%d; a=%s.\n",
		       ct, hptr->pattern, hptr->max_entries, hptr->duplicates,
		       hptr->min_args, hptr->max_args, hptr->legal_args  );

  if ( hptr->num_entries > 0 ) {
    ui_print ( VCONT, "    e=%s; numa=%d; nume=%d.\n",
	       hptr->entry, hptr->num_args, hptr->num_entries );
  
    while ( aptr != NULL ) {
      ui_print ( VCONT, "      arg: %s.\n", aptr->arg );
      aptr = aptr->next_arg;
    } /* while */

    while ( eptr != NULL ) {
      aptr = eptr->arg_list;
      ui_print ( VCONT, "    e=%s: numa=%d.\n", eptr->entry, eptr->num_args );
    
      while ( aptr != NULL ) {
	ui_print ( VCONT, "      arg: %s\n", aptr->arg );
	aptr = aptr->next_arg;
      } /* while */

      eptr = eptr->next_entry;
    } /* while */
  } /* if */
}                                                         /* print internals */



int	ui_load ( va_alist )

	/*
	 *  Looks for exact match to pattern and loads the table
	 *  entry.  It peals off the first two arguments to get the
	 *  pattern to match and the arg count.  It then creates a
	 *  **char list of the rest of the arguments and passes this
	 *  to the functions that load entries.
	 *  Returns OK if it is successful, else ERROR.
	 */

    va_dcl

{
    HTABLE      * hptr;                             /* points to one pattern */
    va_list       arglist;                       /* variable length arg list */
    char        * pattern,                      /* pattern to match in table */
                * match,                                  /* misc string ptr */
               ** vlist;             /* holds list of arguments from arglist */
    int           argct,                     /* number of arguments to parse */
                  ct,                                       /* dummy counter */
		  rvalue = OK;	  			     /* return value */

  from_ui_load = TRUE;
  fflush ( stdout );
  va_start ( arglist );
  pattern = va_arg ( arglist, char * );             /* load first two fields */
  argct = va_arg ( arglist, int );

  if (( match = built_ins ( pattern )) == NULL ) {
    if ( special_case ( pattern )) {           /* try built in, special case */
      va_end ( arglist );
      from_ui_load = FALSE;
      return ( rvalue );
    } /* if */

    pattern = salloc ( pattern );
  } /* if */

  else
    pattern = match;

  if (( hptr = match_pattern ( pattern )) == NULL ) {
    ui_print ( VDEBUG, "pattern: %s, not in entry table.\n", pattern );
    rvalue = ERROR;
  } /* if */

  if ( rvalue != ERROR && hptr->max_entries != 1 ) {
    ui_print ( VDEBUG, "cannot ui_load pattern: %s, with max_entries: %d.\n",
			pattern, hptr->max_entries );
    rvalue = ERROR;
  } /* if */

  if ( rvalue != ERROR ) {
    if ( argct > 0 ) {
      argct++;                                        /* account for pattern */

      if (( vlist = ( char ** ) calloc ( argct, sizeof ( char * ))) == NULL )
	uquit ( ERROR, FALSE, "sys: could not calloc space for vlist.\n" );

      *vlist = pattern;

      for ( ct = 1; ct < argct; ct++ )
      *(vlist + ct) = salloc ( va_arg ( arglist, char * ));
    } /* if */

    ct = 0;

    if ( hptr->num_entries == 0 ) {
      if ( load_entry ( hptr, NULL, vlist, pattern, argct, &ct ) == ERROR )
	rvalue = ERROR;
    } /* if */

    else if ( hptr->duplicates == TOGGLE ) {
      if ( ui_unset ( pattern ) == ERROR )
	rvalue = ERROR;
    } /* else if */

    else {
      if ( set_duplicates ( hptr, NULL, pattern ) == ERROR )
	rvalue = ERROR;
      else if ( load_entry ( hptr, NULL, vlist, pattern, argct, &ct ) == ERROR )
	rvalue = ERROR;
    } /* else */
  } /* if */

  va_end ( arglist );
  from_ui_load = FALSE;
  return ( rvalue );
}                                                                 /* ui load */



BOOLEAN ui_is_set ( pattern )

	/*
	 * Returns TRUE if pattern's entry is set; FALSE otherwise.
 	 */

    char      * pattern;                        /* pattern to match in table */

{
    HTABLE      * hptr;                             /* points to one pattern */

  if (( hptr = match_pattern ( pattern )) == NULL ) {
    ui_print ( VDEBUG, "pattern: %s, not in entry table.\n", pattern );
    return ( FALSE );
  } /* if */

  return ( hptr->num_entries != 0 );
}                                                               /* ui is set */



int	ui_entry_cnt ( pattern )
	/*
	 * Returns number of entries which match pattern; 0 if no entries
	 * matched the pattern; ERROR if no such pattern.
 	 */

    char      * pattern;                        /* pattern to match in table */

{
    HTABLE      * hptr;                             /* points to one pattern */

  if (( hptr = match_pattern ( pattern )) == NULL ) {
    ui_print ( VDEBUG, "pattern: %s, not in entry table.\n", pattern );
    return ( ERROR );
  } /* if */

  return ( hptr->num_entries );
}                                                            /* ui entry cnt */



int	ui_arg_cnt ( pattern, entry_num )

	/*
	 * Returns number of arguments in entry; 0 if no entries matched
	 * the pattern; ERROR if no such pattern or entry_num.
 	 */

    char      * pattern;                        /* pattern to match in table */
    int         entry_num;               /* entry number of matching pattern */

{
    HTABLE      * hptr;                             /* points to one pattern */
    ENTRY_SET   * eptr;                               /* points to one entry */
    int           ct;                                        /* misc counter */

  if ( entry_num < 1 ) {
    ui_print ( VDEBUG, "entry number: %d, is less than 1.\n", entry_num );
    return ( ERROR );
  } /* if */

  if (( hptr = match_pattern ( pattern )) == NULL ) {
    ui_print ( VDEBUG, "pattern: %s, not in entry table.\n", pattern );
    return ( ERROR );
  } /* if */

  if ( hptr->num_entries == 0 )
    return ( 0 );            /* can use this to check for existence of match */

  if ( entry_num == 1 )
    return ( hptr->num_args );

  if ( entry_num > hptr->num_entries ) {
    ui_print ( VDEBUG,
      "entry number: %d, greater than number of entries: %d.\n",
       entry_num, hptr->num_entries );
    return ( ERROR );
  } /* if */

  eptr = hptr->entry_list;

  for ( ct = 2; ct < entry_num; ct++ )
    eptr = eptr->next_entry;

  return ( eptr->num_args );
}                                                              /* ui arg cnt */



char * 	ui_entry_value ( pattern, entry_num )

	/*
 	 * Value of a particular entry of a particular pattern.
	 * Returns pointer to string; NULL if no such pattern or entry_num
	 * or pattern not matched.
 	 */

    char      * pattern;                        /* pattern to match in table */
    int         entry_num;               /* entry number of matching pattern */

{
    HTABLE      * hptr;                             /* points to one pattern */
    ENTRY_SET   * eptr;                               /* points to one entry */
    int           ct;                                        /* misc counter */

  if ( entry_num < 1 ) {
    ui_print ( VDEBUG, "entry number: %d, is less than 1.\n", entry_num );
    return ( NULL );
  } /* if */

  if (( hptr = match_pattern ( pattern )) == NULL ) {
    ui_print ( VDEBUG, "pattern: %s, not in entry table.\n", pattern );
    return ( NULL );
  } /* if */

  if ( hptr->num_entries == 0 )
    return ( NULL );         /* can use this to check for existence of match */

  if ( entry_num == 1 )
    return ( hptr->entry );

  if ( entry_num > hptr->num_entries ) {
    ui_print ( VDEBUG,
      "entry number: %d, greater than number of entries: %d.\n",
       entry_num, hptr->num_entries );
    return ( NULL );
  } /* if */

  eptr = hptr->entry_list;

  for ( ct = 2; ct < entry_num; ct++ )
    eptr = eptr->next_entry;

  return ( eptr->entry );
}                                                          /* ui entry value */



char * 	ui_arg_value ( pattern, entry_num, arg_num )

	/*
 	 * Value of a particular argument of a particular entry.
	 * Returns pointer to string; NULL if no such pattern, entry_num,
	 * or arg_num, also NULL if pattern not matched.
 	 */

    char      * pattern;                        /* pattern to match in table */
    int         entry_num;               /* entry number of matching pattern */
    int         arg_num;                         /* argument number of entry */

{
    HTABLE      * hptr;                             /* points to one pattern */
    ENTRY_SET   * eptr;                               /* points to one entry */
    int           ct;                                        /* misc counter */

  if ( entry_num < 1 ) {
    ui_print ( VDEBUG, "entry number: %d, is less than 1.\n", entry_num );
    return ( NULL );
  } /* if */

  if ( arg_num < 1 ) {
    ui_print ( VDEBUG, "argument number: %d, is less than 1.\n", arg_num );
    return ( NULL );
  } /* if */

  if (( hptr = match_pattern ( pattern )) == NULL ) {
    ui_print ( VDEBUG, "pattern: %s, not in entry table.\n", pattern );
    return ( NULL );
  } /* if */

  if ( hptr->num_entries == 0 )
    return ( NULL );

  if ( entry_num == 1 )
    return ( arg_value ( hptr->arg_list, arg_num, hptr->num_args ));

  if ( entry_num > hptr->num_entries ) {
    ui_print ( VDEBUG,
      "entry number: %d, greater than number of entries: %d.\n",
       entry_num, hptr->num_entries );
    return ( NULL );
  } /* if */

  eptr = hptr->entry_list;

  for ( ct = 2; ct < entry_num; ct++ )
    eptr = eptr->next_entry;

  return ( arg_value ( eptr->arg_list, arg_num, eptr->num_args ));
}                                                            /* ui arg value */



static	char * arg_value ( arglist, argnum, maxargs )

	/*
	 * Finds the argument in the arglist specified by the argnum.
	 * Returns pointer to this string; NULL if if cannot find string
	 * for any reason.
	 */

    ARG_SET     * arglist;                       /* list of args to run down */
    int           argnum,                        /* argument number to reach */
                  maxargs;                    /* maximum arguments available */

{
    int           ct;                                        /* misc counter */

  if ( argnum > maxargs ) {
    ui_print ( VDEBUG,
      "argument number: %d, greater than number of argument: %d.\n",
       argnum, maxargs );
    return ( NULL );
  } /* if */

  for ( ct = 1; ct < argnum; ct++ )
    arglist = arglist->next_arg;

  return ( arglist->arg );
}                                                               /* arg value */



int	ui_entries_to_argv ( pattern, entries )

	/*
 	 * Place all entries for a pattern into a char ** list.
 	 * Returns number of entries put in "entries"; 0 if no
	 * entries; ERROR if no such pattern or entry.
 	 */
    char      * pattern;                        /* pattern to match in table */
    char    *** entries;                             /* list to hold entries */

{
    HTABLE      * hptr;                             /* points to one pattern */
    ENTRY_SET   * eptr;                               /* points to one entry */
    int           ct;                                        /* misc counter */

  if (( hptr = match_pattern ( pattern )) == NULL ) {
    ui_print ( VDEBUG, "pattern: %s, not in entry table.\n", pattern );
    return ( ERROR );
  } /* if */

  if ( hptr->num_entries == 0 )
    return ( 0 );

  if (( *entries = ( char ** ) calloc ( hptr->num_entries, sizeof ( char * )))
       == NULL )
    uquit ( ERROR, FALSE, "sys: could not calloc space for entry argv list.\n");

  **entries = hptr->entry;
  eptr = hptr->entry_list;

  for ( ct = 1; ct < hptr->num_entries; ct++ ) {
    *(*entries + ct) = eptr->entry;
    eptr = eptr->next_entry;
  } /* for */

  return ( hptr->num_entries );
}                                                      /* ui entries to argv */



int	ui_args_to_argv ( pattern, entry_num, arguments )

	/*
 	 * Place all arguments for a particular entry of a pattern into
	 * a char ** list.
 	 * Returns number of arguments put in "arguments"; 0 if no
	 * arguments; ERROR if no such pattern or entry_num.
 	 */
    char      * pattern;                        /* pattern to match in table */
    int         entry_num;               /* entry number of matching pattern */
    char    *** arguments;                          /* list to hold argument */

{
    HTABLE      * hptr;                             /* points to one pattern */
    ENTRY_SET   * eptr;                               /* points to one entry */
    ARG_SET     * aptr;                            /* points to one argument */
    int           total,                                     /* misc integer */
                  ct;                                        /* misc counter */

  if ( entry_num < 1 ) {
    ui_print ( VDEBUG, "entry number: %d, is less than 1.\n", entry_num );
    return ( ERROR );
  } /* if */

  if (( hptr = match_pattern ( pattern )) == NULL ) {
    ui_print ( VDEBUG, "pattern: %s, not in entry table.\n", pattern );
    return ( ERROR );
  } /* if */

  if ( hptr->num_entries == 0 )
    return ( 0 );

  if ( entry_num == 1 ) {
    aptr = hptr->arg_list;
    total = hptr->num_args;
  } /* if */

  else {
    if ( entry_num > hptr->num_entries ) {
      ui_print ( VDEBUG,
	"entry number: %d, greater than number of entries: %d.\n",
	 entry_num, hptr->num_entries );
      return ( ERROR );
    } /* if */

    eptr = hptr->entry_list;

    for ( ct = 2; ct < entry_num; ct++ )
      eptr = eptr->next_entry;

    aptr = eptr->arg_list;
    total = eptr->num_args;
  } /* else */

  if ( total == 0 )
    return ( 0 );

  if (( *arguments = ( char ** ) calloc ( total, sizeof ( char * ))) == NULL )
    uquit ( ERROR, FALSE, "could not calloc space for argument argv list.\n" );

  for ( ct = 0; ct < total; ct++ ) {
    *(*arguments + ct) = aptr->arg;
    aptr = aptr->next_arg;
  } /* for */

  return ( total );
}                                                         /* ui args to argv */



char * 	ui_ver_switch ( )

	/*
 	 * Returns a command line switch, "string", corresponding to
	 * the current verbosity level.
 	 */

{
  switch ( verbosity ) {
    case VQUIET  : return ( salloc ( QUIET_OP ));
    case VNORMAL : return ( salloc ( NORMAL_OP ));
    case VDETAIL : return ( salloc ( VERBOSE_OP ));
    case VDEBUG  : return ( salloc ( DEBUG_OP ));
    default      : ui_print ( VFATAL, "illegal level of verbosity: %d.\n",
				       verbosity );
    		   return ( NULL );
  } /* switch */
}                                                       /* ui verbose switch */



BOOLEAN ui_is_auto ()

	/*
 	 * Shortcut function.
	 * Returns is -auto set?
 	 */
{
  return ( (htbl+auto_pos)->num_entries != 0 );
}                                                              /* ui is auto */



BOOLEAN ui_is_info ()

	/*
 	 * Shortcut function.
	 * Returns is -info set?
 	 */
{
  return ( (htbl+info_pos)->num_entries != 0 );
}                                                              /* ui is info */



int 	ui_ver_level ()

	/*
 	 * Returns the current verbosity level as an integer level.
 	 */
{
  return ( verbosity );
}                                                            /* ui ver level */

ui_print_revision ( )
{
  ui_print ( VALWAYS, "program :  %s\nrelease :  %s\nlibsb   :  %s\n",
                      progname, BUILD_VERSION, BUILD_DATE );
}
