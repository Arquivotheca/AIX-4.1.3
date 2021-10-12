#! /bin/ksh
# @(#)96	1.1  src/bldenv/pkgtools/updatebuild.sh, pkgtools, bos412, GOLDA411a 2/8/94 15:28:57
#
# COMPONENT_NAME: (PKGTOOLS)
#
# FUNCTIONS: updatebuild
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1994
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

. bldkshconst # initializes ksh variables
. bldinitfunc # brings in buildinit and chkset functions
. bldloginit  # brings in login function
. bldkshfunc

typeset Missing_LPP="${FALSE}"

# Get ${build_arguments}, this will be $*, pull out arguments that are
# really for updatebuild such as PTFSETUPFLAGS.  If BUILD_TYPE passed
# then set local BUILD_TYPE
build_arguments=$*

typeset -x BUILD_TYPE
typeset -x CMVC_RELEASE
typeset -x ODE_TOOLS
typeset -x SHIP_PATH
typeset -x TOP

# Try to determine if we are in a sandbox, if sbinfo returns error assume
# we are not
sbinfo 1> sbinfo.results 2>&1
[[ $? != 0 ]] && print -u2 "sbinfo not found, insure you running from a sandbox"

# If the following are not already set, set them
#    SHIP_PATH from value in sbinfo.results
#    ODE_TOOLS from value in sbinfo.results
#    TOP from SOURCEBASE value in sbinfo.results, will be SOURCEBASE/../selfix

if [[ -z "${SHIP_PATH}" ]]
then
   SHIP_PATH=`awk -F= '/SHIP_PATH/ {print $2}' sbinfo.results`
fi

if [[ -z "${ODE_TOOLS}" ]]
then
  ODE_TOOLS=`awk -F= '/ODE_TOOLS/ {print $2}' sbinfo.results`
fi

if [[ -z "${TOP}" ]]
then
  SOURCEBASE=`awk -F= '/SOURCEBASE/ {print $2}' sbinfo.results` 
  TOP=$SOURCEBASE/../selfix
fi

# If TOP directory does no exist then create it.
if [[ ! -d "${TOP}" ]]
then
   mkdir ${TOP}
fi

chksetrelease
bldinit build # will call chksettop
# Insure the following values have been set, if sbinfo runs it may be
# possible one of the values was not set.
chkset_build_type
chkset_ship_path

typeset    command=${0##*/}
typeset -x TD_BLDCYCLE=${BLDCYCLE}		# Set the build cycle variable
						# use in src/Makeconf.

trap 'logset -r; exit 1' INT QUIT HUP TERM

logset -C ${command} -c ${command} -F $(get_log_file ${command} $(bldlogpath))
if [[ $? -eq 0 ]]
then
  setlog=1
fi

[[ "${build_type}" = "production" ]] && \
   log +l -x "${command} does not support production builds"

# Check build cycle state and open if needed.
CheckPhases open test prebuild build selfix package distribution
[[ $? -ne 0 ]] && bldstatus -o ${BLDCYCLE} -f

# Assume packaging directory will have "src/packages" is its path.
curdir=`pwd`
echo $curdir | egrep src/packages > /dev/null 2>&1
rc=$?
if [[ "${build_type}" = "area" || $rc != 0 ]]
then
   DeleteStatus $_TYPE $T_BUILD $_BLDCYCLE $BLDCYCLE $_RELEASE $CMVC_RELEASE
   build ${build_arguments}
   rc=$?
   [[ $rc -ne 0 ]] && \
      log +l -x "${command}: update build failed with a return code $rc"
   bldsetstatus $_TYPE $T_BUILD $_BLDCYCLE $BLDCYCLE \
                $_RELEASE $CMVC_RELEASE $_UPDATE $U_UPDATE
   if [[ $setlog -eq 1 ]]
   then
     logset -r
   fi
   exit 0
fi

DeleteStatus $_TYPE $T_BUILD $_BLDCYCLE $BLDCYCLE $_RELEASE pkg410

export CMVC_RELEASE=pkg410

# ptfsetup will put status to test and subptf seems to run fine
ptfsetup ${PTFSETUPFLAGS}
[[ $? -ne 0 ]] && log +l -x "${command}: ptfsetup failed"

bldinit build
build ${build_arguments}
rc=$?
[[ $rc -ne 0 ]] && \
  log +l -x "${command}: packaging build failed with a return code $rc"
bldsetstatus $_TYPE $T_BUILD $_BLDCYCLE $BLDCYCLE \
             $_RELEASE $CMVC_RELEASE $_UPDATE $U_UPDATE

#
# Don't allow subptf or ptfpkg to start until all files that were updated
# have been packaged into their LPP.
#
(cd ${TOP}/PTF/${BLDCYCLE}/XREF; cat *.xref > BIGxref.updatebuild)
for file in `cat ${TOP}/PTF/${BLDCYCLE}/*/lmupdatelist`
do
   grep ${file} ${TOP}/PTF/${BLDCYCLE}/XREF/BIGxref.updatebuild 1> /dev/null 2>&1
   if [[ $? -ne 0 ]] 
   then
      log +l -e "The LPP containing the file ${file}"
      log +l -e "must be packaged"
      Missing_LPP="${TRUE}"
   fi
done
if [[ "${Missing_LPP}" = "${TRUE}" ]]
then
   log +l -e "All LPPs containing updated files must be packaged"
   log +l -e "to continue with subptf and ptfpkg"
   log +l -x "Cannot continue"
fi

set -x
subptf
[[ $? -ne 0 ]] && log +l -x "${command}: subptf failed"
set +x

ptfpkg
[[ $? -ne 0 ]] && log +l -x "${command}: ptfpkg failed"

if [[ $setlog -eq 1 ]]
then
  logset -r
fi
exit 0
