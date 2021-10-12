/* @(#)75   1.3  src/bos/kernext/cie/fddidds.h, sysxcie, bos411, 9428A410j 4/1/94 15:49:47 */

/* 
 * 
 * COMPONENT_NAME: (SYSXCIE) COMIO Emulator
 * 
 * FUNCTIONS:
 * 
 * DESCRIPTION:
 * 
 *   FDDI Device-Dependent Structure
 * 
 * ORIGINS: 27
 * 
 *   (C) COPYRIGHT International Business Machines Corp. 1988, 1994    
 *   All Rights Reserved                                               
 *   Licensed Materials - Property of IBM                              
 *                                                                   
 *   US Government Users Restricted Rights - Use, duplication or       
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp. 
 * 
 */

#ifndef _H_FDDIDDS
#define _H_FDDIDDS
/* -------------------------------------------------------------------- */
/* Define Device Structure                                     */
/* -------------------------------------------------------------------- */
#define FDDI_USR_DATA_LEN  (32)    /* length of the usr data */
#define FDDI_PASSWD_SZ     (8)   /* 8 byte PMF password */

struct fddi_dds
{
   int   bus_type;       /* the bus type */
   int   bus_id;         /* the bus id */
   int   bus_intr_lvl;   /* the interrupt level */
   int   intr_priority;  /* for use with i_init */
   int   rcv_que_size;   /* one for each open from a user process */
   int   stat_que_size;  /* one for each open from a user process */
   int   rdto;           /* Receive data transfer offset */
   uint  slot;           /* card slot number of primary card */
   uchar    *bus_io_addr;   /* PIO bus address for IO */
   uchar    *bus_mem_addr;  /* PIO bus address for MEMORY */
   uint  dma_lvl;        /* DMA arbitration level */
   uint  dma_base_addr;  /* DMA base address */
   uint  dma_length;    /* length of DMA address space */
   uchar lname[ERR_NAMESIZE]; /* device logical name (i.e. fddi0) */

   uchar use_alt_mac_smt_addr;   /* TRUE => use the following
                * MAC SMT addr otherwise get from
                * the VPD.
                */
   /* alternate MAC SMT addr */
   uchar alt_mac_smt_addr[FDDI_NADR_LENGTH];

   int   tvx;     /* value of the tvx command in activation*/
   int   t_req;      /* value of the t_req command in activation*/

   uchar pmf_passwd[FDDI_PASSWD_SZ];   /* PMF password */



   uchar pass_bcon_frames; /* pass beacon frames to host */
   uchar pass_smt_frames;  /* pass SMT frames to host */
   uchar pass_nsa_frames;  /* pass Next Station Addressing (NSA)
                * frames to host
                */
   uchar user_data[FDDI_USR_DATA_LEN]; /* user data */
   int   tx_que_sz;

};
typedef struct fddi_dds fddi_dds_t;

/* -------------------------------------------------------------------- */
/*  FDDI VPD structure and status codes                              */
/* -------------------------------------------------------------------- */

#define FDDI_VPD_VALID      0x00    /* VPD obtained is valid */
#define FDDI_VPD_NOT_READ   0x01    /* VPD has not been read from adapter */
#define FDDI_VPD_INVALID    0x02    /* VPD obtained is invalid */
#define FDDI_VPD_LENGTH     257    /* VPD length for Primary card in bytes */
#define FDDI_XCVPD_LENGTH   257    /* VPD length for Xtender card in bytes */

struct fddi_vpd
{
   ulong status;     /* status of VPD */
   ulong    l_vpd;         /* length of VPD */
   uchar    vpd[FDDI_VPD_LENGTH];   /* VPD */
   ulong xc_status;     /* status of Extender card VPD */
   ulong l_xcvpd;    /* length of Extender card VPD */
   uchar xcvpd[FDDI_XCVPD_LENGTH]; /* VPD of Xtender card */
};
typedef struct fddi_vpd fddi_vpd_t;


#endif /* end if ! _H_FDDIDDS */
