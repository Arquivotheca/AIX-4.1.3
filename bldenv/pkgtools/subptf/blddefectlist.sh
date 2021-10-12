#! /bin/ksh
# @(#)38	1.5  src/bldenv/pkgtools/subptf/blddefectlist.sh, pkgtools, bos412, GOLDA411a 5/26/92 12:27:09
#
# COMPONENT_NAME: (BLDTOOLS) BAI Level Tools 
#
# FUNCTIONS: blddefectlist
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1991, 1992
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

. bldloginit
. bldkshconst

#
# NAME: blddefectlist
#
# FUNCTION: Format and return list of build cycle defects for specified release
#
# INPUT: release (parameter) - the current CMVC level release
#	 defects (file) - list of the release's defects for current cycle
#
# OUTPUT: list of defect numbers (one per line)
#
# FORMAT: the output list of defects is in the following form -
#		<release>.<defect#>
# 	  For example: bos320.29883
#
# EXECUTION ENVIRONMENT: the build process environment
#
# RETURNS: 0 (successful) or non-zero (failure)
#
  rc=$UNSET; logset +l -c$0  # init log command
  [[ $# = 1 ]] || log -x "illegal syntax"
  typeset release=$1 # command line parameter
  DEFECTLIST=$(bldreleasepath $release)/defects
  UNIQLIST=$(bldtmppath)/blddefectlist.uniqlist$$

  trap 'rm -f $UNIQLIST; logset -r; trap - EXIT; exit $rc' \
	EXIT HUP INT QUIT TERM

  if [[ -r "$DEFECTLIST" ]] ; then
  	sort -u $DEFECTLIST > $UNIQLIST || log -x "unable to remove duplicates"
	while read defect  ## add release name to each defect number
	do
		print "$release.$defect" 
	done < $UNIQLIST
  else
	log -x "unable to open defectlist ($DEFECTLIST)"
  fi

  [[ $rc = $UNSET ]] && rc=$SUCCESS; exit
