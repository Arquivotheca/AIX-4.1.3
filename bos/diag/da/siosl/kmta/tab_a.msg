$ @(#)83	1.4.1.8  src/bos/diag/da/siosl/kmta/tab_a.msg, datablet, bos411, 9428A410j 3/17/94 19:46:26
$
$   COMPONENT_NAME: DATABLET
$
$   FUNCTIONS: paragraph
$		
$
$   ORIGINS: 27
$
$
$   (C) COPYRIGHT International Business Machines Corp. 1988,1994
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
$ END OF GENERAL NOTES


$quote "
$set TBL_GENERIC

TBL_TITLE "TESTING TABLET CONNECTOR"
$
$ See general notes on length of title.

TBL_TITLE_A "TESTING TABLET CONNECTOR IN ADVANCED MODE"
$
$ Display diagnostics menu title.
$ See general notes on length of title.

TBL_STND_BY "Please stand by."
$ Infom the user that diagnostics is in process.

TBL_PRS_ENTER "When finished, press Enter."
$ The user must press enter key after testing is done.

DEVICE_CANNOT_UNCONFIGURE2 "821008 \
THE DEVICE CANNOT BE UNCONFIGURED\n\
Diagnostics cannot unconfigure the device attached to the adapter\n\
under test.\n\n\
The resource is\n\
- %1$s          %2$s            Tablet"
$

DEVICE_CANNOT_CONFIGURE2 "821009 \
THE DEVICE CANNOT BE CONFIGURED\n\
Diagnostics cannot configure the device attached to the adapter\n\
under test.\n\n\
The resource is\n\
- %1$s          %2$s            Tablet"
$

KBT_TITLE "TESTING KEYBOARD/TABLET ADAPTER"
$
$ See general notes on length of title.

KBT_TITLE_A "TESTING KEYBOARD/TABLET ADAPTER IN ADVANCED MODE"
$
$ Display diagnostics menu title.
$ See general notes on length of title.

KTA_DEVICE_CANNOT_OPEN "821012 \
ADDITIONAL RESOURCES ARE REQUIRED FOR TESTING\n\n\
A problem has been detected that can not be isolated without\n\
freeing additional resources.\n\n\
The resource needed is\n%s\n\
To test this resource, you can:\n\
  Free this resource and continue testing.\n\
  Shut down the system and run in maintenance mode.\n\
  Run Diagnostics from the Diagnostic Standalone package."
$
$ %s is a substitution for the name of the resource.
$ In order to test additional resources, they must be freed, or made available
$ to the system.  All programs using the resource must be stopped and the
$ resource can then be used by diagnostics.

DEVICE_CANNOT_UNCONFIGURE1 "824008 \
THE DEVICE CANNOT BE UNCONFIGURED\n\
Diagnostics cannot unconfigure the device attached to the adapter\n\
under test.\n\n\
The resource is\n\
- %1$s          %2$s            Tablet"
$

DEVICE_CANNOT_CONFIGURE1 "824009 \
THE DEVICE CANNOT BE CONFIGURED\n\
Diagnostics cannot configure the device attached to the adapter\n\
under test.\n\n\
The resource is\n\
- %1$s          %2$s            Tablet"
$

$set TBL_WRP_NUM

TBL_WRP_TITLE "TESTING TABLET CONNECTOR\n\n\
The following test requires a wrap plug, Part Number 22F9689.\n\n\
Do you have this wrap plug?"
$
$ Ask the user for required wrap plug.
$ Leave line spacing as shown.  See general notes on length of title.

TBL_WRP_TITLE_A "TESTING TABLET CONNECTOR IN ADVANCED MODE\n\n\
The following test requires a wrap plug, Part Number 22F9689.\n\n\
Do you have this wrap plug?"
$ Ask the user for required wrap plug.
$ Leave line spacing as shown.  See general notes on length of title.

TBL_WRP_YES "YES"
$ This option is shown when a YES answer is possible. 

TBL_WRP_NO "NO"
$ This option is shown when a NO answer is possible. 

TBL_WRP_ACTION "Move cursor to selection, then press Enter."
$ Users action

KBT_WRP_TITLE "TESTING KEYBOARD/TABLET ADAPTER\n\n\
The following test requires a wrap plug, Part Number 22F9689.\n\n\
Do you have this wrap plug?"
$
$ Ask the user for required wrap plug.
$ Leave line spacing as shown.  See general notes on length of title.

KBT_WRP_TITLE_A "TESTING KEYBOARD/TABLET ADAPTER IN ADVANCED MODE\n\n\
The following test requires a wrap plug, Part Number 22F9689.\n\n\
Do you have this wrap plug?"
$ Ask the user for required wrap plug.
$ Leave line spacing as shown.  See general notes on length of title.


$set TBL_RMV_ATTACHEMENT

TBL_UNPLUG_CBL "\
UNPLUG.........the cable, if attached, from the tablet connector."
$
$ Disconnect the cable from the tablet port and then to insert a wrap plug.
$ See general notes on expanding ACTION lines if necessary.

TBL_PLUG_WRP "\
PLUG...........the wrap plug (Part Number 22F9689) into the tablet\n\
               connector."
$
$ Insert wrap plug into the tablet connector.
$ See general notes on expanding ACTION lines if necessary.

$set TBL_RECONNECT

TBL_UNPLUG_WRP "\
UNPLUG.........the wrap plug from the tablet connector."
$
$ Remove the wrap plug the tablet connector
$ See general notes on expanding ACTION lines if necessary.

TBL_PLUG_CBL "\
PLUG...........the tablet cable, if removed, into the tablet connector."
$
$ Reconnect the cable if removed from the tablet connector
$ See general notes on expanding ACTION lines if necessary.

TBL_CON_PRS_ENT "When finished, press Enter."

$set TBL_INLOOP

TBL_INLOOP_TITLE "TESTING TABLET CONNECTOR IN ADVANCED MODE\n\n\
Testing multiple times.\n\n\
%1$6d\tpasses completed.\n\
%2$6d\terrors logged.\n\n\
To stop the test, use 'Cancel.'"
$
$ %1$6d\t sets the number of diagnostic test passes completed.
$ %2$6d\t sets the number of errors logged during the tests.
$ Display menu title, number of times the loop passing through and number of
$ errors found while it's running.
$ Leave line spacing as shown.  See general notes on length of title.

KBT_INLOOP_TITLE "TESTING KEYBOARD/TABLET ADAPTER IN ADVANCED MODE\n\n\
Testing multiple times.\n\n\
%1$6d\tpasses completed.\n\
%2$6d\terrors logged.\n\n\
To stop the test, use 'Cancel.'"
$
$ %1$6d\t sets the number of diagnostic test passes completed.
$ %2$6d\t sets the number of errors logged during the tests.
$ Display menu title, number of times the loop passing through and number of
$ errors found while it's running.
$ Leave line spacing as shown.  See general notes on length of title.

$set TBL_WRAPWARN

TBL_WARN_WRP "\
Please remove the tablet wrap plug, if attached."
$
$ Remove the wrap plug from the tablet connector
$ See general notes on expanding ACTION lines if necessary.
