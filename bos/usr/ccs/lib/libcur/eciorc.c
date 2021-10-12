static char sccsid[] = "@(#)11	1.8  src/bos/usr/ccs/lib/libcur/eciorc.c, libcur, bos411, 9428A410j 1/23/91 23:08:45";
/*
 * COMPONENT_NAME: (LIBCUR) Extended Curses Library
 *
 * FUNCTIONS: eciopc, eciofl, eciowr, eciock, _tputvs
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1988
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include        "cur00.h"
#include        "errno.h"

extern
int     errno;			/* error code number            */

static
char    need_refr = FALSE;	/* Was error detected and thus  */
				/* refresh needed?              */

static
char    doing_refr = FALSE;	/* Is a refresh (recovery) being */
				/* done now (don't start another */

/*
 * NAME:                eciopc
 *
 * FUNCTION:            Invoke stdio putchar function with error
 *                      checking
 *
 * EXECUTION ENVIRONMENT:
 *
 *   PARAMETERS:        c - character to be written to stdout
 *
 *   INITIAL CONDITIONS stdout defined and open
 *
 *   FINAL CONDITIONS:
 *     NORMAL:          character added to stdout buffer
 *
 *     ABNORMAL:        flag need_refr set TRUE if error occured
 *
 * EXTERNAL REFERENCES: putchar
 *
 * RETURNED VALUES:     return code from putchar
 */

eciopc(c)
NLSCHAR c;			/* argument character           */

{

    int     rc;			/* return code                  */

    if ((rc = c > 255 ? putchar(c >> 8), putchar(c) : putchar(c)) < 0 &&
				/* write character and ck for   */
	    (errno == EINTR)) {	/* error.                       */
	need_refr = TRUE;	/* if error due to signal then  */
    }				/* flag error occurred. no retry */
				/* because stdio modifies buffer */
				/* even if an error occurrs     */
    return(rc);
}

/*
 * NAME:                eciofl
 *
 * FUNCTION:            Invoke stdio fflush function with error
 *                      checking
 *
 * EXECUTION ENVIRONMENT:
 *
 *   PARAMETERS:        f - file descriptor (stdio not number)
 *
 *   INITIAL CONDITIONS stdout defined and open
 *
 *   FINAL CONDITIONS:
 *     NORMAL:          any buffered data written to stdout
 *
 *     ABNORMAL:        flag need_refr set TRUE if error occured
 *
 * EXTERNAL REFERENCES: fflush
 *
 * RETURNED VALUES:     return code from fflush
 */

eciofl(f)
FILE * f;			/* file - stdout - to flush     */

{

    int     rc;			/* return code                  */

    if ((rc = fflush(f)) < 0 &&	/* Flush any pending data and   */
	    (errno == EINTR)) {	/* check for errors             */
	need_refr = TRUE;	/* if error due to signal then  */
    }				/* flag error occurred. no retry */
				/* because stdio modifies buffer */
				/* even if an error occurrs     */
    return(rc);
}

/*
 * NAME:                eciowr
 *
 * FUNCTION:            Invoke write function with error checks
 *
 * EXECUTION ENVIRONMENT:
 *
 *   PARAMETERS:        f - file descriptor (number not FILE)
 *                      b - buffer pointer
 *                      n - size of buffer
 *
 *   INITIAL CONDITIONS file f defined and open
 *
 *   FINAL CONDITIONS:
 *     NORMAL:          characters written to file f
 *
 *     ABNORMAL:        Retry the write if interrupted by signal
 *
 * EXTERNAL REFERENCES: write
 *
 * RETURNED VALUES:     return code from write
 */

eciowr(f, b, n)
int     f;			/* file descriptor number       */
char   *b;			/* pointer to buffer            */
int     n;			/* size of buffer to write      */

{
    int old_n = n;		/* original count */
    char *p = b;		/* pointer into the buffer */
    int rc;			/* return from write */

    while (n) {
      if ((rc = write(f, b, n)) < 0) {
	if (errno == EINTR)	/* interrupted system call */
	  continue;		/* just retry */

	if (errno == EAGAIN) {	/* NDELAY set and device is full */
	  usleep(100000);	/* sleep for .1 seconds */
	  continue;		/* and then retry */
	}

	if (old_n == n)		/* other error -- nothing written */
	  return -1;		/* just return -1 */
	else			/* partial write and then error */
	  break;		/* return what we wrote */
      }
      
      n -= rc;			/* decrement counter */
      p += rc;			/* move on to next batch */
    }
    return old_n - n;
}

/*
 * NAME:                eciock
 *
 * FUNCTION:            Retry to refresh the display if an error
 *                      was detected during an earlier update
 *
 * EXECUTION ENVIRONMENT:
 *
 *   PARAMETERS:        none
 *
 *   INITIAL CONDITIONS
 *
 *   FINAL CONDITIONS:
 *     NORMAL:          if earlier error, retry one time to refresh
 *
 *     ABNORMAL:        n/a
 *
 * EXTERNAL REFERENCES: cresetty
 *
 * RETURNED VALUES:     OK
 */

eciock(c) {


    if (need_refr && !doing_refr) {/* if error found and not in the */
				/* process of refresh then do   */
	doing_refr = TRUE;	/* set in refresh flag          */
	cresetty(TRUE);		/* refresh the display          */
	clearerr(stdout);	/* clear stdio's error flag     */
	doing_refr = FALSE;
	need_refr = FALSE;	/* clear error flag             */
    }

    return(OK);			/* no error detection           */
}


/*
 * NAME:                _tputvs
 *
 * FUNCTION: put string with possible varying length
 *      if varying length force output via write
 *      parm st is pointer to string to write
 */

_tputvs(st)			/* put string to display        */
char   *st;			/* pointer to string            */

{
    char opflag=0;

    if (st) {			/* if initialize string spec.   */
	if (*st)                /* if 1st char not null         */
	    _puts(st)		/* write as a string            */
	else {			/* 2nd char is length of data   */
	    eciofl(stdout);	/* flush any pending output     */
	    if ((_tty.c_oflag & OPOST) == OPOST) {
		_tty.c_oflag &= ~OPOST; /* turn off opost       */
		opflag = 1;             /* set a flag           */
		Stty(_tty_ch, &_tty);
	    }
	    eciowr(1, st + 2,(int)(*(st + 1)));
				/* write the control, bypass    */
				/* - buffering                  */
	    if (opflag) {
		_tty.c_oflag |= OPOST;  /* restore tty flags    */
		Stty(_tty_ch, &_tty);
	    }
	}
    }
}				/* end function _tputvs         */
