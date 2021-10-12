# @(#)92	1.7  src/bos/etc/security/.profile.S, cfgsauth, bos411, 9428A410j 9/3/91 22:38:06
#  COMPONENT_NAME: CFGSAUTH
#
#  ORIGINS: 3, 26, 27
#
#  (C) COPYRIGHT International Business Machines Corp. 1989, 1991
#  All Rights Reserved
#  Licensed Materials - Property of IBM
#
#  US Government Users Restricted Rights - Use, duplication or
#  disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

# Searching the current directory last is usually a BIG time saver.
# Add default directory searches here before the last period
# If /usr/ucb is at the beginning of the PATH the BSD version of commands will be 
# found. 

PATH=/usr/bin:/etc:/usr/sbin:/usr/ucb:$HOME/bin:/usr/bin/X11:/sbin:.

export PATH

if [ -s "$MAIL" ]           # This is at Shell startup.  In normal
then echo "$MAILMSG"        # operation, the Shell checks
fi                          # periodically.
