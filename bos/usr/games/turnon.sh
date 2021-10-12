# src/bos/usr/games/turnon.sh, cmdgames, bos411, 9428A410j 6/15/90 21:26:54
#
# COMPONENT_NAME: (CMDGAMES) unix games
#
# FUNCTIONS: turnon
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1985, 1989
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

# Enter any new games with mode 111

cd /usr/games
find . -perm 0 -print | xargs chmod 111
