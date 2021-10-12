#!/usr/bin/psh
# @(#)36	1.2  src/bos/usr/bin/ksh/exec-builtin.sh, cmdksh, bos41J, 9523A_all 6/1/95 00:09:57
#
# COMPONENT_NAME: (CMDKSH) Korn Shell
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1993, 1995
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

#
# This shell script gets installed in /usr/bin to provide an exec()-able
# version of certain shell builtins:
#	alias		command		getopts		umask
#	bg		fc		jobs		unalias
#	cd		fg		read		wait
#	hash		type
#
# These are also builtins, but already have historical version in /usr/bin:
#	false		kill		newgrp		true
#
command=`/usr/bin/basename $0`		# Invoke the ksh builtin
if [ "$command" = "type" ]
then
	whence -v "$@"
elif [ "$command" = "hash" ]
then
	alias -t - "$@"
else
	$command "$@"
fi
