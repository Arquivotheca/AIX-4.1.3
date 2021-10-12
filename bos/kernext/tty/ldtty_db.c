#ifndef lint
static char sccsid[] = "@(#)56 1.7 src/bos/kernext/tty/ldtty_db.c, sysxldterm, bos412, 9447A 11/11/94 14:27:34";
#endif
/*
 *  
 * COMPONENT_NAME: sysxtty (ldtty_db extension for ldterm debugging).
 *  
 * FUNCTIONS: ldtty_print(), ldtty_prtbits(), ldtty_dobit(), ldtty_dochar().
 *  
 * ORIGINS: 27, 83
 *  
 */
/*
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */


#include "ldtty.h"

/* function definitions	*/
int	ldtty_print();
static	void ldtty_prbits();
static	int  ldtty_dobit();
static	char *ldtty_dochar();

#if defined(_KERNEL) && defined(IN_TTYDBG)
#define ldtty_db_printf      tty_db_printf
#else
#define ldtty_db_printf      printf
#endif  /* _KERNEL && IN_TTYDBG */

struct names {
        char *n_str;
        unsigned n_mask;
        unsigned n_state;
};

/*
 * NAME:	ldtty_prtbits
 * 
 * FUNCTION:	print a word of bits according to the names table. Keep track of
 *		the current column, and if line width exceeded, go to new line
 *		and indent by the given amount.
 *
 * RETURNS:	0 on success, return code from tty_db_nomore() macro otherwise
 */
static int
ldtty_prtbits(unsigned word, struct names *np, int ind)
{
	register int col;

	col = ind;

    for (; np->n_str != NULL; np++) {
        if ((word & np->n_mask) == np->n_state) {
            if (col + strlen(np->n_str) + 1 > 79) {
                tty_db_nomore(ldtty_db_printf("\n%*s", ind, ""));
                col = ind;
            } else {
                tty_db_nomore(ldtty_db_printf(" "));
                col++;
            }
            ldtty_db_printf(np->n_str);
            col += strlen(np->n_str);
        }
    }
    return(0);
}

static	int
ldtty_dobit(flag, s, col)
	uint	flag;
	char	*s;
	int	col;
{
	int l;

	if (flag) {
		l = strlen(s) + 1;
		if ((col += l) > 70) {
			tty_db_nomore(ldtty_db_printf("\n	"));
			col = l + 8;
		}
		tty_db_nomore(ldtty_db_printf(" %s", s));
	}
	return(col);
}

static	char	*
ldtty_dochar(c)
	char	c;
{
	static char buf[8];
	char	*p = buf;

	if (c < ' ' || c == 127) {
		*p++ = '^';
		c ^= '@';
	}
	*p++ = c;
	*p = 0;
	return(buf);
}


/*
 * NAME:	ldtty_print
 * 
 * FUNCTION:	print a ldtty structure
 *
 * RETURNS:	0 if success
 *		-1 or return code from tty_db_nomore() if failure.
 *
 */
int
ldtty_print(struct ldtty *tp, int arg)
{
	struct	ldtty	buftty;
	int res;
	
	static struct names state[] = {
		{"timeout", TS_TIMEOUT, TS_TIMEOUT},
		{"setneeded", TS_SETNEEDED, TS_SETNEEDED},
		{"isopen", TS_ISOPEN, TS_ISOPEN},
		{"rawbackup", TS_RAWBACKUP, TS_RAWBACKUP},
		{"carr_on", TS_CARR_ON, TS_CARR_ON},
		{"nocanon", TS_NOCANON, TS_NOCANON},
		{"waitoutbuf", TS_WAITOUTBUF, TS_WAITOUTBUF},
		{"waitoutput", TS_WAITOUTPUT, TS_WAITOUTPUT},
		{"ttstop", TS_TTSTOP, TS_TTSTOP},
		{"vtime_flag", TS_VTIME_FLAG, TS_VTIME_FLAG},
		{"tblock", TS_TBLOCK, TS_TBLOCK},
		{"waiteuc", TS_WAITEUC, TS_WAITEUC},
		{"asleep", TS_ASLEEP, TS_ASLEEP},
		{"typen", TS_TYPEN, TS_TYPEN},
		{"ondelay", TS_ONDELAY, TS_ONDELAY},
		{"bksl", TS_BKSL, TS_BKSL},
		{"mbenabled", TS_MBENABLED, TS_MBENABLED},
		{"minsat", TS_MINSAT, TS_MINSAT},
		{"erase", TS_ERASE, TS_ERASE},
		{"lnch", TS_LNCH, TS_LNCH},
		{"cnttb", TS_CNTTB, TS_CNTTB},
		{"intimeout", TS_INTIMEOUT, TS_INTIMEOUT},
		  0,
	};

	static struct names hflag[] = {
		{"rtsxoff", RTSXOFF, RTSXOFF},
		{"ctsxon", CTSXON, CTSXON},
		{"dtrxoff", DTRXOFF, DTRXOFF},
		{"cdxon", CDXON, CDXON},
		 0,
	};

	static struct names sflag[] = {
		{"dtropen", DTR_OPEN, DTR_OPEN},
		{"wtopen", WT_OPEN, WT_OPEN},
		{"riopen", RI_OPEN, RI_OPEN},
		 0,
	};

	static	struct	names	iflag[] = {
		{"ignbrk", IGNBRK, IGNBRK},
		{"brkint", BRKINT, BRKINT},
		{"ignpar", IGNPAR, IGNPAR},
		{"parmrk", PARMRK, PARMRK},
		{"inpck", INPCK, INPCK},
		{"istrip", ISTRIP, ISTRIP},
		{"inlcr", INLCR, INLCR},
		{"igncr", IGNCR, IGNCR},
		{"icrnl", ICRNL, ICRNL},
		{"ixon", IXON, IXON},
		{"ixoff", IXOFF, IXOFF},
#ifdef	_XOPEN_SOURCE
		{"iuclc", IUCLC, IUCLC},
		{"ixany", IXANY, IXANY},
#ifdef	_ALL_SOURCE
		{"imaxbel", IMAXBEL, IMAXBEL},
#endif	/* _ALL_SOURCE */
#endif	/* _XOPEN_SOURCE */
		0,
	};

	static struct names	oflag[] = {
		{"opost", OPOST, OPOST},
#ifdef _XOPEN_SOURCE
		{"olcuc", OLCUC, OLCUC},
		{"onlcr", ONLCR, ONLCR},
		{"ocrnl", OCRNL, OCRNL},
		{"onocr", ONOCR, ONOCR},
		{"onlret", ONLRET, ONLRET},
		{"ofill", OFILL, OFILL},
		{"ofdel", OFDEL, OFDEL},
		{"cr0", CRDLY, CR0},
		{"cr1", CRDLY, CR1},
		{"cr2", CRDLY, CR2},
		{"cr3", CRDLY, CR3},
		{"tab0", TABDLY, TAB0},
		{"tab1", TABDLY, TAB1},
		{"tab2", TABDLY, TAB2},
		{"tab3", TABDLY, TAB3},
		{"bs0", BSDLY, BS0},
		{"bs1", BSDLY, BS1},
		{"ff0", FFDLY, FF0},
		{"ff1", FFDLY, FF1},
		{"nl0", NLDLY, NL0},
		{"nl1", NLDLY, NL1},
		{"vt0", VTDLY, VT0},
		{"vt1", VTDLY, VT1},
#ifdef	_ALL_SOURCE
		{"oxtabs", OXTABS, OXTABS},
		{"onoeot", ONOEOT, ONOEOT},
#endif	/* _ALL_SOURCE */
#endif	/* _XOPEN_SOURCE */
		0,
	};

	static struct names	cflag[] = {
		{"b50/", CIBAUD, B50<<IBSHIFT},
		{"b75/", CIBAUD, B75<<IBSHIFT},
		{"b110/", CIBAUD, B110<<IBSHIFT},
		{"b134/", CIBAUD, B134<<IBSHIFT},
		{"b150/", CIBAUD, B150<<IBSHIFT},
		{"b200/", CIBAUD, B200<<IBSHIFT},
		{"b300/", CIBAUD, B300<<IBSHIFT},
		{"b600/", CIBAUD, B600<<IBSHIFT},
		{"b1200/", CIBAUD, B1200<<IBSHIFT},
		{"b1800/", CIBAUD, B1800<<IBSHIFT},
		{"b2400/", CIBAUD, B2400<<IBSHIFT},
		{"b4800/", CIBAUD, B4800<<IBSHIFT},
		{"b9600/", CIBAUD, B9600<<IBSHIFT},
		{"b19200/", CIBAUD, B19200<<IBSHIFT},
		{"b38400/", CIBAUD, B38400<<IBSHIFT},
		{"b0", CBAUD, B0},
		{"b50", CBAUD, B50},
		{"b75", CBAUD, B75},
		{"b110", CBAUD, B110},
		{"b134", CBAUD, B134},
		{"b150", CBAUD, B150},
		{"b200", CBAUD, B200},
		{"b300", CBAUD, B300},
		{"b600", CBAUD, B600},
		{"b1200", CBAUD, B1200},
		{"b1800", CBAUD, B1800},
		{"b2400", CBAUD, B2400},
		{"b4800", CBAUD, B4800},
		{"b9600", CBAUD, B9600},
		{"b19200", CBAUD, B19200},
		{"b38400", CBAUD, B38400},
		{"cs5", CSIZE, CS5},
		{"cs6", CSIZE, CS6},
		{"cs7", CSIZE, CS7},
		{"cs8", CSIZE, CS8},
		{"cstopb", CSTOPB, CSTOPB},
		{"cread", CREAD, CREAD},
		{"parenb", PARENB, PARENB},
		{"parodd", PARODD, PARODD},
		{"hupcl", HUPCL, HUPCL},
		{"clocal", CLOCAL, CLOCAL},
		{"parext", PAREXT, PAREXT},
		0,
	};


	static struct names	lflag[] = {
		{"isig", ISIG, ISIG},
		{"icanon", ICANON, ICANON},
#ifdef	_XOPEN_SOURCE
		{"xcase", XCASE, XCASE},
#endif	/* _XOPEN_SOURCE */
		{"echo", ECHO, ECHO},
		{"echoe", ECHOE, ECHOE},
		{"echok", ECHOK, ECHOK},
		{"echonl", ECHONL, ECHONL},
		{"noflsh", NOFLSH, NOFLSH},
		{"tostop", TOSTOP, TOSTOP},
#ifdef	_ALL_SOURCE
		{"echoctl", ECHOCTL, ECHOCTL},
		{"echoprt", ECHOPRT, ECHOPRT},
		{"echoke", ECHOKE, ECHOKE},
		{"flusho", FLUSHO, FLUSHO},
		{"altwerase", ALTWERASE, ALTWERASE},
		{"pendin", PENDIN, PENDIN},
#endif /* _ALL_SOURCE */
		{"iexten", IEXTEN, IEXTEN},
		0,
	};

	static	struct	names	allflags[] = {
		{"tandem", TANDEM, TANDEM},
		{"cbreak", CBREAK, CBREAK},
		{"lcase", LCASE, LCASE},
		{"echo", ECHO, ECHO},
		{"crmod", CRMOD, CRMOD},
		{"raw", RAW, RAW},
		{"oddp", ODDP, ODDP},
		{"evenp", EVENP, EVENP},
		{"anyp", ANYP, ANYP},
		{"cr0", CRDELAY, CR0},
		{"cr1", CRDELAY, CR1},
		{"cr2", CRDELAY, CR2},
		{"cr3", CRDELAY, CR3},
		{"tab0", TBDELAY, TAB0},
		{"tab1", TBDELAY, TAB1},
		{"tab2", TBDELAY, TAB2},
		{"bs0", BSDELAY, BS0},
		{"bs1", BSDELAY, BS1},
		{"ff0", VTDELAY, FF0},
		{"ff1", VTDELAY, FF1},
		{"nl0", NLDELAY, NL0},
		{"nl1", NLDELAY, NL1},
		{"nl2", NLDELAY, NL2},
		{"nl3", NLDELAY, NL3},
		{"tostop", TOSTOP, TOSTOP},
		{"prtera", PRTERA, PRTERA},
		{"crtera", CRTERA, CRTERA},
		{"tilde", TILDE, TILDE},
		{"flusho", FLUSHO, FLUSHO},
		{"litout", LITOUT, LITOUT},
		{"crtbs", CRTBS, CRTBS},
		{"mdmbuf", MDMBUF, MDMBUF},
		{"nohang", NOHANG, NOHANG},
		{"l001000", L001000, L001000},
		{"crtkil", CRTKIL, CRTKIL},
		{"pass8", PASS8, PASS8},
		{"ctlech", CTLECH, CTLECH},
		{"pendin", PENDIN, PENDIN},
		{"decctq", DECCTQ, DECCTQ},
		{"noflush", NOFLUSH, NOFLUSH},
		0,
	};

	int	col = 8;

	if (tty_read_mem(tp, &buftty, sizeof(struct ldtty)) == 0) {

/*
 * Part of the ldtty structure in all cases. 
 */

		tty_db_nomore(ldtty_db_printf("intrc:%s ", ldtty_dochar(buftty.t_cc[VINTR])));
		tty_db_nomore(ldtty_db_printf("quitc:%s ", ldtty_dochar(buftty.t_cc[VQUIT])));
		tty_db_nomore(ldtty_db_printf("erase:%s ", ldtty_dochar(buftty.t_cc[VERASE])));
		tty_db_nomore(ldtty_db_printf("kill:%s ", ldtty_dochar(buftty.t_cc[VKILL])));
		tty_db_nomore(ldtty_db_printf("eofc:%s ", ldtty_dochar(buftty.t_cc[VEOF])));
		tty_db_nomore(ldtty_db_printf("eol:%s ", ldtty_dochar(buftty.t_cc[VEOL])));
		tty_db_nomore(ldtty_db_printf("eol2:%s ", ldtty_dochar(buftty.t_cc[VEOL2])));
		tty_db_nomore(ldtty_db_printf("startc:%s ", ldtty_dochar(buftty.t_cc[VSTART])));
		tty_db_nomore(ldtty_db_printf("\n"));
		tty_db_nomore(ldtty_db_printf("stopc:%s ", ldtty_dochar(buftty.t_cc[VSTOP])));
		tty_db_nomore(ldtty_db_printf("suspc:%s ", ldtty_dochar(buftty.t_cc[VSUSP])));
		tty_db_nomore(ldtty_db_printf("dsuspc:%s ", ldtty_dochar(buftty.t_cc[VDSUSP])));
		tty_db_nomore(ldtty_db_printf("rprntc:%s ", ldtty_dochar(buftty.t_cc[VREPRINT])));
		tty_db_nomore(ldtty_db_printf("flushc:%s ", ldtty_dochar(buftty.t_cc[VDISCRD])));
		tty_db_nomore(ldtty_db_printf("werasc:%s ", ldtty_dochar(buftty.t_cc[VWERSE])));
		tty_db_nomore(ldtty_db_printf("lnextc:%s ", ldtty_dochar(buftty.t_cc[VLNEXT])));

		tty_db_nomore(ldtty_db_printf("\ninput:  ")); tty_db_nomore(ldtty_prtbits(buftty.t_iflag, iflag, 8));
		tty_db_nomore(ldtty_db_printf("\noutput:  ")); tty_db_nomore(ldtty_prtbits(buftty.t_oflag, oflag, 8));
		tty_db_nomore(ldtty_db_printf("\ncntrl:  ")); tty_db_nomore(ldtty_prtbits(buftty.t_cflag, cflag, 8));
		tty_db_nomore(ldtty_db_printf("\ndispln:  ")); tty_db_nomore(ldtty_prtbits(buftty.t_lflag, lflag, 8));

		tty_db_nomore(ldtty_db_printf("\nstate: "));
		tty_db_nomore(ldtty_prtbits(buftty.t_state, state, 8));

		tty_db_nomore(ldtty_db_printf("\ncol = %d, rocount = %d, rocol = %d\n",
		       buftty.t_col, buftty.t_rocount, buftty.t_rocol));

		tty_db_nomore(ldtty_db_printf("\nreadqp = 0x%08x, rawbufp = 0x%08x, rawtailp = 0x%08x",
		       buftty.t_queue, buftty.t_rawbuf, buftty.t_rawtail));
		tty_db_nomore(ldtty_db_printf("\nunsentp = 0x%08x, outbufp = 0x%08x",
		       buftty.t_unsent, buftty.t_outbuf));
		tty_db_nomore(ldtty_db_printf("\nrawcc = %d, unsent_ndx = %d, shcc = %d\n",
		       buftty.t_rawcc, buftty.t_unsent_ndx, buftty.t_shcc));

		tty_db_nomore(ldtty_db_printf("\ntermiox: "));
		tty_db_nomore(ldtty_prtbits(buftty.t_control.x_hflag, hflag, 8));
		tty_db_nomore(ldtty_prtbits(buftty.t_control.x_sflag, sflag, 8));

		tty_db_nomore(ldtty_db_printf("\nihog = %d, ohog = %d", buftty.t_ihog, buftty.t_ohog));
		tty_db_nomore(ldtty_db_printf("\n"));
/*
 * Part of the ldtty structure to print in case of verbose.
 */
		if ((arg & TTYDBG_ARG_V) == TTYDBG_ARG_V) {
			tty_db_nomore(ldtty_db_printf("\nsparebufp = 0x%08x", buftty.t_sparebuf));
			tty_db_nomore(ldtty_db_printf("\nintimeout_posted = %d, intimeout = %d",
		       		buftty.t_intimeout_posted, 
				buftty.t_intimeout));
			tty_db_nomore(ldtty_db_printf("\noutbid = %d, ackbid = %d, hupbid = %d",
		       		buftty.t_outbid, buftty.t_ackbid, 
				buftty.t_hupbid));
			tty_db_nomore(ldtty_db_printf("\nwinsize: "));
			tty_db_nomore(ldtty_db_printf("rows = %d, cols = %d, xpixel = %d, ypixel = %d",
				buftty.t_winsize.ws_row, 
				buftty.t_winsize.ws_col,
				buftty.t_winsize.ws_xpixel,
				buftty.t_winsize.ws_ypixel));
			
			tty_db_nomore(ldtty_db_printf("\nuser_input:  "));
			tty_db_nomore(ldtty_prtbits(buftty.u_termios.c_iflag, iflag, 8));
			tty_db_nomore(ldtty_db_printf("\nuser_output:  "));
			tty_db_nomore(ldtty_prtbits(buftty.u_termios.c_oflag, oflag, 8));
			tty_db_nomore(ldtty_db_printf("\nuser_cntrl:  "));
			tty_db_nomore(ldtty_prtbits(buftty.u_termios.c_cflag, cflag, 8));
			tty_db_nomore(ldtty_db_printf("\nuser_displn:  "));
			tty_db_nomore(ldtty_prtbits(buftty.u_termios.c_lflag, lflag, 8));
			tty_db_nomore(ldtty_db_printf("\ndriver_input:  "));
			tty_db_nomore(ldtty_prtbits(buftty.d_termios.c_iflag, iflag, 8));
			tty_db_nomore(ldtty_db_printf("\ndriver_output:  "));
			tty_db_nomore(ldtty_prtbits(buftty.d_termios.c_oflag, oflag, 8));
			tty_db_nomore(ldtty_db_printf("\ndriver_cntrl:  "));
			tty_db_nomore(ldtty_prtbits(buftty.d_termios.c_cflag, cflag, 8));
			tty_db_nomore(ldtty_db_printf("\ndriver_displn:  "));
			tty_db_nomore(ldtty_prtbits(buftty.d_termios.c_lflag, lflag, 8));
			
			tty_db_nomore(ldtty_db_printf("\ncompat_flags:  "));
			tty_db_nomore(ldtty_prtbits(buftty.t_flags, allflags, 8));
			tty_db_nomore(ldtty_db_printf("\norig_ioctl_cmd = 0x%08x", 
				buftty.t_ioctl_cmd));
			tty_db_nomore(ldtty_db_printf("\norig_ioctl_datap = 0x%08x, qioctlp = 0x%08x", 
				buftty.t_ioctl_data, buftty.t_qioctl));
			tty_db_nomore(ldtty_db_printf("\nshad_time = 0x%08x", buftty.t_shad_time));

                        tty_db_nomore(ldtty_db_printf("\nt_cswidth = "));
                        tty_db_nomore(ldtty_db_printf("%d : %d, ", buftty.t_cswidth.eucw[1],
                                            buftty.t_cswidth.scrw[1]));
                        tty_db_nomore(ldtty_db_printf("%d : %d, ", buftty.t_cswidth.eucw[2],
                                            buftty.t_cswidth.scrw[2]));
                        tty_db_nomore(ldtty_db_printf("%d : %d ", buftty.t_cswidth.eucw[3],
                                           buftty.t_cswidth.scrw[3]));

			tty_db_nomore(ldtty_db_printf("\ncodeset = %d, eucleft = %d, eucind = %d",
				buftty.t_codeset, buftty.t_eucleft,
				buftty.t_eucind));
			tty_db_nomore(ldtty_db_printf("\nout_codeset = %d, out_eucleft = %d",
				buftty.t_codeset, buftty.t_eucleft));
			tty_db_nomore(ldtty_db_printf(", out_eucind = %d", buftty.t_out_eucind));
			tty_db_nomore(ldtty_db_printf("\n"));
		}
	} else
		return(-1);
	return(0);
}
