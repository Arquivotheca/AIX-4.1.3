#!/bin/ksh
#
# @(#)36	1.2  src/bos/usr/lib/nim/methods/c_add_routes.sh, cmdnim, bos411, 9428A410j  7/14/94  14:55:39
#
#   COMPONENT_NAME: CMDNIM
#
#   FUNCTIONS: ./usr/lib/nim/methods/c_add_routes.sh
#		
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1993
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.

# ---------------------------- c_add_routes           
#
# NAME: c_add_routes
#
# FUNCTION:	 Imports the "new" niminfo file and adds any routes 
#
# RETURNS: (int)
#	1	= The niminfo file is zero length or does not exist
#	0	= always assumes no errors
#
# --------------------------------------------------------------------------- #
#
NIM_INFO="/etc/niminfo" 
ROUTE="/usr/sbin/route"
CHDEV="/usr/sbin/chdev"

[[ -s ${NIM_INFO} ]] && . ${NIM_INFO} || exit 1
[[ -z "${ROUTES}" ]] && exit 0

#
# set up route tables
#
for route_args in `echo "$ROUTES"`
do
	OIFS="$IFS"; IFS=':'
	set -- $route_args
	IFS="$OIFS"


	${CHDEV} -l inet0 -a route="net,${1},-netmask,${2},${3}" >/dev/null

done

#
# assume all is ok
# 
exit 0
