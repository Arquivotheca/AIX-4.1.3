static char sccsid[] = "@(#)37	1.11  src/bos/usr/bin/csh/sem.c, cmdcsh, bos411, 9428A410j 11/30/93 18:11:38";
/*
 * COMPONENT_NAME: CMDCSH  c shell(csh)
 *
 * FUNCTIONS: execute doio mypipe chkclob
 *
 * ORIGINS:  10,26,27,18,71
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985,1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 * 
 * OSF/1 1.1
 */

#define DEFFILEMODE 0666

#include "sh.h"
#include "proc.h"
#include "pathnames.h"
#include <fcntl.h>
#include <sys/ioctl.h>

void
execute(register struct command *t, int wanttty, int *pipein, int *pipeout)
{
	bool forked = 0;
	struct biltins *bifunc;
	pid_t pid = 0;
	int pv[2];

#ifdef DEBUG
printf("debug: entering execute\n"); fflush(stdout);
#endif
	if (t == 0)
		return;
	if ((t->t_dflg & FAND) && wanttty > 0)
		wanttty = 0;
	switch (t->t_dtyp) {

	case TCOM:
		/*
		 * If first byte of command is ALIASCHR, we expanded an alias
		 * name which was defined in terms of itself.  This
		 * prevented an infinite loop.  See asyn3() in parse.c.
		 */
		if (t->t_dcom[0][0] == ALIASCHR)
			strcpy(t->t_dcom[0], t->t_dcom[0] + 1);
		if ((t->t_dflg & FREDO) == 0)
			Dfix(t);                /* $ " ' \ */
		if (t->t_dcom[0] == 0)
			return;
		/* fall into... */

	case TPAR:
		if (t->t_dflg & FPOU)
			mypipe(pipeout);
		/*
		 * Must do << early so parent will know
		 * where input pointer should be.
		 * If noexec then this is all we do.
		 */
		if (t->t_dflg & FHERE) {
			close(0);
			heredoc(t->t_dlef);
			if (noexec)
				close(0);
		}
		if (noexec)
			break;

		if (t->t_dtyp==TCOM && !EQ(t->t_dcom[0], "exit"))
                        set("status", (uchar_t *)"0");


		/*
		 * This mess is the necessary kludge to handle the prefix
		 * builtins: nice, nohup, time.  These commands can also
		 * be used by themselves, and this is not handled here.
		 * This will also work when loops are parsed.
		 */
		while (t->t_dtyp == TCOM)
			if (EQ(t->t_dcom[0], "nice"))
				if (t->t_dcom[1])
					if (strchr("+-",t->t_dcom[1][0]))
						if (t->t_dcom[2]) {
							setname((uchar_t *)"nice");
							t->t_nice = getn(t->t_dcom[1]);
							lshift(t->t_dcom, 2);
							t->t_dflg |= FNICE;
						} else
							break;
					else {
						t->t_nice = NICE;
						lshift(t->t_dcom, 1);
						t->t_dflg |= FNICE;
					}
				else
					break;
			else if (EQ(t->t_dcom[0], "nohup"))
			{
				if (t->t_dcom[1])
					t->t_dflg |= FNOHUP;
				break;
			}
			else if (EQ(t->t_dcom[0], "time"))
				if (t->t_dcom[1]) {
					t->t_dflg |= FTIME;
					lshift(t->t_dcom, 1);
				} else
					break;
			else
				break;

		/*
		 * Check if we have a builtin function and remember which one.
		 */
		bifunc = (!(t->t_dflg & FNOHUP) && (t->t_dtyp == TCOM))
                         ? isbfunc(t) : (struct biltins *) 0;

		/*
		 * We fork only if we are timed, or are not the end of
		 * a parenthesized list and not a simple builtin function.
		 * Simple meaning one that is not pipedout, niced, nohupped,
		 * or &'d.
		 * It would be nice(?) to not fork in some of these cases.
		 */

#ifdef DEBUG
printf("execute(TPAR): about to fork\n"); fflush(stdout); sleep(1);
#endif
		if (((t->t_dflg & FTIME) || (t->t_dflg & FPAR) == 0 &&
		     (!bifunc || t->t_dflg & (FPOU|FAND|FNICE|FNOHUP))))
		{
			forked++;
			pid = pfork(t, wanttty);
		}
#ifdef DEBUG
printf("execute(TPAR): back from fork\n"); fflush(stdout); sleep(1);
#endif
		if (pid != 0) {
			/*
			 * It would be better if we could wait for the
			 * whole job when we knew the last process
			 * had been started.  Pwait, in fact, does
			 * wait for the whole job anyway, but this test
			 * doesn't really express our intentions.
			 */

			if (didfds==0 && t->t_dflg&FPIN) {
#ifdef DEBUG
printf("execute(TPAR): closing pipes \n"); fflush(stdout);
#endif
				(void) close(pipein[0]);
				(void) close(pipein[1]);
			}
			if ((t->t_dflg & (FPOU|FAND)) == 0)
				pwait();
			break;
		}
#ifdef DEBUG
printf("execute(TPAR): doio \n"); fflush(stdout);
#endif
		/*
		 * If there is a child don't have to
		 * do a pfork().
		 */
		doio(t, pipein, pipeout);
		if (t->t_dflg & FPOU) {
			(void) close(pipeout[0]);
			(void) close(pipeout[1]);
		}

#ifdef DEBUG
printf("execute(TPAR): execute builtin \n"); fflush(stdout);
#endif
		/*
		 * Perform a builtin function.
		 * If we are not forked, arrange for possible stopping
		 */
		if (bifunc) {
			func(t, bifunc);
			if (forked)
				exitstat();
			break;
		}
		if (t->t_dtyp != TPAR) {
#ifdef DEBUG
printf("execute(TPAR): doexec call\n"); fflush(stdout);
#endif
			doexec(t);
			/*NOTREACHED*/
		}
		/*
		 * For () commands must put new 0,1,2 in FSH* and recurse
		 */
		OLDSTD = dcopy(0, FOLDSTD);
		SHOUT = dcopy(1, FSHOUT);
		SHDIAG = dcopy(2, FSHDIAG);
		close(SHIN), SHIN = -1;
		didcch = 0, didfds = 0;
		wanttty = -1;
		t->t_dspr->t_dflg |= t->t_dflg & FINT;
#ifdef DEBUG
printf("execute(TPAR): execute\n"); fflush(stdout);
#endif
		execute(t->t_dspr, wanttty,(int *)0, (int *)0);
		exitstat();

	case TFIL:
		t->t_dcar->t_dflg |= FPOU |
		    (t->t_dflg & (FPIN|FAND|FDIAG|FINT));
#ifdef DEBUG
printf("execute(TFIL): execute\n"); fflush(stdout);
#endif
		execute(t->t_dcar, wanttty, pipein, pv);
		t->t_dcdr->t_dflg |= FPIN |
		    (t->t_dflg & (FPOU|FAND|FPAR|FINT));
		if (wanttty > 0)
			wanttty = 0;		/* got tty already */
#ifdef DEBUG
printf("execute(TFILE): execute2\n"); fflush(stdout);
#endif
		execute(t->t_dcdr, wanttty, pv, pipeout);
		break;

	case TLST:
		if (t->t_dcar) {
			t->t_dcar->t_dflg |= t->t_dflg & FINT;
#ifdef DEBUG
printf("execute(TLST): execute\n"); fflush(stdout);
#endif
			execute(t->t_dcar, wanttty,(int *)0, (int *)0);
			/*
			 * In strange case of A&B make a new job after A
			 */
			if (t->t_dcar->t_dflg&FAND && t->t_dcdr &&
			    (t->t_dcdr->t_dflg&FAND) == 0)
				pendjob();
		}
		if (t->t_dcdr) {
			t->t_dcdr->t_dflg |= t->t_dflg & (FPAR|FINT);
#ifdef DEBUG
printf("execute(TLST): execute2\n"); fflush(stdout);
#endif
			execute(t->t_dcdr, wanttty,(int *)0, (int *)0);
		}
		break;

	case TOR:
	case TAND:
		if (t->t_dcar) {
			t->t_dcar->t_dflg |= t->t_dflg & FINT;
#ifdef DEBUG
printf("execute(TOR): execute\n"); fflush(stdout);
#endif
			execute(t->t_dcar, wanttty,(int *)0, (int *)0);
			if ((getn(value("status")) == 0) != (t->t_dtyp == TAND))
				return;
		}
		if (t->t_dcdr) {
			t->t_dcdr->t_dflg |= t->t_dflg & (FPAR|FINT);
#ifdef DEBUG
printf("execute(TOR): execute\n"); fflush(stdout);
#endif
			execute(t->t_dcdr, wanttty,(int *)0, (int *)0);
		}
		break;
	}
	/*
	 * Fall through for all breaks from switch
	 *
	 * If there will be no more executions of this
	 * command, flush all file descriptors.
	 * Places that turn on the FREDO bit are responsible
	 * for doing donefds after the last re-execution
	 *
	 */
	if (didfds && !(t->t_dflg & FREDO))
		donefds();
}

/*
 * Perform io redirection.
 * We may or maynot be forked here.
 */
void
doio(register struct command *t, int *pipein, int *pipeout)
{
	register uchar_t *cp;
	register int flags = t->t_dflg;

#ifdef DEBUG
printf("doio: flags=%d\n", flags); fflush(stdout);
#endif
	if (didfds || (flags & FREDO))
		return;
	if ((flags & FHERE) == 0) {	/* FHERE already done */
		close(0);
#ifdef DEBUG
if(t->t_dlef == NULL)
{
  printf("doio: t->t_dlef=NULL\n");fflush(stdout);
}
else
{
  printf("doio: t->t_dlef=%s\n", t->t_dlef);fflush(stdout);
}
#endif
		if (cp = t->t_dlef) {
			cp = globone(Dfix1(cp));
			if (open((char *)cp, O_RDONLY) < 0)
				Perror_free((char *)cp);
			xfree(cp);
                } else if (flags & FPIN) {
                        track_open(dup(pipein[0]));
                        close(pipein[0]);
                        close(pipein[1]);
                }
		else if ((flags & FINT) && tpgrp == -1) {
			close(0);
			open(_PATH_DEVNULL, O_RDONLY);
		}
		else
			track_open(dup(OLDSTD));
	}
	close(1);
#ifdef DEBUG
printf("doio: close of stdout\n");fflush(stdout);
#endif
	if (cp = t->t_drit) {
		cp = globone(Dfix1(cp));
		if ((flags & FCAT) && open((char *)cp, O_WRONLY) >= 0)
			lseek(1, 0L, SEEK_END);
		else {
			if(!(flags & FANY) && adrof("noclobber")) { 
				if (flags & FCAT)
					Perror_free((char *)cp);
				chkclob(cp);
			}
			if (creat((char *)cp, DEFFILEMODE) < 0)
				Perror((char *)cp);
		}
		xfree(cp);
        } else if (flags & FPOU) {
                track_open(dup(pipeout[1]));
        }
        else {
                track_open(dup(SHOUT));
        }

#ifdef DEBUG
printf("doio: after dup\n"); fflush(stdout);
#endif
	close(2);
	track_open(dup((flags & FDIAG) ? 1 : SHDIAG));
	didfds = 1;
#ifdef DEBUG
printf("doio return\n"); fflush(stdout);
#endif
}

void
mypipe(register int *pv)
{

	if (pipe(pv) < 0) {
		error(MSGSTR(M_NOPIPE, "Can't make pipe"));
	}
	else {
		pv[0] = dmove(pv[0], -1);
		pv[1] = dmove(pv[1], -1);
		if (pv[0] >= 0 && pv[1] >= 0)
			return;
		else
			error(MSGSTR(M_NOPIPE, "Can't make pipe"));
	}
	return;
}

void
chkclob(register uchar_t *cp)
{
	struct stat stb;
	char e[NL_TEXTMAX];

	if (stat((char *)cp, &stb) < 0)
		return;
	if (S_ISCHR(stb.st_mode))
		return;
	sprintf(e,MSGSTR(M_FILE, "%s: File exists"), cp);
	error(e);
}
