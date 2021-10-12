$ @(#)72        1.1  src/bos/diag/da/baud/dabaud.msg, foxclub, bos411, 9435B411a 8/26/94 13:31:03
$
$   COMPONENT_NAME: dabaud
$
$   FUNCTIONS: none
$
$   ORIGINS: 27
$
$
$   (C) COPYRIGHT International Business Machines Corp. 1994
$   All Rights Reserved
$   Licensed Materials - Property of IBM
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
$ END OF GENERAL NOTES

$set BAUD_SRN_MSG
$quote "
$ This set should always be the first set.  Do NOT move this set or put other
$ message sets before this one.

BAUD_101 "Diagnostics Device Driver load failed."
BAUD_102 "VPD registers test failed."
BAUD_103 "MCI registers test failed."
BAUD_104 "Codec registers test failed."
BAUD_105 "Capture/playback test failed."
BAUD_106 "Closing device failed."
BAUD_107 "Error log analysis indicated a hardware failure."
BAUD_108 "N/A"
BAUD_109 "N/A"
BAUD_110 "N/A"
BAUD_111 "N/A"
BAUD_112 "N/A"
BAUD_113 "N/A"
BAUD_114 "N/A"
BAUD_115 "N/A"
BAUD_116 "N/A"
BAUD_117 "N/A"
BAUD_118 "N/A"
BAUD_NAME "Ultimedia Audio Adapter"

$set BAUD_MSG
$quote "


ADVANCED_MODE_MENU "\nIN ADVANCED MODE"
$ Leave line spacing as shown.  See general notes on length of title line.

CUSTOMER_MODE_MENU " "
$ This is a blank message because there is no additional information for this
$ mode.
$ Leave line spacing as shown.  See general notes on length of title line.

LOOPMODE_MODE_MENU " "
$ This is a blank message because there is no additional information for this
$ mode.
$ Leave line spacing as shown.  See general notes on length of title line.

TITLE   "\
TESTING %1$s %2$s IN LOCATION %3$s %4$s\n\
"
$ %1$s is the type of adapter.  This is gotten from devices.cat.
$ %2$s is the adapter that is tested.
$ %3$s is the location code of the device as described in the general notes
$ %4$s is whether this is replaced with "IN ADVANCED MODE" if this is advanced
$       mode.
$ Leave line spacing as shown.  See general notes on length of title line.

SEL_OPTION "Move cursor to selection, then press Enter "
$ Leave line spacing as shown.  See general notes on length of title line.

FINISHED "\
When finished, press Enter."

YES_OPTION "YES"
$ This option is shown when the user is asked to have a YES NO option.
$ Leave line spacing as shown.  See general notes on length of title line.

NO_OPTION "NO"
$ This option is shown when the user is asked to have a YES NO option.
$ Leave line spacing as shown.  See general notes on length of title line.
