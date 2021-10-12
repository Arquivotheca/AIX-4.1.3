$ @(#)25	1.13  src/bos/usr/lib/methods/common/console.msg, cfgmethods, bos411, 9428A410j 2/1/94 10:43:49
$
$ COMPONENT_NAME: (CFGMETHODS) console command messages
$
$ FUNCTIONS:
$
$ ORIGINS: 27
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1989, 1994
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$
$ messages for command iostat
$

$set MS_CONSOLE

$quote "
$ messages for the swcons command
USAGESW		"usage: swcons [pathname] \n"
SWCONPRIM	"swcons: console output redirected to: %s\n"
SWCONDFLT	"swcons: returning console to: %s\n"
SWCONE01	"0514-400 swcons: device or file pathname must be fully qualified \n"
SWCONE02	"0514-401 swcons: stat on %s failed, errno = %d\n"
SWCONE03	"0514-402 swcons: invalid request - the console cannot be redirected to itself \n"
SWCONE04	"0514-403 swcons: switch command to driver failed, errno = %d\n"
SWCONE05	"0514-404 swcons: unable to access new console selection \n\t returning console to: %s\n"
SWCONE06	"0514-405 swcons: invalid request - the console cannot be redirected\n\t\t to a block device \n"

$ messages for the chcons command

USAGECHC	" usage: chcons [-a login=enable | disable] [pathname] \n"
CHCONPATH	"chcons: console assigned to: %s, effective on next system boot\n"
CHCONLOGIN	"chcons: console login: %sd, effective on next system boot\n"
CHCONE01	"0514-410 chcons: device or file pathname must be fully qualified \n"
CHCONE02	"0514-411 chcons: stat on %s failed, errno = %d\n"
CHCONE03	"0514-412 chcons: invalid request - the console cannot be redirected to itself \n"
CHCONE04	"0514-413 chcons: failure accessing the device database (ODM) \n"
CHCONE05	"0514-414 chcons: failure accessing the lft0 object in ODM \n"
CHCONE06	"0514-415 chcons: failure accessing the sys0 object in ODM \n"
CHCONE07	"0514-416 chcons: savebase of sys0 object failed \n"
CHCONE08	"0514-417 chcons: invalid request - Console login is enabled and \n\t\t the specified device is not an available lft or tty. \n"
CHCONE09	"0514-418 chcons: Console login cannot be enabled because the console device \n\t\t is not an available lft or tty. \n"
CHCONE10	"0514-419 chcons: invalid request - the console cannot be redirected\n\t\t to a block device \n"

$ messages for the cfgcon program
CFCONE01	"0514-420 cfgcon: failure accessing the device database (ODM) \n"
CFCONE02	"0514-421 cfgcon: failure accessing the sys0 object in ODM \n"
CFCONE03	"0514-422 cfgcon: failure accessing the lft0 object in ODM \n"
CFCONE04	"0514-423 cfgcon: failed to set physical display on LFT \n"));
CFCONE05	"0514-424 cfgcon: switch command to console driver failed \n"
CFCONE06	"0514-425 cfgcon: console is not defined and no candidate terminals were detected \n"
CFCONE07	"0514-426 cfgcon: a system failure has occurred (malloc failure) \n"
CFGCTTY		"\n\n\r ******* Please define the System Console. *******\n\n\r Type a %d and press Enter to use this terminal as the\n  system console. \n\n\n\n\n"
CFGCHFT		"\n\n\r ******* Please define the System Console. *******"
CFGCHFT1	"\n\n\r Type the F%d key and press Enter to use this display as\n\
  the system console.\n\t\t\t\t\r"
CFGCHFT2	""
CFGCLIC1	"\n\r HARDWARE SYSTEM MICROCODE\n\r Licensed Internal Code - Property of IBM \n\r (C) Copyright IBM Corp. 1990, 1994.\n\r All rights reserved.\n\n\r US Government Users Restricted Rights -"
CFGCLIC2	"\n\r Use, duplication or disclosure restricted\n\r by GSA ADP Schedule Contract with IBM Corp.\n\n\n\r"

$ messages for the lscons program
USAGELSC	" usage: lscons [-s] [-b] [-d] \n"
LSCONE01	"0514-430 lscons: failure accessing the device database (ODM) \n"
LSCONE02	"0514-431 lscons: system error occurred accessing console driver \n"
