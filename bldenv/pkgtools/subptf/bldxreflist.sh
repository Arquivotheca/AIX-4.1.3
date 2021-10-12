#! /bin/ksh
# @(#)55	1.12  src/bldenv/pkgtools/subptf/bldxreflist.sh, pkgtools, bos412, GOLDA411a 6/8/94 15:06:27
#
# COMPONENT_NAME: (BLDTOOLS) BAI Build Tools 
#
# FUNCTIONS: bldxreflist
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
# NAME: bldxreflist
#
# FUNCTION: 
#
# INPUT: BIGxref (file) - xref data from make/build
#        DEBUG (environment) - flag to generate debug data, when true
#
# OUTPUT: the normalized xref list written to stdout
#
# FORMAT:
#
# SIDE EFFECTS: none
#
# EXECUTION ENVIRONMENT: the build process environment
#
# RETURNS: 0 (successful) or 1 (failure)
#
  logset +l -c$0  # init log command; rc=$UNSET
  logarg="-e"
  [[ -n "$DEBUG" ]] && log -b "entering ($@)"
  [[ $# = 0 ]] || log -x "illegal syntax"

  trap 'rm -f $XREFFILENAMES $NORMNAMES $NORMLIST $XREFOPTIONS; logset -r; \
	trap - EXIT; exit $rc' EXIT HUP INT QUIT TERM

  XREF=$(bldglobalpath)/BIGxref; > $XREF ;readonly XREF
  XREFFILENAMES=$(bldtmppath)/bldxreflist.xreffnames$$; readonly XREFFILENAMES
  NORMNAMES=$(bldtmppath)/bldxreflist.normnames$$; readonly NORMNAMES
  NORMLIST=$(bldtmppath)/bldxreflist.normlist$$; readonly NORMLIST
  XREFOPTIONS=$(bldtmppath)/bldxreflist.xrefoptions$$; readonly XREFOPTIONS
  XREFLIST=$(bldglobalpath)/xreflist; readonly XREFLIST
  XREFGLOBAL=$(bldhistorypath)/XREF

  cd $XREFGLOBAL
  for xref in ""*.xref
  do
        cat $xref >> $XREF || log -x "Building $XREF"
  done
  cd -
  bldcut $XREF SPACE 1 > $XREFFILENAMES || log -x "cutting xref names"
  bldnormalize $XREFFILENAMES > $NORMNAMES ||
  	{ [[ $? -ne 1 ]] && logarg="-x"; log $logarg "normalizing xref names"; }
  bldcut $XREF SPACE 2 > $XREFOPTIONS || log -x "cutting xref names"
  paste -d' ' $NORMNAMES $XREFOPTIONS > $NORMLIST ||
  	log -x "pasteing normalized xref"
  ## unique and remove all microcode (.mc) files/options
  bldunique $NORMLIST | grep -v "\.mc$" > $XREFLIST ||
  	log -x "removing duplicates from xref"

  [[ -n "$DEBUG" ]] && log -b "exiting"
  [[ $rc = $UNSET ]] && rc=$SUCCESS; exit

