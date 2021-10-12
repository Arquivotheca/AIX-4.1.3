$ @(#)23	1.5.1.4  src/bos/diag/lscfg/lscfg.msg, lscfg, bos411, 9428A410j 1/26/94 15:07:49
$
$   COMPONENT_NAME: LSCFG
$
$   FUNCTIONS: Diagnostic message file.
$
$   ORIGINS: 27
$
$
$   (C) COPYRIGHT International Business Machines Corp. 1989,1993
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
$   DO NOT TRANSLATE any of the echnical terms used in this message file
$   unless an equivalent term exeists in the translating language.  Each
$   term is described in the message comments.
$
$ VPD MESSAGE FILE
$       This file contains the corresponding messages for the menus and
$	errors generated from the lscfg command.
$
$ END OF GENERAL NOTES


$set SET0
$quote "
$ All of these lines should be the same length and be 28 characters long.
$ Fill out the remaining spaces with periods.

AD_MSG "Addressing Field............"
AT_MSG "Adapter Type................"
CD_MSG "Adapter Card ID............."
$ ID stands for indentification.
DC_MSG "Date Code..................."
DD_MSG "Device Driver Level........."
DG_MSG "Diagnostic Level............"
DL_MSG "Drawer Level................"
DS_MSG "Displayable Message........."
DU_MSG "Drawer Unit................."
EC_MSG "EC Level...................."
$ EC stands for Engineering Change.
FC_MSG "Feature Code/Marketing ID..."
FN_MSG "FRU Number.................."
$ FRU stands for Field Replaceable Unit.
LA_MSG "Loadable Microcode Ptr......"
$ Ptr stands for Pointer.
LL_MSG "Loadable Microcode Level...."
LO_MSG "Location...................."
MF_MSG "Manufacturer................"
NA_MSG "Network Address............."
NX_MSG "Next Adapter VPD Ptr........"
$ VPD stands for Vital Product Data.  Ptr stands for Pointer.
PC_MSG "Processor Component ID......"
$ ID stands for Identification.
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
$ VPD stands for Vital Product Data.  Ptr stands for Pointer.
Z0_MSG "Device Specific.(%c%c)........"
$ The %c is a character substitution.  This message takes up one extra 
$ character due to the substitution.  Fill this message out 29 spaces to
$ make sure that it matches previous lines.

$set INST_SET
$quote "

INST_TITLE "INSTALLED RESOURCE LIST\n\n\
The following resources are installed on the machine.\n\
+/- = Added or deleted from Diagnostic Test List.\n\
*   = Diagnostic support not available."
$
$ Leave line spacing as shown.  See general notes on length of title.

$set VPD_SET
$quote "	Set quote character to double quotes

VPD_TITLE "INSTALLED RESOURCE LIST WITH VPD\n\n\
The following resources are installed on your machine."
$
$ VPD stands for Vital Product Data.
$ Leave line spacing as shown.  See general notes on length of title.

VPD_DEVICE   "DEVICE"
$ This message can only be 16 characters long.  DO NOT exceed this 16
$ character limit.

VPD_LOCATION "LOCATION"
$ This message can only be 16 characters long.  DO NOT exceed this 16
$ character limit.

VPD_DESC     "DESCRIPTION"
$ This message can only be 40 characters long.  DO NOT exceed this 40
$ character limit.


$set MS_LSCFG
$quote "	Set quote character to double quotes

USAGE1 "usage: lscfg [-l logical device ] [-v]\n"
LFLAG  "lscfg: devicename must be specified with 'l' option\n"
NODEV  "lscfg: device %s not found.\n"
