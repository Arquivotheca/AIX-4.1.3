# src/bos/usr/games/turnoff.sh, cmdgames, bos411, 9428A410j 6/15/90 21:26:50
#
# COMPONENT_NAME: (CMDGAMES) unix games
#
# FUNCTIONS: turnoff
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
find . -type f -perm 111 -print | xargs chmod 0
