#!/bin/bsh
# @(#)99        1.19  src/bos/diag/diags/diagipl.sh, diagsup, bos411, 9430C411a 7/28/94 15:49:08
#
# COMPONENT_NAME: CMDDIAG  DIAGNOSTIC SUPERVISOR
#
# FUNCTIONS: Service IPL Diagnostic invoke script
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1989, 1994
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
###################################################################
# diagipl: script executed by rc.boot during IPL.
#
#          The environment variable DIAG_ENVIRONMENT is initially set to
#          EXENV_STD which indicates Standalone Diagnostics.
#
#          Next the Diagnostic Supervisor is called.
#          Upon return a 'shutdown -F'
#          command is executed.
#

# Set up to trap interrupts ( INT, QUIT, TERM )
trap 'RESPAWN_FLAG=0' 2 3 15

# Set execution environment
DIAG_ENVIRONMENT=2
DIAG_DISKETTE=0
DIAG_KEY_POS=1

# Now set up basic environment variables
DIAGNOSTICS=/usr/lpp/diagnostics

# Diagnostic Applications path
DADIR=$DIAGNOSTICS/da
DIAGX_SLIH_DIR=/usr/lpp/diagnostics/slih
DIAG_UTILDIR=$DIAGNOSTICS/bin

# Diagnostic data directory - diagnostic conclusion files, etc
DIAGDATA=/etc/lpp/diagnostics
DIAGDATADIR=$DIAGDATA/data

# Object class repository directory
: ${ODMDIR:=/etc/objrepos}

# Set up the boot environment
IPLSOURCE_DISK=0
IPLSOURCE_CDROM=1
IPLSOURCE_TAPE=2
IPLSOURCE_LAN=3

BOOTYPE=`bootinfo -t`
case $BOOTYPE in
          1)    DIAG_IPL_SOURCE=$IPLSOURCE_DISK;;
          3)    DIAG_IPL_SOURCE=$IPLSOURCE_CDROM;;
          4)    DIAG_IPL_SOURCE=$IPLSOURCE_TAPE;;
          5)    DIAG_IPL_SOURCE=$IPLSOURCE_LAN;;
          *)    ;;
esac
export DIAG_IPL_SOURCE

# Default terminal type
TRM=`/usr/bin/tty`
case $TRM in
        /dev/con* ) : ${TERM:=dumb};;
        /dev/tty* ) : ${TERM:=dumb};;
        /dev/lft* ) : ${TERM:=lft};;
        * ) : ${TERM:=dumb};;
esac

# Search path for the catalog files
if [ "X$LANG" = X -o "$LANG" = C ]; then
        LANG=En_US
fi
: ${NLSPATH:=/usr/lib/nls/msg/%L/%N:/usr/lib/nls/msg/prime/%N}
NLSPATH=$DIAGNOSTICS/catalog/%L/%N:$DIAGNOSTICS/catalog/prime/%N:$NLSPATH

# Search path for the shared libraries
LIBPATH=/lib:/usr/lib

export TERM DIAGNOSTICS DIAGDATADIR DIAGDATA
export NLSPATH DADIR LANG ODMDIR DIAGX_SLIH_DIR DIAG_UTILDIR
export DIAG_ENVIRONMENT DIAG_DISKETTE DIAG_KEY_POS
export LIBPATH

# Execute the Diagnostic Supervisor with any parameters passed on
RESPAWN_FLAG=1
while :
do
        $DIAGNOSTICS/bin/diags $*

        # if rc=201 then user hit <ctrl-c> to reset terminal and restart
        if [ $? = 201 ]
        then
                # Default terminal type
                TRM=`/usr/bin/tty`
                case $TRM in
                        /dev/con* ) : ${TERM:=dumb};;
                        /dev/tty* ) : ${TERM:=dumb};;
                        /dev/lft* ) : ${TERM:=lft};;
                        * ) : ${TERM:=dumb};;
                esac
        else
                if [ $RESPAWN_FLAG = 1 ]; then
                        /usr/sbin/shutdown -F
                        exit 0
                fi
        fi
        RESPAWN_FLAG=1
done
