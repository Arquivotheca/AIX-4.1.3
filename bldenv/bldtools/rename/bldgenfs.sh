#!/bin/ksh
# @(#)80        1.1  src/bldenv/bldtools/rename/bldgenfs.sh, bldtools, bos412, GOLDA411a 1/28/93 08:16:10
#
# COMPONENT_NAME: (BLDTOOLS) BAI Build Tools
#
# FUNCTIONS: bldgenfs
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
# NAME: bldgenfs
#
# FUNCTION:  does the genfsimage in current directory with all the flags.
#            Expects the list of ptfs to gen in a file of the name
#            "basename \`pwd`\".ptfs or override with -l flag.  Specify
#	     -B if genning for bull ptfs.  Can override -p/-s with
#	     your own -p PROD_DIR and -s SHIP_DIR, if $AFSPROD|$AFSSHIP
#	     or $AFSPROD/bull|$AFSSHIP/bull not correct.
#
# INPUT: [-p PROD_DIR] [-s SHIP_DIR] [-t (genfsimage flag)] [-B]
#
# OUTPUT: genfsimage.out
#
###############################################################################

usage()
{
    print >$2 "USAGE: bldgenfs [-p PROD_DIR] [-s SHIP_DIR] [-t] [-B] \
[-l ptf_list]\n\t where -p,-s,-t are genfsimage flags and -B is genfsimage \
for BULL build"
    exit 1
}

bullgen=
proddir=
shipdir=
filelist=
tflag=
flag=
for arg 
do
    case "$arg" in
	-p|-s|-l) flag=$arg ;;
	-p*) proddir=$arg ;;
	-s*) shipdir=$arg ;;
	-l*) filelist=$arg ;;
	 -t) tflag="-t" ;;
	 -B) bullgen="y" ;;
	  *) case $flag in 
		-p) proddir="-p"$arg ;;
		-s) shipdir="-s"$arg ;;
		-l) filelist="-l"$arg ;;
		 *) usage ;;
	    esac
	    flag=
    esac
done

if [ "q$bullgen" != "q" ] ; then
    if [ "q$proddir" = "q" ] ; then
	proddir="-p$AFSPROD/bull"
    fi
    if [ "q$shipdir" = "q" ] ; then
	shipdir="-s$AFSSHIP/bull"
    fi
fi

if [ "q$filelist" = "q" ] ; then
    name=`basename \`pwd\``
    filelist="$name.ptfs"
    ls -l $filelist >/dev/null 2>&1
    if [ $? != 0 ] ; then
	print >$2 "CANNOT FIND LIST of PTF'S: $filelist"
	usage
    fi
    filelist="-l$filelist"
fi
    genfsimage $filelist -B -b. -d. -x $proddir $shipdir $tflag 2>&1 | tee genfsimage.out
