/* @(#)54	1.15  src/bos/kernel/io/machdd/md_extern.h, machdd, bos41J, 9516a 4/17/95 17:20:48 */
#ifndef _H_MDEXTRN
#define _H_MDEXTRN
/*
 * COMPONENT_NAME: (MACHDD) Machine Device Driver
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27, 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   
/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */

#include <sys/types.h>
#include <sys/malloc.h>
#include <sys/mdio.h>

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

struct d_st {
    long source;
    long target;
    long size;
    int  iodir;
    int  bcopy;
#define IO_SOURCE 0
#define IO_TARGET 1
};

/* Bus Definitions */
#define BUS0		0x02000000|BUID_T|ADDR_INCR	/* POWER_RS BUS0 */
#define MD_MAX_BUS	16		/* max number of buses supported */
#define MD_NVRAM_IDX	MD_MAX_BUS	/* special bus array index for NVRAM */
#define MD_EXT_IDX	MD_MAX_BUS+1	/* special bus array index for ioctlx buid */
#define MD_BUS_TABSZ	MD_MAX_BUS+2	/* includes NVRAM and EXT */
#define MD_MAX_IOCC	4		/* max IOCCs for POWER_RS */
#define MD_MAX_BUID	16		/* valid buid table size */
#define INVALID		0xFFFFFFFF
#define MAX_PCI_SLOTS	8		/* max legal slots */
#define PCI_CFG_MASK	0xFF		/* valid addr bits */
#define PCI_CFG_ADDR	0xCF8		/* PCI Configuration Address Register*/
#define PCI_CFG_DATA	0xCFC		/* PCI Configuration Data Register*/
#define PCI_CFG_VALID	0x80000000	/* PCI Configuration Valid bit */

struct md_bus_info {
#define IO_RGN		0	/* I/O region index */
#define MEM_RGN		1	/* Memory region index */
#define CFG_RGN		2	/* Config region */
#define IOCC_RGN	3	/* IOCC */
#define MD_MAX_RGN	4	/* Number of regions */
	int bid[MD_MAX_RGN];	/* BID or BUID, one for each region */
#define MAP_T0		0	/* T=0 mapping */
#define MAP_T1		1	/* T=1 mapping */
#define NO_MAP		2	/* already mapped */
	int map;		/* Type of mapping required */
	int io_excpt;		/* I/O exception handling TRUE/FALSE */
	int hw_busnum;		/* PCI Hardware Bus Number */
	int pci_base_addr;	/* PCI base address	*/
	int pci_data_addr;	/* PCI data address	*/
};

#define	MD_MAX_SLOT	32		/* max # of bus slots */
#define REAL_601	0x07f00000	/* access real memory - 601 only */

#define	MAX_CFGADDR	0x000fffff
#define MAX_VPDADDR	0x001fffff
#define CFG_SLOT_MASK	0x000001ff
#define BUID_T		0x80000000	/* T=1 */
#define IOCC_SELECT	0x00000080	/* I=1 */
#define BYPASS_TCW	0x00000020
#define ADDR_CHK	0x00080000
#define ADDR_INCR	0x00040000
#define SEGREGMASK	0xf0000000
#define ADDRMASK	0x000fffff
#define NVRAMMASK	0x000fffff
#define RTC_ADDRMASK	0x0000001f

/* PowerPC Addresses */
#define CFG_REGS	0xff200000
#define VPD_SPACE	0xffa00000

/* 
 *----------------------------------------------------------------
 * 
 * The following defines are used to access and check the 
 * "nvram_section_1_valid" indicator in the IPL_INFO section of
 * the IPL_CB.
 * 
 *    SECT1_STATUS_MASK = mask to get the section 1 status bits
 *                        (bits 30 and 31).
 *    SECT1_CRC_BAD     = ROS detected bad CRC for section 1 and
 *                        was unable to reinitialize the area.
 *    SECT1_OK		= ROS determined that section 1 CRC was
 *                        OK and all previously set values remain
 *                        intact.
 *    SECT1_ROS_INITED  = ROS detected bad CRC for section 1 and
 *                        successfully reinitialized the area.
 *                        
 * A value of 2 is currently undefined.  Note that only the 
 * entry level diskless ROS is capable of setting these values.
 * All other levels set the indicator to 0.
 * 
 */

#define SECT1_STATUS_MASK 0x03
#define SECT1_CRC_BAD     0x00
#define SECT1_OK	  0x01
#define SECT1_ROS_INITED  0x03

/* 
 *----------------------------------------------------------------
 */

extern Simple_lock md_lock;	/* lock for serialization of Machine Device Driver */

#define MD_LOCK() 	simple_lock(&md_lock)
#define MD_UNLOCK() 	simple_unlock(&md_lock)
#define MD_LOCK_INIT() 	simple_lock_init(&md_lock)
#define MD_LOCK_ALLOC()	lock_alloc(&md_lock, LOCK_ALLOC_PAGED, MD_LOCK_CLASS, -1)

#define MD_PIN_ALLOC(bsize) ( (char *)xmalloc((uint)bsize,(uint)0,pinned_heap) )
#define MD_PG_ALLOC(bsize)  ( (char *)xmalloc((uint)bsize,(uint)0,kernel_heap) )
#define MD_PIN_FREE(xaddr) xmfree((void *)xaddr,pinned_heap)
#define MD_PG_FREE(xaddr)  xmfree((void *)xaddr,kernel_heap)

extern Simple_lock md_pci_lock;	/* lock for serialization of PCI config access */
extern Simple_lock md_nv_lock;	/* lock for serialization of NVRAM access */
extern int nvstatus;

    /* EXCEPTION HANDLER (hand.c) */
extern int md_move(struct d_st *data);
extern int 
md_exp_hand(int (*io_func)(), caddr_t io_data, void *ptr);
extern char *
mdget(int busnum, int region, ulong addr, long size, char *buf, int bcpy, int *err);
extern void 
mdput(int busnum, int region, ulong addr, long size, char *udata, int bcpy, int *err);

#ifdef _MD_C
    /* MACHINE DRIVER ENTRY POINTS (md.c) */
int mdinit(dev_t devno, int iodn, int ilev, int ddilen, char *ddiptr);
int mdopen(dev_t devno, ulong rwflag, int chan, int ext);
int mdclose(dev_t devno, int chan, int ext);
int mdioctl(dev_t devno, int op, int arg, ulong flag, int chan, int ext);
int mdread(dev_t devno, struct uio *uiop, int chan);
int mdmpx(dev_t devno, int *chanp, char *channame);
#endif

    /* NON-VOLATILE RAM DRIVER (nvdd.c) */
extern void nvinit();
extern int nvrw();
extern int nvled();

    /* PCI Config service */
extern int pci_cfgrw(int busnum, struct mdio *md, int write_flag);

    /* internal functions called by new ioctls for PEGASUS */

#ifdef _PEGASUS
void mdsetkey(uint key);
void mdnvstrled(char *str);
int mdcpuset(uint cpu_num, uint cpu_stat);
int mdcpuget(uint cpu_num, uint *cpu_stat);
int mdeepromget(uint eenum, uint eep_off, char *iodata, uint iosize,
		uint *length_addr);
int mdeevpdget(uint eenum, char *vpd_desc, char *iodata, uint iosize,
	       uint *length_addr);
int mdeevpdput(uint eenum, char *vpd_desc, char *iodata, uint iosize,
	       uint *length_addr);
int mdfepromput(char *iodata, uint iosize, uint *length_addr);
int mdpowerset(uint cbnum, uint pwr_cmd, char *iodata, uint iosize,
	       uint *length_addr);
int mdpowerget(uint cbnum, char *iodata, uint iosize, uint *length_addr);
int mdrdsset(uint cbnum, uint dknum, uint rds_state);
int mdrdsget(uint cbnum, uint dknum, uint *rds_state);
int mdinfoset(uint type, char *iodata, uint iosize, uint *length_addr);
int mdinfoget(uint type, char *iodata, uint iosize, uint *length_addr);
int mdsurvset(uint mode, uint delay);
int mdsurvreset();
int mdkeyconnect(int signo);
int mdkeydisconnect();
int mdreboot();
#endif /* _PEGASUS */

#endif /* _H_MDEXTRN */
