# @(#)03	1.1  src/bos/kernel/ml/POWER/signal.m4, sysml, bos411, 9428A410j 12/7/93 17:18:53
#*****************************************************************************
#
# COMPONENT_NAME: (SYSML) Kernel Assembler Defines
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1993
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#****************************************************************************

#****************************************************************************
#
#  NAME:    signal.m4
#
#           This is the assembly language version of "<sys/signal.h>"
#
#****************************************************************************
	
	.set	SIGHUP,	   1	# hangup, generated when terminal disconnects
	.set	SIGINT,	   2	# interrupt, generated from terminal special char
	.set	SIGQUIT,   3	# (*) quit, generated from terminal special char 
	.set	SIGILL,	   4	# (*) illegal instruction (not reset when caught)
	.set	SIGTRAP,   5	# (*) trace trap (not reset when caught) 
	.set	SIGABRT,   6	# (*) abort process 
	.set 	SIGEMT,	   7	# EMT intruction 
	.set	SIGFPE,	   8	# (*) floating point exception 
	.set	SIGKILL,   9	# kill (cannot be caught or ignored) 
	.set	SIGBUS,	  10	# (*) bus error (specification exception) 
	.set	SIGSEGV,  11	# (*) segmentation violation 
	.set	SIGSYS,	  12	# (*) bad argument to system call 
	.set	SIGPIPE,  13	# write on a pipe with no one to read it 
	.set	SIGALRM,  14	# alarm clock timeout 
	.set	SIGTERM,  15	# software termination signal 
	.set	SIGURG,	  16	# (+) urgent contition on I/O channel 
	.set	SIGSTOP,  17	# (@) stop (cannot be caught or ignored) 
	.set	SIGTSTP,  18	# (@) interactive stop 
	.set	SIGCONT,  19	# (!) continue (cannot be caught or ignored) 
	.set 	SIGCHLD,  20	# (+) sent to parent on child stop or exit 
	.set 	SIGTTIN,  21	# (@) background read attempted from control terminal
	.set 	SIGTTOU,  22	# (@) background write attempted to control terminal 
	.set 	SIGIO,	  23	# (+) I/O possible, or completed 
	.set 	SIGXCPU,  24	# cpu time limit exceeded (see setrlimit()) 
	.set 	SIGXFSZ,  25	# file size limit exceeded (see setrlimit()) 
	.set 	SIGMSG,   27	# input data is in the HFT ring buffer 
	.set 	SIGWINCH, 28	# (+) window size changed 
	.set 	SIGPWR,   29	# (+) power-fail restart 
	.set 	SIGUSR1,  30	# user defined signal 1 
	.set 	SIGUSR2,  31	# user defined signal 2 
	.set 	SIGPROF,  32	# profiling time alarm (see setitimer) 
	.set 	SIGDANGER, 33	# system crash imminent; free up some page space 
	.set 	SIGVTALRM, 34	# virtual time alarm (see setitimer) 
	.set 	SIGMIGRATE, 35	# migrate process 
	.set 	SIGPRE,	  36	# programming exception 
	.set 	SIGVIRT,  37	# AIX virtual time alarm 
	.set 	SIGKAP,   60   	# keep alive poll from native keyboard 
	.set 	SIGGRANT, 60    # HFT monitor mode granted 
	.set 	SIGRETRACT, 61   # HFT monitor mode should be relinguished 
	.set 	SIGSOUND, 62    # HFT sound control has completed 
	.set 	SIGSAK,   63	# secure attention key 

