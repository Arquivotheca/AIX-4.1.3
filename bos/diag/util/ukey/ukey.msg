$  @(#)72 1.1 src/bos/diag/util/ukey/ukey.msg, dsaukey, bos411, 9430C411a 7/22/94 04:30:51

$ COMPONENT_NAME: DSAUKEY
$ 
$ FUNCTIONS: Message set for Electronic Mode Switch Service Aids.
$ 
$ ORIGINS: 83 
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
$ errors generated from the UKEY Utility.
$
$ END OF GENERAL NOTES


$set UKEY
$quote "

$ Must be the first message in the first set of this file.
$ Was formerly in dutil.msg.
UKEY "Display or Change Electronic Mode Switch\n\
  This selection displays the setting of the Mode Switch\n\
  and allows the Electronic Mode Switch to be changed."

UKEYTITLE "\
DISPLAY OR CHANGE ELECTRONIC MODE SWITCH"
$
$ See general notes on length of title.

UKEYOPT1 "Display Mode Switch"
UKEYOPT2 "Change Electronic Mode Switch"
UKEYLASTLINE "Move cursor to selection, then press Enter."

$set DSP_UKEY
$quote "

DSPUKEYTITLE "\
DISPLAY MODE SWITCH"
$
$ See general notes on length of title.

DSPUKEYLASTLINE "To continue, press Enter."

$set CHG_UKEY
$quote "

CHGUKEYTITLE "\
CHANGE ELECTRONIC MODE SWITCH"
$
$ See general notes on length of title.

ALTERPROMPT "Enter the Electronic Mode Switch value"

ALTERLASTLINE "Select value in the entry field.\n"

$set UKEY_ERR
$quote "

CMDERR "COMMAND FAILED:\n\n\%s"
$
$ %s is the command error returned.

CMDOK "COMMAND SUCCESSFULL:\n\n%s"
$
$ %s is the command output  returned.

MEM_ERR "** FATAL ERROR --> out of memory **"

UNKNOWN "unknown reason"
