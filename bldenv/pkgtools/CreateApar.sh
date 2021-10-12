#!/bin/ksh
# @(#)31        1.41  src/bldenv/pkgtools/CreateApar.sh, pkgtools, bos41J, 9523A_all 5/26/95 14:21:43
#
#   COMPONENT_NAME: PKGTOOLS
#
#   FUNCTIONS: 
#		adjust_symptom
#		apar_already_created
#		bailout
#		create_apar
#		display_help
#		display_msg
#		file_cleanup
#		get_defect_data
#		get_defect_fields
#		get_prefix_and_ref
#		get_track_releases
#		log_it
#		main
#		update_reference
#		write_to_input_area
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1991,1993
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

function display_help
{
cat <<-EOM
SYNTAX: ${program_name} [-d][+f][+l][-n][-o][-p][-r][+s][-t][-v][-P][-V] -h | -i | FileName
FUNCTION: This program creates a companion RETAIN apar for a given list of
CMVC defects.  The reference field of the defect is updated with the
newly created apar number.
If your CMVC ID is not the same as the login ID you are running under,
then you must have the environment variable CMVC_ID set to your CMVC ID.
PARAMETERS: Either the name of a file is specified as a command line
argument or a list of defects is specified via stdin.  Each line of input is
expected to contain a single CMVC defect number.  If the PE in error mode
is invoked with the -P flag, the defect number must be followed by the PTF
that is to be PEed.
FLAGS:
-c do not logoff of RETAIN when finished
-d (debug mode) actual APARs will not be created, but dummy APAR numbers 
   will be returned as if they were
+f turns off the default action of saving existing copies of all output files
-h displays this help text
-i read the list of defects from standard input
+l all RETAIN screens are NOT to be logged
-n specifies the base name of all output files when input is from stdin
-o the list of defects for which RETAIN apars are created will be written 
   to standard output instead of to the default output file
-p specifies the subdirectory into which all output files are to be written
   (overrides RETAIN_OUTPUT)
-r run only the RETAIN portion of the script 
+s turns off the display of status messages to standard output
-t turns on test mode where the RETAIN test system is used
-u searches unannounced (restricted) database for APARs and PTFs
-w specifies the time to wait for a RETAIN command to finish
   (overrides RETAIN_WAIT)
-x turns on tracing of the script; results are written to stderr
-v display the window used to access RETAIN 
-z run only the CMVC portion of the script 
-P the apar to be created is a PE apar (the apar indicates that a particular
   ptf is in error)
-V the AIX version (defaults to AIX version 3)
EOM
}

function display_msg
{
  print -u2 -r ${@}

  if [[ ${status_on} = 0 ]]
  then
    print -u1 -r ${@}
  fi
}

function log_it
{
  display_msg ${@}

  if [[ ${logging_on} = 0 ]]
  then
    print "~~~~~${@}~~~~~" >> ${retain_log}
    hget -t0 >> ${retain_log}
    print "~~~~~END OF SCREEN~~~~~" >> ${retain_log}
    if hexpect -w "(PF5)"
    then
      hsend -t${RETAIN_WAIT} -pf5
      print "~~~~~ERROR MESSAGE~~~~~" >> ${retain_log}
      hget -t0 >> ${retain_log}
      print "~~~~~END OF SCREEN~~~~~" >> ${retain_log}
      hsend -t${RETAIN_WAIT} -pf5
    fi
  fi
}

function file_cleanup
{
  if ((cmvc_only==1))
  then
    # close output files
    exec 4>&-
    exec 8>&-
    exec 9>&-
  fi

  # close output files
  exec 5>&-
  # close input files
  exec 6<&-
  exec 7<&-
}

#------------------------------------------------------------------
# NAME: get_defect_fields
#
# DESCRIPTION: Get all relevent CMVC fields of the current defect.  If no 
# data is returned, the current number is probably a feature, which has a 
# different set of relevent fields.
#
# PRE CONDITIONS:
#
# POST CONDITIONS: sets values for: prefix, cmvc_component, cmvc_severity, 
#		   cmvc_symptom, reported_release, old_ref, abstract, owner,
#                  cmvc_priority
#
# PARAMETERS: (1) Defect or Feature number
#
# NOTES:
#
# DATA STRUCTURES:
#
# RETURNS: error code if CMVC command fails
#-------------------------------------------------------------------
function get_defect_fields
{
  if [[ ${tracing_on} = 0 ]]
  then
    	set -x
  fi

  if [[ ${MULTI_APAR_DEFECT} = "yes" ]]
  then
      old_ref=$( qryAPARByDefect -d ${1} -v ${aix_version} -r \
		 | awk -F"|" '{print $2}')
  fi

  if [[ ${CMVC_VERSION} = "2" ]]
  then
     data=$(Report -vi defectview -w "name='${1}'" -raw -be ${CMVC_ID} | dv.filter \
         2> ${errmsg_file})
     rc=${?}
  else
     data=$(Report -vi defectview -w "name='${1}'" -raw -be ${CMVC_ID} \
         2> ${errmsg_file})
     rc=${?}
  fi
  if ((rc==1))
  then
    	display_msg "CMVC command to get defect data failed"
    	display_msg "(<${errmsg_file})"
    	return 1
  fi

  if [[ -n ${data} ]]
  then
     #------------------------------------------------------------------
     # parse out needed defect data fields
     # get optional fields and those that may contain blanks separately
     #------------------------------------------------------------------
     print -r "${data}" | awk -F"|" '{print $1,$3,$8,$12}' |
  	read prefix cmvc_component cmvc_severity cmvc_symptom
     reported_release=`echo ${data} | cut -d'|' -f4`
     abstract=`echo ${data} | cut -d'|' -f9`
     owner=`echo ${data} | cut -d'|' -f24`
     if [[ ${MULTI_APAR_DEFECT} != "yes" ]]
     then
         old_ref=`echo ${data} | cut -d'|' -f26`
     fi

     if [[ ${prefix} = 'p' || ${prefix} = 'a' ]]
     then
      	return 0
     fi
  fi

  #------------------------------------------------------------------
  # if we get here, we have a feature instead of a defect number
  #------------------------------------------------------------------
  if [[ ${CMVC_VERSION} = "2" ]]
  then
     data=$(Report -view featureview -where "name='${1}'" -raw \
    	-become ${CMVC_ID}  | fv.filter 2> ${errmsg_file})
     rc=${?}
  else
     data=$(Report -view featureview -where "name='${1}'" -raw \
    	-become ${CMVC_ID}  2> ${errmsg_file})
     rc=${?}
  fi
  if ((rc==1))
  then
	display_msg "CMVC command to get feature data failed"
	display_msg "(<${errmsg_file})"
	return 2
  fi

  if [[ ${prefix} = 'd' ]]
  then
	#----------------------------------------------------------
  	# parse out needed feature data fields
	#----------------------------------------------------------
  	print -r "${data}" | awk -F"|" '{print $1,$3,$10}' |
  	      read prefix cmvc_component cmvc_priority 
  	owner=`echo ${data} | cut -d'|' -f5`
  	abstract=`echo ${data} | cut -d'|' -f7`
	cmvc_severity=3
        if [[ ${MULTI_APAR_DEFECT} != "yes" ]]
        then
	    old_ref=`echo ${data} | cut -d'|' -f18`
        fi

	return 0
  else
	return 3
  fi 
}

#-----------------------------------------------------------------------------
# NAME: get_prefix_and_ref
#
# DESCRIPTION: Get the prefix and reference fields of a defect or feature
# 
# PRE CONDITIONS:
#
# POST CONDITIONS: sets values for: prefix, old_ref
#
# PARAMETERS: (1) Defect or Feature number
#
# NOTES: Called by update_reference.
#
# DATA STRUCTURES:
#
# RETURNS: error code if CMVC command fails
#-----------------------------------------------------------------------------
function get_prefix_and_ref
{
  if [[ ${tracing_on} = 0 ]]
  then
    	set -x
  fi

  if [[ ${MULTI_APAR_DEFECT} = "yes" ]]
  then
      old_ref=$( qryAPARByDefect -d ${1} -v ${aix_version} -r \
		| awk -F"|" '{print $2}')
  fi

  data=$(Report -view defectview -where "name='${1}'" -raw \
         -become ${CMVC_ID}  2> ${errmsg_file})
  if ((${?}==1))
  then
    	display_msg "CMVC command to get defect data failed"
    	display_msg "(<${errmsg_file})"
    	return 1
  fi

  if [[ -n ${data} ]]
  then
     #---------------------------------------------------
     # parse out prefix and reference
     # If the family is aix, we already have the
     # reference field.
     #---------------------------------------------------
     if [[ ${MULTI_APAR_DEFECT} = "yes" ]]
     then
         prefix=$(print -r "${data}" | awk -F"|" '{print $1}')
     else
         print -r "${data}" | awk -F"|" '{print $1,$26}' | read prefix old_ref
     fi

     if [[ ${prefix} = 'p' || ${prefix} = 'a' ]]
     then
      	return 0
     fi
  fi

  #---------------------------------------------------------------
  # if we get here, we have a feature instead of a defect number
  #---------------------------------------------------------------
  data=$(Report -view featureview -where "name='${1}'" -raw \
    	-become ${CMVC_ID}  2> ${errmsg_file})
  if ((${?}==1))
  then
	display_msg "CMVC command to get feature data failed"
	display_msg "(<${errmsg_file})"
	return 2
  fi

  if [[ -n ${data} ]]
  then
     #---------------------------------------------------
     # parse out prefix and reference
     # If the family is aix, we already have the
     # reference field.
     #---------------------------------------------------
     if [[ ${MULTI_APAR_DEFECT} = "yes" ]]
     then
         prefix=$(print -r "${data}" | awk -F"|" '{print $1}')
     else
         print -r "${data}" | awk -F"|" '{print $1,$18}' | read prefix old_ref
     fi
  fi

  if [[ ${prefix} = 'd' ]]
  then
  	return 0
  else
	return 3
  fi 
}

#-----------------------------------------------------------------------------
# NAME: get_track_releases
#
# DESCRIPTION: find all the tracks in the integrate state whose release name 
# ends in the specified release suffix
# 
# PRE CONDITIONS:
#
# POST CONDITIONS: sets values for: cmvc_releases
#
# PARAMETERS: (1) Defect or Feature number
#
# NOTES: 
#
# DATA STRUCTURES:
#
# RETURNS: error code if CMVC command fails
#-----------------------------------------------------------------------------
function get_track_releases
{
 if [[ ${tracing_on} = 0 ]]
 then
   set -x
 fi

 cmvc_releases=$(Report -vi trackview -w "defectname='${1}' and \
   releasename like '%${RELEASE_SUFFIX}' and not state in ('approve','fix')" \
   -raw | awk -F"|" '{printf "%s ", $1}')
 rc=${?}
 # if the command failed
 if ((rc!=0))
 then
    display_msg "CMVC command to get releases failed with return code of ${rc}"
    display_msg "$(<${errmsg_file})"
 else
    if [[ -n ${cmvc_releases} ]]
    then
       return 0
    else
       display_msg "CMVC command to get releases failed to return anything"
       display_msg "$(<${errmsg_file})"
       cmvc_releases=${reported_release}
    fi
 fi
 return ${rc}
}

#-----------------------------------------------------------------------------
# NAME: update_reference
#
# DESCRIPTION: Adds a note to a defect or feature to indicate the RETAIN
# APAR number created for it.  Changes the reference field to the APAR number.
# 
# PRE CONDITIONS:
#    aix_version is set to either 32 or 41.
#
# POST CONDITIONS: 
#
# PARAMETERS: (1) Defect or Feature number
#             (2) RETAIN APAR number
#
# NOTES: Calls 'get_prefix_and_ref'
#
# DATA STRUCTURES:
#
# RETURNS: error code if CMVC command fails
#-----------------------------------------------------------------------------
function update_reference
{
 if [[ ${tracing_on} = 0 ]]
 then
   set -x
 fi

 if get_prefix_and_ref ${1}
 then
    if [[ ${MULTI_APAR_DEFECT} = "yes" ]]
    then
	assocDef2Apar -d ${1} -a ${2} -r ${aix_version}
	return $?
    fi

    if [[ ${prefix} = 'd' ]]
    then
       CMVC_CMD=Feature
    else
       CMVC_CMD=Defect
    fi

    # if test mode is off
    if [[ ${test_mode} = 1 ]]
    then
       # modify only if 'old_ref' has a value, else just add a note
       if [[ -n ${old_ref} ]]
       then
          ${CMVC_CMD} -modify ${1} -ref ${2} -be ${CMVC_ID} -rem \
          "RETAIN APAR: ${2} CREATED FOR THIS PTM, reference was '${old_ref}'" 2> ${errmsg_file}
     	  rc=${?}
     	  # if the command failed
     	  if ((rc!=0))
     	  then
      	     display_msg "CMVC command to set reference field failed with return code of ${rc}"
      	     display_msg "$(<${errmsg_file})"
      	     ${CMVC_CMD} -note ${1} -be ${CMVC_ID} -rem \
      	     "RETAIN APAR: ${2} CREATED FOR THIS PTM, could not change reference field"
      	     return ${rc}
     	  fi
       else
     	  ${CMVC_CMD} -note ${1} -be ${CMVC_ID} -rem \
     	  "RETAIN APAR: ${2} CREATED FOR THIS PTM "
       fi
    else
       ${CMVC_CMD} -note ${1} -be ${CMVC_ID} -rem \
       "TEST RETAIN APAR: T${2} CREATED FOR THIS PTM, reference field is ${old_ref}"
    fi
    return 0
 else
    return 1
 fi
}

#-----------------------------------------------------------------------------
# NAME: get_defect_data
#
# DESCRIPTION: Gets the symptom text from the notes attached to a defect or
# feature and counts the number of lines in the symptom text.  Also removes
# any leading blanks or tabs from the abstract.
# 
# PRE CONDITIONS:
#
# POST CONDITIONS: 
#
# PARAMETERS: (1) Prefix of defect or feature
#             (2) Defect or Feature number
#
# NOTES: Calls 'get_track_releases'
#
# DATA STRUCTURES:
#
# RETURNS: error code if CMVC command fails
#-----------------------------------------------------------------------------
function get_defect_data
{

  if [[ ${tracing_on} = 0 ]]
  then
    set -x
  fi

  #-------------------------------------------------------------------------
  # remove any leading space(s) or tab(s) from the abstract
  # the abstract is a maximum of 63 characters in CMVC, but RETAIN allows
  # up to 128 (64 per line) for the abstract.  This code assumes the CMVC
  # abstract is contained within one record (i.e. has no embedded line ends).
  #-------------------------------------------------------------------------
  while [[ ${abstract} = +([        ])* ]]
  do
     abstract=${abstract#[    ]}
  done
  display_msg "get_defect_data - abstract"
  display_msg "${abstract}"

  if [[ ${1} = 'p' ]]
  then
    #-------------------------------------------------------------------------
    # if the prefix is a 'p' it is a defect
    #
    # get problem symptom (i.e. the text between the lines START_SYMPTOM
    # and STOP_SYMPTOM in the last note in which these two phrases
    # appear on a line by themselves)
    # parse the input record at each vertical bar
    # The first line of the note is the ninth field of the first
    # record of the noteview data.  The note may exceed one line, therefore,
    # all lines after the first will be in a separate record.  These subsequent
    # records will be parsed just like the first one.
    #-------------------------------------------------------------------------
    symptom=$(Report -view noteview -wh "defectname='${2}'" -raw -become ${CMVC_ID} |
	cut -d'|' -f9 |
	awk '
	  BEGIN { stop_found = 0 }
	  /^START_SYMPTOM$/,/^STOP_SYMPTOM$/\
	    {if ($0 == "START_SYMPTOM") {
	     symptom = ""
	     stop_found = 0}
	     if ($0 == "STOP_SYMPTOM") stop_found = 1
	     if ($0 == "START_SYMPTOM" || $0 == "STOP_SYMPTOM") record = NR
	     else addline($0)
	    }
	  END {if (record > 0 && stop_found == 1) print symptom}
	  function addline(line) {
	    if (length(symptom) > 0)
	      symptom = symptom " " line
	    else
	      symptom = line
	  }' 2> ${errmsg_file})

    rc=${?}
    display_msg "get_defect_data - symptom"
    display_msg "${symptom}"
    if ((rc==0))
    then
	number_of_symptom_lines=$(print -r "${symptom}" | wc -l)
	display_msg "number_of_symptom_lines"
	display_msg "${number_of_symptom_lines}"
    else
	display_msg "CMVC command to get defect symptom failed with return code of ${rc}"
	display_msg "(<${errmsg_file})"
	return 2
    fi

  elif [[ ${1} = 'd' ]]
  then
    #-------------------------------------------------------------------------
    # if prefix is 'd' it is a feature
    #
    # get problem symptom (i.e. create note), the remarks entered when
    # the feature was created
    # parse the input record at each vertical bar
    # The first line of the creation note is the ninth field of the first
    # record of the noteview data.  The note may exceed one line, therefore,
    # all lines after the first will be in a separate record.  These subsequent
    # records will be parsed just like the first one.
    # if no input records were received, assume the create note was not found.
    #-------------------------------------------------------------------------
    symptom=$(Report -view noteview \
	-wh "defectname='${2}' and action='create'" -raw -become ${CMVC_ID} |
      awk -F"|" '{if (NF > 1) print $9; else print $0}
	END {if (NR == 0) {print "Description Not Found"}}' 2> ${errmsg_file})

    display_msg "get_defect_data - symptom"
    display_msg "${symptom}"
    rc=${?}
    if ((rc==0))
    then
	number_of_symptom_lines=$(print -r "${symptom}" | wc -l)
    else
	display_msg "CMVC command to get feature symptom failed with return code of ${rc}"
	display_msg "(<${errmsg_file})"
	return 2
    fi
  else
    #-------------------------------------------------------------------------
    # should not be an apar, there has already been a check for an apar
    #-------------------------------------------------------------------------
    return 1
  fi

  #-------------------------------------------------------------------------
  # if the PE in error mode has NOT been indicated (the -P flag)
  #-------------------------------------------------------------------------
  if ((pe_mode==1))
  then
    if get_track_releases ${2}
    then
     return ${?}
    else
     rc=${?}
     display_msg "CMVC command to get defect releases failed with return code of ${rc}"
     display_msg "(<${errmsg_file})"
     return 2
    fi
  fi

}

#-----------------------------------------------------------------------------
# NAME: adjust_symptom
#
# DESCRIPTION: Adjust the selected line(s) of the defect/feature note to a
# maximum length of 63
# 
# PRE CONDITIONS:
#
# POST CONDITIONS: 
#
# PARAMETERS: none
#
# NOTES: Called from main.
#
# DATA STRUCTURES:
#
# RETURNS: 
#-----------------------------------------------------------------------------
function adjust_symptom
{
  #--------------------------------------------------------------------------
  # select every field (word) of the current input line and concatenate them 
  # to the current output line, unless doing so would cause the output line 
  # to exceed 63 characters.
  # if the current input line is empty, print the left over contents of the
  # current output line and then print put an empty line in the output
  #--------------------------------------------------------------------------
  awk '/./  {for (i = 1; i <= NF; i++) addword($i)}
      /^$/ {printline(); print ""}
      END  {printline()}

  #--------------------------------------------------------------------------
  # subroutine to add words to the output line
  # if concatenating the next word to the current output line would make it
  # longer than 63 characters, send the current contents to the output file
  # Concatenate a space and the next available word to the current output
  # line. (If a line has just be sent to the output file, the new output
  # line will be empty.  Therefore, adding a space and the next word to
  # the new line will cause the line to begin with a space.)
  #--------------------------------------------------------------------------
  function addword(w) {
    if (length(line) + length(w) > 63)
      printline()
    line = line " " w
  }

  #--------------------------------------------------------------------------
  # subroutine to print the current output line
  # (The addword routine creates every output line with a space as the 
  # first character.  Therefore, this space must be eliminated when 
  # sending the line to the output file.)
  # reset the current output line to null
  #--------------------------------------------------------------------------
  function printline() {
  if (length(line) > 0) {
      printf "%-63.63s\n", substr(line,2)
      line = ""
    }
  }'
}

#-----------------------------------------------------------------------------
# NAME: write_to_input_area
#
# DESCRIPTION: 
# reads stdin and puts the text into a nine line by 64 character entry area
#
# PRE CONDITIONS:
#
# POST CONDITIONS: 
#
# PARAMETERS: none
#
# NOTES: Called by create_apar
#
# DATA STRUCTURES:
#
# RETURNS: 
#-----------------------------------------------------------------------------
function write_to_input_area
{
  if [[ ${tracing_on} = 0 ]]
  then
    set -x
  fi

  typeset -i input_area_line=0

  while next_line=$(line)
  do
      ((input_area_line=input_area_line+1))
      # if this is the 10th line for this screen
      if [[ ${input_area_line} > 9 ]]
      then
	  # send ENTER to trigger new screen
	  hsend -t${RETAIN_WAIT} -enter
	  display_msg "Get more lines for symptom"
	  if hexpect -t${RETAIN_WAIT} "VERIFY CREATE"
	  then
	    # send PA2 to actually get new screen, 9 more lines
	    hsend -t${RETAIN_WAIT} -pa2
	    sleep 1
	  else
	    display_msg "Description input failed"
	    return 1
	  fi
	  display_msg "Got more lines for symptom"
	  ((input_area_line=1))
      fi
      # there are already line ends in the text
      case "${next_line}" in
      "") hsend -t${RETAIN_WAIT} -n "." -newline;;
      *)  hsend -t${RETAIN_WAIT} -n "$next_line" -newline;;
      esac
  done
  hsend -t${RETAIN_WAIT} -pf11
  sleep 1
  display_msg "Description input finished"
  if hexpect -t${RETAIN_WAIT} "VERIFY CREATE"
  then
    display_msg "Description input successfull"
    hsend -t${RETAIN_WAIT} -pa2
  else
    display_msg "Description input failed"
    return 1
  fi
  return 0
}

#-----------------------------------------------------------------------------
# NAME: apar_already_created
#
# DESCRIPTION: Determines if a RETAIN APAR has been created for a defect.
# 
# PRE CONDITIONS:
#
# POST CONDITIONS: 
#
# PARAMETERS: (1) Defect or Feature number
#
# NOTES: Called by create_apar
#
# DATA STRUCTURES:
#
# RETURNS: True if an APAR has been created for the given defect.
#-----------------------------------------------------------------------------
function apar_already_created
{
 if [[ ${tracing_on} = 0 ]]
 then
   set -x
 fi

 if [[ ${debug_mode} = 0 ]]
 then
   return 1
 else
   # if unannounced mode is on
   if ((unannounced_mode==0))
   then
     hsend -t${RETAIN_WAIT} "n;-/pr/ac/p;INTERNALLY REPORTED DEFECT ${1} ${CMVC_FAMILY}"
   else
     hsend -t${RETAIN_WAIT} "n;-/p;INTERNALLY REPORTED DEFECT ${1} ${CMVC_FAMILY}"
   fi

   if hexpect -t${QUICK_WAIT} "THE ABOVE SEARCH ARGUMENT RESULTED IN     1 MATCH"
   then
     hsend -t${RETAIN_WAIT} "1"
     hsend -t${RETAIN_WAIT} "x"
     if hexpect -t${QUICK_WAIT} "INTERNALLY REPORTED DEFECT ${1} ${CMVC_FAMILY}"
     then
       log_it "APAR ALREADY CREATED FOR THIS DEFECT"
       return 0
     else
       log_it "DEFECT NUMBER NOT FOUND ON SUBMITTER PAGE"
       return 1
     fi
   else
     if [[ ${CMVC_FAMILY} = "aix" ]]
     then
       if ((unannounced_mode==0))
       then
         hsend -t${RETAIN_WAIT} "n;-/pr/ac/p;INTERNALLY REPORTED DEFECT ${1}"
       else
         hsend -t${RETAIN_WAIT} "n;-/p;INTERNALLY REPORTED DEFECT ${1}"
       fi

       if hexpect -t${QUICK_WAIT} "THE ABOVE SEARCH ARGUMENT RESULTED IN     1 MATCH"
       then
         hsend -t${RETAIN_WAIT} "1"
         hsend -t${RETAIN_WAIT} "x"
         if hexpect -t${QUICK_WAIT} "INTERNALLY REPORTED DEFECT ${1}"
         then
           log_it "APAR ALREADY CREATED FOR THIS DEFECT"
           return 0
         else
           log_it "DEFECT NUMBER NOT FOUND ON SUBMITTER PAGE"
           return 1
         fi
       fi
     else
       return 1
     fi
   fi
 fi
}

#-----------------------------------------------------------------------------
# NAME: create_apar
#
# DESCRIPTION: Create a RETAIN APAR for the given Defect or Feature.
#
# PRE CONDITIONS:
#
# POST CONDITIONS: 
#
# PARAMETERS: (1) Prefix of a Defect or Feature number
#             (2) Defect or Feature number
#             (3) if specified, the PTF to be PEed
#
# NOTES: Called from main
#
# DATA STRUCTURES:
#
# RETURNS: 
#-----------------------------------------------------------------------------
function create_apar
{

 typeset -i release_found_in_table=1

 if [[ ${tracing_on} = 0 ]]
 then
   set -x
 fi

 #---------------------------------------------------------------
 # if the PE in error mode has NOT been indicated (the -P flag)
 #---------------------------------------------------------------
 if ((pe_mode==1))
 then
  #---------------------------------------------------------------
  # get the release names
  #---------------------------------------------------------------
  if [[ -n ${cmvc_releases} ]]
  then
   if [[ ${debug_mode} = 0 ]]
   then
     #---------------------------------------------------------------
     # return a dummy apar number and indicate successful creation of a ptf
     #---------------------------------------------------------------
     print -u4 "${2}\tIX${dummy_apar_num}"
     ((dummy_apar_num+=1))
     display_msg ${2} ABSTRACT
     display_msg "${abstract}"
     display_msg ${2} CREATE NOTE
     display_msg "${symptom}"
     display_msg ${2} COMPONENT
     display_msg ${cmvc_component}
     if [[ ${1} = 'p' ]]
     then
       display_msg ${2} RELEASE
       display_msg ${cmvc_releases}
       display_msg ${2} SEVERITY
       display_msg ${cmvc_severity}
       display_msg ${2} SYMPTOM
       display_msg ${cmvc_symptom}
     fi
     return 0
   else
    #---------------------------------------------------------------
    # remove all comment lines in compids.table and save in /tmp/compids.tmp
    #---------------------------------------------------------------
    cat ${RETAIN_PATH}/compids.table | sed -e '/^[#*   ]/d' | sed -e '/^[        ]*$/d' > /tmp/compids.tmp.${$}
    COMPIDS="/tmp/compids.tmp.${$}"

    ifs=${IFS}
    IFS=" "

    ((release_found_in_table=1))
    for cmvc_release in ${cmvc_releases}
    do
     prefix=${cmvc_release%?}
     lastdigit=${cmvc_release#$prefix}
     grep -i $prefix ${COMPIDS} |
     awk -F":" -v lastdigit=$lastdigit '{
                if ( lastdigit >= substr($7,length($7) ) {
                     printf "%s %s %s %s", $2,$4,$5,$7}
     }' |
     read retain_component retain_comp_release retain_sys_release retain_release
     display_msg "cmvc_release=${cmvc_release}"
     display_msg "retain_component=${retain_component}"
     display_msg "retain_comp_release=${retain_comp_release}"
     display_msg "retain_sys_release=${retain_sys_release}"
     if [[ -n ${retain_component} && -n ${retain_comp_release} && -n ${retain_sys_release} ]]
     then
      ((release_found_in_table=0))
      #---------------------------------------------------------------
      # since only one APAR will be created, use the first release 
      # found in the table
      #---------------------------------------------------------------
      break
     fi
    done

    if ((release_found_in_table==1))
    then
     display_msg "CMVC release ${cmvc_release} not found in compids.table"
     grep -i "bos${aix_version}0" ${COMPIDS} |
     awk -F":" '{printf "%s %s %s %s", $2,$4,$5,$7}' |
     read retain_component retain_comp_release retain_sys_release retain_release
     display_msg "cmvc_release=bos${aix_version}0"
     display_msg "retain_component=${retain_component}"
     display_msg "retain_comp_release=${retain_comp_release}"
     display_msg "retain_sys_release=${retain_sys_release}"
     if [[ -z ${retain_component} && -z ${retain_comp_release} && -z ${retain_sys_release} ]]
     then
       display_msg "Using hardcoded bos${aix_version}0 component ID and levels."
       if [ $aix_version = "41" ]
       then
         retain_component="576539300"
       else
         retain_component="575603001"
       fi
       retain_comp_release=${aix_version}0
       retain_sys_release=${aix_version}0
     else
       if [[ -z ${retain_component} || -z ${retain_comp_release} || -z ${retain_sys_release} ]]
       then
         display_msg "Using default bos${aix_version}0 component ID and levels."
	 # use the default values
         if [ $aix_version = "41" ]
         then
	   retain_component="576539300"
         else
           retain_component="575603001"
         fi
	 retain_comp_release=${aix_version}0
	 retain_sys_release=${aix_version}0
       fi
     fi
    fi

    # remove temp file
    rm -f ${COMPIDS}

    IFS=${ifs}
   fi
  else
   log_it "THERE ARE NO TRACKS FOR DEFECT ${2}"
   return 1
  fi
 else
  #-------------------------------------------
  # the PE in error mode has been invoked
  # display the ptf summary screen
  #-------------------------------------------
  hsend -t${RETAIN_WAIT} "n;-/r ${3}"
  #------------------------------------------------------------------
  # if the ptf was found, make sure it is closed cor or per
  #------------------------------------------------------------------
  if hexpect -t${RETAIN_WAIT} "@1,2:PTF= ${3}"
  then
   ptf_status=$(hget -t0 "1,45:1,54")
   if [[ ${ptf_status} = "CLOSED COR" || ${ptf_status} = "CLOSED PER" ]]
   then
    retain_component=$(hget -t0 "4,9:4,17")
    retain_comp_release=$(hget -t0 "1,58:1,60")
    #------------------------------------------------------------------
    # display ptf's cover letter to display system release level
    #------------------------------------------------------------------
    hsend -t${RETAIN_WAIT} "l"
    if hexpect -t${RETAIN_WAIT} "@17,2:APPLICABLE RELEASE:"
    then
     retain_sys_release=$(hget -t0 "17,28:17,30")
    else
     log_it "COULD NOT FIND APPLICABLE RELEASE FOR PTF ${3}"
     return 1
    fi
   else
     log_it "THE PTF IN ERROR (${3}) IS NOT CLOSED COR NOR PER"
     return 1
   fi
  else
   log_it "THE PTF IN ERROR (${3}) WAS NOT FOUND"
   return 1
  fi
 fi

 display_msg "retain_component=${retain_component}"
 display_msg "retain_comp_release=${retain_comp_release}"
 display_msg "retain_sys_release=${retain_sys_release}"

 if apar_already_created ${2}
 then
   existing_apar=$(hget -t0 "1,8:1,14")
   print -u4 "${2}\t${existing_apar}\tAPAR ALREADY CREATED"
 else
   hsend -t${RETAIN_WAIT} "n;ssf/ca"
   if hexpect -t${RETAIN_WAIT} "ENTER FIELDS FOR VERIFICATION BY SSF:"
   then
     log_it "First CA screen - blank"
   else
    log_it "Failed to reach first CA screen"
    if hexpect -t${RETAIN_WAIT} "RETAIN USER SATISFACTION SURVEY"
    then
      hsend -t${RETAIN_WAIT} -pf1
      if hexpect -t${RETAIN_WAIT} "ENTER FIELDS FOR VERIFICATION BY SSF:"
      then
	log_it "First CA screen - blank"
      else
	hsend -t${RETAIN_WAIT} -pf1
	hsend -t${RETAIN_WAIT} "n;ssf/ca"
	if hexpect -t${RETAIN_WAIT} "ENTER FIELDS FOR VERIFICATION BY SSF:"
	then
	  log_it "First CA screen - blank"
	else
	  log_it "Failed to reach first CA screen"
	  return 1
	fi
      fi
    else
      log_it "Failed to reach first CA screen"
      return 1
    fi
   fi

   display_msg "retain_component=${retain_component}"
   display_msg "retain_comp_release=${retain_comp_release}"
   display_msg "retain_sys_release=${retain_sys_release}"


   hsend -t${RETAIN_WAIT} -n "@3,30" "${retain_component}"
   # if the PE in error mode is on
   if ((pe_mode==0))
   then
    hsend -t${RETAIN_WAIT} -n "@3,65" "Y"
   fi
   hsend -t${RETAIN_WAIT} -n "@4,30" "${retain_comp_release}"
   hsend -t${RETAIN_WAIT} -n "@5,30" "${retain_sys_release}"
   hsend -t${RETAIN_WAIT} -n "@6,30" "0009999900"
   # 916 is the Austin branch office number, 999 is a Boulder BO number
   hsend -t${RETAIN_WAIT} -n "@6,67" "916"
   hsend -t${RETAIN_WAIT} -n "@7,30" "in"
   hsend -t${RETAIN_WAIT} -n "@8,30" "incorrout"
   hsend -t${RETAIN_WAIT} -n "@9,30" "${cmvc_severity}"
   hsend -t${RETAIN_WAIT} -n "@10,30" "${2}"
   hsend -t${RETAIN_WAIT} -n "@13,12" "${abstract}"
   hsend -t${RETAIN_WAIT} -enter
   if hexpect -t${RETAIN_WAIT} "VERIFY CREATE"
   then
    log_it "First CA screen - filled in"
    hsend -t${RETAIN_WAIT} -pa2
    if hexpect -t${RETAIN_WAIT} "ENTER SUBMITTOR INFORMATION"
    then
     log_it "Second CA screen - blank"
     hsend -t${RETAIN_WAIT} -n "${owner}" -tab "."
     hsend -t${RETAIN_WAIT} -enter
     if hexpect -t${RETAIN_WAIT} "VERIFY CREATE"
     then
      log_it "Second CA screen - filled in"
      hsend -t${RETAIN_WAIT} -pa2
      if hexpect -t${RETAIN_WAIT} "ENTER OPTIONAL FIELDS:"
      then
       log_it "Third CA screen - blank (skipping it)"
       hsend -t${RETAIN_WAIT} -pf11
       # if the PE in error mode is on
       if ((pe_mode==0))
       then
	if hexpect -t${RETAIN_WAIT} "ENTER APPLICABLE PTF NUMBERS"
	then
	 hsend -t${RETAIN_WAIT} "${3}"
	 if hexpect -t${RETAIN_WAIT} "VERIFY ENTRY - (PA2)"
	 then
	  log_it "APPLICABLE PTF NUMBERS screen - filled in"
	  hsend -t${RETAIN_WAIT} -pa2
	 else
	  log_it "Failed to confirm input for APPLICABLE PTF NUMBERS"
	  return 1
	 fi
	else
	 log_it "Failed to reach PE ptfs screen"
	 return 1
	fi
       fi
       if hexpect -t${RETAIN_WAIT} "LIST COMPONENT LEVEL'S/SU'S "
       then
	log_it "Fourth CA screen - blank"
	hsend -t${RETAIN_WAIT} ${retain_comp_release}
	if hexpect -t${RETAIN_WAIT} "VERIFY ENTRY - (PA2)"
	then
	 log_it "Fourth CA screen - filled in"
	 hsend -t${RETAIN_WAIT} -pa2
	 if hexpect -t${RETAIN_WAIT} "SELECT MATERIALS SUBMITTED WITH APAR:"
	 then
	  log_it "Fifth CA sreen - blank (skipping it)"
	  hsend -t${RETAIN_WAIT} -pf11
	  if hexpect -t${RETAIN_WAIT} "2.SYS EXECUTION"
	  then
	   log_it "Sixth CA screen"
	   hsend -t${RETAIN_WAIT} "2"
	   if hexpect -t${RETAIN_WAIT} "1.ABENDS"
	   then
	    log_it "Seventh CA screen"
	    hsend -t${RETAIN_WAIT} "1"
	    if hexpect -t${RETAIN_WAIT} \
	      "UAAAA - U for user/application issued ABENDs"
	    then
	     log_it "Eighth CA screen"
	     hsend -t${RETAIN_WAIT} "u0000"
	     if hexpect -t${RETAIN_WAIT} \
	      "Unknown.......Reply.......na"
	     then
	      log_it "Ninth CA screen"
	      hsend -t${RETAIN_WAIT} "na"
	      if hexpect -t${RETAIN_WAIT} "1.ABEND OCCURRED"
	      then
	       log_it "Tenth CA screen"
	       hsend -t${RETAIN_WAIT} "1"
	       if hexpect -t${RETAIN_WAIT} "TO BYPASS SELECTION PRESS ENTER"
	       then
		log_it "Eleventh CA screen"
		hsend -t${RETAIN_WAIT} -enter
		if hexpect -t${RETAIN_WAIT} "1.PGM INDICATION"
		then
		 log_it "Twelveth CA screen"
		 hsend -t${RETAIN_WAIT} "1"
		 if hexpect -t${RETAIN_WAIT} "TO BYPASS SELECTION PRESS ENTER"
		 then
		  log_it "Thirteenth CA screen"
		  hsend -t${RETAIN_WAIT} -enter
		  if hexpect -t${RETAIN_WAIT} "TO BYPASS SELECTION PRESS ENTER"
		  then
		   log_it "Fourteenth CA screen"
		   hsend -t${RETAIN_WAIT} -enter
		   if hexpect -t${RETAIN_WAIT} "TO BYPASS SELECTION PRESS ENTER"
		   then
		    log_it "Fifteenth CA screen"
		    hsend -t${RETAIN_WAIT} -enter
		    if hexpect -t${RETAIN_WAIT} "6. EXIT"
		    then
		     log_it "Sixteenth CA screen"
		     hsend -t${RETAIN_WAIT} "6"
		     if hexpect -t${RETAIN_WAIT} \
		       "ENTER PROBLEM ERROR DESCRIPTION"
		     then
		      log_it "Error Description CA screen"
		      hsend -t${RETAIN_WAIT} -n "THIS APAR CREATED TO DELIVER INTERNALLY REPORTED DEFECT ${2} ${CMVC_FAMILY}" -newline
		      hsend -t${RETAIN_WAIT} -n "VERS=PTMCVTFNX" -newline
		      # if the original reference field was an APAR number for
		      # another product
		      if [[ ${old_ref} = HB[0-9][0-9][0-9][0-9][0-9] ]]
		      then
		       hsend -t${RETAIN_WAIT} -n "THIS AIX APAR CORRESPONDS TO APAR ${old_ref}" -newline
		      fi
		      hsend -t${RETAIN_WAIT} -enter
		      log_it "First two lines of error symptom"
		      hsend -t${RETAIN_WAIT} -pa2
		      print -r "${adjusted_symptom}" | write_to_input_area
		      if hexpect -t${RETAIN_WAIT} \
			"ENTER ANY 'LOCAL FIX' INFORMATION"
		      then
		       log_it "Local Fix CA screen - blank (skipping)"
		       hsend -t${RETAIN_WAIT} -pf11
		       if hexpect -t${RETAIN_WAIT} "CREATE COMPLETE: "
		       then
			new_apar=$(hget -t0 "1,8:1,14")
			print -u4 "${2}\t${new_apar}"
			log_it "Creation of APAR completed successfully"
			# the reference field will be updated at the
			# end of the program along with those for the
			# other defects
			return 0
		       else
			log_it "Creation of APAR failed"
			return 1
		       fi
		      else
		       log_it "Failed to reach local fix screen"
		       return 1
		      fi
		     else
		      log_it "Failed to reach error description screen"
		      return 1
		     fi
		    else
		     log_it "Failed to reach 16th CA screen"
		     return 1
		    fi
		   else
		    log_it "Failed to reach 15th CA screen"
		    return 1
		   fi
		  else
		   log_it "Failed to reach 14th CA screen"
		   return 1
		  fi
		 else
		  log_it "Failed to reach 13th CA screen"
		  return 1
		 fi
		else
		 log_it "Failed to reach 12th CA screen"
		 return 1
		fi
	       else
		log_it "Failed to reach 11th CA screen"
		return 1
	       fi
	      else
	       log_it "Failed to reach 10th CA screen"
	       return 1
	      fi
	     else
	       log_it "Failed to reach 9th CA screen"
	       return 1
	     fi
	    else
	     log_it "Failed to reach 8th CA screen"
	     return 1
	    fi
	   else
	     log_it "Failed to reach 7th CA screen"
	     return 1
	   fi
	  else
	   log_it "Failed to reach 6h CA screen"
	   return 1
	  fi
	 else
	   log_it "Failed to reach fifth CA screen"
	   return 1
	 fi
	else
	 log_it "Error on fourth CA screen"
	 return 1
	fi
       else
	log_it "Failed to reach fourth CA screen"
	return 1
       fi
      else
       log_it "Failed to reach third CA screen"
       return 1
      fi
     else
       log_it "Error on second CA screen"
       return 1
     fi
    else
      log_it "Failed to reach second CA screen"
      return 1
    fi
   else
    log_it "Problem with first CA screen"
    return 1
   fi
 fi
}

function main
{
  if [[ ${tracing_on} = 0 ]]
  then
    set -x
  fi

  #---------------------------------------------------------
  # Set all default values
  #---------------------------------------------------------
  RETAIN_SYSTEM="${RETAIN_SYSTEM:=BDC}"

  #---------------------------------------------------------
  # set font for the emulator
  #---------------------------------------------------------
  HTN_OPTIONS="${HTN_OPTIONS:=-fzn Rom6.500}"

  #---------------------------------------------------------
  # list hosts to which the connection to VTAM should be tried
  #---------------------------------------------------------
  HOST_LIST="${HOST_LIST:=ausvm1 ausvm2 ausvm6 ausvmq}"

  #---------------------------------------------------------
  # time to wait for expected data to appear
  #---------------------------------------------------------
  RETAIN_WAIT="${RETAIN_WAIT:=5}"
  QUICK_WAIT="${QUICK_WAIT:=5}"

  #---------------------------------------------------------
  # subdirectory into which all output files are to be stored
  #---------------------------------------------------------
  RETAIN_OUTPUT="${RETAIN_OUTPUT:=.}"

  VM_PROMPT="${VM_PROMPT:=Ready}"

  #---------------------------------------------------------
  # if the environment variable CMVC_ID is not set, assume 
  # that the current login id is also the user's CMVC id
  #---------------------------------------------------------
  CMVC_ID="${CMVC_ID:=$(logname)}"

  #---------------------------------------------------------
  # if the environment variable CMVC_VERSION is not set, assume 
  # that the new CMVC release is used. 
  #---------------------------------------------------------
  CMVC_VERSION=${CMVC_VERSION:=2}

  #---------------------------------------------------------
  # path to find compids.table file
  #---------------------------------------------------------
  if [ ${aix_version} = "32" ]; then
     RETAIN_PATH="${RETAIN_PATH:=/afs/austin/aix/320/bldenv/prod/usr/bin}"
  else
     RETAIN_PATH="${RETAIN_PATH:=${ODE_TOOLS:-/afs/austin/aix/410/project/aix4/build/latest/ode_tools/power}/usr/lib}"
  fi

  typeset -i RETAIN_RETRY_WAIT
  RETAIN_RETRY_WAIT="${RETAIN_RETRY_WAIT=5*RETAIN_WAIT}"

  #-----------------------------------------------------------------------
  # if the release suffix environment variable is not set, use the default
  #-----------------------------------------------------------------------
  RELEASE_SUFFIX="${RELEASE_SUFFIX:=${aix_version}0}"

  typeset -i vm_connection_made=1
  typeset -i vtam_connection_made=1
  typeset -i retain_connection_made=1
  typeset -i retain_login_successful=1

  if [[ ${CMVC_VERSION} = "2" ]]
  then
     typeset -LZ defect_num
  else
     typeset -RZ6 defect_num
  fi

  #---------------------------------------------------------
  # if the standard input flag was not specified, the first
  # command parameter is taken to be the name of the input file
  #---------------------------------------------------------
  if [[ ${from_stdin} = 1 ]]
  then
    if [[ -n ${1} ]]
    then
      #------------------------------------------
      # open noref input file as descriptor 3
      #------------------------------------------
      exec 3< ${1}
      ((input_data=3))
      display_msg "Reading input from ${1}"
    else
      display_msg \
	"No input file name was specified and no list of defects was supplied"
      display_msg "No action will be taken."
      exit 2
    fi

    #-----------------------------------------------------------------------
    # create the base name of the output files and relate them to the base
    # name of the input file.
    # set the base name of the output files to the RETAIN_OUTPUT variable
    #-----------------------------------------------------------------------
    base_name=${1##*/}
    base_name=${base_name%.*}
    base_name="${RETAIN_OUTPUT}/${base_name}"

  else
    #---------------------------
    # read input from stdin
    #---------------------------
    display_msg "Reading defect list from standard input"
    ((input_data=0))
    #-----------------------------------------------------------------------
    # create the base name of the output files and relate them to the name
    # of this program
    # if the base name was not specified with the -n flag
    #-----------------------------------------------------------------------
    if [[ -z ${base_name} ]]
    then
      base_name="${RETAIN_OUTPUT}/${action}"
    else
      base_name="${RETAIN_OUTPUT}/${base_name}"
    fi
  fi

  created_file="${base_name}.created"
  notcreated_file="${base_name}.notcreated"
  updated_file="${base_name}.updated"
  notupdated_file="${base_name}.notupdated"
  cmvc_file="${base_name}.${action}.cmvc"
  retain_log_base_name="${base_name}.${action}"
  retain_log="${retain_log_base_name}.retainlog"
  errmsg_file="${base_name}.${action}.errmsg"

  #---------------------------------------------
  # if the save files mode was not turned off,
  # save any existing versions of the output files
  #---------------------------------------------
  if [[ ${save_files} = 0 ]]
  then
    if [[ -f ${notcreated_file} ]]
    then
      mv ${notcreated_file} "${notcreated_file}${time_suffix}"
    fi
  fi

  #---------------------------------------------
  # open notcreated output file as descriptor 5
  #---------------------------------------------
  exec 5> ${notcreated_file}

  #---------------------------------------------
  # if the retain only flag was NOT specified
  #---------------------------------------------
  if [[ ${retain_only} = 1 ]]
  then
    # if the save files mode was not turned off
    if [[ ${save_files} = 0 && -f ${cmvc_file} ]]
    then
      mv ${cmvc_file} ${cmvc_file}${time_suffix}
      display_msg "Saving existing intermediate data file"
    fi

    display_msg "Opening intermediate data file as ${cmvc_file}"
    #---------------------------------------------------
    # open cmvc data output file as descriptor 7
    #---------------------------------------------------
    exec 7> ${cmvc_file}

    while read -u${input_data} defect_num ptf_in_error remainder
    do
      get_defect_fields ${defect_num}
      if [[ ${prefix} != 'a' ]]
      then
	if get_defect_data ${prefix} ${defect_num}
	then
          #---------------------------------------
	  # write to the intermediate data file
          #---------------------------------------
	  print -u7 -r "${prefix}|${defect_num}|${old_ref}|${owner}|${cmvc_component}|${cmvc_severity}|${cmvc_symptom}|${reported_release}|${cmvc_releases}|${abstract}|${number_of_symptom_lines}|${ptf_in_error}"
	  print -u7 -r "${symptom}"
	else
	  print -u5 "${defect_num}\tCOULD NOT GET DATA"
	fi
      else
	print -u5 "${defect_num}\tIS AN APAR ALREADY OR UNDETERMINED"
      fi
    done

    display_msg "All CMVC data extracted, closing intermediate data file"
    #-------------------------
    # close CMVC data file
    #-------------------------
    exec 7<&-
  fi

  if ((cmvc_only==0))
  then
    # don't run RETAIN portion of the program
    :
  else
    #------------------------------------------
    # if the debug mode has not been set
    #------------------------------------------
    if [[ ${debug_mode} = 1 ]]
    then
      LogonRetain ${logging_option} ${tracing_option} ${viewing_option} \
	${files_option} ${status_option} ${test_option} ${wait_option} \
	-n ${retain_log_base_name} ${path_option}
      if [[ ${?} = 0 ]]
      then
	# continue
	:
      else
	log_it "Could not gain access to RETAIN"
	hsend -t${RETAIN_WAIT} -t0 undial
	exit 2
      fi
    fi

    #-------------------------------------------------
    # if the save files mode was not turned off,
    # save any existing versions of the output files
    #-------------------------------------------------
    if [[ ${save_files} = 0 ]]
    then
      if [[ -f ${created_file} ]]
      then
	mv ${created_file} "${created_file}${time_suffix}"
      fi
    fi

    #-------------------------------------------------
    # if the -o flag was not specified, open created apar
    # output file as descriptor 4, else assign file 4 to stdout
    #-------------------------------------------------
    if [[ ${to_stdout} = 1 ]]
    then
      exec 4> ${created_file}
    else
      exec 4<&1
    fi

    if [[ ${retain_only} = 1 ]]
    then
      #-------------------------------------------------
      # if the retain only flag was NOT specified, reopen
      # cmvc data output file as descriptor 7 for input
      #-------------------------------------------------
      display_msg "Reopening intermediate data file for input"
      exec 7< ${cmvc_file}
    else
      #-------------------------------------------------
      # if the standard input flag was NOT specified,
      # redirect descriptor 7 from the input file, else
      # redirect descriptor 7 from standard input
      #-------------------------------------------------
      if [[ ${from_stdin} = 1 ]]
      then
	display_msg "Reading intermediate data from file ${1}"
	exec 7<&3
      else
	display_msg "Reading intermediate data from standard input"
	exec 7<&0
      fi
    fi

    default_ifs=${IFS}
    IFS="|"

    while read -u7 -r prefix defect_num old_ref owner cmvc_component cmvc_severity \
      cmvc_symptom reported_release cmvc_releases abstract number_of_symptom_lines ptf_in_error
    do
      display_msg "prefix defect_num old_ref owner cmvc_component cmvc_severity cmvc_symtom reported_release cmvc_releases number_of_symptom_lines"
      display_msg "${prefix} ${defect_num} ${old_ref} ${owner} ${cmvc_component} ${cmvc_severity} ${cmvc_symtom} ${reported_release} ${cmvc_releases} ${number_of_symptom_lines}"
      display_msg "abstract"
      display_msg "${abstract}"
      IFS=${default_ifs}
      ((i=1))
      while ((i <= number_of_symptom_lines))
      do
	read -u7 -r next_line
	display_msg "next_line - ${i}"
	display_msg "${next_line}"
	if ((i==1))
	then
	  symptom="${next_line}"
	  display_msg "accumulated symptom - ${i}"
	  display_msg "${symptom}"
	else
	  symptom="${symptom}\n${next_line}"
	  display_msg "accumulated symptom - ${i}"
	  display_msg "${symptom}"
	fi
	((i+=1))
      done
      display_msg "entire symptom"
      display_msg "${symptom}"
      adjusted_symptom=$(print -r "${symptom}" | adjust_symptom)
      display_msg "adjusted_symptom"
      display_msg "${adjusted_symptom}"
      IFS="|"
      if create_apar ${prefix} ${defect_num} ${ptf_in_error}
      then
	display_msg "APAR CREATED FOR ${defect_num}"
      else
	display_msg "FIRST ATTEMPT TO CREATE APAR FOR ${defect_num} FAILED"
	hsend -t${RETAIN_WAIT} -pf1
	# try once more (after a delay), then give up if it fails a second time
	display_msg "SLEEPING FOR ${RETAIN_RETRY_WAIT} SECONDS BEFORE TRYING AGAIN"
	sleep ${RETAIN_RETRY_WAIT}
	if create_apar ${prefix} ${defect_num} ${ptf_in_error}
	then
	  display_msg "APAR CREATED FOR ${defect_num}"
	else
	  print -u5 "${defect_num}\tAPAR CREATION FAILED"
	  hsend -t${RETAIN_WAIT} -pf1
	fi
      fi
    done

    IFS=${default_ifs}

    #---------------------------
    # close intermediate file
    #---------------------------
    exec 7<&-
    #---------------------------
    # close output files
    #---------------------------
    exec 4>&-
    exec 5>&-

    if [[ ${debug_mode} = 1 ]]
    then
      if [[ ${continuous_on} = 1 ]]
      then
        #-----------------------------------------------------------
        # if the continue RETAIN login flag is NOT on, logoff RETAIN
        #-----------------------------------------------------------
	hsend -t${RETAIN_WAIT} -home
	hsend -t${RETAIN_WAIT} logoff
	log_it "Logged off RETAIN"
	hsend -t0 undial
      fi
    fi

    #--------------------------------------------------------------
    # if the save files mode was not turned off, save output files
    #--------------------------------------------------------------
    if [[ ${save_files} = 0 ]]
    then
      if [[ -f ${updated_file} ]]
      then
	mv ${updated_file} "${updated_file}${time_suffix}"
      fi
      if [[ -f ${notupdated_file} ]]
      then
	mv ${notupdated_file} "${notupdated_file}${time_suffix}"
      fi
    fi

    #--------------------------------------------------------------
    # open updated reference output file as descriptor 8
    # open created apar output file for input as descriptor 6
    #--------------------------------------------------------------
    exec 8> ${updated_file}
    exec 6< ${created_file}

    # ******************CHANGE TO HANDLING OF notupdated FILE ********
    # copy not created output file to not updated output file as
    # its initial contents (defects for which apars were not created
    # cannot possibly be updated either)
    #cp -p ${notcreated_file} ${notupdated_file}

    # open for append the not updated reference output file as descriptor 9
    #exec 9>> ${notupdated_file}
    # ******************CHANGE TO HANDLING OF notupdated FILE ********

    #--------------------------------------------------------------
    # open the not updated reference output file as descriptor 9
    #--------------------------------------------------------------
    exec 9> ${notupdated_file}

    while read -u6 defect_num apar_num remainder
    do
      if [[ ${defect_num} = [0-9][0-9][0-9][0-9][0-9][0-9] && ${apar_num} = [iI][xX][0-9][0-9][0-9][0-9][0-9] ]]
      then
 	update_reference ${defect_num} ${apar_num}
	rc=${?}
	if ((rc==0))
	then
	  print -u8 "${defect_num}\t${apar_num}"
	else
	  print -u9 "${defect_num}\t${apar_num}\tNOT UPDATED RC=${rc}"
	fi
      else
	print -u9 "${defect_num}\t${apar_num}\tBAD DATA"
      fi
    done

  fi

  return 0
} ## END OF MAIN ##

function bailout
{
  if [[ ${tracing_on} = 0 ]]
  then
    set -x
  fi

  file_cleanup

  display_msg "CreateApar Program Interrupted"
  htn_running=$(ps -e | grep htn | awk '{printf "%s", $1}')
  if [[ ${#htn_running} > 0 ]]
  then
    hsend -t0 -pf1
    hsend -t0 logoff
    hsend -t0 undial
    htn_running=$(ps -e | grep htn | awk '{printf "%s", $1}')
    if [[ ${#htn_running} > 0 ]]
    then
      kill -1 ${htn_running}
    fi
  fi
  exit 3
}

##### START OF PROGRAM #####
trap "bailout" HUP INT QUIT TERM

#------------------------------------
# find base name of the program
#------------------------------------
program_name=${0##*/}

display_msg "RUNNING ${program_name} WITH PARAMETERS ${@}"

#-----------------------------------------------------------------------------
# remove any program name suffix to use remaining portion to name output files
#-----------------------------------------------------------------------------
action=${program_name%.*}

typeset -i rc=0
typeset -i debug_mode=1
typeset -i test_mode=1
typeset -i save_files=0
typeset -i from_stdin=1
typeset -i input_data=0
typeset -i to_stdout=1
typeset -i status_on=0
typeset -i tracing_on=1
typeset -i logging_on=0
typeset -i retain_only=1
typeset -i cmvc_only=1
typeset -i continuous_on=1
typeset -i unannounced_mode=1
typeset -i pe_mode=1
test_option=""
files_option=""
status_option=""
tracing_option=""
logging_option=""
viewing_option=""
wait_option=""
path_option=""
aix_version="32"
typeset -i dummy_apar_num=90000
typeset -i index
typeset -i number_of_symptom_lines

#---------------------------------------
# check for command line options
#---------------------------------------
while getopts :cdhifln:op:rstuvw:xzPV: next_option
do
  case ${next_option} in
  c) ((continuous_on=0));;
  d) ((debug_mode=0));;
  h) display_help
     exit 1;;
  i) ((from_stdin=0));;
  +f) ((save_files=1))
      files_option="+f";;
  +l) ((logging_on=1))
      logging_option="+l";;
  n) base_name=${OPTARG};;
  o) ((to_stdout=0));;
  p) if [[ ${#OPTARG} > 0 ]]
     then
       path_option="-p${OPTARG}"
       RETAIN_OUTPUT=${OPTARG}
     fi
     ;;
  r) ((retain_only=0));;
  +s) ((status_on=1))
     status_option="+s";;
  t) ((test_mode=0))
     test_option="-t";;
  u) ((unannounced_mode=0));;
  v) viewing_option="-v";;
  w) if [[ ${#OPTARG} > 0 ]]
     then
       wait_option="-w${OPTARG}"
       RETAIN_WAIT=${OPTARG}
     fi
     ;;
  x) ((tracing_on=0))
     tracing_option="-x";;
  z) ((cmvc_only=0));;
  P) ((pe_mode=0));;
  V) echo ${OPTARG} | grep "^4" >/dev/null
     if [ $? -eq 0 ]; then
        aix_version="41"
     fi;;
  :) display_msg "${program_name}: ${OPTARG} requires a value"
     exit 2;;
  \?) display_msg "${program_name}: unknown option ${OPTARG}"
      exit 2;;
  esac
done
#---------------------------------------------------------
# shift the command line parameters to the left as many 
# positions as there are flags
#---------------------------------------------------------
shift OPTIND-1

if [[ ${to_stdout} = 0 ]]
then
  ((status_on=1))
fi

if [[ ${status_on} = 1 ]]
then
  status_option="+s";
fi

time_suffix=".$(date +%m).$(date +%d).$(date +%H):$(date +%M):$(date +%S)"

#---------------------------------------------------------------
# no write permission by group and other for all created files
#---------------------------------------------------------------
umask 022

#--------------------
# call main routine
#--------------------
main ${@}
rc=${?}
file_cleanup
exit ${rc}
##### END OF PROGRAM #####
