$ @(#)94        1.1.3.1  src/bos/diag/util/umblist/umblist.msg, dsaumblist, bos412, 9447B 11/18/94 13:41:34
$
$   COMPONENT_NAME: DSAUMBLIST
$
$   FUNCTIONS: Diagnostic message file.
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
$       This file contains the corresponding messages for the menus and
$       errors generated from the UMBLIST Utility.
$
$ END OF GENERAL NOTES

$quote "

$ /**********************************************************************/
$ /* This set used in main()                                            */
$ /**********************************************************************/
$set MODE_SET

MODE_TITLE "DISPLAY/ALTER BOOTLIST"
$
$ See GENERAL NOTES on titles

MODE_OPT1 "\
Normal mode bootlist\n\
  This selection allows displaying, altering, or erasing\n\
  the normal mode bootlist."

MODE_OPT2 "\
Service mode bootlist\n\
  This selection allows displaying, altering, or erasing\n\
  the service mode bootlist."

MODE_OPT3 "\
Previous boot device\n\
  This selection allows displaying or erasing\n\
  the previous boot device."

MODE_LAST "Move cursor to selection, then press Enter."

$ /**********************************************************************/
$ /* This set used in action()                                          */
$ /* ACTION_TITLE_N, ACTION_TITLE_S, and ACTION_TITLE_P are all titles  */
$ /**********************************************************************/
$set ACTION_SET

ACTION_TITLE_N "NORMAL MODE BOOTLIST"
$
$ See GENERAL NOTES on titles

ACTION_TITLE_S "SERVICE MODE BOOTLIST"
$
$ See GENERAL NOTES on titles

ACTION_TITLE_P "PREVIOUS BOOT DEVICE"
$
$ See GENERAL NOTES on titles

ACTION_OPT1 "Display Current bootlist"
ACTION_OPT2 "Alter Current bootlist"
ACTION_OPT3 "Erase Current bootlist"

ACTION_LAST "Move cursor to selection, then press Enter."

ACTION_OPT4 "Display Previous bootlist"
ACTION_OPT5 "Erase Previous bootlist"

$ /**********************************************************************/
$ /* This set used in display_IPList().                                 */
$ /* DISP_TITLE_N, DISP_TITLE_S, and DISP_TITLE_P are all title lines   */
$ /**********************************************************************/
$set DISP_SET

DISP_TITLE_N "\
CURRENT BOOTLIST DEVICES FOR NORMAL MODE\n\n\
The following devices are listed in the order they appear\n\
in the normal mode bootlist."
$
$ See GENERAL NOTES on titles

DISP_TITLE_S "\
CURRENT BOOTLIST DEVICES FOR SERVICE MODE\n\n\
The following devices are listed in the order they appear\n\
in the service mode bootlist."
$
$ See GENERAL NOTES on titles

DISP_TITLE_P "\
PREVIOUS BOOT DEVICE\n\n\
The following device was the last device booted from."
$
$ See GENERAL NOTES on titles

DISP_OPT1       "To continue, press Enter."

DISP_EMPTY_N    "The normal mode bootlist is empty."
DISP_EMPTY_S    "The service mode bootlist is empty."
DISP_EMPTY_P    "There is no previous boot device listed."

$ /**********************************************************************/
$ /* set GEN_SET is used for identifying generic devices.               */
$ /* set GEN_SET has no title line.                                     */
$ /**********************************************************************/
$set GEN_SET

GEN_FD          "Generic Diskette Drive"
GEN_SCDISK      "Generic SCSI Disk"
GEN_BADISK      "Generic Bus-Attached Disk"
GEN_CDROM       "Generic CD-ROM Drive"
GEN_RMT         "Generic Magnetic Tape Drive"
GEN_TOK         "Generic Token-Ring Adapter"
GEN_ENT         "Generic Ethernet Adapter"
GEN_FDDI        "Generic FDDI Adapter"

$ /**********************************************************************/
$ /* This set used in alter_IPList().                                   */
$ /* ALT_TITLE_N and ALT_TITLE_S are both title lines.                  */
$ /**********************************************************************/
$set ALT_SET

ALT_TITLE_N "ALTER NORMAL MODE BOOTLIST DEVICES"
$
$ See GENERAL NOTES on titles

ALT_TITLE_S "ALTER SERVICE MODE BOOTLIST DEVICES"
$
$ See GENERAL NOTES on titles

ALT_OPT1 "\
Specify the access order of the boot devices.\n\
Indicate by placing the appropriate integer in the\n\
bracketed [] field.  (1 is first, 2 is second, etc.)"

$ /**********************************************************************/
$ /* This set used in erase_IPListi() for normal mode.                  */
$ /* ERASE_TITLE_N is a title line.                                     */
$ /**********************************************************************/
$set ERASE_SET_N

ERASE_TITLE_N "ERASE NORMAL MODE BOOTLIST"
$
$ See GENERAL NOTES on titles

ERASE_NO_N  "\
REJECT\n\
  Do NOT erase normal mode bootlist"

ERASE_YES_N "\
ACCEPT\n\
  This action will erase the normal mode bootlist.\n\
  While the normal mode bootlist is empty, subsequent\n\
  IPL in normal mode will default to the first\n\
  available bootable device."
$
$ IPL stands for Initial Program Load and does not have to be translated

ERASE_ACTION_N "Move cursor to selection, then press Enter."

ERASE_OPT_N "The normal mode bootlist has been erased."

$ /**********************************************************************/
$ /* This set used in erase_IPListi() for service mode.                 */
$ /* ERASE_TITLE_S is a title line.                                     */
$ /**********************************************************************/
$set ERASE_SET_S

ERASE_TITLE_S "ERASE SERVICE MODE BOOTLIST"
$
$ See GENERAL NOTES on titles

ERASE_NO_S  "\
REJECT\n\
  Do NOT erase service mode bootlist"

ERASE_YES_S "\
ACCEPT\n\
  This action will erase the service mode bootlist.\n\
  While the service mode bootlist is empty, subsequent\n\
  IPL in service mode will default to the native\n\
  floppy disk drive."
$
$ IPL stands for Initial Program Load
$ IPL and IPList do not have to be translated

ERASE_ACTION_S "Move cursor to selection, then press Enter."

ERASE_OPT_S "The service mode bootlist has been erased."

$ /**********************************************************************/
$ /* This set used in erase_IPListi() for the prevboot IPList.          */
$ /* ERASE_TITLE_P is a title line.                                     */
$ /**********************************************************************/
$set ERASE_SET_P

ERASE_TITLE_P "ERASE PREVIOUS BOOT DEVICE"
$
$ See GENERAL NOTES on titles

ERASE_NO_P  "\
REJECT\n\
  Do NOT erase previous boot device."

ERASE_YES_P "\
ACCEPT\n\
  This action will erase the previous boot device.\n\
  Subsequent IPL will re-write this list with\n\
  the device used for IPL."
$
$ IPL stands for Initial Program Load and does not have to be translated

ERASE_ACTION_P "Move cursor to selection, then press Enter."

ERASE_OPT_P "The previous boot device has been erased."

$ /**********************************************************************/
$ /* This set used for displaying errors in the umblist utility.        */
$ /* set ERROR_SET has no title line.                                   */
$ /**********************************************************************/
$set ERROR_SET

OPEN_ERR        "Error opening file %s."
IOCTL_ERR       "Error reading file %s."
IPLIST_ERR      "Bootlist is improperly initialized."
NO_DEVICE       "Unavailable Device"

ODM_ACCESS_ERR  "Error retrieving ODM data from %s class."
$
$ ODM stands for Object Data Manager and does not have to be translated

BOOTLIST_ERR    "\
***  WARNING  ***\n\n\
Bootlist was NOT altered.\n\
Error upon return from /bin/bootlist.\n\
Error message received = \n%s"

CRITERIA_ERR    "\
Error retrieving ODM data.\n\
Number of devices retrieved %s."
$
$ ODM stands for Object Data Manager and does not have to be translated

UNAVAILABLE     "\
***  WARNING  ***\n\n\
One or more devices in the current bootlist are\n\
not available as IPL devices.  They may not be\n\
used when modifying the bootlist.  Either ERASE\n\
the bootlist first, or do not use these devices\n\
in the new list."
$
$ IPL stands for Initial Program Load and does not have to be translated

NOT_FOUND_ERR   "\
Device was not found in ODM.\n\
Device bus/slot = %s."
$
$ ODM stands for Object Data Manager and does not have to be translated
