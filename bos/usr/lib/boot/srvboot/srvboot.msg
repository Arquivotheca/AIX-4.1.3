$ @(#)98  1.3.1.1  src/bos/usr/lib/boot/srvboot/srvboot.msg, bosboot, bos41J, 9516B_all 4/21/95 11:24:47
$
$ COMPONENT_NAME: (BOSBOOT) Base Operating System Boot
$
$ FUNCTIONS: srvboot.msg
$
$ ORIGINS: 27
$
$ (C) COPYRIGHT International Business Machines Corp. 1991, 1994
$ All Rights Reserved
$ Licensed Materials - Property of IBM
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$

$quote "

$set 10

100 "\n\
DIAGNOSTIC OPERATING INSTRUCTIONS\n\n\
POWERstation and POWERserver\n\
LICENSED MATERIAL and LICENSED INTERNAL CODE - PROPERTY OF IBM.\n\
(C) COPYRIGHTS BY IBM AND BY OTHERS 1982,1991-1994.\n\
ALL RIGHTS RESERVED.\n\n"
$
$  Do not translate the copyright statements
$
$
$  Messages which follow are for the function selection menu
$  which originate from diagnostics menu.
$  They are used by srvboot.sh when booted in service mode.
$

102 "These programs allow you to enter diagnostics, service aids, single\n\
user mode, or low-level maintenance mode.  The diagnostics should be\n\
used whenever problems with the system occur which have not been\n\
corrected by any software application procedures available.\n\n\
In general, the diagnostics will run automatically.  However, sometimes\n\
you will be required to select options, inform the system when to\n\
continue, do simple tasks, and exchange diskettes.\n\n\
Several keys are used to control the diagnostics:\n\
- The Enter key continues the procedure or performs an action.\n\
- The Backspace key allows keying errors to be corrected.\n\
- The cursor keys are used to select an option.\n\n\
To continue, press Enter.\n"
$
$ The message above is almost the same as the message in diags.cat
$ and may need very little change in translation
$

104 "FUNCTION SELECTION\n\n\
1. Diagnostic Routines\n\
    This selection will test the machine hardware. A problem will be \n\
    indicated by a SRN (Service Request Number).\n\
2. Service Aids\n\
    This selection will look at the machine configuration, exercise\n\
    external interfaces, format media, look at past diagnostic\n\
    results, control what resources are tested, check out media, etc.\n\
3. Advanced Diagnostic Routines\n\
    This selection will normally be used by the service representative.\n\
4. System Exerciser\n\
    This selection will test resources running in an overlap mode.\n\
5. Single User Mode\n\
    The system will enter single-user mode for software maintenance.\n\
\n\n\n\n\n\n\n\nTo make a selection, type the number and press Enter [1]:"

$
$  Options 1,2,3,4 are the same as in diag.cat.
$  Options 5 is a new option.
$

106 "\nDiagnostics not installed.  Press Enter to continue.\n"
108 "\t\t\t\tWARNING!\n\n\
Any type of system use in this environment is NOT recommended\n\
and NOT supported!  Proceed at your own risk.\n\n\
Exit from this shell to return to the main menu.\n\n"
110 "The system will now continue the boot process.  Please wait..."

112 "FUNCTION SELECTION\n\n\
1. Diagnostic Routines\n\
    This selection will test the machine hardware. A problem will be \n\
    indicated by a SRN (Service Request Number).\n\
2. Service Aids\n\
    This selection will look at the machine configuration, exercise\n\
    external interfaces, format media, look at past diagnostic\n\
    results, control what resources are tested, check out media, etc.\n\
3. Advanced Diagnostic Routines\n\
    This selection will normally be used by the service representative.\n\
4. Single User Mode\n\
    The system will enter single-user mode for software maintenance.\n\
\n\n\n\n\n\n\n\n\n\nTo make a selection, type the number and press Enter [1]:"

$
$  Options 1,2,3 are the same as in diag.cat.
$  Option 4 is not available on RSPC systems.
$  Options 5 is a new option.
$

$ End of srvboot.msg
