#! /bin/ksh
# @(#)29	1.15  src/bldenv/bldtools/initchanges.sh, bldprocess, bos412, GOLDA411a 8/25/93 16:48:52
#
# COMPONENT_NAME: (BLDPROCESS) BAI Build Process
#
# FUNCTIONS: initchanges
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

function usage {
        print -u2 "USAGE: ${cmd} [file]"
	exit 2
}

###############################################################################

function clean_up {
	rm -f $DEFECTS "$BLDTMP/abstract.$$"
	logset -r
}

###############################################################################

function GetAbstract {
	typeset TEMPFILE="$BLDTMP/abstract.$$"
	typeset logline
	
	log -b "Getting the abstract for $DEFECT (${FAMILY}) from CMVC"
	Report -view defectview \
	       -where "name = '$DEFECT'" \
	       -family ${FAMILY} \
	       -raw > $TEMPFILE
	[[ $? = 0 ]] || log -x "CMVC Error"
	ABSTRACT=$(cat $TEMPFILE | cut -d'|' -f9)
	if [[ -z "$ABSTRACT" ]]
	then
		Report -view featureview \
		       -where "name = '$DEFECT'" \
		       -family ${FAMILY} \
		       -raw > $TEMPFILE
		[[ $? = 0 ]] || log -x "CMVC Error"
		ABSTRACT=$(cat $TEMPFILE | cut -d'|' -f7)
	fi
	if [[ -z "$ABSTRACT" ]] 
        then
		log -e "The defect/feature $DEFECT (${FAMILY}) is not valid"
		return 2
	fi
}

###############################################################################

function ReplaceAbstract {
	typeset TEMPFILE

	TEMPFILE="$BLDTMP/abstractlist.$$"
	log -b "Replacing the abstract for $DEFECT (${FAMILY}) in the file $ABSTRACTLIST"
	trap "" INT QUIT HUP TERM
	if [[ "${FAMILY}" = "${DEFAULT_CMVCFAMILY}" ]]
	then
		# Need to handle old format of defect and new format of
		# defect:family.  The old format check can be removed once
                # there are no abstractlist files with old file format.
        	sed -e "/^${SPACECHARACTER}*0*${DEFECT} /d" \
		    -e "/^${SPACECHARACTER}*0*${DEFECT}:${FAMILY} /d" \
                    ${ABSTRACTLIST} \
		> ${TEMPFILE}
	else
        	sed -e "/^${SPACECHARACTER}*0*${DEFECT}:${FAMILY} /d" \
                    ${ABSTRACTLIST} \
		> ${TEMPFILE}
	fi
        print "\nFollowing abstract updated by ReplaceAbstract on $(date)" \
           >> ${TEMPFILE}
        print -R "${DEFECT}:${FAMILY}  ${ABSTRACT}" >> ${TEMPFILE}
	mv $TEMPFILE $ABSTRACTLIST
	rm -f $TEMPFILE
	trap 'clean_up;exit 2' INT QUIT HUP TERM
}

###############################################################################

function call_CheckSymptom {
	typeset defect
	if [[ -n "${infile}" ]]
	then
	   rm -f ${DEFECTS}
	   cp ${infile} ${DEFECTS}
        else
	   while read defect
	   do
	      print ${defect} >> ${DEFECTS}
	   done
	fi
	[[ -s "$DEFECTS" ]] && CheckSymptom -l $LOG $DEFECTS
}

##################################Main#########################################

cmd=$(basename $0)

[[ $1 = "-?" || $# -gt 2 ]] && usage

if [[ -n "$1" ]] 
then
   infile="$1"
   if [[ ! -f "${infile}" ]]
   then
      print -u2 "Cannot find file: ${infile}"
      exit 1
   fi
else
   infile=""
fi

. bldinitfunc
. bldloginit

test ${BLDTMP:=/tmp}
DEFECTS=$BLDTMP/${cmd}_defects.$$

bldinit
LOG="$(bldlogpath)/${cmd}.all"
logset -c ${cmd} -F $LOG

trap 'clean_up;exit 2' INT QUIT HUP TERM

chksetbecome
