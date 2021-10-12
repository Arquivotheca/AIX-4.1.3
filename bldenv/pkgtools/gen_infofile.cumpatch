#!/bin/ksh
# @(#)47        1.13  src/bldenv/pkgtools/gen_infofile.sh, pkgtools, bos41J, 9513A_all  3/8/95  15:51:10
#   COMPONENT_NAME: pkgtools
#
#   FUNCTIONS: abort
#	       get_requisite_info
#	       getvalue
#              output_ptfs
#	       read_lppname_desc_field
#	       syntax
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1991,1993
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#------------------------------------------------------------------
# NAME: gen_infofile.sh
#
# DESCRIPTION: generate the infofile that is part of the ccss image
#
# PRE CONDITIONS:
#
# POST CONDITIONS:
#
# PARAMETERS:
#     INPUT
#       table_file - Path & name of compids.table file
#       ptf number - ptf for which the infofile is being created 
#       ptf.options table - table containing information about
#                           each ptf
#       internal vrmf table - table containing the vrmf level of
#                             each fileset
#       lbl_table - Path & name of label table
#       bff_file - bff input file in 'backup' format
#                      or
#       tarfile - tar input file in 'tar' format
#       
#     OUTPUT
#       infofile - Path & name of output infofile
#
# NOTES:
#
# RETURNS: 1 - for error
#-------------------------------------------------------------------

####################################################################
#              subroutines start here
####################################################################

#------------------------------------------------------------------
# NAME:syntax
#
# DESCRIPTION: called when options are entered incorrectly or if help
#              is requested
#
# RETURNS: 1 - for error
#-------------------------------------------------------------------
syntax() {
    echo "\n$SCCSID"
    echo "USAGE: $cmd -i info_file -t table_file -p ptf_number"
    echo "            -o ptf.options table -v internal vrmf table"
    echo "            {-b bff_file|-r tar_file} [-h|-?]"
    echo ""
    echo "FLAGS: -i infofile  Path & name of output infofile"
    echo "       -t table_file Path & name of compids.table file to append"
    echo "       -p ptf number"
    echo "       -o ptf.options table"
    echo "       -v internal vrmf table"
    echo "       -l lbl_table Path & name of label table"
    echo "       -b bff_file  bff input file in 'backup' format"
    echo "       -r tarfile   tar input file in 'tar' format"
    echo "       -h           Displays this help screen.\n"
    exit $error_code
}

#------------------------------------------------------------------
# NAME: abort
#
# DESCRIPTION: called when an error occurs while generating the 
#              infofile
#
# RETURNS: 1 - for error
#-------------------------------------------------------------------
abort() {
    echo ""
    cd $START_DIR
    rm -rf $WORK
    exit 1
}

#------------------------------------------------------------------
# NAME: get_requisite_info
#
# DESCRIPTION: called to extract the requisite information out of
#              the toc file
#
# PRE CONDITIONS: lpp_name file exists before invocation
#
# POST CONDITIONS: creates the file req_temp which contains the
#                  requisite information
#
# PARAMETERS: none
#
# RETURNS: 0 always
#-------------------------------------------------------------------
get_requisite_info() {

    # Create file
    >  req_temp

    # Initialize destination to echo stuff to trash
    target='/dev/null'

    # Read through the lpp_name file placing the data in the appropriate
    # file: req_temp or /dev/null
    cat lpp_name | 
    while read First_parm Rest_of_line; do
        case "$First_parm" in
            '[' )  # prereq info always follows '['
                target='req_temp';;
            '%' )  # first % indicates transition of requisite info to size info
                # don't care about what follows requisite information
                target='/dev/null';;
            * )    # Write all data to appropriate file
                echo "$First_parm $Rest_of_line" >> $target
        esac
    done
} # End of get_requisite_info function

#------------------------------------------------------------------
# NAME: read_lppname_desc_field
#
# DESCRIPTION:
#        Parse lpp_file to get seventh field of second record
#        The seventh field is the product description. This
#        description may contain blanks. The end of text is denoted
#        by the end of line or a # (pound sign).
#
# PRE CONDITIONS: lpp_name file exists before invocation
#
# POST CONDITIONS: creates the file $WORK/desc_temp which contains the
#                  product description
#
# PARAMETERS: none
#
# RETURNS: 0 always
#-------------------------------------------------------------------
read_lppname_desc_field () {

    # Get 2nd record
    tmp_record=`cat lpp_name | awk ' NR==2 '`
    echo "$tmp_record" > record_temp

    # Parse 2nd record
    # and process seventh field (last)
    cat record_temp |
    while read fld1 fld2 fld3 fld4 fld5 fld6 last; do
        descrip=""
        first_wrd="y"
        for wrd in $last
        do
            # break if pound sign found
            if [ "'$wrd'" = "'#'" ]
            then
                break
            else
                # Check if this is first word to suppress leading 
                # blank in description
                if [ "$first_wrd" = "y" ]; then
                    descrip="$wrd"
                    first_wrd="n"
                else
                    descrip="$descrip $wrd"
                fi
                echo "$descrip" > $WORK/desc_temp
            fi
        done
    done
}

#------------------------------------------------------------------
# NAME: output_ptfs
#
# DESCRIPTION: formats the requisite and supersede information in
#              the infofile
#
# PRE CONDITIONS: ptfs_file is the name of the file containing the ptfs
#                 ptf_type is set to prereq, coreq, ifreq, or supersede
#                 The above variables are set before invocation.
#
# PARAMETERS: none
#
# RETURNS: 0 always
#-------------------------------------------------------------------
output_ptfs(){

    ptfs_found="n"; count=0; need_newline="n";      # Init flags.

    # Read each unique ptf
    cat "$ptfs_file" 2>/dev/null | sort -u | 
    while read req_ptf; do
        if [ "$ptfs_found" = "n" ]; then              
            echo "$ptf_type FOR $PTF =\c" >> infofile.$PTF
            ptfs_found="y"
        fi

        echo "$req_ptf" | grep "\." > /dev/null 2>&1
        rc=$?
        if [ $rc -eq 0 ]; then
            if [ $count -ge 3 ]; then
                echo " " >> infofile.$PTF
                need_newline="n"
                count=0
            fi
            echo " $req_ptf\c" >> infofile.$PTF
            need_newline="y"
            count=`expr $count + 2`
        else
            if [ $count -eq 4 ]; then
                echo " " >> infofile.$PTF
                need_newline="n"
                count=0
            fi
            echo " $req_ptf\c" >> infofile.$PTF
            need_newline="y"
            count=`expr $count + 1`
        fi 
    done

    if [ "$need_newline" = "y" ]; then
       echo " " >> infofile.$PTF
    fi

    # If there are no requisites or supersedes, it should say NONE
    if [ "$ptfs_found" = "n" ]; then  
       echo "$ptf_type FOR $PTF = NONE" >> infofile.$PTF
    fi

    echo "ENDSET" >> infofile.$PTF                #   print out "ENDSET"

}  #  End of output_ptfs function

#------------------------------------------------------------------
# NAME: getvalue
#
# DESCRIPTION: called to get the value of a field in a stanza.
#              The field value may span multiple lines.
#
# PRE CONDITIONS: the variable 'value' is set before invocation
#
# PARAMETERS: none
#
# RETURNS: value of field in $fieldvalue
#
#------------------------------------------------------------------
getvalue() {

   # see if there is a quote at the beginning of the line
   echo $value | grep ^\" > /dev/null 2>&1
   rc1=$?
   # see if there is a quote at the end of the line
   echo $value | grep \"$ > /dev/null 2>&1
   rc2=$?

   # strip off ending newline and line continuation
   y=${value%\\n\\}
   # if the value ends with a continuation character or if the value
   # begins with a quote and doesn't have an ending quote,
   # then the value spans multiple lines
   if [ "$y" != "$value" -o  $rc1 = 0 -a $rc2 != 0 ]
   then
       # strip off leading quote
       y=${y#\"}
       fieldvalue="$y"
       flag=0
       while [ $flag -eq 0 ]
       do
           read -r value
           # strip off newline and continuation character
           x=${value%\\n\\}
           # see if there is a quote at the end of the line
           echo $value | grep \"$ > /dev/null 2>&1
           rc2=$?
           # if line does not contain a newline and continuation character
           # or the line ends with a quote, then this is the last line of
           # the field value
           if [ "$value" = "$x"  -o  $rc2 = 0 ]
           then
               # this is the last line of the value
               # strip off the ending quote
               x=${value%\"}
               # terminate the while loop
               flag=1
           fi
           if [ "$x" != "" ]
           then
               # join the lines to form value
               fieldvalue="$fieldvalue\n $x"
           else
               fieldvalue="$fieldvalue\n"
           fi
       done
   else
       y=${value%\"}
       y=${y#\"}

       fieldvalue=$y
   fi
   return "$fieldvalue"
}


#------------------------------------------------------------------
# NAME: add_ptf_to_ifreq_list
#
# DESCRIPTION: called to add a ptf and product id to the list
#              of ifreqs.
#
# PRE CONDITIONS: the variables 'ptf' and 'option' are set before 
#                 invocation
#
# PARAMETERS: none
#
# RETURNS: none
#
#------------------------------------------------------------------
add_ptf_to_ifreq_list() {

    # if the ptf is not listed in the prereq or the coreq list,
    # then add it to the ifreq list
    grep "$ptf" prereq_ptfs > /dev/null 2>&1
    rc1=$?
    grep "$ptf" coreq_ptfs > /dev/null 2>&1
    rc2=$?
    if [ $rc1 -ne 0 -a $rc2 -ne 0 ]; then
        # Get the product id from packaging table
        prod_id=""
        prod_id=`cat $table_file   |
                 grep "^$option:"    |
                 head -1              |
                 awk -F: '{print $2}' `
        if [ "x$prod_id" = "x" ]; then
            echo "\n$cmd: ERROR: ifreq option: $option not found in compids.table file  "
            abort
        fi
        # Put the ifreq ptf number and product id into a file
        echo "${ptf}.${prod_id}" >> ifreq_ptfs
    fi
    
}

#------------------------------------------------------------------
# NAME: lookup_ptf
#
# DESCRIPTION: given a fileset vrmf, find the ptf that matches
#
# PRE CONDITIONS:  $internal_vrmf_table $ptf_options_file are set
#
# PARAMETERS:
#       $1 = fileset
#       $2 = vrmf
#
# RETURNS: ptf number on stdout
#
#------------------------------------------------------------------
lookup_ptf () {
    typeset -r fileset=$1
    typeset -r vrmf=$2

    cat $ptf_options_file $internal_vrmf_table |
    grep "$fileset " |
    egrep "$vrmf$|$vrmf " |
    head -1 |
    cut -f1 -d" "
}

################################################################################
# Beginning of generate infofile                                               #
################################################################################


########################
# Initialize variables #
########################
START_DIR=`pwd`
cmd=`basename $0`
WORK_DATE=`date +"%y""%m""%d""%H""%M""%S"`      # Work directory date stamp
table_file=""
info_file=""
tar_file=""
bff_file=""
error_code=0

#  Check command line
if [ $# -lt 1 ]; then
    error_code=1
    syntax
fi
set -- `getopt "i:t:b:l:r:p:o:v:h?" $*`
if [ $? != 0 ]; then
    echo "\n$cmd: ERROR: Bad option found on command line."
    error_code=1
    syntax
fi

# Parse the command line
while [ $1 != "--" ]; do
    case $1 in
        -i)
            info_file=$2;     shift 2;;
        -b)
            bff_file=$2;        shift 2;;
        -r)
            tar_file=$2;        shift 2;;
        -t)
            table_file=$2;   shift 2;;
        -p)
            PTF=$2;   shift 2;;
        -o)
            ptf_options_file=$2;   shift 2;;
        -v)
            internal_vrmf_table=$2;   shift 2;;
        -l)
            lbl_table=$2;        shift 2;;
        -h)
            syntax;;
    esac
done
shift                                   # Get past the '--' flag.

# Check for extra arguments
if [ $# -ne 0 ]; then
    echo "\n$cmd: ERROR: Extra arguments found at end of command line: $*"
    error_code=1
fi

# remove the infofile and the files in /tmp if the shell
#  does not complete.
trap "abort" 1 2 3 15


######################
# Validate arguments #
######################

if [ "x$PTF" = "x" ]; then
    echo "\n$cmd: ERROR: The -p ptf_number is required."
    error_code=1
fi

# Check output file has correct path, and is writable
info_dir=`dirname $info_file`
if [ "x$info_file" = "x" ]; then
    echo "\n$cmd: ERROR: The -i output infofile is required."
    error_code=1
else
    if [ ! -d $info_dir ]; then
        echo "\n$cmd: ERROR: Invalid infofile file path: `dirname $info_file`"
        error_code=1
    else
        # Make the output file path absolute
        cd `dirname $info_file`
        info_file=`pwd`/`basename $info_file`
        cd $START_DIR
        # check if output file exist and is writable
        if [ -f $info_file ]; then
            if [ ! -w $info_file ]; then
                echo "\n$cmd: ERROR: Info file: $info_file is not writeable"
                error_code=1
            fi
        else
            #check to make sure file name was given
            #the user could have entered just the directory name such as ./
            file_info=`basename $info_file`
            if [ "x$file_info" = "x" ]; then
                echo "\n$cmd: ERROR: name of output file has only the directory part"
                echo "$cmd: ERROR: the file name is blank"
                error_code=1
            else
                #if output file does not exist create file
                > $info_file
                if [ ! -f $info_file ]; then
                    echo "\n$cmd: ERROR: Info file: $info_file could not be created"
                    error_code=1
                fi
            fi
        fi
    fi
fi

# See if ptf.options file is accessable.
if [ "x$ptf_options_file" = "x" ]; then
    echo "\n$cmd: ERROR: The -o ptf.options file is required."
    error_code=1
else
    cd `dirname $ptf_options_file`
    ptf_options_file=`pwd`/`basename $ptf_options_file`
    cd $START_DIR
    if [ ! -r "$ptf_options_file" ]; then
          echo "\n$cmd: ERROR: Cannot read ptf.options file: $ptf_options_file"
          error_code=1
    fi
fi

# See if internal vrmf table file is accessable.
if [ "x$internal_vrmf_table" = "x" ]; then
    echo "\n$cmd: ERROR: The -v internal vrmf table file is required."
    error_code=1
else
    cd `dirname $internal_vrmf_table`
    internal_vrmf_table=`pwd`/`basename $internal_vrmf_table`
    cd $START_DIR
    if [ ! -s "$internal_vrmf_table" ]; then
        echo "\n$cmd: ERROR: internal vrmf table: $internal_vrmf_table does not exist or has size of zero"
        error_code=1
    else
        if [ ! -r "$internal_vrmf_table" ]; then
              echo "\n$cmd: ERROR: Cannot read internal vrmf table file: $internal_vrmf_table"
              error_code=1
        fi
    fi
fi

# See if table file is accessable.
if [ "x$table_file" = "x" ]; then
    echo "\n$cmd: ERROR: The -t table file is required."
    error_code=1
else
    cd `dirname $table_file`
    table_file=`pwd`/`basename $table_file`
    cd $START_DIR
    if [ ! -s "$table_file" ]; then
        echo "\n$cmd: ERROR: Table file: $table_file does not exist or has size of zero"
        error_code=1
    else
        if [ ! -r "$table_file" ]; then
            echo "\n$cmd: ERROR: Cannot read table file: $table_file"
            error_code=1
        fi
    fi
fi

# See if the label table file is accessable.
if [ "x$lbl_table" = "x" ]; then
    echo "\n$cmd: ERROR: The -p label table is required."
    error_code=1
else
    cd `dirname $lbl_table`
    lbl_table=`pwd`/`basename $lbl_table`
    cd $START_DIR
    if [ ! -s "$lbl_table" ]; then
        echo "\n$cmd: ERROR: Label table: $lbl_table does not exist or has size of zero"
        error_code=1
    else
        if [ ! -r "$lbl_table" ]; then
            echo "\n$cmd: ERROR: Cannot read label table: $lbl_table"
            error_code=1
        fi
    fi
fi

# See if tar file is accessable.
if [ "x$tar_file" != "x" ]; then
    cd `dirname $tar_file`
    tar_file=`pwd`/`basename $tar_file`
    cd $START_DIR
    if [ ! -s "$tar_file" ]; then
        echo "\n$cmd: ERROR: tar file: $tar_file does not exist or has size of zero"
        error_code=1
    else
        if [ ! -r "$tar_file" ]; then
            echo "\n$cmd: ERROR: Cannot read tar file: $tar_file"
            error_code=1
        fi
    fi
fi

# See if bff file path is accessable.
if [ "x$bff_file" != "x" ]; then
    cd `dirname $bff_file`
    bff_file=`pwd`/`basename $bff_file`
    cd $START_DIR
    if [ ! -s "$bff_file" ]; then
        echo "\n$cmd: ERROR: bff file: $bff_file does not exist or has size of zero"
        error_code=1
    else
        if [ ! -r "$bff_file" ]; then
            echo "\n$cmd: ERROR: Cannot read bff file: $bff_file"
            error_code=1
        else
            backup_pattern="0000000  004400"
            od "$bff_file" | head -1 | grep "^$backup_pattern" > /dev/null
            if [ $? -ne 0 ]; then
                echo "\n$cmd: ERROR: $bff_file is not a backup file."
                error_code=1
            fi
        fi
    fi
fi

# Check tar or bff paramter given
if [ "x$tar_file" = "x" -a "x$bff_file" = "x" ]; then
    echo "\n$cmd: ERROR: -b or -r option must be specified"
    error_code=1
fi

# Check to see if both are given
if [ "x$tar_file" != "x" -a "x$bff_file" != "x" ]; then
    echo "\n$cmd: ERROR: -b and -r must be specified exclusively from each other"
    error_code=1
fi

# Print syntax if error has occured
if [ "$error_code" -ne 0 ]; then
    syntax
fi

# Create temporary directory
if [ "$BLDTMP" = "" ]; then
    BLDTMP="/tmp"
fi
WORK=$BLDTMP/gen_infofile_work.$$
rm -f $WORK
if [ -d $WORK ]; then
    echo "\n$cmd: ERROR: The temporary work directory $WORK could not be removed"
    abort
fi

# Create the work directory
mkdir $WORK
if [ $? != 0 ]; then
    echo "\n$cmd: ERROR: The temporary work directory $WORK could not be created"
    abort
fi

# chmod added to make sure that the directory can be overwritten in the future
chmod 777 $WORK

cd $WORK

# Extract 'lpp_name' from tar or bff, depending on input
if [ "x$bff_file" != "x" ]; then
    restore -xqf${bff_file} ./lpp_name > /dev/null 2>&1
else
    tar -xf${tar_file} ./lpp_name > /dev/null 2>&1
fi

if [ ! -r lpp_name ]; then
    echo "\n$cmd: ERROR: lpp_name file could not be extracted from ${tar_file}${bff_file}"
    abort
fi

# From the lpp_name, extract the Media Type field (I, S, O, ML, SF, or SR)
# which is in the 1st record 3rd field.
mt=`awk 'NR == 1 {print $3; exit}' < lpp_name`
case "$mt" in
    I|S|O|ML|SF|SR) ;;
    *) echo "\n$cmd: ERROR: Incorrect media type field in the lpp_name file"
        abort;;
esac

# From lpp_name, extract the fileset name. 2nd record, 1st field
FILESET_NAME=`cat lpp_name | awk ' NR == 2 { print $1; exit } '`

#Extract the Product Description text from the lpp_name
read_lppname_desc_field
DESC=`cat $WORK/desc_temp | awk 'NR == 1 '`

# Extract 'liblpp.a' from tar or bff, depending on input.
if [ "x$bff_file" != "x" ]; then
    restore -Tqf$bff_file 2>/dev/null | grep "liblpp.a$" > liblpp.list
    cat liblpp.list 2>/dev/null | 
    while read liblpp; do
        restore -xqf$bff_file $liblpp > /dev/null 2>&1
    done
else
  tar -tf$tar_file | grep "liblpp.a$" > liblpp.list
  cat liblpp.list 2>/dev/null | 
  while read liblpp; do
      tar -xf$tar_file $liblpp
  done
fi

# Extract 'lpp.doc', 'fixdata'and 'productid' files
cat liblpp.list 2>/dev/null | while read liblpp; do
    ar x $liblpp lpp.doc ${FILESET_NAME}.fixdata productid 2>/dev/null
done

# Initialize error code
error_code=0

# Check if fixdata and productid were extracted
if [ ! -r ${FILESET_NAME}.fixdata ]; then
    echo "\n$cmd: ERROR: ${FILESET_NAME}.fixdata file could not be extracted from liblpp.a."
# PATCH FOR CUM
#    error_code=1
fi

# Must be able to read productid
if [ ! -r productid ]; then
    echo "\n$cmd: ERROR: productid file could not be extracted from liblpp.a"
    error_code=1
fi

# See if either of the two previous files were not found
if [ $error_code != 0 ]; then
    abort
fi

# Generate requisite information from the lpp_name file
get_requisite_info

# Write infofile through the date line.
echo "***************************************************" >> infofile.$PTF
echo "** $PTF FILE"        >> infofile.$PTF
echo "**"                  >> infofile.$PTF     # Purty stuff.
dd=`date +%d`;  mm=`date +%m`; yy=`date +%y`
echo "** DATE = $mm/$dd/$yy" >> infofile.$PTF   # Today's date on date line.

# Get the default id from the table and store in id.
id=""
grep -y -e "DEFAULT_ID" $table_file > $BLDTMP/def_id 
read com comp id < $BLDTMP/def_id

# Look for the compid in the table file and put into cid
compid=`awk ' { print $2 } ' productid |       # comp-ID is 2nd field
         sed -e "s/-//g" `                     # take out minus sign
cid=""

cids=`grep -e "$FILESET_NAME:$compid:" $table_file | awk -F":" '{print $6}'`
for cid in $cids; do
    if [ "$cid" != "" ]; then
        id="$cid"
    fi
done

rm -f $BLDTMP/def_id

# Append phone, contact, user, etc. from table file.
#   strip out lines starting with '#' as comments
cat $table_file  |
grep "^** " | grep -v "DEFAULT_ID" | sed -e "s/CT_ID/$id/" >> infofile.$PTF
echo "**"                  >> infofile.$PTF     # More purty stuff.
echo "***************************************************" >> infofile.$PTF

###########################################################################
###########################################################################
#########   obtain the apar information from the internal vrmf ############
#########   table and the fixdata stanza file                  ############
###########################################################################
###########################################################################
# get the apars for this PTF from the internal vrmf table
# and put the apars in the aparsfile file
line=`grep "$PTF" $internal_vrmf_table`

echo $line | awk '
NR > 0 {
 # apars start in the 4th field
 for (i=4;i<=NF;i++)
 {
     print $i
 }
}' | sort > aparsfile

# for each apar in the aparsfile
# get the stanza from the fixdata file and
# put the information into the info file
typeset dummyAPARUsed=""
aparFlag="n";                                   # Begin with new paragraph.
cat aparsfile |
while read APAR
do 
    #-----------------------------------------------------------
    # If this APAR is not "real" (ie. it is a keyword),
    #    then continue with the next APAR in the file.
    # NOTE: the prefix is hardcoded to the AIX prefix of "IX"
    #       IF your prefix is different, you will need to 
    #       change it.
    #-----------------------------------------------------------
    if [[ $(echo ${APAR} | cut -c1-2) != "IX" ]]
    then
	continue
    fi

# CUMPATCH
if [[ ${APAR} = "IX50489" && ${FILESET_NAME} != bos.rte.install ]]
then continue
fi

    aparFlag="y";                                   # Begin with new paragraph.
    typeset -i fsCnt=0
    getstanza $APAR $WORK/${FILESET_NAME}.fixdata aparinfo
    cat aparinfo |
    while read -r field equals value
    do
        if [ $field = "name" ]
        then
            # get the apar number
            apar=$value
        elif [ $field = "abstract" ]
        then
            # get the abstract
            getvalue
            abstract=$fieldvalue
        elif [ $field = "symptom" ]
        then   
            # get the symptom
            getvalue
            symptom=$fieldvalue
        elif [ $field = "filesets" ]
        then   
	    #-----------------------------------------------------------
            # need to see if there are multiple fileset needed to
            # fix this APAR.
	    # 
	    # MOST of this code below may be removed.
	    # It was added to work around Boulder's order by
	    # APAR problem by req'ing the PTFs together in
	    # the CCSS infofile (not in the toc).  This
	    # won't work for PMP's.  The temp file that
	    # is created here is ignored later on.
	    #
	    # However the fsCnt variable is set here
	    # to count filesets.  Don't remove it later
	    # without implementing an alternative method!!
	    #-----------------------------------------------------------
            tempval=${value%\\n\\} 
            tempval=${tempval#\"}  

            echo $tempval | IFS=": " read fileset vrmf
            ptf=`lookup_ptf $fileset $vrmf`
            # if the option and vrmf were not found in the internal vrmf table 
            # or in the ptfoptions file
            if [ "$ptf" = "" ]
            then
                echo "\n$cmd: ERROR1: ifreq option: '$fileset' '$vrmf' was not found in "
                echo "        the files: $ptf_options_file or $internal_vrmf_table."
                abort
            else
                # check to make sure not to add the current ptf being
                # generated as an ifreq
                if [ $ptf != $PTF ]
                then
                    print $ptf $fileset $vrmf >> ifreq_temp
                fi
            fi
	    fsCnt=1                    #if we are here, then we have 1 fileset
            done=false
            while [ $done = "false" ]
            do
                read -r value
                echo $value | grep \"$ > /dev/null 2>&1
                # if the value ends with a quote
                if [ $? = 0 ]
                then
                    done=true
                else
		    fsCnt=$fsCnt+1             # Got another fileset.
                    # remove the new line and continuation character
                    tempval=${value%\\n\\}
                    echo $tempval | IFS=": " read fileset vrmf
		    ptf=`lookup_ptf $fileset $vrmf`
                    # if the option and vrmf were not found in the internal
                    # vrmf table or ptfoptions file
                    if [ "$ptf" = "" ]
                    then
                        echo "\n$cmd: ERROR2: ifreq option: '$fileset' '$vrmf' was not found in "
                        echo "        the files: $ptf_options_file or $internal_vrmf_table."
                        abort
                    else
                        # check to make sure not to add the current ptf being
                        # generated as an ifreq
                        if [ $ptf != $PTF ]
                        then
                            print $ptf $fileset $vrmf >> ifreq_temp
                        fi
                    fi
                fi
            done
        fi
    done
    #----------------------------------------------------------
    # Append APARTXT to first line of every APAR number
    # print the apar information to the infofile
    # NOTE that if this APAR is fixed in multiple filesets,
    #      then we want to put dummy APAR in the infofile.
    #      A Boulder only packaging PTF (IPP) will be used 
    #      to tie the PTFs together.  Only the IPP will
    #      identify the APAR as being fixed in that PTF
    #      so that customers can only order 1 PTF to get
    #      the entire fix.  It also works around Boulder's
    #      order-by-apar limitation that multiple PTFs in
    #      same compId can be shipped when ordering by APAR.
    #      BOS has several filesets and it is quite likely
    #      that multiple PTFs in the BOS compId will be 
    #      required to fix some APARs.
    #----------------------------------------------------------
    if [[ $fsCnt -eq 1 ]]
    then
	print "APARTXT $apar = $abstract" >> infofile.$PTF
	print -n "$symptom" >> infofile.$PTF
	
    elif [[ -z $dummyAPARUsed ]]
    then
	dummyAPARUsed="Dummy in infofile"
	print "APARTXT IX46187 = Place Holder APAR number" >>infofile.$PTF
	print " Place holder APAR number" >>infofile.$PTF
    fi
done

if [ "$aparFlag" = "y" ]; then                  # Were apars read?
    echo "ENDSET" >> infofile.$PTF              # End apar set
fi
###########################################################################
###########################################################################
########   end of fixdata processing ######################################
###########################################################################
###########################################################################

###########################################################################
###########################################################################
########    get the version information for this fileset ##################
###########################################################################
###########################################################################
# Version
version=`grep -e "$FILESET_NAME:$compid:" $table_file | awk -F":" '{print $4}'`

# Append the COMPID line to infofile.
echo "COMPID FOR PTF $PTF = $compid,$version" >> infofile.$PTF

# Output the PRODID field
prodid=`echo $compid | cut -c1-7`

# Boulder reports that new release(3.0) of CCSS reduces the product
# description to 20 characters. Please see defect 124530.
# max length of descritpion text is 20 char
desc=`echo $DESC | cut -c1-20`
echo "PRODID FOR PTF $PTF = $prodid,$desc" >> infofile.$PTF

###########################################################################
###########################################################################
########     prereqs  processing                      #####################
###########################################################################
###########################################################################
# From lpp_name, extract the vrmf level - 2nd record, 2nd field
VRMF_LEVEL=`cat lpp_name | awk ' NR == 2 { print $2; exit } '`

# break out the version, release, mod, and fix levels of the current ptf
echo $VRMF_LEVEL | awk -F"." '{print $1, $2, $3, $4}' | read V R M F

# If the current fix level is greater than 0 then add an implicit prereq
# for this fileset at its zero-fix-level vrmf.
if [[ $F > 0 ]]
then
	echo "*prereq $FILESET_NAME $V.$R.$M.0" >> req_temp
fi

# Scan req file for prereq ptfs 
> prereq_info
prereq_vrmf=""
cat req_temp 2> /dev/null |
while read reqtype option junk vrmf
do
    vrmf=${vrmf:-$junk}
    if [ $reqtype = "*prereq" ]
    then
	ptf=`lookup_ptf $option $vrmf`

        # if the option and vrmf were not found in the internal vrmf table 
        if [ "$ptf" = "" ]
        then
          if [[ $option != $FILESET_NAME ]]
	  then
              echo "\n$cmd: ERROR: prereq option: $option $vrmf was not found in "
              echo "        the files: $ptf_options_file or $internal_vrmf_table."
              abort
	  fi
        else
            print "$option $ptf" >> prereq_info
        fi
    fi
done  

> prereq_ptfs
cat prereq_info 2> /dev/null | 
while read option prereq_ptf
do
    prod_id=""
    prod_id=`cat $table_file   |
             grep "^$option:"    |
             head -1              |
             awk -F: '{print $2}' `
    if [ "x$prod_id" = "x" ]; then
        echo "\n$cmd: ERROR: prereq option: $option not found in compids.table file  "
        abort
    fi
    echo "$prereq_ptf.$prod_id" >> prereq_ptfs
done   

# Output prereq ptfs found above into infofile
ptf_type="PREREQ"
ptfs_file="prereq_ptfs"
output_ptfs

###########################################################################
###########################################################################
########      coreq   processing                      #####################
###########################################################################
###########################################################################
# Scan req file for coreq ptfs
> coreq_info
cat req_temp 2> /dev/null |
while read reqtype option junk vrmf
do
    vrmf=${vrmf:-$junk}
    if [ $reqtype = "*coreq" ]
    then
	ptf=`lookup_ptf $option $vrmf`

        # if the option and vrmf were not found in the internal vrmf table 
        if [ "$ptf" = "" ]
        then
            echo "\n$cmd: ERROR: coreq option: $option $vrmf was not found in "
            echo "        the files: $ptf_options_file or $internal_vrmf_table."
            abort
        else
            # if the ptf is not listed in the prereq list, then add it to the
            # coreq list
            grep "$ptf" prereq_ptfs >/dev/null 2>&1
            if [ $? -ne 0 ]; then
                print "$option $ptf" >> coreq_info
            fi
        fi
    fi
done  

> coreq_ptfs 
cat coreq_info 2> /dev/null | 
while read option coreq_ptf 
do
    prod_id=""
    prod_id=`cat $table_file   |
             grep "^$option:"    |
             head -1              |
             awk -F: '{print $2}' `
    if [ "x$prod_id" = "x" ]; then
        echo "\n$cmd: ERROR: coreq option: $option not found in compids.table file  "
        abort
    fi
    echo "$coreq_ptf.$prod_id" >> coreq_ptfs 
done   
rm -f coreq_info 

# Output coreq ptfs found above into infofile
ptf_type="COREQ"
ptfs_file="coreq_ptfs"
output_ptfs
###########################################################################
###########################################################################
########      end of coreq processing                  ####################
###########################################################################
###########################################################################

###########################################################################
###########################################################################
########             ifreq processing                  ####################
###########################################################################
###########################################################################
>ifreq_ptfs

cat req_temp 2> /dev/null |
while read reqtype option junk vrmf
do
    vrmf=${vrmf:-$junk}
    if [ $reqtype = "*ifreq" ]
    then
	ptf=`lookup_ptf $option $vrmf`

        # if the option and vrmf were not found in the internal vrmf table 
        if [ "$ptf" = "" ]
        then
            echo "\n$cmd: ERROR3: ifreq option: '$option' '$vrmf' was not found in "
            echo "        the files: $ptf_options_file or $internal_vrmf_table."
            abort
        else
            # add the ptf and its product id to the list of ifreqs
            add_ptf_to_ifreq_list
        fi
    fi
done  
#----------------------------------------------------------------
# add the ptfs that fix the same apar(s) as this ptf
#
# removing code below due to problems with creating
# monstrously large media requirements caused by 
# req'ing the PTFs together (the 412 PMP was 214 PTFs).
# The IPP solution will be the required solution.
#----------------------------------------------------------------
#cat ifreq_temp 2> /dev/null |
#while read ptf option vrmf
#do
    # add the ptf and its product id to the list of ifreqs
#    add_ptf_to_ifreq_list
#done

###########################################################################
###########################################################################
########      end of ifreq processing                  ####################
###########################################################################
###########################################################################


# Output sup ptfs line to infofile.  This will always be 'NONE' since
# there are no supersedes for 41.
> sup_ptfs
ptf_type="SUP"
ptfs_file="sup_ptfs"
output_ptfs

# Output ifreq ptfs found above into infofile
ptf_type="IFREQ"
ptfs_file="ifreq_ptfs"
output_ptfs

# Insert memo to users
if [ -r "lpp.doc" -a -s "lpp.doc" ]; then
    echo "MEMO TO USERS = " >> infofile.$PTF
    echo "PTF# $PTF" >> infofile.$PTF
    echo " " >> infofile.$PTF
    sed -e 's/^I/APAR# I/' -e 's/^i/APAR# i/' < lpp.doc >> infofile.$PTF
    echo " " >> infofile.$PTF 
    echo " ________________________________________________________________" >> infofile.$PTF
    echo " ________________________________________________________________" >> infofile.$PTF
    echo " " >> infofile.$PTF
    echo " " >> infofile.$PTF
else
    echo "MEMO TO USERS = NONE" >> infofile.$PTF
fi
echo "ENDSET" >> infofile.$PTF

# Get the LABELTXT from label table
grep "^$mt " $lbl_table | sed -e 's/.*[-]//' | sed -e 's/^[ 	]//g' > label
if [ $? -ne 0 ]; then
    echo "\n$cmd: ERROR: No label text found for media type $mt in label_text.table file"
    abort
fi
grep "^X" $lbl_table | sed -e 's/.*[-]//' | sed -e 's/^[ 	]//g' >> label

# Check to make sure that the label text was found
if [ "x`cat label`" = "x" ]; then
    echo "\n$cmd: ERROR: No label text found for lpp name: $FILESET_NAME in packaging file"
    abort
fi

# Insert LABELTXT information
echo "LABELTXT" >> infofile.$PTF
cat label >> infofile.$PTF
echo "ENDSET" >> infofile.$PTF

# Finish up
echo "***************************************************" >> infofile.$PTF

# If any line in the infofile is longer than 70 characters, then
# split that line into multiple lines each less than 70 characters long
lang=$LANG
LANG="C"; export LANG
type fformat | grep -s 'not found' > /dev/null 2>&1
rc=$?
LANG=$lang; export LANG

# If fformat is found on the system, it will be used to format the line
# which will do a word break.  If it is not there, then the line will be
# wrapped at col 70.
if [ $rc -eq 0 ]; then
    cat "infofile.$PTF" |
    awk '{ info_line = $0;                             # set info_line = inputted line
	  while ( length(info_line) > 70 )             # while line is too long
	      { print substr(info_line,1,70)           # print 1st 70 chars
	        info_line = " "substr(info_line,71)    # work now with the rest of the
	      }                                        # line starting with 71st char
	  print  info_line                             # print the last part
	 }' > outinfo.$PTF
else
    IFS="~@" 				               # Set field separator so leading
    # Convert "\" character to "#@#" because while read treats it as
    # special character
    cat "infofile.$PTF" | sed -e 's/\\/#@#/g' > "infofile.$PTF.tmp"
    mv "infofile.$PTF.tmp" "infofile.$PTF"
    cat "infofile.$PTF" | 
    while read line
    do	# blanks are kept.
        len=`expr "$line" : ".*"`
        if [ "$len" -gt 70 ]; then
	    echo "$line" | fformat -dj -l2 -r70 \
	    | sed -e 's/APARTXT[  ]*/APARTXT /' \
	    | sed -e 's/[  ]*=[  ]*/ = /' \
	    | sed -e 's/#@#/\\/g' \
	    >> outinfo."$PTF"
        else
            echo "$line" | sed -e 's/#@#/\\/g' >> outinfo."$PTF"
        fi
    done

fi
# Convert the infofile to 80 column EBCDIC file.
dd cbs=80 if="outinfo.$PTF" of="$info_file" conv=ebcdic > /dev/null 2>&1

# Graceful exit
cd $START_DIR
rm -rf $WORK
exit 0
