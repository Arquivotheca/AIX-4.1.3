/* @(#)82	1.38.1.10  src/bos/usr/include/unistd.h, incstd, bos411, 9428A410j 6/22/94 09:23:42 */

/*
 * COMPONENT_NAME: (INCSTD) Standard Include Files
 *
 * FUNCTIONS:
 *
 * ORIGINS: 3 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1984 AT&T	
 * All Rights Reserved  
 *
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	
 * The copyright notice above does not evidence any   
 * actual or intended publication of such source code.
 */

#ifndef _H_UNISTD
#define _H_UNISTD

#ifndef _H_STANDARDS
#include <standards.h>
#endif

#ifndef _H_TYPES
#include <sys/types.h>  /* for time_t */
#endif

#ifndef _H_ACCESS
#include <sys/access.h>	/* for the "access" function */
#endif

/*
 * POSIX requires that certain values be included in unistd.h.  It also
 * requires that when _POSIX_SOURCE is defined only those standard
 * specific values are present.  This header includes all the POSIX
 * required entries.
 */

#ifdef _POSIX_SOURCE


/* Symbolic constants for the "lseek" function: */
#ifndef SEEK_SET
#define SEEK_SET 0	/* Set file pointer to "offset" */
#define SEEK_CUR 1	/* Set file pointer to current plus "offset" */
#define SEEK_END 2	/* Set file pointer to EOF plus "offset" */
#endif /* SEEK_SET */

#ifdef _NO_PROTO

#ifndef _KERNEL
extern int access();
extern unsigned int alarm();
extern int chdir();
extern int chown();
extern int close();
extern char *ctermid();
extern char *cuserid();
extern int dup();
extern int dup2();
extern int execl();
extern int execv();
extern int execle(); 
extern int execve();
extern int execlp();
extern int execvp();
extern void _exit();
extern pid_t fork();
extern long fpathconf();
extern char *getcwd();
extern gid_t getegid();
extern uid_t geteuid();
extern gid_t getgid();
extern int getgroups();
extern char *getlogin();
extern pid_t getpgrp();
extern pid_t getpid();
extern pid_t getppid();
extern uid_t getuid();
extern int isatty();
extern int link();
extern off_t lseek();
extern long pathconf();
extern int pause();
extern int pipe();
extern int read();
extern int rmdir();
extern int setgid();
extern int setpgid();
extern int setsid(); 
extern int setuid();
extern unsigned int sleep();
extern long sysconf();
extern pid_t tcgetpgrp();
extern int tcsetpgrp();
extern char *ttyname();
extern int unlink();
extern int write(); 
#endif		/* !_KERNEL	*/

#else		/* POSIX required prototypes */

#ifndef _KERNEL
extern int access(const char *, int);
extern unsigned int alarm(unsigned int);
extern int chdir(const char *);
extern int chown(const char *, uid_t, gid_t);
extern int close(int);
extern char *ctermid(char *);
extern char *cuserid(char *);
extern int dup(int);
extern int dup2(int, int);
extern int execl(const char *, const char *, ...);
extern int execv(const char *, char *const []);
extern int execle(const char *, const char *, ...); 
extern int execve(const char *, char *const [], char *const []);
extern int execlp(const char *, const char *, ...); 
extern int execvp(const char *, char *const []);
extern void _exit(int);
extern pid_t fork(void);
extern long fpathconf(int, int);
extern char *getcwd(char *, size_t);
extern gid_t getegid(void);
extern uid_t geteuid(void);
extern gid_t getgid(void);
extern int getgroups(int, gid_t []);
extern char *getlogin(void);
#ifndef _BSD
extern pid_t getpgrp(void);
#endif /* _BSD */
extern pid_t getpid(void);
extern pid_t getppid(void);
extern uid_t getuid(void);
extern int isatty(int);
extern int link(const char *, const char *);
extern off_t lseek(int, off_t, int);
extern long pathconf(const char *, int);
extern int pause(void);
extern int pipe(int []);
extern ssize_t read(int, void *, size_t);
extern int rmdir(const char *);
extern int setgid(gid_t);
extern int setpgid(pid_t, pid_t);
extern pid_t setsid(void);
extern int setuid(uid_t);
extern unsigned int sleep(unsigned int);
extern long sysconf(int);
extern pid_t tcgetpgrp(int);
extern int tcsetpgrp(int, pid_t);
extern char *ttyname(int);
extern int unlink(const char *);
extern ssize_t write(int, const void *, size_t); 
#endif		/* !_KERNEL	*/
#endif		/* !_NO_PROTO	*/

#define STDIN_FILENO	0
#define STDOUT_FILENO	1
#define STDERR_FILENO	2

#define _POSIX_JOB_CONTROL	1
#define _POSIX_SAVED_IDS	1
#define _POSIX_VERSION		199009L
#define _POSIX2_VERSION		199209L
#define _POSIX2_C_VERSION	199209L

#define _POSIX_REENTRANT_FUNCTIONS		1
#define _POSIX_THREADS				1
#undef  _POSIX_THREAD_ATTR_STACKADDR
#define _POSIX_THREAD_ATTR_STACKSIZE		1
#undef  _POSIX_THREAD_FORKALL
#define _POSIX_THREAD_PRIORITY_SCHEDULING	1
#undef  _POSIX_THREAD_PRIO_INHERIT
#undef  _POSIX_THREAD_PRIO_PROTECT
#undef  _POSIX_THREAD_PROCESS_SHARED

#define _POSIX_CHOWN_RESTRICTED	 0
#define _POSIX_VDISABLE 	 0xFF
#define _POSIX_NO_TRUNC	    	 0

#ifndef NULL 
#define NULL	0
#endif

/* arguments for the confstr() function */

#define _CS_PATH	1
#define _CSPATH		"/usr/bin"

/* arguments for the pathconf() function */

#define _PC_CHOWN_RESTRICTED	10
#define _PC_LINK_MAX		11
#define _PC_MAX_CANON		12
#define _PC_MAX_INPUT		13
#define _PC_NAME_MAX		14
#define _PC_NO_TRUNC		15
#define _PC_PATH_MAX		16
#define _PC_PIPE_BUF		17
#define _PC_VDISABLE 		18

/* arguments for the sysconf() function, the defined numbers are used as 
 * array index in sysconf().
 *
 * POSIX.1(1990), Table 4-2 
 */
#define _SC_ARG_MAX			0
#define _SC_CHILD_MAX			1
#define _SC_CLK_TCK			2
#define _SC_NGROUPS_MAX			3
#define _SC_OPEN_MAX			4
#define _SC_STREAM_MAX			5
#define _SC_TZNAME_MAX			6
#define _SC_JOB_CONTROL			7
#define _SC_SAVED_IDS			8
#define _SC_VERSION			9

/* POSIX.1(1990), Table 2-3, required by command getconf */
	
#define _SC_POSIX_ARG_MAX		10
#define _SC_POSIX_CHILD_MAX		11	
#define _SC_POSIX_LINK_MAX		12
#define _SC_POSIX_MAX_CANON		13
#define _SC_POSIX_MAX_INPUT		14
#define _SC_POSIX_NAME_MAX		15
#define _SC_POSIX_NGROUPS_MAX		16
#define _SC_POSIX_OPEN_MAX		17
#define _SC_POSIX_PATH_MAX		18
#define _SC_POSIX_PIPE_BUF		19
#define _SC_POSIX_SSIZE_MAX		20
#define _SC_POSIX_STREAM_MAX		21
#define _SC_POSIX_TZNAME_MAX		22

/* POSIX.2 (Draft 10), Table 41)	*/

#define _SC_BC_BASE_MAX			23
#define _SC_BC_DIM_MAX			24
#define _SC_BC_SCALE_MAX		25
#define _SC_BC_STRING_MAX		26
#define _SC_EQUIV_CLASS_MAX		27
#define _SC_EXPR_NEST_MAX		28
#define _SC_LINE_MAX			29
#define _SC_RE_DUP_MAX			30
#define _SC_2_VERSION			31
#define _SC_2_C_DEV			32
#define _SC_2_FORT_DEV			33
#define _SC_2_FORT_RUN			34
#define _SC_2_LOCALEDEF			35
#define _SC_2_SW_DEV			36

/* POSIX.2 (Draft 10), Table 13)	*/

#define _SC_POSIX2_BC_BASE_MAX		37
#define _SC_POSIX2_BC_DIM_MAX		38
#define _SC_POSIX2_BC_SCALE_MAX		39
#define _SC_POSIX2_BC_STRING_MAX	40
#define _SC_POSIX2_EQUIV_CLASS_MAX	41
#define _SC_POSIX2_EXPR_NEST_MAX	42
#define _SC_POSIX2_LINE_MAX		43
#define _SC_POSIX2_RE_DUP_MAX		44
#define _SC_PASS_MAX			45
#define _SC_XOPEN_VERSION		46
#define _SC_ATEXIT_MAX			47
#define _SC_PAGE_SIZE			48
#define _SC_AES_OS_VERSION		49
#define _SC_COLL_WEIGHTS_MAX		50
#define _SC_2_C_BIND			51
#define _SC_2_C_VERSION			52
#define _SC_2_UPE			53
#define _SC_2_CHAR_TERM			54
#define _SC_XOPEN_SHM			55
#define _SC_XOPEN_CRYPT			56
#define _SC_XOPEN_ENH_I18N		57
#define _SC_PAGESIZE                    _SC_PAGE_SIZE
#define _SC_IOV_MAX                     58
#define _SC_REENTRANT_FUNCTIONS		59
#define _SC_THREADS			60
#define _SC_THREAD_ATTR_STACKADDR	61
#define _SC_THREAD_ATTR_STACKSIZE	62
#define _SC_THREAD_FORKALL		63
#define _SC_THREAD_PRIORITY_SCHEDULING	64
#define _SC_THREAD_PRIO_INHERIT		65
#define _SC_THREAD_PRIO_PROTECT		66
#define _SC_THREAD_PROCESS_SHARED	67
#define _SC_THREAD_DATAKEYS_MAX		68
#define _SC_THREAD_STACK_MIN		69
#define _SC_THREAD_THREADS_MAX		70
#define _SC_NPROCESSORS_CONF		71
#define _SC_NPROCESSORS_ONLN		72

#endif /* _POSIX_SOURCE */

#ifdef _XOPEN_SOURCE

#define _XOPEN_VERSION		4
#define _XOPEN_XCU_VERSION	4
#define _XOPEN_XPG3		1
#define _XOPEN_XPG4		1

#define _POSIX2_C_BIND		1
#define _POSIX2_C_DEV		1
#define _POSIX2_CHAR_TERM	1
#define _POSIX2_LOCALEDEF	1
#define _POSIX2_UPE		1
#define _POSIX2_FORT_DEV	(-1)
#define _POSIX2_FORT_RUN	(-1)
#define _POSIX2_SW_DEV		1	

#define _XOPEN_CRYPT		1
#define _XOPEN_SHM		1	
#define _XOPEN_ENH_I18N		1	

extern  char    *optarg;
extern  int     optind, opterr, optopt;

#ifdef _NO_PROTO
	extern  int     chroot();
	extern	size_t	confstr();
	extern  char    *crypt();
	extern  void    encrypt();
	extern  int     fsync();
	extern	int	getopt();
	extern  char    *getpass();
	extern	int	nice();
	extern  void    swab();

#else
	extern  int     chroot(const char *);
	extern	size_t	confstr(int, char*, size_t);
	extern  char    *crypt(const char *, const char *);
	extern  void    encrypt(char *, int);
	extern  int     fsync(int);
	extern	int	getopt(int, char* const*, const char*);
	extern  char    *getpass(const char *);
	extern	int	nice(int);
	extern  void    swab(const void *, void *, ssize_t);

#endif

#endif /* _XOPEN _SOURCE */


#ifdef _ALL_SOURCE

#ifndef _H_LOCKF
#include <sys/lockf.h>		/* lockf definitions for portability	*/
#endif

extern char **environ;

#ifdef _NO_PROTO
	extern int		brk();
	extern int		fchdir();
	extern int		fchown();
	extern int		ftruncate();
	extern int		getdtablesize();
	extern int		gethostid();
	extern int		gethostname();
	extern int		getpagesize();
	extern pid_t		getpgid();
	extern pid_t		getsid();
	extern char		*getwd();
	extern int		ioctl();
	extern int		lchown();
	extern int		lockf();
	extern int		readlink();
	extern int 		readx();
	extern void		*sbrk();
	extern int 		setgroups();
	extern pid_t		setpgrp();
	extern int		setregid();
	extern int		setreuid();
	extern int		symlink();
	extern void		sync();
	extern int		truncate();
	extern unsigned int	ualarm();
	extern int		usleep();
	extern pid_t		vfork();
	extern int 		writex();
#ifdef _LONG_LONG
	extern offset_t llseek();
#endif /* _LONG_LONG */

#else
	extern int		brk(void *);
	extern int		fchdir(int);
	extern int		fchown(int, uid_t, gid_t);
	extern int		ftruncate(int, off_t);
	extern int		getdtablesize(void);
	extern int		gethostid(void);
	extern int		gethostname(char *, int);
	extern int		getpagesize(void);
	extern pid_t		getpgid(pid_t);
	extern pid_t		getsid(pid_t);
	extern char		*getwd(char *);
#ifndef _BSD
	extern int		ioctl(int, int, ...);
#endif /* _BSD */
	extern int		lchown(const char *, uid_t, gid_t);
	extern int		lockf(int, int, off_t);
	extern int		readlink(const char *, char *, size_t);
	extern void		*sbrk(int);
	extern int 		setgroups(int, gid_t []);
#ifndef _BSD
	extern pid_t		setpgrp(void);
#endif /* _BSD */
	extern int		setregid(gid_t, gid_t);
	extern int		setreuid(uid_t, uid_t);
	extern int		symlink(const char *, const char *);
	extern void		sync(void);
	extern int		truncate(const char *, off_t);
	extern unsigned int	ualarm(unsigned int, unsigned int);
	extern int		usleep(unsigned int);
	extern pid_t		vfork(void);
#ifndef _KERNEL
	extern int 	readx(int, char*, unsigned, long);
	extern int 	writex(int, char*, unsigned, long);

#ifdef _LONG_LONG
	extern offset_t llseek(int, offset_t, int);
#endif /* _LONG_LONG */

#endif /* ndef _KERNEL */

#endif /* _NO_PROTO */

#define _AES_OS_VERSION 1               /* OSF, AES version */

#endif /* _ALL_SOURCE */

#endif /* _H_UNISTD */
