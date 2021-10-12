$ @(#)09	1.2  src/bos/diag/util/udisacfg/udisacfg.msg, dsauisacfg, bos41J, 9512A_all 3/14/95 13:13:09
$
$   COMPONENT_NAME: DSAUISACFG
$
$   FUNCTIONS: Message Catalog for  ISA Device Configuration Service Aid
$
$   ORIGINS: 27
$
$
$   (C) COPYRIGHT International Business Machines Corp. 1995
$   All Rights Reserved
$   Licensed Materials - Property of IBM
$   US Government Users Restricted Rights - Use, duplication or
$   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
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
$ These comments concern the techincal terms used in the messages.
$   SCSI:  is an acronym for "Small Computer Systems Interface."  SCSI is
$      a technical term.  Do not translate unless an equivalent tern is
$      used in the translating country.  A SCSI Drawer is a drawer that
$      installs in a rack mounted system.
$
$       This file contains the corresponding messages for the menus and
$       errors generated from the Diagnostic Utility.
$
$ END OF GENERAL NOTES


$ Leave two blank spaces at the beginning of every line after the first.
$quote "

$ DO NOT ADD ANY SETS BEFORE THIS FIRST SET.

$set ISA_SERVICE_AID

ISA_SERVICE_AID_SELECTION "\
ISA Adapter Configuration Service Aid\n\
  This selection helps in the identification and configuration of\n\
  ISA adapters in the system. This Service Aid should be run whenever\n\
  ISA adapters are added to or removed from the system."

$
$ ISA stands for Industry Standard Adapters
$
$ The ISA_SERVICE_AID_SELECTION message is displayed by the Utility 
$ Controller during Service Aids selection. This set should always be 1
$ and the message should always be 1. The set and msg number are referenced
$ by the DSMOptions stanza for this Service Aid. 

$set ISA_SET

ISA_MSG_T   "ISA DEVICE CONFIGURATION SERVICE AID"

$
$ ISA stands for Industry Standard Adapters
$

ISA_LLINE "Move cursor to the selection, then press Enter."

ISA_MSG_ADD "Add an ISA Adapter"

$
$ ISA stands for Industry Standard Adapters
$

ISA_MSG_DEL "Remove an ISA Adapter"

$
$ ISA stands for Industry Standard Adapters
$

ISA_MSG_DEF "List all Defined ISA Adapters"

$
$ ISA stands for Industry Standard Adapters
$

ISA_MSG_SP "List all Supported ISA Adapters"

$
$ ISA stands for Industry Standard Adapters
$

$set ISA_ADD

ADD_ADP "ADD AN ISA ADAPTER\n\n\
This list shows all supported ISA adapters that can\n\
be added into the system. Select an adapter from this\n\
list, or if the adapter support is located on a\n\
Supplemental Diskette, then select 'Read Supplemental\n\
ISA Adapter Diskette.'"

$
$ ISA stands for Industry Standard Adapters
$

ADD_SUP "\
Read Supplemental ISA Adapter Diskette"
$ Menu 802301

$
$ ISA stands for Industry Standard Adapters
$

ADD_BUS "ADD AN ISA ADAPTER\n\n\
This list shows all supported ISA busses. Select\n\
the bus that the adapter is plugged into."

$
$ Menu 802302
$ ISA stands for Industry Standard Adapters
$

ADD_IN_PROCESS "ADDING ISA ADAPTER"

$
$ ISA stands for Industry Standard Adapters
$

ADD_MSG3 "Adding ISA Adapter '%s'.\n\n\
Please stand by."

$
$ Menu 802304
$ ISA stands for Industry Standard Adapters
$

ADD_MSG4_POP "\
ISA Adapter device '%s'\n\
Configuration complete.\n\n\n\
To continue, press Enter."

$
$ Menu 802304 Popup
$ ISA stands for Industry Standard Adapters
$

$
$set ADD_ERR
ADD_ERR_MSG "ISA ADAPTER PROCESSING ERROR\n\n\
An Error has occurred while trying to %1$s ISA adapter\n\
'%2$s'."

$
$ ISA stands for Industry Standard Adapters
$ %1$s will be substituted with the word 'add' or 'remove'
$ as referenced by ADD_STRING or REM_STRING below.
$ %2$s is the adapter description. 
$

ADD_CMD_MSG "%s"

$
$ ADD_CMD_ERR will be filled in by the output of the 
$ mkdev or rmdev command

ADD_CONTINUE "\
To continue, press Enter."


$ Menu 802305 error message adding the adapter.
$

ADD_STRING  "add"
REM_STRING  "remove"

$
$ The words 'add' or 'remove' are substituted into the
$ ADD_ERR_MSG message in location %1$s location. 

$
$set ISA_DEL 

ISA_DEL_T "REMOVE AN ISA ADAPTER\n\n\
This list shows all defined or configured ISA adapters\n\
in the system. Select the ISA adapter from this list to\n\
be removed from the system."

$
$ Menu 802306 error message removing the adapter.
$ ISA stands for Industry Standard Adapters
$

REM_IN_PROCESS "REMOVE AN ISA ADAPTER"

$
$ ISA stands for Industry Standard Adapters
$

REM_ADP "\n\
Removing ISA Adapter '%s'.\n\n\
Please stand by."

$
$ ISA stands for Industry Standard Adapters
$

REM_ADP_POP "\
ISA adapter '%s'\n\n\
has been removed from the system.\n\n\n\n\
To continue, press Enter."

$
$ ISA stands for Industry Standard Adapters
$

$
$set LIST_DEF

LIST_DEF_T "LIST ALL DEFINED ISA ADAPTERS\n\n\
This list indicates all defined or configured ISA\n\
Adapters in the system. If necessary, use the Up\n\
Arrow and/or Down Arrow to view all the selections.\n"

$
$ ISA stands for Industry Standard Adapters
$

NO_DEF_ISA "\
No ISA adapters have\n\
been found in the system.\n\n\n\n\
To continue, press Enter."

$
$ ISA stands for Industry Standard Adapters
$

DEFINE_MSG "Defined"
AVAIL_MSG  "Available"
DIAGN_MSG  "Diagnose"
UNDEF_MSG  "????????"

$ 
$ Defined, Available, and Diagnose are the states 
$ that an adapter can be in.
$

$set LIST_SUPP

LIST_SUPP_T "LIST ALL SUPPORTED ISA ADAPTERS\n\n\
This list indicates all supported ISA Adapters.\n\
If necessary, use the Up Arrow and/or Down Arrow\n\
to view the selections in the list.\n"

$
$ ISA stands for Industry Standard Adapters
$

$set READ_DSKT
DSKT_T "\
READ SUPPLEMENTAL ISA ADAPTER DISKETTE"

DSKT_MSG "\
This selection reads a user supplied diskette with\n\
configuration data for a particular ISA adapter. This\n\
diskette must contain all the necessary code to\n\
define, configure, and diagnose the ISA adapter."

$
$ ISA stands for Industry Standard Adapters
$

DSKT_INST "\
Insert the diskette containing the ISA adapter\n\
software, then press Enter."

$
$ ISA stands for Industry Standard Adapters
$

DSKT_ERR_POP "\
There is an error reading the Supplemental\n\
Diskette. Either the data is bad,\n\
or the diskette is not a valid Supplemental\n\
Diagnostic Diskette.\n\n\n\
Press Enter to continue."

$
DSKT_GOOD_POP "\
Supplemental ISA Adapter\n\
Configuration complete.\n\n\n\
To continue, press Enter."

$
$ Menu 802303 Popup
$ ISA stands for Industry Standard Adapters
$

$set BUS_SET
BUSIOTITLE "ISA ADAPTER BUS I/O ADDRESS SELECTION"
$
$ See general notes on length of title.

BUSIOPROMPT  "Bus I/O Address................................."

BUSIODEFAULT "Press 'List' to select additional values. Press\n\
'Commit' when finished.\n\n"
$
$ This is a blank message. Needs to stay.

