#! /bin/ksh
# @(#)28	1.10  src/bldenv/bldtools/ChangeSymptoms.sh, bldprocess, bos41J, 9520B_all 5/18/95 14:46:04
#
# COMPONENT_NAME: (BLDPROCESS) BAI Build Process
#
# FUNCTIONS: ChangeSymptoms
#
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1991, 1993
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

###############################################################################

function processSymptom
{
   typeset defnum APAR logline

   while read DEFECT
   do
      FAMILY=${DEFECT##*:}
      DEFECT=${DEFECT%%:*}
      if [[ "${DEFECT}" = "${FAMILY}" ]]
      then
         # No family specified.
         FAMILY=${CMVC_FAMILY}
      else
         # Expand to full family name.
         FAMILY="$(bldCMVCfamily ${FAMILY})"
         [[ $? -ne 0 ]] && log +l -x "Need valid CMVC Family"
      fi
      GetAbstract || continue
      if [[ -s "$ABSTRACTLIST" ]]
      then
         defnum=$(grep "^${SPACECHARACTER}*0*$DEFECT:${FAMILY} " $ABSTRACTLIST)
         # If grep fails try another grep using old file format where defect
         # did not have family following.  Restrict search only to the case
         # where current family matches the default family.  This check can
         # be removed once there are no abstractlist files with old file format.
         [[ -z "$defnum" && ( "${FAMILY}" = "${DEFAULT_CMVCFAMILY}" ) ]] && \
            defnum=$(grep "^${SPACECHARACTER}*0*$DEFECT " $ABSTRACTLIST)
         if [[ -n "$defnum" ]]
         then
            ReplaceAbstract
         else
            logline="Appending the defect/feature $DEFECT (${FAMILY}) and its"
            logline="$logline abstract to the file ${ABSTRACTLIST}"
            log -b "$logline"
            print "\nFollowing abstract appended by ChangeSymptoms on $(date)" \
            >> ${ABSTRACTLIST}
            print -R "${DEFECT}:${FAMILY}  $ABSTRACT" >> $ABSTRACTLIST
         fi
      else
         logline="Creating the file $ABSTRACTLIST for the"
         log -b "$logline defect/feature $DEFECT (${FAMILY})"
         print "\nFollowing abstract appended by ChangeSymptoms on $(date)" \
         >> ${ABSTRACTLIST}
         print -R "${DEFECT}:${FAMILY}  $ABSTRACT" >> $ABSTRACTLIST
      fi
      if [[ ${MULTI_APAR_DEFECT} != "yes" ]
      then
        APAR=`Report -view defectview \
                   -where "name = '$DEFECT'" \
		   -family ${FAMILY} \
                   -raw \
            | cut -d\| -f26`
        if [[ -z "${APAR}" ]]
        then
        APAR=`Report -view featureview \
                   -where "name = '$DEFECT'" \
                   -family ${FAMILY} \
                   -raw \
            | cut -d\| -f18`
        fi
      else
        APAR=$(qryAPARByDefect -d ${DEFECT} -v ${BPD_AIX_RELEASE} -r | cut -d\| -f2)
      fi
      if [[ $APAR = IX* ]] || [[ $APAR = ix* ]]
      then
         log -b "Calling sym_get to append to the file ${ABSTRACTS}"
         sym_get $APAR $DEFECT:${FAMILY} $ABSTRACTLIST $SYMPTOMS $ABSTRACTS
      else
         logline="An APAR has not been created for defect/feature"
         log -b "$logline $DEFECT (${FAMILY})"
      fi
   done < ${DEFECTS}
   sym_get "" "" "" "" $ABSTRACTS -w
}

###############################################################################

. initchanges
. bldkshconst
. bldinitfunc

chksetfamily

SYMPTOMS=$(bldglobalpath)/symptoms
ABSTRACTS=$(bldhistorypath)/abstracts
ABSTRACTLIST=$(bldglobalpath)/abstractlist
call_CheckSymptom
processSymptom
clean_up
