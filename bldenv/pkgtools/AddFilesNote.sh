#!/bin/ksh
# @(#)98        1.15  src/bldenv/pkgtools/AddFilesNote.sh, pkgtools, bos41B, 9504A 1/12/95 12:58:40
#
# COMPONENT_NAME: (RETAIN) Tools for interfacing to the Retain system
#
# FUNCTIONS: AddFilesNote
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1991
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

function display_help
{
cat <<-EOM
SYNTAX: ${program_name} [-a][-b][-c][-d][+f][-i][+l][-n][-o][-p][-r][+s][-t]
[-u][-v][-w][-x][-z][-V] -i | FileName | -h
FUNCTION: This program updates a RETAIN PTF with files that were shipped
by the PTF. This file list is taken from the ".mstr.preqtbl" files for the
LPP to which to the PTF applies.  This requires access to the afs directory
"/afs/austin/aix/320/UPDATES" which is read protected.
If your CMVC ID is not the same as the login ID your are running under,
then you must have the environment variable CMVC_ID set to your CMVC ID.
PARAMETERS: Either the name of a file is specified as a command line argument
or a list of ptfs followed its corresponding apars is specified via stdin.
For both types of input, the format is: one ptf number per line followed
by all the apars it fixes on the same line.  For alternate mode, the defect
is substituted for the apar list.
FLAGS:
-a turns on alternate mode in which the defect list follows the PTF number
   instead of the APAR list
-b specifies that the specified RETAIN ID should be used
   (the default is to use the first one listed in $HOME/.netrc)
-c do not logoff of RETAIN when finished
-d (debug mode) indicates that actual
+f turns off the default action of saving existing copies of all output files
-h displays this help text
-i indicates that the input data is to be read from standard input
+l indicates that all RETAIN screens are NOT to be logged
-n specifies the base name of all output files when input is from stdin
-o indicates that the list of PTFs for which the files list is updated
   is to be written to standard output instead of to the default output file
-p specifies the subdirectory into which all output files are to be written
   (overrides RETAIN_OUTPUT)
-r indicates that the only the RETAIN portion of the script needs to be run
+s turns off the displaying of status messages to standard output
-t turns on test mode where the RETAIN test system is used
-u searches unannounced (restricted) database for APARs and PTFs
-v indicates that the window which is used to access RETAIN is to be displayed
-V specifies the AIX version (defaults to AIX version 3)
-w specifies the time to wait for a RETAIN command to finish
   (overrides RETAIN_WAIT)
-x turns on tracing of the script, results are written to stderr
-z indicates that the only the CMVC portion of the script is to be run
EOM
}

function display_msg
{
  print -u2 ${@}

  if [[ ${status_on} = 0 ]]
  then
    print -u1 ${@}
  fi
}

function log_it
{
  # all parameters are assumed to be part of message to be displayed
  display_msg ${@}

  # if debug mode has NOT been set
  if ((debug_mode==1))
  then
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
  fi
}

function file_cleanup
{
  # if the cmvc only flag was NOT turned on
  if ((cmvc_only==1))
  then
    # close "ptffiles" output file
    exec 4<&-
    # close "noptffiles" output file
    exec 5<&-
  fi

  # if the retain only flag was NOT turned on
  if ((retain_only==1))
  then
    # close "deffiles" output file
    exec 6<&-
    # close "nodeffiles" output file
    exec 7<&-
  fi

  # close the input file
  exec 3<&-

  rm ${errmsg_file}
}

function command_expect
{
 # this function searches all the possible positions on the screen in
 # which a given "allowed" command may appear

 # the first parameter must be the command that is being searched for

 typeset -i row=3
 typeset -i command_length
 typeset -i end_position

 ((rc=1))
 command_length=${#1}
 ((end_position=70+command_length-1))
 while ((row<=15))
 do
  command=$(hget -t1 ${row},70:${row},${end_position})
  if [[ ${command} = ${1} ]]
  then
   ((rc=0))
   break
  else
   ((row=row+1))
  fi
 done
 return ${rc}
}

function get_lpp_name
{
  # first parameter must be the PTF number

  if ((tracing_on==0))
  then
    set -x
  fi

  # if test mode is NOT on and debug mode is NOT on
  if ((test_mode==1)) && ((debug_mode==1))
  then
    # check for the presence of the PTF in the ptfapardef file on the PROD server
    ptf_entries=$(fgrep -i ${1} ${PROD_DIR}/ptfapardef.ship)
    rc=${?}
    if ((rc==0))
    then
      # search the input file to find the base lpp for this ptf
      lpp_name=$(fgrep -i "${1}|" ${PROD_DIR}/ptfapardef.ship |
	awk -F"|" 'BEGIN {lpp = ""}
		   {lpp = $2}
		   END {print lpp}
		  ')
    else
      print -u${badptffiles_filedes} "${1}\tNO ENTRIES IN ${PROD_DIR}/ptfapardef.ship"
      return 1
    fi
  else
    lpp_name="?"
  fi

  # if the lpp name was found
  if [[ -n ${lpp_name} ]]
  then
    return 0
  else
    print -u${badptffiles_filedes} "${1}\tLPP NAME NOT FOUND"
    return 2
  fi
}

function get_files_from_build
{
  # first parameter must be the PTF number
  # second parameter must be the lpp name

  if ((tracing_on==0))
  then
    set -x
  fi

  # if test mode is NOT on and debug mode is NOT on
  if ((test_mode==1)) && ((debug_mode==1))
  then
    # Search the corresponding lpp's UPDATE subdirectory (e.g. bos for the
    # bos lpp) to find the files shipped
    #  The names of the files that need to be searched are begin with the
    #  base lpp name and end with the suffix .mstr.preqtbl
    #  Example for bos:    bos/bos.*.mstr.preqtbl
    #  E/xample for bosadt: bosadt/bosadt.*.mstr.preqtbl
    #  Example for X11rte: X11rte/X11rte.*.mstr.preqtbl
    preqtbl_files=$(ls ${UPDATE_DIR}/${2}/${2}.*.mstr.preqtbl)
    if [[ -n ${preqtbl_files} ]]
    then
      # First extract all lines in the paragraph that contains the name of
      # the ptf in question (i.e. all lines starting with the ptf name
      # and ending with the first blank line).  There should only be one
      # occurrence of the ptf number in the "mstr.preqtbl" files.
      # Extract only the lines containing file names
      # This can't be done by just getting lines that begin with
      # a slash (all files names are full path names) because
      # grep returns the full path name of the searched file
      # as the first line of its output.
      # You have to both reject the first line and reject any
      # lines that begin with U or I (ptf number or apar number)
      build_file_list=$(grep -i -p ${1} \
		  ${preqtbl_files} |
		  awk 'BEGIN {files = ""; inputname_found = 0}
		       /^[^UI]/ {if (inputname_found==1)
				  if (length(files) > 0)
				   files = files $1 "|"
				  else
				   files = $1 "|"
				}
		       /:$/ {if  (inputname_found==0)
			      inputname_found = 1
			    }
		       END {print files}
		      ')
    else
      build_file_list=""
    fi
  else
    build_file_list="NONE|"
  fi

  if [[ -n ${build_file_list} ]]
  then
    return 0
  else
    return 1
  fi
}

function check_cmvc_note
{
  # first parameter must be the PTF number
  # second parameter must be the apar number (for consistency)
  # third parameter must be the defect number
  # fourth parameter must be the list of files, separated by vertical bars
  # the fifth parameter must be the lpp name

  if ((tracing_on==0))
  then
    set -x
  fi

  # if test mode is off
  if ((test_mode==1))
  then
    # search the list of files shipped in the defect's notes
    cmvc_file_list=$(Report -view noteview -wh "defectname='${3}'" -raw \
		      -become ${CMVC_ID} 2> ${errmsg_file} |
		    cut -d'|' -f9 |
		    awk 'BEGIN {files = ""; key_found = 0}
			 # select all lines from the "files shipped" tag until
			 # a null line is found
			 /^FILES SHIPPED BY '${5}' PTF\: '${1}'/,/^$/\
			 {if ($0 ~ /FILES SHIPPED BY '${5}'/) {
			    files = ""
			    if (key_found == 0) key_found = 1
			    }
			  if (($0 !~ /FILES SHIPPED BY '${5}'/) && ($0 != ""))
			    if (substr($0,1,1) == "/") {
			      if (length(files) > 0)
				files = files $1 "|"
			      else
				files = $1 "|"
			    }
			 }
			 END {
			   if (key_found==1)
			     if (length(files) > 0)
			       print files
			     else
			       print "NONE|"
			   else
			     print "NO ENTRY"
			 }
			')
  else
    # search the list of files shipped in the defect's notes
    cmvc_file_list=$(Report -view noteview -wh "defectname='${3}'" -raw \
		      -become ${CMVC_ID} 2> ${errmsg_file} |
		    cut -d'|' -f9 |
		    awk 'BEGIN {files = ""; key_found = 0}
			 /^FILES SHIPPED BY '${5}' TEST PTF\: '${1}'/,/^$/\
			 {if ($0 ~ /FILES SHIPPED BY '${5}' TEST/) {
			    files = ""
			    if (key_found == 0) key_found = 1
			    }
			  if (($0 !~ /FILES SHIPPED BY '${5}' TEST/) && ($0 != ""))
			    if (substr($0,1,1) == "/") {
			      if (length(files) > 0)
				files = files $1 "|"
			      else
				files = $1 "|"
			    }
			 }
			 END {
			   if (key_found==1)
			     if (length(files) > 0)
			       print files
			     else
			       print "NONE|"
			   else
			     print "NO ENTRY"
			 }
			')
  fi

  if [[ -n ${cmvc_file_list} && ! -s ${errmsg_file} ]]
  then
    if [[ "${cmvc_file_list}" != "NO ENTRY" ]]
    then
      if [[ "${cmvc_file_list}" = "${4}" ]]
      then
	return 0
      else
	return 3
      fi
    else
      return 2
    fi
  else
    cmvc_file_list="NONE|"
    return 1
  fi
}

function add_cmvc_note
{
  # first parameter must be the PTF number
  # second parameter must be the apar number (just to maintain consistency)
  # third parameter must be the defect/feature prefix
  # fourth parameter must be the defect number
  # fifth parameter must be the list of files, separated by vertical bars
  # the sixth parameter must be the lpp name

  if ((tracing_on==0))
  then
    set -x
  fi

  files=${5}
  files_note=""
  while [[ ${#files} > 0 ]]
  do
    next_file=${files%%[|]*}
    files_note="${files_note}${next_file}
"
    files=${files#*[|]}
  done

  # if test mode is off
  if ((test_mode==1))
  then
    total_note="FILES SHIPPED BY ${6} PTF: ${1}
"
    total_note="${total_note}${files_note}

"
    display_msg "Adding following note to defect ${4}"
    display_msg "${total_note}"
    if [[ ${3} = 'd' ]]
    then
      Feature -note ${4} -become ${CMVC_ID} -remarks "${total_note}"
    else
      Defect -note ${4} -become ${CMVC_ID} -remarks "${total_note}"
    fi
  else
    total_note="FILES SHIPPED BY ${6} TEST PTF: ${1}
"
    total_note="${total_note}${files_note}

"
    display_msg "Adding following note to defect ${4}"
    display_msg "${total_note}"
    if [[ ${3} = 'd' ]]
    then
      Feature -note ${4} -become ${CMVC_ID} -remarks "${total_note}"
    else
      Defect -note ${4} -become ${CMVC_ID} -remarks "${total_note}"
    fi
  fi
}

function in_production_build
{
 # the first parameter must be the defect number
 # the second parameter must be the base release name (i.e. 320)

 typeset -i in_prod

 if [[ ${tracing_on} = 0 ]]
 then
   set -x
 fi

 ((in_prod=1))

 members=$(Report -vi levelmemberview -where "defectName='$1' and \
	   releasename like '%${2}'" -raw |
	   awk -F"|" '{printf "%s %s\t%s\t%s\n",$8,$3,$2,$1}')
 rc=${?}
 if [[ ${rc} = 0 ]]
 then
  echo "${members}" |
  while read prefix defect release level
  do
   level_type=$(Report -vi levelview -wh "name='${level}' and \
		releasename='${release}'" -raw | awk -F"|" '{print $3}')
   rc=${?}
   if [[ ${rc} = 0 ]]
   then
    if [[ ${level_type} = production ]]
    then
     ((in_prod=0))
     break
    fi
   fi
  done
 fi
 return ${in_prod}
}

function update_cmvc_note
{
  # first parameter must be the PTF number
  # second parameter must be the list of files changed, separated by vertical bars
  # third parameter must be the lpp name
  # remaining parameters must the apars that the ptf fixes except for test
  # mode where the remaining parameters are apar/defect pairs

  if [[ ${tracing_on} = 0 ]]
  then
    set -x
  fi

  # if not test mode
  if ((test_mode==1))
  then
    # send list of apars that this ptf fixes
    typeset -i i=4
    while ((i<=${#}))
    do
      eval apar_num='$'{${i}}
      # if debug mode is not on
      if ((debug_mode==1))
      then
	# if the alternate mode is not on (i.e. the apar list is truly apars)
	if ((alternate_mode==1))
	then
	  apar_digits=${apar_num#[iI][xX]*}
          if [[ ${MULTI_APAR_DEFECT} = "yes" ]]
          then
	    defects=$(qryDefectByAPAR -a IX${apar_digits} -v ${aix_version} \
		    -r 2>${errmsg_file} | awk -F"|" '{print $1}')
          else
            if [[ ${CMVC_VERSION} = "2" ]]
            then
               defects=$(Report -view defectview -where "reference='IX${apar_digits}'\
                     or reference='ix${apar_digits}'" -raw -become ${CMVC_ID} 2> ${errmsg_file} |
                   dv.filter | awk -F"|" '{printf "%s\n",$2}')
            else
               defects=$(Report -view defectview -where "reference='IX${apar_digits}'\
                     or reference='ix${apar_digits}'" -raw -become ${CMVC_ID} 2> ${errmsg_file} |
                   awk -F"|" '{printf "%s\n",$2}')
            fi
            if [[ ${CMVC_VERSION} = "2" ]]
            then
	       features=$(Report -view featureview -where "reference='IX${apar_digits}'\
		     or reference='ix${apar_digits}'" -raw -become ${CMVC_ID} \
		     2> ${errmsg_file} | fv.filter | awk -F"|" '{printf "%s\n",$2}')
            else
	       features=$(Report -view featureview -where "reference='IX${apar_digits}'\
		     or reference='ix${apar_digits}'" -raw -become ${CMVC_ID} \
		     2> ${errmsg_file} | awk -F"|" '{printf "%s\n",$2}')
            fi
	    defects="${defects} ${features}" 
          fi
	  # if any defects were found
	  if [[ -n ${defects} && ! -s ${errmsg_file} ]]
	  then
	    # check each defect and add note if appropriate
	    for defect in "${defects}"
	    do
	      # if this defect has been in any production builds
	      if in_production_build ${defect} ${aix_version}0
	      then
		  print -u6 "${1}\t${apar_num}\t${defect}\t${3}"
	      fi
	    done
	  else
	      # write to bad defect file with defect number of 00000
	      defect_num="000000"
	      print -u8 "${1}\t${apar_num}\t${defect_num}\tDEFECT NOT FOUND"
	      ((i+=1))
	      continue
	  fi
	else
	  # in alternate mode, the apar list is actually a list of defects
	  defect_num=${apar_num}
	  # look up the apar number for this defect
          if [[ ${MULTI_APAR_DEFECT} = "yes" ]]
          then
	    apar=$(qryAPARByDefect -d ${defect_num} -v ${aix_version} -r \
			2>${errmsg_file} | awk -F"|" '{print $2}')
          else
            if [[ ${CMVC_VERSION} = "2" ]]
            then
	       apar=$(Report -view defectview -where "name='${defect_num}'" \
		 -raw -become ${CMVC_ID} 2> ${errmsg_file} \
                 | dv.filter | awk -F"|" '{printf "%s",$26}')
            else
	       apar=$(Report -view defectview -where "name='${defect_num}'" \
		 -raw -become ${CMVC_ID} 2> ${errmsg_file} \
                 | awk -F"|" '{printf "%s",$26}')
            fi
	    if [[ -z ${apar} ]]
	    then
               if [[ ${CMVC_VERSION} = "2" ]]
               then
	    	apar=$(Report -view featureview -w "name='${defect_num}'" \
		      -raw -become ${CMVC_ID} 2> ${errmsg_file} \
                      | fv.filter | awk -F"|" '{printf "%s",$18}')
               else
	    	apar=$(Report -view featureview -w "name='${defect_num}'" \
		      -raw -become ${CMVC_ID} 2> ${errmsg_file} \
                      | awk -F"|" '{printf "%s",$18}')
               fi
	    fi
          fi
	  # if an apar was found
	  if [[ -n ${apar} && ! -s ${errmsg_file} ]]
	  then
	    apar_num=${apar}
	    print -u6 "${1}\t${apar_num}\t${defect_num}\t${3}"
	  else
	    # write to bad defect file with defect number of 00000
	    apar_num="IX00000"
	    print -u8 "${1}\t${defect_num}\t${3}\tREFERENCE NOT FOUND"
	    ((i+=1))
	    continue
	  fi
	fi
      else
	# parrot back what was read as updated successfully with defect of 00000
	defect_num="000000"
	print -u6 "${1}\t${apar_num}\t${defect_num}\t${3}"
      fi
      ((i+=1))
    done
  else
    # send list of apars that this ptf fixes
    typeset -i i=4
    typeset -i j
    while ((i<=${#}))
    do
      eval apar_num='$'{${i}}
      j=i+1
      eval defect_num='$'{${j}}
      # if debug mode is not on
      if ((debug_mode==1))
      then
	if [[ -n ${defect_num} ]]
	then
           if [[ ${CMVC_VERSION} = "2" ]]
           then 
	      prefix=$(Report -vi defectview -wh "name='${defect_num}'" -raw -become ${CMVC_ID} |
		   dv.filter | awk -F"|" '{print $1}')
           else
    	      prefix=$(Report -vi defectview -wh "name='${defect_num}'" -raw -become ${CMVC_ID} |
		   awk -F"|" '{print $1}')
           fi
	  if [[ ${prefix} = 'a' || ${prefix} = 'p' ]]
	  then
	    :
	  else
            if [[ ${CMVC_VERSION} = "2" ]]
            then
	       prefix=$(Report -vi featureview -wh "name='${defect_num}'" -raw -become ${CMVC_ID} |
		     fv.filter | awk -F"|" '{print $1}')
            else
	       prefix=$(Report -vi featureview -wh "name='${defect_num}'" -raw -become ${CMVC_ID} |
		     awk -F"|" '{print $1}')
            fi
	    if [[ ${prefix} = 'd' ]]
	    then
	      :
	    else
	      print -u8 "${1}\t${apar_num}\t${3}|INVALID DEFECT NUMBER"
	      ((i+=2))
	      continue
	    fi
	  fi
	else
	  # write to bad defect file with defect number of 00000
	  defect_num="000000"
	  print -u8 "${1}\t${apar_num}\t${defect_num}\\t${3}tDEFECT NUMBER NOT SPECIFIED"
	  ((i+=2))
	  continue
	fi
       ## check the corresponding CMVC defect
       #if check_cmvc_note ${1} ${apar_num} ${defect_num} ${2} ${3}
       #then
       #  print -u6 "${1}\t${apar_num}\t${defect_num}\t${3}|CMVC ALREADY UPDATED"
       #else
       #  if add_cmvc_note ${1} ${apar_num} ${prefix} ${defect_num} ${2} ${3}
       #  then
	    print -u6 "${1}\t${apar_num}\t${defect_num}\t${3}"
       #  else
       #    print -u7 "${1}\t${apar_num}\t${defect_num}\t${3}|CMVC NOT UPDATED"
       #  fi
       #fi
      else
	# parrot back what was read as updated successfully
	print -u6 "${1}\t${apar_num}\t${defect_num}"
      fi
      ((i+=2))
    done
  fi

}

function check_retain_note
{
  # first parameter must be the PTF number
  # second parameter must be the list of files, separated by vertical bars
  # third parameter must be the lpp name

  if ((tracing_on==0))
  then
    set -x
  fi

  typeset -i last_page
  typeset -i current_page
  typeset -i search_line
  typeset -i last_search_line=23
  typeset -i first_search_line=14
  typeset -i comment_tag_found=1

  ((retain_files_found=1))

  # if the debug mode has not be specified
  if ((debug_mode==1))
  then
    # display the ptf record
    hsend -t${RETAIN_WAIT} -home
    hsend -t${RETAIN_WAIT} "n;-/r ${1}"
    if hexpect -t${RETAIN_WAIT} "@1,2:PTF= ${1}"
    then
      log_it "First summary page for ${1}"
      # display first page of cover letter
      hsend -t${RETAIN_WAIT} "l"
      # confirm that cover letter is being displayed
      page_name=$(hget -t0 16,69:16,77)
      if [[ ${page_name} = "COVER LTR" ]]
      then
	log_it "Cover Letter page for ${1}"
	# search for first line of comment field
	last_page=$(hget -t0 17,79:17,80)
	display_msg "Last Page of Cover Letter = ${last_page}"
	current_page=$(hget -t0 17,73:17,74)
	# search for the first line of the comment field
	while ((current_page<=last_page))
	do
	  display_msg "Current Page of Cover Letter = ${current_page}"
	  ((search_line=last_search_line))
	  while ((search_line>=first_search_line))
	  do
	    display_msg "Current Line on Cover Letter = ${search_line}"
	    line_contents=$(hget -t0 ${search_line},2:${search_line},65)
	    if [[ ${line_contents} = COMMENTS:* ]]
	    then
	      ((comment_tag_found=0))
	      # get the first fixed portion of the comment, up to the size
	      first_comment=$(hget -t0 ${search_line},12:${search_line},65)
	      break 2
	    fi
	    ((search_line-=1))
	  done
	  # display next page of cover letter
	  hsend -t${RETAIN_WAIT} -enter
	  current_page=$(hget -t0 17,73:17,74)
	done
	if ((comment_tag_found==0))
	then
	  # search for the first line of the file list
	  while ((current_page<=last_page))
	  do
	    ((search_line=last_search_line))
	    while ((search_line>=first_search_line))
	    do
	      line_contents=$(hget -t0 ${search_line},2:${search_line},65)
	      if [[ ${line_contents} = *"Files shipped by this ptf:"* ]]
	      then
		((retain_files_found=0))
		break 2
	      fi
	      ((search_line-=1))
	    done
	    # if this is the last page of the cover letter
	    if ((current_page==last_page))
	    then
	      break 1
	    else
	      # display next page of cover letter
	      hsend -t${RETAIN_WAIT} -enter
	      current_page=$(hget -t0 17,73:17,74)
	    fi
	  done
	  if ((retain_files_found==0))
	  then
	    # increment the current coverletter line by one to get to the
	    # first file
	    ((search_line+=1))
	    # reset the file list to null
	    retain_file_list=""
	    # extract all the files listed for this ptf
	    while ((current_page<=last_page))
	    do
	      while ((search_line<=last_search_line))
	      do
		file_name=$(hget -t0 ${search_line},2:${search_line},65)
		# all files names must be the full path name, therefore
		# they must begin with a slash
		if [[ ${file_name} = "/"* ]]
		then
		  retain_file_list="${retain_file_list}${file_name}|"
		else
		  break 2
		fi
		((search_line+=1))
	      done
	      # display next page of cover letter
	      hsend -t${RETAIN_WAIT} -enter
	      ((search_line=first_search_line))
	      current_page=$(hget -t0 17,73:17,74)
	    done
	    if [[ ${#retain_file_list} > 0 ]]
	    then
	      if [[ "${retain_file_list}" = "${2}" ]]
	      then
		return 0
	      else
		return 12
	      fi
	    else
	      retain_file_list="NONE|"
	      return 0
	    fi
	  else
	    log_it "first files line not found for ${1}"
	    return 11
	  fi
	else
	  log_it "first comment line not found for ${1}"
	  return 5
	fi
      else
	log_it "cover letter not found for ${1}"
	if hexpect -t${RETAIN_WAIT} "@11,5:REQUESTED PAGE NOT AVAILABLE"
	then
	  return 3
	else
	  return 4
	fi
      fi
    else
      if hexpect -t${RETAIN_WAIT} "@21,2:NO RECORD FOUND"
      then
	log_it "${1} was not found"
	return 2
      else
	log_it "Unknown error in search for ${1}"
	return 1
      fi
    fi
  else
    return 0
  fi
}

function add_retain_note
{
  # first parameter must be the PTF number
  # second parameter must be the list of files, separated by vertical bars
  # third parameter must be the lpp name

  if ((tracing_on==0))
  then
    set -x
  fi

  typeset -i edit_line
  typeset -i last_edit_line=22
  typeset -i first_edit_line=12
  typeset -i max_lines
  typeset -i current_last_line
  typeset -i last_line_on_page
  typeset -i first_comment_line
  typeset -i comment_tag_found=1

  # if the debug mode has not be specified
  if ((debug_mode==1))
  then
    # display the ptf record
    hsend -t${RETAIN_WAIT} -home
    hsend -t${RETAIN_WAIT} "n;-/r ${1}"
    if hexpect -t${RETAIN_WAIT} "@1,2:PTF= ${1}"
    then
      log_it "Summary page for ${1}"
      # get first four characters of the external status
      external_status=$(hget -t0 1,45:1,48)
      if [[ ${external_status} = "OPEN" || ${external_status} = "CAND" ]]
      then
	log_it "${1} external status is open or cand"
	# confirm that EDIT is a valid option for this ptf
	if command_expect "EDIT"
	then
	  # display first page of cover letter
	  hsend -t${RETAIN_WAIT} "l"
	  # confirm that cover letter is being displayed
	  page_name=$(hget -t0 16,69:16,77)
	  if [[ ${page_name} = "COVER LTR" ]]
	  then
	    log_it "Cover Letter page for ${1}"
	    # issue the EDIT command
	    hsend -t${RETAIN_WAIT} "edit"
	    # confirm that EDIT command activated
	    if hexpect -t${RETAIN_WAIT} "@1,6:COVER LETTER PAGE 1   FOR PTF  ${1}  SELECTED FOR EDITING"
	    then
	      # confirm that multi-line editing is an option
	      if hexpect -t${RETAIN_WAIT} "@5,2:TO EDIT TEXT FIELDS USING EXPANDED EDIT OPTIONS"
	      then
		# issue the PF3 key to invoke multi-line editing
		hsend -t${RETAIN_WAIT} -pf3
		# confirm multi-line edit mode was activated
		if hexpect -t${RETAIN_WAIT} "@1,2:MULTIPLE LINE EDIT SELECTED"
		then
		  # search for comment field tag
		  log_it "multi_line editing mode for cover letter"
		  # save the maximum number of lines that the cover letter
		  # can have
		  max_lines=$(hget -t0 8,75:8,77)
		  # save the current number of cover letter lines
		  current_last_line=$(hget -t0 9,75:9,77)
		  # find the number of the last line currently displayed
		  ((edit_line=last_edit_line))
		  while ((edit_line>=first_edit_line))
		  do
		    last_line_on_page=$(hget -t0 ${edit_line},3:${edit_line},5)
		    if ((last_line_on_page>0))
		    then
		      break
		    fi
		    ((edit_line-=1))
		  done
		  # end of loop to find last line currently displayed

		  # find the line number of the first comment line
		  while ((last_line_on_page<=current_last_line))
		  do
		    # search for the comment tag line starting with the last
		    # line currently displayed
		    ((edit_line=last_edit_line))
		    while ((edit_line>=first_edit_line))
		    do
		      line_contents=$(hget -t0 ${edit_line},17:${edit_line},80)
		      if [[ ${line_contents} = COMMENTS:* ]]
		      then
			((comment_tag_found=0))
			# get the variable portion of the comment
			first_comment=$(hget -t0 ${edit_line},27:${edit_line},80)
			first_comment_line=$(hget -t0 ${edit_line},3:${edit_line},5)
			break 2
		      fi
		      ((edit_line-=1))
		    done
		    # end of loop to search for the comment tag

		    # scroll to next set of cover letter lines
		    hsend -t${RETAIN_WAIT} -enter

		    # find the last line number displayed after scrolling
		    ((edit_line=last_edit_line))
		    while ((edit_line>=first_edit_line))
		    do
		      last_line_on_page=$(hget -t0 ${edit_line},3:${edit_line},5)
		      if ((last_line_on_page>0))
		      then
			break
		      fi
		      ((edit_line-=1))
		    done
		    # end of loop to find last line currently displayed
		  done
		  # end of loop to find find first line of comment

		  # if the first line of the comment was found
		  if ((comment_tag_found==0))
		  then
		    # scroll the lines to bring the comment tag line to the top
		    hsend -t${RETAIN_WAIT} ${first_comment_line}
		    # check that first comment line is first one displayed
		    if hexpect -t${RETAIN_WAIT} "@12,17:COMMENT"
		    then
		      # invoke ADD LINES mode
		      hsend -t${RETAIN_WAIT} -pf9
		      # indicate the new lines are to be added after the first
		      # comment line
		      hsend -t${RETAIN_WAIT} ${first_comment_line}
		      # write the files list tag line
		      hsend -t${RETAIN_WAIT} "Files shipped by this ptf:"
		      # write the list of files into the comment fields
		      files=${2}
		      while [[ ${#files} > 0 ]]
		      do
			next_file=${files%%[|]*}
			hsend  -t${RETAIN_WAIT} ${next_file}
			files=${files#*[|]}
		      done
		      # end add subfunction
		      hsend -t${RETAIN_WAIT} -pf11
		      # verify added data
		      hsend -t${RETAIN_WAIT} -pf11

		      # verify the data and terminate the edit session
		      hsend -t${RETAIN_WAIT} -pa2
		      # check for proper completion of edit
		      if hexpect -t${RETAIN_WAIT} "@11,2:EDIT COMPLETED, PAGES QUEUED FOR UPDATE"
		      then
			log_it "Files note added to PTF ${1}"
			return 0
		      else
			log_it "edit failed to complete successfully"
			return 13
		      fi
		    else
		      hget -t0 12,17:12:24
		      log_it "comment tag line not first one displayed"
		      return 11
		    fi
		  else
		    log_it "first comment line not found for ${1}"
		    hsend -t${RETAIN_WAIT} -pf1
		    return 9
		  fi
		else
		  log_it "MULTI_LINE EDIT command did not activate"
		  hsend -t${RETAIN_WAIT} -pf1
		  return 8
		fi
	      else
		log_it "MULTI-LINE EDIT is not an option"
		return 7
	      fi
	    else
	      log_it "EDIT command did not activate"
	      hsend -t${RETAIN_WAIT} -pf1
	      return 6
	    fi
	  else
	    log_it "cover letter not found for ${1}"
	    return 5
	  fi
	else
	  log_it "EDIT is not a valid option"
	  return 4
	fi
      else
	log_it "${1} external status is not open"
	return 3
      fi
    else
      if hexpect -t${RETAIN_WAIT} "@21,2:NO RECORD FOUND"
      then
	log_it "${1} was not found"
	return 2
      else
	log_it "Unknown error in search for ${1}"
	return 1
      fi
    fi
  else
    return 0
  fi
}

function delete_retain_note
{
  # first parameter must be the PTF number
  # second parameter must be the list of files, separated by vertical bars
  # third parameter must be the lpp name

  if ((tracing_on==0))
  then
    set -x
  fi

  typeset -i edit_line
  typeset -i last_edit_line=22
  typeset -i first_edit_line=12
  typeset -i max_lines
  typeset -i current_last_line
  typeset -i last_line_on_page
  typeset -i first_file_line
  typeset -i last_file_line
  typeset -i file_tag_found=1

  # if the debug mode has not be specified
  if ((debug_mode==1))
  then
    # display the ptf record
    hsend -t${RETAIN_WAIT} -home
    hsend -t${RETAIN_WAIT} "n;-/r ${1}"
    if hexpect -t${RETAIN_WAIT} "@1,2:PTF= ${1}"
    then
      log_it "Summary page for ${1}"
      # get first four characters of the external status
      external_status=$(hget -t0 1,45:1,48)
      if [[ ${external_status} = "OPEN" || ${external_status} = "CAND" ]]
      then
	log_it "${1} external status is open or cand"
	# confirm that EDIT is a valid option for this ptf
	if command_expect "EDIT"
	then
	  # display first page of cover letter
	  hsend -t${RETAIN_WAIT} "l"
	  # confirm that cover letter is being displayed
	  page_name=$(hget -t0 16,69:16,77)
	  if [[ ${page_name} = "COVER LTR" ]]
	  then
	    log_it "Cover Letter page for ${1}"
	    # issue the EDIT command
	    hsend -t${RETAIN_WAIT} "edit"
	    # confirm that EDIT command activated
	    if hexpect -t${RETAIN_WAIT} "@1,6:COVER LETTER PAGE 1   FOR PTF  ${1}  SELECTED FOR EDITING"
	    then
	      # confirm that multi-line editing is an option
	      if hexpect -t${RETAIN_WAIT} "@5,2:TO EDIT TEXT FIELDS USING EXPANDED EDIT OPTIONS"
	      then
		# issue the PF3 key to invoke multi-line editing
		hsend -t${RETAIN_WAIT} -pf3
		# confirm multi-line edit mode was activated
		if hexpect -t${RETAIN_WAIT} "@1,2:MULTIPLE LINE EDIT SELECTED"
		then
		  # search for comment field tag
		  log_it "multi_line editing mode for cover letter"
		  # save the maximum number of lines that the cover letter
		  # can have
		  max_lines=$(hget -t0 8,75:8,77)
		  # save the current number of cover letter lines
		  current_last_line=$(hget -t0 9,75:9,77)
		  # find the number of the last line currently displayed
		  ((edit_line=last_edit_line))
		  while ((edit_line>=first_edit_line))
		  do
		    last_line_on_page=$(hget -t0 ${edit_line},3:${edit_line},5)
		    if ((last_line_on_page>0))
		    then
		      break
		    fi
		    ((edit_line-=1))
		  done
		  # end of loop to find last line currently displayed

		  # find the line number of the first file note line
		  while ((last_line_on_page<=current_last_line))
		  do
		    # search for the file tag line starting with the last
		    # line currently displayed
		    ((edit_line=last_edit_line))
		    while ((edit_line>=first_edit_line))
		    do
		      line_contents=$(hget -t0 ${edit_line},17:${edit_line},80)
		      if [[ ${line_contents} = *"Files shipped by this ptf:"* ]]
		      then
			((file_tag_found=0))
			# get the variable portion of the comment
			first_file_line=$(hget -t0 ${edit_line},3:${edit_line},5)
			break 2
		      fi
		      ((edit_line-=1))
		    done
		    # end of loop to search for the comment tag

		    # scroll to next set of cover letter lines
		    hsend -t${RETAIN_WAIT} -enter

		    # find the last line number displayed after scrolling
		    ((edit_line=last_edit_line))
		    while ((edit_line>=first_edit_line))
		    do
		      last_line_on_page=$(hget -t0 ${edit_line},3:${edit_line},5)
		      if ((last_line_on_page>0))
		      then
			break
		      fi
		      ((edit_line-=1))
		    done
		    # end of loop to find last line currently displayed
		  done
		  # end of loop to find find first line of comment

		  # if the first line of the comment was found
		  if ((file_tag_found==0))
		  then
		    # scroll the lines to bring the comment tag line to the top
		    hsend -t${RETAIN_WAIT} ${first_file_line}
		    # check that first file line is first one displayed
		    if hexpect -t${RETAIN_WAIT} "@12,17:Files shipped by this ptf:"
		    then
		      # invoke DELETE LINES mode
		      hsend -t${RETAIN_WAIT} -pf2
		      # confirm that delete mode was invoked
		      if hexpect -t${RETAIN_WAIT} "@5,2:TO INDICATE WHICH LINE OR LINES YOU WISH TO DELETE"
		      then
			# delete the first line of the file note sequence
			hsend -t${RETAIN_WAIT} "${first_file_line}"
			# get the first file line
			line_contents=$(hget -t0 ${first_edit_line},17:${first_edit_line},80)
			# delete the remaining lines (as long the first character
			# is slash)
			while [[ ${line_contents} = "/"* ]]
			do
			  # delete the first line of the file note sequence
			  hsend -t${RETAIN_WAIT} "${first_file_line}"
			  # get the next file line
			  line_contents=$(hget -t0 ${first_edit_line},17:${first_edit_line},80)
			done
			# end delete subfunction
			hsend -t${RETAIN_WAIT} -pf11
			# check for proper delete operation
			if hexpect -t${RETAIN_WAIT} "@7,2:PF11 -STORE PREV SUBFUNCTION CHANGES"
			then
			  # verify changes
			  hsend -t${RETAIN_WAIT} -pf11
			  # verify the data and terminate the edit session
			  hsend -t${RETAIN_WAIT} -pa2
			  # check for proper completion of edit
			  if hexpect -t${RETAIN_WAIT} "@11,2:EDIT COMPLETED, PAGES QUEUED FOR UPDATE"
			  then
			    log_it "Old file note deleted from PTF ${1}"
			    return 0
			  else
			    log_it "edit failed to complete successfully"
			    hsend -t${RETAIN_WAIT} -pf1
			    return 14
			  fi
			else
			  log_it "delete operation failed"
			  hsend -t${RETAIN_WAIT} -pf1
			  return 13
			fi
		      else
			log_it "delele mode failed to activate"
			hsend -t${RETAIN_WAIT} -pf1
			return 12
		      fi
		    else
		      hget -t0 12,17:12:24
		      log_it "comment tag line not first one displayed"
		      hsend -t${RETAIN_WAIT} -pf1
		      return 11
		    fi
		  else
		    log_it "file tag line not found for ${1}"
		    hsend -t${RETAIN_WAIT} -pf1
		    return 9
		  fi
		else
		  log_it "MULTI_LINE EDIT command did not activate"
		  hsend -t${RETAIN_WAIT} -pf1
		  return 8
		fi
	      else
		log_it "MULTI-LINE EDIT is not an option"
		return 7
	      fi
	    else
	      log_it "EDIT command did not activate"
	      hsend -t${RETAIN_WAIT} -pf1
	      return 6
	    fi
	  else
	    log_it "cover letter not found for ${1}"
	    return 5
	  fi
	else
	  log_it "EDIT is not a valid option"
	  return 4
	fi
      else
	log_it "${1} external status is not open"
	return 3
      fi
    else
      if hexpect -t${RETAIN_WAIT} "@21,2:NO RECORD FOUND"
      then
	log_it "${1} was not found"
	return 2
      else
	log_it "Unknown error in search for ${1}"
	return 1
      fi
    fi
  else
    return 0
  fi

}

function update_retain_note
{
  # first parameter must be the PTF number
  # second parameter must be the list of files, separated by vertical bars
  # third parameter must be the lpp name
  # fourth parameter must be the list of APAR numbers

  if [[ ${tracing_on} = 0 ]]
  then
    set -x
  fi

  check_retain_note ${1} ${2} ${3}
  rc=${?}
  if ((rc==0))
  then
    print -u4 "${1}\t${3}\t${4}|RETAIN ALREADY UPDATED"
  else
    # if the return code was greater than 10
    if ((rc>10))
    then
      # if a size comment line was found in retain
      if ((retain_files_found==0))
      then
	# edit the existing size comment line on the cover letter page
	if delete_retain_note ${1} ${2} ${3}
	then
	  # add the size comment line as the first comment line on the cover
	  # letter page
	  if add_retain_note ${1} ${2} ${3}
	  then
	    print -u4 "${1}\t${3}\t${4}"
	  else
	    print -u5 "${1}\t${3}\t${4}|RETAIN NOT UPDATED-ADD FAILED"
	  fi
	else
	  print -u5 "${1}\t${3}\t${4}|RETAIN NOT UPDATED-DELETE FAILED"
	fi
      else
	# add the size comment line as the first comment line on the cover
	# letter page
	if add_retain_note ${1} ${2} ${3}
	then
	  print -u4 "${1}\t${3}\t${4}"
	else
	  print -u5 "${1}\t${3}\t${4}|RETAIN NOT UPDATED-ADD FAILED"
	fi
      fi
    else
      print -u5 "${1}\t${3}\t${4}|RETAIN NOT UPDATED - PROBLEM WITH PTF"
    fi
  fi
}


function main
{
  if [[ ${tracing_on} = 0 ]]
  then
    set -x
  fi

  # default Retain system
  RETAIN_SYSTEM="${RETAIN_SYSTEM:=BDC}"

  # set font for the emulator
  HTN_OPTIONS="${HTN_OPTIONS:=-fzn Rom6.500}"

  # list hosts to which the connection to VTAM should be tried
  HOST_LIST="${HOST_LIST:=ausvm1 ausvm2 ausvm6 ausvmq}"

  # time to wait for expected data to appear
  RETAIN_WAIT="${RETAIN_WAIT:=5}"

  # subdirectory into which all output files are to be stored
  RETAIN_OUTPUT="${RETAIN_OUTPUT:=.}"

  # set VM prompt
  VM_PROMPT="${VM_PROMPT:=Ready}"

  # if the environment variable CMVC_ID is not set, assume that the current
  # login id is also the user's CMVC id
  CMVC_ID="${CMVC_ID:=$(logname)}"

  # if the environment variable CMVC_VERSION is not set, assume that the 
  # new CMVC release is used
  CMVC_VERSION="${CMVC_VERSION:=2}"

  # set the subdirectory name of the default directories
  PROD_DIR="/afs/austin/aix/ptf.images/320/prod"
  SHIP_DIR="/afs/austin/aix/ptf.images/320/ship"
  UPDATE_DIR="/afs/austin/aix/320/UPDATE"

  typeset -i RETAIN_RETRY_WAIT
  RETAIN_RETRY_WAIT="${RETAIN_RETRY_WAIT=5*RETAIN_WAIT}"

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

  typeset -i retain_files_found=1

  # if the standard input flag was not specified
  if [[ ${from_stdin} = 1 ]]
  then
    # the first command parameter is taken to be the name of the input file
    if [[ -n ${1} ]]
    then
      # open input file as descriptor 3
      exec 3< ${1}
      ((input_data=3))
      display_msg "Reading defect list from file ${1}"
    else
      display_msg \
	"No input file name was specified and no list of defects was supplied."
      display_msg "No action will be taken."
      return 2
    fi

    # create the base name of the output files and relate them to the base
    # name of the input file
    base_name=${1##*/}
    base_name=${base_name%.*}
    # set the base name of the output files to the RETAIN_OUTPUT variable
    base_name="${RETAIN_OUTPUT}/${base_name}"
  else
    display_msg "Reading defect list from standard input"
    # read input from stdin
    ((input_data=0))
    # create the base name of the output files and relate them to the name
    # of this program
    # if the base name was not specified with the -n flag
    if [[ -z ${base_name} ]]
    then
      base_name="${RETAIN_OUTPUT}/${action}"
    else
      base_name="${RETAIN_OUTPUT}/${base_name}"
    fi
  fi

  ptffiles_file="${base_name}.ptffiles"
  ptffiles_filedes="4"
  noptffiles_file="${base_name}.noptffiles"
  noptffiles_filedes="5"
  deffiles_file="${base_name}.deffiles"
  deffiles_filedes="6"
  nodeffiles_file="${base_name}.nodeffiles"
  nodeffiles_filedes="7"
  badptffiles_file="${base_name}.badptffiles"
  badptffiles_filedes="8"
  retain_log_base_name="${base_name}.${action}"
  retain_log="${retain_log_base_name}.retainlog"
  errmsg_file="${base_name}.${action}.errmsg"

  # if the cmvc only flag was NOT turned on
  if ((cmvc_only==1))
  then
    # if the save files mode was not turned off
    if [[ ${save_files} = 0 ]]
    then
      if [[ -f ${ptffiles_file} ]]
      then
	mv ${ptffiles_file} "${ptffiles_file}${time_suffix}"
      fi

      if [[ -f ${noptffiles_file} ]]
      then
	mv ${noptffiles_file} "${noptffiles_file}${time_suffix}"
      fi
    fi
    # open ptffiles output file as descriptor 4
    exec 4> ${ptffiles_file}
    # open noptffiles output file as descriptor 5
    exec 5> ${noptffiles_file}

    # if the debug mode has not been set and cmvc only has not been set
    if ((debug_mode==1)) && ((cmvc_only==1))
    then
      display_msg "Attempting to logon to RETAIN"
      LogonRetain ${logging_option} ${tracing_option} ${viewing_option} \
	${files_option} ${status_option} ${test_option} ${wait_option} \
	-n ${retain_log_base_name} ${path_option} ${retain_id_option}
      if [[ ${?} = 0 ]]
      then
	# continue
	:
      else
	log_it "Could not gain access to RETAIN"
	hsend -t0 undial
	exit 2
      fi
    fi
  fi

  # if the retain only flag was NOT turned on
  if ((retain_only==1))
  then
    # if the save files mode was not turned off
    if [[ ${save_files} = 0 ]]
    then
      if [[ -f ${deffiles_file} ]]
      then
	mv ${deffiles_file} "${deffiles_file}${time_suffix}"
      fi

      if [[ -f ${nodeffiles_file} ]]
      then
	mv ${nodeffiles_file} "${nodeffiles_file}${time_suffix}"
      fi
    fi
    # open deffiles output file as descriptor 6
    exec 6> ${deffiles_file}
    # open nodeffiles output file as descriptor 7
    exec 7> ${nodeffiles_file}
  fi

  # open bad output file as descriptor 8
  exec 8> ${badptffiles_file}

  while read -u${input_data} ptf_num apar_list
  do
    # remove any error messages from apar list due to previous run
    apar_list=${apar_list%\|*}
    # store apars numbers an array
    set -A apar_array ${apar_list}

    # find the name of the LPP/OPP that corresponds to this PTF
    # by searching the ptfapardef file on the PROD server
    if get_lpp_name ${ptf_num}
    then
      # see if the files shipped for this ptf has already been determined
      # (i.e. is stored in the ${PROD_DIR}/ptfs.files file)
      ptf_files=$(grep "^${ptf_num} " ${PROD_DIR}/ptfs.files |
		 awk '{printf "%s|",$2}')
      if [[ ${#ptf_files} > 0 ]]
      then
       build_file_list=${ptf_files}
      else
       # extract the list of shipped files from the master tables
       get_files_from_build ${ptf_num} ${lpp_name}
       rc=${?}
       # if did not get the PTF files
       if ((rc!=0))
       then
	 print -u${badptffiles_filedes} "${ptf_num}\t${apar_array[@]}|FAILED TO GET FILES FROM BUILD"
	 continue
       fi
      fi

      # if the retain only flag was NOT turned on
      if ((retain_only==1))
      then
	# update the cmvc size note, if it needs to be
	update_cmvc_note ${ptf_num} ${build_file_list} ${lpp_name} ${apar_array[@]}
      fi

      # if the cmvc only flag was NOT turned on
      if ((cmvc_only==1))
      then
	# update the retain size note, if it needs to be
	update_retain_note ${ptf_num} ${build_file_list} ${lpp_name} "${apar_array[@]}"
      fi
    fi
  done

  # if the debug mode has not been set
  if ((debug_mode==1))
  then
    # if the cmvc only flag was NOT turned on
    if ((cmvc_only==1))
    then
      # if the continue RETAIN login flag is NOT on
      if [[ ${continuous_on} = 1 ]]
      then
	# logoff RETAIN
	hsend -t${RETAIN_WAIT} -home
	hsend -t${RETAIN_WAIT} logoff
	log_it "Logged off RETAIN"
	hsend -t0 undial
      fi
    fi
  fi
  return 0
} ## END OF MAIN ##

function bailout
{
  if ((tracing_on==0))
  then
    set -x
  fi

  file_cleanup

  display_msg "${program_name} Program Interrupted"
  htn_running=$(ps -e | grep htn | awk '{print $1}')
  if [[ ${#htn_running} > 0 ]]
  then
    hsend -t0 -pf1
    hsend -t0 logoff
    hsend -t0 undial
    htn_running=$(ps -e | grep htn | awk '{print $1}')
    if [[ ${#htn_running} > 0 ]]
    then
      kill -1 ${htn_running}
    fi
  fi
  exit 3
}

##### START OF PROGRAM #####
trap "bailout" HUP INT QUIT TERM

# find base name of the program
program_name=${0##*/}

display_msg "START OF ${program_name} for ${@}"

# remove any program name suffix to use remaining portion to name output files
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
typeset -i alternate_mode=1
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

# check for command line options
while getopts :ab:cdhifln:op:rstuvw:xz next_option
do
  case ${next_option} in
  a) ((alternate_mode=0));;
  b) if [[ ${#OPTARG} > 0 ]]
     then
       retain_id_option="-b${OPTARG}"
     fi
     ;;
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
  V) echo ${OPTARG} | grep "^4" >/dev/null
     if [ $? -eq 0 ]; then
        aix_version="41"
     fi;;
  w) if [[ ${#OPTARG} > 0 ]]
     then
       wait_option="-w${OPTARG}"
       RETAIN_WAIT=${OPTARG}
     fi
     ;;
  x) ((tracing_on=0))
     tracing_option="-x";;
  z) ((cmvc_only=0));;
  \?) display_msg "${program_name}: unknown option ${OPTARG}"
      exit 2;;
  esac
done
# shift the commands line parameters to the left as many positions as
# there are flags
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

# no write permission by group and other for all created files
umask 022
# call main routine
main ${@}
rc=${?}
file_cleanup
exit ${rc}
##### END OF PROGRAM #####
