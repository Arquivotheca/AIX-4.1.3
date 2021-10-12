$  @(#)45        1.6  src/bos/usr/bin/alog/alog.msg, cmdalog, bos411, 9428A410j 2/22/94 10:29:14 
$
$ COMPONENT_NAME: (CMDALOG) General purpose progress logging facility
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
$quote "         define quote character.
$len
$set MS_ALOG     Define initial set#

NON_ALOG_FILE	"0526-001 alog: %s is not an alog file.\n"
$ do not translate alog

FILE_NOT_OPEN   "0526-002 alog: Could not open file, %s.\n\
Possible cause(s):\n\t\
- The file does not exist.\n\t\
- The user does not have the proper authorities for the file.\n"
$ do not translate alog

INVALID_TYPE    "0526-003 alog: %s is not an alog type.\n"
$ do not translate alog

ATTR_NOT_FOUND  "0526-004 alog: Unable to find attribute, %s, in the ODM object class SWservAt.\n\
Possible cause(s):\n\t\
- The attribute does not exist in the ODM object class SWservAt.\n\t\
- The ODM object class SWservAt is corrupted.\n"
$ do not translate alog or ODM

UPD8_ATTR_ERR   "0526-005 alog: Unable to update attribute, %s, in the ODM object class SWservAt.\n\
Possible cause(s):\n\t\
- The user is not a root user.\n"
$ do not translate alog or ODM

ERR_LOCK_ODM    "0526-006 alog: Unable to lock ODM.\n"
$ do not translate alog or ODM

BAD_FILE_NAME   "0526-007 alog: Invalid file name.\n\
Possible cause(s):\n\t\
- The file name associated with the specified type\n\t\
  was not found in the ODM database, or it does not have a value.\n\t\
- /dev/null was specified.\n"
$ do not translate alog or ODM

BAD_FLAG_COMBO  "0526-008 alog: Invalid combination of flags.\n"
$ do not translate alog

LIST_HEADERS    "#file:size:verbosity\n"

USAGE		"Usage:\n\t\
alog -f File [-o] | [ [-s Size] [-q] ]\n\t\
alog -t LogType [-f File] [-o] | [ [-s Size] [-q] ]\n\t\
alog -t LogType -V\n\t\
alog -C -t LogType [-f File] [-s Size] [-w Verbosity]\n\t\
alog -L [-t LogType]\n\t\
alog -H\n"
$ do not translate alog
$ Numbers 10-998 are reserved for this message file.

SWSERV_NO_ACCESS  "\
0526-009 alog: Unable to access the ODM object class SWservAt.\n\
Possible causes:\n\
1.  The ODM object class SWservAt does not exist.\n\
2.  The ODM object class SWservAt does not have read permissions.\n"

BAD_VERBOSITY "\
0526-010 alog: The verbosity '%s' is not a valid verbosity value.\n\
The verbosity value must be within the range 0 to 9.\n"

LIST_ALL "#type:file:size:verbosity\n"
