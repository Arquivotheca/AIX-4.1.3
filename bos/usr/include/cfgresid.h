/* @(#)12 1.5 src/bos/usr/include/cfgresid.h, libcfg, bos41J 5/31/95 17:15:37 */
/*
 * COMPONENT_NAME:  LIBCFG   cfgresid.h
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1994,1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* See sys/pnp.h and sys/residual.h for associated definitions and structs */

/* prevent multiple inclusion */
#ifndef _H_CFGRESID
#define _H_CFGRESID

/* CPU states */
#define	ENABLE  0
#define	DISABLE 1
#define	FAULTY  2

/* Device Flags */
#define	PROCESSOR_DEVICE	0x80 
#define	Bridge_Controller	6
#define	Host_Bridge		0
#define	ISA_Bridge		1
#define	EISA_Bridge		2
#define	MCA_Bridge		3
#define	PCI_Bridge		4
#define	PCMCIA_Bridge		5
#define	ISA_DEVICE		0x01
#define	EISA_DEVICE		0x02
#define	PNP_ISA_DEVICE		0x10
#define	PCI_DEVICE		0x04
#define	INTEGRATED		0x2000
#define	ENABLED			0x4000
#define	NO_TRI_STATE		ENABLED
#define	System_Peripheral	8
#define	Power_Management	6

/* RSPC types of descriptors */
#define	PCI_BRIDGE_DESC  3
#define	BRIDGE_ADDR_DESC 4

/* I/O triggers */
#define	HIGH_EDGE  	0x1
#define	LOW_EDGE   	0x2
#define	HIGH_LEVEL 	0x4
#define	LOW_LEVEL  	0x8

/* size of PNP ID str */
#define PNPIDSIZE	8

typedef struct _CFG_CPU {
  unsigned long cputype;
  unsigned int  cpustate;
  uchar	cpuname[12];	/* cpu name based on cpu type */
} CFG_CPU;

typedef struct _CFG_DEVICE_ID {
  unsigned long busid;	
  unsigned long devid;
  unsigned long serialnum;
  unsigned long flags;	 
  unsigned char basetype;	       /* See pnp.h for bit definitions */
  unsigned char subtype;	/* See pnp.h for bit definitions */
  unsigned char interface;	      /* See pnp.h for bit definitions */
  unsigned char spare;
} CFG_DEVICE_ID;

typedef union _CFG_BUS_ACCESS {
  struct _pnpaccess{
    unsigned char csn;
    unsigned char logicaldevnumber;
    unsigned short readdataport;
    } pnpaccess;
  struct _isaaccess{
    unsigned char slotnumber;
    unsigned char logicaldevnumber;
    unsigned short isareserved;
    } isaaccess;
  struct _mcaaccess{
    unsigned char slotnumber;
    unsigned char logicaldevnumber;
    unsigned short mcareserved;
    } mcaaccess;
  struct _pcmciaaccess{
    unsigned char slotnumber;
    unsigned char logicaldevnumber;
    unsigned short pcmciareserved;
    } pcmciaaccess;
  struct _eisaaccess{
    unsigned char slotnumber;
    unsigned char functionnumber;
    unsigned short eisareserved;
    } eisaaccess;
  struct _pciaccess{
    unsigned char busnumber;
    unsigned char devfuncnumber;
    unsigned short pcireserved;
    } pciaccess;
  struct _bridgeaccess{       
    unsigned char busnumber; 
    unsigned char numberofslots; 
    unsigned short bridgereserved; 
    } bridgeaccess;	       
} CFG_BUS_ACCESS;



typedef struct _CFG_DEVICE {
  CFG_DEVICE_ID deviceid;
  CFG_BUS_ACCESS busaccess;

  /* The following three are offsets into the DevicePnPHeap */
  unsigned long allocatedoffset;	/* Allocated resource description */
  unsigned long possibleoffset;		/* Possible resource description */
  unsigned long compatibleoffset;	/* Compatible device identifiers */

  unsigned char pnpid[PNPIDSIZE];	/* Null terminated PNP identifier */
} CFG_DEVICE;


typedef struct _CFG_VPD {

  /* Box dependent stuff */
  unsigned char PrintableModel[32];     /* Null terminated string.
                                           Must be of the form:
                                           vvv,<20h>,<model designation>,<0x0>
                                           where vvv is the vendor ID
                                           e.g. IBM PPS MODEL 6015<0x0>       */
  unsigned char Serial[16];             /* 12/94:
                                           Serial Number; must be of the form:
                                           vvv<serial number> where vvv is the
                                           vendor ID.
                                           e.g. IBM60151234567<20h><20h>      */
  unsigned char Reserved1[48];
  unsigned long FirmwareSupplier;       /* See FirmwareSuppliers enum         */
  unsigned long FirmwareSupports;       /* See FirmwareSupport enum           */
  unsigned long NvramSize;              /* Size of nvram in bytes             */
  unsigned long NumSIMMSlots;
  unsigned short EndianSwitchMethod;    /* See EndianSwitchMethods enum       */
  unsigned short SpreadIOMethod;        /* See SpreadIOMethods enum           */
  unsigned long SmpIar;
  unsigned long Reserved4;
  unsigned long Reserved5;
  unsigned long Reserved6;
  unsigned long ProcessorHz;            /* Processor clock frequency in Hertz */
  unsigned long ProcessorBusHz;         /* Processor bus clock frequency      */
  unsigned long Reserved7;
  unsigned long TimeBaseDivisor;        /* (Bus clocks per timebase tic)*1000 */
  unsigned long WordWidth;              /* Word width in bits                 */
  unsigned long PageSize;               /* Page size in bytes                 */
  unsigned long CoherenceBlockSize;     /* Unit of transfer in/out of cache
                                           for which coherency is maintained;
                                           normally <= CacheLineSize.         */
  unsigned long GranuleSize;            /* Unit of lock allocation to avoid   */
                                        /*   false sharing of locks.          */

  /* L1 Cache variables */
  unsigned long CacheSize;              /* L1 Cache size in KB. This is the   */
                                        /*   total size of the L1, whether    */
                                        /*   combined or split                */
  unsigned long CacheAttrib;            /* L1CACHE_TYPE                       */
  unsigned long CacheAssoc;             /* L1 Cache associativity. Use this
                                           for combined cache. If split, put
                                           zeros here.                        */
  unsigned long CacheLineSize;          /* L1 Cache line size in bytes. Use
                                           for combined cache. If split, put
                                           zeros here.                        */
  /* For split L1 Cache: (= combined if combined cache) */
  unsigned long I_CacheSize;
  unsigned long I_CacheAssoc;
  unsigned long I_CacheLineSize;
  unsigned long D_CacheSize;
  unsigned long D_CacheAssoc;
  unsigned long D_CacheLineSize;

  /* Translation Lookaside Buffer variables */
  unsigned long TLBSize;                /* Total number of TLBs on the system */
  unsigned long TLBAttrib;              /* Combined I+D or split TLB          */
  unsigned long TLBAssoc;               /* TLB Associativity. Use this for
                                           combined TLB. If split, put zeros
                                           here.                              */
  /* For split TLB: (= combined if combined TLB) */
  unsigned long I_TLBSize;
  unsigned long I_TLBAssoc;
  unsigned long D_TLBSize;
  unsigned long D_TLBAssoc;

  void * ExtendedVPD;                   /* Pointer to extended VPD area;
                                           zeros if unused                    */
} CFG_VPD;


typedef struct {   /* Used for the 0x22 and 0x23 packet types, irq */
	unsigned long value;
	unsigned long flags;
	unsigned char inttype;
	unsigned char intctlr;
} CFG_irqpack_t;

typedef struct {   /* Used for the 0x2a packet type, dma */
	unsigned long value;
	unsigned long flags;
} CFG_dmapack_t;

typedef struct {    /* Used for 0x4b and 0x47 packet type, bus i/o */
	unsigned long sigbits;
	unsigned long min;
	unsigned long max;
	unsigned long incr;
	unsigned long width;

} CFG_iopack_t;

typedef struct {   /* Used for the 0x81 packet type, bus mem */
	unsigned long sigbits;
	unsigned long min;
	unsigned long max;
	unsigned long incr;
	unsigned long width;
} CFG_mempack_t;

typedef struct {   /* Used for Bridge Addr Xlat Desc 0x84 */
	uchar flags; /* Translation flags */
	uchar type;  /* Translation type & Addr conversion */
	uchar conv;  /* Translation conv & Addr conversion */
	unsigned long long min;   /* Base address */
	unsigned long long max;   /* Base address + Range - 1 */
	unsigned long long sys;   /* System memory address */
} CFG_bax_descriptor_t;

typedef struct {
	unsigned char slotnumber; 
	unsigned char devfunc; /* For function 0 */
	unsigned char inttype;
	unsigned char intctlr;
	unsigned long inta;
	unsigned long intb;
	unsigned long intc;
	unsigned long intd;
} CFG_pci_slot_t;

typedef struct {   /* Used for PCI Bridge Descriptor 0x84 */
	uchar   busnum;
	ulong   numslots;
	CFG_pci_slot_t slotdata[1];
} CFG_pci_descriptor_t;

typedef struct {   /* Used for ISA bridge descriptor */
	unsigned char inttype;
	unsigned char intctlr;
	unsigned long irq[16];
} CFG_isa_descriptor_t;

#endif /* _H_CFGRESID */
