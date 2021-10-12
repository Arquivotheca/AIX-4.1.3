static char sccsid[] = "@(#)71	1.6  src/bos/kernel/db/POWER/dbtty_dvr_rs.c, sysdb, bos41B, 412_41B_sync 12/6/94 14:35:14";

/*
 * COMPONENT_NAME: (SYSDB) Kernel Debugger
 *
 * FUNCTIONS:	db_al_lock, db_al_unlock, drs_ttyopen, drs_ttyclose,
 *		d_get, drs_ttyget, drs_ttyput, drs_ttybinput,
 *		d_ttycomput, set_POS, posr, posw, delay_ms, IO_att,
 *		IO_det, drs_tty_ischar
 *
 * ORIGINS: 27 83
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
/*
 * LEVEL 1,  5 Years Bull Confidential Information
*/

/* 
 * Machine specific interface to the async ports.  This currently is 
 * for RS/6K boxes and can talk to the native i/o ports only.
 *
 * Created to work with the debugger, uses polling, not interrupts.
 */

#include <sys/ioacc.h>			/* bus access macros */
#include <sys/adspace.h>
#include <sys/intr.h>
#include <sys/mstsave.h>
#include <sys/low.h>
#include <sys/syspest.h>
#include <sys/systemcfg.h>
#include <sys/ppda.h>
#ifdef _POWER_MP
#include <sys/ppda.h>
#include  "dbdebug.h"
#endif /* _POWER_MP */

extern ulong dbterm;			/* 0 when first time into debugger */
extern uint nio_buid;			/* variable native I/O bus unit id */
extern uint nio_slot;
extern uint nio_posdata;

struct rs_port {
    char r_rbr;				/* 0 */
#define r_thr r_rbr
#define r_dll r_rbr
    char r_ier;				/* 1 */
#define r_dlm r_ier
#define ERBDAI 0x01
#define ETHREI 0x02
#define ELSI   0x04
#define EMSSI  0x08
    char r_iir;				/* 2 */
#define r_fcr r_iir
#define FIFO_ENABLE 0x01		/* enable fifo's */
#define RFIFO_RESET 0x02		/* reset receive fifo */
#define XFIFO_RESET 0x04		/* reset xmit fifo */
#define DMA_MODE    0x08		/* use mode 1 */
#define r_afr r_iir
#define CON_WRITE 0x01			/* gang bang writes */
#define BAUDOUT   0x02			/* BAUDOUT select */
#define RXRD_SEL  0x04			/* RXRD select */
    char r_lcr;				/* 3 */
#define WLS0  0x01
#define WLS1  0x02
#define STB   0x04
#define PEN   0x08
#define EPS   0x10
#define STICK 0x20
#define BREAK 0x40
#define DLAB  0x80
    char r_mcr;				/* 4 */
#define DTR   0x01
#define RTS   0x02
#define OUT1  0x04
#define OUT2  0x08
#define LOOP  0x10
    char r_lsr;				/* 5 */
#define DR    0x01
#define OE    0x02
#define PE    0x04
#define FE    0x08
#define BI    0x10
#define THRE  0x20
#define TEMT  0x40
#define EFIFO 0x80
    char r_msr;				/* 6 */
#define DCTS  0x01
#define DDSR  0x02
#define TERI  0x04
#define DDCD  0x08
#define CTS   0x10
#define DSR   0x20
#define RI    0x40
#define DCD   0x80
    char r_scr;				/* 7 */
};
#define GR(x) BUSIO_GETC(&(x))
#define SR(x, v) BUSIO_PUTC(&(x), v)
#define set_dlab(port) SR(port->r_lcr, GR(port->r_lcr)|DLAB)
#define clr_dlab(port) SR(port->r_lcr, GR(port->r_lcr)&~DLAB)

#define	IOCC_SEG_REG		13
#define IOCC_SPACE_PTR		(IOCC_SEG_REG << 28)
#define IOCC_SEG_REG_VALUE 	0x82000080
#define COMP_RESET_REG		0x0040002c
#define COMP_RESET_REG_PPC	0x000100a0
#define SCRATCH_SEG		7
#define IOCC_BUS_DELAY		0x000000e0

/* 
 * The only machine specific parts are the RS_BUID macro and the list 
 * of port offsets (assuming that we are talking to a 16450 style port
 */
#define RS_BUID(num) (0x800C0060|(num << 20))
#define XTAL_R1 8000000
#define XTAL_R2 24000000
#ifdef _RS6K_SMP_MCA
#define XTAL_R3 1800000
#endif /* _RS6K_SMP_MCA */

static ulong offsets[] = {
    0x0030, 0x0038,
};
#define DMA_PORT 0x0041

#ifdef _POWER_MP
#define CUR_CPU db_get_processor_num()
#else
#define CUR_CPU 0
#endif


ulong d_offset;			/* set at open time */
static char d_ier;
static char d_mcr;
static char d_afr;
static char d_dma;
#define DBUF 32
static char d_buf[DBUF];
static int d_head, d_tail;

static ulong orig_segval;		/* saved so IO_det can restore */

/* These things should be configured somehow */
int d_conf_baud = 9600;
/*
static char d_conf_csize = bits8;
static char d_conf_par = nopar;
*/


/* 
 * It may be necessary to flush characters at open and close time.  It 
 * may also be necessary to do a bus reset.  I'd prefer not to do 
 * these things unless I really have to since that will only confuse 
 * more people.
 * 
 * To make ATE happy, we do not drop DTR at close time.  Also for future
 * purposes, we want to keep the port configured as it is if it is already
 * opened.  Because of this, we do not restore the baud rate, etc at close
 * time although we do restore other things.  The logic is that if it is
 * already open, we would not have changed it in the first place.  And if
 * was no open, there is no need to set things back.  If we don't do this,
 * then the second open, sees DTR high and so does not setup the baud rate.
 * Understand...?
 */

/* if you decided to do your own d_* calls instead of using the printf
 * from the debugger, the assert will catch you if you don't disable
 * interrupts first.  If interrupts are not disabled, then bad things
 * happen when the uart wakes up the real tty driver.
 * If you disagree with this, queue up a lot of output on the tty port
 * and do a kernel printf, stand back, and watch the sparks fly.
 *
 * I changed the assert to ASSERT in *open, and changed the other
 * asserts to if's.  I don't think this is a severe enough to trap the
 * machine.  If they don't call these routines at INTMAX, then the message
 * just won't print.
 */

#ifdef _POWER_MP

extern volatile struct db_lock debugger_lock;  
 
extern status_t status[];   

int db_open_line[(sizeof(offsets) / sizeof(offsets[0]))] = {0}; /* count for mp_safe printf */

static int
db_al_lock(p)
int p;
{
#ifdef _KDB
	int dbline;
	dbline = p & TTY_PORT;
	if (__kdb() && __power_mp())
		kdb_lock_line(dbline);
#endif /* _KDB */
	if (__power_mp()){
#if 0
		while (status[CUR_CPU] == running){
		while(1) {
			if (db_lock_try(&debugger_lock,cpunb))
				return TRUE;
		}
#endif
	}
	return TRUE;
}

static db_al_unlock(p)
{
	int dbline;
	if (__power_mp()){
#if 0
		if (status[CUR_CPU] != debugging)
			db_unlock(&debugger_lock);
#endif
	}
#ifdef _KDB
	dbline = p & TTY_PORT;
	if (__kdb() && __power_mp())
		kdb_unlock_line(dbline);
#endif /* _KDB */
}
	
#endif /* _POWER_MP */

drs_ttyopen(p)
int p;					/* 0 for serial a, 1 for b */
{
    register struct rs_port *d_port;
    short div;
    char temp;
    unsigned long time, otime;
    char *base_page;
    volatile int pos0, pos1;
    int save_seg;
 
    ASSERT((ppda[CUR_CPU])._csa->intpri == INTMAX);

#ifdef _POWER_MP
	if (__rs6k_smp_mca())
		if (status[CUR_CPU] == debugging)
			/* Don't use db_start_rtc - we must ALWAYS start rtc */
			pgs_rtc_start();
#endif /* _POWER_MP */
	set_POS();
#ifdef _POWER_MP
	if (__rs6k_smp_mca())
		if (status[CUR_CPU] == debugging)
			/* Don't use db_stop_rtc - we must ALWAYS stop rtc */
			pgs_rtc_stop();
#endif /* _POWER_MP */

    if (p >= (sizeof(offsets) / sizeof(offsets[0]))) {
	d_offset = 0;
	return -1;
    }
#ifdef _POWER_MP
    db_al_lock(p);
#endif /* POWER_MP */
    d_offset = offsets[p];
    base_page = (char *) IO_att(nio_buid, 0);
    d_port = (struct rs_port *)(base_page + d_offset);

#ifdef _RS6K_SMP_MCA
    if (! __rs6k_smp_mca())
    {
#endif /* _RS6K_SMP_MCA */
    d_dma = base_page[DMA_PORT];	/* get old dma status */
    base_page[DMA_PORT] = 0;		/* stop the dma */
#ifdef _RS6K_SMP_MCA
    }
#endif /* _RS6K_SMP_MCA */

    d_ier = GR(d_port->r_ier);		/* save intr. reg */
    SR(d_port->r_ier, 0);		/* Turn off interrupts */

    d_mcr = GR(d_port->r_mcr);		/* save modem control */
    SR(d_port->r_mcr, DTR|RTS);		/* raise DTR and RTS only ?? */
 
    /*
     * Read device id from pos register;  identify hardware level
     *   controlling serial ports.
     */
    save_seg = mfsr(IOCC_SEG_REG);
    mtsr(IOCC_SEG_REG,IOCC_SEG_REG_VALUE);
    mtsr(IOCC_SEG_REG,save_seg);

    set_dlab(d_port);
 
#ifdef _RS6K_SMP_MCA
	if (nio_posdata == 0xD9FE) {
		d_afr = GR(d_port->r_afr);		/* save afr junk   */
		SR(d_port->r_afr, 0);			/* zap it to 0     */
		div = (XTAL_R3 /d_conf_baud + 8) / 16;	/* compute divisor */
	}
	else
#endif /* _RS6K_SMP_MCA */ 
    if (nio_posdata == 0xE6DE) {
    	d_afr = GR(d_port->r_afr) | 0x10;	/* save afr junk   */
    	SR(d_port->r_afr, 0x10);		/* zap it to 0     */
	div = (XTAL_R2 /d_conf_baud + 8) / 16;	/* compute divisor */
	}
    else {
    	d_afr = GR(d_port->r_afr);		/* save afr junk   */
    	SR(d_port->r_afr, 0);			/* zap it to 0     */
	div = (XTAL_R1 /d_conf_baud + 8) / 16;	/* compute divisor */
	}
 
    clr_dlab(d_port);
    
    /* If port is not open (DTR not raised, we setup the baud rate and
     * the bits per char, parity, etc.  These are NOT restore at close
     * time since we do not drop DTR.
     */
    if (!dbterm || !(d_mcr & DTR)) {		/* first init after reboot or */
	set_dlab(d_port);
	SR(d_port->r_dll, div & 0xff);	/* set baud */
	SR(d_port->r_dlm, (div >> 8) & 0xff);
	clr_dlab(d_port);
	temp = WLS0|WLS1;
	SR(d_port->r_lcr, temp);	/* setup baud, etc. */
    }    

#ifdef _POWER_MP
	if (__rs6k_smp_mca())
		if (status[CUR_CPU] == debugging)
			/* Don't use db_start_rtc - we must ALWAYS start rtc */
			pgs_rtc_start();
#endif /* POWER_MP */

    /* sleep for about 1/10 of a second */

    switch (_system_configuration.rtc_type)	{

    case RTC_POWER_PC:		/* processors with time base */
    {
	int flag = 1;
	int cmp_constant;   /* approximate # tics to equal 1/10 second */

	cmp_constant = (100000000 / _system_configuration.Xint) * 
	    _system_configuration.Xfrac;

	otime = db_mftbl();
	while (flag)
	{
	    time = db_mftbl();
	    if (time < otime)		/* in case TBL wraps, we */
		otime = time;		/* just start over */
	    else
		if ((time - otime) > cmp_constant)
		    flag = 0;
	}
    }
    
	break;
	
    case RTC_POWER:		/* processors with real time clock */

    otime = (mfrtcu() * 1000000000) + mfrtcl();
    while (((time = (mfrtcu() * 1000000000) + mfrtcl()) - otime) < 100000000)
	;

    break;
    }
	
#ifdef _POWER_MP
	if (__rs6k_smp_mca())
		if (status[CUR_CPU] == debugging)
			/* Don't use db_stop_rtc - we must ALWAYS stop rtc */
			pgs_rtc_stop();
#endif /* POWER_MP */

    /* get the modem status register to check carrier detect */
    temp = GR(d_port->r_msr);
    IO_det(d_port);

#ifdef _POWER_MP
	if (temp & DCD)
		db_open_line[p]++;
    db_al_unlock(p);
#endif /* POWER_MP */
    /* If we have carrier, return true, otherwise return false */
    return ((temp & DCD) ? 1 : 0);
}

drs_ttyclose()
{
    register struct rs_port *d_port;
    char *base_page;
    
    if (!d_offset)
	return;
    if ((ppda[CUR_CPU])._csa->intpri != INTMAX)
        return;

    base_page = (char *) IO_att(nio_buid, 0);
    d_port = (struct rs_port *)(base_page + d_offset);

#ifdef _POWER_MP
    db_al_lock(dbterm);
	if (db_open_line[dbterm & TTY_PORT] > 1){
		db_open_line[dbterm & TTY_PORT]--;
		db_al_unlock(dbterm);
		return;
	}
#endif /* POWER_MP */

    while (!(GR(d_port->r_lsr)&TEMT));	/* wait for last char to clear */
    SR(d_port->r_mcr, d_mcr|DTR|RTS);	/* leave DTR and RTS high for ATE */
    set_dlab(d_port);
    SR(d_port->r_afr, d_afr);		/* restore afr junk */
    clr_dlab(d_port);
    SR(d_port->r_ier, d_ier);		/* restore interrupt enable reg */

#ifdef _RS6K_SMP_MCA
    if (! __rs6k_smp_mca())
#endif /* _RS6K_SMP_MCA */
    base_page[DMA_PORT] = d_dma;	/* restore the dma */

    IO_det(d_port);
#ifdef _POWER_MP
	db_open_line[dbterm & TTY_PORT] = 0;
    db_al_unlock(dbterm);
#endif /* _POWER_MP */
}

/* IO_att has already been done! */
static d_get(d_port)
register struct rs_port *d_port;
{
#ifdef _POWER_MP 
    int retval;
    db_al_lock(dbterm);
    if (GR(d_port->r_lsr)&DR){
	    retval = GR(d_port->r_rbr) & 0xff;
    } else
	retval = -1;
    db_al_unlock(dbterm);
    return retval;
#else /* POWER_MP */
    if (GR(d_port->r_lsr)&DR)
	return GR(d_port->r_rbr) & 0xff;
    else
	return -1;
#endif /* POWER_MP */
}

drs_ttyget()
{
    register struct rs_port *d_port;
    int c;

    if (!d_offset)
	return;
    if ((ppda[CUR_CPU])._csa->intpri != INTMAX)
        return;
    d_port = (struct rs_port *)IO_att(nio_buid, d_offset);

    if (d_head != d_tail) {
	c = d_buf[d_tail] & 0xff;
	++d_tail;
	d_tail &= (DBUF-1);
    } else
	while ((c = d_get(d_port)) == -1);
    IO_det(d_port);
    if (c == '\r')
	c = '\n';
    d_ttyput(c);				/* echo the puppy */
    return c;
}

drs_ttyput(c_out)
char c_out;
{
	d_ttycomput (c_out,0);
}

drs_ttybinput(c_out)
char c_out;
{
	d_ttycomput (c_out,1);
}

static d_ttycomput(c_out, bin)
char c_out;
int bin;
{
    register struct rs_port *d_port;
    int c, newhead;

    if (!d_offset)
	return;

    if ((ppda[CUR_CPU])._csa->intpri != INTMAX)
        return;

    if (!bin && c_out == '\n')
	d_ttyput('\r');

    d_port = (struct rs_port *)IO_att(nio_buid, d_offset);

    if ((c = d_get(d_port)) != -1)
	if (c == 'S'-'@')		/* st`op */
	    while ((c = d_get(d_port)) != 'Q'-'@');
	else {
	    newhead = (d_head + 1) & (DBUF-1);
	    if (newhead != d_tail) {
			d_buf[d_head] = c;
			d_head = newhead;
	    }
	}
	
#ifdef _POWER_MP
    db_al_lock(dbterm);
#endif /* _POWER_MP */
    while (!(GR(d_port->r_lsr)&TEMT));	/* wait for char to clear */
    SR(d_port->r_thr, c_out);
    IO_det(d_port);
#ifdef _POWER_MP
    db_al_unlock(dbterm);
#endif /* _POWER_MP */
}

/* Check to see if we entered the debugger because of a bus reset.	*/
/* If so, we need to reset the POS register for serial port 1		*/
/* Most of this code was adapted from code written by Steve Sombar,	*/
/* other pieces were lifted wholesale.					*/

static set_POS()
{
	void posw();
	int  posr();
	void delay_ms();

	int save_seg;
	int io_value;
	int pos_value, reset_value, enable_value;
	int crr_offset, crr_value;

#ifdef _RS6K_SMP_MCA
	if (__rs6k_smp_mca())
		return;
#endif /* _RS6K_SMP_MCA */

	save_seg = mfsr(IOCC_SEG_REG);
	mtsr(IOCC_SEG_REG,IOCC_SEG_REG_VALUE);

	if ( __power_pc()) 
		crr_offset = COMP_RESET_REG_PPC;
	else
		crr_offset = COMP_RESET_REG;

	io_value = *(unsigned int *)((IOCC_SPACE_PTR) + crr_offset);

#ifdef _RS6K_SMP_MCA
	if (__rs6k_smp_mca())
		crr_value = 0x80000000;
	else
#endif /* _RS6K_SMP_MCA */
		crr_value = 1;

	if (!(io_value & crr_value))
	{
		/* set the native I/O bit in the Component Reset Register */
		*(unsigned int *)((IOCC_SPACE_PTR) + crr_offset) =
		  io_value | crr_value;

		/* reset the POS register	*/

		pos_value = posr(2,nio_slot);
#ifdef _RS6K_SMP_MCA
		if (__rs6k_smp_mca()) {
			reset_value = pos_value | 0x02;
			enable_value = (pos_value & ~0x02) | 0x01;
		} else {
#endif /* _RS6K_SMP_MCA */
			reset_value = pos_value | 0x10;
			enable_value = (pos_value & 0xef) | 0x01;
#ifdef _RS6K_SMP_MCA
		}
#endif /* _RS6K_SMP_MCA */
		posw(2,nio_slot,reset_value);
		delay_ms(1);
		posw(2,nio_slot,enable_value);
	}

	mtsr(IOCC_SEG_REG,save_seg);
}

/***********************************/
/* posr reads the pos registers    */
/***********************************/
static int posr(addr,slot)   /* read 8 bits (from POS REGS) */
int addr; int slot;
{
    char y;
    int p;
    y =   ( *(unsigned char volatile *)( (IOCC_SPACE_PTR) + 0x400100 + addr +
		    (slot<<16)            )     );
    p = y;
    return(p);
}  /* end of posr  function */

/***********************************/
/* posw writes to the pos registers*/
/***********************************/
static void
posw(addr,slot,data)  /* write 8 bits (to POS regs) */
char data;
int addr; int slot;
{
	   *(unsigned char volatile *)( (IOCC_SPACE_PTR) + 0x400100 + addr +
		   (slot<<16)            )     = data;
	   return;
}  /* end of posw  function */


/***********************************************************/
/* delay_ms waits the specified number of milliseconds,    */
/* then returns control.                                   */
/***********************************************************/
static void
delay_ms(msec)
int msec;
{
	int save_segreg, io_ptr, delay;
	save_segreg = mfsr(IOCC_SEG_REG);
	mtsr(IOCC_SEG_REG,IOCC_SEG_REG_VALUE);
	io_ptr = (ulong *) (IOCC_SPACE_PTR + IOCC_BUS_DELAY);
	for (delay=0; delay<=msec*1000; delay++);
		iocc_delay(io_ptr,1);
	mtsr(IOCC_SEG_REG,save_segreg);
	return;
}  /* end of delay_ms function */

/*
 * NAME: IO_att
 *
 * FUNCTION: Stores value in a scratch register and returns address of segment
 *
 * RETURNS: Address of new segment
 */

static IO_att (segval, offset)
ulong segval;
ulong offset;
{
	ulong addr;

	/* save old segment register value for IO_det */
	orig_segval = mfsr(SCRATCH_SEG);

	/* setup segment register */
	mtsr (SCRATCH_SEG, segval);

	/* return address of new segment */
	addr = ((offset & 0xFFFFFFF) | ((SCRATCH_SEG) << SEGSHIFT));
	return (addr);
}

/*
 * NAME: IO_det
 * 
 * FUNCTION:  Restore the contents of the scratch segment register
 *
 * NOTES:  Undoes the effects of IO_att
 */

static IO_det (addr)
ulong addr;
{
	/* io_att checks addr for validity and panics if addr is bad */
	mtsr (SCRATCH_SEG, orig_segval);
}

#ifdef	_RS6K_SMP_MCA

/*
 * Return true if a new char is available.
 */
drs_tty_ischar(line)
int line; /* only for compatibility, the check will always be done on dbterm */
{
	register struct rs_port *d_port;
	int c, newhead;
	
    if (d_offset)
		return 1;
    d_port = (struct rs_port *)IO_att(nio_buid, d_offset);

    if ((c = d_get(d_port)) != -1)
	if (c == 'S'-'@')		/* stop */
	    while ((c = d_get(d_port)) != 'Q'-'@');
	else {
	    newhead = (d_head + 1) & (DBUF-1);
	    if (newhead != d_tail) {
			d_buf[d_head] = c;
			d_head = newhead;
	    }
	}
	IO_det(d_port, line);
	if (d_head != d_tail)
		return 1;
	return 0;
}

#endif 

