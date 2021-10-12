#!/usr/bin/ksh
# @(#)62	1.2  src/bos/etc/security/migration/secur_mig.sh, cfgsauth, bos411, 9428A410j 5/17/94 13:25:53
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

export MIGDIR=/usr/lpp/bos/migrate

function pathconv
{
    /usr/bin/sed -f $MIGDIR/newpaths.sed $1 > $1.new
    /usr/bin/rm -f $1
    /usr/bin/mv $1.new $1
}

if [ "$BOSLEVEL" != "3.2" ]; then
    exit 0
fi

cd /tmp/bos/etc/security

# /etc/security/limits
/usr/bin/sed "s/multiples of blocks/multiples of 512 byte blocks/" limits > limits.new
/usr/bin/rm -f limits
/usr/bin/mv limits.new limits
echo "nobody:\n\nlpd:\n" >> limits

# The remaining merges are in the 'audit' dir.
cd audit

# /etc/security/audit/bincmds
pathconv bincmds

# /etc/security/audit/events
pathconv events
/usr/bin/sed -f $MIGDIR/events.sed events > events.new
/usr/bin/rm -f events
/usr/bin/mv events.new events

# /etc/security/audit/streamcmds
pathconv streamcmds

exit 0
