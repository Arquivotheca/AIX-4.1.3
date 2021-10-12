#! /bin/ksh
# @(#)50	1.9  src/bldenv/pkgtools/subptf/bldmissdefects.sh, pkgtools, bos412, GOLDA411a 1/14/94 16:11:06
#
# COMPONENT_NAME: (BLDTOOLS) BAI Build Tools
#
# FUNCTIONS:    bldmissdefects
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
. bldinitfunc

RELDEFECTS=$(bldtmppath)/reldefects$$; readonly RELDEFECTS
CHANGES=$(bldtmppath)/changefilelist$$; readonly CHANGES
DEFECTS=$(bldtmppath)/uniquedefects$$; readonly DEFECTS

get_filename_definitions

[[ ! -f ${PTFINFO} ]] && \
   log +l -x "Cannot find/open PTFINFO file: ${PTFINFO}"
[[ ! -f ${RELEASE_LIST} ]] && \
   log +l -x "Cannot find/open RELEASE_LIST file: ${RELEASE_LIST}"

trap 'rm -f $CHANGES $DEFECTS $RELDEFECTS' HUP INT QUIT TERM

while read release
do
     if [[ -r "$(bldreleasepath $release)/defects" ]] ; then
          sort -u $(bldreleasepath $release)/defects > $DEFECTS
          changeview=$(bldreleasepath $release)/changeview
          grep -w $release ${PTFINFO} > $RELDEFECTS
          while read defect
          do
               grep -w $defect $RELDEFECTS > /dev/null
               if [[ $? != 0 ]] ; then
                    log -w "defect $defect in release $release is not in a PTF"
                    if [[ -s $changeview ]] ; then
                         first=1
                         grep "^.*\|$defect\|" $changeview | 
                              cut -d'|' -f4,5,6,8 | tr '|' ' ' > $CHANGES
		         while read sid file cmd abstract ; do
                              if [[ $first = 1 ]] ; then
                                   log -b +l -c $defect "abstract: $abstract"
                                   first=0
                              fi
 	                      log -b +l -c $defect "$cmd: $file ($sid)"
                         done < $CHANGES
                    fi
               fi
          done < $DEFECTS
     fi
done < ${RELEASE_LIST}
rm -f $CHANGES $DEFECTS $RELDEFECTS
