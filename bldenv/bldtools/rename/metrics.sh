#!/bin/ksh
# @(#)69        1.2  src/bldenv/bldtools/rename/metrics.sh, bldprocess, bos412, GOLDA411a 8/14/93 17:15:03
#
# COMPONENT_NAME: (BLDPROCESS) BAI Build Process
#
# FUNCTIONS: metrics
#
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1991, 1992
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# NAME: metrics
#
# FUNCTION:  generates a report for a given list of build cycles, reporting
#	     number of defects per release per cycle, the total defects,
#	     the number of PTFs per release per cycle, and the total PTFs.
#
# INPUT: <build_cycle> [<build_cycle> ... <build_cycle> ]
#
# OUTPUT: stdout
#
###############################################################################

. bldkshconst
. bldhostsfile
. bldnodenames

integer total=0 defs_total=0 ptfs_total=0

CYCLES=""
while [ $# != 0 ]
do
#DEFECTS:
#   wc -l /selfix/PTF/$BLDCYCLE/<prod>/defects for each product
#   wc -l /selfix/PTF/$BLDCYCLE/defectapars for the total

	CYCLES=`echo $CYCLES $1`
	echo "\n\n"
	echo "$1	RELEASE			DEFECTS"

	for i in `find /selfix/PTF/$1 -type d -print`
	do
		bldhostsfile ${i##*/} "${FALSE}"
		[[ $? -ne 0 ]] && continue

		if [ -s $i/defects ]
		then
			defs=`sort -d -u $i/defects | wc -l `
			prod=`echo $i | awk -F'/' '{print $NF}' -`
			if [ ${#prod} -ge 8 ]
			then
				echo "$1	$prod		$defs"
			else
				echo "$1	$prod			$defs"
			fi
			total=total+defs
		fi
	done
	if [ -s /selfix/PTF/$1/defectapars ]
	then
		defs=`sort -d -u /selfix/PTF/$1/defectapars | wc -l | awk '{print $1}' - `
	else
		defs=0
	fi
	echo "$1\t\tTOTAL DEFECTS\t\t$total"
	echo "$1\t\tUNIQUE DEFECTS TOTAL\t$defs"
	defs_total=defs_total+defs

#PTFS:
#  for each product directory in /selfix/UPDATE
#	count unique lines in /selfix/UPDATE/<prod>/ptf_pkg_back_$BLDCYCLE
#	keep track of the pieces and add for the $BLDCYCLE total

	total=0
	echo ""
	echo "$1	LPP			   PTFS"
	for i in `file /selfix/UPDATE/* | grep directory | awk -F':' '{print $1}' -`
	do
		if [ -s $i/ptf_pkg_back_$1 ]
		then
			prod=`echo $i | awk -F'/' '{print $NF}' -`
			ptfs=`cat $i/ptf_pkg_back_$1 | awk -F'\|' '{print $1}' - |  sort -d -u | wc -l`
			echo "$1	$prod			$ptfs"
			total=total+$ptfs
		fi
	done	
	echo "$1\t\tTOTAL PTFS\t\t$total"
	ptfs_total=ptfs_total+total
	shift
	total=0
done
	echo "\nCycles processed: $CYCLES"
	echo "\nTOTALS for build cycles processed:"
	echo "	Total Unique defects		$defs_total"
	echo "	Total PTFs			$ptfs_total"
		
