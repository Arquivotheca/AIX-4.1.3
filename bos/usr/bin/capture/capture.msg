$ @(#)70	1.4  src/bos/usr/bin/capture/capture.msg, cmdsh, bos411, 9428A410j 7/7/94 14:21:54
$
$ COMPONENT_NAME: (CMDSH) Bourne shell and related commands
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
$ messages for capture
$

$set MS_CAPTURE
$ Do not translate script

$quote "
INV_OCT_ESC     "Invalid octal escape value %s\n"
$ INV_OCT_ESC   The value in the SCREENDUMP shell variable was not a numeric
$ INV_OCT_ESC   string. The user should verify that the string displayed
$ INV_OCT_ESC   by the message (<arg 1>) is a string of octal digits.
$ INV_OCT_ESC   The escape character is now a Control-P (^P).
USAGE           "Usage: capture [ -a ] [ %s ]\n"
CAPTURESTART    "Capture command is started. The file is %s.\n"
CAPTURESTART1   "Use %s to dump screen to file %s.\n"
CAPTURESTART2	"You are now emulating a vt100 terminal.\n"
CAPTURESTART3   "Press Any Key to continue.\n"
FORK            "fork"
$ Do not translate.
STARTCAP        "Capture command is started on %s."
ENDCAP          "\nCapture command is complete on %s."
DONECAP         "Capture command is complete. The file is %s.\n"
DONECAP2	"You are NO LONGER emulating a vt100 terminal.\n"
NOPTY           "Out of pty's.\n"
VT100_XLATE     "vtif(vt1): xlate invalid = %d\n"
$ VT100_XLATE     This error message indicates that something is out of
$ VT100_XLATE     sync in vt1().  vt1() is coded such that this message
$ VT100_XLATE     cannot occur.
$ This is an obsolete message. Do not translate.
CHMOD_ERR	"chmod failed to change permissions of tty. Errno = %d.\n"
$ do not translate chmod
CHOWN_ERR	"chown failed to change ownership of tty. Errno = %d.\n"
$ do not translate chown
ACLSET_ERR	"acl_set failed to change permissions of tty. Errno = %d.\n"
$ do not translate acl_set
