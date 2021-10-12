$ @(#)09	1.6.1.1  src/bos/usr/bin/chhwkbd/chhwkbd.msg, cmdinputdd, bos411, 9434B411a 8/25/94 05:22:34
$
$  COMPONENT_NAME: (CMDINPUTDD) input device commands
$
$  FUNCTIONS:  none
$
$  ORIGINS: 27
$
$  IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$  combined with the aggregated modules for this product)
$                   SOURCE MATERIALS
$  (C) COPYRIGHT International Business Machines Corp. 1993
$  All Rights Reserved
$
$  US Government Users Restricted Rights - Use, duplication or
$  disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$

$quote "


$ ----------------------------------------------------------
$ the following messages are used by the chgkbd smit dialog
$ ----------------------------------------------------------

$set 1

1 "Keyboard repeat rate"
2 "Keyboard repeat delay"
3 "Alarm volume"

$ msg 4 contains four ranges used to select alarm and clicker volume
4 "off,low,medium,high"

5 "Clicker volume"
6 "Extended keyboard identifier"
7 "none,nonum"
8 "Available tty Devices"


$ -------------------------------------------------------
$ the following messages are used by the chhwkbd cmd
$ -------------------------------------------------------

$set CHHWKBD

$ Do not translate the following groups of characters that are used in
$ the messages below:
$          %1$s, %2$d,  %2$s, %3$d,  %4$s, %2$c
$          chhwkbd, -d, -r, -c, -a, -m, -t, KR, JP, TW, nonum
$          CHHWKBD_USAGE, ODM_INIT, ODM_INIT, ODM_NOKBD,
$          BADDELAY, BADRATE, BADCLICK, BADVOL, BADMAP,
$          CFG_MSG, ODM_NOATTR, BADFLAG, PARSE, MFLAG, KBDFAIL
$          SAVEBASE_ERR,  savebase, TFLAG, BADTYPE

CHHWKBD_USAGE "Usage: chhwkbd [-r Rate][-d Delay][-c Click]\n\
\t       [-a Alarm][-m [KR,JP,TW]][-t [nonum]]\n\
\n\
\t-r  Changes keyboard repetition rate. Rate is\n\
\t    an integer from 2 to 30.\n\
\t-d  Changes keyboard repetition delay. Delay values\n\
\t    are 250, 500, 750, and 1000.\n\
\t-c  Changes clicker volume. Click is an integer from\n\
\t    0 (lowest volume) to 3 (highest volume).\n\
\t-a  Changes alarm volume. Alarm is an integer from\n\
\t    0 (lowest volume) to 3 (highest volume).\n\
\t-m  Identifies international keyboard. Follow this flag with KR when\n\
\t    using a Korean keyboard, with JP when Japanese keyboard,\n\
\t    and with TW when Chinese keyboard.\n\
\t-t  Enable/disable ten-key emulation. Follow this flag with nonum when\n\
\t    using ten-key emulation.\n"


ODM_INIT "%1$s: 0832-001 ODM (Object Data Management) database\n\
\tinitialization failed with error code of %2$d.\n"

ODM_FUNC "%1$s: 0832-002 %2$s failed with error code of %3$d while\n\
\taccessing ODM (Object Data Management) %4$s database.\n"

ODM_NOKBD "%1$s: 0832-003 A keyboard must be configured to run this command.\n"

BADDELAY "%1$s: 0832-004 Missing or invalid value specified.\n\
\tAfter the -d flag, specify 250, 500, 750 or 1000.\n"

BADRATE "%1$s: 0832-005 Missing or invalid value specified.\n\
\tAfter the -r flag, specify an integer between 2 and 30.\n"

BADCLICK "%1$s: 0832-006 Missing or invalid value specified.\n\
\tAfter the -c flag, specify an integer between 0 and 3.\n"

BADVOL "%1$s: 0832-007 Missing or invalid value specified.\n\
\tAfter the -a flag, specify an integer between 0 and 3.\n"

BADMAP "%1$s: 0832-008 Value following -m flag is not valid.\n"

CFG_MSG "%1$s: 0832-009 Your changes have been saved and will\n\
\ttake effect after the next system boot.\n"

ODM_NOATTR "%1$s: 0832-010 Could not find %2$s attribute\n\
\tin ODM (Object Data Management) PdDv database.\n"

BADFLAG "%1$s: 0832-011 Invalid flag specified. -%2$c is not valid.\n"

PARSE "%1$s: 0832-012 Command line options are invalid.\n"

MFLAG "%1$s: 0832-013 -m flag is not valid for the configured keyboard.\n"

KBDFAIL "%1$s: 0832-014 Keyboard operation failed with error code of %2$s.\n\
\tYour changes have been saved and will not completely take effect\n\
\tuntil the next system boot.\n"

SAVEBASE_ERR "%1$s: 0832-015 savebase command failed.\n"

BADTYPE "%1$s: 0832-016 Value following -t flag is not valid.\n"

TFLAG "%1$s: 0832-017 -t flag is not valid for the configured keyboard.\n"

