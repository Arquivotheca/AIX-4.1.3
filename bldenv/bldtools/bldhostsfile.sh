# @(#)02	1.25  src/bldenv/bldtools/bldhostsfile.sh, bldprocess, bos412, GOLDA411a 6/16/94 16:56:43
#
# COMPONENT_NAME: (BLDPROCESS) BAI Build Process
#
# FUNCTIONS: bldhostsfile
#            bldhostsfile_checkfile
#            bldhostsfile_search
#            bldhostsfile_search_data1
#            bldhostsfile_set_variables
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1992, 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

#
# The following functions must be included when this function is included:
#    bldkshconst
#    bldloginit
#    bldnodenames
#

#
# NAME: bldhostsfile
#
# FUNCTION: User interface to the hostsfile.dat data base.  This routine
#           should be called to search the hostsfile.dat data base for a
#           release.
#
# INPUT: release - release name to search for in hostsfile.dat.
#        logerrors - log any error messages if set to TRUE, value
#                    of TRUE as defined in bldkshconst.
#
# OUTPUT: HOSTSFILE_AFSBASE (global) - AFS tree of release.
#         HOSTSFILE_AFSDELTA (global) - AFS delta source trees of release.
#         HOSTSFILE_AFSPROD (global) - AFS full source tree of release.
#         HOSTSFILE_BASE (global) - Production tree of release.
#         HOSTSFILE_BLDNODE (global) - CMVC name of release directory.
#         HOSTSFILE_BUILD (global) - Directory where a full build should
#                                    start.
#         HOSTSFILE_CMVCFAMILY (global) - The CMVC Family the release
#                                         resides in.
#         HOSTSFILE_DATA1 (global) - Data field one.  Use of the field varies,
#                                    see ENV_variables man page for current
#                                    uses.
#         HOSTSFILE_DELTA (global) - Production delta source tree of release.
#         HOSTSFILE_ENVIRONMENT (global) - Type of environment the build
#                                          occurs in.  Values are are aix32
#                                          or ade.
#         HOSTSFILE_HOST (global) - Host the release is built on.
#         HOSTSFILE_OBJECT (global) - Directory where the objects files are
#                                     located.
#         HOSTSFILE_PROD (global) - Production full source tree of release.
#         HOSTSFILE_REL (global) - REL name of the release.
#         HOSTSFILE_SHIP (global) - Directory where the ship tree is located.
#
# SIDE EFFECTS: none.
#
# RETURNS: 0 if successful, 1 if error encountered, and 2 if multiple entries.
#

#
# Variables exported to the user.
#
typeset -x HOSTSFILE_AFSBASE=""
typeset -x HOSTSFILE_AFSDELTA=""
typeset -x HOSTSFILE_AFSPROD=""
typeset -x HOSTSFILE_BASE=""
typeset -x HOSTSFILE_BLDNODE=""
typeset -x HOSTSFILE_BUILD=""
typeset -x HOSTSFILE_CMVCFAMILY=""
typeset -x HOSTSFILE_DATA1=""
typeset -x HOSTSFILE_DELTA=""
typeset -x HOSTSFILE_ENVIRONMENT=""
typeset -x HOSTSFILE_HOST=""
typeset -x HOSTSFILE_OBJECT=""
typeset -x HOSTSFILE_PROD=""
typeset -x HOSTSFILE_REL=""
typeset -x HOSTSFILE_SHIP=""

typeset -r HOSTSFILE_SEPARATOR="|"	# Separator in hostsfile.dat.
typeset    hostsfile_release		# Release name read from hostsfile.dat,
					# used for error checking.  It better
					# match the release value searching on.


function bldhostsfile
{
   typeset    release=$1
   typeset    line                      # Line read from hostsfile.dat.
   typeset -i line_count=0              # Number of lines read from
                                        # hostsfile.dat.
   typeset    logerrors=$2
   typeset    old_IFS			# Hold IFS value, IFS is changed to
					# read in file.
   typeset    prev_line                 # Last valid line read from
                                        # hostsfile.dat.

   bldhostsfile_checkfile ${logerrors}
   [[ $? -ne 0 ]] && return 1

   # Find release entry in hostsfile.dat.
   grep "^${release}${HOSTSFILE_SEPARATOR}" ${HOSTSFILE} | \
      while read line
      do
         prev_line=${line}
         ((line_count=${line_count}+1))
      done

   # No entry for release found.
   [[ ${line_count} -eq 0 ]] && \
   {
      [[ "${logerrors}" = "${TRUE}" ]] && \
         log +l -c bldhostsfile -e "${release} not found in ${HOSTSFILE}"
      return 1
   }

   # Multiple enteries for release found.
   [[ ${line_count} -ne 1 ]] && \
   {
      [[ "${logerrors}" = "${TRUE}" ]] && \
         log +l -c bldhostsfile -e "Multiple ${release}'s found in ${HOSTSFILE}"
      return 2
   }

   old_IFS="${IFS}"
   IFS="|"
   print "${prev_line}" | \
      read hostsfile_release HOSTSFILE_HOST HOSTSFILE_ENVIRONMENT \
           HOSTSFILE_CMVCFAMILY HOSTSFILE_REL HOSTSFILE_BLDNODE \
           HOSTSFILE_BASE HOSTSFILE_PROD HOSTSFILE_DELTA \
           HOSTSFILE_BUILD HOSTSFILE_OBJECT HOSTSFILE_SHIP \
           HOSTSFILE_AFSBASE HOSTSFILE_AFSPROD HOSTSFILE_AFSDELTA \
           HOSTSFILE_DATA1
   IFS="${old_IFS}"

   # Entries do not match.
   [[ "${release}" != "${hostsfile_release}" ]] && \
   {
      logline="Searching for ${release} and found ${hostsfile_release} in"
      logline="${logline} ${HOSTSFILE}"
      log +l -c bldhostsfile -x "${logline}"
   }

   bldhostsfile_set_variables ${release} "${logerrors}"

   return $?
}

#
# NAME: bldhostsfile_checkfile
#
# FUNCTION: Check for the existance of the BLDHOSTSFILE.
#
# INPUT: logerrors - log any error messages if set to TRUE, value
#                    of TRUE as defined in bldkshconst.
#
# OUTPUT: none.
#
# SIDE EFFECTS: none.
#
# RETURNS: 0 if file exists, 1 if file does not exist.
#
#
function bldhostsfile_checkfile
{
   typeset    logerrors=$1

   get_filename_definitions

   [[ ! -f "${HOSTSFILE}" ]] && \
   {
      [[ "${logerrors}" = "${TRUE}" ]] && \
         log +l -c bldhostsfile_check -e "File ${HOSTSFILE} does not exist"
      return 1
   }

   return 0
}

#
# NAME: bldhostsfile_search
#
# FUNCTION: User interface to the hostsfile.dat data base.  This routine
#           should be called to search the hostsfile.dat data base for all
#           release names.  Each release entry found is returned to the caller
#           in global environment variables (see OUTPUT below)
#           Initial call must have request set to SEARCH_FIRST this will
#           open hostsfile.dat and return the first element.  Calls to get
#           data should all then be with request set to SEARCH_NEXT this
#           will return each successive element.  When the search is
#           complete a request of SEARCH_STOP will close the hostsfile.dat
#           file.
#
# INPUT: logerrors ($2) - log any error messages if set to TRUE, value
#                         of TRUE as defined in bldkshconst.
#        request ($1) - SEARCH_FIRST -- open and get first entry from
#                       nodenames file.
#                       SEARCH_NEXT -- get next entry from the nodenames file.
#                       SEARCH_STOP  -- close the nodenames file.
#
# OUTPUT: HOSTSFILE_AFSBASE (global) - AFS tree of release.
#         HOSTSFILE_AFSDELTA (global) - AFS delta source trees of release.
#         HOSTSFILE_AFSPROD (global) - AFS full source tree of release.
#         HOSTSFILE_BASE (global) - Production tree of release.
#         HOSTSFILE_BLDNODE (global) - CMVC name of release directory.
#         HOSTSFILE_BUILD (global) - Directory where a full build should
#                                    start.
#         HOSTSFILE_CMVCFAMILY (global) - The CMVC Family the release
#                                         resides in.
#         HOSTSFILE_DATA1 (global) - Data field one.  Use of the field varies,
#                                    see ENV_variables man page for current
#                                    uses.
#         HOSTSFILE_DELTA (global) - Production delta source tree of release.
#         HOSTSFILE_ENVIRONMENT (global) - Type of environment the build
#                                          occurs in.  Values are are aix32
#                                          or ade.
#         HOSTSFILE_HOST (global) - Host the release is built on.
#         HOSTSFILE_OBJECT (global) - Directory where the objects files are
#                                     located.
#         HOSTSFILE_PROD (global) - Production full source tree of release.
#         HOSTSFILE_REL (global) - REL name of the release.
#         HOSTSFILE_SHIP (global) - Directory where the ship tree is located.
#
# SIDE EFFECTS: none
#
# RETURNS: 0 if successful, 1 unsuccessful.
#
function bldhostsfile_search
{
   typeset    request=$1
   typeset    logerrors=$2
   typeset    old_IFS			# Hold IFS value, IFS is changed to
					# read in file.

   if [[ "${request}" = "${SEARCH_STOP}" ]]
   then
      exec 8<&-
      return 0
   fi

   if [[ "${request}" = "${SEARCH_FIRST}" ]]
   then
      bldhostsfile_checkfile "${logerrors}"
      [[ $? -ne 0 ]] && return 1
      exec 8< ${HOSTSFILE}
   fi

   HOSTSFILE_RELEASE=""
   old_IFS="${IFS}"
   IFS="|"
   # Skip blank lines and those with # as first non-white character
   while [[ -z "${HOSTSFILE_RELEASE}" || \
            "${HOSTSFILE_RELEASE#\#}" != "${HOSTSFILE_RELEASE}" ]]
   do
      read -u8 HOSTSFILE_RELEASE HOSTSFILE_HOST HOSTSFILE_ENVIRONMENT \
               HOSTSFILE_CMVCFAMILY HOSTSFILE_REL HOSTSFILE_BLDNODE \
               HOSTSFILE_BASE HOSTSFILE_PROD HOSTSFILE_DELTA \
               HOSTSFILE_BUILD HOSTSFILE_OBJECT HOSTSFILE_SHIP \
               HOSTSFILE_AFSBASE HOSTSFILE_AFSPROD HOSTSFILE_AFSDELTA \
               HOSTSFILE_DATA1
      [[ $? -ne 0 ]] && { IFS="${old_IFS}" ; return 1 ; }
   done
   IFS="${old_IFS}"

   bldhostsfile_set_variables ${HOSTSFILE_RELEASE} "${logerrors}"
   if [[ -n "${HOSTSFILE_RELEASE}" ]]
   then
      return 0
   else
      return 1
   fi
}

#
# NAME: bldhostsfile_search_data1
#
# FUNCTION: Search the current data1 field for a argument value.
#
# INPUT: SPACECHARACTER (global) - space character constant.
#        value ($1) - argument value to search for.
#        logerrors ($2) - log any error messages if set to TRUE, value
#                         of TRUE as defined in bldkshconst.
#
# OUTPUT: Value of argument displayed to stdout.
#
# SIDE EFFECTS: Will exit program is expr functions fail.  Function failure
#               indicates format in hostsfile.dat incorrect.
#
# RETURNS: 0 if argument found, 1 if argument not found.
#
#
function bldhostsfile_search_data1
{
   typeset    assignment		# Current assignment declaration 
					# being worked on from DATA1.
   typeset    assignments=${HOSTSFILE_DATA1}
					# Current assignments declarations
					# left from DATA1.
   typeset    logerrors=$2
   typeset    value=$1			# Declaration being searched for.

   while [[ -n "${assignments}" ]]
   do
      # Pulling out first assignment statement; format is ARG="arg1 ... argn"
      # "\(.[^\"]*\".[^\"]*\"[${SPACECHARACTER}]\).*" \| \
      #    <-----><><-----><><------------------>
      #       |   |    |   |   a space character      
      #       |   |    |   |-- the second quote
      #       |   |    |------ all characters from first quote to second quote
      #       |   |----------- the first quote
      #       |--------------- all characters up to first quote
      assignment=$(expr "${assignments}" : \
                        "\(.[^\"]*\".[^\"]*\"[${SPACECHARACTER}]\).*" \| \
                        "${assignments}")
      if [[ $? -ne 0 ]]
      then
         [[ "${logerrors}" = "${TRUE}" ]] && \
            log +l -c bldhostsfile_search_data1 \
                -x "Searching ${release} data1 field for ${value}"
      fi
      assignments=${assignments##${assignment}}
      if [[ "${value}" = "${assignment%%=*}" ]]
      then
         assignment=$(expr "${assignment}" : ".*\"\(.*\)\"")
         if [[ $? -ne 0 ]]
         then
            [[ "${logerrors}" = "${TRUE}" ]] && \
               log +l -c bldhostsfile_search_data1 \
                   -x "${release} data1 field assignment of ${value}"
         fi
         print ${assignment}
         return 0
      fi
   done

   return 1
}

#
# NAME: bldhostsfile_set_variables
#
# FUNCTION: User interface to the hostsfile.dat data base.  This routine
#           should be called to search the hostsfile.dat data base
#
# INPUT: DEFAULT_CMVCFAMILY (global) - Default value for HOSTSFILE_CMVCFAMILY.
#        NODENAMES_NODE (global) - Source name of release directory, returned
#                                  from call to bldnodenames.
#        release - release name to search for in hostsfile.dat.
#        logerrors - log any error messages if set to TRUE, value
#                    of TRUE as defined in bldkshconst.
#
# INPUT/OUTPUT: On input these variables are set to the values from
#               hostsfile.dat.  On output they are the calculated values
#               based upon the rules described in the man page for 
#               hostsfile.dat.
#        HOSTSFILE_AFSBASE (global) - AFS tree of release.
#        HOSTSFILE_AFSDELTA (global) - AFS delta source trees of release.
#        HOSTSFILE_AFSPROD (global) - AFS full source tree of release.
#        HOSTSFILE_BASE (global) - Production tree of release.
#        HOSTSFILE_BLDNODE (global) - CMVC name of release directory.
#        HOSTSFILE_BUILD (global) - Directory where a full build should
#                                   start.
#        HOSTSFILE_CMVCFAMILY (global) - The CMVC Family the release
#                                        resides in.
#        HOSTSFILE_DATA1 (global) - Data field one.  Use of the field varies,
#                                   see ENV_variables man page for current
#                                   uses.
#        HOSTSFILE_DELTA (global) - Production delta source tree of release.
#        HOSTSFILE_ENVIRONMENT (global) - Type of environment the build
#                                         occurs in.  Values are are aix32
#                                         or ade.
#        HOSTSFILE_HOST (global) - Host the release is built on.
#        HOSTSFILE_OBJECT (global) - Directory where the objects files are
#                                    located.
#        HOSTSFILE_PROD (global) - Production full source tree of release.
#        HOSTSFILE_REL (global) - REL name of the release.
#        HOSTSFILE_SHIP (global) - Directory where the ship tree is located.
#
# SIDE EFFECTS: none.
#
# RETURNS: 0 if successful, 1 if error encountered.
#
function bldhostsfile_set_variables
{
   typeset    release=$1
   typeset    logerrors=$2

   # EXBASE overrides delta extract directory.
   # NOTE: for this  value to override the values of $HOSTSFILE_PROD and
   #       $HOSTSFILE_DELTA they must be supplied as relative paths in
   #       the hostsfile.dat.  Relative paths do not begin with /.
   [[ -n "${EXBASE}" ]] && HOSTSFILE_BASE=${EXBASE}

   if [[ -z "${HOSTSFILE_HOST}" ]] 
   then
      HOSTSFILE_HOST="$(hostname -s)"
      [[ -z "${HOSTSFILE_HOST}" && "${logerrors}" = "${TRUE}" ]] && \
         log +l -c bldhostsfile_set_variables \
             -x "Cannot find host name for HOSTSFILE_HOST"
   fi

   # Use host cmvc_landru instead of landru.
   # cmvc_landru is a direct link to the CMVC machines
   if [[ "${HOSTSFILE_HOST}" = "landru" ]] 
   then
      HOSTSFILE_HOST=cmvc_landru
   fi

   # Default building environment is 3.2 aix.
   [[ -z "${HOSTSFILE_ENVIRONMENT}" ]] && HOSTSFILE_ENVIRONMENT="aix32"

   # Default CMVC family is value of ${DEFAULT_CMVCFAMILY}
   if [[ -z "${HOSTSFILE_CMVCFAMILY}" ]]
   then
      HOSTSFILE_CMVCFAMILY=$(bldCMVCfamily ${DEFAULT_CMVCFAMILY})
   else
      HOSTSFILE_CMVCFAMILY=$(bldCMVCfamily ${HOSTSFILE_CMVCFAMILY})
   fi
   if [[ $? -ne 0 ]]
   then
      log +l -c bldhostsfile \
          -e "Value of HOSTSFILE_CMVCFAMILY cannot be set"
      return 1
   fi

   # Set NODENAMES_NODENAME to NULL each run so data from previous is not
   # accidently used.
   NODENAMES_NODENAME=""
   # If values are not supplied in hostsfile.dat, revert back to the old
   # method of calculating the values.  Search the nodenames file for list
   # of nodenames.
   [[ -z "${HOSTSFILE_REL}" ]] && \
   {
      bldnodenames_search ${SEARCH_FIRST} "${logerrors}"
      [[ $? -ne 0 ]] && return 1
      while [[ -n "${NODENAMES_NODENAME}" ]]
      do
         HOSTSFILE_REL=${release%${NODENAMES_NODENAME}}
         [[ "${HOSTSFILE_REL}" != "${release}" ]] && break
         bldnodenames_search ${SEARCH_NEXT} "${logerrors}"
         [[ $? -ne 0 ]] && break
      done
      bldnodenames_search ${SEARCH_STOP} "${logerrors}"
      [[ $? -ne 0 ]] && return 1
      # Did not break out relname and nodename, probably a missing value
      # in nodenames.dat.
      [[ "${HOSTSFILE_REL}" = "${release}" ]] && \
      {
         # Did not find a nodename entry in nodenames.dat.
         log +l -c bldhostsfile -e "Cannot determine HOSTSFILE_REL for ${release}"
         log +l -c bldhostsfile -e "Check for nodename entry in ${NODENAMES}"
         return 1
      }
   }
   # If there is a nodename value from above use it.
   [[ -z "${HOSTSFILE_BLDNODE}" ]] && \
   { 
      if [[ -n "${NODENAMES_NODENAME}" ]] 
      then
         # Use value from call to bldnodenames_search.
         HOSTSFILE_BLDNODE=${NODENAMES_NODENAME}
      else
         # Calculate the value.
         HOSTSFILE_BLDNODE=${release#${HOSTSFILE_REL}}
      fi
      # Did not break out relname and nodename.
      [[ "${HOSTSFILE_BLDNODE}" = "${release}" ]] && \
      {
         log +l -c bldhostsfile -e "Cannot determine HOSTSFILE_BLDNODE for ${release}"
         log +l -c bldhostsfile -e "Check rel field entry in ${HOSTSFILE}"
         return 1
      }
   }

   #
   # BASE directories.
   #
   if [[ -z "${HOSTSFILE_BASE}" ]]
   then
      # no path supplied, revert back to old calculation method, don't
      # need to call bldnodenames if a previous call has already gotten
      # the value.
      [[ -z "${NODENAMES_NODE}" ]] && \
      {
         bldnodenames ${HOSTSFILE_BLDNODE} "${logerrors}"
         [[ $? -ne 0 ]] && return 1
      }
      HOSTSFILE_BASE="/${HOSTSFILE_HOST}/${HOSTSFILE_REL}/${NODENAMES_NODE}"
   elif [[ "${HOSTSFILE_BASE}" != /* ]]
   then
      # relative pathname, tack afs base onto front.
      HOSTSFILE_BASE="/${HOSTSFILE_HOST}/${HOSTSFILE_BASE}"
   fi

   #
   # PROD directories.
   #
   if [[ -z "${HOSTSFILE_PROD}" ]]
   then
      # no path supplied, revert back to old calculation method
      HOSTSFILE_PROD="${HOSTSFILE_BASE}/src"
   elif [[ "${HOSTSFILE_PROD}" != /* ]]
   then
      # relative pathname, tack base onto front.
      HOSTSFILE_PROD="${HOSTSFILE_BASE}/${HOSTSFILE_PROD}"
   fi

   # DELTA directories.
   #
   if [[ -z "${HOSTSFILE_DELTA}" ]]
   then
      # no path supplied, revert back to old calculation method
      HOSTSFILE_DELTA="${HOSTSFILE_BASE}/delta"
   elif [[ "${HOSTSFILE_DELTA}" != /* ]]
   then
      # relative pathname, tack afs base onto front.
      HOSTSFILE_DELTA="${HOSTSFILE_BASE}/${HOSTSFILE_DELTA}"
   fi

   #
   # BUILD directories.
   #
   if [[ -z "${HOSTSFILE_BUILD}" ]]
   then
      if [[ "${HOSTSFILE_ENVIRONMENT}" = "aix32" ]]
      then
         # no path supplied, aix32 environment, set to PROD directory.
         HOSTSFILE_BUILD="${HOSTSFILE_PROD}"
      else
         # no path supplied, ade environment, set to PROD/src directory.
         HOSTSFILE_BUILD="${HOSTSFILE_PROD}/src"
      fi
   elif [[ "${HOSTSFILE_BUILD}" != /* ]]
   then
      # relative pathname, tack PROD onto front.
      HOSTSFILE_BUILD="${HOSTSFILE_PROD}/${HOSTSFILE_BUILD}"
   fi

   #
   # OBJECT directories.
   #
   if [[ -z "${HOSTSFILE_OBJECT}" ]]
   then
      if [[ "${HOSTSFILE_ENVIRONMENT}" = "aix32" ]]
      then
         # no path supplied, aix32 environment, set to PROD/nls directory.
         HOSTSFILE_OBJECT="${HOSTSFILE_PROD}/nls"
      else
         # no path supplied, ade environment, set to PROD/obj directory.
         HOSTSFILE_OBJECT="${HOSTSFILE_PROD}/obj"
      fi
   elif [[ "${HOSTSFILE_OBJECT}" != /* ]]
   then
      # relative pathname, tack PROD onto front.
      HOSTSFILE_OBJECT="${HOSTSFILE_PROD}/${HOSTSFILE_OBJECT}"
   fi

   #
   # SHIP directories.
   #
   if [[ -z "${HOSTSFILE_SHIP}" ]]
   then
      if [[ "${HOSTSFILE_ENVIRONMENT}" = "aix32" ]]
      then
         # no path supplied, aix32 environment, set to PROD/nls/ship directory.
         HOSTSFILE_SHIP="${HOSTSFILE_PROD}/nls/ship"
      else
         # no path supplied, ade environment, set to PROD/ship directory.
         HOSTSFILE_SHIP="${HOSTSFILE_PROD}/ship"
      fi
   elif [[ "${HOSTSFILE_SHIP}" != /* ]]
   then
      # relative pathname, tack PROD onto front.
      HOSTSFILE_SHIP="${HOSTSFILE_PROD}/${HOSTSFILE_SHIP}"
   fi

   #
   # AFS directories.
   #
   if [[ "${HOSTSFILE_AFSBASE}" = "*" ]]
   then
      # if * is supplied for afs base, then afs directories do not exist.
      HOSTSFILE_AFSPROD="*"
      HOSTSFILE_AFSDELTA="*"
   else
      # if no value supplied for afs base, revert back to old calculation value.
      if [[ -z "${HOSTSFILE_AFSBASE}" ]]
      then
         HOSTSFILE_AFSBASE="${AFSBASE}/${HOSTSFILE_BLDNODE}"
         HOSTSFILE_AFSBASE="${HOSTSFILE_AFSBASE}/${HOSTSFILE_REL}"
      elif [[ "${HOSTSFILE_AFSBASE}" != /* ]] 
      then
         HOSTSFILE_AFSBASE="${AFSBASE}/${HOSTSFILE_AFSBASE}"
      fi
      if [[ -z "${HOSTSFILE_AFSPROD}" ]]
      then
         # no path supplied, revert back to old calculation method
         HOSTSFILE_AFSPROD="${HOSTSFILE_AFSBASE}/prod/src"
      elif [[ "${HOSTSFILE_AFSPROD}" != /* ]] 
      then
         # relative pathname, tack afs base onto front.
         HOSTSFILE_AFSPROD="${HOSTSFILE_AFSBASE}/${HOSTSFILE_AFSPROD}"
      fi
      if [[ -z "${HOSTSFILE_AFSDELTA}" ]]
      then
         # no path supplied, revert back to old calculation method
         HOSTSFILE_AFSDELTA="${HOSTSFILE_AFSBASE}/delta"
      elif [[ "${HOSTSFILE_AFSDELTA}" != /* ]]
      then
         # relative pathname, tack afs base onto front.
         HOSTSFILE_AFSDELTA="${HOSTSFILE_AFSBASE}/${HOSTSFILE_AFSDELTA}"
      fi
   fi

   return 0
}

. bldinitfunc

