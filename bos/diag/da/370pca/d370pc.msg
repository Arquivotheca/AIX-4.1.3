$ @(#)72	1.3  src/bos/diag/da/370pca/d370pc.msg, da370pca, bos411, 9428A410j 6/12/93 17:21:51
$ $ *
$ $ * COMPONENT_NAME: DA370PC
$ $ *
$ $ * FUNCTIONS:   This file contains the English language messages for the
$ $ *              370 Parallel Channel Attach adapter diagnostic application
$ $ *              menus.
$ $ *
$ $ * ORIGINS: 27
$ $ *
$ $ * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ $ * combined with the aggregated modules for this product)
$ $ *                  SOURCE MATERIALS
$ $ * (C) COPYRIGHT International Business Machines Corp. 1991
$ $ * All Rights Reserved
$ $ *
$ $ * US Government Users Restricted Rights - Use, duplication or
$ $ * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$ $ *
$
$ GENERAL NOTES
$
$ Do not translate %d, %s, %x, %07X, or \n in any message.  They
$   are used for word or number substitution and are noted in the
$   comments for the individual messages.  The 1$, 2$, 3$, etc,
$   within the substitutions are used to denote the order of the
$   substitutions.
$
$ These comments concern the TITLE LINES at the top of the diagnostic screen.
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
$   If translations require the creation of new lines, begin the new lines
$   in the column immediately following the row of periods.
$   For example:
$
$ ACTION.........the one line of translation might need more than one line, so
$                begin the next line at the same point of the previous line.
$ ACTION.........the next action follows with no blank line preceding it.
$
$ The location of a resource is in the form of xx-xx-xx where x is an
$   alpha-numeric character.  The location is not translatable.  It is
$   an alpha-numeric descriptor of where the resource can be found.
$
$ END OF GENERAL NOTES

$quote "
$set D370PCA_DA

DM370PC_1  "\
TESTING 370 PARALLEL CHANNEL ATTACH ADAPTER %1$s IN LOCATION %2$s\n\n\
Please stand by.\n\n\
This test may take up to 5 minutes."
$
$ %1$s is the name of the device being tested.
$ %2$s is the location of the planar slot containing the adapter.
$ The location code of the device is described in the general notes.

DM370PC_2  "\
TESTING 370 PARALLEL CHANNEL ATTACH ADAPTER %1$s IN LOCATION %2$s\n\
IN ADVANCED MODE\n\n\
Please stand by.\n\n\
This test may take up to 5 minutes."
$
$ %1$s is the name of the device being tested.
$ %2$s is the location of the planar slot containing the adapter.
$ The location code of the device is described in the general notes.

DM370PC_3  "\
TESTING 370 PARALLEL CHANNEL ATTACH ADAPTER %1$s IN LOCATION %2$s\n\
IN ADVANCED MODE\n\n\
Testing multiple times.\n\n\
%3$6d\tpasses completed.\n\
%4$6d\terrors logged.\n\n\
This test may take up to 5 minutes.\n\
To stop the test, use 'Cancel.'"
$
$ %1$s is the name of the device being tested.
$ %2$s is the location of the planar slot containing the adapter.
$ %3$6d\t sets the number of diagnostic test passes completed.
$ %4$6d\t sets the number of errors logged during the tests.
$ The location code of the device is described in the general notes.

DM370PC_4  "\
TESTING 370 PARALLEL CHANNEL ATTACH ADAPTER %1$s IN LOCATION %2$s\n\
IN ADVANCED MODE"
$
$ %1$s is the name of the device being tested.
$ %2$s is the location of the planar slot containing the adapter.
$ The location code of the device is described in the general notes.

DM370PC_5  "YES"
$ This option is shown when a YES answer is possible.

DM370PC_6  "NO"
$ This option is shown when a NO answer is possible.

DM370PC_7  "\
* * * WARNING * * *\n\n\
The channel must be varied off at the host system before running\n\
the wrap test.\n\n\
Has the channel been varied off?\n\n\
Move cursor to selection, then press Enter."
$
$ Leave line spacing as shown.

DM370PC_8  "\
The following test requires a wrap plug, (Part Number 71F1184).\n\n\
Do you have this wrap plug?\n\n\
Move cursor to selection, then press Enter."
$
$ 370 Parallel Channel Attach test unit 10.
$ 71F1184 is the part number of the wrap plug. Do not translate.
$ Leave line spacing as shown.

DM370PC_9  "\
SET............the Select-Out Bypass switch on the 78 pin connector that\n\
	       attaches the cable to the adapter to the (B) position.\n\
UNPLUG.........the cable, if attached, from the 370 Parallel Channel Attach\n\
	       adapter.\n\
PLUG...........the wrap plug (Part Number 71F1184) into the 370 Parallel\n\
	       Channel Attach adapter.\n\n\
When finished, press Enter."
$
$ 370 Parallel Channel Attach test unit 10.
$ 71F1184 is the part number of the wrap plug. Do not translate.
$ See set notes for information on how to expand lines if necessary.
$ Leave line spacing as shown.  See general notes on user ACTIONS.

DM370PC_10 "\
UNPLUG.........the wrap plug from the 370 Parallel Channel Attach adapter.\n\
PLUG...........the cable, if it was removed, into the 370 Parallel Channel\n\
	       Attach adapter.\n\
SET............the Select-Out Bypass switch on the 78 pin connector that\n\
	       attaches the cable to the adapter to the (S) position.\n\n\
When finished, press Enter."
$
$ 370 Parallel Channel Attach test unit 10.
$ See set notes for information on how to expand lines if necessary.
$ Leave line spacing as shown.  See general notes on user ACTIONS.

DM370PC_11 "\
The following test requires two wrap plugs (Part Number 8575337 and\n\
Part Number 8575338) and two terminators (Part Number 2282675 and\n\
Part Number 2282676).\n\n\
Do you have these wrap plugs and terminators?\n\n\
Move cursor to selection, then press Enter."
$
$ 370 Parallel Channel Attach test unit 15.
$ 8575337 and 8575338 are wrap plug part numbers. Do not translate.
$ Leave line spacing as shown.

DM370PC_12 "\
UNPLUG.........the bus in, bus out, tag in, and tag out cables from the\n\
	       Channel Cable Interface Box.\n\
PLUG...........the bus wrap plug (Part Number 8575337) into the bus in\n\
	       connector, the tag wrap plug (Part Number 8575338) into\n\
	       the tag in connector, the bus terminator (Part Number\n\
	       2282675) into the bus out connector and the tag terminator\n\
	       (Part Number 2282676) into the tag out connector on the\n\
	       Channel Cable Interface Box.\n\n\
When finished, press Enter."
$
$ 370 Parallel Channel Attach test unit 15.
$ 8575337 and 8575338 are wrap plug part numbers. Do not translate.
$ See set notes for information on how to expand lines if necessary.
$ Leave line spacing as shown.  See general notes on user ACTIONS.

DM370PC_13 "\
UNPLUG.........the wrap plugs from the Channel Cable Interface Box.\n\
PLUG...........the bus in, bus out, tag in, and tag out cables, if they were\n\
	       removed, into the Channel Cable Interface Box.\n\n\
When finished, press Enter."
$
$ 370 Parallel Channel Attach test unit 15.
$ See set notes for information on how to expand lines if necessary.
$ Leave line spacing as shown.  See general notes on user ACTIONS.

DM370_14 "862100 \
370 PARALLEL CHANNEL ATTACH ADAPTER OPERATIONAL MICROCODE IS MISSING\n\n\
The 370 Parallel Channel Attach adapter operational microcode is either\n\
missing or not accessible.\n\n\
This microcode is necessary in order to use the 370 Parallel Channel\n\
Attach adapter card in normal system operations."
$
$ Leave line spacing as shown.  See general notes on length of title.
$ Do not translate the number 862100 at the beginning of the message.
$ Leave it exactly as shown.

DM370_15 "862200 \
370 PARALLEL CHANNEL ATTACH ADAPTER DIAGNOSTIC MICROCODE IS MISSING\n\n\
The 370 Parallel Channel Attach adapter diagnostic microcode is either\n\
missing or not accessible.\n\n\
This microcode is necessary in order to use the 370 Parallel Channel\n\
Attach adapter card in the diagnostic mode."
$
$ Leave line spacing as shown.  See general notes on length of title.
$ Do not translate the number 862200 at the beginning of the message.
$ Leave it exactly as shown.

DM370PC_16 "0"

DM370PC_17 "1"

DM370PC_18 "2"

DM370PC_19 "3"

DM370PC_20 "4"

DM370PC_21 "\
Test unit %d is executing\n\
Move cursor to selection, then press Enter."
