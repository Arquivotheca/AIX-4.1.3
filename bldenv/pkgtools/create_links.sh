#!/bin/bsh
# @(#)87	1.3  src/bldenv/pkgtools/create_links.sh, pkgtools, bos412, GOLDA411a 6/18/93 10:45:13
#
#   COMPONENT_NAME: PKGTOOLS
#
#   FUNCTIONS:  usage
#		add_header
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1992,1993
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

# NAME: create_links
#
# FUNCTION: This script will process an inslist and produce a script that
#		will add any links to a system upon installation.
#
# EXECUTION ENVIRONMENT:
#
#	This script should only be executed in the build environment on
#	inslists known to have mis-matched symbolic and hard links.
#
# PURPOSE :
#
#	This script was written due to the fact that there is currently
#	no way to ship only a link with an image.  Until there are 
#	changes made to both the CMDINSTL code and build tools this 
#	script will continue to be needed.
#
# ARGUMENTS :
#	-c	The name of the config script that should be created
#	-i	The name of the inslist file to be used
#	-u	The name of the unconfig script that should be created
# 
# ERROR RETURN CODES :
#	1	Usage error
#	2	Invalid/unreadable argument
#
# -------------------------------------------------------------------------

# -------------------------------------------------------------------------
# usage
# -------------------------------------------------------------------------
usage()
{
$echo "Usage : create_links -c <config script> -u <unconfig script> -i <inslist>"
}

# -------------------------------------------------------------------------
# add_header
# -------------------------------------------------------------------------
add_header()
{
FILE=$1

$echo "#!/bin/bsh" > $FILE
$echo "#" >> $FILE
$echo "# COMPONENT_NAME: pkg">> $FILE
$echo "#" >> $FILE
$echo "# FUNCTIONS: ">> $FILE
$echo "#" >> $FILE
$echo "# ORIGINS: 27" >> $FILE
$echo "#" >> $FILE
$echo "# (C) COPYRIGHT International Business Machines Corp. 1993" >> $FILE
$echo "# All Rights Reserved" >> $FILE
$echo "# Licensed Materials - Property of IBM" >> $FILE
$echo "#" >> $FILE
$echo "# US Government Users Restricted Rights - Use, duplication or" >> $FILE
$echo "# disclosure restricted by GSA ADP Schedule Contract with IBM Corp." >> $FILE
$echo "#" >> $FILE
$echo "# NAME: $FILE" >> $FILE
$echo "#" >> $FILE
$echo >> $FILE

}

# -------------------------------------------------------------------------
# MAIN Routine
# -------------------------------------------------------------------------

# Set up ODE_TOOLS
echo=$ODE_TOOLS/usr/bin/echo
getopt=$ODE_TOOLS/usr/bin/getopt
grep=$ODE_TOOLS/bin/grep
cat=$ODE_TOOLS/usr/bin/cat

# Initialize variables
WORKING_INSLIST=/tmp/working_inslist
unset FILE
unset CONFIG
unset INSLIST
unset UNCONFIG

[ $# -eq 0 ] && { usage; exit 1; }

# Process the line arguments
set -- `$getopt c:i:u: $*`
if [ $? != 0 ]
then
	usage
	exit 1
fi

for i
do
	case $i in
		-c) CONFIG=$2
		    shift; shift;
			;;
		-i) INSLIST=$2
		    shift; shift;
			;;
		-u) UNCONFIG=$2
		    shift; shift;
			;;
		--) ;;
		-*) usage
		    exit 1
			;;
	esac
done

# Verify that we were given all the required inputs.
if [ ! -r $INSLIST ]
then
	$echo "ERROR : create_links - $INSLIST is not readable"
	exit 2
fi

if [ $CONFIG ]
then
	# argument was given
	if [ -f $CONFIG -a ! -w $CONFIG ]
	then
		$echo "ERROR : create_links - $CONFIG is not writeable"
		exit 2
	fi
else
	usage
	exit 1
fi

if [ $UNCONFIG ]
then
	# argument was given
	if [ -f $UNCONFIG -a ! -w $UNCONFIG ]
	then
		$echo "ERROR : create_links - $UNCONFIG is not writeable"
		exit 2
	fi
else
	usage
	exit 1
fi

# If we are not adding to an existing config and unconfig script, then add a 
# standard prolog to the top of the file since we will be shipping it to the
# customer.
if [ ! -f $CONFIG ]
then
	add_header $CONFIG
fi

if [ ! -f $UNCONFIG ]
then
	add_header $UNCONFIG
fi

# Strip the inslist for what is needed
$grep "^#[SsHh]" $INSLIST > $WORKING_INSLIST

# Work through the list of links
$cat $WORKING_INSLIST | while read line
do

	# get the type, source and targets from $line
	set -- $line
		type=`$echo $1 | cut -c2`
		source=$2
		shift ; shift
		targets=$*


	# loop through the possible targets
	for target in $targets
	do

	# if this should be a hard link
		if [ $type = "H" -o $type = "h" ]
		then
			$echo "ln $source $target || exit 1" >> $CONFIG
			$echo "rm -rf $target || exit 1" >> $UNCONFIG

	# else, this is a symbolic link
		else
			$echo "ln -s $source $target || exit 1" >> $CONFIG
			$echo "rm -f $target || exit 1" >> $UNCONFIG
		fi
	done
done

# make sure that the script ends in an "exit 0"
$echo "exit 0" >> $CONFIG
$echo "exit 0" >> $UNCONFIG

exit 0
