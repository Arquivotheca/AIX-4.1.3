#!/bin/ksh
# @(#) 27 1.8  src/bldenv/bldtools/chksubsys/chksubsys.sh, bldprocess, bos412, GOLDA411a 1/20/94 08:41:07
#
# COMPONENT_NAME: (BLDPROCESS) BAI Build Process
#
# FUNCTIONS:     build_options_list    usage
#                		       usage_exit
#                check_diskspace       validate_level
#                cleanup               verify_cmvc
#                cleanup_all           verify_directories
#                find_temp_dir         verify_file
#                init_proc             verify_path
#                merge_leveldata       verify_var
#                proc_arguments        verify_var_multiple
#                proc_leveldata        walk_notedata
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1989,1993
# All Rights Reserved
# Licensed Materials - Property of IBM
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# NAME:          chksubsys
#
# PURPOSE:       chksubsys reads defect notes for all defects in a
#                specified level/release/set of defects and finds
#                defects with subsystem notes
#
# INPUT:         derived from CMVC, prebuild, and some
#                environment variables
#
# OPTIONS:       -h     display help information
#                -v     display source version information
#                -r     specifies CMVC release name (use with -n)
#                -n     specifies CMVC level name (use with -r)
#                -l     specifies logfile to be used
#
# ARGUMENTS:     none
#
##############################################################################


#
# -=-=-=- PROGRAM USAGE -=-=-=-
#

#
# usage - display program usage message
#
usage() {
    cmd_name="$1"

    basemessage="$cmd_name: usage: $cmd_name"
    spacemessage=`echo $basemessage | sed -e '1,$s/./ /g'`
    echo "$basemessage [-h] [-v] [-l logfile] -r release -n level"
    echo "     where:"
    echo "          -h        displays this help message and exits"
    echo "          -v        displays source version information"
    echo "          -r        specifies name of CMVC release to be"
    echo "                    searched (must be used with -n)"
    echo "          -n        specifies name of CMVC level to be"
    echo "                    searched (must be used with -r)"
    echo "          -l        specifies output logfile to be used"
    echo ""
    echo "     Checks defects in a level in a specific release for"
    echo "     note fields indicating defects marked as being in"
    echo "     specific subsystems."
    echo ""
}


#
# -=-=-=- INITIALIZATION FUNCTIONS -=-=-=-
#

#
# init_proc - set default variables for program
#
init_proc() {
    # set host variable if not previously defined
    if [ "q$HOST" = "q" ]
    then
        HOST=`uname -n`
        export HOST
    fi

    # null
    N=/dev/null

    # define default user, release, level, and logfile .....
    QUERY_USER_DEF="$CMVC_BECOME" ; QUERY_USER=""
    RELEASE_NAME_DEF="NONE" ; RELEASE_NAME=""
    LOGFILE_NAME_DEF="DEFAULT" ; LOGFILE_NAME=""
    LEVEL_NAME_DEF="NONE" ; LEVEL_NAME=""

    # get the proper temp directory to use .....
    BLDTMP_TEMP=`find_temp_dir` ; BLDTMP="$BLDTMP_TEMP"

    # temporary files
    TMPFILE0="$BLDTMP/Scksb.a$$.$HOST"
    TMPFILE1="$BLDTMP/Scksb.b$$.$HOST"
    TMPFILE2="$BLDTMP/Scksb.c$$.$HOST"
    TMPFILE3="$BLDTMP/Scksb.d$$.$HOST"
    TMPFILE4="$BLDTMP/Scksb.e$$.$HOST"
    TMPFILE5="$BLDTMP/Scksb.f$$.$HOST"
    TMPFILE6="$BLDTMP/Scksb.g$$.$HOST"

    # global temporary files .....
    NEW_ENTRIES="$TMPFILE5"

    # workfile list
    ALL_FILES="$TMPFILE0 $TMPFILE1 $TMPFILE2 $TMPFILE3"
    ALL_FILES="$ALL_FILES $TMPFILE4 $TMPFILE5 $TMPFILE6"
}


#
# proc_arguments - process argument list
#
proc_arguments() {
    argument_list="$*"

    for each_search in $argument_list
    do
        while [ "q$1" != "q" ]
        do
            case $1 in
                # Process help/version options .....
                '-r')
                    RELEASE_NAME="$RELEASE_NAME $2" ; shift 2 ;;
                '-n')
                    LEVEL_NAME="$LEVEL_NAME $2" ; shift 2 ;;
                '-l')
                    LOGFILE_NAME="$LOGFILE_NAME $2" ; shift 2 ;;
                '-h')
                    usage $CMD_NAME ; exit 0 ;;
                '-v')
                    # Display SCCS status ....
                    SCCS_CHECKEDIN="@(#)"
                    SCCS_VERSIONSTR="1.8 - 94/01/20 08:41:07"

                    sccs_threechar=`echo $SCCS_CHECKEDIN | cut -c3-3`
                    if [ "q$sccs_threechar" != "q#" ]
                    then
                         echo "$CMD_NAME: not checked in."
                    else
                         echo "$CMD_NAME: version $SCCS_VERSIONSTR."
                    fi
                    exit 0 ;;
                *)  ;;
            esac
        done
    done

    # verify the options first, then perform some setup
    # stuff .....
    RELEASE_NAME=`verify_var "CMVC release name" "$RELEASE_NAME_DEF" \
                  $RELEASE_NAME`
        [[ $? -ne 0 || "q$RELEASE_NAME" = "qNONE" ]] && usage_exit
    LEVEL_NAME=`verify_var "CMVC level name" "$LEVEL_NAME_DEF" \
                  $LEVEL_NAME`
        [[ $? -ne 0 || "q$RELEASE_NAME" = "qNONE" ]] && usage_exit
    LOGFILE_NAME=`verify_path "logfile" "f" "$LOGFILE_NAME_DEF" \
                  $LOGFILE_NAME`
        [[ $? -ne 0 ]] && usage_exit

    # grab values for things we need .....
    bldinit
    chkmaindirs create
    chkcreate $(bldlogpath)

    # make sure the user's a valid CMVC user .....
    QUERY_USER=`verify_cmvc "$QUERY_USER_DEF" $QUERY_USER`
        [[ $? -ne 0 ]] && {
            cleanup_all ; exit 1
        }

    # initialize logfile .....
    if [ "q$LOGFILE_NAME" = "qDEFAULT" ]
    then
        verify_directories $(bldlogpath)/chksubsys
        logset -c"chksubsys" -C"chksubsys" \
               -F $(bldlogpath)/chksubsys
    else
        verify_directories $LOGFILE_NAME
        logset -c"chksubsys" -C"chksubsys" \
               -F "$LOGFILE_NAME"
    fi
}


#
# -=-=-=- TEMPORARY DIRECTORY FUNCTIONS -=-=-=-
#
# Dependencies:
#     BLDTMP          contains the build temporary directory name,
#                     defaults to /tmp
#
#

#
# find_temp_dir - find the proper temporary directory to use
#
find_temp_dir() {
    # grab the directory name argument .....
    directory_base="$BLDTMP"
    directory_name=`echo $directory_base | sed -e '1,$s;/$;;g'`

    # if the directory name's not set, use /tmp, otherwise,
    # use /tmp if we can't find the specified directory .....
    if [ "q$directory_name" = "q" ]
    then
        echo "/tmp"
    else
        # make sure the directory's writeable, otherwise, use
        # /tmp instead .....
        direct_writeable="no"
        if [ -w "$directory_name" ]
        then
            if [ -d "$directory_name" ]
            then
                direct_writeable="yes"
            fi
        fi

        # if the directory was writeable, echo the directory
        # specified, otherwise, use /tmp .....
        if [ "q$direct_writeable" = "qyes" ]
        then
            echo "$directory_name"
        else
            echo "/tmp"
        fi
    fi
    return 0
}


#
# -=-=-=- CLEANUP ROUTINES -=-=-=-
#
# Dependencies:
#     ALL_FILES            contains a list of files to cleanup on exit
#

#
# cleanup - cleanup workfiles
#
cleanup() {
    /bin/rm -rf $ALL_FILES 2>/dev/null >/dev/null
}



#
# -=-=-=- VERIFICATION ROUTINES -=-=-=-
#
# Dependencies:
#     CMD_NAME             contains the command name of this tool
#


#
# check_diskspace - make sure we've enough space and inodes to do
#                   the work required
#
# usage: check_diskspace space-free-kb inodes-free dir-list
#
check_diskspace() {
    if [ $# -lt 3 ]
    then
        echo "usage: `basename $0` space-free-kb inodes-free dir-list" >&2
        echo "           checks to make sure there is at least a certain" >&2
        echo "           amount of disk space and inodes free" >&2
        return 1
    fi

    # fetch the proper arguments .....
    space_free="$1" ; inodes_free="$2" ; shift 2
    directory_list="$*" ; cmd_namebase=`basename $CMD_NAME`

    for each_dir in $directory_list
    do
        # see if we can get the disk space stats first .....
        spaceline=`/bin/df -i -v $each_dir | tail -1`
        if [ "q$spaceline" = "q" ]
        then
            a1="ERROR: cannot check free disk space on $each_dir"
            echo "$cmd_namebase: $a1"
            return 1
        fi

        # now split disk space line into the pieces we need .....
        free_kb=`echo $spaceline | awk '{ print $4 }'`
        free_ino=`echo $spaceline | awk '{ print $7 }'`

        # check free disk space .....
        if [ $free_kb -lt $space_free ]
        then
            a1="ERROR: must be at least ${space_free}k free in $each_dir"
            echo "$cmd_namebase: $a1"
            return 1
        fi

        # check free disk inode space .....
        if [ "q$free_ino" != "q-" ]
        then
            if [ $free_ino -lt $inodes_free ]
            then
                a1="ERROR: must be at least ${inodes_free} inodes"
                echo "$cmd_namebase: $a1 free in $each_dir"
                return 1
            fi
        fi
    done
}


#
# verify_file - expand a filename with a relative path to a filename
#               with a full path, make sure it exists, and echo
#               the full path and return by exit
#
verify_file() {
    if [ $# -lt 3 ]
    then
        echo "usage: `basename $0` option-name type-path path-default" >&2
        echo "                     file-dir-pathname" >&2
        echo "           verifies that the file option specified is not" >&2
        echo "           specified multiple times and the file exists" >&2
        exit 1
    fi

    # fetch the proper arguments .....
    option_name="$1" ; type_path="$2" ; path_default="$3" ; shift 3
    path_list="$*" ; cmd_namedisp=`basename $CMD_NAME`

    # Resolve which option is to be used .....
    var_path=`verify_var "$option_name" "$path_default" $path_list`
    [[ $? -ne 0 ]] && exit 1

    p_savedir=`pwd`
    # get file/directory part of name .....
    if [ "q$type_path" != "qd" ]
    then
        dirpart=`dirname $var_path`
        filepart=`basename $var_path`
    else
        dirpart="$var_path"
    fi

    # delocalize the directory .....
    if [ "q$dirpart" = "q." ]
    then
        dirpart="$p_savedir"
    fi

    # change directory to the new directory, see if we got there,
    # and grab where it resolved to .....
    if [ -d "$dirpart" ]
    then
        if [ ! -r "$dirpart" ]
        then
            a1="ERROR: cannot read $option_name directory $dirpart"
            echo "$cmd_namedisp: $a1" >&2
            exit 1
        fi
    else
        a1="ERROR: cannot find $option_name directory $dirpart"
        echo "$cmd_namedisp: $a1" >&2
        exit 1
    fi

    # determine what type of search this is and perform a file
    # verify if required .....
    if [ "q$type_path" != "qd" ]
    then
        cd $p_savedir 2>/dev/null
        if [ -r "$dirpart/$filepart" ]
        then
            echo "$dirpart/$filepart"
            exit 0
        else
            a1="ERROR: cannot read $option_name file $dirpart/$filepart"
            echo "$cmd_namedisp: $a1" >&2
            exit 1
        fi
    else
        cd $p_savedir 2>/dev/null
        echo "$dirpart"
        exit 0
    fi

    # shouldn't get down here, but it's ok anyhow .....
    exit 0
}


#
# verify_path - expand a filename with a relative path to a filename
#               with a full path, echoing the path and returning by
#               exit
#
verify_path() {
    if [ $# -lt 3 ]
    then
        echo "usage: `basename $0` option-name type-path path-default" >&2
        echo "                     file-dir-pathname" >&2
        echo "           expands a filename to a full path but does" >&2
        echo "           not verify the existence of the file" >&2
        exit 1
    fi

    # fetch the proper arguments .....
    option_name="$1" ; type_path="$2" ; path_default="$3" ; shift 3
    path_list="$*" ; cmd_namedisp=`basename $CMD_NAME`

    # resolve which option is to be used .....
    var_path=`verify_var "$option_name" "$path_default" $path_list`
    [[ $? -ne 0 ]] && exit 1

    # return default if found .....
    if [ "q$var_path" = "qDEFAULT" ]
    then
        echo "DEFAULT"
        exit 0
    fi

    # bail out if no default .....
    if [ "q$var_path" = "qNONE" ]
    then
        a1="ERROR: option $option_name has no default!"
        echo "$cmd_namedisp: $a1" >&2
        exit 1
    fi

    p_savedir=`pwd`
    # get file/directory part of name
    if [ "q$type_path" != "qd" ]
    then
        dirpart=`dirname $var_path`
        filepart=`basename $var_path`
    else
        dirpart="$var_path"
    fi

    # delocalize the directory .....
    if [ "q$dirpart" = "q." ]
    then
        dirpart="$p_savedir"
    fi

    # change directory to the new directory, see if we got there,
    # and grab where it resolved to .....
    if [ -d "$dirpart" ]
    then
        if [ ! -r "$dirpart" ]
        then
            a1="ERROR: cannot read $option_name directory $dirpart"
            echo "$cmd_namedisp: $a1" >&2
            exit 1
        fi
    else
        a1="ERROR: cannot find $option_name directory $dirpart"
        echo "$cmd_namedisp: $a1" >&2
        exit 1
    fi

    # determine what type of search this is and perform a file
    # verify if required .....
    if [ "q$type_path" != "qd" ]
    then
        echo "$dirpart/$filepart"
    else
        echo "$dirpart"
    fi

    # change directory back to where we're supposed to be .....
    cd $p_savedir 2>/dev/null
    exit 0
}


#
# verify_cmvc - verify a user with cmvc
#
verify_cmvc() {
    if [ $# -lt 1 ]
    then
        echo "usage: `basename $0` default-ID defined-ID" >&2
        echo "           verifies that the CMVC ID specified is valid" >&2
        exit 1
    fi

    # fetch the proper arguments .....
    cmvc_default="$1" ; shift ; cmvc_list="$*"
    cmd_namedisp=`basename $CMD_NAME`

    # resolve which option is to be used .....
    cmvc_resolve=`verify_var "CMVC login" "$cmvc_default" $cmvc_list`
    [[ $? -ne 0 ]] && exit 1

    if [ "q$CMVC_FAMILY" = "q" ]
    then
        CMVC_FAMILY="aix@aix@2035" ; export CMVC_FAMILY
    fi

    User -view $cmvc_resolve -become $cmvc_resolve \
         -family "$CMVC_FAMILY" >/dev/null 2>/dev/null </dev/null
    status=$?
    if [ $status -ne 0 ]
    then
        a1="$cmd_namedisp: ERROR: user $cmvc_resolve"
        echo "$a1 does not have valid CMVC permissions!" >&2
        exit 1
    else
        echo "$cmvc_resolve"
    fi
    exit 0
}


#
# verify_var - verify a parameter/variable
#
verify_var() {
    if [ $# -lt 2 ]
    then
        echo "usage: `basename $0` option-name var-default" >&2
        echo "                     variable-list" >&2
        echo "           resolves a list of options to the proper" >&2
        echo "           one and flags multiply defined variables" >&2
        echo "           as an error" >&2
        exit 1
    fi

    # fetch the proper arguments .....
    var_option="$1" ; var_default="$2" ; shift 2
    var_list="$*" ; cmd_namedisp=`basename $CMD_NAME`
    components=0

    # if no var list specified, use the default .....
    if [ "q$var_list" = "q" ]
    then
       var_list="$var_default"
       if [ "q$var_default" = "q" ]
       then
           a1="$cmd_namedisp: ERROR: $var_option option does not"
           echo "$a1 have a default!" >&2
           exit 1
       fi
    fi

    for each_var in $var_list
    do
       # make sure we're only going to do this once .....
       let components="$components + 1"
       if [ $components -gt 1 ]
       then
           a1="$cmd_namedisp: ERROR: $var_option option"
           echo "$a1 specified multiple times!" >&2
           exit 1
       fi
       echo "$each_var"
    done
    exit 0
}


#
# verify_var_multiple - verify a parameter/variable which can be
#                       specified multpile times on the command line
#
verify_var_multiple() {
    if [ $# -lt 2 ]
    then
        echo "usage: `basename $0` option-name var-default" >&2
        echo "                     multiple-variable-list" >&2
        echo "           resolves a list of options to the proper" >&2
        echo "           set of variables" >&2
        exit 1
    fi

    var_option="$1" ; var_default="$2"
    shift 2 ; var_list="$*"
    cmd_namedisp=`basename $CMD_NAME`

    # if no var list specified, use the default .....
    if [ "q$var_list" = "q" ]
    then
       var_list="$var_default"
       if [ "q$var_default" = "q" ]
       then
           a1="$cmd_namedisp: ERROR: $var_option option does not"
           echo "$a1 have a default!" >&2
           exit 1
       fi
    fi

    for each_var in $var_list
    do
        echo "$each_var"
    done
    exit 0
}


#
# verify_directories - verify that the following files are in directories
#                      which are writeable
#
verify_directories () {
    direct_verify=`echo $*`
    for each_file in $direct_verify
    do
      verify_base=`dirname $each_file`
      verify_isok=0
      if [ -d "$verify_base" ]
      then
          if [ -w "$verify_base" ]
          then
              verify_isok=1
          fi
      fi
      if [ $verify_isok -eq 0 ]
      then
          echo "$CMD_NAME: ERROR: cannot write to directory $verify_base" >&2
          exit 1
      fi
    done
}


#
# -=-=-=- MISCELLANEOUS FUNCTIONS -=-=-=-
#


#
# cleanup_all - remove temporary files and log definitions
#
cleanup_all() {
    cleanup
    logset -r
}


#
# usage_exit - print usage, cleanup, exit with error
#
usage_exit() {
    usage $CMD_NAME >&2
    cleanup_all ; exit 1
}


#
# -=-=-=- SUBSYSTEM/CMVC NOTES FUNCTIONS -=-=-=-
#

#
# validate_level - make sure the level in the specified release
#                  exists
#
validate_level() {
    # count the number of times we actually see this level .....
    lines_found=$(echo `Report -view levelView -where \
                            "releaseName in ('$RELEASE_NAME') and \
                             name in ('$LEVEL_NAME')" -raw | wc -l`)

    # make sure we could actually get this data .....
    if [ $? -ne 0 ]
    then
        log -b "FATAL ERROR: cannot access CMVC server!"
        cleanup_all ; exit 1
    fi

    # if we didn't find the data, then gripe loudly and exit .....
    if [ "q$lines_found" = "q0" ]
    then
        a1="ERROR: level $LEVEL_NAME ($RELEASE_NAME) does not exist!"
        log -b "$a1" ; cleanup_all ; exit 2
    else
        return 0
    fi
}


#
# walk_notedata - walk through the lines in a set of notes and
#                 find the proper subsystem text
#
walk_notedata() {
    # defect number, temp file to use .....
    defect_note="$TMPFILE0"
    defectno="$1" ; output_note="$2"

    # remove old workfiles .....
    /bin/rm -rf $defect_note $temp_notefile 2>/dev/null >/dev/null

    # fetch the note records for the defect .....
    Report -view noteView -where \
        "defectName in ('$each_defect') \
         order by defectName, addDate asc" -raw > $defect_note

    # handle the note records .....
    bldsbprint -i $defect_note -o $output_note -d $defectno
    if [ $? -ne 0 ]
    then
        a1="WARNING: could not process defect $defectno"
        log -b "${a1} in level $LEVEL_NAME ($RELEASE_NAME)!"
        cleanup_all ; exit 2
    fi
}


#
# build_options_list - build a list of subsystem options for
#                      verifying the subsystem data
#
build_options_list() {
    options_list="$1" ; source_file="${SSTEXTNAME}"

    # if it's not there, just put nothing in the file, let it all
    # not match .....
    if [ ! -r "$source_file" ]
    then
        /bin/cat /dev/null > $options_list 2>/dev/null
        return 1
    fi

    # if it's there, then build the list .....
    /bin/pr -e4 -t -l66 < $source_file | egrep -v "^#|^$" | \
        awk '{ print $1 }' | sort | uniq > $options_list
    return 0
}



#
# proc_leveldata - process release/level data to create a list of
#                  defects which can be scanned individually
#
proc_leveldata() {
    # workfiles we need to process the level .....
    new_leventries="$1" ; defect_list="$TMPFILE2"
    note_def="$TMPFILE3" ; opts_list="$TMPFILE4"
    opts_list_valid=0

    # remove old workfiles .....
    /bin/rm -rf $defect_list $note_def 2>/dev/null >/dev/null
    /bin/rm -rf $opts_list $new_leventries 2>/dev/null >/dev/null

    # first, make sure we can actually access this level ....
    validate_level

    # build options list .....
    build_options_list $opts_list ; opts_list_valid=$?
    if [ $opts_list_valid -ne 0 ]
    then
        a1="WARNING: cannot find sstext.name for subsystem option"
        log -b "$a1 verification!"
    fi

    # build a list of defects for this level .....
    Report -view levelMemberView -where \
        "releaseName in ('$RELEASE_NAME') and \
         levelName in ('$LEVEL_NAME')" -raw |
         awk -F\| '{ print $3 }' | sort | uniq > $defect_list

    # see if we have any defects in this level, and then let the
    # user know we made it this far .....
    number_defects=$(echo `wc -l < $defect_list`)
    if [ "q$number_defects" = "q0" ]
    then
        a1="ERROR: level $LEVEL_NAME ($RELEASE_NAME) has no defects!"
        log -b "$a1" ; cleanup_all ; exit 2
    else
        # make sure we've got good grammar .....
        if [ "q$number_defects" = "q1" ]
        then
            quant_print="defect"
        else
            quant_print="defects"
        fi

        a1="level $LEVEL_NAME ($RELEASE_NAME) exists"
        log -b "$a1 (${number_defects} ${quant_print})"
    fi

    # process each defect .....
    while read each_defect
    do
        # remove the old note before fetching a new one .....
        /bin/rm -rf $note_def 2>/dev/null >/dev/null

        # get the proper text from the defect .....
        walk_notedata $each_defect $note_def

        # set counts of good/bad options .....
        options_ok=0 ; options_bad=0

        # process each subsystem set .....
        while read each_sb_line
        do
            # skip blank lines .....
            [[ "q$each_sb_line" = "q" ]] && continue

            # if we've got a valid options list, then process
            # the option from the file .....
            if [ $opts_list_valid -ne 0 ]
            then
                echo "$each_defect|$each_sb_line" >> $new_leventries
                let options_ok="$options_ok + 1"
            else
                # figure out if we've got a match, then print it
                # or log it .....
                lines_match=$(echo `egrep "^${each_sb_line}\$" \
                                        $opts_list | wc -l`)
                if [ "q$lines_match" = "q0" ]
                then
                    a1="defect $each_defect references invalid"
                    log -b "$a1 subsystem ${each_sb_line}!"
                    let options_bad="$options_bad + 1"
                else
                    echo "$each_defect|$each_sb_line" >> $new_leventries
                    let options_ok="$options_ok + 1"
                fi
            fi
        done < $note_def

        # display log note about defect options .....
        let total_options="$options_ok + $options_bad"
        if [ $total_options -gt 0 ]
        then
            if [ $options_bad -gt 0 ]
            then
                badopts=" ($options_bad invalid)"
            else
                badopts=""
            fi

            a1="defect $each_defect has $options_ok valid options"
            log -b "${a1}${badopts}"
        fi
    done < $defect_list
}


#
# merge_leveldata - merge the old level data in the appropriate PTF
#                   directory with the new set of data
#
merge_leveldata() {
    # specify output file, file containing new data .....
    output_file="$TOP/PTF/$BLDCYCLE/$RELEASE_NAME/ssXREF"
    new_leveldata="$1" ; temp_mergedata="$TMPFILE6"

    # build the directories first thing, just to be sure .....
    /bin/mkdir "$TOP" 2>/dev/null >/dev/null
    /bin/mkdir "$TOP/PTF" 2>/dev/null >/dev/null
    /bin/mkdir "$TOP/PTF/$BLDCYCLE" 2>/dev/null >/dev/null
    /bin/mkdir "$TOP/PTF/$BLDCYCLE/$RELEASE_NAME" 2>/dev/null >/dev/null

    # merge the data from the old output file with the new
    # data before writing over the old file .....
    /bin/rm -rf $temp_mergedata 2>/dev/null >/dev/null
    /bin/cat $output_file $new_leveldata 2>/dev/null > $temp_mergedata
    if [[ -s ${temp_mergedata} ]]
    then
       sort < $temp_mergedata | uniq > $output_file
    else
       > ${output_file}
    fi
    /bin/rm -rf $temp_mergedata 2>/dev/null >/dev/null

    # tell the user we updated the file .....
    log -b "finished processing level $LEVEL_NAME ($RELEASE_NAME)"
}



#
# -=-=-=- MAIN PROGRAM-=-=-=-
#
. bldkshconst
. bldinitfunc
. bldloginit
. bldkshfunc


# Save command name, etc. .....
CMD_NAME="$0"
curdir=`pwd`

# Initialize variables .....
init_proc

# Process arguments .....
proc_arguments $*

# Set signal traps, remove files from previous run .....
trap 'log -b $CMD_NAME aborted ; logset -r ; cleanup_all ; exit 1' \
     1 2 3 4 5 6 7 8 10 14 15
cleanup

# Make sure we've enough disk space to run .....
check_diskspace 3072 400 $BLDTMP

# process the level data we've got .....
proc_leveldata $NEW_ENTRIES
merge_leveldata $NEW_ENTRIES

# All done now .....
cleanup_all
exit 0


#
# End of chksubsys.sh.
#

