#
# @(#)95	1.5  src/bldenv/bldtools/bldCMVCfamily.sh, bldtools, bos412, GOLDA411a 1/17/94 19:47:54
#
#   COMPONENT_NAME: BLDTOOLS
#
#   FUNCTIONS: bldCMVCfamily
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1993
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

#
# NAME: bldCMVCfamily.
#
# FUNCTION: Return a consistent CMVC family name.  Family names stored
#           with the selfix database must all be in the same format to
#           simplify matching.  The longest form has been chosen.
#
#           The family name is taken of the CMVC family name passed and
#           searched for in the CMVC family name database.  If found the
#           full family name is returned; if not found then NULL is
#           returned.
#
#           Current design ignores the hostname and portnumber of the
#           CMVC family passed.  If the CMVC family name database contains
#           aix@ausaix02@2035 and entry of the form aix@<other characters>
#           is passed the family name of aix@ausaix02@2035 is returned.
#
# INPUT: family ($1) - CMVC family name.  Can be in the form of: familyname,
#                      familyname@hostname or familyname@hostname@portnumber.
#
# OUTPUT: fullfamilyname (stdout) - The full CMVC family name returned in
#                                   form familyname@hostname@portnumber.
#
# SIDE EFFECTS: none.
#
# RETURNS: 0 if family name found, 1 if family not found.
#

. bldinitfunc

typeset -i RC=0
typeset    familyname=${1%%@*}
typeset    fullfamilyname

get_filename_definitions
 
if [[ ! -f ${BLDCMVCFAMILY_FILE} ]] 
then
   print -u2 "Cannot find/open BLDCMVCFAMILY_FILE file: ${BLDCMVCFAMILY_FILE}"
   print ""
   exit 1
fi
   

fullfamilyname=$(grep "^${familyname}@" ${BLDCMVCFAMILY_FILE} 2> /dev/null)
if [[ $? -ne 0 ]] 
then
   print -u2 "A entry for ${familyname} must exist in ${BLDCMVCFAMILY_FILE}"
   fullfamilyname=""
   RC=1
fi

print ${fullfamilyname}

exit ${RC}
