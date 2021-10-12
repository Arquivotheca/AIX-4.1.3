#!/bin/sh
# @(#)85	1.32  src/bldenv/pkgtools/adelppname.sh, pkgtools, bos41J, 9513A_all 3/8/95 12:52:59
#
#   COMPONENT_NAME: PKGTOOLS
#
#   FUNCTIONS: netLS
#	       usage
#              stripComments
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

# INPUT PARAMETERS:
#	1. -f the file format (1 digit)
#       2. -v the version number (2 digits)
#       3. -r the release number (2 digits)
#       4. -m the maintenance level (4 digits)
#	5. -F the fix level (4 digits)
#       6. -p the platform (1 char)
#	7. -t the (type) media designator (1 char) (optional)
#       8. -u The name of the input lpp name file
#       9. -l The product name
#      10. -c The compids.table file name
#      11. -o the output file name
#      12. -a the input liblpp archive file name
#      13. -d the directory where .size, .prereq, and .supersede files reside
#      14. -k the name of the aparsinfo file
#      14. -L the flag to specify it iFOR/LS information should be added
#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#
echo=$ODE_TOOLS/usr/bin/echo
sed=$ODE_TOOLS/usr/bin/sed
awk=$ODE_TOOLS/usr/bin/awk
getopt=$ODE_TOOLS/usr/bin/getopt
rm=$ODE_TOOLS/usr/bin/rm
cat=$ODE_TOOLS/usr/bin/cat
ar=$ODE_TOOLS/usr/bin/ar
cp=$ODE_TOOLS/usr/bin/cp
mkdir=$ODE_TOOLS/usr/bin/mkdir
expr=$ODE_TOOLS/usr/bin/expr
du=$ODE_TOOLS/usr/bin/du
grep=$ODE_TOOLS/usr/bin/grep
touch=$ODE_TOOLS/usr/bin/touch
ls=$ODE_TOOLS/bin/ls
xargs=$ODE_TOOLS/usr/bin/xargs

#
# NAME: usage
#
# FUNCTION: usage statement.
#
# INPUT: None.
#
# OUTPUT: Prints usage statement.
#
# SIDE EFFECTS: None
#
# RETURNS: 0 always.
#

usage()
{
    $echo "$myName: Usage: -f<fmt> -v<ver> -r<rel> -m<mnt1> -F<fix> "
    $echo "$myFill         -p<pltfm> -u<lpp file>.lp [-t<t>] [-l<product_name>]"
    $echo "$myFill         -c<compids file> -o<out_file> [-a <liblpp.a file>]"
    $echo "$myFill         [-d <directory>] [-k<aparsinfo file>] [-L]"
    exit 1
}

#
# NAME: stripComments
#
# FUNCTION: Removes comments and blank lines from input file
#
# INPUT: a file
#
# OUTPUT: File without comments and blank lines to stdout.
#
# SIDE EFFECTS: None.
#
# RETURNS: 0 always.
#

stripComments()
{
    $sed -e 's/^[#].*$//' -e '/^[ 	]*$/d' < $1
    return 0
}

#-----------------------------------------------------------------
# Generate NetLS information from compids.table for lpp_name.    |
#-----------------------------------------------------------------
# Read the compids.table file and extract the NetLS information. |
# The NetLS information is inserted into the lpp_name file.      |
#-----------------------------------------------------------------
netLS()
{
   ids_entry=`$grep "^$oldlpp\:" $idsfile` 2> /dev/null
   if [ -z "$ids_entry" ];then
     $echo "\n adelppname: WARNING: An entry for $oldlpp was not found in compids table.\n"
     return 
   fi
   vendor_name=`$echo $ids_entry | $awk -F":" '{print $8}'`
   vendor_id=`$grep "^\%\%\_" $idsfile 2> /dev/null | $grep $vendor_name 2> /dev/null | $sed -e 's/%%_'$vendor_name'=//'`
   netls_prod_id=`$echo $ids_entry | $awk -F":" '{print $9}'`
   netls_prod_ver=`$echo $ids_entry | $awk -F":" '{print $10}'`
   if [ -n "$vendor_id" -a -n $"netls_prod_id" -a -n $"netls_prod_ver" ]; then
      $echo "$vendor_id"          >> $outfile 
      $echo "$netls_prod_id"      >> $outfile 
      $echo "$netls_prod_ver"     >> $outfile 
   fi
   return
}

#----------------------------------------------------------------------
# Put the name of this program into variable myName for error messages.
# Also create a variable myFill of spaces same length for 2nd line.
#----------------------------------------------------------------------

myName=`$echo $0 | $sed -e 's/.*\/\([^/]*\)$/\1/'`
myFill=`$echo $myName | $sed -e 's/./ /g'`

media="I"			# Set media type to 'install'
prodname=""
iFORLSFlag=no
LIBLPPDEFAULT="./liblpp.a"
liblppfile=$LIBLPPDEFAULT         # Set default liblpp.a file
# Default location for .size, .prereq, and .supersede files is pwd
filesdir="."
ei=0

set -- `$getopt "f:l:v:r:m:p:F:t:u:c:o:a:d:k:L" $*`

if [ $? -ne 0 ]; then
    usage
fi

if [ $# -lt 16 ]; then
    usage
fi

while [ $1 != "--" ]; do
    case $1 in
	-f)
	    fmt=$2;  shift 2;
	    if [ `$expr $fmt : ".*"` -ne 1 ]; then
	       $echo "$myName: The "-f" flag value must be one digit"
	       ei=1;
	    fi
	    if [ `$expr "$fmt" : "[0-9]*"` -ne 1 ]; then
	       $echo "$myName: The "-f" flag value must be numeric"
	       ei=1;
	    fi
	    ;;
	-v)
	    ver=$2;  shift 2;
	    if [ `$expr $ver : ".*"` -ge 3 ]; then
	       $echo "$myName: The "-v" flag value must be atmost two digits"
	       ei=1;
	    fi
	    if [ `$expr "$ver" : "[0-9]*"` -eq 0 ]; then
	       $echo "$myName: The "-v" flag value must be numeric"
	       ei=1;
	    fi
	    ;;
	-r)
	    rel=$2;  shift 2;
	    if [ `$expr $rel : ".*"` -ge 3 ]; then
	       $echo "$myName: The "-r" flag value must be atmost two digits"
	       ei=1;
	    fi
	    if [ `$expr "$rel" : "[0-9]*"` -eq 0 ]; then
	       $echo "$myName: The "-r" flag value must be numeric"
	       ei=1;
	    fi
	    ;;
	-m)
	    mnt1=$2;  shift 2;
	    if [ `$expr $mnt1 : ".*"` -ge 5 ]; then
	       $echo "$myName: The "-m" flag value must be atmost four digits"
	       ei=1;
	    fi
	    if [ `$expr "$mnt1" : "[0-9]*"` -eq 0 ]; then
	       $echo "$myName: The "-m" flag value must be numeric"
	       ei=1;
	    fi
	    ;;
	-p)
	    plat=$2; shift 2;
	    if [ `$expr $plat : ".*"` -ne 1 -o `$expr $plat : [A-Z]` -ne 1 ]; then
	       $echo "$myName: The "-p" flag value must be one uppercase character"
	       ei=1;
	    fi
	    ;;
	-F)
	    fix=$2;  shift 2;
	    if [ `$expr $fix : ".*"` -ge 5 ]; then
	       $echo "$myName: The "-F" flag value must be atmost four digits"
	       ei=1;
	    fi
	    if [ `$expr "$fix" : "[0-9]*"` -eq 0 ]; then
	       $echo "$myName: The "-F" flag value must be numeric"
	       ei=1;
	    fi
	    ;;
	-t)
	    media=$2; shift 2;
	    case ${media} in
		I | \
		S | \
		O | \
		ML | \
		SF | \
		SR )
		   ;;
		* )
	           $echo "$myName: The -t flag value is invalid."
		   $echo "\tValid types are I, S, O, ML ,SR and SF."
	           ei=1;
	    esac
	    ;;
        -u)
	    lppname=$2; shift 2;
	    ;;
        -k)
	    aparsinfoFile=$2; shift 2;
	    ;;
	-l)
	    prodname=$2; shift 2;
	    ;;
	-c)
	    idsfile=$2; shift 2;
	    ;;
	-o)
	    outfile=$2; shift 2;
	    ;;
	-a)
	    liblppfile=$2; shift 2;
	    ;;
	-d)
	    filesdir=$2; shift 2;
	    ;;
	-L)
	    iFORLSFlag=yes; shift;
	    ;;
    esac
done
if [ ! "$lppname" ]; then
   $echo "$myName: The input lpp file parameter is missing"
   ei=1
fi

inFile=$lppname


#----------------------------------------------------------------
# All 4.1 (fmt=4) type images require the -l argument and the   |
# product file.  Check to make sure it is there.                |
#----------------------------------------------------------------
if [ -z "$prodname" ] && [ "$fmt" = "4" ]
then
    $echo "$myName:  The product name should be specified with the"
    $echo "$myFill  -l option for all 4.1 images.  Please rerun using"
    $echo "$myFill  the -l option.  Processing stopped."
    rc=12
    exit $rc
fi

#----------------------------------------------------------------
# If iFOR/LS flag is set, check to make sure that compids table  |
# is specified using -c flag.                                    |
#----------------------------------------------------------------
if [ -z "$idsfile" ] && [ "$iFORLSFlag" = "yes" ]
then
    $echo "$myName:  The compids file should be specified with the"
    $echo "$myFill  -c option for all iFOR/LS information.  Please rerun using"
    $echo "$myFill  the -c option.  Processing stopped."
    rc=12
    exit $rc
fi

#----------------------------------------------------------------
# For 4.1 (fmt=4) type images, bos.rte is not a normal install  |
# image that installp would want to install.  Display a warning |
# message if product is bos.rte and media type is not O.        | 
#----------------------------------------------------------------
if [ "$fmt" = "4" ] && [ "$prodname" = "bos.rte" ] && [ "$media" != "O" ]
then
   $echo "$myName: WARNING: Expected media type (-t option) of O for"
   $echo "$myFill  the 4.1 bos.rte product.  Processing will continue."
fi

#----------------------------------------------------
# Check to see if the input lppname file is present.
#----------------------------------------------------
if [ ! -s "$inFile" ]; then
   $echo "$myName: The input lppname file $inFile is not found or empty"
   $echo "$myFill  Please correct,  processing stopped."
   rc=10
   exit $rc
fi

if [ -z "$outfile" ]
then
   $echo "$myName: No output file specified on the command line."
   $echo "$myFill  Please rerun using the -o<out_file> option."
   $echo "$myFill  Processing stopped."
   rc=14
   exit $rc
fi

#---------------------------------------------------------------
# Check the return code from parsing the command line arguments
#---------------------------------------------------------------
if [ $ei -ne 0 ]; then
   $echo "$myName: Processing stopped due to errors on command line"
   rc=11
   exit $rc
fi

#---------------------------------------------------------------
# If the input lppname file is exactly lpp_name, rename it
# or else we would get unpredictable results.
#---------------------------------------------------------------
if [ $inFile = "lpp_name" ]; then
    $cp lpp_name in_lpp_name
    lppname="in_lpp_name"
    inFile="in_lpp_name"
fi

#-----------------------------------
# Begin making "lpp_name" file.
#-----------------------------------
if [ "$fmt" -ne "4" ]
then
    $echo "$fmt $plat $media {" > $outfile
else
    $echo "$fmt $plat $media $prodname {" > $outfile
fi

#------------------------------------------------------------------------
# Get rid of blank lines & comment lines from input file into temp file.
#------------------------------------------------------------------------
$sed -e 's/^[#*].*$//' -e '/^[ 	]*$/d' < $inFile > /tmp/lpn.$$

#------------------------------------------------------------------------
# Store LPP name (1st field, 1st rec, ends with period, space or tab).
#------------------------------------------------------------------------
LPPName=`$awk 'BEGIN { FS = "[. \t]" }
	    NR = 1 { print $1;exit }' < /tmp/lpn.$$`

#------------------------------------------------------------------------
# Determine if this is microcode by looking at the third field.  If it is
# "D" then this is microcode and we want a 3.1 looking lpp_name file.
#------------------------------------------------------------------------

tf=`$awk 'NR == 1{print $3;exit}' < /tmp/lpn.$$` 
if [ "$tf" = "D" ]; then
    if [ "$fmt" != "1" ]
    then
	$echo "$myName:  WARNING:  the content field of the  $LPPName.lp" 
	$echo "$myFill  file is D which indicates a 3.1 type lpp_name"
	$echo "$myFill  should be generated.  The -f argument was $fmt"
	$echo "$myFill  which will be changed to a 1."
	$echo "$myFill  Processing will continue."
	fmt=1
    fi
    $echo "$fmt $plat $media {" > $outfile
    $cat /tmp/lpn.$$ |
    while read lpp rest; do
	lpptest=`$echo $lpp | $sed "s/[\.	 ].*//"`
	if [ "$LPPName" = "$lpptest" ]; then
	    $echo "$lpp $ver.$rel.$mnt1.$fix 01 $rest" >> $outfile
	else
	    $echo "$lpp" "$rest" >> $outfile
	fi
    done
    $echo "}" >> $outfile
    $rm -f /tmp/lpn.$$
    exit
fi

trap "$rm -rf i_w $outfile /tmp/lpn.$$; exit 3" 1 2 3 15

#----------------------------------------------------------------
# If a size file is not found in library then create a temp file 
# with INSTWORK 0 it in and archive it into liblpp.a. 
#----------------------------------------------------------------
$ar t $liblppfile | $grep ".size$" > /dev/null 2>&1
if [ $? -ne 0 ];then
   $echo INSTWORK 0 > $prodname.size
   $ar q $liblppfile $prodname.size
   $rm $prodname.size
fi

#----------------------------------------------------------------
# Calculate size values for INSTWORK
#----------------------------------------------------------------
$mkdir i_w                              # Make a temporary sub_directory
$cp $liblppfile i_w/.	                # Copy library to the temp dir.
filesize=`$du -s i_w | $awk '{print $1}'` # Get liblpp.a file size
cd i_w > /dev/null 2>&1
$ar x liblpp.a                          # Unarchive library to get full package size.
$rm -f liblpp.a				# Dont count the liblpp.a file size twice.
cd .. >/dev/null 2>&1
libsize=`$du -s i_w|$awk '{print $1}'`	# Get the full pkg size in 512B blocks.
$rm -rf i_w		                # Remove temporary dir 


$echo "$LPPName.EOF" >> /tmp/lpn.$$	# Put dummy EOF rec at end of file.
oldlpp=""				# Init old lpp name to null.
$cat /tmp/lpn.$$ |
  while read lpp rest; do

    #----------------------------------------------------------------------
    # Store suspected LPP name in lpptest (ends with period, space or tab).
    #----------------------------------------------------------------------
    lpptest=`$echo $lpp | $sed "s/[\.	 ].*//"`

    #-----------------------------------------------
    # If this text is LPP name, put old lpp/option 
    # record into lpp_name file, else write out text.
    #-----------------------------------------------
    if [ "$LPPName" != "$lpptest" ]; then
	$echo "$lpp" "$rest" >> $outfile
	continue
    fi

    if [ "$oldlpp" -a "$oldlpp" != "$LPPName.EOF" ]; then
        $echo "["              >> $outfile

        #-------------------------------------------------------------
	# Set up file IDs for prerequisite, size, and supersede files.
        #-------------------------------------------------------------
        pr_ID="$filesdir/$oldlpp.prereq"
        sz_ID="$filesdir/$oldlpp.size"
        sp_ID="$filesdir/$oldlpp.supersede"

        #-------------------------------------------------------------
	# Affix optional prereq lines,  if found.
	# Get rid of blank lines & comment lines from prereq file
	#------------------------------------------------------------------------
        if [ -s "$pr_ID" ]; then
	    stripComments $pr_ID >> $outfile
        fi

        #-------------------------------------------------------------
	# Affix size lines, if found, after % 
	# Get rid of blank lines & comment lines from size file
        #-------------------------------------------------------------
        $echo "%"              >> $outfile
        if [ -s "$sz_ID" ]; then
	    stripComments $sz_ID >> $outfile
        fi
        
        #------------------------------------------------------------------
        # If a root liblpp.a does not exist then put the INSTWORK values
        # in outfile; else add the root and usr INSTWORK library values and 
        # get the larger value of the 2 liblpp.a files.  This should only
	# be done for usr options, so we'll guess that if the liblppfile
	# is liblpp.a then we're doing a usr option.  Otherwise it would
	# probably be data/liblpp.a or root/liblpp.a.  If this assumption
	# is wrong then we could possibly require more space than we really
	# need.
        #------------------------------------------------------------------
        if [ ! -f root/liblpp.a -o "$liblppfile" != $LIBLPPDEFAULT ]; then
           $echo "INSTWORK $libsize $filesize" >> $outfile
        else
           cd root >/dev/null 2>&1
           trap "$rm -rf i_w; exit 3" 1 2 3 15
           $mkdir i_w         # Make a temporary sub_directory
           $cp liblpp.a i_w/. # Copy library to the temp dir.
           rfilesize=`$du -s i_w | $awk '{print $1}'` # Get liblpp.a file size.
           cd i_w >/dev/null 2>&1
           $ar x liblpp.a     # Unarchive library to get full package size.
	   $rm -f liblpp.a    # Don't count the liblpp.a file size twice.
           cd ..	       >/dev/null 2>&1
           rlibsize=`$du -s i_w|$awk '{print $1}'` # Get the full pkg size in 512B blocks.
	   #-------------------------------------------------------
	   # Generate size file entry for the /lpp/<package>
	   # directory.  This should be the size of the root liblpp.a
	   # files (if they exist) for this fileset only.
	   #-------------------------------------------------------
	   cd i_w >/dev/null 2>&1
	   $ls -1 | $grep -sv $oldlpp\. | $xargs $rm
	   $ls $oldlpp.* >/dev/null 2>&1
	   if [ $? -eq 0 ]
	   then
	        cd .. >/dev/null 2>&1
		pkgdirsize=`$du -s i_w|$awk '{print $1}'`
	   else
	        cd .. >/dev/null 2>&1
		pkgdirsize=""
	   fi

           $rm -rf i_w         # Remove temp dir created for INSTWORK calc.
           cd .. >/dev/null 2>&1

           #---------------------------------------------------------
           # Add the root INSTWORK to usr INSTWORK, get the larger of 
           # the 2 liblpp.a file sizes, and put in outfile.
           #---------------------------------------------------------
           value1=`$expr $libsize + $rlibsize`
	   if [ $filesize -ge $rfilesize ]; then
		value2=$filesize
	   else
	 	value2=$rfilesize
	   fi
           $echo "INSTWORK $value1 $value2" >> $outfile
	   [ -n "$pkgdirsize" ] && $echo "/lpp/$prodname $pkgdirsize" \
		>> $outfile

           sz_ID="$oldlpp.size"
           if [ -s root/"$sz_ID" ]; then
               #---------------------------------------
               # Affix root size lines, if found
       	       # Get rid of blank lines & comment lines from root size file
               #---------------------------------------
	       stripComments root/$sz_ID  >> $outfile
           fi
        fi

        $echo "%"              >> $outfile
        #----------------------------------------------
	# Affix optional supersede lines,  if found.
       	# Get rid of blank lines & comment lines from supersedes file
        #----------------------------------------------
        if [ -s "$sp_ID" ]; then
	    stripComments $sp_ID >> $outfile
        fi

        #----------------------------------------------
        # Generate the NetLS information if it exists
        #----------------------------------------------
        $echo "%"              >> $outfile
	if [ "$iFORLSFlag" = "yes" ]; then
            netLS
        fi

        #----------------------------------------------
	# Affix the aparsinfo file lines
	# This file contains all the apar numbers and their abstracts
	# for a ptf.
       	# Get rid of blank lines & comment lines from aparsinfo file
        #----------------------------------------------
        $echo "%"              >> $outfile
	if [ -n "$aparsinfoFile" ]; then
		if [ ! -s $aparsinfoFile ]; then 
			$echo "$myName: WARNING: The $aparsinfoFile file is empty."
			$echo "processing will continue."
		else
	    		stripComments $aparsinfoFile >> $outfile
        	fi
	fi
        $echo "%"              >> $outfile

        #----------------------------------------------
	# Affix closing brace, signifying end of option.
        #----------------------------------------------
        $echo "]"              >> $outfile
    fi

    if [ "$lpp" != "$LPPName.EOF" ]; then
	  $echo "$lpp $ver.$rel.$mnt1.$fix 01 $rest" >> $outfile	#Write curr.
         oldlpp=$lpp
    fi

  done

$echo "}"                  >> $outfile
$rm -f /tmp/lpn.$$       # Remove work file
exit 0			############ end of adelppname #############
