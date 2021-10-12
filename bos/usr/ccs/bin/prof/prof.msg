$ @(#)65	1.14  src/bos/usr/ccs/bin/prof/prof.msg, cmdstat, bos411, 9428A410j 5/26/93 22:29:12
$
$ COMPONENT_NAME: (CMDSTAT) prof
$
$ FUNCTIONS:
$
$ ORIGINS: 27
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1989
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$
$ messages for command prof
$

$quote "
$set MS_PROF
$ Do not move the %s from the beginning of the line. It is replaced by prof:

NOTREC          "%s: 0551-081 %c is not a recognized flag.\n"

OVERRIDE        "%s: 0551-082 %c overrides previous specifications.\n"

ONLYOPT         "%s: 0551-083 The only option allowed after a.out is '-m mon.out ...'\n"
$ Do not translate a.out, '-m mon.out ...'

BADFOR          "%s: 0551-084 %s: The format is not correct.\n"
$ This is no longer used. Do not translate.
$ File is not in the proper Xcoff format, i.e. it is not an executable
$ for this machine.

TITLE           "%Time     Seconds     Cumsecs  #Calls   msec/call"
$ These are column headings. They must start in these columns.
$ Cumsecs is an abbreviation for cumulative seconds
$ #Calls is an abbreviation for number of calls
$ msec/call is an abbreviation of milliseconds per call

TITL            "Name              "
$ This message must be 18 characters long. After translating the word
$ Name, you must leave blanks in the line out to column 19.

CALLCNT         "%5d/%d call counts used\n"

QSYM            "%5d/%d symbols qualified"

ZEROCNT         "        %d had zero time and zero call-counts\n"
$ Leave 8 blanks at the start of this line.

FACTOR          "%#x scale factor\n"

CUSED           "%5d/%d call counts used\n"

SQ              "%5d/%d symbols qualified"

ZEROT           ", %d had zero time and zero call-counts\n"

SQFAC           "%#x scale factor\n"

EARLYEND        "%s: 0551-071 %s: Reached end of file.\n"
$ the mon.out file is corrupt.  Should generate a new one.

IERR            "%s: 0551-072 There is an internal error.\n"
$ ....SNH (SHOULD NEVER HAPPEN)

NOBASE          "No base text address found in a.out file.  assuming 0x10000000\n"
$ Do not translate a.out, 0x10000000

SF_CHANGED      "The scale factor is different per region.\n"

BADMON          "prof: 0551-073 mon.out %d file doesn't match.\n"
$ the mon.out file is corrupt.  Should generate a new one.
$ Do not translate mon.out

STR2BIG         "prof: 0551-074 The path names are too big in the loader table.\n"
$ pathname to object is too long.  rerun program in /tmp....

FILEHEAD        "prof: 0551-075 Cannot read the file header.\n"
$ loaded object is of incorrect format

NOACHIVE        "prof: 0551-076 ldahread failed. Not an archive.\n"
$ ....SNH (SHOULD NEVER HAPPEN)
$ Do not translate ldahread

SEEKER          "prof: 0551-077 The ldtbseek function failed.\n"
$ ....SNH (SHOULD NEVER HAPPEN)
$ Do not translate ldtbseek

NOSYM           "prof: 0551-078 Cannot read symbol: %d\n"
$ loaded object is of incorrect format

STR2LONG        "prof: 0551-079 The file name string is too long.\n"
$ pathname to object is too long.  rerun program in /tmp....

MEMBER          "prof: 0551-080 The member string is too long.\n"
$ pathname to object is too long.  rerun program in /tmp....

SCN             "prof: 0551-085 ldnshread failed.\n"
$ Do not translate ldnshread
$ this should never happen.

NOCNTS1         "prof: Warning: mon.out file has no call counts.  Program\n\
      possibly not compiled with profiled libraries.\n"
$ Do not translate mon.out

NOCNTS2         ""
$ This is a null message.

STRIPPED	"prof: %s(%s) is stripped.\n"
$ Do not translate stripped.
