/* @(#)22 1.3 src/bldenv/sbtools/include/odedefs.h, bldprocess, bos412, GOLDA411a 93/12/04 18:56:13 */
/*
 *   COMPONENT_NAME: BLDPROCESS
 *
 *   FUNCTIONS: max
 *		min
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
**	This header file is for the sandbox programs.  It contains the common
**	includes and defines.
**
**    Changes:
**      04/09/93 - Michael Winestock
**                 Added definition of INST_DIR after SHIP_DIR.
**      09/03/93 - Alan Christensen
**                 Removed prolog for sleep (with long return value)
**      12/03/93 - Alan Christensen
**                 Create hostexp directory (to build tools in sandbox)
**      12/03/93 - Michael Winestock
**                 Changed TOOL_DIR to ode_tools (to build tools in sandbox)
*/
/******************************************************************************

                         INCLUDES					     

******************************************************************************/

#  include <sys/types.h>
#  include <sys/dir.h>
#  include <sys/file.h>
#  include <string.h>
#  include <stdio.h>
#  include <ode/interface.h>
#  include <ode/portable.h>

/******************************************************************************

                         DEFINES

******************************************************************************/

	/* integers */

#ifndef TRUE
#  define  TRUE			1
#endif

#ifndef FALSE
#  define  FALSE		0
#endif

#  define  ERROR		-1
#  define  OK			0
#  define  ABORT		-2
#  define  CHILD		0

#  define  NAME_LEN             50
#  define  STRING_LEN           256

#ifndef PATH_LEN
#  define  PATH_LEN             1024	/* should use MAXPATHLEN instead */
#endif

	/* chars */

#  define  NUL                  '\0'
#  define  SPACE                ' '
#  define  TAB                  '\t'
#  define  DASH           	'-'
#  define  NEWLINE      	'\n'
#  define  COLON          	':'
#  define  ESC            	''
#  define  CTRL_D         	''
#  define  BACKSPACE      	'\b'
#  define  SLASH      		'/'
#  define  PERIOD      		'.'
#  define  COMMA      		','
#  define  AT_SIGN     		'@'
#  define  STAR        		'*'


	/* strings */

#  define  EMPTY_STRING         ""              /* empty string with no <cr> */
#  define  WHITESPACE           " \t\n"            /* white space characters */
#  define  STAR_ST              "*"            /* universal matching pattern */
#  define  CR_STRING            "\n"                /* string with only <cr> */
#  define  YES            	"y"
#  define  NO             	"n"
#  define  SYS                  "SYSTEM"              /* for system error id */

#  define  READ                 "r"                                  /* open */
#  define  WRITE                "w"                        /* create or open */
#  define  APPEND               "a"     /* create or open for writing at eof */
#  define  O_UPDATE             "r+"         /* open for reading and writing */
#  define  C_UPDATE             "w+"    /* create/open for reading & writing */
#  define  A_UPDATE             "a+"  /* create/open for read & write at eof */

	/* macros */

#  define  max(A,B)     ((A) > (B) ? (A) : (B))          /* maximum function */
#  define  min(A,B)     ((A) < (B) ? (A) : (B))          /* minimum function */

	/* for ODE defines */

			/* misc needs */

#  define DEFAULT_BLIST "/project/projects/build_list"
	/* default build list is hard-coded but is overridden by rc variable */
#  define  BCSSET       "BCSSET"                     /* environment variable */
#  define  BCS_SET_L    8                        /* length of BCS_SET define */
#  define  EDITOR       "EDITOR"                     /* environment variable */
#  define  SANDBOX      "SANDBOX"                    /* environment variable */

			/* location and/or names of files */

#  define  BCSCONFIG    ".BCSconfig"                /* file with config info */
#  define  BCSLOCK      ".BCSlock"               /* file with bcs lock in it */
#  define  BCSLOG       ".BCSlog-"                /* file with bcs log in it */
#  define  BCSPATH      ".BCSpath-"              /* file with bcs path in it */
#  define  BCS_SET      ".BCSset-"               /* file with bcs co's in it */
#  define  BUILD_LIST	"build_list"              	  /* rc file keyword */
#  define  DEFUNCT      "DEFUNCT"                    /* name of defunct file */
#  define  LOCAL_RC     "rc_files/local"        /* location of local rc file */
#  define  LOCAL_T_RC   "rc_files/local.tmpl"      /* template local rc file */
#  define  HOLD_FILE    "bsubmit.hold"              /* files held by bsubmit */
#  define  LOCK_HOLD    "lock_hold"         /* dir to lock bsubmit hold file */
#  define  LOCK_LOGS    "lock_logs"              /* dir to lock bsubmit logs */
#  define  MKCONF_LINK  "src/Makeconf"           /* marker for top of source */
#  define  PROJECTS	"rc_files/projects"  	      /* sb rc projects file */
#  define  SHARED_RC    "rc_files/shared"      /* location of shared rc file */
#  define  SHARED_T_RC  "rc_files/shared.tmpl"    /* template shared rc file */
#  define  SANDBOXRC    ".sandboxrc"            /* files to know location of */
#  define  SET_RC       "rc_files/sets"          /* location of sets rc file */
#  define  SNAPSHOT     "SNAPSHOT"                           /* name of file */
#  define  SUBLOG       "bsubmit.log"            /* permenent build log file */
#  define  TMP_DEFUNCT  "Tdefunct"                 /* temporary defunct file */
#  define  TMP_HOLD     "Thold"                       /* temporary hold file */
#  define  TMP_LOG      "Tlog"                         /* temporary log file */
#  define  TMP_SNAP     "Tsnapshot"               /* temporary snapshot file */

			/* build and sandbox directories */

#  define  LINK_DIR     "link"
#  define  EXP_DIR      "export"
#  define  OBJ_DIR      "obj"
#  define  RC_DIR       "rc_files"
#  define  SRC_DIR      "src"
/* ------------------ Commented out by Michael Winestock -------------------
#  define  TOOL_DIR     "tools"
   ------------------------------------------------------------------------- */
/* ---------------------- Added by Michael Winestock ----------------------- */
#  define  TOOL_DIR     "ode_tools"
/* ------------------------------------------------------------------------- */
#  define  SHIP_DIR     "ship"
#  define  INST_DIR     "inst.images"

/* ------------------ Added by Alan Christensen        --------------------- */
#  define  HOST_DIR     "hostexp"
/* ------------------------------------------------------------------------- */

#  define  SPOW_DIR     "ship/power"
#  define  POW_DIR      "src/POWER"
#  define  PBOOT_DIR    "src/POWER/boot"


			/* key words */

#  define  BASE         "base"
#  define  DEFAULT      "default"
#  define  SB           "sb"
#  define  SET_KEY      "set"                    /* key word in sets rc file */
#  define  REPLACE      "replace"
#  define  DEFUNCT_MARK "defunct"         /* "Rev" to look for in .BCSconfig */

			/* rc file values */

#  define  BUILD_BASE   "build_base"            /* base for project's builds */
#  define  DEF_BUILD    "default_build"           /* project's default build */
#  define  DEF_SET      "default_set"               /* project's default set */
#  define  SB_BASE      "sandbox_base"         /* directory where sandbox is */
#  define  SOURCE_COVER "source_cover"          /* program to run for source */
#  define  SOURCE_BASE  "source_base"              /* directory where src is */
#  define  SUBMIT_COVER "submit_cover"     /* program to run for submissions */
#  define  SUBMIT_DEFECT "submit_defect"    /* does submission need defect # */
#  define  SUBMIT_HOST  "submit_host"                 /* server to submit to */
#  define  SUBMIT_BASE  "submit_base"          /* directory on submit server */
#  define  SUBMIT_OWNER "submit_owner"               /* owner of submissions */


			/* commands */

#  define  BCS		"bcs"
#  define  BCI		"bci"
#  define  BCO		"bco"
#  define  BLOG		"blog"
#  define  BMERGE	"bmerge"
#  define  BSTAT	"bstat"

			/* command line options */

#  define  ARGS_OP	"!-*"
#  define  ADD_OP       "-add"
#  define  ALL_OP       "-all"
#  define  AUTO_OUT_OP	"-autooutdate"
#  define  BACK_OP	"-back"
#  define  C_OP		"-c"
#  define  COPY_OP      "-copy"
#  define  DATE_OP      "-date"
#  define  DEF_OP	"-def"
#  define  DEFUNCT_OP   "-defunct"
#  define  DIR_OP	"-dir"
#  define  ECHO_OP	"echo"
#  define  EDIT_OP      "-edit"
#  define  I_OP		"-i"
#  define  L_OP		"-L"
#  define  LIST_OP	"-list"
#  define  LOCK_OP      "-lock"
#  define  LOG_OP	"-log"
#  define  M_OP		"-m"
#  define  NEWC_OP	"-newconfig"
#  define  NEWP_OP	"-newpath"
#  define  NOLOCK_OP	"-nolock"
#  define  NOLOG_OP	"-nolog"
#  define  NOSH_OP      "-nosh"
#  define  NOWRITE_OP   "-nowrite"
#  define  OUTDATE_OP	"-o"
#  define  WRITE_OP	"-okwrite"
#  define  PATH_OP	"-path"
#  define  R_OP		"-R"
#  define  RCONLY_OP	"-rconly"
#  define  RM_OP        "-rm"
#  define  Q_OP		"-q"
#  define  SB_OP        "-sb"
#  define  SET_OP       "-set"
#  define  SETDIR_OP    "-setdir"
#  define  SUBDIR_OP    "-subdir"
#  define  TRACK_OP     "-track"
#  define  FUNLOCK_OP   "-u"
#  define  UNDO_OP	"-undo"
#  define  UNLOCK_OP	"-unlock"
#  define  USER_OP      "-user"
#  define  VC_OP	"-V"
#  define  V_OP         "-v"
#  define  XLOG_OP	"-xlog"

			/* non-dash command line options */

#  define  BUILD_OPT	"build"
#  define  DEFUNCT_OPT	"defunct"
#  define  HOLD_OPT     "hold"
#  define  LOG_OPT      "log"
#  define  SNAP_OPT     "snap"
#  define  SNAPONLY_OPT "snaponly"

			/* names used to call to submit functions */

#  define  LOCKHOLD     "lockhold"
#  define  UNLOCKHOLD   "unlockhold"
#  define  COPYFILE     "copyfile"
#  define  LOCKLOG      "locklog"
#  define  UNLOCKLOG    "unlocklog"
#  define  COPYLOG      "copylog"
#  define  ADMINCOPY    "admincopy"
#  define  APPENDHOLD   "appendhold"
#  define  UPDATELOGS   "updatelogs"
#  define  HOLDCLEANUP  "holdcleanup"


/******************************************************************************

		         TYPEDEFS

******************************************************************************/

typedef    int   BOOLEAN;                   /* distinguishes type of integer */
                            

/******************************************************************************

		         RETURN VALUES OF FUNCTIONS

******************************************************************************/

  FILE        * fopen ();       /* standard function returns pointer to file */
  DIR         * opendir ();                                                     
  char        * gets (),      /* standard functions return pointers to chars */
	      *	getenv (),
	      *	mktemp (),
	      * nxtarg (),
	      * concat ();
  void          perror (),  /* standard functions which do not return values */
		exit (),
		rm_newline ();
