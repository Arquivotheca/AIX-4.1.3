$ @(#)09	1.6.1.3  src/bos/diag/da/sol/optics.msg, dasol, bos411, 9428A410j 2/10/93 16:00:18
$
$   COMPONENT_NAME: DASOL
$
$   FUNCTIONS: 
$		
$
$   ORIGINS: 27
$
$
$   (C) COPYRIGHT International Business Machines Corp. 1991,1993
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
$ END OF GENERAL NOTES

 
$set 1
$quote "


OPTICS_WRAP_PLUG_TITLE "\
The following test requires a wrap plug, Part Number 56F0203.\n\n\
Do you have this wrap plug?\n\n\
Move cursor to selection, then press Enter."
$
$ These line ask the user if he has a wrap plug part number 56F0203.
$ Display diagnostics menu title and ask the user for a wrap plug.
$ Leave line spacing as shown.  See general notes on length of title line.

OPTICS_YES_OPTION "YES"
$ This option is shown when the user is asked to have a YES NO option.
$ Leave line spacing as shown.  See general notes on length of title line.

OPTICS_NO_OPTION "NO"
$ This option is shown when the user is asked to have a YES NO option.
$ Leave line spacing as shown.  See general notes on length of title line.

OPTICS_OPTION "Move cursor to selection, then press Enter "
$ Leave line spacing as shown.  See general notes on length of title line.

OPTICS_FINISHED "\
When finished, press Enter."
$
$ Leave line spacing as shown.  See general notes on length of title line.

OPTICS_PLUG_WRAP "\
UNPLUG.........the fiber optics cable(s), if attached, from Port %1$s\n\
               at location %2$s.\n\n\
PLUG...........the wrap plug (Part Number 56F0203) into Port %1$s\n\
               at location %2$s."
$ 
$
$ This message is displayed while testing is performed in 
$ advanced mode.
$ %1$s is the port that is tested.
$ %2$s is the location code of the device as described in the general notes
$ Leave line spacing as shown.  See general notes on length of title line.

OPTICS_UNPLUG_WRAP "\
UNPLUG.........the wrap plug from Port %1$s at location %2$s.\n\n\
PLUG...........the fiber optics cable(s), if removed, into Port %1$s\n\
               at location %2$s."
$
$ This message is displayed while testing is performed in 
$ advanced mode.
$ %1$s is the port that is tested.
$ %2$s is the location code of the device as described in the general notes
$ Leave line spacing as shown.  See general notes on length of title line.

OPTICS_WARNING_MSG "\
* * * WARNING! * * *\n\n\
Port %1$s at location %2$s is currently being used by another process.\n\n\
Testing the resource will terminate communication with this resource\n\
and possibly cause data to be lost.\n\n\
Do you want to continue this test?\n\n\
Move cursor to selection, then press Enter."
$
$ This message is displayed while testing is performed in 
$ advanced mode.
$ %1$s is the port that is tested.
$ %2$s is the location code of the device as described in the general notes
$ Leave line spacing as shown.  See general notes on length of title line.

OPTICS_CONFIRM_MSG "\
Are you sure that you want to continue testing ?\n\
Move cursor to selection, then press Enter."
$
$ Leave line spacing as shown.  See general notes on length of title line.

OPTICS_DD_FILE_BAD "861009 \
SERIAL OPTIC CHANNEL CONVERTER DEVICE DRIVER FILE MISSING\n\
The Serial Optic Channel Converter device driver file is either missing or\n\
not accessible.\n\n\
This device driver is necessary in order to use the Serial Optic Channel\n\
Converter in normal system operation or to run Diagnostics."

$
$ Note to the translator.
$ the feedthrough connector has the shape of the letter H.
$ There are two female connectors on the top and bottom  which can be 
$ inserted into the male connectors of the optics card.
$ Leave line spacing as shown.  See general notes on length of title line.

TESTING_CABLE "\
Do you want to test the fiber optics cable?\n\n\
NOTE:  This test requires the following test tool available\n\
       at the branch office.\n\n\
       - The feedthrough connector Part Number 46F2438\n\
         and the half meter fiber optics cable Part Number 99F0489.\n\
                                 or\n\
       - The functionally equivalent end-of-cable wrap plug (SC EOCWP).\n\n\
       The one way length of the fiber optics cable to be tested should\n\
       not exceed 750 meters.\n\n\
Move cursor to selection, then press Enter."

$ Leave line spacing as shown.  See general notes on length of title line.

PLUG_CABLE "\
UNPLUG.........the wrap plug if attached from Port %1$s at location %2$s.\n\n\
PLUG...........the fiber optics cable to be tested into Port %1$s\n\
               at location %2$s.\n\n\
PLUG...........the feedthrough connector (Part Number 46F2438) \n\
               into the other end of the cable to be tested.\n\n\
PLUG...........the half meter fiber optics cable (Part Number 99F0489)\n\
               into the other end of the feedthrough connector,\n\
               if it is not already attached."

$ Leave line spacing as shown.  See general notes on length of title line.

UNPLUG_TOOL "\
UNPLUG.........the end-of-cable wrap plug.\n\n\
PLUG...........the fiber optics cable, if removed, back into the network."

$ Leave line spacing as shown.  See general notes on length of title line.

FIBER_CABLE "Fiber optics cable"

$ Leave line spacing as shown.  See general notes on length of title line.



NEW_TESTING_REGULAR "\
TESTING SERIAL OPTICAL LINK PORT %1$s IN LOCATION %2$s\n\n\
Please stand by."
$
$ This message is displayed when the Serial optic channel converter Diagnostic
$ Application begins execution.
$ %1$s is the port that is tested.
$ %2$s is the location code of the device as described in the general notes.
$ Leave line spacing as shown.  See general notes on length of title line.

NEW_TESTING_REGULAR_SLC "\
TESTING  SERIAL OPTICAL LINK CHIP %1$s IN LOCATION %2$s\n\n\
Please stand by."
$
$ This message is displayed when the Serial Link Chip  Diagnostic
$ Application begins execution.
$ %1$s is the port that is tested.
$ %2$s is the location code of the device as described in the general notes.
$ Leave line spacing as shown.  See general notes on length of title line.

NEW_TESTING_LOOPMODE "\
TESTING SERIAL OPTICAL LINK PORT %1$s IN LOCATION %2$s\n\
IN ADVANCED MODE\n\n\
Testing multiple times.\n\n\
%3$6d\tpasses completed.\n\
%4$6d\terrors logged.\n\n\
To stop the test, use 'Cancel.'"
$
$ This message is displayed while testing is performed in 
$ loop mode.
$ %1$s is the port that is tested.
$ %2$s is the location code of the device as described in the general notes
$ %3$6d\t sets the number of diagnostics test passes completed.
$ %4$6d\t sets the number of errors logged during the tests. 
$ Leave line spacing as shown.  See general notes on length of title line.

NEW_TESTING_LOOPMODE_SLC "\
TESTING SERIAL OPTICAL LINK CHIP %1$s IN LOCATION %2$s\n\
IN ADVANCED MODE\n\n\
Testing multiple times.\n\n\
%3$6d\tpasses completed.\n\
%4$6d\terrors logged.\n\n\
To stop the test, use 'Cancel.'"
$
$ This message is displayed while testing is performed in 
$ loop mode.
$ %1$s is the port that is tested.
$ %2$s is the location code of the device as described in the general notes
$ %3$6d\t sets the number of diagnostics test passes completed.
$ %4$6d\t sets the number of errors logged during the tests. 
$ Leave line spacing as shown.  See general notes on length of title line.

NEW_TESTING_ADVANCED_MODE "\
TESTING SERIAL OPTICAL LINK PORT %1$s IN LOCATION %2$s\n\
IN ADVANCED MODE\n\n\
Please stand by."
$
$ This message is displayed while testing is performed in 
$ advanced mode.
$ %1$s is the port that is tested.
$ %2$s is the location code of the device as described in the general notes
$ Leave line spacing as shown.  See general notes on length of title line.

NEW_TESTING_ADVANCED_MODE_SLC "\
TESTING SERIAL OPTICAL LINK CHIP %1$s IN LOCATION %2$s\n\
IN ADVANCED MODE\n\n\
Please stand by."
$
$ This message is displayed while testing is performed in 
$ advanced mode.
$ %1$s is the port that is tested.
$ %2$s is the location code of the device as described in the general notes
$ Leave line spacing as shown.  See general notes on length of title line.

NEW_TESTING_ADVANCED_MODE_WAITING "\
TESTING SERIAL OPTICAL LINK PORT %1$s IN LOCATION %2$s\n\
IN ADVANCED MODE\n\n\
This test may require up to 2 minutes.\n\n\
Please stand by."
$
$ This message is displayed while testing is performed in 
$ advanced mode and after the user chooses YES option for wrap mode.
$ %1$s is the port that is tested.
$ %2$s is the location code of the device as described in the general notes
$ Leave line spacing as shown.  See general notes on length of title line.
 
NEW_ADVANCED_NO_STANDBY "\
TESTING SERIAL OPTICAL LINK PORT %1$s IN LOCATION %2$s\n\
IN ADVANCED MODE"
$
$
$ This message is displayed while testing is performed in 
$ advanced mode.
$ %1$s is the port that is tested.
$ %2$s is the location code of the device as described in the general notes
$ Leave line spacing as shown.  See general notes on length of title line.

