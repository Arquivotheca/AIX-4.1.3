$ @(#)46 1.1 src/bos/diag/util/ump/ump.msg, dsaump, bos411, 9428A410j 12/14/93 03:27:53

$ COMPONENT_NAME: DSAUMP
$ 
$ FUNCTIONS: Message set for UMP Service Aid.
$ 
$ ORIGINS: 83 
$ 
$ (C) COPYRIGHT International Business Machines Corp.1993
$ All Rights Reserved
$
$ LEVEL 1, 5 Years Bull Confidential Information
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
$ errors generated from the UMP Utility.
$
$ END OF GENERAL NOTES


$set UMP
$quote "

$ Must be the first message in the first set of this file.
$ Was formerly in dutil.msg.
UMP_MOPT1 "Multi-Processor Service Aids\n\
  This selection allows various functions to be performed on\n\
  a multi-processor system."

UMPTITLE "\
MULTI-PROCESSOR SERVICE AIDS"
$
$ See general notes on length of title.

UMPOPT1 "Display or Change Processor States"
UMPLASTLINE "Move cursor to selection, then press Enter."

$set UMP_PRST
$quote "

PRSTTITLE "\
DISPLAY OR CHANGE PROCESSOR STATE SERVICE AID"
$
$ See general notes on length of title.

PRSTOPT1 "Display Processor States"
PRSTOPT2 "Disable a Processor"
PRSTOPT3 "Enable a Processor"
PRSTLASTLINE "Move cursor to selection, then press Enter."

$set UMP_DISPLAY
$quote "

DSPTITLE "DISPLAY PROCESSOR STATES"
$
$ See general notes on length of title.

DSPLASTLINE "To continue, press Enter."

$set UMP_ALTER
$quote "

DISABLETITLE "DISABLE A PROCESSOR"
$
$ See general notes on length of title.

ENABLETITLE "ENABLE A PROCESSOR"
$
$ See general notes on length of title.

ALTERPROMPT "Enter the processor number"

ALTERLASTLINE "Type or select value in the entry field.\n"

$set UMP_ERR
$quote "

CMDERR "COMMAND FAILED:\n\n\%s"
$
$ %s is the command error returned.

CMDOK "COMMAND SUCCESSFULL:\n\n%s"
$
$ %s is the command output  returned.

MEM_ERR "** FATAL ERROR --> out of memory **"

UNKNOWN "unknown reason"
