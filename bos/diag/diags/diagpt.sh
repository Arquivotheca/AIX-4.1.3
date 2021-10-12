#!/bin/bsh
# @(#)00	1.19  src/bos/diag/diags/diagpt.sh, diagsup, bos411, 9428A410j 4/4/94 17:29:40
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
# diagpretest: script executed by rc.boot during IPL.
# 	   This script is executed only when booting in Service Mode. 
#
#	   The environment variable DIAG_ENVIRONMENT is initially set to
#	   EXENV_IPL which indicates Standalone Diagnostics booting in
#	   IPL phase. 
#
#	   The environment variable DIAG_DISKETTE is set to TRUE if running
#	   from the standalone package. The variable is set in the rc.boot 
#	   script on the standalone package. Otherwise, this variable is
#	   set to FALSE.

# Set execution environment
DIAG_ENVIRONMENT=1
: ${DIAG_DISKETTE:=0}

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

# Default terminal type - the console is not used.
: ${TERM:=dumb}

# Search path for the catalog files
if [ "X$LANG" = X -o "$LANG" = C ]; then
	LANG=En_US
fi
NLSPATH=$DIAGNOSTICS/catalog/%L/%N:$DIAGNOSTICS/catalog/prime/%N:$NLSPATH

# Search path for the shared libraries
: ${LIBPATH:=/lib:/usr/lib}

export TERM DIAGNOSTICS DIAGDATADIR DIAGDATA
export NLSPATH DADIR LANG ODMDIR DIAGX_SLIH_DIR DIAG_UTILDIR
export DIAG_ENVIRONMENT DIAG_DISKETTE
export LIBPATH

# Execute the Diagnostic Controller with the -D
#   If input argument is null
# 	Run those devices flagged with PDiagDev.PreTest = [1,2,...] will be tested.
#   If input argument is 1
# 	Those devices that are flagged (PDiagDev.PreTest = 1) will be tested.
#   If input argument is 2
# 	Those devices that are flagged (PDiagDev.PreTest = 2) will be tested.
#   If input argument is -d device name
# 	Test that selected device in Pretest mode
#
# 	NO menus, prompts, ... will be displayed.
# 	Problems are reported with the LEDS.
# 	The system will be halted if a problem is found.

if [ $# -gt 0 ]
then
	case $1 in
		-d) $DIAGNOSTICS/bin/dctrl -D 1 -d $2
			exit $?
			;;
		[0-9]) $DIAGNOSTICS/bin/dctrl -D $1 
			exit $?
			;;	
	esac
elif [ -s $DIAGDATADIR/fastdiag ]
then
	selection=`cat $DIAGDATADIR/fastdiag`
	case $selection in
		[1-4])	exit 0 ;;
	esac	
fi

# If called with no options - then test all devices
# with the PRETEST bit set.
$DIAGNOSTICS/bin/dctrl -D 0 
exit $?
