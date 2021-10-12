#! /bin/ksh
# @(#)49	1.15  src/bldenv/pkgtools/subptf/bldchangelist.sh, pkgtools, bos412, GOLDA411a 11/17/92 11:19:28
#
# COMPONENT_NAME: (BLDTOOLS) BAI Build Tools 
#
# FUNCTIONS: bldchangelist
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
. bldkshconst	# define constants

#
# NAME: bldchangelist
#
# FUNCTION: Return list of changed CMVC files with defects which changed them
#
# INPUT: release (parameter) - CMVC build release name
#	 changeview (file) - CMVC changeview data 
#
# OUTPUT: the normalized change list written to stdout
#
# FORMAT: stdout - <defect>|<change file> (one per line)
#
# EXECUTION ENVIRONMENT: the build process environment
#
# RETURNS: 0 (successful) or 1 (failure)
#
  logset +l -c$0  # init log command; rc=$UNSET
  [[ $# = 1 ]] || log -x "illegal syntax"
  typeset release=$1  # command line parameter
  logarg="-e";

  trap 'rm -f $MODVIEW $CHANGENAMES $NORMNAMES $CHANGEDEFECTS; logset -r; \
	trap - EXIT; exit $rc' EXIT HUP INT QUIT TERM

  CHANGEVIEW=$(bldreleasepath $release)/changeview
  MODVIEW=$(bldtmppath)/bldchangelist.modview$$
  CHANGENAMES=$(bldtmppath)/bldchangelist.changenames$$
  NORMNAMES=$(bldtmppath)/bldchangelist.normnames$$
  CHANGEDEFECTS=$(bldtmppath)/bldchangelist.changedefects$$

  [[ -r $CHANGEVIEW ]] || log -x "changeview file ($CHANGEVIEW) not found"

  ## only get the CMVC changes which cause the source to be built (i.e. do
  ## not get 'delete')
  grep '|delta|' $CHANGEVIEW > $MODVIEW 
  grep '|create|' $CHANGEVIEW >> $MODVIEW 
  grep '|recreate|' $CHANGEVIEW >> $MODVIEW 
  grep '|rename|' $CHANGEVIEW >> $MODVIEW 
  grep '|link|' $CHANGEVIEW >> $MODVIEW 

  ## cut the path name separately so it can be normalized
  cut -d'|' -f5 $MODVIEW > $CHANGENAMES || log -x "cutting change names"
  bldnormalize $CHANGENAMES > $NORMNAMES || {
	[[ $rc -ne 1 ]] && logarg="-x";
  	log $logarg "normalizing change names"; }

  cut -d'|' -f2 $MODVIEW > $CHANGEDEFECTS ||
	log -x "cutting change defects"

  paste -d'|' $CHANGEDEFECTS $NORMNAMES | sort -u ||
	log -x "pasteing change list"

  rm -f $CHANGENAMES $NORMNAMES $CHANGEDEFECTS $MODVIEW
  [[ $rc = $UNSET ]] && rc=$SUCCESS; exit
~

