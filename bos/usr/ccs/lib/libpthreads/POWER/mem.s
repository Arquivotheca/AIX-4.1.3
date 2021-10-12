#static char sccsid[] = "@(#)64	1.1  src/bos/usr/ccs/lib/libpthreads/POWER/mem.s, libpth, bos411, 9428A410j 10/22/93 07:45:34";
#
# COMPONENT_NAME: libpth
# 
# FUNCTIONS:
#	mem_isync
#	mem_sync
#
#  ORIGINS:  83
#
# LEVEL 1, 5 Years Bull Confidential Information
#
# Temporary file, it must be in libc.

	.file "mem.s"

        S_PROLOG(mem_isync)
				# Called after lock taken (cs())
	isync			# stop pre-fetch
        br
        FCNDES(mem_isync)

        S_PROLOG(mem_sync)
				# Called from unlock primitives
        sync			
        br
        FCNDES(mem_sync)
