$ @(#)64        1.7.4.2  src/bos/diag/da/morps/morps.msg, damorps, bos41J, 9517B_all 4/25/95 17:38:35
$
$   COMPONENT_NAME: DAMORPS
$
$   FUNCTIONS: message
$		
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
$ Do not translate %d, %s, or \t in any messages.  They
$   are used for word or number substitution and noted in the
$   comments for the individual messages.  The 1$, 2$, 3$, etc,
$   within the substitutions are used to denote the order of the
$   substitutions.
$
$ These comments concern the user ACTIONS in all capital letters.
$   If translations require the creation of new lines, begin the new lines
$   in the column immediately following the row of periods.
$   For example:
$
$ *****
$ ACTION.........the one line of translation might need more than one line, so
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
$ Additional notes:
$
$ Missing resources can have the following values:
$   fdx, scdiskx, cdx, or rmtx where x=0-9.
$ These values are not translatable.
$
$ The location of a resource is in the form of xx-xx-xx where x is an
$   alpha-numeric character.  The location is not translatable.  It is
$   an alpha-numeric descriptor of where the resource can be found.
$
$ All On/Off switches on equipment made in the USA have indicators on the
$ machine next to the switch that indicate the position:
$
$ (O) is the symbol for the Off position of a switch in USA.  This may need
$   to be changed in the translating country if different.
$ (I) is the symbol for the On position of a switch in USA.  This may need
$   to be changed in the translating country if different.
$
$ END OF GENERAL NOTES


$quote "
$set MORPS
$

Yes_option "YES"
$ This option is shown when a YES answer is possible.

No_option "NO"
$ This option is shown when a NO answer is possible.

Choose "Move cursor to selection, then press Enter."
$ This message is shown when a multiple selection list is presented.

LOCATION "Missing resource resolution for %1$s in location %2$s.\n\n\
Is the missing device, %3$s, an internally connected device?\n\n\
NOTE: Devices located inside drawers or towers are considered to be\n\
      internally connected."
$
$ %1$s is the resource that is missing.
$ %2$s is the location of the resource.
$ %3$s will be the same as %1$s.
$ Leave line spacing between sentences as shown.

WHERE_R_YOU "Missing resource resolution for %1$s in location %2$s.\n\n\
Select the enclosure type where %3$s is located.\n"
$
$ %1$s is the resource that is missing.
$ %2$s is the location of the resource.
$ %3$s will be the same as %1$s.
$ Leave line spacing between sentences as shown.

WHERERYOU_2 "Missing resource resolution for %1$s in location %2$s.\n\n\
Is the missing device, %3$s, located in a SCSI Device Drawer or a SCSI\n\
Disk Drawer?\n\n"
$
$ SCSI is a technical term.  See general notes.
$ A SCSI Drawer is a drawer that installs in a rack mounted system.
$ %1$s is the resource that is missing.
$ %2$s is the location of the resource.
$ %3$s will be the same as %1$s.
$ Leave line spacing between sentences as shown.

WHERE_R_YOU1 "Missing resource resolution for %1$s in location %2$s.\n\n\
Is the missing device, %3$s, a Portable Disk Drive?\n\n"
$
$ Portable Disk Drive is an externally connected drive. The drive
$ is removable from the external enclosure.
$ %1$s is the resource that is missing.
$ %2$s is the location of the resource.
$ %3$s will be the same as %1$s.
$ Leave line spacing between sentences as shown.

DASD_DRAWER "SCSI Disk Drawer"
$
$ SCSI is a technical term.  See general notes.
$ A SCSI Disk Drawer is an optional feature of the rack mounted system.
$ Only DASD devices are installed in a SCSI Disk Drawer.  DASD stands for
$ direct access storage device and includes fixed disks and diskette drives.

MEDIA_DRAWER "SCSI Device Drawer"
$
$ SCSI is a technical term.  See set notes.
$ The SCSI Device Drawer is a standard feature of the rack mount system.
$ DASD or other SCSI devices can be installed in a SCSI Device Drawer.

SYS_UNIT "System Unit or CPU Drawer"
$
$ The system unit or CPU drawer is the part of the machine that contains the
$ system processor.  CPU is the central processing unit.

BRIDGE_MENU_1 "Missing resource resolution for %1$s in location %2$s.\n\n\
OBSERVE........the power on light of the device.\n\n\
Is the light on?\n"
$
$ The device power on light is the visual indicator that the device is on.
$ %1$s is the resource that is missing.
$ %2$s is the location of the resource.
$ Leave line spacing between sentences as shown.  See general notes on ACTIONS.

BRIDGE_MENU_2 "Missing resource resolution for %1$s in location %2$s.\n\n\
ENSURE.........that the power switch of the device is in the On (I)\n\
               position.\n\n\
Is the power switch in the On position?\n"
$
$ Machine markings: (I) indicates On, (O) indicates Off.  See general notes.
$ %1$s is the resource that is missing.
$ %2$s is the location of the resource.
$ Leave line spacing between sentences as shown.  See general notes on ACTIONS.

BRIDGE_MENU_3 "Missing resource resolution for %1$s in location %2$s.\n\n\
ENSURE.........that the power cord of the device is properly\n\
               plugged into:\n\
               1. The device.\n\
               2. A functioning power source.\n\n\
Is the power cord properly plugged into both places?\n"
$
$ %1$s is the resource that is missing.
$ %2$s is the location of the resource.
$ Leave line spacing between sentences as shown.  See general notes on ACTIONS.

BRIDGE_MENU_4 "Missing resource resolution for %1$s in location %2$s.\n\n\
ENSURE.........that the SCSI address of the device, %3$s, is\n\
               set correctly.\n\n\
Is the SCSI address set correctly?\n"
$
$ SCSI is a technical term.  See general notes.
$ %1$s is the resource that is missing.
$ %2$s is the location of the resource.
$ %3$s will be the same as %1$s.
$ Leave line spacing between sentences as shown.  See general notes on ACTIONS.

BRIDGE_MENU_5 "Missing resource resolution for %1$s in location %2$s.\n\n\
ENSURE.........that the signal cable is properly connected.\n\n\
Is the signal cable properly connected?\n"
$
$ %1$s is the resource that is missing.
$ %2$s is the location of the resource.
$ Leave line spacing between sentences as shown.  See general notes on ACTIONS.

BRIDGE_MENU_6 "Missing resource resolution for %1$s in location %2$s.\n\n\
ENSURE.........that the the power switch of the external diskette\n\
               drive is in the On (I) position.\n\n\
Is the power switch in the On position?\n"
$
$ Machine markings: (I) indicates On, (O) indicates Off.  See general notes.
$ %1$s is the resource that is missing.
$ %2$s is the location of the resource.
$ Leave line spacing between sentences as shown.  See general notes on ACTIONS.

BRIDGE_MENU_7 "Missing resource resolution for %1$s in location %2$s.\n\n\
ENSURE.........that the power cord of the external diskette drive\n\
               is properly plugged into:\n\
               1. The external diskette drive.\n\
               2. A functioning power receptacle.\n\n\
Is the power cord properly plugged into both places?\n"
$
$ %1$s is the resource that is missing.
$ %2$s is the location of the resource.
$ Leave line spacing between sentences as shown.  See general notes on ACTIONS.

BRIDGE_MENU_8 "Missing resource resolution for %1$s in location %2$s.\n\n\
ENSURE.........that the power switch, located at the rear of the device,\n\
               is in the On (I) position.\n\n\
Is the power switch in the On position?\n"
$
$ Machine markings: (I) indicates On, (O) indicates Off.  See general notes.
$ %1$s is the resource that is missing.
$ %2$s is the location of the resource.
$ Leave line spacing between sentences as shown.  See general notes on ACTIONS.

HARR1_MENU_1  "Missing resource resolution for %1$s in location %2$s.\n\n\
REMOVE.........the front cover (if necessary) of the drawer or tower and\n\
	       observe the lights located behind the front cover.\n\n\
Are all of the lights off?\n"
$
$ %1$s is the resource that is missing.
$ %2$s is the location of the resource.
$ Leave line spacing between sentences as shown.  See general notes on ACTIONS.

HARR1_MENU_2  "Missing resource resolution for %1$s in location %2$s.\n\n\
ENSURE.........that the drawer or tower power switch is in the On (I)\n\
               position.\n\n\
Is the power switch in the On position?\n"
$
$ Machine markings: (I) indicates On, (O) indicates Off.  See general notes.
$ %1$s is the resource that is missing.
$ %2$s is the location of the resource.
$ Leave line spacing between sentences as shown.  See general notes on ACTIONS.

HARR1_MENU_3  "Missing resource resolution for %1$s in location %2$s.\n\n\
OBSERVE........the Fans Good light (L2).\n\n\
Is the light on?\n"
$
$ L2 is a designation embossed on the system.  This should be the same in
$   all countries.
$ %1$s is the resource that is missing.
$ %2$s is the location of the resource.
$ Leave line spacing between sentences as shown.  See general notes on ACTIONS.

HARR1_MENU_4  "Missing resource resolution for %1$s in location %2$s.\n\n\
OBSERVE........the drawer or tower Main Power Present light (L3) located\n\
               at the rear of the drawer or tower .\n\n\
Is the light on?\n"
$
$ L3 is a designation embossed on the system.  This should be the same in
$   all countries.
$ %1$s is the resource that is missing.
$ %2$s is the location of the resource.
$ Leave line spacing between sentences as shown.  See general notes on ACTIONS.

HARR1_MENU_5  "Missing resource resolution for %1$s in location %2$s.\n\n\
OBSERVE........the drawer or tower compartment power light for %3$s.\n\n\
Is the light on?\n"
$
$ %1$s is the resource that is missing.
$ %2$s is the location of the resource.
$ %3$s will be the same as %1$s.
$ Leave line spacing between sentences as shown.  See general notes on ACTIONS.

HARR1_MENU_6  "Missing resource resolution for %1$s in location %2$s.\n\n\
ENSURE.........that the drawer or tower compartment power switch for %3$s\n\
               at location %4$s is in the On (I) position.\n\n\
Is the power switch in the On position?\n"
$
$ Machine markings: (I) indicates On, (O) indicates Off.  See general notes.
$ %1$s is the resource that is missing.
$ %2$s is the location of the resource.
$ %3$s will be the same as %1$s.
$ %4$s will be the same as %2$s.
$ Leave line spacing between sentences as shown.  See general notes on ACTIONS.

HARR1_MENU_7  "Missing resource resolution for %1$s in location %2$s.\n\n\
ENSURE.........that the SCSI address of %3$s is set correctly.\n\n\
Is the SCSI address set correctly?\n"
$
$ SCSI is a technical term.  See general notes.
$ %1$s is the resource that is missing.
$ %2$s is the location of the resource.
$ %3$s will be the same as %1$s.
$ Leave line spacing between sentences as shown.  See general notes on ACTIONS.

PORTA_MENU_1  "Missing resource resolution for %1$s in location %2$s.\n\n\
OBSERVE........the power on light of the Portable Disk Drive enclosure.\n\n\
Is the light on?\n"
$
$ %1$s is the resource that is missing.
$ %2$s is the location of the resource.
$ Leave line spacing between sentences as shown.  See general notes on ACTIONS.

PORTA_MENU_2  "Missing resource resolution for %1$s in location %2$s.\n\n\
ENSURE.........that the power switch of the enclosure is in the\n\
               On (I) position.\n\n\
Is the power switch in the On position?\n"
$
$ Machine markings: (I) indicates On, (O) indicates Off.  See general notes.
$ %1$s is the resource that is missing.
$ %2$s is the location of the resource.
$ Leave line spacing between sentences as shown.  See general notes on ACTIONS.

PORTA_MENU_3  "Missing resource resolution for %1$s in location %2$s.\n\n\
ENSURE.........that the power cord of the Portable Disk Drive enclosure\n\
               is properly plugged into:\n\
               1. The Portable Disk Drive enclosure.\n\
               2. A functioning power receptacle.\n\n\
Is the power cord properly plugged into both places?\n"
$
$ %1$s is the resource that is missing.
$ %2$s is the location of the resource.
$ Leave line spacing between sentences as shown.  See general notes on ACTIONS.

PORTA_MENU_5  "Missing resource resolution for %1$s in location %2$s.\n\n\
ENSURE.........that the SCSI address of %3$s is set correctly.\n\n\
Is the SCSI address set correctly?\n\n"
$
$ SCSI is a technical term.  See general notes.
$ %1$s is the resource that is missing.
$ %2$s is the location of the resource.
$ %3$s will be the same as %1$s.
$ Leave line spacing between sentences as shown.  See general notes on ACTIONS.

PORTA_MENU_4  "Missing resource resolution for %1$s in location %2$s.\n\n\
ENSURE.........that the signal cable is properly connected.\n\n\
Is the signal cable properly connected?\n"
$
$ %1$s is the resource that is missing.
$ %2$s is the location of the resource.
$ Leave line spacing between sentences as shown.  See general notes on ACTIONS.

PORTA_MENU_6  "Missing resource resolution for %1$s in location %2$s.\n\n\
ENSURE.........that a Disk Module is plugged into the Portable Disk Drive\n\
               enclosure.\n\n\
Is a Disk Module plugged into the enclosure?\n"
$
$ Disk Module is the term used to refer to the removable disk drive.
$ %1$s is the resource that is missing.
$ %2$s is the location of the resource.
$ Leave line spacing between sentences as shown.  See general notes on ACTIONS.

HARR2_MENU_C0 "\
Missing resource resolution for %1$s in location %2$s.\n\n\
OPEN...........the Controller front cover flap to expose the\n\
               operator controls.\n\
OBSERVE........the controller reference display.\n\n\
Is the reference display showing a '-' ?\n"
$ %1$s is the resource that is missing.
$ %2$s is the location of the resource.
$ Leave line spacing between sentences as shown.  See general notes on ACTIONS.

HARR2_MENU_C1 "\
Missing resource resolution for %1$s in location %2$s.\n\n\
OBSERVE........the controller power switch.\n\n\
Is the power switch in the On (I) position ?\n"
$ %1$s is the resource that is missing.
$ %2$s is the location of the resource.
$ Leave line spacing between sentences as shown.  See general notes on ACTIONS.

HARR2_MENU_C2 "\
Missing resource resolution for %1$s in location %2$s.\n\n\
OBSERVE........the controller reference display.\n\n\
Is the reference display blank ?\n"
$ %1$s is the resource that is missing.
$ %2$s is the location of the resource.
$ Leave line spacing between sentences as shown.  See general notes on ACTIONS.

HARR2_MENU_C3 "\
Missing resource resolution for %1$s in location %2$s.\n\n\
OBSERVE........the rack power indicator.\n\n\
Is the rack power indicator on ?\n"
$ %1$s is the resource that is missing.
$ %2$s is the location of the resource.
$ Leave line spacing between sentences as shown.  See general notes on ACTIONS.

HARR2_MENU_D0 "\
Missing resource resolution for %1$s in location %2$s.\n\n\
OPEN...........the Controller front cover flap to expose the\n\
               operator controls.\n\
OBSERVE........the DASD power good indicator.\n\n\
Is the DASD power good indicator on ?\n"
$ %1$s is the resource that is missing.
$ %2$s is the location of the resource.
$ Leave line spacing between sentences as shown.  See general notes on ACTIONS.

HARR2_MENU_D1 "\
Missing resource resolution for %1$s in location %2$s.\n\n\
OBSERVE........the DASD power switch.\n\n\
Is the DASD power switch in the On (I) position ?\n"
$ %1$s is the resource that is missing.
$ %2$s is the location of the resource.
$ Leave line spacing between sentences as shown.  See general notes on ACTIONS.

HARR2_MENU_D2 "\
Missing resource resolution for %1$s in location %2$s.\n\n\
OBSERVE........the DASD check indicator.\n\n\
Is the DASD check indicator on ?\n"
$ %1$s is the resource that is missing.
$ %2$s is the location of the resource.
$ Leave line spacing between sentences as shown.  See general notes on ACTIONS.

HARR2_MENU_D3 "\
Missing resource resolution for %1$s in location %2$s.\n\n\
OBSERVE........the DASD ready indicator.\n\n\
Is the DASD ready indicator on, or flickering ?\n"
$ %1$s is the resource that is missing.
$ %2$s is the location of the resource.
$ Leave line spacing between sentences as shown.  See general notes on ACTIONS.

HARR2_MENU_D4 "\
Missing resource resolution for %1$s in location %2$s.\n\n\
OBSERVE........the controller reference display.\n\n\
Is the failing DASD I.D. indicated by a continuous or cycling\n\
number in the reference display ?\n"
$ %1$s is the resource that is missing.
$ %2$s is the location of the resource.
$ Leave line spacing between sentences as shown.  See general notes on ACTIONS.

WHERERYOU_2A "Missing resource resolution for %1$s in location %2$s.\n\n\
Is the missing device, %3$s, located in either a SCSI Device Drawer or\n\
a SCSI Disk Drawer or a Tower? \n\
NOTE: If the device is located in the CPU drawer or CPU enclosure,\n\
      then select NO.\n\n"
$
$ SCSI is a technical term.  See general notes.
$ A SCSI Drawer is a drawer that installs in a rack mounted system.
$ %1$s is the resource that is missing.
$ %2$s is the location of the resource.
$ %3$s will be the same as %1$s.
$ Leave line spacing between sentences as shown.

HARR1_MENU_1A  "Missing resource resolution for %1$s in location %2$s.\n\n\
REMOVE.........the front cover (if necessary) of the drawer or tower.\n\
OBSERVE........all the lights located in the front of the drawer or\n\
	       the tower.\n\n\
Are all of the lights off?\n"
$
$ %1$s is the resource that is missing.
$ %2$s is the location of the resource.
$ Leave line spacing between sentences as shown.  See general notes on ACTIONS.

HARR1_MENU_3A  "Missing resource resolution for %1$s in location %2$s.\n\n\
OBSERVE........the Fans Good light (Denoted by L2 or Fan Symbol).\n\n\
Is the light on?\n"
$
$ L2 is a designation embossed on the system.  This should be the same in
$   all countries.
$ %1$s is the resource that is missing.
$ %2$s is the location of the resource.
$ Leave line spacing between sentences as shown.  See general notes on ACTIONS.

HARR1_MENU_4A "Missing resource resolution for %1$s in location %2$s.\n\n\
OBSERVE........the drawer or tower Main Power Present light (L3) located\n\
               at the rear of the drawer or tower containing the missing\n\
	       device.\n\n\
Is the light on?\n"
$
$ L3 is a designation embossed on the system.  This should be the same in
$   all countries.
$ %1$s is the resource that is missing.
$ %2$s is the location of the resource.
$ Leave line spacing between sentences as shown.  See general notes on ACTIONS.

HARR1_MENU_5A  "Missing resource resolution for %1$s in location %2$s.\n\n\
OBSERVE........The compartment power indicator light for %3$s located in\n\
	       the drawer or in the tower.\n\n\
Is the light on?\n"
$
$ %1$s is the resource that is missing.
$ %2$s is the location of the resource.
$ %3$s will be the same as %1$s.
$ Leave line spacing between sentences as shown.  See general notes on ACTIONS.

HARR2_MENU_C4 "\
Missing resource resolution for %1$s in location %2$s.\n\n\
OBSERVE........the controller reference display.\n\n\
Is the reference display showing a 'H' or 'L'?\n\n"
$ %1$s is the resource that is missing.
$ %2$s is the location of the resource.
$ Leave line spacing between sentences as shown.  See general notes on ACTIONS.

HARR2_MENU_C5 "\
Missing resource resolution for %1$s in location %2$s.\n\n\
OBSERVE........the controller reference display.\n\n\
Is the reference display flashing a 'P' followed by another character?\n\n"
$ %1$s is the resource that is missing.
$ %2$s is the location of the resource.
$ Leave line spacing between sentences as shown.  See general notes on ACTIONS.

HARR2_MENU_C6 "\
Missing resource resolution for %1$s in location %2$s.\n\n\
ENSURE.........that the signal cable is properly connected.\n\n\
Is the signal cable properly connected?\n"
$ %1$s is the resource that is missing.
$ %2$s is the location of the resource.
$ Leave line spacing between sentences as shown.  See general notes on ACTIONS.

$ ------------------------------------------------------------------------------
$ messages for 7013 (Pegasus)

msg7013PowerIndicator "\
Missing resource resolution for %1$s in location %2$s.\n\n\
OBSERVE........the power indicator on the front of the expansion unit.\n\n\
Is the power indicator on?\n"
$ %1$s is the resource that is missing.
$ %2$s is the location of the resource.
$ Leave line spacing between sentences as shown.  See general notes on ACTIONS.

$ ------------------------------------------------------------------------------
$ The following set of messages are for 7134 enclosure (Saracen)

msg7134PowerSwitch "\
Missing resource resolution for %1$s in location %2$s.\n\n\
LOCATE.........the 7134 enclosure containing the missing device, in the rack.\n\
OBSERVE........the drawer and rack power switches\n\n\
Are the power switches in the On (I) position?\n"
$ %1$s is the resource that is missing.
$ %2$s is the location of the resource.
$ Leave line spacing between sentences as shown.  See general notes on ACTIONS.

msg7134PowerIndicator "\
Missing resource resolution for %1$s in location %2$s.\n\n\
OBSERVE........the power indicator on the front of the enclosure.\n\n\
Is the power indicator on?\n"
$ %1$s is the resource that is missing.
$ %2$s is the location of the resource.
$ Leave line spacing between sentences as shown.  See general notes on ACTIONS.

msg7134FanPowerSupplyIndicator "\
Missing resource resolution for %1$s in location %2$s.\n\n\
OBSERVE........the Fan-Power supply assembly power indicators in the rear\n\
	       of the 7134 unit (see Operator Guide for help).\n\n\
Are any of the fan indicators on?\n"
$ %1$s is the resource that is missing.
$ %2$s is the location of the resource.
$ Leave line spacing between sentences as shown.  See general notes on ACTIONS.

msg7134PowerCord "\
Missing resource resolution for %1$s in location %2$s.\n\n\
OBSERVE........the power cord connected between the rack and the enclosure.\n\n\
Is the power cord properly plugged in?\n"
$ %1$s is the resource that is missing.
$ %2$s is the location of the resource.
$ Leave line spacing between sentences as shown.  See general notes on ACTIONS.

msg7134FanIndicator "\
Missing resource resolution for %1$s in location %2$s.\n\n\
OBSERVE........the fan indicator in the center unit (DC/Fan assembly).\n\n\
Is the fan indicator on?\n"
$ %1$s is the resource that is missing.
$ %2$s is the location of the resource.
$ Leave line spacing between sentences as shown.  See general notes on ACTIONS.

msg7134CheckIndicator "\
Missing resource resolution for %1$s in location %2$s.\n\n\
OBSERVE........the check indicator in the AC/Fan assembly unit located on\n\
	       the right hand (or located on the left hand, if installed).\n\n\
Is the check indicator on?\n"
$ %1$s is the resource that is missing.
$ %2$s is the location of the resource.
$ Leave line spacing between sentences as shown.  See general notes on ACTIONS.

$ ------------------------------------------------------------------------------
$ messages for the main menus

msgEnclMenuHead "MISSING RESOURCE RESOLUTION FOR %1$s IN LOCATION %2$s.\n\n\
Select the enclosure type where %3$s is located.\n"
$ This msg replaces WHERE_R_YOU msg
$ %1$s is the resource that is missing.
$ %2$s is the location of the resource.
$ %3$s will be the same as %1$s.
$ Leave line spacing between sentences as shown.

msgEnclCPUEnclosure "System Unit or CPU Drawer"
$ This msg replaces SYS_UNIT

msgEnclBridgeBox "Bridge Box"

msgEnclDevDrawerTower "SCSI Device Drawer or External Tower""
$ This msg replaces DASD_DRAWER, MEDIA_DRAWER

msgEnclDiskDevDrawerTower "SCSI Device Drawer, SCSI Disk Drawer or External Tower""
$ This msg replaces DASD_DRAWER, MEDIA_DRAWER

msgEnclPortable "External Portable Disk Drive"

msgEncl7013 "7013 Model J Series Expansion Unit"

msgEncl7134 "7134 SCSI Disk Drawer"

msgEnclOther "Other"

msgEncl7131 "7131 SCSI Media Tower"

$ ===========================================================================
$set MORPS_M

BRIDGE_MENU_GOL_1 "950700 \
Missing resource resolution for %1$s in location %2$s.\n\n\
TURN...........the power switch of the device to the On (I) position.\n\
RESTART........the system."
$
$ Machine markings: (I) indicates On, (O) indicates Off.  See general notes.
$ %1$s is the resource that is missing.
$ %2$s is the location of the resource.
$ DO NOT TRANSLATE the number at the beginning of this message.  Leave it
$ exactly as shown and in the same position.  See general notes on ACTIONS.
$ Leave line spacing between sentences as shown.

BRIDGE_MENU_GOL_2 "950701 \
Missing resource resolution for %1$s in location %2$s.\n\n\
TURN...........the power switch of the device to the Off (O) position.\n\
PLUG...........the power cord in properly.\n\
TURN...........the power switch to the On (I) position.\n\
RESTART........the system."
$
$ Machine markings: (I) indicates On, (O) indicates Off.  See general notes.
$ %1$s is the resource that is missing.
$ %2$s is the location of the resource.
$ DO NOT TRANSLATE the number at the beginning of this message.  Leave it
$ exactly as shown and in the same position.
$ Leave line spacing between sentences as shown.  See general notes on ACTIONS.

BRIDGE_MENU_GOL_3 "950702 \
Missing resource resolution for %1$s in location %2$s.\n\n\
TURN...........the power switch of the device to the Off (O) position.\n\
SET............the correct SCSI address for the device, %3$s.\n\
TURN...........the power switch to the On (I) position.\n\
RESTART........the system."
$
$ SCSI is a technical term.  See general notes.
$ Machine markings: (I) indicates On, (O) indicates Off.  See general notes.
$ %1$s is the resource that is missing.
$ %2$s is the location of the resource.
$ %3$s will be the same as %1$s.
$ DO NOT TRANSLATE the number at the beginning of this message.  Leave it
$ exactly as shown and in the same position.
$ Leave line spacing between sentences as shown.  See general notes on ACTIONS.

BRIDGE_MENU_GOL_4 "950703 \
Missing resource resolution for %1$s in location %2$s.\n\n\
TURN...........the power switch of the device to the Off (O) position.\n\
PLUG...........the signal cable in properly.\n\
TURN...........the power switch to the On (I) position.\n\
RESTART........the system."
$
$ Machine markings: (I) indicates On, (O) indicates Off.  See general notes.
$ %1$s is the resource that is missing.
$ %2$s is the location of the resource.
$ DO NOT TRANSLATE the number at the beginning of this message.  Leave it
$ exactly as shown and in the same position.
$ Leave line spacing between sentences as shown.  See general notes on ACTIONS.

PORTA_MENU_GOL_1 "950750 \
Missing resource resolution for %1$s in location %2$s.\n\n\
TURN...........the power switch of the Portable Disk Drive enclosure\n\
               to the On (I) position.\n\
RESTART........the system."
$
$ Machine markings: (I) indicates On, (O) indicates Off.  See general notes.
$ %1$s is the resource that is missing.
$ %2$s is the location of the resource.
$ DO NOT TRANSLATE the number at the beginning of this message.  Leave it
$ exactly as shown and in the same position.
$ Leave line spacing between sentences as shown.  See general notes on ACTIONS.

PORTA_MENU_GOL_2 "950751 \
Missing resource resolution for %1$s in location %2$s.\n\n\
TURN...........the power switch of the Portable Disk Drive enclosure\n\
               to the Off (O) position.\n\
PLUG...........the power cord in properly.\n\
TURN...........the power switch to the On (I) position.\n\
RESTART........the system."
$
$ Machine markings: (I) indicates On, (O) indicates Off.  See general notes.
$ %1$s is the resource that is missing.
$ %2$s is the location of the resource.
$ DO NOT TRANSLATE the number at the beginning of this message.  Leave it
$ exactly as shown and in the same position.
$ Leave line spacing between sentences as shown.  See general notes on ACTIONS.

PORTA_MENU_GOL_3 "950752 \
Missing resource resolution for %1$s in location %2$s.\n\n\
TURN...........the power switch of the Portable Disk Drive\n\
               enclosure to the Off (O) position.\n\
PLUG...........the signal cable in properly.\n\
TURN...........the power switch to the On (I) position.\n\
RESTART........the system."
$
$ Machine markings: (I) indicates On, (O) indicates Off.  See general notes.
$ %1$s is the resource that is missing.
$ %2$s is the location of the resource.
$ Leave line spacing between sentences as shown.  See general notes on ACTIONS.

PORTA_MENU_GOL_4 "950753 \
Missing resource resolution for %1$s in location %2$s.\n\n\
TURN...........the power switch of the Portable Disk Drive\n\
               enclosure to the Off (O) position.\n\
SET............the correct SCSI address for the Portable Disk Drive\n\
               enclosure.\n\
TURN...........the power switch to the On (I) position.\n\
RESTART........the system."
$
$ SCSI is a technical term.  See general notes.
$ Machine markings: (I) indicates On, (O) indicates Off.  See general notes.
$ %1$s is the resource that is missing.
$ %2$s is the location of the resource.
$ DO NOT TRANSLATE the number at the beginning of this message.  Leave it
$ exactly as shown and in the same position.
$ Leave line spacing between sentences as shown.  See general notes on ACTIONS.

PORTA_MENU_GOL_5 "950754 \
Missing resource resolution for %1$s in location %2$s.\n\n\
TURN...........the power switch of the Portable Disk Drive enclosure\n\
               to the Off (O) position.\n\
PLUG...........a Disk Module into the Portable Disk Drive enclosure.\n\
TURN...........the power switch to the On (I) position.\n\
RESTART........the system."
$
$ Disk Module is the term used to refer to the removable disk drive.
$ Machine markings: (I) indicates On, (O) indicates Off.  See general notes.
$ %1$s is the resource that is missing.
$ %2$s is the location of the resource.
$ DO NOT TRANSLATE the number at the beginning of this message.  Leave it
$ exactly as shown and in the same position.
$ Leave line spacing between sentences as shown.  See general notes on ACTIONS.


HARR1_GOAL_1 "950800 \
Missing resource resolution for %1$s in location %2$s.\n\n\
TURN...........the drawer or tower power switch to the On (I) position.\n\
RESTART........the system."
$
$ Machine markings: (I) indicates On, (O) indicates Off.  See general notes.
$ %1$s is the resource that is missing.
$ %2$s is the location of the resource.
$ DO NOT TRANSLATE the number at the beginning of this message.  Leave it
$ exactly as shown and in the same position.
$ Leave line spacing between sentences as shown.  See general notes on ACTIONS.

HARR1_GOAL_2 "950801 \
Missing resource resolution for %1$s in location %2$s.\n\n\
TURN...........the drawer or tower compartment power switch for %3$s\n\
               to the On (I) position.\n\
RESTART........the system."
$
$ Machine markings: (I) indicates On, (O) indicates Off.  See general notes.
$ %1$s is the resource that is missing.
$ %2$s is the location of the resource.
$ %3$s will be the same as %1$s.
$ DO NOT TRANSLATE the number at the beginning of this message.  Leave it
$ exactly as shown and in the same position.
$ Leave line spacing between sentences as shown.  See general notes on ACTIONS.

HARR1_GOAL_3 "950802 \
Missing resource resolution for %1$s in location %2$s.\n\n\
SET............the correct SCSI address for the %3$s at\n\
               location %4$s.\n\
RESTART........the system."
$
$ SCSI is a technical term.  See general notes.
$ %1$s is the resource that is missing.
$ %2$s is the location of the resource.
$ %3$s will be the same as %1$s.
$ %4$s will be the same as %2$s.
$ DO NOT TRANSLATE the number at the beginning of this message.  Leave it
$ exactly as shown and in the same position.
$ Leave line spacing between sentences as shown.  See general notes on ACTIONS.

HARR2_GOAL_C0 "950803 \
Missing resource resolution for %1$s in location %2$s.\n\n\
TURN...........the drawer or tower power switch to the On (I) position.\n\
RESTART........the system."
$
$ Machine markings: (I) indicates On, (O) indicates Off.  See general notes.
$ %1$s is the resource that is missing.
$ %2$s is the location of the resource.
$ DO NOT TRANSLATE the number at the beginning of this message.  Leave it
$ exactly as shown and in the same position.
$ Leave line spacing between sentences as shown.  See general notes on ACTIONS.

HARR2_GOAL_D0 "950805 \
Missing resource resolution for %1$s in location %2$s.\n\n\
TURN...........the DASD power switch to the On (I) position.\n\
RESTART........the system."
$
$ Machine markings: (I) indicates On, (O) indicates Off.  See general notes.
$ %1$s is the resource that is missing.
$ %2$s is the location of the resource.
$ DO NOT TRANSLATE the number at the beginning of this message.  Leave it
$ exactly as shown and in the same position.
$ Leave line spacing between sentences as shown.  See general notes on ACTIONS.

HARR2_GOAL_C1 "950804 \
Missing resource resolution for %1$s in location %2$s.\n\n\
TURN...........the power switch of the device to the Off (O) position.\n\
PLUG...........the signal cable in properly.\n\
TURN...........the power switch of the device to the On (I) position.\n\
RESTART........the system."
$
$ Machine markings: (I) indicates On, (O) indicates Off.  See general notes.
$ %1$s is the resource that is missing.
$ %2$s is the location of the resource.
$ DO NOT TRANSLATE the number at the beginning of this message.  Leave it
$ exactly as shown and in the same position.  See general notes on ACTIONS.
$ Leave line spacing between sentences as shown.

$ ----------------------------------------------------------------------------
$ menu goals for 7134 (Saracen)

msg7134PowerSwitchGoal "950810 \
Missing resource resolution for %1$s in location %2$s.\n\n\
TURN...........the power switch of the 7134 unit to the On (I) position.\n\
TURN...........the power switch of the 7134 unit's rack to the On (I) position.\n\
RESTART........the system."
$
$ Machine markings: (I) indicates On, (O) indicates Off.  See general notes.
$ %1$s is the resource that is missing.
$ %2$s is the location of the resource.
$ DO NOT TRANSLATE the number at the beginning of this message.  Leave it
$ exactly as shown and in the same position.  See general notes on ACTIONS.
$ Leave line spacing between sentences as shown.

msg7134PowerCordGoal "950811 \
Missing resource resolution for %1$s in location %2$s.\n\n\
TURN...........the power switch of the 7134 unit to the Off (O) position.\n\
PLUG...........the power cord in properly.\n\
TURN...........the power switch of the 7134 unit to the On (I) position.\n\
RESTART........the system."
$
$ Machine markings: (I) indicates On, (O) indicates Off.  See general notes.
$ %1$s is the resource that is missing.
$ %2$s is the location of the resource.
$ DO NOT TRANSLATE the number at the beginning of this message.  Leave it
$ exactly as shown and in the same position.  See general notes on ACTIONS.

msg7131PubsReference "950610 \
Missing resource resolution for %1$s in location %2$s.\n\n\
For problem isolation, please refer to the documentation that came\n\
with the Model 7131 SCSI Media Tower."
$ %1$s is the resource that is missing.
$ %2$s is the location of the resource.
$ Leave line spacing between sentences as shown.  See general notes on ACTIONS.

$ ===========================================================================
$set MSG_HELP_SET

msgEnclHelp "\
This menu allows you to enter the type of\n\
enclosure that the missing device is located in.\n\
The selection that you make on this screen is\n\
used to further diagnose the missing problem.\n\
Move the cursor to selection, then press Enter.\n\n\
A bridge box is any external unit such as\n\
o IBM 7204 Model 010 1GB External Disk Drive\n\
o IBM 7206 Model 005 External 4-mm Tape Drive\n\
o IBM 7207 Model 012 1.2GB External 1/4-Inch\n\
  Cartridge Tape Drive\n\
o IBM 7208 Model 001 2.3GB External 8-mm Tape\n\
  Drive\n\
o IBM 7208 Model 011 5/10GB External Tape Drive\n\
o IBM 7209 Model 002 External Rewritable Optical\n\
  Disk Drive\n\
o IBM 7210 Model 001 External CD-ROM Drive\n\
o IBM 7210 Model 005 External CD-ROM Drive"
$
$ 3456789*123456789*123456789*123456789*12345678|      < 49 character line
$ Text should not be more than 49 characters per line.
$ Maintain spacing as shown.
$
