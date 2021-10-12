#!/usr/bin/ksh
# @(#)45	1.2  src/bos/etc/security/migration/mkusr_mig.sh, cfgsauth, bos411, 9428A410j 2/21/94 12:44:44
#
#   COMPONENT_NAME: CFGSAUTH
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1994
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

if [ "$BOSLEVEL" != "3.2" ]; then
    exit 0
fi

cd /tmp/bos/usr/lib/security

# /usr/lib/security/mkuser.default
/usr/bin/sed "
s%^[[:space:]]*group[[:space:]]*=%	pgrp =%
s%^[[:space:]]*prog[[:space:]]*=[[:space:]]*/bin/%	shell = /usr/bin/%
s%^[[:space:]]*prog[[:space:]]*=%	shell =%
s%^[[:space:]]*home[[:space:]]*=[[:space:]]*/u/%	home = /home/%
" mkuser.default > mkuser.default.new
/usr/bin/rm -f mkuser.default
/usr/bin/mv mkuser.default.new mkuser.default

exit 0
