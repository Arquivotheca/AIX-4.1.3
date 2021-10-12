
$
$   COMPONENT_NAME: DSAUDISKMNT
$
$   FUNCTIONS: Diagnostic message file.
$		
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
$      a technical term.  Do not translate unless an equivalent term is
$      used in the translating country.  A SCSI Drawer is a drawer that
$      installs in a rack mounted system.
$
$ END OF GENERAL NOTES


$quote "

$ No line in these messages may exceed 40 columns in width.  Re-flow the lines
$ if necessary to fit within that maximum.
$ The following $ marks column 40:     $
$set HELP
$ Lines containing ".p" represent page breaks.  Do not translate these.
TEXT "\
Hex_edit allows you edit the ACTUAL byte\n\
stream that comprises blocks.  The bytes\n\
are displayed in 208 byte blocks, set in\n\
a hex window (624 hex nibbles) and in an\n\
ASCII window (non-displayable characters\n\
are represented by a '.').  Input in hex\n\
to change nibbles via the hex window or\n\
in characters to change bytes via the\n\
ASCII window.\n\n\
Changes will be saved automatically\n\
when you leave the current block.\n\n\
You are allowed only to CHANGE bytes in\n\
the blocks. You cannot insert/delete/add\n\
bytes.\n\
.p\n\
Pf key settings for Hex_edit are:-\n\n\
pf1 :- Help. \n\
pf3 :- Return to Previous Window.\n\
pf7 :- Save Changes.\n\
pf10:- Exit Program.\n\
.p\n\
Other key settings for Hex_edit are:-\n\n\
Home:- help.\n\
End :- exit Hex_edit or help window.\n\
Tab :- Flip from Hex to Ascii window.\n\
PageUp  :- scroll up the blocks.\n\
PageDown:- scroll down the blocks.\n\n\
- :- cursor to right\n\
<- :- cursor to left\n\
Down Arrow Key:- cursor down \n\
Up Arrow Key  :- cursor up \n\n"


HEAD0 "\
  Hex_edit Simple Help           page ?."

HEAD1 "\
 PageUp, PageDown to scroll Help Text   "

HEAD2 "\
        End to exit Simple Help         "

HEAD4 "\
           Hex_edit Simple Help         "

$ "cont" is short for "continued", meaning there is more on the next page.
HEAD6 "\
                           ...(cont)    "


