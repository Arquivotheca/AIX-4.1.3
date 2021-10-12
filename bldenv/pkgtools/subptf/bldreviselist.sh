#! /bin/ksh
# @(#)48	1.7  src/bldenv/pkgtools/subptf/bldreviselist.sh, pkgtools, bos412, GOLDA411a 11/17/92 11:19:51
#
# COMPONENT_NAME: (BLDTOOLS) BAI Build Tools 
#
# FUNCTIONS: bldrevise
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
# NAME: bldreviselist
#
# FUNCTION: Return list of CMVC name changes which may affect the selfix
#	    history.
#
# INPUT: release (parameter) - CMVC level release name
#	 changeview (file) - changeview data from CMVC
#
# OUTPUT: the normalized change list written to stdout (one per line)
#
# EXECUTION ENVIRONMENT: the build process environment
#
# RETURNS: 0 (successful) or 1 (failure)
#
logset +l -c$0  # init log command; rc=$UNSET
[[ $# = 1 ]] || log -x "illegal syntax"
logarg="-e"
typeset release=$1  # command line parameter
trap 'rm -f $MODVIEW $CHANGETYPES $CHANGENAMES $NORMNAMES $OLDNAMES \
    $NORMOLDNAMES; logset -r; trap - EXIT; exit $rc' EXIT HUP INT QUIT TERM

CHANGEVIEW=$(bldreleasepath $release)/changeview
RENAMELIST=$(bldreleasepath $release)/renamelist
MODVIEW=$(bldtmppath)/bldreviselist.modview$$
CHANGETYPES=$(bldtmppath)/bldreviselist.changetypes$$
CHANGENAMES=$(bldtmppath)/bldreviselist.changenames$$
NORMNAMES=$(bldtmppath)/bldreviselist.normnames$$
OLDNAMES=$(bldtmppath)/bldreviselist.oldnames$$
NORMOLDNAMES=$(bldtmppath)/bldreviselist.normoldnames$$

touch $CHANGENAMES $OLDNAMES

## get only the change type which may affect the history
[[ -r $CHANGEVIEW ]] || log -x "changeview file ($CHANGEVIEW) not found"
grep '|delete|'   $CHANGEVIEW >  $MODVIEW 
grep '|recreate|' $CHANGEVIEW >> $MODVIEW 
grep '|rename|'   $CHANGEVIEW >> $MODVIEW 
grep '|link|'     $CHANGEVIEW >> $MODVIEW 

## cut out the path names and normalize them
cut -d'|' -f6 $MODVIEW    > $CHANGETYPES || log -x "cutting change types"
cut -d'|' -f5 $MODVIEW    > $CHANGENAMES || log -x "cutting change names"
bldnormalize $CHANGENAMES > $NORMNAMES   || { [[ $? -ne 1 ]] && logarg="-x"; log $logarg "normalizing change names"; }

typeset -i linenum=1

while read line
do
	changetype=`echo $line | cut -d'|' -f6`
	name=`echo $line | cut -d'|' -f5`
	oldname=$name
	if [ "$changetype" = "rename" ]
	then
		oldname=`sed -e "$linenum!d" $RENAMELIST`
		linenum=linenum+1
	fi
	print "$oldname" >> $OLDNAMES
done < $MODVIEW

# normalizing pathnames from OLDNAMES file
bldnormalize $OLDNAMES > $NORMOLDNAMES
paste -d'|' $CHANGETYPES $NORMOLDNAMES $NORMNAMES | sort -u ||
	log -x "pasting change list"

rm -f $CHANGENAMES $NORMNAMES $OLDNAMES $NORMOLDNAMES $MODVIEW $CHANGETYPES

[[ $rc = $UNSET ]] && rc=$SUCCESS; exit

~

