#!/bin/ksh
# @(#)34        1.21  src/bldenv/pkgtools/AparsNeeded.sh, pkgtools, bos41B, 9504A 1/12/95 12:58:27
#
# COMPONENT_NAME: (RETAIN) Tools for interfacing to the Retain system
#
# FUNCTIONS: AparsNeeded
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this release)
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
SYNTAX: ${program_name} [+f][-n][-o][-p][+s][-t][-x][-V] -h | -i | levelname [releaseName]
FUNCTION: This program returns all the PTMs/FEATUREs in a given level that need
to have a companion RETAIN APAR created.  These defects are identified by
searching for blank or invalid reference fields.
If your CMVC ID is not the same as the login ID you are running under,
then you must have the environment variable CMVC_ID set to your CMVC ID.
PARAMETERS: Either the name of a level may be specified as a command line
argument or a list of defects may be specified via stdin, one defect per line.
If a level name is specified, the search can optionally be limited to a given
release or to all releases with a given suffix.  If no release nor suffix is
specified, the default is to search for all releases with a suffix of 320.
To run this script for defects pertaining to AIX version 4, use the -V flag.

FLAGS:
+f turns off the default action of saving existing copies of all output files
-h displays this help text
-i indicates that a list of defects is to be read from standard input
-n specifies the base name of all output files when input is from stdin
-o indicates that the list of defects that need apars created is to be
   written to standard output instead of to the default output file
-p specifies the subdirectory into which all output files are to be written
   (overrides RETAIN_OUTPUT)
+s turns off the displaying of status messages to standard output
   (the -o option also suppresses the status display)
-t turns on test mode where the RETAIN test system is used
-x turns on tracing of the script, results are written to stderr
-V version of AIX (defaults to AIX version 3)
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

function file_cleanup
{
  # close all output files
  exec 3>&-
  exec 4>&-
  exec 5>&-
  exec 6>&-
}

function get_releases
{
 # the first parameter must be a defect number
 if [[ ${tracing_on} = 0 ]]
 then
   set -x
 fi

 # find all the tracks whose release name ends in the specified release suffix
 # that are in integrate state
 cmvc_releases=$(Report -view trackview -wh "defectname='${1}' and \
   releasename like '%${RELEASE_SUFFIX}' and state='integrate'" \
   -become ${CMVC_ID} -raw |
   awk -F"|" '{print $1}')
 rc=${?}
 # if the command failed
 if ((rc!=0))
 then
  display_msg "CMVC command to get release list failed with return code of ${rc}"
  display_msg "$(<${errmsg_file})"
 fi
 if [[ -n ${cmvc_releases} ]]
 then
  return ${rc}
 else
  return -1
 fi
}

function confirm_presence_of_release
{
 # make sure the CMVC release for this defect is in the compids.table
 # so the CreateApar tool will be able to find it
 # an entry for this release must be present because CreateApar has to
 # supply the component id during the creation of an APAR

 # first parameter must be a CMVC defect number
 # second parameter must be a this defect's reference field, even if its blank

 typeset -i release_found_in_table=1

 if [[ ${tracing_on} = 0 ]]
 then
   set -x
 fi

 if get_releases ${1}
 then
  ((release_found_in_table=1))
  for cmvc_release in ${cmvc_releases}
  do
   # search for this release in the compids.table file
   fgrep ":${cmvc_release}:" ${RETAIN_PATH}/compids.table | grep -v "^#" > /dev/null 2>&1
   rc=${?}
   if ((rc==0))
   then
    ((release_found_in_table=0))
    break
   fi
  done
 else
  print -u6 "${1}\t${2}\t${cmvc_releases}\tCMVC RELEASE NOT FOUND FOR DEFECT"
  return ${?}
 fi

 # if any release was found in the table
 if ((release_found_in_table==0))
 then
  return 0
 else
  print -u6 "${1}\t${2}\t${cmvc_release}\tCMVC RELEASE NOT FOUND IN compids.table"
  # return the results of the search
  return ${rc}
 fi
}

function check_reference
{
  # first parameter must be prefix
  # second parameter must be defect number
  # third parameter msut be reference field, even if its blank
  if [[ ${tracing_on} = 0 ]]
  then
    set -x
  fi

  if [[ ${CMVC_VERSION} = "2" ]]
  then
     typeset -LZ defect_num
  else
     typeset -RZ6 defect_num
  fi
  defect_num=${2}

  display_msg "checking reference field of ${defect_num}"

  # if the test RETAIN system has been used to create APARs
  if ((test_mode==0))
  then
    if [[ ${3} = [tT][iI][xX][0-9][0-9][0-9][0-9][0-9] ]]
    then
      reference=${3#[tT][iI][xX]}
      apar_num="IX${reference}"
      # this defect already has a valid APAR number
      print -u5 "${defect_num}\t${apar_num}"
      return 0
    else
     phrase=$(Report -view noteview -where "defectname='${2}'" -raw -become ${CMVC_ID} |
	     cut -d'|' -f9 | grep "TEST RETAIN APAR:")
     if [[ -n ${phrase} ]]
     then
       phrase=$(Report -view noteview -where "defectname='${2}'" -raw -become ${CMVC_ID} |
       cut -d'|' -f9 | awk '/TEST RETAIN APAR:/{ref = $4};END {printf "%s\n",ref}')
       if [[ ${phrase} = [tT][iI][xX][0-9][0-9][0-9][0-9][0-9] ]]
       then
	 # this defect already has a valid APAR number
	 print -u5 "${defect_num}\t${phrase}"
	 return 0
       fi
     fi
    fi
  fi

  # if the reference field contains a valid RETAIN or NEDS APAR number
  if [[ ${3} = [iI][xX][0-9][0-9][0-9][0-9][0-9] || ${3} = [aA]00[0-9][0-9][0-9][0-9][0-9] ]]
  then
    reference=${3#[iI][xX]}
    reference=${reference#[aA]00}
    apar_num="IX${reference}"
    # this defect already has a valid APAR number
    print -u5 "${defect_num}\t${3}"
  else
    # if the defect is an apar
    if [[ ${1} = 'a' ]]
    then
      # all apar's should have a valid reference field already
      print -u4 "${defect_num}\t${3}"
    else
      # if the reference field is blank
      if [[ -z ${3} ]]
      then
	# as long as the defect is not an apar, a blank field indicates
	# it needs a RETAIN APAR created
	print -u3 "${defect_num}"
	confirm_presence_of_release ${defect_num} ${3}
      else
	# if a ptm's reference field contains a NEDS ptm number
	if [[ ${1} = 'p' && ${3} = [pP]00[0-9][0-9][0-9][0-9][0-9] ]]
	then
	  # treat this ptm like its reference field is blank
	  print -u3 "${defect_num}"
	  confirm_presence_of_release ${defect_num} ${3}
	# if a feature's reference field contains a NEDS dcr number
	elif [[ ${1} = 'd' && ${3} = [dD]00[0-9][0-9][0-9][0-9][0-9] ]]
	then
	  # treat this feature like its reference field is blank
	  print -u3 "${defect_num}\t${3}"
	  confirm_presence_of_release ${defect_num} ${3}
	else
	  # the reference field is unrecognizable, for now treat this ptm
	  # like its reference field is blank
	  # (ONCE A RETAIN APAR IS CREATED FOR THIS PTM, ITS REFERENCE
	  # FIELD WILL BE REPLACED WITH THE RETAIN APAR NUMBER.)
	  print -u3 "${defect_num}\t${3}"
	  confirm_presence_of_release ${defect_num} ${3}
	fi
      fi
    fi
  fi
  return 0
} ## END OF CHECK_REFERENCE ##

function main
{
  if [[ ${tracing_on} = 0 ]]
  then
    set -x
  fi

  # if the environment variable CMVC_ID is not set, assume that the current
  # login id is also the user's CMVC id
  CMVC_ID="${CMVC_ID:=$(logname)}"

  # if the environment variable CMVC_VERSION is not set, assume that the 
  # new CMVC release is used
  CMVC_VERSION="${CMVC_VERSION:=2}"

  # if the release suffix environment variable is not set, use the default
  RELEASE_SUFFIX="${RELEASE_SUFFIX:=${aix_version}0}"

  # subdirectory into which all output files are to be stored
  RETAIN_OUTPUT="${RETAIN_OUTPUT:=.}"

  # path to find compids.table file
  if [ ${aix_version} = "32" ]
  then
    RETAIN_PATH="${RETAIN_PATH:=/afs/austin/aix/320/bldenv/prod/usr/bin}"
  else
    RETAIN_PATH="${RETAIN_PATH:=${ODE_TOOLS:-/afs/austin/aix/410/project/aix4/build/latest/ode_tools/power}/usr/lib}"
  fi

  # if the standard input flag was not specified
  if [[ ${from_stdin} = 1 ]]
  then
    # the first command parameter is taken to be the name of a level
    if [[ -n ${1} ]]
    then
      level=${1}
    else
      display_msg "No level name was specified and no list of defects was supplied"
      display_msg "via standard input.  No action will be taken."
      display_msg "Invoke the script with the -h flag to display the help text."
      return 4
    fi
    # if the release name was not supplied
    if [ -z "${2}" ]
    then
      # use only the release suffix to search for defects
      release=${RELEASE_SUFFIX}
      ((level_name_unique=1))
    else
      # the second command line parameter is taken to be the full name
      # of a release or the suffix of a set of releases
      release=${2}
      # if only a suffix was specified
      if [[ ${2} = [3,4][0-9][0-9] ]]
      then
	((level_name_unique=1))
      else
	((level_name_unique=0))
      fi
    fi

    # create the base name of the output files and relate them to the name
    # of the level and the release or suffix
    base_name="${RETAIN_OUTPUT}/${level}.${release}"
  else
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

  needed_file="${base_name}.needed"
  notuniq_file="${base_name}.notuniq"
  badref_file="${base_name}.badrefs"
  badrelease_file="${base_name}.badrelease"
  not_needed_file="${base_name}.notneeded"
  errmsg_file="${base_name}.${action}.errmsg"

  # if the save files mode was not turned off
  if [[ ${save_files} = 0 ]]
  then
    display_msg "Saving all existing versions of output files"
    # save any existing versions of the output files
    if [[ -f ${needed_file} ]]
    then
      mv ${needed_file} "${needed_file}${time_suffix}"
    fi
    if [[ -f ${badref_file} ]]
    then
      mv ${badref_file} "${badref_file}${time_suffix}"
    fi
    if [[ -f ${badrelease_file} ]]
    then
      mv ${badrelease_file} "${badrelease_file}${time_suffix}"
    fi
    if [[ -f ${not_needed_file} ]]
    then
      mv ${not_needed_file} "${not_needed_file}${time_suffix}"
    fi
  fi

  # if the -o flag was not specified
  if [[ ${to_stdout} = 1 ]]
  then
    # open noref output file as descriptor 3
    exec 3> ${needed_file}
    display_msg "Writing list of defects without references to file ${needed_file}"
  else
    # assign file 3 to stdout
    exec 3>&1
    display_msg "Writing list of defects without references to standard output"
  fi

  # open badref output file as descriptor 4
  exec 4> ${badref_file}

  # open notneeded output file as descriptor 5
  exec 5> ${not_needed_file}

  # open badrelease output file as descriptor 6
  exec 6> ${badrelease_file}

  # if the standard input flag was not specified
  if [[ ${from_stdin} = 1 ]]
  then
    display_msg "Defect list will be taken from ${level} level"
    # if only a suffix was specified (i.e. the release name begins with a number)
    if [[ ${release} = [0-9]* ]]
    then
      display_msg "Search for defects will be limited to releases with suffix of ${release}"
      # run the CMVC Report command for a levelmember view where the releasename
      # ends with the given suffix
      level_members=$(Report -view levelmemberview -where "levelname='${level}' and releasename like '%${release}'" -raw -become ${CMVC_ID} 2> ${errmsg_file})
      rc=${?}
      if ((rc==0))
      then
	# pass the list of level members to awk to
	echo "${level_members}" |
	# select only the prefix, defect name, and reference field
	awk -F"|" '{printf "%s %s %s\n", $8, $3, $4}' |

	while read prefix defect reference
	do
          if [[ ${MULTI_APAR_DEFECT} = "yes" ]]
          then
              reference=$( qryAPARByDefect -d ${defect} -v ${aix_version} -r | \
			   awk -F"|" '{print $2}' )
          fi
	  check_reference ${prefix} ${defect} ${reference}
	done
      else
	display_msg "CMVC command to get level members failed with a return code of ${rc}"
	display_msg "$(<${errmsg_file})"
	((rc+=100))
	return ${rc}
      fi
    else
      display_msg "Search for defects will be limited to ${release} release"
      # run the CMVC command command for a levelmember view for releasename only
      level_members=$(Report -view levelmemberview -where "levelname='${level}' and releasename='${release}'" -raw -become ${CMVC_ID} 2> ${errmsg_file})
      rc=${?}
      if ((rc==0))
      then
	# pass the list of level members to awk to
	echo "${level_members}" |
	# select only the prefix, defect name, and reference field
	awk -F"|" '{printf "%s %s %s\n", $8, $3, $4}' |

	while read prefix defect reference
	do
          if [[ ${MULTI_APAR_DEFECT} = "yes" ]]
          then
              reference=$( qryAPARByDefect -d ${defect} -v ${aix_version} -r \
			   awk -F"|" '{print $2}' )
          fi
	  check_reference ${prefix} ${defect} ${reference}
	done
      else
	display_msg "CMVC command to get level members failed with a return code of ${rc}"
	display_msg "$(<${errmsg_file})"
	((rc+=100))
	return ${rc}
      fi
    fi
  else
    display_msg "Defect list being read from standard input"
    # standard input flag specified, read list of defects from stdin
    while read next_num remainder
    do
      if [[ ${CMVC_VERSION} = "2" ]]
      then
         type=$(Report -view defectview -where "name='${next_num}'" -raw -become ${CMVC_ID} \
            | dv.filter | awk -F"|" '{print $1}' 2> ${errmsg_file})
         rc=${?}
      else
         type=$(Report -view defectview -where "name='${next_num}'" -raw -become ${CMVC_ID} \
            | awk -F"|" '{print $1}' 2> ${errmsg_file})
         rc=${?}
      fi
      if ((rc==0))
      then
	# if the entry is a ptm or apar
	if [[ ${type} = 'p' || ${type} = 'a' ]]
	then
          if [[ ${CMVC_VERSION} = "2" ]]
          then
             defect_data=$(Report -view defectview -where "name='${next_num}'" -raw -become ${CMVC_ID} 2> ${errmsg_file} | dv.filter)
             rc=${?}
          else
             defect_data=$(Report -view defectview -where "name='${next_num}'" -raw -become ${CMVC_ID} 2> ${errmsg_file})
             rc=${?}
          fi
	  if ((rc==0))
	  then
	    echo "${defect_data}" |
	    # select only the prefix, the defect name, the reference field, and the release name
	    awk -F"|" '{printf "%s %s %s\n", $1, $2, $26}' |

	    read prefix defect reference

            if [[ ${MULTI_APAR_DEFECT} = "yes" ]]
            then
                reference=$( qryAPARByDefect -d ${defect} -v ${aix_version} -r \
			     awk -F"|" '{print $2}' )
            fi
	    check_reference ${prefix} ${defect} ${reference}
	  else
	    display "CMVC command to get data for ${next_num} failed with rc=${rc}"
	    display_msg "$(<${errmsg_file})"
	    ((rc+=100))
	    return ${rc}
	  fi
	else
	  if [[ -n ${type} ]]
	  then
            if [[ ${CMVC_VERSION} = "2" ]]
            then
               feature_data=$(Report -view featureview -where "name='${next_num}'" -raw -become ${CMVC_ID} 2> ${errmsg_file} | fv.filter)
               rc=${?}
            else
               feature_data=$(Report -view featureview -where "name='${next_num}'" -raw -become ${CMVC_ID} 2> ${errmsg_file})
               rc=${?}
            fi
	    if ((rc==0))
	    then
	      echo "${feature_data}" |
	      # select only the prefix, the feature name, the reference field, and the release name
	      awk -F"|" '{printf "%s %s %s\n", $1, $2, $18}' |

	      read prefix feature reference

              if [[ ${MULTI_APAR_DEFECT} = "yes" ]]
              then
                  reference=$( qryAPARByDefect -d ${defect} -v ${aix_version} -r \
			       awk -F"|" '{print $2}' )
              fi
	      check_reference ${prefix} ${feature} ${reference}
	    else
	      display_msg "CMVC command to get data for ${next_num} failed with rc=${rc}"
	      display_msg "$(<${errmsg_file})"
	      ((rc+=100))
	      return ${rc}
	    fi
	  else
	    display_msg "CMVC command to get prefix of ${next_num} did not return anything"
	    display_msg "Defect/feature number ${next_num} probably does not exist"
	    ((rc+=100))
	    return ${rc}
	  fi
	fi
      else
	display_msg "CMVC command to get prefix for ${next_num} failed with rc=${rc}"
	display_msg "$(<${errmsg_file})"
	((rc+=100))
	return ${rc}
      fi
    done
  fi

  exec 3>&-

  if ((level_name_unique!=0))
  then
    mv ${needed_file} ${notuniq_file}
    cat ${notuniq_file} | sort -u > ${needed_file}
    rm ${notuniq_file}
  fi

  # delete error message file
  rm ${errmsg_file}
  return 0
} ## END OF MAIN ##

function bailout
{
  if [[ ${tracing_on} = 0 ]]
  then
    set -x
  fi

  display_msg "AparsNeeded Program Interrupted"
  exit 3
}

##### START OF PROGRAM #####
trap "bailout" HUP INT QUIT TERM

# find base name of the program
program_name=${0##*/}

display_msg "RUNNING ${program_name} WITH PARAMETERS ${@}"

# remove any program name suffix to use remaining portion to name output files
action=${program_name%.*}

typeset -i rc=0
typeset -i level_name_unique=1

typeset -i tracing_on=1
typeset -i status_on=0
typeset -i save_files=0
typeset -i from_stdin=1
typeset -i to_stdout=1
typeset -i test_mode=1
typeset -i unannounced_mode=1

typeset aix_version="32"

# check for command line options
while getopts :fhin:sop:tuw:xV: next_option
do
  case ${next_option} in
  h) display_help
     exit 1;;
  i) ((from_stdin=0));;
  +f) ((save_files=1));;
  n) base_name=${OPTARG};;
  o) ((to_stdout=0));;
  p) if [[ ${#OPTARG} > 0 ]]
     then
       RETAIN_OUTPUT=${OPTARG}
     fi
     ;;
  +s) ((status_on=1))
      status_option="+s";;
  t) ((test_mode=0));;
  u) ((unannounced_mode=0));;
  x) ((tracing_on=0));;
  V) echo ${OPTARG} | grep "^4" >/dev/null
     if [ $? -eq 0 ]
     then
       aix_version="41"
     fi;;
  :) display_msg "${program_name}: ${OPTARG} requires a value"
     exit 2;;
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
