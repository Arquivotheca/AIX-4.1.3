static char sccsid[] = "@(#)72	1.3  src/bos/kernel/db/POWER/dbtty_dvr_sf.c, sysdb, bos411, 9428A410j 5/18/94 10:32:43";

/*
 * COMPONENT_NAME: (SYSDB) Kernel Debugger
 *
 * FUNCTIONS:   dsf_ttyopen, dsf_ttyclose, dsf_ttyget, dsf_ttyput,
 *              sio_init, sio_stat, sio_getc, sio_putc
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

/*
 * Machine specific interface to the async ports.  This currently is
 * for a SANDALFOOT box and can talk to the native i/o ports only.
 *
 *   601 WARNING: This module takes advantage of an I/O mode that is unique
 * to the 601 chip.  It is a special case of I/O controller interface address
 * translation that forces I/O to buid 7F to use memory access protocols
 * instead of I/O protocols.  Since SANDALFOOT I/O is memory mapped(T=0),
 * this mode allows us to make very few changes to AIX I/O mechinisms other
 * than to change the I/O segment to a single special value and adjust for
 * the new devices and offsets.
 *
 * Created to work with the debugger, uses polling, not interrupts.
 *
 * The basic strategy is that the debugger calls d_tty* functions in order to
 * do terminal I/O.  These calls use a level of indirection based on the type
 * of system we are running on to call the correct d???_tty* calls.  In support
 * of this new function, the dbtty_dvr.c module has been rewritten to have the
 * redirection calls.  The original dbtty_dvr.c module has been renamed to
 * dbtty_rs.c, and the new Dakota tty routines are in a new file called
 * dbtty_sf.c(this file).
 *
 */

#include <sys/ioacc.h>
#include <sys/adspace.h>
#include <sys/tty.h>
#include <sys/intr.h>
#include <sys/mstsave.h>
#include <sys/low.h>
#include <sys/syspest.h>
#include <sys/systemcfg.h>
#include <sys/ioacc.h>
#include <sys/inline.h>

/* Local defines                                                             */
#define MAXPORTS 2                 /* Maximum number serial ports serviced   */
#define D_BAUD 9600                /* Default baud rate is 9600              */
#define D_BITS 8                   /* Default is 8 bits, no parity           */

/* Local macros for accessing UART registers                                 */
#define THR(x)  (com_bases[x]+0)   /* Transmit hold register                 */
#define RBR(x)  (com_bases[x]+0)   /* Receive buffer register                */
#define DLL(x)  (com_bases[x]+0)   /* Divisor latch least significant bit    */
#define IER(x)  (com_bases[x]+1)   /* Interrupt Enabler Register             */
#define DLM(x)  (com_bases[x]+1)   /* Divisor latch most significant bit     */
#define IIR(x)  (com_bases[x]+2)   /* Interrupt identification register      */
#define FCR(x)  (com_bases[x]+2)   /* Fifo control register (16550A only)    */
#define LCR(x)  (com_bases[x]+3)   /* Line control register                  */
#define MCR(x)  (com_bases[x]+4)   /* Modem control register                 */
#define LSR(x)  (com_bases[x]+5)   /* Line status register                   */
#define MSR(x)  (com_bases[x]+6)   /* Modem status register                  */
#define MTB(x)  (com_bases[x]+7)   /* Modem temporary byte (scratch pad)     */

#define SEG_REG 13                 /* Segment register to do I/O off of      */
#define SEG_VAL (0x87f00000 | SEG_REG)  /* Contents of that register         */

extern struct io_map nio_map;		/* native IO mapping */
extern ulong d_offset;		   /* Defined in dbtty_dvr_rs.c, set by open */

/* Local storage                                                             */
static int  com_stats[MAXPORTS] = {0, 0};
static int  com_bases[MAXPORTS] = {0x3f8, 0x2f8};
static int  dbgdev = -1;           /* The current debug device               */
static char oldier;                /* Original Interrupt enable register     */
static char oldmcr;                /* Original MCR (Out2 gates ints)         */

#define DBUF 32
static char dbuf[DBUF];            /* Holds keys pressed while writing       */
static int head = 0;               /* Where keys are added                   */
static int tail = 0;               /* Where they are read from               */

/* pio function for serial port
 */
uchar
inbyte(int port)
{
	uchar rc;
	volatile uchar *ioaddr;

	ioaddr = iomem_att(&nio_map);
	rc = *(ioaddr+port);
	__iospace_sync();
	iomem_det(ioaddr);
	return(rc);
}

void
outbyte(int port,			/* io port */
	uchar val)			/* value to write */
{
	volatile uchar *ioaddr;

	ioaddr = iomem_att(&nio_map);
	*(ioaddr+port) = val;
	__iospace_sync();
	iomem_det(ioaddr);
}

/* sio_init - This routine will initialize a serial port.  It always comes   */
/*            up in either 7 bit,even or 8 bit,no parity with one stop bit.  */
/*            Return code is 1 if initialization worked, or 0 if it did not  */
static int sio_init(int port, int baud, int words)

{
  int divisor;                     /* Baud rate divisor                      */
  short baudlow;                   /* Low byte of baud rate                  */
  short baudhigh;                  /* High byte of same                      */
  int oldmtb;                      /* Holds original contents of modem       */
  char tmplcr;                     /* Holds LCR while resetting baud rate    */

  /* Validate parameters                                                     */
  if ((port < 0) || (port >= MAXPORTS))
    {
    return(0);
    }
  if ((words != 7) && (words != 8))
    {
    return(0);
    }
  switch (baud)
    {
    case   300:
    case  1200:
    case  2400:
    case  9600:
    case 19200:
      break;                       /* These are all ok baud rates            */
    default:
      return(0);
    }

  /* Make sure that the serial port they asked for exists                    */
  oldmtb = inbyte(MTB(port));      /* Get current temp contents              */
  outbyte(MTB(port),0x55);         /* Write a new pattern                    */
  if (inbyte(MTB(port)) != 0x55)   /* Did we get our value back?             */
    {
    return(0);
    }
  outbyte(MTB(port),oldmtb);       /* Restore original contents              */

  /* Setup parity, stop bits, and word size.  Note we only do:               */
  /*      None, 1, 8  Note that DLAB is always set off                       */
  /*      Even, 1, 7                                                         */
  if (words == 8)
    outbyte(LCR(port), 0x03);      /* Set 8 data bits, 1 stop bit, no parity */
  else
    outbyte(LCR(port), 0x1A);      /* Set 7 data, 1 stop, parity ena, even   */

  /* Make sure we don't have any interrupts going off                        */
  oldier = inbyte(IER(port));      /* Remember the way things used to be     */
  outbyte(IER(port),0x00);         /* All 4 interrupts off (DLAB off)        */

  /* Decode UART type and enable biggest on chip buffer possible             */
  /*   Remember: Only the 16550A has a working fifo.  Anything else must not */
  /*             enable the fifo.                                            */
  /*   Also clear any outstanding stuff                                      */
  outbyte(FCR(port), 0x01);        /* Enable the FIFO (or try)               */
  if ((inbyte(IIR(port)) & 0xC0) != 0xC0)
    outbyte(FCR(port),0);          /* Disable FIFO except on 16550A          */
  else
    {                              /* Ok, good chip.  Clear buffers & go     */
    outbyte(FCR(port),0x07);       /* enable, clear tx, clear rx             */
    outbyte(FCR(port),0x01);       /* Just enable, no ints so no int mark    */
    };

  /* Fill in the baud rate divisor                                           */
  divisor = 115200 / baud;         /* Might as well figure out baud divisor  */
  baudlow = divisor & 0xff;        /* Get low byte                           */
  baudhigh = divisor >> 8;         /* And high byte                          */
  if (baudhigh != (baudhigh & 0xff))
    {
    return(0);
    }
  tmplcr = inbyte(LCR(port));
  outbyte(LCR(port), 0x80);        /* Turn on DLAB, the rest doesn't matter  */
  outbyte(DLL(port), baudlow);
  outbyte(DLM(port), baudhigh);
  outbyte(LCR(port),tmplcr);

  /* Set DTR & RTS even though we don't look at that stuff for in or out     */
  oldmcr = inbyte(MCR(port));      /* PCs use OUT2 as interrupt enable gate  */
  outbyte(MCR(port), 0x03);        /* DTR enable, RTS enable, out1&2 off     */

  /* All done with modem junk                                                */
  com_stats[port] = 0xff;          /* Remember we have initialized           */
  while (inbyte(LSR(port)) & 0x01) /* Clear out any bytes sitting there      */
    inbyte(RBR(port));
  return(1);
}


/* sio_stat - This routine will return 1 if there is data waiting at the com */
/*            port selected, 0 if there is none, and -1 if there is no com   */
/*            port installed.                                                */
static int sio_stat(int port)

{
  /* Make sure this port exists in the system                                */
  ASSERT((port >= 0) && (port < MAXPORTS) && (com_stats[port] != 0));

  /* Wait for the Data Ready bit to come on                                  */
  return(inbyte(LSR(port)) & 0x01);
}


/* sio_getc - This routine will return 1 byte from the serial port.  It will */
/*            wait forever for that byte to show up.  It does not care about */
/*            the hardware flow control stuff.                               */
static int sio_getc(int port)

{
  /* Make sure this port exists in the system                                */
  ASSERT((port >= 0) && (port < MAXPORTS) && (com_stats[port] != 0));

  /* Wait for some data to show up                                           */
  while (!sio_stat(port));         /* Loop forever here                      */

  /* Read it and sent it on its way                                          */
  return(inbyte(RBR(port)));
}


/* sio_putc - This routine will write one byte to the selected serial port.  */
/*            it does not care about hardware flow control stuff, only that  */
/*            there is space in the UART for the data.  If there is any      */
/*            problem with the serial port it will return 0, otherwise 1.    */
static int sio_putc(int port,int value)

{
  /* Make sure this port exists in the system                                */
  ASSERT((port >= 0) && (port < MAXPORTS) && (com_stats[port] != 0));

  /* Make sure value is a byte                                               */
  value &= 0xff;

  /* Wait for the transmitter hold register to empty                         */
  while ((inbyte(LSR(port)) & 0x60) != 0x60);

  /* Send the byte & done                                                    */
  outbyte(THR(port),value);
  return(1);
}



/*****************************************************************************/
/*                                                                           */
/*    START OF COMMENTS FROM RIOS VERSION OF DBTTY_DVR.C                     */
/*                                                                           */
/*****************************************************************************/
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
 * from the debugger, the ASSERT will catch you if you don't disable
 * interrupts first.  If interrupts are not disabled, then bad things
 * happen when the uart wakes up the real tty driver.
 * If you disagree with this, queue up a lot of output on the tty port
 * and do a kernel printf, stand back, and watch the sparks fly.
 *
 * I changed the ASSERT to ASSERT in *open, and changed the other
 * ASSERTs to if's.  I don't think this is a severe enough to trap the
 * machine.  If they don't call these routines at INTMAX, then the message
 * just won't print.
 */
/*****************************************************************************/
/*                                                                           */
/*    END OF COMMENTS FROM RIOS VERSION OF DBTTY_DVR.C                       */
/*                                                                           */
/*****************************************************************************/






/* dsf_ttyopen - This routine takes a parameter that is the serial port you  */
/*             want opened, starting with 0 which is com1.                   */
int dsf_ttyopen(int port)

{

  /* Make sure we are in polled mode.  See comments above                    */
  ASSERT(csa->intpri == INTMAX);

  /* Validate port and try to initialize a good one                          */
  if ((port < 0) || (port >= MAXPORTS))
    return(0);
  if (com_stats[port] == 0)
    if (sio_init(port, D_BAUD, D_BITS) == 0)
      return(0);

  /* Always return true, even when CD is not seen or wired up                */
  dbgdev = port;                   /* Remember what the device is            */
  d_offset = com_bases[port];	   /* Set d_offset for use by general tty    */
				   /* driver to determine whether to accept  */
				   /* the debugger-invocation key sequence.  */
  return(1);
}


/* dsf_ttyclose - Do clean up processing on the debugger console.  See the   */
/*              comments above for some of the non-standard thinking that    */
/*              went into this routine.                                      */
void dsf_ttyclose()

{

  return;			/* brinup hack */

  /* Get out if we could/would do damage                                     */
  if (dbgdev == -1)                /* Is there anything open?                */
    return;                        /*  NO - Done then                        */
  if (csa->intpri != INTMAX)       /* Are we running interrupts disabled?    */
    return;                        /*  NO - We should be                     */

  /* Clear out the buffer of waiting characters                              */
  while (sio_stat(dbgdev) != 0)
    sio_getc(dbgdev);

  /* Restore interrupt enable stuff to pre-d_ttyopen() status                */
  outbyte(MCR(dbgdev), oldmcr | 0x03);  /* Keep DTR & RTS high for ATE       */
  outbyte(IER(dbgdev), oldier);

}


/* d_ttycomput - This routine will handle character output, input scanning,  */
/*               and common d_ttyput/d_ttybinput stuff                       */
static void d_ttycomput(unsigned char cout, int bin)

{
  int newhead;                     /* Used to figure new head value          */
  unsigned char cin;               /* The character we will read             */

  /* Get out if we could/would do damage                                     */
  if (dbgdev == -1)                /* Is there anything open?                */
    return;                        /*  NO - Done then                        */
  if (csa->intpri != INTMAX)       /* Are we running interrupts disabled?    */
    return;                        /*  NO - We should be                     */

  /* Handle <CR><LF> if we are not in binary mode                            */
  if ((bin == 0) && (cout == '\n'))
    d_ttycomput('\r', 0);          /* Call me again to do the <CR>           */

  /* See if there is anything waiting at the keyboard                        */
  if (sio_stat(dbgdev) != 0)       /* Is there any data waiting at kbd?      */
    {                              /* YES - process it                       */
    cin = sio_getc(dbgdev);        /* Read it                                */
    if (cin == 'S' - '@')          /* If it is a ^S wait for ^Q              */
      while (sio_getc(dbgdev) != ('Q' - '@'));
    else
      {                            /* Not ^S/^Q stuff, put it in buffer      */
      dbuf[head] = cin;            /* Save the character for later use       */
      newhead = (1 + head) % DBUF;
      if (newhead != tail)         /* Would we eat our tail?                 */
        head = newhead;            /*  NO - Update our new position then     */
      }
    }

  /* Write the character                                                     */
  sio_putc(dbgdev, cout);
  return;
}


/* dsf_ttyput - This routine will write a byte to the serial port. It handles*/
/*            some cooking on the output stream.                             */
void dsf_ttyput(unsigned char cout)

{
  d_ttycomput(cout, 0);
}


/* dsf_ttybinput - This routine will output a byte to the serial port.  It   */
/*               does no cooking.                                            */
void dsf_ttybinput(unsigned char cout)

{
  d_ttycomput(cout, 1);
}


/* dsf_ttyget - This routine will read and echo a character.  It strips the  */
/*            byte to 7 bits.                                                */
unsigned char dsf_ttyget()

{
  unsigned char cin;               /* The character we will return           */

  /* Get out if we could/would do damage                                     */
  if (dbgdev == -1)                /* Is there anything open?                */
    return(0);                     /*  NO - Done then                        */
  if (csa->intpri != INTMAX)       /* Are we running interrupts disabled?    */
    return(0);                     /*  NO - We should be                     */

  /* Check the buffer for a character                                        */
  if (head != tail)                /* We got something in buffer             */
    {
    cin = dbuf[tail++];            /* Read the byte                          */
    if (tail >= DBUF)              /* Should we wrap?                        */
      tail = 0;                    /* YES - Go back to the beginning         */
    }
  else
    cin = sio_getc(dbgdev);        /* If not from buffer, read from tty      */

  /* The comment said we would get 7 bit ASCII.  Make it so.                 */
  cin &= 0x7F;

  /* Convert <CR> to new line(\n)                                            */
  if (cin == '\r')
    cin = '\n';

  /* Echo the character and return                                           */
  dsf_ttyput(cin);
  return(cin);
}
