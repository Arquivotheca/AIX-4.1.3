$ @(#)71	1.4.1.3  src/bos/diag/util/uformat/kazfor.msg, dsauformat, bos411, 9428A410j 12/10/92 16:28:11
$
$   COMPONENT_NAME: DSAUFORMAT
$
$   FUNCTIONS: 
$
$   ORIGINS: 27
$
$   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$   combined with the aggregated modules for this product)
$                    SOURCE MATERIALS
$
$   (C) COPYRIGHT International Business Machines Corp. 1989,1992
$   All Rights Reserved
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
$ END OF GENERAL NOTES


$quote "
$set STANDBY
$

STITLE  "FORMATTING %1$s DISK DRIVE IN LOCATION %2$s\n\n\
Please stand by.\n\n\
%3$d\tcylinders have been formatted.\n\n\
To stop the format, use 'Cancel.'"
$
$ %1$s is the name of the device /dev/hdx
$ %2$s is the location of the device as described in the general notes.
$ %3$d\t is the number of cylinders that have been formatted to this point.
$ Leave line spacing as shown.  See general note on length of title.

$quote "
$set VER_STDBY
$

VTITLE  "CERTIFYING %1$s DISK DRIVE IN LOCATION %2$s\n\n\
This process will take approximately 3 minutes.\n\n\
Please stand by."
$
$ %1$s is the name of the device /dev/hdx
$ %2$s is the location of the device as described in the general notes.
$ Leave line spacing as shown.  See general note on length of title.

$quote "
$set ASK
$

ASK_TITLE "FORMATTING DISK DRIVE\n\n\
* * * WARNING! * * *\n\n\
THE FORMAT OPERATION WILL DESTROY ALL DATA ON THE DISK DRIVE!\n\n\
NOTE:  The disk drive should be powered on a minimum of 30 minutes\n\
       prior to attempting a format operation.\n\n\
Do you want to format this disk drive?"
$
$ This line inquires the user if formatting should really be done.
$ Leave line spacing as shown.  See general note on length of title.

ASK_YES "YES"
$ This option is shown when a YES answer is possible.

ASK_NO "NO"
$ This option is shown when a NO answer is possible.

ASK_ACTION "Move cursor to selection, then press Enter."

$quote "
$set HOUR_3
$

HOUR_3_TITLE "CERTIFYING DISK DRIVE\n\n\
NOTE:  Certification with surface analysis will take about 3 hours.\n\n\
Do you want to certify this disk drive?"
$
$ This line inquires the user if formatting should really be done.
$ Leave line spacing as shown.  See general notes on length of title.

HOUR_3_YES "YES"
$ This option is shown when a YES answer is possible.

HOUR_3_NO "NO - Return to Main Menu"
$ This option is shown when a NO answer is possible.

HOUR_3_ACTION "Move cursor to selection, then press Enter."

$quote "
$set COMPLETE
$

COMPLETE_TITLE "FORMATTING %1$s DISK DRIVE\n\n\
The format is complete.\n\n\
To return to the Selection menu, press Enter."
$
$ %1$s is the name of the device /dev/hdx
$ Leave line spacing as shown.  See general notes on length of title.

$quote "
$set V_COMPLETE
$

V_COMPLETE_TITLE "CERTIFYING %1$s DISK DRIVE\n\n\
The certify is complete.\n\n\
To return to the Selection menu, press Enter."
$
$ %1$s is the name of the device /dev/hdx
$ Leave line spacing as shown.  See general notes on length of title.

$quote "
$set CHECK
$

CHECK_TITLE "FORMATTING DISK DRIVE\n\n\
A format cancel has been detected.\n\n\
* * * WARNING! * * *\n\n\
STOPPING THE FORMAT OPERATION WILL LEAVE THE DISK DRIVE NOT USABLE.\n\n\
Do you want to stop the format process?"
$
$ inquires the user if stopping the process should really be done.
$ Leave line spacing as shown.  See general notes on length of title.

CHECK_YES "YES"
$ This option is shown when a YES answer is possible.

CHECK_NO "NO"
$ This option is shown when a NO answer is possible.

CHECK_ACTION "Move cursor to selection, then press Enter."

$quote "
$set VERIFY
VERIFY_TITLE "CERTIFYING DISK DRIVE\n\n\
A certify cancel has been detected.\n\n\
Do you want to stop the certify process?"
$
$ inquires the user if stopping the process should really be done.
$ Leave line spacing as shown.  See general notes on length of title.

VERIFY_YES "YES"
$ This option is shown when a YES answer is possible.

VERIFY_NO "NO"
$ This option is shown when a NO answer is possible.

VERIFY_ACTION "Move cursor to selection, then press Enter."

$quote "
$set SURE
$

SURE_TITLE "FORMATTING %1$s DISK DRIVE\n\n\
The format process has been stopped.\n\n\
* * * WARNING! * * *\n\n\
THE DISK DRIVE WILL NEED TO BE FORMATTED FOR FUTURE USE.\n\n\
To return to the Selection menu, press Enter."
$
$ This line warns the user to re-format the file.
$ %1$s is the name of the device /dev/hdx
$ Leave line spacing as shown.  See general notes on length of title.

$quote "
$set V_ABORT
$

V_ABORT_TITLE "CERTIFYING %1$s DISK DRIVE\n\n\
The certify process has stopped.\n\n\
To return to the Selection menu, press Enter."
$
$ %1$s is the name of the device /dev/hdx
$ Leave line spacing as shown.  See general notes on length of title.

$quote "
$set CANCEL
$

CANCEL_TITLE "FORMATTING %1$s DISK DRIVE\n\n\
The format process has stopped.\n\n\
* * * WARNING! * * *\n\n\
THE DISK DRIVE WILL NEED TO BE FORMATTED FOR FUTURE USE.\n\n\
To return to the Selection menu, press Enter."
$
$ %1$s is the name of the device /dev/hdx
$ The above line warns the user to re-format the file.
$ Leave line spacing as shown.  See general notes on length of title.

$quote "
$set QUIT
$

QUIT_TITLE "FORMATTING %1$s DISK DRIVE\n\n\
A cancel has been detected.\n\n\
The request to format has been cancelled.\n\n\
To return to the Selection menu, press Enter."
$
$ %1$s is the name of the device /dev/hdx
$ The above line warns the user to re-format the file.
$ Leave line spacing as shown.  See general notes on length of title.

$quote "
$set V_CANCEL
$

V_CANCEL_TITLE "CERTIFYING %1$s DISK DRIVE\n\n\
The certify process has stopped.\n\n\
To return to the Selection menu, press Enter."
$
$ %1$s is the name of the device /dev/hdx
$ Leave line spacing as shown.  See general notes on length of title.

$quote "
$set RESUME
$

RESUME_TITLE "FORMATTING %1$s DISK DRIVE\n\n\
The format process will be resumed.\n\n\
To continue, press Enter."
$
$ %1$s is the name of the device /dev/hdx
$ Leave line spacing as shown.  See general notes on length of title.

$quote "
$set V_RESUME
$

V_RESUME_TITLE "CERTIFYING %1$s DISK DRIVE\n\n\
The certify process will be resumed.\n\n\
To continue, press Enter."
$
$ %1$s is the name of the device /dev/hdx
$ Leave line spacing as shown.  See general notes on length of title.

$quote "
$set FAIL
$

FAIL_TITLE "FORMATTING %1$s DISK DRIVE\n\n\
The format process has failed.\n\n\
Replace the disk drive.\n\n\
To return to the Selection menu, press Enter."
$
$ %1$s is the name of the device /dev/hdx
$ Leave line spacing as shown.  See general notes on length of title.

FAIL_TITLE2 "FORMATTING %1$s DISK DRIVE\n\n\
The format process has failed.\n\n\
Perform a format with Surface Analysis. If this operation\n\
fails, replace the disk drive.\n\n\
To return to the Selection menu, press Enter."
$
$ %1$s is the name of the device /dev/hdx
$ Leave line spacing as shown.  See general notes on length of title.

$quote "
$set V_FAIL
$

V_FAIL_TITLE "CERTIFYING %1$s DISK DRIVE\n\n\
The certify process has failed.\n\n\
Replace or reformat the disk drive.\n\n\
To return to the Selection menu, press Enter."
$
$ %1$s is the name of the device /dev/hdx
$ Leave line spacing as shown.  See general notes on length of title.
$
$set CONFIG_FAIL
$quote "
$
CONFIG_FAIL_TITLE "FORMATTING AND CERTIFYING %s DISK DRIVE\n\n\
A software error has occurred.\n\n\
The disk drive cannot be configured.\n\n\
To return to the Selection Menu, press Enter."
$
$ Leave line spacing as shown.  See general notes on length of title.

$set OPEN_FAIL
$quote "
$
OPEN_FAIL_TITLE "FORMATTING AND CERTIFYING %s DISK DRIVE\n\n\
A software error has occurred.\n\n\
The device driver cannot be opened.\n\n\
To return to the Selection Menu, press Enter."
$
$ Leave line spacing as shown.  See general notes on length of title.
