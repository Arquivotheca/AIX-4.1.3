static char sccsid[] = "@(#)93	1.10  src/bos/usr/bin/syscall/syscall.c, cmdsh, bos41B, 412_41B_sync 12/15/94 12:12:35";
/*
 * COMPONENT_NAME: (CMDSH) Bourne shell and related commands
 *
 * FUNCTIONS:
 *
 * ORIGINS: 3, 26, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 * Copyright 1976, Bell Telephone Laboratories, Inc.
 */

/*
 * syscall -- system call interface program
 *
 * format is generally:
 *	syscall [-n] name [arg ...] [; name [arg ...] ] ... 
 * where "name" is the name of a system call.  If n is specified, syscall
 * performs the system call(s) n times.  Any args are passed to the system     
 * call without error checking.
 * generally of the format:
 *	0x nnn		hex constant nnn
 *	0 nnn		octal constant nnn
 *	nnn		decimal constant nnn
 *      + nnn
 *   	- nnn
 *   	"string		character string "string"
 * 	'string
 * 	\string		
 *	#string		length of the character string "string" 
 *	&&n		address of argument n (0=system call name)
 *	&n		address of nth byte in an internal 10k buffer
 *	$n		result of nth system call (n=0 is first)
 *	string	 	anything else is a literal character string	
 *
 * e.g. to simulate x=open("x",1); write(x,"hello",strlen("hello"))
 * one could do:
 *	syscall open x 1 \; write \$0 hello \#hello
 */

/* define to use faster MACROS instead of functions for performance */
#define _ILS_MACROS

#include "syscall_msg.h" 
static nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_SYSCALL,n,s) 

#include <stdio.h>
#include <sys/types.h>
#include <signal.h>
#include <ctype.h>
#include <locale.h>

#define MAXARG 7
#define EXTRA_ARGS	1024	/* extra for execl */
#define MAXRESULTS	20	/* number of syscall results to remember */
static char *args[MAXARG+EXTRA_ARGS];	/* args to each system call */
static char *fmts[MAXARG+EXTRA_ARGS];	/* format strings for debugging printf's */
#define NSYSCALLS (sizeof syscallnames) / sizeof syscallnames[0]
static char buff[10240];		/* buffer for misc stuff */
static int results[MAXRESULTS];	/* results of system calls */
static int count;			/* # of this system call on command line */
static int repeat = 1;			/* # of repetitions */

/*
 * System call names.
 */
static char *syscallnames[] = {
	"exit",			/*   0 = exit */
	"fork",			/*   1 = fork */
	"read",			/*   2 = read */
	"write",		/*   3 = write */
	"open",			/*   4 = open */
	"close",		/*   5 = close */
	"creat",		/*   6 = creat */
	"link",			/*   7 = link */
	"unlink",		/*   8 = unlink */
	"execv",		/*   9 = execv */
	"chdir",		/*  10 = chdir */
	"mknod",		/*  11 = mknod */
	"chmod",		/*  12 = chmod */
	"chown",		/*  13 = chown; now 3 args */
	"lseek",		/*  14 = lseek */
	"getpid",		/*  15 = getpid */
	"mount",		/*  16 = mount */
	"umount",		/*  17 = umount */
	"getuid",		/*  18 = getuid */
	"ptrace",		/*  19 = ptrace */
	"access",		/*  20 = access */
	"sync",			/*  21 = sync */
	"kill",			/*  22 = kill */
	"stat",			/*  23 = stat */
	"lstat",		/*  24 = lstat */
	"dup",			/*  25 = dup */
	"pipe",			/*  26 = pipe */
	"profil",		/*  27 = profil */
	"getgid",		/*  28 = getgid */
	"acct",			/*  29 = turn acct off/on */
	"ioctl",		/*  30 = ioctl */
	"reboot",		/*  31 = reboot */
	"symlink",		/*  32 = symlink */
	"readlink",		/*  33 = readlink */
	"execve",		/*  34 = execve */
	"umask",		/*  35 = umask */
	"chroot",		/*  36 = chroot */
	"fstat",		/*  37 = fstat */
	"getpagesize",		/*  38 = getpagesize */
	"vfork",		/*  39 = vfork */
	"getgroups",		/*  40 = getgroups */
	"setgroups",		/*  41 = setgroups */
	"getpgrp",		/*  42 = getpgrp */
	"setpgrp",		/*  43 = setpgrp */
	"setitimer",		/*  44 = setitimer */
	"wait",			/*  45 = wait */
	"getitimer",		/*  46 = getitimer */
	"gethostname",		/*  47 = gethostname */
	"sethostname",		/*  48 = sethostname */
	"getdtablesize",	/*  49 = getdtablesize */
	"dup2",			/*  50 = dup2 */
	"fcntl",		/*  51 = fcntl */
	"fsync",		/*  52 = fsync */
	"setpriority",		/*  53 = setpriority */
	"socket",		/*  54 = socket */
	"connect",		/*  55 = connect */
	"accept",		/*  56 = accept */
	"getpriority",		/*  57 = getpriority */
	"send",			/*  58 = send */
	"recv",			/*  59 = recv */
	"bind",			/*  60 = bind */
	"setsockopt",		/*  61 = setsockopt */
	"listen",		/*  62 = listen */
	"sigvec",		/*  63 = sigvec */
	"sigblock",		/*  64 = sigblock */
	"sigsetmask",		/*  65 = sigsetmask */
	"sigpause",		/*  66 = sigpause */
	"sigstack",		/*  67 = sigstack */
	"recvmsg",		/*  68 = recvmsg */
	"sendmsg",		/*  69 = sendmsg */
	"gettimeofday",		/*  70 = gettimeofday */
	"getrusage",		/*  71 = getrusage */
	"getsockopt",		/*  72 = getsockopt */
	"readv",		/*  73 = readv */
	"writev",		/*  74 = writev */
	"settimeofday",		/*  75 = settimeofday */
	"fchown",		/*  76 = fchown */
	"fchmod",		/*  77 = fchmod */
	"recvfrom",		/*  78 = recvfrom */
	"setreuid",		/*  79 = setreuid */
	"setregid",		/*  80 = setregid */
	"rename",		/*  81 = rename */
	"truncate",		/*  82 = truncate */
	"ftruncate",		/*  83 = ftruncate */
	"flock",		/*  84 = flock */
	"sendto",		/*  85 = sendto */
	"shutdown",		/*  86 = shutdown */
	"socketpair",		/*  87 = socketpair */
	"mkdir",		/*  88 = mkdir */
	"rmdir",		/*  89 = rmdir */
	"utimes",		/*  90 = utimes */
	"adjtime",		/*  91 = adjtime */
	"getpeername",		/*  92 = getpeername */
	"gethostid",		/*  93 = gethostid */
	"sethostid",		/*  94 = sethostid */
	"getrlimit",		/*  95 = getrlimit */
	"setrlimit",		/*  96 = setrlimit */
	"killpg",		/*  97 = killpg */
	"setquota",		/*  98 = setquota */
	"quota",		/*  99 = quota */
	"getsockname",		/* 100 = getsockname */
	"exect"			/* 101 = exect (exec with trace) */
};

/*
 * NAME: my_syscall
 * FUNCTION: does the actual system call. Arg0 is the index to the
 *           syscallnames array.
 */
static my_syscall(arg0, arg1, arg2, arg3, arg4, arg5, arg6)
char arg0[], arg1[], arg3[], arg4[], arg5[], arg6[];
{
	int code;

	switch((int)arg0)
	 	{
	case 0: 
		code = exit(arg1);
 		break; 
	case 1: 
		code = fork();
		break;
	case 2: 
		code = read(arg1, arg2, arg3);
		break;
	case 3: 
		code = write(arg1, arg2, arg3);
		break;
	case 4:
		code = open(arg1, arg2, arg3);
		break;
	case 5:
		code = close(arg1);
		break;
	case 6:
		code = creat(arg1, arg2);
		break;
	case 7:
		code = link(arg1, arg2);
		break;
	case 8:
		code = unlink(arg1);
		break;
	case 9:
		code = execv(arg1, arg2);
		break;
	case 10:
		code = chdir(arg1);
		break;
	case 11:
		code = mknod(arg1, arg2, arg3);
		break;
	case 12:
		code = chmod(arg1, arg2);
		break;
	case 13:
		code = chown(arg1, arg2, arg3);
		break;
	case 14:
		code = lseek(arg1, arg2, arg3);
		break;
	case 15:
		code = getpid();
		break;
	case 16:
		code = mount(arg1, arg2, arg3);
		break;
	case 17:
		code = umount(arg1);
		break;
	case 18:
		code = getuid();
		break;
	case 19:
		code = ptrace(arg1, arg2, arg3, arg4, arg5);
		break;
	case 20:
		code = access(arg1, arg2);
		break;
	case 21:
		code = sync();
		break;
	case 22:
		code = kill((pid_t)arg1, (int)arg2);
		break;
	case 23:
		code = stat(arg1, arg2);
		break;
	case 24:
		code = lstat(arg1, arg2);
		break;
	case 25:
		code = dup(arg1);
		break;
	case 26:
		code = pipe(arg1);
		break;
	case 27:
		code = profil(arg1, arg2, arg3, arg4);
		break;
	case 28:
		code = getgid();
		break;
	case 29:
		code = acct(arg1);
		break;
	case 30:
		code = ioctl(arg1, arg2, arg3);
		break;
	case 31:
		code = reboot(arg1);
		break;
	case 32:
		code = symlink(arg1, arg2);
		break;
	case 33:
		code = readlink(arg1, arg2, arg3);
		break;
	case 34:
		code = execve(arg1, arg2, arg3);
		break;
	case 35:
		code = umask(arg1);
		break;
	case 36:
		code = chroot(arg1);
		break;
	case 37:
		code = fstat(arg1, arg2);
		break;
	case 38:
		code = getpagesize();
		break;
	case 39:           
		code = vfork();
		break;	
	case 40:           
		code = getgroups(arg1, arg2);
		break;
	case 41:           
		code = setgroups(arg1, arg2);
		break;
	case 42:           
		code = getpgrp();
		break;
	case 43:           
		code = setpgrp();
		break;
	case 44:           
		code = setitimer(arg1, arg2, arg3);
		break;
	case 45:           
		code = wait(arg1);
		break;
	case 46:           
		code = getitimer(arg1, arg2);
		break;
	case 47:           
		code = gethostname(arg1, arg2);
		break;
	case 48:           
		code = sethostname(arg1, arg2);
		break;
	case 49:           
		code = getdtablesize();
		break;
	case 50:           
		code = dup2(arg1, arg2);
		break;
	case 51:           
		code = fcntl(arg1, arg2, arg3);
		break;
	case 52:           
		code = fsync(arg1);
		break;
	case 53:           
		code = setpriority(arg1, arg2, arg3);
		break;
	case 54:           
		code = socket(arg1, arg2, arg3);
		break;
	case 55:           
		code = connect(arg1, arg2, arg3);
		break;
	case 56:           
		code = accept(arg1, arg2, arg3);
		break;
	case 57:           
		code = getpriority(arg1, arg2);
		break;
	case 58:           
		code = send(arg1, arg2, arg3, arg4);
		break;
	case 59:           
		code = recv(arg1, arg2, arg3, arg4);
		break;
	case 60:           
		code = bind(arg1, arg2, arg3);
		break;
	case 61:           
		code = setsockopt(arg1, arg2, arg3, arg4, arg5);
		break;
	case 62:           
		code = listen(arg1, arg2);
		break;
	case 63:           
		code = sigvec(arg1, arg2, arg3);
		break;
	case 64:           
		code = sigblock(arg1);
		break;
	case 65:           
		code = sigsetmask(arg1);
		break;
	case 66:           
		code = sigpause(arg1);
		break;
	case 67:           
		code = sigstack(arg1, arg2);
		break;
	case 68:           
		code = recvmsg(arg1, arg2, arg3);
		break;
	case 69:           
		code = sendmsg(arg1, arg2, arg3);
		break;
	case 70:           
		code = gettimeofday(arg1, arg2);
		break;
	case 71:           
		code = getrusage(arg1, arg2);
		break;
	case 72:           
		code = getsockopt(arg1, arg2, arg3, arg4, arg5);
		break;
	case 73:           
		code = readv(arg1, arg2, arg3);
		break;
	case 74:           
		code = writev(arg1, arg2, arg3);
		break;
	case 75:           
		code = settimeofday(arg1, arg2);
		break;
	case 76:           
		code = fchown(arg1, arg2, arg3);
		break;
	case 77:           
		code = fchmod(arg1, arg2);
		break;
	case 78:           
		code = recvfrom(arg1, arg2, arg3, arg4, arg5, arg6);
		break;
	case 79:           
		code = setreuid(arg1, arg2);
		break;
	case 80:           
		code = setregid(arg1, arg2);
		break;
	case 81:           
		code = rename((char *)arg1, (char *)arg2);
		break;
	case 82:           
		code = truncate(arg1, arg2);
		break;
	case 83:           
		code = ftruncate(arg1, arg2);
		break;
	case 84:           
		code = flock(arg1, arg2);
		break;
	case 85:           
		code = sendto(arg1, arg2, arg3, arg4, arg5, arg6);
		break;
	case 86:           
		code = shutdown(arg1, arg2);
		break;
	case 87:           
		code = socketpair(arg1, arg2, arg3, arg4);
		break;
	case 88:           
		code = mkdir(arg1, arg2);
		break;
	case 89:           
		code = rmdir(arg1);
		break;
	case 90:           
		code = utimes(arg1, arg2);
		break;  
/* not available yet to V3.1
	case 91:           
		code = adjtime(arg1, arg2);
		break;
   not available yet to V3.1 */
	case 92:           
		code = getpeername(arg1, arg2, arg3);
		break;
	case 93:           
		code = gethostid();
		break;
	case 94:           
		code = sethostid(arg1);
		break;
	case 95:           
		code = getrlimit(arg1, arg2);
		break;
	case 96:           
		code = setrlimit(arg1, arg2);
		break;
	case 97:           
		code = killpg(arg1, arg2);
		break; 
/* not supported for V3.1
	case 98:           
		code = setquota(arg1, arg2);
		break;
	case 99:           
		code = quota(arg1, arg2, arg3, arg4);
		break;
   not supported for V3.1 */
	case 100:           
		code = getsockname(arg1, arg2, arg3);
		break;
	case 101:           
		code = exect(arg1, arg2, arg3);
		break;  
	default:
		fprintf(stderr, MSGSTR(NOSUPP, "system call not supported \n"));
	 	exit(1);	
		break;
		}
	return code;
}

/*
 * NAME: badsys
 * FUNCTION: prints an error message when it has bad argument to system
 *           call and exits.
 */
static badsys(void)
{
	error(MSGSTR(BADSYS, "bad system call"), 0);
}

/*
 * MAIN
 */
main(argc,argv)
	char **argv;
{

	register int i;
	register char *argp;

	(void ) setlocale(LC_ALL,"");

	catd = catopen(MF_SYSCALL,NL_CAT_LOCALE);


	for (i=1; i<MAXARG; ++i)
		args[i] = 0;
	--argc;
	++argv;
	while (argc > 0 && *(argp = argv[0]) == '-')
		{
		++argv;
		--argc;
		if (isdigit((int)argp[1]))
			{
			repeat = atoi(argp+1);
			break;
			}
		else 
			{
			fprintf(stderr, MSGSTR(USAGE, "syscall: Unknown arg (-%c) \n"), argp[1]);
			exit(1);
			}
		}
	signal(SIGSYS, (void (*)(int))badsys);
	do
		doonce(argc,argv);
	while (--repeat > 0);
	exit(0);
}

/* 
 * NAME: doonce
 * FUNCTION: performs all specified syscall call(s) once.
 */
static doonce(argc,argv) 
	register int argc;
	register char **argv;
{
	register int nargs;
	count = 0;
	for (; argc > 0; argc -= nargs, argv += nargs)
		nargs = docall(argc,argv,args);
}

/*
 * NAME: docall
 * FUNCTION: parses one syscall call and performs it.
 */
static docall(argc,argv,args)
	register char **argv;
	register char **args;
{
	register char *cmd;
	register int nargs;
	register int i;
	register char *argp;
	register int n;
	extern int errno;

	cmd=argv[0];
	for (i=1; i<argc; ++i)
		{
		register char *fmt = "0x%x";
		argp = argv[i];
		if (strcmp(argp,";") == 0)
			break;
		if (argp[0] == '0' && (argp[1] == 'x' || argp[1] == 'X'))
			args[i] = (char *) atox(argp+2);
		else if (argp[0] == '0' && isdigit((int)argp[1]))
			args[i] = (char *) atoo(argp);
		else if (isdigit((int)*argp) || ((*argp == '+' || *argp == '-') && isdigit((int)argp[1])))
			args[i] = (char *) atoi(argp);
		else if (argp[0] == '"' || argp[0] == '\'')
			args[i] = argp+1, fmt = "%s";
		else if (argp[0] == '&')
			{
			if (argp[1] == '&')
				args[i] = (char *) (args+atoi(argp+2));
			else
				args[i] = buff + atoi(argp+1);	/* use pre-allocated buffer */
			}
		else if (argp[0] == '$')
			{
			args[i] = (char *) results[argp[1] ? 
				((n = atoi(argp+1)) >= 0 ? n : count+n)
				: (count-1)];
			}
		else if (argp[0] == '#')
			args[i] = (char *) strlen(argp+1);
		else
			args[i] = argp, fmt = "%s";
		fmts[i] = fmt;
		}
	nargs = i+1;
	errno = 0;
	
	if (strcmp(cmd,"if") == 0)
		{
		if (i=ifcmd(args[1], args[2], args[3]))
			{
			args += 4;
			cmd = args[0];
			}
		else
			cmd = 0;
		}
	if (cmd == 0)
		;		/* we failed the if above */
	else if (strcmp(cmd,"=") == 0)
		{
		i=ifcmd(args[1], args[2], args[3]);
		if (nargs > 5)
			results[(int) args[4]] = i;
		}
	else
	if (strcmp(cmd,"sleep") == 0)
		i = sleep(args[1]);
	else
		{
		i = getsyscall(cmd);
		if (i < 0 || i>=NSYSCALLS)
			error(cmd, MSGSTR(NONSYS, ": not a system call"));
		
		args[0] = (char *) i;		/* system call number */
		i = my_syscall(args[0],args[1],args[2],args[3],args[4],args[5], args[6]);
		}

	if (count >= MAXRESULTS)
		error(MSGSTR(TOOMANY, "too many commands"), 0);
	results[count] = i;
	if (i == -1 && errno)
		{
		perror(cmd);
		exit(1);
		}
	++count;
	return(nargs);
}

/*
 * NAME: atox
 * FUNCTION: converts string in hex to binary.
 */
static int atox(ptr)
	register char *ptr;
{
	register int n = 0;
	register int c;

	for (; c = *ptr++;) {
		if (isdigit(c))
			c -= '0';
		else if ('a' <= c && c <= 'f')
			c -= 'a' - 10;
		else if ('A' <= c && c <= 'F')
			c -= 'A' - 10;
		else
			break;
		n = (n << 4) + c;
	}
	return (n);
}

/*
 * NAME: atoo
 * FUNCTION: convert string in octal to binary.
 */
static int atoo(ptr) 
	register char *ptr;
{
	register int n = 0;
	register int c;

	for (; c = *ptr++;) {
		if (('0' <= c) && (c <= '7'))
			c -= '0';
		else
			break;
		n = (n << 3) + c;
	}
	return (n);
}

/*
 * NAME: err
 * FUNCTION: prints an error message and exits.
 */
/*
 * err(s)
 * char *s;
 * {
 *	perror(s);
 *	exit(1);
 * }
 */

/*
 * NAME: getsyscall
 * FUNCTION: gets the index to the syscallnames array associated with the 
 * 	     command.
 */
static getsyscall(cmd) 
	register char *cmd;
{
	register int i;

	if (isdigit((int)cmd[0]))
		return(atoi(cmd));
	for (i=0; i<NSYSCALLS; ++i)
		if (strcmp(cmd,syscallnames[i]) == 0)
			return(i);
	return(-1);
}

/*
 * NAME: ifcmd
 * FUNCTION: handles the special case for if statement. This part is not
 * 	     documented in the aixv3 technical reference. It is kept for 
 *	     future reference.
 */ 
static ifcmd(lhs,op,rhs)
int lhs, rhs;
char *op;
{

#define OP(o) if (strcmp(op,"o") == 0) return(lhs o rhs)

	OP(>);
	OP(<);
	OP(==);
	OP(!=);
	OP(<=);
	OP(>=);
	OP(<<);
	OP(>>);
	OP(&);
	OP(^);
	OP(+);
	OP(-);
	OP(*);
	OP(/);
	OP(<<);
	OP(>>);

	if (strcmp(op,"!") == 0)
		return(fetch(lhs,rhs));

	error(MSGSTR(BADOP, "bad operator: "), op);
}

/* 
 * NAME: error
 * FUNCTION: prints an error message and exits.
 */
static error(msg,value)
char *msg, *value;
{
	switch(*value){
		case 0:
			write(2,msg,strlen(msg));
			break;
		default:
			write(2,msg,strlen(msg));
			write(2,value,strlen(value));
			break;
	}
	write(2,"\n",1);
	exit(1);
}

/*
 * NAME: fetch
 *	 This part is used by the special case for if statement. 
 */
static fetch(addr,size)
char *addr;
{
	if (size == sizeof (long))
		return(* (long *) addr);
	if (size == sizeof (short))
		return(* (unsigned short *) addr);
	if (size == sizeof (char))
		return(* (unsigned char *) addr);
	error(MSGSTR(RHS, "RHS of ! isn't 1, 2, or 4"), 0);
}
