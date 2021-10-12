#
# @(#)03	1.16  src/bldenv/bldtools/bldnodenames.sh, bldprocess, bos412, GOLDA411a 1/21/94 11:26:36
#
# COMPONENT_NAME: (BLDPROCESS) BAI Build Process
#
# FUNCTIONS: bldnodenames
#            bldnodenames_checkfile
#            bldnodenames_search
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1992, 1993
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

#
# The following functions must be included when this function is included:
#    bldkshconst
#    bldloginit
#    bldinitfunc
#    bldnodenames
#

#
# NAME: bldnodenames
#
# FUNCTION: User interface to the nodenames.dat data base.  This routine
#           should be called to search the nodenames.dat data base for a
#           specified nodename.
#
# INPUT: nodename - CMVC name of release directory.
#        logerrors - log any error messages if set to TRUE, value
#                    of TRUE as defined in bldkshconst.
#
#
# OUTPUT: NODENAMES_NODE (global) - Source name of release directory.
#         NODENAMES_PRIORITIES (global) - Priorities tracks must be to be
#                                         acceptable for this build.
#
# SIDE EFFECTS:
#
# RETURNS: 0 if successful, 1 if no entry, and 2 if multiple entries.
#

#
# Variables exported to the user.
#
typeset -x NODENAMES_NODE=""
typeset -x NODENAMES_NODENAME=""
typeset -x NODENAMES_PRIORITIES=""

typeset -r NODENAMES_SEPARATOR="${SPACECHARACTER}"
					# Separator in nodenames.dat.
typeset    nodenames_node		# Source name of release directory
					# from nodenames.dat, used for error
                                        # checking.  It better match the 
                                        # release value searching on.

function bldnodenames
{
   typeset    nodename=$1
   typeset    line			# Line read from nodenames.dat.
   typeset -i line_count=0		# Number of lines read from
					# nodenames.dat.
   typeset    logerrors=$2
   typeset    prev_line			# Last valid line read from
					# nodenames.dat.

   bldnodenames_checkfile ${logerrors}
   [[ $? -ne 0 ]] && return 1

   # Find node entry in nodenames.dat.
   grep "^${nodename}${NODENAMES_SEPARATOR}" ${NODENAMES} | \
      while read line
      do
         prev_line=${line}
         ((line_count=${line_count}+1))
      done

   # No entry for node found.
   [[ ${line_count} -eq 0 ]] && \
   {
      [[ "${logerrors}" = "${TRUE}" ]] && \
         log +l -c bldnodenames -e "${nodename} not found in ${NODENAMES}"
      return 1
   }

   # Multiple enteries for node found.
   [[ ${line_count} -ne 1 ]] && \
   {
      [[ "${logerrors}" = "${TRUE}" ]] && \
         log +l -c bldnodenames \
            -e "Multiple ${nodename}'s found in ${NODENAMES}"
      return 2
   }

   print ${prev_line} | \
      read nodenames_node NODENAMES_NODE NODENAMES_PRIORITIES

   # Entries do not match.
   [[ "${nodename}" != "${nodenames_node}" ]] && \
   {
      logline="Searching for ${nodename} and found ${nodenames_node}"
      logline="${logline} in ${NODENAMES}"
      log +l -c bldnodenames -x "${logline}"
   }

   return 0
}

#
# NAME: bldnodenames_checkfile
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
function bldnodenames_checkfile
{
   typeset    logerrors=$1

   get_filename_definitions

   [[ ! -f "${NODENAMES}" ]] && \
   {
      [[ "${logerrors}" = "${TRUE}" ]] && \
         log +l -c bldnodenames_check -e "File ${NODENAMES} does not exist"
      return 1
   }

   return 0
}

#
# NAME: bldnodenames_search
#
# FUNCTION: User interface to the nodenames.dat data base.  This routine
#           should be called to search the nodenames.dat data base for all
#           node names.  Each nodename entry found is returned to the caller
#           in global environment variables (see OUTPUT below)
#           Initial call must have request set to SEARCH_FIRST this will
#           open the nodenames and return the first element.  Calls to get
#           data should all then be with request set to SEARCH_NEXT this
#           will return each successive element.  When the search is 
#           complete a request of SEARCH_STOP will close the nodenames file.
#
# INPUT: logerrors ($2) - log any error messages if set to TRUE, value
#                         of TRUE as defined in bldkshconst.
#        request ($1) - SEARCH_FIRST -- open and get first entry from
#                       nodenames file.
#                       SEARCH_NEXT -- get next entry from the nodenames file.
#                       SEARCH_STOP  -- close the nodenames file.
#
#
# OUTPUT: NODENAMES_NODE (global) - Source name of release directory.
#         NODENAMES_NODENAME (global) - CMVC name of release directory.
#         NODENAMES_PRIORITIES (global) - Priorities tracks must be to be
#                                         acceptable for this build.
#
# SIDE EFFECTS:
#
# RETURNS: 0 if successful, 1 unsuccessful.
#
function bldnodenames_search
{
   typeset    request=$1
   typeset    logerrors=$2

   if [[ "${request}" = "${SEARCH_STOP}" ]]
   then
      exec 9<&-
      return 0
   fi

   if [[ "${request}" = "${SEARCH_FIRST}" ]]
   then
      bldnodenames_checkfile "${logerrors}"
      [[ $? -ne 0 ]] && return 1
      exec 9< ${NODENAMES}
   fi

   NODENAMES_NODENAME=""
   # Skip blank lines and those with # as first non-white character
   while [[ -z "${NODENAMES_NODENAME}" || \
            "${NODENAMES_NODENAME#\#}" != "${NODENAMES_NODENAME}" ]]
   do
      read -u9 NODENAMES_NODENAME NODENAMES_NODE NODENAMES_PRIORITIES
      [[ $? -ne 0 ]] && return 1
   done

   if [[ -n "${NODENAMES_NODE}" ]] 
   then
      return 0
   else
      return 1
   fi
}
