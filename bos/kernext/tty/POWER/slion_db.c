/* @(#)93 1.3 src/bos/kernext/tty/POWER/slion_db.c, sysxtty, bos411, 9428A410j 6/10/94 10:09:46 */
/*
 * COMPONENT_NAME: sysxtty
 *
 * FUNCTIONS:	
 * slion_print	slion_print_data	slion_prbits	slion_pstr
 *
 * ORIGINS: 27 83
 */
/*
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */

/* This is a debug and dump facility  of driver which supports 64_Port 
 * Async Controller. 
 */

#include "ttydrv.h"			/* Substitute by stty.h */
#include "slion.h"			/* Lion driver definitions */

#if defined(_KERNEL) && !defined(IN_TTYDBG)
extern	void slion_pio(struct sliondata *ld, pud_t *pud, int logit);
#endif /* _KERNEL && !IN_TTYDBG */

#if defined(_KERNEL) && defined(IN_TTYDBG)
#define slion_db_printf	tty_db_printf
#else
#define slion_db_printf	printf
#endif  /* _KERNEL && IN_TTYDBG */

struct names {				/* used by slion_print_data */
    char	*n_str;
    unsigned	n_mask;
    unsigned	n_state;
};

int		slion_print(str_lionp_t tp, int vv);
static int	slion_print_data(str_lionp_t tp, int vv);
static		slion_pstr(char *name, char *to, int bcnt);
static 		slion_prbits(unsigned word, struct names *np, int ind);

/*
 * FUNCTION:	slion_print
 *
 * PURPOSE:	Printout  FW information.
 *
 * INPUT:
 *      tp:     Pointer to str_lion structure.
 *      vv:	Verbose, print all information.	
 *
 * RETURN:
 *	 0:	on sucess
 *	-1:	on failure
 *
 * INTERNAL PROCEDURES CALLED:
 *	slion_print_data
 *
 * EXTERNAL PROCEDURES CALLED:
 *	tty_read_mem
 *
 * DESCRIPTION:
 *	This function locally builds the main structure to printout by
 *	accessing the str_lion structure itself, each sub-structures and
 *	it brings up to date the pointers to these sub-structures in the
 *	str_lion structure.
 *	Calls the print function proper slion_print_data.
 *
 * CALLED BY:
 *	ttydbg function
 */
int 
slion_print(str_lionp_t tp, int vv)
{
	struct str_lion strl;
	struct sliondata ld;
	struct slionprms prms;
	int res;

	if ((tty_read_mem(tp, &strl, sizeof(strl))) == 0) {
		if ((tty_read_mem(strl.t_hptr, &ld, sizeof(ld))) != 0)
			return(-1);
		strl.t_hptr = &ld;

		if ((tty_read_mem(ld.prms, &prms, sizeof(prms))) != 0)
			return(-1);
		ld.prms = &prms;

		if (res = slion_print_data(&strl, vv))
			return(res);
	}
	else {
		return(-1);
	}

	return(0);
} /* slion_print */

/*
 * FUNCTION:	slion_print_data
 *
 * PURPOSE:	Printout  FW information itself.
 *
 * INPUT:
 *      tp:     Pointer to str_lion structure.
 *      vv:	Verbose, print all information.	
 *
 * RETURN:	0 on success,
 *		return code from tty_db_nomore() macro otherwise
 *
 * INTERNAL PROCEDURES CALLED:
 *	slion_prbis  slion_pstr
 *
 * EXTERNAL PROCEDURES CALLED:
 *	printf  
 *
 * DESCRIPTION:
 *	This function print the contents of table prms in FW. 
 *
 * CALLED BY:
 *	slion_print function
 */
static int 
slion_print_data(str_lionp_t tp, int vv)
{
#if defined(_KERNEL) && !defined(IN_TTYDBG)
    DoingPio;
#endif /* _KERNEL && !IN_TTYDBG */

    struct sliondata *ld = (struct sliondata *)tp->t_hptr;
    struct slionprms *prms = ld->prms;
    uchar *BusMem;
    ushort bb, be, bh, bt;
    static struct names out_flags[] = {
	"comm", COMM_FLAG, COMM_FLAG,
	"flush", FLSH_FLAG, FLSH_FLAG,
	"xmit", XMIT_FLAG, XMIT_FLAG,
	"diag", DIAG_FLAG, DIAG_FLAG,
	"xbox", XBOX_FLAG, XBOX_FLAG,
	0,
    };
    static char *pars[] = {
	"no",
	"odd",
	"???",
	"even",
	"?!?",
	"",
	"",
	"",
    };
    static struct names gcomm_commands[] = {
	"flsh_main", GC_FLSH_MAIN, GC_FLSH_MAIN,
	"flsh_vger", GC_FLSH_VGER, GC_FLSH_VGER,
	"flsh_xpar", GC_FLSH_XPAR, GC_FLSH_XPAR,
	"set_inhib", GC_SET_INHIB, GC_SET_INHIB,
	"zap_inhib", GC_ZAP_INHIB, GC_ZAP_INHIB,
	"e_fl_main", GC_E_FL_MAIN, GC_E_FL_MAIN,
	"e_fl_vger", GC_E_FL_VGER, GC_E_FL_VGER,
	"e_fl_xpar", GC_E_FL_XPAR, GC_E_FL_XPAR,
	"set_parms", GC_SET_PARMS, GC_SET_PARMS,
	"vger_defs", GC_VGER_DEFS, GC_VGER_DEFS,
	"goto_xpar", GC_GOTO_XPAR, GC_GOTO_XPAR,
	"leav_xpar", GC_LEAV_XPAR, GC_LEAV_XPAR,
	"def_error", GC_DEF_ERROR, GC_DEF_ERROR,
	"clr_inhib", GC_CLR_INHIB, GC_CLR_INHIB,
	"frce_main", GC_FRCE_MAIN, GC_FRCE_MAIN,
	0,
    };

    tty_db_nomore(slion_db_printf("Lion adapter at 0x%08x, ip at 0x%08x.\n", ld->board, ld->ip));

    if (ld->txcnt)
	tty_db_nomore(slion_db_printf("%d chars for output at 0x%08x.\n", ld->txcnt, ld->txbuf));
    if (ld->comm_head != ld->comm_tail)
	tty_db_nomore(slion_db_printf("comm_head=%d, comm_tail=%d.\n",
		ld->comm_head, ld->comm_tail));
    tty_db_nomore(slion_db_printf("has_died: %s, has_died2: %s, open_evt = 0x%08x.\n",
	   prms->has_died?"true":"false", prms->has_died2?"true":"false",
	   prms->m_open_evt));

    if (ld->flags) 
    {
	tty_db_nomore(slion_db_printf("Output flags:"));
	tty_db_nomore(slion_prbits(ld->flags, out_flags, 8));
	tty_db_nomore(slion_db_printf("\n"));
    }
    if (ld->block)
	tty_db_nomore(slion_db_printf("input blocked\n"));

    tty_db_nomore(slion_db_printf("%d data bits, %s, %s parity, baud=%d.\n",
	   5+prms->char_size, prms->stop_bits?"2 stop bits":"1 stop bit",
	   pars[prms->parity], prms->baud));

    tty_db_nomore(slion_db_printf("Data Set Signals:"));
    if (prms->dcd || prms->cts || prms->dtr || prms->rts) 
    {
	if (prms->dcd) tty_db_nomore(slion_db_printf(" dcd"));
	if (prms->cts) tty_db_nomore(slion_db_printf(" cts"));
	if (prms->dtr) tty_db_nomore(slion_db_printf(" dtr"));
	if (prms->rts) tty_db_nomore(slion_db_printf(" rts"));
    } else {
	tty_db_nomore(slion_db_printf(" none"));
    }
    tty_db_nomore(slion_db_printf("\n"));

    if ((vv & TTYDBG_ARG_V) == TTYDBG_ARG_V)
    {
	slion_pstr("goto1", prms->goto1, sizeof(prms->goto1));
	slion_pstr("goto2", prms->goto2, sizeof(prms->goto2));
	slion_pstr("\nscreen1", prms->screen1, sizeof(prms->screen1));
	slion_pstr("screen2", prms->screen2, sizeof(prms->screen2));
	slion_pstr("\nin_xpar", prms->in_xpar, sizeof(prms->in_xpar));
	slion_pstr("lv_xpar", prms->lv_xpar, sizeof(prms->lv_xpar));
	tty_db_nomore(slion_db_printf("\npriority = %d, tr_width = %d, tbc = %d, rc_width = %d\n",
	       prms->priority, prms->tr_width, prms->tr_tbc, prms->rc_width));

	tty_db_nomore(slion_db_printf("rc_state = %x.\n", ld->rc_state));
	tty_db_nomore(slion_db_printf("o_count=%d, i_count=%d.\n", ld->o_count, ld->i_count));

	tty_db_nomore(slion_db_printf("Pacing flags:"));
	if (prms->dtrp || prms->dcdp || prms->rtsp ||
	    prms->ctsp || prms->xonp || prms->xoff || prms->xany) {
	    if (prms->dtrp) tty_db_nomore(slion_db_printf(" dtrp"));
	    if (prms->dcdp) tty_db_nomore(slion_db_printf(" dcdp"));
	    if (prms->rtsp) tty_db_nomore(slion_db_printf(" rtsp"));
	    if (prms->ctsp) tty_db_nomore(slion_db_printf(" ctsp"));
	    if (prms->xonp) tty_db_nomore(slion_db_printf(" xonp"));
	    if (prms->xoff) tty_db_nomore(slion_db_printf(" xoff"));
	    if (prms->xany) tty_db_nomore(slion_db_printf(" xany"));
	} else {
	    tty_db_nomore(slion_db_printf(" none"));
	}
	tty_db_nomore(slion_db_printf("\n"));

	if (prms->xonp || prms->xany)
	    tty_db_nomore(slion_db_printf("remote chars stop: 0%o start: 0%o\n", prms->rxoc,
		   prms->rxac));
	if (prms->xoff)
	    tty_db_nomore(slion_db_printf("local chars stop: 0%o start: 0%o\n", prms->lxoc,
		   prms->lxac));
	
	if (prms->gcomm_mask) {
	    tty_db_nomore(slion_db_printf("gcomm_mask: "));
	    tty_db_nomore(slion_prbits(prms->gcomm_mask, gcomm_commands, 12));
	}
    }

#if defined(_KERNEL) && !defined(IN_TTYDBG)
    ATT_BD(ld);

    StartPio(slion_pio, ld, DET_BD(); return 0);
    bb = RD_SHORT(&ld->tr_buf->buf_beg);
    be = RD_SHORT(&ld->tr_buf->buf_end);
    bh = RD_SHORT(&ld->tr_buf->buf_head);
    bt = RD_SHORT(&ld->tr_buf->buf_tail);
    EndPio();
    slion_db_printf("tr_buf: beg=%04x end=%04x head=%04x tail=%04x.\n",
	    bb, be, bh, bt);

    if (tp->t_channel == CHAN_XPAR)
	slion_db_printf("rc_buf: XPAR!\n");
    else {
	StartPio(slion_pio, ld, DET_BD(); return 0);
	bb = RD_SHORT(&ld->rc_buf->buf_beg);
	be = RD_SHORT(&ld->rc_buf->buf_end);
	bh = RD_SHORT(&ld->rc_buf->buf_head);
	bt = RD_SHORT(&ld->rc_buf->buf_tail);
	EndPio();
	slion_db_printf("rc_buf: beg=%04x end=%04x head=%04x tail=%04x.\n",
		bb, be, bh, bt);
    }

    DET_BD();
#endif /* _KERNEL && !IN_TTYDBG */

    return 0;
} /* slion_print_data */

/*
 * FUNCTION:	slion_pstr
 *
 * PURPOSE:	Printout the specified string.
 *
 * INPUT:
 *      name:   Pointer to  header name string. 
 *      from:   Pointer to  displayed string 
 *      bcnt:   Size of the displayed string field. 
 *
 * RETURN:
 *	0:	Always.
 *
 * EXTERNAL PROCEDURES CALLED:
 *	printf  
 *
 * DESCRIPTION:
 *	This function print the contents of the specified string(from)
 *	followed by header(name). 
 *
 * CALLED BY:
 *	slion_print_data
 */
static 
slion_pstr(char *name, char *from, int bcnt)
{
	int c;
	char *to = from + bcnt;

	slion_db_printf("%s=`", name);
	while (from < to && --bcnt >= 0) 
	{
		if ((c = *from++) == 0xff)
		    break;
		if (c & 0x80) 
		{
		    slion_db_printf("M");
		    --bcnt;
		    c &= 0x7f;
		}
		if (c < ' ' || c == 0x7f) 
		{
		    slion_db_printf("^");
		    --bcnt;
		    c ^= 0x40;
		}
		slion_db_printf("%c", c);
    	}
    	slion_db_printf("' ");
} /* slion_pstr */

/*
 * FUNCTION:	slion_prbits
 *
 * PURPOSE:	Printout the flag status.
 *
 * INPUT:
 *      word:   Pointer to the exmined field.
 *      np:     Pointer to names structure.
 *      ind:    Printed field width.
 *
 * RETURN:	0 on success,
 *		return code from tty_db_nomore() macro otherwise
 *
 * EXTERNAL PROCEDURES CALLED:
 *	printf  
 *
 * DESCRIPTION:
 *	This function print the flag status of the specified field(word)
 *	based on the name structure(np). 
 *
 * CALLED BY:
 *	slion_print_data
 */
static int 
slion_prbits(unsigned word, struct names *np, int ind)
{
	register int col;

	col = ind;

	for (; np->n_str != 0; np++) 
	{
		if ((word & np->n_mask) == np->n_state) 
		{
		    if (col + strlen(np->n_str) + 1 > 79) 
		    {
			tty_db_nomore(slion_db_printf("\n%*s", ind, ""));
			col = ind;
		    } else {
			slion_db_printf(" ");
			col++;
		    }
		    slion_db_printf(np->n_str);
		    col += strlen(np->n_str);
		}
	}
	return(0);
} /* slion_prbits */

