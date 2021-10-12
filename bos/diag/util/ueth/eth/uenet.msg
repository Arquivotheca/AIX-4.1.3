$ @(#)38	1.2  src/bos/diag/util/ueth/eth/uenet.msg, dsaueth, bos411, 9428A410j 2/15/94 13:59:48
$
$   COMPONENT_NAME: DSAUETH
$
$   FUNCTIONS: n
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
$ The location of a resource is in the form of xx-xx-xx where x is an
$   alpha-numeric character.  The location is not translatable.  It is
$   an alpha-numeric descriptor of where the resource can be found.
$
$ BNC and DIX are technical terms.  DO NOT TRANSLATE unless an equivalent
$ term is used in the translating language.
$
$ This is the message file for the Ethernet service aid.  The Ethernet
$ adapter is an interface to the Ethernet network.  It has two connectors
$ on it: a DIX connector which is a 9-pin type and connects to a "thick-net"
$ network and a BNC, which is a round coaxial-type connector and connects
$ to a "thin-net" network.  The Ethernet service aid runs wrap tests on the
$ adapter to see if it gets back the data it sends out, either through a 
$ wrap plug, or to its own address on the network.
$ 
$ END OF GENERAL NOTES


$set 1
$quote "

DESCRIP_TITLE "ETHERNET SERVICE AID DESCRIPTION"
$
$ The above line is the title line for the description menu, which is the
$ first menu displayed.
$ See general notes on length of title.

DESCRIP "\
This service aid is used to exercise the Ethernet adapter and\n\
parts of the Ethernet network.  The service aid works by\n\
transmitting a data block to itself.  This service aid will\n\
work with a wrap plug or with any valid Ethernet network and\n\
can be used as a tool to diagnose Ethernet network problems.\n\n\
When the Ethernet service aid is executed, one of the\n\
following messages is returned:\n"
$
$ This message is the text of the first menu, which describes the
$ service aid.
$ Leave line spacing as shown.

NO_ERR "\
- No errors occurred."
$
$ This is a possible result of running the Ethernet service aid.

ADAP_ERR "\
- An adapter error occurred."
$
$ This is a possible result of running the Ethernet service aid.

TX_TIME "\
- A transmit time-out occurred."
$
$ This is a possible result of running the Ethernet service aid.

TX_ERR "\
- A transmit error occurred."
$
$ This is a possible result of running the Ethernet service aid.

RX_TIME "\
- A receive time-out occurred."
$
$ This is a possible result of running the Ethernet service aid.

RX_ERR "\
- A receive error occurred."
$
$ This is a possible result of running the Ethernet service aid.

SYS_ER "\
- A system error occurred."
$ 
$ This is a possible result of running the Ethernet service aid.

WRAP_ERR "\
- Receive and transmit data did not match."
$
$ This is a possible result of running the Ethernet service aid.

UNID_ERR "\
- An error occurred that could not be identified."
$ 
$ This is a possible result of running the Ethernet service aid.

NO_ADAPS "\
- The configuration indicates that there are no Ethernet adapters\n\
  in this system unit."
$
$ This is a possible result of running the Ethernet service aid.

ADAP_BUSY "\
- Another application is currently using the adapter."
$
$ This is a possible result of running the Ethernet service aid.


$ $ The above messages are displayed as part of the text for
$ $ the description menu.
$ $ One of the above messages will be displayed as part of the results menu as
$ $ well.

ENTER "To continue, press Enter."
$ This line will be displayed on menus that require the user to press
$ Enter.

ACTION "\
ETHERNET ADAPTER SELECTION\n\n\
Select the adapter to be exercised.\n"
$
$ This message is displayed asking the user to choose which
$ adapter is to be tested.
$ Leave line spacing as shown.  See general notes on length of title.

TRANSMIT "  Transmit a data block from %1$s at location %2$s."
$
$ This line is displayed as part of the ACTION menu.  One copy of
$ this line is displayed for each Ethernet adapter in the system.
$ %1$s is the adapter
$ %2$s is the location of the adapter as described in the general notes.
$ Leave two blank spaces at the beginning of this line.

SELECT "Move cursor to selection, then press Enter."
$
$ This instruction is displayed on menus that require the user to select
$ an option.

STANDBY "\
TRANSFER IN PROGRESS\n\n\
Transmission and reception of data from %1$s at\n\
location %2$s is in progress.\n\n\
Please stand by."
$
$ This message is displayed while the data is being wrapped.
$ %1$s is the adapter
$ %2$s is the location of the adapter as described in the general notes.
$ Leave line spacing as shown.  See general notes on length of title.

WC_TITLE "\
ETHERNET CONNECTOR CHOICE"
$
$ This line is the title for the menu that requires the user to choose
$ either the DIX or BNC connector.
$ DIX is a technical term.  See general notes.
$ BNC is a technical term.  See general notes.
$ See general notes on length of title.

WHICH_CON "\
Which connector is being used on the Ethernet\n\
adapter %1$s in location %2$s?\n\
(The BNC connector is round.)\n\n\
Move cursor to selection, then press Enter.\n"
$
$ This message is displayed as the text for the menu that requires the
$ user to choose either the DIX or BNC connector.
$ DIX is a technical term.  See general notes.
$ BNC is a technical term.  See general notes.
$ %1$s is the adapter
$ %2$s is the location of the adapter as described in the general notes.
$ Leave line spacing as shown.

DIX_OPTION "DIX"
$ This message is a selectable menu option.
$ DIX is a technical term.  See general notes.

BNC_OPTION "BNC"
$ This message is a selectable menu option.
$ BNC is a technical term.  See general notes.

RESULTS "ETHERNET SERVICE AID RESULTS"
$
$ This line is the title for the results menu, which displays one
$ of the previous messages with _ERR in its name.
$ Leave line spacing as shown.  See general notes on length of title.

$ Note to the translator : LPP stands for Licensed Product Program
$
LPP_DEVICE_CANNOT_UNCONFIGURED "\
LPP DEVICE COULD NOT BE CONFIGURED\n\
No trouble was found. However, the system \n\
indicated that the resource could not be \n\
unconfigured.\n\n\
The resource is\n\
- %1$s %2$s    Ethernet Adapter"

$ Note to the translator : LPP stands for Licensed Product Program
$

LPP_DEVICE_CANNOT_SET_TO_DIAGNOSE "\
LPP DEVICE COULD NOT BE SET TO DIAGNOSE STATE\n\
No trouble was found.  However, the system \n\
indicated that the resource could not be set \n\
to DIAGNOSE state.\n\n\
The resource is\n\
- %1$s  %2$s  Ethernet Adapter"

$ Note to the translator : LPP stands for Licensed Product Program
$
$ DEFINE state of the device : the device is defined in the system
$ DIAGNOSE state of the device : the device is in diagnostics state

LPP_DEVICE_CANNOT_SET_TO_DEFINE "\
LPP DEVICE COULD NOT BE SET TO DEFINE STATE\n\
No trouble was found.  However, the system \n\
indicated that the resource could not be set \n\
from DIAGNOSE state to DEFINE state.\n\n\
The resource is\n\
- %1$s %2$s Ethernet Adapter"

$ Note to the translator : LPP stands for Licensed Product Program
$

DIAG_DEVICE_CANNOT_CONFIGURED "\
DIAGNOSTICS DEVICE COULD NOT BE CONFIGURED\n\
No trouble was found.  However, the system \n\
indicated that the resource could not be \n\
configured.\n\n\
The resource is\n\
- %1$s %2$s Ethernet Adapter"

$ Note to the translator : LPP stands for Licensed Product Program
$

DIAG_DEVICE_CANNOT_UNCONFIGURED "\
DIAGNOSTICS DEVICE COULD NOT BE UNCONFIGURED\n\
No trouble was found.  However, the system \n\
indicated that the resource could not be \n\
unconfigured.\n\n\
The resource is\n\
- %1$s %2$s Ethernet Adapter"

$ Note to the translator : LPP stands for Licensed Product Program
$

LPP_DEVICE_CANNOT_CONFIGURED " \
LPP DEVICE COULD NOT BE CONFIGURED\n\
No trouble was found.  However, the system \n\
indicated that the resource could not be \n\
configured.\n\n\
The resource is\n\
- %1$s %2$s Ethernet Adapter"

