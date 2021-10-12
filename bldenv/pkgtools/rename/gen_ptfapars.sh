#!/bin/ksh
# @(#)48        1.2  src/bldenv/pkgtools/rename/gen_ptfapars.sh, pkgtools, bos412, GOLDA411a 6/24/93 08:38:32
#
#   COMPONENT_NAME: PKGTOOLS
#
#   FUNCTIONS: abort
#		create_ptffile
#		get_ptfapars
#		syntax
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
# PURPOSE: Create ptfapars file that contains PTF and its related 
#          APARs information.
#
# FUNCTION: 1. Get the PTF from ship server and product server.
#           2. For each PTF, get its related APARs from its info file
#           3. Write the PTF and its APARs to ptfapars file


#-------------------------------------------------------------------
# syntax function: display the syntax information
#------------------------------------------------------------------- 
syntax() {
echo "gen_ptfapars [-p prod_dir] [-s ship_dir] [-d dir] [-h|-?]"
echo "\nFLAG:"
echo "     -p prod_dir		the directory of product sever"
echo "     -s ship_dir		the directory of ship sever"
echo "     -d dir       	the directory that the ptfapars file will be"
echo "     -h|-?		lists help message"
exit 1
}


#----------------------------------------------------------------
# abort function: remove the working temporary directory and
#                 exit the program with fail message
#----------------------------------------------------------------
abort() {
   rm -rf $WORK
   echo "\ngen_ptfapars FAILED!"
   exit 1
}


#----------------------------------------------------------------
# create_ptffile function: create the file that contains the 
#                          PTF number  
#----------------------------------------------------------------
create_ptffile() {

dir_name=$1

if [ $dir_name = "" ]; then
   echo "$cmd: ERROR: $dir_name does not exist"
   rc=1
else
   if [ ! -d $dir_name ]; then
      echo "$cmd: ERROR: $dir_name is not a directory"
      rc=1
   else
      if [ ! -r $dir_name ]; then
         echo "$cmd: ERROR: $dir_name is not readable"
         rc=1
      else
         ls -1 $dir_name |grep ".ptf$" | while read ptf
                                         do
                                          echo "$dir_name/$ptf" >> $WORK/ptffile
                                         done
         rc=0
      fi
   fi
fi
return rc
}
 

#-----------------------------------------------------------------
# get_ptfapars function: unpack the ptf ccss format file to get
#                        its info file and get the APARs from this
#                        info file. Then, write the PTF and its
#                        APARs to ptfapars file
#-----------------------------------------------------------------
get_ptfapars() {
if [ ! -f $WORK/ptffile ]; then
   echo "$cmd: ERROR: $WORK/ptffile does not exist"
   abort
else
   cat $WORK/ptffile |   
   while read ptf
   do
      ptf_dir=`dirname $ptf`
      ptf=`basename $ptf`
      ptf_name=`echo $ptf | sed -e 's/.ptf//'`
      unpack $ptf_dir $ptf_name
      if [ ! -f $dir/ptfapars ]; then
         > $dir/ptfapars
         grep "^APARTXT" /tmp/$ptf_name.cvinfo |   
         awk -F" " '{print $2}' | 
         while read apar
         do
            echo "$ptf_name|$apar" >> $dir/ptfapars
         done
      else
         grep -v "^$ptf_name" $dir/ptfapars > $dir/ptfapars.new
         rm -f $dir/ptfapars
         mv $dir/ptfapars.new $dir/ptfapars 
         grep "^APARTXT" /tmp/$ptf_name.cvinfo | 
         awk -F" " '{print $2}' | 
         while read apar
         do
            echo "$ptf_name|$apar" >> $dir/ptfapars
         done
      fi 
      rm -f /tmp/$ptf_name.cvinfo
   done
fi
}

#--- Main Program ---
cmd="gen_ptfapars"
WORK="/tmp/gen_ptfapars"
rm -rf $WORK
mkdir $WORK
chmod 777 $WORK
start_dir=`pwd`

type 32_pkg_environment | grep "not found"
if [ $? = 0 ]; then
   echo "$cmd: ERROR: 32_pkg_environment not found in $PATH"
   abort
fi
. 32_pkg_environment
prod_dir=$PROD_DIRNAME
ship_dir=$SHIP_DIRNAME

if [ $# != 0 ]; then 
set -- `getopt "s:p:d:h?" $* `
if [ $? != 0 ]; then
   echo "$cmd: ERROR: Problem in the command line: $*"
   syntax
fi

while [ $1 != "--" ]; do  
   case $1 in
     -s)
        ship_dir=$2
        shift 2;;
     -p)
        prod_dir=$2
        shift 2;;
     -d)
        dir=$2
        shift 2;;
   -[h?])
        syntax;;
   esac
done
shift
fi

ship_dir=`cd $ship_dir; pwd`
prod_dir=`cd $prod_dir; pwd`
dir=`cd $dir; pwd`
cd $start_dir

create_ptffile $ship_dir
rc=$?
if [ $rc != 0 ]; then
   abort
fi

create_ptffile $prod_dir
rc=$?
if [ $rc != 0 ]; then
   abort
fi

get_ptfapars
if [ $? != 0 ]; then
   abort           
else
   echo "\ngen_ptfapars SUCCESSFUL!"
fi
  
rm -rf $WORK
sync
exit 0
