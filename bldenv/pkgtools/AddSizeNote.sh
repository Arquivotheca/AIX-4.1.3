#!/bin/ksh
# @(#)97        1.15  src/bldenv/pkgtools/AddSizeNote.sh, pkgtools, bos41B, 9504A 1/12/95 12:58:44
#
#   COMPONENT_NAME: PKGTOOLS
#
#   FUNCTIONS: none
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
SYNTAX: ${program_name} [-a] [-b] [-c] [-d] [+f] [+l] [-n] [-o] [-p] [-r] 
  echo "SYNTAX: ${program_name} [-a] [-b] [-c] [-d] [+f] [+l] [-n] [-o] [-p] "
  echo "                        [-r] [+s] [-F] [-t] [-u] [-v] [-w] [-x] [-z] "
  echo "                        [-V] -i | FileName | -h"
  echo "FUNCTION: This program updates a RETAIN PTF with the size (in diskettes)"
  echo "	that it and all its requisites will occupy. This size is taken "
  echo "	from the entry in the "ptfs.size" file on the PROD server.  If "
  echo "	the size has not already been determined, size is determined "
  echo "	from the PTF itself.  This requires access to the directory "
  echo "	where the PTF is stored along with the PROD and SHIP server"
  echo "	directories in afs."
  echo "	If your CMVC ID is not the same as the login ID you are running"
  echo "	under, then you must have the environment variable CMVC_ID set "
  echo "	to your CMVC ID.\n"
  echo "PARAMETERS: Either the name of a file is specified as a command line argument"
  echo "	or a list of ptfs followed by its corresponding apars is"
  echo "	specified via stdin. For both types of input, the format is: "
  echo "	one ptf number per line followed by all the apars it fixes "
  echo "	on the same line.\n"
  echo "FLAGS:"
  echo "  -a turns on alternate mode in which a defect list follows the PTF "
  echo "     number instead of an APAR list"
  echo "  -b specifies that the specified RETAIN ID should be used"
  echo "     (the default is to use the first one listed in $HOME/.netrc)"
  echo "  -c do not logoff of RETAIN when finished"
  echo "  -d (debug mode) indicates that actual"
  echo "  +f turns off the default action of saving existing copies of output files"
  echo "  -h displays this help text"
  echo "  -i indicates that the input data is to be read from standard input"
  echo "  +l indicates that all RETAIN screens are NOT to be logged"
  echo "  -n specifies the base name of all output files when input is from stdin"
  echo "  -o indicates that the list of defects for which the reference field is updated"
  echo "     is to be written to stdout instead of to the default output file"
  echo "  -p specifies the subdirectory into which all output files are written"
  echo "     (overrides RETAIN_OUTPUT)"
  echo "  -r indicates that only the RETAIN portion of the script is to be run"
  echo "  +s turns off the displaying of status messages to standard output"
  echo "  -t turns on test mode where the RETAIN test system is used"
  echo "  -F CMVC family name"
  echo "  -u searches unannounced (restricted) database for APARs and PTFs"
  echo "  -v indicates that the window used to access RETAIN is to be displayed"
  echo "  -w specifies the time to wait for a RETAIN command to finish"
  echo "     (overrides RETAIN_WAIT)"
  echo "  -x turns on tracing of the script; results are written to stderr"
  echo "  -z indicates that only the CMVC portion of the script is to be run"
  echo "  -V specifies the version of AIX for these PTFs"
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
    # close "ptfsize" output file
    exec 4<&-
    # close "noptfsize" output file
    exec 5<&-
  fi

  # if the retain only flag was NOT turned on
  if ((retain_only==1))
  then
    # close "defsize" output file
    exec 6<&-
    # close "nodefsize" output file
    exec 7<&-
  fi

  # close the input file
  exec 3<&-

  # close the bad defect file
  exec 8<&-

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

function get_size_from_build
{
  # first parameter must be the PTF number

  if [[ ${tracing_on} = 0 ]]
  then
    set -x
  fi

  # if test mode is NOT on and debug mode is NOT on
  if ((test_mode==1)) && ((debug_mode==1))
  then
    build_size_note=$(grep "^${1}" $prod_dir/ptfs.size 2> /dev/null)
    # if the size note was not found
    if [[ -z ${build_size_note} ]]
    then
     # (Removed call to PtfSize here, per request from Sagy Mintz 7/25/94)
     build_size_note="${1} AND ITS PRE/CO/IF REQUISITES OCCUPY >1 DISKETTE(S)"
    fi
  else
    build_size_note="${1} AND ITS PRE/CO/IF REQUISITES OCCUPY 1 DISKETTE(S)"
  fi

  if [[ -n ${build_size_note} ]]
  then
    build_size=${build_size_note##*OCCUPY[ ]}
    build_size=${build_size%%[ ]DISKETTE*}
    # if the number of diskettes needed is a single, double, or triple digit number
    if [[ ${build_size} = ?(">")[1-9] || ${build_size} = [1-9][0-9] || ${build_size} = [1-9][0-9][0-9] ]]
    then
      return 0
    else
      build_size="?"
      return 1
    fi
  else
    build_size="?"
    return 1
  fi
}

function check_cmvc_note
{
  # first parameter must be the PTF number
  # second parameter must be the apar number (just for consistency)
  # third parameter must be the defect number
  # fourth parameter must be the size in diskettes

  if [[ ${tracing_on} = 0 ]]
  then
    set -x
  fi

  # search the size note in the defect's notes
  cmvc_size_note=$(Report -view noteview -wh "defectname='${3}'" -raw \
		    -become ${CMVC_ID} 2> ${errmsg_file} |
		  cut -d'|' -f9 |
		  awk 'BEGIN {note_found = 0}
		       /^'${1}' AND ITS PRE/\
		       {if (note_found == 0) note_found = 1; size_note = $0}
		       END {if (note_found==1) print size_note;else print "NO ENTRY"}
		      ')
  # if a size note was found and there was not error message from CMVC
  if [[ -n ${cmvc_size_note} && ! -s ${errmsg_file} ]]
  then
    if [[ ${cmvc_size_note} != "NO ENTRY" ]]
    then
      cmvc_size=${cmvc_size_note##*OCCUPY[ ]}
      cmvc_size=${cmvc_size%%[ ]DISKETTE*}
      if [[ ${cmvc_size} = ${4} ]]
      then
	return 0
      else
	return 3
      fi
    else
      cmvc_size="?"
      return 2
    fi
  else
    cmvc_size="?"
    return 1
  fi
}

function add_cmvc_note
{
  # first parameter must be the PTF number
  # second parameter must be the apar number (just to maintain consistency)
  # third parameter must be the defect/feature prefix
  # fourth parameter must be the defect number
  # fifth parameter must be the size in diskettes

  if [[ ${tracing_on} = 0 ]]
  then
    set -x
  fi

  note="${1} AND ITS PRE/CO/IF REQUISITES OCCUPY ${5} DISKETTE(S)"

  display_msg "Adding following note to defect/feature ${4}"
  display_msg ${note}

  # if the prefix is 'd', add note to a feature
  if [[ ${3} = 'd' ]]
  then
    Feature -note ${4} -become ${CMVC_ID} -remarks "${note}"
  else
    Defect -note ${4} -become ${CMVC_ID} -remarks "${note}"
  fi
}

function update_cmvc_note
{
  # first parameter must be the PTF number
  # second parameter must be the size in diskettes
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
    typeset -i i=3
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
		   or reference='ix${apar_digits}'" -raw -become ${CMVC_ID} 2> ${errmsg_file} |
 		 fv.filter | awk -F"|" '{printf "%s\n",$2}')
            else
 	       features=$(Report -view featureview -where "reference='IX${apar_digits}'\
		   or reference='ix${apar_digits}'" -raw -become ${CMVC_ID} 2> ${errmsg_file} |
 		 awk -F"|" '{printf "%s\n",$2}')
            fi
	    defects="${defects} ${features}"
          fi

	  # if a defect was found
	  if [[ -n ${defects} && ! -s ${errmsg_file} ]]
	  then
	    for defect_num in "${defects}"
	    do
		 print -u6 "${1}\t${apar_num}\t${defect_num}\t${2}"
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
			2> ${errmsg_file} | awk -F"|" '{print $2}')
          else
            if [[ ${CMVC_VERSION} = "2" ]]
            then
               apar=$(Report -view defectview -where "name='${defect_num}'" -raw \
                 -become ${CMVC_ID} 2> ${errmsg_file} |
                 dv.filter | awk -F"|" '{printf "%s",$26}')
            else
               apar=$(Report -view defectview -where "name='${defect_num}'" -raw \
                 -become ${CMVC_ID} 2> ${errmsg_file} |
                 awk -F"|" '{printf "%s",$26}')
            fi
	    if [[ -z ${apar} ]]
	    then
               if [[ ${CMVC_VERSION} = "2" ]]
               then
                  apar=$(Report -vi featureview -w "name='${defect_num}'" -raw \
                    -become ${CMVC_ID} 2> ${errmsg_file} |
                    fv.filter | awk -F"|" '{printf "%s",$18}')
               else
                  apar=$(Report -vi featureview -w "name='${defect_num}'" -raw \
                    -become ${CMVC_ID} 2> ${errmsg_file} |
                    awk -F"|" '{printf "%s",$18}')
               fi
	    fi
          fi

	  # if an apar was found
	  if [[ -n ${apar} && ! -s ${errmsg_file} ]]
	  then
	      apar_num=${apar}
 	      print -u6 "${1}\t${apar_num}\t${defect_num}\t${2}"
	  else  # Could not find an APAR for this defect.
	      # write to bad defect file with defect number of 00000
	      apar_num="IX00000"
	      print -u8 "${1}\t${defect_num}\tREFERENCE NOT FOUND"
	      ((i+=1))
	      continue
	  fi
	fi
      else
	# parrot back what was read as updated successfully with defect of 00000
	defect_num="000000"
	print -u6 "${1}\t${apar_num}\t${defect_num}\t${2}"
      fi
      ((i+=1))
    done
  else
    # send list of apars that this ptf fixes
    typeset -i i=3
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
	      print -u8 "${1}\t${apar_num}\tINVALID DEFECT NUMBER"
	      ((i+=2))
	      continue
	    fi
	  fi
	else
	  # write to bad defect file with defect number of 00000
	  defect_num="000000"
	  print -u8 "${1}\t${apar_num}\t${defect_num}\tDEFECT NUMBER NOT SPECIFIED"
	  ((i+=2))
	  continue
	fi
       ## check the corresponding CMVC defect
       #if check_cmvc_note ${1} ${apar_num} ${defect_num} ${2}
       #then
       #  print -u6 "${1}\t${apar_num}\t${defect_num}\t${2}\tCMVC ALREADY UPDATED"
       #else
       #  if add_cmvc_note ${1} ${apar_num} ${prefix} ${defect_num} ${2}
       #  then
	    print -u6 "${1}\t${apar_num}\t${defect_num}\t${2}"
       #  else
       #    print -u7 "${1}\t${apar_num}\t${defect_num}\t${2}\tCMVC NOT UPDATED"
       #  fi
       #fi
      else
	# parrot back what was read as updated successfully
	print -u6 "${1}\t${apar_num}\t${defect_num}\t${2}"
      fi
      ((i+=2))
    done
  fi

}

function check_retain_note
{
  # first parameter must be the PTF number
  # second parameter must be the size in diskettes
  # third parameter is the superseding PTF number, if there is one

  if [[ ${tracing_on} = 0 ]]
  then
    set -x
  fi

  typeset -i coverletter_line
  typeset -i last_coverletter_line=23
  typeset -i first_coverletter_line=14
  typeset -i comment_tag_found=1

  ((retain_size_found=1))

  # if the debug mode has not be specified
  if ((debug_mode==1))
  then
    # display the ptf record
    hsend -t${RETAIN_WAIT} -home
    hsend -t${RETAIN_WAIT} "n;-/r ${1}"
    if hexpect -t${RETAIN_WAIT} "@1,2:PTF= ${1}"
    then
      log_it "Summary page for ${1}"
      # display first page of cover letter
      hsend -t${RETAIN_WAIT} "l"
      # confirm that cover letter is being displayed
      page_name=$(hget -t0 16,69:16,77)
      if [[ ${page_name} = "COVER LTR" ]]
      then
	log_it "Cover Letter page for ${1}"
	# search for first line of comment field
	last_page=$(hget -t0 17,79:17,80)
	current_page=$(hget -t0 17,73:17,74)
	while [[ (${current_page} < ${last_page}) || (${current_page} = ${last_page}) ]]
	do
	  ((coverletter_line=last_coverletter_line))
	  while ((coverletter_line>=first_coverletter_line))
	  do
	    field_name=$(hget -t0 ${coverletter_line},2:${coverletter_line},10)
	    if [[ ${field_name} = "COMMENTS:" ]]
	    then
	      ((comment_tag_found=0))
	      # get the first fixed portion of the comment, up to the size
	      first_comment=$(hget -t0 ${coverletter_line},12:${coverletter_line},50)
	      break 2
	    fi
	    ((coverletter_line-=1))
	  done
	  # display next page of cover letter
	  hsend -t${RETAIN_WAIT} -enter
	  current_page=$(hget -t0 17,73:17,74)
	done
	if ((comment_tag_found==0))
	then
	  if [[ ${first_comment} = "This ptf and all its requisites occupy" ]]
	  then
	    # indicate that the size comment line has been found
	    ((retain_size_found=0))
	    log_it "first comment line is size line for ${1}"
	    # get entire comment line
	    first_comment=$(hget -t0 ${coverletter_line},12:${coverletter_line},66)
	    # extract the size field from the comment line
	    retain_size=${first_comment##*occupy[ ]}
	    retain_size=${retain_size%%[ ]diskette*}
	    # extract the size field from the cmvc size note
	    if [[ ${retain_size} = ${2} ]]
	    then
	      return 0
	    else
	      return 6
	    fi
	  else
	    log_it "first comment line is not size line for ${1}"
	    return 5
	  fi
	else
	  log_it "first comment line not found for ${1}"
	  return 4
	fi
      else
	log_it "cover letter not found for ${1}"
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

function add_size_comment
{
  # first parameter must be the PTF number
  # second parameter must be the size in diskettes
  # third parameter is the superseding PTF number, if there is one
  if [[ ${tracing_on} = 0 ]]
  then
    set -x
  fi

  typeset -i coverletter_line
  typeset -i last_coverletter_line=22
  typeset -i first_coverletter_line=11
  typeset -i comment_tag_found=1

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
	  ((coverletter_line=last_coverletter_line))
	  while ((coverletter_line>=first_coverletter_line))
	  do
	    last_line_on_page=$(hget -t0 ${coverletter_line},3:${coverletter_line},5)
	    if [[ ${#last_line_on_page} > 0 ]]
	    then
	      break
	    fi
	    ((coverletter_line-=1))
	  done
	  # end of loop to find last line currently displayed

	  # find the line number of the first comment line
	  while [[ (${last_line_on_page} < ${current_last_line}) || (${last_line_on_page} = ${current_last_line}) ]]
	  do
	    # search for the comment tag line starting with the last
	    # line currently displayed
	    ((coverletter_line=last_coverletter_line))
	    while ((coverletter_line>=first_coverletter_line))
	    do
	      field_name=$(hget -t0 ${coverletter_line},17:${coverletter_line},25)
	      if [[ ${field_name} = "COMMENTS:" ]]
	      then
		((comment_tag_found=0))
		# get the variable portion of the comment
		first_comment=$(hget -t0 ${coverletter_line},27:${coverletter_line},80)
		first_comment_line=$(hget -t0 ${coverletter_line},3:${coverletter_line},5)
		break 2
	      fi
	      ((coverletter_line-=1))
	    done
	    # end of loop to search for the comment tag

	    # scroll to next set of cover letter lines
	    hsend -t${RETAIN_WAIT} -enter

	    # find the last line number displayed after scrolling
	    ((coverletter_line=last_coverletter_line))
	    while ((coverletter_line>=first_coverletter_line))
	    do
	      last_line_on_page=$(hget -t0 ${coverletter_line},3:${coverletter_line},5)
	      if [[ ${#last_line_on_page} > 0 ]]
	      then
		break
	      fi
	      ((coverletter_line-=1))
	    done
	    # end of loop to find last line currently displayed
	  done
	  # end of loop to find find first line of comment

	  # if the first line of the comment was found
	  if ((comment_tag_found==0))
	  then
	    # if the first comment line is already the size note
	    if [[ ${first_comment} = "This ptf and all its requisites occupy" ]]
	    then
	      log_it "first comment line is already size line"
	      return 10
	    else
	      # scroll the lines to bring the comment tag line to the top
	      hsend -t${RETAIN_WAIT} ${first_comment_line}
	      # check that first comment line is first one displayed
	      if hexpect -t${RETAIN_WAIT} "@12,17:COMMENT"
	      then
		# position cursor to the entry area after the comment tag
		hsend -n -t${RETAIN_WAIT} -tab
		# check for the existence of text on the this line
		# and save any that is there
		old_comment=$(hget -t0 12,27:12,80)

		# if there was already some text on the first comment line
		if [[ -n ${old_comment} ]]
		then
		  # clear the old size note
		  hsend -n -t${RETAIN_WAIT} "                                                      "
		  # reposition the cursor to the start of the field
		  hsend -n -t${RETAIN_WAIT} -home -tab
		fi

		# write the retain version of the size note
		hsend -t${RETAIN_WAIT} "This ptf and all its requisites occupy ${2} diskette(s)"
		# end the modify subfunction
		hsend -t${RETAIN_WAIT} -pf11

		# if there was already some text on the first comment line
		if [[ -n ${old_comment} ]]
		then
		  # invoke ADD LINES mode
		  hsend -t${RETAIN_WAIT} -pf9
		  # position cursor to the entry area after the comment tag
		  hsend -t${RETAIN_WAIT} -tab -enter
		  # write original contents for first comment line
		  hsend -t${RETAIN_WAIT} "${old_comment}"
		  # end add subfunction
		  hsend -t${RETAIN_WAIT} -pf11
		  # store previous subfunction changes
		  hsend -t${RETAIN_WAIT} -pf11
		fi

		# verify the data and terminate the edit session
		hsend -t${RETAIN_WAIT} -pa2
		# check for proper completion of edit
		if hexpect -t${RETAIN_WAIT} "@11,2:EDIT COMPLETED, PAGES QUEUED FOR UPDATE"
		then
		  log_it "Size note added to PTF ${1}"
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
}

function add_retain_note
{
  # first parameter must be the PTF number
  # second parameter must be the size in diskettes
  # third parameter is the superseding PTF number, if there is one
  if [[ ${tracing_on} = 0 ]]
  then
    set -x
  fi

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
	  add_size_comment ${1} ${2} ${3}
	  return ${?}
	else
	  log_it "EDIT is not a valid option"
	  return 4
	fi
      else
	if [[ ${external_status} = "CLOS" ]]
	then
	  # if logged on under a level 2 authority RETAIN id
	  if ((level2_mode==0))
	  then
	    log_it "${1} external status is closed"
	    # confirm that EDIT is a valid option for this ptf
	    if command_expect "EDIT"
	    then
	      add_size_comment ${1} ${2} ${3}
	      return ${?}
	    else
	      log_it "EDIT is not a valid option"
	      return 4
	    fi
	  else
	    log_it "${1} external status closed, needs no level 2 authority id"
	    return 14
	  fi
	else
	  log_it "${1} external status is not open, cand, nor closed"
	  return 3
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

function edit_size_comment
{
  # first parameter must be the PTF number
  # second parameter must be the size in diskettes
  # third parameter is the superseding PTF number, if there is one
  if [[ ${tracing_on} = 0 ]]
  then
    set -x
  fi

  typeset -i coverletter_line
  typeset -i last_coverletter_line=22
  typeset -i first_coverletter_line=11
  typeset -i comment_tag_found=1

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
	  ((coverletter_line=last_coverletter_line))
	  while ((coverletter_line>=first_coverletter_line))
	  do
	    last_line_on_page=$(hget -t0 ${coverletter_line},3:${coverletter_line},5)
	    if [[ ${#last_line_on_page} > 0 ]]
	    then
	      break
	    fi
	    ((coverletter_line-=1))
	  done
	  # end of loop to find last line currently displayed

	  # find the line number of the first comment line
	  while [[ (${last_line_on_page} < ${current_last_line}) || (${last_line_on_page} = ${current_last_line}) ]]
	  do
	    # search for the comment tag line starting with the last
	    # line currently displayed
	    ((coverletter_line=last_coverletter_line))
	    while ((coverletter_line>=first_coverletter_line))
	    do
	      field_name=$(hget -t0 ${coverletter_line},17:${coverletter_line},25)
	      if [[ ${field_name} = "COMMENTS:" ]]
	      then
		((comment_tag_found=0))
		# get the variable portion of the comment
		first_comment=$(hget -t0 ${coverletter_line},27:${coverletter_line},80)
		first_comment_line=$(hget -t0 ${coverletter_line},3:${coverletter_line},5)
		break 2
	      fi
	      ((coverletter_line-=1))
	    done
	    # end of loop to search for the comment tag

	    # scroll to next set of cover letter lines
	    hsend -t${RETAIN_WAIT} -enter

	    # find the last line number displayed after scrolling
	    ((coverletter_line=last_coverletter_line))
	    while ((coverletter_line>=first_coverletter_line))
	    do
	      last_line_on_page=$(hget -t0 ${coverletter_line},3:${coverletter_line},5)
	      if [[ ${#last_line_on_page} > 0 ]]
	      then
		break
	      fi
	      ((coverletter_line-=1))
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
	      # position cursor to the entry area after the comment tag
	      hsend -t${RETAIN_WAIT} -tab
	      # clear the old size note
	      hsend -n -t${RETAIN_WAIT} "                                                      "
	      # reposition the cursor to the start of the field
	      hsend -n -t${RETAIN_WAIT} -home -tab
	      # write the retain version of the size note
	      hsend -t${RETAIN_WAIT} "This ptf and all its requisites occupy ${2} diskette(s)"
	      # end the modify subfunction
	      hsend -t${RETAIN_WAIT} -pf11
	      # verify the data and terminate the edit session
	      hsend -t${RETAIN_WAIT} -pa2
	      # check for proper completion of edit
	      if hexpect -t${RETAIN_WAIT} "@11,2:EDIT COMPLETED, PAGES QUEUED FOR UPDATE"
	      then
		log_it "Size note added to PTF ${1}"
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
}

function edit_retain_note
{
  # first parameter must be the PTF number
  # second parameter must be the size in diskettes
  # third parameter is the superseding PTF number, if there is one
  if [[ ${tracing_on} = 0 ]]
  then
    set -x
  fi

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
	  edit_size_comment ${1} ${2} ${3}
	  return ${?}
	else
	  log_it "EDIT is not a valid option"
	  return 4
	fi
      else
	if [[ ${external_status} = "CLOS" ]]
	then
	  # if logged on under a level 2 authority RETAIN id
	  if ((level2_mode==0))
	  then
	    log_it "${1} external status is closed"
	    # confirm that EDIT is a valid option for this ptf
	    if command_expect "EDIT"
	    then
	      edit_size_comment ${1} ${2} ${3}
	      return ${?}
	    else
	      log_it "EDIT is not a valid option"
	      return 4
	    fi
	  else
	    log_it "${1} external status closed, needs no level 2 authority id"
	    return 14
	  fi
	else
	  log_it "${1} external status is not open, cand, nor closed"
	  return 3
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

function update_retain_note
{
  # first parameter must be the PTF number
  # second parameter must be the size in diskettes
  # third parameter is the superseding PTF number, if there is one
  if [[ ${tracing_on} = 0 ]]
  then
    set -x
  fi

  if check_retain_note ${1} ${2} ${3}
  then
    print -u4 "${1}\tRETAIN ALREADY UPDATED"
  else
    # if a size comment line was found in retain
    if ((retain_size_found==0))
    then
      # edit the existing size comment line on the cover letter page
      if edit_retain_note ${1} ${2} ${3}
      then
	print -u4 "${1}"
      else
	print -u5 "${1}\tRETAIN NOT UPDATED"
      fi
    else
      # add the size comment line as the first comment line on the cover
      # letter page
      if add_retain_note ${1} ${2} ${3}
      then
	print -u4 "${1}"
      else
	print -u5 "${1}\tRETAIN NOT UPDATED"
      fi
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

  CMVC_FAMILY="${FAMILY:=aix}"

  # set the subdirectory name of the default directories
  type "$aix_version"_pkg_environment >/dev/null 2>&1
  [ $? -eq 0 ] && . "$aix_version"_pkg_environment
  prod_dir="$PROD_DIRNAME"


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

  typeset -i retain_size_found=1

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
      base_name="${RETAIN_OUTPUT}/${nickname}"
    else
      base_name="${RETAIN_OUTPUT}/${base_name}"
    fi
  fi

  ptfsize_file="${base_name}.ptfsize"
  ptfsize_filedes="4"
  noptfsize_file="${base_name}.noptfsize"
  noptfsize_filedes="5"
  defsize_file="${base_name}.defsize"
  defsize_filedes="6"
  nodefsize_file="${base_name}.nodefsize"
  nodefsize_filedes="7"
  baddefsize_file="${base_name}.baddefsize"
  baddefsize_filedes="8"
  retain_log_base_name="${base_name}.${nickname}"
  retain_log="${retain_log_base_name}.retainlog"
  errmsg_file="${base_name}.${nickname}.errmsg"

  # if the cmvc only flag was NOT turned on
  if ((cmvc_only==1))
  then
    # if the save files mode was not turned off
    if [[ ${save_files} = 0 ]]
    then
      if [[ -f ${ptfsize_file} ]]
      then
	mv ${ptfsize_file} "${ptfsize_file}${time_suffix}"
      fi

      if [[ -f ${noptfsize_file} ]]
      then
	mv ${noptfsize_file} "${noptfsize_file}${time_suffix}"
      fi
    fi
    # open ptfsize output file as descriptor 4
    exec 4> ${ptfsize_file}
    # open noptfsize output file as descriptor 5
    exec 5> ${noptfsize_file}

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
      if [[ -f ${defsize_file} ]]
      then
	mv ${defsize_file} "${defsize_file}${time_suffix}"
      fi

      if [[ -f ${nodefsize_file} ]]
      then
	mv ${nodefsize_file} "${nodefsize_file}${time_suffix}"
      fi
    fi
    # open defsize output file as descriptor 6
    exec 6> ${defsize_file}
    # open nodefsize output file as descriptor 7
    exec 7> ${nodefsize_file}
  fi

  # open defect not found output file as descriptor 8
  exec 8> ${baddefsize_file}

  while read -u${input_data} ptf_num apar_list
  do
    # remove any error messages from apar list due to previous run
    apar_list=${apar_list%\|*}
    # store apars numbers an array
    set -A apar_array ${apar_list}

    # see if the initial ptf has been superseded
    superseding_ptf=""
### SPM    fgrep "${ptf_num} HAS" ${prod_dir}/ptfs.superseded | awk '{print $6}'|
### SPM    while read ptf
### SPM    do
### SPM          superseding_ptf=${ptf}
### SPM    done
    # if a superseding ptf was found
    if [[ ${superseding_ptf} != ""  ]]
    then
     # see if the number of diskettes for this ptf has already been determined
     # (i.e. is stored in the $prod_dir/ptfs.size file)
     ptf_size=$(grep "^${superseding_ptf} " ${prod_dir}/ptfs.size |
		awk '{print $7}')
     if [[ ${#ptf_size} > 0 ]]
     then
      build_size=${ptf_size}
     else
      # get the number of diskettes this ptf and its requisites would occupy
      get_size_from_build ${superseding_ptf}
      rc=${?}
      # if did not get the PTF size
      if ((rc!=0))
      then
	print -u7 "${superseding_ptf}\t${apar_array[@]}|FAILED TO GET SIZE FROM BUILD"
	continue
      fi
     fi

     # if the retain only flag was NOT turned on
     #if ((retain_only==1))
     #then
       # update the cmvc size note for the SUPERSEDING ptf, if it needs to be
       # (SPM) update_cmvc_note ${superseding_ptf} ${build_size} ${apar_array[@]}
     #fi

     # if the cmvc only flag was NOT turned on
     if ((cmvc_only==1))
     then
       # update the retain size note for the SUPERSEDED ptf, if it needs to be
       update_retain_note ${ptf_num} ${build_size} ${superseding_ptf}
     fi
    else
     # see if the number of diskettes for this ptf has already been determined
     # (i.e. is stored in the $prod_dir/ptfs.size file)
     ptf_size=$(grep "^${ptf_num} " ${prod_dir}/ptfs.size |
		awk '{print $7}')
     if [[ ${#ptf_size} > 0 ]]
     then
      build_size=${ptf_size}
     else
      # get the number of diskettes this ptf and its requisites would occupy
      get_size_from_build ${ptf_num}
      rc=${?}
      # if did not get the PTF size
      if ((rc!=0))
      then
	print -u7 "${ptf_num}\t${apar_array[@]}|FAILED TO GET SIZE FROM BUILD"
	continue
      fi
     fi

     # if the retain only flag was NOT turned on
     #if ((retain_only==1))
     #then
       # update the cmvc size note, if it needs to be
       # (SPM) update_cmvc_note ${ptf_num} ${build_size} ${apar_array[@]}
     #fi

     # if the cmvc only flag was NOT turned on
     if ((cmvc_only==1))
     then
       # update the retain size note, if it needs to be
       update_retain_note ${ptf_num} ${build_size}
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
  if [[ ${tracing_on} = 0 ]]
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
program_name=${program_name%.*}

# create a nickname for this program to be used as the prefix for all output
# files by deleting all lowercase letters from the program name
nickname=$(echo ${program_name} | tr -d '[a-z]')
# if the nickname produced is null, assign the default nickname
if [[ ${#nickname} = 0 ]]
then
 nickname="ASN"
 fi

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
typeset -i level2_mode=1
test_option=""
files_option=""
status_option=""
tracing_option=""
logging_option=""
viewing_option=""
wait_option=""
path_option=""
typeset  aix_version="32"
typeset -i dummy_apar_num=90000
typeset -i index

# check for command line options
while getopts :ab:cdhifln:F:V:op:rstuvw:xz next_option
do
  case ${next_option} in
  a) ((alternate_mode=0));;
  b) if [[ ${#OPTARG} > 0 ]]
     then
       retain_id_option="-b${OPTARG}"
       ((level2_mode=0))
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
  F) FAMILY=${OPTARG};;
  o) ((to_stdout=0));;
  p) if [[ ${#OPTARG} > 0 ]]
     then
       path_option="-p${OPTARG}"
       RETAIN_OUTPUT=${OPTARG}
     fi
     ;;
  V) echo ${OPTARG} | grep "^4" >/dev/null
     if [ $? -eq 0 ]; then
        aix_version="41"
     fi;;
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
