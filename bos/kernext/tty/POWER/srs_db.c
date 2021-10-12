#ifndef lint
static char sccsid[] = "@(#)61 1.4 src/bos/kernext/tty/POWER/srs_db.c, sysxtty, bos411, 9428A410j 6/10/94 10:08:12";
#endif
/*
 *  
 * COMPONENT_NAME: sysxtty
 *  
 * FUNCTIONS: srs_bits srs_print srs_print_help srs_kdb srs_print_details
 *  
 * ORIGINS: 83
 *  
 */
/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */

/*
 * These are print utilities for debugging RS.
 */

#include "srs.h"

#ifndef   DB_TTY

#if defined(_KERNEL) && defined(IN_TTYDBG)
#define srs_db_printf	tty_db_printf
#else
#define srs_db_printf	printf
#endif  /* _KERNEL && IN_TTYDBG */

#if defined(_KERNEL) && !defined(IN_TTYDBG)
extern void srs_pio_catch(rymp_t rsp, pud_t *pud, int logit);
#endif /* _KERNEL && !IN_TTYDBG */
/*
 * Redefine a "light" adap_info array here to get apapter information */
struct adap_info_db {
    enum adap_type ra_type;        /* type of adapter */
    char *ra_name;                 /* name for debugging */
};

struct adap_info_db sadap_infos_db[] = {
    { NativeA_SIO, "native i/o" },
    { NativeB_SIO, "native i/o" },
    { NativeA_SIO1, "native i/o" },
    { NativeB_SIO1, "native i/o" },
    { NativeA_SIO2, "native i/o" },
    { NativeB_SIO2, "native i/o" },
    { NativeA_SIO3, "native i/o" },
    { NativeB_SIO3, "native i/o" },
    { NativeC_SIO3, "native i/o" },
    { Eight_232, "rs-232 8port" },
    { Eight_422, "EIA-530 8port" },
    { Eight_Mil, "MIL-188 8port" },
    { Sixteen_232, "16 port 232" },
    { Sixteen_422, "16 port 422" },
    { Unknown_Adap, "Unknown" },
};

void srs_bits(int f, char **p);

void
srs_print_help()
{
    printf("\nUsage:\tcall srs_kdb v/x @(str_rs struct)\n");
}

int
srs_print(str_rsp_t ktp)
{
    struct str_rs tp;
    struct rs_ym rsp;
    int res = 0;

    if (tty_read_mem(ktp, &tp, sizeof(struct str_rs))) {
        return(-1);    /* what else? */
    }

    if (tty_read_mem(tp.t_hptr, &rsp, sizeof(struct rs_ym))) {
        return(-1);    /* what else? */
    }
    
    tp.t_hptr = &rsp;
    res = srs_print_details(&tp, 0); /* never verbose!! */
    return(res);
}

    
#ifdef     _KERNEL
        
/*
 * srs_kdb
 *    Print function for kdb debugger.
 *      Converts the string to parameter list,
 *      Prints some local information,
 *      Calls the lldb print function srs_print.
 *
 * ARGUMENTS:
 *    buf    : kdb sub-command line.
 *    poff    : index in buf string.
 *    len    : length of buf.
 *    
 */
void
srs_kdb(unsigned char *buf, int *poff, int len)

{
    int ch;
    long tp;
    int v;
	
    /*
     * Read all blanks between function name and first parameter
     */
    while ((len - *poff) > 0 && (buf[*poff] == ' ')) (*poff)++;
    ch = buf[*poff] & 0xFF;
    (*poff)++;
    
    /*
     * Remove all blanks between first and second parameters
     */
    while ((len - *poff) > 0 && (buf[*poff] == ' ')) (*poff)++;
    if ((len - *poff) <= 0) {
        srs_print_help();
        return;
    }
    v = (ch == 'v');
    tp = mi_strtol (buf + *poff, NULL, 16);
    srs_print_details((str_rsp_t) tp, v);
    return;
}
#endif     /* _KERNEL */

int 
srs_print_details(str_rsp_t tp, int v)
{
#define SRS_PRINT
#ifdef SRS_PRINT
#if defined(_KERNEL) && !defined(IN_TTYDBG)
    DoingPio;
#endif /* _KERNEL && !IN_TTYDBG */
    static char *mdm_status[8] = {
		" delta CTS",
		" delta DSR",
		" Ring Fell",
		" delta CD",
		" CTS",
		" DSR",
		" RI",
		" DCD",
    };
    rymp_t rsp = tp->t_hptr;
    int old_lcr;
#if defined(_KERNEL) && !defined(IN_TTYDBG)
    struct srs_port *port;
#endif /* _KERNEL && !IN_TTYDBG */
    struct adap_info_db *adap;
    char l_rbr, l_iir, l_lsr, l_msr, l_ier;
	
    for (adap = sadap_infos_db;
		 adap->ra_type != Unknown_Adap && adap->ra_type != rsp->ry_type;
		 ++adap);
    tty_db_nomore(srs_db_printf("%s adapter in slot %d at slih level %d\n", adap->ra_name,
		   rsp->ry_slot, rsp->ry_level));
    tty_db_nomore(srs_db_printf("seg:0x%08x port:0x%08x iseg:0x%08x ibase:0x%08x\n",
		   rsp->ry_intr.bid, rsp->ry_port, rsp->ry_iseg, rsp->ry_ibase));
	
#if defined(_KERNEL) && !defined(IN_TTYDBG)
    port = Port(rsp);
    StartPio(srs_pio_catch, rsp, io_det(port); return 0);
    old_lcr = BUSIO_GETC(&port->r_lcr);
    BUSIO_PUTC(&port->r_lcr, old_lcr&~DLAB);
    if (v) {                /* if verbose */
		l_iir = BUSIO_GETC(&port->r_iir);
		l_rbr = BUSIO_GETC(&port->r_rbr);
		l_lsr = BUSIO_GETC(&port->r_lsr);
		l_msr = BUSIO_GETC(&port->r_msr);
    }
	
    {                    /* Interrupt Enable */
		static char *temp[8] = {
			" Rx Data",
			" Tx Hold Empty",
			" Rx Line Status",
			" Modem Status",
		};
		if (l_ier = BUSIO_GETC(&port->r_ier)) {
			printf("Interrupts enabled for:");
			srs_bits(l_ier, temp);
		} else
		  printf("No interrupts enabled");
    }
	
    {                    /* Line status */
		static char *par[4] = {
			"odd",
			"even",
			"mark",
			"space",
		};
		
		printf("\n%d data bits, %s, %s parity, %s set, dlab %s",
			   5+(old_lcr&(WLS0|WLS1)),
			   old_lcr&STB ? "2 stop bits" : "1 stop bit",
			   old_lcr&PEN ? par[(old_lcr>>4)&3] : "no",
			   old_lcr&BREAK ? "break" : "no break",
			   old_lcr&DLAB ? "set" : "clear");
    }
	
    {                    /* modem control */
		static char *temp[8] = {
			" DTR",
			" RTS",
			" Out1",
			" Out2",
			" Loop",
		};
		printf("\nmodem control bits:");
		srs_bits(BUSIO_GETC(&port->r_mcr), temp);
    }
    printf("\nscratch register = 0x%2x", BUSIO_GETC(&port->r_scr));
	
    {
		static char *temp[8] = {
			" Concurrent Writes",
			" BAUDOUT selected",
			" RXRD selected",
		};
		uchar dlm, dll, afr;
		
		SR(port->r_lcr, old_lcr|DLAB);
		dlm = GR(port->r_dlm);
		dll = GR(port->r_dll);
		afr = GR(port->r_afr);
		SR(port->r_lcr, old_lcr&~DLAB);
		printf("\nDivisor = 0x%02x%02x => ", dlm, dll);
		if (dlm || dll) {
			int temp = (rsp->ry_xtal * 100) / 16 / (dlm * 256 + dll);
			printf("%d.%02d baud", temp / 100, temp % 100);
		} else
		  printf("stopped\n");
		srs_bits(afr, temp);
    }
	
    if (v) {
		static char *i_source[8] = {
			"modem status",
			"transmit empty",
			"receive data",
			"receive line status",
			0,
			0,
			"char timeout",
			0,
		};
		
		printf("\nReceive char = %x, %s pending. fifo's %s",
			   l_rbr,
			   l_iir & 1 ? "no interrupt" : i_source[(l_iir>>1)&7],
			   l_iir & 0x80 ? "enabled" : "disabled");
		{
			printf("\nModem Status:");
			srs_bits(l_msr, mdm_status);
		}
		{
			static char *temp[8] = {
				" data ready",
				" overrun",
				" parity error",
				" framing error",
				" break",
				" xmit hold empty",
				" xmit empty",
				" error in fifo",
			};
			printf("\nLine status:");
			srs_bits(l_lsr, temp);
		}
    }
    printf("\n");
#endif /* _KERNEL && !IN_TTYDBG */
	
    /*dbg_clist("slih queue:", &rsp->ry_data); */
    tty_db_nomore(srs_db_printf("%d bytes for output at 0x%x\n", rsp->ry_xcnt, rsp->ry_xdata));
    tty_db_nomore(srs_db_printf("xtal:%d rtrig:0x%x tbc:%d posted:%d xmit:%d dma:%d\n",
		   rsp->ry_xtal, rsp->ry_rtrig, rsp->ry_tbc, rsp->ry_posted,
		   rsp->ry_xmit, rsp->ry_dma));
	
    tty_db_nomore(srs_db_printf("Last Modem Status:"));
    srs_bits(rsp->ry_msr, mdm_status);
	
    tty_db_nomore(srs_db_printf("\nflags:"));
    if (rsp->ry_open)
	  tty_db_nomore(srs_db_printf(" open"));
    if (rsp->ry_watch)
	  tty_db_nomore(srs_db_printf(" watch"));
    if (rsp->ry_mil)
	  tty_db_nomore(srs_db_printf(" mil"));
    if (rsp->ry_xbox)
	  tty_db_nomore(srs_db_printf(" xbox"));
    if (rsp->ry_conf)
	  tty_db_nomore(srs_db_printf(" conf"));
    if (rsp->ry_dsleep)
	  tty_db_nomore(srs_db_printf(" dsleep"));
    {
		int i = rsp->ry_tridx;
		int j = 0;
		
		tty_db_nomore(srs_db_printf("\nlast %d iir's:", TRC_SIZE));
		do {
			if (--i < 0)
			  i += TRC_SIZE;
			tty_db_nomore(srs_db_printf(" %02x", rsp->ry_trace[i]));
		} while (i != rsp->ry_tridx);
    }
    tty_db_nomore(srs_db_printf("\n"));
	
#if defined(_KERNEL) && !defined(IN_TTYDBG)
    SR(port->r_lcr, old_lcr);        /* restore dlab */
    EndPio();
    io_det(port);
#endif /* _KERNEL && !IN_TTYDBG */
	
    tty_db_nomore(srs_db_printf("ocnt=%d icnt=%d bcnt=%d",
		   rsp->ry_ocnt, rsp->ry_icnt, rsp->ry_bcnt));
    if (rsp->ry_watched)
	  tty_db_nomore(srs_db_printf(" watched=%d", rsp->ry_watched));
    tty_db_nomore(srs_db_printf("\n"));
	
    return 0;
}

void 
srs_bits(int f, char **p)
{
    int none = 1;
	
    for (f &= 0xff ; f; f >>= 1, ++p)
	  if (f & 1 && *p) {
		  if (none)
			srs_db_printf("%s", *p);
		  else
			srs_db_printf(",%s", *p);
		  none = 0;
	  }
#endif                    /* SRS_PRINT */
}
#endif                    /* DB_TTY */

