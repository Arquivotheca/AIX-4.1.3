#! /bin/sh
# @(#)56	1.9  src/bos/usr/lpp/bosinst/scripts/scrip555/getdb.sh, bosinst, bos411, 9428A410j 10/21/91 12:24:51
#
# COMPONENT_NAME: (BOSINST) Base Operating System Installation
#
# FUNCTIONS: getdb
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1989
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

#
# calls:	dbextract
#

if [ "$Clients_Header_Version" != "2.0" ]
then
	fullpath=`dirname $0`
	$fullpath/getdb.old $*
	exit $?
fi

. `dirname $0`/defaults		# load defaults

if test $# -gt 1 		# test arguments
then
	echo "usage: $0 [client_IP_address]" 1>&2
	exit 1
elif test $# -eq 1
then
	cIPaddr=$1
	export cIPaddr
fi

IPfilename=`echo $1 |
	awk -F. '
		{
			printf("%03d%03d%03d%03d\n",$1,$2,$3,$4)
		}
	'
`

	#
	# set trap to clean up work files when procedure exits
	#
trap 'rm -rf /tmp/netinstall/$IPfilename/choicelist 2>/dev/null; trap 0; exit' 0 1 2 3 15

umask 077
mkdir /tmp/netinstall 2>/dev/null		# make working directory, or-
mkdir /tmp/netinstall/$IPfilename 2>/dev/null	# make working directory, or-
rm -rf /tmp/netinstall/$IPfilename/* 2>/dev/null # clean working directory

exec > /tmp/netinstall/$IPfilename/clientdb

fnames=`cd $DB; ls $IPfilename cl.* 2> /dev/null` #get clean list of class files
for fn in $fnames	# guaranteed that fnames contains existing file names
do
	func="`dbextract $fn`"	# get query script from db file $fn
	eval "getdata(){	# load query as shell function "getdata"
	$func			# so we don't have to go to disk
	}"
	name="`getdata classname`"	# extract the classname from $fn
	test X"$name" = X && name=$fn	# if no classname, use filename (fn)
	bos="`getdata bos | cut -d' ' -f1`"	# extract bos name from db
	rest_of_bos="`getdata bos | cut -d' ' -f2-`"
	lpps="`getdata lpp`"		# extract lpp name from db
	if test X"$bos" != X		# did we get a bos name?
	then
		if test -r "$bos"
		then
			echo "classname $name"	# if so, save to temp file
			echo "classentry $bos $rest_of_bos"
			if test X"$lpps" != X
			then
				OIFS="$IFS"
				IFS="
"
				for lppline in $lpps
				do
				    lppname="`echo $lppline | cut -d' ' -f1`"
				    rest_of_lpps="`echo $lppline | cut -d' ' -f2-`"
				    if test -r "$lppname"
				    then
			echo classentry $lppname $rest_of_lpps
				    fi
				done
				IFS="$OIFS"
			fi
		fi
	fi
done > /tmp/netinstall/$IPfilename/choicelist

	#
	# get a clean, expanded, list of filenames from the choices file
	#
echo `cat $DB/choices` |
xargs ls -Lld 2> /dev/null |
while read fmode garb garb garb fsize garb garb garb fn garb
do
	case $fmode in
	-* | F* | l* | L*)	;;
	*)	continue;;
	esac

	if test ! -r $fn; then continue; fi

	case $fn in
	*bos.obj*) fdescr=bos;;
	*) fdescr=lpp;;
	esac

	echo $fdescr $fn file $fsize	# to the tempfile

done >> /tmp/netinstall/$IPfilename/choicelist


	#
	# output a usage message for the script we are writing
	#
echo 'if test $# -lt 1
then
	exit 1
fi
case "$1" in'

if test ! -s /tmp/netinstall/$IPfilename/choicelist
then
	echo 'display)
	case $2 in
	screen1)
		echo "No installable options on the server";;
	screen1.maxchoice)
		echo 0;;
	maxscreen)
		echo 1;;
	esac
	;;
esac'

	exit 0
fi

	#
	# this awk program reformats the information written to the tempfile
	# above into a shell script (case statement), which is used as a
	# database query program.
	#
	# the information available in the script includes any data out of
	# the client description file, if available, and information
	# used to let the user select the file(s) to be downloaded.  this
	# includes images of the selection screens, the number of screens
	# available, the number of choices per screen, the catagory of each
	# selection (ie, bos, lpp, class), the filename(s) associated with
	# each selection.
	#
	# NOTE: don't use and single quotes (') in comments in the awk
	# program, since the awk program is itself in single quotes.
	#
awk '
	BEGIN {		# initialization
		screen = 1		# which screen are we describing?

		lines_used = 0		# how many display lines have been used
					# 	on this screen?

		choices_used = 0	# how many choices are available on
					#	this screen?

		printf("display)\n")	# the "recname" for the screens
		printf("\tcase $2 in\n")	# nested case for each screen
		printf("\tscreen1)\n")	# start w/ screen 1
	}

		#
		# the variable "quote" isn-t available until after the first
		# input record is read.  so, the first thing we do after
		# reading the first record is to start an echo statement that
		# will eventually echo out the first selection screen to the
		# user.
		#
	NR == 1 {			# processing input record 1?
		printf("\t\techo %s",quote)	# start the echo
	}

		#
		# if we just filled up the last screen, start assigning
		# remaining selections to the next screen.
		#
	lines_used > 13 {	# used over 13 lines on the current screen?
		printf("%s\n",quote)	# end the echo statement
		printf("\t\t;;\n")	# end the case pattern

			#
			# record the number of choices available on the screen
			#
		printf("\tscreen%d.maxchoice)\n",screen)
		printf("\t\techo %d\n", choices_used)
		printf("\t\t;;\n")

		screen++		# initialize the next screen
		printf("\tscreen%d)\n",screen)	# start the pattern
		printf("\t\techo %s",quote)	# start the echo
		lines_used = 0	# haven-t used any lines on this screen yet
		choices_used = 0	# and haven-t used any choices yet
	}

	$1 == "classname" {	# does the current input line start a new class?
		choices_used++	# if so, it will be a choice line
		lines_used++	# all lines tie up space on the screen
		classentry_used=0	# reset number of files in this class

	        save_class_screen = screen	# screen on which class start
	        save_class_choice = choices_used	# choice num for clas

			#
			# remember the catagory of this selection
			#
		selection_type[screen "." choices_used] = "class"

			#
			# output the text of the "echo" statement that we
			# are building.  for example, selection 3 on
			# screen 2 might look like this:
			#
			#	3. $sel2_3  RT working class
			#
			# the variable sel2_3 will either hold a marker, or will
			# be blank, to show that the selection has be chosen,
			# or has not been chosen, respectively.
			#
		printf("%2d. $sel%d_%d\t%s\n", choices_used, screen, choices_used, substr($0,length($1) + 2))

		next	# skip forward to the next input record
	}

		#
		# does this input record describe a file that is part of a
		# class?
		#
	$1 == "classentry" {
		lines_used++		# this ties up a line on the screen,
					#	but it is not a choice
		classentry_used++	# count number of entries in this class
		printf("\t\t%s\n",$2)	# print file name as a comment for user

			#
			# save the file name and file type (file, dir, pipe,
			# char, block) for the class entry.  index the
			# array by screen, choice number, class entry number
			#
choice_name[save_class_screen "." save_class_choice "." classentry_used] = $2
choice_type[save_class_screen "." save_class_choice "." classentry_used] = $3
choice_size[save_class_screen "." save_class_choice "." classentry_used] = $4

		next	# skip to next input record
	}

		#
		# does input record describe a non-class selection?
		#
	$1 == "bos" || $1 == "lpp" {
		choices_used++	# is choosable and uses a line on the screen
		lines_used++

			#
			# again we have the selS_C variable to potentially
			# display a marker
			#
		printf("%2d. $sel%d_%d\t%s\t\n", choices_used, screen, choices_used, $2)

			#
			# save filename type catagory for later inclusion
			# into the client query file
			#
		choice_name[screen "." choices_used "." 1] = $2
		choice_type[screen "." choices_used "." 1] = $3
		choice_size[screen "." choices_used "." 1] = $4
		selection_type[screen "." choices_used] = $1
		next
	}

		#
		# we match this pattern when we hit end of file on our
		# input.  so far, we have only been outputting information
		# about what the screens look like and how many choices per
		# screen.  now, we finish off the last screen, record the
		# number of screens, and dump the information we have been
		# keeping in arrays.
		#
	END {
		if(lines_used != 0)	# this should ALWAYS be true
		{
				#
				# end the echo statement and the case pattern,
				# then output the number of choices on the
				# last screen
			printf("%s\n",quote)
			printf("\t\t;;\n")
			printf("\tscreen%d.maxchoice)\n",screen)
			printf("\t\techo %d\n", choices_used)
			printf("\t\t;;\n")
		}
		printf("\tmaxscreen)\n")	# output the number of screens
		printf("\t\techo %d\n", screen)
		printf("\t\t;;\n")
		printf("\tesac\n")		# end the sub-case statement
		printf("\t;;\n")		# end the "display" pattern

		for(s = 1; s <= screen; s++)	# step through each screen
		{
			if(s > 1)
			{
					#
					# end the preceding case and pattern
					#
				printf("\tesac\n")
				printf("\t;;\n")	
			}
			printf("screen%d)\n",s)		# start the new screen
			printf("\tcase $2 in\n")	# start subcase

				#
				# step through each choice for this screen
				#
			for(c = 1; choice_name[s "." c "." 1] != ""; c++)
			{
					#
					# is choice a class, bos, or lpp?
					#
				printf("\tchoice%d.type)\n",c)
				printf("\t\techo %s\n",selection_type[s "." c])
				printf("\t\t;;\n")

					#
					# what file(s) are associated w/ choice?
					#
				printf("\tchoice%d)\n",c)
				for(e = 1;choice_name[s "." c "." e] != "";e++)
				{
printf("\t\techo %s %s %s\n",choice_name[s "." c "." e],choice_type[s "." c "." e],choice_size[s "." c "." e])
				}
				printf("\t\t;;\n")
			}
		}
		printf("\tesac\n")
		printf("\t;;\n")
	}

' quote='"' /tmp/netinstall/$IPfilename/choicelist

dbextract -p $IPfilename	# get stuff out of client description file
dbextract -p default.client	# get stuff out of default client file

echo esac			# end script

chmod u+x /tmp/netinstall/$IPfilename/clientdb
