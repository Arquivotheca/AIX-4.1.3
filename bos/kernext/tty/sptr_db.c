#ifndef lint
static char sccsid[] = "@(#)58 1.3 src/bos/kernext/tty/sptr_db.c, sysxtty, bos411, 9428A410j 6/10/94 10:06:57";
#endif
/*
 * COMPONENT_NAME: sysxtty (sptr_db extension for tty debugging)
 * 
 * FUNCTIONS: sptr_print(),
 *	      sptr_config_print(), sptr_line_print(), sptr_prbits()
 *
 * ORIGINS: 83
 *
 */
/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */

#include "sptr.h"

/* function definitions of sptr_db extension for tty debugging */

#ifdef IN_TTYDBG
int sptr_print();
#endif	/* IN_TTYDBG */

#if defined(_KERNEL) && defined(IN_TTYDBG)
#define sptr_db_printf	tty_db_printf
#else
#define sptr_db_printf	printf
#endif	/* _KERNEL && IN_TTYDBG */

int sptr_config_print(), sptr_line_print(); 
static int sptr_prbits(); 

struct names {
	char *n_str;
	unsigned n_mask;
	unsigned n_state;
};



/*
 * NAME:     sptr_config_print
 *
 * FUNCTION: Print a config structure
 *
 * RETURNS:  0 on success, return code from tty_db_nomore() macro otherwise
 *
 */

int
sptr_config_print(struct sptr_config *configp)
{

	static struct names modes[] = {
		{"plot", PLOT, PLOT},
		{"noff", NOFF, NOFF},
		{"nonl", NONL, NONL},
		{"nocl", NOCL, NOCL},
		{"notb", NOTB, NOTB},
		{"nobs", NOBS, NOBS},
		{"nocr", NOCR, NOCR},
		{"caps", CAPS, CAPS},
		{"wrap", WRAP, WRAP},
		{"fontinit", FONTINIT, FONTINIT},
		{"rpterr", RPTERR, RPTERR},
		  0,
	};

	tty_db_nomore(sptr_db_printf("modes:  "));
	tty_db_nomore(sptr_prbits(configp->modes, modes, 8));

	tty_db_nomore(sptr_db_printf("\n"));

	tty_db_nomore(sptr_db_printf("next = 0x%08x line_p = 0x%08x devno = 0x%08x\n",
		configp->next, configp->line_p, configp->devno));

	tty_db_nomore(sptr_db_printf("timout = %d ind = %d col = %d line = %d\n",
		configp->timout, configp->ind, configp->col, configp->line));
	return(0);
}

/*
 * NAME:     sptr_line_print
 *
 * FUNCTION: Print a line structure
 *
 * RETURNS:  0 on success, return code from tty_db_nomore() macro otherwise
 *
 */

int
sptr_line_print(struct line *linep)
{

	static struct names sp_modes[] = {
		{"plot", PLOT, PLOT},
		{"noff", NOFF, NOFF},
		{"nonl", NONL, NONL},
		{"nocl", NOCL, NOCL},
		{"notb", NOTB, NOTB},
		{"nobs", NOBS, NOBS},
		{"nocr", NOCR, NOCR},
		{"caps", CAPS, CAPS},
		{"wrap", WRAP, WRAP},
		{"fontinit", FONTINIT, FONTINIT},
		{"rpterr", RPTERR, RPTERR},
		  0,
	};
    
	static struct names flags[] = {
		{"openread", SP_OPEN_READ, SP_OPEN_READ},
		{"openwrite", SP_OPEN_WRITE, SP_OPEN_WRITE},
		{"iodone", SP_IODONE, SP_IODONE},
		{"initfail", SP_DUMMYSEND, SP_DUMMYSEND},
		  0,
	};
    
	static struct names write_status[] = {
		{"write_in_process", SP_WRITE_IN_PROCESS, SP_WRITE_IN_PROCESS},
		{"write_not_possible", SP_WRITE_NOT_POSSIBLE, SP_WRITE_NOT_POSSIBLE},
		{"write_timedout", SP_WRITE_TIMEDOUT, SP_WRITE_TIMEDOUT},
		{"write_finished", SP_WRITE_FINISHED, SP_WRITE_FINISHED},
		{"write_data_queued", SP_WRITE_DATA_QUEUED, SP_WRITE_DATA_QUEUED},
		{"write_data_flushed", SP_WRITE_DATA_FLUSHED, SP_WRITE_DATA_FLUSHED},
		  0,
	};
    
	static struct names mp_stored_status[] = {
		{"output_data_stored", SP_OUTPUT_DATA_STORED, SP_OUTPUT_DATA_STORED},
		{"ioctl_wreq_stored", SP_IOCTL_WREQ_STORED, SP_IOCTL_WREQ_STORED},
		{"ioctl_lprseta_stored", SP_IOCTL_LPRSETA_STORED, SP_IOCTL_LPRSETA_STORED},
		{"ioctl_lprgeta_stored", SP_IOCTL_LPRGETA_STORED, SP_IOCTL_LPRGETA_STORED},
		  0,
	};
    
	static struct names sp_cts[] = {
		{"cts_on", SP_CTS_ON, SP_CTS_ON},
		{"cts_off1", SP_CTS_OFF1, SP_CTS_OFF1},
		{"cts_off1", SP_CTS_OFF2, SP_CTS_OFF2},
		  0,
	};

	tty_db_nomore(sptr_db_printf("sp_modes:  "));
	tty_db_nomore(sptr_prbits(linep->sp_modes, sp_modes, 8));

	tty_db_nomore(sptr_db_printf("\nflags: "));
	tty_db_nomore(sptr_prbits((unsigned)linep->flags, flags, 8));

	tty_db_nomore(sptr_db_printf("\nwrite_status: "));
	tty_db_nomore(sptr_prbits((unsigned)linep->write_status, write_status, 8));

	tty_db_nomore(sptr_db_printf("\nmp_stored_status: "));
	tty_db_nomore(sptr_prbits((unsigned)linep->mp_stored_status, mp_stored_status, 8));

	tty_db_nomore(sptr_db_printf("\nsp_cts: "));
	tty_db_nomore(sptr_prbits((unsigned)linep->sp_cts, sp_cts, 8));

	tty_db_nomore(sptr_db_printf("\n"));

	tty_db_nomore(sptr_db_printf("config_p = 0x%08x sendff_mp = 0x%08x\n", linep->config_p,
		linep->sendff_mp));

	tty_db_nomore(sptr_db_printf("write_mdata_mp = 0x%08x write_mioctl_mp = 0x%08x\n",
		linep->write_mdata_mp, linep->write_mioctl_mp));

	tty_db_nomore(sptr_db_printf("mioctl_mp = 0x%08x input_data_mp = 0x%08x\n",
		linep->mioctl_mp, linep->input_data_mp));

	tty_db_nomore(sptr_db_printf("ind = %d col = %d line = %d v_timout = %d ihog = %d\n",
		linep->ind, linep->col, linep->line, linep->v_timout, linep->ihog));

	tty_db_nomore(sptr_db_printf("ccc = %d mcc = %d mlc = %d chars_sent = %d\n", linep->ccc,
		linep->mcc, linep->mlc, linep->chars_sent));

	tty_db_nomore(sptr_db_printf("sp_timerset = %d, sp_timerid = %d, sp_timeout = %d, sp_ctsid = %d\n",
		linep->sp_timerset, linep->sp_timerid, linep->sp_timeout,
		linep->sp_ctsid));

	tty_db_nomore(sptr_db_printf("wbid = %d rbid = %d wtid = %d rtid = %d\n",
		linep->wbid , linep->rbid , linep->wtid , linep->rtid ));

	return(0);
}

/*
 * NAME:     sptr_prbits
 *
 * FUNCTION: Print a word of bits according to the names table.  Keep track of
 *	     current column, and if line width exceeded, go to new line and
 *	     indent by the given amount.
 *
 * RETURNS:  0 on success, return code from tty_db_nomore() macro otherwise
 *
 */

static int
sptr_prbits(unsigned word, struct names *np, int ind)
{
    register int col;

    col = ind;

    for (; np->n_str != NULL; np++) {
	if ((word & np->n_mask) == np->n_state) {
	    if (col + strlen(np->n_str) + 1 > 79) {
		tty_db_nomore(sptr_db_printf("\n%*s", ind, ""));
		col = ind;
	    } else {
		sptr_db_printf(" ");
		col++;
	    }
	    sptr_db_printf(np->n_str);
	    col += strlen(np->n_str);
	}
    }
    return(0);
}

#ifdef IN_TTYDBG
/*
 * NAME:     sptr_print
 *
 * FUNCTION: Print a line structure
 *
 * RETURNS:  0 on success, -1 on failure.
 *
 */

int
sptr_print(struct line *linep, int arg)
{
	struct line line;
	struct sptr_config config;
	int res;

	if (tty_read_mem(linep, &line, sizeof(struct line)) == 0) {
		if (res = sptr_line_print(&line))
			return(res);
	} else {
		return(-1);
	}
	
	if ((arg & TTYDBG_ARG_V) == TTYDBG_ARG_V) {
		if (tty_read_mem(line.config_p, &config, sizeof(struct sptr_config)) == 0) {
			if (res = sptr_config_print(&config))
				return(res);
		} else {
			return(-1);
		}
	}
	return(0);
}
#endif	/* IN_TTYDBG */
