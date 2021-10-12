$ @(#)34	1.3.1.2  src/bos/diag/da/mpa/dampa.msg, mpada, bos411, 9439C411a 9/29/94 17:20:44
$
$   COMPONENT_NAME: (MPADIAG) MP/A DIAGNOSTICS
$
$   FUNCTIONS: ADAPTER/
$		
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

$ $ *
$ $ * COMPONENT_NAME: diag_MPA
$ $ *
$ $ * FUNCTIONS:   This file contains the English language messages for the
$ $ *              MPA diagnostic application menus.
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
$set DMPA_VEND

$ ---------------------------------------------------------------
$ Reason code set used by device type "Multiprocotol Adapter/A (MP/A)"
$ $ Do not add any messages above DC_MPA.
$ $ Do not add any messages between DMPA_BMN and DMPA_1.
$ $ Do not add any messages above DMPA_26.

DC_MPA          "Multiprocotol Adapter/A (MP/A)"
DMPA_CCIB       "Cable"
DMPA_ECFG       "Error configuring device."
DMPA_EDCFG      "Error configuring diagnostic driver."
DMPA_EOMD       "ODM Error configuring diagnostic driver."
DMPA_OPEN       "Error opening diagnostic device driver."
DMPA_BMN        "Reserved"
$ DMPA_BMN is the message number all FRU bucket rmsg's are referenced to.

DMPA_1          "POS register test failed."
DMPA_2          "MP/A adapter wrap test failed."
DMPA_3          "MP/A adapter dma test failed."
DMPA_4          "Reserved"
DMPA_5          "MP/A adapter register test failed."
DMPA_6          "MP/A modem signals external wrap test failed."
DMPA_7          "MP/A modem signals internal wrap test failed."

$set DMPA_DA

DMMPA_1  "\
TESTING MULTIPROTOCOL ADAPTER/A (MP/A) %1$s IN LOCATION %2$s\n\n\
Please stand by.\n\n\
This test may take up to 2 minutes."
$
$ %1$s is the name of the device being tested.
$ %2$s is the location of the planar slot containing the adapter.
$ The location code of the device is described in the general notes.

DMMPA_2  "\
TESTING MULTIPROTOCOL ADAPTER/A (MP/A) %1$s IN LOCATION %2$s\n\
IN ADVANCED MODE\n\n\
Please stand by.\n\n\
This test may take up to 5 minutes."
$
$ %1$s is the name of the device being tested.
$ %2$s is the location of the planar slot containing the adapter.
$ The location code of the device is described in the general notes.

DMMPA_3  "\
TESTING MULTIPROTOCOL ADAPTER/A (MP/A) %1$s IN LOCATION %2$s\n\
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

DMMPA_4  "\
TESTING MULTIPROTOCOL ADAPTER/A (MP/A) %1$s IN LOCATION %2$s\n\
IN ADVANCED MODE"
$
$ %1$s is the name of the device being tested.
$ %2$s is the location of the planar slot containing the adapter.
$ The location code of the device is described in the general notes.

DMMPA_5  "YES"
$ This option is shown when a YES answer is possible.

DMMPA_6  "NO"
$ This option is shown when a NO answer is possible.

DMMPA_7  "\
* * * WARNING * * *\n\n\
The channel must be varied off at the host system before running\n\
the wrap test.\n\n\
Has the channel been varied off?\n\n\
Move cursor to selection, then press Enter."
$
$ Leave line spacing as shown.

DMMPA_8  "\
The following test requires a wrap plug,\n\
(Part Number 72X8546 or 62X1083).\n\n\
Do you have this wrap plug?\n\n\
Move cursor to selection, then press Enter."
$
$ MPA MULTIPROTOCOL test unit 2.
$ 72X8546 / 62X1083 are the part numbers of wrap plugs. Do not translate.
$ Leave line spacing as shown.

DMMPA_9  "\
UNPLUG.........the cable, from the adapter.\n\
PLUG...........the wrap plug (Part Number 72X8546 or 62X1083)\n\
               into the MP/A MULTIPROTOCOL adapter.\n\n\
When finished, press Enter."
$
$ MPA MULTIPROTOCOL test unit 2.
$ 72X8546 / 62X1083 are the part numbers of wrap plugs. Do not translate.
$ See set notes for information on how to expand lines if necessary.
$ Leave line spacing as shown.  See general notes on user ACTIONS.

DMMPA_10 "\
UNPLUG.........the wrap plug from the MPA MULTIPROTOCOL adapter.\n\
PLUG...........the cable, if it was removed, into the MPA MULTIPROTOCOL\n\
	       adapter.\n\
When finished, press Enter."
$
$ MPA MULTIPROTOCOL test unit 2.
$ See set notes for information on how to expand lines if necessary.
$ Leave line spacing as shown.  See general notes on user ACTIONS.

DMMPA_16 "0"

DMMPA_17 "1"

DMMPA_18 "2"

DMMPA_19 "3"

DMMPA_20 "4"

DMMPA_21 "\
Test unit %d is executing\n\
Move cursor to selection, then press Enter."

DMMPA_22 "Single Port Multiprotocol Adapter"
DMMPA_23 "MPA Diagnostic Device Driver"
