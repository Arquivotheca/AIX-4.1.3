$ @(#)30	1.2  src/bos/usr/lib/lpd/plotgbe/plotgbe.msg, cmdpsla, bos411, 9428A410j 5/2/94 11:19:34
$
$ COMPONENT_NAME: cmdpsla
$
$ FUNCTIONS: message catalog source file for graPHIGS plotter support
$
$ ORIGINS: 27
$
$ This module contains IBM CONFIDENTIAL code. -- (IBM
$ Confidential Restricted when combined with the aggregated
$ modules for this product)
$                  SOURCE MATERIALS
$
$ (C) COPYRIGHT International Business Machines Corp. 1992, 1993
$ All Rights Reserved
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ GENERAL NOTES
$
$ Do not translate %d, %s, %x, %07X, or \t in any messages.  They
$   are used for word or number substitution and are noted in the
$   comments for the individual messages.  The 1$, 2$, 3$, etc,
$   within the substitutions are used to denote the order of the
$   substitutions.
$
$ END OF GENERAL NOTES
 
$quote "
$
$
$ Following codes have no local variables within them.
$
$set PSLA_MISC_MSG
PSLA_NO_STORAGE "Unable to obtain storage \n"
PSLA_VIEW       "   Press REMOTE or VIEW to ready device \n"
PSLA_NOPAPER    "   Load media and ready device \n"
PSLA_BAD_FR     "'fr' option invalid for non-roll feed \n"
PSLA_CLOSEPEN   "   Close pen cover to ready device \n"
PSLA_CLOSEVIEW  "   Close pen cover and press REMOTE or VIEW to ready device \n"
PSLA_CLOSEMSG   "Error closing message catalog \n"
PSLA_NOPAPERCLOSE  "   Load media and close pen cover to ready device \n"
PSLA_CONTINUE   "Load new media to continue \n"
$
$ The following codes have variables substituted within them.
$
PSLA_NOTREADY   "Device attached to '%s' is not ready \n"
PSLA_OPENPORT   "Unable to open port '%s' . Error returned = %d\n"
PSLA_BADOPTION  "Invalid option '%s' on enq command\n"
PSLA_ORG_ERR    "Error setting origin of device attached to '%s' . Error = %d \n"
PSLA_OPEN_FILE  "Error opening file '%s' . Error returned = %d \n"
PSLA_WR_PORT    "Error writing to '%s' - error returned = %d\n"
PSLA_RD_FILE    "Error reading from '%s' - error returned = %d\n"
PSLA_RMVPAPER   "Remove media from device attached to '%s' \n"
PSLA_RS232_ERR  "RS232 error status = %s for device '%s'\n"
PSLA_BAD_STATUS "   Unrecognized status '%c' from device \n"
PSLA_TIME_OUT   \
  "Device attached to '%s' failed to respond - check cable, power, speed \n"
PSLA_NORESPONSE "No response from device attached to '%s'\n"
PSLA_READY      "Ready device to continue or cancel job \n"
PSLA_CANCEL     "Processing of file '%s' is cancelled \n"
PSLA_ENA_QERR   "Error enabling queue for device '%s' \n"
PSLA_BAD_SPEED  "Speed operand '%s' is not valid \n"
PSLA_STAT_ERR   "Unable to get file status '%s' \n"
