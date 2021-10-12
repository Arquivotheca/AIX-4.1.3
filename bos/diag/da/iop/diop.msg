$ @(#)73	1.4.4.1  src/bos/diag/da/iop/diop.msg, daiop, bos41J, 9513A_all 3/9/95 09:25:08
$
$   COMPONENT_NAME: DAIOP
$
$   FUNCTIONS: 
$
$   ORIGINS: 27, 83
$
$
$   (C) COPYRIGHT International Business Machines Corp. 1989,1995
$   All Rights Reserved
$   Licensed Materials - Property of IBM
$   US Government Users Restricted Rights - Use, duplication or
$   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$   LEVEL 1, 5 Years Bull Confidential Information

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


$quote "
$set IO_Planar_Diagnostics
$ This is the message catalog file for the I/O Planar
$ .......................................................................

Testing_msg "TESTING I/O PLANAR %1$s"
$
$ This message is displayed when generic testing is being done
$ %1$s is replaced by  either IN ADVANCED MODE or blanks.
$ I/O stands for Input/Output and is a technical term.

Stand_by  "Please stand by."

Yes_option "YES"
$ This option is shown when a YES answer is possible.

No_option "NO"
$ This option is shown when a NO answer is possible.

Service "SERVICE"
$ This will be substituted in a sentence.  Translate accordingly.

Normal "NORMAL"
$ This will be substituted in a sentence.  Translate accordingly.

Secure "SECURE" 
$ This will be substituted in a sentence.  Translate accordingly.

Advanced_Mode "IN ADVANCED MODE"
$
$ This message can be appended to "TESTING I/O PLANAR".  Together, these two
$ messages cannot be more than 70 characters on the first line.
$ See general notes for title line restrictions.

No_Mode " "
$ This message is blank.

LED_Test_Start "Operator Panel Three-Digit Display Test\n\n\
The following test will be used to ensure the Three-Digit Display\n\
in the operator panel is working properly.\n\n\
When the test is started, the three digits will display 666 and 999\n\
for approximately 5 seconds each.\n\n\
OBSERVE........the Three-Digit Display after starting the test.\n\n\
To start the test, press Enter."
$
$ This menu  is used when entering the
$ Operator panel Three-Digit Display test.
$ Leave line spacing as shown.

LED_Test_End "Operator Panel Three-Digit Display Test\n\n\
Did the values 666 and 999 show up in the Three-Digit Display?\n\n\
Move cursor to selection, then press Enter."
$
$ This menu is used when exiting the
$ Operator panel Three-Digit Display test.
$ Leave line spacing as shown.

Keylock_Start "Operator Panel Key Mode Switch Test\n\n\
The following tests will be used to ensure the Key Mode Switch\n\
in the operator panel is working properly.\n\n\
When asked, turn the key in the Key Mode Switch to the\n\
functional position indicated.\n\n\
To start the tests, press Enter."
$
$ Leave line spacing between sentences as shown.

Keylock_Testing "Operator Panel Key Mode Switch Test\n\n\
TURN...........the key to the %1$s position.\n\n\
To continue, press enter."
$
$ This menu is for the Operator Panel keylock test
$ The %1$s is one of the three key positions that are to be tested:
$  SERVICE, NORMAL, or SECURE.  Please translate in conjunction with
$ these substitutions to ensure proper grammar.
$ Leave line spacing between sentences as shown.

TOD_Set_msg "Time of Day Reset\n\n\
A previous test set the time of day to Jan 1, 1989.\n\
The date and time should be reset to the current time after\n\
exiting from these diagnostic tests.\n\n\
To continue, press Enter."
$
$ This message is displayed if the testing has show that the 
$ day/date clock was reset
$ Leave line spacing between sentences as shown.

Loop_msg "Testing multiple times.\n\n\
%1$6d\tpasses completed.\n\
%2$6d\terrors logged.\n\n\
To stop the test, use 'Cancel.'"
$
$ %1$6d\t sets the number of diagnostic test passes completed.
$ %2$6d\t sets the number of errors logged during the tests.
$ Leave line spacing between sentences as shown.
$ This message is displayed when in the loop mode 

NVRAM_TST "NVRAM Battery Test\n\n\
Has the battery recently been replaced or unplugged while the power\n\
to the system unit was OFF?\n\n\
Move cursor to selection, then press Enter."

$ This message apprears when testing indicates that the NVRAM may
$ be faulty.  NVRAM stands for Non-Volatile Random Access Memory.  
$ Do not translate NVRAM unless an equivalent term exists.

TESTING_msg "TESTING I/O PLANAR (%1$s) %2$s"
$
$ This message is displayed when generic testing is being done
$ %1$s is replaced by the device name.
$ %2$s is replaced by  either IN ADVANCED MODE or blanks.
$ I/O stands for Input/Output and is a technical term.

SWITCH_TESTING "Operator Panel %1$s Mode Switch Test\n\n\
%2$s...........the %3$s to the %4$s position.\n\n\
To continue, press enter."
$
$ This menu is for the Operator Panel keylock/switch test
$ %1$s is either "key" or blank.
$ %2$s will be TURN or MOVE. 
$ %3$s is either "key" or "switch".
$ %4$s is one of the three key positions that are to be tested:
$ SERVICE, NORMAL, or SECURE.  Please translate in conjunction with
$ these substitutions to ensure proper grammar.
$ Leave line spacing between sentences as shown.

SWITCH_START "Operator Panel %1$s Mode Switch Test\n\n\
The following tests will be used to ensure the %2$s Mode Switch\n\
in the operator panel is working properly.\n\n\
When asked, move the Mode Switch to the\n\
functional position indicated.\n\n\
To start the tests, press Enter."
$
$ %1$s is either "Key" or ""
$ %2$s is either "Key" or ""
$ Leave line spacing between sentences as shown.

KEY "Key"
$ This will be substituted in a sentence.  Translate accordingly.

NO_KEY ""
$ This will be substituted in a sentence.  Translate accordingly.

SWITCH "switch"
$ This will be substituted in a sentence.  Translate accordingly.

TURN "TURN"
$ This will be substituted in a sentence.  Translate accordingly.

MOVE "MOVE"
$ This will be substituted in a sentence.  Translate accordingly.

LCD_Test_Start "Operator Panel Liquid Crystal Display Test\n\n\
The following test will be used to ensure the Liquid Crystal Display\n\
in the operator panel is working properly.\n\n\
When the test is started, the LCD will display U*U* and VAS\n\
for approximately 5 seconds each.\n\n\
OBSERVE........the Liquid Crystal Display after starting the test.\n\n\
To start the test, press Enter."
$
$ This menu  is used when entering the
$ Operator panel Liquid Crystal Display test.
$ Leave line spacing as shown.
$ U*U*, VAS are displayed strings and must be kept unchanged

LCD_Test_End "Operator Panel Liquid Crystal Display Test\n\n\
Did the values U*U* and VAS show up in the Liquid Crystal Display?\n\n\
Move cursor to selection, then press Enter."
$
$ This menu is used when exiting the
$ Operator panel Liquid Crystal Display test.
$ Leave line spacing as shown.
$ U*U*, VAS are displayed strings and must be kept unchanged

$ 
$set IO_FRUS
$ Reason code set used by device type "IoPlanar"

DIOP_ERR_NVRAM   "I/O Planar - NVRAM test failed."
$ I/O stands for Input/Output.
$ NVRAM stands for Non-Volatile Random Access Memory.
DIOP_ERR_TOD     "I/O Planar - Time of Day RAM test failed."
$ I/O stands for Input/Output.
$ RAM stands for Random Access Memory.
DIOP_ERR_CRRA    "I/O Planar - CRRA register test failed."
$ I/O stands for Input/Output.
$ CRRA stands for
DIOP_ERR_OP      "I/O Planar - operator test panel LED failed."
$ I/O stands for Input/Output.
$ LED stands for Light Emitting Diode.
DIOP_ERR_BAT     "I/O Planar - battery test failed."
$ I/O stands for Input/Output.
DIOP_ERR_TOD_P   "I/O Planar - Time of Day a POR"
$ I/O stands for Input/Output.
$ POR stands for
DIOP_ERR_TOD_N   "I/O Planar - Time of Day clock is not running."
$ I/O stands for Input/Output.
DIOP_ERR_KEY     "I/O Planar - key lock failed."
$ I/O stands for Input/Output.
OP_PANEL         "Operator panel"
$ I/O stands for Input/Output.
BATT             "I/O Planar - battery"
$ I/O stands for Input/Output.
KEYLOCK          "Operator panel key lock"
IOP_LED          "Three-Digit Display"
DIOP_EPOW "The EPOW connector test failed."
$ EPOW stands for Early Power Off Warning.
DIOP_ERR_VPD     "I/O Planar - VPD test failed."
$ I/O stands for Input/Output.
DIOP_ERR_ECREG   "The register test on the second I/O planar failed."
$ I/O stands for Input/Output.
DIOP_ERR_MODE_SWITCH   "I/O Planar - mode switch failed."
$ I/O stands for Input/Output.
DIOP_DMA_ERR   "Error log indicates that a DMA error occurred"
$
DIOD_ERR_NVRAM   "IOD Board - NVRAM test failed."
$ IOD Board stands for Input/Output Daughter Board.
$ NVRAM stands for Non-Volatile Random Access Memory.
DIOD_ERR_LCD     "IOD Board - operator panel LCD test failed."
$ IOD Board stands for Input/Output Daughter Board.
$ LCD stands for Liquid Crystal Display.
DIOD_ERR_KEY     "IOD Board - key lock failed."
$ IOD Board stands for Input/Output Daughter Board.
DIOD_ERR_TOD_P   "IOD Board - Time of Day at POR"
$ IOD Board stands for Input/Output Daughter Board.
$ POR stands for Power On Reset.
DIOD_ERR_TOD_N   "IOD Board - Time of Day RAM test failed."
$ IOD Board stands for Input/Output Daughter Board.
DIOD_ERR_TOD     "IOD Board - Time of Day clock is not running."
$ IOD Board stands for Input/Output Daughter Board.
$ RAM stands for Random Access Memory.
DIOD_ERR_BAT     "IOD Board - battery test failed."
$ IOD Board stands for Input/Output Daughter Board.
BBU_ERR		"Battery backup failure"
$ Battery backup unit for rack systems.
FAN_FAILURE     "Cooling fan failure"
$ This is for fan failure indicated in Power status registers.
MEDIA_FAN     "Media fan failure"
$ This is for fan failure indicated in Power status registers.
CPU_FAN     "CPU fan failure"
$ This is for fan failure indicated in Power status registers.
PS1_FAN     "Power supply 1 (primary) fan failure"
$ This is for fan failure indicated in Power status registers.
PS2_FAN     "Power supply 2 (secondary) or shell fan failure"
$ This is for fan failure indicated in Power status registers.
IO_BUS_CARD    "System interface bus card"
$ This is the interface card between the fans and the planars.

EXP_CAB_POWER_FAULT "Power problem in expansion cabinet"
$ This is to report a failure related to power supply

EXP_CAB_PS          "Expansion Cabinet Power Supply"
$ This is a power supply unit in an expansion cabinet
