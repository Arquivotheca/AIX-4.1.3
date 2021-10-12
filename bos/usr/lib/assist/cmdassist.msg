$
$  @(#)75  1.6.1.1  src/bos/usr/lib/assist/cmdassist.msg, cmdassist, bos411, 9430C411a 7/18/94 14:35:17
$
$ COMPONENT_NAME:  cmdassist 
$
$ FUNCTIONS:  Message catalog for cmdassist (Installation Assistant)
$
$ ORIGINS:  27
$
$  (C) COPYRIGHT International Business Machines Corp. 1994
$   All Rights Reserved
$   Licensed Materials - Property of IBM
$   US Government Users Restricted Rights - Use, duplication or
$   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$ --------------------------------------------------------------------


$quote "
$
$set ASSIST_ERR_SET
$
$ These are the messages for the Installation Assistant.
$ Current message numbers used are 0851-001 through 0851-012.
$


$
$ NOTE to translators:
$   The following two messages should be able to be displayed on
$   a 24 row by 80 column screen.
$
$   The 88 and the 99 values cannot be changed.
$
$   The three columns that start with ibm3101, tvi912, and vt330
$   represent terminal types and cannot be translated.
$
$   Placement of blank lines within the message is at the discretion
$   of the translator.  The total number of lines in the message
$   should be 24.
$
$   If the terminal type is not set (that is, the TERM environment
$   variable), the first of these two messages will be displayed
$   before the smit Installation Assistant menus are called.
$
$   If an undefined terminal type is entered, the second of the two
$   messages is displayed to indicate an error and to allow the user
$   to try again.
$

ASSIST_TTY1_I "\
                            Set Terminal Type \n\
The terminal is not properly initialized.  Please enter a terminal type \n\
and press Enter.  Some terminal types are not supported in \n\
non-English languages. \n\n\
     ibm3101          tvi912          vt330 \n\
     ibm3151          tvi920          vt340 \n\
     ibm3161          tvi925          wyse30 \n\
     ibm3162          tvi950          wyse50 \n\
     ibm3163          vs100           wyse60 \n\
     ibm3164          vt100           wyse100 \n\
     ibmpc            vt320           wyse350 \n\
     lft              sun \n\n\
                      +-----------------------Messages------------------------\n\
                      | If the next screen is unreadable, press Break (Ctrl-c)\n\
    88  Help ?        | to return to this screen.\n\
    99  Exit          | \n\
                      | \n\
>>> Choice []: "


ASSIST_TTY2_I "\
                            Set Terminal Type \n\
The terminal is not properly initialized.  Please enter a terminal type \n\
and press Enter.  Some terminal types are not supported in \n\
non-English languages. \n\n\
     ibm3101          tvi912          vt330 \n\
     ibm3151          tvi920          vt340 \n\
     ibm3161          tvi925          wyse30 \n\
     ibm3162          tvi950          wyse50 \n\
     ibm3163          vs100           wyse60 \n\
     ibm3164          vt100           wyse100 \n\
     ibmpc            vt320           wyse350 \n\
     lft              sun \n\n\
                      +-----------------------Messages----------------------- \n\
                      | ERROR:  Undefined terminal type.  Please try again. \n\
    88  Help ?        | \n\
    99  Exit          | If the next screen is unreadable, press Break (Ctrl-c) \n\
                      | to return to this screen. \n\
>>> Choice []: "



$
$ Do not translate the term "install_assist" in any of the messages
$ below.
$

$ In the message below "Add License Passwords for Applications"
$ is the title of a menu.  It must match the corresponding menu
$ title in the sm_assist.msg file.
$
ASSIST_NO_KEYS_I "\
install_assist:  You have indicated that you have no licenses \n\
to add.  You have completed the Add License Passwords for \n\
Applications task.\n"

$ Do not translate "bos.ifor_ls.server" in the message below.
$
ASSIST_INSTL_NETLS_I "\
install_assist:  Please wait.  The bos.ifor_ls.server \n\
package will be installed.\n\n"

$ Do not translate "bos.ifor_ls.server" in the message below.
$
ASSIST_INSTL_NETLS_E "\
0851-001 install_assist:  Concurrent-use and use-once licenses\n\
cannot be added to the system until the bos.ifor_ls.server package\n\
is installed.  An attempt to install this package failed.\n\n"

$
$ NOTE to translators:  BEGIN POPUP MESSAGES
$   The following messages can appear in a smit popup window.
$   No line should be more than 50 characters long in order for
$   it to fit into one line of the popup window on a 80 column
$   display screen.  The information in the popup window can be
$   paged, so there is no limit to the number of lines that can
$   be used.
$

$ Do not translate "bos.sysmgt.sysbr" in the message below.
$
ASSIST_INSTL_SYSBR_E "\
0851-002 install_assist:  The system cannot \n\
be backed up until the bos.sysmgt.sysbr \n\
package is installed.  An attempt to install \n\
this package failed.\n\n"

$ Do not translate "bos.net.tcp.client" in the message below.
$
ASSIST_INSTL_TCP_E "\
0851-003 install_assist:  TCP/IP cannot \n\
be started until the bos.net.tcp.client \n\
package is installed.  An attempt to install \n\
this package failed.\n\n"

$ Do not translate "bos.net.nfs.client" in the message below.
$
ASSIST_INSTL_NFS_E "\
0851-004 install_assist:  NFS cannot \n\
be started until the bos.net.nfs.client \n\
package is installed.  An attempt to install \n\
this package failed.\n\n"

$ In the message below "Set Installation Device"
$ is the title of a menu.  It must match the corresponding menu
$ title in the sm_assist.msg file.
$
$ If the following message is displayed, it will follow one of
$ the error messages: 0851-001, 0851-002, 0851-003, 0851-004, or 0851-013.
$
ASSIST_INSTL_FAIL_I "\
The installation failure may have been caused \n\
by one of the following: \n\
  - The input device/directory is not correct. \n\
    The device may be changed through the Set \n\
    Installation Device menu. \n\
  - The input device does not contain a valid \n\
    installation medium. \n\
  - There is a valid installation medium, but \n\
    it does not contain the necessary packages.  \n\
If you are able to correct this problem, you \n\
may wish to try this function again.\n"


$ The %s in the message below will be replaced with a
$ device name.  The message will be followed by a list of
$ one or more packages.
$
ASSIST_INSTL_PKG_E "\
0851-005 install_assist:  The following packages \n\
could not be installed from device %s.\n"

$ CD-ROM is an abbreviation for compact disc, read-only memory.
$
$ The first %s in the message below will be replaced with a device
$ name.  The second %s will be replaced with a directory name.
$
ASSIST_CDRFS_E "\
0851-006 mount_cd:  Cannot create a \n\
CD-ROM file system for device %s \n\
with mount point %s.\n"

ASSIST_MNT_CD_E "\
0851-007 mount_cd:  Cannot mount the \n\
CD-ROM file system for device %s \n\
on mount point %s.\n"

$  CD is an abbreviation for compact disc.
$
ASSIST_INVALID_CD_E "\
0851-008 mount_cd:  The CD is not a valid \n\
installation CD.\n"

$
$   The above message is the end of the messages that can appear
$   in the smit popup window.
$ NOTE to translators:  END POPUP MESSAGES
$


ASSIST_PGSP_DECREASE_E "\
0851-009 install_assist:  You have requested a paging space \n\
size that is less than the current size.  The paging space \n\
size cannot be decreased from this menu.\n"

$ In the message below "Add/Show Disks for System Storage (rootvg)"
$ is the title of a menu.  It must match the corresponding menu
$ title in the sm_assist.msg file.
$
$ Do not translate "rootvg".
$
ASSIST_FREE_SPACE_E "\
0851-010 install_assist:  You have requested a paging space \n\
size that requires more free space than you have available \n\
in rootvg (the system volume group).\n\n\
If you have available disks that are not yet allocated, you \n\
can add another disk to the rootvg volume group.  \n\
In order to show the disks in rootvg or to add disks from \n\
the available disks, use the Add/Show Disks for System \n\
Storage (rootvg) menu.\n\n\
If you do not choose to add another disk to rootvg, then \n\
the value entered for the new paging space size cannot be \n\
greater than the current paging space size plus the space \n\
available in rootvg.\n"

ASSIST_PGSP_UNKNOWN_E "\
0851-011 install_assist:  An unexplained error occurred \n\
while attempting to increase the paging space.\n"

$ The %d below will be replaced by a numeric value.
$
ASSIST_PGSP_SUCCESS_I "\
install_assist:  The paging space was successfully increased \n\
to %d megabytes.\n"

ASSIST_PGSP_DEFAULT_E "\
0851-012 install_assist:  An unexplained error occurred \n\
while attempting to increase the paging space to the \n\
default size.\n"


$
$ NOTE to translators:
$   The following message should be able to be displayed on
$   a 24 row by 80 column screen.
$
$   The 99 value cannot be changed.
$
$   Placement of blank lines within the message is at the discretion
$   of the translator.  The total number of lines in the message
$   should be 24.
$
$   This message is displayed as help for the Set Terminal Type
$   screen (messages ASSIST_TTY1_I and ASSIST_TTY2_I above).
$
ASSIST_TTYHLP_I "\
                    Help for Set Terminal Type \n\
The terminal type must be correctly set in order for the System\
Management Interface Tool (SMIT) to display readable menus.\n\
If you have a graphics display, then enter \"lft\" for your terminal\
type.\n\
There may be other terminal types in addition to those listed that\
are valid for your system (if the terminal information data is\
installed).  If you enter a terminal type that is not valid, you will\
receive an error, and you can try again.\n\
If you enter a valid terminal type but one that does not correctly\
match your terminal, you may find that the next screen is unreadable.\
If this happens, then press Break to return to the Set Terminal Type\
screen.  For most terminal types, Break is the Ctrl-c key sequence\
(that is, while holding down the Ctrl key, also press the c key).\n\n\n\
>>> 99  Previous Menu \n\
>>> Choice [99]: "


$
$ NOTE to translators:  BEGIN POPUP MESSAGES
$   The following messages can appear in a smit popup window.
$   No line should be more than 50 characters long in order for
$   it to fit into one line of the popup window on a 80 column
$   display screen.  The information in the popup window can be
$   paged, so there is no limit to the number of lines that can
$   be used.
$

$ Do not translate "printers.rte" in the message below.
$
ASSIST_INSTL_PRINTER_E "\
0851-013 install_assist:  These commands\n\
cannot be run until the printers.rte package\n\
is installed.  An attempt to install this\n\
package failed.\n\n"

$
$   The above message is the end of the messages that can appear
$   in the smit popup window.
$ NOTE to translators:  END POPUP MESSAGES
$

ASSIST_CONFIG_NETLS_I "\
\n\ninstall_assist:  No prior configuration of NCS was found for\n\
this system.  Attempting to configure NCS for use of this system\n\
as an iFOR/LS license server.\n"

ASSIST_CONFIG_NETLS_E "\
0851-014 install_assist:  An error occurred while attempting\n\
to configure this system as a standalone iFOR/LS license server.\n\
Consult the iFOR/LS product documentation for more specific\n\
information about any errors displayed by the configuration\n\
procedure.\n"

ASSIST_START_NETLS_I "\
\n\ninstall_assist:  An existing configuration of NCS was detected\n\
for this system.  Attempting to start the iFOR/LS license server\n\
with the current configuration.\n\n"

ASSIST_START_NETLS_E "\
\n0851-015 install_assist:  Unable to start the iFOR/LS license\n\
server.  Consult the iFOR/LS product documentation for more\n\
specific information.\n" 

$
$ This is the first message that will be displayed to the user if
$ The GUI helpvolumes (.sdl files) for their language are
$ not installed. 
$
ASSIST_LANG_I "\
 The message files for the Graphical Installation Assistant are not\n\
 present on this system for your language.  Would you like to use\n\
 the Ascii Installation Assistant in your language, or the Graphical\n\
 Installation Assistant in English only?\n\n\
 1  Ascii Installation Assistant in your language\n\
 2  Graphical Installation Assistant in English\n\n\
>>> Choice []: "
