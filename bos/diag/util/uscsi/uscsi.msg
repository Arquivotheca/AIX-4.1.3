$ @(#)11        1.3.1.4  src/bos/diag/util/uscsi/uscsi.msg, dsauscsi, bos411, 9428A410j 3/17/94 15:12:09
$
$   COMPONENT_NAME: DSAUSCSI
$
$   FUNCTIONS: Diagnostic message file.
$		
$
$   ORIGINS: 27
$
$
$   (C) COPYRIGHT International Business Machines Corp. 1989,1990,1992
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
$ These comments concern the technical terms used in the messages.
$   SCSI:  is an acronym for "Small Computer Systems Interface."  SCSI is
$      a technical term.  Do not translate unless an equivalent term is
$      used in the translating country.  A SCSI Drawer is a drawer that
$      installs in a rack mounted system.
$
$ END OF GENERAL NOTES


$quote "
$set FDSAID_SET1

$
$ >>>>>>>>>> New messages for SCSI Inquiry implementation <<<<<<<<<<
$ NOTES: - this catalog file has been modified to implement the SCSI Inquiry
$          command instead of the Test Unit Ready command as used in AIX325.
$          The following changes were made from AIX325's TUR version:
$        - Most messages change the wordings from 'Test Unit Ready' to 'Inquiry'
$        - The name of the messages are also changed to be more meaningful. 
$          Some messages have the same contents.
$
MENU_SA_INTRO "SCSI BUS SERVICE AID DESCRIPTION\n\n\
This service aid is used to exercise a SCSI bus. The service aid transmits a\n\
SCSI Inquiry command to a selectable SCSI address. The service aid\n\
will work with any valid SCSI bus. It can be used to diagnose SCSI bus\n\
problems that prevent diagnostics from communicating with a SCSI device.\n\n\
The service aid will return one of the following messages:\n\
-  The service aid transmitted a SCSI Inquiry command and\n\
   received a valid response back without any errors being detected.\n\
-  The service aid transmitted a SCSI Inquiry command and did not\n\
   receive any response. Check that the address is valid, then retry.\n\
-  The service aid transmitted a SCSI Inquiry command and\n\
   the adapter indicated a SCSI bus error.\n\
-  The service aid transmitted a SCSI Inquiry command and\n\
   an adapter error occurred.\n\
-  The service aid transmitted a SCSI Inquiry command and\n\
   an unrecoverable I/O error occured.\n\n\
To continue, press Enter."
$
$ This is a rename and combine of MENU_INTRO & MENU_INTRO1

MENU_ADAPTER_SELECT_MSG "SCSI BUS  SERVICE AID\n\n\
Select the SCSI adapter you wish to exercise."
MENU_ADAPTER_SELECT_PROMPT "Move cursor to selection, then press Enter."
$
$ This is a rename of MENU_NINE_TITTLE
$ This panel will be displayed when the SCSI adapters are selectable
$ by the user.
$ SCSI is a technical term.  See general notes for description.
$ Leave line spacing as shown.  See general notes on length of title.

MENU_SCSI_SA_TITLE  "SCSI BUS  SERVICE AID\n"
MENU_SCSI_ID_PROMPT  "Enter the SCSI address for the device ...."
MENU_SCSI_ID_DEFAULT " "
$
$ These are used instead of MENU_TWO 's 

MENU_INQU_USER_CONFIRM "SCSI BUS SERVICE AID\n\n\
Press Enter to transmit the SCSI Inquiry command to SCSI address %1$d."
$
$ This is used instead of MENU_THREE
$ This panel will be presented after the user has selected an address
$ to be tested.
$ %1$d is the address of the SCSI device.
$ SCSI is a technical term.  See general notes for description.
$ Leave line spacing as shown.  See general notes on length of title.

MENU_INQU_IN_PROCESS "SCSI BUS SERVICE AID\n\n\
Transmit and receive in process."
$
$ >>> This is used instead of MENU_FOUR <<<
$ This panel will be displayed after the user has pressed Enter
$ to send the Inquiry command.
$ SCSI is a technical term.  See general notes for description.
$ Leave line spacing as shown.  See general notes on length of title.

MENU_INQU_OK "SCSI BUS SERVICE AID\n\n\
The service aid transmitted a SCSI Inquiry command and\n\
received a valid response back without any errors being detected."
$
$ >>> This is used instead of MENU_FIVE <<<
$ This panel will be displayed if the test finished with no error.
$ SCSI is a technical term.  See general notes for description.
$ Leave line spacing as shown.  See general notes on length of title.

MENU_INQU_NO_RESPONSE "SCSI BUS SERVICE AID\n\n\
The service aid transmitted a SCSI Inquiry command and did not\n\
receive any response back. Ensure that the address is valid, then try\n\
this option again."
$
$ >>> This is used instead of MENU_SIX <<<
$ This panel will be displayed if the device at the selected address
$ failed to respond.
$ SCSI is a technical term.  See general notes for description.
$ Leave line spacing as shown.  See general notes on length of title.

MENU_INQU_BUS_ERROR "SCSI BUS SERVICE AID\n\n\
The service aid transmitted a SCSI Inquiry command and\n\
the adapter indicated a SCSI bus error."
$
$ >>> This is used instead of MENU_SEVEN <<<
$ This panel will be displayed if the adapter return codes indicate
$ a SCSI bus error occurred.
$ SCSI is a technical term.  See general notes for description.
$ Leave line spacing as shown.  See general notes on length of title.

MENU_SCIOSTART_ERROR "SCSI BUS SERVICE AID\n\n\
The service aid transmitted a SCSI Start command and\n\
the operation was not successful."
$
$ >>> This is used instead of MENU_EIGHT <<<
$ This panel will be displayed if the SCSI Start command failed.
$ SCSI is a technical term.  See general notes for description.
$ Leave line spacing as shown.  See general notes on length of title.

MENU_INQU_IO_ERROR "SCSI BUS SERVICE AID\n\n\
The service aid transmitted a SCSI Inquiry command and\n\
the adapter indicated an unrecoverable I/O error."
$
$ >>> This is a new message <<<
$ This panel will be displayed if the adapter return codes indicate
$ a SCSI bus error occurred.
$ SCSI is a technical term.  See general notes for description.
$ Leave line spacing as shown.  See general notes on length of title.

MENU_INQU_DEV_BUSY "SCSI BUS SERVICE AID\n\n\
The Inquiry command cannot be sent because\n\
the SCSI device was already in use by another process."
$
$ >>> This is used instead of MENU_TWELVE <<<
$ This panel will be displayed if the Inquiry command cannot
$ be sent because the SCSI device was already in use.
$ SCSI is a technical term.  See general notes for description.
$ Leave line spacing as shown.  See general notes on length of title.

MENU_ADAPTER_OPEN_ERROR "SCSI BUS SERVICE AID\n\n\
The service aid was unable to open the adapter. The Inquiry\n\
command cannot be sent to any device attached to this adapter."
$
$ >>> This is used instead of MENU_THIRTEEN <<<
$ This panel will be displayed if the open failed. 
$ SCSI is a technical term.  See general notes for description.
$ Leave line spacing as shown.  See general notes on length of title.

MENU_ADAPTER_DEV_SAME_ADDR "SCSI BUS SERVICE AID\n\n\
The address selected is the address for the SCSI adapter.\n\n\
Please enter or select another SCSI address."
$
$ >>> This is used instead of INVALID_CHOICE <<<
$ This message will be displayed if the address chosen is the SCSI
$ adapter address.
$ SCSI is a technical term.  See general notes for description.
$ Leave line spacing as shown.  See general notes on length of title.

MENU_ADAPTER_ADDR_NOT_FOUND  "SCSI BUS SERVICE AID\n\n\
The service aid was unable to determine the address of the adapter."
$
$ >>> This is used instead of INVALID_CHOICE <<<
$ This panel will be displayed if the address of the adapter can
$ not be found.
$ SCSI is a technical term.  See general notes for description.
$ Leave line spacing as shown.  See general notes on length of title.

MSG_PRESS_ENTER "To continue, press Enter."
$
$ >>> This is used instead of PRESS_ENTER <<<

MENU_SCSI_DEV_NONE_FOUND "SCSI BUS SERVICE AID\n\n\
The service aid found no SCSI devices in the system.\
The service aid will exit now.\n\
To continue, press Enter."
$
$ >>> This is a new message <<<
