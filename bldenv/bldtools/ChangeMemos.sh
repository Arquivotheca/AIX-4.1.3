#! /bin/ksh
# @(#)27	1.6  src/bldenv/bldtools/ChangeMemos.sh, bldprocess, bos412, GOLDA411a 6/29/93 13:36:30
# COMPONENT_NAME: (BLDPROCESS) BAI Build Process
#
# FUNCTIONS: ChangeMemos
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

###############################################################################

function process_memos
{
   typeset logline
   typeset apar defect family

   while read defect
   do
      family=${defect##*:}
      defect=${defect%%:*}
      if [[ "${defect}" = "${family}" ]]
      then
         # No family specified.
         family=${CMVC_FAMILY}
      else
         # Expand to full family name.
         family="$(bldCMVCfamily ${family})"
         [[ $? -ne 0 ]] && log +l -x "Need valid CMVC Family"
      fi
      apar=$(Report -view defectview \
                    -where "name = '$defect'" \
                    -family ${family} \
                    -raw \
             | cut -d\| -f26)
      if [[ $apar = IX* ]] || [[ $apar = ix* ]]
      then
         memo_get $apar $defect:${family} $MEMOS $MEMO_INFO
      else
         log -b "An APAR has not been created for defect ${defect} (${family})"
      fi
   done < ${DEFECTS}
   memo_get "" "" "" $MEMO_INFO -w
}

###############################################################################

. initchanges
. bldkshconst
. bldloginit

typeset -x CMVC_FAMILY

chksetfamily

MEMOS=$(bldglobalpath)/memos
MEMO_INFO=$(bldhistorypath)/memo_info
call_CheckSymptom
process_memos
clean_up

