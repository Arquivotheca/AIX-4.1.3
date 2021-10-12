static char sccsid[] = "@(#)53  1.40.1.11  src/bos/kernel/db/POWER/dbdriver.c, sysdb, bos41J, 9508A 2/22/95 10:12:49";

/*
 * COMPONENT_NAME: (SYSDB) Kernel Debugger
 *
 * FUNCTIONS:	driver, Help, Back, Clear, Display, Bdisplay, Find, Fmtu, Fmtut,
 *		Iorfunc, Iowfunc, Brat, Break, Go, Loop, Next, Origin, Quit,
 *		Restore, Screen, Sregs, fpregs, Set, St, Stc, Sth, Swap,
 *		Proc, Thread, Step, Watch, Xlate, quitrtn, bpr, Cpu,
 *		Switch, LBreak, Udisplay, Reason
 *
 * ORIGINS: 27 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
*/

#include <sys/types.h>
#include <sys/seg.h>
#include <sys/ppda.h>
#include "parse.h"
#include "vdberr.h"
#include "debvars.h"
#include "debaddr.h"
#include "dbdebug.h"
#include "dbbreak.h"
#include "vdbfindm.h"		/* necessary for findit() */
#include <sys/systemcfg.h>
#include <sys/iplcb.h>

#define SREG(x)  ((x)>>SEGSHIFT)        /* Segment reg. #               */
#define SEG(x)   (SRTOSID(debvars[IDSEGS+SREG(x)].hv)) /* seg. id.      */
#define is_alpha(c)  ((((c) >= 'a') && ((c) <= 'z')) ||\
		     (((c) >= 'A') && ((c) <= 'Z')))
#define is_digit(c)  (((c) >= '0') && ((c) <= '9'))


/*
* EXTERNAL PROCEDURES CALLED:
*/

extern  int d_ttyopen(),d_ttyclose();
extern	void	fmtu();
#ifdef _THREADS
extern	void	fmtut();
extern int pr_thread();
#endif /* THREADS */
extern int get_put_aligned();
extern char *getterm();

#ifdef _POWER_MP
extern int db_resume_others();
#endif


/*
* EXTERNAL VARIABLES:
*/

extern struct debaddr addr;
extern struct func func[];
extern int step_s1, restore_screen;
extern char *in_string;
extern int save_reason,save_ext_arg;
#ifdef _POWER_MP
#ifndef _THREADS
extern struct user u;
#endif /* THREADS */
#endif /* POWER_MP */
int dbkdbx = 0;

/*
* NAME: driver
*
* FUNCTION: driver is the routine that calls all the commands that can
*	be executed by the debugger. 
*	The index into the command array of structures (func) is passed
*	to driver which then uses this index to point to the function that
*	will do the work. Sometimes the function is in this file and other
*	times the function will be found in other files. I did this to 
*	eliminate unnecessary functions. 
*
* PARAMETERS: driver is called from the mainline debugger program (debug.c).
*	The parameters are: 
*		j: an index in the command array of structures, this index
*			indicates the command to be executed.
*		ps: a pointer to the parser structure which most commands
*			need.
* RETURN VALUE DESCRIPTION: a return code of 0 will be returned from every
*   	function called by driver except "QUIT" which will return a 1 if
*	the user wants to exit the debugger.
*/

#define vbit(i)	ps->token[i].debaddr->virt
#ifdef _POWER_MP
extern int step_processing_level;
extern int switch_ctxt, switch_ctxt_sr1, switch_ctxt_sr2 ,switch_ctxt_sr13;

extern long debugger_lock;                      /* to protect db_main */
 
extern status_t status[];                /* Set by each cpu when changing its own state
					    used to display the status, and to avoid
					    spurious MPC_stop sendings
					  */

extern action_t action[];                /* Only modified in db_main, under protection
					    of debugger_lock. Used by the cpu in
					    debugging state to control others (stop, resume)
					  */

extern int selected_cpu;                 /* set by the cpu command */

#endif /* POWER_MP */

int
driver(j,ps)
register int	j;		/* index into the structure */
struct parse_out *ps;		/* parser structure */

{
	if (func[j].psflag == 0)		/* don't pass parser struct */
		return((*func[j].action)());	/* calls the correct function */
	else
		return((*func[j].action)(ps));	/* calls the correct function */
}

/*
* NAME: Help
*
* FUNCTION: this routine will print out the list of available commands.
*	Simply index thru the command array and print out the first field.
*	The list of commands can be found in the first field of the func
*	structure found in file func.h. Help is the default first command.
*
* RETURN VALUE DESCRIPTION: always 0
*/

Help()				/* request for help .. ? .. */
{
	register int i=0,j;

	while (1) {
		for (j=0; j<=20; i++) {		/* 20 lines per screen */
			if (*func[i].label == ' ')  /* until end of file */
				return(0);
			if (! func[i].text)
				continue;
			printf("%s ... %s \n",func[i].label,
				func[i].text);	/*display label */
			++j;
		}

		if (debpg() == FALSE)
			return(0);
		if (*func[i].label == ' ')
			i = 0;
	}
/*NOTREACHED*/
	return(0);
}

/*
* NAME: Back
*
* FUNCTION: decrement the IAR by the value entered by the user or default
*	of 4 bytes.
*
* RETURN VALUE DESCRIPTION: always 0
*/

Back(ps)				/* back 4 or 2 */
struct parse_out *ps;
{
	if (ps->num_tok<1) { 		/* no value indicated by user */
		debvars[IDIAR].hv = debvars[IDIAR].hv - 4; /* default */
		debug_screen(ps, DEFLTSCR);
	}
	else
		if (ps->token[1].tflags & HEX_OVERFLOW)
			printf("Argument too large -- overflow occurred\n");
		else {
			debvars[IDIAR].hv = debvars[IDIAR].hv - ps->token[1].hv;
			ps->num_tok = 0;	/* to avoid conflict when */
						/* calling debug_screen	*/
			debug_screen(ps, DEFLTSCR);
		}
	return(0);
}

/*
* NAME: Clear
*
* FUNCTION: call the function to clear breakpoints pass the parser structure
*	as a parameter. clear_breakpoint can't be called directly because it
*	doesn't always return a 0 return code, which is what must be returned
*	in this instance.
*
* RETURN VALUE DESCRIPTION: always 0
*/

Clear(ps)				/* clear hex addr or * */
struct parse_out *ps;
{

        if ((ps -> num_tok == 1) && (!strcmp(ps->token[1].sv,"w") ||
                !strcmp(ps->token[1].sv,"watch" )))
        {
                clear_watchpoint();             /* clear watchpoint*/
                clear_watch_regs();
                return 0;
        }
        if ((ps -> num_tok == 1) && (!strcmp(ps->token[1].sv,"brat")))
        {
                clear_brat();
                clear_brat_regs();
                return 0;
        }

        if ((ps -> num_tok == 1) && (ps->token[1].tflags & HEX_OVERFLOW))
        {
                printf("Number too large\n");
                return 0;
        }
        else
        {
                clear_breakpoint(ps);           /* clear breaks */
        }

        return(0);

}

#ifdef _POWER_MP

/*
 * NAME: Cpu
 *
 * FUNCTION: this routine to exchange the debugging cpu with a stopped one
 *	
 *	
 *	
 *
 * RETURN VALUE DESCRIPTION: 0 is always returned
 */

Cpu(ps)
	struct parse_out *ps;
{
	int target_cpu;
	int i;
	char buf[80];
	
	if (__power_mp()){
		
		if (ps->num_tok < 1) { /* print status of all valid cpus */
			printf("  CPU         STATUS        ACTION  \n");
			for (i=0;i<number_of_cpus;i++){
				switch (status[i]){
				case running : sprintf(buf,"   %d %16s ",i, "Running");
					break;
				case stopped : sprintf(buf,"   %d %16s ",i, "Stopped");
					break;
				case debug_waiting : sprintf(buf,"   %d %16s ",i, "Debug_waiting");
					break;
				case debugging : sprintf(buf,"   %d %16s ",i, "Debugging");
					break;
				}
				switch (action[i]){
				case NONE : printf("%s %s\n",buf, "        NONE");
					break;
				case stop : printf("%s %s\n",buf, "        Stop");
					break;
				case debug : printf("%s %s\n",buf, "        Debug");
					break;
				case resume : printf("%s %s\n",buf, "        Resume");
					break;
				}
			}
			return 0;	
		}
		else if (ps->token[1].hv>=0){
			target_cpu =  ps->token[1].hv; /* specified cpu */
			if (target_cpu == cpunb)
				return 0;
			if ((target_cpu >= number_of_cpus) ||  
			    ((status[target_cpu] != stopped) && (status[target_cpu] != debug_waiting))){
				printf("This cpu is not available yet \n");
				return 0;
			}
			action[cpunb] = stop;
			action[target_cpu] = debug;
			selected_cpu = target_cpu;
		}
		return 1;
	}else 
		if (ps->num_tok < 1) { /* print status of all valid cpus */
			printf("  CPU         STATUS        ACTION  \n");
			sprintf(buf,"   %d %16s ",0 , "Debugging");
			printf("%s %s\n",buf, "        NONE");
		return 0;
		}
		else 
			if (ps->token[1].hv != 0){
				printf("This cpu is not available yet \n");
			}
	return 0;
}				


/*
 * NAME: Ppd
 *
 * FUNCTION: this routine displays the ppda structure
 *	    if no argument, the current one, else the specified
 *	
 *	
 *
 * RETURN VALUE DESCRIPTION: 0 is always returned
 */

Ppd(ps)
	struct parse_out *ps;
{
	int  target_cpu;
	
	if (__power_mp()){
		
		if (ps->num_tok < 1)  /* print ppda of current processor */
			target_cpu = cpunb;
		else if (ps->token[1].hv>=0)
			target_cpu =  ps->token[1].hv; /* specified cpu */
		if (target_cpu >= number_of_cpus) {
			printf("This cpu is not available yet \n");
			return 0;
		}
	}
	else 
	{
		if (ps->num_tok < 1)  /* print ppda of current processor */
			target_cpu = cpunb;
		else 
			if (ps->token[1].hv>=0)
				target_cpu =  ps->token[1].hv; /* specified cpu */
		if (target_cpu > 0) {
			printf("This cpu is not available yet \n");
			return 0;
		}
	}	
	
	disp_ppda(target_cpu);
	return 0;
}


/*
 * NAME: Switch
 *
 * FUNCTION: this routine will call the function to display the 
 *	address of the mst associated to a process, and updates
 *       three data to the value of SR1 SR2 and SR13 of this mst.
 *
 * RETURN VALUE DESCRIPTION: 0 is always returned
 */

Switch(ps)
	struct parse_out *ps;
{
#ifdef _THREADS
	int ad_mst;
#endif /* THREADS */
	if (dbkdbx){
		if (ps->num_tok > 0){
#ifdef _THREADS
			if ((ad_mst = Switch_ctxt(ps)) == -1)
#else
				if (Switch_ctxt(ps) == -1)
#endif /* THREADS */
					printf("0\n");
				else
#ifdef _THREADS
					printf("%08x\n",ad_mst);
#else 
			printf("%08x\n",&u); 
#endif /* THREADS */
			
		}
		else{
			switch_ctxt = FALSE;
			switch_ctxt_sr1 = 0;
			switch_ctxt_sr2 = 0;
			switch_ctxt_sr13 =0;
#ifdef _THREADS
			printf("%08x\n",ad_mst);
#else 
			printf("%08x\n",&u); 
#endif /* THREADS */
		}
	}
	else
		printf("0\n");
	return(0);
}
/*
 * NAME: LBreak
 *
 * FUNCTION: determine the address where it should be set and then call
 *	the routine to set the trap so the system will trap at the specified
 *	address 
 *	Only valid if dbkdbx is set
 *
 * RETURN VALUE DESCRIPTION: always 0
 */

LBreak(ps)
	struct parse_out *ps;
{
	ulong	num;
	ulong	virt;
	
	/* Feature for kdbx, if operator is not kdbx, just return */
	if (!dbkdbx) return(0);
	
	if (ps->num_tok >= 1) {			/* user specified bkpt */
		if (ps->token[1].tflags & HEX_OVERFLOW) {
			printf("Address too large\n");
			return(0);
		}
		
		num = ps->token[1].hv;		/* may be virt or real */
		virt = vbit(1);
	}
	else {					/* default to break at iar */
		num = debvars[IDIAR].hv;
		virt = INSTT_BIT;
	}
	
	if (lqra(SRTOSID(getaprsegid (num,virt)), SEGOFFSET(num)) == -1)
	{
		printf ("Address 0x%08x (srval: 0x%08x) ", num,
			getaprsegid (num,virt));
		printf ("is not currently accessible.\n");
		printf ("Cannot set the breakpoint.\n");
	}
	else
		new_local_breakpoint(num,virt);	/* set new breakpoint */
	return(0);
}


#endif  /* POWER_MP */
 
/*
* NAME: Display
*
* FUNCTION: call the function to clear breakpoints pass the parser structure
*	as a parameter.
*
* RETURN VALUE DESCRIPTION: always 0
*/

Display(ps)				/* Display <hex addr> <hex length> */
struct parse_out *ps;
{
	ulong	num;
	int j;

	/* Check supplied arguments for hex overflow */
	for (j = 0; j < ps -> num_tok; j++)
		if (ps -> token[j + 1].tflags & HEX_OVERFLOW)
		{
			printf ("Argument %s too large\n",
				ps -> token[j + 1].sv);
			return 0;
		}

	if (ps->num_tok > 1)
		num = ps->token[2].hv;
	else
		num = 16;
	debug_display(ps->token[1].hv, num, vbit(1));
	return(0); 
}
 

/*
* NAME: Bdisplay
*
* FUNCTION: call the function to clear breakpoints pass the parser structure
*	as a parameter.
*
* RETURN VALUE DESCRIPTION: always 0
*/

Bdisplay(ps)				/* Display <hex addr> <hex length> */
struct parse_out *ps;
{
	ulong	num;
	int j;

	if (!dbkdbx)
		return 0;

#ifdef _POWER_MP
	if (switch_ctxt != 0){
		switch_ctxt_sr1 = aim_sr(switch_ctxt_sr1, 0x10000000);
		switch_ctxt_sr2 = aim_sr(switch_ctxt_sr2, 0x20000000);
		switch_ctxt_sr13 = aim_sr( switch_ctxt_sr13, 0xD0000000);
	}
#endif /* POWER_MP */

	/* Check supplied arguments for hex overflow */
	for (j = 0; j < ps -> num_tok; j++)
		if (ps -> token[j + 1].tflags & HEX_OVERFLOW)
		{
#ifdef _SNOOPY
			if (__snoopy())
				d_ttyput(0);
			else
#endif /* SNOOPY */
			d_ttybinput (0);
			return 0;
		}

	if (ps->num_tok > 1)
		num = ps->token[2].hv;
	else
		num = 16;
	debug_bin_disp(ps->token[1].hv, num, vbit(1));
#ifdef _POWER_MP
	if (switch_ctxt != 0){
		switch_ctxt_sr1 = aim_sr(switch_ctxt_sr1, 0x10000000);
		switch_ctxt_sr2 = aim_sr(switch_ctxt_sr2, 0x20000000);
		switch_ctxt_sr13 = aim_sr(switch_ctxt_sr13, 0xD0000000);
	}
#endif /* POWER_MP */
	return(0); 
}

/*
* NAME: Find
*
* FUNCTION: call the find function and pass the parser structure
*	as a parameter.
*
* RETURN VALUE DESCRIPTION: always 0
*/

Find(ps)					/* memory search */
struct parse_out *ps;
{
	static struct parse_out old_ps;
	static int first_time = TRUE;
	caddr_t addr;
	int i;
	struct token_def fx;

	/* If any of the parameters are "*" then we need to reuse the	*/
	/* last value in that slot					*/

	for (i = 1; i < MAXTOKENS; i++)
		if (strcmp(ps->token[i].sv,"*") == 0) 
			ps->token[i] = old_ps.token[i];

	/* check to make sure that there is something to search for,	*/
	/* if not, display the syntax of the command.			*/

	if (ps->num_tok < 1) {
	    printf("usage: find <value> <start addr> <end addr> [1,2 or 4]\n");
	    return(0);		/* bail out right now!	*/
	}

	/* check to see if there is a starting address -- if not,	*/
	/* start at 0.  (Warning: this doesn't set up the entire	*/
	/* structure, but it sets up enough for findit to work)		*/

	if (ps->num_tok < 2) {
		ps->token[2].tflags = HEXDEC;
		ps->token[2].hv = 0;
		ps->num_tok++;
	}


	/* check to see if there is a ending address -- if not, search	*/
	/* to the end of the segment that contains the starting address.*/
	/* (Warning: this doesn't set up the entire structure, but it 	*/
	/* sets up enough for findit to work)				*/

	if (ps->num_tok < 3) {
		ps->token[3].tflags = HEXDEC;
		ps->token[3].hv = (ps->token[2].hv & 0xf0000000) | 0x0fffffff;
		ps->num_tok++;
	}

	/* check to make sure none of the arguments overflowed		*/
	if ((ps->token[2].tflags & HEX_OVERFLOW) ||
	    (ps->token[3].tflags & HEX_OVERFLOW)) {
		printf("Number too large\n");
		return 0;
	}

	/* save this set of tokens in case they need to be used next time  */

	old_ps = *ps;
	first_time = FALSE;

	/* vit = 1 , always virt mem these days, never real mem */
	if ((int)(addr=findit(ps,1,10 /*NOPAGELIMIT*/))==NOTFOUND) 
		printf("Value not found \n");
	else {
		/* display the line where value was found */
		debug_display(addr, 16, 1);

		/* save the location in the variable "fx"	*/

		fx.tflags = HEXDEC;
		fx.hv = (ulong) addr;
		fx.dv = (ulong) addr;
		fx.sv[0] = 0;
		add_var("fx",&fx);
	}

	return(0);
}

/*
* NAME: Fmtu
*
* FUNCTION: call the formatted user area function and pass the 
*	parser structure as a parameter. Fmtu can't be called directly in this
*	case because it doesn't always return a 0, which it must in this case.
*
* RETURN VALUE DESCRIPTION: always 0
*/

Fmtu(ps)
struct parse_out *ps;
{
	if ((ps -> num_tok >= 1) && (ps->token[1].tflags & HEX_OVERFLOW))
	{
		printf("Number too large\n");
		return 0;
	}

	fmtu(ps);
	return(0);
}

#ifdef _THREADS
/*
* NAME: Fmtut
*
* FUNCTION: call the formatted uthread area function and pass the
*	parser structure as a parameter. Fmtut can't be called directly in this
*	case because it doesn't always return a 0, which it must in this case.
*
* RETURN VALUE DESCRIPTION: always 0
*/

Fmtut(ps)
struct parse_out *ps;
{
	if ((ps -> num_tok == 1) && (ps->token[1].tflags & HEX_OVERFLOW))
	{
		printf("Number too large\n");
		return 0;
	}

	fmtut(ps);
	return(0);
}
#endif /* _THREADS */

/*
* NAME: Brat
*
* FUNCTION: determine the type of bratpoint to be set and the address
*       where it should be set and then call the routine to set the
*       trap so the system will trap at the specified address
*
* RETURN VALUE DESCRIPTION: always 0
*/

Brat(ps)
struct parse_out *ps;
{
        caddr_t e_addr;
        if (ps->num_tok >= 1) {
                if (ps->token[1].tflags & HEX_OVERFLOW) {
                        printf("Address too large\n");
                        return(0);
                }
        }
        else {
                printf("Specify address\n");
                return(0);
        }
        e_addr = ps->token[1].hv;       /* may be virt or real */
        new_brat(e_addr);
        return(0);
}


/*
* NAME: Break
*
* FUNCTION: determine the type of breakpoint to be set and the address
*	where it should be set and then call the routine to set the 
*	trap so the system will trap at the specified address 
*
* RETURN VALUE DESCRIPTION: always 0
*/

Break(ps)
struct parse_out *ps;
{
	ulong	num;
	extern 	int 	brk_type;
	ulong	virt;

	brk_type = BRKPT;			/* vanilla breakpt */
	if (ps->num_tok >= 1) {			/* user specified bkpt */
		if (ps->token[1].tflags & HEX_OVERFLOW) {
			printf("Address too large\n");
			return(0);
		}

		num = ps->token[1].hv;		/* may be virt or real */
		if (ps->token[2].hv >= 2) {
			if (ps->token[2].sv[0] == CLEART)
				brk_type = TRACEPT;	/* trace entry */
		}
		virt = vbit(1);
	}
	else {					/* default to break at iar */
		num = debvars[IDIAR].hv;
#ifdef _POWER
		virt = INSTT_BIT;
#endif
	}

	if (lqra(SRTOSID(getaprsegid (num,virt)), SEGOFFSET(num)) == -1)
	{
		printf ("Address 0x%08x (srval: 0x%08x) ", num,
			getaprsegid (num,virt));
		printf ("is not currently accessible.\n");
		printf ("Cannot set the breakpoint.\n");
	}
	else
		new_breakpoint(num,brk_type,virt);	/* set new breakpoint */
	return(0);
}

/*
* NAME: Go
*
* FUNCTION: restart program execution from the iar (default) or a specified
*	address. Check if the address is a breakpoint, if it is then step 
*	over it before resuming execution.

* RETURN VALUE DESCRIPTION: always 0
*/

Go(ps)
struct parse_out *ps;
{
int retcode=1, startflag=0;

	/*  Input can now be of the form "go [addr] [dump]"	*
	 *  If address is entered , startflag is set to 1.      *
	 *  If keyword dump entered,retcode is set to DUMPRET.	*/

	if(ps->num_tok >= 1)
		{
		if(ps->num_tok >= 2)
			{
			startflag=1;
			if(!strcmp(ps->token[2].sv,"dump"))
				retcode=DUMPRET;
			}
		else if (!strcmp(ps->token[1].sv,"dump"))
			retcode=DUMPRET;
		     else
			startflag=1;
		}

#if 0
printf(" %d      %d \n",startflag,retcode);
#endif

	if (startflag) {			 /* user defined start pt */

                /* Check for valid entry of start pt. HXV|ADV */
		/* Functionality of diagnose() for Go moved here to add the dump param */
		if ( ! ((ps->token[1].tflags & HEX_VALID) &&
		        !(ps->token[1].tflags & HEX_OVERFLOW) && get_addr(1,ps)) )
			{
			printf("You entered a parameter %s that is not valid.\n",ps->token[1].sv);
			return 0;
			}
		
		if ((ps->num_tok == 1) && (ps->token[1].tflags & HEX_OVERFLOW))
		{
			printf("Number too large\n");
			return 0;
		}
		debvars[IDIAR].hv = ps->token[1].hv;
	}
	
       	else if (is_static_break(debvars[IDIAR].hv,T_BIT)) {
		debvars[IDIAR].hv += sizeof(INSTSIZ);	/* around static trap*/
	}
	else if (is_break(debvars[IDIAR].hv,T_BIT,FROMDEBVARS)) { /* step gingerly over */
		if (new_step(TRUE,step_s1,FALSE))      /* brkpt at cur instr */
#ifdef _POWER_MP        /* have to prevent from resume stopped cpus */
			{
				if (__power_mp())
					if (step_processing_level++ == 0)
						hold_cpus_while_stepping();
				return(retcode);
			}
#else /* _POWER_MP */
			return(retcode);		/* step 1 if possible */
#endif /* _POWER_MP */
		vdbperr(cant_step_or_go);
	}
#ifdef _POWER_MP        /* have to resume stopped cpus */
        if (retcode)
		if (__power_mp())
			db_resume_others();
#endif /* POWER_MP */

	return(retcode);
}

/*
* NAME: Loop
*
* FUNCTION: this routine will loop around an address for a specified
*	number of iterations 
*
* RETURN VALUE DESCRIPTION: 0 if a problem is encounterd 
*			    1 if exiting to resume execution
*/

Loop(ps)
struct parse_out *ps;
{
	extern int loop_count;

	if (ps->num_tok > 0) {
		if (ps->token[1].tflags & DEC_OVERFLOW) {
			printf("Loop count too large\n");
			return 0;
		}
		loop_count = ps->token[1].dv;	/* # of loops */ 
	}
	else
		loop_count = 1;				/* default */
	if (new_breakpoint(debvars[IDIAR].hv,LOOP,T_BIT)) {
		if (new_step(TRUE,step_s1,FALSE)) { 	/* setup is okay */
			if (is_static_break(debvars[IDIAR].hv,T_BIT)) 
				debvars[IDIAR].hv += sizeof(INSTSIZ);
#ifdef _POWER_MP        /* have to prevent from resuming stopped cpus */
			if (__power_mp())
				if (step_processing_level++ == 0)
					hold_cpus_while_stepping();
#endif /* POWER_MP */
			return(1);		/* resume execution */
		}      
		else
			vdbperr(cant_step_or_go);
	}
	loop_count = 0;				/* something is wrong */
	return(0);
}

/*
* NAME: Next
*
* FUNCTION: this routine will  increment the iar by the number specified
*	by the user. Default is 4 bytes.
*
* RETURN VALUE DESCRIPTION: always 0
*/

Next(ps)
struct parse_out *ps;
{
	if (ps->num_tok<1)  			/* back up the iar */
		debvars[IDIAR].hv = debvars[IDIAR].hv + 4;	
	else {
		if (ps->token[1].tflags & HEX_OVERFLOW) {
			printf("Number too large\n");
			return 0;
		}
		debvars[IDIAR].hv = debvars[IDIAR].hv + ps->token[1].hv;
		ps->num_tok = 0;	/* to avoid conflict when */
					/* calling debug_screen	*/
	}
	debug_screen(ps, DEFLTSCR);

	return(0);
}

/*
* NAME: Origin
*
* FUNCTION: this routine will increment the local variable for the origin
*
* RETURN VALUE DESCRIPTION: always 0
*/

Origin(ps)
struct parse_out *ps;
{
	if (ps->token[1].tflags & HEX_OVERFLOW) {
		printf("Number too large\n");
		return 0;
	}
	debvars[IDORG].hv = ps->token[1].hv;	/* set origin */
	return(0);
}

/*
* NAME: Quit
*
* FUNCTION: this routine will clear all breakpoint and reset the mst data
*	areas then quit the debugger
*
* RETURN VALUE DESCRIPTION: 0 if an error encountered
*			    !0 if quitting the debugger
*/

Quit(ps)
struct parse_out *ps;
{
	int i;

	if ((i=quitrtn(ps)) != -1) {
#ifdef _POWER_MP        /* have to resume stopped cpus */
		if (__power_mp()){
			step_processing_level = 0;
			db_resume_others();
		}
#endif /* POWER_MP */
		return(i);
	}
	return(0);
}

/*
* NAME: Restore
*
* FUNCTION: this routine will modify the restore_screen variable as
*	specified by the user
*
* RETURN VALUE DESCRIPTION: 0 is always returned
*/

Restore(ps)
struct parse_out *ps;
{
	extern int restore_screen;

	if (ps->num_tok < 1)
		restore_screen = TRUE;		/* default */
	else
		restore_screen = ps->token[1].tflags & YES_ON;
	return(0);
}

/*
* NAME: Screen
*
* FUNCTION: Call the function to display the screen with just the general
*		purpose regs and 7 lines of memory.
*
* RETURN VALUE DESCRIPTION: 0 is always returned
*/

Screen(ps)
struct parse_out *ps;
{
	debug_screen(ps, MEMSCR);
	return(0);
}


/*
* NAME: Sregs
*
* FUNCTION: Call the routine to display all the general purpose, segment and
*		floating point registers.
*
* RETURN VALUE DESCRIPTION: 0 is always returned
*/

Sregs(ps)
struct parse_out *ps;
{
	debug_screen(ps, SREGSCR);
	return(0);
}

#ifdef _POWER
/*
* NAME: float
*
* FUNCTION: Call the routine to display all the general purpose and
*		floating point registers.
*
* RETURN VALUE DESCRIPTION: 0 is always returned
*/

fpregs(ps)
struct parse_out *ps;
{
	debug_screen(ps, FPREGSCR);
	return(0);
}
#endif /* _POWER */

/*
* NAME: Set
*
* FUNCTION: this routine will call the fucntion to set a variable
*
* RETURN VALUE DESCRIPTION: 0 is always returned
*/

Set(ps)
struct parse_out *ps;
{
	extern char *in_string;
	char *ptr;

	if (((ps->token[2].tflags & (HEX_VALID | HEX_OVERFLOW)) == 
	     (HEX_VALID | HEX_OVERFLOW)) ||
	    ((ps->token[2].tflags & (DEC_VALID | DEC_OVERFLOW)) ==
	     (DEC_VALID | DEC_OVERFLOW))) {
		printf("Number too large\n");
		return 0;
	}

	for (ptr = ps->token[1].sv; *ptr != '\0'; ptr++)
	{
		if (! (is_digit(*ptr) || is_alpha (*ptr)))
		{
			vdbperr (ivd_text1, ps->token[1].sv, ivd_text2);
			return (0);
		}
	}

	set_var(ps, in_string);

	return(0);
}

/*
* NAME: St
*
* FUNCTION: this routine will store a full word to the address specified
*	by the user
*
* RETURN VALUE DESCRIPTION: 0 is always returned
*/

St(ps)
struct parse_out *ps;
{
	ulong	data;

	if ((ps->token[1].tflags & HEX_OVERFLOW) ||
	    (ps->token[2].tflags & HEX_OVERFLOW)) {
		printf("Number too large\n");
		return 0;
	}

	data = ps->token[2].hv;
	if (!get_put_aligned(ps->token[1].hv,vbit(1), &data,TRUE,4))
		printf("error writing to memory\n");
		
	return(0);
}

/*
* NAME: Stc
*
* FUNCTION: this routine will store a char to the address specified by
*	the user
*
* RETURN VALUE DESCRIPTION: 0 is always returned
*/

Stc(ps)
struct parse_out *ps;
{
	ulong data;

	if ((ps->token[1].tflags & HEX_OVERFLOW) ||
	    (ps->token[2].tflags & HEX_OVERFLOW)) {
		printf("Number too large\n");
		return 0;
	}

	data = ps->token[2].hv;
	if (!get_put_aligned(ps->token[1].hv,vbit(1), &data,TRUE,1))
		printf("error writing to memory\n");
		
	return(0);
}

/*
* NAME: Sth
*
* FUNCTION: this routine will store a short to the address specified by
*	the user
*
* RETURN VALUE DESCRIPTION: 0 is always returned
*/

Sth(ps)
struct parse_out *ps;
{
	ulong data;

	if ((ps->token[1].tflags & HEX_OVERFLOW) ||
	    (ps->token[2].tflags & HEX_OVERFLOW)) {
		printf("Number too large\n");
		return 0;
	}

	data = ps->token[2].hv;
	if (!get_put_aligned(ps->token[1].hv,vbit(1), &data,TRUE,2))
		printf("error writing to memory\n");
		
	return(0);
}

/*
* NAME: Swap
*
* FUNCTION: this routine will move the debugger to an async terminal
*
* RETURN VALUE DESCRIPTION: 0 is always returned
*/

Swap(ps)
struct parse_out *ps;
{
	extern ulong dbterm;
	extern char restore_cntl;
	ulong tmpdbterm;
	int b;

	tmpdbterm = dbterm;
	if(ps->num_tok == 0) {
		/* Just return if already using the default. */
		if ((dbterm & USE_TTY) && ((dbterm & TTY_PORT) == TTY_DEFAULT))
			return(0);
		dbterm |= (USE_TTY | TTY_DEFAULT);
	}
	else {
		/* stick tty number in dbterm */
		dbterm = (USE_TTY | ps->token[1].hv);
	}

	/* Open the terminal. */
	if (dbterm & USE_TTY) {
		/* Close old port if we are changing ports. */
		if (tmpdbterm != dbterm) {
			d_ttyclose(tmpdbterm & TTY_PORT); /* close port */
		}

		if((b=d_ttyopen(dbterm & TTY_PORT)) <=0) {
			if (!b)	/* if we were just missing carrier
				   try to restore port */
				d_ttyclose(dbterm & TTY_PORT);
			dbterm = tmpdbterm;	/* restore original display */
			/* bad news if the re-open fails */
			d_ttyopen(dbterm & TTY_PORT);
			printf("Couldn't open requested display. (%d)\n",b);
		}
	}

	return(0);
}

/*
* NAME: Proc
*
* FUNCTION: this routine will call the function to display the process
*	table
*
* RETURN VALUE DESCRIPTION: 0 is always returned
*/

Proc(ps)
struct parse_out *ps;
{
	pr_proc(ps);     
	return(0);
}
#ifdef _THREADS
/* 
* NAME: Thread
*
* FUNCTION: this routine will call the function to display the thread
*	table
*
* RETURN VALUE DESCRIPTION: 0 is always returned
*/

Thread(ps)
struct parse_out *ps;
{
	pr_thread(ps);
	return(0);
}
#endif /* THREADS */

/*
* NAME: Step
*
* FUNCTION: this routine will cause single-multiple stepping through a 
*	program. The default step_count is 1, the  user can specify any
*	decimal number or by using "s" step-over a subroutine and resume
*	single stepping.
*
* RETURN VALUE DESCRIPTION: 0 is always returned
*/

Step(ps)
struct parse_out *ps;
{
	uchar step_s = FALSE;
	extern int step_count;
	extern char restore_cntl;

	if (ps->num_tok < 1)
		step_count = 1;			/* default count is 1 */
	else if ((ps->token[1].tflags&DEC_VALID) && (ps->token[1].dv>0))
		step_count = ps->token[1].dv; /* specified count */
	else if (ps->token[1].sv[0] == 's')
		step_s = TRUE;			/* step over a subroutine */
	else {
		vdbperr(ivd_text1, ps->token[1].sv, ivd_text2);
		return(0);
	}

	if ((step_count == 1) || step_s) 
		restore_cntl = FALSE; /* don't screen restore if single step */
	if (new_step(FALSE, step_s,FALSE)) {		/* step 1 if possible */
		if (is_static_break(debvars[IDIAR].hv, T_BIT)) /* static bk */
			/* get around if static bk */
			debvars[IDIAR].hv += sizeof(INSTSIZ);
#ifdef _POWER_MP        /* have to prevent from resume stopped cpus */
		if (__power_mp())
			if (step_processing_level++ == 0)
				hold_cpus_while_stepping();
#endif /* POWER_MP */
		return(1);
	}
	vdbperr(cant_step_or_go);
	return(0);
}

/*
* NAME: Watch
*
* FUNCTION: determine the type of watchpoint to be set and the address
*       where it should be set and then call the routine to set the
*       trap so the system will trap at the specified address
*
* RETURN VALUE DESCRIPTION: always 0
*/

Watch(ps)
struct parse_out *ps;
{

        caddr_t e_addr;
        uchar   type='b';


        switch(ps->num_tok) {
        case 0:
                printf("Specify address\n");
                return(0);
                break;
        case 1:
                if (!(ps->token[1].tflags & HEXDEC)) {
                        printf("Bad address\n");
                        return(0);
                }
                if (ps->token[1].tflags & HEX_OVERFLOW) {
                        printf("Address too large\n");
                        return(0);
                }
                e_addr = ps->token[1].hv;       /* may be virt or real */
                /*virt = vbit(1);*/
                new_watchpoint(e_addr,type);    /* set new watchpoint */
#if 0
        if (new_step(FALSE,FALSE,TRUE)) {               /* step 1 if possible */
                if (is_static_break(debvars[IDIAR].hv, T_BIT)) /* static bk */
                        /* get around if static bk */
                        debvars[IDIAR].hv += sizeof(INSTSIZ);
        }
#endif
                break;
        case 2:
                if (!strcmp(ps->token[1].sv,"l")) type='l';
                else
                        if (!strcmp(ps->token[1].sv,"s")) type='s';
                        else {
                                printf("Wrong watch type.\n");
                                return(0);
                        }
                if (ps->token[2].tflags & HEX_OVERFLOW) {
                        printf("Address too large\n");
                        return(0);
                }
                if (!(ps->token[2].tflags & HEXDEC)) {
                        printf("Bad address\n");
                        return(0);
                }
                e_addr = ps->token[2].hv;       /* may be virt or real */
                new_watchpoint(e_addr,type);    /* set new watchpoint */
#if 0
        if (new_step(FALSE,FALSE,TRUE)) {               /* step 1 if possible */
                if (is_static_break(debvars[IDIAR].hv, T_BIT)) /* static bk */
                        /* get around if static bk */
                        debvars[IDIAR].hv += sizeof(INSTSIZ);
        }
#endif
                break;
        default:
                printf("Wrong number of parameters\n");
                return(0);
                break;
        }

        return(0);
}





/*
* NAME: Xlate
*
* FUNCTION: this routine will display a transalteed real address for a 
*	user specified virtual address
*
* RETURN VALUE DESCRIPTION: always 0
*/
 
Xlate(ps)
struct parse_out *ps;
{
	ulong	addr;

	if (ps->token[1].tflags & HEX_OVERFLOW) {
		printf("Number too large\n");
		return 0;
	}

	
	if ((addr=lqra(SEG(ps->token[1].hv),ps->token[1].hv)) != -1)
		/* address mapped */
		printf("%x -virtual- = %6x -real- \n", ps->token[1].hv,
			(addr & 0x0fffffff));
	else 	/* address NOT mapped */
		printf("%x -virtual- = no real mapping\n", ps->token[1].hv);

	return(0);
}

/*
* NAME: quitrtn
*
* FUNCTION: this routine will handle the quit command
*
* RETURN VALUE DESCRIPTION: -1 is returned if error
*			    non_zero if successful
*/

int
quitrtn(ps)
struct parse_out *ps;
{
	extern int restore_screen;
	extern char restore_cntl;
	extern int debug_init;
	int code = QUITRET;

	restore_screen = TRUE;
	debug_init = TRUE;	/* next time in, initialize from scratch */

	/* check to see if parm specified */
	if (ps->num_tok >= 1) {
		if (strcmp(ps->token[1].sv, QUIT_PARM)) {
			printf("Invalid parameter \n");
			return(-1);
		}
		else	
		{
		code = DUMPRET;		/* code of 8 */
		restore_cntl = FALSE;
		}
	}
	if (is_static_break(debvars[IDIAR].hv, T_BIT)) 
		debvars[IDIAR].hv += sizeof(INSTSIZ); /* get past SDT */
	/* this is to fake out clear_breakpoint so it will clear all bp's */
	ps->token[1].sv[0] = ASTRISK;	
	ps->token[1].tflags = EXP_TYPE;
	ps->num_tok = 1;

	/* failed if we couldn't clear breakpoints and this is a normal quit */
	/* clear watchpoints then bratpoints the breakpoints */
	if (!clear_breakpoint(ps) && (code != DUMPRET)) 
		code = -1;
        clear_watchpoint();             /* clear watchpoint*/
        clear_watch_regs();
        clear_brat();
        clear_brat_regs();
	return(code);
}



#ifdef _POWER
#ifdef DEBUG
/*
* NAME: bpr
*
* FUNCTION: set special purpose register with data address to trap on
*
*/
bpr(ps)
struct parse_out *ps;
{


	if (ps->num_tok <= 0) 
	   printf("usage: \"bpr <address>\"  or \"bpr 0\" to disable breaks\n");
	else
		if(ps->token[1].hv == 0)
			mtbpr(0);	/* shut off bpr */
		else
			mtbpr(ps->token[1].hv|0x1); /* or in 1 into bit 31 - 
						   indicating DO trap on this 
	   					range (16 bytes) of addressed */

	return 0;
}
#endif /* DEBUG */
#endif /* _POWER */


Udisplay(ps)				/* Display <hex addr> <dec number> */
					/* Displays storage as opcodes,	   */
					/* starting at addr, for number    */
					/* opcodes (i.e. words).  Rounds   */
					/* addr down to word boundary      */
					/* before displaying.		   */
struct parse_out *ps;
{
	ulong	num;
	int j;

	/* Check supplied arguments for hex overflow */
	for (j = 0; j < ps -> num_tok; j++)
		if (ps -> token[j + 1].tflags & HEX_OVERFLOW)
		{
			printf ("Argument %s too large\n",
				ps -> token[j + 1].sv);
			return 0;
		}
 
	if (ps->num_tok > 1)
		num = ps->token[2].dv;
	else
		num = 1;
	debug_disasm(ps->token[1].hv, num, vbit(1));
	return(0); 
}

/* Reason - Tells user the reason the debugger was entered.  Called via the */
/* 	"reason" command.						    */

Reason()
{
  tell_reason(save_reason,save_ext_arg);
  return(0);
}

/* Sysinfo - Gives the user information about the current system from the */
/* _system_configuration structure.					  */

extern int ipl_cb;
Sysinfo()
{

  int i,j;
  struct ipl_cb   *iplcb_ptr;             /* ros ipl cb pointer */
  struct ipl_info *info_ptr;              /* info pointer */
  struct ipl_directory *dir_ptr;          /* ipl dir pointer */


  typedef struct cfgstring {
      int	number;
      char	*string;
  } CFGSTRING;

  static CFGSTRING architecture[] = {
	{POWER_RS, "POWER"},
	{POWER_PC, "POWER PC"},
	{0, "Unknown"}
  };

  static CFGSTRING implementation[] = {
	{POWER_RS1, "RS1"},
	{POWER_RSC, "RSC"},
	{POWER_RS2, "RS2"},
	{POWER_601, "601"},
	{POWER_603, "603"},
	{POWER_604, "604"},
	{POWER_620, "620"},
	{0, "Unknown"}
  };

/* The usage of #ifdefs here corresponds with their usage in 
   sys/systemcfg.h.  Without them, this file won't compile properly, since
   in many cases, not all of these macros are defined. */
  static CFGSTRING model_arch[] = {
#ifdef _RS6K
	{RS6K, "RS/6000"},
#endif
#ifdef _RSPC
	{RSPC, "RS/PC"},
#endif
	{0, "Unknown"}
  };

  static CFGSTRING model_impl[] = {
#ifdef _RS6K_UP_MCA
	{RS6K_UP_MCA, "UP MCA"},
#endif
#ifdef _RS6K_SMP_MCA
	{RS6K_SMP_MCA, "SMP MCA"},
#endif
#ifdef _RSPC_UP_PCI
	{RSPC_UP_PCI, "UP PCI"},
#endif
	{0, "Unknown"}
  };

  /* Architecture information */
  for (i=0; architecture[i].number != 0; i++)
	if (architecture[i].number == _system_configuration.architecture)
		break;
  printf("%s architecture\n",architecture[i].string);

  /* Processor/implementation information */
  for (i=0; implementation[i].number != 0; i++)
	if (implementation[i].number == _system_configuration.implementation)
		break;
  printf("%s processor\n",implementation[i].string);

  /* Model architecture information */
  for (i=0; model_arch[i].number != 0; i++)
	if (model_arch[i].number == _system_configuration.model_arch)
		break;
  printf("%s model architecture\n",model_arch[i].string);

  /* Model/bus information */
  for (i=0; model_impl[i].number != 0; i++)
	if (model_impl[i].number == _system_configuration.model_impl)
		break;
  printf("%s model implementation\n",model_impl[i].string);

  /* get addressability to iplinfo structure, and display the
   * model information
   */
  iplcb_ptr = (struct ipl_cb *)ipl_cb;
  dir_ptr = &(iplcb_ptr->s0);
  info_ptr = (struct ipl_info *)((char *) iplcb_ptr +
	realbyt(&(dir_ptr->ipl_info_offset),4));
  printf("Machine model: %08x\n", realbyt(&(info_ptr->model),4));

  return(0);
}
