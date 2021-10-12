#
# @(#)51	1.48  src/bldenv/bldtools/bldkshconst.sh, bldtools, bos412, GOLDA411a 3/28/94 16:10:20
# COMPONENT_NAME: (BLDTOOLS) BAI Build Tools
#
# FUNCTIONS: bldkshconst
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1991, 1993
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

####################### D A T A   C O N S T A N T S #####################

TRUE=1
FALSE=0
SUCCESS=0
FAILURE=1
UNSET=2
PSEP="|"
SEP=","

#####################   L O C K   C O N S T A N T S   ###################
PTFMAIN="mainptf-$BLDCYCLE"
STATUS="statusfunctions -t 180"
PERM="-p $HOST.$(basename $0).$$"; export PERM  # export for status functions

##################### S T A T U S   C O N S T A N T S ###################

##### build status arguments with associated constants #####
_TYPE="-a"
_BLDCYCLE="-b"
_RELEASE="-c"
_LEVEL="-d"
_BUILDER="-e"; export _BUILDER  # export for bldsetstatus
_DATE="-f"; export _DATE  # export for bldsetstatus
_LPP="-g"
_PTF="-h"
_SUBTYPE="-i"
_STATUS="-j"
  ST_START="start"
  ST_SUCCESS="success"
  ST_FAILURE="failure"
_TRACKINGDEFECT="-g"
_HOSTNAME="-h"; export _HOSTNAME  # export for bldsetstatus
_PHASE="-j"
#   Phase - open
#   Phase - prebuild
#   Phase - build
#   Phase - selfix
#   Phase - package
#   Phase - distribution
  PHASE_CLOSED="closed"
#   Phase - removed
_UPDATE="-j"
  U_UPDATE="update"
  U_NOUPDATE="no_update"
# flag definitions for bldstatus entries
_FROMPHASE="-g"
_TOPHASE="-h"
_FORCEFLG="-i"

  ##### types with associated subtypes #####
  T_V3BLD="v3bld"				# Remove this definition when
						# 410 conversion is complete.
  T_BUILD="v4bld"
  T_BLDCYCLE="bldcycle"
  T_BLDPTF="subptf"
    S_BLDGETLISTS="getlists"
    S_BLDMID="bldmid"
    S_BLDGETLINK="getlinkdata"
    S_BLDGETXREF="normalizexref"
    S_BLDPTFDEPEND="finddependents"
    S_BLDPTFPKG="package"
    S_LPP="lpp"
  T_PREBUILD="prebuild"
    S_RELMERGE="levelmerge"
    S_SOURCEMERGE="cmvcmerge"
    S_DELTAEXTRACT="cmvcextract"
    S_CMVCCOMMIT="cmvccommit"
    S_PTFSETUP="ptfsetup"
  T_GENPTF="cumptf"				# Remove this definition when
						# 410 conversion is complete.
  T_PTFPKG="ptfpkg"
  T_POSTBUILD="postbuild"
    S_BLDHISTORYCOMMIT="ptfhistorycommit"
  T_POSTPACKAGE="postpackage"
  S_MAIN="main"
  T_BLDRETAIN="bldretain"
    S_OPENAPAR="OpenApar"
    S_UPDATEREF="UpdateReference"
  T_CHECKSYMPTOM="CheckSymptom"
    S_SYMPTOMS="symptoms"
  T_BLDABSTRACTS="bldabstracts"
#   S_SYMPTOMS="symptoms"
  T_BLDQUERYMERGE="bldquerymerge"
  T_PROMOTEPTF="promoteptf"
  T_SNIFF="sniff"
  T_BLDSTATUS="bldstatus"
  T_BLDAFSMERGE="bldafsmerge"
    AFSMERGE_SUBTYPES=5
    S_BLDENVMERGE="bldenvmerge"
    S_DELTAMERGE="deltamerge"
    S_OBJECTMERGE="objectmerge"
    S_PTFMERGE="ptfmerge"
    S_SHIPMERGE="shipmerge"
  
_DONTCARE='*'

AFSBASE="/afs/austin/aix"	# Base for AFS tree
export DEFAULT_CMVCFAMILY="aix@ausaix02@2035"
				# Default value for CMVC_FAMILY.
MAX_LEVELNAME_LENGTH=15         # CMVC limit on the length a levelname can be.
SEARCH_FIRST="first"		# Begin the search with the first element.
				# Will do any intialization to get search
				# started.
SEARCH_NEXT="next"		# Get next element in search.
SEARCH_STOP="stop"		# Search is finished, return resources that
				# are no longer needed.
SPACECHARACTER=" "		# SPACE character.
TABCHARACTER="	"		# TAB character.
