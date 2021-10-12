/* @(#)82       1.28  src/bos/kernel/sys/POWER/mdio.h, machdd, bos41J, bai15 4/4/95 08:27:34 */
#ifndef _H_MDIO
#define _H_MDIO
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

#define NV_VERSION 1

typedef struct mdio {
    ulong md_addr;	/* specified address */
    ulong md_size;	/* size of md_data */
    int md_incr;	/* increment type: MV_BYTE, MV_WORD, MV_SHORT */
    char *md_data;	/* pointer to space of size md_size */
    int md_sla;		/* entry buid value, exit error code */
    ulong *md_length;	/* length of data read in user buffer */
} MACH_DD_IO;

#define md_seqnum md_size 
#define md_type md_addr
#define md_buid md_sla
#define md_slaerr md_sla
#define md_eenum md_sla
#define md_cbnum md_addr
#define md_cmd md_sla
#define md_dknum md_sla
#define md_mode md_addr
#define md_delay md_sla
#define md_cpunum md_addr

#define MV_BYTE		0	/* 8 bit access */
#define MV_WORD		1	/* 32 bit access */
#define MV_SHORT	2	/* 16 bit access */

#define MSLA0   0x80
#define MSLA1   0x81
#define MSLA2   0x82
#define MSLA3   0x83
#define MSLA4   0x84
#define MSLA5   0x85
#define MSLA6   0x86
#define MSLA7   0x87

/* 
 * ioctl codes 
 *
 */

#define MIOBUSGET         1
#define MIOBUSPUT         2
#define MIOCCGET       	  3
#define MIOCCPUT       	  4
#define MIONVGET       	  5
#define MIONVPUT       	  6
#define MIOGETPS       	  7
#define MIOGETKEY         8
#define MIOBUSMEM         9
#define MIOBUSIO         10
#define MIONVTEST        11
#define MIONVLED         12
#define MIOAIPLCB        13
#define MIOIPLCB         14
#define MIONVCHCK        15
#define MSLAGET          16
#define MSLAPUT          17
#define MIOTODGET        18
#define MIOTODPUT        19
#define MIOVPDGET        20
#define MIOCFGGET        21
#define MIOCFGPUT        22
#define MIORESET         23
#define MIOSETKEY        24
#define MIONVSTRLED      25
#define MEEPROMGET       26
#define MEEVPDGET        27
#define MEEVPDPUT        28
#define MFEPROMPUT       29
#define MPOWERGET        30
#define MPOWERSET        31
#define MRDSGET          32
#define MRDSSET          33
#define MSURVSET         34
#define MSURVRESET       35
#define MCPUSET          36
#define MCPUGET          37
#define MDINFOSET        38
#define MDINFOGET        39
#define MIOKEYCONNECT    40
#define MIOKEYDISCONNECT 41
#define MIOPCFGET	 42
#define MIOPCFPUT	 43
#define MIOMEMGET	 44
#define MIOMEMPUT	 45
#define MIOGEARD	 46
#define MIOGEAUPD	 47
#define MIOGEAST	 48
#define MIOGEARDA	 49

/* values of status for MRDSGET/MRDSSET */
#define MRDS_ON		1
#define MRDS_OFF	0

/* values of md_mode for MSURVSET */
#define MSOFT_RESET	1
#define MHARD_RESET	2

/* values of cpu status for MCPUSET/MCPUGET */
#define MCPU_START	0x9
#define MCPU_DISABLED	0xa
#define MCPU_ENABLED	0xb

/* values returned by MIONVCHK */
#define NVRAM_OK    0
#define NVRAM_RESET 1
#define NVRAM_BAD   2

/* 
 *  Segment Regs
 *
 */

#define MIOMEMSYS 	0x0
#define MIOMEMBUS	0x00020
#define MIOMEMRT	0x00060
#define MIOMEMADDR      0x40000


struct dsc_disk {
    char disk0;
};

struct dsc_native {
    char nat0;
    char nat1;
};

struct dsc_exp {
    char exp0;
    char exp1;
};

struct dsc_pvid {
    struct unique_id pvid;
};

struct dsc_scsi {
    char buid;		/* Bus ID */
    char loc;		/* location 'I' = Internal, 'E' = External */
    char slot;		/* slot number */
    char adp_sid;	/* adapter scsi ID */
    char type;		/* type ?? */
    char target_sid;	/* target device scsi ID */
    char lun;		/* lun ID */
};

struct dsc_pvscsi {
    struct unique_id pvid;
    char   stype;
    struct dsc_scsi d_scsi;
};

struct dsc_sla {
    uchar sla[5];
};

struct dsc_general {
    uchar gen;
};

struct devdesc {
    char   dsc_length;
    char   dsc_type;
    union {
	struct dsc_disk    d_disk;	/* type = 'N' */
	struct dsc_native  d_native;	/* type = 'K' */
	struct dsc_exp     d_exp;	/* type = 'R' */
	struct dsc_pvid    d_pvid;	/* type = 'V' */
	struct dsc_scsi    d_scsi;	/* type = 'S' */
	struct dsc_pvscsi  d_pvscsi;	/* type = 'V', length = 25 */
	struct dsc_sla     d_sla;	/* type = 'L' */
	struct dsc_general d_general;	/* type = 'G' */
    } dsc;
    char  dsc_unused[8];
};

struct drv_desc {
    ushort drv_magic;		/* ( 2) A5A5 */
    short  drv_length;		/* ( 2)  */
    ulong  drv_start;		/* ( 4)  */
    ulong  drv_crc;		/* ( 4)  */
    uchar  drv_unused0[12];	/* (12)  */
				/* (24) TOTAL */
};

struct ros_cb {
    long   rcb_unused0;		/* (   4)   0 */
    long   nv_size;		/* (   4)   4 */
    long   nv_version;		/* (   4)   8 */
    long   rcb_unused1;		/* (   4)   C */
    uchar  scsi_adap_id[16];	/* (  16)  10, 14, 18, 1C */
    long   mem_config[56];	/* ( 224)  20 - FC */
    uchar  mem_data[256];	/* ( 256) 100 - 1FC */
    struct devdesc prevdev;	/* (  36) 200 - 220 */
    uchar  norm_dev_list[84];	/* (  84) 224 - 274 */
    uchar  serv_dev_list[84];	/* (  84) 278 - 2C8 */
    struct drv_desc drv0;	/* (  24) 2CC - 2E0 */
    struct drv_desc drv1;	/* (  24) 2E4 - 2F8 */
    ulong  ros_crc;		/* (   4) 2FC */
				/* () */
};

struct gea_attrib {
    long   gea_length;		/* 4/0	num bytes in the GEArea	*/
    long   gea_used;		/* 4/4  num bytes used in the GEArea */
    long   gea_thresh;		/* 4/8  GEArea threshhold value */
};

/*
 * Macros
 *
 *      POSREG(p, slot) : returns addr appropriate for MACH_DD_IO md_addr
 *		p = POS Register number
 *           slot = card slot in microchannel
 */

#define POSREG(p, slot) (((slot) << 16) | (p))

#ifdef _KERNEL
#define NVREAD  1
#define NVWRITE 0
#define nvread(t,p,s,l)  nvrw(NVREAD,  t, p, s, l)
#define nvwrite(t,p,s,l) nvrw(NVWRITE, t, p, s, l)
extern int nvrw(int rbarw, int type, uchar *dptr, int dstart, int dlen);

/*
 * led display values are 0-9 for hex digits 0-9, and A-E are displayed below:
 * hex digit   '8'   '9'   'A'   'B'   'C'   'D'   'E'   
 *             +--+  +--+              +  +  +--+  +
 *             |  |  |  |              |  |  |     |
 *             +--+  +--+  +--+  +--+  +--+  +--+  +--+
 *             |  |     |  |        |              |
 *             +--+     +  +--+  +--+        +--+  +--+
 * hex digit   '8'   '9'   'A'   'B'   'C'   'D'   'E'   
 * 
 * hex digit 'F' is blank, eg. '8F8' would display '8 8'
 */ 

/* 0<=ledvalue<=0xFFFF (only lower 3 nybbles significant) */
extern int nvled(ulong ledv);
#endif /* _KERNEL */

#endif /* _H_MDIO */
