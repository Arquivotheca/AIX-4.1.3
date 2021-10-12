#! /bin/ksh
# @(#)30	1.12  src/bldenv/bldtools/showstatus.sh, bldtools, bos412, GOLDA411a 3/28/94 16:10:25
#
# COMPONENT_NAME: (BLDTOOLS) BAI Build Tools
#
# FUNCTIONS: showstatus
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1991
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# NAME: showstatus
#
# PURPOSE: Queries the status file and generates various formatted reports 
#	   from its contents.  Also allows user to modify/delete entries in
#	   the status file.  Operates from a menu mode or by passing command
#	   line options.
#
# INPUT: optional command line arguments, BLDCYCLE and STATUS_FILE environment
#	 variables may optionally be initialized before the call.
#
# OUTPUT: Standard output
#
# RETURNS: 0 if successful, 1 if errors
#
# EXECUTION ENVIRONMENT: Build process environment
#

. bldinitfunc	# define build init functions
. bldkshconst	# define constants

############################################################################
# function:	PrintLine
# description:	Prints a formatted line of data to stdout
# input:	1 line of unformatted data of the form:
#
# 		field1|field2|field3|...
#
# output:	1 formatted line
# remarks:	Expects the global variable $formatstr to be initialized so
# 		that it can be passed to awk and used to format output.  This
#		routine closely paralells the Format routine.  If this routine
#		changes then it may be necessary to change the Format routine
#		as well.
############################################################################
function PrintLine {
	echo "$1" | awk -F"|" ' { 
		printf fmt,$1,$2,$3,$4,$5,$6,$7,$8,$9,$10
	} ' fmt="$formatstr"
}

############################################################################
# function:	PrintHeader
# description:	Prints out title and header bar for a report
# input:	1 title string
#		1 unformatted header string containing all the field header
#		names for a given report; should be in the form:
#
#		fldhdr1|fldhdr2|fldhdr3|...
#
# output:	report header of the form:
#
#		report title
#		fldhdr1 fldhdr2 fldhdr3 ...
#		------- ------- ------- ...
#
############################################################################
function PrintHeader {
	print 		# blank line
	print "$1\c"
	if [ -z "$BLDCYCLE" ]
	then
		print " FOR ALL BUILD CYCLES"
	else
		print " FOR BUILD CYCLE $BLDCYCLE"
	fi
	PrintLine "$2"
	PrintLine "$BAR_HDR"
}

############################################################################
# function:	Format
# description:	Prints out a complete formatted report
# input:	1 title
# 		1 file name containing raw report data
#		1 unformatted header string containing all the field header
#		names for a given report; should be in the form:
#
#		fldhdr1|fldhdr2|fldhdr3|...
# 
# output:	A complete formatted report
# remarks:	This routine closely paralells the PrintLine routine.  If this 
#		routine changes then it may be necessary to change the Format 
#		routine	as well.
############################################################################
function Format {
	typeset title=$1
	typeset data=$2

	PrintHeader "$title" "$3"

	awk -F"|" ' { 
		printf fmt,$1,$2,$3,$4,$5,$6,$7,$8,$9,$10
	} ' fmt="$formatstr" $data

	print # blank line
}

############################################################################
# function:	SearchStatus
# description:	Prompts user for a search string (unless invoked with
#		-q <search str> option from command line) then searches
# 		entire status file looking for occurrence of string(s).
# 		If several separate search strings are given, the status
# 		file is searched for lines containing all of the strings.
#		All matches are then printed out in a formatted report.
# input:	status file entries of the form
#
#		field1|field2|field3...
#
# output:	formatted report containing all entries in status file that 
#		match string(s) specified by the given search string
############################################################################
function SearchStatus {
	Search
	if [ "$?" = "0" ]
	then
		formatstr="$ALL_FMT"
		Format "SEARCH: \"$searchstr\"" $TEMP "$ALL_HDR"
	fi
}

############################################################################
# function:	Search
# description:	Prompts user for a search string (unless invoked with
#		-q <search str> option from command line) then searches
# 		entire status file looking for occurrence of string(s).
# 		if several separate search strings are given, the status
# 		file is searched for lines containing all of the strings.
# input:	status file entries of the form
#
#		field1|field2|field3...
#
# output:	$TEMP file will contain unformatted status file entries that 
#		match string(s) specified by the given search string
############################################################################
function Search {
	typeset line 

	if [ ! -n "$searchstr" ]
	then
		print -n -u2 "search string: "; read searchstr < /dev/tty
	fi
	if [ ! -z "$BLDCYCLE" ]
	then
		QueryStatus $_BLDCYCLE $BLDCYCLE -A > $TEMP
	else
		QueryStatus -A > $TEMP
	fi

	for i in $searchstr
	do
		fgrep "$i" $TEMP > $SCRATCH_FILE
		mv $SCRATCH_FILE $TEMP
	done

	if [[ -f $TEMP && ! -s $TEMP ]]
	then
		print -u2 "No matches found."
		print -n -u2 "Press \"Enter\" key to continue..."
		read dummy < /dev/tty
		return 1
	fi

	return 0
}

############################################################################
# function:	ModifyLine
# description:	prompts user with current field values for a given line from
#		the status file; user is then able to change any field; new
#		line is then stored in the global variable $newline; any
#		fields that get changed are recorded in the global variable
#		$rptline which is used by the RepeatMod routine
# input:	1 line of raw data from status file of the form:
#
#		field1|field2|field3...
#
#		Field header strings for each field of any given line in 
#		the status file
# output:	For each field header string, the current value of the field
#		is displayed and the user is prompted for a new value
############################################################################
function ModifyLine {
	typeset line=$1; shift
	typeset -i count	# local counting variable
	typeset cur new 	# local string variables

	# make sure $BUILDER is set 
	if [ -z "$BUILDER" ] 
	then
		print 
		print "Builder name environment variable (\$BUILDER) must \c"
		print "be set for this action."
		chksetbuilder
	fi

	print
	# create and/or zero scratch files
	newline=""
	rptline=""
	count=1
	for fieldname in "$@"
	do
		cur=`echo $line | cut -d'|' -f$count`
		count=count+1

		# print out current field value; read in new one
		if [ ! -z "$cur" ]
		then
			print -u2 $fieldname" = $cur\c"
		else
			print -u2 $fieldname" = \"\"\c"
		fi

		# builder name is the only field for which the default value
		# is not the current value but rather the value of the 
		# $BUILDER environment variable with a '*' prepended to it
		if [ "$fieldname" = "$BUILDER_HDR" ]
		then
		    # resetting the current value to "*$BUILDER" so that 
		    # it will be taken as the default
		    cur="*$BUILDER"

		    # print out the exception message for builder which
		    # also gives the value of $BUILDER
		    print -u2 " (default = \"*\$BUILDER\" = \"*$BUILDER\")\c"
		fi
		print -u2 ": \c"

		# read in new value
		read new < /dev/tty

		# if a new value was entered, process it
		if [ -n "$new" ]
		then
			# prepend '*' to a modified builder field
			if [ "$fieldname" = "$BUILDER_HDR" ]
			then
				new="*$new"
			fi

			# if the new value is different from the old one
			# append it to rptline
			if [ "$new" != "$cur" ]
			then
				# disregard case where new='""' and
				# cur="" because the two are really equal
				# since '""' means to leave the field
				# blank
				if [ "$new" != "\"\"" -o "$cur" != "" ]
				then
					rptline="$rptline$new"
				fi
			fi
			# "" means leave field blank
			if [ "$new" = "\"\"" ]
			then
				new=""
			fi

			# append new value to newline
			newline="$newline$new"
		else
			# if the default builder name was taken then append
			# it to rptline now; normally the field is left blank
			# to indicate that the default was taken, but since
			# the builder name field is the only field whose 
			# default may be different from its current value,
			# an exception must be made here
			if [ "$fieldname" = "$BUILDER_HDR" ]
			then
				rptline="$rptline$cur"
			fi
			newline="$newline$cur"
		fi
		if [ count -le $# ]
		then
			newline="$newline|"
			rptline="$rptline|"
		fi
	done
	SetNewFieldVals "$newline"
}

############################################################################
# function:	RepeatMod
# description:	Repeats the last modifications made by the ModifyLine 
#		function; these modifications were recorded in the global
#		variable $rptline by the ModifyLine routine
# input:	1 line of raw data from the status file of the form:
#
#		field1|field2|field3|...
#
#		Field header strings for each field of any given line in 
#		the status file
# output:	none
############################################################################
function RepeatMod {
	typeset line=$1; shift
	typeset -i count
	typeset cur new

	# null out newline
	newline=""

	# creating and/or zeroing scratch file
	count=1
	for fieldname in "$@"
	do
		cur=`echo $line | cut -d'|' -f$count`
		new=`echo $rptline | cut -d'|' -f$count`
		count=count+1

		# if a new value was entered, process it
		if [ -n "$new" ]
		then
			if [ "$new" = "\"\"" ]
			then
				new=""
			fi
			cur="$new"
		fi
		
		newline="$newline$cur"
		if [ count -le $# ]
		then
			newline="$newline|"
		fi
	done
	SetNewFieldVals "$newline"
}

############################################################################
# function:	AcceptMod
# description:	prompts the user to accept or reject a modification made
#		to a line from the status file
# input:	"a" or "r" from the user, for accept or reject
# output:	prompts user to accept or reject modification; return 0
#		if user accepts mod, 1 if user rejects it
############################################################################
function AcceptMod {
	typeset choice=""

	PrintLine "$newline"
	while [[ "$choice" != "a" && "$choice" != "r" ]]
	do
        	print -n -u2 "a)ccept r)eject: "
        	read choice < /dev/tty
	done
	
        if [ $choice = "a" ]
        then
    	    return 0
	else
	    return 1
        fi
}

############################################################################
# function:	GetCurFieldVals
# description:	Parses line of raw data to read from status file into 
#		respective field values.  Each field value is stored in 
#		a string variable called Fn_CUR (where n is an integer from 
#		1 to # of fields in line) which will be used by the
#		DeleteStatus script to actually remove the entry from the 
#		status file
# input:	1 line of raw data of the form:
#
#		field1|field2|field3|...
#
# output:	initialized Fn_NEW variables
############################################################################
function GetCurFieldVals {
    IFS="|"; echo "$1" | read F1_CUR F2_CUR F3_CUR F4_CUR F5_CUR \
    			      F6_CUR F7_CUR F8_CUR F9_CUR F10_CUR
    IFS=" "
}

############################################################################
# function:	SetNewFieldVals
# description:	Parses line of raw data to be written to status file into 
#		respective field values.  Each field value is stored in 
#		a string variable called Fn_NEW (where n is an integer from 
#		1 to # of fields in line) which will be used by the SetStatus
#		script to actually update the status file
# input:	1 line of raw data of the form:
#
#		field1|field2|field3|...
#
# output:	initialized Fn_NEW variables
############################################################################
function SetNewFieldVals {
    IFS="|"; echo "$1" | read F1_NEW F2_NEW F3_NEW F4_NEW F5_NEW \
    			      F6_NEW F7_NEW F8_NEW F9_NEW F10_NEW
    IFS=" "
}

############################################################################
# function:	ModDelRep
# description:	Prompts user for search string; searches status file for
#		entries that match string(s) given in search string.  For
#		each match, the user is asked if he/she wishes to Modify
#		the entry, delete the entry, repeat the last modification
#		on the entry, or quit.  
# input:	user responses 
# output:	displays any matching line; prompts user to modify,delete,
#		repeat mod, or quit
############################################################################
function ModDelRep {
	typeset searchstr	# local search string(s)
	typeset line		# local string for holding line of data from
				# status file
	typeset choice 		# local variable getting user input
	typeset -i delete quit	# local flags

	Search
	if [ "$?" != "0" ]
	then
		return 1
	fi

	# don't want to quit before we start
	quit=0

	# set up format string
	formatstr="$ALL_FMT"

	# print out header
	PrintHeader "DUPLICATE/CHANGE/PURGE:" "$ALL_HDR"

	while read line
	do
		# if we still don't want to quit yet, then continue
		if [ quit -eq 0 ]
		then
		    reject=1
		    while [ "$reject" = "1" ]
		    do
			# get current field values
			GetCurFieldVals "$line"

			# print out formatted line
			PrintLine "$line"

			# prompt user with choices
			print -n -u2 "d)uplicate c)hange p)urge r)epeat q)uit: "

			# get response 
			read choice < /dev/tty

			# perform selected task
			case $choice in
			"d")
				# modifying line
			        ModifyLine "$line" "$TYPE_HDR" "$BLDCYCLE_HDR" \
			          "$RELEASE_HDR" "$LEVEL_HDR" "$BUILDER_HDR" \
			          "$DATE_HDR" "$MISC1_HDR" "$MISC2_HDR" \
			          "$SUBTYPE_HDR" "$STATUS_HDR"

				# give user opportunity to accept/reject changes
				AcceptMod

				# if changes accepted then set line to $newline
				if [ "$?" = "0" ]
				then
				    SetStatus 	 -a "$F1_NEW" \
						 -b "$F2_NEW" \
						 -c "$F3_NEW" \
						 -d "$F4_NEW" \
						 -e "$F5_NEW" \
						 -f "$F6_NEW" \
						 -g "$F7_NEW" \
						 -h "$F8_NEW" \
						 -i "$F9_NEW" \
						 -j "$F10_NEW"
				    reject=0
				fi;;
			"c")
				# modifying line
			        ModifyLine "$line" "$TYPE_HDR" "$BLDCYCLE_HDR" \
			          "$RELEASE_HDR" "$LEVEL_HDR" "$BUILDER_HDR" \
			          "$DATE_HDR" "$MISC1_HDR" "$MISC2_HDR" \
			          "$SUBTYPE_HDR" "$STATUS_HDR"

				# give user opportunity to accept/reject changes
				AcceptMod

				# if changes accepted then set line to $newline
				if [ "$?" = "0" ]
				then
				    DeleteStatus -a "$F1_CUR" \
						 -b "$F2_CUR" \
						 -c "$F3_CUR" \
						 -d "$F4_CUR" \
						 -e "$F5_CUR" \
						 -f "$F6_CUR" \
						 -g "$F7_CUR" \
						 -h "$F8_CUR" \
						 -i "$F9_CUR" \
						 -j "$F10_CUR"
				    SetStatus 	 -a "$F1_NEW" \
						 -b "$F2_NEW" \
						 -c "$F3_NEW" \
						 -d "$F4_NEW" \
						 -e "$F5_NEW" \
						 -f "$F6_NEW" \
						 -g "$F7_NEW" \
						 -h "$F8_NEW" \
						 -i "$F9_NEW" \
						 -j "$F10_NEW"
				    reject=0
				fi;;
			"p")
				    # delete entry if delete option selected
				    DeleteStatus -a "$F1_CUR" \
						 -b "$F2_CUR" \
						 -c "$F3_CUR" \
						 -d "$F4_CUR" \
						 -e "$F5_CUR" \
						 -f "$F6_CUR" \
						 -g "$F7_CUR" \
						 -h "$F8_CUR" \
						 -i "$F9_CUR" \
						 -j "$F10_CUR"
				    reject=0;;
			"r")
				# repeat modifications if repeat option selected
			        RepeatMod "$line" "$TYPE_HDR" "$BLDCYCLE_HDR" \
			          "$RELEASE_HDR" "$LEVEL_HDR" "$BUILDER_HDR" \
			          "$DATE_HDR" "$MISC1_HDR" "$MISC2_HDR" \
			          "$SUBTYPE_HDR" "$STATUS_HDR"
				# give user opportunity to accept/reject changes
				AcceptMod
				# if changes accepted then delete old line and
				# add new line
				if [ "$?" = "0" ]
				then
				    DeleteStatus -a "$F1_CUR" \
						 -b "$F2_CUR" \
						 -c "$F3_CUR" \
						 -d "$F4_CUR" \
						 -e "$F5_CUR" \
						 -f "$F6_CUR" \
						 -g "$F7_CUR" \
						 -h "$F8_CUR" \
						 -i "$F9_CUR" \
						 -j "$F10_CUR"
				    SetStatus 	 -a "$F1_NEW" \
						 -b "$F2_NEW" \
						 -c "$F3_NEW" \
						 -d "$F4_NEW" \
						 -e "$F5_NEW" \
						 -f "$F6_NEW" \
						 -g "$F7_NEW" \
						 -h "$F8_NEW" \
						 -i "$F9_NEW" \
						 -j "$F10_NEW"
				    reject=0
				fi;;
			    "q")
				reject=0
				# set quit flag if quit option selected
			        quit=1;;
			    "")
				reject=0;;
			    esac
		    done
		fi
	done < $TEMP
}

############################################################################
# function:	showall
# description:	generates a formatted report showing values of all fields
# 		of each entry in the status file for a given build cycle
#		(or all build cycles if BLDCYCLE is not set)
# input:	unformatted entries from the status file of the form
#
#		field1|field2|field3...
#
# output:	formatted report containing field data from all fields
############################################################################
function showall { 
	# if BLDCYCLE specified, execute QueryStatus then pipe into sed
	# in order to get only the entries pertaining to the selected build
	# cycle;  otherwise just do a straight QueryStatus which will cause
	# all entries to be displayed no matter what build cycle
	if [ ! -z "$BLDCYCLE" ]
	then
		QueryStatus $_BLDCYCLE $BLDCYCLE -A > $TEMP
	else
		QueryStatus -A > $TEMP
	fi

	# init formatstr; this will be used by Format to format the report
	formatstr="$ALL_FMT"

	# print out formatted report
	Format "ALL STATUS:" $TEMP "$ALL_HDR"
}

############################################################################
# function:	showv3bldstatus
# description:	prints out a formatted report of all entries in the status
#		file with the type field = v3bld.  If BLDCYCLE set only
#		prints out entries corresponding to that build cycle.
# input:	unformatted entries from the status file of the form
#
#		field1|field2|field3...
#
# output:	formatted report containing field data form v3bld entries
############################################################################
function showv3bldstatus {
	# query status file and select required fields
	# if BLDCYCLE set then filter out any fields not pertaining to
	# the specified build cycle
	if [ ! -z "$BLDCYCLE" ]
	then
		QueryStatus $_TYPE $T_BUILD $_BLDCYCLE $BLDCYCLE -A > \
			$SCRATCH_FILE
	else
		QueryStatus $_TYPE $T_T_BUILD -A > $SCRATCH_FILE
	fi

	awk -F"|" ' BEGIN { OFS="|" }
 	{
		print $2,$3,$4,$5,$6,$10
	} ' $SCRATCH_FILE > $TEMP

	# init formatstr; this will be used by Format to format the report
	formatstr="$BUILD_FMT"

	# print out formatted report
	Format "BUILD STATUS:" "$TEMP" "$BUILD_HDR"
}
			
############################################################################
# function:	showprebuildstatus
# description:	prints out a formatted report of all entries in the status
#		file with the type field = prebuild.  If BLDCYCLE set only
#		prints out entries corresponding to that build cycle.
# input:	unformatted entries from the status file of the form
#
#		field1|field2|field3...
#
# output:	formatted report containing field data from prebuild entries
############################################################################
function showprebuildstatus {
	# if BLDCYCLE specified, execute QueryStatus then pipe into sed
	# in order to get only the entries pertaining to the selected build
	# cycle;  otherwise just do a straight QueryStatus which will cause
	# all entries to be displayed no matter what build cycle
	if [ ! -z "$BLDCYCLE" ]
	then
		QueryStatus $_TYPE $T_PREBUILD $_BLDCYCLE $BLDCYCLE -A > \
			$SCRATCH_FILE
	else
		QueryStatus $_TYPE $T_PREBUILD -A > $SCRATCH_FILE
	fi

	awk -F"|" ' BEGIN { OFS="|" }
	{ 
		print $2,$3,$4,$5,$6,$9,$10 
	} ' $SCRATCH_FILE > $TEMP

	# init formatstr; this will be used by Format to format the report
	formatstr="$PREBUILD_FMT"

	# print out formatted report
	Format "PRODMERGE STATUS:" $TEMP "$PREBUILD_HDR"
}

############################################################################
# function:	showbldretainstatus
# description:	prints out a formatted report of all entries in the status
#		file with the type field = bldretain.  If BLDCYCLE set only
#		prints out entries corresponding to that build cycle.
# input:	unformatted entries from the status file of the form
#
#		field1|field2|field3...
#
# output:	formatted report containing field data from prebuild entries
############################################################################
function showbldretainstatus {
	# query status file and filter out only required fields
	if [ ! -z "$BLDCYCLE" ]
	then
		QueryStatus $_TYPE $T_BLDRETAIN $_BLDCYCLE $BLDCYCLE -A > \
			$SCRATCH_FILE
	else
		QueryStatus $_TYPE $T_BLDRETAIN -A > $SCRATCH_FILE
	fi

	awk -F"|" ' BEGIN { OFS="|" }
 	{
		print $2,$3,$4,$5,$6,$10
	} ' $SCRATCH_FILE > $TEMP

	# init formatstr; this will be used by Format to format the report
	formatstr="$BLDRETAIN_FMT"

	# print out formatted report
	Format "BLDRETAIN STATUS:" $TEMP "$BLDRETAIN_HDR"
}

############################################################################
# function:	showgenptfstatus
# description:	prints out a formatted report of all entries in the status
#		file with the type field = cumptf.  If BLDCYCLE set only
#		prints out entries corresponding to that build cycle.
# input:	unformatted entries from the status file of the form
#
#		field1|field2|field3...
#
# output:	formatted report containing field data from cumptf entries
############################################################################
function showgenptfstatus {
	# if BLDCYCLE specified, execute QueryStatus then pipe into sed
	# in order to get only the entries pertaining to the selected build
	# cycle;  otherwise just do a straight QueryStatus which will cause
	# all entries to be displayed no matter what build cycle
	if [ ! -z "$BLDCYCLE" ]
	then
		QueryStatus $_TYPE $T_GENPTF $_BLDCYCLE $BLDCYCLE -A > \
			$SCRATCH_FILE
	else
		QueryStatus $_TYPE $T_GENPTF -A > $SCRATCH_FILE
	fi

	awk -F"|" ' BEGIN { OFS="|" } 
	{
		print $2,$5,$6,$7,$10
	} ' $SCRATCH_FILE > $TEMP

	# init formatstr; this will be used by Format to format the report
	formatstr="$GENPTF_FMT"

	# print out formatted report
	Format "GENPTF STATUS:" $TEMP "$GENPTF_HDR"
}

############################################################################
# function:	showbldptfstatus
# description:	prints out a formatted report of all entries in the status
#		file with the type field = subptf.  If BLDCYCLE set only
#		prints out entries corresponding to that build cycle.
# input:	unformatted entries from the status file of the form
#
#		field1|field2|field3...
#
# output:	formatted report containing field data from bldptf entries
############################################################################
function showbldptfstatus {
	# Query for subptf requires knowledge of the order that fields
	# appear in status file; if the order ever changes, then the code
	# below will have to change as well
	if [ ! -z "$BLDCYCLE" ]
	then
		QueryStatus $_TYPE $T_BLDPTF $_BLDCYCLE $BLDCYCLE -A > \
			$SCRATCH_FILE
	else
		QueryStatus $_TYPE $T_BLDPTF -A > $SCRATCH_FILE
	fi

	awk -F"|" ' BEGIN { OFS="|" }
	{ 
		print $2,$3,$5,$6,$7,$9,$10 
	} ' $SCRATCH_FILE > $TEMP

	# init formatstr; this will be used by Format to format the report
	formatstr="$BLDPTF_FMT"

	# print out formatted report
	Format "BLDPTF STATUS:" $TEMP "$BLDPTF_HDR"

	rm -f $SCRATCH_FILE
}

############################################################################
# function:	showbldstatus
# description:	prints out a formatted report of all entries in the status
#		file with the type field = bldstatus.  If BLDCYCLE set then
#		it only prints out entries corresponding to that build cycle.
# input:	unformatted entries from the status file of the form
#
#		field1|field2|field3...
#
# output:	formatted report containing field data from subptf entries
############################################################################
function showbldstatus {
	# Query for bldstatus requires knowledge of the order that fields
	# appear in status file; if the order ever changes, then the code
	# below will have to change as well
	if [ ! -z "$BLDCYCLE" ]
	then
		QueryStatus $_TYPE $T_BLDSTATUS $_BLDCYCLE $BLDCYCLE -A > \
			$SCRATCH_FILE
	else
		QueryStatus $_TYPE $T_BLDSTATUS -A > $SCRATCH_FILE
	fi

	awk -F"|" ' BEGIN { OFS="|" }
	{ 
		print $2,$7,$8,$9,$10 
	} ' $SCRATCH_FILE > $TEMP

	# init formatstr; this will be used by Format to format the report
	formatstr="$BLDSTATUS_FMT"

	# print out formatted report
	Format "BLDSTATUS STATUS:" $TEMP "$BLDSTATUS_HDR"

	rm -f $SCRATCH_FILE
}

############################################################################
# function:	getstatusfile
# description:	Called when -f <status_file> option is used from the command
#		line.  Checks to see if the status file name given actually
#		exists.  If so then that file is used for all status file
#		routines.
# input:	status file name found in $OPTARG
# output:	error message if status file not found.
############################################################################
function getstatusfile {
	if [ ! -f "$OPTARG" ]
	then
		print -u2 "Status file: $OPTARG not found."
		clean_up 1
	else
		export STATUS_FILE="$OPTARG"
	fi
}

############################################################################
# function:	getbldcycle
# description:	prompts user to enter build cycle
# input:	build cycle name found in $OPTARG
# output:	none.
############################################################################
function getbldcycle {
	print -u2 "Enter the Build Cycle: \c"
	read BLDCYCLE < /dev/tty
	print -u2 "BLDCYCLE = $BLDCYCLE"
	print -u2 "  Is this correct? (y/n): \c"
	read answer < /dev/tty
	if [[ $answer = "y" && -n "$BLDCYCLE" ]]
	then
		export BLDCYCLE
	else
		BLDCYCLE=""
	fi
}

############################################################################
# function:	getsearchstr
# description:	Called when -q <string> option is used from the command
#		line.  Initializes searchstr to given string for later use 
#		in the SearchStatus routine.
# input:	search string found in $OPTARG
# output:	none.
############################################################################
function getsearchstr {
	searchstr="$OPTARG"
}

############################################################################
# function:	prompt_bldcycle
# description:	Called before invoking the showsummary routine.  Prompts 
#		user for BLDCYCLE until one is entered since BLDCYCLE is
#		required for showsummary. 
# input:	build cycle 
# output:	prompt
############################################################################
function prompt_bldcycle {
	if [ ! -n "$BLDCYCLE" ]
	then
		tput clear > /dev/tty
	fi
	while [ ! -n "$BLDCYCLE" ]
	do
		print -u2 "Build Cycle required for showsummary."
		getbldcycle
		tput clear > /dev/tty
	done
}

############################################################################
# function:	print_usage
# description:	Prints usage statement when showstatus command invoked 
#		improperly.
# input:	none.
# output:	Usage statement
############################################################################
function print_usage {
	print -u2 "usage: showstatus [-a] [-b <bldcycle>] [-f <status_file>] [-g] [-l] [-p]"
	print -u2 "                  [-q <string>] [-s] [-t] [-v]" 
	print -u2 "\twhere"
	print -u2 "\t\t-a shows all status"
	print -u2 "\t\t-b <bldcycle> selects bldcycle"
	print -u2 "\t\t-f <status_file> selects status file"
	print -u2 "\t\t-g shows cumptf status"
	print -u2 "\t\t-l shows bldstatus status"
	print -u2 "\t\t-p shows prebuild merge status"
	print -u2 "\t\t-q <string> searches status file"
	print -u2 "\t\t   for entries containing given string"
	print -u2 "\t\t-r shows bldretain status"
	print -u2 "\t\t-s shows summary information"
	print -u2 "\t\t-t shows subptf status"
	print -u2 "\t\t-v shows v3bld status"

	clean_up 1
}

############################################################################
# function:	cleanup
# description:	cleans up temporary files before exitting
# input:	exit status of 0 or 1 passed to routing by calling routine
# output:	exits with exit status that was passed to it
############################################################################
function clean_up {
	if [ -f "$TEMP" ]          ; then rm -f $TEMP          ; fi
	if [ -f "$SCRATCH_FILE" ]  ; then rm -f $SCRATCH_FILE  ; fi

	exit $1
}

############################### M A I N ##############################

# call cleanup on interrupts
trap "clean_up 1; exit" HUP INT QUIT TERM

# process command line options
while getopts "ab:f:glpq:rstv" option
do 	case $option in
		a) 	optionset=1; aflag=1;; 
		b) 	BLDCYCLE="$OPTARG"; export BLDCYCLE;;
		f) 	getstatusfile;;
		g) 	optionset=1; gflag=1;;
		l) 	optionset=1; lflag=1;;
		p) 	optionset=1; pflag=1;;
		q)	optionset=1; qflag=1; getsearchstr;;
		s)	optionset=1; sflag=1;;
		r)	optionset=1; rflag=1;;
		t)	optionset=1; tflag=1;;
		v)	optionset=1; vflag=1;;
		\?)	optionset=1; print_usage;;
	esac
done

bldinit		# initialize build environment

TMPDIR=$(bldtmppath)
TEMP=$TMPDIR/temp.data$$		# temp file for output from QueryStatus
SCRATCH_FILE=$TMPDIR/scratch.data$$	# scratch file

# field headers

TYPE_HDR="Type"		# type field header
BLDCYCLE_HDR="Bcycl"	# bldcycle field header
RELEASE_HDR="Release"	# release field header
LEVEL_HDR="Level"	# build level field header
BUILDER_HDR="Builder"	# builder field header
DATE_HDR="Date"		# date field header
MISC1_HDR="Misc 1"	# misc1 field header
MISC2_HDR="Misc 2"	# misc2 field header
LPP_HDR="LPP"		# lpp field header
SUBTYPE_HDR="Subtype"	# subtype field header
MODE_HDR="Mode"		# mode field header
STATUS_HDR="Status"	# status field header
FPHASE_HDR="From Phase"	# status field header
TPHASE_HDR="To Phase"	# status field header
FORCEFLG_HDR="Force"	# status field header

# field formats that get passed to awk printf 

TYPE_FMT="%-9.9s"	# type field format
BLDCYCLE_FMT="%-5.5s"	# bldcycle field format
RELEASE_FMT="%-13.13s"	# release field format
LEVEL_FMT="%-8.8s"	# build level field format
BUILDER_FMT="%-10.10s"	# builder field format
DATE_FMT="%-12.12s"	# date field format
MISC1_FMT="%-11.11s"	# misc1 field format
MISC2_FMT="%-7.7s"	# misc2 field format
LPP_FMT="%-19.19s"	# lpp field format
SUBTYPE_FMT="%-15.15s"	# subtype field format
MODE_FMT="%-9.9s"	# mode field format
STATUS_FMT="%-9.9s"	# status field format
FPHASE_FMT="%-10.10s"	# status field header
TPHASE_FMT="%-10.10s"	# status field header
FORCEFLG_FMT="%-6.6s"	# status field header

ALL_FMT=\
"$TYPE_FMT $BLDCYCLE_FMT $RELEASE_FMT $LEVEL_FMT $BUILDER_FMT $DATE_FMT\
 $MISC1_FMT $MISC2_FMT $SUBTYPE_FMT $STATUS_FMT\n"
BUILD_FMT=\
"$BLDCYCLE_FMT $RELEASE_FMT $LEVEL_FMT $BUILDER_FMT $DATE_FMT $MODE_FMT\n"
PREBUILD_FMT=\
"$BLDCYCLE_FMT $RELEASE_FMT $LEVEL_FMT $BUILDER_FMT $DATE_FMT $SUBTYPE_FMT\
 $STATUS_FMT\n"
BLDRETAIN_FMT=\
"$BLDCYCLE_FMT $RELEASE_FMT $LEVEL_FMT $BUILDER_FMT $DATE_FMT $STATUS_FMT\n"
GENPTF_FMT=\
"$BLDCYCLE_FMT $BUILDER_FMT $DATE_FMT $LPP_FMT $STATUS_FMT\n"
BLDPTF_FMT=\
"$BLDCYCLE_FMT $RELEASE_FMT $BUILDER_FMT $DATE_FMT $MISC1_FMT $SUBTYPE_FMT \
$STATUS_FMT\n" 
BLDSTATUS_FMT=\
"$BLDCYCLE_FMT $FPHASE_FMT $TPHASE_FMT $FORCEFLG_FMT $STATUS_FMT\n" 

ALL_HDR=\
"$TYPE_HDR|$BLDCYCLE_HDR|$RELEASE_HDR|$LEVEL_HDR|$BUILDER_HDR|\
$DATE_HDR|$MISC1_HDR|$MISC2_HDR|$SUBTYPE_HDR|$STATUS_HDR"
BUILD_HDR=\
"$BLDCYCLE_HDR|$RELEASE_HDR|$LEVEL_HDR|$BUILDER_HDR|$DATE_HDR|$MODE_HDR"
PREBUILD_HDR=\
"$BLDCYCLE_HDR|$RELEASE_HDR|$LEVEL_HDR|$BUILDER_HDR|$DATE_HDR|$SUBTYPE_HDR|\
$STATUS_HDR"
BLDRETAIN_HDR=\
"$BLDCYCLE_HDR|$RELEASE_HDR|$LEVEL_HDR|$BUILDER_HDR|$DATE_HDR|$STATUS_HDR"
GENPTF_HDR=\
"$BLDCYCLE_HDR|$BUILDER_HDR|$DATE_HDR|$LPP_HDR|$STATUS_HDR"
BLDPTF_HDR=\
"$BLDCYCLE_HDR|$RELEASE_HDR|$BUILDER_HDR|$DATE_HDR|$MISC1_HDR|$SUBTYPE_HDR|\
$STATUS_HDR" 
BLDSTATUS_HDR=\
"$BLDCYCLE_HDR|$FPHASE_HDR|$TPHASE_HDR|$FORCEFLG_HDR|$STATUS_HDR"

BAR="----------------------------------------"
BAR_HDR="$BAR|$BAR|$BAR|$BAR|$BAR|$BAR|$BAR|$BAR|$BAR|$BAR"

# unset BLDCYCLE if it exists but is null
if [[ -z "$BLDCYCLE" ]]
then
	unset BLDCYCLE
fi

# process flags set by command line options

if [[ ! -z "$aflag" ]]
then
	showall
	clean_up 0
fi

if [[ ! -z "$gflag" ]]
then
	showgenptfstatus
	clean_up 0
fi

if [[ ! -z "$lflag" ]]
then
	showbldstatus
	clean_up 0
fi

if [[ ! -z "$pflag" ]]
then
	showprebuildstatus
	clean_up 0
fi

if [[ ! -z "$qflag" ]] 
then 
	SearchStatus
	clean_up 0
fi

if [[ ! -z "$sflag" ]]
then
	prompt_bldcycle
	showsummary $STATUS_FILE
	clean_up 0
fi

if [[ ! -z "$rflag" ]]
then
	showbldretainstatus
	clean_up 0
fi

if [[ ! -z "$tflag" ]]
then
	showbldptfstatus
	clean_up 0
fi

if [[ ! -z "$vflag" ]]
then
	showv3bldstatus
	clean_up 0
fi

# if there were no command line options set then go into menu mode
if [[ -z "$optionset" ]]  ## if no options then display menu
then
	# printing out menu and processing choices
	quit=0
	while [[ $quit != "1" ]]
	do 	
	        tput clear > /dev/tty
		print "SHOWSTATUS\n"
		print "	 1) summary" > /dev/tty
		print "	 2) prebuild" > /dev/tty
		print "	 3) bldretain" > /dev/tty
		print "	 4) v3bld" > /dev/tty
		print "	 5) subptf" > /dev/tty
		print "	 6) cumptf" > /dev/tty
		print "	 7) bldstatus" > /dev/tty
		print "	 8) all" > /dev/tty
		print "	 9) search" > /dev/tty
		print "	10) duplicate/change/purge" > /dev/tty
		print "	11) change build cycle ($BLDCYCLE)" > /dev/tty
		print "	12) change status file ($STATUS_FILE)" > /dev/tty
		print "	13) exit\n" > /dev/tty
		print "choice: \c" > /dev/tty; read choice < /dev/tty
	 	case $choice in
		1|"summary")
	        	tput clear > /dev/tty
			prompt_bldcycle
			tput clear > /dev/tty
			showsummary $STATUS_FILE | pg;;
		2|"prebuild")
	        	tput clear > /dev/tty
			showprebuildstatus | pg;;
		3|"bldretain")
	        	tput clear > /dev/tty
			showbldretainstatus | pg;;
		4|"v3bld")
	        	tput clear > /dev/tty
			showv3bldstatus | pg;;
		5|"subptf")
	        	tput clear > /dev/tty
			showbldptfstatus | pg;;
		6|"cumptf")
	        	tput clear > /dev/tty
			showgenptfstatus | pg;;
		7|"bldstatus")
	        	tput clear > /dev/tty
			showbldstatus | pg;;
		8|"all")
	        	tput clear > /dev/tty
			showall | pg;;
		9|"search")
	        	tput clear > /dev/tty
			searchstr=""
			SearchStatus | pg;;
		10|"modify/delete")
	        	tput clear > /dev/tty
			searchstr=""
			ModDelRep;;
		11|"change build cycle")
	        	tput clear > /dev/tty
			getbldcycle;;
		12|"change status file")
	        	tput clear > /dev/tty
			unset STATUS_FILE
			chksetstatus;;
		13|"exit"|"x"|"quit"|"q")
			quit=1;;
		esac
	done # while
fi

clean_up 0

