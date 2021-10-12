$ @(#)01	1.3  src/bos/diag/da/corvette/dacorv.msg, dascsi, bos411, 9428A410j 8/18/93 13:33:22
$
$   COMPONENT_NAME: DASCSI
$
$   FUNCTIONS: none
$
$   ORIGINS: 27
$
$   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$   combined with the aggregated modules for this product)
$                    SOURCE MATERIALS
$
$   (C) COPYRIGHT International Business Machines Corp. 1993
$   All Rights Reserved
$   US Government Users Restricted Rights - Use, duplication or
$   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$
$ GENERAL NOTES
$
$ Do not translate %c, %d, %s, %x, %07X, or \t in any messages.  They
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
$ The location of a resource is in the form of xx-xx-xx where x is an
$   alpha-numeric character.  The location is not translatable.  It is
$   an alpha-numeric descriptor of where the resource can be found.
$
$ END OF GENERAL NOTES


$quote "
 
$set FRU_SET

RESET_FAILED "Reset test failed."
REGISTER_FAILED "Command interface register test failed."
IMMEDIATE_PACE "Immediate pacing command test failed."
SCB_FAILED "Subsystem control block test failed."
ADDRESS_LINE "System address line test failed."
PTC_INTERNAL "Internal SCSI bus terminator power failure."
PTC_EXTERNAL "External SCSI bus terminator power failure."
INT_BUS "Internal SCSI bus error."
EXT_BUS "External SCSI bus error."
ELA_MCODE "Error log analysis indicates microcode corrupted."
ELA_TIMEOUT "Error log analysis indicates get adapter information timed out."
ELA_RESET "Error log analysis indicates adapter reset failed."
ELA_DMA "Error log analysis indicates adapter DMA failed."
ELA_PTC_INT "Error log analysis indicates internal SCSI bus terminator power failure."
ELA_PTC_EXT "Error log analysis indicates external SCSI bus terminator power failure."
ELA_INT_BUS "Error log analysis indicates internal SCSI bus error."
ELA_EXT_BUS "Error log analysis indicates external SCSI bus error."
HARD_FAILURE "Indeterminate cause of Hardware failure."
ELA_FAILED "Error log analysis indicates hardware failure."
CABLE_TERM "SCSI cable and terminators."
SCSI_SUBSYSTEM "SCSI Subsystem."

$set DCORV
DCORV_TITLE "TESTING SCSI-2 FAST/WIDE (%1$s) IN LOCATION %2$s\n\n\n\
Please stand by.\n\n\
This test may take up to 2 minutes."

$
$ This line is displayed during testing while in the non-advanced mode.
$ Leave line spacing as shown.  See general notes on length of title line.

DCORV_TITLE_ADV "TESTING SCSI-2 FAST/WIDE (%1$s) IN LOCATION %2$s\n\n\
IN ADVANCED MODE\n\n\
Please stand by.\n\n\
This test may take up to 2 minutes."

$
$ This line is displayed during testing while in the advanced mode.
$ Leave line spacing as shown.  See general notes on length of title line.

DCORV_LOOP "\
TESTING SCSI-2 FAST/WIDE %1$s IN LOCATION %2$s\n\n\
IN ADVANCED MODE\n\n\
Testing multiple times.\n\n\
%3$6d\tpasses completed.\n\
%4$6d\terrors logged.\n\n\
To stop the test, use 'Cancel.'"

$
$ %3$6d\t sets the number of diagnostic test passes completed.
$ %4$6d\t sets the number of errors logged during the tests.
$ This line is displayed during advanced mode testing in loop mode.
$ Leave line spacing as shown.  See general notes on length of title line.

