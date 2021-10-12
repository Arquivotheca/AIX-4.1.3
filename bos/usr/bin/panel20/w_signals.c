#ifdef COMPILE_SCCSID
static char sccsid[] = "@(#)94 1.1 src/bos/usr/bin/panel20/w_signals.c, cmdhia, bos411, 9428A410j 2/14/90 17:19:55";
#endif

/*
 * COMPONENT_NAME: (CMDHIA) Converts signal call to sigaction call
 *
 * FUNCTIONS: whp_signal
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1986, 1990
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <signal.h>

void (*whp_signal(int sig, void (*func)(int))) (int);

void (*whp_signal(int sig, void (*func)(int))) (int)
{
    struct sigaction act, oact;

    sigaction(sig, (struct sigaction *)NULL, &act);  /* get current signal */
    act.sa_handler = func;                           /* set new handler    */
    
    switch (sig)  {
        case SIGCONT:
        case SIGTTIN:
        case SIGTTOU:
        case SIGTSTP:
        case SIGSTOP:  
            /* system calls are restartable after a job control signal */ 
            act.sa_flags |= SA_RESTART;
            break;
        }  

    sigaction(sig, &act, &oact);
    return(oact.sa_handler);         /* return old signal handler */
}
