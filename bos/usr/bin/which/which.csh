#!/usr/bin/csh -f
# @(#)84        1.2.1.4  src/bos/usr/bin/which/which.csh, cmdscan, bos411, 9428A410j 5/9/94 17:41:55
#
# COMPONENT_NAME: (CMDSCAN) commands that scan files
#
# FUNCTIONS:
#
# ORIGINS: 26, 27
#
# (C) COPYRIGHT International Business Machines Corp. 1989, 1994
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# (Copyright statements and/or associated legends of other
# companies whose code appears in any part of this module must
# be copied here.)
#
#                                                                    
# Copyright (c) 1980 Regents of the University of California.
# All rights reserved.  The Berkeley software License Agreement
# specifies the terms and conditions for redistribution.
#
#	which.csh	5.2 (Berkeley) 7/29/85
#
#	which : tells you which program you get
#
set prompt = "% "
if ( -f ~/.cshrc) then
	source ~/.cshrc
endif
if ( $#argv == 0 ) then
	dspmsg which.cat 3 'usage: which command...\n'
	exit 1
endif
set noglob
set _exit_val = 0
foreach arg ( $argv )
    (alias $arg >& /dev/null)
    if ( $status == 0 ) then
	set alius = `alias $arg`
    else
	set alius = `;`
    endif
    switch ( $#alius )
	case 0 :
	    breaksw
	case 1 :
	    set arg = $alius[1]
	    breaksw
        default :
	    echo ${arg}: "	" aliased to $alius
	    continue
    endsw
    unset found
    if ( $arg:h != $arg:t ) then
	if ( -e $arg ) then
	    echo $arg
	else
	    dspmsg which.cat 1 '%1$s not found\n' "$arg"
	    set _exit_val = 1
	endif
	continue
    else
	foreach i ( $path )
	    if ( -x $i/$arg && ! -d $i/$arg ) then
		echo $i/$arg
		set found
		break
	    endif
	end
    endif
    if ( ! $?found ) then
	dspmsg which.cat 2 'no %1$s in %2$s\n' "$arg" "$path" 
	set _exit_val = 1
    endif
end
exit $_exit_val
