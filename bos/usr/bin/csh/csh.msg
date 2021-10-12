$ @(#)15	1.15.1.1  src/bos/usr/bin/csh/csh.msg, cmdcsh, bos411, 9434A411a 8/17/94 15:31:48
$ 
$ COMPONENT_NAME: CMDCSH  c shell(csh)
$
$ FUNCTIONS:
$
$ ORIGINS: 10,26,27,18,71
$
$ This module contains IBM CONFIDENTIAL code. -- (IBM
$ Confidential Restricted when combined with the aggregated
$ modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1985, 1994
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$
$ (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
$ ALL RIGHTS RESERVED 
$
$ OSF/1 1.1
$
$
$ 	Messages for the C shell.
$
$quote  "

$set MS_CSH
$ Do not translate "csh"

$ The following message is for dodirs in dir.c
M_DIRS		"Usage: dirs [ -l ]"
$ Do not translate "dirs [ -l ]"

$ The following messages are for dochngd in dir.c
M_NOHOME	"No home directory"
M_NOCHANGE	"Can't change to home directory"

$ The following message is for dopushd in dir.c
M_NODIR		"No other directory"

$ The following message is for dfind in dir.c
M_STACK		"Directory stack not that deep"

$ The following messages are for dopopd in dir.c
M_BADDIR	"Bad directory"
M_EMPTY		"Directory stack empty"

$ The following message is for Dfix1 in dol.c
M_AMBIG		"Ambiguous"

$ The following message is for Dword in dol.c
M_NOQUOTE	"Unmatched %c"

$ The following messages are for Dgetdol in dol.c
M_TOOLONG	"$< line too long"
$ Do not translate "$<"
M_NOFILE	"No file for $0"
$ Do not translate "$0"
M_VARSYN	"Variable syntax"
M_SUBOUT	"Subscript out of range"
M_DOLMOD	"Bad : mod in $"
$ Do not translate ":" or "$"

$ The following messages are for heredoc in dol.c
M_TERM		"<< terminator not found"
$ Do not translate "<<"
M_OVER		"Line overflow"
M_NOERR		"Unknown error %d occurred.\n"

$ The following message is for doexec in exec.c
M_NOMATCH	"No match"

$ The following message is for pexerr in exec.c
M_NOTFOUND	"Command not found"

$ The following messages are for exp5 in exp.c
M_DIV		"Divide by zero"
M_MOD		"Modulo by zero"

$ The following messages are for exp6 in exp.c
M_EXPR		"Expression syntax"
M_NOFNAME	"Missing file name"

$ The following messages are for func in func.c
M_TOOFEW	"Too few arguments"
M_TOOMANY	"Too many arguments"

$ The following message is for doonintr and donohup in func.c
M_CANT		"Can't from terminal"

$ The following message is for doalias in func.c
M_ALIAS		"Too dangerous to alias that"
$ Do not translate "alias"

$ The following messages are for doif in func.c
M_EMPTYIF	"Empty if"
$ Do not translate "if"
M_THEN		"Improper then"
$ Do not translate "then"

$ The following message is for doswitch in func.c
M_SYNERR	"Syntax error"

$ The following message is for dobreak, docontin and doend in func.c
M_WHILE		"Not in while/foreach"
$ Do not translate "while" or "foreach"

$ The following messages are for doforeach in func.c
M_INVVAR	"Invalid variable"
M_PAREN		"Words not surrounded by ()"
$ Do not translate "()"

$ The following messages are for getword in func.c
M_ZIF		"then/endif not found"
$ Do not translate "then" or "endif"
M_ZELSE		"endif not found"
$ Do not translate "endif"
M_ZSWITCH	"endsw not found"
$ Do not translate "endsw"
M_ZBREAK	"end not found"
$ Do not translate "end"
M_ZGOTO		"label not found"

$ The following message is for findlim in func.c
M_NOLIMIT	"No such limit"

$ The following message is for dolimit in func.c
M_NORAISE	"Limits cannot be raised\n"

$ The following message is for getval in func.c
M_SCALE		"Improper or unknown scale factor"

$ The following message is for islogin in func.c
M_NOTLOGIN	"Not login shell"

$ The following message is for limtail in func.c
M_BAD		"Bad scaling"

$ The following messages are for dosuspend in func.c
M_SUSPEND	"Can't suspend a login shell (yet)"
M_TTY		"Switching to new tty driver...\n"

$ The following message is for plim in func.c
M_UNLIMITED	"unlimited"

$ The following message is for expand in glob.c
M_UNKNOWN	"Unknown user: %s"

$ The following messages are for execbrc, amatch, Gmatch in glob.c
M_MISSBRK	"Missing ]"
$ Do not translate "]"
M_MISSBRC	"Missing }"
$ Do not translate "}"

$ The following message is for Gcat in glob.c
M_ARGS		"Arguments too long"

$ The following message is for addpath in glob.c
M_PATH		"Pathname or word too long"

$ The following message is for dobackp in glob.c
M_NOBACK	"Unmatched `"
$ Do not translate "`"

$ The following message is for psave in glob.c
M_WORD		"Word too long"

$ The following message is for pword in glob.c
M_COMSUB	"Too many words from ``"
$ Do not translate "``"

$ The following message is for word in lex.c
M_UNMATCH	"Unmatched "

$ The following message is for addla in lex.c
M_LABUF		"Expansion buffer overflow"

$ The following message is for getexcl in lex.c
M_BADEXCL	"Bad ! form"
$ Do not translate "!"

$ The following messages are for getsub in lex.c
M_SUB		"No previous substitution"
M_BADSUB	"Bad substitute"
M_BADLHS	"No previous left hand side"
M_BADRHS	"Right hand side too long"
M_BADEXCLM	"Bad ! modifier: "
$ Do not translate "!" or ":"

$ The following message is for dosub in lex.c
M_MODFAIL	"Modifier failed"

$ The following message is for subword in lex.c
M_OVFLO		"Substitute buffer overflow"

$ The following message is for getsel in lex.c
M_BADARG	"Bad ! argument selector"
$ Do not translate "!"

$ The following message is for gethent in lex.c
NO_SEARCH	"No previous search"

$ The following message is for noev in lex.c
M_NOEVENT	": Event not found"
$ Do not translate ":"

$ The following messages are for readc in lex.c
M_LOGOUT	"\nUse \"logout\" to logout.\n"
$ Do not translate "logout"
M_EXIT		"\nUse \"exit\" to leave csh.\n"
$ Do not translate "exit"

$ The following message is for calloc in misc.c
M_NOMEM		"Out of memory"

$ The following message is for udvar in misc.c
M_UNDEF		"Undefined variable"

$ The following message is for alias in parse.c
M_ALIASLOOP	"Alias loop"
$ Do not translate "Alias"

$ The following message is for asyn0 and syn0 in parse.c
M_RPARENS	"Too many )'s"
$ Do not translate ")"
M_LPARENS	"Too many ('s"
$ Do not translate "("

$ The following messages are for syn3 in parse.c
M_BADPAREN	"Badly placed ("
$ Do not translate "("
M_REDIRECT	"Missing name for redirect"
M_BADOUT	"Ambiguous output redirect"
M_HERE		"Can not << within ()'s"
$ Do not translate "<<" or "()"
M_INPUT		"Ambiguous input redirect"
M_BADPARENS	"Badly placed ()'s"
$ Do not translate "()"
M_NULL		"Invalid null command"

$ The following message is for pjwait in proc.c
M_BUG1		"csh software error: waiting for background job!\n"

$ The following message is for pflush in proc.c
M_BUG2		"csh software error: process flushed twice"

$ The following messages are for pprint in proc.c
M_RUN		"Running "
M_EXITNUM	"Exit %-16d"
M_DONE		"Done"
M_CORE		" (core dumped)"
M_WD		"(wd now: "
M_BUG3		"csh software error: unknown process status=%-9o"

$ The following message is for dojobs in proc.c
M_JOBS		"Usage: jobs [ -l ]"
$ Do not translate "jobs [ -l ]"

$ The following messages are for dokill in proc.c
M_BADSIG	"Bad signal number"
M_NOSIG		"Unknown signal; \"kill -l\" lists signals"
$ Do not translate "kill -l"

$ The following messages are for pkill in proc.c
M_STOPD		"%s: Already stopped\n"
M_ID		"Arguments should be job number or process id"

$ The following message is for panystop in proc.c
M_SJOBS		"\nThere are stopped jobs"

$ The following messages are for pfind in proc.c
M_CURJOB	"No current job"
M_PREVJOB	"No previous job"
M_SUCHJOB	"No such job"
M_NOJOB		"No job matches pattern"

$ The following message is for xpfork in sem.c
M_NOPROCS	"No more processes"

$ The following messages are for okpcntl in sem.c
M_NOJCNTL	"No job control in this shell"
M_NOSJCNTL	"No job control in subshells"

$ The following message is for mypipe in sem.c
M_NOPIPE	"Can't make pipe"

$ The following message is for chkclob in sem.c
M_FILE		"%s: File exists"

$ The following message is for doset in set.c
M_MISSP		"Missing )"
$ Do not translate ")"

$ The following message is for getinx in set.c
M_SUBERR	"Subscript error"

$ The following message is for getn in set.c
M_BADNUM	"Badly formed number"

$ The following message is for shift in set.c
M_NOMORE	"No more words"

$ The following message is for exportpath in set.c
M_LONGPATH	"Warning: long PATH truncated\n"
$ Do not translate "PATH"

$ The following message is for main in sh.c
M_NOTTY		"Warning: no access to tty; thus no job control in this shell...\n"
M_BYEBYE	"logout\n"
$ Do not translate "logout"

$ The following message is for dosource in sh.c
M_OPTION	"Unknown option : %s \n"

$ The following message is for pintr1 in sh.c
M_INTER		"Interrupted"

$ The following messages are for mailchk in sh.c
M_NEWMAIL	"You have new mail.\n"
M_MAIL		"You have mail.\n"
M_NEWMAILIN	"New mail in %s.\n"
M_MAILIN	"Mail in %s.\n"

$ The following messages are for file.c
YIKES1          "Too many names in password file\n"
YIKES2          "Too many files\n"
YIKES3		"\nTIOCSTI ioctl unsupported -- file name completion code not possible\n"

M_HITMISS	"%d hits, %d misses, %d%%\n"

M_SIGHUP	"Hangup"
M_SIGINT	"Interrupt"
M_SIGQUIT	"Quit"
M_SIGILL	"Illegal instruction"
M_SIGTRAP	"Breakpoint"
M_SIGABRT	"Abort process"
M_SIGEMT	"EMT instruction"
M_SIGFPE	"Floating exception"
M_SIGKILL	"Killed"
M_SIGBUS	"Bus error"
M_SIGSEGV	"Segmentation fault"
M_SIGSYS	"Bad system call"
M_SIGPIPE	"Broken pipe"
M_SIGALRM	"Alarm call"
M_SIGTERM	"Terminated"
M_SIGURG	"Urgent I/O condition"
M_SIGSTOP	"Stopped"
M_SIGTSTP	"Suspended"
M_SIGCONT	"Continue"
M_SIGCHLD	"Child exited"
M_SIGTTIN	"Background read"
M_SIGTTOU	"Background write"
M_SIGIO		"I/O completed"
M_SIGXCPU	"CPU limit exceeded"
M_SIGXFSZ	"File size limit exceeded"
M_SIGMSG	"Monitor mode data"
M_SIGWINCH	"Window size changed"
M_SIGPWR	"Power fail"
M_SIGUSR1	"User signal 1"
M_SIGUSR2	"User signal 2"
M_SIGPROF	"Profiling alarm"
M_SIGDANGER	"Crash imminent"
M_SIGVTALARM	"Virtual time alarm"
M_SIGMIGRATE	"Migrate process"
M_SIGPRE	"Programming exception"
M_SIGGRANT	"Monitor mode granted"
M_SIGRETRACT	"Monitor mode retracted"
M_SIGSOUND	"Sound control completed"
M_SIGSAK	"Secure attention key"
M_SIGBAD	"Bad trap"


$ These are for the loader builtins which are not supported by AIX.
M_BADINLIB	"inlib install failed.\n"
$ Do not translate this message.  It is not used by AIX.
M_BADRMLIB	"rmlib removal failed.\n"
$ Do not translate this message.  It is not used by AIX. 

$ The following message is for doumask in func.c
M_MASK		"Improper mask"

$ The following message is for trim in glob.c
M_INTERNAL	"Internal error - trim called\n"

M_FULLFS	"error: File system is full"
