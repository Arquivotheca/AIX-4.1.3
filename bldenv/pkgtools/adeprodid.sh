#!/bin/ksh
# @(#)38	1.1  src/bldenv/pkgtools/adeprodid.sh, pkgtools, bos412, GOLDA411a 5/9/94 09:09:19
#
#   COMPONENT_NAME: pkgtools
#
#   FUNCTIONS: usage
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1994
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

#--------------------------------------------------------
# Set up aliases for use in ADE build environment.	|
#--------------------------------------------------------
echo=$ODE_TOOLS/usr/bin/echo
getopt=$ODE_TOOLS/usr/bin/getopt
sed=$ODE_TOOLS/usr/bin/sed
grep=$ODE_TOOLS/usr/bin/grep
cut=$ODE_TOOLS/usr/bin/cut
awk=$ODE_TOOLS/usr/bin/awk

#------------------------------------------------------------------------
# NAME:  usage								|
#									|
# DESCRIPTION:	Display command line syntax and exit.			|
#									|
# PRE CONDITIONS:  Invalid or incomplete syntax specified on command	|
#		   line.						|
#									|
# POST CONDITIONS:  Exits with non-zero return code.			|
#									|
# PARAMETERS:  None							|
#									|
# NOTES:  None								|
#									|
# DATA STRUCTURES:  None						|
#									|
# RETURNS:  Exits program with non-zero return code.  Does not return	|
#	    to caller.							|
#------------------------------------------------------------------------
usage()
{
	$echo "USAGE:  $cmd -l <product_name>  -t <compids.table>"
	$echo "\t\t[ -d <output_directory> ]\n"

	exit 1
}


#------------------------------------------------------------------------
# NAME: main module							|
#									|
# DESCRIPTION: Generate a product id file given a product name and	|
#	       the location of the compids.table file.			|
#									|
# PRE CONDITIONS:  None							|
#									|
# POST CONDITIONS:  A product id file exists in the current working	|
#		    directory.						|
#									|
#     The format of the product id file is:				|
#	<package name> xxxx-yyyyy					|
#     where xxxx-yyyyy is the nine-digit compid from the compids.table	|
#     file.								|
#									|
# PARAMETERS: See usage							|
#									|
# NOTES:  The format of entries in the compids.table file is:		|
#									|
#     package name:product id:feature code:product release level:	|
#         system release level:change team id(s):CMVC release:vendor_id:|
#         NetLS_product_id:NetLS_product_version:			|
#									|
#     Example:								|
#									|
#     bos.adt:575603001:5005:410:410:TX2527:XXX:IBM:1234567:12345678901:|
#									|
# DATA STRUCTURES:  None						|
#									|
# RETURNS:								|
#------------------------------------------------------------------------

#--------------------------------
# Initialize variables.		|
#--------------------------------

cmd=`$echo $0 | $sed -e 's/.*\/\([^/]*\)$/\1/'`
LPP_NAME=""
COMPIDSFILE=""
OUTPUTDIR=.

set -- `$getopt "l:t:d:" $*`

[ $? -ne 0 ] && usage

while [ "$1" != "--" ]
do
	case $1 in
	        "-l")
	                LPP_NAME=$2
	                shift
	        ;;

	        "-t")
	                COMPIDSFILE=$2
	                shift
	        ;;

		"-d")
			OUTPUTDIR=$2
			shift
		;;
	esac
	shift
done

#------------------------------------------------
# If either parameter was not specified, exit.	|
#------------------------------------------------
if [ -z "$LPP_NAME" ] ||  [ -z "$COMPIDSFILE" ]
then
	usage
fi


#--------------------------------
# Create productid file.	|
#--------------------------------

num=`$grep -c "^${LPP_NAME}:" ${COMPIDSFILE}`

if [ $num -gt 1 ]
then
	$echo "$cmd:  FATAL ERROR:  Multiple entries in ${COMPIDSFILE}\n"
	$echo "\tfor product ${LPP_NAME}.\n"
	exit 1
fi

#----------------------------------------------------------------
# Product id is the nine-digit value in the second field of the	| 
# entry for this lpp.						|
#----------------------------------------------------------------

PRODUCT_ID=`$grep "^${LPP_NAME}:" ${COMPIDSFILE} | $awk -F":" '{print $2}'`

if [ -z "$PRODUCT_ID" ]
then
	$echo "$cmd:  FATAL ERROR:  Could not get product id from\n"
	$echo "\t${COMPIDSFILE} for product ${LPP_NAME}.\n"
	exit 1
fi

#----------------------------------------------------------------
# Type is first 4 digits of the nine-digit value.		|
#----------------------------------------------------------------

TYPE=`$echo $PRODUCT_ID | $cut -c1-4`
if [ -z "$TYPE" ]
then
	$echo "$cmd:  FATAL ERROR:  Could not get type field from\n"
	$echo "\tproduct id value ${PRODUCT_ID} for ${LPP_NAME}.\n"
	exit 1
fi

#----------------------------------------------------------------
# Model is last 5 digits of the nine-digit value.		|
#----------------------------------------------------------------

MODEL=`$echo $PRODUCT_ID | $cut -c5-`
if [ -z "$MODEL" ]
then
	$echo "$cmd:  FATAL ERROR:  Could not get model field from\n"
	$echo "\tproduct id value ${PRODUCT_ID} for ${LPP_NAME}.\n"
	exit 1
fi

#----------------------------------------------------------------
# Put them together with a dash in the middle.			|
#----------------------------------------------------------------

PRODID=${TYPE}-${MODEL}

echo "$LPP_NAME $PRODID" > ${OUTPUTDIR}/productid

exit 0
