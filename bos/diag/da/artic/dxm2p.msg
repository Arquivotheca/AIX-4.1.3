$ @(#)93	1.3  src/bos/diag/da/artic/dxm2p.msg, daartic, bos411, 9428A410j 5/12/94 19:33:31
$
$   COMPONENT_NAME: DAARTIC
$
$   FUNCTIONS: plug
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
$quote "
$set 1

TESTING_ADVANCED_MESSAGE_PM "\
TESTING %1$s - %2$s \n\
IN LOCATION %3$s IN ADVANCED MODE."

PLEASE_STANDBY_PM "Please stand by."

YES_OPTION_PM "YES"
$ This option is shown when a YES answer is possible.

NO_OPTION_PM "NO"
$ This option is shown when a NO answer is possible.

HAVE_WRAP_PLUG_PM "\
The following test requires a wrap plug, Part Number %1$s.\n\n\
Do you have this wrap plug?\n\n\
Move cursor to selection, then press Enter."
$
$  %1$s is the part number of the wrap plug used for test. Do not translate.
$

BIG_WRAP_PLUG_8PORT_232 "16F2478"
BIG_WRAP_PLUG_8PORT_422 "16F2478"
BIG_WRAP_PLUG_6PORT_232 "15F8859"
BIG_WRAP_PLUG_4PORT_232 "16F2478"

WRAP_PLUG_1 "64F5494"
WRAP_PLUG_2 "64F5494"
WRAP_PLUG_3 "64F5494"


INSERT_GENERIC_WRAP_PLUG_PM "\
UNPLUG.........the multiport interface cable assembly, if attached, from the\n\
	       %1$s.\n\
PLUG...........the wrap plug (Part Number %2$s) into the\n\
	       %3$s.\n\n\
When finished, press Enter."

REMOVE_GENERIC_WRAP_PLUG_PM "\
UNPLUG.........the wrap plug from the\n\
	       %1$s.\n\
PLUG...........the multiport interface cable assembly, if it was removed,\n\
	       into the %2$s.\n\n\
When finished, press Enter."

INSERT_GENERIC_WRAP_PLUG_INTO_PORT "\
UNPLUG.........the cable, if attached, from connector # %1$d\n\
               of the interface cable assembly.\n\
PLUG...........the wrap plug (Part Number %2$s) into connector # %3$d\n\
               of the interface cable assembly.\n\n\
When finished, press Enter."

REMOVE_GENERIC_WRAP_PLUG_INTO_PORT  "\
UNPLUG.........the wrap plug from connector # %1$d\n\
               of the interface cable assembly.\n\
CONNECT........any network cables that were attached to connector # %2$d\n\
               of the interface cable assembly before testing began.\n\n\
When finished, press Enter."

CABLE_NOTE_PM "\
The following cable test is used to test interconnecting cables\n\
that are used between the multiport interface cable assembly\n\
and peripheral equipment.\n\n\
NOTE:  It is assumed that the cables being tested are 'pin for pin' \n\
       compatible with the connectors on the interface cable.\n\n\
Is testing the cable required?\n\n\
Move cursor to selection, then press Enter."

PORTS_NOTE_PM "\
Is testing individual connector of the interface cable assembly required?\n\n\
Move cursor to selection, then press Enter."


CABLE_PORT_0  "%1$s %2$s cable attached to Port 0 -  Connector # 0"
CABLE_PORT_1  "%1$s %2$s cable attached to Port 1 -  Connector # 1"
CABLE_PORT_2  "%1$s %2$s cable attached to Port 2 -  Connector # 2"
CABLE_PORT_3  "%1$s %2$s cable attached to Port 3 -  Connector # 3"
CABLE_PORT_4  "%1$s %2$s cable attached to Port 4 -  Connector # 4"
CABLE_PORT_5  "%1$s %2$s cable attached to Port 5 -  Connector # 5"
CABLE_PORT_6  "%1$s %2$s cable attached to Port 6 -  Connector # 6"
CABLE_PORT_7  "%1$s %2$s cable attached to Port 7 -  Connector # 7"

CONNECTOR_PORT_0  "%1$s %2$s Port 0 -  Connector # 0"
CONNECTOR_PORT_1  "%1$s %2$s Port 1 -  Connector # 1"
CONNECTOR_PORT_2  "%1$s %2$s Port 2 -  Connector # 2"
CONNECTOR_PORT_3  "%1$s %2$s Port 3 -  Connector # 3"
CONNECTOR_PORT_4  "%1$s %2$s Port 4 -  Connector # 4"
CONNECTOR_PORT_5  "%1$s %2$s Port 5 -  Connector # 5"
CONNECTOR_PORT_6  "%1$s %2$s Port 6 -  Connector # 6"
CONNECTOR_PORT_7  "%1$s %2$s Port 7 -  Connector # 7"

TESTED_PM "          * "
$ Prefix string for connectors that have been tested
$ Do not translate this message, unless an '*' is not available.

NOT_TESTED_PM "            "
$ Prefix string for connectors that have not been tested
$ This is a blank message


V_35 "V.35"
$ embedded into the menu text

RS_232 "RS- 232"
$ embedded into the menu text

RS_422 "RS-422"
$ embedded into the menu text

X_21 "X.21"
$ embedded into the menu text

INTERFACE_CABLE_MESSAGE_PM "\
Move the cursor to the interface cable connector to be tested,\n\
then press Enter to continue.\n\
%1$s"

NOTE_ASTERISK_PM "\n\
NOTE: '*' Indicates that the connector has been tested successfully."

NOTE_ASTERISK_CABLE_PM "\n\
NOTE: '*' Indicates that the cable has been tested successfully."
$
$ See set notes for information on how to expand lines if necessary.
$ Leave line spacing as shown.  See general notes on user ACTIONS.

CABLE_SELECTION_MESSAGE_PM "\
Move the cursor to the cable to be tested\n\
and press Enter to continue.\n\
%1$s"

INSERT_GENERIC_CABLE_PM "\
REMOVE.........any wrap plug or cable if attached from connector # %1$d\n\
               of the interface cable assembly.\n\
CONNECT........%2$s cable to be tested to connector # %3$d \n\
               of the interface cable assembly.\n\
INSERT.........the wrap plug (Part number %4$s) into the other end \n\
               of the cable to be tested.\n\n\
When finished, press Enter."

REMOVE_WRAP_PLUG_FROM_CABLE_PM "\
REMOVE.........any wrap plug from the tested cable.\n\
CONNECT........any network cables that were attached to connector # %1$d \n\
               of the interface cable assembly before testing began.\n\n\
When finished, press Enter."

INSERT_WRAP_AFTER_BAD_CABLE_TEST "\
Problem isolation procedure.\n\n\
REMOVE.........any wrap plug from the tested cable.\n\
REMOVE.........%1$s tested cable  from  connector # %2$d \n\
               of the interface cable assembly.\n\
INSERT.........the wrap plug (Part number %3$s) into connector # %4$d\n\
               of the interface cable assembly.\n\n\
When finished, press Enter."

INSERT_GENERIC_WRAP_INTO_PORT "\
REMOVE.........any cable if attached from connector # %1$d\n\
               of the interface cable assembly.\n\
INSERT.........the wrap plug (Part number %2$s) into connector # %3$d\n\
               of the interface cable assembly.\n\n\
When finished, press Enter."

REMOVE_GENERIC_WRAP_FROM_PORT "\
REMOVE.........any wrap plug from connector # %1$d\n\
               of the interface cable assembly.\n\
CONNECT........any network cables that were attached to connector # %1$d \n\
               of the interface cable assembly before testing began.\n\n\
When finished, press Enter."


SKIPPING_TEST "\
NOTE:  Skipping these tests will mean the adapter\n\
       will NOT be tested completely."

CABLE_V35_PM "V.35   Cable"
CABLE_232_PM "RS 232 Cable"
CABLE_X21_PM "X.21   Cable"
CABLE_422_PM "RS 422 Cable"

wp_0 "0x009F1803"
$  78 pin MP2 rs232 rs422 EIB wrap plug

wp_1 "0x015F8856"
$  78 pin MP2 6P Sync EIB wrap plug

wp_2 "15F8848"
$ 100 pin PM 8P-rs232 EIB wrap plug

wp_3 "57F0678"
$ 100 pin PM 8P-rs422 EIB wrap plug

wp_4 "72F0168"
$ 100 pin PM V.35 EIB wrap plug

wp_5 "85F0205"
$ 78 pin PM X.21 EIB wrap plug

wp_6 "6425494"
$ 25 pin MP2 ports 0 & 1 wrap plug

wp_7 "09F1799"
$ 25 pin MP2 ports 2 - 7 wrap plug

wp_8 "33F8968"
$ 25 pin MP2 ports 0 - 5 wrap plug

wp_9 "33F8964"
$ 25 pin PM 8P-rs422 cable wrap plug

wp_a "33F8985"
$ 25 pin PM 8P-rs232 cable wrap plug

wp_b "72F0167" 
$ 25 pin PM V.35 cable wrap plug

wp_c "85F0206"
$ 25 pin PM X.21 cable wrap plug

BIG_WRAP_PLUG_8_4PORT "09F1803"

BIG_WRAP_PLUG_6_PORT "15F8856"

MDB "Molded Distribution Box (interface cable)"

