static char sccsid[] = "@(#)20	1.1  src/bos/diag/da/sys/dmptu.c, dasys, bos412, 9446A412a 11/9/94 07:06:40";
/*
 *   COMPONENT_NAME: DASYS
 *
 *   FUNCTIONS: Multi-Processor Test Unit
 *
 *   ORIGINS: 83,27
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *
 *   Copyright (C) Bull S.A. 1994
 *   LEVEL 1, 5 Years Bull Confidential Information
 */

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <nl_types.h>
#include <limits.h>
#include <math.h>
#include <locale.h>
#include <sys/ioctl.h>
#include <sys/devinfo.h>
#include <sys/iplcb.h>
#include <sys/mdio.h>
#include <time.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include <sys/errids.h>
#include <signal.h>
#include <sys/mman.h>
#include <pthread.h>
#include <sys/processor.h>
#include "diag/class_def.h"	/* Includes odm definitions */
#include "diag/da.h"		/* The following header files contain */
#include "diag/diag_exit.h"	/* information that is specific to this */
#include "diag/tmdefs.h"	/* application. */
#include "diag/tm_input.h"
#include "diag/diago.h"
#include "diag/diag.h"
#include "diag/dcda_msg.h"
#include "diag/modid.h"

/* EXTERNALLY DEFINED VARIABLES */
extern int      optind;		/* defined by getopt()   */
extern char     *optarg;	/* defined by getopt()   */
extern  nl_catd diag_catopen(char *, int);
nl_catd	fdes;				/* file descriptor for catalog file */
int     rc;

struct errdata     err_data;
int rc;
int fd;
int get_enddate(char *, char *, int);
int find_cpucard(int, fru_t *);
int int_proc_test();

#define	MAXCPU	8

caddr_t	data1;
caddr_t	data2;
int	pagesize,pagesizeint;

void	f_sigbus();
void	f_sigsegv();

struct sigaction sigbus = {
	(void (*)(int))f_sigbus,0,(int)NULL};

struct sigaction sigsegv = {
	(void (*)(int))f_sigsegv,0,(int)NULL};

int	exit_code=0;
int	processor_present[MAXCPU];
main(argc,argv)
int argc;
char *argv[];
{
int c,i,m,p;
	p=0;
	while((c = getopt(argc,argv,"p:")) != EOF)
	switch((char) c) {
		case 'p':
			p=atoi(optarg);
			break;
		default:
			break;
	}
	for(i=0,m=1;i<MAXCPU;i++)
	{
		if(p & m)	processor_present[i]=1;
		else	processor_present[i]=0;
		m= m << 1;
	}
	int_proc_test();
	exit(exit_code);
}
/*
* NAME: thread_procedure
*
* FUNCTION:
*
* EXECUTION ENVIRONMENT:
*
* RETURNS: NONE
*/

void *thread_procedure(void *ind)
{
volatile int *target;
int offset;
cpu_t	cpu;
tid_t tid;
pthread_t       tidp;
	cpu= *(int *)ind;
	tid=thread_self();
	bindprocessor(BINDTHREAD,tid,cpu);
	tidp=pthread_self();
	offset=(int)tidp % pagesizeint;
	target=(volatile int *)data1 + offset;
	(void *)sigaction(SIGBUS,&sigbus,(struct sigaction *)NULL);
	(void *)sigaction(SIGSEGV,&sigsegv,(struct sigaction *)NULL);

	for(*target=0;target;)
	{
		++*target;
	}
	return((void *)0);
}

/*
* NAME: int_proc_test
*
* FUNCTION:	Interprocessor Test
*
* EXECUTION ENVIRONMENT:
*
* RETURNS: NONE
*/
int int_proc_test()
{
uint	a1,a2;
volatile int	*i1,*i2;
pthread_t new_thread,thread_table[MAXCPU];
int	i,k,rc;
char	buf[80];
uint	errors;
int	arg[MAXCPU][1];
struct ot {
	volatile int *target;
	volatile int *save;
	} active_threads[MAXCPU];

	errors = 0;
	pagesize = getpagesize();
	pagesizeint = pagesize / 4;

	data1=0;
	data1=mmap(data1,pagesize,PROT_READ|PROT_WRITE,
		MAP_ANONYMOUS|MAP_SHARED, -1,0);

	data2=0;
	data2=mmap(data2,pagesize,PROT_READ|PROT_WRITE,
		MAP_ANONYMOUS|MAP_SHARED, -1,0);

	pthread_init();

	(void *)sigaction(SIGBUS,&sigbus,(struct sigaction *)NULL);
	(void *)sigaction(SIGSEGV,&sigsegv,(struct sigaction *)NULL);

	for (k=0; k < MAXCPU && !errors;k++)
	{
	tid_t	tid;
	volatile int j;
		if(processor_present[k]!=1)	continue;
		tid=thread_self();
		bindprocessor(BINDTHREAD,tid,k);
		for(i=0,i1=(volatile int *)data1; i < pagesizeint ; i++,i1++)
			*i1=0;

		for(i=0; i < MAXCPU;i++)
		{
		int offset;
			if(processor_present[i]!=1 || i==k)	continue;
			arg[i][0]=i;
			rc=pthread_create(&new_thread,&pthread_attr_default,
				thread_procedure, &arg[i]);
			if(rc) {
				DA_SETRC_ERROR(DA_ERROR_OTHER);
				DA_EXIT();
			}
			thread_table[i]=new_thread;
			offset=(int)new_thread % pagesizeint;

			active_threads[i].target=(volatile int *)data1 + offset;
			active_threads[i].save  =(volatile int *)data2 + offset;
		}

		/* Check that thread is running	*/
		for(i=0; i < MAXCPU;i++)
		{
			if(processor_present[i]!=1 || i==k)     continue;
			for(j = 1000000; --j>=0;); /* wait just a bit */
			i1=active_threads[i].target;
			a1= *i1;

			for(j = 1000000; --j>=0 && a1== *i1;); /* still wait */

			if(a1 == *i1) {		/* wait with usleep */
				for(j = 10000; --j>=0 && a1== *i1;)
					usleep(1000);
			}

			if(a1 == 0 && *i1 ==0 ) { /* thread seems not running */
				/*	ERRORS		*/
				/*	60% MPB		*/
				/*	20% CPU k	*/
				/*	20% CPU i;	*/
				errors++;
				exit_code |= 1 << i;
				exit_code |= 1 << k;
			}
		}

		mprotect(data1,pagesize,PROT_READ);
		for(i=0; i < MAXCPU;i++)
		{
			if(processor_present[i]!=1)     continue;
			for(j = 1000000; --j>=0;); /* wait just a bit */
		}

		for(i=0; i <MAXCPU ;i++)
		{
			if(processor_present[i]!=1 || i==k)	continue;
			i1=active_threads[i].target;
			i2=active_threads[i].save;
			*i2 = *i1;
		}

		for(i=0; i < MAXCPU;i++)
		{
			if(processor_present[i]!=1)     continue;
			for(j = 1000000; --j>=0;); /* wait just a bit */
		}

		for(i = 0;i< MAXCPU; i++)
		{
			if(processor_present[i]!=1 || i==k)	continue;
			i1=active_threads[i].target;
			i2=active_threads[i].save;

			a1= *i1;
			a2= *i2;
			if(a1 != a2) {
				/*	ERRORS		*/
				/*	60% MPB		*/
				/*	20% CPU k	*/
				/*	20% CPU i;	*/
				errors++;
				exit_code |= 1 << i;
				exit_code |= 1 << k;
			}
		}
		for(i = 0;i< MAXCPU; i++)
		{
			if(processor_present[i]!=1 || i==k)	continue;
			rc=pthread_detach(thread_table[i]);
		}
		mprotect(data1,pagesize,PROT_READ|PROT_WRITE);

	}
	return(errors);
}

void f_sigbus()
{
sigset_t set;
	SIGINITSET(set);
	SIGADDSET(set,SIGBUS);
	sigthreadmask(SIG_UNBLOCK,&set,(sigset_t *)NULL);
	(void *)sigaction(SIGBUS,&sigbus,(struct sigaction *)NULL);
	pthread_exit(0);
}
void f_sigsegv()
{
sigset_t set;
	SIGINITSET(set);
	SIGADDSET(set,SIGSEGV);
	sigthreadmask(SIG_UNBLOCK,&set,(sigset_t *)NULL);
	(void *)sigaction(SIGSEGV,&sigsegv,(struct sigaction *)NULL);
	pthread_exit(0);
}

