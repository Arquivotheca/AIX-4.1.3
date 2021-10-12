#! /bin/ksh
# @(#)74	1.13  src/bldenv/bldtools/postbuild.sh, bldprocess, bos412, GOLDA411a 3/16/94 16:03:02
# COMPONENT_NAME: (BLDPROCESS) BAI Build Process
#
# FUNCTIONS: postbuild
#            Usage
#            GetGenptfLpps
#            UndoGenptf
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1991, 1992
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
#

#
# NAME: usage
#
# FUNCTION: Displays the syntax for postbuild
#
# INPUT: none
#
# OUTPUT: none
#
# SIDE EFFECTS: command syntax logged
#
# EXECUTION ENVIRONMENT: the build process environment
#
# RETURNS: n/a
#
function Usage {
    log -e "Usage: postbuild [-u] [<bldcycle>]"
}

#
# NAME: GetGenptfLpps
#
# FUNCTION: Determine which lpps had genptf run against them
#
# INPUT: none
#
# OUTPUT: list of lpps names
#
# SIDE EFFECTS: none
#
# EXECUTION ENVIRONMENT: the build process environment
#
# RETURNS: n/a
#
function GetGenptfLpps {
    QueryStatus $_TYPE $T_PTFPKG $_BLDCYCLE $BLDCYCLE \
                $_BUILDER "$_DONTCARE" $_DATE "$_DONTCARE" \
                $_STATUS "$_DONTCARE" -A | \
		awk -F"|" ' { \
			print $7 \
		} '
}

#
# NAME: UndoGenptf
#
# FUNCTION: Reverses genptf actions on ins_exp file in "old" directory
#
# INPUT: lpp name
#
# OUTPUT: none
#
# SIDE EFFECTS: errors are logged to log file
#
# EXECUTION ENVIRONMENT: the build process environment
#
# RETURNS: 0 if success
#          1 if usage error
#          2 if package directory/build machine indeterminable
#          3 if copy of ins_exp from older to old fails
#          4 if ins_exp does not exist in either old or older
#
function UndoGenptf {
    [[ $# = 1 ]] || { log -e "Usage: UndoGenptf <lpp>"; return 1; }
    lpp=$1
    pkgdir=$(grep "^$lpp	" $lpppackdir | cut -f2)
    rel=$(grep "^$lpp	" $lpppackdir | cut -f3)
    bldhostsfile ${rel} ${TRUE}
    [[ $? -ne 0 ]] && return 1

    if [[ -z "$pkgdir" || -z "${HOSTSFILE_HOST}" ]] ; then
        log -e "Determining package directory or build machine for $lpp"
        return 2
    else
        rc=$(rsh ${HOSTSFILE_HOST} -l build -n\
         "cd $pkgdir; \
          for dir in . root share; do \
              if [[ -d \$dir/old ]] ; then
                  if [[ ! -f \$dir/old/ins_exp ]] ; then \
                      if [[ -f \$dir/older/ins_exp ]] ; then \
                          cp -p \$dir/older/ins_exp \$dir/old/ins_exp ; \
                          [[ \$? = 0 ]] || { print 1; exit; } ; \
                      else \
                          print 2 ; \
                          exit ; \
                      fi ; \
                  fi ; \
              fi ; \
          done ; \
          print 0")
        if [[ "$rc" = 1 ]] ; then
            log -e "Copying $dir/older/ins_exp to $dir/old/ins_exp."
        elif [[ "$rc" = 2 ]] ; then
            log -e "Neither $dir/older/ins_exp nor $dir/old/ins_exp exists."
        fi
        return $rc
    fi
}
              
#
# NAME: main (postbuild)
#
# FUNCTION: performs all necessary processing which must take place during
#           phase transitions from "build" to "selfix"
#
# INPUT: build cycle, -u when moving from selfix to build
#
# OUTPUT: none
#
# SIDE EFFECTS: errors are logged,
#               ins_exp files are copied back to "old" directory if
#               necessary
#
# EXECUTION ENVIRONMENT: the build process environment
#
# RETURNS: 0 if successful
#          2 if genptf reversal operations are unsuccessful
#
. bldloginit
trap 'logset -r; trap - EXIT; exit $rc' EXIT INT QUIT HUP TERM
. bldinitfunc
. bldkshconst
. bldhostsfile
. bldnodenames
export lpppackdir=$BLDENV/usr/bin/lpp_pack_dir
rc=0

while getopts :u option; do
    case $option in
      u)  un=un;;
     \?)  print -u2 "unknown option $OPTARG"; Usage; exit 1;;
    esac
done
shift OPTIND-1

[[ $# -eq 1 ]] && export BLDCYCLE=$1

bldinit

LOG=${LOGPOSTBUILD:-"$(bldlogpath)/postbuild"}
logset -c$0 +l -F$LOG

if [[ "$un" = un ]] ; then
    lpp_list=$(GetGenptfLpps)
    if [[ -n "$lpp_list" ]] ; then
        for lpp in $lpp_list ; do
            UndoGenptf $lpp
            [[ $? = 0 ]] || {
                log -e "Copying back ins_exp file for $lpp."
                rc=2
            }
        done
    fi
fi
exit $rc

