static char sccsid[] = "@(#)71  1.1  src/bldenv/sbtools/mklinks/mklinks.c, bldprocess, bos412, GOLDA411a 4/29/93 12:26:08";
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
**                               April 1990                                  **
*******************************************************************************
**
**  Description:
**	This program makes the links or copies of files in the backing
**	build to sandboxes.
**
**  Functions:
**    main (int, * * char) int
**      symlink_file (* char, * char) int
**      cmdline_syntax (* * char, * * char) int
**        setup_absolutes (* * char, * * char) int
**        setup_sandbox (* * char, * * char) int
**        check_base (* char) int
**      copy_file (* char, * char) int
**      create_links (* char, * char, * () int) int
**        mklinks : library routine from libsb.a
**        cmp_files (* char, * char) int
**    print_usage () int
 */

static char * rcsid =
 "$RCSfile: mklinks.c,v $ $Revision: 1.12.2.2 $ $Date: 92/03/25 22:46:24 $";

#  include <ode/odedefs.h>
#  include <sys/param.h>
#  include <sys/stat.h>
#  include <sys/errno.h>
#  include <sys/time.h>
/*
  Note, the S on the end of NO_UTIMES is signifigant!
*/
#ifdef NO_UTIMES
#include <utime.h>
#endif

#ifndef	MAXBSIZE
#  define MAXBSIZE 8192
#endif

#  define  CMP_OP   	"-cmp"
#  define  LINK_FR_OP  	"-link_from"
#  define  LINK_TO_OP  	"-link_to"
#  define  NORECURSE_OP "-norecurse"
#  define  OVER_OP   	"-over"
#  define  QUERY_OP   	"-query"
#  define  MAX_ARGS	9

#ifdef NO_DIRENT
#  define dirent direct
#endif

    UIINIT init [] = {
      { COPY_OP,      1, OVERWRITE, 0, 0, "" },
      { OVER_OP,      1, OVERWRITE, 0, 0, "" },
      { NORECURSE_OP, 1, OVERWRITE, 0, 0, "" },
      { CMP_OP,       1, OVERWRITE, 0, 0, "" },
      { QUERY_OP,     1, OVERWRITE, 0, 0, "" },
      { LINK_FR_OP,   1, OVERWRITE, 1, 1, ARGS_OP },
      { LINK_TO_OP,   1, OVERWRITE, 1, 1, ARGS_OP },
      { SB_OP,        1, OVERWRITE, 1, 1, ARGS_OP },
      { ARGS_OP,      1, OVERWRITE, 0, 0, "" } 
    };

    int  	  symlink_file ();
    int 	  copy_file ();
    int 	  cmp_files (); 
    extern int    errno;
    char 	* progname = "mklinks";

char *malloc();
main ( argc, argv )

    int   	  argc;
    char       ** argv;

{
    char	* linkfrom,
         	* linkto;
    int        (* linkfunc ) () = symlink_file;

  ui_init ( argc, argv, MAX_ARGS, init );
  linkfrom = malloc(PATH_LEN);
  linkto = malloc(PATH_LEN);
  cmdline_syntax ( &linkfrom, &linkto );

  if ( ui_is_set ( COPY_OP ))
    linkfunc = copy_file;

  create_links ( linkfrom, linkto, linkfunc );
  return( OK );
}                                                                    /* main */



int symlink_file ( src, dst )

    char 	* src, 
		* dst;

{
  if ( symlink ( src, dst ) < 0 ) {
    ui_print ( VFATAL, "symlink: %s\n", errmsg(-1));
    return( ERROR );
  } /* if */

  return ( OK );
}                                                                 /* symlink */



cmdline_syntax ( linkfrom, linkto )

	/* This procedure makes sure all the options agree with each
	   other. */

    char        ** linkfrom,                /* buffer for where to link from */
                ** linkto;                    /* buffer for where to link to */

{
  if (( ui_is_set ( LINK_FR_OP ) && ! ui_is_set ( LINK_TO_OP )) ||
      ( ! ui_is_set ( LINK_FR_OP ) && ui_is_set ( LINK_TO_OP )))
    uquit ( ERROR, TRUE, "\toptions %s and %s must be set together.\n",
	       		    LINK_FR_OP, LINK_TO_OP );

  if ( ui_is_set ( OVER_OP )) {
    if ( ! ui_is_set ( COPY_OP ))
      uquit ( ERROR, TRUE, "\t%s option must be set to used the %s option.\n",
	      		      COPY_OP, OVER_OP );
  } /* if */

  if ( ui_is_set ( LINK_FR_OP ) && ui_is_set ( LINK_TO_OP )) {
    if ( ui_is_set ( ARGS_OP ))
      uquit ( ERROR, TRUE,
	      "\tCannot set options %s and %s and still give directory.\n",
	       	 LINK_FR_OP, LINK_TO_OP );
       
    setup_absolutes ( linkfrom, linkto );
  } /* if */

  else {
    if ( ! ui_is_set ( ARGS_OP ))
      uquit ( ERROR, TRUE,
	      "\tdirectory entry or the -link_ options required.\n" );

    setup_sandbox ( linkfrom, linkto );
  } /* else */

  check_base ( *linkfrom );
  check_base ( *linkto );

  ui_print ( VDETAIL, "%s: %s\n   to     : %s\n",
	     ui_is_set ( COPY_OP ) ? "copying": "linking", *linkfrom, *linkto );
}                                                          /* cmdline syntax */



setup_absolutes ( linkfrom, linkto )

	/* This procedure sets up the absolute base for the from
	   and to copy. */

    char       ** linkfrom,                 /* buffer for where to link from */
               ** linkto;                     /* buffer for where to link to */

{
    char          curdir [ PATH_LEN ],            /* holds current directory */
                  tmpline [ PATH_LEN ],                       /* misc string */
                * tfrom,                              /* temporary link from */
                * tto;                                  /* temporary link to */

  tfrom = ui_arg_value ( LINK_FR_OP, 1, 1 );
  tto = ui_arg_value ( LINK_TO_OP, 1, 1 );

  if ( *tfrom != SLASH ) {
    if ( getwd ( curdir ) == NULL )
      uquit ( ERROR, FALSE, "getwd failed: %s.\n", curdir );

    concat ( tmpline, PATH_LEN, curdir, "/", tfrom, NULL );
    strcpy(*linkfrom, tmpline );
  } /* if */

  else
    strcpy(*linkfrom, tfrom );

  if ( *tto != SLASH ) {
    if ( getwd ( curdir ) == NULL )
      uquit ( ERROR, FALSE, "getwd failed: %s.\n", curdir );

    concat ( tmpline, PATH_LEN, curdir, "/", tto, NULL );
    strcpy ( *linkto , tmpline );
  } /* if */

  else
    strcpy ( *linkto , tto );
}                                                         /* setup absolutes */



setup_sandbox ( linkfrom, linkto )

	/* This procedure sets up the linking based on the sandbox. */

    char       ** linkfrom,                 /* buffer for where to link from */
               ** linkto;                     /* buffer for where to link to */

{
    char        * sb,                              /* string to hold sandbox */
                * basedir = NULL,          /* string to hold sandbox basedir */
                * sb_rcfile,                    /* string to hold sb rc file */
                * rc_file,                     /* string to hold usr rc file */
                  sandboxdir [ PATH_LEN ],/* string with sandbox and basedir */
                  curdir [ PATH_LEN ],            /* holds current directory */
                  tmpline [ PATH_LEN ],                       /* misc string */
                * directory,                       /* user entered directory */
                * curpath;                           /* path from sandboxdir */

  sb = ui_arg_value ( SB_OP, 1, 1 );
  sb_rcfile = ui_arg_value ( SB_RC_OP, 1, 1 );
  rc_file = ui_arg_value ( RC_OP, 1, 1 );

  if ( current_sb ( &sb, &basedir, &sb_rcfile, &rc_file ) != OK )
    uquit ( ERROR, FALSE, "could not get current sandbox information.\n" );

  if ( *(directory = ui_entry_value ( ARGS_OP, 1 )) == SLASH ) {
    concat ( tmpline, PATH_LEN, basedir, "/", sb, "/link", directory, NULL );
    strcpy ( *linkfrom , tmpline );
    concat ( tmpline, PATH_LEN, basedir, "/", sb, directory, NULL );
    strcpy ( *linkto , tmpline );
  } /* if */

  else {
    concat ( sandboxdir, PATH_LEN, basedir, "/", sb, NULL );

    if ( getwd ( curdir ) == NULL )
      uquit ( ERROR, FALSE, "getwd failed: %s.\n", curdir );

    if ( strncmp ( sandboxdir, curdir, strlen ( sandboxdir )) != OK )
      uquit ( ERROR, TRUE,
	 "directory given is relative; current directory not in sandbox.\n" );
    
    curpath = &curdir [ strlen ( sandboxdir ) ];
    concat ( tmpline, PATH_LEN, sandboxdir, "/link", curpath, "/",
	     directory, NULL );
    strcpy ( *linkfrom , tmpline );
    concat ( tmpline, PATH_LEN, curdir, "/", directory, NULL);
    strcpy ( *linkto , tmpline );
  } /* else */
}                                                           /* setup sandbox */



check_base ( base )

	/* This procedure checks to see that the to path is absolute. */

    char      * base;                                       /* base to check */

{
    char      * ptr;                                     /* misc ptr to char */

  ptr = base + strlen ( base ) - 1;

  if ( *base != '/' )
    uquit ( ERROR, FALSE, "the directory to link to, %s, is not absolute.\n",
	       		   base );

  while ( ptr > base && *ptr == SLASH )        /* eliminate trailing slashes */
    ptr--;

  *(++ptr) = NUL;
}                                                              /* check base */



int copy_file ( src, dst )

    char 	* src, 
		* dst;

{
    struct 	stat st;
#ifdef NO_UTIMES
    struct utimbuf tv;
#else
    struct timeval tv[2];
#endif
    int 	  fd1,
		  fd2;

  if ((fd1 = open(src, O_RDONLY, 0)) < 0) {
    ui_print ( VFATAL, "open %s: %s\n", src, errmsg(-1));
    return( ERROR );
  } /* if */

  if (fstat(fd1, &st) < 0) {
    ui_print ( VFATAL, "fstat %s: %s\n", src, errmsg(-1));
    (void) close(fd1);
    return( ERROR );
  } /* if */

  if ((fd2 = open(dst, O_WRONLY|O_TRUNC|O_CREAT, 0666)) < 0) {
    ui_print ( VFATAL, "open %s: %s\n", dst, errmsg(-1));
    (void) close(fd1);
    return( ERROR );
  } /* if */

  if (filecopy(fd1, fd2) < 0) {
    ui_print ( VFATAL, "filecopy: %s\n", errmsg(-1));
    (void) close(fd1);
    (void) close(fd2);
    return( ERROR );
  } /* if */

  if (close(fd1) < 0) {
    ui_print ( VFATAL, "close %s: %s\n", src, errmsg(-1));
    (void) close(fd1);
    (void) close(fd2);
    return( ERROR );
  } /* if */

  if (close(fd2) < 0) {
    ui_print ( VFATAL, "close %s: %s\n", dst, errmsg(-1));
    (void) close(fd2);
    return( ERROR );
  } /* if */

  /* if (chmod(dst, st.st_mode&0777) < 0) {
    ui_print ( VFATAL, "chmod %s: %s\n", dst, errmsg(-1));
    return( ERROR );
  } if */

#ifdef NO_UTIMES
  tv.actime = st.st_atime;
  tv.modtime = st.st_mtime;
  if (utime(dst, &tv) < 0) {
    ui_print ( VFATAL, "utime %s: %s\n", dst, errmsg(-1));
    return( ERROR );
  } /* if */
#else
  tv[0].tv_sec = st.st_atime;
  tv[0].tv_usec = 0;
  tv[1].tv_sec = st.st_mtime;
  tv[1].tv_usec = 0;
  if (utimes(dst, tv) < 0) {
    ui_print ( VFATAL, "utimes %s: %s\n", dst, errmsg(-1));
    return( ERROR );
  } /* if */
#endif

  return(0);
}                                                               /* copy file */



create_links ( linkfrom, linkto, linkfunc )

	/* This procedure actually sets up the call to create
	   the links after it has checked to see that the
	   directories are legal. */


    char        * linkfrom,                 /* buffer for where to link from */
                * linkto;                     /* buffer for where to link to */
    int        (* linkfunc) ();                     /* pointer to a function */

{
    char        * frombase,                            /* points to linkfrom */
                * tobase,                                /* points to linkto */
                  newdir [ PATH_LEN ];                        /* misc string */
    int           isnew;                                     /* misc boolean */


  frombase = linkfrom;
  tobase = linkto;

  if ( chdir ( linkto ) < OK ) {             /* change directories to linkto */
    if ( ui_is_set ( CMP_OP ) || errno != ENOENT )
      uquit ( ERROR, FALSE, "chdir %s: %s\n", linkto, errmsg ( ERROR ));

    concat ( newdir, PATH_LEN, linkto, "/.", NULL );
    ui_print ( VDEBUG, "makepath %s.\n", linkto );

    if ( makepath ( newdir, NULL, ui_ver_level () == VDEBUG, TRUE ) != OK )
      uquit ( ERROR, FALSE, "makepath on %s failed.\n", linkto );

    isnew = TRUE;

    if ( chdir ( linkto ) < OK )
      uquit ( ERROR, FALSE, "chdir %s: %s\n", linkto, errmsg(-1));
  } /* if */

  else
    isnew = FALSE;

  ui_print ( VDEBUG, "chdir to %s.\n", linkto );

  if ( chdir ( frombase ) < OK )
    uquit ( ERROR, FALSE, "chdir failed on %s: %s.\n", frombase, errmsg ( -1 ));

  ui_print ( VDEBUG, "chdir to %s.\n", frombase );

  if ( ! ui_is_set ( CMP_OP ))
    ui_print  ( VNORMAL, "\n%sing %s:\n  From: %s\n  To:   %s\n\n",
	           ui_is_set ( COPY_OP ) ? "Copy" : "Link",
	           ui_is_set ( NORECURSE_OP ) ? "files" : "directory subtree",
	           frombase, tobase);
  else
    ui_print  ( VNORMAL, "\nComparing %s:\n      %s\n  to: %s\n\n",
	         ui_is_set ( NORECURSE_OP ) ? "files in" : "directory subtree",
	         frombase, tobase);

  if ( ! ui_is_auto () && ! ui_is_info () ) {
    if ( !getbool ( "Is this correct?", TRUE ))
      uquit ( ERROR, FALSE, "Aborting...\n" );
  } /* if */

  if ( ! ui_is_info ())
    mklinks ( isnew, ui_is_set ( QUERY_OP ), ! ui_is_set ( NORECURSE_OP ),
	        ui_is_set ( OVER_OP ), frombase, tobase, NULL, 
	        ui_is_set ( CMP_OP ) ? cmp_files : NULL, linkfunc );
}                                                            /* create links */



int cmp_files ( namebuf, curpath )

    char *namebuf, *curpath;

{
    static u_char buf1 [ MAXBSIZE ], buf2 [ MAXBSIZE ];
    int len1, len2;
    int fd1, fd2;

  if ((fd1 = open(namebuf, O_RDONLY, 0)) < 0) {
    ui_print ( VFATAL, "open %s: %s\n", namebuf, errmsg(-1));
    return( ERROR );
  } /* if */

  if ((fd2 = open(curpath, O_RDONLY, 0)) < 0) {
    ui_print ( VFATAL, "open %s: %s\n", curpath, errmsg(-1));
    return( ERROR );
  } /* if */

  for (;;) {
    len1 = read(fd1, buf1, sizeof(buf1));
    if (len1 < 0) {
      ui_print ( VFATAL, "read: %s\n", errmsg(-1));
      return( ERROR );
    } /* if */

    len2 = read(fd2, buf2, (len1 == 0) ? 1 : len1);
    if (len2 < 0) {
      ui_print ( VFATAL, "read: %s\n", errmsg(-1));
      return( ERROR );
    } /* if */

    if (len1 == 0 || len2 != len1) {
      (void) close(fd1);
      (void) close(fd2);
      return(len2 != len1);
    } /* if */

    if (bcmp(buf1, buf2, len2) != 0) {
      (void) close(fd1);
      (void) close(fd2);
      return(1);
    } /* if */
  } /* for */
}                                                               /* cmp files */



print_usage ()

{
  printf ( "USAGE:\n" );
  printf ( "%s [-copy [-over] -norecurse] [sb_opts]\n", progname );
  printf ( "\t<directory> | [abs_opts]\n");
  printf ( "\t-copy: copy the files instead of linking them\n");
  printf ( "\t  -over: overwrite file on copy even if it exists\n");
  printf ( "\t-norecurse: do files in the directory, not sub-directories\n");
  printf ( "\tdirectory: the directory to link in from the backing tree.\n" );
  printf ( "\tabs_opts:\n" );
  printf ( "\t  -link_from <dir>: non-sandbox directory to link from\n");
  printf ( "\t  -link_to <dir>: non-sandbox directory to link to\n");
  printf ( "\t  -cmp: compare and link identical files\n");
  printf ( "\t  -query: with -cmp flag, query for each file to be relinked\n");
  printf ( "\tsb_opts:\n" );
  printf ( "\t  -sb <sandbox>, -rc <user rc> -sb_rc <sb rc>\n");
  printf ( "%s -usage | -rev\n", progname );
}                                                             /* print usage */
