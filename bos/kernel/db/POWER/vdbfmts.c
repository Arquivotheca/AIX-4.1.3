static char sccsid[] = "@(#)75	1.22  src/bos/kernel/db/POWER/vdbfmts.c, sysdb, bos411, 9435A411a 8/29/94 19:53:52";
/*
 * COMPONENT_NAME: (SYSDB) Kernel Debugger
 *
 * FUNCTIONS: fmts, next_module, get_stackframe
 *
 * ORIGINS: 27 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
*/

#include <sys/types.h>
#include <sys/seg.h>			/* segment values		*/
#include <sys/debug.h>			/* generalized debug field      */
#include <sys/user.h>
#include "debaddr.h"			/* Address structure		*/
#include "stack.h"			/* COMLINK stack stuff		*/
#include "parse.h"			/* Parser structure.		*/
#include "vdbfindm.h"			/* declares for findit 		*/
#include "debvars.h"			/* access to current GPR15 	*/
#include "vdbfmts.h"			/* declares for this file 	*/


/*                                                                   
 * EXTERNAL PROCEDURES CALLED: 
 */

extern	char *getterm();		/* Get from the terminal	*/

/*                                                                   
 * GLOBAL VARIABLES USED 
 */


/*
 * NAME: fmts
 *                                                                    
 * FUNCTION:    Provide a formatted stack traceback display.
 *                                                                    
 * RETURN VALUE:  Must always return 0.
 */  
#define VIRT t->virt
fmts(ps)				/* formatted stack trace */
struct parse_out *ps;
{
	struct debaddr	*setup_debaddr(),*t;
	itemtype 	*s;	 		/* stack pointer	*/
	int 		First;
	caddr_t		iar;			/* iar */
	struct dfdata 	*proctag,*oldproctag;	/* DF procedure tag */
	itemtype 	*Frame_ptr;
	int seg_id,paged;
#ifndef _THREADS
	struct user *read_uarea(), *user_area=0;
#else /* _THREADS */
	struct uthread *read_utarea(), *uthread_area=0, *ut;
#endif /* THREADS */
	
	t = setup_debaddr(debvars[IDIAR].hv,T_BIT);
	if (ps->num_tok<1) { 		/* no pid indicated by user */
		iar = t->addr;
		s = (itemtype *) debvars[IDGPRS+1].hv;
	}
	else {				/* get other procs iar&stack */
	
#ifndef _THREADS
		printf("Stack Traceback for PID 0x%x\n",ps->token[1].hv);
		/* find the PID we are intersted in */
		if((seg_id=get_proc(ps->token[1].hv))<0) 
			return 0;
	
		/* copy the uarea using Get_from_memory() */
		if((user_area=(struct user *) read_uarea(seg_id,&paged)) ==
		(struct user *) -1) {
			printf("User area unreachable.\n");
			return 0;
		}
		iar = (caddr_t) user_area->u_save.iar;
		s =  (itemtype *) user_area->u_save.gpr[1];
#else /* _THREADS */
		printf("Stack Traceback for TID 0x%x\n",ps->token[1].hv);
		/* find the TID we are interested in */
		if((seg_id=get_thread(ps->token[1].hv,&ut))<0)
			return 0;

		/* copy the utarea using Get_from_memory() */
		if((uthread_area=(struct uthread *) read_utarea(seg_id,ut)) ==
		(struct uthread *) -1) {
			printf("Uthread area unreachable.\n");
			return 0;
		}
		iar = (caddr_t) uthread_area->ut_save.iar;
		s =  (itemtype *) uthread_area->ut_save.gpr[1];
#endif /* THREADS */
	}

	/* 
	 * Clear the screen.
	 * print the iar and stack addresses.
	 */
	clrdsp();
	printf("Beginning IAR: 0x%08x      Beginning Stack: 0x%08x\n",iar,s);

	First = 1;
    	Frame_ptr = s;

    	do  {

		/* Locate and print the standard parts of the stack */
#ifndef _THREADS
		if(get_stackframe(proctag,Frame_ptr,VIRT,user_area) == -1) {
#else /* _THREADS */
		if(get_stackframe(proctag,Frame_ptr,VIRT,uthread_area) == -1) {
#endif /* THREADS */
			printf("Trace back terminated.\n");
			return 0;
		}

		/* get the new code module address and the new stack area */
#ifndef _THREADS
	       if(next_module(proctag,&Frame_ptr,&iar,VIRT,First,user_area)==-1)
#else /* _THREADS */
	    if(next_module(proctag,&Frame_ptr,&iar,VIRT,First,uthread_area)==-1)
#endif /* THREADS */
		{
			printf("Trace back terminated.\n");
			return 0;
		}

		printf("Returning to Stack frame at 0x%x\n", Frame_ptr);

		printf("Press ENTER to continue or x to exit:\n");
		if (*getterm()=='x')  {
			printf("Trace back terminated.\n");
			return 0;
		}
		printf("\n\n");
		First = 0;

    	} while(Frame_ptr);

	printf("Trace back complete.\n");
	return(0);
}	/* end of fmts() */



/* 
 * Function:	Find the calling code module to search for a DF procedure tag.
 * 		Also, increment the stack pointer to the calling modules stack.
 *		(This is overkill currently, but when we handle the
 *		full spectrum of stacks this routine will be necessary.
 *		Otherwise, the simple cases could be easily handled in the
 *		previous routines.)
 * Returns:	-1 if problem requiring termination of traceback.
 *		Otherwise, sets frame pointer and return address to new values
 *		and returns 0.
 */
#ifndef _THREADS
next_module(ptag,stk_ptr,ret_addr,xlate,First,user_area)
#else /* _THREADS */
next_module(ptag,stk_ptr,ret_addr,xlate,First,uthread_area)
#endif
struct dfdata *ptag;			/* procedure tag following code module*/
itemtype **ret_addr,**stk_ptr;		/* the iar and the stack pointer */
#ifndef _THREADS
struct user  *user_area;		/* if not 0 then tracing another proc*/
#else /* _THREADS */
struct uthread *uthread_area;		/* if not 0 then tracing another thrd*/
#endif /* THREADS */
int	xlate;				/* translation on/off	*/
int	First;				/* !0 = begining of traceback	*/
{
	itemtype cnt, back_and_lr[3];
	itemtype srval;	      /* segreg val for stack segmant of another proc*/
#ifndef _THREADS
	int other = user_area != 0;		/* tracing another proc */
#else /* _THREADS */
	int other = uthread_area != 0;	/* tracing another thread */
#endif /* THREADS */

	/* if we are getting another procs stack to trace we have to */
	/* initialize the seg reg for it. */
	if(other) {	/* another procs stack */
		/* get other procs segval to use, load it and save current */
		srval= 
#ifndef _THREADS
		 aim_sr(user_area->u_save.as.srval[((ulong)*stk_ptr>>SEGSHIFT)],
		  *stk_ptr);
#else /* _THREADS */
		 aim_sr(uthread_area->ut_save.as.srval
				       [((ulong)*stk_ptr>>SEGSHIFT)], *stk_ptr);
#endif /* THREADS */
	}
	if(!Get_from_memory(*stk_ptr,xlate,back_and_lr,3*REGSIZE))
	{
		printf("The stack does not currently reside in memory.\n");
		if (other)
			aim_sr(srval, *stk_ptr); /* replace orig segval */
		return -1;
	}

	if(other) 	/* another procs stack */
		aim_sr(srval,*stk_ptr); /* replace orig segval */

	*stk_ptr = (itemtype *) back_and_lr[0];

	return 0;	/* OK */
}


/* 
 * Function:	find the begining of the stack frame and print
 * 		the useful values.
 * Returns:	-1 if problem with stack traceback; 
 *		0 otherwise;
 */
#ifndef _THREADS
get_stackframe(proctag,fp,xlate,user_area)
#else /* _THREADS */
get_stackframe(proctag,fp,xlate,uthread_area)
#endif /* THREADS */
struct dfdata *proctag;			/* procedure tag following code module*/
itemtype *fp;				/* the stack pointer, R1 */
int	xlate;				/* translation on/off	*/
#ifndef _THREADS
struct user  *user_area;		/* if not 0 then tracing another proc*/
#else /* _THREADS */
struct uthread *uthread_area;		/* if not 0 then tracing another thrd*/
#endif /* THREADS */
{
	itemtype *tmp_fp,stk[MAXREGS+1]; /* all the stk items we'll ever need */
	itemtype srval;	/* segreg val for stack segmant of another proc*/
	register int cnt,byte_cnt,back_chain;
	int lines=1, page=10;	/* 10 lines per "page" */
#ifndef _THREADS
	int other = user_area != 0;		/* tracing another proc */
#else /* _THREADS */
	int other = uthread_area != 0;	/* tracing another thread */
#endif

	/* if we are getting another procs stack to trace we have to */
	/* initialize the seg reg for it. */
	if(other) {	/* another procs stack */
#ifndef _THREADS
		/* get other procs segval to use, load it and save current */
		srval= aim_sr(user_area->u_save.as.srval[((ulong)fp>>SEGSHIFT)],
			fp);
#else /* _THREADS */
		/* get other thrds segval to use, load it and save current */
		srval= aim_sr(uthread_area->ut_save.as.srval
						   [((ulong)fp>>SEGSHIFT)], fp);
#endif /* THREADS */
	}

	if(!Get_from_memory(fp,xlate,stk,(LINK_AREA+OUT_ARGS)*REGSIZE)) {
		printf("The stack does not currently reside in memory.\n");
		if (other)
			aim_sr(srval,fp); /* replace orig segval */
		return -1;
	}
	tmp_fp = stk;
	
	back_chain = *tmp_fp;
	
	printf("Chain:0x%08x  CR:0x%08x  Ret Addr:0x%08x  TOC:0x%08x\n",
		*tmp_fp,*(tmp_fp+1),*(tmp_fp+2),*(tmp_fp+5));
	tmp_fp += 6;
	printf("P1:0x%08x  P2:0x%08x  P3:0x%08x  P4:0x%08x\n",
	       *tmp_fp,*(tmp_fp+1),*(tmp_fp+2),*(tmp_fp+3),*(tmp_fp+4));
	tmp_fp += 4;
	printf("P5:0x%08x  P6:0x%08x  P7:0x%08x  P8:0x%08x\n",
	       *tmp_fp,*(tmp_fp+1),*(tmp_fp+2),*(tmp_fp+3),*(tmp_fp+4));

	/* print rest of stack */
	tmp_fp = fp + (LINK_AREA+OUT_ARGS);
	cnt = (back_chain - (int)tmp_fp) ;

	while(cnt > 0) {
		if(cnt >= 16)
			byte_cnt = 16;
		else
			byte_cnt = cnt;

		debug_display(tmp_fp,byte_cnt,xlate);
		
		tmp_fp += byte_cnt/REGSIZE;
		cnt -= byte_cnt;

		/* paginate here */
		if(lines++ > page){
			printf("Press ENTER to continue or x to exit:\n");
			if (*getterm()=='x')  {
				if(other) { /* another procs stack */
				  aim_sr(srval,fp);  /* replace orig segval */
				}
				return -1;
			}
			lines = 1;
		}

	}

	if(other) 	/* another procs stack */
		aim_sr(srval,fp); /* replace orig segval */

	return 0;
}

#ifdef _THREADS

#ifndef ULONG
#define ULONG unsigned long
#endif
#define MAX_WITHIN_SEG(addr,offset) \
	((((ULONG)(addr)+(ULONG)(offset)) > (((ULONG)(addr))|0x0fffffff)) ?\
			((ULONG)(addr))|0x0fffffff :\
			(ULONG)(addr) + (ULONG)(offset))

stack_traceback(ps)		/* Show only ret addrs & routine names */
struct parse_out *ps;
{
struct debaddr	*setup_debaddr(),*t;
	itemtype 	*s;	 		/* stack pointer	*/
	int 		First,count;
	caddr_t		iar,lr;			/* iar,lr */
	struct dfdata 	*proctag;	/* DF procedure tag */
	itemtype 	*Frame_ptr;
	int seg_id,paged;
	struct uthread *read_utarea(), *uthread_area=0, *ut;
	itemtype *tmp_fp,stk[MAXREGS+1]; /* Local vars to work w/ stack items */
	itemtype curr_addr;	/* Var to hold addr we are displaying */
	itemtype srval;		/* segreg val for stack seg of a diff thread */
	char cmdstring[40];	/* string for creating find command */
	struct parse_out parse_find; /* Parser structure for find command */
	caddr_t found_addr;	/* Address where fullword 0 was found */

	
	t = setup_debaddr(debvars[IDIAR].hv,T_BIT);
	if (ps->num_tok<1) { 		/* no pid indicated by user */
		iar = t->addr;
		t = setup_debaddr(debvars[IDLR].hv,T_BIT);
		lr = t->addr;
		s = (itemtype *) debvars[IDGPRS+1].hv;
	}
	else {				/* get other procs iar&stack */
	
		printf("Stack Traceback with routine names for TID 0x%x\n",ps->token[1].hv);
		/* find the TID we are interested in */
		if((seg_id=get_thread(ps->token[1].hv,&ut))<0)
			return 0;

		/* copy the utarea using Get_from_memory() */
		if((uthread_area=(struct uthread *) read_utarea(seg_id,ut)) ==
		(struct uthread *) -1) {
			printf("Uthread area unreachable.\n");
			return 0;
		}
		iar = (caddr_t) uthread_area->ut_save.iar;
		lr = (caddr_t) uthread_area->ut_save.lr;
		s =  (itemtype *) uthread_area->ut_save.gpr[1];
	}

	/* 
	 * Clear the screen.
	 * print the iar and stack addresses.
	 */
	clrdsp();
	printf("Beginning Stack: 0x%08x\n",s);

	First = 1;
    	Frame_ptr = s;
	count = 0;

    	do  {

		/* If this is the first pass through, use the iar as the */
		/* address to display. */
		if (count == 0) {
			curr_addr = (itemtype)iar;
			/* Now show the user the return address we found */
			printf("IAR: 0x%08x\n", curr_addr);
			if (uthread_area != 0) { /* Looking at a different thread */
				/* Get other thread's segval to use */
				srval = aim_sr(uthread_area->ut_save.as.srval
						[((ulong)iar>>SEGSHIFT)], iar);
			}
		}
		/* On the second pass, use the LR. */
		else if (count == 1) {
			curr_addr = (itemtype)lr;
			/* Now show the user the return address we found */
			printf("LR: 0x%08x\n", curr_addr);
			if (uthread_area != 0) { /* Looking at a different thread */
				/* Get other thread's segval to use */
				srval = aim_sr(uthread_area->ut_save.as.srval
						[((ulong)lr>>SEGSHIFT)], lr);
			}
		}
		/* Otherwise, use the stack */
		else {
			/* Find return address for this stack frame */
			if (uthread_area != 0) { /* Looking at a different thread */
				/* Get other thread's segval to use */
				srval = aim_sr(uthread_area->ut_save.as.srval
						[((ulong)Frame_ptr>>SEGSHIFT)], Frame_ptr);
			}

			if(!Get_from_memory(Frame_ptr,VIRT,stk,(LINK_AREA+OUT_ARGS)*REGSIZE)) {
				printf("The stack does not currently reside in memory.\n");
				if (uthread_area != 0)
					aim_sr(srval,Frame_ptr); /* replace orig segval*/
				return;
			}
			tmp_fp = stk;
			curr_addr = *(tmp_fp+2);

			/* If T bit is on in segreg for addr, we're done */
			if (debvars[IDSEGS+(curr_addr>>SEGSHIFT)].hv & 0x80000000) {
				if (uthread_area != 0)
					aim_sr(srval,Frame_ptr); /* replace orig segval*/
				printf("Ret Addr: 0x%08x: in I/O space\n", curr_addr);
				return 0;
			}

			/* Now show the user the return address we found */
			printf("Ret Addr: 0x%08x\n", *(tmp_fp+2));
		}

		/* Now, we want to search for a fullword 0, starting at the */
		/* return address.  Easiest way to do this is to build a */
		/* find command, parse it, then call findit with the */
		/* resulting parse structure.  We have to specify an ending */
		/* address because findit expects one (the actual Find */
		/* routine fakes one up if the user didn't specify it). */
		/* We use the current address plus 0x4000 - it should be */
		/* at least that close... */
		sprintf(cmdstring,"find 00000000 %08x %08x", curr_addr,
			MAX_WITHIN_SEG(curr_addr,0x4000));
		parse_line(cmdstring,&parse_find," ");

		/* Now call findit to do the looking.  If found, display */
		/* enough bytes to ensure the routine name is displayed. */
		/* virt = 1 , always virt mem these days, never real mem */
		/* Stop if any page is paged out along the way */
		if ((int)(found_addr=findit(&parse_find,1,0))==NOTFOUND)
			printf("Tag prefix not found \n");
		else {
			/* display three lines where value was found */
			debug_display(found_addr, 48, 1);
		}
		printf("\n");  /* Leave space between routine name and module */

#if 0
		/* Now, we want to search for the @(#) string, starting at */
		/* the return address.  Once again, we build a find command, */
		/* then call findit, etc., etc.  We use an ending address of */
		/* curr_addr + 0x8000, since the end of the module could be */
		/* a lot farther away. */
		sprintf(cmdstring,"find @(#) %08x %08x", curr_addr,
			MAX_WITHIN_SEG(curr_addr,0x8000));
		parse_line(cmdstring,&parse_find," ");

		/* Now call findit to do the looking.  If found, display */
		/* enough bytes to ensure the what string is displayed. */
		/* virt = 1 , always virt mem these days, never real mem */
		/* Stop if any page is paged out along the way */
		if ((int)(found_addr=findit(&parse_find,1,0))==NOTFOUND)
			printf("What prefix '@(#)' not found \n");
		else {
			/* display seven lines where value was found */
			debug_display(found_addr, 112, 1);
		}
#endif

		/* If this is only the first or second pass, don't advance */
		/* the Frame_ptr, but restore segval if necessary. */
		if (count == 0) {
			/* Replace original segment register value */
			if (uthread_area != 0)
				aim_sr(srval,iar);
			count = 1;
		}
		else if (count == 1) {
			/* Replace original segment register value */
			if (uthread_area != 0)
				aim_sr(srval,lr);
			count = 2;
		}
		else {
			/* Replace original segment register value */
			if (uthread_area != 0)
				aim_sr(srval,Frame_ptr);
			/* get the new code module address and */
			/* the new stack area */
			if (next_module(proctag,&Frame_ptr,&iar,VIRT,First,
					uthread_area)==-1)
			{
				printf("Trace back terminated.\n");
				return 0;
			}
			First = 0;
		}

		printf("Press ENTER to continue or x to exit:\n");
		if (*getterm()=='x')  {
			printf("Trace back terminated.\n");
			return 0;
		}
		printf("\n");

    	} while(Frame_ptr);

	printf("Trace back complete.\n");
	return(0);
}	/* end of stack_traceback() */
#endif /* _THREADS */

