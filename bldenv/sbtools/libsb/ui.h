/* @(#)84 1.1 src/bldenv/sbtools/libsb/ui.h, bldprocess, bos412, GOLDA411a 93/04/29 12:30:22 */
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
**      This header file is for the internal use with the interface library
**	routine.
 */
/*                       INCLUDES                                            */

#  include <ode/odedefs.h>
#  include <ode/versions.h>
#  include <varargs.h>

/*                       DEFINES                                             */

#  define  HTABLESIZE           53                     /* size of hash table */
#  define  TOKEN_LENGTH		500
#  define  WHITESPACEFN(S) 	(*S == ' ' || *S == '\t')

#  define  RCEXT		"rc"


/*                       GLOBAL DECLARATIONS                                 */

	/* The following structure is used to store a list of
           arguments for a single entry. */

    typedef struct arg_set {
      char      * arg;                                     /* entry argument */
      struct    arg_set * next_arg;                    /* points to next arg */
    } ARG_SET;

	/* The following structure is used to store a list of
           entries for a single pattern. */

    typedef struct entry_set {
      char      * entry;                                    /* pattern entry */
      int         num_args;                /* number of arguments this entry */
      ARG_SET   * arg_list;              /* list of arguments for this entry */
      struct    entry_set * next_entry;              /* points to next entry */
    } ENTRY_SET;

	/* The following structure is an entry in the hash table which
	   ui routines use to store interface information. */

    typedef struct htable {
      char      * pattern,                               /* pattern to match */
                * entry;                        /* actual entry made by user */
      int         max_entries,         /* maximum number entries per pattern */
                  duplicates,    /* how to handle args for duplicate entries */
                  min_args,                        /* miminum number of args */
                  max_args,                        /* maximum number of args */
                  num_entries,                    /* total number of entries */
                  num_args;                  /* total number args this entry */
      char      * legal_args;                   /* description of legal args */
      ARG_SET  	* arg_list;                  /* list of arguments this entry */
      ENTRY_SET * entry_list;                /* list of entries this pattern */
      struct    htable * next_pattern;           /* multi-pattern for hash # */
    } HTABLE;

    static HTABLE * htbl;                    /* points to head of hash table */

    static char	  * save_progname;  		  /* keeps previous progname */

    static unsigned auto_pos,                    /* holds positon of AUTO_OP */
                    info_pos,                    /* holds positon of INFO_OP */
                    verbosity;                 /* current level of verbosity */

    static BOOLEAN  from_ui_load;           /* is call to load from ui_load  */
    extern char   * progname;                     	  /* program running */

/*                       RETURN VALUES OF FUNCTIONS                          */

    static int	  	  init_htable ();
    static unsigned	  hvalue ();
    static void	  	  init_one (),
			  load_htable (),
			  test_conflicts ();
    static HTABLE	* match_pattern ();
    static BOOLEAN	  rc_file_options (),
			  get_local_rcfile (),
			  get_dotsbx_options ();
    static char 	* get_options ();
    static BOOLEAN	  next_token ();
    static void	  	  skip_white_space ();
    static int	  	  load_options ();
    static char 	* entry_pattern (),
			* built_ins ();
    static BOOLEAN	  special_case ();
    static int	  	  load_entry (),
               	  	  load_min_args ();
    static BOOLEAN	  in_arg_list ();
    static void		  load_max_args ();
    static BOOLEAN	  match_arg_list ();
    static int	  	  set_duplicates ();
    static void      	  match_entry ();
    static void		  print_htable (),
			  print_internals ();
    static char 	* arg_value ();

           void   	  ui_print ();

