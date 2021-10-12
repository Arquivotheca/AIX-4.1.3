$ @(#)35        1.7.6.2  src/bos/diag/da/cdrom/dcdrom.msg, dacdrom, bos41J, 9512A_all 3/17/95 08:50:01
$
$   COMPONENT_NAME: DACDROM
$
$   FUNCTIONS: 
$
$   ORIGINS: 27
$
$
$   (C) COPYRIGHT International Business Machines Corp. 1989,1995
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
$ This is the messages catalog file for CD-ROM DRIVE Diagnstics.
$ CD-ROM DRIVE is the Compact Disc Read Only Media device that can
$ be either internally or externally the system cabinet.
$
$ END OF GENERAL NOTES


$quote "
$set CDROM_MENU

MENU_ADVANCED_MODE "\
IN ADVANCED MODE"
$
$ The string "IN ADVANCED MODE" is substituted in the string "TESTING SCSI
$ CD-ROM DRIVE" and must be translated in context.
$ This is displayed when running in advanced mode.

MENU_NORMAL_MODE "\
                "
$
$ This is a blank message.  DO NOT TRANSLATE.
$ This is displayed when running customer mode.

YES_OPTION "YES"
$ This option is shown when a YES answer is possible.

NO_OPTION "NO"
$ This option is shown when a NO answer is possible.

MENU_HEAD "\
TESTING SCSI CD-ROM DRIVE IN LOCATION %1$s\n\
%2$s"
$
$ This text is displayed at the top of every menu.
$ SCSI, is the acronym for Small Computer System Interface.
$ %1$s is the location code for the device, for example "00-00-F1"
$ The location is described in the general notes.
$ %2$s can either be "IN ADVANCED MODE" or blanks.  Translate in context.

MENU_CHECKOUT "\
Please stand by."
$
$ This menu is displayed when the device is being tested under
$ the SYSTEM CHECKOUT option.  No user interaction is required.
$ Leave line spacing as shown.

MENU_WORKING "\
Please stand by."
$
$ This menu is displayed after each menu in which a person has
$ to press a key.  This is an indication to the user that the computer
$ has detected his key press.
$ Leave line spacing as shown.

MENU_ONE "\
The following test requires the use of the CD-ROM test disc,\n\
Part Number %s.\n\n\
Do you have this test disc?\n\n\
Move cursor to selection, then press Enter."
$
$ the user will be given the options "YES_OPTION" and "NO_OPTION".
$ Leave line spacing as shown.

MENU_TWO "\
REMOVE.........the test disc from its caddy.\n\
INSERT.........the empty caddy in the CD-ROM Drive.\n\n\
When finished, press Enter."
$
$ Caddy is the term used to refer to the container that holds the CD-ROM
$ Test Disc.
$ Leave line spacing as shown.  See general notes on expanding ACTION lines.

MENU_THREE "\
Is the drive in-use light continuously ON?\n\n\
Move cursor to selection, then press Enter."
$
$ the user will be given the options "YES_OPTION" and "NO_OPTION".
$ The drive in-use light is a light on the front of the CD-ROM Drive.
$ Leave line spacing as shown.

MENU_FOUR "\
REMOVE.........the test caddy from the CD-ROM drive.\n\
PLACE..........the test disc in its caddy.\n\
INSERT.........caddy into the CD-ROM Drive.\n\n\
When finished, press Enter."
$
$ Leave line spacing as shown.  See general notes on expanding ACTION lines.

MENU_NINE "\
The following test requires the use of a known working stereo headset.\n\n\
Do you have a known working stereo headset?\n\n\
Move cursor to selection, then press Enter."
$
$ the user will be given the options "YES_OPTION" and "NO_OPTION".
$ Leave line spacing as shown.

MENU_TEN "\
PLUG...........the stereo headset into the audio output jack.\n\
SET............the volume control to the middle of its range.\n\
PLACE..........the headset over your ears.\n\n\
When finished, press Enter."
$
$ Leave line spacing as shown.  See general notes on expanding ACTION lines.

MENU_ELEVEN "\
The following test will emit a tone\n\
from the left and right speakers.\n\n\
LISTEN.........for a tone from each speaker.\n\n\
To start the test, press Enter."
$
$ Leave line spacing as shown.  See general notes on expanding ACTION lines.

MENU_TWELVE "\
Was a tone heard from both the left and right speakers?\n\n\
Move cursor to selection, then press Enter."
$
$ the user will be given the options "YES_OPTION" and "NO_OPTION".
$ Leave line spacing as shown.  See general notes on expanding ACTION lines.

LOOP_MODE_MENU "\
Testing multiple times.\n\n\
%1$6d\tpasses completed.\n\
%2$6d\terrors logged.\n\n\
To stop the test, use 'Cancel.'"
$
$ This menu is displayed during testing when the option has been
$ selected for multiple passes of testing.
$ %1$6d\t sets the number of diagnostic test passes completed.
$ %2$6d\t sets the number of errors logged during the tests.
$ Leave line spacing as shown.

LAST_LOOP_MENU "\
Testing multiple times.\n\n\
%1$6d\tpasses completed.\n\
%2$6d\terrors logged.\n\n\
Test is completing last pass."
$
$ This menu is displayed during testing when the option was
$ selected for multiple passes of testing and the user pressed Cancel.
$ Numbers will be substituted for the two "%d".
$ %1$6d\t sets the number of diagnostic test passes completed.
$ %2$6d\t sets the number of errors logged during the tests.
$ Leave line spacing as shown.

MENU_GOAL "974014 \
TESTING SCSI CD-ROM DRIVE IN LOCATION %1$s %2$s\n\
No Problems were found, however, error log analysis \n\
indicated that the device has a number of media error\n\
entries. It is recommended that the devices cleaning\n\
procedure be performed.\n\
For instructions on cleaning, refer to the Setup and Operator\n\
Guide for this device.\n"
$
$ The above menu is presented when error log analysis has detected
$ a number of media errors.
$ DO NOT TRANSLATE the number at the beginning of this message. Leave it
$ exactly as shown - 974014.
$ %1$s is the location code of the device.
$ %2$s can either be ADVANCED_MODE or blanks.
$ Leave line spacing as shown. See general notes on length of title.

BAD_MEDIA_MSG "\
A possible media error has been detected.\n\n\
ENSURE.........that there is a disc inserted into the caddy and that it is\n\
               Part Number %1$s.\n\
INSERT.........caddy into the CD-ROM Drive.\n\n\
To continue, press Enter."
$
$ The above menu is displayed when it is possible that there is an error
$ on the test disc itself, rather than in the CD-ROM Drive.
$ Leave line spacing as shown.  See general notes on expanding ACTION lines.

EXER_MENU "\
DIAGNOSTIC SYSTEM EXERCISER SETUP MENU\n\n\
You may decide to run the System Exerciser with media installed in\n\
a device or with no media installed in a device.  Running the test\n\
with media installed in a device is a more thorough test.\n\n\
The test for the CD-ROM drive in location %1$s can be run\n\
with a CD-ROM test disk installed.  The test will require a CD-ROM\n\
test disk, Part Number %2$s."
$ Used for the Sytem Exerciser set-up menu.

OPTION_EXER_1 "\
Run System Exerciser with Media Installed"
$ Used for the Sytem Exerciser set-up menu.

OPTION_EXER_2 "\
Run System Exerciser with No Media Installed"
$ Used for the Sytem Exerciser set-up menu.

MOVE_CURSOR "\
Move cursor to selection, then press Enter."
$ General message for selecting an option.

EXER_INSERT "\
PLACE..........the test disc, Part Number %1$s, in its caddy.\n\
INSERT.........caddy into the CD-ROM Drive.\n\n\
When finished, press Enter."

REMOVE_THE_DISK "\
REMOVE.........the test disc and caddy from the CD-ROM Drive.\n\n\
When finished, press Enter."

MSG_UNKNOWN_DEVICE "%1$06.6lx \
ADDITIONAL INFORMATION FOR CD-ROM DRIVE %2$s IN LOCATION %3$s\n\n\
This CD-ROM is recognized by the system as an\n\
Other SCSI CD-ROM device.\n\
For this reason, the SRN may be invalid."
$
$ %1$l = Menu Goal screen number (not translated).
$ %2$s = the device name (i.e. rmt0).
$ %3$s = the device location (i.e. 00-00-00).
$ NOTE TO TRANSLATOR: The space at the end of the first line is
$                     required. Do not remove.
$

MENU_GOAL_2 "%1$06.61x \
TESTING SCSI CD-ROM DRIVE IN LOCATION %2$s %3$s\n\
No Problems were found, however, error log analysis \n\
indicated that the device has a number of media error\n\
entries. It is recommended that the devices cleaning\n\
procedure be performed.\n\
For instructions on cleaning, refer to the Setup and Operator\n\
Guide for this device."
$
$ The above menu is presented when error log analysis has detected
$ a number of media errors.
$ %1$l = Menu Goal screen number (not translated).
$ %1$s is the location code of the device.
$ %2$s can either be ADVANCED_MODE or blanks.
$ Leave line spacing as shown. See general notes on length of title.
$ NOTE TO TRANSLATOR: The space at the end of the first line is
$                     required. Do not remove.
$

MENU_THIRTEEN "\
Do you hear a continuous tone from both \n\
the left and right speakers?\n\n\
Move cursor to selection, then press Enter."
$
$ the user will be given the options "YES_OPTION" and "NO_OPTION".
$ Leave line spacing as shown.  See general notes on expanding ACTION lines.
$

NEW_MEDIA_MSG "\
A possible media error has been detected.\n\
\n\
REMOVE.........the test caddy from the CD-ROM drive.\n\
OBTAIN.........a NEW CD-ROM test disc Part Number %1$s.\n\
PLACE..........the NEW test disc in the caddy.\n\
INSERT.........the caddy into the CD-ROM drive.\n\
\n\
When finished, press Enter.\n\
\n\
Note: When a media error is detected, it is recommended that the\n\
      devices cleaning procedure be performed. For instructions on\n\
      cleaning, refer to the Setup and Operator Guide for this device."
$
$ The above menu is displayed when it is possible that there is an error
$ on the test disc itself, rather than in the CD-ROM Drive.
$ Leave line spacing as shown.  See general notes on expanding ACTION lines.

DCDROM_DEVICE  " "
DCDROM_SCSI_CABLE " "
DCDROM_MSG_ID_111 " "
DCDROM_MSG_ID_112 " "
DCDROM_MSG_ID_113 " "
DCDROM_MSG_ID_121_164 " "
DCDROM_MSG_ID_171 " "
DCDROM_MSG_ID_172 " "
DCDROM_MSG_ID_181 " "
DCDROM_MSG_ID_182 " "
DCDROM_MSG_ID_199 " "
DCDROM_MSG_ID_211 " "
DCDROM_MSG_ID_281 " "
DCDROM_MSG_ID_301 " "
$ Medium refers to the disc used by the CD-ROM.
DCDROM_MSG_ID_302 " "
DCDROM_MSG_ID_183 " "
DCDROM_SOFTWARE  " "
DCDROM_TEST_DISC " "
DCDROM_MSG_ID_150 " "
$

$ >>>>> The following messages replace some of the above messages <<<<<
$ >>>>> The replaced messages have been zero-out to save space <<<<<
msgCdromDevice "CD-ROM Drive"
$ This message replaces 'DCDROM_DEVICE'

msgScsiCable "Cable, SCSI generic"
$ This message replaces 'DCDROM_SCSI_CABLE'

msgDeviceReserveFailed "Unable to reserve device."
$ This message replaces 'DCDROM_MSG_ID_111'

msgConfigFailed "Unable to do configuration."
$ This message replaces 'DCDROM_MSG_ID_112'

msgDeviceDriverOpenFailed "Unable to open device driver."
$ This message replaces 'DCDROM_MSG_ID_113'

msgCdromError "CD-ROM drive indicates an error."
$ This message replaces 'DCDROM_MSG_ID_121_164'

msgAudioError "Error during audio test."
$ This message replaces 'DCDROM_MSG_ID_171', 'DCDROM_MSG_ID_172',
$ 'DCDROM_MSG_ID_181', 'DCDROM_MSG_ID_182'

msgUnknownError " "
$ This message replaces 'DCDROM_MSG_ID_199'

msgResponseError "After a response, an error occured."
$ This message replaces 'DCDROM_MSG_ID_211', 'DCDROM_MSG_ID_281'

msgElaMediumFailed "Error log analysis indicates medium failure."
$ This message replaces 'DCDROM_MSG_ID_301'
$ Medium refers to the disc used by the CD-ROM.

msgElaHardwareFailed "Error log analysis indicates hardware failure."
$ This message replaces 'DCDROM_MSG_ID_302'

msgMediaReadError "Error reading data on media."
$ This message replaces 'DCDROM_MSG_ID_183'

msgSoftware "Software"
$ This message replaces 'DCDROM_SOFTWARE'

msgTestDisc "Test disc"
$ This message replaces 'DCDROM_TEST_DISC'

msgMediaError "A media error was detected."
$ This message replaces 'DCDROM_MSG_ID_150'

$
$ >>>>> The above set of messages replaces some of earlier messages<<<<<

$ >>>>> The following are new messages <<<<<
msgTimeout "A command timeout was detected."
$ This message is 974-151 FRU

msgReserveConflict "A command reservation conflict was detected."
$ This message is 974-152 FRU

msgUnknownErrorFound "Undefined error detected."
$ This message replaces 'DCDROM_MSG_ID_199' and 'msgUnknownError'

MENU_FOUR_TRAY "\
PLACE..........the test disc in the tray.\n\
PUSH...........the tray into the CD-ROM drive.\n\n\
When finished, press Enter."
$
$ This, and the following 3 messages are similar to earlier ones in the file.
$ The differences are that _TRAY has been appended to the message
$ name, references to a caddy have been changed to tray, and INSERT has
$ been changed to PUSH. A tray is a small drawer which can be pulled out
$ from the CD-ROM drive. The disc is placed directly into the tray.
$ Leave line spacing as shown.  See general notes on expanding ACTION lines.

EXER_INSERT_TRAY "\
PLACE..........the test disc, Part Number %1$s, in the tray.\n\
PUSH...........the tray into the CD-ROM drive.\n\n\
When finished, press Enter."

REMOVE_THE_DISK_TRAY "\
REMOVE.........the test disc from the CD-ROM Drive.\n\n\
When finished, press Enter."

NEW_MEDIA_MSG_TRAY "\
A possible media error has been detected.\n\
\n\
REMOVE.........the test disc from the CD-ROM drive.\n\
OBTAIN.........a NEW CD-ROM test disc Part Number %1$s.\n\
PLACE..........the NEW test disc in the tray.\n\
PUSH...........the tray into the CD-ROM drive.\n\
\n\
When finished, press Enter.\n\
\n\
Note: When a media error is detected, it is recommended that the\n\
      devices cleaning procedure be performed. For instructions on\n\
      cleaning, refer to the Setup and Operator Guide for this device."
$
$ The above menu is displayed when it is possible that there is an error
$ on the test disc itself, rather than in the CD-ROM Drive.
$ This message is similar to NEW_MEDIA_MSG but the word, caddy, has been 
$ replaced with tray.
$ Leave line spacing as shown.  See general notes on expanding ACTION lines.


MENU_GOAL_3 "%1$06.6lx \
TESTING SCSI CD-ROM DRIVE IN LOCATION %2$s %3$s\n\
Error log analysis indicates that the device has a\n\
number of media error entries. These errors may be due\n\
to a scratched or dirty CD-ROM disc (customer media) or\n\
because the CD-ROM drive needs to be cleaned.\n\
For instructions on cleaning, refer to the Setup and Operator\n\
Guide for this device."
$
$ The above menu is presented when error log analysis has detected
$ a number of media errors.
$ %1$l = Menu Goal screen number (not translated).
$ %1$s is the location code of the device.
$ %2$s can either be ADVANCED_MODE or blanks.
$ Leave line spacing as shown. See general notes on length of title.
$ NOTE TO TRANSLATOR: The space at the end of the first line is
$                     required. Do not remove.
$
