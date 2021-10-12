$  @(#)90        1.52.2.9  src/bos/usr/lpp/bosinst/BosMenus/BosMenus.msg, bosinst, bos41J, 9519A_all 5/3/95 08:37:10
$ COMPONENT_NAME: (BOSINST) Base Operating System Install
$
$ FUNCTIONS: include file for BosMenus
$
$ ORIGINS: 27
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1993,1994
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$

$ NOTES:
$	All messages in this file must be ISO8859-1 characters.
$

$quote "         define quote character.

$len 
$set BI        Define initial set#

$ strings common to all screens

$ pad spaces so that the 3 strings below are the same length
BS_IM_PRES	"Preservation              "
BS_IM_NEW	"New and Complete Overwrite"
BS_IM_MIG	"Migration                 "


$ do not translate vg, rootvg. Pad spaces so that the 3 strings below
$ are the same length
BS_VGS_NONE	"not in any vg"
BS_VGS_ROOT	"rootvg       "
BS_VGS_OTHER	"other vg     "

$ pad spaces so that the strings are the same length
BS_NO		"No "
BS_YES		"Yes"

$  This message may be up to 3 lines long, 55 characters/line.
BS_INVALID	"\n    Invalid Choice.  Please try again\n\n"

$  This message may be up to 3 lines long, 55 characters/line.
BS_OVERWRITEWARN "\
    WARNING: Base Operating System Installation will\n\
    destroy or impair recovery of ALL data on the\n\
    destination disk %s.\n"

$  This message may be up to 3 lines long, 55 characters/line.
BS_NONOVERWRITEWARN "\
    WARNING: Base Operating System Installation will\n\
    destroy or impair recovery of SOME data on the\n\
    destination disk %s.\n"

$  This message may be up to 3 lines long, 55 characters/line.
BS_NOSPACE "\
\n    Selected disk does not have enough space\n"

$  This message may be up to 3 lines long, 55 characters/line.
BS_NODISK "\
\n    You must select at least one bootable disk.\n"

$ do not translate %2.2d
BS_PREV_CHOICE	"    %2.2d  ...PREVIOUS CHOICES"
BS_MORE_CHOICE  "    %2.2d  MORE CHOICES..."

BS_HELP 	"    88  Help ?"
BS_PREV_MENU	"    99  Previous Menu"
BS_CHOICE	">>> Choice "

BS_NODISKS 	"     No Disks Available"
BS_NOTAPE	"No tape devices available"

$ fig 3
$ substitute your language for English
$ This string is restricted to 7-bit ASCII - no national characters.
$ WARNING: if the string is longer than 72 characters, place a
$ newline ('\n') in the string where you want the line wrap to occur.
$ Otherwise the line count will be inaccurate and the top of the screen will
$ scroll off the top.
BOSINST "     %d Type %d and press Enter to have English during install."

$  Fig 4 - welcome 
$ the welcome message should be centered
BS_WELCOME "\
              Welcome to Base Operating System\n\
                      Installation and Maintenance"

BS_INSTALL_PROMPT "\
Type the number of your choice and press Enter.  Choice is indicated by >>>."

BS_INSTALL 	"    2 Change/Show Installation Settings and Install"

$ The following message is no longer used
BS_INSTALL_EXP	"         Install with Current Settings or Change Settings."

BS_MAINT	"    3 Start Maintenance Mode for System Recovery"

$ The following message is no longer used
BS_MAIN_EXP	"         Options for System Recovery."

$ Fig 05 - network image selection
$ description, type and name must be in the same columns when translated
BS_NIM_TITLE	"        Select Software for Network Installation"
BS_NIM_INST	"Type the number of your choice and press Enter."
BS_NIM_HEADER	"       Name                  Type     Description",


$  Fig 6 - Confirm settings 
BS_TITLE	"                  Installation and Settings"
BS_CONFIRM "\
Either type 0 and press Enter to install with current settings, or type the\n\
 number of the setting you want to change and press Enter."

BS_CNFRM_HDNG	"       Setting:                             Current Choice: "
BS_CNFRM_SET	"    1  System Settings:"
BS_CNFRM_SI	"         Method of Installation............."
BS_CNFRM_DSK	"         Disk Where You Want to Install....."
BS_CNFRM_LNG	"    2  Primary Language Environment Settings (AFTER Install):"
BS_CNFRM_CC	"         Cultural Convention................"
BS_CNFRM_MT	"         Language .........................."
BS_CNFRM_KBM	"         Keyboard .........................."
BS_DEFAULT	"         Keyboard Type......................Default"
BS_ALTERNATE	"         Keyboard Type......................Alternate"

BS_CNFRM_GO	"    0  Install AIX with the current settings listed above."

$  Fig 7 Install method strings 
BS_CHG_MTHD	"                Change Method of Installation"
BS_CHG_MTHD_INST "\
  Type the number of the installation method and press Enter."
BS_CHG_MTHD_NEW	"    1 New and Complete Overwrite"

$ this message may use at most 4 lines.
BS_INSTALL_OVERWRITE "\
      Overwrites EVERYTHING on the disk selected for installation.\n\
      Warning: Only use this method if the disk is totally empty or if there\n\
      is nothing on the disk you want to preserve.\n"

BS_CHG_MTD_PSRV	"    2 Preservation Install"

$ this message may use at most 5 lines. do not translate "/usr", "/var", 
$ "/tmp", or "/".
BS_INSTALL_PRESERVE "\
      Preserves SOME of the existing data on the disk selected for\n\
      installation.  Warning: This method overwrites the usr (/usr),\n\
      variable (/var), temporary (/tmp), and root (/) file systems.  Other\n\
      product applications) files and configuration data will be destroyed.\n"

BS_CHG_MTHD_MIG	"    3 Migration Install"

$ this message may be at most 3 lines long.
BS_INSTALL_MIGRATE "\
      Upgrades the Base Operating System to the current release.\n\
      Other product (applications) files and configuration data are saved.\n"


$ Fig 8 Change destination disk
BS_CHGDSK_OVR_TITLE	"                Change Disk Where You Want to Install" 

BS_CHGDSK_OVR_INST "\
Type the number for the disk to be used for installation and press Enter."

$ column heading names must start in the same columns.
BS_CHGDSK_OVR_HDG "\
        Name      Location Code   Size(MB)  VG Status   Bootable",

BS_CFGDSK 	"    66  Disks not known to Base Operating System Installation"

$  Fig 9 Sumplemental disks 
BS_SUPLEMENTAL_TITLE "\
        Support for Disks not known to Base Operating System Installation"

BS_SUPLEMENTAL_INST "\
 Device Support must be supplied for disks not known to Base Operating\n\
 System Installation."

BS_SUPLEMENTAL_PROMPT "\
Insert the diskette containing the device support into the diskette drive\n\
 /dev/fd0, and press Enter."


$ Fig 10 Configuration in progress
BS_SUPLEMENTAL_READ	"                Reading diskette."

BS_SUPLEMENTAL_WAIT	"                Please Wait..."


$ Fig 11 - disk selection
BS_CHGDSK_TITLE		"            Change Disks Where You Want to Install"

BS_CHGDSK_INST "\
Type the number for the disks to be used for installation and press Enter."

$ translated column headings should start in same column as below
BS_CHGDSK_HDG "\
       Level    Disks In Rootvg       Location Code      Size(MB)"


$ Fig 11.5 - 15 - Primary langauge environment
BS_NLS_TITLE	"              Set Primary Language Environment"

BS_LOCALE_MSG "\
   Type the number for the Cultural Convention (such as date, time, and\n\
   money), Language, and Keyboard for this system and press Enter, or type\n\
   %d and press Enter to create your own combination.\n"

$ translated column headings should start in same column as below
BS_NLS_HDG "\
        Cultural Convention    Language               Keyboard "

BS_LOCALE_COMBO "\
    %2d  Create your own combination of Cultural Convention, Language, and\n\
	Keyboard"

$  Fig 16 - Primary Cultural Convention 
BS_CC_TITLE	"            Set Primary Cultural Convention"

BS_CC_INST "\
    Type the number for the Primary Cultural Conventions (such as date, time,\n\
    and money  for this system and press Enter."


$  Fig 18 - primary language 
BS_MSG_TITLE	"           Set Primary Language "

BS_MSG_INST "\
Type the number for the Primary Language for this system and press Enter."

$ Fig 20 - set keyboard
BS_KB_TITLE	"               Set Keyboard "

BS_KB_INST "\
Type the number for the Keyboard for this system and press Enter."


$  Fig 21 - Keyboard type selection strings 
BS_KBTYPE_TITLE "             Choose %s Keyboard"
BS_KBTYPE_INSTRUCT "\
Type the number of your choice and press Enter to select the correct\n\
description for your keyboard."

$  Fig 27 mksysb confirm 
BS_MKSYSB_CONFIRM_TITLE	"         System Backup Installation and Settings"

BS_MKSYSB_CONFIRM_INST "\
Either type 0 and press Enter to install with the current settings, or type the\n\
number of the setting you want to change and press Enter."

$ translated column headings should start in same column as below
BS_MKSYSB_CONFIRM_HDG "\
      Setting:                                 Current Choice(s):"

BS_MKSYSB_CONFIRM_DSK	"    1 Disk(s) where you want to install ......"
BS_MKSYSB_CONFIRM_MAP	"         Use Maps............................."
BS_MKSYSB_CONFIRM_SHRK	"    2 Shrink File Systems....................."
BS_MKSYSB_CONFIRM_GO	"    0 Install with the settings listed above."


$  Fig 28 Mksysb disk selection 
BS_MKSYSB_DSK_TITLE "         Change Disk(s) Where You Want to Install"

BS_MKSYSB_DSK_INST "\
Type one or more numbers for the disk(s) to be used for installation and press\n\
Enter. To cancel a choice, type the corresponding number and Press Enter.\n\
At least one bootable disk must be selected. The current choice is indicated\n\
by >>>.",

BS_MKSYSB_DSK_HDG "\
        Name     Location Code    Size(MB)   VG Status   Bootable   Maps",

BS_MKSYSB_DSK_GO	"     0   Continue with choices indicated above"


$  Fig 28.5 use maps dialog 
BS_MAPS_TITLE	"            Use Maps"

BS_MAPS_INST "\
Maps are available for use during installation.  Type 1 and press Enter to use\n\
 the maps for installation."

BS_MAPS_PROMPT	" Type the number of your choice and press Enter."

BS_MAPS_YES	"    1  Use maps for installation"

BS_MAPS_NO	"    2  Do not use maps for installation"


$  Fig 29 maintenance utilities
BS_MAINT_TITLE	"            Maintenance "

BS_MAINT_INST	"Type the number of your choice and press Enter."

BS_MAINT_RVG	"    1 Access a Root Volume Group "

BS_MAINT_DUMP	"    2 Copy a System Dump to Removable Media"

BS_MAINT_SHELL	"    3 Access Advanced Maintenance Functions"

BS_MAINT_MKSYSB	"    4 Install from a System Backup"

BS_MAINT_DR1	"Unable to get the dump information for copying \nthe dump."

BS_MAINT_DR2	"No previous dump exists."

BS_MAINT_DR3	"Dump is not in a logical volume. \nNo dump copy is allowed."

BS_MAINT_DR4	"Unable to initialize ODM.  \nNo dump copy is allowed."

$  Fig 30 access RootVG warning 
BS_ROOTVG_WARN_TITLE	"          Warning:"

BS_ROOTVG_WARN "\
If you choose to access a root volume group, you will not be able to return\n\
to the Base Operating System Installation menus without rebooting."

BS_ROOTVG_GO	"    0 Continue"


$  Fig 37 info for advanced maintenance shell 
BS_SHELL_TITLE	"      Information for Advanced Maintenance Functions"

BS_SHELL_INFO "\
 To return to the Maintenance Menu after completing maintenance\n\
 activities, type exit on the command line and press Enter."

BS_SHELL_PROMPT	" Type the number of your choice and press Enter."

BS_SHELL_GO	"    0 Enter the Limited Function Maintenance Shell"


$  Fig 39 Choose tape drive (mksysb maint install) 
BS_MKSYSB_TAPE_TITLE	"            Choose Tape Drive"

BS_MKSYSB_TAPE_INST "\
Type the number of the tape drive containing the system backup to be\n\
installed and press Enter."

BS_MKSYSB_TAPE_HDG "\
      Tape Drive                     Path Name"

$ NEW MESSAGE:
$ Note to translators: This message should not be translated
$ to more than 34 characters, to leave room for at least 3 .'s
$ After the ...'s will be a field "No" or "Yes"
BS_TCB "    3  Install Trusted Computing Base......."

BS_INSTALL_EASY	"    1 Start Install Now with Default Settings"

BS_INST_WARN "                 Installation Warning"
BS_WARN_INST "\
WARNING:  Base Operating System Installation may destroy or\nimpair recovery of data.  Before installing, you should back up\nyour system.",

BS_INSTALL_CONT "    1 Continue with Install"

BS_CONTINUE  ">>>  0  Continue\n"
BS_MIGCHOICE ">>> Choice[0]: "

$  This message may be up to 3 lines long, 55 characters/line.
BS_NODISK_SELECT "\
\n    No disks have been selected.\n"

$set BL

$ Notes to translators:
$ - All messages in the range 100-199 should be limited to a maximum of
$   23 characters.
$ - Do not translate the expressions: (EUC), (ISO), or (PC)

100 "C (POSIX)"
101 "Chinese (Traditional)"
102 "Danish"
103 "Dutch (Belgium)"
104 "Dutch"
105 "English (Great Britain)"
106 "English (United States)"
107 "Finnish"
108 "French (Belgium)"
109 "French (Canada)"
110 "French (Switzerland)"
111 "French"
112 "German (Switzerland)"
113 "German"
114 "Greek"
115 "Icelandic"
116 "Italian"
117 "Japanese(EUC)"
118 "Korean"
119 "Norwegian"
120 "Portuguese"
121 "Spanish"
122 "Swedish"
123 "Turkish"
124 "Arabic(ISO)"
125 "Bulgarian"
126 "Croatian"
127 "Czech"
128 "Hebrew(ISO)"
129 "Hungarian"
130 "Macedonian"
131 "Polish"
132 "Romanian"
133 "Russian"
134 "Serbian Cyrillic"
135 "Serbian Latin"
136 "Slovak"
137 "Slovene"
138 "Arabic(PC)"
139 "Hebrew(PC)"
140 "Japanese(PC)"
141 "Portuguese (Brazil)"
142 "Chinese (Simplified EUC)"
143 "Chinese (Simplified UTF)"
144 "Catalan"


200 "C (POSIX) keyboard."

201 "Chinese (Traditional) keyboard."

202 "Danish keyboard."

203 "Dutch (Belgium) keyboard."

$ Default Dutch Keyboard Message (nl_NL).
204 "Dutch keyboard: The top row of letters starts with 'Q W'."

$ Default English (Great Britain) Keyboard Message (en_GB).
205 "English (UK) Default: The '8' key has an '*' on it."

206 "English (United States) keyboard."

$ Default Finnish Keyboard Message (fi_FI).
207 "Finnish Default: The front side of the '7' key has a '{' on it."

208 "French (Belgium) keyboard."

209 "French (Canada) keyboard."

210 "French (Switzerland) keyboard."

$ Default French Keyboard Message (fr_FR).
211 "French Default: The '8' key has an '!' on it."

212 "German (Switzerland) keyboard."

213 "German keyboard."

214 "Greek keyboard."

215 "Icelandic keyboard."

$ Default Italian Keyboard Message (it_IT).
216 "Italian Default: The front side of the 'Q' key has an '@' on it."

217 "Japanese(EUC) keyboard."
$ This message is obsolete.  It has been replaced by messages 255 and 256
$ in this set.

218 "Korean keyboard."

219 "Norwegian keyboard."

220 "Portuguese keyboard."

221 "Spanish keyboard."

$ Default Swedish Keyboard Message (sv_SE).
222 "Swedish Default: The front side of the '7' key has a '{' on it."

$ Default Turkish Keyboard Message (tr_TR).
223 "Turkish Default: The top row of letters starts with 'Q W'."

$ Alternate Turkish Keyboard Message (tr_TR@alt).
224 "Turkish Alternate: The top row of letters starts with 'F G'."

$ Alternate English (Great Britain) Keyboard Message (en_GB@alt).
225 "English (UK) Alternate: The '8' key has a '(' on it."

$ Alternate Finnish Keyboard Message (fi_FI@alt).
226 "Finnish Alternate: The front side of the '7' key has nothing on it."

$ Alternate French Keyboard Message (fr_FR@alt).
227 "French Alternate: The '8' key as an '_' on it."

$ Alternate Italian Keyboard Message (it_IT@alt).
228 "Italian Alternate: The front side of the 'Q' key has nothing on it."

$ Alternate Swedish Keyboard Message (sv_SE@alt).
230 "Swedish Alternate: The front side of the '7' key has nothing on it."

231 "Arabic(ISO) keyboard."
232 "Bulgarian Default: The keyboard has 102 keys."
233 "Croatian keyboard."
234 "Czech keyboard."
235 "Hebrew(ISO) Default: The keyboard has 102 keys."
236 "Hungarian keyboard."
237 "Macedonian keyboard."
238 "Polish keyboard."
239 "Romanian keyboard."
240 "Russian Default: The keyboard has 101 keys."
241 "Serbian Cyrillic keyboard."
242 "Serbian Latin keyboard."
243 "Slovak keyboard."
244 "Slovene keyboard."
245 "Arabic(PC) keyboard."
246 "Hebrew(PC) Default: The keyboard has 102 keys."
247 "Japanese(PC) keyboard."
$ This message is obsolete.  It has been replaced by messages
$ 253 and 254 in this set.
248 "Bulgarian Alternate: The keyboard has 101 keys."
249 "Russian Alternate: The keyboard has 102 keys."
250 "Hebrew(ISO) Alternate: The keyboard has 101 keys."
251 "Hebrew(PC) Alternate: The keyboard has 101 keys."
252 "Portuguese (Brazil) keyboard."
253 "Japanese(PC) Default: The 'Caps Lock' is on the bottom row of keys."
254 "Japanese(PC) Alternate: The 'Caps Lock' is above the left shift key."
255 "Japanese(EUC) Default: The 'Caps Lock' is on the bottom row of keys."
256 "Japanese(EUC) Alternate: The 'Caps Lock' is above the left shift key."
257 "Chinese (Simplified EUC) keyboard."
258 "Chinese (Simplified UTF) keyboard."

$ Note to Translators:
$    This Limitation and Restriction applies to the Help Set (set BH) below. 
$ The Text for each Help Message Must be at most One Full Screen.
$ A Full Screen is 21 Lines with each Line at most with 79 Characters.
$ The 21 Lines Includes the Heading and any empty Lines.
$ If the Text is greater that 21 lines or if a line contains more
$ than 79 Characters then the Help text will scroll off the screen.
$
$set BH
$ help set
$  fig_03.hlp
FIG_03 "\
                  Help for Select Language for Install\n\n\
Select the language in which you want the installation instructions, options,\n\
help screens, and messages displayed during the installation process."

$  fig_04.hlp
FIG_04 "\
                  Help for Installation and Maintenance\n\n\
Type 1 and press Enter to begin installation immediately using the defaults\n\
selected by the system.\n\
\n\
Type 2 and press Enter to go to the Installation and Settings menu.\n\
Use this option to view or change the default settings.  You can begin the\n\
installation from this menu.\n\
\n\
Type 3 and press Enter to select from a list of maintenance options.\n\
\n\
Maintenance options are primarily intended for recovery procedures."

$  fig_05.hlp
FIG_05  "\
                  Help for Network Software Selection\n\n\
Type the number indicating the base operating system you want to install and\n\
press enter."

$  fig_06.hlp
FIG_06  "\
                  Help for Installation and Settings\n\n\
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
system integrity checking are installed. The trusted path protects your\n\
system in case a program is masquerading as the program you want to use.\n\
The trusted path tries to ensure that the programs you run are trusted programs.\n\
Installation time is reduced if you do not install the trusted path."
   
$  fig_07.hlp
FIG_07 "\
                        Help for Method of Installation\n\
\n\
There are three methods of installation:\n\
\n\
* New and Complete Overwrite - New and complete Overwrite should be used on a\n\
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

$  fig_08.hlp  
FIG_08 "\
                 Help for Change Destination Disk(s)\n\
\n\
The Change Disk(s) Where You Want to Install screen, displays the available\n\
system disks and status information. The disk name, the physical slot location\n\
(SCSI Address), the size of the disk, the root volume group status is displayed\n\
for each available disk. The Bootable column indicates whether the disk is\n\
bootable. To select the default disk displayed at the top, press Enter.\n\
\n\
To select another disk, type the number indicating the disk where you want to\n\
install, and press Enter."

$  fig_09.hlp
FIG_09 "\
                  Help for Support for Unknown Disk\n\
\n\
Non-supported Disks are not recognized by the system until additional\n\
configuration information is added. Configuration data is supplied on diskette\n\
by the manufacturer. Insert the diskette containing the configuration data\n\
into device /dev/fd0 and press Enter."

$  fig_11.hlp
FIG_11 "\
                     Help for Destination Disk\n\
\n\
You can change the installation destination disk to another disk containing an\n\
existing root volume group. The level refers to the version of the Base \n\
Operating System installed for the root volume group.  The disks in Root VG\n\
indicate which disks are used by the root volume group. The location code \n\
indicates the slot where the SCSI board for the disk is connected. The size\n\
indicates the number of Megabytes of space for the disk."

$  fig_12.hlp
FIG_12 "\
                    Help for Primary Language Support\n\
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

$  fig_16.hlp
FIG_16 "\
                    Help for Primary Culture Convention\n\
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

$  fig_18.hlp
FIG_18  "\
                     Help for Primary Lanaguage\n\
\n\
Type the number that represents your choice for the language in which system\n\
messages will be displayed on this system, after the installation process is\n\
complete.\n\
\n\
After the installation process is complete, you can change the Language\n\
selection."

$  fig_19.hlp
FIG_19 "\
                      Help for Primary Keyboard\n\
\n\
Type the number indicating the keyboard selection you want to use for this\n\
system after the installation process is complete.\n\
\n\
The keyboard selection associates the keyboard of an graphics terminal with\n\
a particular keyboard mapping or template.\n\
\n\
After the installation process is complete, you can change the keyboard\n\
selection."

$ fig_21.hlp
FIG_21 "\
                     Help for Keyboard Choice\n\
\n\
Choose 1 if the description next to the number 1 matches your keyboard.\n\
Choose 2 if the description next to the number 2 matches your keyboard."

$  fig_27.hlp
FIG_27 "\
             Help for System Backup Installation and Settings\n\
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

$  fig_28.5.hlp
FIG_285 "\
                     Help for Use Maps\n\
\n\
You may choose to use maps for those disks that have them installed.\n\
\n\
Select choice 1 to use the maps.\n\
\n\
Select choice 2 to not use the maps."

$  fig_28.hlp
FIG_28 "\
                 Help for Change Destination Disk(s)\n\
\n\
The Change Disk(s) Where You Want to Install screen, displays the available\n\
system disks and status information. The disk name, the physical slot location\n\
(SCSI Address), the size of the disk, the root volume group status is displayed\n\
for each available disk. The Bootable column indicates whether the disk is\n\
bootable, and the Maps column indicates whether maps files have been\n\
established for the root volume group. To select the default disk displayed at\n\
the top, press Enter.\n\
\n\
To select another disk, type the number indicating the disk where you want to\n\
install, and press Enter."

$  fig_29.hlp
FIG_29 "\
                      Help for Maintenance\n\
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

$  fig_30.hlp
FIG_30 "\
                   Help for Rootfs Warning\n\
\n\
The Access a Root Volume Group command allows you to access an existing root\n\
volume group on the current system. This option should be used when the system\n\
will not boot from disk. You can boot from tape, CD-ROM, or the network. This\n\
allows you to either fix the problem, or back up the data from the root volume\n\
group before reinstalling the Base Operating System software. If this option\n\
is selected, you must reboot to complete the Base Operating System software\n\
installation."

$  fig_37.hlp
FIG_37 "\
                Help for Advanced Maintenance Functions\n\
\n\
Press Enter to open the limited function maintenance shell. After you have\n\
entered the appropriate commands, type exit at the command line and press Enter\n\
to return to the Base Operating System installation."

$  fig_39.hlp
FIG_39 "\
                   Help for Choose Tape Drive\n\
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

CAT_CD_HELP                     "\
                Help for Copy a System Dump\n\n\
A system dump consists of a hexadecimal dump of the system data\n\
as it existed before the system crash.  This information can be\n\
copied to the output device that you specify.\n\n\
Select an output device from the list displayed on the screen\n\
and insert the appropriate media.  Type the number indicating\n\
the device selected and press Enter."

$ NEW SCREEN HELP -fig 4.1
FIG_041 "\
		Help for Installation Warning\n\
\n\
To begin installation using the current settings, type 1 and press Enter.\n\
The current settings are initially set by the system.  If you want to\n\
view or change the settings, go back to the Installation and Maintenance\n\
menu and select option 2, Change/Show Installation Settings and Install."

$ Migration Confirmation Screen Help
23 "\
                  Help for Migration Confirmation\n\n\
The Migration Confirmation screen displays information regarding file \n\
movement and removal, fileset removal, and potential file system expansion.\n\
\n\
To continue with the migration installation, press Enter.\n\
\n\
Type number 1 and press Enter to see which base system configuration files\n\
will be stored in /tmp/bos and not automatically merged into the system.\n\
\n\
Type number 2 and press Enter to see which filesets (installable options)\n\
will be removed from the system and will not be replaced with new filesets.\n\
Base message filesets in the list may be replaced depending on the \n\
language selection.\n\
\n\
Type number 3 and press Enter to see which directories will have the\n\
current contents removed during the migration installation.\n\
\n\
Type number 4 and press Enter to cancel the migration installation.\n\
Place the switch in the service position to reboot from the installation\n\
media or place the switch in the normal position to reboot your system.\n\
\n\
>>> 99  Previous Menu\n\n\
>>> Choice[99]: \
"

$ Messages for errors which occur prior to BosMenus or which are simply
$ informational
$ 
$ NOTE TO TRANSLATOR:  All strings in set 10 can be no longer than three lines
$                      long with 55 columns in each line.

$set 10

$ Do not translate the following word(s) or phras(es):
$	"bosinst.data"
1	"The bosinst.data file specified doing a migration\ninstall, but there is no existing root volume group\nof level 3.2 or greater.\n"

2 "The bosinst.data file specified doing either a migration\nor a preservation install, but there is no existing root\nvolume group.\n"


$ Do not translate the following word(s) or phras(es) from the message:
$	"image.data"
$	"vg_data"
$	"rootvg"
3	"    The image.data file contains no vg_data stanza\n    for rootvg.  The installation cannot continue.\n"

$ Do not translate the following word(s) or phras(es) from the message:
$	"image.template"
4	"    Missing image.template file.  The network install\n    server is not set up correctly.\n"

$ Do not translate the following word(s) or phras(es) from the message:
$	"image.data"
5	"    Missing image.data file.  The tape does not\n    contain a valid install image.\n"

$ Do not translate the following word(s) or phras(es) from the message:
$	"image.data"
$	"CDROM"
6	"    Missing image.data file.  The CDROM does not\n    contain a valid install image.\n"

$ Do not translate the following word(s) or phras(es) from the message:
$	"image.data"
7	"image.data has invalid logical volume data.\nCan not continue.\n"

$ Do not translate the following word(s) or phras(es) from the message:
$	"image.data"
8	"image.data has invalid file system data.\nCan not continue.\n"

$ Do not translate the following word(s) or phras(es) from the message:
$	"mksysb"
$	"SPOT"
9	"The data file indicated that a mksysb install be done\nfrom a SPOT.  This is an illegal operation.\n"

10	"There are no disks on this system which can be booted.\n"

$ Do not translate the following word(s) or phras(es) from the message:
$	"mksysb"
11	"You chose to install only onto disks which are not\ncontained in a volume group, but there are not enough\nof those disks to contain the mksysb image.\n"

12	"You chose to install only onto disks which are not\ncontained in a volume group, but there are none which\ncan be booted.\n"

13	"The data file did not specify enough disk space to\ncontain the operating system.\n"

14	"The data file indicated that all logical volumes should\nbe created exactly as they were before, but the disks\nare not the same as they were on the source system.\n"

15	"The shrink and exact fit operations can not be\nspecified together in the same install.\n"

16 "An invalid disk (%s) was specified in the\nlocation field of the data file."

$ Do not translate the following word(s) or phras(es) from the message:
$	"bosinst.data"
17 "A disk of size %s was specified in the\nbosinst.data file, but there is not a disk of at least\nthat size on the system."

18 "The size field %s is not numeric."

19 "A disk of name %s was not found."

$ Do not translate the following word(s) or phras(es) from the message:
$	"bosinst.data"
20	"Disks specified in bosinst.data do not define a\nroot volume group.\n"

$ Do not translate the following word(s) or phras(es) from the message:
$	"bosinst.data"
21	"The bosinst.data file does not specify any bootable disks.\n"

$ Do not translate the following word(s) or phras(es) from the message:
$	"bosinst.data"
22	"The bosinst.data file indicated that you do not want to\noverwrite an existing system, but disks selected in the\nbosinst.data file specify disks which contain data.\n"

$ Do not translate the following word(s) or phras(es) from the message:
$	"bosinst.data"
23 "    Invalid install method %s specified in the\nbosinst.data file."

$ Do not translate the following word(s) or phras(es) from the message:
$	"bosinst.data"
24 "    Invalid startup flag %s specified in the\nbosinst.data file."

$ Do not translate the following word(s) or phras(es) from the message:
$	"bosinst.data"
$	"X11"
25 "Invalid flag (%s) specified in the bosinst.data file\n\
regarding the installation of X11."

$ Do not translate the following word(s) or phras(es) from the message:
$	"BOS install"
$	"bosinst.data"
26 "Invalid BOS install language (%s) specified in the\nbosinst.data file."

$ Do not translate the following word(s) or phras(es) from the message:
$	"bosinst.data"
27 "Invalid cultural convention (%s) specified in the\nbosinst.data file."

$ Do not translate the following word(s) or phras(es) from the message:
$	"bosinst.data"
28 "Invalid message catalogs (%s) specified in the\nbosinst.data file."

$ Do not translate the following word(s) or phras(es) from the message:
$	"bosinst.data"
29 "Invalid keyboard map (%s) specified in the\nbosinst.data file."

30	"Encountered an unrecoverable error.\n"

31	"Not enough disk space selected to contain the\noperating system.\n"

$ Do not translate the following word(s) or phras(es) from the message:
$ "lv_data"
$ "image.data"
32  "Duplicate lv_data stanzas specified in the image.data\nfile.  The installation cannot continue because data\nmay be lost.\n"

$ Do not translate the following word(s) or phras(es) from the message:
$ "fs_data"
$ "image.data"
33  "Duplicate fs_data stanzas specified in the image.data\nfile.  The installation cannot continue because data\nmay be lost.\n"

$ Do not translate the following word(s) or phras(es) from the message:
$ "target_disk_data"
$ "bosinst.data"
34  "Duplicate target_disk_data stanzas specified in the\nbosinst.data file.\n"

35  "The following disks failed the preliminary\ndiagnostic tests:\n%s."

36  "over mounting /.\n"

37  "Initializing disk environment.\n"

$ Do not translate the "Cu*" in the above phrase.
38  "Copying Cu* to disk.\n"

39  "Merging.\n"

40  "Installing all packages.\n"

41  "Initializing trusted computing base.\n"

42  "Initializing dump device.\n"

43  "Importing other volume groups.\n"

44  "Network installation manager customization.\n"

45  "Creating boot image.\n"

46  "Preparing target disks.\n"

47  "Making paging logical volumes.\n"

48  "Making boot logical volume.\n"

49  "Making logical volumes.\n"

$ Do not translate the word "jfs" in the following sentence.
50  "Forming the jfs log.\n"

51  "Making file systems.\n"

52  "Mounting file systems.\n"

53  "Importing root volume group.\n"

54  "Replaying the log.\n"

55  "Preserving old data.\n"

56  "Copying old files\n."

57  "Creating logical volumes.\n"

58  "Mounting logical volumes.\n"

59  "Restoring base operating system\n"

60  "        Support for Disks not known to Base Operating System Installation\n\n\
 Device Support must be supplied for disks not known to Base Operating\n\
 System Installation.\n\n\n\
 Insert the diskette containing the device support into the diskette drive\n\
 /dev/fd0, and press Enter."

$ Do not translate the word "/tmp" in the following sentences.
61  "  The total space needed to contain the configuration files is\n\
  %s 1024-byte blocks.  The total /tmp space plus the total space not\n\
  already allocated is %s 1024-byte blocks.  The migration cannot\n\
  continue.\n\
  To reboot the system, turn the key to the NORMAL position and\n\
  press Enter.  To reboot from other media in order to select another\n\
  install method, turn the key to the SERVICE position and press Enter: "

$ Do not translate /tmp/bos
62 "\
                           Migration Confirmation\n\n\
  Either type 0 and press Enter to continue the installation, or type the\n\
  number of your choice and press Enter.\n\n\
    1  List the saved Base System configuration files which will not be\n\
       merged into the system.  These files are saved in /tmp/bos.\n\n\
    2  List the filesets which will be removed and not replaced.\n\n\
    3  List directories which will have all current contents removed.\n\n\
    4  Reboot without migrating.\n\n\
>>> 0  Continue with the migration.\n\n\
   88  Help ?\n\n\
+---------------------------------------------------------------------------\n\
  WARNING: Selected files, directories, and filesets (installable options)\n\
    from the Base System will be removed.  Choose 2 or 3 for more information.\n\n\
  %s\n\
>>> Choice[0]: "

$ This message must take up two lines on the menu.  If the translation does
$ not expand beyond the first line, please be certain that the '\\n  \\n  '
$ remains exactly as it is.  If the message does expand beyond the first line
$ in translation, place the rest of the sentence after the first '\\n' but
$ before the second.  For those of you concerned about context, this "WARNING"
$ will appear immediately after the "WARNING" in set 10 message 62 (above).
63  "WARNING: /tmp is currently %s 1024-byte blocks and will be increased to\\n\
              %s 1024-byte blocks."

64  "End of list.  Press Enter  "

65  "Migration could not be completed because /tmp could not be extended.\n\
Reboot by turning the switch to normal and pressing Enter.  "

$ Do not translate /var/adm/ras/devinst.log
66 "Some ILS packages did not install.  See /var/adm/ras/devinst.log \n\
for details"


$ Do not translate /var/adm/ras/devinst.log or /var/adm/sw/installp.summary
67 "An error occurred while migrating packages.\nSome packages have not been installed.\nPlease see /var/adm/ras/devinst.log for details or perform an overwrite\nor preservation install."

68 "The migration cannot continue.\n\
To reboot the system, turn the key to the NORMAL position and\n\
press Enter.  To reboot from other media and select another\n\
install method, turn the key to the SERVICE position and press Enter:\n"

$ Do not translate lpp_name or CDROM.
69 "Cannot restore the lpp_name file from CDROM.\n"

$ Do not translate lpp_name.
70 "Cannot restore the lpp_name file from tape.\n"

$ Do not translate lpp_name.
71 "Cannot restore the lpp_name file from the network.\n"

$ Do not translate BOS, SPOT and lpp_source.
72 "Migration installations from the network are only supported when\n\
the BOS runtime files are in a SPOT or part of an lpp_source resource.\n"

73 "You attempted a migration install from an unsupported device.\n"

$ Do not translate BOS
74 "The BOS pre-installation procedure detected an internal\n\
processing error (%s).\n"

75 "Cannot expand the %s filesystem. An additional\n\
%s 512-byte blocks are required.\n"

$ Do not translate BOS
76 "The following directories conflicted with BOS installation files and\n\
had to be renamed:\n"

$ Do not translate BOS
77 "The following directories conflicted with BOS installation files and\n\
had to be removed:\n"

78 "\n\n\
THIS OPERATING SYSTEM PRODUCT IS NOT SUPPORTED ON THIS\n\
HARDWARE CONFIGURATION.\n\
Contact your Product or Service Representative and report\n\
the model code shown below to assist in obtaining the correct operating\n\
system product for this hardware configuration.\n\
MODEL CODE: %s\n\n\
*** SYSTEM HALTED ***\n\n"

$
$ The first sentence of the previous message should be capitalized when
$ translation permits.

$ The following message must fit on two lines.
79 "WARNING: Trusted Computing Base is disabled on this system and cannot be\n\
enabled by migration installation."

80 "The previous boot image could not be recovered.  Reboot the system in\n\
maintenance mode and use the bosboot command to restore the boot image.\n"

81 "Recovering previous boot image..."

82 "Turbo Installing.\n"

83 "Checking file systems.\n"

84 "Installing root file systems.\n"

85 "Select 1 or 2 to install from tape device %s\n" 

$ Messages for errors occuring after BOS install has exited BosMenus.
$set 11

1	"%s: Could not initialize %s.\nYou may want to run diagnostics on the listed disk(s).\n"

$ Do not translate the following word(s) or phras(es) from the message:
$	"jfs log"
2	"%s: Could not create the jfs log.\n"

3	"%s: No paging devices could be found on the following disks:\n%s\n"

$ Do not translate the following word(s) or phras(es) from the message:
$	"jfs log"
4	"%s: Could not replay the jfs log.\n"

5	"%s: Can not preserve customized object classes of previous system.\nContinuing.\n"

$ Do not translate the following word(s) or phras(es) from the message:
$	"/tmp"
$	"/etc/preserve.list"
6	"%s: After saving all the data from the previous system into /tmp, it\nwas discovered that there will not be enough free space in /tmp to make the\nboot image.  Please reboot in normal mode and increase the size of /tmp or\nreduce the number of files to save as listed in the \"/etc/preserve.list\" file.\n"

7	"%s: Could not remove the %s logical volume.  Can not continue.\n"

8	"%s: You chose to create logical volumes mapped exactly as they\nwere on the previous disks, but there are no map files specified in the\nimage.data file.\n"

9	"%s: Could not create the %s logical volume.\n"

10	"%s: Could not create the %s file system.\n"

11	"%s: Could not create the root volume group.\n"

12	"%s: Although the root volume group was created successfully, it could\nnot be accessed.\n"

$ Do not translate the following word(s) or phras(es) from the message:
$	"CDROM"
13	"%s: Restore of Base Operating System from CDROM failed.\n"

14	"%s: Restore of Base Operating System from %s failed.\n"

15	"%s: Could not populate devices directory.\n"

16	"%s: Could not copy volume group information to the disk.\n"

17	"%s: Could not copy customized device object classes to disk.\n"

18	"%s: Could not create boot image.\n"

19	"%s: Invalid error code %d\n"

20	"%s:  function name required.\n"

21	"\n%s:  invalid action code %d\n"

$ Do not translate the following word(s) or phras(es) from the message:
$	"ID"
22	"   ID#        OPTION\n     1        Continue\n     2        Perform System Maintenance and Then Continue\n   Enter ID number: "

23	"\nusage:  %s -f function  -e error code -a action [ -r return code ]\n"

24 "\n                               Running Customization\n\n\n\n\n        Please wait...\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"

25 "\n                        Installing Base Operating System\n\n\n\n\n        Please wait...\n\n\n\n\n\n\n\n\n\n\n\n\n\n        Approximate     Elapsed time\n     %% tasks complete   (in minutes)\n\n\n"

26 "\n\n\n\n\n        Base Operating System installation is complete.\n\n        Please perform the following steps to activate\n        the changes made during this installation.\n\n"

27 "\n        1. Turn the system key to the NORMAL position.\n        2. Press the ENTER key to restart (reboot) the system.\n\n\n\n\n\n"

28 "%s:  Could not install the %s package.  This package provides\n  the /unix kernel.\n"

29 "%s:  Interrupted by user.\n"

30  "\n                        Installing Base Operating System\n\n\
Turn the system key to the NORMAL position any time before the \n\
installation ends.\n\n        Please wait...\n\n\n\n\n\n\n\n\n\n\n\n\n\
        Approximate     Elapsed time\n     %% tasks complete   (in minutes)\n\n\n"

31 "Recovering volume group information for %s onto disk %s.\n"

32 "Could not import volume group %s.  Continuing post-install processing.\n"

33 "Error in merging the saved and new files.  The running of the system should\nnot be impaired.  For further information, check the log located in\n/var/adm/ras/bosinstlog.\n"

34 "Error in merging the saved and new files.  A successful BOS installation\ncannot be completed with the new version of the file left in place.  For more\ninformation, see the log in /var/adm/ras/bi.log.\n"

35 "\n        1. Remove the installation media.\n        2. Press the ENTER key to restart (reboot) the system.\n\n\n\n\n\n"

36 "Migration menu preparation in progress.\n\n\n\n\n        Please wait...\n\n\n\n\n\n\n\n\n\n\n\n\n\n        Approximate     Elapsed time\n     %% tasks complete   (in minutes)\n\n\n"

$ Do not translate /tmp/bos
37 "Saving system configuration files in /tmp/bos..."

38 "\nRemoving obsolete filesets, directories, and files..."

$ Do not translate /var/adm/ras/devinst.log, or 'exit'.
39 "Basic operating system support could not be installed.\n\
System administrator should see /var/adm/ras/devinst.log for further\n\
information.  Probable cause of failure is insufficient free disk space.\n\
Type '2' to perform system maintenance to correct the problem, then type\n\
'exit' to continue the installation, or restart the installation with\n\
different installation options.\n"


$ set MS_COPYDUMP is for copydumpmenu.c and copydump.c

$set MS_COPYDUMP

COPYDUMP_USAGE            "\
Usage:\n\
copydump dumpdevicename dumpsize locationcode outputdevice [bosinstflag]\n"

ODMINIT_ERR           "\
Unable to initialize ODM\n"

CANT_GET_PVNAME         "\
Unable to get the phyical volume name for\n\
the dump device from ODM.\n"

CANT_IMPORTVG           "\
Unable to import the physical volume for\n\
the dump device.\n"

$ Do not translate rootvg
CANT_VARYONVG           "\
Unable to vary on the rootvg volume group\n\
to access the dump device.\n"


OPEN_ERR                "\
Unable to open the dump device to read.\n"

LSEEK_ERR               "\
Unable to move the file pointer for the dump device.\n"

READ_ERR                "\
Unable to read from the dump device.\n"

INVALID_DUMP            "\
No valid dump in the dump device.\n"

CANT_GET_DEVNAME        "\
Unable to get the device information for\n\
the output device /dev/%1$s.\n"


MKDEV_ERR               "\
Unable to configure the output device /dev/%1$s.\n"

COPY_ERR                "\
Fail to copy the system dump to the output device\n\
/dev/%1$s.\n"

$ this message may use at most 10 lines.
CAT_CD_HEAD             "\
                \Copy a System Dump to Removable Media\n\n\
    The system dump is %d bytes and will be copied from %s\n\
    to media inserted into the device from the list below.\n\n\
    Please make sure that you have sufficient blank, formatted\n\
    media before you continue.\n\n\
    Step One:  Insert blank media into the chosen device.\n\
    Step Two:  Type the number for that device and press Enter."


CAT_HEAD2               "\
           Device Type                   Path Name"

CAT_HELP                "\
    %1$2d  Help ?"

CAT_EXIT1               "\
    %1$2d  Previous Menu"

CAT_EXIT2               "\
    %1$2d  Exit"

CAT_CHOICE              "\
>>> Choice"

$ This message is not used because of defect 140010.  Instead
$ the message for status_str below is used.
STATUS_STR_OLD              "\
        Copy in Progress from %1$s to /dev/%2$s"

$ This message is not used because of defect 140010.  Instead the
$ message for wait_str below is used.
WAIT_STR_OLD                "\
                Please Wait..."

CAT_INV_SEL             "\
Invalid Choice. Please try again."

$ Do not translate /dev
CD_SUCCESS                      "\
System Dump has been copied to /dev/%1$s.\n"

STATUS_STR             "\
Copy in Progress from %1$s to /dev/%2$s"

WAIT_STR               "\
Please Wait..."

$ Do not translate tar
TAR_FORMAT    "\
After reboot, use the tar command to extract the dump.\n"
