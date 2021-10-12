$ @(#)27        1.9.3.5  src/bos/diag/da/async/dasync.msg, daasync, bos41J, 9517A_all 4/19/95 20:42:22
$ $ *
$ $ * COMPONENT_NAME: DAASYNC
$ $ *
$ $ * FUNCTIONS:   This file contains the English language messages for the
$ $ *              Standard Serial Port, 8 Port, 16 Port, 64 Port , and 128
$ $ *              port adapter diagnostic application menus.
$ $ *
$ $ * ORIGINS: 27
$ $ *
$ $ * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ $ * combined with the aggregated modules for this product)
$ $ *                  SOURCE MATERIALS
$ $ * (C) COPYRIGHT International Business Machines Corp. 1989, 1995
$ $ * All Rights Reserved
$ $ *
$ $ * US Government Users Restricted Rights - Use, duplication or
$ $ * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$ $ *
$
$
$ GENERAL NOTES
$
$ Do not translate %d, %s, %x, %1$s, or \t in any message.  They
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
$set ASYNC_DIAG
$
$ These comments concern the technical terms used in messages.
$   EIA-232:  is a standard asynchronous communications interface.  EIA is
$      a technical term.  Do not translate unless an equivalent term is
$      used in translating country.
$   EIA-422:  is a standard asynchronous communications interface.  EIA is
$      a technical term.  Do not translate unless an equivalent term is
$      used in translating country.
$   MIL-STD 188:  is a standard asynchronous communications interface.
$      MIL-STD is a technical term for military standard.  Do not translate
$      unless an equivalent term is used in translating country.
$   interposer:  is a small connector that fits between a cable and a device,
$      converting a modem cable to a cable that will interface with some
$      other serial device.

DM_1A   "\
TESTING STANDARD SERIAL PORT %1$s\n\n\
Please stand by."
$
$ %1$s is the number of the serial port that is being tested.
$ Leave line spacing as shown.  See general notes on length of title line.

DM_1B   "\
TESTING STANDARD SERIAL PORT %1$s IN ADVANCED MODE"
$
$ %1$s is the number of the serial port that is being tested.
$ See general notes on length of title line.

DM_1C   "\
TESTING STANDARD SERIAL PORT %1$s IN ADVANCED MODE\n\n\
Please stand by."
$
$ %1$s is the number of the serial port that is being tested.
$ Leave line spacing as shown.  See general notes on length of title line.

DM_1D   "\
TESTING STANDARD SERIAL PORT %1$s IN ADVANCED MODE\n\n\
Testing multiple times.\n\n\
%2$6d\tpasses completed.\n\
%3$6d\terrors logged.\n\n\
To stop the test, use 'Cancel.'"
$
$ %1$s is the number of the serial port that is being tested.
$ %2$6d\t sets the number of diagnostic test passes completed.
$ %3$6d\t sets the number of errors logged during the tests.
$ Leave line spacing as shown.  See general notes on length of title line.

DM_2A   "\
TESTING PORT %1$x ON THE EIA-232 %2$x-PORT ASYNCHRONOUS ADAPTER IN\n\
LOCATION %3$s\n\n\
Please stand by."
$
$ %1$x is the port number being tested.
$ %2$x is the number of ports supported by the adapter - 8, 16, or 64.
$ %3$s is the location of the planar slot containing the adapter.
$ The location code of the device is described in the general notes.
$ EIA 232 is a standard asynchronous communications interface.  See set notes.
$ Leave line spacing as shown.  See general notes on length of title line.

DM_2B   "\
TESTING PORT %1$x ON THE EIA-232 %2$x-PORT ASYNCHRONOUS ADAPTER IN\n\
LOCATION %3$s IN ADVANCED MODE"
$
$ %1$x is the port number being tested.
$ %2$x is the number of ports supported by the adapter - 8, 16, or 64.
$ %3$s is the location of the planar slot containing the adapter.
$ The location code of the device is described in the general notes.
$ EIA 232 is a standard asynchronous communications interface.  See set notes.
$ Leave line spacing as shown.  See general notes on length of title line.

DM_2C   "\
TESTING PORT %1$x ON THE EIA-232 %2$x-PORT ASYNCHRONOUS ADAPTER IN\n\
LOCATION %3$s IN ADVANCED MODE\n\n\
Please stand by."
$
$ %1$x is the port number being tested.
$ %2$x is the number of ports supported by the adapter - 8, 16, or 64.
$ %3$s is the location of the planar slot containing the adapter.
$ The location code of the device is described in the general notes.
$ EIA 232 is a standard asynchronous communications interface.  See set notes.
$ Leave line spacing as shown.  See general notes on length of title line.

DM_2D   "\
TESTING PORT %1$x ON THE EIA-232 %2$x-PORT ASYNCHRONOUS ADAPTER IN\n\
LOCATION %3$s IN ADVANCED MODE\n\n\
Testing multiple times.\n\n\
%4$6d\tpasses completed.\n\
%5$6d\terrors logged.\n\n\
To stop the test, use 'Cancel.'"
$
$ %1$x is the port number being tested.
$ %2$x is the number of ports supported by the adapter - 8, 16, or 64.
$ %3$s is the location of the planar slot containing the adapter.
$ The location code of the device is described in the general notes.
$ %4$6d\t sets the number of diagnostic test passes completed.
$ %5$6d\t sets the number of errors logged during the tests.
$ EIA 232 is a standard asynchronous communications interface.  See set notes.
$ Leave line spacing as shown.  See general notes on length of title line.

DM_3A   "\
TESTING PORT %1$x ON THE EIA-422 %2$x-PORT ASYNCHRONOUS ADAPTER IN\n\
LOCATION %3$s\n\n\
Please stand by."
$
$ %1$x is the port number being tested.
$ %2$x is the number of ports supported by the adapter - 8, 16, or 64.
$ %3$s is the location of the planar slot containing the adapter.
$ The location code of the device is described in the general notes.
$ EIA 422 is a standard asynchronous communications interface.  See set notes.
$ Leave line spacing as shown.  See general notes on length of title line.

DM_3B   "\
TESTING PORT %1$x ON THE EIA-422 %2$x-PORT ASYNCHRONOUS ADAPTER IN\n\
LOCATION %3$s IN ADVANCED MODE"
$
$ %1$x is the port number being tested.
$ %2$x is the number of ports supported by the adapter - 8, 16, or 64.
$ %3$s is the location of the planar slot containing the adapter.
$ The location code of the device is described in the general notes.
$ EIA 422 is a standard asynchronous communications interface.  See set notes.
$ Leave line spacing as shown.  See general notes on length of title line.

DM_3C   "\
TESTING PORT %1$x ON THE EIA-422 %2$x-PORT ASYNCHRONOUS ADAPTER IN\n\
LOCATION %3$s IN ADVANCED MODE\n\n\
Please stand by."
$
$ %1$x is the port number being tested.
$ %2$x is the number of ports supported by the adapter - 8, 16, or 64.
$ %3$s is the location of the planar slot containing the adapter.
$ The location code of the device is described in the general notes.
$ EIA 422 is a standard asynchronous communications interface.  See set notes.
$ Leave line spacing as shown.  See general notes on length of title line.

DM_3D   "\
TESTING PORT %1$x ON THE EIA-422 %2$x-PORT ASYNCHRONOUS ADAPTER IN\n\
LOCATION %3$s IN ADVANCED MODE\n\n\
Testing multiple times.\n\n\
%4$6d\tpasses completed.\n\
%5$6d\terrors logged.\n\n\
To stop the test, use 'Cancel.'"
$
$ %1$x is the port number being tested.
$ %2$x is the number of ports supported by the adapter - 8, 16, or 64.
$ %3$s is the location of the planar slot containing the adapter.
$ The location code of the device is described in the general notes.
$ %4$6d\t sets the number of diagnostic test passes completed.
$ %5$6d\t sets the number of errors logged during the tests.
$ EIA 232 is a standard asynchronous communications interface.  See set notes.
$ Leave line spacing as shown.  See general notes on length of title line.

DM_4A   "\
TESTING PORT %1$x ON THE MIL-STD 188 %2$x-PORT ASYNCHRONOUS ADAPTER IN\n\
LOCATION %3$s\n\n\
Please stand by."
$
$ %1$x is the port number being tested.
$ %2$x is the number of ports supported by the adapter - 8, 16, or 64.
$ %3$s is the location of the planar slot containing the adapter.
$ The location code of the device is described in the general notes.
$ MIL 188 is a standard asynchronous communications interface.  See set notes.
$ Leave line spacing as shown.  See general notes on length of title line.

DM_4B   "\
TESTING PORT %1$x ON THE MIL-STD 188 %2$x-PORT ASYNCHRONOUS ADAPTER IN\n\
LOCATION %3$s IN ADVANCED MODE"
$
$ %1$x is the port number being tested.
$ %2$x is the number of ports supported by the adapter - 8, 16, or 64.
$ %3$s is the location of the planar slot containing the adapter.
$ The location code of the device is described in the general notes.
$ MIL 188 is a standard asynchronous communications interface.  See set notes.
$ Leave line spacing as shown.  See general notes on length of title line.

DM_4C   "\
TESTING PORT %1$x ON THE MIL-STD 188 %2$x-PORT ASYNCHRONOUS ADAPTER IN\n\
LOCATION %3$s IN ADVANCED MODE\n\n\
Please stand by."
$
$ %1$x is the port number being tested.
$ %2$x is the number of ports supported by the adapter - 8, 16, or 64.
$ %3$s is the location of the planar slot containing the adapter.
$ The location code of the device is described in the general notes.
$ MIL 188 is a standard asynchronous communications interface.  See set notes.
$ Leave line spacing as shown.  See general notes on length of title line.

DM_4D   "\
TESTING PORT %1$x ON THE MIL-STD 188 %2$x-PORT ASYNCHRONOUS ADAPTER IN\n\
LOCATION %3$s IN ADVANCED MODE\n\n\
Testing multiple times.\n\n\
%4$6d\tpasses completed.\n\
%5$6d\terrors logged.\n\n\
To stop the test, use 'Cancel.'"
$
$ %1$x is the port number being tested.
$ %2$x is the number of ports supported by the adapter - 8, 16, or 64.
$ %3$s is the location of the planar slot containing the adapter.
$ The location code of the device is described in the general notes.
$ %4$6d\t sets the number of diagnostic test passes completed.
$ %5$6d\t sets the number of errors logged during the tests.
$ MIL 188 is a standard asynchronous communications interface.  See set notes.
$ Leave line spacing as shown.  See general notes on length of title line.

DM_5A   "\
TESTING PORT %1$x ON CONCENTRATOR %2$x ATTACHED TO THE 64-PORT\n\
CONTROLLER IN LOCATION %3$s\n\n\
Please stand by."
$
$ %1$x is the port number being tested.
$ %2$x is the concentrator number being tested.
$ %3$s is the location of the planar slot containing the 64-port adapter.
$ The location code of the device is described in the general notes.
$ Leave line spacing as shown.  See general notes on length of title line.

DM_5B   "\
TESTING PORT %1$x ON CONCENTRATOR %2$x ATTACHED TO THE 64-PORT\n\
CONTROLLER IN LOCATION %3$s IN ADVANCED MODE"
$
$ %1$x is the port number being tested.
$ %2$x is the concentrator number being tested.
$ %3$s is the location of the planar slot containing the 64-port adapter.
$ The location code of the device is described in the general notes.
$ Leave line spacing as shown.  See general notes on length of title line.

DM_5C   "\
TESTING PORT %1$x ON CONCENTRATOR %2$x ATTACHED TO THE 64-PORT\n\
CONTROLLER IN LOCATION %3$s IN ADVANCED MODE\n\n\
Please stand by."
$
$ %1$x is the port number being tested.
$ %2$x is the concentrator number being tested.
$ %3$s is the location of the planar slot containing the 64-port adapter.
$ The location code of the device is described in the general notes.
$ Leave line spacing as shown.  See general notes on length of title line.

DM_5D   "\
TESTING PORT %1$x ON CONCENTRATOR %2$x ATTACHED TO THE 64-PORT\n\
CONTROLLER IN LOCATION %3$s IN ADVANCED MODE\n\n\
Testing multiple times.\n\n\
%4$6d\tpasses completed.\n\
%5$6d\terrors logged.\n\n\
To stop the test, use 'Cancel.'"
$
$ %1$x is the port number being tested.
$ %2$x is the concentrator number being tested.
$ %3$s is the location of the planar slot containing the 64-port adapter.
$ The location code of the device is described in the general notes.
$ %4$6d\t sets the number of diagnostic test passes completed.
$ %5$6d\t sets the number of errors logged during the tests.
$ Leave line spacing as shown.  See general notes on length of title line.

DM_6    "\
The following test requires a wrap plug, Part Number %1$s.\n\n\
Do you have this wrap plug?\n\n\
Move cursor to selection, then press Enter."
$
$ %1$s is the part number of the wrap plug used for test. Do not translate.
$ Leave line spacing as shown.

DM_7    "YES"
$ This option is shown when a YES answer is possible.

DM_8    "NO"
$ This option is shown when a NO answer is possible.

DM_9    "Which type of device is attached to the port that is being tested?\n\n\
Move cursor to selection, then press Enter."
$
$ Leave line spacing as shown.

DM_10   "Terminal"
$ Possible answer to a question.
$ The terminal is an asyncronous ascii terminal device

DM_11   "Modem"
$ Possible answer to a question.

DM_12   "Printer or plotter"
$ Possible answer to a question.

DM_13   "No device attached"
$ Possible answer to a question.

DM_14A_OLD  ""
$ This message has been changed.  See end of set for new message.

DM_14B_OLD  ""
$ This message has been changed.  See end of set for new message.

DM_14C_OLD  ""
$ This message has been changed.  See end of set for new message.

DM_14D_OLD  ""
$ This message has been changed.  See end of set for new message.

DM_15A_OLD   " "
$ This message has been changed.  See end of set for new message.

DM_15B_OLD   " "
$ This message has been changed.  See end of set for new message.

DM_15C_OLD   " "
$ This message has been changed.  See end of set for new message.

DM_15D_OLD   " "
$ This message has been changed.  See end of set for new message.

DM_16   "\
IS THE ADAPTER UNDER TEST\n\n"
$
$ Question for user.

DM_17_OLD  ""
$ This message has been changed.  See end of set for new message.

DM_18  "\
This message will be an adapter error that looks like a FRU"
$
$ FRU - Field Replacable Unit

DM_19   "\
UNPLUG.........the converter cable, if attached, from the remote async node.\n\
PLUG...........the wrap plug (Part Number %1$s) into the remote async node.\n\n\
When finished, press Enter."
$
$ $ 128 port controller remote async node test unit 50.
$ %1$s is the part number of the wrap plug used for test. Do not translate.
$ The remote async node is a unit that provides the physical space
$ to plug in all the devices running off of the adapter.
$ See set notes for information on how to expand lines if necessary.
$ Leave line spacing as shown.  See general notes on user ACTIONS.

DM_20   "\
UNPLUG.........the wrap plug from the remote async node.\n\n\
PLUG...........the converter cable, if it was removed, into\n\
               the remote async node.\n\n\
When finished, press Enter."
$
$ $ 128 port controller remote async node test unit 50.
$ The remote async node is a unit that provides the physical space
$ to plug in all the devices running off of the adapter.
$ See set notes for information on how to expand lines if necessary.
$ Leave line spacing as shown.  See general notes on user ACTIONS.

DM_21   " "
$ This message is no longer used.

DM_22   " "
$ This message is no longer used.

DM_23_OLD ""
$ This message has changed.  See end of set for new message.

DM_24_OLD ""
$ This message has changed.  See end of set for new message.

DM_25    "836118 \
REMOTE ASYNC NODE FAILURE.\n\n\
Make sure that the Remote Async Node under test is powered on.  If\n\
it is, reboot the Remote Async Node by switching the power off and\n\
and then on again.  The front panel display should reach the \'AC\'\n\
state.\n\n\
If the front panel display shows \'En\', followed by a reboot, the\n\
Remote Async Node node number is invalid and must be reprogrammed.\n\
For instructions on how to perform this operation, refer to the\n\
installation instructions.\n\n\
If the front panel display shows \'Ed\', followed by a reboot, the\n\
Remote Async Node has detected a fatal hardware error during Power\n\
On Self Test.  Service is required."
$
$ The remote async node is a unit that provides the physical space
$ to plug in all the devices running off of the adapter.
$ See set notes for information on how to expand lines if necessary.
$ Leave line spacing as shown.  See general notes on user ACTIONS.

DM_26   "\
UNPLUG.........the controller cable, if attached, from the 128-port\n\
               controller.\n\
PLUG...........the wrap plug (Part Number %1$s) into the 128-port\n\
               controller.\n\n\
When finished, press Enter."
$
$ $ 128 port controller test unit 30.
$ %1$s is the part number of the wrap plug used for test. Do not translate.
$ See set notes for information on how to expand lines if necessary.
$ Leave line spacing as shown.  See general notes on user ACTIONS.

DM_27   "\
UNPLUG.........the wrap plug from the 128-port controller.\n\n\
PLUG...........the controller cable, if it was removed, into the 128-port\n\
               adapter.\n\n\
When finished, press Enter."
$
$ $ 128 port controller test unit 30.
$ See set notes for information on how to expand lines if necessary.
$ Leave line spacing as shown.  See general notes on user ACTIONS.

DM_28   "\
DIAGNOSTIC SELECTION FOR THE 128-PORT CONTROLLER IN LOCATION %1$s"
$
$ %1$s is the location of the planar slot containing the 128-port adapter.
$ The location code of the device is described in the general notes.
$ Leave line spacing as shown.  See general notes on length of title line.

DM_29   "\
ADVANCED DIAGNOSTIC SELECTION FOR THE 128-PORT CONTROLLER IN\n\
LOCATION %1$s"
$
$ %1$s is the location of the planar slot containing the 128-port adapter.
$ The location code of the device is described in the general notes.
$ Leave line spacing as shown.  See general notes on length of title line.

DM_30   " "
$ This message is no longer used.

DM_31   " "
$ This message is no longer used.

DM_32   " "
$ This message is no longer used.

DM_33   " "
$ This message is no longer used.

DM_32A  " "
$ This message is no longer used.

DM_33A  " "
$ This message is no longer used.

DM_34   "\
UNPLUG.........the controller cable, if attached, from the 64-port\n\
               controller.\n\
PLUG...........the wrap plug (Part Number %1$s) into the 64-port\n\
               controller.\n\n\
When finished, press Enter."
$
$ $ 64 port controller test unit 30.
$ %1$s is the part number of the wrap plug used for test. Do not translate.
$ See set notes for information on how to expand lines if necessary.
$ Leave line spacing as shown.  See general notes on user ACTIONS.

DM_35   "\
UNPLUG.........the wrap plug from the 64-port controller.\n\n\
PLUG...........the controller cable, if it was removed, into the 64-port\n\
               adapter.\n\n\
When finished, press Enter."
$
$ $ 64 port controller test unit 30.
$ See set notes for information on how to expand lines if necessary.
$ Leave line spacing as shown.  See general notes on user ACTIONS.

DM_36A  "\
DIAGNOSTIC SELECTION FOR STANDARD SERIAL PORT %1$s"
$
$ %1$s is the number of the serial port that is being tested.
$ Leave line spacing as shown.  See general notes on length of title line.

DM_37B   "\
ADVANCED DIAGNOSTIC SELECTION FOR STANDARD SERIAL PORT %1$s"
$
$ %1$s is the number of the serial port that is being tested.
$ See general notes on length of title line.

DM_38A   "\
DIAGNOSTIC SELECTION FOR THE EIA-232 %1$x-PORT ASYNCHRONOUS ADAPTER IN\n\
LOCATION %2$s"
$
$ %1$x is the number of ports supported by the adapter - 8, 16, or 64.
$ %2$s is the location of the planar slot containing the adapter.
$ The location code of the device is described in the general notes.
$ EIA 232 is a standard asynchronous communications interface.  See set notes.
$ Leave line spacing as shown.  See general notes on length of title line.

DM_38B   "\
ADVANCED DIAGNOSTIC SELECTION FOR THE EIA-232 %1$x-PORT ASYNCHRONOUS\n\
ADAPTER IN LOCATION %2$s"
$
$ %1$x is the number of ports supported by the adapter - 8, 16, or 64.
$ %2$s is the location of the planar slot containing the adapter.
$ The location code of the device is described in the general notes.
$ EIA 232 is a standard asynchronous communications interface.  See set notes.
$ Leave line spacing as shown.  See general notes on length of title line.

DM_39A   "\
DIAGNOSTIC SELECTION FOR THE EIA-422 %1$x-PORT ASYNCHRONOUS ADAPTER IN\n\
LOCATION %2$s"
$
$ %1$x is the number of ports supported by the adapter - 8, 16, or 64.
$ %2$s is the location of the planar slot containing the adapter.
$ The location code of the device is described in the general notes.
$ EIA 422 is a standard asynchronous communications interface.  See set notes.
$ Leave line spacing as shown.  See general notes on length of title line.

DM_39B   "\
ADVANCED DIAGNOSTIC SELECTION FOR THE EIA-422 %1$x-PORT ASYNCHRONOUS\n\
ADAPTER IN LOCATION %2$s"
$
$ %1$x is the number of ports supported by the adapter - 8, 16, or 64.
$ %2$s is the location of the planar slot containing the adapter.
$ The location code of the device is described in the general notes.
$ EIA 422 is a standard asynchronous communications interface.  See set notes.
$ Leave line spacing as shown.  See general notes on length of title line.

DM_40A   "\
DIAGNOSTIC SELECTION FOR THE MIL-STD 188 %1$x-PORT ASYNCHRONOUS\n\
ADAPTER IN LOCATION %2$s"
$
$ %1$x is the number of ports supported by the adapter - 8, 16, or 64.
$ %2$s is the location of the planar slot containing the adapter.
$ The location code of the device is described in the general notes.
$ MIL 188 is a standard asynchronous communications interface.  See set notes.
$ Leave line spacing as shown.  See general notes on length of title line.

DM_40B   "\
ADVANCED DIAGNOSTIC SELECTION FOR THE MIL-STD 188 %1$x-PORT ASYNCHRONOUS\n\
ADAPTER IN LOCATION %2$s"
$
$ %1$x is the number of ports supported by the adapter - 8, 16, or 64.
$ %2$s is the location of the planar slot containing the adapter.
$ The location code of the device is described in the general notes.
$ MIL 188 is a standard asynchronous communications interface.  See set notes.
$ Leave line spacing as shown.  See general notes on length of title line.

DM_41A   "\
DIAGNOSTIC SELECTION FOR THE 64-PORT CONTROLLER IN LOCATION %1$s"
$
$ %1$s is the location of the planar slot containing the 64-port adapter.
$ The location code of the device is described in the general notes.
$ Leave line spacing as shown.  See general notes on length of title line.

DM_41B   "\
ADVANCED DIAGNOSTIC SELECTION FOR THE 64-PORT CONTROLLER IN\n\
LOCATION %1$s"
$
$ %1$s is the location of the planar slot containing the 64-port adapter.
$ The location code of the device is described in the general notes.
$ Leave line spacing as shown.  See general notes on length of title line.

DM_42   "\
CONFIGURING ASYNCHRONOUS ADAPTER\n\n\
Please stand by."
$
$ Leave line spacing as shown.  See general notes on length of title line.

DM_43   "\
tty* indicates that the port is not configured.\n\n\
Choose the port that you want to test."
$
$ Do not translate tty*.  It is a port and should not be translated.
$ Leave line spacing as shown.

DM_44   "\
UNPLUG.........the interposer, if attached, from the terminal.\n\
PLUG...........the wrap plug (Part Number %1$s) into the interposer.\n\n\
When finished, press Enter."
$
$ $ rs232 and mil188 adapters.
$ 8, 16, and 64 port and standard serial port test unit 80.
$ %1$s is the part number of the wrap plug used for test. Do not translate.
$ The terminal is an asyncronous ascii terminal device
$ See set notes for information on how to expand lines if necessary.
$ See set notes for explanation of an interposer.
$ Leave line spacing as shown.  See general notes on user ACTIONS.

DM_45   "\
UNPLUG.........the wrap plug from the interposer.\n\
PLUG...........the interposer, if it was removed, into the terminal.\n\n\
When finished, press Enter."
$
$ $ rs232 and mil188 adapters.
$ 8, 16, and 64 port and standard serial port test unit 80.
$ The terminal is an asyncronous ascii terminal device
$ See set notes for information on how to expand lines if necessary.
$ See set notes for explanation of an interposer.
$ Leave line spacing as shown.  See general notes on user ACTIONS.

DM_46   "\
UNPLUG.........the interposer, if attached, from the printer or plotter.\n\
PLUG...........the wrap plug (Part Number %1$s) into the interposer.\n\n\
When finished, press Enter."
$
$ $ rs232 and mil188 adapters.
$ 8, 16, and 64 port and standard serial port test unit 80.
$ %1$s is the part number of the wrap plug used for test. Do not translate.
$ See set notes for information on how to expand lines if necessary.
$ See set notes for explanation of an interposer.
$ Leave line spacing as shown.  See general notes on user ACTIONS.

DM_47   "\
UNPLUG.........the wrap plug from the interposer.\n\
PLUG...........the interposer, if it was removed, into the printer or\n\
               plotter.\n\n\
When finished, press Enter."
$
$ $ rs232 and mil188 adapters.
$ 8, 16, and 64 port and standard serial port test unit 80.
$ See set notes for information on how to expand lines if necessary.
$ See set notes for explanation of an interposer.
$ Leave line spacing as shown.  See general notes on user ACTIONS.

DM_48   "\
UNPLUG.........the cable, if attached, from the modem.\n\
PLUG...........the wrap plug (Part Number %1$s) into the cable.\n\n\
When finished, press Enter."
$
$ $ rs232 and mil188 adapters.
$ 8, 16, and 64 port and standard serial port test unit 70.
$ %1$s is the part number of the wrap plug used for test. Do not translate.
$ See set notes for information on how to expand lines if necessary.
$ Leave line spacing as shown.  See general notes on user ACTIONS.

DM_49   "\
UNPLUG.........the wrap plug from the cable.\n\
PLUG...........the cable, if it was removed, into the modem.\n\n\
When finished, press Enter."
$
$ $ rs232 and mil188 adapters.
$ 8, 16, and 64 port and standard serial port test unit 70.
$ See set notes for information on how to expand lines if necessary.
$ Leave line spacing as shown.  See general notes on user ACTIONS.

DM_50   "\
UNPLUG.........the interposer, if attached, from the cable.\n\
PLUG...........the wrap plug (Part Number %1$s) into the cable.\n\n\
When finished, press Enter."
$
$ $ rs232 and mil188 adapters.
$ 8, 16, and 64 port and standard serial port test unit 70.
$ %1$s is the part number of the wrap plug used for test. Do not translate.
$ See set notes for information on how to expand lines if necessary.
$ See set notes for explanation of an interposer.
$ Leave line spacing as shown.  See general notes on user ACTIONS.

DM_51   "\
UNPLUG.........the wrap plug from the cable.\n\
PLUG...........the interposer, if it was removed, into the cable.\n\n\
When finished, press Enter."
$
$ $ rs232 and mil188 adapters.
$ 8, 16, and 64 port and standard serial port test unit 70.
$ See set notes for information on how to expand lines if necessary.
$ See set notes for explanation of an interposer.
$ Leave line spacing as shown.  See general notes on user ACTIONS.

DM_52   "\
UNPLUG.........the cable, if attached, from the terminal.\n\
PLUG...........the wrap plug (Part Number %1$s) into the cable.\n\n\
When finished, press Enter."
$
$ $ rs422 adapter
$ 8 and 16 port test unit 70.
$ %1$s is the part number of the wrap plug used for test. Do not translate.
$ The terminal is an asyncronous ascii terminal device
$ See set notes for information on how to expand lines if necessary.
$ Leave line spacing as shown.  See general notes on user ACTIONS.

DM_53   "\
UNPLUG.........the wrap plug from the cable.\n\
PLUG...........the cable, if it was removed, into the terminal.\n\n\
When finished, press Enter."
$
$ $ rs422 adapter
$ 8 and 16 port test unit 70.
$ The terminal is an asyncronous ascii terminal device
$ See set notes for information on how to expand lines if necessary.
$ Leave line spacing as shown.  See general notes on user ACTIONS.

DM_54   "\
UNPLUG.........the cable, if attached, from the printer or plotter.\n\
PLUG...........the wrap plug (Part Number %1$s) into the cable.\n\n\
When finished, press Enter."
$
$ $ rs422 adapter
$ 8 and 16 port test unit 70.
$ %1$s is the part number of the wrap plug used for test. Do not translate.
$ The terminal is an asyncronous ascii terminal device
$ See set notes for information on how to expand lines if necessary.
$ Leave line spacing as shown.  See general notes on user ACTIONS.

DM_55   "\
UNPLUG.........the wrap plug from the cable.\n\
PLUG...........the cable, if it was removed, into the printer or plotter.\n\n\
When finished, press Enter."
$
$ $ rs422 adapter
$ 8 and 16 port test unit 70.
$ The terminal is an asyncronous ascii terminal device
$ See set notes for information on how to expand lines if necessary.
$ Leave line spacing as shown.  See general notes on user ACTIONS.

DM_56   "\
UNPLUG.........the cable, if attached, from the converter cable.\n\
PLUG...........the wrap plug (Part Number %1$s) into the converter\n\
               cable.\n\n\
When finished, press Enter."
$
$ $ 64 port and standard serial port test unit 60.
$ %1$s is the part number of the wrap plug used for test. Do not translate.
$ See set notes for information on how to expand lines if necessary.
$ Leave line spacing as shown.  See general notes on user ACTIONS.

DM_57   "\
UNPLUG.........the wrap plug from the converter cable.\n\
PLUG...........the cable, if it was removed, into the converter cable.\n\n\
When finished, press Enter."
$
$ $ 64 port and standard serial port test unit 60.
$ See set notes for information on how to expand lines if necessary.
$ Leave line spacing as shown.  See general notes on user ACTIONS.

DM_58   "\
UNPLUG.........the cable, if attached, from the multiport interface cable\n\
               connector.\n\
PLUG...........the wrap plug (Part Number %1$s) into the multiport\n\
               interface cable connector.\n\n\
When finished, press Enter."
$
$ $ 8 and 16 port adapter interface cable assembly test unit 50.
$ %1$s is the part number of the wrap plug used for test. Do not translate.
$ "multiport" is the fru term for multiple port.
$ See set notes for information on how to expand lines if necessary.
$ Leave line spacing as shown.  See general notes on user ACTIONS.

DM_59   "\
UNPLUG.........the wrap plug from the multiport interface cable connector.\n\
PLUG...........the cable, if it was removed, into the multiport interface\n\
               cable connector.\n\n\
When finished, press Enter."
$
$ $ 8 and 16 port adapter interface cable assembly test unit 50.
$ "multiport" is the fru term for multiple port.
$ See set notes for information on how to expand lines if necessary.
$ Leave line spacing as shown.  See general notes on user ACTIONS.

DM_60   "\
UNPLUG.........the converter cable, if attached, from the concentrator.\n\
PLUG...........the wrap plug (Part Number %1$s) into the concentrator.\n\n\
When finished, press Enter."
$
$ $ 64 port controller concentrator test unit 50.
$ %1$s is the part number of the wrap plug used for test. Do not translate.
$ The concentrator is a unit that provides the physical space
$ to plug in all the devices running off of the adapter.
$ See set notes for information on how to expand lines if necessary.
$ Leave line spacing as shown.  See general notes on user ACTIONS.

DM_61   "\
UNPLUG.........the wrap plug from the concentrator.\n\n\
PLUG...........the converter cable, if it was removed, into\n\
               the concentrator.\n\n\
When finished, press Enter."
$
$ $ 64 port controller concentrator test unit 50.
$ The concentrator is a unit that provides the physical space
$ to plug in all the devices running off of the adapter.
$ See set notes for information on how to expand lines if necessary.
$ Leave line spacing as shown.  See general notes on user ACTIONS.

DM_62   "\
UNPLUG.........the controller cable, if attached, from the concentrator.\n\
PLUG...........the wrap plug (Part Number %1$s) into the controller\n\
               cable.\n\n\
When finished, press Enter."
$
$ $ 64 port controller cable test unit 40.
$ %1$s is the part number of the wrap plug used for test. Do not translate.
$ The concentrator is a unit that provides the physical space
$ to plug in all the devices running off of the adapter.
$ See set notes for information on how to expand lines if necessary.
$ Leave line spacing as shown.  See general notes on user ACTIONS.

DM_63   "\
UNPLUG.........the wrap plug from the controller cable.\n\
PLUG...........the controller cable, if it was removed, into the\n\
               concentrator.\n\n\
When finished, press Enter."
$
$ $ 64 port controller cable test unit 40.
$ The concentrator is a unit that provides the physical space
$ to plug in all the devices running off of the adapter.
$ See set notes for information on how to expand lines if necessary.
$ Leave line spacing as shown.  See general notes on user ACTIONS.

DM_64_OLD   ""
$ See below for updated DM_64 message

DM_65_OLD   ""
$ See below for updated DM_65 message

DM_66   "\
UNPLUG.........the converter cable, if attached, from the standard serial\n\
               port connector.\n\
PLUG...........the wrap plug (Part Number %1$s) into the standard serial\n\
               port connector.\n\n\
When finished, press Enter."
$
$ $ standard serial port test unit 30.
$ %1$s is the part number of the wrap plug used for test. Do not translate.
$ See set notes for information on how to expand lines if necessary.
$ Leave line spacing as shown.  See general notes on user ACTIONS.

DM_67   "\
UNPLUG.........the wrap plug from the standard serial port connector.\n\
PLUG...........the converter cable, if it was removed, into the standard\n\
               serial port connector.\n\n\
When finished, press Enter."
$
$ $ standard serial port test unit 30.
$ See set notes for information on how to expand lines if necessary.
$ Leave line spacing as shown.  See general notes on user ACTIONS.

DM_68   "\
UNPLUG.........the multiport cable assembly, if attached, from the 8-port\n\
               adapter.\n\
PLUG...........the wrap plug (Part Number %1$s) into the 8-port\n\
               adapter.\n\n\
When finished, press Enter."
$
$ $ 8 port adapter test unit 30.
$ %1$s is the part number of the wrap plug used for test. Do not translate.
$ See set notes for information on how to expand lines if necessary.
$ Leave line spacing as shown.  See general notes on user ACTIONS.

DM_69   "\
UNPLUG.........the wrap plug from the 8-port adapter.\n\
PLUG...........the multiport interface cable assembly, if it was removed,\n\
               into the 8-port adapter.\n\n\
When finished, press Enter."
$
$ $ 8 port adapter test unit 30.
$ See set notes for information on how to expand lines if necessary.
$ Leave line spacing as shown.  See general notes on user ACTIONS.

DM_70_OLD   ""
$ See below for updated DM_70 message.

DM_71_OLD   ""
$ See below for updated DM_71 message.

DM_72   "\
UNPLUG.........the multiport cable assembly, if attached, from the 16-port\n\
               adapter.\n\
PLUG...........the wrap plug (Part Number %1$s) into the 16-port\n\
               adapter.\n\n\
When finished, press Enter."
$
$ $ 16 port adapter test unit 30.
$ %1$s is the part number of the wrap plug used for test. Do not translate.
$ "multiport" is the fru term for multiple port.
$ See set notes for information on how to expand lines if necessary.
$ Leave line spacing as shown.  See general notes on user ACTIONS.

DM_73   "\
UNPLUG.........the wrap plug from the 16-port adapter.\n\
PLUG...........the multiport interface cable assembly, if it was removed,\n\
               into the 16-port adapter.\n\n\
When finished, press Enter."
$
$ $ 16 port adapter test unit 30.
$ "multiport" is the fru term for multiple port.
$ See set notes for information on how to expand lines if necessary.
$ Leave line spacing as shown.  See general notes on user ACTIONS.

DM_74_OLD   ""
$ See below for updated DM_74 message.

DM_75_OLD   ""
$ See below for updated DM_75 message.

DM_76   "\
UNPLUG.........the controller cable, if attached, from the 64-port\n\
               controller.\n\
PLUG...........the wrap plug (Part Number %1$s) into the 64-port\n\
               controller.\n\n\
When finished, press Enter."
$
$ $ 64 port controller test unit 30.
$ %1$s is the part number of the wrap plug used for test. Do not translate.
$ See set notes for information on how to expand lines if necessary.
$ Leave line spacing as shown.  See general notes on user ACTIONS.

DM_77   "\
UNPLUG.........the wrap plug from the 64-port controller.\n\n\
PLUG...........the controller cable, if it was removed, into the 64-port\n\
               adapter.\n\n\
When finished, press Enter."
$
$ $ 64 port controller test unit 30.
$ See set notes for information on how to expand lines if necessary.
$ Leave line spacing as shown.  See general notes on user ACTIONS.

DM_78   "Any other device"
$ Possible answer to a question.

DM_79   "64-port to 128-port Pinout Converter Cable"
$ Possible answer to a question.

DM_80   "\
UNPLUG.........the cable, if attached, from the standard serial port\n\
               connector.\n\
PLUG...........the wrap plug (Part Number %1$s) into the standard serial\n\
               port connector.\n\n\
When finished, press Enter."
$
$ $ standard serial port test unit 30.
$ %1$s is the part number of the wrap plug used for test. Do not translate.
$ See set notes for information on how to expand lines if necessary.
$ Leave line spacing as shown.  See general notes on user ACTIONS.

DM_81   "\
UNPLUG.........the wrap plug from the standard serial port connector.\n\
PLUG...........the cable, if it was removed, into the standard serial port\n\
               connector.\n\n\
When finished, press Enter."
$
$ $ standard serial port test unit 30.
$ See set notes for information on how to expand lines if necessary.
$ Leave line spacing as shown.  See general notes on user ACTIONS.

DM_82   "\
The following test requires converter cable, Part Number %1$s to be\n\
attached to the standard serial port connector.\n\n\
Is the converter cable attached?\n\n\
Move cursor to selection, then press Enter."
$
$ %1$s is the part number of the cable to be tested. Do not translate.
$ Leave line spacing as shown.

DM_83   "\
UNPLUG.........the cable, if attached, from the converter cable.\n\
PLUG...........the wrap plug (Part Number %1$s) into the converter\n\
               cable.\n\n\
When finished, press Enter."
$
$ %1$s is the part number of the wrap plug used for test. Do not translate.
$ See set notes for information on how to expand lines if necessary.
$ Leave line spacing as shown.  See general notes on user ACTIONS.

DM_84   "\
UNPLUG.........the wrap plug from the converter cable.\n\
PLUG...........the cable, if it was removed, into the converter cable.\n\n\
When finished, press Enter."
$
$ See set notes for information on how to expand lines if necessary.
$ Leave line spacing as shown.  See general notes on user ACTIONS.

DM_85   "\
UNPLUG.........the converter cable, if attached, from the remote async node.\n\
PLUG...........the wrap plug (Part Number %1$s) into the remote async node.\n\n\
When finished, press Enter."
$
$ %1$s is the part number of the wrap plug used for test. Do not translate.
$ See set notes for information on how to expand lines if necessary.
$ Leave line spacing as shown.  See general notes on user ACTIONS.

DM_86   "\
UNPLUG.........the wrap plug from the remote async node.\n\
PLUG...........the converter cable, if it was removed, into\n\
               the remote async node.\n\n\
When finished, press Enter."
$
$ See set notes for information on how to expand lines if necessary.
$ Leave line spacing as shown.  See general notes on user ACTIONS.

DM_15A   "\
TESTING PORT %1$x ON REMOTE ASYNC NODE %2$s ATTACHED TO THE 128-PORT\n\
CONTROLLER IN LOCATION %3$s\n\n\
Please stand by."
$
$ %1$x is the port number being tested.
$ %2$s is the remote async node name being tested.
$ %3$s is the location of the planar slot containing the 128-port adapter.
$ The location code of the device is described in the general notes.
$ Leave line spacing as shown.  See general notes on length of title line.

DM_15B   "\
TESTING PORT %1$x ON REMOTE ASYNC NODE %2$s ATTACHED TO THE 128-PORT\n\
CONTROLLER IN LOCATION %3$s IN ADVANCED MODE"
$
$ %1$x is the port number being tested.
$ %2$s is the remote async node name being tested.
$ %3$s is the location of the planar slot containing the 128-port adapter.
$ The location code of the device is described in the general notes.
$ Leave line spacing as shown.  See general notes on length of title line.

DM_15C   "\
TESTING PORT %1$x ON REMOTE ASYNC NODE %2$s ATTACHED TO THE 128-PORT\n\
CONTROLLER IN LOCATION %3$s IN ADVANCED MODE\n\n\
Please stand by."
$
$ %1$x is the port number being tested.
$ %2$s is the remote async node name being tested.
$ %3$s is the location of the planar slot containing the 128-port adapter.
$ The location code of the device is described in the general notes.
$ Leave line spacing as shown.  See general notes on length of title line.

DM_15D   "\
TESTING PORT %1$x ON REMOTE ASYNC NODE %2$s ATTACHED TO THE 128-PORT\n\
CONTROLLER IN LOCATION %3$s IN ADVANCED MODE\n\n\
Testing multiple times.\n\n\
%4$6d\tpasses completed.\n\
%5$6d\terrors logged.\n\n\
To stop the test, use 'Cancel.'"
$
$ %1$x is the port number being tested.
$ %2$s is the remote async node name being tested.
$ %3$s is the location of the planar slot containing the 128-port adapter.
$ The location code of the device is described in the general notes.
$ %4$6d\t sets the number of diagnostic test passes completed.
$ %5$6d\t sets the number of errors logged during the tests.
$ Leave line spacing as shown.  See general notes on length of title line.

DM_14A   "\
TESTING ASYNC CONTROLLER %1$s IN LOCATION %2$s\n\n\
Please stand by."
$
$ %1$s is the name of the 128-port async controller being tested.
$ %2$s is the scsi bus location of the 128-port async controller adapter.
$ Leave line spacing as shown.  See general notes on length of title line.

DM_14B   "\
TESTING ASYNC CONTROLLER %1$s IN LOCATION %2$s"
$
$ %1$s is the name of the 128-port async controller being tested.
$ %2$s is the scsi bus location of the 128-port async controller adapter.
$ Leave line spacing as shown.  See general notes on length of title line.

DM_14C   "\
TESTING ASYNC CONTROLLER %1$s IN LOCATION %2$s IN ADVANCED MODE\n\n\
Please stand by."
$
$ %1$s is the name of the 128-port async controller being tested.
$ %2$s is the scsi bus location of the 128-port async controller adapter.
$ Leave line spacing as shown.  See general notes on length of title line.

DM_14D   "\
TESTING ASYNC CONTROLLER %1$s IN LOCATION %2$s IN ADVANCED MODE"
$
$ %1$s is the name of the 128-port async controller being tested.
$ %2$s is the scsi bus location of the 128-port async controller adapter.
$ Leave line spacing as shown.  See general notes on length of title line.

DM_17   "\
Was the last test performed with the terminator (Part Number %1$s) placed\n\
on the adapter ?"
$
$ $ rscmenu2 - query user if terminator is on adapter
$ %1$s is the part number of the wrap plug

DM_23    "\
                           CONTROLLER LINE FAILURE.\n\n\
To isolate the failure, begin by plugging a terminator (Part Number %1$s)\n\
into the \'OUT\/T\' connector on the LAST Remote Async Node in the controller\n\
line.  Run the test again.  If a failure occurs, unplug the controller cable\n\
from the \'IN\' connector on the last Remote Async Node, and plug the\n\
terminator into the controller cable.  Run the test again.  If a failure\n\
occurs, proceed toward the 128-Port Async Controller and continue to repeat\n\
this testing procedure until the test passes.\n\n\
Press Enter to continue controller line testing."
$
$ The remote async node is a unit that provides the physical space
$ to plug in all the devices running off of the adapter.
$ See set notes for information on how to expand lines if necessary.
$ Leave line spacing as shown.  See general notes on user ACTIONS.

DM_24   "\
                 THE CONTROLLER LINE TEST SUCCEEDED.\n\n\
The previous controller cable section or Remote Async Node removed from the\n\
controller line was probably defective.  Service is required.\n\n\n\
NOTE:  The Remote Async Node and location codes indicated in the \n\
       following screen apply to the initial component under test.\n\
       It does not necessarily indicate the failing component."
$
$ The remote async node is a unit that provides the physical space
$ to plug in all the devices running off of the adapter.
$ See set notes for information on how to expand lines if necessary.
$ Leave line spacing as shown.  See general notes on user ACTIONS.

DM_87_OLD    ""
$ See below for updated DM_87 message.

DM_14E   "\
TESTING LINE %1$d ON ASYNC CONTROLLER %2$s IN LOCATION %3$s\n\
IN ADVANCED MODE"
$
$ %1$s is the sync line being tested.
$ %2$s is the name of the 128-port async controller being tested.
$ %3$s is the scsi bus location of the 128-port async controller adapter.
$ Leave line spacing as shown.  See general notes on length of title line.

DM_14F   "\
TESTING LINE %1$d ON ASYNC CONTROLLER %2$s IN LOCATION %3$s\n\
IN ADVANCED MODE\n\n\
Please stand by."
$
$ %1$s is the sync line being tested.
$ %2$s is the name of the 128-port async controller being tested.
$ %3$s is the scsi bus location of the 128-port async controller adapter.
$ Leave line spacing as shown.  See general notes on length of title line.

DM_64   "\
UNPLUG.........the multiport cable assembly, if attached, from the 8/16-\n\
               port rack extension cable.\n\
PLUG...........the wrap plug (Part Number %1$s) into the 8/16-port rack\n\
               extension cable connector.\n\n\
When finished, press Enter."
$
$ $ Model 930 8 and 16 port rack extension cable wrap test unit 40.
$ %1$s is the part number of the wrap plug used for test. Do not translate.
$ See set notes for information on how to expand lines if necessary.
$ Leave line spacing as shown.  See general notes on user ACTIONS.

DM_65   "\
UNPLUG.........the wrap plug from the 8/16-port rack extension cable\n\
               connector.\n\
PLUG...........the multiport interface cable assembly, if it was removed,\n\
               into the 8/16-port rack extension cable connector.\n\n\
When finished, press Enter."
$
$ $ Model 930 8 and 16 port rack extension cable wrap test unit 40.
$ See set notes for information on how to expand lines if necessary.
$ Leave line spacing as shown.  See general notes on user ACTIONS.

DM_70   "\
UNPLUG.........the 8/16-port rack extension cable, if attached, from\n\
               the 8-port adapter.\n\
PLUG...........the wrap plug (Part Number %1$s) into the 8-port\n\
               adapter.\n\n\
When finished, press Enter."
$
$ $ Model 930 8 port adapter test unit 30.
$ %1$s is the part number of the wrap plug used for test. Do not translate.
$ See set notes for information on how to expand lines if necessary.
$ Leave line spacing as shown.  See general notes on user ACTIONS.

DM_71   "\
UNPLUG.........the wrap plug from the 8-port adapter.\n\
PLUG...........the 8/16-port rack extension cable, if it was removed,\n\
               into the 8-port adapter.\n\n\
When finished, press Enter."
$
$ $ Model 930 8 port adapter test unit 30.
$ See set notes for information on how to expand lines if necessary.
$ Leave line spacing as shown.  See general notes on user ACTIONS.

DM_74   "\
UNPLUG.........the 8/16-port rack extension cable, if attached, from\n\
               the 16-port adapter.\n\
PLUG...........the wrap plug (Part Number %1$s) into the 16-port\n\
               adapter.\n\n\
When finished, press Enter."
$
$ $ Model 930 16 port adapter test unit 30.
$ %1$s is the part number of the wrap plug used for test. Do not translate.
$ See set notes for information on how to expand lines if necessary.
$ Leave line spacing as shown.  See general notes on user ACTIONS.

DM_75   "\
UNPLUG.........the wrap plug from the 16-port adapter.\n\
PLUG...........the 8/16-port rack extension cable, if it was removed,\n\
               into the 16-port adapter.\n\n\
When finished, press Enter."
$
$ $ Model 930 16 port adapter test unit 30.
$ See set notes for information on how to expand lines if necessary.
$ Leave line spacing as shown.  See general notes on user ACTIONS.

DM_87    "\
                        LINE %1$d SYNC WRAP FAILURE\n\n\
The 128-port adapter is reporting an error with the sync line listed above.\n\
The most common cause for this failure is an unterminated sync line.\n\n\
Please check the following, then answer the question below:\n\n\
\t\- Are all of the sync lines properly connected?\n\
\t\- Does the last Remote Async Node (RAN) in the chain have a terminator\n\
\t\     (Part Number %2$s) installed in the OUT\/T port of the RAN, or\n\
\t\- Is there a terminator installed on the sync line of the adapter?\n\n\n\
Were all the cables connected, and did the adapter or last RAN in the chain\n\
have a teminator installed on the sync line listed above?"
$
$ 128 port sync line test.
$ %1$d is the sync line that has been found to be unterminated
$ %2$s is the part number fo the 128-port terminator
$ Leave line spacing as shown.  See general notes on user ACTIONS.

DM_88   "\
Modem Cable and Interposer"
$
$ Possible response to a question

DM_89   "\
Printer/Terminal Cable"
$
$ Possible response to a question

DM_90   "\
Please select which type of cabling is being used on this port to connect to\n\
the selected printer or terminal device.\n\n\
Move cursor to selection, then press Enter."
$
$ Printer/Terminal or Modem/Interposer cabling question
$ Leave line spacing as shown.  See general notes on user ACTIONS.

DM_91   "\
PLUG...........the terminator (Part Number %1$s) into the last Remote\n\
               Async Node connected to sync line port %2$d.\n\n\
When finished, press Enter."
$
$ $ sync line test in loop mode
$ %1$s is the part number of the 128-port terminator. Do not translate.
$ %2$s is the sync line that has been found to be unterminated
$ See set notes for information on how to expand lines if necessary.
$ Leave line spacing as shown.  See general notes on user ACTIONS.




$ $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
$set ASYNC_CABLE
$
$ The messages in this set have a 16-character length limit.
$

AC1     "Converter Cable";
$ This is a small cable that fits between a cable and either the system unit,
$ a RemoteAsyncNode, or a Concentrator.  One end of the cable is typically a
$ DB-25 D-shell and the other end is usually a 10-pin MODU plug or a RJ-45
$ clip.

AC2     "Cable Assembly";
$ Cable Assembly is used to denote a 8-port or 16-port fanout box or other
$ type of specialized cable.

AC3     "Interposer";
$ An interposer is a small connector that fits between a cable and a device,
$ converting a modem cable to a cable that will interface with some other
$ device.

AC4     "Concentrator";
$ This is a small box with 16 RJ-45 outlets that is used in conjunction with
$ a 64-port adapter to connect to the external devices.

AC5     "\0";
$ Do not translate.  This message is used for those cases where no device is
$ to be called out.

AC6     "Cable";
$ This is the RS-232 or RS-422 modem cable.

AC7     "RemoteAsyncNode";
$ A RemoteAsyncNode is a small box with 16 RJ-45 outlets on its back that is
$ used in conjunction with a 128-port adapter to connect to external devices.

AC8     "Adapter";
$ This is the physical card that plugs into the adapter slots in a system unit.

AC9     "Controller Line";
$ This is a cable that runs from an adapter to a Concentrator or
$ RemoteAsyncNode.


$ $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
$set ASYNC_MENUGOAL
$

MG1 ""
$ This message is no longer used.

MG2 "\
MISSING 128-PORT SYNC LINE TERMINATOR\n\
The operation of the 128-port adapter and RANs require that the sync lines\n\
be terminated by a terminator (Part Number %1$s).  Please re-test the\n\
128-port adapter once a terminator has been obtained."
$
$ $ 128 port sync line test
$ This menugoal is displayed if the user does not have a terminator available.
