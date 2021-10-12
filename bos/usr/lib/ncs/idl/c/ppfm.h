/*
 * $Log: ppfm.h,v $
 * Revision 1.15  1993/02/25  16:56:02  ddj
 * Added hpux to list of machines that use setjmp vs sigsetjmp.
 *
 * Revision 1.14  1993/01/14  14:15:53  ddj
 * Added SVR4 to list of platforms that support volatile.
 *
 * Revision 1.13  1992/12/06  17:41:33  ddj
 * Change setjmp/sigsetjmp decision to list exceptions only.
 *
 * Revision 1.12  1992/12/06  17:20:53  ddj
 * Fixed last change since UNIX is not always defined when ppfm.h is used.
 *
 * Revision 1.11  1992/12/01  16:45:48  ddj
 * Use sigsetjmp for all UNIX ports; VMS & ms-dos still use old setjmp.
 * Use Volatile for ultrix again, since conflicts have been worked around.
 *
 * Revision 1.10  1992/06/25  15:03:08  ddj
 * Make AIX use setjmp, not sigsetjmp.
 *
 * Revision 1.9  1992/04/14  22:09:57  ddj
 * Changed HPUX to hpux to decide on including "ultrix.h".
 *
 * Revision 1.8  1992/02/27  19:00:48  robin
 * Ultrix was not ready for the Volatile/volatile stuff, Backed
 * out this just for ultrix.
 *
 * Revision 1.7  1992/02/06  18:44:00  ddj
 * Include "ultrix.h" on HPUX ports as well, since we link with a system-
 * provided libncs.a that is contaminated with Dollar Signs.
 *
 * Revision 1.6  1991/11/06  22:27:29  ddj
 * Made definition of volatile and Volatile clearer and added ultrix to list
 * that support volatile declarations.
 *
 * Revision 1.5  1991/07/29  15:59:26  ddj
 * Added include of ultrix.h to use digital-provided ncs library
 * with our include files and nidl compiler.
 *
 * Revision 1.4  1991/05/16  14:00:19  ddj
 * Put old clix conditional code under control of "clix_old" define.
 *
 * Revision 1.3  1991/04/18  17:09:30  ddj
 * Changes to support clix platform.
 *
 * Revision 1.2  1991/04/01  21:28:31  ddj
 * Use sigjmp_buf and sigsetjmp for SYS5 builds.
 *
 * Revision 1.1  1991/04/01  21:26:25  ddj
 * Initial revision
 *
 * $EndLog$
 */
/*
 * ========================================================================== 
 * Copyright  1987 by Apollo Computer Inc., Chelmsford, Massachusetts
 * 
 * All Rights Reserved
 * 
 * All Apollo source code software programs, object code software programs,
 * documentation and copies thereof shall contain the copyright notice above
 * and this permission notice.  Apollo Computer Inc. reserves all rights,
 * title and interest with respect to copying, modification or the
 * distribution of such software programs and associated documentation,
 * except those rights specifically granted by Apollo in a Product Software
 * Program License or Source Code License between Apollo and Licensee.
 * Without this License, such software programs may not be used, copied,
 * modified or distributed in source or object code form.  Further, the
 * copyright notice must appear on the media, the supporting documentation
 * and packaging.  A Source Code License does not grant any rights to use
 * Apollo Computer's name or trademarks in advertising or publicity, with
 * respect to the distribution of the software programs without the specific
 * prior written permission of Apollo.  Trademark agreements may be obtained
 * in a separate Trademark License Agreement.
 * 
 * Apollo disclaims all warranties, express or implied, with respect to
 * the Software Programs including the implied warranties of merchantability
 * and fitness, for a particular purpose.  In no event shall Apollo be liable
 * for any special, indirect or consequential damages or any damages
 * whatsoever resulting from loss of use, data or profits whether in an
 * action of contract or tort, arising out of or in connection with the
 * use or performance of such software programs.
 * ========================================================================== 
 */

#ifndef ppfm_included
#define ppfm_included

#if defined(ultrix) || defined(hpux)
#include "ultrix.h"
#endif

/*
 * Portable Process Fault Manager definitions.
 *
 * Because this interface has undergone so many variants, we enumerate the
 * exact interface below.  Portable programs should use only what appears below.
 * (Note that some of them are actually macros.)
 *
 * void pfm_$init(unsigned long flags);
 * status_$t pfm_$cleanup(pfm_$cleanup_rec *crec);
 * void pfm_$signal(status_$t st);
 * void pfm_$reset_cleanup(pfm_$cleanup_rec *crec, status_$t *st);
 * void pfm_$rls_cleanup(pfm_$cleanup_rec *crec, status_$t *st);
 * void pfm_$inhibit_faults(void);
 * void pfm_$enable_faults(void);
 * void pfm_$inhibit(void);
 * void pfm_$enable(void);
 * void pgm_$exit(void);
 *
 *
 * To create portable code, all users of pfm_$cleanup() must declare a local variable 
 * to be "Volatile" (note the upper case 'V') if proper execution of the cleanup handler 
 * code path requires that modifications made to the variable in the non-handler path be 
 * visible to the handler. This is necessary because pfm_$cleanup() uses the standard C 
 * runtime routine setjmp() (which has certain implications -see K&R or the ANSI spec-).
 * For those that don't support volatile, there's nothing we can do.
 */

#if defined(apollo)
#  define Volatile
#else
#  if defined(__STDC__)
#      define Volatile volatile
#  else
#    if defined(vaxc) || defined(hpux) || defined(ultrix) || defined(SVR4)
#      define Volatile volatile
#    else
#      define Volatile
#    endif
#  endif
#endif


#ifdef apollo

#include <apollo/pfm.h>

#define pfm__init(junk)                 /* No initialization required on Apollo */

#else

#ifdef MSDOS
#define _JBLEN 20                   /* Must match std.h */
#define setjmp  setjmp_nck
#define longjmp longjmp_nck
typedef char jmp_buf[_JBLEN];
extern long setjmp_nck(jmp_buf);
#endif

#ifndef _JBLEN
#  include <setjmp.h>
#  ifndef _JBLEN
#    define _JBLEN (sizeof(jmp_buf) / sizeof(int))
#  endif
#endif

#ifdef	clix
#ifndef sigmask
#define	sigmask(n)		((unsigned long)1 << ((n) - 1))
#endif
#ifdef clix_old
typedef long sigset_t;

#define	sigemptyset(set)	(*(set) = 0)
#define	sigfillset(set)		(*(set) = 0xffffffff)
#define	sigaddset(set,sig)	(*(set) |= sigmask(sig))
#define	sigdelset(set,sig)	(*(set) &= ~sigmask(sig))
#define	sigismember(set,sig)	(sigmask(sig) & *(set))

#define	SIG_SETMASK	0
#define	SIG_BLOCK	1
#define	SIG_UNBLOCK	2

typedef double sigjmp_buf[_JBLEN/2 + _FLTLEN + 2];

#define	_SAVEFLAG	(_JBLEN + _FLTLEN*2 + 1)
#define	_SAVEDMASK	(_JBLEN + _FLTLEN*2 + 2)
#define	saveflag(sjb)	(((long *)(sjb))[_SAVEFLAG])
#define	savedmask(sjb)	(((long *)(sjb))[_SAVEDMASK])

int sigsetjmp();
void siglongjmp();
#endif	/* clix_old */
#endif	/* clix */


#define pfm__module_code                    0x03040000
#define pfm__bad_rls_order                  (pfm__module_code + 1)
#define pfm__cleanup_set                    (pfm__module_code + 3)
#define pfm__signalled_zero                 (pfm__module_code + 0xffff)

typedef struct jmp_buf_elt_t {
    struct jmp_buf_elt_t *next;
    int fault_inh_count;
#if 	!defined(vms) && !defined(MSDOS) && !defined(hpux)
    sigjmp_buf buf;
#else	/* UNIX */
    jmp_buf buf;
#endif	/* UNIX */
    long setjmp_val;        /* This is "long" and not "int" on purpose -- keep MS/DOS happy */
} pfm__cleanup_rec;  

void pfm__signal(
#ifdef __STDC__
    status__t st
#endif
);

void pfm__inhibit_faults(
#ifdef __STDC__
    void
#endif
);

void pfm__enable_faults(
#ifdef __STDC__
    void
#endif
);

void pfm__inhibit(
#ifdef __STDC__
    void
#endif
);

void pfm__enable(
#ifdef __STDC__
    void
#endif
);

void pfm___rls_cleanup(
#ifdef __STDC__
    pfm__cleanup_rec *crec, 
    status__t *st
#endif
);

void pfm___reset_cleanup(
#ifdef __STDC__
    pfm__cleanup_rec *crec, 
    status__t *st
#endif
);

status__t pfm___cleanup(
#ifdef __STDC__
    long st, 
    pfm__cleanup_rec *crec
#endif
);


#if 	!defined(vms) && !defined(MSDOS) && !defined(hpux)
#define pfm____cleanup(crec) ( \
    (crec)->setjmp_val = sigsetjmp((crec)->buf, 1), \
    pfm___cleanup((crec)->setjmp_val, crec) \
)
#else	/* UNIX */
#define pfm____cleanup(crec) ( \
    (crec)->setjmp_val = setjmp((crec)->buf), \
    pfm___cleanup((crec)->setjmp_val, crec) \
)
#endif	/* UNIX */

void pfm__init(
#ifdef __STDC__
    unsigned long flags
#endif
);

void pgm__exit(
#ifdef __STDC__
    void
#endif
);

#ifndef pfm_included 
#define pfm__cleanup            pfm____cleanup
#define pfm__rls_cleanup        pfm___rls_cleanup
#define pfm__reset_cleanup      pfm___reset_cleanup
#endif
                                     
#endif

/*
 * Flags to "pfm_$init"
 */  

#define pfm__init_signal_handlers   0x00000001

#endif
