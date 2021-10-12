#! /bin/ksh
# @(#)12	1.3  src/bldenv/pkgtools/subptf/bldlinkdata.sh, pkgtools, bos412, GOLDA411a 2/13/92 13:54:54
# COMPONENT_NAME: (BLDTOOLS) BAI Build Tools 
#
# FUNCTIONS: 	bldlinkdata
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1991
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

#
# NAME: bldlinkdata
#
# FUNCTION: normalizes the 'lmlinkdata' file produced by 'genlmlinkdata'
#
# INPUT: 'lmlinkdata' file in the 'bldglobalpath'
#
# OUTPUT: 'linkdata' file in the 'bldglobalpath'
#
# SIDE EFFECTS: none
#
# EXECUTION ENVIRONMENT: the build process environment
#
# RETURNS: 0 (successful) or 2 (failure)
#
. bldinitfunc
bldinit
trap 'exit 2' HUP INT QUIT TERM
LMLINKDATA=$(bldglobalpath)/lmlinkdata
LINKDATA=$(bldglobalpath)/linkdata
export NONORMERR=1
[[ -f "$LMLINKDATA" ]] || touch $LMLINKDATA
bldnormalize $LMLINKDATA | sed 's/ /\|/g' > $LINKDATA || \
			{ print -u2 "normalizing linkdata"; exit 2; }
exit 0
