static char sccsid[] = "@(#)60	1.11  src/bos/usr/bin/que/libque/debug.c, cmdque, bos411, 9428A410j 1/29/93 12:01:31";
/*
 * COMPONENT_NAME: (CMDQUE) spooling commands
 *
 * FUNCTIONS: prtq, lprtq, prtargs, showparm, prtdev, prtfnames, prte, prtjdf, qdbg,
 *            dumpql, dumpdl
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "common.h"
#include "enq.h"
#include <stdio.h>

#ifdef DEBUG

prtq(stream,q)
FILE * stream;
struct q * q;
{
	if(getenv("DEBUG"))
	{
		if (!q)
		{
			fprintf(stream,"q is null\n");
			return(0);
		}
		fprintf(stream," q_next is %x \n", q->q_next?q->q_next:0);
        	fprintf(stream," q_devlist is %x \n", q->q_devlist?q->q_devlist:0);
        	fprintf(stream," q_entlist is %x \n",q->q_entlist?q->q_entlist:0);
        	fprintf(stream," q_name is %s \n", q->q_name);
        	fprintf(stream," q_rname is %s \n", q->q_rname);
        	fprintf(stream," q_acctf is %s \n", q->q_acctf);
        	fprintf(stream," q_hostname is %s \n", q->q_hostname);
        	fprintf(stream," q_disc is %s \n", q->q_disc ?"TRUE":"FALSE");
        	fprintf(stream," q_up is %s \n", q->q_up ?"TRUE":"FALSE");
        	fprintf(stream," q_devcount is %d \n", q->q_devcount);
	}
	return(0);
}


lprtq(stream,ql)
FILE * stream;
register struct q * ql;
{
	struct q * q;

	for ( q=ql ; 
		q ; 
	 	q=q->q_next)
			prtq(stream,q);
	return(0);
}

prtargs(e)
struct e *e;
{
	int i;
	if(getenv("DEBUG"))
	{	
		fprintf(stderr,"\te->e_argnum=%d prtfargs says...\n",e->e_argnum);
		for ( i=1;  i <= e->e_argnum;  i++ )
			fprintf(stderr,"\t|%s|\n",e->e_argvec[i]?e->e_argvec[i]:"NULL");
		fprintf(stderr,"\tleaving prtargs\n");
	}
	return(0);
}

showparms(p)
struct params *p;
{
	if(getenv("DEBUG"))
	{
printf("p->p_Aflg %d \n",	p->p_Aflg);
printf("p->p_Cflg %d \n",	p->p_Cflg);
printf("p->p_cflg %d \n",	p->p_cflg);
printf("p->p_qflg %d \n",	p->p_qflg);
printf("p->p_nflg %d \n",	p->p_nflg);
printf("p->p_xflg %d \n",	p->p_xflg);
printf("p->p_Dflg %d \n",	p->p_Dflg);
printf("p->p_Gflg %d \n",	p->p_Gflg);
printf("p->p_rflg %d \n",	p->p_rflg);
printf("p->p_Kflg %d \n",	p->p_Kflg);
printf("p->p_Uflg %d \n",	p->p_Uflg);
printf("p->p_Nflg %d \n",	p->p_Nflg);
printf("p->p_Mflg %d \n",	p->p_Mflg);
printf("p->p_mflg %d \n",	p->p_mflg);
printf("p->p_Rflg %d \n",	p->p_Rflg);
printf("p->p_Tflg %d \n",	p->p_Tflg);
printf("p->p_tflg %d \n",	p->p_tflg);
printf("p->p_aflg %d \n",	p->p_aflg);
printf("p->p_Bflg %d \n",	p->p_Bflg);
printf("p->p_oflg %d \n",	p->p_oflg);
printf("p->p_numflg %d \n",	p->p_numflg);
printf("p->p_Pflg %d \n",	p->p_Pflg);
printf("p->p_Nrem !%s! \n",	p->p_Nrem);
printf("p->p_Mrem !%s! \n",	p->p_Mrem);
printf("p->p_mrem !%s! \n",	p->p_mrem);
printf("p->p_Rrem !%s! \n",	p->p_Rrem);
printf("p->p_Trem !%s! \n",	p->p_Trem);
printf("p->p_trem !%s! \n",	p->p_trem);
printf("p->p_arem !%s! \n",	p->p_arem);
printf("p->p_Brem !%s! \n",	p->p_Brem);
printf("p->p_orem !%s! \n",	p->p_orem);
printf("p->p_numrem !%s! \n",	p->p_numrem);
printf("p->p_xrem !%s! \n",	p->p_xrem);
printf("p->p_Prem !%s! \n",	p->p_Prem);
	}
	return(0);
}


prtdev(stream,d)
FILE *stream;
register struct d *d;
{
	if(getenv("DEBUG"))
	{
		if (!d) fprintf(stream,"d is NULL\n");
		if (!d) return(0);
		fprintf(stream,"d_head %d d_align %d  d_trail %d d_next %x d_q %x d_e %x d_backend %s d_pid %d d_user %s d_file %s d_feed %d \n",
 			d->d_head,  d->d_align, d->d_trail,d->d_next, d->d_q , d->d_e ,
			d->d_backend,d->d_pid,d->d_user,d->d_file ,d->d_feed);
	}
}

/* PRinT File NAMES	*/
void prtfnames(head)
register struct str_list *head;  /* head of the list of filenames */
{
	register struct str_list *fl, *prev_p ;

	if(getenv("DEBUG"))
	{
		fprintf(stderr,"\t prtfnames says...\n");
		for (prev_p=NULL,fl=head; fl; fl = fl->s_next)
		 	fprintf(stderr,"\tunffp->s_name=%s\t",fl->s_name);
		printf("\n");
	}
}

prte(e)
register struct e *e;
{	
	if(getenv("DEBUG"))
	{
		if (!e) fprintf(stderr,"\te is NULL\n");
		if (!e) return(0);
		prtfnames(e->e_fnames);
		prtargs(e);
	}
	return(0);
}


prtjdf(dev,e)
register struct d *dev;
register struct e *e;
{
	int i;

	if(getenv("DEBUG"))
	{
		for (i = 0; i < e->e_argnum; i++)
		    printf("argvec[%d]=%s\n",i,e->e_argvec[i]);
		printf("msg=/%s/\n",e->e_msg);
		prtdev(stderr,dev);
		prte(e);
		printf("chk e,stfile\n");
	}
	return(0);
}

qdbg(q)
struct q *q;
{
	if(getenv("DEBUG"))
	{
		fprintf(stderr,"q_name     %s\n", q->q_name);
		fprintf(stderr,"q_rname    %s\n", q->q_rname);
		fprintf(stderr,"q_hostname %s\n", q->q_hostname);
		fprintf(stderr,"q_devcount %d\n", q->q_devcount);
	}
	return(0);
}

dumpql(ql)
	struct q *ql; {

	if(getenv("DEBUG"))
	{
		while(ql)
		{
			printf("QUEUE: name %s, acctf %s\n",
				ql->q_name, ql->q_acctf);
			printf(" disc %d, up %d, dcount %d \n",
				 ql->q_disc, ql->q_up, ql->q_devcount);
			if( ql->q_devlist )
				dumpdl(ql->q_devlist);
			ql = ql->q_next;
		}
	}
	return(0);
}

dumpdl(dl)
	struct d *dl; {

	if(getenv("DEBUG"))
	{
		fprintf(stderr,"in dumpdl\n");
		while(dl)
		{
			prtdev(stderr,dl);
			dl = dl->d_next;
		}
	}
	return(0);
}

#endif
