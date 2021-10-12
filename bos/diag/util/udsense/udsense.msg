$ @(#)61        1.3.1.2  src/bos/diag/util/udsense/udsense.msg, dsaudsense, bos41J, 9512A_all 3/9/95 21:10:36
$
$   COMPONENT_NAME: DSAUDSENSE
$
$   FUNCTIONS:
$
$
$   ORIGINS: 27
$
$   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$   combined with the aggregated modules for this product)
$                    SOURCE MATERIALS
$
$   (C) COPYRIGHT International Business Machines Corp. 1992,1994
$   All Rights Reserved
$   US Government Users Restricted Rights - Use, duplication or
$   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$

$
$
$ GENERAL NOTES
$
$ Do not translate %d, %s, %x, %07X, or \t in any messages.  They
$   are used for word or number substitution and are noted in the
$   comments for the individual messages.  The 1$, 2$, 3$, etc,
$   within the substitutions are used to denote the order of the
$   substitutions.
$
$ These comments concern the TITLE LINES at the top the diagnostic screen.
$   The title must be in all capital letters. The first line
$   of the title cannot be longer than 70 characters starting from
$   column 1.  If the line is greater than 70, it may be continued on
$   the next line.  Leave line spacing as shown:  one blank line after
$   the last title line.  For example:
$
$ *****
$ TESTING PORT 12 OF THE 16-PORT ASYNCHRONOUS ADAPTER IN PLANAR SLOT 2
$ IN ADVANCED MODE
$
$ Please stand by.
$ *****
$
$ These comments concern the user ACTIONS in all caps.
$   If translations require the creation of new lines, begin the
$   new lines in the column immediately following the row of periods.
$   For example:
$
$ *****
$ ACTION.........one line of English might require several when translated, so
$                begin the next line at the same point of the previous line.
$ ACTION.........the next action follows with no blank line preceding it.
$ *****
$
$ This file contains the corresponding messages for the menus and
$ errors generated from the UDSENSE Utility.
$
$ END OF GENERAL NOTES

$set MSET
$quote "

UDCONT ""
$ This message is no longer used. (172698)

MTITLE "\
HARDWARE ERROR REPORT"
$
$ See general notes on length of title.


MOPT1 "Display Error Summary"
MOPT2 "Display Error Detail"


MOPT3 ""
$ This message is no longer used. (172698)
MOPT4 ""
$ This message is no longer used. (172698)


MLASTLINE "Move cursor to selection, then press Enter."

STANDBY_OLD ""
$ This message has been replaced.  See the STANDBY message below.


UDERRSUM "ERROR SUMMARY"
UDERRDTL "ERROR DETAIL"


UDERRASUM ""
$ This message is no longer used. (172698)
UDERRADTL ""
$ This message is no longer used. (172698)


CMD_MISSING "The System Management Tools and Applications package is not\n\
currently installed on the system. This package needs to be\n\
installed before using the Hardware Error Report Service Aid.\n\n"

UDRET ""
$ This message is no longer used. (172698)

UDCANCEL "\
To see additional items in the list, use the Up Arrow and Down Arrow keys.\n\
When finished, press the Enter key to return to the Selection Menu."

STANDBY "\
HARDWARE ERROR REPORT\n\n\
Reading current error log.\n\n\
Please stand by."

$ $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
$set MWAITING

WAITING "\
PROCESSING THE ERROR LOG\n\
MAY TAKE SEVERAL MINUTES\n\
The screen will be blank during processing.\n\n\
Press Enter to start processing the error log.\n\n\
Press F3 To Cancel (return\n\
      to %1$s Menu).\n\
Press F10 To Exit."

$ $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
$set MERROR

ODM_RUN_ERROR "\
An error occurred while executing the following command:\n\n\
  %1$s\n"
$
$ %1$s is the failing command that was executed by the odm_run_method() call.

NO_DATA "\
The system error log does not contain any\n\
entries related to hardware errors."
