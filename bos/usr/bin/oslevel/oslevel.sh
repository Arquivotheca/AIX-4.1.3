#!/bin/ksh
# @(#)40        1.5  src/bos/usr/bin/oslevel/oslevel.sh, cmdswvpd, bos412, 9446A412a 11/14/94 13:09:27
#
#   COMPONENT_NAME: CMDSWVPD
#
#   FUNCTIONS:  usage
#		toomanylevels
#		interrupted
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
# Print usage message if invalid arguments specified
#
usage () {
	/usr/sbin/inuumsg 163 2>&1
	exit 1
}

#
# User specified more than one of -l, -g, -q
#
toomanylevels () {
	/usr/sbin/inuumsg 164 2>&1
	usage
	exit 2
}

#
# User interrupted command
#
interrupted () {
	[ -f $mlinfo ] && /usr/bin/rm $mlinfo
	exit 7
}

#
# Main logic for oslevel
#
LANG=C		# Must have LANG=C because output of lslpp is parsed
basic=1		# Basic form of output requested
downlevel=0	# Products below maintenance level requested
postlevel=0	# Products updated since maintenance level requested
query=0		# Query of known maintenance levels requested
level=		# Maintenance level specified with -l
mlinfo="/tmp/.oslevel.mlinfo$$"
mlcache="/tmp/.oslevel.mlinfo.cache"
usetmpfile=0	# Can we use the saved output from the previous instfix call?
MLTAG="_AIX_ML"
mlfound=0

trap interrupted INT QUIT TERM

# Test for version 4 or greater
if [ $(uname -v) -lt 4 ]
then
	/usr/sbin/inuumsg 165 2>&1
	exit 6
fi

# process valid options, watching for invalid combinations
while [ -n "$1" ]
do
	case $1 in
		-l) downlevel=1
		    basic=0
	 	    level=$2
		    [ -z "$level" ] && usage
		    shift
		    ;;
		-g) postlevel=1
		    basic=0
	 	    level=$2
		    if [ -n "$level" ]
		    then
		       shift
		    fi
		    ;;
		-q) query=1
		    basic=0
		    ;;
		--) break
		    ;;
		* ) usage
		    ;;
	esac
	shift
done

[ $(expr $downlevel + $postlevel + $query) -gt 1 ] && toomanylevels

# Get the maintenance level information from the fix database
# This information is cumulatively refreshed with each maintenance level

# But first check to see if we can reuse old instfix output
if [ -r $mlcache ] && [ -s $mlcache ] && \
   [ $mlcache -nt /usr/lib/objrepos/history ] && \
   [ $mlcache -nt /usr/share/lib/objrepos/history ]
then
      usetmpfile=1
fi

if [ $usetmpfile -gt 0 ]
then
   mlinfo=$mlcache
else
   mlinfo=$mlcache
   umask 111
   /usr/sbin/instfix -qic -t p | /usr/bin/awk -F: '$1 ~ /_AIX_ML$/' >$mlcache 2>/dev/null
fi

if [ ! -s $mlinfo ]
then
	echo `lslpp -qLc bos.rte | cut -d':' -f3 | cut -d'.' -f1-3`.0
	# /usr/sbin/inuumsg 166
	/usr/bin/rm $mlinfo
	exit 0
fi


# If level has been specified, make sure it exists
if [ -n "$level" ] && (cat $mlinfo | cut -d: -f1 | /usr/bin/grep -q "$level$MLTAG$")
then
	:
else
   if [ -n "$level" ]
   then
	/usr/sbin/inuumsg 167 $level 2>&1
	#/usr/bin/rm $mlinfo
	exit 1
   fi
fi

# If query mode, list the available levels, most recent first, then end.
if [ $query -eq 1 ]
then
	/usr/sbin/inuumsg 168
	cat $mlinfo | cut -d':' -f1 | sed 's:_AIX_ML::' | sort -n -r -t . | uniq
        #/usr/bin/rm $mlinfo
	exit 0
fi

# Determine most recently complete maintenance level 
if [ -z "$level" ]
then
	# Get list of maintenance levels, most recent first
	cat $mlinfo | cut -d':' -f1 | sort -n -r -t . | uniq |
	# Check instfix output to see if any have no downlevel filesets
	while read ml
	do
	   cat $mlinfo | /bin/grep "^$ml" | /bin/grep -q ":-:"
	   if [ $? -eq 1 ]
	   then
	      level=`echo $ml | sed "s:$MLTAG::"`
	      # If we're just checking for the most recent complete ML
	      if [ $basic -eq 1 ]
	      then
		 echo $level
		 exit 0
	      fi
	      mlfound=1
	      break
	   fi
	done
	# If no maintenance levels exist, then we have to punt....
	if [ $mlfound -eq 0 ]
	then
	   baselevel=`lslpp -qLc bos.rte | cut -d':' -f3 | cut -d'.' -f1-2`.0.0
	   if [ $basic -eq 1 ]
	   then
	      echo $baselevel
	      exit 0
	   elif [ $postlevel -eq 1 ]
	   then
	      /usr/bin/lslpp -qLc bos.\* printers.\* devices.\* X11.\* 2>/dev/null |
              /usr/bin/awk -F: -v header=0 -v oslevel="$baselevel" \
	          '$3 > oslevel {
           	   if ( header == 0 ) 
           	   { 
	       	       system("/usr/sbin/inuumsg 170"); 
	               header = 1 
                   }
                   printf("%-40s%-20s%s\n",$2,$3,oslevel) 
              }'

	      exit 0
	   fi
	fi
fi

if [ $postlevel -eq 1 ]
then
   diffsym="+"
else
   diffsym="-"
fi

cat $mlinfo | /bin/grep "^$level$MLTAG:" |
   /usr/bin/awk -F: -v header=0 -v diffsymbol="$diffsym" '$5 == diffsymbol {
           if ( header == 0 ) 
           { 
	       system("/usr/sbin/inuumsg 170"); 
	       header = 1 
           }
           printf("%-40s%-20s%s\n",$2,$4,$3) 
       }'

# Goodbye
#/usr/bin/rm $mlinfo 
exit 0
