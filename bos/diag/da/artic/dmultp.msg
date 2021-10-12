$ @(#)68	1.1  src/bos/diag/da/artic/dmultp.msg, daartic, bos411, 9428A410j 4/1/94 15:17:45
$
$   COMPONENT_NAME: DAARTIC
$
$   FUNCTIONS: n
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
$ Message catalog file for the MultiProtocol Quad Port Adapter
$
$ END OF GENERAL NOTES


$quote "
$set 1 

MPQP_ADVANCED_NO_STANDBY "\
TESTING 4-PORT MULTIPROTOCOL COMMUNICATIONS CONTROLLER %1$s\n\
IN LOCATION %2$s IN ADVANCED MODE"



DO_YOU_HAVE_78_PIN_WRAP_PLUG "\
The following test requires a 78-pin wrap plug \n\
(Part Number 40F9902).\n\n\
NOTE:  Skipping these tests will mean the adapter\n\
       will NOT be tested completely.\n\n\
Do you have this wrap plug?\n\n\
Move cursor to selection, then press Enter."

$

YES_OPTION_MPQP "YES"

NO_OPTION_MPQP "NO"


INSERT_78_PIN_WRAP_PLUG "\
REMOVE.........78-pin connector end of the 4-Port Multiprotocol\n\
               interface cable assembly from the adapter.\n\
INSERT.........the 78-pin wrap plug (Part Number 40F9902)\n\
               into the adapter.\n"

MPQP_FINISHED "\
When finished, press Enter."

NO_PORT_TO_RUN "\
The system indicates that there is no port associated with this adapter.\n\
Please re-seat the adapter, the daughter card and rerun diagnostics."

PORT_MENU "\
Port...........%3$s\n\
Test Type......INTERFACE CABLE CONNECTOR WRAP TEST\n\n\n\
Move the cursor to the connector to be tested  of the 4-Port Multiprotocol \n\
interface cable assembly,\n\
then press Enter to continue.\n\
NOTE: '*' Indicates that the connector has been tested successfully."

TESTED "          * "
$ Prefix string for connectors that have been tested
$ Do not translate this message, unless an '*' is not available.

NOT_TESTED "            "
$ Prefix string for connectors that have not been tested
$ This is a blank message


P0_V35 "%1$s Connector 0 Port 0 - V.35"
$ embedded into the menu text

P0_232 "%1$s Connector 1 Port 0 - 232D"
$ embedded into the menu text

P0_X21 "%1$s Connector 2 Port 0 - X.21"
$ embedded into the menu text

P0_422 "%1$s Connector 3 Port 0 - 422A"
$ embedded into the menu text

P1_232 "%1$s Connector 4 Port 1 - 232D"
$ embedded into the menu text

P1_V35 "%1$s Connector 5 Port 1 - V.35"
$ embedded into the menu text

P3_232  "%1$s Connector 6 Port 3 - 232D"
$ embedded into the menu text

P2_232 "%1$s Connector 7 Port 2 - 232D"
$ embedded into the menu text

P2_422 "%1$s Connector 8 Port 2 - 422A"
$ embedded into the menu text

INDIVIDUAL_PORT_CONNECTOR "\
Is testing individual connector of the 4-Port Multiprotocol \n\
interface cable assembly required for adapter %1$s?\n\n\
Move cursor to selection, then press Enter."

CABLE_TEST_FOR_EACH_PORT "\
Is testing Port %1$d connector(s) of the 4-Port Multiprotocol \n\
interface cable assembly required for adapter %2$s?\n\n\
Move cursor to selection, then press Enter."

$ Do not translate %1$d  
$ Do not translate %2$s

REMOVE_78_PIN_WRAP_PLUG "\
REMOVE.........the 78-pin wrap plug from the adapter.\n\
ATTACH.........78-pin connector end of the 4-Port Multiprotocol \n\
               interface cable assembly to the adapter." 

INTERFACE_CABLE_MESSAGE "\
Move the cursor to the interface cable connector to be tested,\n\
then press Enter to continue.\n\
%1$s"

NOTE_ASTERISK "\n\
NOTE: '*' Indicates that the connector has been tested successfully."

NOTE_ASTERISK_CABLE "\n\
NOTE: '*' Indicates that the cable has been tested successfully."

$ Do  not translate '*'

WRAP_PLUG_232 "Part Number 40F9903"
$

WRAP_PLUG_V35 "Part Number 40F9900"
$

WRAP_PLUG_X21 "Part Number 40F9904"
$

WRAP_PLUG_422 "Part Number 53F3886"
$

GENERIC_WRAP_PLUG "\
The following test requires a wrap plug (%1$s).\n\n\
NOTE:  Skipping these tests will mean the adapter\n\
       will NOT be tested completely.\n\n\
Do you have this wrap plug?\n\n\
Move cursor to selection, then press Enter."


REMOVE_CABLE "%1$s"

REMOVE_PORT_0_CABLE "\
REMOVE.........all cables (if any) from Port 0."
$ See general notes on expanding ACTION lines.

REMOVE_PORT_1_CABLE "\
REMOVE.........all cables (if any) from Port 1."
$ See general notes on expanding ACTION lines.

REMOVE_PORT_2_CABLE "\
REMOVE.........all cables (if any) from Port 2."
$ See general notes on expanding ACTION lines.

REMOVE_PORT_3_CABLE "\
REMOVE.........any cables (if any) from Port 3."
$ See general notes on expanding ACTION lines.


INSERT_WRAP_PLUG "\
INSERT.........the wrap plug (%1$s)\n\
               into %2$s of the the 4-Port Multiprotocol \n\
               interface cable assembly  (78-pin connector cable)"

CABLE_PORT_0_V35  "%1$s V.35   cable attached to Port 0 - V.35. Connector # 0"
PORT_0_V35 "Port 0 - V.35"
$ embedded into the menu text

CABLE_PORT_0_232  "%1$s RS 232 cable attached to Port 0 - 232D. Connector # 1"
PORT_0_232  "Port 0 - 232D"
$ embedded into the menu text

CABLE_PORT_0_X21  "%1$s X.21   cable attached to Port 0 - X.21. Connector # 2"
PORT_0_X21 "Port 0 - X.21"
$ embedded into the menu text

CABLE_PORT_0_422  "%1$s RS 422 cable attached to Port 0 - 422A. Connector # 3"
PORT_0_422 "Port 0 - 422A"
$ embedded into the menu text

CABLE_PORT_1_232  "%1$s RS 232 cable attached to Port 1 - 232D. Connector # 4"
PORT_1_232 "Port 1 - 232D"
$ embedded into the menu text

CABLE_PORT_1_V35  "%1$s V.35   cable attached to Port 1 - V.35. Connector # 5"
PORT_1_V35 "Port 1 - V.35"
$ embedded into the menu text

CABLE_PORT_3_232  "%1$s RS 232 cable attached to Port 3 - 232D. Connector # 6"
PORT_3_232 "Port 3 - 232D"
$ embedded into the menu text

CABLE_PORT_2_232  "%1$s RS 232 cable attached to Port 2 - 232D. Connector # 7"
PORT_2_232 "Port 2 - 232D"
$ embedded into the menu text

CABLE_PORT_2_422  "%1$s RS 422 cable attached to Port 2 - 422A. Connector # 8"
PORT_2_422 "Port 2 - 422A"
$ embedded into the menu text

REMOVE_WRAP_PLUG "\
REMOVE.........any wrap plug or test cable from %1$s."

REMOVE_WRAP_PLUG_FROM_CABLE "\
REMOVE.........any wrap plug from the tested cable."

RECONNECT_CABLE "%1$s"

RECONNECT_CABLE_PORT_0 "\
CONNECT........any network cables that were attached to Port 0 \n\
               before testing began."

RECONNECT_CABLE_PORT_1 "\
CONNECT........any network cables that were attached to Port 1 \n\
               before testing began."

RECONNECT_CABLE_PORT_2 "\
CONNECT........any network cables that were attached to Port 2 \n\
               before testing began."

RECONNECT_CABLE_PORT_3 "\
CONNECT........any network cables that were attached to Port 3 \n\
               before testing began."

CABLE_NOTE "\
The following cable test is used to test interconnecting cables\n\
that are used between the 4-Port Multiprotocol interface cable assembly\n\
and peripheral equipment.\n\n\
NOTE:  It is assumed that the cables being tested are 'pin for pin' \n\
       compatible with the connectors on the interface cable.\n\n\
Is testing the cable required?\n\n\
Move cursor to selection, then press Enter."

$
CABLE_V35 "%1$s         V.35   Cable"
$ embedded into the menu text

CABLE_232 "%1$s         RS 232 Cable"
$ embedded into the menu text

CABLE_X21 "%1$s         X.21   Cable"
$ embedded into the menu text

CABLE_422 "%1$s         RS 422 Cable"
$ embedded into the menu text

$
CABLE_V35_BAD "V.35   Cable"
CABLE_232_BAD "RS 232 Cable"
CABLE_X21_BAD "X.21   Cable"
CABLE_422_BAD "RS 422 Cable"


CABLE_SELECTION_MESSAGE "\
Move the cursor to the type of cable to be tested\n\
and press Enter to continue.\n\
%1$s"

CONNECT_CABLE "\
REMOVE.........any wrap plug or cable if attached from %1$s\n\
               of the interface cable.\n\
CONNECT........the cable to be tested to %2$s \n\
               of the interface cable.\n\
INSERT.........the wrap plug (%3$s) into the other end of \n\
               the cable to be tested."

CABLE_ISOLATE "\
The following test is used for problem isolation between \n\
the tested cable and the adapter.\n\n\
REMOVE.........any wrap plug from of the tested cable.\n\
REMOVE.........the tested cable from %1$s of the interface cable.\n\
INSERT.........the wrap plug (%2$s)\n\
               into %3$s of the interface cable."
