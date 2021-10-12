#!/bin/ksh
# @(#)96        1.2  src/bldenv/pkgtools/genPMPFile.sh, pkgtools, bos41B, 412_41B_sync  12/16/94  13:52:55
#
# COMPONENT_NAME: (PKGTOOLS) BAI Build Tools
#
# FUNCTIONS: usage
#            createCumPtfPkgFile
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1994
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

function usage
{
   print -u1 "genPMPFile [-p PTFNumber] [-l \"lpp1 lpp2 lpp3\"] [-a aparNmbr]"
   print -u1 "           [-f File_containing_PTFs]" 
   print -u1 "genPMPFile -h" 
   exit 0
}


function createCumPtfPkgFile
{
   typeset FILESET=${1}
   typeset -i version
   typeset -i release
   typeset -i mod
   typeset -i fix
#   print -u1 "Fileset is ${FILESET}"   

   PTFPKG=${UPDATETOP}/$(echo ${FILESET} | awk -F"." '{print $1}')/ptf_pkg.${BLDCYCLE}
   MODFILE=${UPDATETOP}/$(echo ${FILESET} | awk -F"." '{print $1}')/maintLevelFile
   OPTIONPATH=${UPDATETOP}/$(echo ${FILESET} | sed -e 's?\.?/?g')
   PTFSLIST=${OPTIONPATH}/ptfsList 
   VRMFFILE=${OPTIONPATH}/vrmfFile

   if [[ -s ${PTFSLIST} ]]
   then
      cat ${VRMFFILE} | awk -F"." '{print $1 " " $2 " " $3 " " $4}' |
      read version release mod fix
      cat ${MODFILE} | read mod keyword
#      echo "2 AIX" | read mod keyword
      [[ "${RERUNFLAG}" = "${FALSE}" ]] && mod=$mod+1
      vrmf="${version}.${release}.${mod}.0"
      if [[ -n ${PTFFile} ]]
      then
	 read -u5 PTFNUMBER
	 if [[ $? -ne 0 ]]
	 then
	    print -u1 "ERROR reading the PTF ID File for fileset ${FILESET}"
	    exit 1
	 fi
      fi
      if [[ -n $APARNmbr ]]
      then
	 aparField="${vrmf}_${keyword}_ML ${APARNmbr}"
      else
	 aparField="${vrmf}_${keyword}_ML"
      fi
      if [[ -f ${PTFPKG} ]] 
      then
         print -r "${PTFNUMBER}|${aparFiled}|cum_ptf|${FILESET}:${vrmf}||||" >> ${PTFPKG}
         # print -r "${PTFNUMBER}|${vrmf}_${keyword}_ML|cum_ptf|${FILESET}:${vrmf}||||" 
      else
         print -r "${PTFNUMBER}|${aparFiled}|cum_ptf|${FILESET}:${vrmf}||||" > ${PTFPKG}
         #print -r "${PTFNUMBER}|${vrmf}_${keyword}_ML|cum_ptf|${FILESET}:${vrmf}||||" 
      fi
      
      if [[ -z ${PTFFile} ]]
      then
         ptfnumber=$ptfnumber+1
         PTFNUMBER="${ptfprefix}$ptfnumber"
      fi
   fi
}


###############
# Main Program
###############

. bldkshconst
. bldinitfunc

RERUNFLAG=${FALSE}
typeset PTFFile=""
typeset APARNmbr=""
PTFNUMBER=""
LPPs=""
while getopts :a:f:p:l:rh option
do
    case ${option} in
       a) APARNmbr="${OPTARG}"
          ;;
       f) if [[ -r ${OPTARG} ]]
	  then
	      PTFFile="${OPTARG}"
	  fi
          ;;
       p) PTFNUMBER="${OPTARG}"
          ;;
       l) LPPs="${OPTARG}"
          ;;
       r) RERUNFLAG=${TRUE}
          ;;
       h) usage
          ;;
    esac
done
shift OPTIND-1

#bldinit genCumPtfPkgFile
chksetbldcycle -o
chksettop

typeset    HISTORYTOP=${TOP}/HISTORY
typeset    UPDATETOP=${TOP}/UPDATE              # Top of the update tree
typeset    PTFOPTFILE=${HISTORYTOP}/ptfoptions

# Set default value 
if [[ -z ${PTFNUMBER} && -z ${PTFFile} ]]
then
   PTFNUMBER="TX10000" 
fi

typeset ptfprefix=`echo ${PTFNUMBER} | cut -c1-2`
typeset -i ptfnumber=`echo ${PTFNUMBER} | cut -c3-7`

exec 5< ${PTFFILE}

if [[ -z ${LPPs} ]]
then
   cat ${PTFOPTFILE} |
   awk '{print $2}' |
   sort -u |
   while read fileset
   do
       createCumPtfPkgFile ${fileset}  
   done
else
   for LPP in ${LPPs}
   do
       grep "${LPP}." ${PTFOPTFILE} |
       awk '{print $2}' |
       sort -u |
       while read fileset
       do
           createCumPtfPkgFile ${fileset}
       done
   done
fi
exec 5<&-
exit 0
