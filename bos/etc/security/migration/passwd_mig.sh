#!/usr/bin/ksh
# @(#)05	1.1  src/bos/etc/security/migration/passwd_mig.sh, cfgsadm, bos411, 9428A410j 2/24/94 13:28:41
#
#   COMPONENT_NAME: CFGSADM
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

function dircheck
{
    # if Directory then dont alter path.
    # "-d" follows symlinks, so "-L" is needed.
    if ( [ -d $1 ] && [ ! -L $1 ] ); then
	return 1
    fi

    # if Link, then only alter if it points to the right place.
    if [ -L $1 ]; then
	if ( /usr/bin/ls -l $1 | /usr/bin/grep -q " $2$" ); then
	    return 0
	else
	    return 1
	fi
    fi

    # At this point, $1 is not a link or a directory.
    # It is either non-existent or irrelevant at this point,
    # so make the path alteration.
    return 0
}

if [ "$BOSLEVEL" != "3.2" ]; then
    exit 0
fi

cd /tmp/bos/etc

# /etc/passwd
SUB_GUEST=""
SUB_ADM=""
DO_SUB=0

if ( dircheck /usr/guest /home/guest ); then
    SUB_GUEST="s%:/usr/guest:%:/home/guest:%"
    DO_SUB=1
fi
if ( dircheck /usr/adm /var/adm ); then
    SUB_ADM="s%:/usr/adm:%:/var/adm:%"
    DO_SUB=1
fi

if [ $DO_SUB -ne 0 ]; then
    /usr/bin/sed "$SUB_GUEST;$SUB_ADM" passwd > passwd.new
    /usr/bin/rm -f passwd
    /usr/bin/mv passwd.new passwd
fi

exit 0
