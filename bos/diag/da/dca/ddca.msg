$ @(#)31	1.6.1.3  src/bos/diag/da/dca/ddca.msg, dadca, bos411, 9428A410j 2/4/93 13:37:43
$
$   COMPONENT_NAME: DADCA
$
$   FUNCTIONS: 
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
$ This is the message file for the 3270 Connection.  This adapter card
$ enables the machine to communicate, through a control unit, with an IBM
$ mainframe host.  A coaxial cable attaches the adapter to the control unit.
$
$ The LOCATION of a resource is in the form of xx-xx-xx where x is an
$   alpha-numeric character.  The location is not translatable.  It is
$   an alpha-numeric descriptor of where the resource can be found.
$
$ END OF GENERAL NOTES


$set 1
$quote "
$  

TITLE "TESTING THE 3270 CONNECTION ADAPTER IN LOCATION %1$s"
$
$ This message is displayed as the title to most menus.
$ %1$s is a location code and will have a format described in the general notes.
$ Leave line spacing as shown.  See general notes on length of title.

TITLE_ADVANCED "TESTING THE 3270 CONNECTION ADAPTER IN LOCATION %1$s\n\
IN ADVANCED MODE"
$
$ This message is displayed as the title to most menus in advanced mode.
$ %1$s is a location code and will have a format described in the general notes.
$ Leave line spacing as shown.  See general notes on length of title.

STANDBY "TESTING THE 3270 CONNECTION ADAPTER IN LOCATION %1$s\n\n\
Please stand by."
$
$ This message is displayed as soon as testing begins on the adapter.
$ %1$s is a location code and will have a format described in the general notes.
$ Leave line spacing as shown.  See general notes on length of title.

STANDBY_ADVANCED "TESTING THE 3270 CONNECTION ADAPTER IN LOCATION %1$s\n\
IN ADVANCED MODE\n\n\
Please stand by."
$
$ This message is displayed as soon as testing
$ in advanced mode begins on the adapter.
$ %1$s is a location code and will have a format described in the general notes.
$ Leave line spacing as shown.  See general notes on length of title.

TESTING_INLM "TESTING THE 3270 CONNECTION ADAPTER IN LOCATION %1$s\n\
IN ADVANCED MODE\n\n\
Testing multiple times.\n\n\
%2$6d\tpasses completed.\n\
%3$6d\terrors logged.\n\n\
To stop the test, use 'Cancel.'"
$
$ This message is displayed before testing begins each time
$ the DA is executed in loop mode.  
$ %1$s is a location code and will have a format described in the general notes.
$ %2$6d\t sets the number of diagnostic test passes completed.
$ %3$6d\t sets the number of errors logged during the tests.
$ Leave line spacing as shown.  See general notes on length of title.

SHOULD_BE_COMM "\
The tests indicate that the 3270 Connection Adapter cannot\n\
communicate with the control unit.\n\n\
Should the adapter be communicating with the control unit at this time?\n\n\
Move cursor to selection, then press Enter.\n"
$
$ This message appears if the Connection Test fails for the adapter.
$ An example of an adapter that should not be communicating with the 
$ control unit is one that has not yet been connected or one that has
$ been intentionally disconnected.  If the user indicates that there
$ should be no communication, testing will be discontinued for the adapter.
$ Leave line spacing as shown.

IS_CABLE_CONNECTED "\
Is there a cable connecting the adapter with the host system?\n\
\nMove cursor to selection, then press Enter.\n"
$
$ This message appears if the Connection test has failed and the 
$ user indicated that the adapter should be communicating with the control
$ unit.
$ Leave line spacing as shown.
 
IS_CABLE_DAMAGED "\
CHECK..........the cable connecting the adapter to the 3270 control unit.\n\n\
Is the cable damaged?\n\
\nMove cursor to selection, then press Enter.\n"
$
$ This message appears if the adapter has failed the Connection
$ test and the user has indicated that there is a cable connecting the 
$ adapter to the control unit.
$ Leave line spacing as shown.  See general notes on expanding ACTION lines.

IS_CU_PROBLEM "\
CALL...........the system operator and ask if there is a \n\
               control unit or host system problem.\n\
\n\
Is there a control unit or host system problem?\n"
$
$ This message appears if the adapter has failed the Connection test and 
$ the user has indicated there is no trouble with the cable.
$ Leave line spacing as shown.  See general notes on expanding ACTION lines.

IS_CABLE_AVAILABLE "\
Is there a good cable available?\n\
\nMove cursor to selection, then press Enter.\n"
$
$ This message appears if the adapter has failed the Connection 
$ test and the user has indicated that the cable is either missing or
$ damaged.
$ Leave line spacing as shown.

YES_OPTION "YES"
$ This option is displayed when a YES answer is possible.

NO_OPTION "NO"
$ This option is displayed when a NO answer is possible.

PLEASE_CONNECT "TESTING THE 3270 CONNECTION ADAPTER IN LOCATION %1$s\n\n\
CONNECT........the cable from the 3270 Connection Adapter\n\
               to the 3270 Control Unit.\n\n\
When finished, press Enter.\n"
$
$ This message appears if the adapter has failed the Connection test
$ and the user has indicated that there is a cable available to replace
$ the one that is missing or damaged. 
$ %1$s a location code and will have a format described in the general notes.
$ Leave line spacing as shown.  See general notes on expanding ACTION lines.

PLEASE_CONNECT_ADVANCED "\
TESTING THE 3270 CONNECTION ADAPTER IN LOCATION %1$s\n\
IN ADVANCED MODE\n\n\
CONNECT........the cable from the 3270 Connection Adapter\n\
               to the 3270 Control Unit.\n\n\
When finished, press Enter. \n"
$
$ This message is the advanced mode equivalent of PLEASE_CONNECT.
$ %1$s a location code and will have a format described in the general notes.
$ Leave line spacing as shown.  See general notes on expanding ACTION lines.

CONNECT_CABLE "85400A \
CONNECT CABLE TO 3270 CONNECTION ADAPTER IN LOCATION %1$s\n\n\
The 3270 Connection Adapter failed its tests because the cable\n\
connecting it to the control unit was either missing or damaged.\n\
Connecting an undamaged cable may correct the problem."
$
$ This message appears if the adapter has failed the Connection test, the 
$ user has indicated that the cable is either missing or damaged, and there is
$ no cable available for replacement.
$ DO NOT TRANSLATE the number at the beginning of this message.  Leave it
$ exactly as shown - 85400A.
$ %1$s a location code and will have a format described in the general notes.
$ Leave line spacing as shown.  See general notes on length of title.

DCA_DRIVER_FILE_BAD "85400B \
The 3270 device driver is either missing or not accessible.\n\n\
This device driver is necessary in order to use the 3270 in normal\n\
system operation or to run Diagnostics."
$
$ This message is display when the device driver file is missing
$ or not accessible
$
