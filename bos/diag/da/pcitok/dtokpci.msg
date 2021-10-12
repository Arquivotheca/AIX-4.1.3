$ @(#)98	1.1  src/bos/diag/da/tokpci/dtokpci.msg, datokpci, bos41J 3/21/95 16:44:05
$
$   COMPONENT_NAME: DATOKPCI
$
$   FUNCTIONS: 
$		
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
$ This is the message file for the Token-Ring Network adapter,
$ which allows the system to communicate with other systems
$ over the Token-Ring Network.  The adapter can connect to the
$ network in one of two ways: over an IBM cable or over telephone
$ wiring.
$
$ END OF GENERAL NOTES


$set 1
$quote "

TESTING_REGULAR "\
TESTING THE %1$s %2$s\n\
IN LOCATION %3$s\n\n\
This test may require up to 5 minutes.\n\n\
Please stand by."
$ 
$ This message is displayed as the title to most messages.
$ %1$s is the description of the device.  This is retrieved from devices.cat
$ %2$s is the name of the device
$ %3$S is the location of the device
$ See general notes on length of title line.

TESTING_ADVANCED_MODE "\
TESTING THE %1$s %2$s\n\
IN LOCATION %3$s IN ADVANCED MODE\n\n\
This test may require up to 5 minutes.\n\n\
Please stand by."
$ 
$ This message is displayed as the title to most messages.
$ %1$s is the description of the device.  This is retrieved from devices.cat
$ %2$s is the name of the device
$ %3$S is the location of the device
$ See general notes on length of title line.

TESTING_ADVANCED_NO_STANDBY  "\
TESTING THE %1$s %2$s\n\
IN LOCATION %3$s IN ADVANCED MODE"


TESTING_LOOPMODE "\
TESTING THE %1$s %2$s\n\
IN LOCATION %3$s IN ADVANCED MODE\n\n\
Testing multiple times.\n\n\
%4$6d\tpasses completed.\n\
%5$6d\terrors logged.\n\n\
To stop the test, use 'Cancel.'"
$ 
$ This message is displayed while testing is performed in
$ loop mode.
$ %1$s is the description of the device.  This is retrieved from devices.cat
$ %2$s is the name of the device
$ %3$S is the location of the device
$ %4$6d\t sets the number of diagnostics test passes completed.
$ %5$6d\t sets the number of errors logged during the tests.
$ Leave line spacing as shown.  See general notes on length of title line.

NETWORK_UP "\
The diagnostic tests indicate that this system cannot\n\
communicate with the token-ring network.  This may be because\n\
the token-ring network adapter does not have a cable connecting\n\
it to the token-ring network.\n\n\
Is your token-ring network supposed to be working at this time?\n\n\
Move cursor to selection, then press Enter."
$ 
$ This message is displayed if the open test unit fails because the
$ wrap failed.
$ Leave line spacing as shown.
 
WIRING_TYPE "\
Select the type of cable that is used to connect the token-ring Network\n\
adapter to the network?\n\n\
(Note: The type 3 media filter has a 6 pin modular telephone connector\n\
on the end that plugs into the token-ring Network.)\n\n\
Move cursor to selection, then press Enter."
$ 
$ The DA first asks the user what type of wiring is used.
$ Leave line spacing as shown.

PART_NUMBER_1 "60G1063"
PART_NUMBER_2 "60G1066"

YES_OPTION "YES"

NO_OPTION  "NO"

CABLE_REQUIRE "\
The following test requires a token-ring network cable \n\
(Part Number %1$s) or equivalent.\n\n\
Do you have this cable?\n\n\
Move cursor to selection, then press Enter."
 
OPTION "Move cursor to selection, then press Enter "
$ Leave line spacing as shown.  See general notes on length of title line.

FINISHED "\
When finished, press Enter."
$

NONTELE_CABLE "token-ring network cable"
$ This message is a selectable menu option.

TELE_CABLE "type 3 media filter"
$ This message is a selectable menu option.

NO_CABLE "no Cable"
$ This message is a selectable menu option.

UNKNOWN_TYPE "Type not known"

NETWORK_CONNECTED "\
Is the token-ring network cable connected to a Token-Ring network?\n\n\
Move cursor to selection, then press Enter."

PLUG_CABLE_TYPE_3 "\
PLUG...........the token-ring network cable (Part Number 60G1066) into\n\
               the connector on the token-ring adapter, if not already\n\
               plugged in.\n\
PLUG...........the Type 3 Media Filter into the other end of the token-ring\n\
               network cable.\n\
ENSURE.........the other end of Type 3 Media Filter is attached\n\
               to the network.\n\n\
NOTE:          If the token-ring network adapter is not connected to\n\
               the network, the token-ring network adapter will NOT\n\
               be tested completely."
PLUG_CABLE "\
PLUG...........the token-ring network cable (Part Number 60G1063) into\n\
               the connector on the token-ring adapter, if not already\n\
               plugged in.\n\
ENSURE.........the other end of token-ring network cable is attached\n\
               to the network.\n\n\
NOTE:          If the token-ring network adapter is not connected to\n\
               the network, the token-ring network adapter will NOT\n\
               be tested completely."

PUT_CABLE_BACK "\
PLUG...........the token-ring network cable back into the network,\n\
               if it was plugged before testing."

UNPLUG_CABLE "\
UNPLUG.........the token-ring network cable from the token-ring network.\n\n\
ENSURE.........the other end of the token-ring network cable is connected\n\
               to the token-ring network adapter."


$set TOKENRING_SRN 

TOK_NET_MSG "Token-ring Network"
CABLE_MSG  "Token-ring Cable"
TOK_200 "Config register test failure"
TOK_201  "I/O register test failure"
TOK_202 "Adapter self-test Failure"
TOK_203 "Connect test failure"
TOK_204 "Token-ring internal wrap failure"
TOK_300 "Token-ring external wrap failure"
TOK_400 "Token-ring initialization failure"
TOK_700 "Error log analysis indicates hardware failure."
TOK_SOFT_ERROR "Software."
$
