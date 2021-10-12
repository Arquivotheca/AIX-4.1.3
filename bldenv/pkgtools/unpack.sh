#!/bin/sh
# @(#)49        1.3  src/bldenv/pkgtools/unpack.sh, pkgtools, bos412, GOLDA411a 6/24/93 08:40:15
#
#   COMPONENT_NAME: PKGTOOLS
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1991,1993
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# FUNCTION: TO EXTRACT THE THREE ELEMENTS OF A CCSS FORMAT PTF.
#           1. THE BFF
#           2. THE TOC
#           3. THE INFOFILE
#           THE INFOFILE IS CONVERTED BACK TO ASCII FOR EDITING.

# This tool has two positional parameters as input. #1 is the full path
# name to the prod or ship server and # 2 is the name of the 
# PTF (ex. U401977.ptf) that the data is to be extracted for.
# The infofile is extracted, put into a useable form for editing.

if [ $# != 2 ]; then
   echo "unpack: ERROR: required two argements: ship or product"
   echo "               server and PTF name"
   echo "Example: unpack ship_dir U401977.ptf"
   exit 1
fi

path=""
ptfno=""
path="$1"
if [ "$path" = "" ]; then
   echo "Must enter the path to the ship or prod directory"
   exit
else
   if [ ! -d $path ]; then
      echo "unpack: ERROR: $path is not a directory"
      exit 1
   fi
fi
stdir=`pwd`
ptfno=$2
if [ "$ptfno" = "" ]; then
   echo "Must enter the ptf number as second positional parameter"
   exit
else
   ptfno=`echo $ptfno | sed -e 's/.ptf//'`
   cd $path
   ptf="$ptfno".ptf""
   if [ ! -f $ptf ]; then
      echo "unpack: ERROR: $ptf is not a file"
      exit 1
   fi
fi
    ccss_unpack -c "$path"/"$ptf" -i /tmp/"$ptfno".info 
    dd if=/tmp/"$ptfno".info of=/tmp/"$ptfno".cvinfo conv=ascii cbs=80
cd $stdir
rm -f /tmp/"$ptfno".info

