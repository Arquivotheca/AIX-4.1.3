$ @(#)03	1.5.1.1  src/bos/diag/util/udiagmon/udiagmon.msg, dsaudiagmon, bos411, 9430C411a 7/26/94 14:44:24
$
$   COMPONENT_NAME: DSAUDIAGMON
$
$   FUNCTIONS: Diagnostic message file.
$		
$
$   ORIGINS: 27
$
$
$   (C) COPYRIGHT International Business Machines Corp. 1993, 1994
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
$ These comments concern the technical terms used in the messages.
$   SCSI:  is an acronym for "Small Computer Systems Interface."  SCSI is
$      a technical term.  Do not translate unless an equivalent tern is
$      used in the translating country.  A SCSI Drawer is a drawer that
$      installs in a rack mounted system.
$
$ The location of a resource is in the form of xx-xx-xx where x is an
$   alpha-numeric character.  The location is not translatable.  It is
$   an alpha-numeric descriptor of where the resource can be found.
$
$ This file contains the corresponding messages for the menus and
$ errors generated from the UDIAGMON Utility.
$
$ END OF GENERAL NOTES


$set UDIAGMON_SET
$quote "

UDIAGMON_CONTINUE "To continue, press 'Enter'."

UDIAGMON_TITLE "\
PERIODIC DIAGNOSTICS SERVICE AID"
$
$ See general notes on length of title.

UDIAGMON_MENU_1 "PERIODIC DIAGNOSTICS SERVICE AID\n\n\
This service aid is used to periodically test hardware resources and\n\
monitor hardware errors in the error log.\n\n\
A hardware resource can be chosen to be tested once a day, at a user\n\
specified time of day. If the resource cannot be tested because it is\n\
busy, error log analysis will be performed.\n\
Hardware errors logged against a resource can also be monitored by enabling\n\
Automatic Error Log Analysis. This will allow error log analysis to be\n\
performed every time a hardware error is put into the error log.\n\n\
If a problem is detected, a message will be posted to the system console\n\
and a mail message sent to user(s) belonging to system group with information\n\
about the failure such as Service Request Number.\n\n\
To continue, press 'Enter'."
$
$ This panel will be the first panel displayed when invoking the service aid.
$ It provides instruction on what the service aid will do.
$
UDIAGMON_SELECTION1 "\
Add a resource to the periodic test list\n\
  This selection allows a resource to be periodically tested."
$
$ This is one of the selection option on the selection menu.
$
UDIAGMON_SELECTION2 "\
Delete a resource from the periodic test list\n\
  This selection removes a resource from the list of periodically\n\
  tested resources."
$
$ This is one of the selection option on the selection menu.
$
UDIAGMON_SELECTION3 "\
Modify the time to test a resource\n\
  This selection allows the time of day to test a resource to be\n\
  changed."
$
$ This is one of the selection option on the selection menu.
$
UDIAGMON_SELECTION4 "\
Display the periodic test list\n\
  This selection displays all resources being tested periodically\n\
  by diagnostics."
$
$ This is one of the selection option on the selection menu.
$
UDIAGMON_SELECTION5 "\
Enable Automatic Error Log Analysis\n\
  Automatic Error Log Analysis is currently disabled.\n\
  This selection allows Error Log Analysis to be invoked every time a\n\
  hardware error is put into the error log."
$
$ This is one of the selection option on the selection menu.
$
UDIAGMON_SELECTION6 "\
Disable Automatic Error Log Analysis\n\
  Automatic Error Log Analysis is currently enabled.\n\
  This selection stops the Automatic Error Log Analysis."
$
$ This is one of the selection option on the selection menu.
$
UDIAGMON_LASTLINE "\
Move cursor to selection, then press 'Enter'."

UDIAGMON_ADD "\
PERIODIC DIAGNOSTICS SERVICE AID\n\n\
%1$s\n\n%2$s\n\n"

UDIAGMON_ENTERTIME "\
Enter the time, between the brackets, when the resource should be tested."

UDIAGMON_HOUR "\
* HOUR (00-23) ................................."

UDIAGMON_MINUTE "\
* MINUTES (00-59) .............................."

UDIAGMON_ADD_DISPLAY "\
ADD RESOURCES TO THE PERIODIC DIAGNOSTICS TEST LIST\n\n\
The following resources are installed on your machine. The resources\n\
that are already present in the periodic test list are not shown here."

UDIAGMON_ADD_DISPLAY_LAST "\
Move the cursor to selection, then press 'Enter' to add it to the test list."

UDIAGMON_DEL_DISPLAY "\
DELETE RESOURCES FROM THE PERIODIC DIAGNOSTICS TEST LIST\n\n\
The following resources are currently being tested periodically.\n\
Test time is shown inside the brackets in 24 hour format.\n\
An '@' in front of a resource indicates that it will be deleted from the\n\
periodic test list.  Once deleted, a resource cannot be tested until\n\
it is added back to the list."  

UDIAGMON_SCROLL "\
Scroll through the list using the arrow keys, to see additional resources."

UDIAGMON_DEL_DISPLAY_LAST "\
Move cursor to selection, then press 'Enter' to select or deselect the\n\
resources to be deleted from the test list. When finished, select 'Commit'\n\
to save the changes."

UDIAGMON_MODIFY_DISPLAY "\
MODIFY PERIODIC DIAGNOSTICS TEST LIST\n\n\
The following resources are currently being tested periodically.\n\
Test time is shown inside the brackets in 24 hour format."

UDIAGMON_MODIFY_DISPLAY_LAST "\
Move the cursor to selection, then press 'Enter' to modify\n\
the test time."

UDIAGMON_DISPLAY "\
DISPLAY PERIODIC DIAGNOSTICS TEST LIST\n\n\
The following resources are currently being tested periodically.\n\
Test time is shown inside the brackets in 24 hour format."

UDIAGMON_ELA_ENABLED "\
PERIODIC DIAGNOSTICS SERVICE AID\n\n\
Automatic Error Log Analysis has been enabled."
$
$ This panel will be displayed after the user has
$ selected the option to enable Automatic Error Log Analysis.
$
UDIAGMON_ELA_DISABLED "\
PERIODIC DIAGNOSTICS SERVICE AID\n\n\
Automatic Error Log Analysis has been disabled."
$
$ This panel will be displayed after the user has
$ selected the option to disable Automatic Error Log Analysis.
$
NO_ADD "\
All available resources are currently\n\
in the periodic test list."
$
$ 3456789*123456789*123456789*123456789*12345678|      < 49 character line
$ Text should not be more than 49 characters per line.
$ Maintain spacing as shown.
NO_DEL "\
All available resources are currently\n\
removed from the test list."
$
$ 3456789*123456789*123456789*123456789*12345678|      < 49 character line
$ Text should not be more than 49 characters per line.
$ Maintain spacing as shown.
NO_DISPLAY "\
Resources must first be added to the\n\
test list."
$
$ 3456789*123456789*123456789*123456789*12345678|      < 49 character line
$ Text should not be more than 49 characters per line.
$ Maintain spacing as shown.
NO_ENABLE "\
Automatic Error Log Analysis cannot be\n\
enabled."
$
$ 3456789*123456789*123456789*123456789*12345678|      < 49 character line
$ Text should not be more than 49 characters per line.
$ Maintain spacing as shown.
NO_DISABLE "\
Automatic Error Log Analysis cannot be\n\
disabled."
$
$ 3456789*123456789*123456789*123456789*12345678|      < 49 character line
$ Text should not be more than 49 characters per line.
$ Maintain spacing as shown.
SAVE_CANCEL "\
Recent changes to the periodic test\n\
list will NOT be saved!\n\n\
If you want to save them, press 'Enter'."
$
$ 3456789*123456789*123456789*123456789*12345678|      < 49 character line
$ Text should not be more than 49 characters per line.
$ Maintain spacing as shown.

NO_UDIAGMON "\
\n\n           ** Error! **\n\
   Another 'udiagmon'(pid=%1$d) is\n\
           already running!\n\n\
\nNote: 'pid' stands for Process IDentifier.\n"
$
$ 3456789*123456789*123456789*123456789*12345678|      < 49 character line
$ Text should not be more than 49 characters per line.
$ Maintain spacing as shown.
NO_UDIAGMON_LOCK "\
Cannot create lock file for 'udiagmon' utility!"
$
$ 3456789*123456789*123456789*123456789*12345678|      < 49 character line
$ Text should not be more than 49 characters per line.
$ Maintain spacing as shown.
DATABASE_ERR "Error with Object Class %s."
$
$ 3456789*123456789*123456789*123456789*12345678|      < 49 character line
$ Text should not be more than 49 characters per line.
$ Maintain spacing as shown.
BLANK " "

UDIAGMON_ENTERTIME_1 "\
To enter the time when the resource should be tested,\n\
use the 'Tab' or 'List' key."
$
$set DIAGD_SET
NO_DIAGD "\
Error: Another 'diagd'(pid=%1$d) is already running!\n\n\
\nNote: 'pid' stands for Process IDentifier.\n"
$
NO_DIAGD_LOCK "\
Cannot create lock file for 'diagd' utility!\n"
$
NO_DIAGD_RES "\
There are no resources in the Periodic Diagnostics Test list.\n\
'diagd' utility exiting.\n"
$
$
$set HELP_SET
HELP_MSG1 "\
The information requested on this screen is the\n\
time of day, when the resource will be tested.\n\
To enter the time, use the 'Tab' key to select\n\
the desired value.\n\
(NOTE: minutes are in increment of 5)\n\
Once, the time has been selected, the 'Commit' \n\
key must be entered for the value to be \n\
registered."
$
$ 3456789*123456789*123456789*123456789*12345678|      < 49 character line
$ Text should not be more than 49 characters per line.
$ Maintain spacing as shown.
$

$set DIAGTEST
$quote "

ERROR_DETECTED "\
Periodic Diagnostics has detected error(s) that may require your attention."

NOTEST_PERFORMED "\
Unable to test resource %1$s because another diagnostic session was in progress."

