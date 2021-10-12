/*
 * @(#) 87 1.13 src/bos/usr/lpp/bosinst/BosMenus/BosInstTrans.h, bosinst, bos411, 9428A410j 94/06/16 09:40:02
 * COMPONENT_NAME: (BOSINST) Base Operating System Install
 *
 * FUNCTIONS: include file for BosMenus
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* BosInstTrans.h
 *
 * This file contains the multi-line and dynamic messages in BosMenus.
 */

/* Fig 4 - welcome */
#define BM_WELCOME \
"                   Welcome to Base Operating System\n\
                      Installation and Maintenance"
/* Fie 4.1 - easy install popup warn */
#define BM_WARN_INST \
"WARNING:  Base Operating System Installation may destroy or\n\
impair recovery of data.  Before installing, you should back up\n\
your system."

/* Fig 6 - Confirm settings */
#define BM_CONFIRM \
" Either type 0 and press Enter to install with current settings, or type the\n\
 number of the setting you want to change and press Enter."

#define BM_DEFAULT \
"         Keyboard Type......................Default"

#define BM_ALTERNATE \
"         Keyboard Type......................Alternate"

#define BM_OVERWRITEWARN \
    "  WARNING: Base Operating System Installation will\n\
        destroy or impair recovery of ALL data on the\n\
            destination disk %s.\n"

#define BM_NONOVERWRITEWARN \
"  WARNING: Base Operating System Installation will\n\
destroy or impair recovery of SOME data on the\n\
destination disk %s.\n"


/* Fig 7 Install method strings */
#define BM_INSTALL_OVERWRITE \
"      Overwrites EVERYTHING on the disk selected for installation.\n\
      Warning: Only use this method if the disk is totally empty or if there\n\
      is nothing on the disk you want to preserve.\n"

#define BM_INSTALL_PRESERVE \
"      Preserves SOME of the existing data on the disk selected for\n\
      installation.  Warning: This method overwrites the usr (/usr),\n\
      variable (/var), temporary (/tmp), and root (/) file systems.  Other\n\
      product (applications) files and configuration data will be destroyed.\n"

#define BM_INSTALL_MIGRATE \
"      Upgrades the Base Operating System to the current release.\n\
      Other product (applications) files and configuration data are saved.\n"


/* Fig 9 Sumplemental disks */
#define BM_SUPLEMENTAL_INST \
" Device Support must be supplied for disks not known to Base Operating\n\
 System Installation."

#define BM_SUPLEMENTAL_PROMPT \
"Insert the diskette containing the device support into the diskette drive\n\
 /dev/fd0, and press Enter."

/* Fig 12 Set primary language environemnt */
#define BM_LOCALE_MSG \
"   Type the number for the Cultural Convention (such as date, time, and\n\
   money), Language, and Keyboard for this system and press Enter, or type\n\
   %d and press Enter to create your own combination.\n"

#define BM_LOCALE_COMBO \
"    %2d Create your own combination of Cultural Convention, Language, and\n\
        Keyboard"

/* Fig 16 - Primary Cultutural Convention */
#define BM_CC_INST \
"    Type the number for the Primary Cultural Conventions (such as date, time,\n\
    and money) for this system and press Enter."

/* Fig 18 - primary language */

/* Fig 21 - Keyboard type selection strings */
#define BM_KBTYPE_TITLE "                Choose %s Keyboard"
#define BM_KBTYPE_INSTRUCT \
"Type the number of your choice and press Enter to select the correct\n\
description for your keyboard."

/* Fig 27 mksysb confirm */
#define BM_MKSYSB_CONFIRM_INST \
"Either type 0 and press Enter to install with the current settigns, or type the\n\
number of the setting you want to change and press Enter."


/* Fig 28 Mksysb disk selection */
#define BM_MKSYSB_INST \
"Type one or more numbers for the disk(s) to be used for installation and press\n\
Enter. To cancel a choice, type the corresponding number and Press Enter.\n\
At least one bootable disk must be selected. The current choice is indicated\n\
by >>>."

/* Fig 28.5 use maps dialog */
#define BM_MAPS_INST \
"Maps are available for use during installation.  Type 1 and press Enter to use\n\
 the maps for installation."

/* Fig 20 copydumpmenu return code messages */
#define BM_MAINT_DR1    "Unable to get the dump information for copying \nthe dump."

#define BM_MAINT_DR2    "No previous dump exists."

#define BM_MAINT_DR3    "Dump is not in a logical volume. \nNo dump copy is allowed."

#define BM_MAINT_DR4    "Unable to initialize ODM. \nNo dump copy is allowed."

/* Fig 30 access RootVG warning */
#define BM_ROOTVG_WARN \
"If you choose to access a root volume group, you will NOT be able to return\n\
to the Base Operating System Installation menus without rebooting."

/* Fig 37 info for advanced maintenance shell */
#define BM_SHELL_INFO \
" To return to the Maintenance Menu after completing maintenance\n\
 activities, type exit on the command line and press Enter."


/* Fig 39 Choose tape drive (mksysb maint install) */
#define BM_MKSYSB_TAPE_INST \
"Type the number of the tape drive containing the system backup to be\n\
installed and press Enter."

/* Help strings */
/*  fig_03.hlp */
#define BM_FIG_03 \
"                  Help for Select Language for Install\n\n\
Select the language in which you want the installation instructions, options,\n\
help screens, and messages displayed during the installation process."

/*  fig_04.hlp */
#define BM_FIG_04 \
"                  Help for Installation and Maintenance\n\n\
Type 1 and press Enter to begin installation imediatly using the defaults\n\
selected by the system\n\
\n\
Type 2 and press Enter to go to the Installation and Settings menu.\n\
Use this option to view or change the default settings.  You can begin the\n\
installation from this menu.\n\
\n\
Type 3 and press Enter to select from a list of maintenance options.\n\
\n\
Maintenance options are primarily intended for recovery procedures."

/* fig 4.1 help */
#define BM_FIG_041 \
"                Help for Installtion Warning\n\
\n\
To begin installation using the current settings, type 1 and press Enter.\n\
The current settigns are initially set by the system.  If you want to\n\
view or change the settigns, go back to the Installation and Maintenance\n\
menu and select option 2, Change/Show Installation Settings and Install."

/*  fig_05.hlp */
#define BM_FIG_05  \
"                  Help for Network Software Selection\n\n\
Type the number indicating the base operating system you want to install and\n\
press enter."

/*  fig_06.hlp */
#define BM_FIG_06  \
"                  Help for Installation and Settings\n\n\
The Installation and Settings screen displays the system default settings\n\
for the installation method, installation disk and primary language support,\n\
and trusted computing base (TCB).\n\n\
To install using the displayed defaults, press Enter.\n\
\n\
Type number 1 and press Enter to change the method of installation\n\
or the disk where you want to install.\n\
\n\
Type number 2 and press Enter to change Primary Language Support (AFTER\n\
Install).\n\
\n\
Type number 3 and press Enter to change no to yes or yes to no.\n\
When you install the TCB, the trusted path, the trusted shell, and\n\
system integrity checking is enabled. If you want to install the\n\
TCB, it must be installed now. The TCB cannot be installed later.\n\
Installation time is reduced if you do not install the TCB."
 
/*  fig_07.hlp */
#define BM_FIG_07 \
"                        Help for Method of Installation\n\
\n\
There are three methods of installation:\n\
* New and Complete Overwrite - New and Complete Overwrite should be used on a\n\
  new system. This method should only be used on an existing system if none\n\
  of the data  on the existing system needs to be saved.\n\
\n\
* Preservation Install - The Preservation Install saves the user created\n\
  logical volumes, the volume groups, and the /home directory.  Preservation\n\
  Install is the recommended method for installing the Base Operating System\n\
  Version 4.1 on a 3.1 machine.\n\
\n\
* Migration Install - Migration Install preserves the user created logical\n\
  volumes, the volume groups, and the /, /usr, /home, and /var directories.\n\
  Migration Install is the recommended method for installing the Base\n\
  Operating System Version 4.1 on a 3.2 machine."

/*  fig_08.hlp   */
#define BM_FIG_08 \
"                 Help for Change Destination Disk(s)\n\
\n\
The Change Disk(s) Where You Want to Install screen displays the available\n\
system disks and status information. The disk name, the physical slot location\n\
(SCSI Address), the size of the disk, and the root volume group status is \n\
displayed for each available disk. The Bootable column indicates whether \n\
the disk is bootable. To select the default disk displayed at the top, \n\
press Enter.\n\
\n\
To select another disk, type the number indicating the disk where you want to\n\
install, and press Enter."

/*  fig_09.hlp */
#define BM_FIG_09 \
"                  Help for Support for Unknown Disk\n\
\n\
Non-supported Disks are not recognized by the system until additional\n\
configuration information is added. Configuration data is supplied on diskette\n\
by the manufacturer. Insert the diskette containing the configuration data\n\
into device /dev/fd0 and press Enter."

/*  fig_11.5.hlp */
#define BM_FIG_12  \
"                    Help for Primary Language Support\n\
\n\
Type the number that represents the Cultural Convention, Language, and Keyboard\n\
you want to use for this system after completion of the installation process.\n\
\n\
Type the number for creating your own combination and press Enter \n\
to make individual selections for the Cultural Convention, Language, \n\
and Keyboard.\n\
\n\
Cultural Convention specifies the date and time representation, monetary\n\
symbols, collation, case conversion, character classification, and numeric\n\
representation unique to the selected option.\n\
\n\
Language specifies the language in which system messages are displayed.\n\
\n\
The Keyboard selection associates the keyboard with a particular keyboard\n\
mapping or template. This selection applies to graphics terminals only.\n\
\n\
After the installation process is complete, you can change primary language\n\
support selections and add secondary language support selections."

/*  fig_11.hlp */
#define BM_FIG_11 \
"                     Help for Destination Disk\n\
\n\
You can change the installation destination disk to another disk containing an\n\
existing root volume group. The level refers to the version of the Base \n\
Operating System installed for the root volume group.  The disks in Root VG\n\
indicate which disks are used by the root volume group. The location code \n\
indicates the slot where the SCSI board for the disk is connected. The size\n\
indicates the number of Megabytes of space for the disk."


/*  fig_16.hlp */
#define BM_FIG_16 \
"                    Help for Primary Cultural Convention\n\
\n\
Enter the number that represents the Cultural Convention you want to use on\n\
this system after the installation process is complete.\n\
\n\
Cultural Convention specifies the date and time representation, monetary\n\
symbols, collation, case conversion, character classification, and numeric\n\
representation unique to the selected option.\n\
\n\
After the installation process is complete, you can change the Cultural\n\
Convention selection."

/*  fig_18.hlp */
#define BM_FIG_18  \
"                     Help for Primary Lanaguage\n\
\n\
Type the number that represents your choice for the language in which system\n\
messages will be displayed on this system, after the installation process is\n\
complete.\n\
\n\
After the installation process is complete, you can change the Language\n\
selection."

/*  fig_19.hlp */
#define BM_FIG_19 \
"                      Help for Primary Keyboard\n\
\n\
Type the number indicating the keyboard selection you want to use for this\n\
system after the installation process is complete.\n\
\n\
The keyboard selection associates the keyboard of an graphics terminal with\n\
a particular keyboard mapping or template.\n\
\n\
After the installation process is complete, you can change the keyboard\n\
selection."

/* fig_21.hlp */
#define BM_FIG_21 \
"                     Help for Keyboard Choice\n\
\n\
Choose 1 if the description next to the number 1 matches your keyboard.\n\
Choose 2 if the description next to the number 2 matches your keyboard."

/*  fig_27.hlp */
#define BM_FIG_27 \
"             Help for System Backup Installation and Settings\n\
\n\
The System Backup Installation and Settings Screen displays the system\n\
default settings for Use Maps Files, and Shrink File Systems. You can change\n\
the disk where the backup copy (image) of the root and user volume groups\n\
will be installed. The Maps Files are used to indicate the exact physical\n\
placement of the root and user volume groups. When you change the installation\n\
disk, unless it is the disk used to create the backup copy, or an exact\n\
duplicate, the Use Maps option should be set to no.\n\
\n\
The Shrink File Systems option installs the file systems using the minimum\n\
required space. The default setting is yes.\n\
\n\
To install the backup copy using the default settings, press Enter.\n\
\n\
To change the installation disk and Maps option, type 1 and press Enter.\n\
\n\
To change the shrink file systems option, type 2 and press Enter."

/*  fig_28.5.hlp */
#define BM_FIG_285 \
"                     Help for Use Maps\n\
\n\
You may choose to use maps for those disks that have them installed.\n\
\n\
Select choice 1 to use the maps.\n\
\n\
Select choice 2 to not use the maps."

/*  fig_28.hlp */
#define BM_FIG_28 \
"                 Help for Change Destination Disk(s)\n\
\n\
The Change Disk(s) Where You Want to Install screen displays the available\n\
system disks and status information. The disk name, the physical slot location\n\
(SCSI Address), the size of the disk, and the root volume group status is \n\
displayed for each available disk. The Bootable column indicates whether \n\
the disk is bootable, and the Maps column indicates whether maps files \n\
have been established for the root volume group. To select the default \n\
disk displayed at the top, press Enter.\n\
\n\
To select another disk, type the number indicating the disk where you want to\n\
install, and press Enter."

/*  fig_29.hlp */
#define BM_FIG_29 \
"                      Help for Maintenance\n\
\n\
Use the maintenance utilities to assist in system recovery procedures.\n\
\n\
*  Access a Root Volume Group - The get root volume group option provides\n\
   access to an existing root volume group Use this option to boot from tape,\n\
   CD-ROM, or the network when the system will not boot from disk. You can fix\n\
   the problem, or back up data before reinstallation.\n\
\n\
*  Copy a System Dump to Removable Media - This option copies a hexidecimal\n\
   dump of the system to the specified output device (tape or diskette). This\n\
   file can be used to help determine the cause of the problem.\n\
\n\
*  Access Advanced Maintenance Functions - This option provides a Korn shell\n\
   and allows you to perform a limited number of tasks.\n\
\n\
*  Install from a System Backup - This option allows you to install the Base\n\
   Operating System from a backup copy (image) of the root and user volume\n\
   group of a previously installed system."

/*  fig_30.hlp */
#define BM_FIG_30 \
"                   Help for Rootfs Warning\n\
\n\
The Access a Root Volume Group command allows you to access an existing root\n\
volume group on the current system. This option should be used when the system\n\
will not boot from disk. You can boot from tape, CD-ROM, or the network. This\n\
allows you to either fix the problem, or back up the data from the root volume\n\
group before reinstalling the Base Operating System software. If this option\n\
is selected, you must reboot to complete the Base Operating System software\n\
installation."

/*  fig_37.hlp */
#define BM_FIG_37 \
"                Help for Advanced Maintenabce Functions\n\
\n\
Press Enter to open the limited function maintenance shell. After you have\n\
entered the appropriate commands, type exit at the command line and press Enter\n\
to return to the Base Operating System installation."

/*  fig_39.hlp */
#define BM_FIG_39 \
"                   Help for Choose Tape Drive\n\
\n\
The system backup contains the boot (startup) files, the Base Operating System,\n\
system configuration data, user volume groups and any optional software\n\
products that were installed.\n\
\n\
Insert the media containing the system backup into the appropriate device\n\
selected from the list displayed on the screen. Type the number indicating the\n\
device containing the backup and press Enter.\n\
\n\
If the backup consists of more than one volume, you will be prompted when the\n\
next volume is required."

