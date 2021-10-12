/* @(#)30	1.15  src/bos/kernel/sys/POWER/ioacc.h, sysios, bos412, 9445C412a 11/9/94 13:22:00 */
#ifndef _H_IOACC
#define _H_IOACC
/*
 * COMPONENT_NAME: (SYSIOS) IO Access Macros
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 *				I/O ACCESS
 *
 *	This header file defines a set of macros that provide a machine
 *	independent interface to either the PC AT bus or the Micro
 *	Channel bus. A different version of this header file exists
 *	for each unique machine type that supports these busses.
 *	Multiple I/O buses are supported by these macros.
 *
 *	These macros are typically used as follows.
 *
 *      ulong *p;
 *	p = (ulong *)BUSMEM_ATT(config_bid + my_device_offset);
 *	BUS_PUTL(p,value_to_write_to_device);
 *	BUSMEM_DET(p);
 */

#include <sys/types.h>

/*
 *	The following macros write the specified data to bus memory.
 *	These macros are statements.
 */

#define BUS_PUTL(p,v)   *(ulong volatile *)(p) = (ulong)(v)
#define BUS_PUTS(p,v)   *(ushort volatile *)(p) = (ushort)(v)
#define BUS_PUTC(p,v)   *(uchar volatile *)(p) = (uchar)(v)


/*
 *	The following macros read the specified data from bus memory.
 *	These macros are expressions.
 */

#define BUS_GETL(p)     (*((ulong volatile *)(p)))
#define BUS_GETS(p)     (*((ushort volatile *)(p)))
#define BUS_GETC(p)     (*((uchar volatile *)(p)))

/*
 *	The following macros write the specified data to bus I/O.
 *	These macros are statements.
 */

#define BUSIO_PUTL(p,v) *(ulong volatile *)(p) = (ulong)(v)
#define BUSIO_PUTS(p,v) *(ushort volatile *)(p) = (ushort)(v)
#define BUSIO_PUTC(p,v) *(uchar volatile *)(p) = (uchar)(v)

/*
 *	The following macros read the specified data from bus I/O.
 *	These macros are expressions.
 */

#define BUSIO_GETL(p)   (*((ulong volatile *)(p)))
#define BUSIO_GETS(p)   (*((ushort volatile *)(p)))
#define BUSIO_GETC(p)   (*((uchar volatile *)(p)))

/*
 * Bus Memory and I/O Access, byte reversed.  These routines are not exported 
 * to the user community and are not documented.
 *
 * p is the pointer to the address to be accessed
 * v is the data to be written
 *
 */

#define	BUS_GETLR(p)		((unsigned long)BusGetLR(p))
#define	BUS_GETSR(p)		((unsigned short)BusGetSR(p))

#define	BUS_PUTLR(p,v)		((void)BusPutLR(p,v))
#define	BUS_PUTSR(p,v)		((void)BusPutSR(p,v))

#define	BUSIO_GETLR(p)		((unsigned long)BusGetLR(p))
#define	BUSIO_GETSR(p)		((unsigned short)BusGetSR(p))

#define	BUSIO_PUTLR(p,v)	((void)BusPutLR(p,v))
#define	BUSIO_PUTSR(p,v)	((void)BusPutSR(p,v))

/* 
 * Bus Memory and I/O Access, String Movement.  These routines are not 
 * exported to the user community and are not documented.
 *
 * d is the destination pointer
 * s is the source pointer
 * l is the byte count of the transfer
 *
 */

#define	BUS_GETSTR(d,s,l)	((void)BusCpy(d,s,l))
#define	BUS_PUTSTR(d,s,l)	((void)BusCpy(d,s,l))

#define	BUSIO_GETSTR(d,s,l)	((void)BusCpy(d,s,l))
#define	BUSIO_PUTSTR(d,s,l)	((void)BusCpy(d,s,l))

/*
 *	The following macros read/write the specified data to bus memory.
 *	These functions set up an exception hanlder.  They will return
 *	0 if the IO access was successful, and the excption code if an
 *	exception occured
 */
#define BUS_PUTLX(p,v)		busputl(p,v)
#define BUS_PUTLRX(p,v)		busputlr(p,v)
#define BUS_PUTSRX(p,v)		busputsr(p,v)
#define BUS_PUTSX(p,v)		busputs(p,v)
#define BUS_PUTCX(p,v)		busputc(p,v)
#define BUS_GETLX(p,v)		busgetl(p,v)
#define BUS_GETLRX(p,v)		busgetlr(p,v)
#define BUS_GETSRX(p,v)		busgetsr(p,v)
#define BUS_GETSX(p,v)		busgets(p,v)
#define BUS_GETCX(p,v)		busgetc(p,v)
#define BUS_GETSTRX(io,s,l)	buscpy(io,s,l)
#define BUS_PUTSTRX(io,s,l)	buscpy(s,io,l)

#ifndef _NO_PROTO
int busputl(long *ioaddr, long data);
int busputs(short *ioaddr, short data);
int busputc(char *ioaddr, char data);
int busgetl(long *ioaddr, long *data);
int busgets(short *ioaddr, short *data);
int busgetc(char *ioaddr, char *data);
int busputlr(long *ioaddr, long data);
int busgetlr(long *ioaddr, long *data);
int busputsr(short *ioaddr, short data);
int busgetsr(short *ioaddr, short *data);
int buscpy(void *source, void *dest, int count);
#endif /* _NO_PROTO */

/*
 *      The BID is an input parameter to many bus dependent kernel
 *      services (PIO, DMA, interrupt).  A BID is defined for T=0
 *      buses to contain information describing the type, number,
 *      and regions for one bus.  This same information is also
 *      contained in T=1 BIDs, so this approach provides a more
 *      consistent programming model.
 *
 * T=0 BID format
 *
 *   +---------------------------------------------------------------+
 *   |   reserved    | flags |region |      type         |   number  |
 *   |               |       |       |                   |           |
 *   | | | | | | | | | | |1|1|1|1|1|1|1|1|1|1|2|2|2|2|2|2|2|2|2|2|3|3|
 *   |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
 *   +---------------------------------------------------------------+
 *
 *      flags - These flags are bus type dependent.
 *
 *      region - This is used if a IO bus does not occupy one
 *      contiguous space.  For example, PC buses distinguish IO
 *      space and bus memory.
 *
 *      type - This is a bus architecture the bid refers to.  Examples
 *	of bus type are PCI and ISA
 *
 *      num - This is the instance of particular type.  It is not a
 *      global bus number.  Instead it is incremented for each type
 *
 * 	The following macros and defines are for T=0 mappings
 */

/*
 * The following macro sould be used by device drivers or config methods
 * to generate a BID.
 */
#define BID_VAL(type, region, num) \
	(((type) << 6) | ((region) << 16) | (num))

/*
 * The following macros can bue used to extract type, number and region
 * from a T=0 BID
 */
#define BID_TYPE(bid) ((((uint)(bid)) >> 6) & 0x3FF)
#define BID_NUM(bid) ((bid) & 0x3F)
#define BID_REGION(bid) ((((uint)(bid)) >> 16) & 0xF)

/*
 * generate a BID, given a bid and region.  i.e. alters the region field
 * of the BID
 */
#define BID_ALTREG(bid, region) ((bid & 0xFFF0FFFF) | (region << 16))

/*
 *	Valid bus types known to AIX
 */
#ifdef _KERNSYS
#define IO_REALMEM	0x0001		/* Memory mapped IO */
#endif
#define IO_DIAG		0x0002		/* Testing (internal use only */
#define IO_PCI		0x0003		/* PCI bus */
#define IO_ISA		0x0004		/* ISA bus */
#define IO_VME		0x0005		/* VME bus */
#define IO_FT		0x0006		/* Fault-tolerant bus */
#define IO_MBUS		0x0007		/* Maintenance bus */

#define	MAX_REGIONS	3		/* maximium number of regions per bus */

/*
 * The following defines are for the PCI bus
 *
 * The PCI bus has three regions:
 *	IO memory - PCI_IOMEM
 *	BUS memory - PCI_BUSMEM
 *	configuration - PCI_CFGMEM
 *
 */	
#define PCI_IOMEM	0		/* PCI I/O memory region */
#define PCI_BUSMEM	1		/* PCI bus memory region */
#ifdef _KERNSYS
#define PCI_CFGMEM	2		/* PCI configuration region */
#endif

/*
 * The following defines are for the ISA bus
 *
 * The ISA bus has two regions:
 *	IO memory - ISA_IOMEM
 *	BUS memory - ISA_BUSMEM
 */
#define ISA_IOMEM	0		/* ISA I/O memory region */
#define ISA_BUSMEM	1		/* ISA bus memory region */


/*
 * The following structure is for internal use only
 */
struct businfo {
	int bid;			/* bus type and number */
	void *(*d_map_init)();		/* DMA services */
	int num_regions;		/* number of regions */
	long long ioaddr[MAX_REGIONS];	/* real address */
};

#ifdef _KERNSYS

/*
 * BID for real memory PIO access
 */
#define REALMEM_BID	BID_VAL(IO_REALMEM, 0, 0)	/* real memory access */

/*
 * This macro produces an index from a BID.  It must produce unique indexes
 * for all valid bus confgiuration.  For now type alone can be used.  When
 * machines come with multiple buses (of the same type) bus number will need
 * to be included
 */
#define BID_INDEX(bid) BID_TYPE(bid)

#define MAX_BID_INDEX 7				/* maximium BIDs to register */

#endif /* _KERNSYS */

#endif /* _H_IOACC */




