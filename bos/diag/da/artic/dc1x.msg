$ @(#)33  1.1  src/bos/diag/da/artic/dc1x.msg, daartic, bos41J, 9511A_all 3/3/95 11:02:16
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
$ GENERAL NOTES
$
$ Do not translate %c, %d, %s, %x, %07X, or \t in any messages.  They
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
$ END OF GENERAL NOTES


$set REGULAR
$quote "
$
RTITLE  "TESTING X.25 CO-PROCESSOR ADAPTER\n\n\
Please stand by."
$
$ This line is displayed during testing while in the non-advanced mode.
$ Leave line spacing as shown.  See general notes on length of title line.


$set ADVANCED
$quote "
$
ATITLE  "TESTING X.25 CO-PROCESSOR ADAPTER IN ADVANCED MODE\n\n\
Please stand by."
$
$ This line is displayed during testing while in the advanced mode.
$ Leave line spacing as shown.  See general notes on length of title line.


$set ALOOP
$quote "
$
ALTITLE "TESTING X.25 CO-PROCESSOR ADAPTER IN ADVANCED MODE\n\n\
Testing multiple times.\n\n\
%1$6d\tpasses completed.\n\
%2$6d\terrors logged.\n\n\
To stop the test, use 'Cancel.'"
$
$ %1$6d\t sets the number of diagnostic test passes completed.
$ %2$6d\t sets the number of errors logged during the tests.
$ This line is displayed during advanced mode testing in loop mode.
$ Leave line spacing as shown.  See general notes on length of title line.


$set LOOP
$quote "
$
LTITLE  "TESTING X.25 CO-PROCESSOR ADAPTER\n\n\
Testing multiple times.\n\n\
%1$6d\tpasses completed.\n\
%2$6d\terrors logged.\n\n\
To stop the test, use 'Cancel.'"
$
$ %1$6d\t sets the number of diagnostic test passes completed.
$ %2$6d\t sets the number of errors logged during the tests.
$ This line is displayed during non-advanced mode testing in loop mode.
$ Leave line spacing as shown.  See general notes on length of title line.


$set Q_PLUG_37_PIN
$quote "
$
Q_PLUG_37_PIN_TITLE "TESTING X.25 CO-PROCESSOR ADAPTER IN ADVANCED MODE\n\n\
The following test requires a 37 pin wrap plug. (Part Number 07F3132)\n\n\
Do you have this wrap plug ?\n\n\
        NOTE:  If this wrap plug is not available the adapter\n\
               will not be tested completely."
$
$ Check for appropriate part number in translating country.
$ Leave line spacing as shown.  See general notes on length of title line.

Q_PLUG_37_PIN_YES "YES"
$ This option is shown when a YES answer is possible.

Q_PLUG_37_PIN_NO "NO"
$ This option is shown when a NO answer is possible.

Q_PLUG_37_PIN_ACTION "Move cursor to selection, then press Enter."
$ This message is shown when a multiple selection list is presented.


$set PLUG_37_PIN
$quote "
$
PLUG_37_PIN_TITLE "TESTING X.25 CO-PROCESSOR ADAPTER IN ADVANCED MODE\n\n\
REMOVE.........the cable, if attached, from the adapter in\n\
               location %1$s.\n\
PLUG...........the wrap plug (Part Number 07F3132) into\n\
               the adapter.\n\n\
When finished, press Enter."
$
$ %1$s is the location of the adapter as described in the general notes.
$ See general notes on how to expand ACTION lines if necessary.
$ Check for appropriate part number in translating country.
$ Leave line spacing as shown.  See general notes on length of title line.


$set UNPLUG_37_PIN
$quote "
$
UNPLUG_37_PIN_TITLE "TESTING X.25 CO-PROCESSOR ADAPTER IN ADVANCED MODE\n\n\
UNPLUG.........the wrap plug from the adapter.\n\
PLUG...........the interface cable, if it was removed,\n\
               into the adapter.\n\n\
When finished, press Enter."
$
$ This line instructs the user to restore things to the original state
$ after testing is done.
$ See general notes on how to expand ACTION lines if necessary.
$ Leave line spacing as shown.  See general notes on length of title line.


$set NO_PLUG
$quote "
$
NO_PLUG_TITLE "TESTING X.25 CO-PROCESSOR ADAPTER IN ADVANCED MODE\n\n\
Since the wrap plug is not available,\n\
this particular test will not be performed."
$
$ This line informs the user that a particular test cannot be performed
$ due to the lack of a wrap plug. This screen will stay on for a few seconds
$ and then go away by itself.
$ Leave line spacing as shown.  See general notes on length of title line.


$set Q_WHICH_INT
$quote "
$
Q_WHICH_INT_TITLE "TESTING X.25 CO-PROCESSOR ADAPTER IN ADVANCED MODE\n\n\
Identify the interface you want to test in location %1$s."
$
$ %1$s is the location of the adapter as described in the general notes.
$ Leave line spacing as shown.  See general notes on length of title line.


Q_WHICH_INT_X21 "X.21 Interface"
Q_WHICH_INT_V24 "V.24 Interface"
Q_WHICH_INT_V35 "V.35 Interface"
Q_WHICH_INT_NO_IDEA "Not able to identify the interface."

Q_WHICH_INT_ACTION "Move cursor to selection, then press Enter."
$ This message is shown when a multiple selection list is presented.
$ It inquires the user to identify the interface of the
$ communication device under test.


$set NO_INTERFACE
$quote "
$
NO_INTERFACE_TITLE "TESTING X.25 CO-PROCESSOR ADAPTER IN ADVANCED MODE\n\n\
Since the interface cannot be identified,\n\
this particular test will not be performed."
$
$ Leave line spacing as shown.  See general notes on length of title line.


$set Q_INTX21_OR_NOT
$quote "
$
Q_INTX21_OR_NOT_TITLE "TESTING X.25 CO-PROCESSOR ADAPTER IN ADVANCED MODE\n\n\
The X.21 interface test requires a wrap plug, Part Number 07F3153\n\
to be plugged into the device end of the interface cable.\n\n\
Do you have this wrap plug ?"
$
$ Check for appropriate part number in translating country.
$ Leave line spacing as shown.  See general notes on length of title line.


Q_INTX21_OR_NOT_YES "YES"
$ This option is shown when a YES answer is possible.

Q_INTX21_OR_NOT_NO "NO"
$ This option is shown when a NO answer is possible.

Q_INTX21_OR_NOT_ACTION "Move cursor to selection, then press Enter."
$ This message is shown when a multiple selection list is presented.


$set Q_INTV24_OR_NOT
$quote "
$
Q_INTV24_OR_NOT_TITLE "TESTING X.25 CO-PROCESSOR ADAPTER IN ADVANCED MODE\n\n\
The V.24 interface test requires a wrap plug, Part Number 07F3163\n\
to be plugged into the device end of the interface cable.\n\n\
Do you have this wrap plug ?"
$
$ Check for appropriate part number in translating country.
$ Leave line spacing as shown.  See general notes on length of title line.

Q_INTV24_OR_NOT_YES "YES"
$ This option is shown when a YES answer is possible.

Q_INTV24_OR_NOT_NO "NO"
$ This option is shown when a NO answer is possible.

Q_INTV24_OR_NOT_ACTION "Move cursor to selection, then press Enter."
$ This message is shown when a multiple selection list is presented.


$set Q_INTV35_OR_NOT
$quote "
$
Q_INTV35_OR_NOT_TITLE "TESTING X.25 CO-PROCESSOR ADAPTER IN ADVANCED MODE\n\n\
The V.35 interface test requires a wrap plug, Part Number 07F3173\n\
to be plugged into the device end of the interface cable.\n\n\
Do you have this wrap plug ?"
$
$ Check for appropriate part number in translating country.
$ Leave line spacing as shown.  See general notes on length of title line.

Q_INTV35_OR_NOT_YES "YES"
$ This option is shown when a YES answer is possible.

Q_INTV35_OR_NOT_NO "NO"
$ This option is shown when a NO answer is possible.

Q_INTV35_OR_NOT_ACTION "Move cursor to selection, then press Enter."
$ This message is shown when a multiple selection list is presented.


$set PUT_INTX21_PLUG
$quote "
$
PUT_INTX21_PLUG_TITLE "TESTING X.25 CO-PROCESSOR ADAPTER IN ADVANCED MODE\n\n\
ENSURE.........the cable is connected to the adapter.\n\
DISCONNECT.....the cable from the communication device.\n\
PLUG...........the wrap plug (Part Number 07F3153)\n\
               into the end of the interface cable.\n\n\
When finished, press Enter."
$
$ See general notes on how to expand ACTION lines if necessary.
$ Check for appropriate part number in translating country.
$ Leave line spacing as shown.  See general notes on length of title line.


$set PUT_INTV24_PLUG
$quote "
$
PUT_INTV24_PLUG_TITLE "TESTING X.25 CO-PROCESSOR ADAPTER IN ADVANCED MODE\n\n\
ENSURE.........the cable is connected to the adapter.\n\
DISCONNECT.....the cable from the communication device.\n\
PLUG...........the wrap plug (Part Number 07F3163)\n\
               into the end of the interface cable.\n\n\
When finished, press Enter."
$
$ See general notes on how to expand ACTION lines if necessary.
$ Check for appropriate part number in translating country.
$ Leave line spacing as shown.  See general notes on length of title line.


$set PUT_INTV35_PLUG
$quote "
$
PUT_INTV35_PLUG_TITLE "TESTING X.25 CO-PROCESSOR ADAPTER IN ADVANCED MODE\n\n\
ENSURE.........the cable is connected to the adapter.\n\
DISCONNECT.....the cable from the communication device.\n\
PLUG...........the wrap plug (Part Number 07F3173)\n\
               into the end of the interface cable.\n\n\
When finished, press Enter."
$
$ See general notes on how to expand ACTION lines if necessary.
$ Check for appropriate part number in translating country.
$ Leave line spacing as shown.  See general notes on length of title line.


$set RESTORE
$quote "
$
RESTORE_TITLE "TESTING X.25 CO-PROCESSOR ADAPTER IN ADVANCED MODE\n\n\
UNPLUG.........the wrap plug at the end of the cable.\n\
RECONNECT......the cable back to the communication device.\n\n\
When finished, press Enter."
$
$ This line instructs the user to restore things to
$ the original state after testing is done.
$ See general notes on how to expand ACTION lines if necessary.
$ Leave line spacing as shown.  See general notes on length of title line.

$set NO_RCM
$quote "
$
NO_RCM_TITLE "849000 \
X.25 INTERFACE CO-PROCESSOR OPERATIONAL MICROCODE IS MISSING\n\n\
The X.25 Interface Co-Processor operational microcode is either\n\
missing or not accessible.\n\n\
This microcode is necessary in order to use the X.25 adapter card\n\
in normal system operations."
$
$ Leave line spacing as shown.  See general notes on length of title.
$ Do not translate the number 849000 at the beginning of the message.
$ Leave it exactly as shown.
