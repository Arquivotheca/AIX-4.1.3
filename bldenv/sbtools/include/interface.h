/* @(#)21 1.1 src/bldenv/sbtools/include/interface.h, bldprocess, bos412, GOLDA411a 93/04/29 12:18:45 */
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
**                              December 1990                                **
*******************************************************************************
**
**    Description:
**      This header file is for the user interface library routine in libsb.a.
 */
/*                       DEFINES                                             */

#  define   UNLIMITED       -1               /* number of max_args unlimited */

#  define   TOGGLE          -1                /* on re-entry, toggle pattern */
#  define   OVERWRITE        0               /* on re-entry, write over args */
#  define   ACCUM            1                    /* on re-entry, accum args */

#  define   VALWAYS          0            /* always printed - necessary info */
#  define   VFATAL           1               /* always printed - fatal error */
#  define   VQUIET           2   /* no msg has this verbosity, but level can */
#  define   VWARN            3                    /* normal warning messages */
#  define   VNORMAL          4                         /* normal information */
#  define   VDIAG            5     		  /* -verbose error messages */
#  define   VDETAIL          6                        /* verbose information */
#  define   VDEBUG           7                         /* -debug information */
#  define   VCONT	     8			/* continue previous message */

#  define   AUTO_OP         "-auto"     /* standard options always supported */
#  define   DEBUG_OP        "-debug"
#  define   INFO_OP         "-info"
#  define   NOAUTO_OP       "-noauto"
#  define   NORMAL_OP       "-normal"
#  define   QUIET_OP        "-quiet"
#  define   RC_OP           "-rc"
#  define   REV_OP          "-rev"
#  define   SB_RC_OP        "-sb_rc"
#  define   USAGE_OP        "-usage"
#  define   VERBOSE_OP      "-verbose"
#  define   VERSION_OP	    "-version"	/* synonym for "-rev" */

/*                       GLOBAL DECLARATIONS                                 */

	/* The following structure is used by the user to initialize
	   the ui routine.  The legal_args is a space separated, quoted
	   string of legal arguments to the pattern.  duplicates can
	   have one of four values: TOGGLE, WOENTRY, WOPATTERN, ACCUM. */

    typedef struct uiinit {
      char    * pattern;                                 /* pattern to match */
      int       max_entries,  	       /* maximum number entries per pattern */
		duplicates,      /* how to handle args for duplicate entries */
                min_args,                          /* miminum number of args */
                max_args;                          /* maximum number of args */
      char    * legal_args;                     /* description of legal args */
    } UIINIT;

/*                       RETURN VALUES OF FUNCTIONS                          */

    int		ui_init (),
		ui_set_progname (),
		ui_restore_progname (),
		ui_unset (),
		ui_load (),
		ui_entry_cnt (),
		ui_arg_cnt (),
		ui_entries_to_argv (),
		ui_args_to_argv (),
		ui_ver_level (),
       		ui_is_set (),
		ui_is_auto (),
		ui_is_info ();
    void	ui_print ();
    char      * ui_entry_value (),
	      * ui_arg_value (),
	      * ui_ver_switch ();
