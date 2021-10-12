static char sccsid[] = "@(#)48	1.22  src/bos/kernel/db/POWER/dbbreak.c, sysdb, bos411, 9439C411a 9/29/94 17:44:58";

/*
 * COMPONENT_NAME: (SYSDB) Kernel Debugger
 *
 * FUNCTIONS:	set_breakpoint_traps, set_step_traps, set_this_step,
 *		remove_breakpoint_traps, remove_step_traps, replace_trap
 *		new_step, remove_this_step, is_continue, new_breakpoint,
 *              new_watchpoint, new_brat, set_watch_regs, set_brat_regs,
 *              clear_breakpoint, clear_watch_regs, 
 *		clear_brat_regs, clear_watchpoint, clear_brat,  
 *		remove_break, display_watch_brat_break, display_breakpoints,
 *              is_break, is_break_for_me, new_local_breakpoint,
 *              is_static_break, is_step_for_me, is_step, is_watch_step,
 *		is_brat_step, getaprsegid
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
#include <sys/ppda.h>
#include <sys/seg.h>
#include "vdberr.h"			/* Error message stuff		*/
#include "parse.h"			/* parser structure 		*/
#include "debaddr.h"			/* debugger address structure  	*/
#include "debvars.h"
#include "dbdebug.h"			/* descr struct			*/
#include "dbbreak.h" 			/* constants and structures	*/
#include <sys/systemcfg.h>

/*
 * EXTERNAL PROCEDURES CALLED:
 */

extern int cmderror();
extern clrdsp();
extern debug_xlate();
extern debug_opcode();
extern get_from_memory();
extern write_to_memory();
extern lqra();
extern char *getterm ();

#ifdef _POWER_MP
	/* We need some macros to retrieve the segment register value */
	/* These macros already defined at many places (in .c, not .h) */
#define SREG(x)  ((x)>>SEGSHIFT)        /* Segment reg. #               */
#define SEG(x)   (SRTOSID(debvars[IDSEGS+SREG(x)].hv)) /* seg. id.      */
	/* We need to use mst declared global only in dbdebug.c */
extern struct mstsave *mst;		/* global mst area ptr */
#define CURCPU db_get_processor_num()
#else /* _POWER_MP */
#define CURCPU 0
#endif /* _POWER_MP */

struct steptab stepstoptable = {
	    0,0,0,0,0,0,0,0,
	    0,0,0,0,0,0,0,0,
	    0,0,0,0,0,0,0,0,
	    0,0,0,0,0,0,0,0,
	    0,0,0,0,0,0,0,0,
	    0,0,0,0,0,0,0,0,
#ifdef _POWER_MP
	    0,0,0,0,0,0,0,0,
	    0,0,0,0,0,0,0,0,
#endif /* POWER_MP */
	    0, 
	    -1,0,0,0,0,0,0,0,	
	    0,0,0,0,0,0,0,0,
	    0,0,0,0,0,0,0,0,
	    0,0,0,0,0,0,0,0,0,
   -1,0x13ffc,0x13ffc,0x13ffc,0x13ffc,0x13ffc,0x13ffc,0x13ffc,
   0x13ffc,0x13ffc,0x13ffc,0x13ffc,0x13ffc,0x13ffc,0x13ffc,0x13ffc,
   0x13ffc,0x13ffc,0x13ffc,0x13ffc,0x13ffc,0x13ffc,0x13ffc,0x13ffc,
   0x13ffc,0x13ffc,0x13ffc,0x13ffc,0x13ffc,0x13ffc,0x13ffc,0x13ffc,
	0
	};

struct watch_data watch_data = {
        NULL,NULL,BOTH,WP_OFF
        };

struct brat_data brat_data = {
        NULL,NULL,BP_OFF
        };

#define WRITE 1

/*
 * NAME: dbbreak
 *
 * FUNCTION:
 *
 *   VRM Debugger breakpoint and step functions
 *
 *
 * RETURN VALUE:
 *
 * INTERNAL FUNCTIONS:
 */

ulong getaprsegid();			/* get the appropriate seg id */
uchar is_break();			/* is this addr a breakpoint? */
uchar new_breakpoint();                 /* add a new bp to the table */
uchar new_watchpoint();                 /* set a new wp */
int display_watch_brat_break();         /* display wp, btp, bp's from tables */
int display_breakpoints();		/* display the bp's from the table */
void set_breakpoint_traps();		/* remove entries from break table */
void set_this_step();			/* remove entries from step table */
void set_step_traps();			/* replace instruct with trap */
void remove_breakpoint_traps();		/* restore instruct from break tab */
void remove_step_traps();		/* restore instruct from step tab */
void replace_trap();			/* replace trap with correct inst */
void remove_this_step();		/* free entry in step in use array */
uchar is_continue();			/* is ste_continue true or false */
int clear_breakpoint();			/* remove a breakpoint */
int remove_break();			/* actually deletes the break */
int is_step();				/* is the trap a step ? */
int is_static_break();			/* is trap a static break ? */
#ifdef _POWER_MP
uchar is_step_for_me();                 /* is this step for this debug session */
uchar is_break_for_me();
uchar new_local_breakpoint();
#endif /* POWER_MP */



#ifdef _POWER
ulong	bp=0x7c800008;			/* user breakpoint */
#endif /* _POWER */

					/* FYI - static breakpoint 7c810808 */

/*
* NAME: set_breakpoint_traps
*
* FUNCTION: This function is the driver for moving the trap instruction
*	into memory. The step traps are moved in first then the
*	breakpoint traps are handled.
*
* PARAMETERS:
*	INPUT:	 the 32 bit virt or real address
*	OUTPUT: step traps are set in memory
*
* RETURN VALUE: 0 is always returned to the caller
*
*/

void
set_breakpoint_traps(addr)
ulong 	addr;
{
	register int i,loopin_off;
	ulong addr_segid,bpaddr,t_bit;

	/*
	 *	set the step traps before the break point traps
	 */

#ifdef _POWER_MP
	set_step_traps(addr);		/* set step traps on exit */
#else
	set_step_traps();		/* set step traps on exit */
#endif /* POWER_MP */
	if (loop_count > 0)
	  	loopin_off = 0;			/* looping is active */
	else  {
		if (nbreaks == 0)
	    		return;      	/* no breakpoints so exit routine */
	  	else
	    		loopin_off = 1;		/* start with regular breaks */
	}

#ifdef _POWER
    	t_bit = INSTT_BIT;
#endif /* _POWER */
	/* there are breakpoints to set */
	if (addr != -1) 		/* if address compare */
	     	addr_segid = getaprsegid(addr, t_bit);

	for (i=loopin_off; i<=nbreaks; i++)
	{
		/* Paged-Out flag only valid for current invocation */
		stop_flags(i) &= ~PAGED_OUT;

   	  if((bpaddr=lqra(SRTOSID(stop_segid(i)),SEGOFFSET(stop_addr(i))))==-1)
	    {
		/* error:this address paged out or no such real memory*/
		/* NOTE: Change This Error Msg - it is no longer correct ! */
#if 0
	       	sprintf(errst,"%x",bpaddr);
	       	vdbperr(page_not_in_real1, errst, page_not_in_real2);
#endif
		continue;
	    } 

	   /* else if stop table address is paged in */
	   /* looping */
	    if (((loopin_off==0)&&(i>0)) && (stop_segid(i)==stop_segid(0))
	       && ((SEGOFFSET(stop_addr(i)))==(SEGOFFSET(stop_addr(0))))) {
	        	/* save from loop's save */
		    	stop_save(i) = stop_save(0);
	    }

	    /* not looping */
	    else {
		/* get the 2 or 4 bytes at this addr in real memory */
		pre_get_put_aligned(stop_addr(i),!WRITE,(char *)&stop_save(i),
			stop_segid(i),t_bit,sizeof(INSTSIZ));

	       /*
	        * don't set a breakpoint if it is at the
	        * current address
	        * or if we are looping and this isn't the
		* loop point
	   	*/
	  	if (((addr != -1) &&(stop_segid(i) == addr_segid)
		   	&&(SEGOFFSET(stop_addr(i))==SEGOFFSET(addr)))
		   || ((loopin_off==0) && (i>0)))
		{
	    		continue;
		}

	        /*
		*  set a breakpoint,
		*  write trap to break location
		*/
        	pre_get_put_aligned(stop_addr(i),WRITE,(char *)&bp,
			stop_segid(i),t_bit,sizeof(INSTSIZ));
	    } /* else if (!loopin_off etc.) */

	}  /* for */	
	
	return;
}

/*
* NAME: set_step_traps
*
* FUNCTION: this routine will replace the original instructions with trap
*	instructions. The area table field should be marked in use if a
*	step pt is to be inserted into memory. The step table is intialized
*	to NULLC.
*
* PARAMETERS: all data areas used are global
*	INPUT:	the 32 bit virt or real address (if _POWER_MP only)
*	OUTPUT: step traps are set in memory
*
* RETURN VALUE: 0 is always returned to the caller
*
*/
#ifdef _POWER_MP
void
set_step_traps(addr)
ulong addr;
#else
void
set_step_traps()
#endif /* POWER_MP */
{
	register int i;

	/*
	 * set the step traps in the reverse order from when saved
	 */

#ifdef _POWER_MP
	ulong addr_segid;
	if (addr != -1) 		/* if address compare */
	     	addr_segid = getaprsegid(addr, INSTT_BIT);
#endif /* POWER_MP */
	for (i=LASTSTEP-1; i>=0; i--) {		/* don't use pos 0 in array */
	  	if (step_i(i) == 1)  {		/* go on if step not in use */

			/*
			 * set trap 1 if step 1 valid
			 */

	    		if (step1(i) != -1)	/* step pt active */
#ifdef _POWER_MP
				/*
				 * don't set a step breakpoint if it is at the
				 * current address for an other ctxt than the
				 * current one.
				 */
				if ((addr == -1) || 
				    ((addr != -1) &&
				     ((step_seg1(i) != addr_segid) ||
				     (SEGOFFSET(step1(i))!=SEGOFFSET(addr)) ||
				     ((step_local_mst_addr(i) == (ulong)mst) &&
				     (step_local_mst_sregval(i) == SEG((ulong)mst))))))
					set_this_step((SEGOFFSET(step1(i))),
						      step_seg1(i),&step_save1(i));
#else /* POWER_MP */
	      			set_this_step((SEGOFFSET(step1(i))),
					step_seg1(i),&step_save1(i));
#endif /* POWER_MP */

			/*
			 * set trap 2 if step 2 valid
			 */

	    		if (step2(i) != -1)	/* set secondary step pt */
#ifdef _POWER_MP
				    /*
				     * don't set a step breakpoint if it is at the
				     * current address
				     */
				if ((addr == -1) || 
				    ((addr != -1) &&
				     ((step_seg2(i) != addr_segid) ||
				     (SEGOFFSET(step2(i))!=SEGOFFSET(addr)))))
					set_this_step((SEGOFFSET(step2(i))),
						      step_seg2(i),&step_save2(i));
#else /* POWER_MP */
				set_this_step((SEGOFFSET(step2(i))),
					step_seg2(i),&step_save2(i));
#endif /* POWER_MP */
	  	}
	}
	return;
}

/*
* NAME: set_this_step
*
* FUNCTION: this routine will get the instruction at the address to be
*	stepped to; save this data in the area called save and then write
*	the trap instruction out to memory.
*
* PARAMETERS: all data areas used are global
*	INPUT:	address displacement
*		segment id
*		pointer to the instruction save area in the stepstoptable
*	OUTPUT: step traps are set in memory
*
* RETURN VALUE: 0 is always returned to the caller
*
*/

void
set_this_step(disp,segval,save)
ulong 	disp;				/* displacement */
ulong	segval;				/* segment id */
INSTSIZ	*save;				/* ptr to save area */
{
	ulong	bpaddr;
	INSTSIZ data;			/* temporary data save area */

	/*
	 * check that the address is valid
	 */

	if ((bpaddr=lqra(SRTOSID(segval),disp)) == -1) {
#ifdef this_msg_not_needed
		/* if this addr paged out then we don't need to replace
		 * the step anyway.  If the user just tried to place this
		 * this step then it failed before getting here anyway.
	 	 */
	    	sprintf(errst,"segval:0x%x,displacement:0x%x",segval,disp);
	    	vdbperr(page_not_in_real1, errst, page_not_in_real2);
#endif
		return;
	}
	
  	/* save the contents */
        pre_get_put_aligned(disp,!WRITE,(char *)&data,segval,
			1,sizeof(INSTSIZ));
  	*save = data;

  	/* and set the step */
        pre_get_put_aligned(disp,WRITE,(char *)&bp,segval,
			1,sizeof(INSTSIZ));

	return;
}

/*
* NAME: remove_breakpoint_traps
*
* FUNCTION: remove trap instructions for breakpoints, restoring the
*	    original opcodes. This is done before the step traps are
*	    restored.
*
* PARAMETERS: none
*	INPUT:	none
*	OUTPUT: breakpoint traps are removed and replaced by the orig instr
*
* RETURN VALUE: 0 is always returned to the caller
*
*/

void
remove_breakpoint_traps()
{
	register int i;

	if (loop_count > 0) 		/* remove loop point */
	  	i = 0; 			/* loop active, remove trap */
	else
		i = 1;			/* no loop, keep going */

	for ( ; i<=nbreaks; i++)	 /* replace breaks with orig data */
	{
		/* If the trap can be replaced, it will */
		replace_trap(stop_addr(i),stop_segid(i),
			stop_save(i));

		/* Otherwise, it will be marked paged out */
		if (lqra (SRTOSID(stop_segid(i)), stop_addr(i)) ==
                    -1)
 		{
			/* Not Paged In */
			stop_flags(i) |= PAGED_OUT;
			continue;
		}
		else
		{
			/*
				Only after the trap has been replaced
				can it safely be removed.  If this breakpoint
				has been marked for removal, then remove
				it now.
			*/

			if (stop_flags (i) & CANNOT_CLEAR)
			{
				remove_break (i);
				/*
					Since the step/stop table has
					been closed up, i must be decremented
					in order to avoid skipping an entry.
				*/
				i--;
				continue;
			}
		}
	}
	remove_step_traps();		/* replace the step table traps */
	return;
}

/*
* NAME: remove_step_traps
*
* FUNCTION: remove trap instructions for steps. This must occur after
*	    breakpoint traps are removed.
*
* PARAMETERS: none
*	INPUT:	none
*	OUTPUT: step traps are removed and replaced by the orig instruction
*
* RETURN VALUE: 0 is always returned to the caller
*
*/

void
remove_step_traps()
{
	register int i;

	for (i=0; i<LASTSTEP; i++)  {		/* for each step in use */
	  	if (step_i(i) == 1)  {  	/* if step is inuse  */
	    		if (step1(i) != -1)	/* check the step table addr */
				replace_trap(step1(i),
					step_seg1(i),step_save1(i));
	    		if (step2(i) != -1)
				replace_trap(step2(i),
					step_seg2(i),step_save2(i));
		}
	}
	return;
}

/*
* NAME: replace_trap
*
* FUNCTION: replace a trap at the displacement within the segment
*	    with the data, if the address is valid.
*
* PARAMETERS: none
*	INPUT:	none
*	OUTPUT: traps are removed and replaced by the orig instruction
*
* RETURN VALUE: 0 is always returned to the caller
*
*/

void
replace_trap(disp,segval,data)
ulong	disp;				/* displacement */
ulong  	segval;				/* segment register contents */
INSTSIZ	data;				/* 2 or 4 bytes of original data */
{

	ulong bpaddr;
	INSTSIZ tmpdata;

	/* if paged out, return without replacing */
	if ((bpaddr=lqra(SRTOSID(segval),SEGOFFSET(disp))) == -1)
		return;

	/*
	 * make sure the instruction at the addr is a trap
	 */
        pre_get_put_aligned(disp,!WRITE,(char *)&tmpdata,segval,
			1,sizeof(INSTSIZ));

  	if (tmpdata == BREAKTRAP)
        	pre_get_put_aligned(disp,WRITE,(char *)&data,segval,
			1,sizeof(INSTSIZ));
}

/*
* NAME: new_step
*
* FUNCTION:
*
* PARAMETERS: none
*	INPUT:	none
*	OUTPUT: traps are removed and replaced by the orig instruction
*
* RETURN VALUE: 0 is always returned to the caller
*
*/

uchar
new_step(procede,step_s,step_watch)     /* return a true or false */
uchar procede;				/* 1=step after break 	*/
uchar step_s;				/* 1=execute whole subroutine */
uchar step_watch;                       /* 1=watchpoint step */


{
	register int i;
	uchar flag=TRUE;
	struct descr descr;		/* opcode description structure */
	char temp[20];
	char *p1, *p2;

	/* check to make sure the iar contains a valid instruction */
	if (!debug_opcode(debvars[IDIAR].hv,&descr))  /* possible I-stream */
	  	flag = FALSE;			/* directions */
	else  {
	  	for (i=0; i<LASTSTEP; i++) { /* find a step in table */
	    		if (step_i(i) == 0) 	/* found a step */
	      			break;		/* leave the for loop */
  	    		else if (i >= LASTSTEP)	/* table is full */
      	      			flag = FALSE;
	  	}

	  	step1(i) = (ulong) descr.D_NSI;	/* fall thru address:next-seq-inst */

	  	if (flag) {
	  		/* don't set target address if branch instruction and
				step-around-sub is set */
#ifdef _POWER
			if (descr.D_Brbit) {
				p1 = descr.D_Mnemonic;
				while (*p1 == ' ')   /* skip leading blanks */
					p1++;
				p2 = temp;
				while (*p1 != ' ')   /* copy the instruction */
					if (*p1 == 0)
						break;
					else
						*p2++ = *p1++;
				*p2 = 0;

				if (step_s &&
				    ((strcmp(temp, "bccl") == 0) ||
				     (strcmp(temp, "bcl" ) == 0) ||
				     (strcmp(temp, "bcla") == 0) ||
				     (strcmp(temp, "bcrl") == 0) ||
				     (strcmp(temp, "bl"  ) == 0) ||
				     (strcmp(temp, "bla" ) == 0)))
	     				step2(i) = -1;
				else
	      				step2(i) = (ulong) descr.D_Target;	
			}
			else
	     			step2(i) = -1;

#endif /* _POWER */
#ifdef _POWER
	    		if (DATAT_BIT) {
	    			if (!debug_xlate(step1(i),-1)) 
					flag = FALSE;
				if (step2(i) != -1)
	    				if (!debug_xlate(step2(i),-1)) 
						flag = FALSE;
	    		}
	
	    		if (flag)  {
				step_i(i) = TRUE;	/* mark it in use */
				/* keep going if step-aft-bk */
				step_continue(i) = procede;	
				/* get seg reg */
				step_seg1(i) = getaprsegid(step1(i),DATAT_BIT);
				step_seg2(i) = getaprsegid(step2(i),DATAT_BIT);
#ifdef _POWER_MP
				step_local_mst_addr(i) = (ulong)mst;
				step_local_mst_sregval(i) = SEG((ulong)mst) ;
#endif /* POWER_MP */
	    		}
                        if (step_watch) step_wb(i) = TRUE;  /* mark watch step*/
                        else step_wb(i) = FALSE;
#endif /* _POWER */
          	}
	}
	return(flag);
}		

/*
* NAME: remove_this_step
*
* FUNCTION:
*
* PARAMETERS: none
*	INPUT:	none
*	OUTPUT: the input index into the step_inuse table is set to false
*		meaning that field is now open
*
* RETURN VALUE: no return value
*
*/

void
remove_this_step(i)
int i;					/* location of step in step table */
{
	step_i(i) = FALSE;		/* remove this step */
	step_wb(i) = FALSE;
	return;
}

/*
* NAME: is_continue
*
* FUNCTION:
*
* PARAMETERS: none
*	INPUT:	index into the step_continue table
*	OUTPUT: true or false whether the step is a continue or not. A step
*		continue is an under the covers step, where the debugger
*		doesn't really let the user know he stepped. This is used
*		to step over breakpoints.
*
* RETURN VALUE: true or false
*
*/

uchar
is_continue(i)
int i;			/* location of step in step table */

{
	return(step_continue(i));
}

/*
* NAME: new_breakpoint
*
* FUNCTION: this function will add another entry to the debugger
*		breakpoint table (stepstoptable).
*	    accepts to enter a break at this address if a local breakpoint 
*	    exist.
*
* PARAMETERS: none
*	INPUT:	addr: 32 bit address
*		type: breakpoint or loop point
*		virt: either true or false (virt or real)
*	OUTPUT: the input index into the step_inuse table is set to false
*		meaning that field is now open
*
* RETURN VALUE: no return value
*
*/

uchar
new_breakpoint(addr, type, virt)
ulong addr;
uchar type;
ulong virt;
{
 	uchar 	returnval=TRUE;
	register int 	i,k;
	ulong	addr_segid;

	addr_segid = getaprsegid(addr,virt);	/* get the seg id */
	if (type!=LOOP) {			/* find space the bk table */
	  	for (i=1; i<=nbreaks; i++)  { 	/* check if already in table*/
	    		if ((SEGOFFSET(stop_addr(i))==SEGOFFSET(addr)) &&
#ifdef _POWER_MP
				(! is_local_break(i)) &&
#endif /* POWER_MP */
				(stop_segid(i)==addr_segid))  {
				printf ("breakpoint already set\n");
	      			return(TRUE);	/* addr already in bkpt tab */
	    		}
	  	}
	}

	if ((nbreaks>=LASTSTOP) && (type!=LOOP)) { /* bkpt table is full */
	  	vdbperr(brktablefull);
	  	returnval = FALSE;
	}
	else  {
  	  	if (type!=LOOP)  {		/* increment bkpt total */
	    		nbreaks++;
	    		k = nbreaks;		/* set index variable */
	  	}
	  	else
			/* 0 is index of loop count in bkpt table */
	    		k = 0;			
	
	  	stop_addr(k) = addr;		/* insert addr into table */
	  	stop_segid(k) = addr_segid;	/* insert seg id into tab */
	  	stop_type(k) = type;		/* insert bkpt type in tab */
		stop_flags(k) = 0;		/* reset all flags */
#ifdef _POWER_MP
		stop_local_mst_addr(k) = 0;	/* Mark breakpoint as global */
		stop_local_mst_sregval(k) = 0;
#endif /* POWER_MP */
	}
	return(returnval);
}

/*
* NAME: new_watchpoint
*
* FUNCTION: This function will set a watchpoint in the watchpoint table,
*               watch_data.
*
* PARAMETERS: none
*       INPUT:  addr: 32 bit address
*               virt: either true or false (virt or real)
*       OUTPUT: none
*
* RETURN VALUE: 0 if successful
*               1 if failure
*
*/

#if defined(_POWER_RS2) || defined(_POWER_601)
uchar
new_watchpoint(addr,type)
caddr_t addr;
uchar  type;
{
        uchar   returnval=TRUE;
        ulong   addr_segid;
        ulong   virt=1;

	if (!(__power_rs2() || __power_601())) {
		printf("Watchpoints not supported by hardware\n");
		return(1);
	}
        addr_segid = getaprsegid(addr,virt);    /* get the seg id */

        /* set up data in watchpoint table */
        watch_data.addr = addr;         /* insert addr into table */
        watch_data.segid = addr_segid;  /* insert segreg id into table */
        watch_data.wtype = type;        /* insert wp type into table */
        watch_data.active  = WP_ON;     /* turn wp on */
        return(returnval);
}
#endif /* _POWER_RS2 || _POWER_601 */


/*
* NAME: new_brat
*
* FUNCTION: This function will set a bratpoint in the bratpoint table,
*               watch_data.
*
* PARAMETERS: none
*       INPUT:  addr: 32 bit address
*               virt: either true or false (virt or real)
*       OUTPUT: none
*
* RETURN VALUE: 0 if successful
*               1 if failure
*
*/

#ifdef _POWER_601
uchar
new_brat(addr)
caddr_t addr;

{
        uchar   returnval=TRUE;
        ulong   addr_segid;
        ulong   virt=1;

	if (!__power_601()) {
		printf("Bratpoints not supported by hardware\n");
		return(1);
	}

        addr_segid = getaprsegid(addr,virt);    /* get the seg id */

        /* set up data in bratpoint table */
        brat_data.addr = addr;          /* insert addr into table */
        brat_data.segid = addr_segid;   /* insert segreg id into table */
        brat_data.active  = BP_ON;      /* turn bp on */
        return(returnval);
}
#endif /* _POWER_601 */


/*
* NAME: set_watch_regs
*
* FUNCTION: This function will set a watchpoint in the dabr register.
*
* PARAMETERS: none
*       INPUT:  addr: 32 bit address
*               virt: either true or false (virt or real)
*       OUTPUT: none
*
* RETURN VALUE: 0 if successful
*               1 if failure
*
*/

uchar
set_watch_regs()
{
	int temp;
        uchar   returnval=TRUE;
        if (watch_data.wtype == 'l') {
                /* enable for load accesses only */
        	if ( __power_rs2() )
                        mtdabr( (int) watch_data.addr |  0x1);
       		else
               		if ( __power_601() )
                        	mtdabr601( (int) watch_data.addr |  0x1);
	}
        else
		{
                if (watch_data.wtype == 's') {
                        /* enable for store accesses only */
        		if ( __power_rs2() )
                       		mtdabr( (int) watch_data.addr |  0x2);
       			else if ( __power_601() )
                        	mtdabr601( (int) watch_data.addr |  0x2);
		}
                else
			{
                        /* enable for load and store accesses */
        		if ( __power_rs2() )
                       		mtdabr( (int) watch_data.addr |  0x3);
       			else if ( __power_601() )
                        	mtdabr601( (int) watch_data.addr |  0x3);
		}
	}

        return(returnval);
}


/*
* NAME: set_brat_regs
*
* FUNCTION: This function will set a brathpoint in the hid registers.
*
* PARAMETERS: none
*       INPUT:  addr: 32 bit address
*               virt: either true or false (virt or real)
*       OUTPUT: none
*
* RETURN VALUE: 0 if successful
*               1 if failure
*
*/

uchar
set_brat_regs()
{
	int temp;
        uchar   returnval=TRUE;
	if ( __power_601() ) {
		mthid2(brat_data.addr);
		mthid1(0x70800000);
	}

        return(returnval);
}

/*
/*
* NAME: clear_breakpoint
*
* FUNCTION: clear a breakpoint or clear all breakpoints.
*	    Return a FALSE if an error occurred.
*
* PARAMETERS:
*	INPUT:	ps: a pointer to the parser structure
*	OUTPUT: the input index into the step_inuse table is set to false
*		meaning that field is now open
*
* RETURN VALUE: true or false whether the bkpt was deleted or not
*
*/

int
clear_breakpoint(ps)
struct parse_out *ps;			/* pointer to parser structure */
{
	register int	returnval=TRUE, i;
	uchar 	type;
	ulong	virt, num, num_segid;
	int	found = 0;
#ifdef _POWER_MP
	int	found_a_local =0;
#endif /* POWER_MP */

	/* check for clear all breakpoints "CL *" */
	if ((ps->num_tok >= 1) && (ps->token[1].sv[0] == ASTRISK))
	{
	  	if (ps->num_tok >= 2)		/* "CL * T" or CL * B" */
		{
	    		if (((ps->token[2].sv[0])==CLEARB)||
			    ((ps->token[2].sv[0])==CLEART))
			{
	      			if ((ps->token[2].sv[0]) == CLEARB)
					type = BRKPT;
	      			else
					type = TRACEPT;
	      			for (i=nbreaks; i>=1; i--)
					/* check break type */
	        			if (type==stop_type(i))
	          				returnval = returnval &&
						  remove_break(i);
            		}
			else
			{
				printf ("Third token wrong\n");
				returnval = FALSE;
			}
	  	}
	  	else {			/* user entered "CL *": clear all */
	    		for (i=nbreaks; i>=1; i--)
	      			returnval = returnval && remove_break(i);
	  	}	
	}
	else {
  	  	/* clear by address */
	  	if (ps->num_tok < 1)  {		/* clear by address   	 */
	    		num = debvars[IDIAR].hv;/* default address is the IAR */
#ifdef _POWER
	  		  virt = INSTT_BIT;
#endif /* _POWER */
	  	}
	  	else {
	    		num = ps->token[1].hv; /* get the user addr */
	    		virt = ps->token[1].debaddr->virt;
	  	}
	  	num_segid = getaprsegid(num,virt);  /* get the segment id */
		for (i=nbreaks; i>=1; i--)
		{
			if ((SEGOFFSET(stop_addr(i)) == SEGOFFSET(num)) &&
			    (! (stop_flags (i) & CANNOT_CLEAR)))
				found++;
#ifdef _POWER_MP
			if (is_local_break(i) && 
			    (stop_local_mst_addr(i) == mst) &&
			    (stop_local_mst_sregval(i) == SEG((ulong)mst)))
				found_a_local = TRUE;
#endif /* POWER_MP */
		}

	  	/* find the break address in the table */
	  	for (i=nbreaks; i>=1; i--)
		{
	    		if (SEGOFFSET(stop_addr(i)) == SEGOFFSET(num))
			{
				/* only 1 address found */
				if (found == 1)
					break;
#ifdef _POWER_MP
				/* If several breaks at the same address and
				   a local breakopoint for the current mst
				   found in the table, remove this one
				   without asking questions
				   (kdbx does not like questions !)
				*/
				if (found_a_local && is_local_break(i) &&
				(stop_local_mst_addr(i) == mst) &&
				(stop_local_mst_sregval(i) == SEG((ulong)mst)))
					break;
#endif /* POWER_MP */

				/* several breakpoints at same offset */
				printf ("Seg ID: 0x%08x  Addr: 0x%08x (n)",
					stop_segid(i), stop_addr(i));
				if (*getterm() == 'y')
					break;
			}
		}

	  	if (i>=1)
		{
	    		returnval =  remove_break(i); /* remove the break */
		}
	  	else
		{
			printf ("Breakpoint not in step/stop table\n");
	    		returnval = FALSE; 	/* value not found in table */
	  	}
      	}
	return(returnval);
}

/*
* NAME: clear_watch_regs
*
* FUNCTION: Machine dependent portion of the clear watchpoint function which
*               checks to see which processor the kernel debugger is running
*               on and clears the approriate dabr register/registers.
*
* PARAMETERS: none
*       INPUT:  none
*       OUTPUT: none
*
* RETURN VALUE: 0 if successful
*               1 if failure
*
*/

uchar
clear_watch_regs()
{
        uchar   returnval=TRUE;
        /* Check which processor type kernel debugger is running on */


        if ( __power_rs2() ) {
        	/* set dabr register for rios2 */
        	mtdabr(0);
        }
        else if ( __power_601() ) {
                /* set hid5 register for 601 */
        	mtdabr601(0);
	}
        return(TRUE);
}

/*
* NAME: clear_brat_regs
*
* FUNCTION: Machine dependent portion of the clear watchpoint function which
*               checks to see which processor the kernel debugger is running
*               on and clears the approriate dabr register/registers.
*
* PARAMETERS: none
*       INPUT:  none
*       OUTPUT: none
*
* RETURN VALUE: 0 if successful
*               1 if failure
*
*/

uchar
clear_brat_regs()
{
        uchar   returnval=TRUE;
        /* Check which processor type kernel debugger is running on */


        if ( __power_601() ) {
		/* clear hid1 and hid2 registers for 601 */
		mthid1(0);
		mthid2(0);
        }
        return(TRUE);
}

/*
* NAME: clear_watchpoint
*
* FUNCTION: This function will clear a watchpoint if one is set.
*
* PARAMETERS: none
*       INPUT:  none
*       OUTPUT: flag in watch_data structure is reset.
*
* RETURN VALUE: 0 if successful
*               1 if failure
*
*/

uchar
clear_watchpoint()
{
        uchar   returnval=TRUE;

                watch_data.active  = WP_OFF;    /* turn wp off */
        return(returnval);
}

/*
* NAME: clear_brat
*
* FUNCTION: This function will clear a watchpoint if one is set.
*
* PARAMETERS: none
*       INPUT:  none
*       OUTPUT: flag in watch_data structure is reset.
*
* RETURN VALUE: 0 if successful
*               1 if failure
*
*/

uchar
clear_brat()
{
        uchar   returnval=TRUE;

                brat_data.active  = BP_OFF;     /* turn bp off */
        return(returnval);
}

/*
* NAME: remove_break
*
* FUNCTION: This function will remove a breakpoint from the breakpoint
*		table and then close up the table.
*
* PARAMETERS: i is the index into the bkpt table
*	INPUT:	
*	OUTPUT: the # of breakpoints is decremented by 1 and the entry in
*		the table is removed and the table compressed
*
* RETURN VALUE: true or false whether the breakpoint removed successfully
*		or not
*
*/

int
remove_break(i)
int 	i;				/* location in break table */
{
	ulong bpaddr;

	/* check that breakpoint is in memory */
	if((bpaddr=lqra(SRTOSID(stop_segid(i)),SEGOFFSET(stop_addr(i))))!= -1)
	{
	  	nbreaks--;			/* remove entry; close up tab */
	  	for ( ; i<=nbreaks; i++)	/* close up the bkprt table */
		{
	    		stop_addr(i) = stop_addr(i+1);
	    		stop_segid(i) = stop_segid(i+1);
	    		stop_type(i) = stop_type(i+1);
			stop_flags(i) = stop_flags(i+1);
#ifdef _POWER_MP
			stop_local_mst_addr(i) = stop_local_mst_addr(i+1);
			stop_local_mst_sregval(i) = stop_local_mst_sregval(i+1);
#endif /* POWER_MP */
	  	}
	}
	else					/* breakpoint paged out */
		stop_flags(i) |= CANNOT_CLEAR;

	return(TRUE);
}


/*
* NAME: display_watch_brat_break
*
* FUNCTION: This function will display the contents of the watchpoint table,
*       bratpoint table and breakpoint table.
*
* PARAMETERS: none
*       INPUT:
*       OUTPUT: display of entries in the watchpoint structure,
*               bratpoint structure and stepstoptable.
*
* RETURN VALUE: always return a 0
*
*/

int
display_watch_brat_break()
{

        /* display breakpoints first */
        display_breakpoints();

        /* display watchpoints second */
#if defined(_POWER_RS2) || defined(_POWER_601)
	if (__power_rs2() || __power_601())	/* Only if watches supported */
	        if (watch_data.active == WP_ON) { /* is there a wp active? */
        	        printf("watchpoint:  %08x (srval:%08x) ",
                	        watch_data.addr,watch_data.segid);
			switch(watch_data.wtype) {
				case 'l':
					printf("load\n");
					break;
				case 's':
					printf("store\n");
					break;
				default:
					printf("load/store\n");
			}
		}
#endif /* _POWER_RS2 || POWER_601 */

        /* display bratpoints third */
#if defined(_POWER_601)
	if (__power_601())			/* Only if brats supported */
		if (brat_data.active == BP_ON)  /* is there a bp active? */
			printf("bratpoint:  %08x (srval:%08x)\n",
				brat_data.addr,brat_data.segid);
#endif /* _POWER_601 */
        return(0);
}

/*
* NAME: display_breakpoints
*
* FUNCTION: This function will simply display the contents of the
*	breakpoint table.
*
* PARAMETERS: none
*	INPUT:	
*	OUTPUT: display of all entries in the stepstoptable
*
* RETURN VALUE: always return a 0
*
*/

int
display_breakpoints()
{
	register int i;

	if (nbreaks == 0) 			/* break table empty */
		vdbperr(no_breaks_set);		/* display msg	*/
	else  {
		for (i=1; i<=nbreaks; i++)  {	/* slot 0 not used */
                        printf("%08x (srval:%08x)", stop_addr(i),
				stop_segid(i));
			if (stop_flags(i) & CANNOT_CLEAR)
				printf (" Cleared");
			else if (stop_flags(i) & PAGED_OUT)
				printf (" Paged  ");
			else
				printf ("        ");
			printf("     ");
		  	if ((i%2) == 0) printf("\n");	/* 2 cols per line */
		}
		printf("\n");
	}
	return(0);
}

/*
* NAME: is_break
*
* FUNCTION:
*	This function will check if the address passed in is a breakpoint
*	that is in the breakpoint table. A return code of true or false
*	will be passed back to the calling routine.
*
* PARAMETERS:
*	INPUT:	addr: the 32 bit address
*		virt: virt or real (true or false)
*       srval: (-1 if in debugger, else the one associated)
*	OUTPUT: a true or false value is returned to the caller
*
* RETURN VALUE: true if input addr found in bkpt table
*
*/

uchar
is_break(addr,virt,srval)
ulong addr;
ulong virt;
int srval;
{
	ulong addr_segid;
	ulong cur_addr;
	register int i=0;
	uchar returnval=FALSE;		/* type returned to caller */

	if (srval == FROMDEBVARS)
		addr_segid = getaprsegid(addr,virt); /* get the seg id */
	else{
		cur_addr = (ppda[CURCPU])._csa->prev->iar;
		srval = ((ppda[CURCPU])._csa->prev->as).srval[(cur_addr)>>SEGSHIFT];
		addr_segid = srval & segid_mask;
	}

	if (loop_count==0)  i = 1;	/* LOOP is at position 0 */

	for ( ; i<=nbreaks; i++)  {	/* search stop table */
	  	if ((SEGOFFSET(addr)==SEGOFFSET(stop_addr(i))) &&
	       		(addr_segid==stop_segid(i)))  {
	     		returnval = stop_type(i); /* set the return type */
	     		break;		/* breakpoint FOUND! */
	  	}
	}
	return(returnval);
}


#ifdef _POWER_MP
/*
* NAME: is_break_for_me
*
* FUNCTION:
*	This function will check if the address passed in is a breakpoint
*	that is in the breakpoint table. A return code of true or false
*	will be passed back to the calling routine.
*	checks against locality if the breakpoint is local.
*
* PARAMETERS:
*	INPUT:	addr: the 32 bit address
*		virt: virt or real (true or false)
*	OUTPUT: a true or false value is returned to the caller
*
* RETURN VALUE: true if input addr found in bkpt table
*
*/

uchar
is_break_for_me(addr,virt)
ulong addr;
ulong virt;
{
	ulong addr_segid;
	register int i=0;
	uchar returnval=FALSE;		/* type returned to caller */

	addr_segid = getaprsegid(addr,virt); /* get the seg id */
	if (loop_count==0)  i = 1;	/* LOOP is at position 0 */

	for ( ; i<=nbreaks; i++)  {	/* search stop table */
	  	if ((SEGOFFSET(addr)==SEGOFFSET(stop_addr(i))) &&
	       		(addr_segid==stop_segid(i)))  {
			if (is_local_break(i)) {
				/* Is this local breakpoint for the cur. mst */
				if ((stop_local_mst_addr(i) == mst) &&
				(stop_local_mst_sregval(i) == SEG((ulong)mst))){
				returnval = stop_type(i); /* set the return type */
				break;		/* breakpoint FOUND! */
				} else {
					/* Local breakpoint for an other
					   thread: continue the search */
				}
			} else {
				/* Normal breakpoint */
				returnval = stop_type(i); /* set the return type */
				break;		/* breakpoint FOUND! */
			}
	  	}
	}
	return(returnval);
}

/*
* NAME: new_local_breakpoint
   Same as new_breakpoint, except that there is no type parameter.
   Accepts to set a local breakpoint, even if a breakpoint already exists
   at this address (global or for an other context).
*
*
* PARAMETERS: none
*	INPUT:	addr: 32 bit address
*		type: breakpoint or loop point
*		virt: either true or false (virt or real)
*	OUTPUT: the input index into the step_inuse table is set to false
*		meaning that field is now open
*
* RETURN VALUE: no return value
*
*/
uchar
new_local_breakpoint(addr, virt)
ulong addr;
ulong virt;
{
 	uchar 	returnval=TRUE;
	register int 	i,k;
	ulong	addr_segid;

	addr_segid = getaprsegid(addr,virt);	/* get the seg id */
	for (i=1; i<=nbreaks; i++)  { 	/* check if already in table*/
		if ((SEGOFFSET(stop_addr(i))==SEGOFFSET(addr)) &&
			(stop_segid(i)==addr_segid) &&
			(stop_local_mst_addr(i) == mst) &&
			(stop_local_mst_sregval(i) == SEG((ulong)mst)))  {
			printf ("breakpoint already set\n");
			return(TRUE);	/* addr already in bkpt tab */
		}
	}

	if (nbreaks>=LASTSTOP) { /* bkpt table is full */
	  	vdbperr(brktablefull);
	  	returnval = FALSE;
	}
	else  {
		/* increment bkpt total */
		nbreaks++;
		k = nbreaks;		/* set index variable */
	
	  	stop_addr(k) = addr;		/* insert addr into table */
	  	stop_segid(k) = addr_segid;	/* insert seg id into tab */
	  	stop_type(k) = BRKPT;		/* insert bkpt type in tab */
		stop_flags(k) = 0;		/* reset all flags */
		stop_local_mst_addr(k) = mst;	/* Set locality information */
		stop_local_mst_sregval(k) = SEG((ulong)mst);
	}
	return(returnval);
}

#endif /* POWER_MP */

/*
* NAME: is_static_break
*
* FUNCTION: this function will check if the address passed in is a
*	static breakpoint. Compare the contents of the address with
*	a static breakpoint.
*
* PARAMETERS:
*	INPUT:	addr: the 32 bit address
*		virt: virt or real (true or false)
*	OUTPUT: a true or false value is returned to the caller
*
* RETURN VALUE: true if address is a static bkpt
*
*/

int
is_static_break(addr,virt)
caddr_t	addr;
int	virt;
{
	INSTSIZ data;
	int returnval=FALSE;

	if (get_from_memory(addr,virt,&data,sizeof(data))) {
	  	if (data==STATIC_BREAK_TRAP)	
	    		returnval = TRUE;
	  	else
	    		returnval = FALSE;
	}
	else  {
        	sprintf(errst,"%x",addr);
          	vdbperr(page_not_in_real1, errst, page_not_in_real2);
	}
	return(returnval);
}

#ifdef _POWER_MP
/*
* NAME: is_step_for_me
*
* FUNCTION: This function checks the address against the step_trap table
*	to see if the address is a step trap. The Step_id is returned
*	to the caller. It checks against locality.
*
* PARAMETERS:
*	INPUT:	addr: the 32 bit address
*		virt: virt or real (true or false)
*		step_id:
*		cont:
*	OUTPUT:
*
* RETURN VALUE: index into bkpt table is returned via step_id and returnval
*		either returns a true or false if the addr is a step pt
*
*/

uchar
is_step_for_me(addr,virt,step_id)
ulong	addr;
ulong	virt;
int 	*step_id;
{
	int returnval=FALSE;
	ulong addr_segid;
	register int i;

	addr_segid = getaprsegid(addr, virt);	/* get the IAR's seg id */
	/* look for this address in the step table */
	for (i=0; i<LASTSTEP; i++) {
	  	if ((step_i(i)==TRUE) &&		/* stepinuse bit set? */
	  	    (((SEGOFFSET(addr)==SEGOFFSET(step1(i)))&&
		    (addr_segid==step_seg1(i)))||
		    ((SEGOFFSET(addr)==SEGOFFSET(step2(i)))&&
		    (addr_segid==step_seg2(i))))
		    && (step_local_mst_addr(i) == mst)
		    && (step_local_mst_sregval(i) == SEG((ulong)mst))
	    ) {
 			returnval = TRUE;
			*step_id = i;		/* loc in the bkpt tab */
			break;			/* address FOUND! */
	  	}
	}
	return(returnval);
}
#endif /*  _POWER_MP */

/*
* NAME: is_step
*
* FUNCTION: This function checks the address against the step_trap table
*	to see if the address is a step trap for an other debug session.
*
* PARAMETERS:
*	INPUT:	addr: the 32 bit address
*		virt: virt or real (true or false)
*		step_id:
*       srval: (-1 if in debugger, else the one to take into account)
*
*	OUTPUT:
*
* RETURN VALUE: index into bkpt table is returned via step_id and returnval
*		either returns a true or false if the addr is a step pt
*
*/

int
is_step(addr,virt,step_id,srval)
ulong	addr;
ulong	virt;
int	*step_id;
int srval;
{
	int returnval=FALSE;
	ulong addr_segid;
	ulong cur_addr;
	register int i;

	if (srval == FROMDEBVARS)
		addr_segid = getaprsegid(addr, virt);	/* get the IAR's seg id */
	else{
		cur_addr = (ppda[CURCPU])._csa->prev->iar;
		srval = ((ppda[CURCPU])._csa->prev->as).srval[(cur_addr)>>SEGSHIFT];
		addr_segid = srval & segid_mask;
	}

	/* look for this address in the step table */
	for (i=0; i<LASTSTEP; i++) {
	  	if ((step_i(i)==TRUE) &&		/* stepinuse bit set? */
	  	    (((SEGOFFSET(addr)==SEGOFFSET(step1(i)))&&
		    (addr_segid==step_seg1(i)))||
		    ((SEGOFFSET(addr)==SEGOFFSET(step2(i)))&&
		    (addr_segid==step_seg2(i))))
		    ) {
 			returnval = TRUE;
			*step_id = i;		/* loc in the bkpt tab */
			break;			/* address FOUND! */
	  	}
	}
	return(returnval);
}

/*
* NAME: is_watch_step
*
* FUNCTION: This function checks the address against the step_trap table
*       to see if the address is a step trap. The Step_id is returned
*       to the caller
*
* PARAMETERS:
*       INPUT:  addr: the 32 bit address
*               virt: virt or real (true or false)
*               step_id:
*               cont:
*       OUTPUT:
*
* RETURN VALUE: index into bkpt table is returned via step_id and returnval
*               either returns a true or false if the addr is a step pt
*
*/

int
is_watch_step(step_id)
int     *step_id;
{
        /* check if this is a watch type of step */
	if (step_wb(*step_id) == 1)
		return(TRUE);
	else
		return(FALSE);
}


/*
* NAME: is_brat_step
*
* FUNCTION: This function checks the address against the step_trap table
*       to see if the address is a step trap. The Step_id is returned
*       to the caller
*
* PARAMETERS:
*       INPUT:  addr: the 32 bit address
*               virt: virt or real (true or false)
*               step_id:
*               cont:
*       OUTPUT:
*
* RETURN VALUE: index into bkpt table is returned via step_id and returnval
*               either returns a true or false if the addr is a step pt
*
*/

int
is_brat_step(step_id)
int     *step_id;
{
        /* check if this is a watch type of step */
	if (step_wb(*step_id) == 2)
		return(TRUE);
	else
		return(FALSE);
}


/*
* NAME: getaprsegid
*
* FUNCTION: This function will return the 32 bit contents of the segment
*	register. If a virtual address is passed then get the segment
*	register number and then bitwise and it with the segment id mask.
*
* PARAMETERS:
*	INPUT:	addr: the 32 bit address
*		virt: virt or real (true or false)
*	OUTPUT:
*
* RETURN VALUE: 24-bit segment id
*
*/

ulong
getaprsegid(addr,virt)
ulong	addr;				/* raw 32-bit address */
ulong 	virt;				/* virt or real mode */
{
	ulong	addr_segid;		/* contents of seg reg */

	if (virt) 			/* get from seg reg */
	 	addr_segid = debvars[IDSEGS+(addr>>SEGSHIFT)].hv;
 	else 
		addr_segid = KERNELSEGVAL;

	return(addr_segid & segid_mask);

}
