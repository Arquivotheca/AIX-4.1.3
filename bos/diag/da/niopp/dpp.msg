$ @(#)06	1.3.1.2  src/bos/diag/da/niopp/dpp.msg, daniopp, bos411, 9428A410j 2/5/93 14:09:04
$
$   COMPONENT_NAME: DANIOPP
$
$   FUNCTIONS: Diagnostic Message File.
$		
$
$   ORIGINS: 27
$
$
$   (C) COPYRIGHT International Business Machines Corp. 1989,1993
$   All Rights Reserved
$   Licensed Materials - Property of IBM
$   US Government Users Restricted Rights - Use, duplication or
$   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$
$ GENERAL NOTES
$
$ Do not translate %d or %s in any message.  They
$   are used for word or number substitution and are noted in the
$   comments for the individual messages.  The 1$, 2$, 3$, etc,
$   within the substitutions are used to denotethe order of the
$   substitutions.
$
$ These comments concern the TITLE LINES at the top of the diagnostic screen.
$   The title must be in all capital letters.  The first line
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
$ These comments concern the user ACTIONS in all capital letters.
$   If translations require the creation of new lines, begin the new lines
$   in the column immediately following the row of periods.
$   For example:
$
$ *****
$ ACTION.........the one line of translation might need more than one line, so
$                begin the next line at the same point of the previous line.
$ ACTION.........the next action follows with no blank line preceding it.
$ *****
$
$ END OF GENERAL NOTES


$quote "
$set PARALLEL_DIAG

DM_NPP  "TESTING STANDARD PARALLEL PORT P %1$s %2$s"
$
$ %1$s will either be blank or will be "IN ADVANCED MODE"
$ %2$s will either be blank, "Please stand by." or "Testing multiple times."
$ Leave line spacing as shown.  See general notes on length of title line.

DM_ADV  "IN ADVANCED MODE."
$
$ This message can be the first substitution in the TITLE line, message DM_NPP.
$ These two messages combined cannot be greater than 70 characters on the
$ first line.

DM_SBY  "\n\n\
Please stand by."

DM_TLM  "\n\n\
Testing multiple times.\n\n\
%1$6d\tpasses completed.\n\
%2$6d\terrors logged.\n\n\
To stop the test, use 'Cancel.'"
$
$ %1$6d\t sets the number of diagnostic test passes completed.
$ %2$6d\t sets the number of errors logged during the tests.

DM_YES   "YES"
$ This option is shown when a YES answer is possible.

DM_NO    "NO"
$ This option is shown when a NO answer is possible.

DM_MCS  "Move cursor to selection, then press Enter."

DM_ENT  "When finished, press Enter."

DM_CNT  "To continue, press Enter."

DM_UCB  "\
UNPLUG.........the cable, if attached, from Standard Parallel Port P."
$
$ See set notes for infomation on how to expand lines if necessary.

DM_WPR  "\
The following test requires a wrap plug, Part Number 71F0690.\n\n\
Do you have this wrap plug?\n\n%s"
$
$ %s can be the instruction "Move cursor to selection, then press enter."

DM_PWP  "\n\
PLUG...........the wrap plug (Part Number 71F0690) into\n\
               Standard Parallel Port P."
$
$ See set notes for infomation on how to expand lines if necessary.

DM_UWP  "\
UNPLUG.......the wrap plug from Standard Parallel Port P.\n"
$
$ See set notes for infomation on how to expand lines if necessary.

DM_PCB  "\
PLUG.........the cable, if it was removed, into Standard Parallel Port P."
$
$ See set notes for infomation on how to expand lines if necessary.

DM_NTF  "No trouble was found with the parallel port in the system.\n\n\
If a device is attached to the port, go to the problem determination\n\
procedures (test procedures) for the device and test that device.\n\n\
If a problem is found, follow the procedures for correcting the problem."
$
$ Leave line spacing between sentences as shown.
