#! /bin/ksh
# @(#)30	1.5  src/bldenv/bldtools/ChangeAbstracts.sh, bldprocess, bos412, GOLDA411a 6/2/93 15:57:38
#
# COMPONENT_NAME: (BLDPROCESS) BAI Build Process
#
# FUNCTIONS: ChangeAbstracts
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

function ProcessAbstract {
	typeset DEFAPARS="$(bldglobalpath)/defectapars"
	typeset APAR=""
	typeset ABSTRACTS="$(bldhistorypath)/abstracts"
	typeset SYMPTOMS="$(bldglobalpath)/symptoms"
	typeset ABSTRACTLIST="$(bldglobalpath)/abstractlist"
	typeset logline

	GetAbstract || return 2
	ReplaceAbstract
	[[ -s "$ABSTRACTS" ]] || return 2
	[[ -s "$SYMPTOMS" ]] || return 2
	if [[ -s "$DEFAPARS" ]]
	then
		APAR=$(grep "^$DEFECT:${FAMILY}|" $DEFAPARS | cut -d'|' -f2)
		[[ "${FAMILY}" = "${CMVC_FAMILY}" ]] && \
			APAR=$(grep "^$DEFECT|" $DEFAPARS | cut -d'|' -f2)
		[[ -n "$APAR" ]] || return 2
		grep "$APAR" $ABSTRACTS > /dev/null
		logline="Replacing the abstract for $DEFECT (${FAMILY})"
		log -b "$logline in the file $ABSTRACTS"
		sym_get $APAR $DEFECT:${FAMILY} $ABSTRACTLIST $SYMPTOMS \
		        $ABSTRACTS
		sym_get "" "" "" "" $ABSTRACTS -w
	else
		return 2
	fi
}

################################## main #######################################

. initchanges
. bldkshconst
. bldinitfunc

typeset -x CMVC_FAMILY=""

chksetfamily

[[ -s "$(bldglobalpath)/abstractlist" ]] || \
			   {
	    			log -b "Could not find the file $ABSTRACTLIST"
				clean_up
				exit 0
			   }
if [ -n "${infile}" ]
then
	exec 0< ${infile}
fi

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
      FAMILY=$(bldCMVCfamily ${FAMILY})
      [[ $? -ne 0 ]] && log +l -x "Need valid CMVC Family"
   fi
   ProcessAbstract || continue
done
clean_up
