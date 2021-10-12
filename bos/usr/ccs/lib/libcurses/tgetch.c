#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)96  1.3  src/bos/usr/ccs/lib/libcurses/tgetch.c, libcurses, bos41J, 9518A_all 5/1/95 08:57:18";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: _catch_alarm
 *		_fpk
 *		_getkey
 *		_pk
 *		_readchar
 *		tgetch
 *		
 *
 *   ORIGINS: 27, 4
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993, 1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#endif /* _POWER_PROLOG_ */


/*
 *	Copyright (c) 1984 AT&T	
 *	  All Rights Reserved 

 *	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
 *	The copyright notice above does not evidence any 
 *	actual or intended publication of such source code.
 */

/* #ident	"@(#)curses:screen/tgetch.c	1.5"		*/



#include	"curses_inc.h"
#include	<signal.h>
#include	<errno.h>
#include	<sys/time.h>
#ifdef	DEBUG
#include	<ctype.h>
#endif	/* DEBUG */

/*
 * Read a key typed from the terminal
 *
 * interpret:	= 0 for single-char key only
 * 	 	= 1 for matching function key and macro patterns.
 * 		= 2 same as 1 but no time-out for funckey matching.
 */

int
tgetch(interpret)
int	interpret;
{
    int			inp, i = 0, j, collapse = 1;
    register	short	*inputQ = cur_term->_input_queue;
    char		*chars_onQ = &(cur_term->_chars_on_queue);

#ifdef	SYSV
    /*
     * Register the fact that getch is being used so
     * that typeahead checking can be done.
     * This code should GO AWAY when a poll() or FIONREAD can
     * be done on the file descriptor as then the check
     * will be non-destructive.
     */
    cur_term->fl_typeahdok = TRUE;
#endif	/* SYSV */

    /* ask for input */
    if (cur_term->_ungotten > 0)
    {
	cur_term->_ungotten--;
	/* decode an ungetch()'d character */
	inp = -(inputQ[0] + 0100);
    }
    else
    {
	/* Only read a character if there is no typeahead/peekahead. */
	if (*chars_onQ == 0)
	{
	    (*chars_onQ)++;
#ifdef	FIONREAD
	    inp = _readchar();
#else	/* FIONREAD */
	    if ((inp = _pk()) == ERR)
	    {
		/*
		 * interpret is set to 0 so that down below we don't
		 * drop into getkey since we already know there can't be
		 * a key that starts with -1.  Also, we don't want to
		 * access funckeystarter[-1].
		 */
		interpret = FALSE;
	    }
#endif	/* FIONREAD */
	}
	else
	    inp = inputQ[0];

#ifdef	DEBUG
	if (outf)
	    fprintf(outf, "TGETCH read '%s'\n", unctrl(inp));
#endif	/* DEBUG */

	/* Check for arrow and function keys */
	/* but only if inp is not -1, because if it is, nodelay is on */
	/* and there were no characters in the type-ahead buffer. arnold 02/28/93 */
	if (interpret && (inp >= 0) && cur_term->funckeystarter[inp])
	    collapse = _getkey(interpret - 1, &inp);
    }

/* Collapse the input queue to remove the escape sequence from the stack. */

    j = *chars_onQ;
    (*chars_onQ) -= collapse;
    while (collapse < j)
	inputQ[i++] = inputQ[collapse++];
    return (inp);
}

#ifdef	FIONREAD
static	int
_readchar()
{
    register	int	i;
    unsigned	char	c;

    if (cur_term->_delay <= 0)	/* If don't want to timeout */
    {
	int	arg;

	(void) ioctl(cur_term->_inputfd, FIONREAD, &arg);
#ifdef	DEBUG
	if (outf) fprintf(outf, "FIONREAD returns %d\n", arg);
#endif	/* DEBUG */
	if (cur_term->_delay==0 && arg < 1)	/* if nodelay && no chars */
	    return (-1);			/*  in typeahead buffer.  */
						/* but if there's a char  */
	i = read(cur_term->_inputfd, &c, 1);    /*  read it in.           */
    }
    else
	if (cur_term->_delay > 0)
	{
	    char	c;
	    int		infd;
	    struct	timeval t;

	    infd = 1 << cur_term->_inputfd;
	    t.tv_sec = cur_term->_delay / 1000;
	    t.tv_usec = (cur_term->_delay % 1000) * 1000;
	    i = select(20, &infd, (int *)NULL, (int *)NULL, &t);
	    if (i <= 0)
		return (ERR);
	    i = read(cur_term->_inputfd, &c, 1);
	}

#ifdef	DEBUG
    if (outf)
	fprintf(outf, "read from %d returns %d chars, first %o\n",
	    cur_term->_inputfd, i, c);
#endif	/* DEBUG */

    if (i > 0)
	return (c);
    else
	return (ERR);
}
#endif	/* !FIONREAD */

#ifdef	DEBUG
extern	char	*_asciify();
#endif	/* DEBUG */

    /*
     * This algorithm is a "learning" algorithm. The premise is
     * that keys used once are like to be used again and again.
     * Since the time for a linear search of the table is so
     * expensive, we move keys that are found up to the top of
     * the list, making the access to a repeated key very fast and
     * keys that have been used before close to the top.
     */

static	int
_getkey(blockpeek, inp)
register	int	*inp;
register	int	blockpeek;
{
    register	_KEY_MAP	**kp = cur_term->_keys;
    register	int		i, key, num_keys = cur_term->_ksz;
    short			*inputQ = cur_term->_input_queue;
    char			*chars_onQ = &(cur_term->_chars_on_queue),
				flag = cur_term->funckeystarter[*inp];
    int				first, collapse = 1;

#ifdef	DEBUG
    if (outf)
	fprintf(outf, "getkey(): looking in linear table, inp=%d\n", *inp);
#endif	/* DEBUG */

    if (flag & _KEY)
	key = 0;
    else
    {
	key = cur_term->_first_macro;
	blockpeek = TRUE;
    }
    first = key;

    for (; key < num_keys; key++)
    {
	if (kp[key]->_sends[0] == *inp)
	{
	    for (i = 1; i < INP_QSIZE; i++)
	    {
		/* found it? */
		if (kp[key]->_sends[i] == '\0')
		    break;
		/* partial match? peek ahead. */
		if (*chars_onQ == i)
		{
		    (*chars_onQ)++;
		    switch (inputQ[i] = (blockpeek) ? _pk() : _fpk())
		    {
			case -2:
			    /*
			     * Since -2 signifies a timeout we don't really
			     * want to put it on the queue so we decrement
			     * our counter.
			     */
			    (*chars_onQ)--;
#ifdef	DEBUG
			    if (outf)
				fprintf(outf, "Timed out\n");
#endif	/* DEBUG */
			    if (flag & _MACRO)
			    {
#ifdef	DEBUG
				if (outf)
				    fprintf(outf, "Found macro\n");
#endif	/* DEBUG */
				/*
				 * We have to decrement one because key will be
				 * incremented at the bottom of the out loop.
				 */
				key = (first = blockpeek = cur_term->_first_macro) - 1;
				goto outerloop;
			    }
			case -1:
			    goto ret;
		    }
		}

		/* not this one? */
		if (kp[key]->_sends[i] != inputQ[i])
		    goto outerloop;
	    }

	    /* We found it! Read in any chars left in _sends */

	    if ((collapse = i) == INP_QSIZE)
		for ( ; kp[key]->_sends[i]; i++)
		    (void) _fpk();

	    /* move key to top of ordered list */
	    if (key != first)
	    {
		_KEY_MAP	*savekey = kp[key];
		short		*lorder;
		int		j;

		if (key > cur_term->_first_macro)
		    lorder = &(cur_term->_lastmacro_ordered);
		else
		    lorder = &(cur_term->_lastkey_ordered);
		/*
		 * If we're below the last ordered key, swap next unordered
		 * key with this one and ripple from there.
		 */
		if (key > *lorder)
		    kp[key] = kp[(i = ++(*lorder))];
		else
		    i = key;
		/* ripple the ordered keys down */
		for (j = i--; j > first; )
		    kp[j--] = kp[i--];
		kp[first] = savekey;
	    }
	    *inp = kp[first]->_keyval;
	    goto ret;
	}
outerloop:
	;
    }

ret:
    /* key not found */
#ifdef	DEBUG
    if (outf)
	if (key == num_keys)
	    fprintf(outf, "Did not match anything.\n");
#endif	/* DEBUG */
    return (collapse);
}

/*
 * Fast peek key.  Like getchar but if the right flags are set, times out
 * quickly if there is nothing waiting, returning -1.
 * f is an output stdio descriptor, we read from the fileno.
 * We wait for long enough for a terminal to send another character
 * (at 15cps repeat rate, this is 67 ms, I'm using 100ms to allow
 * a bit of a fudge factor) and time out more quickly.
 * -2 is returned if we time out, -1 is returned if interrupted, and the
 * character is returned otherwise.
 */

#ifndef	FIONREAD

/*
 * Traditional implementation.  The best resolution we have is 1 second,
 * so we set a 1 second alarm and try to read.  If we fail for 1 second,
 * we assume there is no key waiting.  Problem here is that 1 second is
 * too long; people can type faster than this.
 *
 * Another possible implementation of changing VMIN/VTIME before and
 * after each read does not work because the tty driver's timeout
 * mechanism is too unreliable when the timeouts are changed too quickly.
 */

static	char	sig_caught;

static
_catch_alarm()
{
    sig_caught = 1;
}

static
_fpk()
{
    extern	int	_catch_alarm();
    unsigned	char	c;
    int		rc, infd = cur_term->_inputfd;
#ifdef	SIGPOLL	/* Vr3 and beyond */
    void	(*oldsig)();
#else	/* SIGPOLL */
    int		(*oldsig)();
#endif	/* SIGPOLL */
    unsigned	int	oldalarm, alarm();

    /* turn off any user alarms and set our own */
    oldalarm = alarm(0);
    sig_caught = 0;
    oldsig = signal(SIGALRM, _catch_alarm);
    (void) alarm(1);
    rc = read(cur_term->_inputfd, (char *) &c, 1);
    (void) alarm(0);

    /*
     * This code is to take care of the possibility of
     * the process getting swapped out in the middle of
     * read() call above. The interrupt will cause the
     * read() call to return, even if a character is really
     * on the clist. So we do a non-blocking read() to make
     * sure that there really isn't a character there.
     */

    if (sig_caught && rc != 1)
	if (cur_term->_check_fd != -1)
	    rc = read(cur_term->_check_fd, (char *) &c, 1);
	else
	{
#include	<fcntl.h>
	    register	int	fcflags = fcntl(infd, F_GETFL);

	    (void) fcntl(infd, F_SETFL, fcflags | O_NDELAY);
	    rc = read(infd, (char *) &c, 1);
	    (void) fcntl(infd, F_SETFL, fcflags);
	}

    /* restore the user alarms */
	(void) signal(SIGALRM, oldsig);
	if (sig_caught && oldalarm > 1)
	    oldalarm--;
	(void) alarm(oldalarm);
	if (rc == 1)			/* got a character */
	    return (c);
	else
	    if (sig_caught)		/* timed out */
		return (-2);
	    else			/* EOF or got interrupted */
		return (-1);
}
#else	/* FIONREAD */
/*
 * If we have the select system call, we can do much better than the
 * traditional method. Even if we don't have the real 4.2BSD select, we
 * can emulate it with napms and FIONREAD.  napms might be done with only
 * 1 second resolution, but this is no worse than what we have in the
 * traditional implementation.
 */
static
_fpk()
{
    static long timeOut = 0;		/* times to read	*/
    char *escDelay;			/* ENV var. ESCDELAY    */
#define max_reads 100000		/* maximum timeout      */

    int		infd, rc;
    int		*outfd, *exfd;
    unsigned	char	c;
    struct  	timeval t;

	if (!timeOut)                   /* if first time here.          */
					/* if ESCDELAY exists.          */
	    if ((escDelay = (char *) getenv ("ESCDELAY")) != (char *) NULL){
					/* convert it to numeric.       */
		timeOut = (long) atoi (escDelay) * 200;
	    }
	    else
		timeOut = (long) max_reads ;

    infd = 1 << cur_term->_inputfd;
    outfd = exfd = (int *) NULL;
    t.tv_sec = timeOut / 1000000;
    t.tv_usec = timeOut % 1000000;		/* 100 milliseconds */
    rc = select(20, &infd, outfd, exfd, &t);
    if (rc <= 0)
	return (-2);
/*    rc = read(fileno(f), &c, 1);     */
    rc = read(cur_term->_inputfd, &c, 1);
    return (rc == 1 ? c : -1);
}
#endif	/* FIONREAD */

/*
 * Plain peekchar function.  Nothing fancy.  This is just like _fpk
 * but will wait forever rather than time out.
 */

static
_pk()
{
    unsigned	char	c;

    return ((read(cur_term->_inputfd, (char *) &c, 1) == 1) ? c : ERR);
}
