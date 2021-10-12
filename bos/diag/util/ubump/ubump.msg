$ @(#)48 1.3 src/bos/diag/util/ubump/ubump.msg, dsaubump, bos411, 9428A410j 5/19/94 05:28:40
$ COMPONENT_NAME: DSAUBUMP
$ 
$ FUNCTIONS: Message set for UBUMP Service Aid.
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
$ errors generated from the UBUMP Utility.
$
$ END OF GENERAL NOTES

$set UBUMP
$quote "

$ Must be the first message in the first set of this file.
$ Was formerly in dutil.msg.
UBUMP_MOPT1 "BUMP Service Aids\n\
  This selection allows various functions to be performed on\n\
  the BUMP."

UBUMPTITLE "\
BUMP SERVICE AIDS"
$
$ See general notes on length of title.

UBUMPOPT1 "Display or Change Remote Support Phone Numbers \
or Modem Configuration"
$ message no longer in use
UBUMPOPT2 "Display or Change Diagnostic Modes"
$ message no longer in use
UBUMPOPT3 "Save or Restore Diagnostic Modes and Remote Support Phone Numbers"
$ message no longer in use
UBUMPOPT4 "Flash Eprom Download"
UBUMPLASTLINE "Move cursor to selection, then press Enter."

$ New messages
UBUMPOPT10 "Display or Change Flags and Configuration"
UBUMPOPT13 "Save or Restore Flags and Configuration"

$set UBUMP_DSPCHG_PHNB
$quote "
$ set no longer in use

DSPCHGPHNBTITLE "\
DISPLAY OR CHANGE REMOTE SUPPORT PHONE NUMBERS OR MODEM CONFIGURATION"
$ message no longer in use
$
$ See general notes on length of title.

DSPCHGPHNBOPT1 "Display Remote Support Phone Numbers"
$ message no longer in use
DSPCHGPHNBOPT2 "Display Modem Configuration"
$ message no longer in use
DSPCHGPHNBOPT3 "Change Remote Support Phone Numbers"
$ message no longer in use
DSPCHGPHNBOPT4 "Change Modem Configuration"
$ message no longer in use

$set UBUMP_DSPCHG_FLAG
$quote "
$ set no longer in use

DSPCHGFLAGTITLE "\
DISPLAY OR CHANGE DIAGNOSTIC MODES"
$ message no longer in use
$
$ See general notes on length of title.

DSPCHGFLAGOPT1 "Display Diagnostic Modes"
$ message no longer in use
DSPCHGFLAGOPT2 "Change Diagnostic Modes"
$ message no longer in use

$set UBUMP_DISPLAY
$quote "

DSPPHNBTITLE "\
DISPLAY REMOTE SUPPORT PHONE NUMBERS"
$
$ See general notes on length of title.

DSPMODEMTITLE "\
DISPLAY MODEM AND SITE CONFIGURATION"
$ DISPLAY MODEM CONFIGURATION"
$
$ See general notes on length of title.

DSPFLAGTITLE "\
DISPLAY DIAGNOSTIC FLAGS"
$ DISPLAY DIAGNOSTIC MODES"
$
$ See general notes on length of title.

DSPSERVTITLE "\
DISPLAY SERVICE SUPPORT MODES"
$
$ See general notes on length of title.

DSPLASTLINE "To continue, press Enter."

$set UBUMP_CHANGE
$quote "

CHGPHNBTITLE "\
CHANGE REMOTE SUPPORT PHONE NUMBERS"
$
$ See general notes on length of title.

CHGMODEMTITLE "\
CHANGE MODEM AND SITE CONFIGURATION"
$ CHANGE MODEM CONFIGURATION"
$
$ See general notes on length of title.

CHGFLAGTITLE "\
CHANGE DIAGNOSTIC FLAGS"
$ CHANGE DIAGNOSTIC MODES"
$
$ See general notes on length of title.

CHGSERVTITLE "\
CHANGE SERVICE SUPPORT MODES"
$
$ See general notes on length of title.


CHGLASTLINE "Type or select values in entry fields.\n\
Use cursor key to move between fields.\n\
Press F7 AFTER making all desired changes."

$ New messages

CHGFLAGLASTLINE "Use tab key to sequence through valid selections (1 for Yes, 0 for No).\n\
Use cursor key to move between fields.\n\
Press F7 AFTER making all desired changes."

$set UBUMP_SAV_REST
$quote "

SAVRESTTITLE "\
SAVE OR RESTORE DIAGNOSTIC MODES AND REMOTE SUPPORT PHONE NUMBERS"
$ message no longer in use
$
$ See general notes on length of title.

SAVRESTOPT1 "Save Diagnostic Modes and Remote Support Phone Numbers"
$ message no longer in use
SAVRESTOPT2 "Restore Diagnostic Modes and Remote Support Phone Numbers"
$ message no longer in use

$ New messages
SAVERESTTITLE "\
SAVE OR RESTORE FLAGS AND CONFIGURATION"
$
$ See general notes on length of title.
SAVERESTOPT1 "Save Flags and Configuration"
SAVERESTOPT2 "Restore Flags and Configuration"

$set UBUMP_FEPROM
$quote "

FEPROMTITLE "UPDATE FEPROM"
$
$ See general notes on length of title.

FEPROMPROMPT "Enter the full file name"

FEPROMLASTLINE "Type or select value in the entry field.\n"

$set UBUMP_ERR
$quote "

CMDERR "COMMAND FAILED:\n\n\%s"
$
$ %s is the command error returned.

CMDOK "COMMAND SUCCESSFULL:\n\n%s"
$
$ %s is the command output  returned.

MISSING "FILE %s does not exist.\n"
$
$ %s is a file name

MODERR "This service aid can only be run in SERVICE mode.\n"

MEM_ERR "** FATAL ERROR --> out of memory **"

UNKNOWN "unknown reason"

$ New messages
PASSWDERR "You entered an invalid password.\n"

PASSWDNOTMATCH "The password entry does not match,\nplease try again.\n"
$ Do not translate \n

$set UBUMP_ASK
$quote "

ASKTITLE "UPDATE FEPROM\n\n\
* * * WARNING! * * *\n\n\
THIS OPERATION WILL REBOOT THE MACHINE!\n\n\
Do you want to update the feprom?"

ASKYES "YES"
$ This option is shown when a YES answer is possible.

ASKNO "NO"
$ This option is shown when a NO answer is possible.

ASKLASTLINE "Move cursor to selection, then press Enter."

$ New messages
$set UBUMP_DSPCHG_FLAG_CFG
$ this set replace set UBUMP_DSPCHG_FLAG
$quote "

DSPCHGFLAGCFGTITLE "\
DISPLAY OR CHANGE FLAGS AND CONFIGURATION"
$
$ See general notes on length of title.

DSPCHGFLAGCFGOPT1 "Display Diagnostic Flags"
DSPCHGFLAGCFGOPT2 "Display Service Support Modes"
DSPCHGFLAGCFGOPT3 "Display Modem and Site Configuration"
DSPCHGFLAGCFGOPT4 "Display Remote Support Phone Numbers"
DSPCHGFLAGCFGOPT5 "Change Diagnostic Flags"
DSPCHGFLAGCFGOPT6 "Change Service Support Modes"
DSPCHGFLAGCFGOPT7 "Change Modem and Site Configuration"
DSPCHGFLAGCFGOPT8 "Change Remote Support Phone Numbers"
DSPCHGFLAGCFGOPT9 "Change Maintenance Menu Passwords"

$set UBUMP_PASSWD
$quote "

ASKPASSWDTITLE "\
ROLLING PASSWORD"
$
$ See general notes on length of title.

ASKROLLING "Rolling Password"
ASKCUSTOMER "Customer Maintenance Menu Password"
ASKGENERAL "Maintenance Menu Password"

ASKPASSWDLASTLINE "Enter password in the entry field.\n\
Press F7 AFTER making all desired changes\n."

CHGPASSWDTITLE "\
CHANGE MAINTENANCE MENU PASSWORDS"
$
$ See general notes on length of title.

CHGPASSWDTITLE1 "\
CHANGE CUSTOMER MAINTENANCE MENU PASSWORD"
$
$ See general notes on length of title.

CHGPASSWDTITLE2 "\
CHANGE MAINTENANCE MENU PASSWORD"
$
$ See general notes on length of title.

CHGPASSWDOPT1 "Change Customer Maintenance Menu Password"
CHGPASSWDOPT2 "Change Maintenance Menu Password"

PASSWDOPT0 "Old Password"
PASSWDOPT1 "Customer Maintenance Menu Password"
PASSWDOPT2 "New Password"
PASSWDOPT3 "Enter the new password again"

