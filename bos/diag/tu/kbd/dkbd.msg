$ @(#)83	1.3.1.7  src/bos/diag/tu/kbd/dkbd.msg, dakbd, bos411, 9428A410j 3/17/94 19:40:56
$
$   COMPONENT_NAME: DAKBD
$
$   FUNCTIONS: Diagnostic message file.
$		
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
$
$ LED is an acronym for Light Emitting Diode.  DO NOT TRANSLATE unless an
$    equivalent term exists in the translating language.
$
$ Toggle refers to a particular type of switch or key action where the same
$   action can produce different results depending on the original state.
$   For example, a key press that toggles the video between normal and
$   reverse would change from one state to the next.
$
$ END OF GENERAL NOTES


$quote "
$set	KBRD_MSGS

TESTING	"TESTING KEYBOARD"
$
$ See general notes on length of title.

TESTAM	"TESTING KEYBOARD IN ADVANCED MODE"
$
$ See general notes on length of title.

TESTLM	"TESTING KEYBOARD IN ADVANCED MODE\n\n\
Testing multiple times.\n\n\
%1$6d\tpasses completed.\n\
%2$6d\terrors logged.\n\n\
To stop the test, use 'Cancel.'"
$
$ %1$6d\t sets the number of diagnostic test passes completed.
$ %2$6d\t sets the number of errors logged during the tests.
$ Leave line spacing as shown.  See general notes on length of title.

STANDBY	"Please stand by."
RESULTS "Move cursor to selection, then press Enter."

SAYYES	"YES"
$ This option is shown when a YES answer is possible.

SAYNO	"NO"
$ This option is shown when a NO answer is possible.

RUNOVER	"RUN TEST AGAIN"

LEDSON	"The LED indicators on the keyboard attached to\n\
the system keyboard adapter should now be on.\n\n\
Are the keyboard LED indicators on?\n\n\
Move cursor to selection, then press Enter."
$ 
$ LED is a technical term.  See general notes.
$ Leave line spacing as shown.

LEDSOFF "The LED indicators on the keyboard attached to\n\
the system keyboard adapter should now be off.\n\n\
Are the keyboard LED indicators off?\n\n\
Move cursor to selection, then press Enter."
$ 
$ Leave line spacing as shown.

CLICK_OFF  "When the keystroke auto-click is disabled and a key\n\
is pressed the speaker will not emit a clicking sound.\n\n\
The keystroke auto-click is now disabled.\n\n\
GO.............to the keyboard attached to the system keyboard adapter.\n\
PRESS..........each of the keys on the keyboard (pressing the Esc key\n\
               will exit from this test).\n\
LISTEN.........for the absence of a clicking sound\n\
               when each key is pressed.\n\n\
When finished, press the Esc key on the keyboard attached to the system\n\
keyboard adapter."
$ 
$ LED is a technical term.  See general notes.
$ Leave line spacing as shown.  See general notes on expanding ACTION lines.

WAS_CLICK_OFF	"Was the keystroke auto-click disabled?\n\n\
Move cursor to selection, then press Enter."
$ 

CLICK_ON  "When the keystroke auto-click is enabled and a key\n\
is pressed the speaker emits a clicking sound.\n\n\
Keystroke auto-click is now enabled.\n\n\
GO.............to the keyboard attached to the system keyboard adapter.\n\
PRESS..........each of the keys on the keyboard (pressing the Esc key\n\
               will exit from this test).\n\
LISTEN.........for a clicking sound when each key is pressed.\n\n\
When finished, press the Esc key on the keyboard attached to the system\n\
keyboard adapter."
$ 
$ Leave line spacing as shown.  See general notes on expanding ACTION lines.

WAS_CLICK_ON	"Was the keystroke auto-click enabled?\n\n\
Move cursor to selection, then press Enter."
$ 

KEYPAD	"\
Pressing each key on the keyboard will toggle the corresponding X\n\
on the display between normal and reverse video.\n\n\
GO.............to the keyboard attached to the system keyboard adapter.\n\
PRESS..........each of the keys on the keyboard (pressing the Esc key\n\
               will exit from this test).\n\
OBSERVE........the corresponding X on the display.\n\n\
When finished, press the Esc key on the keyboard attached to the system\n\
keyboard adapter."
$ 
$ In this message, the character X, in english, is used just to indicate
$ a position on the keyboard.  If you should choose another character,
$ make sure that the character in the message KBPAT is changed to match
$ this character.
$ Leave line spacing as shown.  See general notes on expanding ACTION lines.
$ NOTE:  THIS MESSAGE CAN ONLY BE A MAXIMUM OF 9 LINES LONG.  DO NOT EXCEED
$        THIS 9 LINE LIMIT.

KBPAT	"X"
$ 
$ This character should match the character used in the test description
$ for message KEYPAD.

ALL_WORKED  "All of the keys worked correctly."
SOME_WORKED  "Some of the keys worked correctly."
NONE_WORKED  "None of the keys worked correctly."

SPEAKER	"This test will exercise the speaker volume, frequency, and\n\
duration functions.\n\n\
When the test starts you should:\n\n\
LISTEN.........for a high volume, low frequency, tone for 2 seconds.\n\
LISTEN.........for a low volume, low frequency, tone for 4 seconds.\n\
LISTEN.........for a high volume, high frequency, tone for 6 seconds.\n\n\
To start the test, press Enter."
$
$ The frequency refers to the pitch of the tone.
$ Leave line spacing as shown.  See general notes on expanding ACTION lines.

SPEAKER_WORK  "Did the speaker work correctly?"

MENUPROMPT "Exit=Esc                                                    "
$
$ This line appears at the bottom of the display.  It denotes that 
$ the only way to exit from the screen is to press the Esc key.
$ Translate only the word Exit.  DO NOT TRANSLATE Esc unless there is
$ an equivalent key on your country's keyboard.

SPENOENTER "This test will exercise the speaker volume, frequency, and\n\
duration functions.\n\n\
When the test starts you should:\n\n\
LISTEN.........for a high volume, low frequency, tone for 2 seconds.\n\
LISTEN.........for a low volume, low frequency, tone for 4 seconds.\n\
LISTEN.........for a high volume, high frequency, tone for 6 seconds."
$
$ The frequency refers to the pitch of the tone.
$ Leave line spacing as shown.  See general notes on expanding ACTION lines.

KBD_LEDS        "\
The following test will cause the LEDs on the keyboard attached to the\n\
system keyboard adapter to turn on for a minimum of 3 seconds and then\n\
turn off.\n\
NOTE: The LEDs may also flash briefly at the end of the test.\n\n\
OBSERVE.........the LEDs on the keyboard.\n\
PRESS...........Enter key to start the test."
$
$ Leave line spacing as shown.  See general notes on expanding ACTION lines.

KBD_NOENTER        "\
The following test will cause the LEDs on the keyboard attached to the\n\
system keyboard adapter to turn on for a minimum of 3 seconds and then\n\
turn off.\n\
NOTE: The LEDs may also flash briefly at the end of the test.\n\n\
OBSERVE.........the LEDs on the keyboard."
$
$ Leave line spacing as shown.  See general notes on expanding ACTION lines.

LEDS_ON_OK "Did the LEDs turn on for a minimum of 3 seconds?"
$
$ LED is a technical term.  See general notes.

LEDS_OFF_OK "Are the LEDs currently off?"
$
$ LED is a technical term.  See general notes.

FREE_DEVICE     "%1$6X\
Must free the following resource to continue further testing of the\n\
keyboard attached to the system keyboard adapter.\n\n\
The resource is\n\
- %2$s          %3$s            Tablet"
$
$ Leave line spacing as shown.  See general notes on expanding ACTION lines.

ASK_BRAZIL       "\
Are you using a 104-key Brazilian keyboard, Part Number %1$s ?\n\
(See the label on the underside of the keyboard for the part number.)"
$
$ Leave line spacing as shown.
$ %1$s is the part number of the Brazilian keyboard.

ASK_102		"\
Select the type of keyboard attached to the system under test.\n\
(See the label on the underside of the keyboard for the part number.)"

GERMAN_100	"100-key German keyboard, Part Number %s"
$
$ %s is the part number of the German keyboard.

BRAZIL_104	"104-key Brazilian/Portuguese keyboard, Part Number %s"
$
$ %s is the part number of the Brazilian keyboard.

OTHER_102	"Other 102-key keyboard"

KEYPAD_WORKED	"Did all the keys work correctly?"

FREE_DEVICE1    "%1$6X \
FREE THE DEVICE\n\
Must free the following resource to continue further testing\n\
of the keyboard attached to the system keyboard adapter.\n\n\
The resource is\n\
- %2$s          %3$s            Tablet"
$
$ Leave line spacing as shown.  See general notes on expanding ACTION lines.
