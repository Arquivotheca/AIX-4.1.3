$ @(#)01        1.6  src/bos/diag/util/udtl/udtl.msg, dsaudtl, bos411, 9428A410j 8/6/93 16:31:46
$
$   COMPONENT_NAME: DSAUDTL
$
$   FUNCTIONS: Diagnostic message file.
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
$       This file contains the corresponding messages for the menus and
$       errors generated from the UDTL Utility.
$
$ END OF GENERAL NOTES


$set MSET
$quote "

MTITLE "DISPLAY OR CHANGE DIAGNOSTIC TEST LIST SERVICE AID"
$
$ See general notes on length of title.

MOPT1 "\
Display the Diagnostic Test List\n\
  This selection displays what resources are tested by the diagnostics."
$
$ Leave indentation as shown, two blank spaces after the first line.

MOPT2 "\
Add a resource to the Diagnostic Test List\n\
  This selection allows a resource that has previously been deleted\n\
  from the Diagnostic Test List to be added back."
$
$ Leave indentation as shown, two blank spaces after the first line.

MOPT3 "\
Delete a resource from the Diagnostic Test List\n\
  This selection allows a resource to be deleted from the Diagnostic\n\
  Test List.\n\
  NOTE:  Once a resource has been deleted, it is not put back into the\n\
  list unless done so by the 'Add a resource to the Diagnostic Test List'\n\
  selection from this menu."
$
$ Leave indentation as shown, two blank spaces after the first line.

MLASTLINE "Move cursor to selection, then press Enter."

$set MSET1

M1TITLE "DIAGNOSTIC TEST LIST\n\n\
The following resources will be tested by the diagnostics.\n\
A '-' in front of a resource indicates the resource may NOT be\n\
deleted from the Diagnostic Test List."
$
$ Leave line spacing as shown.  See general notes on length of title.

M1LASTLINE "\
Scroll through the list to see additional resources."

$set MSET3

M3TITLE "ADD RESOURCE TO DIAGNOSTIC TEST LIST\n\n\
An '*' in front of a resource indicates that it will be added\n\
to the Diagnostic Test List."
$
$ Leave line spacing as shown.  See general notes on length of title.

M3LASTLINE_A "\
Move cursor to selection, then press 'Enter' to select or deselect the\n\
resources to be added to the test list. When finished, select 'Commit'\n\
to save the changes."

$set MSET4

M4TITLE "DELETE RESOURCE FROM THE DIAGNOSTIC TEST LIST\n\n\
An '*' in front of a resource indicates that it will be deleted from the\n\
Diagnostic Test List.  Once deleted, a resource cannot be tested until\n\
it is added back to the list.  A '-' in front of a resource indicates the\n\
resource may NOT be deleted from the Diagnostic Test List."
$
$ Leave line spacing as shown.  See general notes on length of title.

M4LASTLINE_A "\
Move cursor to selection, then press 'Enter' to select or deselect the\n\
resources to be deleted from the test list. When finished, select 'Commit'\n\
to save the changes."

$set ESET
ERROR1 "Error with Object Class %s."
ERROR2 "\
Resource %s cannot be deleted\n\
from the test list."
NO_ADD "\
No other resources can be added\n\
to the test list."
NO_DEL "\
No other resources can be deleted\n\
from the test list."
