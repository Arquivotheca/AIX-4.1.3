/* @(#)11	1.6  src/bos/kernel/sys/POWER/badisk.h, sysxdisk, bos411, 9428A410j 4/23/92 17:46:27 */
#ifndef _H_BADISK
#define _H_BADISK
/*
 * COMPONENT_NAME: (SYSXDISK) Bus Attached Disk 
 *
 * FUNCTIONS:  Header File for Bus Attached Disk
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/***************************************************************************/
/*                             IOCTLS                                      */
/***************************************************************************/ 

#define BAREADV		0x01                                /* Read Verify */
#define BAHRESET	0x02                                 /* Hard Reset */
#define BAABORT		0x03	                          /* Abort Command */
#define BASRESET        0x04                                 /* Soft Reset */
#define BASEEK       	0x05                                       /* Seek */
#define BABUFFW		0x06                    /* Write Attachment Buffer */
#define BABUFFR		0x07                     /* Read Attachment Buffer */
#define BADIAGSTAT	0x08	            /* Get Diagnostic Status Block */
#define BACCSTAT	0x09	      /* Get Command Complete Status Block */
#define BADEVSTAT 	0x0A                    /* Get Device Status Block */
#define BAPOSINFO	0x0B               /* Get POS Register Information */
#define BADEVCFIG	0x0C	               /* Get Device Configuration */
#define BAMFGH		0x0D	                /* Get Manufacturer Header */
#define BAWAITCC	0x0E	              /* Wait for Command Complete */
#define BAPARK		0x0F	                             /* Park Heads */
#define BAFMTPROG	0x10	                        /* Format Progress */
#define BATRANSRBA	0x11 /* Translate Relative Blk Address to Absolute */
#define BAFORMAT	0x12                                     /* Format */
#define BADIAGTEST	0x13                            /* Diagnostic Test */

/* Format Options*/
#define BAIPDM		0x01                  /* Ignore Primary Defect Map */
#define BAISDM		0x02                /* Ignore Secondary Defect Map */
#define BAUSDM		0x04                /* Update Secondary Defect Map */
#define BAPESA		0x08          /* Perform Extended Surface Analysis */
#define BAFUPI          0x10             /* Format Unit Periodic Interrupt */

/* Diagnostic Tests */
#define BADIAG_ROFF	0x01           /* Retries off */
#define BADIAG_RON	0x02           /* Retries on */
#define BADIAG_RDWR	0x03           /* Diagnostic read/write test */
#define BADIAG_SEEK    	0x04           /* Diagnostic seek test */
#define BADIAG_READV	0x05	       /* Diagnostic read verify test */
#define BADIAG_SELFTEST	0x06           /* Diagnostic self test */

/***************************************************************************/
/*                        STRUCTURES                                       */
/***************************************************************************/

struct badisk_ioctl_parms {              /* structure for ioctls parameters */
	int 	rba;                              /* Relative Block Address */
        int	aba;             /* Absolute Block Address for translate rba*/
 	uint    buff_address;              /* buffer address for buffer r/w */
        ushort  num_blks;     /* number of blocks (for buffer r/w, MAX IS 2)*/
	int	curr_cylinder; /* current cylinder of the format in progress*/
        int     milli_secs;  /*number of milli secs to wait for cmd complete*/
        char    format_options;           /* option bits for format command */
        char    diag_test;                    /* diagnostic test to perform */
        };

struct badisk_mfg_header {
       uchar     title[6];
       uchar  	 lsb_defects;
       uchar     msb_defects;
       uchar     ext_recs;
       uchar     reserved1;
       uchar     bar_code[16];
       uchar     date_mf[8];
       uchar     rbas[4];
       uchar     softe_prv;
       uchar     err_class_dfect;
       uchar     skew_sec_format;
       uchar     spare_sec_trk;
       uchar     spare_sec_cyl;
       uchar     reserved2;
       uchar     defect_type;
       uchar     reserved3[13];
       uchar     dfect_aba[448];
       uchar     reserved4[5];
       uchar     checksum;
       };
       
struct badisk_dstat_blk {             /* Diagnostic Status Block Structure */
	uchar cmd_status;                                /* Command Status */
        uchar cmd_error;                                  /* Command Error */
        uchar dev_status;                                 /* Device Status */
        uchar dev_error;                                   /* Device Error */
	uchar pwron_errcode;                        /* Power-on Error Code */
	uchar test_errcode;                             /* Test Error code */
        ushort diag_cmd;                             /* Diagnostic Command */
        ushort reserved1;                                      /* Reserved */
	ushort reserved2;                                      /* Reserved */
        };

struct badisk_cstat_blk {                  /* Command Complete Status Block */
        ushort wc_dev_cmd;                     /* word count,device,command */
	uchar cmd_status;                                 /* Command Status */
        uchar cmd_error;                                   /* Command Error */
        uchar dev_status;                                  /* Device Status */
        uchar dev_error;                                    /* Device Error */
        ushort blocks_left;                       /* Blocks left to process */
        ushort lsw_lastrba; /* Least significant word of last rba processed */
        ushort msw_lastrba;  /* Most significant word of last rba processed */
        ushort blks_err_recov;           /* Blocks requiring error recovery */
        };
 
struct badisk_pos_regs {                         /* POS registers structure */
        uchar posreg0;                                    /* POS register 0 */
        uchar posreg1;                                    /* POS register 1 */
        uchar posreg2;                                    /* POS register 2 */
        uchar posreg3;                                    /* POS register 3 */
        uchar posreg4;                                    /* POS register 4 */
        uchar posreg5;                                    /* POS register 5 */
        uchar posreg6;                                    /* POS register 6 */
        uchar posreg7;                                    /* POS register 7 */
        };

struct badisk_spec {
	char 	spares_per_cyl;		     /* spare sectors per cylinder */
	char	special_bits;		    /* special bits in config data */
	short	cylinders;		            /* number of cylinders */
	char	tracks_per_cyl;		      /* number of tracks/cylinder */
	char	sectors_per_trk;	        /* number of sectors/track */
	int 	total_blocks;		    /* number of RBA's on the disk */
	};

struct ba_dds {
	uint	base_address;         /* Base address of Bus Attached Disk */
	ulong	bus_id;			                     /* I/O Bus ID */
	char	slot;			                /* I/O Slot Number */
	char	dma_level;		          /* DMA Arbitration Level */
	char	alt_address;	/* 0 = Default Addr, 1 = Alternate Address */
	char	battery_backed; /* 0 = No battery back, 1 = battery backup */
        ushort  bus_type;                                      /* BUS Type */
	int	intr_level;		                /* Interrupt Level */
        int 	intr_priority;               /* Interrupt Priority (CLASS) */
        uint    bucket_size;  /* Size of bucket samples for transfer stats */
        uint    bucket_count;   /* Current bucket count for transfer stats */
        uint    byte_count; /* byte count within bucket for transfer stats */
        uint    max_coalesce;       /* maximun number of bytes to coalesce */
        char    resource_name[8];            /* resource name of this disk */
	};

#endif _H_BADISK
