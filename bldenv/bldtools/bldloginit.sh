#
# @(#)82	1.11  src/bldenv/bldtools/bldloginit.sh, bldprocess, bos412, GOLDA411a 1/13/94 19:38:06
# COMPONENT_NAME: (BLDPROCESS) BAI Build Process
#
# FUNCTIONS: logset 
#	     log
#	     
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1991
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

# Get the real terminal name, BLDTTY will be some form of error message,
# if no terminal is connected to this session.
# Make sure ENV is not set when invoking ksh, if the file named by ENV
# calls bldloginit we will enter a infinite loop.
old_ENV="${ENV}"
unset ENV
typeset -x BLDTTY="$(ksh -c 'tty < /dev/tty' 2>&1)"
export ENV="${old_ENV}"

# NAME: logset
#
# FUNCTION: This is an exported build process environment function which calls
#	    the command bldlogset with all the parameters passed to it (logset)
#	    This also redirects STDERR in the calling function to the specified
#	    log file.
#
# INPUT: Since this function is primarily an ksh interface to 'bldlogset', All
#	 the options and optional arguments intended to be passed to bldlogset 
#	 command will be the input. See man pages of bldlogset for more 
#	 details on the input options.
#
# OUTPUT: none
#
# EXECUTION ENVIRONMENT: Build process environment.
#
# RETURNS: Error codes from 'bldlogset' command.
#

function logset
{
	typeset logfile logrc
	logfile=$(bldlogset $*)   
	logrc=$?
	if (($logrc >= 1))
	then
		rc=$logrc       # rc has been declared in the calling function.
	fi
	if (($logrc == 2))
	then
		exit 2
	else
		# STDERR is piped to a co-process which reads and prints the
		# STDERR messages to both the terminal and the log file.
                # Check to see if /dev/tty can really be written to, BLDTTY
                # will be '/dev/*' if writable.  If not writable BLDTTY will
                # be some form of error message.
                if [[ -n "${BLDTTY}" && -z "${BLDTTY%%/dev/*}" ]]
                then
			( exec 4>>"$logfile"; \
                          while read -r line; \
                          do print -ru4 - "$line"; \
                             print -r - "$line" > /dev/tty; \
                          done ) |&
		else
			( exec 4>>"$logfile"; \
                          while read -r line; \
                          do print -ru4 - "$line"; \
                          done ) |&
		fi
		exec 2>&p
		return $logrc
	fi
}
typeset -fx logset

#
# NAME: log
#
# FUNCTION: This is an exported build process environment function which calls
#	    the command 'bldlog' with all the parameters passed to it (logset)
#
# INPUT: Since this function is primarily an ksh interface to 'bldlog', All
#	 the options and optional arguments intended to be passed to 'bldlog' 
#	 command will be the input. See man pages of 'bldlog' for more 
#	 details on the input options.
#
# OUTPUT: none
#
# EXECUTION ENVIRONMENT: Build process environment.
#
# RETURNS: Error codes from 'bldlog' command.
#

function log
{
	typeset logrc
	bldlog $*
	logrc=$?
	if (($logrc >= 1))
	then
		rc=$logrc
	fi
	if (($logrc == 2))
	then
		kill -QUIT $$
		exit 2         # Exits from the calling function also.
	else
		return $logrc
	fi
}
typeset -fx log
