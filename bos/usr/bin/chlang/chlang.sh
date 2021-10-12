#!/usr/bin/ksh
# @(#)90	1.10  src/bos/usr/bin/chlang/chlang.sh, cmdnls, bos411, 9428A410j 5/16/94 14:23:19
#
# COMPONENT_NAME: (CMDNLS)
#
# FUNCTIONS: chlang.sh
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1989, 1994
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

#
# FILE NAME: chlang
#
# FILE DESCRIPTION: High-level shell command for changing the language
#   (LANG) environment variable in the /etc/environment file or a
#   user's individual .profile file. In addition, the NLSPATH environment
#   variable can be modified.
#
#   Basic functions performed are:
#   1)  change/add the LANG environment variable
#   2)  change/add/delete the NLSPATH environment variable
#   3)  change/add/delete the LC_MESSAGES environment variable
#
#   See Usage message for explanation of parms
#
#
# RETURN VALUE DESCRIPTION:
#                             0         Successful
#                             non-zero  Unsuccessful
#
#
# EXTERNAL PROCEDURES CALLED: none (other than standard utilities)
#
#
# GLOBAL VARIABLES: none
#

################################# usage #######################################
#
# NAME: usage()
#
# DESCRIPTION: Issue "usage" message and exit.
#
# INPUT:
#        None
#
# OUTPUT:
#        Error messages (Standard Error)
#
# RETURN VALUE DESCRIPTION:
#        2    - Indicates invalid usage
#
# NOTE: This function will not return (i.e., it will exit the entire
#       script with exit status of 2).
#
##############################################################################
usage(){
   dspmsg chlang.cat 2 \
	"Usage:\tchlang [ -u UID|Uname ] [ -m MsgTransLst | -M ] Language\n\
\tchlang [ -u UID|Uname ] -m MsgTransLst | -M\n\
\tchlang -d [ -u UID|Uname ]\n\
\tSets the default language for the system or a user.\n" chlang chlang chlang>&2
   echo
   exit 2                       # don't return
}

################################# del_var ###################################
#
# NAME: del_var()
#
# DESCRIPTION: Remove environment variable in /etc/environment file,
#		user's .profile file, or user's .cshrc file, depending 
#		on PROFILE variable.
#
# INPUT:
#        PROFILE   - Set to "/etc/environment","<home_dir>/.profile", or
#			"<home_dir>/.cshrc"
#	 ENV_VAR   - Set to environment variable to be deleted
#
# OUTPUT:
#        none
#
# RETURN VALUE DESCRIPTION:
#        0         - Successful removal of environment variable
#	 non-zero  - Error encountered in removal
#
##############################################################################
del_var(){

# Set EGREP_ARG and SED_ARG to appropriate argument value depending
# on whether we are modifying a .cshrc file or something else.
if [ $cshell -eq 1 ]
then
   EGREP_ARG="^setenv[ |	]+$ENV_VAR"
   SED_ARG="/^setenv[ |	]*$ENV_VAR/d"
else
   EGREP_ARG="$ENV_VAR="
   SED_ARG="/$ENV_VAR=/d"
fi

# Check to see if environment variable is currently used in the file
# to be modified. If it is, then use sed to remove the environment
# variable from the file. If the environment variable is not in the
# file, and the particular variable is "NLSPATH", display a warning
# message telling the user that NLSPATH was not in the file.
if grep -E -qs "$EGREP_ARG" $PROFILE > /dev/null 2>&1 ; then
   sed "$SED_ARG" $PROFILE >/tmp/chlang$$ && cp /tmp/chlang$$ $PROFILE

   # Save the result code for the sed command
   RTNCODE=$?

   # If this is not the C shell, remove the "export" of the variable
   # if it exists in the file.
   if [ $RTNCODE -eq 0 -a $cshell -ne 1 ]
   then
      if grep -E -qs "export[ |	]+$ENV_VAR" $PROFILE > /dev/null 2>&1 
      then
	sed "/export[ |	]*$ENV_VAR/d" $PROFILE >/tmp/chlang$$ && 
		cp /tmp/chlang$$ $PROFILE
      fi
   fi

   # Remove the temporary file
   rm /tmp/chlang$$

   # If an error occurred in deleting the environment variable, display
   # an error message and exit.
   if [ $RTNCODE -ne 0 ] ; then
      dspmsg chlang.cat 1 "chlang: Could not write to $PROFILE\n\
\tCheck permissions on the file.\n" chlang $PROFILE
      echo
      exit $RTNCODE
   fi
else
   # Check to see if the variable being removed is NLSPATH. If so, display
   # a warning message telling the user that NLSPATH was not in the file.
   if [ $ENV_VAR = "NLSPATH" ]
   then
      dspmsg chlang.cat 3 "Warning: NLSPATH not present in $PROFILE\n" $PROFILE
      echo
   fi
fi
}

################################# chng_var ###################################
#
# NAME: chng_var()
#
# DESCRIPTION: Change or add environment variable in /etc/environment,
#		user's .profile file, or user's .cshrc file, depending 
#		on PROFILE variable.
#
# INPUT:
#        PROFILE   - Set to "/etc/environment", "<home_dir>/.profile", or
#			"<home_dir>/.cshrc
#        ENV_VAR   - Set to environment variable to be modified
#        NEW_VAL   - New value for environment variable
#
# OUTPUT:
#        none
#
# RETURN VALUE DESCRIPTION:
#        0         - Successful modification of environment variable
#	 non-zero  - Error encountered in modification
#
##############################################################################
chng_var(){

# Set EGREP_ARG, SED_ARG, and ECHO_ARG to appropriate argument value
# depending on whether we are modifying a .cshrc file or something else
if [ $cshell -eq 1 ]
then
  EGREP_ARG="^setenv[ |	]+$ENV_VAR"
  SED_ARG="s!^setenv[ |	]*$ENV_VAR.*!setenv $ENV_VAR $NEW_VAL!"
  ECHO_ARG="setenv $ENV_VAR $NEW_VAL"
else
  EGREP_ARG="$ENV_VAR="
  SED_ARG="s!$ENV_VAR=.*!$ENV_VAR=$NEW_VAL!"
  ECHO_ARG="$ENV_VAR=$NEW_VAL\nexport $ENV_VAR"
fi

# Check to see if environment variable is currently in file to be
# modified. If it is, then use sed to set it to new value. If the
# environment variable is not present, add it to the end of the file.
if grep -E -qs "$EGREP_ARG" $PROFILE > /dev/null 2>&1 ; then
  sed "$SED_ARG" $PROFILE >/tmp/chlang$$ \
	&& cp /tmp/chlang$$ $PROFILE
else
  echo "$ECHO_ARG" >> $PROFILE
fi

# Save the result code for the file modification
RTNCODE=$?

# Remove the temporary file, if it was used
rm /tmp/chlang$$ > /dev/null 2>&1

# If an error occurred when modifiying the file, report it and exit.
if [ $RTNCODE -ne 0 ] ; then
  dspmsg chlang.cat 1 "chlang: Could not write to $PROFILE\n\
\tCheck permissions on the file.\n" chlang $PROFILE
  echo
  exit $RTNCODE
fi
}

############################## main ############################################
PATH=/usr/bin:/bin:/etc export PATH

# Initialize variables
uflag=0
mflag=0
Mflag=0
dflag=0
cshell=0

# Get effective user id for process owner
eff_id=`id -u`

# Parse command line looking for options
set -- `getopt u:m:dM? $*`

# If invalid syntax, give usage message
if [ $? != 0 ]
then
	usage
fi

# Scan through argument list looking for options. 
while [ $1 != -- ]
do
	case $1 in
	  -u)
		uflag=1
		username=$2
		shift;shift
		;;
	  -m)
		mflag=1
		message=$2
		shift;shift
		;;
	  -M)
		Mflag=1
		message=default
		shift
		;;
	  -d)
		dflag=1
		shift
		;;
	  -?)
		usage
		;;
	esac
done
shift

# Get requested language from command line
language=$1

# Check to see if the -d flag was used on the command line. 
if [ $dflag -eq 1 ]
then
	# If -d flag was used, check to see if a language was provided
	# on the command line or if either -m or -M flag was used. If this
	# is the case, give error message, display usage, and exit.
	if [ $# -ne 0 -o $mflag -eq 1 -o $Mflag -eq 1 ]
	then
		dspmsg chlang.cat 8 "chlang: Cannot use -d flag with "\
"Language, -m flag, or -M flag\n" chlang
		usage
	fi
else
	# If -d flag was not used, check to see if a language was provided
	# on the command line.  If a language was not specified AND neither
	# -M or -m were used, then display usage, and exit.
	if [ $# -eq 0 ]
	then
	   if [ $mflag -eq 0 -a $Mflag -eq 0 ]
	   then
	      dspmsg chlang.cat 4 "chlang: Language must be specified\n" chlang
	      usage
           fi
	fi
fi

# Check to see if -m and -M flags both used. Since these flags are
# mutually exclusive, give error message, display usage, and exit.
if [ $mflag -eq 1 -a $Mflag -eq 1 ]
then
   dspmsg chlang.cat 7 "chlang: Cannot use -m and -M flags together\n" chlang
   usage
fi

# if $SHELL is not defined, ensure that we have a default, so that the
# script will not fail.
if [ "${SHELL}" = "" ]
then
	SHELL=/usr/bin/sh
fi

# Check to see if root is running chlang. If effective ID is 0, then
# assume root is trying to modify /etc/environment file. If effective
# ID is not 0, then assume regular user is trying to modify its own
# .profile file.
if [ $eff_id -eq 0 ]
then
	PROFILE=/etc/environment
else
	login_shell=`basename $SHELL`
	if [ $login_shell = "csh" ]
	then
		cshell=1
		PROFILE=$HOME/.cshrc
	else
		PROFILE=$HOME/.profile
	fi
fi

# If user name provided on command line, do user processing
if [ $uflag -eq 1 ]
then
	# Initialize variable to indicate if user was found
	# in /etc/passwd file
	found=0

	# Get user ID of user, as if user name was entered
	userid=`id -u $username 2>/dev/null`

	# If user ID was used on command line, previous command will fail
	if [ $? != 0 ]
	then
		userid=$username
	fi
	
	# Check if non-superuser is trying to modify another user. This
	# is not allowed, so print an error message and exit.
	if [ "$username" != "$USER" -a "$userid" != "$eff_id" -a $eff_id -ne 0 ]
	then
		dspmsg chlang.cat 5 \
			"chlang: Cannot modify another user\n" chlang
		echo
		exit 2
	fi

	# Scan through /etc/passwd for user's entry. Set home directory
	# for later use when writing user's .profile
	cat /etc/passwd | awk 'BEGIN {FS=":"} {print $1, $3, $6, $7}' |
	while read entry_name entry_id home_dir shell_path
	do
		if [ "$username" = "$entry_name" -o "$userid" = "$entry_id" ]
		then
			# Change PROFILE to make modifications to 
			# user's .profile file
			PROFILE=$home_dir/.profile
			if [ "$shell_path" = "" ]
			then
				login_shell=`basename $SHELL`
			else
				login_shell=`basename $shell_path`
			fi
			if [ $login_shell = "csh" ]
			then
				cshell=1
				PROFILE=$home_dir/.cshrc
			fi
			found=1
			break
		fi
	done
	
	# If user not found in /etc/passwd, give error message and exit
	if [ $found -eq 0 ]
	then
		dspmsg chlang.cat 6 "chlang: User \"$username\""\
" not found in /etc/passwd file\n" chlang $username
		echo
		exit 2
	fi
fi

# If message translation list provided on command line, process it
if [ $mflag -eq 1 -o $Mflag -eq 1 ]
then
	# Initialize ENV_VAR to indicate which variable to work with
	ENV_VAR="LC_MESSAGES"

	# Get first language from message translation list
	first=`echo $message | awk 'BEGIN{FS=":"} {print $1}'`

	no_xlation=0
	if [ $mflag -eq 1 -a "$language" != "" ]
	then
		# does this language have system translations?
		/usr/lib/nls/lsmle -l $language >/dev/null 2>&1
		no_xlation=$?
	fi

	# if the user specified a language, then...

	# If the first language in list is different from the LANG
	# variable and there are no translations available for that language,
	# then set LC_MESSAGES variable to first language.  Otherwise,
	# remove LC_MESSAGES.

	# if the user specifies no language and -M is not used, then
	# LC_MESSAGES is not changed in any way.

	if [ "$language" != "" -a $Mflag -eq 0 ]
	then
		if [ $first != $language -a $no_xlation -eq 0 ]
		then
			NEW_VAL=$first
			chng_var
		else
			del_var
		fi
	fi

	# always remove LC_MESSAGES when -M flag is used
	if [ $Mflag -eq 1 ]
	then
		del_var
	fi

	# Initialize variable with path of languages
	beg="/usr/lib/nls/msg"

	# first add non *.cat paths

	# Set NLSPATH to initial search path
	path_val=$beg/%L/%N

	# If message translation list is not the default list, process
	# the list one language at a time. Add each language, preceded
	# by the default path, to the new NLSPATH value
	if [ $Mflag -ne 1 ]
	then
		echo $message | awk '{cnt=split($1,vals,":")
				      for (i=1;i<=cnt;i++)
				      print vals[i]}' |
		while read item
		do
			if [ "$item" != "C@lft" ]
			then
				path_val=$path_val:$beg/$item/%N
			fi
		done
	fi

	# Now add the *.cat versions

	# add initial *.cat search path
	path_val=$path_val:$beg/%L/%N.cat

	# If message translation list is not the default list, process
	# the list one language at a time. Add each language, preceded
	# by the default path, to the new NLSPATH value
	if [ $Mflag -ne 1 ]
	then
		echo $message | awk '{cnt=split($1,vals,":")
				      for (i=1;i<=cnt;i++)
				      print vals[i]}' |
		while read item
		do
			if [ "$item" != "C@lft" ]
			then
				path_val=$path_val:$beg/$item/%N.cat
			fi
		done
	fi

	# Add or change NLSPATH in the appropriate file
	ENV_VAR="NLSPATH"
	NEW_VAL=$path_val
	chng_var
fi

# If -d on command line, remove NLSPATH from either /etc/environment
# file or user's .profile file. Otherwise, modify LANG variable in
# the particular file
if [ $dflag -eq 1 ]
then
	ENV_VAR="NLSPATH"
	del_var
else
	if [ "$language" != "" ]
	then
		ENV_VAR="LANG"
		NEW_VAL=$language
		chng_var
	fi
fi
