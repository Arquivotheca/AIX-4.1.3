$ @(#)13	1.11  src/bos/usr/bin/sed/sed.msg, cmdedit, bos411, 9428A410j 12/17/93 10:16:55
$
$ COMPONENT_NAME: (CMDEDIT) sed.msg
$ 
$ FUNCTIONS: none
$ 
$ ORIGINS: 3, 10, 18, 27
$ 
$ This module contains IBM CONFIDENTIAL code. -- (IBM
$ Confidential Restricted when combined with the aggregated
$ modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1984, 1993
$ All Rights Reserved
$ 
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ 
$ (c) Copyright 1990 OPEN SOFTWARE FOUNDATION, INC.
$ ALL RIGHTS RESERVED
$ 
$ OSF/1 1.0
$ 
$
$quote "
$set    MS_SED
$len 
$       Do not translate sed.
$       Do not translate %s, %c, %d, etc. These will be replaced by file names,
$       commands, numbers, etc.
AD0MSG       "sed: 0602-400 Cannot specify an address with the %s function.\n"
AD1MSG       "sed: 0602-401 Function %s allows only one address.\n"
APPNDCNT     "sed: 0602-402 The file was not appended after line %ld.\n"
BADCMND      "sed: 0602-403 %s is not a recognized function.\n"
CGMSG        "sed: 0602-404 Function %s cannot be parsed.\n"
CMNDCNT      "sed: 0602-405 There are too many commands for the %s function.\n"
CREATERR     "sed: 0602-406 Cannot create file %s.\n"
DUPLBL       "sed: 0602-407 There are more than one %s labels.\n"
FILECNT      "sed: 0602-408 The w function allows a maximum of ten files.\n"
FILEOPEN     "sed: 0602-409 Cannot open file %s.\n"
FRSTRE       "sed: 0602-410 The first regular expression cannot be null.\n"
$       The term "regular expression" is a Unix technical term.
HOLDSPC      "sed: 0602-411 The hold space work area is full.\n"
LABELCNT     "sed: 0602-412 There are too many labels in file %s.\n"
LEFTBRC      "sed: 0602-413 There are too many '{'.\n"
LINECNT      "sed: 0602-414 There are too many line numbers specified.\n"
LINELEN      "sed: 0602-415 The output line exceeds maximum length.\n"
LTLMSG       "sed: 0602-417 The label %s is greater than eight characters."
MALLOCMSG    "sed: 0602-418 Memory allocation failed.\n"
OPENERR      "sed: 0602-419 Cannot find or open file %s.\n"
PATTFIL      "sed: 0602-420 Cannot open pattern file %s.\n"
PATTSPC      "sed: 0602-421 The pattern space work area is full.\n"
READCNT      "sed: 0602-422 There are too many reads after line %ld.\n"
RGHTBRC      "sed: 0602-423 There are too many '}'.\n"
TMMSG        "sed: 0602-424 The %s function did not end the command line,\n\
\tor was not preceded by a blank character.\n"
TOOMANYCMDNS "sed: 0602-425 There are more than 99 commands in pattern file %s.\n"
UNDFNLBL     "sed: 0602-427 %s is not a defined label.\n"
UNKFLG       "sed: 0602-428 %c is not a valid flag.\n"
NOSCRIPT     "sed: 0602-429 No editing script was provided.\n"
CMDSLINEMAX  "sed: 0602-430 Command line is too long.\n"
USAGE        "Usage:\tsed [-n] Script [File ...]\n\
\tsed [-n] [-e Script] ... [-f Script_file] ... [File ...]\n"

