#!/bin/bsh
# @(#)73	1.8  src/bos/diag/diags/diagdskpt.sh, cmddiag, bos412, 9445B412 11/7/94 13:59:19
#
# COMPONENT_NAME: CMDDIAG  DIAGNOSTIC SUPERVISOR
#
# FUNCTIONS: Diagnostic Disks pre-test during install
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1993, 1994
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#                                                                   
###################################################################
# 	   This script is executed only when booting in Service Mode. 
#
#	   The environment variable DIAG_ENVIRONMENT is initially set to
#	   EXENV_IPL which indicates Standalone Diagnostics booting in
#	   IPL phase. 
#
#	   The environment variable DIAG_DISKETTE is set to TRUE if running
#	   from the diskette package. The variable is set in the rc.boot 
#	   script on the diskette package. Otherwise, this variable is
#	   set to FALSE.
#	   This script returns 1 if diagnostics detects a problem testing
#	   the resource, 0 if there is no problem.
#	   
# Check to see if running on rspc

pcmodel=`bootinfo -T 2>&1`
if [ "$pcmodel" = "rspc" ]
then
	exit 0
fi

# Set execution environment
DIAG_ENVIRONMENT=1

# Now set up basic environment variables
DIAGNOSTICS=/usr/lpp/diagnostics

# Diagnostic Applications path
DADIR=$DIAGNOSTICS/da

# Diagnostic data directory - diagnostic conclusion files, etc
DIAGDATA=/etc/lpp/diagnostics
DIAGDATADIR=$DIAGDATA/data
DIAG_UTILDIR=$DIAGNOSTICS/bin
DIAGX_SLIH_DIR=/usr/lpp/diagnostics/slih
SHOWLEDS=0

# Object class repository path
: ${ODMPATH:=/etc/objrepos:/usr/lib/objrepos}

# Default terminal type - the console is not used.
: ${TERM:=dumb}

# Search path for the catalog files
if [ "X$LANG" = X -o "$LANG" = C ]; then
	LANG=En_US
fi
NLSPATH=$DIAGNOSTICS/catalog/%L/%N:$DIAGNOSTICS/catalog/prime/%N:$NLSPATH

# Search path for the shared libraries
LIBPATH=/lib:/usr/lib:$DIAGNOSTICS/lib

export TERM DIAGNOSTICS DIAGDATADIR DIAGDATA
export NLSPATH DADIR LANG ODMPATH DIAG_UTILDIR DIAGX_SLIH_DIR
export DIAG_ENVIRONMENT SHOWLEDS
export LIBPATH

for i in $*
do

	uniquetype=`odmget -q"name=$i" CuDv | grep PdDvLn | cut -f2 -d"\""`
	dtype=`echo $uniquetype | cut -f3 -d"/"`
	dsclass=`echo $uniquetype | cut -f2 -d"/"`
	dclass=`echo $uniquetype | cut -f1 -d"/"`
	# skip disk pre-test for os disk.
	if [ $dtype = "osdisk" ]
	then
		continue
	fi
	da=`odmget -q"DType=$dtype and DSClass=$dsclass and DClass=$dclass" \
		PDiagDev | grep -v "EnclDaName" | grep DaName | cut -f2 -d"\""`
	if [ -f $DADIR/$da ]
	then
		$DIAGNOSTICS/bin/dctrl -c -d $i  >/dev/null 2>&1
		if [ $? -eq 1 ]
		then
		    exit 1
		fi
	fi
done
exit 0
