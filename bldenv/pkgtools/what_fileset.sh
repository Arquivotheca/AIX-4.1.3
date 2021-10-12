#!/bin/ksh
# @(#)31	1.8  src/bldenv/pkgtools/what_fileset.sh, pkgtools, bos41J, 9509A_all 2/28/95 17:14:07
#
# COMPONENT_NAME: PKGTOOLS
#
# FUNCTIONS: 
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1994, 1995
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#**********************************************************************
#
# NAME: what_fileset
#
# DESCRIPTION: This script determines which fileset the given file
#              belongs to.  It writes the fileset name to STDOUT
#              and to the FILESET.list file.  If the file cannot
#              be mapped to a fileset, an error message is generated
#              and the filename is added to the FILESET.list.err file.
# PRECONDITIONS:
#      FILESET.db - This script expects that the FILESET.db file
#              contains the information to map a filename to a fileset.
#              This database resides in the FILESET_DIR directory in the
#              current build tree.  If this file does not exist, this
#              script exists NORMALLY; assuming that mixed filesets
#              is NOT active for this build.
#      BASE -  Environment variable identifying the base of the build tree
#      ODE_TOOLS - environment variable identifying the location of the 
#              bldenv tools.
#
# POSTCONDITONS: 
#      FILESET.list - file in BLD_LOG_DIR directory. 
#              This file is the list of all the changed filesets.
#      FILESET.list.err - file in the BLD_LOG_DIR directory. 
#              This file identifies the files for which no match was 
#              found in the FILESET.db.
#
# SIDE EFFECTS: None
#
# NOTES       :
#    - BLDCYCLE environment variable is used to identify the location
#      of the FILESET.* files.  This variable may be null or unset
#      and operation will continue successfully.
#    - If the FILESET.db cannot be found, this script exist NORMALLY
#      making the assumption that mixedFilesets is not part of this
#      build.
#
# RETURNS: 0 on succesful completion
#*************************************************************************

#
# EMERGENCY WORKAROUND TO PATH PROBLEM
#
PATH=${ODE_TOOLS}/usr/bin:${ODE_TOOLS}/usr/bld:${PATH}

# set the command name for error messages
myName=${0##*/}
myFill=`echo $myName | sed -e 's/./ /g'`

#===============================================================
# FUNCTION NAME : Usage
# DESCRIPTION   : Display the usage statment for this
#                 script.
# INPUTS        : none
# OUTPUTS       : 
#      usage statement sent to STDERR.
# RETURNS       :
#      exits with a return code of 1
#===============================================================
usage()
{
    cat <<END_USAGE: >&2

$myname USAGE : $myName -i <file>
   where: 
      <file> is the file for which the fileset is desired.
             It is not required that <file> be a fullpath,
	     when fullpath is not used a global search will
	     be used.  Regular expressions are allowed using
	     grep regular expression syntax.
       -i    flag causes the inslist file for the fileset
	     to be touched in the source tree.

END_USAGE:
	exit 1
} # END usage

#===============================================================
# FUNCTION NAME : touch_il
#
# DESCRIPTION   :
#	If -i flag was specified then touch the inslist in the src tree.
#	If the inslist database is empty then give error that inslist 
#	cannot be touched else check to see if src/packages directory 
#	exists and touch the inslist.
#
# INPUTS        : SRC (path to src directory - global)
#		  file (the inslist name. Set before call to touch_il)
#		  ilfile (The inslist name with full path. Set before call)
#
# OUTPUTS       : none
#
# SIDE EFFECTS  : .il file is touched
#
# RETURNS       : 0
#===============================================================
touch_il()
{

	if [ -z "${file}" ]; then
		print -u2 "$myName: WARNING: Could not get inslist \"${file}\" from FILESET.db"
		print -u2 "$myName: WARNING: Inslist will not be touched."
		return
	fi
	if [ ! -d "${SRC}/packages" ]; then
		print -u2 "$myName: WARNING: \"${SRC}/packages\" directory does not exist."
		print -u2 "$myName: WARNING: \"${ilfile}\" cannot be touched."
	else
		if [ ! -f "${ilfile}" ]; then
			print -u2 "$myName: WARNING: \"${ilfile}\" does not exist."
			print -u2 "$myName: WARNING: Inslist will not be touched."
		else
			# touch the inslist
			print -u2 "$myName: Touching \"${ilfile}\" "
			touch ${ilfile}
		fi
		if [ ! -f "${crfile}" ]; then
			print -u2 "$myName: WARNING: \"${crfile}\" does not exist."
			print -u2 "$myName: WARNING: Inslist will not be touched."
		else
			# touch the inslist
			print -u2 "$myName: Touching \"${crfile}\" "
			touch ${crfile}
		fi
	fi
}


#===============================================================
#  B E G I N    M A I N
#===============================================================
# temporary files
tmpfile1=/tmp/temp1.$$
tmpfile2=/tmp/temp2.$$

#-----------------------------------------------------------
# 1st make sure that all env vars are set appropriately.
#-----------------------------------------------------------
if [ -z "${BASE}" ]; then
     print -u2 "$myName: ERROR: Environment variable BASE is not set."
     print -u2 "$myFill Please correct. Processing stopped"
     exit 1
fi

typeset EXPORTBASE=${BASE}/export/power
typeset SRC=${BASE}/src
typeset BLD_LOG_DIR=${BASE}/selfix/LOG

if [[ ! -z ${BLDCYCLE} ]]
then
    BLD_LOG_DIR=${BLD_LOG_DIR}/${BLDCYCLE}
fi

typeset FILESET_LIST_ERR=${BLD_LOG_DIR}/FILESET.list.err
typeset FILESET_LIST=${BLD_LOG_DIR}/FILESET.list
typeset FILESET_DIR=${EXPORTBASE}/usr/fileset

typeset DB="${FILESET_DIR}/FILESET.db"
typeset touchil=""

#------------------------------------------------------------------
# Now check to see that we can get to all the
# files we need.
#
# NOTE that if we can't get to the DB, we'll assume
#      that we are not doing mixedFilesets and exit 1.
#------------------------------------------------------------------
if [ ! -s "${DB}" ]
then
      print -u2 "$myName: WARNING: \"${DB}\" does not exist or is empty."
      print -u2 "$myFill Assuming mixedFilesets is NOT active!"
      exit 1
fi 

#-------------------------------------------------------------------
#  If we can't get to the FILESET.list files in the LOG directory,
#  try to copy them from the export tree or create them.
#
#  The copy is done because normally, we want to build upon the
#  list already created during previous build.  The copy should
#  have already been done by the MFS_admin utility.
#-------------------------------------------------------------------
if [ ! -w "${FILESET_LIST}" ] ; then
    if [[ -f ${FILESET_DIR}/FILESET.list ]]
    then
        cp ${FILESET_DIR}/FILESET.list ${FILESET_LIST} || {
	print -u2 "Could not get FILESET.list from export tree (${FILESET_DIR})"
	exit 1
        }
    else
	touch ${FILESET_LIST} 2> /dev/null
    fi
fi
if [ ! -w "${FILESET_LIST_ERR}" ] ; then
    if [[ -f ${FILESET_DIR}/FILESET.list.err ]]
    then
        cp ${FILESET_DIR}/FILESET.list.err ${FILESET_LIST_ERR} || {
	print -u2 "Could not get FILESET.list.err from export tree (${FILESET_DIR})"
        exit
        }
    else
	touch ${FILESET_LIST_ERR} 2> /dev/null
    fi
fi

#------------------------------------------------------------
# Finally, get the command line parameters
#------------------------------------------------------------
if [ "$1" = "-i" ]; then
      touchil="yes"
      shift
else
      touchil="no"
fi

shipped_file="$1"

# remove the temporary file
rm -f $tmpfile1 $tmpfile2 > /dev/null 2>&1

trap 'rm -f $tmpfile1 $tmpfile2; exit 1' INT QUIT HUP TERM

if [ ! "$shipped_file" ]
then
    usage                # Give usage statement if no ship file given.
fi

# search for the file in the database. If the filename has
# a "/" in it then search for it as such else add the "/" to the it.
echo $shipped_file | grep "/" > /dev/null 2>&1
if [ $? -eq 0 ]
then
    # Check for various language msg catalogs
    echo $shipped_file | grep /lib/nls/msg > /dev/null
    if [ $? -eq 0 ]; then
	echo $shipped_file | grep /msg/C/ > /dev/null
	if [ $? -eq 0 ]; then
	    grep "^$shipped_file:" $DB > $tmpfile1
	    rc=$?
            if [ $rc -ne 0 ]; then
                  print -u2 "$myName: ERROR: No match found for \"$shipped_file\" "
		  if [ -f $FILESET_LIST_ERR ]; then
                  	echo "$shipped_file" >> $FILESET_LIST_ERR
		  fi
                  exit 3
            fi
        else
	    # This part of the code translates the language from 
	    # non english to en_US and looks for a match for the 
	    # english in FILESET.db file. If a match is found then 
	    # converts back to the language and writes the fileset to 
	    # FILESET.list file. Example:
	    # shipped_file: /usr/lib/nls/msg/Ja_JA/foo.cat
	    # front: /usr/lib/nls
	    # backtmp: Ja_JA/foo.cat
	    # back: foo.cat
	    # lang: Ja_JA
	    # newarg: /usr/lib/nls/msg/en_US/foo.cat
	    # if a match is found for newarg in FILESET.db
	    # then convert back to Ja_JA and write the fileset
	    # to FILESET.list file.
	    front=${shipped_file%/msg/*}
            backtmp=${shipped_file#*/msg/}
            back=${backtmp#*/}
            lang=${backtmp%/*}
            newarg="${front}/msg/en_US/${back}"
            grep "^$newarg:" $DB > /$tmpfile1 && {
                    cat $tmpfile1 | sed s,en_US,$lang,g > $tmpfile2
                    mv -f $tmpfile2 $tmpfile1
            }
        fi
    else
            grep "^$shipped_file:" $DB > $tmpfile1
	    rc=$?
            if [ $rc -ne 0 ]; then
                  print -u2 "$myName: ERROR: No match found for \"$shipped_file\" "
		  if [ -f $FILESET_LIST_ERR ]; then
	                  echo "$shipped_file" >> $FILESET_LIST_ERR
		  fi
                  exit 3
            fi
    fi
else
        grep "/$shipped_file:" $DB > $tmpfile1
	rc=$?
        if [ $rc -ne 0 ]; then
             print -u2 "$myName: ERROR: No match found for \"/$shipped_file\" "
	     if [ -f $FILESET_LIST_ERR ]; then
	             echo "$shipped_file" >> $FILESET_LIST_ERR
	     fi
             exit 3
        fi
fi

# Get the fileset from the tempfile.  Only add it to the
# changed fileset list if it is not already there.
cat $tmpfile1 | while read line
do
        fileset=`echo $line | cut -f2 -d:`
        filetype=`echo $line | cut -f3 -d:`
        file=`echo $line | cut -f4 -d:`
	ilfile="${SRC}/packages/${file}"
	crfile=${ilfile%\.*}.cr

	case "$filetype" in
		F|f|FT|ft|I|i|IT|V|v|VT|vt|A|a)
			echo $fileset
			grep $fileset $FILESET_LIST >/dev/null 2>&1
			if [[ $? -ne 0 && -f $FILESET_LIST ]]
			then
		    		echo $fileset >> $FILESET_LIST
			fi
			if [ "${touchil}" = "yes" ]; then
				touch_il
			fi
			;;
	esac
done

rm -f $tmpfile1 $tmpfile2 > /dev/null 2>&1
exit 0
