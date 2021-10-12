/* @(#)51       1.1  src/bldenv/sbtools/include/ode/odedefs.h, bldprocess, bos412, GOLDA411a 1/19/94 17:36:09
 *
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
 * $Log: odedefs.h,v $
 * Revision 1.2.9.1  1993/09/01  16:41:20  damon
 * 	CR 637. Fixed workon usage message
 * 	[1993/09/01  16:41:01  damon]
 *
 * Revision 1.2.6.14  1993/05/25  15:08:59  damon
 * 	CR 395. Added HOLD_FILE_23
 * 	[1993/05/25  14:57:53  damon]
 * 
 * Revision 1.2.6.13  1993/05/14  15:00:19  damon
 * 	CR 481. Added UPGRADE_OP for mksb
 * 	[1993/05/14  14:59:38  damon]
 * 
 * Revision 1.2.6.12  1993/04/28  18:26:22  damon
 * 	CR 463. Pedantic changes
 * 	[1993/04/28  18:12:00  damon]
 * 
 * Revision 1.2.6.11  1993/04/26  19:52:31  damon
 * 	CR 415. Added context and whitespace options
 * 	[1993/04/26  19:51:56  damon]
 * 
 * Revision 1.2.6.10  1993/04/26  15:33:18  damon
 * 	CR 436. Changed HOLD_FILE to use ode2.3_server_base
 * 	[1993/04/26  15:27:44  damon]
 * 
 * Revision 1.2.6.8  1993/04/16  20:41:33  damon
 * 	CR 421. Moved more defines for some switches here
 * 	[1993/04/16  20:41:25  damon]
 * 
 * Revision 1.2.6.7  1993/04/16  20:26:19  damon
 * 	CR 421. Moved defines for some switches here
 * 	[1993/04/16  20:26:14  damon]
 * 
 * Revision 1.2.6.6  1993/04/16  19:03:49  damon
 * 	CR 422. Updated usage messages and options
 * 	[1993/04/16  19:03:34  damon]
 * 
 * Revision 1.2.6.5  1993/04/08  16:19:17  damon
 * 	CR 446. Clean up include files
 * 	[1993/04/08  16:18:36  damon]
 * 
 * Revision 1.2.6.4  1993/02/03  18:46:48  damon
 * 	CR 253. Now use checklocksb to see if build is locked
 * 	[1993/02/03  18:37:20  damon]
 * 
 * Revision 1.2.6.3  1993/01/28  23:34:59  damon
 * 	CR 417. Added SBCONF_RC for sbdata.c
 * 	[1993/01/28  23:34:48  damon]
 * 
 * Revision 1.2.6.2  1993/01/13  17:45:40  damon
 * 	CR 382. Moved define of STATIC to here
 * 	[1993/01/13  17:45:07  damon]
 * 
 * Revision 1.2.2.13  1992/12/03  19:14:02  damon
 * 	ODE 2.2 CR 346. Expanded copyright
 * 	[1992/12/03  18:43:13  damon]
 * 
 * Revision 1.2.2.12  1992/11/13  15:20:19  root
 * 	Include stdlib.h and unistd.h
 * 	[1992/11/13  15:02:37  root]
 * 
 * Revision 1.2.2.11  1992/11/11  15:50:23  damon
 * 	CR 329. Removed PROTO stuff. Moved func decs to portable.h
 * 	[1992/11/11  15:49:50  damon]
 * 
 * Revision 1.2.2.10  1992/11/09  22:01:42  damon
 * 	296. Added forward decl. of skipto and skipover
 * 	[1992/11/09  22:01:16  damon]
 * 
 * Revision 1.2.2.9  1992/11/06  18:16:08  damon
 * 	CR 329. Added forward decl. of strdup
 * 	[1992/11/06  18:14:58  damon]
 * 
 * Revision 1.2.2.8  1992/11/05  17:26:58  damon
 * 	CR 315. Updated with common usage messages.
 * 	[1992/11/05  17:26:42  damon]
 * 
 * Revision 1.2.2.7  1992/11/02  21:24:49  damon
 * 	CR 265
 * 	[1992/11/02  21:24:34  damon]
 * 
 * Revision 1.2.2.6  1992/10/29  17:01:35  damon
 * 	CR 321. Added some defines for libode/lock_sb.c
 * 	[1992/10/29  16:58:34  damon]
 * 
 * Revision 1.2.2.5  1992/09/24  19:01:07  gm
 * 	CR282: Made more portable to non-BSD systems.
 * 	[1992/09/21  20:34:35  gm]
 * 
 * Revision 1.2.2.4  1992/08/06  16:11:12  damon
 * 	CR 265. Added POSIX_ARGS_OP
 * 	[1992/08/06  16:10:58  damon]
 * 
 * Revision 1.2.2.3  1992/06/22  19:53:56  damon
 * 	CR 180. Removed redefinition of sleep
 * 	[1992/06/22  19:52:39  damon]
 * 
 * Revision 1.2.2.2  1992/06/17  12:36:02  damon
 * 	Fixed bug 48. Removed -nosh option.
 * 	[1992/06/17  12:33:24  damon]
 * 
 * Revision 1.2  1991/12/05  21:03:53  devrcs
 * 	Changed COPYHOLD to COPYFILE
 * 	[1991/10/30  21:49:00  damon]
 * 
 * 	Added COPYHOLD for logsubmit.
 * 	[91/09/05  11:17:54  damon]
 * 
 * 	Added define of ABORT
 * 	[91/08/10  18:36:57  damon]
 * 
 * 	split off portability macros and definitions into separate <ode/portable.h>
 * 	[91/07/25  14:41:44  ezf]
 * 
 * 	Changed std_defs.h to odedefs.h
 * 	[91/01/09  15:18:05  randyb]
 * 
 * $EndLog$
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
*/

#ifndef _ODEDEFS_H
#define _ODEDEFS_H

/******************************************************************************

                         INCLUDES					     

******************************************************************************/
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

#  define  STATIC		static
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
#  define  HOLD_FILE    "bsubmit.hold"                         /* files held */
#  define  HOLD_FILE_23 "ode2.3_server_base/bsubmit.hold"      /* files held */
#  define  LOCK_SB      "lock_sb"                     /* dir to lock sandbox */
#  define  LOCK_HOLD    "lock_hold"         /* dir to lock bsubmit hold file */
#  define  LOCK_LOGS    "lock_logs"              /* dir to lock bsubmit logs */
#  define  MKCONF_LINK  "src/Makeconf"           /* marker for top of source */
#  define  PROJECTS	"rc_files/projects"  	      /* sb rc projects file */
#  define  SHARED_RC    "rc_files/shared"      /* location of shared rc file */
#  define  SHARED_T_RC  "rc_files/shared.tmpl"    /* template shared rc file */
#  define  SANDBOXRC    ".sandboxrc"            /* files to know location of */
#  define  SET_RC       "rc_files/sets"          /* location of sets rc file */
#  define  SBCONF_RC    "sb.conf"       /* location of sets rc file */
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
#  define  TOOL_DIR     "tools"

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
#  define  SOURCE_BASE  "source_base"              /* directory where src is */
#  define  SUBMIT_DEFECT "submit_defect"    /* does submission need defect # */
#  define  SUBMIT_HOST  "submit_host"                 /* server to submit to */
#  define  SUBMIT_BASE  "submit_base"          /* directory on submit server */


			/* commands */

#  define  BCS		"bcs"
#  define  BCI		"bci"
#  define  BCO		"bco"
#  define  BLOG		"blog"
#  define  BMERGE	"bmerge"
#  define  BSTAT	"bstat"

			/* command line options */
#define ALL_REVS_OP	"-A"
#define ARGS_OP	"!-*"
#define POSIX_ARGS_OP	"*"
#define ADD_OP       "-add"
#define ALL_OP       "-all"
#define AUTO_OUT_OP	"-autooutdate"
#define BACK_OP	"-back"
#define BACKING_OP       "-b"
#define BIG_SYMNAME_OP  "-N"
#define C_OP		"-c"
#define CHANGED_OP	"-changed"
#define COMMENT_LEADER_OP       "-c"
#define COMMON_OP	"-common"
#define CONTEXT_OP	"-c"
#define COPY_OP	"-copy"
#define DATE_OP      "-date"
#define DEF_OP		"-def"
#define DEFUNCT_OP	"-defunct"
#define DIR_OP		"-dir"
#define ECHO_OP	"echo"
#define EDIT_OP	"-edit"
#define FAST_OP	"-fast"
#define GETREV_OP       "-V"
#define HEADER_OP	"-h"
#define I_OP		"-i"
#define L_OP		"-L"
#define LONG_OP         "-L"
#define LIST_OP		"-list"
#define LOCK_OP		"-lock"
#define LOCKUSERS_OP	"-l"
#define LOG_OP		"-log"
#define M_OP		"-m"
#define MSG_OP		"-m"
#define NOLOG_OP	"-nolog"
#define NOWRITE_OP	"-nowrite"
#define OUTDATE_OP	"-o"
#define WRITE_OP	"-okwrite"
#define RCSFILE_OP      "-R"
#define REVISION_OP	"-r"
#define REVISION2_OP	"-R"
#define R_OP		"-R"
#define RCONLY_OP	"-rconly"
#define RM_OP        "-rm"
#define Q_OP		"-q"
#define SB_OP        "-sb"
#define SET_OP       "-set"
#define SETDIR_OP    "-setdir"
#define SAVED_OP	"-saved"
#define SML_SYMNAME_OP  "-n"
#define SUBDIR_OP    "-subdir"
#define FUNLOCK_OP   "-u"
#define UNDO_OP	"-undo"
#define UNLOCK_OP	"-unlock"
#define UPGRADE_OP	"-u"
#define USER_OP      "-user"
#define VC_OP	"-V"
#define V_OP         "-v"
#define WHITESPACE_OP	"-w"
#define XLOG_OP	"-xlog"

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
#  define  LOCKLOG      "locklog"
#  define  UNLOCKLOG    "unlocklog"
#  define  CHECKLOCKSB  "checklocksb"
#  define  LOCKSB       "locksb"
#  define  UNLOCKSB     "unlocksb"
#  define  COPYFILE     "copyfile"
#  define  COPYLOG      "copylog"
#  define  ADMINCOPY    "admincopy"
#  define  APPENDHOLD   "appendhold"
#  define  UPDATELOGS   "updatelogs"
#  define  HOLDCLEANUP  "holdcleanup"

#  define NOWRITE         (~0222) /* read-only */
#  define MODEMASK        0777    /* file permission bit field */
#  define BASEYEAR        1900    /* offset for tm_year */

/*
 * Usage messages
 */
#  define USAGE_VER_USAGE "[ -version | -usage ]"
#  define USAGE_SB_OPTS "\tsandbox opts:\n\t  -sb <sandbox>, -set <set>, -rc <user rc>"
#  define USAGE_ODE_OPTS "\tODE opts:\n\t  -auto -debug -quiet -normal -verbose"
#  define USAGE_FILE_OPTS "\tfile opts:\n\t  [-changed | -saved] -all | <file>..."

/******************************************************************************

		         TYPEDEFS

******************************************************************************/

typedef    int   BOOLEAN;                   /* distinguishes type of integer */

/*
 * Prototype for print_usage()
 */
void print_usage( void );

#endif
