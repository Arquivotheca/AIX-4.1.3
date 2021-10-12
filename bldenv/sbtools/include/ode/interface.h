/* @(#)48       1.1  src/bldenv/sbtools/include/ode/interface.h, bldprocess, bos412, GOLDA411a 1/19/94 17:35:57
 *
 *   COMPONENT_NAME: BLDPROCESS
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27,71
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * @OSF_FREE_COPYRIGHT@
 * COPYRIGHT NOTICE
 * Copyright (c) 1992, 1991, 1990  
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
 * HISTORY
 * $Log: interface.h,v $
 * Revision 1.2.7.6  1993/11/08  22:23:19  damon
 * 	CR 463. Pedantic changes
 * 	[1993/11/08  22:22:29  damon]
 *
 * Revision 1.2.7.5  1993/11/08  20:17:56  damon
 * 	CR 463. Pedantic changes
 * 	[1993/11/08  20:17:17  damon]
 * 
 * Revision 1.2.7.4  1993/11/04  19:52:05  damon
 * 	CR 463. More pedantic
 * 	[1993/11/04  19:48:48  damon]
 * 
 * Revision 1.2.7.3  1993/11/03  23:42:58  damon
 * 	CR 463. More pedantic
 * 	[1993/11/03  23:40:52  damon]
 * 
 * Revision 1.2.7.2  1993/11/03  20:40:06  damon
 * 	CR 463. More pedantic
 * 	[1993/11/03  20:37:54  damon]
 * 
 * Revision 1.2.7.1  1993/10/22  19:21:36  damon
 * 	CR 761. Removed SB_RC_OP
 * 	[1993/10/22  19:21:20  damon]
 * 
 * Revision 1.2.5.6  1993/04/29  15:00:09  damon
 * 	CR 463. Pedantic changes
 * 	[1993/04/29  15:00:04  damon]
 * 
 * Revision 1.2.5.5  1993/04/28  20:44:35  damon
 * 	CR 463. Pedantic changes
 * 	[1993/04/28  20:44:08  damon]
 * 
 * Revision 1.2.5.4  1993/04/27  15:23:55  damon
 * 	CR 463. First round of -pedantic changes
 * 	[1993/04/27  15:19:21  damon]
 * 
 * Revision 1.2.5.3  1993/01/21  18:38:10  damon
 * 	CR 401. Added stdarg
 * 	[1993/01/21  18:34:06  damon]
 * 
 * Revision 1.2.2.2  1992/12/03  19:14:00  damon
 * 	ODE 2.2 CR 346. Expanded copyright
 * 	[1992/12/03  18:43:11  damon]
 * 
 * Revision 1.2  1991/12/05  21:03:50  devrcs
 * 	added VERSION_OP (as a synonym for REV_OP)
 * 	[91/06/03  13:23:16  ezf]
 * 
 * 	Public header file for interface.c; a libsb.a routine
 * 	[91/01/08  13:39:34  randyb]
 * 
 * $EndLog$
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
#  define   USAGE_OP        "-usage"
#  define   VERBOSE_OP      "-verbose"
#  define   VERSION_OP	    "-version"	/* synonym for "-rev" */

/*                       GLOBAL DECLARATIONS                                 */

	/* The following structure is used by the user to initialize
	   the ui routine.  The legal_args is a space separated, quoted
	   string of legal arguments to the pattern.  duplicates can
	   have one of four values: TOGGLE, WOENTRY, WOPATTERN, ACCUM. */

    typedef struct uiinit {
      const char * pattern;                              /* pattern to match */
      int       max_entries,  	       /* maximum number entries per pattern */
		duplicates,      /* how to handle args for duplicate entries */
                min_args,                          /* miminum number of args */
                max_args;                          /* maximum number of args */
      const char * legal_args;                  /* description of legal args */
    } UIINIT;

/*                       RETURN VALUES OF FUNCTIONS                          */

    int		ui_init ( int, char **, int, UIINIT *, int, UIINIT * );
    void	ui_set_progname ( const char *),
    		ui_restore_progname ( void );
    int		ui_unset ( const char *),
#if __STDC__
                ui_load ( const char * , ... ),
#else
                ui_load ( ),
#endif
		ui_entry_cnt ( const char * ),
		ui_arg_cnt ( const char *, int ),
		ui_entries_to_argv ( const char *, char *** ),
		ui_args_to_argv ( const char *, int, char *** ),
		ui_ver_level ( void ),
       		ui_is_set ( const char * ),
		ui_is_auto ( void ),
		ui_is_info ( void );
#if __STDC__
    void        ui_print ( int a, ... );
#else
    void        ui_print ();
#endif
    char * ui_entry_value ( const char *, int ),
	      * ui_arg_value ( const char *, int, int );
          char * ui_ver_switch ( void );
