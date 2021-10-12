/* @(#)75 1.2 src/bldenv/sbtools/mksb/mksb.h, bldprocess, bos412, GOLDA411a 93/06/25 13:57:22 */
/*
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
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *
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
**                               April 1990                                  **
*******************************************************************************
**
**    Description:
**	This header file is for mksb.c
**
 */
/*                       INCLUDES					     */

#  include <ode/odedefs.h>
#  include <sys/param.h>
#  include <sys/signal.h>
#  include <sys/wait.h>
#  include <ode/parse_rc_file.h>

/*                       DEFINES					     */

#  define  MAX_ARGS	11
#  define  NEW		10
#  define  MODE		0775

#  define  HELP_GRP	"Release Engineering"
#  define  TOOLS_OP	"-tools"
#  define  OBJ_OP	"-obj"
#  define  SRC_OP	"-src"

#  define  DEF_MACH     "power"                       /* default machine list */
#  define  TMP_NAME     "/mksb.tmp"                   /* temporary file name */

#  define  RM           "rm"                             /* commands to exec */
#  define  WORKON	"workon"
#  define  MKLINKS	"mklinks"

#  define  BACK_CH      'b'                       /* default population mode */
#  define  COPY_CH	'c'
#  define  LINK_CH	'l'

#  define  DEFAULT_BUILD_LIST "/project/projects/build_list"

/*                       RETURN VALUES OF FUNCTIONS			     */

   char  /* ----my fix   * strrchr (), -----end of my fix */
		keep_prompt (),
	      *	build_base_dir ();
    FILE      * create_tmp_file ();

/*                       GLOBAL DECLARATIONS				     */

	/* structure to hold an entry from an rc file.*/

    typedef struct entry {
      char      target [ NAME_LEN ],  /* the target for the value (optional) */
                value [ PATH_LEN ];   /* the value for the keyword or target */
      struct    entry  * nextentry;                 /* ptr to the next entry */
    } ENTRY;

	/* structure to hold the contens of an rc file.*/

    typedef struct rc_content {
      char      default_sb [ NAME_LEN ];          /* name of default sandbox */
      ENTRY   * base;                              /* holds list of sb bases */
      ENTRY   * sb;                                /* holds list of sb names */
      char      def_sb_base [ PATH_LEN ],            /* default sandbox base */
                def_machines [PATH_LEN ],                /* default machines */
                tools,                              /* how to populate tools */
                objects,                          /* how to populate objects */
                sources,                          /* how to populate sources */
                obj_dirs [ PATH_LEN ],  /* list of dirs to populate obj with */
                src_dirs [ PATH_LEN ];  /* list of dirs to populate src with */
    } RC_CONT;
