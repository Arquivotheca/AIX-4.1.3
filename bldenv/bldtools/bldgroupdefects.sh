#
# @(#)78	1.6  src/bldenv/bldtools/bldgroupdefects.sh, bldtools, bos412, GOLDA411a 1/13/94 16:33:27
#
#   COMPONENT_NAME: BLDTOOLS
#
#   FUNCTIONS: bldgroupdefects
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
#   NAME: bldgroupdefects
#
#   FUNCTION: Take a list of defect numbers from a file and group them
#             in a comma separated list acceptable to the Report command.
#             Currently the Report command has a limit on the number of
#             defects that may be specified on a single command line.
#
#             Each defect list is broken up when a maximum number of 
#             defects is reached or the family the defects are members
#             of change.
#
#   INPUT: file ($1) - File containing the defects.  Format is:
#                      'defect:family' where the defect is a number and
#                      family is a family specified in bldCMVCfamily.dat
#          function ($3) - Function to all that will pass the defect list
#                          generated onto a report command.  Assume function
#                          will return 0 if no errors are seen and non zero
#                          if errors occur.
#          size ($2) - Maximum size of the defect list generated.
#
#   OUTPUT: CMVC_FAMILY (global) - CMVC Family the defects are from.
#
#   SIDE EFFECTS: Reorders the defect file passed.
#
#   RETURNS: 1 if call to $function returns nonzero; 0 otherwise.
#
function bldgroupdefects
{
   typeset -i count=0
   typeset    defect=""
   typeset    defectlist=""
   typeset    family=""
   typeset    file=$1
   typeset    function=$3
   typeset    size=$2

   # Sort defect file by CMVC family.
   sort -t: -o ${file} -u +1 -2 +0 -1n ${file}
   [[ $? -ne 0 ]] && log +l -c bldgroupdefects -x "Sorting ${file}"

   # Force first defect read to not match on previous family.
   CMVC_FAMILY=""

   while read defect
   do
      family=${defect##*:}
      defect=${defect%%:*}

      # Either the first defect read or family has changed.
      if [[ ( "${CMVC_FAMILY}" != "${family}" ) \
            || -z "${defectlist}" ]]
      then
         if [[ -n "${defectlist}" ]] 
         then
            ${function} ${defectlist}
            if [[ $? -ne 0 ]] 
            then
               log +l -c bldgroupdefects -e "Call to ${function} failed"
               return 1
            fi
         fi
         count=1
         defectlist="${defect}"
         CMVC_FAMILY="${family}"
      # Add defect onto the current list.
      else
         defectlist="${defect},${defectlist}"
         count=count+1
         # Reached the maximum size of defectlist; call function and start
         # building list again.
         if [ count -eq ${size} ]
         then
            ${function} ${defectlist}
            if [[ $? -ne 0 ]] 
            then
               log +l -c bldgroupdefects -e "Call to ${function} failed"
               return 1
            fi
            defectlist=""
           fi
      fi
   done < ${file}
   # Process remaining defects in list.
   if [[ -n "${defectlist}" ]] 
   then
      ${function} ${defectlist}
      if [[ $? -ne 0 ]] 
      then
         log +l -c bldgroupdefects -e "Call to ${function} failed"
         return 1
      fi
   fi
}
