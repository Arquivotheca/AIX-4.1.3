$ @(#)08	1.3  src/bos/usr/bin/common/lftcmds.msg, cmdlft, bos411, 9428A410j 12/6/93 10:49:54
$
$   COMPONENT_NAME: CMDLFT
$
$  FUNCTIONS:  none
$
$   ORIGINS: 27
$
$  IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$  combined with the aggregated modules for this product)
$                   SOURCE MATERIALS
$  (C) COPYRIGHT International Business Machines Corp.  1993
$  All Rights Reserved
$
$  US Government Users Restricted Rights - Use, duplication or
$  disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$

$quote "

$ **************************************************************************
$set COMMON
$ Do not translate  %1$s, %2$s, %3$s, %4$s, LFT, ODM

BADFLAG "%1$s: 0468-001 Flag %2$s is not recognized.\n"
$ The necessary usage message follows this message.

NOT_LFT "%1$s: 0468-002 Run this command only at an LFT workstation.\n"

ODM_INIT "%1$s: 0468-003 Unable to initialize the ODM\n\
\t(Object Data Management) database.\n"

$ This message is used whenever an odm call returns a -1.
ODM_FAIL "%1$s: 0468-004 ODM (Object Data Management) database call %2$s\n\
\tfailed to access the database class %3$s using the criteria:\n\t%4$s.\n"

$ This message is used whenever an object is not found in the odm
ODM_NOOBJ "%1$s: 0468-005 ODM (Object Data Management) database call %2$s\n\
\tfailed to acquire an object from class %3$s using the criteria:\n\t%4$s.\n"

NO_DFLT "%1$s: 0468-006 No default display for LFT was found in the ODM.\n"

NO_SYS_MEM "%1$s: 0468-007 Could not allocate system memory\n"


$ **************************************************************************
$set CHDISP
$ Do not translate chdisp,lsdisp, -p, -d, %1$s,LFT,IPL,ioctl.

CHDISP_PGM "chdisp"

CHDISP_USAGE "Usage: chdisp [-dDeviceName] [-pDeviceName]\n\
\tChanges the default display.\n\
\n\
\t-d Changes the default display for this session.\n\
\t-p Changes the default display in the database.  This\n\
\t   is effective at the next IPL.\n\
\tDeviceName is the logical name of the display.  This is\n\
\tthe name in the first column of output of the lsdisp command.\n"

CHDISP_NO_DISP "chdisp: 0468-020 Display %1$s is not available on your system.\n\
\tUse the lsdisp command to see a list of available displays.\n"

CHDISP_WRITE "chdisp: 0468-021 Write to /dev/lft failed.\n"

CHDISP_ID "chdisp: 0468-022 Use a display name or number after the flag.\n\
\tUse the lsdisp command to see a list of installed displays.\n"

CHDISP_IOCTL "chdisp: 0468-023 LFT set default display ioctl failed.\n"
$ The ioctl that failed is the "set default display" ioctl.



$ **************************************************************************
$set LSDISP
$ Do not translate lsdisp,LFT

LSDISP_PGM "lsdisp"

LSDISP_USAGE "Usage: lsdisp [-l]\n\
\tLists the displays currently available on the system.\n\n\
\t-l List only option. No header information will be displayed.\n"

LSDISP_NODSP "lsdisp: 0468-030 No displays found for the LFT\n"

$ the following message strings are for the lsdisp header
LSDISP_DEV_NAME "DEV_NAME"
$ Do not translate DEV_NAME
LSDISP_SLOT "SLOT"
$ Do not translate SLOT 
LSDISP_BUS "BUS"
$ Do not translate BUS
LSDISP_ADPT_NAME "ADPT_NAME"
$ Do not translate ADPT_NAME
LSDISP_DESC "DESCRIPTION"
$ Do not translate DESCRIPTION



$ **************************************************************************
$set CHFONT
$ Do not translate chfont, IPL

CHFONT_PGM "chfont"

CHFONT_USAGE "Usage: chfont FontID\n\
\tChanges the font currently used on the default display to the \n\
\tfont referenced by FontID. The FontID can be obtained by executing\n\
\tthe lsfont command. The new font will not become active until the\n\
\tnext system IPL.\n"

CHFONT_ID "chfont: 0468-040 FontID %1$d is not valid.\n\
\tUse the lsfont command for a list of available fonts.\n"



$ **************************************************************************
$set LSFONT
$ Do not translate lsfont,%1$s

LSFONT_PGM "lsfont"

LSFONT_USAGE "Usage: lsfont [-l]\n\
\tLists the fonts currently available on the system.\n\n\
\t-l List only option. No header information will be displayed.\n"

LSFONT_NOFILE "lsfont: 0468-050 Failed to open the font file %1$s.\n"

LSFONT_NOSTAT "lsfont: 0468-051 The font file %1$s is not valid.\n"

LSFONT_NOREAD "lsfont: 0468-052 Failed to read the font file %1$s.\n"

LSFONT_BADSIZE "lsfont: 0468-053 The font file %1$s is not the correct size.\n"

$ the following message strings are for the lsfont header
LSFONT_FONT "FONT"
$ Do not translate FONT 
LSFONT_FILE "FILE"
$ Do not translate FILE 
LSFONT_GLYPH "GLYPH"
$ Do not translate GLYPH
LSFONT_ID "ID"
$ Do not translate ID
LSFONT_NAME "NAME"
$ Do not translate NAME 
LSFONT_SIZE "SIZE"
$ Do not translate SIZE 
LSFONT_ENCODING "ENCODING"
$ Do not translate ENCODING



$ **************************************************************************
$set MKFONT
$ Do not translate mkfont,LFT,SNF, %1$s

MKFONT_PGM "mkfont"

MKFONT_USAGE "Usage: mkfont File\n\
\tAdds a font to the system.\n\
\n\
\tFile is the absolute path name of a file containing a font\n\
\tin SNF format.\n"

MKFONT_BADPATH "mkfont: 0468-060 The pathname %1$s must be an absolute path.\n"

MKFONT_NOFILE "mkfont: 0468-061 Failed to open the font file %1$s.\n"

MKFONT_NOSTAT "mkfont: 0468-062 The font file %1$s is not valid.\n"

MKFONT_NOREAD "mkfont: 0468-063 Failed to read the font file %1$s.\n"

MKFONT_BADSIZE "mkfont: 0468-064 The font file %1$s is not the correct size.\n"

MKFONT_VARWIDTH "mkfont: 0468-065 LFT does not support variable width fonts\n" 

MKFONT_BADINFO "mkfont: 0468-066 Font information from file %1$s is invalid\n"

$ **************************************************************************
$set CHKBD
$ Do not translate chkbd,LFT,IPL,%1$s

CHKBD_PGM "chkbd"

CHKBD_USAGE "Usage: chkbd File\n\
\tChanges the software keyboard map loaded by the LFT at IPL time.\n\
\n\
\tFile is the path/file name of a valid software keyboard file.\n\
\tIf File is not an absolute path then the default directory \n\
\t/usr/lib/nls/loc will be used as the pathname and File will be\n\
\tused as the filename.\n"

CHKBD_NOSTAT "chkbd: 0468-070 The software keyboard file %1$s is not valid.\n"

$ **************************************************************************
$set LSKBD
$ Do not translate lskbd,LFT,IPL,%1$s

LSKBD_PGM "lskbd"

LSKBD_USAGE "Usage: lskbd\n\
\tLists the software keyboard map loaded by the LFT at IPL time.\n"

LSKBD_LIST "\nThe current software keyboard map = %1$s.\n"
