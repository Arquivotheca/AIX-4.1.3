$ static char sccsid[] = "@(#)04  1.10  src/bos/usr/bin/sccs/admin.msg, cmdsccs, bos411, 9428A410j 7/9/93 12:48:59";
$
$ COMPONENT_NAME: CMDSCCS       source code control system
$
$ FUNCTIONS:
$
$ ORIGINS: 27
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1989, 1993
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$

$quote "         define quote character.

$len 
$ DO NOT TRANSLATE the (ad#) at the end of each of these lines.
$ DO NOT TRANSLATE SCCS, SID, admin:, 
$ DO NOT TRANSLATE %c, %s, %1$s, etc...
$ DO NOT TRANSLATE -i, -y, -t, -m, -n, -d, etc..
$set 4        Define initial set#

DNOARG      "admin: 1255-001 Specify a header flag with the -d flag. (ad1)\n"
FLGNOVAL2   "admin: 1255-002 Specify a value for the %c header flag. (ad2)\n"
UNKNWNFLG   "admin: 1255-003 %c is not a recognized header flag. (ad3)\n\
Usage: admin {-n|-i[Name]} [-f{l{a|Number ...}|\n\
\t{b|cNumber|dSID|fNumber|i|j|mModule|n|qText|tType}} ...]\n\
\t[-fv[Program] -m[MRList]] [-aUser ...] [-rNumber.Number]\n\
\t[-t File] [-y[Comment]] NewFile ...\n\
\tCreates and initializes SCCS files.\n\
\n\
\tUsage: admin [-aUser ...] [-eUser ...] [-f{l{a|Number ...}|\n\
\tb|cNumber|dSID|fNumber|i|j|mModule|n|qText|tType} ...]\n\
\t[-d{l{a|Number ...}|b|c|d|f|i|j|m|n|q|t ...] [-dv|-fv[Program]]\n\
\t[-t[File]] File ...\n\
\tChanges existing SCCS files.\n"
FLGTWICE    "admin: 1255-004 Cannot use the %c header flag more than once. (ad4)\n"
FFLGNOARG   "admin: 1255-005 Specify a header flag with the -f flag. (ad5)\n"
AFLGARG     "admin: 1255-006 Specify a user name, group name or group number with\n\
\tthe -a flag. (ad8)\n"
AFLGKEYLTRS "admin: 1255-007 Cannot use the -a flag more than 10 times\n\
\ton each command line. (ad9)\n"
BADEARG     "admin: 1255-008 Specify user name, group name or group number to\n\
\tremove. (ad10)\n"
EFLGKEYLTRS "admin: 1255-009 Cannot use the -e flag more than 10 times\n\
\ton each command line. (ad11)\n"
CFLGVAL     "admin: 1255-010 The %c header flag cannot have a value\n\
\twhen used with the -d flag. (ad12)\n"
FLGVAL13    "admin: 1255-011 The %c header flag cannot have a value\n\
\twhen used with the -f flag. (ad13)\n"
NODFLTSID   "admin: 1255-012 Specify an SID number with the d header flag. (ad14)\n"
$ Do not translate SID or d
MTONEFIL    "admin: 1255-013 Create only one SCCS file at a time with the -i flag. (ad15)\n"
$ Do not translate SCCS or -i.
RONLYIORN   "admin: 1255-014 Use the -r flag only with the -i flag or the -n flag. (ad16)\n"
TFLGNOARG   "admin: 1255-015 Specify a file name for the -t flag when creating\n\
\ta new SCCS file. (ad17)\n\
."
NODWITHN    "admin: 1255-016 Cannot use the -d flag with the -n  or -i flags. (ad18)\n"
FILEEXISTS  "admin: 1255-017 File %s exists. Cannot use the -n flag or\n\
\tthe -i flag with an existing file. (ad19)\n"
FLEXST20    "admin: 1255-018 File %s does not exist. (ad20)\n"
ILLGLDATA   "admin: 1255-019 %1$s contains illegal data on line %2$d.\n\
\t (ad21)\n"
$ %1$s is the file name.  %2$s is the line number. 
FLOORNOTNUM "admin: 1255-020 Specify a value between 0 and 10000\n\
\twith the f header flag. (ad22)\n"
FLOORRNG    "admin: 1255-021 Specify a value between 0 and 10000\n\
\twith the f header flag. (ad23)\n"
CEILNONNUM  "admin: 1255-022 Specify a value between 0 and 10000\n\
\twith the c header flag. (ad24)\n"
CEILRNG     "admin: 1255-023 Specify a value between 0 and 10000\n\
\twith the c header flag. (ad25)\n"
DIRIKYLTR   "admin: 1255-024 Specify a file, not a directory, with the -i flag. (ad26)\n"
BADLISTFMT  "admin: 1255-025 The version and release must be numbers\n\
\tseparated by a comma. (ad27)\n"
ELMNTRNG    "admin: 1255-026 Cannot lock a version number or a release number\n\
\tgreater than 9999. (ad28)\n"
BDIR        "admin: 1255-027 Cannot use the -%2$c flag with directory %1$s.\n\
\tSpecify a file, not a directory, with the -i flag or the -t flag. (ad29)\n"
$ %2$c is a flag. %1$s is a directory. 
ILLYORM     "admin: 1255-028 Use the -y flag or the -m flag only when creating\n\
\tan SCCS file. (ad30)"
BYMSG       "by"
DATETIME    "date and time created"
LINECNTWARN "admin: WARNING - The number of lines in this file exceeds\n\
\t99999.  The header section of the s. file will record 99999 in the\n\
\tlines inserted field.\n"
$ The source file line count exceeds 5 digits. 
$ The s. file has a 5 digit limit so it will record 99999. 
CEILCONS    "admin: 1255-037 The ceiling value %1$u is less than the lowest\n\
\trelease(the floor). (ad31)\n"
FLORCONS    "admin: 1255-038 The floor value %1$u is greater than the highest\n\
\trelease(the ceiling). (ad32)\n"
LINE2LONG   "admin: 1255-030 %1$s contains a line longer than 2048 characters on line %2$d."
