$ @(#)06	1.6.2.4  src/bos/diag/util/ucfgvpd/ucfgvpd.msg, dsauchgvpd, bos411, 9428A410j 2/4/94 16:10:13
$
$   COMPONENT_NAME: DSAUCHGVPD
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


$set MSET
$quote "

MUCFGVPD "To continue, press Enter."

MTITLE "\
DISPLAY OR CHANGE CONFIGURATION OR VITAL PRODUCT DATA SERVICE AID"
$
$ See general notes on length of title.

MOPT1 "Display Configuration"
MOPT2 "Display Vital Product Data (VPD)"
MOPT3 "Display/Alter VPD"
MOPT4 "Change Configuration"
MLASTLINE "Move cursor to selection, then press Enter."
MOPT5 "Display Software Product Data"

$set MSET_DSP_VPD
$quote "

DSPTITLE "DISPLAY/ALTER VPD"
$
$ See general notes on length of title.

DSPLASTLINE "\
To see additional items in the list, use the Up Arrow and Down Arrow keys.\n\
Select the option to display or alter the VPD."
$
$ "Up Arrow" and "Down Arrow" should be translated
$ to reflect the up and down cursor keys on IBM keyboards.
$ VPD stands for Vital Product Data.

DSPALTER "ALTER VPD\n\n\%1$s"
$
$ Leave line spacing as shown.  See general notes on length of title.

DSPALTLAST "Type or select the values in the entry field or fields.\n\
Items with data in brackets [] may be altered.\n"

$set MSET6
$quote "

M6TITLE "ADD OR DELETE RESOURCE TO OR FROM CONFIGURATION"
$
$ See general notes on length of title.

M6OPTACP "Add CPU Drawer"
$
$ CPU stands for Central Processing Unit.  DO NOT TRANSLATE unless an
$ equivalent term exists in the translating language.

M6OPTASD "Add SCSI Device Drawer"
$
$ SCSI stands for Small Computer Systems Interface.  DO NOT TRANSLATE unless an
$ equivalent term exists in the translating language.

M6OPTASA "Add SCSI Disk Drawer"
$
$ SCSI stands for Small Computer Systems Interface.  DO NOT TRANSLATE unless an
$ equivalent term exists in the translating language.

M6OPTAAD "Add ASYNC Expansion Drawer"
$
$ ASYNC stands for Asyncronous communications.  DO NOT TRANSLATE unless an
$ equivalent term exists in the translating language.

M6OPTDD  "Delete Drawer"
M6LASTLINE "Select the resource to be added or deleted."

M61BTITLE "ADD DRAWER VPD"
$
$ VPD stands for Vital Product Data.
$ See general notes on length of title.

M61CPROMPT  "Enter the slot number for the drawer adapter...."

M61DEFAULT " "
$
$ This is a blank message. Needs to stay.

M63TITLE "DELETE DRAWER"
$
$ See general notes on length of title.

M63LASTLINE "Select the drawer to be deleted."

DCHILDTITLE "DELETE DRAWER\n\n\
The following devices are attached to the drawer:\n\
%1$s\
Do you wish to delete all of them?"
$
$ %1$s is a listing of all devices that are attached to the drawer.
$ Leave line spacing as shown.  See general notes on length of title.

MCHILDYES  "YES"
MCHILDNO   "NO"
MCHILDLAST "Move cursor to selection, then press Enter."

M61CBUS  "Enter the bus id for the drawer adapter...."

$set SETVPD
$ All of these lines should be the same length and be 28 characters long.
$ fill out the remaining spaces with periods.

AD_MSG "Addressing Field............"
AT_MSG "Adapter Type................"
CD_MSG "Adapter Card ID............."
$ ID stands for identification.
DC_MSG "Date Code..................."
DD_MSG "Device Driver Level........."
DG_MSG "Diagnostic Level............"
DL_MSG "Drawer Level................"
DS_MSG "Displayable Message........."
DU_MSG "Drawer Unit................."
EC_MSG "EC Level...................."
$ EC stands for Engineering Change.
FC_MSG "Feature Code/Marketing ID..."
$ ID stands for identifier. 
FN_MSG "FRU Number.................."
$ FRU stands for Field Replaceable Unit.
LA_MSG "Loadable Microcode Ptr......"
$ Ptr stands for Pointer.
LL_MSG "Loadable Microcode Level...."
LO_MSG "Location...................."
MF_MSG "Manufacturer................"
NA_MSG "Network Address............."
NX_MSG "Next Adapter VPD Ptr........"
$ VPD stands for Vital Product Data.  Ptr stands for Pointer
PC_MSG "Processor Component ID......"
$ ID stands for identification.
PI_MSG "Processor Identification...."
PN_MSG "Part Number................."
RA_MSG "ROS Code on Adapter Ptr....."
$ ROS stands for Read Only Storage.  Ptr stands for Pointer.
RL_MSG "ROS Level and ID............"
$ ROS stands for Read Only Storage.  ID stands for identification.
RN_MSG "System Unit Name............"
RW_MSG "Read/Write Register Ptr....."
$ Ptr stands for Pointer.
SL_MSG "Slot Location..............."
SN_MSG "Serial Number..............."
SZ_MSG "Size........................"
TM_MSG "Machine Type and Model......"
US_MSG "User Data..................."
VE_MSG "VPD Extended Data Ptr......."
$ VPD stands for Vital Product Data.  Ptr stands for Pointer
Z0_MSG "Device Specific.(%c%c)........"
$ The %c is a character substitution.  This message takes up one extra
$ character due to the substitution.  Fill this message out 29 spaces to
$ make sure that it matches the previous lines.

$set ESET
ERR_MKDEV "Cannot run the mkdev command to create the drawer %s."
$
$ %s is the name of the drawer not being created.

$set MSET_DSP_LPP
$quote "

LPPTITLE "DISPLAY SOFTWARE PRODUCT DATA"
$
$ See general notes on length of title.

LPPLASTLINE "\
To see additional items in the list,\n\
use the Up Arrow and Down Arrow keys."
$
$ "Up Arrow" and "Down Arrow" should be translated
$ to reflect the up and down cursor keys on IBM keyboards.
