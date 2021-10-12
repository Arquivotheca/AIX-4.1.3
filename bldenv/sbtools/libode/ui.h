/* @(#)29       1.1  src/bldenv/sbtools/libode/ui.h, bldprocess, bos412, GOLDA411a 1/19/94 17:42:47
 *
 *   COMPONENT_NAME: BLDPROCESS
 *
 *   FUNCTIONS: WHITESPACEFN
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
 * $Log: ui.h,v $
 * Revision 1.2.10.3  1993/11/08  22:23:31  damon
 * 	CR 463. Pedantic changes
 * 	[1993/11/08  22:22:36  damon]
 *
 * Revision 1.2.10.2  1993/11/08  20:18:21  damon
 * 	CR 463. Pedantic changes
 * 	[1993/11/08  20:17:38  damon]
 * 
 * Revision 1.2.10.1  1993/11/05  20:34:45  damon
 * 	CR 463. Pedantic changes
 * 	[1993/11/05  20:33:52  damon]
 * 
 * Revision 1.2.8.1  1993/08/19  18:26:38  damon
 * 	CR 622. Changed if STDC to ifdef STDC
 * 	[1993/08/19  18:25:52  damon]
 * 
 * Revision 1.2.5.4  1993/04/29  14:59:03  damon
 * 	CR 463. Pedantic changes
 * 	[1993/04/29  14:58:53  damon]
 * 
 * Revision 1.2.5.3  1993/01/21  18:38:16  damon
 * 	CR 401. Added stdarg
 * 	[1993/01/21  18:34:13  damon]
 * 
 * Revision 1.2.2.4  1992/12/03  17:22:59  damon
 * 	ODE 2.2 CR 183. Added CMU notice
 * 	[1992/12/03  17:09:28  damon]
 * 
 * Revision 1.2.2.3  1992/08/06  22:07:32  damon
 * 	CR 265. Added unistd.h include
 * 	[1992/08/06  22:07:08  damon]
 * 
 * Revision 1.2.2.2  1992/06/15  18:11:07  damon
 * 	Synched with 2.1.1
 * 	[1992/06/15  18:04:54  damon]
 * 
 * Revision 1.2.5.2  1992/06/15  16:35:21  damon
 * 	Taken from 2.1.1
 * 
 * Revision 1.2.2.2  1992/03/25  22:48:19  damon
 * 	Changes for ui_print_revision
 * 	[1992/03/25  21:52:35  damon]
 * 
 * Revision 1.2  1991/12/05  21:13:25  devrcs
 * 	Added from_ui_load variable to support different way to handle
 * 	failed arguments.
 * 	[91/01/29  11:17:58  randyb]
 * 
 * 	Changed sdm to ode; std_defs.h to  odedefs.h
 * 	[91/01/10  11:47:10  randyb]
 * 
 * 	Header file for interface.c.  There is a public header file,
 * 	interface.h, under /usr/include; this is the private header file.
 * 	[91/01/08  12:23:32  randyb]
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
**      This header file is for the internal use with the interface library
**	routine.
 */
/*                       INCLUDES                                            */

#include <ode/interface.h>
#include <ode/odedefs.h>
#include <ode/versions.h>
#include <stdio.h>
#include <unistd.h>
#ifdef __STDC__
#include <stdarg.h>
#else
#include <varargs.h>
#endif

/*                       DEFINES                                             */

#  define  HTABLESIZE           53                     /* size of hash table */
#  define  TOKEN_LENGTH		500
#  define  WHITESPACEFN(S) 	(*S == ' ' || *S == '\t')

#  define  RCEXT		"rc"


/*                       GLOBAL DECLARATIONS                                 */

	/* The following structure is used to store a list of
           arguments for a single entry. */

    typedef struct arg_set {
      const char * arg;                                     /* entry argument */
      struct    arg_set * next_arg;                    /* points to next arg */
    } ARG_SET;

	/* The following structure is used to store a list of
           entries for a single pattern. */

    typedef struct entry_set {
      const char * entry;                                    /* pattern entry */
      int         num_args;                /* number of arguments this entry */
      ARG_SET   * arg_list;              /* list of arguments for this entry */
      struct    entry_set * next_entry;              /* points to next entry */
    } ENTRY_SET;

	/* The following structure is an entry in the hash table which
	   ui routines use to store interface information. */

    typedef struct htable {
      const char * pattern;                              /* pattern to match */
      const char * entry;                       /* actual entry made by user */
      int         max_entries,         /* maximum number entries per pattern */
                  duplicates,    /* how to handle args for duplicate entries */
                  min_args,                        /* miminum number of args */
                  max_args,                        /* maximum number of args */
                  num_entries,                    /* total number of entries */
                  num_args;                  /* total number args this entry */
      const char * legal_args;                  /* description of legal args */
      ARG_SET  	* arg_list;                  /* list of arguments this entry */
      ENTRY_SET * entry_list;                /* list of entries this pattern */
      struct    htable * next_pattern;           /* multi-pattern for hash # */
    } HTABLE;

    static HTABLE * htbl;                    /* points to head of hash table */

    static const char * save_progname;  	  /* keeps previous progname */

    static unsigned auto_pos,                    /* holds positon of AUTO_OP */
                    info_pos,                    /* holds positon of INFO_OP */
                    verbosity;                 /* current level of verbosity */

    static BOOLEAN  from_ui_load;           /* is call to load from ui_load  */
    extern const char * progname;                     	  /* program running */

/*                       RETURN VALUES OF FUNCTIONS                          */

    static void	  	  init_htable (int, char **);
    static unsigned	  hvalue (const char * );
    static void	  	  init_one (HTABLE *, const char *, int),
			  load_htable (int, UIINIT *),
			  test_conflicts (UIINIT *);
    static HTABLE	* match_pattern (const char *);
    static BOOLEAN	  rc_file_options (int, char **, int *, const char ***),
			  get_local_rcfile (char *, char **, const char *),
			  get_dotsbx_options (const char *, const char *,
                                              int *, const char ***);
    static char 	* get_options (FILE *, const char *);
    static BOOLEAN	  next_token (char **, char token[]);
    static void	  	  skip_white_space (char **);
    static void  	  load_options (int, const char **, int, UIINIT *);
    static char 	* entry_pattern (const char *, UIINIT *, int),
			* built_ins (const char * );
    static BOOLEAN	  special_case (const char * );
    static int	  	  load_entry (HTABLE *, ENTRY_SET *, const char **,
                                      const char *, int, int *),
               	  	  load_min_args (HTABLE *, const char **, ARG_SET **,
                                         int, int *, int * );
    static BOOLEAN	  in_arg_list (ARG_SET *, const char *);
    static void		  load_max_args (HTABLE *, const char **, ARG_SET **,
                                         int, int *, int *);
    static BOOLEAN	  match_arg_list (const char *, const char *);
    static int	  	  set_duplicates (HTABLE *, ENTRY_SET **, const char *);
    static void      	  match_entry (HTABLE *, const char *, ENTRY_SET **);
    static void		  print_htable (void),
			  print_internals (HTABLE *, ARG_SET *, ENTRY_SET *,
                                           int );
    static char * arg_value (ARG_SET *, int, int);
           void           ui_print_revision(void);
