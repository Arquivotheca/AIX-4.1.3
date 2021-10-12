static char sccsid[] = "@(#)59  1.26  src/bos/kernel/db/POWER/dbscreen.c, sysdb, bos411, 9428A410j 3/28/94 17:56:36";
/*
 * COMPONENT_NAME: (SYSDB) Kernel Debugger
 *
 * FUNCTIONS:   debug_screen, show_default, show_segregs, show_floatingpt,
 *              show_gprs, show_fprs, show_regs, show_unique_regs,
 *              show_the_rest, show_sregs, specify_screen
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/types.h>
#include <sys/seg.h>
#include <sys/systemcfg.h>
#include "parse.h"                      /* parser structure             */
#include "debaddr.h"
#include "debvars.h"
#include "dbdebug.h"                    /* display defines              */
#include "vdberr.h"                     /* error msg stuff              */

#define isdigit(c)      (((c>='0') && (c<='9')))
#define isxdigit(c)     ((((c>='0') && (c<='9')) || ((c>='a') && (c<='f')) || \
			  ((c>='A') && (c<='F'))))
#define tolower(c)      ((((c>='A') && (c<='Z')) ? c + ('a' - 'A') : c))

/*
 * EXTERNAL PROCEDURES CALLED:
 */

extern strcmp();                /* clear screen                 */
extern clrdsp();                /* clear screen                 */
extern int get_addr();          /* get R/V addr                 */
extern int get_from_memory();   /* get data from memory         */
extern int debug_display();     /* display data                 */
extern int debug_opcode();      /* descriptive info about Opcode */
extern int view_vidx;           /* for S track          */

uchar screen_track=0;
ulong view_virt=0;
ulong view_addr=0;              /* addr to be displayed */
uchar screen_half=0;
uchar view_addrset=FALSE;
uchar current_screentype=MEMSCR;
uchar screen_on=1;              /*flag indicating display/do not display scrn*/
char *CS_Bit_Lab[8]={" PZ"," LT"," EQ"," GT"," CO","   "," OV"," TB"};


/*
 * NAME: debug_screen
 *
 * FUNCTION:
 *   VRM Debugger display command
 *
 *   This function is called via the screen command from the debugger
 *     driver program. Most of the data is retieved from the debugger
 *     variable area. The offsets are found in debvars.h
 *
 *
 * RETURN VALUE: nothing is returned
 *
 */

char screen_long=FALSE;
struct parse_out *ps;                   /* Parser structure             */

debug_screen(ps, scrntype)
struct parse_out *ps;                   /* Parser structure             */
int     scrntype;                       /* which screen to display      */
{

	if (scrntype == DEFLTSCR)
		scrntype = current_screentype;
	else
		current_screentype = scrntype;

	clrdsp();                       /* clear the screen */
	if (!specify_screen(ps) || !screen_on)
		return(0);

	switch(scrntype) {
		case FPREGSCR:
			show_floatingpt();
			break;
		case SREGSCR:
			show_segregs();
			break;
		case MEMSCR:
			show_default();
			break;
	}
	current_screentype = scrntype;
	return(0);
}

show_default()
{
	show_gprs();                    /* show 32 general purpose regs */
	show_regs();                    /* show the default regs */
	show_the_IAR();                 /* show the instruction address */
	show_the_rest(7);               /* show 8 lines of memory */
	return(0);
}

show_segregs()
{
	show_gprs();                    /* show 32 general purpose regs */
	show_regs();                    /* show the default regs */
	show_sregs();                   /* show the 16 seg regs */
#ifdef _POWER_PC
	/* display the BATs *IF* we have the supporting code
	   AND we are on the right type of processor (603, 604, ...) */
	if (__power_pc() && !__power_601())
	  show_bats();          /* show the BAT registers */
#endif /* #ifdef _POWER_PC */
	show_unique_regs();             /* show the extra regs */
	show_the_IAR();                 /* show the instruction address */
#ifdef _POWER_PC
	/* if we are not on a 603 or 604, just act like we used to */
	if (!__power_pc() || __power_601())
#endif /* #ifdef _POWER_PC */
	  show_the_rest(2);             /* show 2 lines of memory */
	return(0);
}

show_floatingpt()
{
	show_gprs();                    /* show 32 general purpose regs */
	show_regs();                    /* show the default regs */
	show_fprs();                    /* show floating pt regs */
	return(0);
}

show_gprs()
{
	register int    j;              /* for loop var */

	/* line 1: print the general purpose registers 1-7 */
	printf("GPR0  ");
	for (j=0; j<=7; j++)
		printf("%08x ", debvars[IDGPRS+j].hv);

	/* line 2: print the general purpose registers 8-16 */
	printf("\nGPR8  ");
	for (j=8; j<=15; j++)
		printf("%08x ", debvars[IDGPRS+j].hv);

	/* line 3: print the general purpose registers 16-23 */
	printf("\nGPR16 ");
	for (j=16; j<=23; j++)
		printf("%08x ", debvars[IDGPRS+j].hv);

	/* line 4: print the general purpose registers 24-32 */
	printf("\nGPR24 ");
	for (j=24; j<=31; j++)
		printf("%08x ", debvars[IDGPRS+j].hv);
	printf("\n");
	return(0);
}

show_fprs()
{
	register int    j;              /* for loop var */

	/* print the fpscr */
	printf("\nFPSCR %08x\n", debvars[IDFPSCR].hv);

	/* print the  floating point registers */
	printf("\nFPR0  ");
	for (j=0; j<=3; j++)
		printf("%08x ", fr[j]);
	printf("\nFPR4  ");
	for (j=4; j<=7; j++)
		printf("%08x ", fr[j]);

	printf("\nFPR8  ");
	for (j=8; j<=11; j++)
		printf("%08x ", fr[j]);

	printf("\nFPR12 ");
	for (j=12; j<=15; j++)
		printf("%08x ", fr[j]);

	printf("\nFPR16 ");
	for (j=16; j<=19; j++)
		printf("%08x ", fr[j]);

	printf("\nFPR20 ");
	for (j=20; j<=23; j++)
		printf("%08x ", fr[j]);

	printf("\nFPR24 ");
	for (j=24; j<=27; j++)
		printf("%08x ", fr[j]);

	printf("\nFPR28 ");
	for (j=28; j<=31; j++)
		printf("%08x ", fr[j]);
	printf("\n\n");
	return(0);
}


show_regs()
{
	printf("\nMSR %08x",debvars[IDMSR].hv);
	printf("  CR   %08x",debvars[IDCR].hv);
	printf("  LR   %08x",debvars[IDLR].hv);
	printf("  CTR   %08x",debvars[IDCTR].hv);
	printf("  MQ   %08x",debvars[IDMQ].hv);
	printf("\nXER %08x",debvars[IDXER].hv);
	printf("  SRR0 %08x",debvars[IDSRR0].hv);
	printf("  SRR1 %08x",debvars[IDSRR1].hv);
	printf("  DSISR %08x",debvars[IDDSISR].hv);
	printf("  DAR  %08x",debvars[IDDAR].hv);
	printf("\n");
	return(0);
}

show_unique_regs()
{
#if defined (_POWER_RS1) || defined(_POWER_RSC)
	if ( __power_rs1() || __power_rsc() ) {
	printf("\nEIM0 %08x",debvars[IDEIM0].hv);
	printf("  EIM1 %08x",debvars[IDEIM1].hv);
	printf("  EIS0 %08x",debvars[IDEIS0].hv);
	printf("  EIS1 %08x",debvars[IDEIS1].hv);
	}
#endif /* #if defined (_POWER_RS1) || ... */

#if defined (_POWER_RS2)
	if ( __power_rs2() ) {
	printf("\nPEIS0 %08x",debvars[IDPEIS0].hv);
	printf("  PEIS1 %08x",debvars[IDPEIS1].hv);
	printf("  ILCR %08x",debvars[IDILCR].hv);
	}
#endif /* #if defined (_POWER_RS2) */

#ifdef _POWER_PC
	if ( __power_pc() ) {
	printf("\nXIRR %08x",debvars[IDXIRR].hv);
	printf("  DSIER %08x",debvars[IDDSIER].hv);
	}
#endif /* #ifdef _POWER_PC */

	printf("  TID %08x",debvars[IDTID].hv);
	printf("\nSDR0 %08x",debvars[IDSDR0].hv);
	printf("  SDR1 %08x",debvars[IDSDR1].hv);

	/* the RTC (real time counter) on POWER is replaced
	   by the TB (time base) register on POWER_PC */
#ifdef _POWER_PC
	if (__power_pc() && !__power_601())
	  {
	    printf("  TBU  %08x", debvars[IDTBU].hv);
	    printf("  TBL  %08x", debvars[IDTBL].hv);
	  }
	else
#endif /* #ifdef _POWER_PC */
	  {
	    printf("  RTCU %08x", debvars[IDRTCU].hv);
	    printf("  RTCL %08x", debvars[IDRTCL].hv);
	  }

	printf("  DEC %08x",debvars[IDDEC].hv);
	printf("\n");
	return(0);
}

show_the_IAR()
{
	register int i;
	register int position, word, byte;
	caddr_t a;
	struct descr descr;             /* opcode structure */


	printf("\nIAR %08x", debvars[IDIAR].hv);    /* show IAR */
	printf("  (ORG%s",(debvars[IDIAR].hv-debvars[IDORG].hv>0)?"+":"-");
	printf("%08x) ",( (int) (debvars[IDIAR].hv-debvars[IDORG].hv) >=0 ?
	debvars[IDIAR].hv-debvars[IDORG].hv :
		-debvars[IDIAR].hv-debvars[IDORG].hv));
	printf(" ORG=%08x", debvars[IDORG].hv);
	printf("   Mode: %s\n",(T_BIT?"VIRTUAL":"REAL"));

	/* display a line  of memory */
	debug_display(debvars[IDIAR].hv & 0xfffffff0,16,INSTT_BIT);

	/* point to the instruction */
	a = (caddr_t) debvars[IDIAR].hv;
	word = ((int)a & 0x0000000c) / 4;
	byte = (int)a & 0x00000003;
	position = 12 + 9*word + 2*byte;
	for (i=1; i < position; i++) {
		printf(" ");
	}
	printf("| ");

	/* print out the mnemonic */
	if (debug_opcode(a ,&descr)) {  /* decode, if not paged */
		printf("%s",descr.D_Mnemonic);
		if ((int)descr.D_EA != -1) {
			printf(" (");
			printf("%08x",descr.D_EA);
			printf(") ");
		}
	}
	printf("\n");

	/* display a second line  of memory*/
	debug_display(debvars[IDIAR].hv+16 & 0xfffffff0,16,INSTT_BIT);

	printf("\n");
}

show_the_rest(lines)
int lines;                              /* lines of memory to be displayed */
{
	register int i;
	register int position, word, byte;
	caddr_t a;


	/* storage section */
	if (screen_track)  {            /* tracking a variable */
		view_addr = debvars[view_vidx].hv;
		view_virt = INSTT_BIT;
	}                               /* view_addr already set */
	a = (caddr_t) view_addr;

	if (!screen_half)  {
		/* adjust instruction bar */
		word = ((int)a & 0x0000000c) / 4;
		byte = (int)a & 0x00000003;
		position = 12 + 9*word + 2*byte;
		for (i=1; i < position; i++, printf(" "));
		printf("|\n");
		/* display x number of lines */
		debug_display((int)a & 0xfffffff0, lines*16, view_virt);
		printf("\n");
	}
	return(0);
}

show_sregs()
{
	register int    j;              /* for loop var */

	/* line 1: print the segment registers 1-7 */
	printf("\nSR0 ");
	for (j=0; j<=7; j++)
		printf("%08x ", debvars[IDSEGS+j].hv);

	/* line 2: print the segment registers 8-16 */
	printf("\nSR8 ");
	for (j=8; j<=15; j++)
		printf("%08x ", debvars[IDSEGS+j].hv);
	printf("\n");
	return(0);
}

/*
* FUNCTION: specify_screen
*       this function will determine how the information
*       displayed on the screen will be formatted. The first switch
*       statement looks at the number of tokens, either 1, 2 or 3.
*       If only "Screen was entered the IAR is tracked. If 2 tokens
*       were entered, the second token is found and appropriate flags
*       are set. Three flags are mainly for tracking variables.
*       The flags set here are used by debug_screen to decide what
*       data is displayed.
*
* RETURN VALUE: FALSE if error encountered
*               TRUE if successful
*/

specify_screen(ps)
struct parse_out *ps;                   /* parser structure     */
{
	int i;

	if (view_addrset == FALSE) {            /* set these 2 vars once */
		view_addr = debvars[IDIAR].hv;  /* addr to be displayed */
		view_virt = T_BIT;
	}

	switch(ps->num_tok) {             /* tokens = 0,1 or 2       */
	    case 0:                       /* "SCreen with no parameters    */
		break;

	    case 1:
		for (i = 0; ps->token[1].sv[i] != 0; i++)
			ps->token[1].sv[i] = tolower(ps->token[1].sv[i]);

		if (!strcmp(ps->token[1].sv,PLUS)) {  /* SCreen +*/
			screen_track=FALSE;
			view_addr=view_addr+7*16;   /* Scroll forward */
			view_addrset = TRUE;
		}
		else if (!strcmp(ps->token[1].sv, MINUS)) { /* SCreen - */
			screen_track=FALSE;
			view_addr=view_addr-7*16;       /* Scroll backward */
			view_addrset = TRUE;
		}
		else if (!strcmp(ps->token[1].sv, "off")) { /* no display  */
			screen_on=FALSE;
		}
		else if (!strcmp(ps->token[1].sv,"on")) {
			screen_on = TRUE;
			screen_half=FALSE;    /* On - default=Full */
		}
		else if (ps->token[1].tflags & HEXDEC) { /* SCreen <hex addr> */
			screen_track=FALSE;
			get_addr(1,ps);
			view_addr=(ulong)ps->token[1].debaddr->addr;
			view_virt=ps->token[1].debaddr->virt;
			view_addrset = TRUE;
		}
		else {              /* Invalid parameter        */
			vdbperr(ivd_text1,ps->token[1].sv,ivd_text2);
			return(FALSE);
		}
		break;

      default: /* 3 parameters */       /* SCreen Track <Gnn | Rnn>  */
	  switch(ps->token[1].sv[0]) {
	      case 't':                 /* Track specified               */
		  if (ps->token[2].varidx == NOT_A_VAR)          /* Not a lone variable*/
		  {
			vdbperr(ivd_text1,ps->token[2].sv,ivd_text2);
			return(FALSE);
		  }
		  else {                /* variable track */
			screen_track=TRUE;
			view_vidx=ps->token[2].varidx;
		  }
		  break;
	      case 'o':                 /* On half */
		  if (ps->token[2].sv[0] == 'h')
		  {
			screen_on=TRUE;
			screen_half=TRUE;
		  }
		  break;
	      default:
		  if (get_addr(1,ps)) { /* might be R/V address  */
			screen_track = FALSE;
			view_addr = (ulong) ps->token[1].debaddr->addr;
			view_virt = ps->token[1].debaddr->virt;
			view_addrset = TRUE;
		  }
		  else {                /* otherwise invalid */
			vdbperr(ivd_text1,ps->token[1].sv,ivd_text2);
			return(FALSE);
		  }
		  break;
	    }
	}
#define IS_IOSPACE(x) (debvars[IDSEGS+(x>>SEGSHIFT)].hv & 0x80000000)

	/*  if, through a totaly hosed up iar, or a foolish user,
	 *  i/o space is being screened, don't allow it
	 */
	if(IS_IOSPACE(view_addr)) {
	     printf("Address:0x%08x is in I/O space!\n",view_addr);
	     printf("To display I/O space use:    display <address> [count]\n");
	     return(FALSE);
	}

	return(TRUE);
}

#ifdef _POWER_PC

/* show the BAT regs on 601 and 603/604 */

show_bats()
{
  register int  j;              /* for loop var */

  /* line 1: print the upper BAT registers */
  printf("\nBAT0U");
  for (j=0; j<NUM_KERNEL_BATS; j++)
    printf(" %08x", debvars[IDBATU+j].hv);

  /* line 2: print the lower BAT registers */
  printf("\nBAT0L");
  for (j=0; j<NUM_KERNEL_BATS; j++)
    printf(" %08x", debvars[IDBATL+j].hv);
  printf("\n");
  return(0);
}
#endif /* #ifdef _POWER_PC */
