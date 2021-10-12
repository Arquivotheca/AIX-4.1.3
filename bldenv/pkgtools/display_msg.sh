#!/bin/ksh
#
# @(#)87	1.1  src/bldenv/pkgtools/display_msg.sh, pkgtools, bos412, GOLDA411a  8/1/94  16:49:15
#
# COMPONENT_NAME: (PKGTOOLS) BAI Build Tools
#
# FUNCTIONS: display_msg
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1993
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

# NAME: display_msg
#
# FUNCTION: Display a message to stdout or stderr.
#
# INPUT: command line parameters:
#	 -C <commandName> - Name of command invoking display_msg function
#        -w - indicates message is a warning message
#	 -e - indicates message is a non-fatal error message
#	 -x - indicates message is a fatal error
#	 <string> - message text
#
# OUTPUT: formatted message to stdout or stderr
#
# SIDE EFFECTS:  RC set to non-zero return code if non-fatal error is
#		 received.
#
# RETURNS:  0 for informational or warning messages
#	    1 for non-fatal error message
#	    Exits the program with non-zero return for fatal errors.
#
# USAGE:  display_msg -C <commandName> [ -xew ] <messageString>
#
function display_msg
{
    typeset msgtype=i
    typeset msgcommand=""
    typeset message

    while getopts :xewC: option   # parse the command line parameters
    do
	case $option in
	    x)
		msgtype=x;;
	    e)
		msgtype=e;;
	    w)
		msgtype=w;;
	    C)
		msgcommand=$OPTARG;;
	    :)
		print -u2 "$0: $OPTARG requires a value"
		exit 2;;
	    \?)
		print -u2 "$0: unknown option $OPTARG"
		print -u2 -n "USAGE: $0 -C <commandName> [ -xew ] <string>";;
	esac
    done

    shift OPTIND-1

    message=$*              # message string.

    if [[ -z "${msgcommand}" ]]
    then
	print -u2 "$0:  -C option required"
	print -u2 "USAGE:  $0 [-xew] -C <commandName> <string>"
	exit 2
    fi

    case $msgtype in
	(x)
	    print -u2 "${msgcommand}:  FATAL ERROR: $message"
	    exit 1;;
	(e)
	    print -u2 "${msgcommand}:  ERROR: $message"
	    RC=1;
	    return 1;;
	(w)
	    print -u2 "${msgcommand}:  WARNING: $message"
	    return 0;;
	(i)
	    print "${msgcommand}:  $message"
	    return 0;;
    esac
}
