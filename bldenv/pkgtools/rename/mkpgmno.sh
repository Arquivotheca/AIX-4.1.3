#!/bin/sh
SCCSID="@(#)96  1.9  src/bldenv/pkgtools/rename/mkpgmno.sh, pkgtools, bos412, GOLDA411a 11/5/92 15:05:56"
#
# COMPONENT_NAME: (BOSBUILD) Tools for building natively on the R2 platform
#
# FUNCTIONS: mkpgmno
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
#========================================================================
#
#   Purpose:
#       This shell updates the copyright statement with the hone
#       product_id and the year it was built.
#       In addition it creates the archived productid file in 'liblpp.a'.
#
#   Syntax:
#       mkpgmno   -d package_name  -p xref_table
#
#   Description:
#       Based on the specified package_name (-d) this shell:
#          - looks up the product_id using the package_name/product_id table
#          - updates './copyright' with date (keyword string
#            'LATEST DATE') and with product_id (keyword string
#            'PROG#')
#          - creates './productid'.
#
#   Flags:
#       -d:    package_name (ie. xlp, pcsim, em78,,,)
#                No defaults.
#       -p:    filename of the package/productid xref file.
#                No defaults.
#
#========================================================================
########
## create unique name for a temp work file
########
TEMP_FILE=/tmp/mkpgmno.$$

########
## declare proper format of the command
########

USAGE="\
$SCCSID\n\
usage:  mkpgmno   -d package_name  -p xref_table\n\
       where:\n\
	    -d   : package_name (ie. xlp, pcsim, em78)\n\
	             No defaults.\n\
	    -p   : filename of the package/productid xref file.\n\
	             No defaults.\n"


##########
## if no args were specified, exit
##########
if [ $# -eq 0 ]
then
	echo "mkpgmno:  -d and -p arguments are required"
	echo "$USAGE"
	exit 100
fi

##########
## loop thru the flags if any
##########
set -- `getopt "d:p:" $*`
if [ $? -ne 0 ]
then
	echo "mkpgmno:  -d and -p the only possible arguments"
	echo "$USAGE"
	exit 110
fi


##########
## loop thru the flags
##########

while [ "$1" != "--" ]
do
	case $1 in
	        "-d")
	                PACKAGE_NAME=$2
	                shift

	        ;;
	        "-p")
	                XREF_FILENAME=$2
	                shift

	        ;;

	esac
	shift
done


##########
## shift past the --
##########
shift

##########
## if args were specified after the flags, exit
##########
if [ $# -ne 0 ]
then
	echo "mkpgmno:  more arguments specified than used"
	echo "$USAGE"
	exit 140
fi


##########
## if either parameter was not specified, exit
##########
if [ "$PACKAGE_NAME" = "" -o "$XREF_FILENAME" = "" ]
then
	echo "mkpgmno:  arguments required for -d and -p options"
	echo "$USAGE"
	exit 150
fi


##########
## check if the xref file exists
##########
	if [ ! -f $XREF_FILENAME ]
	then
	        echo "mkpgmno: file $XREF_FILENAME does not exist"
	        exit 160
	fi


##########
## pick up year
##########
  YEAR=`date | awk '{print $6}'`


##########
## set feature code to 0000
##########
  FCODE=0000

##########
## use xref table to get hone name
##########
  PRODUCT_ID_LINE=`grep "^${PACKAGE_NAME}:" $XREF_FILENAME | awk 'NR==1'`
  PRODUCT_ID=`echo $PRODUCT_ID_LINE | cut -f2 -d:`


##########
## if did not find entry in xref table, exit
##########
if [ "$PRODUCT_ID" = "" ]
then
	echo "mkpgmno:  did not find package '$PACKAGE_NAME' in\n\
	   package/productid xref table '$XREF_FILENAME'"
	echo "mkpgmno:  './productid' will not be created"
#       echo "$USAGE"
#       exit 170

else
       ##########
       ## create ./productid file
       ##########

	TYPE=`echo $PRODUCT_ID | cut -c1-4`
	MODEL=`echo $PRODUCT_ID | cut -c5-`
	PRODID=${TYPE}-${MODEL}
	echo "$PACKAGE_NAME $PRODID $FCODE" > ./productid

fi


##########
## update copyright file
##########
  if [ -f ./copyright ]
  then
      # exit if file not writable
      if [ ! -w ./copyright ]
      then
	        echo "mkpgmno: ./copyright file not writable"
	        exit 180
      fi

      # put ed commands into temp file
      echo "r ./copyright"             > $TEMP_FILE
      echo "1,\$s/LATEST DATE/$YEAR/g" >> $TEMP_FILE
#     echo "1,\$p"                     >> $TEMP_FILE
      echo "w"                         >> $TEMP_FILE
      echo "q"                         >> $TEMP_FILE

      # execute ed command
      ed < $TEMP_FILE  > /dev/null
#     cat  $TEMP_FILE

      # put ed commands into temp file
      echo "r ./copyright"             > $TEMP_FILE
      echo "1,\$s/PROG#/$PRODID/g"     >> $TEMP_FILE
#     echo "1,\$p"                     >> $TEMP_FILE
      echo "w"                         >> $TEMP_FILE
      echo "q"                         >> $TEMP_FILE

      # execute ed command
      [ -z "$PRODUCT_ID" ] || ed < $TEMP_FILE  > /dev/null
#     cat  $TEMP_FILE

      # delete ed command file
      rm   $TEMP_FILE
  fi


#
# end of shell mkpgmno
#



