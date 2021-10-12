$ static char sccsid[] = "@(#)05	1.6.1.8  src/bos/diag/tu/tablet/dtablet.msg, datablet, bos411, 9428A410j 3/17/94 19:29:17"; 
$
$   COMPONENT_NAME: DATABLET
$
$   FUNCTIONS: Diagnostic message file.
$		
$
$   ORIGINS: 27
$
$
$   (C) COPYRIGHT International Business Machines Corp. 1989,1994
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
$ The location of a resource is in the form of xx-xx-xx where x is an
$   alpha-numeric character.  The location is not translatable.  It is
$   an alpha-numeric descriptor of where the resource can be found.
$
$ END OF GENERAL NOTES


$quote "
$set TABLET_DIAG

TM_1A   "\
TESTING TABLET"
$
$ See general notes on length of title.

TM_1B   "\
TESTING TABLET\n\n\
Please stand by."
$
$ Leave line spacing as shown.  See general notes on length of title.

TM_2A   "\
TESTING TABLET IN ADVANCED MODE"
$
$ Leave line spacing as shown.

TM_2B   "\
TESTING TABLET IN ADVANCED MODE\n\n\
Please stand by."
$
$ Leave line spacing as shown.  See general notes on length of title.

TM_2C   "\
TESTING TABLET IN ADVANCED MODE\n\n\
Testing multiple times.\n\n\
%1$6d\tpasses completed.\n\
%2$6d\terrors logged.\n\n\
To stop the test, use 'Cancel.'"
$
$ %1$6d\t sets the number of diagnostic test passes completed.
$ %2$6d\t sets the number of errors logged during the tests.
$ Leave line spacing as shown.  See general notes on length of title.

TM_5    "\
YES"
$ This option is shown when a YES answer is possible.

TM_6    "\
NO"
$ This option is shown when a NO answer is possible.

TM_7    "\
Move cursor to selection, then press Enter."

TM_8    "\
When finished, press Enter."

TM_9    "\
To continue, press Enter."

TM_10   "\
The following test will cause the light in the\n\
small leftmost square at the top of the tablet\n\
to blink several times.\n\n\
MOVE...........the tablet cursor or stylus off of\n\
               the tablet surface.\n\
OBSERVE........the light in the small leftmost square at the\n\
               top of the tablet after pressing the Enter key.\n\
PRESS..........the Enter key to start the test."
$
$ Leave line spacing as shown.  See general notes on expanding ACTION lines.

TM_11   "\
Did the light in the small leftmost square blink?\n\n%s"
$
$ Leave line spacing as shown.

TM_22   "\
PLACE..........the tablet cursor or stylus on the small rightmost\n\
               square at the top of the tablet.\n\n\
Is the light in the small center square at the top of the tablet on?\n\n%s"
$
$ Leave line spacing as shown.

TM_23   "\
PLACE..........the tablet cursor or stylus inside the large square\n\
               on the tablet.\n\n\
Is the light in the small center square at the top of the tablet off?\n\n%s"
$
$ Leave line spacing as shown.

TM_24   "\
PRESS..........each button on the tablet cursor one at a time.\n\
               (If you are using the stylus, press the tip of the stylus\n\
               on the surface of the tablet to activate the tip switch.)\n\n\
Did the light in the small center square at the top of the tablet turn on\n\
when the stylus or each button of the tablet cursor was pressed?\n\n%s"
$
$ Leave line spacing as shown.

TM_25   "\
The light in the small leftmost square at the top of the tablet\n\
will be on when the tablet cursor or stylus is in motion inside\n\
the large square.\n\n\
MOVE...........the tablet cursor or stylus on the surface of the\n\
               tablet inside the large square.\n\
OBSERVE........the light in the small leftmost square at the top\n\
               of the tablet.\n\n\
Did the light turn on while the tablet cursor or stylus was in motion?\n\n%s"
$
$ Leave line spacing as shown.  See general notes on expanding ACTION lines.

TM_37   "\
The light in the small leftmost square at the top of the tablet\n\
will be on when the tablet cursor or stylus is in motion inside\n\
the large square.\n\n\
MOVE...........the tablet cursor or stylus on the surface of the\n\
               tablet inside the large square.\n\
OBSERVE........the light in the small leftmost square at the top\n\
               of the tablet.\n\n\
Did the light turn on while the tablet cursor or stylus was in motion?\n\n%s"
$
$ Leave line spacing as shown.  See general notes on expanding ACTION lines.

TM_38   "\
The light in the leftmost small square at the top of the tablet\n\
should remain off when the tablet cursor or stylus is in motion\n\
inside the large square and the light in the center square at\n\
the top of the tablet will be on when a switch on the tablet\n\
cursor or stylus is pressed.\n\n\
MOVE...........the tablet cursor or stylus while pressing\n\
               the tablet cursor or stylus device switches.\n\
OBSERVE........the left and center indicator lights.\n\n\
Did both lights operate correctly?\n\n%s"
$
$ Leave line spacing as shown.  See general notes on expanding ACTION lines.

TM_10A  "\
The following test will cause the light in the\n\
small leftmost square at the top of the tablet\n\
to blink several times.\n\n\
MOVE...........the tablet cursor or stylus off of\n\
               the tablet surface.\n\
PRESS..........the Enter key to start the test."
$
$ Leave line spacing as shown.  See general notes on expanding ACTION lines.

TM_11A  "\
OBSERVE........the light in the small leftmost square at the top\n\
               of the tablet.\n\n\
Is the light blinking?\n\n%s"
$
$ Leave line spacing as shown.

TM_23A  "\
Is the light in the small center square at the top\n\
of the tablet still off?\n\n%s"
$
$ Leave line spacing as shown.

TM_24A  "\
PRESS..........each button on the tablet cursor one at a time.\n\
               If you are using the stylus, press the tip of the stylus\n\
               on the surface of the tablet to activate the tip switch.\n\
               Also, if present, press any buttons on the stylus, and then\n\
               activate the tip switch and press any buttons at the same\n\
               time.\n\n\
Did the light in the small center square at the top of the tablet turn on\n\
when the stylus or each button of the tablet cursor was pressed?\n\n%s"
$
$ Leave line spacing as shown.

TM_25A  "\
Are you using a MODEL 5083 tablet ?\n\
(See the label on the underside of the tablet for the model number.)\n\n%s"
$
$ Leave line spacing as shown.

TM_26A  "\
For further problem isolation, please refer to\n\
the documentation that came with the tablet."
$
$ Leave line spacing as shown.

TM_24B   "\
PRESS..........each button on the tablet cursor one at a time.\n\
               (If you are using the stylus, press the tip of the stylus\n\
               on the surface of the tablet to activate the tip switch.)\n\n\
Did the light in the small center square at the top of the tablet turn off\n\
when the stylus or each button of the tablet cursor was pressed?\n\n%s"
$
$ Leave line spacing as shown.

FREE_DEVICE     "%1$6X\
Must free the following resource to continue further testing of the\n\
tablet device.\n\n\
The resource is\n\
- %2$s           %3$s            Keyboard"
$
$ Leave line spacing as shown. See general notes on expanding ACTION lines.

FREE_DEVICE1    "%1$6X \
FREE THE DEVICE\n\
Must free the following resource to continue further testing\n\
of the tablet device.\n\n\
The resource is\n\
- %2$s           %3$s            Keyboard"
$
$ Leave line spacing as shown. See general notes on expanding ACTION lines.
