$ @(#)86	1.28  src/bos/usr/bin/ps/ps.msg, cmdstat, bos411, 9428A410j 2/4/94 03:25:13
$
$ COMPONENT_NAME: (CMDSTAT) status
$
$ FUNCTIONS:
$
$ ORIGINS: 27 83
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1989, 1994
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$
$ LEVEL 1, 5 Years Bull Confidential Information
$

$
$       error/informational text for the ps command
$

$       NOTE: the portion of a text string that begins with "ps:" should
$               not be changed.  This is the command name.
$

$quote "

$set MS_PS

$
$ The following string gives the syntax of the ps command.
$
$ SYSV_USAGE: AIX version
$ BSD_USAGE:  BSD version
$
SYSV_USAGE "Usage: ps [-Aaedfklm] [-n namelist] [-F Format] \
[-o specifier[=header],...]\n\t\t[-p proclist][-G|-g grouplist] \
[-t termlist] [-U|-u userlist] \n"
BSD_USAGE "Usage: ps [aceglnsuvwxU] [t tty] [processnumber]\n";

$ The following messages are for the error messages.
$	0509-001, 0509-002, 0509-003, 0509-004, 0509-005,
$	0509-006, 0509-007, 0509-008, 0509-009, 0509-010,
$	0509-011, 0509-042, 0509-043, 0509-044, 0509-045,
$	0509-046, 0509-047, 0509-048, 0509-049, 0509-060,
$ 	0509-061
$
$	$ XXXX-XXX                 message number
$	MSGID "XXXX-XXX ......."   messages
$	$ ......                   note for translating
$
$ 0509-001
NO_MEM "ps: 0509-001 There is not enough memory available now.\n\
\tTry again later or\n\
\tfollow local problem reporting procedures.\n"
$ Do not translate the command name "ps"
$ 0509-002
OPEN_ERR "ps: 0509-002 Cannot open %s.\n\
\tCheck path name and permissions.\n"
$ Do not translate the command name "ps"
$ 0509-003
NO_MEM2 "ps: 0509-003 There is not enough memory available now.\n\
\tTry again later or\n\
\tfollow local problem reporting procedures.\n"
$ Do not translate the command name "ps"
$ 0509-004
OPEN_ERR2 "ps: 0509-004 Cannot open the directory %s.\n\
\tCheck path name and permissions.\n"
$ Do not translate the command name "ps"
$ 0509-005
CH_ERR "ps: 0509-005 Cannot change the current directory to %s.\n\
\tCheck path name and permissions.\n"
$ Do not translate the command name "ps"
$ 0509-006
NO_MEM3 "ps: 0509-006 There is not enough memory for %d users.\n\
\tTry again later or\n\
\tfollow local problem reporting procedures.\n"
$ "%d" is the number of user
$ Do not translate the command name "ps"
$ 0509-007
OPEN_ERR3 "ps: 0509-007 %s cannot be opened.\n\
\tCheck path name and permissions.\n"
$ Do not translate the command name "ps"
$ 0509-008
READ_ERR "ps: 0509-008 Cannot read the U area.\n\
\tFollow local problem reporting procedures.\n"
$ Do not translate the command name "ps"
$ 0509-009
SEEK_ERR "ps: 0509-009 An error occurred on the lseek process.\n\
\tFollow local problem reporting procedures.\n"
$ Do not translate the command name "ps"
$ 0509-010
READ_ERR2 "ps: 0509-010 An error occurred on the read process.\n\
\tCheck path name and permissions.\n"
$ Do not translate the command name "ps"
$ 0509-011
WRITE_ERR "ps: 0509-011 An error occurred on the write process.\n\
\tCheck path name and permissions.\n"
$ Do not translate the command name "ps"
$ 0509-012
GETWD_ERR "ps: 0509-012 An error occurred in a call to getwd.\n"
$ Do not translate the command name "ps" or the function name "getwd"
$ 0509-042
WRTERR	"ps: 0509-042 Cannot open %s for writing.\n"
$ note: WRTERR is nolonger being printed out.
$ Do not translate the command name "ps"
$ 0509-043
NOTTY	"ps: 0509-43 Cannot identify your terminal\n"
$ Do not translate the command name "ps"
$ 0509-044
NOENV	"ps: 0509-044 Cannot get environment\n"
$ Do not translate the command name "ps"
$ 0509-045
UNIQ	"ps: 0509-045 Specify only one of s,l,v and u\n"
$ Do not translate the command name "ps"
$ 0509-046
ALTNM1 	"ps: 0509-046 The alternate name list [-n] is not supported.\n"
$ Do not translate the command name "ps"
$ 0509-047
ALTNM2 	"ps: 0509-047 The alternate core file [-c] is not supported.\n"
$ Do not translate the command name "ps"
$ 0509-048
LISTERR "ps: 0509-048 Flag %s was used with invalid list.\n"
$ "%s" is the flag name, "-F" or "-o" maybe
$ Do not translate the command name "ps"
$ 0509-049
MEM_OPENERR "ps: 0509-049 Cannot open dump file %s.\n"
$ "%s" is the file name, "/dev/mem" or "/dev/kmem"
$ Do not translate the command name "ps"
$ 0509-060
MEM_LSEEKERR "ps: 0509-060 An error occurred on the lseek process to %s.\n"
$ "%s" is the file name, "/dev/mem" or "/dev/kmem"
$ A error with lseek() to /dev/kmem.
$ Do not translate the command name "ps"
$ 0509-061
MEM_READERR "ps: 0509-061 An error occurred on the read process to %s.\n"
$ "%s" is the file name, "/dev/mem" or "/dev/kmem"
$ A error with read() to /dev/kmem.
$ Do not translate the command name "ps"

$
$ The following messages are column heads.
$ DO NOT TRANSLATE COLUMN HEADS AT ALL.
$
$     Headers		Description
$	F		The flags associated with the process.
$	S		The state of the process.
$	USER		Name of the owner of the process.
$	RUSER		The real user ID of the process with -F|-o flag.
$	USER		The effective user ID of the process with -F|-o flag.
$	UID		The user ID of the process owner.
$	PID		The process ID of the process.
$	PPID		The process ID of the parent process.
$	C		Processor utilization for scheduling.
$	STIME		Starting time of the process.
$	PRI		The priority of the process.
$	NI		Nice value.
$	ADDR		The segment number of the process stack, if normal.
$			If a kernel process, the address of the process data
$			area.
$	SZ		The size in blocks of the core image of the process.
$	WCHAN		The event for which the process is wating or sleeping.
$	TTY, TT		The controlling workstation for the process.
$	TIME		The total execution time for the process.
$	CMD, COMMAND	The command name (with or without the arguments).
$	%CPU		CPU utilization of the process.
$	NICE		Process sheduling increment.
$	SIZE, VSZ	Virtual size of the process (in 1024 byte units).
$	RSS		Real memory (resident set) size of the process (in
$			1024 byte units).
$	LIM		Soft limit on memory used.
$	TSIZ		Size of text (shared program) image.
$	TRS		Size of resident (real memory) set of text.
$	%MEM		Percentage of real memory used by this process.
$	PGIN		Number of disk I/Os resulting from references by the 
$			process to pages not loaded in core.
$	ELAPSED		The elapsed time since the process was started.
$	RGROUP		The real group ID of the process.
$	GROUP, Group	The effective group ID of the process.
$	PGID		The decimal value of the process group ID.
$	TID		The thread ID.
$	SC		The suspend count of the thread.
$	BND		The processor to which the process/thread is bound.
$	THCNT		The number of threads of a process.
$	SCH		The scheduling policy of a thread or a process.
$
$	MSGID    "........"	   column heads
$	$ ......                   note for translating
$	
HEADR1	 "       F S     UID %*s "
$ Do not translate these column heads. Do not change the spacing.
HEADR2	"%*s   C PRI NI ADDR    SZ    WCHAN %*s    TTY  TIME CMD\n"
$ Do not translate these column heads. Do not change the spacing.
HEADR3	"     UID %*s %*s   C %*s    TTY  TIME CMD\n"
$ Do not translate these column heads. Do not change the spacing.
HEADR4	"       F S %*s %*s %*s   C PRI NI ADDR    SZ    WCHAN    TTY  TIME CMD\n"
$ Do not translate these column heads. Do not change the spacing.
HEADR5  " %*s    TTY  TIME CMD\n"
$ Do not translate these column heads. Do not change the spacing.
HEAD_STACK	" SSIZ "
$ Do not translate these column heads. Do not change the spacing.
HEAD_V	"    USER %*s    TTY PGIN  SIZE   RSS   LIM  TSIZ   TRS %%CPU %%MEM CMD\n"
$ Do not translate these column heads. Do not change the spacing.
HEADR6	"    USER %*s %%CPU %%MEM    SZ   RSS    TTY STAT  TIME CMD\n"
$ Do not translate these column heads. Do not change the spacing.
$ HEADR7  "  UID %*s    TTY  TIME CMD\n"
HEADR7  " %*s %*s    TTY  TIME CMD\n"
$ Do not translate these column heads. Do not change the spacing.
HEADR8  " %*s  STAT   TTY  TIME COMMAND\n"
$ Do not translate these column heads. Do not change the spacing.

$
$ The following are the label name of header.
$ DO NOT TRANSLATE these label names at all.
H_F 		"F"
H_S 		"S"
H_USER		"USER"
H_RUSER		"RUSER"
H_UID		"UID"
H_PID		"PID"
H_PPID		"PPID"
H_C		"C"
H_STIME		"STIME"
H_PRI 		"PRI"
H_NI		"NI"
H_ADDR		"ADDR"
H_SZ		"SZ"
H_WCHAN		"WCHAN"
H_TTY		"TTY"
H_TT		"TT"
H_TIME		"TIME"
H_COMMAND	"COMMAND"
H_CMD 		"CMD"
H_PCPU		"%CPU"
H_NICE		"NI"
H_SIZE 		"SIZE"
H_VSZ		"VSZ"
H_RSS 		"RSS"
H_LIM 		"LIM"
H_TSIZ 		"TSIZ"
H_TRS 		"TRS"
H_PMEM 		"%MEM"
H_PGIN 		"PGIN"
H_ELAPSED	"ELAPSED"
H_RGROUP	"RGROUP"
H_GROUP		"GROUP"
H_GROUP2	"Group"
H_PGID		"PGID"
H_SSIZ 		"SSIZ"
H_STAT 		"STAT"
H_CP 		"CP"
H_SL		"SL"
H_RE		"RE"
H_PAGEIN	"PAGEIN"
H_TID		"TID"
H_SCOUNT	"SC"
H_BIND		"BND"
H_SCHED		"SCH"
H_THCOUNT	"THCNT"
H_ST		"ST"
$ Do not translate above label names again.

USAGE5 "Usage: ps [-Aaedfkl] [-F format] [-G|-g grouplist] [-p proclist] \n\
\t\t[-t termlist] [-U|-u userlist] \n"
USAGE6 "Usage: ps [-Aaedfkl] [-o format] ... [-G|-g grouplist] [-p proclist] \n\
\t\t[-t termlist] [-U|-u userlist] \n"
$ Do not translate the command name "ps"
$ Do not translate the flags, "-aedfkl", "-A", "-U", etc...

