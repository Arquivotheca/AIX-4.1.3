/*@(#)34  1.6.1.5  src/bos/usr/bin/lppchk/lppchk.h, cmdswvpd, bos411, 9428A410j 3/18/94 11:09:21*/
/*
 * COMPONENT_NAME: (CMDSWVPD) Software VPD
 *
 * FUNCTIONS: lppchk.h
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include        <stdio.h>               /* standard I/O definitions     */
#include        <errno.h>               /* standard error definitions   */
#include        <sys/stat.h>            /* file status access defs      */
#include        <sys/access.h>          /* access routine definitions   */
#include        <sys/vnode.h>           /* type codes from stat         */
#include        <strings.h>             /* declarations for string fctns*/
#include        <swvpd.h>               /* swvpd definitions            */
#include        <swvpd1.h>              /* more swvpd definitions       */
#include        "swvpd_str.h"           /* tables for enumeration terms */
#include        "swvpd_msg.h"           /* messages equates             */


/************************************************************************/
/* Defines used for local copies of message text.                       */
/************************************************************************/


#define DEF_CHK_USAGE0 "\
%1$s Usage:\n\
%2$s -{f|c|l|v} [-u] [-O {[r][s][u]}] [-mn] [fileset [filelist]]\n \
\t-f  Fast check (file existence, file length)\n \
\t-c  Checksum verification\n \
\t-v  Fileset version consistency check\n \
\t-l  File link verification\n \
\t-u  Update inventory (only valid with -c or -l)\n \
\t-O  Data base(s) to be processed, default is all\n \
\t    u = /usr/lib/objrepos, r = /etc/objrepos,\n \
\t    s = /usr/share/lib/objrepos\n \
\t-mn n=1-3 controls detail of messages, 3 is most verbose\n \
\tfileset specifies filesets to be checked, may include\n \
\t    \"*\", etc to specify multiple filesets\n \
\tfilelist one or more file names (optionally using \"*\", etc.)\n \
\t    to be checked.  May be in form  'member:archive' to specify\n \
\t    archive members that are to be checked.\n \
\t--- one and only one of the flags -f, -c, -l and -v may be specified\n \
\t--- filelist is not allowed with -v option\n"

#define DEF_CHK_BAD_REP "\
%1$s:  Internal error - unexpected code for database - %2$c.\n"

#define DEF_CHK_BAD_OPT "\
%1$s:  Internal error - unexpected function code - %2$c.\n"

#define DEF_CHK_VPDERR "\
%1$s:  Error reading lpp database - rc = %2$d, odmerrno = %3$d.\n"

#define DEF_CHK_UPDATE "\
%1$s:  Inventory entry for file %2$s updated\n \
\twith size %3$d and checksum %4$d.\n"

#define DEF_CHK_NOPERM "\
%1$s:  Unable to access file %2$s because of permissions.\n"

#define DEF_CHK_NOFILE "\
%1$s:  File %2$s could not be located.\n"

#define DEF_CHK_NEWSZ  "\
%1$s:  Size value for %2$s changed from %3$d to %4$d.\n"

#define DEF_CHK_BADSZ  "\
%1$s:  Size of %2$s is %3$d, expected value was %4$d.\n"

#define DEF_CHK_NOCKSUM "\
%1$s:  File %2$s has no checksum in the inventory, \n \
\tno verification can be done.\n"

#define DEF_CHK_RDPERM "\
%1$s:  Unable to read file %2$s because of permissions.\n"

#define DEF_CHK_NEWSUM "\
%1$s:  Changing checksum for file %2$s, from %3$d to %4$d.\n"

#define DEF_CHK_BADCK "\
%1$s:  The checksum for file %2$s is %3$d,\n \
\texpected value is %4$d.\n"

#define DEF_CHK_NOPIPE "\
%1$s:  Unable to open pipe needed to check archive, error was %2$d.\n"

#define DEF_CHK_NOFORK "\
%1$s:  Unable to fork process needed to check archive, error was %2$d.\n"

#define DEF_CHK_ARERR "\
%1$s:  Program %2$s returned the following error text.\n"

#define DEF_CHK_EXEC  "\
%1$s:  Unable to exec the program %2$s, error code %3$d.\n"

#define DEF_CHK_VNOTFND "\
%1$s:  The fileset %2$s, level %3$d.%4$d.%5$d %6$s,\n\
\twas present in the %7$s data but not in %8$s. \n"

#define DEF_CHK_VSTATE "\
%1$s:  The fileset %2$s, level %3$d.%4$d.%5$d %6$s,\n \
\tis not uniformly applied to the system.  It is in\n \
\tstate %7$s in %8$s but in state %9$s in %10$s.\n\
\tThis will probably function properly but should be corrected.\n"

#define DEF_CHK_VINST "\
%1$s:  The fileset %2$s, level %3$d.%4$d.%5$d %6$s, \n \
\tis not uniformly applied to the system. It is in \n \
\tstate %7$s in %8$s but in state %9$s in %10$s.\n\
\tThis may cause the system to function incorrectly.\n"

#define DEF_CHK_LNOSRC "\
%1$s:  No link found from %2$s to %3$s.\n"

#define DEF_CHK_LRLERR "\
%1$s:  Unable to read existing link at %2$s.\n"

#define DEF_CHK_LNEQ "\
%1$s:  Existing symbolic link at %2$s is not a link to %3$s.\n"

#define DEF_CHK_LNLNK "\
%1$s:  Existing entry at %2$s is not a link to %3$s.\n"

#define DEF_CHK_LNEWE "\
%1$s:  An error occurred while creating the link from %2$s to %3$s. \n\
\tThe error code was %4$d.\n"

#define DEF_CHK_LNEW "\
%1$s:  New link created from %2$s to %3$s.\n"

#define DEF_CHK_NOLPP "\
%1$s:  No fileset entries were found that match the name - %2$s.\n"

#define DEF_CHK_FC0 "\
%1$s:  No files were found that matched the file name given for fileset %2$s.\n"

#define DEF_CHK_PRQ "\
%1$s:  The prerequsites for %2$s, %3$d,%4$d.%5$d.%6$d %7$s,\n\
were not satisfied by the current status of the system.\n\
This should be corrected to assure proper operation.\n"

#define DEF_CHK_QPRQ "\
%1$s:  The prerequsites for %2$s, %3$d,%4$d.%5$d.%6$d %7$s,\n\
were satisfied by the current status of the system but at\n\
least one tested fileset was not committed, while this fileset\n\
is already committed.  This probably needs to be corrected to\n\
ensure proper operation of the system.\n"

#define DEF_CHK_PROGRESS "\
%1$s: %2$d files have been checked.\n"

#define DEF_CHK_PRQH "\
%1$s: The following errors were found while verifying\n\
\tfileset prerequisites.\n"

#define DEF_CHK_PRQP "\
\nFileset  %1$s (%2$s part)\n\
at level %3$02d.%4$02d.%5$04d.%6$04d %7$s requires:\n%8$s"

#define DEF_CHK_PRQAL "\
%1$s%2$sAt least %3$d of:\n%4$s"

/* Return codes, message level codes, trace level codes, etc            */

#define CHK_OK 0                /* normal return code                   */
#define CHK_BAD_RET 1           /* error return value                   */
#define CHK_UPDT 2              /* return from verify rtns ==> update   */
                                /* the inventory record as changed      */


#define ARCHIVE_PROG_PATH               "/usr/ccs/bin/ar"
#define ARCHIVE_PROG_NAME               "ar"
#define ARCHIVE_PROG_ARG                "p"

 /* Indexes used with the array of file descriptors returned by pipe() */
#define READ_END        0
#define WRITE_END       1

#define MSG_INFO   3
#define MSG_WARN   2
#define MSG_ERROR  1
#define MSG_SEVERE 0

#define TRC_ALL    9
#define TRC_ENTRY  6
#define TRC_EXIT   3
#define TRC_ERR    2

/* coding assist macros                                                 */

/* invoke message output, depends on msg_lev being defined in current   */
/* context,                                                             */

#define MSG_S(a,b,c,d,e,f) \
if (a<=msg_lev) {fprintf(stderr,vpd_catgets(MSGS_CHK,b,c),lppchk_pn,d,e,f);}

#define MSG_L(a,b,c,d,e,f,g,h,i,j,k,l) \
if (a<=msg_lev) {fprintf(stderr,vpd_catgets(MSGS_CHK,b,c),lppchk_pn,\
d,e,f,g,h,i,j,k,l);}

#define TRC(a,b,c,d,e) \
if (a<=trc_lev) {fprintf(stderr,b,c,d,e);}

/* function prototypes                                                  */

char *vpd_catgets( int, int, char *) ;

/* private type structures                                              */

struct idl                      /* array of lpp_id's                    */
  { int id[1]; } ;              /* actual array size dynamically set    */

struct lpp_ids                  /* structure to hold the lpp_id values  */
  {                             /* for a set of LPPs which match the    */
  int   idcnt ;                 /* current lppname parameter            */
  struct idl *ida  ;            /* pointer to array of integers         */
  } ;


/* global variables - external to all lppchk functions                  */

  char          *lppchk_pn ;    /* pointer to program name              */
  int           msg_lev ;       /* message level requested              */
  int           trc_lev ;       /* trace level requested                */
  int           chk_lpp_cnt ;   /* count LPP matches found              */
  int           chk_file_cnt ;  /* count files processed                */
