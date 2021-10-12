$ @(#)21	1.2  src/bos/diag/diagrpt/diagrpt.msg, dutil, bos411, 9428A410j 12/10/92 16:42:32
$
$   COMPONENT_NAME: DUTIL
$
$   FUNCTIONS:  DIAGRPT MESSAGE FILE
$
$   ORIGINS: 27
$
$
$   (C) COPYRIGHT International Business Machines Corp. 1992
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
$ These comments concern the techincal terms used in the messages.
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
$ errors generated from the UCFGVPD Utility.
$
$ END OF GENERAL NOTES


$set MSGSET
$quote "

DSCROLL "\nUse arrow keys to scroll.\n\n\
Press Enter or 'Cancel' to exit."

DRPTTITLE "DISPLAY PREVIOUS DIAGNOSTIC RESULTS\n\n"

$
$ See general notes on length of title.

