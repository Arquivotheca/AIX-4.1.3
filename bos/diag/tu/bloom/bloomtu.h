/* @(#)40       1.3  src/bos/diag/tu/bloom/bloomtu.h, tu_bloom, bos41J, 9518A_all 4/27/95 13:19:57 */
/*
 *   COMPONENT_NAME: TU_BLOOM
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#ifndef TU_BLOOM
#define TU_BLOOM
#ifdef SIXDG
#define uint unsigned int
#define ulong unsigned long
#define uchar unsigned char
#define CLOCKS_PER_SEC      1000000         /* microseconds in sec */
#endif
typedef struct {
     int             scsi_id;
     int             cfg_fd;            /* file handle for cfg /dev/busx */
#ifndef SIXDG
     diag_struc_t    *handle;           /* handle for adapter used by diagex */
     diagex_dds_t    *diagex_dds;       /* device structure used by diagex   */
#endif
     char            interrupt_routine[80];
     char            intr_rec;          /* interrupt received flag */
     char            intr_ret;          /* interrupt routine return code */
     char            istat;             /* ISTAT register at intr routine */
     char            sist0;             /* SIST0 register at intr routine */
     char            sist1;             /* SIST1 register at intr routine */
     char            dstat;             /* DSTAT register at intr routine */
     char            print;
#define NO_PRINT     0
#define FILE_PRINT   1
#define SCREEN_PRINT 2
     FILE            *fprint;
     int             sixdg_slot;
     unsigned long   bus_io_addr;
     int             ilevel;
} adapter_struc_t;

adapter_struc_t      *ahandle;           /* global handle */

typedef struct {
     char            name[20];
     long            tu;
#define BLOOM_INIT_ATU 0
#define BLOOM_DIAG_ATU 1
#define BLOOM_SCSI_ATU 2
#define BLOOM_TERM_ATU 3
#define BLOOM_RCFG_ATU 4
#define BLOOM_SEDIFF_ATU 5
} tucb_t;

#define BLOOM_SUCCESS  		0x00000000

/* Errors for BLOOM_INIT_ATU */
#define BLOOM_SOFTWARE_E        0xFFFFFFFF /* Software Error */
#define BLOOM_MALLOC_E          0x80000001 /* malloc failure */
#define BLOOM_FOPEN_E           0x80000002 /* /tmp/.BLOOM_DEBUG file */
#define BLOOM_ANAME_E           0x80000003 /* no adapter name */
#define BLOOM_ODMINIT_E         0x80000004 /* odm init failed */
#define BLOOM_ODMLOCK_E         0x80000005 /* odm lock failed */
#define BLOOM_ODMOPEN_E         0x80000006 /* odm open failed */
#define BLOOM_ODMCUDV_E         0x80000007 /* odm no cudv failed */
#define BLOOM_ODMGET_E          0x80000008 /* odm open failed */
#define BLOOM_PARENT_E          0x80000009 /* odm no parent */
#define BLOOM_BUSID_E           0x8000000A /* odm no busid */
#define BLOOM_IOAD_E            0x8000000B /* odm no busio address */
#define BLOOM_ODMPDAT_E         0x8000000C /* odm PDAT  busio address */
#define BLOOM_INTR_E            0x8000000D /* odm CuAt intr level */
#define BLOOM_INTRPRIO_E        0x8000000E /* odm CuAt intr priority */
#define BLOOM_DOPEN_E           0x8000000F /* odm CuAt intr priority */
#define BLOOM_SCSIID_E          0x80000010 /* odm CuAt intr priority */
#define BLOOM_CFGOPEN_E         0x80000011 /* cfg open failed for busx */
#define BLOOM_DEVID_E           0x80000012 /* invalid device vendor id */
#define BLOOM_INTR_DIR_E        0x80000013 /* DIAGX_SLIH_DIR not defined */
#define BLOOM_INTR_LOAD_E       0x80000014 /* could not load intr handler */
#define BLOOM_CHILD_E           0x90000001 /* child can not be unconfigured */
#define BLOOM_DEVICE_E          0x90000002 /* device can not be unconfigured */
#define BLOOM_DIAGNOSE_E        0x90000003 /* device not in diagnose */

/* Errors for BLOOM_DIAG_ATU */
#define BLOOM_CFGMIS_E          0xA0000001 /* Miscompare on wr of cfg regs */
#define BLOOM_IOREGS_E          0xA0000002 /* Error on wr of mem io regs */
#define BLOOM_MEMIOMIS_E        0xA0000003 /* Miscompare on wr of memio regs */
#define BLOOM_FIFOES_E   	0xA0000004 /* Fifo empty bit set */
#define BLOOM_FIFOEC_E   	0xA0000005 /* Fifo empty bit clear */
#define BLOOM_FIFOFS_E   	0xA0000006 /* Fifo full bit set */
#define BLOOM_FIFOFC_E   	0xA0000007 /* Fifo full bit clear */
#define BLOOM_FIFOMIS_E   	0xA0000008 /* Fifo data miscompare */
#define BLOOM_SCSIFIFO_MIS_E   	0xA0000009 /* SCSI Fifo data miscompare */
#define BLOOM_SCSIFIFO_UNDF_E   0xA000000A /* SCSI Fifo underflow */
#define BLOOM_SCSIFIFO_PAR_E    0xA000000B /* SCSI parity error */
#define BLOOM_SCSIFIFO_FLAGS_E  0xA000000C /* SCSI Fifo flags no of bytes */
                                           /* fifo count incorrect */
#define BLOOM_INTR_TIMEOUT      0xA000000D /* Interrupt Not received */

/* Errors for BLOOM_SCSI_ATU */
#define BLOOM_SCSIARB_E         0xB000000D /* Could not arbitrate */
#define BLOOM_SCSFCMP_E         0xB000000E /* Function not complete */
#define BLOOM_SCSIDATA_E        0xB000000F /* scsi bus data miscompare */
#define BLOOM_TERMPOWER_E       0xB0000010 /* No Term Power */
#define BLOOM_BUS_E             0xB0000011 /* scsi bus errors */
#define BLOOM_BUS_TIMEOUT_E     0xB0000012 /* scsi bus timeouts */

/* Errors for BLOOM_TERM_ATU */
#define BLOOM_DCLOSE_E          0xD0000001 /* diag close  failure */
#define BLOOM_INTR_QLOAD_E      0xD0000002 /* qload for intr failure */
#define BLOOM_INTR_ULOAD_E      0xD0000004 /* diag intr hand unload  failure */
#define BLOOM_INTH_E            0xD0000003 /* no Intr handler loaded*/
#define BLOOM_RESDEV_E          0xE0000004 /* device can not be defined */
#define BLOOM_AVAILDEV_E        0xE0000005 /* device can not be made AVAIL */
#define BLOOM_RESCHILD_E        0xE0000006 /* Child can not be restored */

/* Errors for BLOOM_SEDIFF_ATU */
#define BLOOM_DEVICE_SE         0x55555555 /* Adapter is Single Ended */
#define BLOOM_DEVICE_DIFF       0xAAAAAAAA /* Adapter is Differential */

/* General Error codes */
#define BLOOM_TEST_E  		0xF0000000 /* invalid test unit number */
#define BLOOM_PCFGRD_E          0xF0000001 /* error reading pci cfg regs*/
#define BLOOM_PCFGWR_E          0xF0000002 /* error reading pci cfg regs*/

#ifdef SIXDG
#define BLOOM_OINTR_RELEASE_E   0xC0000000 /* could not release old intr */
#define BLOOM_INTR_REGISTER_E   0xC0000001 /* error on registering intr */
#endif

struct attr {
        char *attribute;
        char *value;
};

#define DEV_VENDOR_ID_OFFS      0x00
#define DEV_VENDOR_ID_SIZE      0x01
#define P810_SIGNATURE          0x00011000      /* NCR 53C810 */
#define P820_SIGNATURE          0x00021000      /* NCR 53C820 */
#define P825_SIGNATURE          0x00031000      /* NCR 53C825 */ 
#define FIVEFIVE                0x55555555      
#define ABLEABLE                0xAAAAAAAA      
#define RUNNINGD                0x00FF5A22      

#define STATUS_COMMAND_OFF      0x4    /*Enable Parity, Master and IO space */
#define STATUS_COMMAND_SIZE     0x1    /*Enable Parity, Master and IO space */
#define STATUS_COMMAND          0x02000045 /*Enable Parity,Master and IO addr */
#define NO_MASK                 0xFFFFFFFF
#define BASE_ADDR_MASK          0xFFFFFF00
#define BASE_ADDR_SIZE          0x1      
#define BASE_IOADDR_OFFSET      0x10
#define BASE_MEMADDR_OFFSET     0x14
#define CTEST4_DATA     	0x04
#define CTEST4_DEND    		0x08
#define DMA_FIFO_SIZE   	0x10
#define CLEAR_DMA_FIFO  	0x04
#define SCSI_FIFO_SIZE  	0x09
#define SODL0  	                SODL
#define SODL1  	                SODL+1
#define SBDL0  	                SBDL
#define SBDL1  	                SBDL+1
#define RESPID0  	        RESPID
#define RESPID1  	        RESPID+1
#define INTR_TIMEOUT  	        100

#endif
