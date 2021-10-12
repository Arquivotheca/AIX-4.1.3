#! /bin/ksh
# @(#)94	1.26  src/bldenv/bldtools/CheckSymptom.sh, bldprocess, bos412, GOLDA411a 1/17/94 11:25:04
# COMPONENT_NAME: (BLDPROCESS) BAI Build Process
#
# FUNCTIONS: CheckSymptom
#	     clean_up
#	     call_noteview 
#	     create_defects
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

function clean_up {
	rm -f $NOTEVIEW $NOTEVIEW_ERR ${DEFECTS} ${REPORT_NOTEVIEW}
	logset -r
}

###############################################################################

function call_noteview
{
   typeset    defectlist=$1

   Report -view noteview \
          -where "defectname in ( ${defectlist} ) \
                  order by defectname,adddate asc" \
          -family ${CMVC_FAMILY} \
          -raw > ${REPORT_NOTEVIEW}
   [[ $? != 0 ]] && \
      log +l -x "CMVC Error: Report -view noteview, Please check logs"

   sed -e "/^.*|.*|.*|.*|.*|.*|.*|.*|.*$/s/^\([^|]*\)|/\1:${CMVC_FAMILY}|/" \
      ${REPORT_NOTEVIEW} >> ${NOTEVIEW}
   [[ $? != 0 ]] && \
      log +l -x "Converting ${REPORT_NOTEVIEW} to include CMVC Family"

   rm -f ${REPORT_NOTEVIEW}
}

###############################################################################

function create_defects
{
   if [[ ! -s "$DEFECTSFILE" ]] 
   then
      if [[ -z "$LEVELVERSION" ]]
      then
         logline="Either the defectsfile has to be passed or the"
         logline="$logline env varable LEVELVERSION has to be "
         log +l -x "$logline set to the build name"
      fi
      Report -view Levelmemberview \
             -where "levelname = '${LEVELVERSION}' and \
                     releasename = '${CMVC_RELEASE}'" \
             -family ${CMVC_FAMILY} \
             -raw \
      | awk -F"|" '{ printf "%s:%s\n",$3,CMVC_FAMILY }' \
            CMVC_FAMILY=${CMVC_FAMILY} > ${DEFECTS}
      if [[ $? != 0 ]]
      then
         logline="CMVC Error: Report -view levelmemberview for $LEVELVERSION"
         log +l -x "${logline}, Please check logs"
      fi
   else
      LEVELVERSION="$DEFECTSFILE"
       while read defect
       do
          family="${defect##*:}"
          defect="${defect%%:*}"
          if [[ "${defect}" = "${family}" ]]
          then
             # No family specified.
             print ${defect}:${CMVC_FAMILY} >> ${DEFECTS}
          else
             # Expand to full family name.
             family="$(bldCMVCfamily ${family})"
             [[ $? -ne 0 ]] && log +l -x "Need valid CMVC Family"
             print ${defect}:${family} >> ${DEFECTS}
         fi
      done < ${DEFECTSFILE}
   fi
}

###############################################################################
. bldloginit

test ${BLDTMP:=$(bldtmppath)}

typeset -x CMVC_FAMILY
typeset -x CMVC_RELEASE=""
typeset    DEFECTS=${BLDTMP}/CheckSymptom_defects.$$
typeset -ir DEFECT_LIST_SIZE=100
typeset    REPORT_NOTEVIEW=${BLDTMP}/CheckSymptom_noteview.$$
typeset    status

trap 'clean_up;exit 2' INT QUIT HUP TERM

while getopts :r:l:a option
do      case $option in
	r) CMVC_RELEASE=$OPTARG;;
	l) logfile=$OPTARG;;
	a) export AREABLD=TRUE;;
	:) print -u2 "$0: $OPTARG requires a value"
           exit 2;;
       \?) print -u2 "$0: unknown option $OPTARG"
           print -u2 -n "USAGE: $0 [-a] [-r <ReleaseName>] [-l <logfile>]"
	   print -u2 " [<defectsfile>]"
	   exit 2;
        esac
done

shift OPTIND-1
DEFECTSFILE=$1

. bldinitfunc
. bldkshconst
. bldhostsfile
. bldnodenames
. bldgroupdefects

bldinit

LOG=$(bldlogpath)
test ${logfile:="${LOG}/$(basename $0).all"}
logset -C $0 -c $0 -F $logfile
NOTEVIEW=${BLDTMP}/noteview.$$; readonly NOTEVIEW
NOTEVIEW_ERR=${BLDTMP}/noteview_err.$$; readonly NOTEVIEW_ERR
chksetbecome

[[ ! -f ${RELEASE_LIST} ]] && \
   log +l -x "Cannot find/open RELEASE_LIST file: ${RELEASE_LIST}"

if [[ ! -s "$DEFECTSFILE" ]]
then
   chksetrelease
   prod=`grep "${CMVC_RELEASE}" ${RELEASE_LIST}`
   if [[ -z $prod ]]
   then
      log +l -w "${CMVC_RELEASE} not processed for symptoms since it was"
      log +l -w "not found in the file ${RELEASE_LIST}"
      clean_up
      exit 0
   fi
   bldhostsfile ${CMVC_RELEASE} "${TRUE}"
   [[ $? -ne 0 ]] && log +l -x "bldhostsfile ${CMVC_RELEASE} failed"
   CMVC_FAMILY=${HOSTSFILE_CMVCFAMILY}
else
   chksetfamily
   [[ -n "${CMVC_RELEASE}" ]] || CMVC_RELEASE="defects"
fi
create_defects
bldgroupdefects ${DEFECTS} ${DEFECT_LIST_SIZE} call_noteview
rm -f ${DEFECTS}
SYMPTOMS="$(bldglobalpath)/symptoms"
MEMOS="$(bldglobalpath)/memos"
log -b "Calling sym_lookup to create the files:"
log -b "$SYMPTOMS and $MEMOS"
sym_lookup $NOTEVIEW $SYMPTOMS $MEMOS $NOTEVIEW_ERR
DeleteStatus $_TYPE $T_CHECKSYMPTOM\
               	$_SUBTYPE $S_SYMPTOMS \
               	$_BLDCYCLE $BLDCYCLE \
               	$_RELEASE ${CMVC_RELEASE} \
               	$_LEVEL "$LEVELVERSION" \
               	$_STATUS $ST_SUCCESS
DeleteStatus $_TYPE $T_CHECKSYMPTOM\
               	$_SUBTYPE $S_SYMPTOMS \
               	$_BLDCYCLE $BLDCYCLE \
               	$_RELEASE ${CMVC_RELEASE} \
               	$_LEVEL "$LEVELVERSION" \
               	$_STATUS $ST_FAILURE
if [[ -s $NOTEVIEW_ERR ]] 
then
	if [ -z "$AREABLD" ]
	then
		log +l -w "The following defects have no SYMPTOM strings in"
		log +l -w "their note fields:"
		status=${ST_FAILURE}
	else
		log +l -w "The following defects have no SYMPTOM strings in"
		log +l -w "their note fields but were assigned dummy SYMPTOM"
		log +l -w "strings in the symptoms file:"
		status=${ST_SUCCESS}
	fi
	for defect in $(cat ${NOTEVIEW_ERR})
        do
	   log +l -w "${defect%%:*} (${defect##*:})"
        done
	bldsetstatus $_TYPE $T_CHECKSYMPTOM \
			$_SUBTYPE $S_SYMPTOMS \
			$_BLDCYCLE $BLDCYCLE \
			$_RELEASE ${CMVC_RELEASE} \
			$_LEVEL "$LEVELVERSION" \
			$_STATUS ${status}
else
	bldsetstatus $_TYPE $T_CHECKSYMPTOM \
			$_SUBTYPE $S_SYMPTOMS \
			$_BLDCYCLE $BLDCYCLE \
			$_RELEASE ${CMVC_RELEASE} \
			$_LEVEL "$LEVELVERSION" \
			$_STATUS $ST_SUCCESS
fi
clean_up
