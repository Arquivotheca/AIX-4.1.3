/* @(#)87	1.11.1.7  src/bos/usr/include/utmp.h, cmdoper, bos411, 9428A410j 7/8/94 11:03:32 */
/*
 *   COMPONENT_NAME: CMDOPER
 *
 *   FUNCTIONS: UTMP_DATA_INIT
 *
 *   ORIGINS: 27,71
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */


#ifndef _H_UTMP
#define _H_UTMP

/*	<sys/types.h> must be included.					*/

#define	UTMP_FILE	"/etc/utmp"
#define WTMP_FILE       "/var/adm/wtmp"
#define ILOG_FILE       "/etc/security/failedlogin"
#define	ut_name	ut_user

struct utmp
  {
	char ut_user[8] ;		/* User login name */
	char ut_id[14] ;		/* /etc/inittab id */
	char ut_line[12] ;		/* device name (console, lnxx) */
	short ut_type ; 		/* type of entry */
	pid_t ut_pid ;			/* process id */
	struct exit_status
	  {
	    short e_termination ;	/* Process termination status */
	    short e_exit ;		/* Process exit status */
	  }
	ut_exit ;			/* The exit status of a process
					 * marked as DEAD_PROCESS.
					 */
	time_t ut_time ;		/* time entry was made */
	char ut_host[16] ;		/* host name */
  } ;

/*	Definitions for ut_type						*/

#define	EMPTY		0
#define	RUN_LVL		1
#define	BOOT_TIME	2
#define	OLD_TIME	3
#define	NEW_TIME	4
#define	INIT_PROCESS	5	/* Process spawned by "init" */
#define	LOGIN_PROCESS	6	/* A "getty" process waiting for login */
#define	USER_PROCESS	7	/* A user process */
#define	DEAD_PROCESS	8
#define	ACCOUNTING	9

#define	UTMAXTYPE	ACCOUNTING	/* Largest legal value of ut_type */

/*	Special strings or formats used in the "ut_line" field when	*/
/*	accounting for something other than a process.			*/
/*	No string for the ut_line field can be more than 11 chars +	*/
/*	a NULL in length.						*/

#define RUNLVL_MSG      "run-level %c"
#define	BOOT_MSG	"system boot"
#define	OTIME_MSG	"old time"
#define	NTIME_MSG	"new time"
#ifdef _THREAD_SAFE
struct utmp_data {
	int		ut_fd;
	long		loc_utmp;
	struct utmp 	ubuf;
	char 		*name;
};
#define UTMP_DATA_INIT(__s) (__s.ut_fd=-1, __s.name=UTMP_FILE)
#endif /* _THREAD_SAFE */

#ifdef	_NO_PROTO
extern void endutent();
extern struct utmp *getutent();
extern struct utmp *getutid();
extern struct utmp *getutline();
extern struct utmp *pututline();
extern void setutent();
extern int utmpname();

#ifdef _THREAD_SAFE
extern  void    endutent_r();
extern	int	getutent_r();
extern	int	getutid_r();
extern	int	getutline_r();
extern	int	pututline_r();
extern  void    setutent_r();
/* See comments in stdlib.h on _AIX32_THREADS */
#if _AIX32_THREADS
extern  void    utmpname_r();
#else	/* POSIX 1003.4a Draft 7 prototype */
extern  int     utmpname_r();
#endif /* _AIX32_THREADS */
#endif /* _THREAD_SAFE */

#else	/* _NO_PROTO */
extern void endutent(void);
extern struct utmp *getutent(void);
extern struct utmp *getutid(const struct utmp *);
extern struct utmp *getutline(const struct utmp *);
extern struct utmp *pututline(const struct utmp *);
extern void setutent(void);
extern int  utmpname(char *);

#ifdef _THREAD_SAFE
extern	int	getutent_r(struct utmp **utmp, struct utmp_data *utmp_data);
extern	int	getutid_r(const struct utmp *utent, struct utmp  **utmp, 
			  struct utmp_data *utmp_data);
extern	int	getutline_r(const struct utmp *utent, struct utmp **utmp, 
			    struct utmp_data *utmp_data);
extern	int	pututline_r(const struct utmp *utent, 
			     struct utmp_data *utmp_data);
extern  void    setutent_r(struct utmp_data *utmp_data);
extern  void    endutent_r(struct utmp_data *utmp_data);
/* See comments in stdlib.h on _AIX32_THREADS */
#if _AIX32_THREADS
extern  void    utmpname_r(char *newfile, struct utmp_data *utmp_data);
#else	/* POSIX 1003.4a Draft 7 prototype */
extern  int     utmpname_r(char *newfile, struct utmp_data *utmp_data);
#endif /* _AIX32_THREADS */
#endif /* _THREAD_SAFE */

#endif	/* _NO_PROTO */

#endif /* _H_UTMP */
