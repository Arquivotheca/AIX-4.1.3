static char sccsid[] = "@(#)92	1.7  src/bos/usr/ccs/lib/libbsd/sleep.c, libbsd, bos411, 9428A410j 6/16/90 01:00:52";
/*
 * COMPONENT_NAME: (LIBBSD)  Berkeley Compatibility Library
 *
 * FUNCTIONS: sleep
 *
 * ORIGINS: 26 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#include <signal.h>
#include <unistd.h>
#include <sys/limits.h>
#include <sys/types.h>
#include <sys/times.h>
#include <time.h>

/*
 *
 *  In earlier systems, sleep was a syscall.  Since alarms were
 *  an independent mechanism, carefree combinations of the two
 *  facilities made perfect sense.
 *
 *  Now however, sleep has been moved out of the kernel and into
 *  libc and libbsd.  The new implementation is based upon the alarm signal;
 *  sleep(sleepsec) becomes ( alarm(sleepsec), sigsuspend(msk); ).
 *
 *  The considerations in writing the routine:
 *
 *  1) "Alarm requests are not stacked; successive calls reset
 *     the calling process' alarm clock".  Therefore, to
 *     present the illusion that sleep & alarm are independent,
 *     it is necessary to detect/simulate any pending alarm.
 *
 *  2) There is a scheduling window between alarm(sleepsec) and
 *     sigsuspend(msk).  Due to scheduling delays, the signal might
 *     arrive before the sigsuspend has begun, leaving the proc
 *     hung waiting for a signal that isn't there.  The best
 *     conditions for such a goof is sleepsec=1 on a loaded sys.
 *     Frequencies on the order of 1 hang in 3000 calls have
 *     been observed on some medium-loaded systems.
 *
 *  3) Pauses are prematurely aroused upon receipt of another
 *     caught signal.  In the target world, alarms are immune
 *     to other signals whereas sleeps are ended by them.
 *     Therefore, reissue an alarm whose time hasn't yet come.
 *
 *  4) A tempting simple scheme to close the limbo window is to
 *     have SIGALRM's catching function do a longjmp, so that
 *     control returns to a set point whenever the alarm
 *     sounds, whether or not the sigsuspend has commenced.  Its
 *     flaw is that signals can be lost.  When the alarm
 *     catcher is invoked, it is possible that other signal
 *     catching functions were invoked since sigsuspend() began.  If
 *     these others have not yet finished at the time the alarm
 *     catcher begins, then a longjmp() out of it will bypass
 *     the partially executed signal functions.
 *
 *  5) Alarms can occur up to 1-sec early (cf. sleep(3) man pg).
 *     Unless we round up in the ELAPSED calculation, an un-
 *     interrupted sleep could return 1, which is undesirable.
 *
 *  6) Ideally, signals should be disabled during this rtn.  No
 *     such facility exists, so results are indeterminate if a
 *     signal catcher does a sleep/alarm while we are active.
 */
/* The sleep() function includes all the POSIX requirements. */


#define ALARMOFF()      alarm((unsigned)0)
#define CATCH(new, old) sigaction(SIGALRM, new, old)
#define SLEEP(n)      ( alarm(n), sigsuspend(&msk) )
#define ELAPSED()     ( (int) ((times(&t) - zero + CLK_TCK-1) / CLK_TCK ))

#ifdef sleep
#undef sleep
#endif

unsigned int
sleep(unsigned int sleepsec)
{
	register long   zero;
	register unsigned alarmsec;
	register unsigned sleptsec;
	struct tms      t;            /* for times syscall */
	sigset_t 	msk;
	struct sigaction act, *actp,
		 	 oact, *oactp;
	void snooze(int);

	if (sleepsec == 0) return;

	zero = times(&t);             /* in clock ticks    */
	alarmsec = alarm((unsigned)0);          /* capture prev alrm */

	actp = &act;
	oactp = &oact;
	act.sa_flags = 0;
	SIGINITSET(act.sa_mask);
	act.sa_handler = (void (*)(int))snooze;
	CATCH(actp,oactp);         	      /* capture prev func */

	/* case I:    no prev alarm, only sleep            */
	/* case II:   alarm, but sleep first on agenda     */
	/* case III:  alarm with/before sleep, but ignored */
	/* case IV:   alarm with/before sleep, not ignored */

	/* wait till time of 1st expected non-ignored signal */
	/* usually sleep's; in case IV, tho, it is the alarm */
	/*        case I          case II           case III */
	
	/* P30217 */
	/* The mask passed to sigsuspend should be the current mask *
         * minus SIGALRM.                                           */
	sigprocmask(SIG_BLOCK,NULL,&msk);
	sigdelset(&msk,SIGALRM);

	SLEEP((alarmsec==0 || sleepsec<alarmsec || oact.sa_handler==SIG_IGN)
		? sleepsec : alarmsec);
	ALARMOFF();                 /* no more snooze()ing */
	sleptsec = ELAPSED();

	/*
	 *  we are either at this 1st event (if SLEEP went to
	 *  term) or before (if there was a premature arousal).
	 *
	 *  SETUP the alarm handler as was previously set, and
	 *  either set the alarm clock to go in the amount of
	 *  time left on the alarm, or, if alarmsec has already
	 *  passed, simulate an alarm by sending ourselves a
	 *  signal.
	 */
	CATCH(oactp, (struct sigaction *)NULL);
	if (alarmsec != 0)   /* an alarm was pending upon entry */
	{       if (alarmsec > sleptsec)   /* before its time?  */
			alarm(alarmsec - sleptsec);
		else
			kill(getpid(), SIGALRM); /* self-signal */
	}

	/* return # of unslept secs */
	return;
}
 /*
 *  similar in function to the snooze button on a clock radio.
 *  guarantees a pending alarm when sigsuspend is entered.  else,
 *  under "right" conditions, sigsuspend will never terminate.
 */
void snooze(int signo)
{ 
	struct sigaction sact, *sactp;

	sact.sa_flags = 0;
	SIGINITSET(sact.sa_mask);
	sact.sa_handler = (void (*)(int))snooze;
	sactp = &sact;
	CATCH(sactp, (struct sigaction *)NULL);
	alarm((unsigned)1);
}
