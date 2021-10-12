#!/bin/ksh
# @(#)10        1.18  src/bldenv/pkgtools/gen_ptf_index.sh, pkgtools, bos41J, 9523A_all 6/1/95 08:17:44
#
#   COMPONENT_NAME: PKGTOOLS
#
#   FUNCTIONS: syntax
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1992,1993
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
#------------------------------------------------------------------
# NAME: gen_ptf_index
#
# DESCRIPTION: Create ccss index file from the specified ccss files
#              in the specified directories
#
# PARAMETERS:
#       INPUT 
#         ptfapardef_table - this table contains fileset and vrmf 
#                            information for each ptf.  The format
#                            is as follows: 
#          PTF|apar|defect|lpp_name|subsystem|release|family|abstract|vrmf
#                            
#                            
#         ccss_dir_name(s) - the directories where the ccss images
#                            reside                     
#       OUTPUT 
#         index_filename - the name of the index file to be created
#
# NOTES:
#
#   Usage: gen_ptf_index -o index_filename -T ptfapardef_table 
#                      [-h|-?] ccss_dirname(s)
#
#       where,  -o index_filename   is the path and name of the index
#                                    file to be created
#               -T ptfapardef_table is the path and name of the table
#                                    that contains the mapping of the
#                                    ptf to vrmf for a fileset.
#                                    This table is only required if
#                                    there are any v4 style ptfs.
#               -ccss_dirnames are the directories in which ccss
#                                    ptf files are held
#               -h or -?           specifies to display help & exit.
#
#
# DATA STRUCTURES:none
#
# RETURNS: 1 - for error 
#-------------------------------------------------------------------

######################################################################
#    subroutines start here
######################################################################

#------------------------------------------------------------------
# NAME: syntax
#
# DESCRIPTION: called when parameters are entered incorrectly or
#              if help is requested
#
# PARAMETERS: none
#
# RETURNS: 1 - for error
#-------------------------------------------------------------------
syntax() {
    echo "Usage: gen_ptf_index -o index_filename -T ptfapardef_table"
    echo "                     ccss_dirname(s)"
    echo "                     [ -h | -? ] "
    echo ""
    echo "where,  -o index_filename   is the path and name of the index "
    echo "                              file to be created"
    echo "        -T ptfapardef_table is the path and name of the table"
    echo "                              that contains the mapping of the"
    echo "                              ptf to vrmf for a fileset."
    echo "                              This table is only required if"
    echo "                              there are any v4 style ptfs"
    echo "        ccss_dirname(s) are the directories in which"
    echo "                              ccss ptf files are held"
    echo "        -h or -?           specifies to display help & exit"

    abort
}

#------------------------------------------------------------------
# NAME: abort
#
# DESCRIPTION: called when an error occurs while creating the index 
#              file
#
# PARAMETERS: none
#
# RETURNS: 1 - for error
#-------------------------------------------------------------------
abort() {
    cd $start_dir
    rm -f $index_file > /dev/null 2>&1
    rm -rf $WORK > /dev/null 2>&1
    exit 1
}

#------------------------------------------------------------------
# NAME: create_v3_prereq_supersede
#
# DESCRIPTION: called to extract the requisite information out of 
#              the toc file for a v3 style ptf
#
# PRE CONDITIONS: toc file exists before invocation
#
# POST CONDITIONS: creates the files prereq_temp, size_temp, 
#                  and supersede_temp, which contain the 
#                  requisite and size information
#
# PARAMETERS: none
#
# RETURNS: none
#-------------------------------------------------------------------
create_v3_prereq_supersede() {
    rc=0

    #
    # empty output files
    #
    >  prereq_temp
    >  size_temp
    >  supersede_temp

    #
    # initialize destination to echo stuff to trash
    #
    target='/dev/null'

    #
    # read through the toc file placing the data
    #    in the appropriate output file:
    #               prereq_temp
    #               size_temp
    #               supersede_temp
    #
    cat toc |
    while read First_parm Rest_of_line
    do
        case "$First_parm" in

            '[' )
                # check that we are at beginning of prereq section
                if [ $target != "/dev/null" ]; then
                    echo "$cmd: invalid [ found in toc of $ptf_file"
                    rc=1
                    break
                else
                    # prereq info always follows '['
                    target='prereq_temp'
                fi
                ;;

            '%' )
                # % indicates 1 of 2 possible transitions
                #      prereq info to size info
                #        or
                #      size info to supersede
                #
                case "$target" in

                    'prereq_temp' )
                        # size info follows prereq
                        target='size_temp'
                        ;;

                    'size_temp' )
                        # supersede info follows size
                        target='supersede_temp'
                        ;;

                    * )
                        echo "$cmd: invalid % found in toc of $ptf_file"
                        rc=1
                        break
                        ;;
                esac
                ;;

            ']' )
                # check that we were in the supersede section
                if [ $target != "supersede_temp" ]; then
                    echo "$cmd: invalid ] found in toc of $ptf_file"
                    rc=1
                    break
                else
                    # all data between ] and [ is trashed
                    target='/dev/null'
                fi
                ;;

            * )
                # output all data to appropriate file
                if [ "$target" = "prereq_temp" ]; then
                    echo "$Rest_of_line" | grep "v[=><]" | grep "p=" > /dev/null 2>&1
                    if [ $? -eq 0 ]; then
                        New_rest=`echo "$Rest_of_line" |
                           sed -e 's/[     ]v.*[   ]/ /' |
                           sed -e 's/[     ]r.*[   ]/ /' |
                           sed -e 's/[     ]m.*[   ]/ /'`
                        echo "$First_parm $New_rest" >> $target
                    else
                        echo "$Rest_of_line" | grep "(" >/dev/null 2>&1
                        if [ $? -eq 0 ]; then
                            New_rest=`echo "$Rest_of_line" |
                               sed -e 's/(.*)[   ]//'`
                            echo "$First_parm $New_rest" >> $target
                        else
                            echo "$First_parm $Rest_of_line" >> $target
                        fi
                    fi
                else
                    echo "$First_parm $Rest_of_line" >> $target
                fi
                ;;
        esac
    done

    # get rid of blank lines from supersede data
    cat supersede_temp 2> /dev/null | sed -e "/^ *$/d" > new_supersede_temp
    mv new_supersede_temp supersede_temp

    # get rid of data of the form "> number {" and "}" and blank lines
    # from the requisite data
    cat prereq_temp 2> /dev/null |
        sed -e "s/>[ 0-9]*{//g" | # replace "> number {" with nothing
        sed -e "s/}//g"         | # replace "}" with nothing
        sed -e "/^ *$/d" > new_prereq_temp  # delete blank lines
    mv new_prereq_temp prereq_temp

    # set bad return code if no requisites or supersedes were found
    # (i.e. if both files are empty)
    if [ ! -s prereq_temp -a ! -s supersede_temp ]; then
        echo "$cmd: No requisites found in toc of $ptf_file"
        rc=1
    fi

    return $rc

}  # End of create_v3_prereq_supersede function

#------------------------------------------------------------------
# NAME: get_v4_requisite_info
#
# DESCRIPTION: called to extract the requisite information out of 
#              the toc file for a v4 style ptf
#
# PRE CONDITIONS: toc file exists before invocation
#
# POST CONDITIONS: creates the file req_temp which contains the 
#                  requisite information
#
# PARAMETERS: none
#
# RETURNS: none
#-------------------------------------------------------------------
get_v4_requisite_info() {
    # Create file
    >  req_temp

    # Initialize destination to echo stuff to trash
    target='/dev/null'

    # Read through the lpp_name file placing the data in the appropriate
    # file: req_temp or /dev/null
    cat toc |
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
} # End of get_v4_requisite_info function

#------------------------------------------------------------------
# NAME: add_v3_ptf_to_index_file
#
# DESCRIPTION: This function uses the requisite information from the
#              toc file to generate the information in the index file.
#
#              If an error is found in the format of the toc file, a
#              non-zero return code is set, but the program will keep
#              adding data to the index file.
#
# PRE CONDITIONS: toc and prereq_file files exist before
#                 invocation
#
# POST CONDITIONS: writes entries to the index file
#
# PARAMETERS: none
#
# NOTES: 
#
# RETURNS: 1 - for error
#-------------------------------------------------------------------
add_v3_ptf_to_index_file() {

    rc=0

    # From toc, extract the LPP_NAME & PTF number.  The format of the
    # 2nd record of toc is 'cur_lpp.eeeee vv.rr.mmmm.ffff.cur_ptf'

    cur_lpp=`head -2 toc |
             awk ' NR == 2 { print $1 } ' | # 2nd record, 1st field
             awk ' BEGIN { FS = "." }       # field separator '.'
                   { print $1 }             # print LPP_NAME(1st field)
                 '
            `

    cur_ptf=`head -2 toc |
             awk ' NR == 2 { print $2 } ' |      # 2nd record, 2nd field
             awk ' BEGIN { FS = "." }            # field separator '.'
                   { print $5 }                  # print PTF (5th field)
                 '
            `

    # parse toc into prereq and supersede sections
    create_v3_prereq_supersede || return 1

    
    # for each prerequisite type, write a line to the index file
    cat prereq_temp 2> /dev/null |
    while read req_line; do

        # reset the positional parameters ($1,$2,etc.) to the req_line
        set -- $req_line

        # check if first field
        if [ $1 = "*prereq" ]; then
            req_type="prereq"
            shift
        else
            if [ $1 = "*coreq" ]; then
                req_type="coreq"
                shift
            else
                if [ $1 = "*ifreq" ]; then
                    req_type="ifreq"
                    shift
                else
                    req_type="prereq"  # it's an assumed prereq
                fi
            fi
        fi

        # check that at least 2 fields remain
        if [ "$#" -lt "2" ]; then
            echo "$cmd: invalid data in toc of $ptf_file"
            echo "$cmd:  found in requisite section: $req_line"
            rc=1
            break
        fi

        # strip option from next field yielding lpp_name
        # (get char's before the first period)
        lpp_name=`echo "$1" | cut -d. -f1"`
        shift

        # get first 2 characters of the next field
        first_2_chars=`expr "$1" : "^\(..\)"`
 
        if [ "$first_2_chars" = "p=" ]; then
            # get the chars after "p=" (this is the ptf number)
            req_info=`echo "$1" | sed -e "s/p=//"` # replace "p="
                                                 # with nothing
        else
            # the requisite info is the rest of the line
            req_info="$*"
        fi

        # output line to index file
        echo "$cur_lpp:$cur_ptf:$req_type:$lpp_name:$req_info" \
            >> $index_file 
        if [ $? != 0 ]; then
            echo "$cmd: The directory \"`dirname $index_file`\" is full."
            echo "$cmd: The index file was not created."
            return 1
        fi
    done

    # for each supersede type, write a line to the index file
    cat supersede_temp 2> /dev/null |
    while read sup_line; do

        # reset the positional parameters ($1,$2,etc.) to the sup_line
        set -- $sup_line

        # check that their are exactly 2 fields on the line
        if [ $# != 2 ]; then
            echo "$cmd: invalid data in toc of $ptf_file"
            echo "$cmd:  found in supersede section: $sup_line"
            rc=1
            break
        fi

        # strip option from the first field yielding lpp_name
        # (get char's before the first period)
        lpp_name=`echo "$1" | cut -d. -f1"`

        # ptf is the second field
        ptf=$2

        # output line to index file
        echo "$cur_lpp:$cur_ptf:supersede:$lpp_name:$ptf" \
            >> $index_file
        if [ $? != 0 ]; then
            echo "$cmd: The directory \"`dirname $index_file`\" is full."
            echo "$cmd: The index file was not created."
            return 1
        fi
    done

    return $rc

}  # end of add_v3_ptf_to_index_file

#------------------------------------------------------------------
# NAME: add_v4_ptf_to_index_file
#
# DESCRIPTION: This function uses the requisite information from the
#              toc file and the ptf to vrmf mapping table to generate
#              the information in the index file.
#
#              If an error is found in the format of the toc file, a
#              non-zero return code is set, but the program will keep
#              adding data to the index file.
#
# PRE CONDITIONS: toc, infofile, and req_file files exist before 
#                 invocation
#
# POST CONDITIONS: writes entries to the index file
#
# PARAMETERS: none
#
# NOTES:  The toc file contains requisite information.  The format
#         of the requisite entry is:
#             req_type req_fileset (base_level_vrmf) vrmf     
#     
#         The mapping table has the following format: 
#          PTF|apar|defect|lpp_name|subsystem|release|family|abstract|vrmf
#
#         By using the req_fileset name and vrmf level, the 
#         corresponding ptf number is extracted from the mapping
#         table
#
# RETURNS: 1 - for error
#-------------------------------------------------------------------
add_v4_ptf_to_index_file() {

    rc=0

    # From toc, extract the fileset name & vrmf number.  The format of the
    # 2nd record of toc is 'cur_fileset vv.rr.mmmm.ffff ......'

    cur_fileset=`head -2 toc |
              awk ' NR == 2 { print $1 } '`       # 2nd record, 1st field

    cur_vrmf=`head -2 toc |
              awk ' NR == 2 { print $2 } '`       # 2nd record, 2nd field

    # from the infofile, get the ptf number
    cur_ptf=`head -2 infofile |
              awk ' NR == 2 { print $2 } '`       # 2nd record, 2nd field

    # initialize flag for use to determine if there are any requisite or 
    # supersede ptfs
    ptfs_found="n"

    # Generate requisite information from the toc file
    get_v4_requisite_info

    # for each requisite, write a line to the index file
    cat req_temp 2> /dev/null |
    while read reqtype req_fileset junk req_vrmf
    do
        req_vrmf=${req_vrmf:-$junk}

        # check first field
        if [ $reqtype = "*prereq" ]; then
            req_type="prereq"
        else
            if [ $reqtype = "*coreq" ]; then
                req_type="coreq"
            elif [ $reqtype = "*ifreq" ]; then
                req_type="ifreq"
            fi
        fi

        # check that at least 2 fields remain
        if [[ -z "$req_fileset"  ||  -z "$req_vrmf" ]]; then
            echo "$cmd: invalid data in toc of $ptf_file"
            echo "$cmd:  found in requisite section: $req_line"
            rc=1
            break
        fi

        # if the fileset has an explicit prereq on itself
        if [ $cur_fileset = $req_fileset  -a  $req_type = "prereq" ]
        then
            # save the vrmf number of the fileset
            stop_vrmf=$req_vrmf
	    # generate the zero-th fix level vrmf
	    echo $req_vrmf | awk -F"." '{print $1, $2, $3}' | read V R M
	    zero_vrmf="${V}.${R}.${M}.0"

            # get the ptf number from the mapping table
            req_ptf=`grep "$req_fileset|" $mapping_table | \
                 fgrep "|$zero_vrmf" | cut -f1 -d "|" | head -1`

            if [ -n "$req_ptf" ]
            then 
                # set flag to indicate that at least one requisite was found
                ptfs_found="y"

                # output line to index file
                echo "$cur_fileset:$cur_ptf:$req_type:$req_fileset:$req_ptf" >> $index_file
                if [ $? != 0 ]; then
                    echo "$cmd: The directory \"`dirname $index_file`\" is full."
                    echo "$cmd: The index file was not created."
                    return 1
                fi
            fi
        else    
            # get the ptf number from the mapping table
            req_ptf=`grep "$req_fileset|" $mapping_table | fgrep "|$req_vrmf" | \
                 cut -f1 -d "|" | head -1`
        
            # if a ptf was found for this fileset at this vrmf level
            if [ -n "$req_ptf" ]
            then
                # set flag to indicate that at least one requisite was found
                ptfs_found="y"

                # output line to index file
                echo "$cur_fileset:$cur_ptf:$req_type:$req_fileset:$req_ptf" >> $index_file
                if [ $? != 0 ]; then
                    echo "$cmd: The directory \"`dirname $index_file`\" is full."
                    echo "$cmd: The index file was not created."
                    return 1
                fi
            else
                echo "$cmd: The ptf \"$cur_ptf\" for fileset \"$cur_fileset\" "
                echo "$cmd: requisites the fileset \"$req_fileset\" with vrmf"
                echo "$cmd: \"$req_vrmf\" which does not have an entry in the "
                echo "$cmd: file \"$mapping_table\". "
                return 1
            fi
        fi
    done

    # Check if we need to generate an implicit prereq to the zero-th 
    # vrmf fix level for this fileset.
    echo $cur_vrmf | awk -F"." '{print $1, $2, $3, $4}' | read V R M F
    if [ $F -gt 0 ]
    then
	base_vrmf="${V}.${R}.${M}.0"
	green_light="yes"

	# If the fileset had an explicit prereq to itself at a level
	# that is greater than the implicit vrmf level, then keep the
	# explicit prereq.
	if [ -n "$stop_vrmf" ]; then
	    # For the integer comparison below, make sure that base_vrmf and
	    # stop_vrmf are expanded to the full vv.rr.mmmm.ffff format.
	    base_vrmf_int=`echo $base_vrmf | awk -F"." '{printf "%02d%02d%04d%04d", $1, $2, $3, $4}'`
            stop_vrmf_int=`echo $stop_vrmf | awk -F"." '{printf "%02d%02d%04d%04d", $1, $2, $3, $4}'` 
	    if [ $stop_vrmf_int -gt $base_vrmf_int ]; then
		# don't do implicit prereq
		green_light="no"
 	    fi
	fi
		
	if [ $green_light ]; then
            # get the ptf number from the mapping table at the base vrmf level
            req_ptf=`grep "$cur_fileset|" $mapping_table | fgrep "|$base_vrmf" | \
                 cut -f1 -d "|" | head -1`
	    if [ -n "$req_ptf" ]; then
	        ptfs_found="y"
            	# output line to index file
	    	req_type="prereq"
            	echo "$cur_fileset:$cur_ptf:$req_type:$cur_fileset:$req_ptf" >> $index_file
            	if [ $? != 0 ]; then
                    echo "$cmd: The directory \"`dirname $index_file`\" is full."
                    echo "$cmd: The index file was not created."
                    return 1
		fi
            fi
	fi
    fi

    #
    # add supersede ptfs
    #

    # get the fix and maintenance levels of the ptf
    cur_fix_level=`echo $cur_vrmf | cut -f4 -d "."`
    cur_maint_level=`echo $cur_vrmf | cut -f3 -d "."`

    # if the fileset had an explicit prereq on itself
    if [ X$stop_vrmf != X ]
    then
        # set the stop fix and maintenance levels to the prereqed levels
        stop_fix_level=`echo $stop_vrmf | cut -f4 -d "."`
        stop_maint_level=`echo $stop_vrmf | cut -f3 -d "."`
    # if the ptf is a cum    
    elif [ $cur_fix_level = 0 ]
    then
        # set the stop fix and maintenance levels to the image levels
        stop_fix_level=0
        stop_maint_level=0
    else
        # set the stop fix and maintenance levels to the cum levels
        stop_fix_level=0
        stop_maint_level=$cur_maint_level
    fi

    # for each occurence of the fileset in the mapping file
    grep "$cur_fileset|" $mapping_table |
    while read line
    do
        ptf=${line%%\|*}
        vrmf=`echo $line | cut -f9 -d "|"`
        if [ -z "$vrmf" ]
        then
            echo "$cmd: The file \"$mapping_table\" is bad."
            echo "$cmd: The entry \"$line\" is missing the vrmf."
            return 1
        fi
            
        fix_level=`echo $vrmf | cut -f4 -d "."`
        maint_level=`echo $vrmf | cut -f3 -d "."`

        # determine if this ptf supersedes the current ptf
        if [ "$stop_fix_level" -lt "$fix_level" -a \
             "$fix_level" -lt "$cur_fix_level"  -a \
             "$stop_maint_level" -le "$maint_level" -a \
             "$maint_level" -le "$cur_maint_level " ]
        then
            # set flag to indicate that a least one requisite was found
            ptfs_found="y"
            
            # output supersede to index file
            echo "$cur_fileset:$cur_ptf:supersede:$cur_fileset:$ptf" \
            >> $index_file
            if [ $? != 0 ]; then
                echo "$cmd: The directory \"`dirname $index_file`\" is full."
                echo "$cmd: The index file was not created."
                return 1
            fi
        fi
    done

    # if the ptf does not have any requisite or supersede ptfs
    if [ $ptfs_found = "n" ]
    then 

        # if the fileset had an explicit prereq on itself
        if [ -n "$stop_vrmf" ]
        then
            cur_ver_level=`echo $stop_vrmf | cut -f1 -d "."`
            cur_rel_level=`echo $stop_vrmf | cut -f2 -d "."`
            cur_maint_level=`echo $stop_vrmf | cut -f3 -d "."`
        else
            cur_ver_level=`echo $cur_vrmf | cut -f1 -d "."`
            cur_rel_level=`echo $cur_vrmf | cut -f2 -d "."`
        fi

        # write an entry to the index file for the fileset base level
        echo "$cur_fileset:$cur_ptf:prereq:$cur_fileset:v=$cur_ver_level r=$cur_rel_level m=$cur_maint_level" >> $index_file
        if [ $? != 0 ]; then
            echo "$cmd: The directory \"`dirname $index_file`\" is full."
            echo "$cmd: The index file was not created."
            return 1
        fi
    fi

    return $rc

}  # end of add_v4_ptf_to_index_file

#######################################################################
#######################################################################
#############   Beginning of generate ptf index #######################
#######################################################################
#######################################################################

# Initialize variables.

help_request="n"
index_file=""
WORK=""
err_code=0
start_dir=`pwd`
cmd=`basename $0`

# Check for no parameters
if [ $# -eq 0 ]; then
    syntax
fi

#  Check the command line
set -- `getopt "o:T:h?" $*`
if [ $? != 0 ]; then   # getopt failed because of a bad parameter flag
    syntax
fi

while [ $1 != "--" ]; do                # Set vars, based on params.
    case $1 in
        -o)
            index_file=$2;              shift 2;;
        -T)
            mapping_table=$2;           shift 2;;
        -[?h])
            help_request="y";           shift;;
    esac
done
shift                                   # Get past the '--' flag.

# Validate the parameters

if [ "$help_request" = "y" ]; then
    syntax
fi

# remove the index file and the files in /tmp if the shell 
#  does not complete.
trap "abort" 1 2 3 15

# Create temporary work space with unique directory name
# using process id and current date and time
if [ "x$BLDTMP" = "x" ]; then
    BLDTMP="/tmp"
fi
WORK="$BLDTMP/${cmd}_work.$$"
rm -rf $WORK
mkdir $WORK
if [ $? != 0 ]; then
    echo "$cmd: Could not make work directory $WORK"
    echo "$cmd: aborting process"
    abort
fi
chmod 777 $WORK

# Check ptf to vrmf mapping table
if [ "$mapping_table" != "" ]; then
    cd `dirname $mapping_table`
    mapping_table=`pwd`/`basename $mapping_table`
    cd $start_dir
    if [ ! -s "$mapping_table" ]; then
        echo "\n$cmd: ERROR: mapping table: $mapping_table does not exist or has size of zero"
        err_code=1
    else
        if [ ! -r "$mapping_table" ]; then
              echo "\n$cmd: ERROR: Cannot read mapping table file: $mapping_table"
              err_code=1
        fi
    fi
fi

# Check output index file
if [ "$index_file" = "" ]; then
    echo "$cmd: the -o output index filename is required"
    err_code=1
else
    cd $start_dir
    > "$index_file"
    if [ $? != 0 ]; then   # Was index_file successfully created?
        # No error message necessary as '>' already gave one
        err_code=1
    else
        # make index file accessible to everyone
        chmod 777 $index_file
        # make index file have an absolute path
        cd `dirname $index_file`
        index_file="`pwd`/`basename $index_file`"
    fi
fi

# Check ccss directories
if [ $# -eq 0 ]; then
    echo "$cmd: at least one ccss_dirname is required"
    err_code=1
else
    ccss_dirs=""
    for ccss_dir in $*; do
        cd $start_dir
        if [ ! -d "$ccss_dir" ]; then    # Does directory exist?
            echo "$cmd: $ccss_dir is not a directory"
            err_code=1
        else
            # check for *.ptf files and make path absolute
            cd $ccss_dir
            ccss_dir=`pwd`
            ptf_files=`basename $ccss_dir`
            ls -1 | grep "\.ptf$" 2>/dev/null > $WORK/$ptf_files
            if [ ! -s $WORK/$ptf_files ]; then
                echo "$cmd: \c"
                echo "Warning: no *.ptf files found in $ccss_dir"
            else
                ccss_dirs="$ccss_dirs $ccss_dir"
            fi
        fi
    done
fi

# exit if error was found above
if [ "$err_code" -ne 0 ]; then
    syntax
fi

# Build index file from *.ptf files in the ccss_directories
for ccss_dir in $ccss_dirs; do
    cd $ccss_dir

    # determine the ptfapardef table if it was not specified
    if [ "$mapping_table" = "" ]
    then
        # if there is a ptfapardef.constant file in the current directory
        if [ -s "ptfapardef.constant" ]
        then
            mapping_table="$ccss_dir/ptfapardef.constant"
        # else if there is a ptfapardef.constant file in the parent directory
        elif [ -s "../ptfapardef.constant" ]
        then
            mapping_table="$ccss_dir/../ptfapardef.constant"
        fi
    fi        
    
    ptf_files=`basename $ccss_dir`
    cat $WORK/$ptf_files |
    while read ptf_file
    do
        if [ -f $ptf_file ]; then
            rm -f $WORK/infofile
            rm -f $WORK/toc
            #  Extract toc and infofile entry from ptf file
            ccss_unpack -c $ccss_dir/$ptf_file -i $WORK/infofile.eb -t $WORK/toc

            if [ ! -r $WORK/infofile.eb -o ! -r $WORK/toc ]; then
                # error if no toc or infofile entry found
                echo "$cmd: Either the toc and/or infofile was not found in \c"
                echo "\"$ptf_file\" in \"$ccss_dir\" directory"
                abort
            else
                # convert the infofile from EBCDIC to acsii
                dd if=$WORK/infofile.eb of=$WORK/infofile conv=ascii cbs=80 >/dev/null 2>&1

                # cd to the work directory
                cd $WORK

                # determine if the ptf is v3 or v4 by looking in the toc file
                version=`head -1 toc | awk 'NR == 1 {print $1}'`

                if [ "$version" = "3" ]
                then
                    # add entries to the index file for requisite and 
                    # supersede ptfs
                    add_v3_ptf_to_index_file

                    # if the ptf information was not added to the index file 
                    if [ $? != 0 ]; then
                        abort
                    fi
                else
                    if [ "$mapping_table" = "" ]; then
                        echo "$cmd: the -T flag is required for processing"
                        echo "      version 4 ptfs"
                        syntax
                    fi
 
                    # add entries to the index file for requisite and 
                    # supersede ptfs
                    add_v4_ptf_to_index_file

                    # if the ptf information was not added to the index file 
                    if [ $? != 0 ]; then
                        abort
                    fi
                fi

                # cd back to the directory where the ccss images reside
                cd $ccss_dir
            fi
        fi
    done 
done

# Sort and remove duplicates
sort -u $index_file > $WORK/index
cp $WORK/index $index_file

# Graceful Exit
cd $start_dir
rm -rf $WORK
# exit $rc

#### End of gen_ptf_index #### End of gen_ptf_index ####

