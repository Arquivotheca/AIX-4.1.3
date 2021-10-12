$ @(#)24	1.5.2.4  src/bos/diag/da/tok/dtoken.msg, datok, bos411, 9428A410j 12/9/93 14:57:56
$
$   COMPONENT_NAME: DATOK
$
$   FUNCTIONS: 
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

TESTING "\
TESTING THE TOKEN-RING NETWORK ADAPTER IN LOCATION %1$s"
$ 
$ This message is displayed as the title to most messages.
$ %1$s is the location code of the device as described in the general notes.
$ See general notes on length of title line.

TESTING_ADVANCED "\
TESTING THE TOKEN-RING NETWORK ADAPTER IN LOCATION %1$s\n\
IN ADVANCED MODE"
$ 
$ This message is displayed as the title to most messages.
$ %1$s is the location code of the device as described in the general notes.
$ See general notes on length of title line.

STANDBY "\
TESTING THE TOKEN-RING NETWORK ADAPTER IN LOCATION %1$s\n\n\
Please stand by."
$ 
$ This message is displayed when the Token-Ring Diagnostic
$ Application begins execution.
$ %1$s is the location code of the device as described in the general notes.
$ Leave line spacing as shown.  See general notes on length of title line.

STANDBY_ADVANCED "\
TESTING THE TOKEN-RING NETWORK ADAPTER IN LOCATION %1$s\n\
IN ADVANCED MODE\n\n\
Please stand by."
$ 
$ This message is displayed when the Token-Ring Diagnostic
$ Application begins execution in advanced mode.
$ %1$s is the location code of the device as described in the general notes.
$ Leave line spacing as shown.  See general notes on length of title line.

TESTING_INLM "\
TESTING THE TOKEN-RING NETWORK ADAPTER IN LOCATION %1$s\n\
IN ADVANCED MODE\n\n\
Testing multiple times.\n\n\
%2$6d\tpasses completed.\n\
%3$6d\terrors logged.\n\n\
To stop the test, use 'Cancel.'"
$ 
$ This message is displayed while testing is performed in
$ loop mode.
$ %1$s is the location code of the device as described in the general notes.
$ %2$6d\t sets the number of diagnostics test passes completed.
$ %3$6d\t sets the number of errors logged during the tests.
$ Leave line spacing as shown.  See general notes on length of title line.

NETWORK_UP "\
The diagnostic tests indicate that this system cannot\n\
communicate with the Token-Ring Network.  This may be because\n\
the Token-Ring Network adapter does not have a cable connecting\n\
it to the Token-Ring Network.\n\n\
Is your Token-Ring Network supposed to be working at this time?\n\n\
Move cursor to selection, then press Enter."
$ 
$ This message is displayed if the open test unit fails because the
$ wrap failed.
$ Leave line spacing as shown.
 
WIRING_TYPE "\
What type of attachment is used to connect the Token-Ring Network\n\
adapter to the network?\n\n\
(Note: The type 3 media filter has a 6 pin modular telephone connector\n\
on the end that plugs into the Token-Ring Network.)\n\n\
Move cursor to selection, then press Enter."
$ 
$ The DA first asks the user what type of wiring is used.
$ Leave line spacing as shown.
 
NONTELE_CABLE "Token-Ring Network Cable"
$ This message is a selectable menu option.

TELE_CABLE "Type 3 Media Filter"
$ This message is a selectable menu option.

UNKNOWN_CABLE "Type not known"
$ This message is for a user who is unable to make a choice.

CHECK_CABLE "\
ENSURE.........that the cable from the Token-Ring Network adapter\n\
               is properly connected to the Token-Ring Network\n\
               and is not damaged.\n\n\
When finished, press Enter\n."
$ 
$ If there is a problem with the test units, the DA asks the user
$ to make sure that there is a cable attaching the adapter to the
$ network.  This menu is displayed if the user indicated that a normal
$ Token-Ring cable was being used.
$ Leave line spacing as shown.  See general notes on expanding ACTION lines.
 
CHECK_CABLE_TELE "\
ENSURE.........that the cable from the type 3 media filter to the\n\
               6 pin modular telephone connector is properly connected\n\
	       and not damaged.\n\n\
When finished, press Enter.\n"
$ 
$ If there is a problem with the test units, the DA asks the user
$ to make sure that there is a cable attaching the adapter to the
$ network.  This menu is displayed if the user indicated that a Type 3
$ Media Filter was being used.
$ Leave line spacing as shown.  See general notes on expanding ACTION lines.
 
DETACH_CABLE "\
UNPLUG.........the Token-Ring Network Cable from the network, leaving it\n\
               connected to the adapter.\n\n\
When finished, press Enter.\n"
$ 
$ This menu is used to help the user set up for wrap testing if a regular
$ Token-Ring cable is used.
$ Leave line spacing as shown.  See general notes on expanding ACTION lines.
 
ATTACH_WRAP "\
UNPLUG.........the type 3 media filter, if attached, from the\n\
               Token-Ring Network adapter.\n\
PLUG...........the wrap connector that was sent with this cable\n\
               into the Token-Ring Network adapter.\n\n\
When finished, press Enter.\n"
$ 
$ This menu is used to help the user set up for wrap testing if a Type 3
$ Media Filter is used.
$ Leave line spacing as shown.  See general notes on expanding ACTION lines.
  
NET_BAD "85000C \
CHECK TOKEN-RING NETWORK\n\n\
The Token-Ring adapter fails its self-tests\n\
while attached to the network, but passes its self-tests\n\
when detached from the network.  Have the network checked.\n"
$ 
$ This message is displayed by the diagnostic controller if the
$ Token-Ring tests indicate that the Token-Ring Network is probably the
$ failing feild replaceable unit.
$ DO NOT TRANSLATE the number at the beginning of this message - 85000C.
$ Leave the number exactly how it is shown.
$ Leave line spacing as shown.  See general notes on length of title.

ELA_ADAPTER_OR_NET_BAD_PD0 "85000D \
ERROR LOG ANALYSIS FOUND A TOKEN-RING PROBLEM\n\n\
The Token-Ring adapter in location %1$s was tested and\n\
no problem was found.  However, error log analysis found\n\
a problem which was caused by either the adapter\n\
or the Token-Ring Network.\n"
$ 
$ DO NOT TRANSLATE the number at the beginning of this message - 85000D.
$ Leave the number exactly how it is shown.
$ %1$s is the location code of the device as described in the general notes.
$ Leave line spacing as shown.  See general notes on length of title.

ELA_ADAPTER_OR_NET_BAD_PD1 "85000E \
ERROR LOG ANALYSIS FOUND A TOKEN-RING PROBLEM\n\n\
The Token-Ring adapter in location %1$s was busy and\n\
could not be tested.  However, error log analysis found\n\
a problem which was caused by either the adapter\n\
or the Token-Ring Network.  Vary the resource off and\n\
test it for further isolation of the problem.\n"
$ 
$ DO NOT TRANSLATE the number at the beginning of this message - 85000E.
$ Leave the number exactly how it is shown.
$ vary the resource off means to remove all items using it so that
$ the diagnostics can test the device fully.
$ %1$s is the location code of the device as described in the general notes.
$ Leave line spacing as shown.  See general notes on length of title.

ELA_ADAPTER_OR_NET_BAD_ELA "85000F \
ERROR LOG ANALYSIS FOUND A TOKEN-RING PROBLEM\n\n\
The Token-Ring adapter in location %1$s was not tested.\n\
However, error log analysis found a problem which was\n\
caused by either the adapter or the Token-Ring Network.\n\
Test the device for further isolation of the problem.\n"
$ 
$ DO NOT TRANSLATE the number at the beginning of this message - 85000F.
$ Leave the number exactly how it is shown.
$ %1$s is the location code of the device as described in the general notes.
$ Leave line spacing as shown.  See general notes on length of title.

ELA_NET_BAD_PD0 "850010 \
ERROR LOG ANALYSIS FOUND A TOKEN-RING PROBLEM\n\n\
The Token-Ring adapter in location %1$s was tested and\n\
no problem was found.  However, error log analysis found\n\
a problem which was caused by the Token-Ring Network.\n\
Have the network checked.\n"
$ 
$ DO NOT TRANSLATE the number at the beginning of this message - 850010.
$ Leave the number exactly how it is shown.
$ %1$s is the location code of the device as described in the general notes.
$ Leave line spacing as shown.  See general notes on length of title.

ELA_NET_BAD_PD1 "850011 \
ERROR LOG ANALYSIS FOUND A TOKEN-RING PROBLEM\n\n\
The Token-Ring adapter in location %1$s was busy and\n\
could not be tested.  However, error log analysis found\n\
a problem which was caused by the Token-Ring Network.\n\
Vary the resource off and test the Token-Ring adapter.\n\
If no problem is found, have the network checked.\n"
$ 
$ DO NOT TRANSLATE the number at the beginning of this message - 850011.
$ Leave the number exactly how it is shown.
$ vary the resource off means to remove all items using it so that
$ the diagnostics can test the device fully.
$ %1$s is the location code of the device as described in the general notes.
$ Leave line spacing as shown.  See general notes on length of title.

ELA_NET_BAD_ELA "850012 \
ERROR LOG ANALYSIS FOUND A TOKEN-RING PROBLEM\n\n\
The Token-Ring adapter in location %1$s was not tested.\n\
However, error log analysis found a problem which was\n\
caused by the Token-Ring Network.  Test the adapter.\n\
If no problem is found, have the network checked.\n"
$ 
$ DO NOT TRANSLATE the number at the beginning of this message - 850012.
$ Leave the number exactly how it is shown.
$ %1$s is the location code of the device as described in the general notes.
$ Leave line spacing as shown.  See general notes on length of title.

DETACH_WRAP "\
UNPLUG.........the wrap connector from the Token-Ring Network adapter.\n\
PLUG...........the type 3 media filter, if it was removed, into the\n\
               Token-Ring Network adapter.\n\n\
When finished, press Enter.\n"
$ 
$ If Type 3 Media Filter is used, this menu is used for cleanup after
$ wrap tests.
$ Leave line spacing as shown.  See general notes on expanding ACTION lines.

ATTACH_CABLE "\
PLUG...........the Token-Ring Network cable, if it was removed, into\n\
               the network.\n\n\
When finished, press Enter.\n"
$ 
$ If regular cable is used, this menu is used for cleanup after wrap tests.
$ Leave line spacing as shown.  See general notes on expanding ACTION lines.

YES_OPTION "YES"
$ This option is shown when a YES answer is possible.

NO_OPTION "NO"
$ This option is shown when a NO answer is possible.

HAVE_WRAP "\
This test requires the use of the wrap connector\n\
that came with the type 3 media filter.\n\n\
Do you have this wrap connector?\n\n\
Move cursor to selection, then press Enter."
$
$ This menu is used to determine whether the wrap test can be run if a
$ Type 3 Media Filter is used.
$ Leave line spacing as shown.

MICROCODE_BAD "850103 \
THE MICROCODE FOR TOKEN RING IS BAD\n\
The microcode for Token-Ring is either missing or not accessible.\n\n\
The microcode is necessary in order to use Tokenring in normal system\n\
operation or to run Diagnostics."
$ The microcode file in /etc/microcode/ is bad
$ The card is not configured. It is called in configure device
$ and return value is -1
$ Leave line spacing as shown.

TOK_DD_FILE_BAD "850104 \
The Token-Ring diagnositics device driver is either missing or not \n\
accessible. The diagnostics device driver is necessary in order to \n\
to run Diagnostics."

$ The device driver is either missing or not accessible
$ Leave line spacing as shown.

LPP_DEVICE_CANNOT_UNCONFIGURED "850105 \
THE LICENSED PRODUCT PROGRAM (LPP) DEVICE CANNOT BE UNCONFIGURED\n\
No trouble was found.  However, the LPP device may not be operational\n\
because the system indicated that the resource could not be unconfigured.\n\n\
The resource is\n\
- %1$s             %2$s         Token-Ring High-Performance Adapter"


LPP_DEVICE_CANNOT_SET_TO_DIAGNOSE "850106 \
THE LICENSED PRODUCT PROGRAM (LPP) DEVICE CANNOT BE SET TO DIAGNOSE STATE\n\
No trouble was found.  However, the LPP device may not be operational\n\
because the system indicated that the resource could not be set to DIAGNOSE\n\
state.\n\n\
The resource is\n\
- %1$s             %2$s         Token-Ring High-Performance Adapter"


LPP_DEVICE_CANNOT_SET_TO_DEFINE "850107 \
THE LICENSED PRODUCT PROGRAM (LPP) DEVICE CANNOT BE SET TO DEFINE STATE\n\
No trouble was found.  However, the LPP device may not be operational\n\
because the system indicated that the resource could not be set from DIAGNOSE\n\
state to DEFINE state.\n\n\
The resource is\n\
- %1$s             %2$s         Token-Ring High-Performance Adapter"

DIAG_DEVICE_CANNOT_CONFIGURED "850108 \
THE DIAGNOSTICS DEVICE CANNOT BE CONFIGURED\n\
No trouble was found.  However, the diagnostics device may not be operational\n\
because the system indicated that the resource could not be configured.\n\n\
The resource is\n\
- %1$s             %2$s         Token-Ring High-Performance Adapter"


DEVICE_DOES_NOT_EXIST "850109 \
THE SYSTEM INDICATES THAT THE DEVICE DOES NOT EXIST. \n\
No trouble was found.  However, the because the system indicated that \n\
the resource does not exist.\n\n\
The resource is\n\
- %1$s             %2$s         Token-Ring High-Performance Adapter"

DIAG_DEVICE_CANNOT_UNCONFIGURED "850110 \
THE DIAGNOSTICS DEVICE CANNOT BE UNCONFIGURED\n\
No trouble was found.  However, the diagnostics device may not be operational\n\
because the system indicated that the resource could not be unconfigured.\n\n\
The resource is\n\
- %1$s             %2$s         Token-Ring High-Performance Adapter"


LPP_DEVICE_CANNOT_CONFIGURED "850111 \
THE LPP(LICENSED PRODUCT PROGRAM) DEVICE CANNOT BE CONFIGURED\n\
No trouble was found.  However, the LPP device may not be operational\n\
because the system indicated that the resource could not be configured.\n\n\
The resource is\n\
- %1$s             %2$s         Token-Ring High-Performance Adapter"


$ Token-Ring adapter messages  (SRN)


$set TOKENRING_SRN 

TOKNETMSG "Token-Ring Network"
CABLEMSG  "Token-Ring Cable"
TOK_RMSG_1 "The Token-Ring adapter open test failed at ring speed 4MB."
TOK_RMSG_2 "The Token-Ring wrap data test failed at ring speed 4MB."
TOK_RMSG_3 "Not able to set Token-Ring speed to 4 MB."
TOK_RMSG_4 "The Token-Ring adapter open test failed at ring speed 16MB."
TOK_RMSG_5 "The Token-Ring wrap data test failed at ring speed 16MB."
TOK_RMSG_6 "Not able to set Token-Ring speed to 16 MB."
TOK_RMSG_7 "An error occurred that could not be identified."
TOK_RMSG_8 "Error log analysis indicates hardware failure."
TOK_RMSG_9 "Tokenring configuration fails."
TOK_RMSG_10 "The device driver indicates a hardware failure."
TOK_RMSG_11 "Software."

$
