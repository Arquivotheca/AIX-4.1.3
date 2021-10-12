/* @(#)12	1.10.1.9  src/bos/kernel/sys/POWER/iplcb.h, rosipl, bos411, 9434A411a 8/19/94 17:14:51 */
/*
 * COMPONENT_NAME: ROSIPL
 *
 * FUNCTIONS: Defines the RAM resident interface between
 *            the IPL Boot Process and the Operating System
 *
 * ORIGINS: 27, 83
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * @BULL_COPYRIGHT@
 *
 * Copyright (C) Bull S.A. 1994
 * LEVEL 1,  5 Years Bull Confidential and Proprietary Information
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
*/

#ifndef _H_IPLCB
#define _H_IPLCB

#ifdef _POWER

#define NUM_OF_DRIVES      (2)
#define _NUM_OF_BUIDS      (2)
#define SGA_SIMM           (10)  /* number of simms */
#define MAX_SLOT_NUM       (15)  /* last slot number per buid */
#include <sys/types.h>

typedef struct ipl_directory {                   /* IPL ROS DIRECTORY       */
  char             ipl_control_block_id[8];      /* IPL ROS ASCII string ID */
  unsigned int     ipl_cb_and_bit_map_offset;    /* offset to gpr_save_area */
  unsigned int     ipl_cb_and_bit_map_size;      /* IPL CB and bit map size */
  unsigned int     bit_map_offset;               /* offset to RAM bit map   */
  unsigned int     bit_map_size;                 /* size of the RAM bit map */
  unsigned int     ipl_info_offset;
  unsigned int     ipl_info_size;
  unsigned int     iocc_post_results_offset;
  unsigned int     iocc_post_results_size;
  unsigned int     nio_dskt_post_results_offset;
  unsigned int     nio_dskt_post_results_size;
  unsigned int     sjl_disk_post_results_offset;
  unsigned int     sjl_disk_post_results_size;
  unsigned int     scsi_post_results_offset;
  unsigned int     scsi_post_results_size;
  unsigned int     eth_post_results_offset;
  unsigned int     eth_post_results_size;
  unsigned int     tok_post_results_offset;
  unsigned int     tok_post_results_size;
  unsigned int     ser_post_results_offset;
  unsigned int     ser_post_results_size;
  unsigned int     par_post_results_offset;
  unsigned int     par_post_results_size;
  unsigned int     rsc_post_results_offset;
  unsigned int     rsc_post_results_size;
  unsigned int     lega_post_results_offset;
  unsigned int     lega_post_results_size;
  unsigned int     keybd_post_results_offset;
  unsigned int     keybd_post_results_size;
  unsigned int     ram_post_results_offset;
  unsigned int     ram_post_results_size;
  unsigned int     sga_post_results_offset;
  unsigned int     sga_post_results_size;
  unsigned int     fm2_post_results_offset;
  unsigned int     fm2_post_results_size;
  unsigned int     net_boot_results_offset;
  unsigned int     net_boot_results_size;
  unsigned int     csc_results_offset;
  unsigned int     csc_results_size;
  unsigned int     menu_results_offset;
  unsigned int     menu_results_size;
  unsigned int     console_results_offset;
  unsigned int     console_results_size;
  unsigned int     diag_results_offset;
  unsigned int     diag_results_size;
  unsigned int     rom_scan_offset;       /* Presence of ROMSCAN adapters*/
  unsigned int     rom_scan_size;
  unsigned int     sky_post_results_offset;
  unsigned int     sky_post_results_size;
  unsigned int     global_offset;
  unsigned int     global_size;
  unsigned int     mouse_offset;
  unsigned int     mouse_size;
  unsigned int     vrs_offset;
  unsigned int     vrs_size;
  unsigned int     taur_post_results_offset;
  unsigned int     taur_post_results_size;
  unsigned int     ent_post_results_offset;
  unsigned int     ent_post_results_size;
  unsigned int     vrs40_offset;
  unsigned int     vrs40_size;
  unsigned int     gpr_save_area1[64];
  unsigned int     system_info_offset;
  unsigned int     system_info_size;
  unsigned int     buc_info_offset;
  unsigned int     buc_info_size;
  unsigned int     processor_info_offset;
  unsigned int     processor_info_size;
  unsigned int     fm2_io_info_offset;
  unsigned int     fm2_io_info_size;
  unsigned int     processor_post_results_offset;
  unsigned int     processor_post_results_size;
  unsigned int     system_vpd_offset;
  unsigned int     system_vpd_size;
  unsigned int     mem_data_offset;
  unsigned int     mem_data_size;
  unsigned int     l2_data_offset;
  unsigned int     l2_data_size;
  unsigned int     fddi_post_results_offset;
  unsigned int     fddi_post_results_size;
  unsigned int     golden_vpd_offset;
  unsigned int     golden_vpd_size;
  unsigned int     nvram_cache_offset;
  unsigned int     nvram_cache_size;
  unsigned int     user_struct_offset;
  unsigned int     user_struct_size;
  unsigned int     residual_offset;
  unsigned int     residual_size;
} IPL_DIRECTORY, *IPL_DIRECTORY_PTR;

#include <sys/rosinfo.h>                  /* Information used by IPL ROS   */
typedef int ROS_TEST_TYPE;

typedef unsigned long IP_ADDR;            /* Typedefs used by network boot */
typedef unsigned char HW_ADDR[6];
typedef struct {
  IP_ADDR ip;
  HW_ADDR hw;
  } IP_HW_ADDRS;

typedef struct {
  unsigned short rcf;
  unsigned short seg[8];
  } TOK_ROUTE;

typedef struct {
  IP_ADDR ip;
  HW_ADDR hw;
  TOK_ROUTE route;
  } ARP_ENTRY;

typedef struct {
        unsigned char op;             /* opcode                       */
        unsigned char htype;          /* hardware type ethernet, etc. */
        unsigned char hlen;           /* hardware address length      */
        unsigned char hops;           /* number of gateway hops       */
        unsigned long xid;            /* transaction id               */
        unsigned short secs;          /* seconds since boot started   */
        unsigned short unused;        /* unused                       */
        IP_ADDR ciaddr;               /* client IP address (request)  */
        IP_ADDR yiaddr;               /* client IP address (reply)    */
        IP_ADDR siaddr;               /* server IP address            */
        IP_ADDR giaddr;               /* gateway IP address           */
        unsigned char chaddr[16];     /* client hardware address      */
        char sname[64];               /* server host name             */
        char file[128];               /* boot file name               */
        char vend[64];                /* vendor string                */
        } t_bootp;

typedef struct iocc_post_results {         /* RESULTS of IOCC POST(s).      */
  char          POST_id;                   /* POST identifier is null.      */
  unsigned int  length;                    /* the number of results[]...    */
                                           /*   length=0, no BUID results,  */
                                           /*   length=1, BUID 20 only,     */
                                           /*   length=2, BUID 20 and 21.   */
  union {                                  /* POST results[]...is redefined */
    unsigned int  results[2];              /*   by its BUID.                */
    struct {   /* results[n] == 0, tested good. results[n] != 0, tested bad.*/
    /* RETURNS: IOCC POST return code
     *       0 - good, no errors
     *   0x000?0001 is a bad DMA control reg where ? indicates which reg
     *       2 - bad load/store limit register
     *       3 - bad interrupt enable control register
     *       4 - bad IOCC RAM - probably a data error
     *       5 - bad IOCC RAM - probably an addressing error
     *       6 - DMA or dma buffer cache error
     *   0x?0000007 - data storage interrupt in IOCC ram test where ? is CSR15
     *              load and store status field (bits 0-3) -- p. 4-32 of HTRGI
     *       8 - reference and change bits in tcw table functioning incorrectly
     *       9 - slave dma buffer flush did not work
     *       A - bus master buffer flush did not work
     *       B - dirty bit does not reset on slave dma buffer flush
     *       C - dirty bit does not reset on bus master buffer flush
     */
      unsigned int   buid_20,
                     buid_21;
    } RESULTS;
  } IOCC_POST;

  ROS_TEST_TYPE test_mode;     /* the post test mode                        */
  int detected_error;          /* 0 implies no error detected               */
                               /* != 0 implies device specific error        */
  int adapter_present;         /* 0 implies not present, != 0 is present    */
  int adapter_bad;             /* only valid if adapter_present != 0,       */
                               /* 0 implies good, != 0 implies bad          */

  /* This section is applicable to RSC platform */
  int iocc_reg_err;                  /* error flags for all IOCC registers */
  int nio_pos_reg_err;               /* error flags for NIO POS registers  */
  int nio_pos_sh_reg_err;    /* error flags for NIO shadowed POS registers */
  int ethr_pos_reg_err;             /* error flags for ETHR POS registers  */
  int ethr_pos_sh_reg_err;  /* error flags for ETHR shadowed POS registers */
  int scsi_pos_reg_err;             /* error flags for SCSI POS registers  */
  int scsi_pos_sh_reg_err;  /* error flags for SCSI shadowed POS registers */
  int dsi_address;        /* EA. of inst. causing DSI, used by DSI handler */
  int dsi_occurred;      /* set to 0 by iocc_post, set to 1 by DSI handler */
  int parity_failure_code;            /* indicate source of parity failure */

  /* This section is applicable to both RSC and 601 platform */
  int trace_table_addr;              /* save address of trace table buffer */
  int forced_error_path;            /* Used to forced error path when = 1  */

  /* This section is applicable to 601 platform */
  int dma_slave_ctrl_regs_err;       /* error flags for DMA SLAVE CTRL REGS */
  int csr_regs_err;                       /* error flags for CHNL_STAT_REGS */
  int iocc_regs_err;                /* error flags for other IOCC registers */

} IOCC_POST_RESULTS;

typedef struct NIO_dskt_POST_results {
  int g_nio_adapter_present;
  int g_nio_post_failed;
  int g_adapter_present;
  int g_drive_present[NUM_OF_DRIVES];
  int g_seek_success[NUM_OF_DRIVES];

  struct fdc_drive_parms {
    int g_step_rate;
    int g_head_unload_time;
    int g_head_load_time;
    int g_gap_length;
    int g_max_cyl;
    int g_max_sect;
    int g_max_head;
    int g_data_rate;
    int g_media_drive_flag;
  } fdc[NUM_OF_DRIVES];

  int g_index;
  int g_motor_on;
  int g_msr_data;
  int g_phys_cyl;
  int g_logi_cyl;
  int g_int_flag;
  int g_int_expected;
  int g_eis0;
  struct ipl_cb *g_ipl_ptr;

  unsigned char g_fdc_result[8];

  ROS_TEST_TYPE test_mode;     /* the post test mode                        */
  int detected_error;          /* 0 implies no error detected               */
                               /* != 0 implies device specific error        */
  int adapter_present;         /* 0 implies not present, != 0 is present    */
  int adapter_bad;             /* only valid if adapter_present != 0,       */
                               /* 0 implies good, != 0 implies bad          */
} NIO_POST_RESULTS, *NIO_POST_RESULTS_PTR;

typedef struct sjl_disk_post_results
{
  /* the following are copies of the return codes                            */
  unsigned  sjl_post_result;                    /* set when sjl_post returns */
  unsigned  sjl_read_result;                    /* set when sjl_read returns */
  unsigned  sjl_read_fail_reason;               /* set only if sjl_read_result
                                                   set = READ_CMD_FAILED, so
                                                   perhaps to better explain
                                                               what happened */
} SJL_DISK_POST_RESULTS, *SJL_DISK_POST_RESULTS_PTR;


typedef struct SCSI_POST_results {
  int g_adapter_present;
  int g_drive_present;
  int g_spock_io_addr;

  char g_attr;                             /* attention register              */
  char g_bcr;                              /* basic control register          */
  char g_isr;                              /* interrupt status register       */
  char g_bsr;                              /* basic status register           */

  char g_isr_after_reset;
  char g_spock_dev_num;
  char g_dev_reset_flag;
  char g_spock_reset_flag;

  uint g_eis0;
  uint g_spock_imm_cmd;
  uint g_int_flag;
  uint g_int_expected;
  uint g_not_scsi_int_flag;
  uint g_current_scsi_int_flag;
  uint g_chan_stat_reg;
  uint g_cache_buf_reg_4;
  uint g_cache_buf_reg_8;
  uint g_rc;
  uint g_pf_flag;

  struct subsystem_control_block {
    ushort cmd_word;       /* command word                                    */
    ushort enable_word;    /* enable word                                     */
    ushort lba_low;        /* logical block address least significant word    */
    ushort lba_high;       /* logical block address most  significant word    */
    ushort sba_low;        /* system buffer address least significant word    */
    ushort sba_high;       /* system buffer address most  significant word    */
    ushort sbbc_low;       /* system buff byte count least significant word   */
    ushort sbbc_high;      /* system buff byte count most  significant word   */
    ushort tsba_low;       /* termination status block address least sig word */
    ushort tsba_high;      /* termination status block address most  sig word */
    ushort scbca_low;      /* chain address least significant word (optional) */
    ushort scbca_high;     /* chain address most  significant word (optional) */

    ushort block_count;    /* number of blocks                                */
/*  ushort scsi_cmd_0;                                                        */

    ushort block_length;   /* block size                                      */
/*  ushort scsc_cmd_1;                                                        */

    ushort scsi_cmd_2;
    ushort scsi_cmd_3;
    ushort scsi_cmd_4;
    ushort scsi_cmd_5;
    ushort word1_save;
    ushort word2_save;
    ushort filler[4];
  } g_scb;

  uint dummyx[512]; /* buffer for data cache */

  struct termination_status_block {
    ushort scb_status;     /* SCB end status word                             */
    ushort retry_counts;
    ushort rbc_low;        /* residual byte count least significant word      */
    ushort rbc_high;       /* residual byte count most  significant word      */
    ushort sg_offset_low;  /* scatter gather list offset least signif word    */
    ushort sg_offset_high; /* scatter gather list offset most  signif word    */
    ushort length_below;   /* number of bytes that follow in this structure   */
    ushort cs_status;      /* command/scsi status                             */
    ushort cd_err_codes;   /* command/device error codes                      */
    ushort adem;           /* attachment diagnostic error modifier            */
    ushort ciw;            /* cache information word                          */
    ushort last_scba_low;  /* last SCB address processed least signif word    */
    ushort last_scba_high; /* last SCB address processed most  signif word    */
    ushort  filler[3];
  } g_tsb;

  struct data_buffer {
    char device_type;      /* peripheral device type                          */
    char filler[271];
  } g_db;

  uint dummyy[512]; /* buffer for data cache */

  struct blueb_mbox_data {
    uint mbox_array[8];
  } g_bbmbd;

  struct blueb_mbox_save {
    uint mbox_array[8];
  } g_bbmbds;

  uint g_blueb_isr;
  uint g_blueb_cmd_mask;
  uint g_blueb_mb31_status;
  uint g_recursive_call;
  uint g_blueb_internal_reset;
  struct blueb_mbox_data *g_mbox_ptr;
  struct blueb_mbox_data *g_cbox_ptr;
  struct bbm_entry *g_bbm_ptr;

  uint g_num_bad_sect;
  uint g_read_sect_count;
  uint g_sect;
  uint g_sect_count;

  struct scsi_add {
    uint buid;              /* IOCC select BUID                               */
    uint int_ext;           /* INTERNAL/EXTERNAL flag                         */
    uint type;              /* SCSI type from SCSI inquiry command type field */
    uint slot_no;           /*      slot number                               */

    /* the following value must never be initialized = to target_scsi_id      */
    uint adapt_scsi_id;     /* SCSI adapter ID                                */

    /* the following two values must be initialized to have the same value    */
    uint target_scsi_id;    /* current SCSI target id                         */
    uint i_t_s_id;          /* initial SCSI target id                         */

    /* the following two values must be initialized to have the same value    */
    uint lun_id;            /* current SCSI logical unit number               */
    uint i_l_id;            /* initial SCSI logical unit number               */

    /* the following value two values must be initialized = 0, these variables*/
    /* may be modified by the routine which updates the SCSI addresses        */
    uint w_l_id;            /* SCSI logical unit number to wrap around on     */
    uint sentry;            /* sentry used by SCSI address calculation routine*/
    uint frag_allowed;      /* this flag is non-zero if fragmentation allowed */
    uint iplmode;           /* shows keyswitch position, NORMAL,SERVICE,SECURE*/
  } g_sca;

  struct scsi_config {
    uint buid;            /* IOCC select BUID                               */
    uint int_ext;         /* INTERNAL/EXTERNAL flag                         */
    uint type;            /* SCSI type from SCSI inquiry command type field */
    uint slot_no;         /*      slot number                               */
    uint adapt_scsi_id;   /* SCSI adapter ID                                */
    uint target_scsi_id;  /* current SCSI target id                         */
    uint lun_id;          /* current SCSI logical unit number               */
/* define the number of required bytes for device inquiry command           */
#define SCSI_INQ_DATA_SIZE 36
    char inq_data[SCSI_INQ_DATA_SIZE];
/* define the max number of supported scsi devices                          */
#define SCSI_CFG_DATA_SIZE 56
  } g_scsi_cfg[SCSI_CFG_DATA_SIZE + 1];

  unsigned char g_adapter_id_buid20[16];
  unsigned char g_adapter_id_buid21[16];

  struct bbm_entry {
    uint bad_sector;
    uint good_sector;
  } g_bbm_entry_save;

  uint g_rtcu;

  char              *found_addr;
  int                num_blocks;
  unsigned char  byte_offset[4];

  uint      current_tape_psn;
  int       g_ipl_cb_tcw_offset;
  int       g_data_tcw_offset;
  int       g_first_data_tcw;
  int       g_fatal_error;
  int       g_fatal_slot;

  struct ipl_cb *g_ipl_ptr;
  uint      reserved_a;
  uint      reserved_b;
  uint scsi_gpr_save_area[32];
  uint      g_scsi_tcw;
  uint      g_script_area_tcw;
  uint      *scsi_script_ptr;
  uint      scsi_script_size;
  char      *scsi_script_data_ptr;
  uint      scsi_script_data_size;
  uint      scsi_script_entry;
  uint      script_dma_addr;
  uint      data_dma_addr;
  uint      pf_support;
  uint      scsi_regs[64];
  uint      save_low_bound;

  ROS_TEST_TYPE test_mode;     /* the post test mode                        */
  int detected_error;          /* 0 implies no error detected               */
                               /* != 0 implies device specific error        */
  int adapter_present;         /* 0 implies not present, != 0 is present    */
  int adapter_bad;             /* only valid if adapter_present != 0,       */
                               /* 0 implies good, != 0 implies bad          */
  uint      walk_scsi_bus;
  uint      cfg_has_been_done;
  uint      scsi_is_secure;
  uint      got_scsi_info;

  /* TO FIND THE ACTUAL ADDRESSES OF THE FOLLOWING STRUCTURES YOU MUST      */
  /* CONSULT THE RUN TIME CODE ENVIRONMENT                                  */
  struct harr_mbox_data  {
    struct harr_mbox0_data {   /* the ACTUAL ADDRESS of this struct MAY NOT */
      uint mbox_array[8];      /* be at the next address from the previous  */
    } hmb0d;                   /* field (got_scsi_info). the code assigns   */
                               /* the address such that it is on a 32 byte  */
                               /* boundary.                                 */

    struct harr_mbox_align {   /* THIS MAILBOX NOT FOR USE. It is defined   */
      uint mbox_array[8];      /* for creating proper byte alignment        */
    } hmbad;
  } hmbd;

  struct harr_mbox0_data *hmb_ptr;
  uint harr_dev_addr;
  uint harr_cmplt_reg_data;
  uint harr_alert_reg_data;
  uint tandberg_present;
  uint dma_address;
  uint dma_count;
} SCSI_POST_RESULTS, *SCSI_POST_RESULTS_PTR;

/*****************************************************************************/
/*                                                                           */
/*                         Ethernet POST interface                           */
/*                                                                           */
/*****************************************************************************/
typedef struct eth_data {                      /* ethernet scratch pad area */
  ROS_TEST_TYPE test_mode;     /* the post test mode                        */
  int mau_test_type;           /* Advanced diagnostic ext wrap test type    */
  int detected_error;          /* 0 implies no error detected               */
                               /* != 0 implies device specific error        */
  int adapter_present;         /* 0 implies not present, != 0 is present    */
  int adapter_bad;             /* only valid if adapter_present != 0,       */
                               /* 0 implies good, != 0 implies bad          */

  uchar na[6];                  /* Ethernet hardware address */
  uint      eth_cmd_head;       /* If any non-fatal error occured, this holds*/
                                /* the first longword (the header) of the    */
                                /* Ethernet Control Area's Command Block     */
                                /* See 596 manual pgs 4-54 to 4-55           */
  /* Trace tables */
  uint *post_trace_table;
  uint *dir_trace_table;

  void *eth_ctl;                /* Pointer to ETH_CTL_DATA area */

} ETH_DATA, *ETH_DATA_PTR;

typedef struct tok_data {                    /* token ring scratch pad area */
  ROS_TEST_TYPE test_mode;     /* the post test mode                        */
  int detected_error;          /* 0 implies no error detected               */
                               /* != 0 implies device specific error        */
  int adapter_present;         /* 0 implies not present, != 0 is present    */
  int adapter_bad;             /* only valid if adapter_present != 0,       */
                               /* 0 implies good, != 0 implies bad          */

unsigned char cfg_pos_reg [8];          /* Storage for POS regs value       */
union net_addr                          /* Network address                  */
{
  uchar six[6];                         /* in 6 bytes                       */
  ushort three[3];                      /* in three half-words              */
} na;
uint  tok_adap_state;                   /* Global variable for adapter      */
uint  slot;                             /* Storage for slot number          */
uint  ipl_cb_ptr;                       /* Storage for ipl control pointer  */
uint  rx_tcw_base;                      /* Bus base address for rx buffer   */
uint  diag_wrap_flag;                   /* diagnostic wrap flag             */
uint  rx_continue_flag;                 /* receive continue flag            */
uint  rx_wait_flag;                     /* receive wait flag                */
uint  rx_occurred_flag;                 /* receive already happened         */
         int  rx_rc;                    /* receive return code              */
         int  rx_length;                /* length of receive frame          */
         int  l_loader;                 /* Length of loader program         */
         int  l_ucode;                  /* Length of microcode load         */
         char *p_loader;                /* Pointer to loader program in RAM */
         char *p_ucode;                 /* Pointer to ucode in system RAM   */
/****************************************************************************/
/* Storage area of tcws and effective addresses of adapter control areas    */
/* to be used in dma operation.                                             */
/****************************************************************************/
union ccb_dma                           /* Command control block            */
{
  uint full;                            /* Full word address                */
  ushort half[2];                       /* Two half-word address            */
} ccb_dma_addr;

uint ccb_tcw;                           /* tcw used for ccb                 */

union ssb_dma                           /* System status block              */
{
  uint full;                            /* Full word address                */
  ushort half[2];                       /* Two half-word address            */
} ssb_dma_addr;

uint ssb_tcw;                           /* tcw used for ssb                 */

union scb_dma                           /* System command block             */
{
  uint full;                            /* Full word address                */
  ushort half[2];                       /* Two half-word address            */
} scb_dma_addr;

uint scb_tcw;                           /* tcw used for scb                 */

union prod_id_dma                       /* Product id area                  */
{
  uint full;                            /* Full word address                */
  ushort half[2];                       /* Two half-word address            */
} prod_id_dma_addr;

uint prod_id_tcw;                       /* tcw used for product id          */

union rx_list_dma                       /* Receive list                     */
{
  uint full;                            /* Full word address                */
  ushort half[2];                       /* Two half-word address            */
} rx_list_dma_addr;

uint rx_list_tcw;                       /* tcw used for tx rx list          */

union tx_list_dma                       /* Receive list                     */
{
  uint full;                            /* Full word address                */
  ushort half[2];                       /* Two half-word address            */
} tx_list_dma_addr;

uint tx_list_tcw;                       /* tcw used for tx rx list          */

union adap_check_dma                    /* Adapter check block              */
{
  uint full;                            /* Full word address                */
  ushort half[2];                       /* Two half-word address            */
} acb_dma_addr;

uint acb_tcw;                           /* tcw used for adapter check block  */

union adap_open_dma                     /* Adapter open parameters          */
{
  uint full;                            /* Full word address                */
  ushort half[2];                       /* Two half-word address            */
} open_dma_addr;

uint open_tcw;                          /* tcw used for open parameters      */
uint next_avail_tcw;                    /* next available tcw                */

struct TOK_I_PARMS                      /* Adapter initialization parameters */
{                                       /* structure                         */
 short               init_options;
 unsigned char       cmd;
 unsigned char       xmit;
 unsigned char       rcv;
 unsigned char       ring;
 unsigned char       scb_clear;
 unsigned char       adap_chk;
 unsigned short      rcv_burst_size;
 unsigned short      xmit_burst_size;
 unsigned short      dma_abort_thresh;
 ushort              scb_add1;
 ushort              scb_add2;
 ushort              ssb_add1;
 ushort              ssb_add2;
} tok_i_parms;

/*
 *  pass2_regs_t is a template for the Systems Interface register set
 *  on the token ring adapter
 */

struct PASS2_REGS
{
   unsigned short      data;
   unsigned short      data_autoincr;
   unsigned short      addr;
   unsigned short      cmd;
   unsigned char       enable;
   unsigned char       reset;
   unsigned char       imask_enable;
   unsigned char       imask_disable;
   unsigned short      timer;
   unsigned short      status;
} pass2_regs;

char buffer_start[512];

struct TOK_OPEN_OPTIONS                 /* Token adapter Open Options        */
{                                       /*  structure                        */
   unsigned short   options;            /* Open Options - The PAD Routing    */
                                        /* Field bit MUST be set to 1        */
   unsigned short   node_addr1;         /* High-order 2 bytes of node addr   */
   unsigned short   node_addr2;         /* Middle 2 bytes of node address    */
   unsigned short   node_addr3;         /* Low-order 2 bytes of node addr.   */
   unsigned short   grp_addr1;          /* High-order 2 bytes of group addr  */
   unsigned short   grp_addr2;          /* Low-order 2 bytes of group addr.  */
   unsigned short   func_addr1;         /* High-order 2 bytes of func addr.  */
   unsigned short   func_addr2;         /* Low-order 2 bytes of func addr.   */
   unsigned short   rcv_list_size;      /* RCV List Element size in bytes    */
   unsigned short   xmit_list_size;     /* TX List Element size in bytes     */
   short            buf_size;           /* Adapter Buffer Size               */
   short            res1;               /* Extnl RAM Start Addr. - not used  */
   short            res2;               /* Extnl RAM End Addr. - not used    */
   char             xmit_buf_min_cnt;   /* Minimum # of Adap. Buffers to     */
                                        /* reserve for transmission          */
   char             xmit_buf_max_cnt;   /* Maximum # of Adapter buffers to   */
                                        /* use for transmit data             */
   unsigned short   prod_id_addr1;      /* High-order 2 bytes of product ID  */
                                        /* address                           */
   unsigned short   prod_id_addr2;      /* Low-order 2 bytes of product ID   */
                                        /* address                           */
} tok_open_options;

/* SSB structure */
struct SSB
{
    unsigned short       c_type;         /* Command type */
    unsigned short       stat0;          /* status 0 */
    unsigned short       stat1;          /* status 1 */
    unsigned short       stat2;          /* status 2 */
} ssb;                                   /* end struct SSB */

/* SCB structure */
struct SCB
{
    unsigned short  adap_cmd;            /* Adapter Command */
    unsigned short  addr_field1;         /* Address Field 1 */
    unsigned short  addr_field2;         /* Address Field 2 */
} scb;                                   /* End struct SCB definition */

struct CCB
{
    unsigned short      cmd;
    unsigned short      segment[2];
    unsigned short      recs;
} ccb;

struct ADAP_CHECK_BLOCK
{
   unsigned short  code;
   unsigned short  parm0;
   unsigned short  parm1;
   unsigned short  parm2;
} adap_check_blk;

struct GATHER_BLOCK
{
   unsigned short       cnt;        /* Amount of data at address to xfer */
   unsigned short       addr_hi;    /* high-order 2 bytes of address */
   unsigned short       addr_lo;    /* low-order 2 bytes of address */
}  gather_block;


/************************************************************************/
/*  Transmit List definition                                            */
/************************************************************************/
struct TX_LIST {
   struct TX_LIST       *next;          /* pointer to next in chain */
   unsigned short       status;         /* receive status */
   unsigned short       frame_size;     /* size of the entire frame */
   unsigned short       count1;         /* bytes in this receive list */
   unsigned short       addr1_hi;       /* high 16 bits of data address */
   unsigned short       addr1_lo;       /* low 16 bits of data address */
   unsigned short       count2;         /* bytes in this receive list */
   unsigned short       addr2_hi;       /* high 16 bits of data address */
   unsigned short       addr2_lo;       /* low 16 bits of data address */
   unsigned short       count3;         /* bytes in this receive list */
   unsigned short       addr3_hi;       /* high 16 bits of data address */
   unsigned short       addr3_lo;       /* low 16 bits of data address */
} tx_list;

/************************************************************************/
/*  Receive List definition                                             */
/************************************************************************/
struct RX_LIST {
   struct RX_LIST       *next;          /* pointer to next in chain */
   unsigned short       status;         /* receive status */
   unsigned short       frame_size;     /* size of the entire frame */
   unsigned short       count1;         /* bytes in this receive list */
   unsigned short       addr1_hi;       /* high 16 bits of data address */
   unsigned short       addr1_lo;       /* low 16 bits of data address */
   unsigned short       count2;         /* bytes in this receive list */
   unsigned short       addr2_hi;       /* high 16 bits of data address */
   unsigned short       addr2_lo;       /* low 16 bits of data address */
   unsigned short       count3;         /* bytes in this receive list */
   unsigned short       addr3_hi;       /* high 16 bits of data address */
   unsigned short       addr3_lo;       /* low 16 bits of data address */
} rx_list;

/* -------------------------------------------------------------------- */
/*         Product ID Information Structure                             */
/* -------------------------------------------------------------------- */

/*
 *  The following Product ID Information is taken from the
 *  Token-Ring Network Architecture Reference manual in
 *  the MAC Frames section.
 */
struct TOK_PROD_ID
{
   unsigned char   hardware;      /*
                                   * Bits 0-3 - Reserved
                                   * Bits 4-7 - Product classification
                                   *       0x1:    IBM Hardware
                                   *       0x3:    IBM or non-IBM HW
                                   *       0x4:    IBM software
                                   *       0x9:    Non-IBM hardware
                                   *       0xc:    Non-IBM software
                                   *       0xe:    IBM or non-IBM SW
                                   */

   unsigned char   format_type;   /*
                                   * Format type:
                                   *   0x10 - Product instance is
                                   *       identified by a serial number
                                   *       (that is, IBM plant of manufacture
                                   *       and sequence number) unique by
                                   *       machine type.
                                   *
                                   *   0x11 - Product instance is
                                   *       identified by a serial number
                                   *       unique by machine type and
                                   *       model number.
                                   *
                                   *   0x12 - Product instance is identified
                                   *       by machine type (as in Format 0x10)
                                   *       This format provides the model
                                   *       number not to identify a product
                                   *       instance uniquely, but for
                                   *       additional information only.
                                   */


   unsigned char   machine_type[4];
                                          /*
                                           * Machine type: 4 numeric EBCDIC
                                           * characters
                                           */


   unsigned char   mach_model_num[3];     /*
                                           *  Machine model number: 3 upper
                                           *  case alphanumeric EBCDIC chars
                                           *  for format types 0x11 and 0x12;
                                           *  these bytes are reserved by IBM
                                           *  future use in format type 0x10.
                                           */

   unsigned char   sn_modifier[2];        /*
                                           *  Serial number modifier -
                                           *  IBM plant of manufacture: 2
                                           *  numeric EBCDIC characters.
                                           */

   unsigned char   seq_num[7];            /*
                                           *  Sequence number: 7 upper case
                                           *  alphanumeric EBCDIC characters,
                                           *  right justified with EBCDIC zeros
                                           *  (0xf0) fill on the left.
                                           */


} prod_id;

char buffer_end[512];

int tok_wait_for_intr;                /* flag for tok_read to indicate a    */
                                      /*  indicate a read request has been  */
                                      /*  issued, but not yet returned.     */

struct tok_list
{
  struct tok_list *list_forward;

  /* pointer to next xmit or recv list in the chain.  When it is ODD, it   */
  /* is the last list in the chain.  Lists must be half-word aligned.      */

  unsigned short int     cstat;

  /* cstat is the command status field.  It is set by the system and       */
  /*  overwritten by the adapter to report status.  The bit field is as    */
  /*  follows when set by the system:                                      */
  /*  bit 0 - The Adapter will wait for this bit to be '1' before          */
  /*          processing the current xmit/rcv list.  The system must issue */
  /*          a Transmit or Receive Valid Interrupt Request when changing  */
  /*          Valid bits from '0' to '1'.  This bit is ignored unless the  */
  /*          List is an anticipated Start of Frame (follows End of Frame  */
  /*           or is first list of command).                               */
  /*  bit 1 - Frame  Complete, should be reset to '0'.                     */
  /*  bit 2 - Start of Frame, in transmit, must be set to '1' for Start of */
  /*          Frame.  In receive must be reset to '0'.                     */
  /*  bit 3 - End of Frame, in transmit, must be set to '1' for End of     */
  /*          Frame.  In receive must be reset to '0'.                     */
  /*  bit 4 - Frame Interrupt.  Setting this bit to '1' will cause the     */
  /*          adapter to interrupt when the frame has been xmitted or rcvd */
  /*          rather than waiting for all frames on the chain to be        */
  /*          sent or received.  This bit is ignored unless START OF FRAME */
  /*          is 1.                                                        */
  /*  bit 5   Not used for transmit.  Interframe wait for receive.  When   */
  /*          set to 1, the adapter reacts like a frame interrupt but it   */
  /*          assumes a receive suspended state and waits for a receive    */
  /*          continue.                                                    */
  /*  bit 6-15 Not used.                                                   */
  /*                                                                       */
  /*  The bits are as follows when overwritten by the adapter:             */
  /*  bit 0 - Valid, reset to 0.                                           */
  /*  bit 1 - Frame Complete, set to 1.                                    */
  /*  bit 2 - Start of Frame, set to 1.                                    */
  /*  bit 3 - End of Frame, will be set to one on the last frame.          */
  /*  bit 4 - Frame Interrupt, will be the same as in Request.  Reset to   */
  /*          0 in a receive command.                                      */
  /*  bit 5 - Transmit Error, will be set to 1 if the frame failed to      */
  /*          circulate the ring correctly.  Since the error could have    */
  /*          occured anywhere on the ring, it cannot be assumed that the  */
  /*          destination got correct data.  Reset to 0 in a receive       */
  /*          command.                                                     */
  /*  bit 6-7 Reserved.  Reset to 0 in a receive command.                  */
  /*  bit 8-13 Stripped PCFE, will contain a copy of the PCFE byte returned*/
  /*          when the transmitted frame is stripped off the ring.  IF     */
  /*          transmit error is set to 1, the PCFE should be ignored.      */
  /*  bit 14-15 Reset to 0 in a transmit command.  Address match in a      */
  /*          receive command, where:                                      */
  /*          00 - no address match                                        */
  /*          01 - primary address match                                   */
  /*          10 - secondary address match                                 */
  /*          11 - primary and secondary address match                     */
  /*          Primary address match will be set if the frame is copied     */
  /*          because of a Specific, Group, or Functional Address match in */
  /*          the adapter's address registers.  A Secondary match is set   */
  /*          when some other special match, such as a Source Routing      */
  /*          Field Match.                                                 */
  /*                                                                       */

  unsigned short int frame_size;

  /*  Number of bytes in the frame, not counting CRC, EDEL, or PCFE, but   */
  /*  including PCF0/PCF1, TO ADDRESS, FROM ADDRESS, ROUTING FIELD, and    */
  /*  INFORMATION FIELD.  frame_size = 0 is not valid.                     */

  unsigned short int data_count_0;

  /*  This field contains the number of bytes to be in the                  */
  /*  address defined in the DATA ADDRESS parameter.  There can be a maximum*/
  /*  of three DATA COUNT/DATA ADDRESS parameters per transmit list.  If bit*/
  /*  zero is 0, it is considered the last DATA COUNT in the transmit list  */

  unsigned short int data_address_0_hi;
  unsigned short int data_address_0_lo;
  unsigned short int data_count_1;
  unsigned short int data_address_1_hi;
  unsigned short int data_address_1_lo;

  unsigned short int data_count_2;
  unsigned short int data_address_2_hi;
  unsigned short int data_address_2_lo;


  /* Contains the address of the data to be transmitted or received.        */
} tok_list;

  uint gfi;                     /* global family 2 index */

  union tx_buffer                         /* transmit buffer dma address      */
  {
    uint full;                            /* Full word address                */
    ushort half[2];                       /* Two half-word address            */
  } tx_buffer_dma_addr;

uint tx_buffer_addr;                      /* transmit buffer address          */

  union rx_buffer                         /* receive buffer dma address       */
  {
    uint full;                            /* Full word address                */
    ushort half[2];                       /* Two half-word address            */
  } rx_buffer_dma_addr;

uint rx_buffer_addr;                      /* receive buffer address           */

} TOK_DATA, *TOK_DATA_PTR;

typedef struct ser_data {                   /* serial port scratch pad area */
  ROS_TEST_TYPE test_mode;     /* the post test mode                        */
  int detected_error;          /* 0 implies no error detected               */
                               /* != 0 implies device specific error        */
  int adapter_present;         /* 0 implies not present, != 0 is present    */
  int adapter_bad;             /* only valid if adapter_present != 0,       */
                               /* 0 implies good, != 0 implies bad          */

 /* sp_portx_open is set to SP_PORT_IN_USE when port x is in use.          */
 /* sp_err_reason_portx is currently unused, but is reserved; to           */
 /*  be set to the reason code from the  chip when an error occurs.        */
 /* sp_char_portx is normally 0.  It is set when some character other than */
 /*  XOFF is received while the sp_put() routine (and sp_xwait()) is       */
 /*  running.                                                              */
  int sp_portA_open;
  int sp_err_reason_portA;
  char sp_char_portA;

  int sp_portB_open;
  int sp_err_reason_portB;
  char sp_char_portB;

  int sp_tablet_open;
  int sp_err_reason_tablet;
  char sp_char_tablet;

  /* the following three ints are reserved for future use */
  int sp_uart_initialized;
  int sp_mc_save;  /* save modem control register */
  int sp_lc_save;  /* save line control  register */

} SER_DATA, *SER_DATA_PTR;

typedef struct par_data {                 /* parallel port scratch pad area */
  ROS_TEST_TYPE test_mode;     /* the post test mode                        */
  int detected_error;          /* 0 implies no error detected               */
                               /* != 0 implies device specific error        */
  int adapter_present;         /* 0 implies not present, != 0 is present    */
  int adapter_bad;             /* only valid if adapter_present != 0,       */
                               /* 0 implies good, != 0 implies bad          */

} PAR_DATA, *PAR_DATA_PTR;

typedef struct rsc_data {      /* single chip scratch pad area              */
  ROS_TEST_TYPE test_mode;     /* the post test mode                        */
  int detected_error;          /* 0 implies no error detected               */
                               /* != 0 implies device specific error        */
  int adapter_present;         /* 0 implies not present, != 0 is present    */
  int adapter_bad;             /* only valid if adapter_present != 0,       */
                               /* 0 implies good, != 0 implies bad          */

  int my_variables_go_here;
} RSC_DATA, *RSC_DATA_PTR;

typedef struct lega_data {     /* low end graphics adapter scratch pad area */
  ROS_TEST_TYPE test_mode;     /* the post test mode                        */
  int detected_error;          /* 0 implies no error detected               */
                               /* != 0 implies device specific error        */
  int adapter_present;         /* 0 implies not present, != 0 is present    */
  int adapter_bad;             /* only valid if adapter_present != 0,       */
                               /* 0 implies good, != 0 implies bad          */
/*
 * The position of the cursor
 */
int curx;
int cury;
/*
 * Interrupt is expected
 */
int wait_for_int;
/*
 * What slots have valid legas
 */
char in_slot[MAX_SLOT_NUM +1];
/*
 * Pointer to fonts.
 */
uint *font_bitmap;
} LEGA_DATA, *LEGA_DATA_PTR;

typedef struct keybd_data {                    /* keyboard scratch pad area */
  ROS_TEST_TYPE test_mode;     /* the post test mode                        */
  int detected_error;          /* 0 implies no error detected               */
                               /* != 0 implies device specific error        */
  int adapter_present;         /* 0 implies not present, != 0 is present    */
  int adapter_bad;             /* only valid if adapter_present != 0,       */
                               /* 0 implies good, != 0 implies bad          */

/***************************************************************************/
/*                                                                         */
/*                   KB POST and DIR interface                             */
/*                                                                         */
/***************************************************************************/

  int    keyboard_present;      /* Keyboard Attached */
  int    keyboard_bad;          /* Keyboard bad */

/*The following field will be filled in only if keyboard_present field is set */
  int    kb_type;               /* Type of keyboard attached to machine */
/*****************************************************************************/
/*                                                                           */
/*                         Keyboard POST interface                           */
/*                                                                           */
/*****************************************************************************/


  ulong     kb_name1;          /* Debug labels that are assigned in kb_post*/
  ulong     kb_name2;

/*****************************************************************************/
/*                                                                           */
/*                    Keyboard POST Path test array variables                */
/*                                                                           */
/*****************************************************************************/

  ulong     kb_path_test;     /* Address of memory where test flags are     */
  ulong     kb_path_result;   /* Address of memory where result flags are   */

/*****************************************************************************/
/*                                                                           */
/*                    Keyboard Internal Trace Table Variables                */
/*                                                                           */
/*****************************************************************************/
#define KB_TT_SIZE   (1000)    /* Set up for 1000 words of trace            */
   ulong kb_nexthole;          /* Index number to next available trace word */
   ulong kb_tt_label[5];
   ulong kb_table[KB_TT_SIZE]; /* Trace Table array                        */
#define KB_NUM_PATHS 28
#define KB_PR_SIZE (100)       /* Needs to be enough for one path's traces */
   uchar kb_paths[KB_NUM_PATHS];
   ushort kb_current_path;
   ulong kb_path_index;
   ulong kb_path_records[KB_PR_SIZE];

} KEYBD_DATA, *KEYBD_DATA_PTR;

typedef struct ram_data {                           /* ram scratch pad area */
  ROS_TEST_TYPE test_mode;     /* the post test mode                        */
  int detected_error;          /* 0 implies no error detected               */
                               /* != 0 implies device specific error        */
  int adapter_present;         /* 0 implies not present, != 0 is present    */
  int adapter_bad;             /* only valid if adapter_present != 0,       */
                               /* 0 implies good, != 0 implies bad          */

uint cfg_tbl_array[8][2];               /* Memory config table in logical   *
                                         * slot order                       */
uint bad_simm_report[8];                /* In physical slot order (A-H),    */
        /*                                                                  *
         *      each bad_simm_report word contains the following for the    *
         *      SIMM represented:                                           *
         *                                                                  *
         *      byte 0      byte 1      byte 2      byte 3                  *
         *   +-----------+-----------+-----------+-----------+              *
         *   |   Error   | ORed SBAR |ORed Error |   Error   |              *
         *   |   Count   | syndromes |  Types    |   Class   |              *
         *   +-----------+-----------+-----------+-----------+              *
         *                                                                  *
         *   Error class: 0=No error, 1=data error, 2=address error         *
         *                                                                  *
         *   If error class == 1, then                                      *
         *                                                                  *
         *      Error count = number of data errors (ceiling is 0xFF)       *
         *                                                                  *
         *      SBAR syndromes =  all Single-bit and Uncorrectable Error    *
         *                        Syndromes ORed                            *
         *                                                                  *
         *      Error types = all data error types ORed                     *
         *               where 1=Single-bit, 2=Uncorrectable, 4=Miscompare  *
         *                                                                  */

uint simm_size[8];                      /* In physical slot order (A-H),    *
                                         * size of simm [Meg]               */
uint mem_scratch_pad[16];
uint tcw_addr;
uint tcw_size;
uint *mm_bitmap_ptr;            /* pointer to the bitmap used for memory   */
                                /* management in the C environment         */
} RAM_DATA, *RAM_DATA_PTR;

/***************************************************************************/
/*                                                                         */
/*                   SGA POST and DIR interface                            */
/*                   Power Gt1                                             */
/*                                                                         */
/***************************************************************************/
typedef struct sga_data {                        /*  scratch pad area */
  ROS_TEST_TYPE test_mode;     /* the post test mode                        */
  int detected_error;          /* 0 implies no error detected               */
                               /* != 0 implies device specific error        */
  int adapter_present;         /* 0 implies not present, != 0 is present    */
  int adapter_bad;             /* only valid if adapter_present != 0,       */
                               /* 0 implies good, != 0 implies bad          */

  int disp_typ;                   /* Type of display attached to machine */
  uint vpd_id;                    /* ID value from VPD register 0 */
  uint vpd_ec;                    /* EC value from VPD register 1 */
  uint mem_size;                  /* # of SIMMS installed for memory */
  uint mem_add_fail;              /* Address of first (if any) error */
  uint mem_size_good;             /* Size of contigious good display memory */
  int bad_simm[SGA_SIMM];         /* which simm failed memory test */
  int bad_simm_cnt;
  int dsi;                        /* Data Store Interrupt (DSI) occured */

  /* DIR VARIABLES */

  int line_num;                   /* current line number in screen */
  int cur_loc;                    /* cursor location on current line */

  uint *font_bitmap;
  uint *font_bitmap640;

   /* POST special variables, different per monitor */
   uint vblnk_min;
   uint vblnk_max;

   /* POST sga_testmem data values */
   struct mem_data {
        int  values[8];
        uint lin_add1;
        uint lin_add2;
        uint xy_add1;
        uint xy_add2;
   } mem10_data[5], mem6_data[5];
} SGA_DATA, *SGA_DATA_PTR;

typedef struct fm2_data {                      /* family 2 scratch pad area */
  ROS_TEST_TYPE test_mode;     /* the post test mode                        */
  int detected_error;          /* 0 implies no error detected               */
                               /* != 0 implies device specific error        */
  int adapter_present;         /* 0 implies not present, != 0 is present    */
                               /* bit mapped:                               */
                               /*  bit 31 = slot 0, bit 30 = slot 1 etc...  */

  int adapter_bad;             /* only valid if adapter_present != 0,       */
                               /* 0 implies good, != 0 implies bad          */

  /* The array defined below is designed to handle all adapters in the      */
  /* environment, including native adapters as well as IO slot adapters. In */
  /* the general case, a slot value logically ranges from 0 - 15 and there  */
  /* may be NUM_OF_BUIDS buids on which slots exist. This results in a      */
  /* total of NUM_OF_BUIDS*(MAX_SLOT_NUM + 1) supported adapters in a given */
  /* environment. To convert from the array_index_value to the buid/slot    */
  /* values (or vica/versa) use the following formulas:                     */
  /* buid_value = (array_index_value/(MAX_SLOT_NUM + 1)) + BUID20;          */
  /* slot_value =  array_index_value%(MAX_SLOT_NUM + 1);                    */
  /* array_index_value =                                                    */
  /*                 (buid_value - BUID20)*(MAX_SLOT_NUM + 1) + slot_value; */

  /* this array contains the pos ids for all detected adapters in the       */
  /* environment                                                            */
  unsigned int adapter_devid[_NUM_OF_BUIDS*(MAX_SLOT_NUM+1)];
} FM2_DATA, *FM2_DATA_PTR;

typedef struct net_data {                      /* net boot scratch pad area */
  ROS_TEST_TYPE test_mode;     /* the post test mode                        */
  int detected_error;          /* 0 implies no error detected               */
                               /* != 0 implies device specific error        */
  int adapter_present;         /* 0 implies not present, != 0 is present    */
  int adapter_bad;             /* only valid if adapter_present != 0,       */
                               /* 0 implies good, != 0 implies bad          */

  t_bootp bootpr;                  /* BOOTP Reply packet contents.  Filled    */
                                   /* in if a valid BOOTP reply was received, */
                                   /* otherwise will be zeros                 */
  IP_HW_ADDRS client_IP_HW;        /* Client IP and hardware addresses.       */
  ARP_ENTRY server_IP_HW;          /* BOOTP server IP, HW, and routing info   */
  ARP_ENTRY gateway_IP_HW;         /* Gateway IP, HW, and routing info        */
  ARP_ENTRY tftp_IP_HW;            /* TFTP server IP, HW, and routing info    */
  ARP_ENTRY red_gw_IP_HW;          /* Redirected GW IP, HW, and routing info  */
  ARP_ENTRY bootpr_IP_HW;          /* IP, HW, and routing info for BOOTP rep  */
  unsigned int network_type;       /* Token Ring or Ethernet                  */
  unsigned int slot_number;        /* Network adapter slot number             */
  int is_802_3;                    /* 802.3 Ethernet protocol flag            */
  int eth_dir_type;                /* Integrated ethernet or plug-in          */
  unsigned short ip_id;            /* IP identification counter               */
  unsigned int rand;               /* Random number for transaction ID's      */
  unsigned short tftp_serv_tid;    /* Transaction ID for server port          */
  unsigned short tftp_loc_tid;     /* Transaction ID for local port           */
  unsigned int adapter_open;       /* Adapter state flag                      */
  unsigned int send_counter;       /* Counter for display of packets sent     */
  unsigned int rcv_counter;        /* Counter for display of packets received */
  unsigned int net_timeout;        /* Timeout value for network reads (secs)  */
  char trans_buf[2048];            /* Transmit buffer                         */
  char rec_buf[2048];              /* Receive buffer                          */
  char frag_buf[2048];             /* Fragmentation buffer                    */

} NET_DATA, *NET_DATA_PTR;

typedef struct csc_data {                           /* csc scratch pad area */

  /* generic control block for serial port A (First serial port) */
  int serA_test_mode;
  int serA_detected_error;
  int serA_adapter_present;
  int serA_adapter_bad;

  /* generic control block for tablet port */
  int tab_test_mode;
  int tab_detected_error;
  int tab_adapter_present;
  int tab_adapter_bad;

  int csc_mode;   /* Mode of the machine, see ROS_TT_TYPE in rosdefs.h */

} CSC_DATA, *CSC_DATA_PTR;

typedef struct menu_data {                       /* usrmgr scratch pad area */
  ROS_TEST_TYPE test_mode;     /* the post test mode                        */
  int detected_error;          /* 0 implies no error detected               */
                               /* != 0 implies device specific error        */
  int adapter_present;         /* 0 implies not present, != 0 is present    */
  int adapter_bad;             /* only valid if adapter_present != 0,       */
                               /* 0 implies good, != 0 implies bad          */

#define NUM_TEXT        1000    /* number of text strings */
#define NUM_MENU        28      /* number of menus */
#define SCRN_SIZE       2000    /* size of display in bytes */
                                /* max number of possible menu items   */
                                /* allowing two per token ring adapter */
#define NUM_TASK (_NUM_OF_BUIDS*(MAX_SLOT_NUM+1)+10)
#define MAX_VAR_NUM     10      /* max # variable screens */

/***************************************************************************/
/*                                                                         */
/*                   Menu Manager and Menu Handler interface               */
/*                                                                         */
/***************************************************************************/

/* following described in sgeneric.h */
/* ROS_TEST_TYPE sga_test_mode;    The POST test type to be run */
/* int detected_error;             result error status */
/* int adapter_present;            adapter present */
/* int adapter_bad;                adapter bad */

char *text[NUM_TEXT];           /* array of ptrs to individual text strings */
char *format[NUM_MENU];         /* array of ptrs to individual format */
                                /* control strings */

int lang_val;                   /* language chosen by user */
int current_menu;               /* current menu on screen */
int current_var_menu;           /* current page in variable menus */

/* stores built menus and other items associated with each menu */
struct menustr {
   char bltmenu[SCRN_SIZE];     /* a complete menu */
   int num_tasks;               /* number of selections on each menu */
   struct taskstr {
      int fun;                  /* functions associated with menu selection */
      int val;                  /* value passed to function */
   } task[NUM_TASK - 1];
} menu[NUM_MENU];

/* used for multi page menus - config & diag test selections */
char special_menu[MAX_VAR_NUM][SCRN_SIZE];
int special_page;
int special_num2build;

/* PING ip vars */
int ip_select;                  /* which address user is entering */
char ip_local[16];              /* ping local address */
char ip_remote[16];             /* ping remote address */
char ip_gateway[16];            /* ping for gateway address */
uint ip_results;                /* results of ping test */

#define CFG_TABLE_SIZE 50
/* config table vars */
struct config_table_entry {
        int cntrl_byte;         /* used by diagnostics    */
        char location[10];      /* ABCDEFGHH \0           */
        int device_id;          /* Defined in menudefs.h  */
        int modifier;           /* device dependent field */
} CFG_TABLE[CFG_TABLE_SIZE];

int num_cfg;
int cur_cfg;

/* boot adapter selected vars */
int bootp_adapter;              /* which boot adapter was selected */
int bootp_select;               /* which address user is entering */
char bootp_client[16];          /* bootp client address */
char bootp_server[16];          /* bootp server address */
char bootp_gateway[16];         /* bootp gateway address */
int network_boot;               /* TRUE  => override exiting bootlist */
                                /* FALSE => donot override exiting bootlist */
int boot_failed;                /* TRUE  => put up BOOT FAILED message */
                                /* FALSE => donot put up BOOT FAILED message */
int  slot_info_index;           /* index into the slot information array */
int  menu_buid;                 /* the applicable bus unit id */
char slot_info[MAX_SLOT_NUM+1]; /* information for network boot */
int  adptlist[NUM_TASK];        /* stores adapter info displayed on screen */
int  adpt_pages;                /* number of select adapter pages */

/* ethernet cable vars */
int eth_trans;                  /* ethernet transceiver type (IBM/non-IBM) */
int eth_hear;                   /* ethernet Hearbeat (ON/OFF) */

} MENU_DATA, *MENU_DATA_PTR;

typedef struct console_data {                   /* console scratch pad area */
  ROS_TEST_TYPE test_mode;     /* the post test mode                        */
  int detected_error;          /* 0 implies no error detected               */
                               /* != 0 implies device specific error        */
  int adapter_present;         /* 0 implies not present, != 0 is present    */
  int adapter_bad;             /* only valid if adapter_present != 0,       */
                               /* 0 implies good, != 0 implies bad          */

/*
 * Here are the variables...
 */
int con_called;
int keyboard_sys;
int keyboard_asynch;
int display_sky;
int display_sga;
int display_lega;
int display_asynch;
int init_sys_kb;
int no_display;
int con_service;
char trans_tbl[128];
int key_service;
int kb_switch_test;
int asynch_open;
int key_normal;
int display_taur;
int display_vrs;
} CONSOLE_DATA, *CONSOLE_DATA_PTR;

typedef struct diagnostics_data {               /* console scratch pad area */
  ROS_TEST_TYPE test_mode;     /* the post test mode                        */
  int detected_error;          /* 0 implies no error detected               */
                               /* != 0 implies device specific error        */
  int adapter_present;         /* 0 implies not present, != 0 is present    */
  int adapter_bad;             /* only valid if adapter_present != 0,       */
                               /* 0 implies good, != 0 implies bad          */

/* Diag manager variables */
int runall;  /* =1 run all tests, =0 run only selected test */
int multiple;  /* run test/s continious */
int service;  /* =1 service mode, =0 user mode */

int testcnt; /* initialized test count value */
int rtncode; /* post return code */

int config_index; /* initialize the config table index */
int setup_index; /* initialize the setup table index */
int special_msg; /* special instruction needed on att/rem diag equip menu */

char srn[8];


#define   DIAG_NUM_SETUP  9             /* number of entries in setup table */
/* setup table array of structures */
struct setup_table
  {
  int device_id;
  char *smenu;               /* start menu  */
  int equip_num;
  char pn[10];
  char *fmenu;               /* finish menu */
  int attach_msg;             /* start menu special instruction 1st msg # */
  int attach_len;             /* # of msgs in special instruction */
  int remove_msg;            /* finish menu special instruction 1st msg # */
  int remove_len;            /* # of msgs in special instruction */
  } setup [DIAG_NUM_SETUP];
} DIAG_DATA, *DIAG_DATA_PTR;

/****************************************************************************/
/*                            ROM SCAN interface                            */
/****************************************************************************/

/**************************************************************************
 ROM Scan area in Control Block.
  There is an array of 16 structures in the S21 area of the IPLCB for ROM
  Scan. Each element of this structure will hold the ROM Scan Data (defined
  in the structure below )for a RS/6000 ROM that might be found in the
  corresponding slot, i.e. element 0 holds information for an RS/6000 ROM
  found in slot 0, etc.
**************************************************************************/

typedef struct adapter_slot {                            /* generic presents */
  ROS_TEST_TYPE test_mode;        /* the post test mode                      */

  int detected_error;             /* 0- implies no error detected            */
                                  /* 1- RS/6000 ROM Failed CRC               */
                                  /* 2- Address of RS/6000 ROM was invalid   */
                                  /* 3- PS/2 ROM failed the Checksum         */

  int adapter_present;            /* 0- No RS/6000 ROM Scan adapter was      */
                                  /*    found in this slot.                  */
                                  /* 1- A valid PS/2 ROM is found and it     */
                                  /*    contains the "RISC6000" flag         */

  int adapter_bad;                /* only valid if adapter_present != 0,     */
                                  /* 0 implies good, else ...                */
                                  /* 1- The RISC/6000 ROM failed the CRC     */
                                  /* 2- Address of RS/6000 ROM was invalid   */

   uint  RISC6000_ROM_address;    /* Address of RISC/6000 ROM when enabled   */
   uchar POS_reg_0_setting;       /* Adapter ID                              */
   uchar POS_reg_1_setting;       /* Adapter ID                              */
   uchar POS_reg_2_setting;       /* enables RISC6000 ROM                    */
   uchar POS_reg_3_setting;       /* enables RISC6000 ROM                    */
   uchar POS_reg_4_setting;       /* enables RISC6000 ROM                    */
   uchar POS_reg_5_setting;       /* enables RISC6000 ROM                    */
  ushort RISC6000_length;         /* Length, RISC6000 ROM in 512 byte blocks */
  ushort RISC6000_ROM_CRC;        /* 16-bit CRC for RISC/6000 ROM            */
  uint   RISC6000_ROM_type;       /* Type ROM: 1=Device Boot, 2=Video,3=both */
  uint   dev_boot_expan_code;     /* Offset to Boot expansion code           */
  uint   dev_boot_data_size;      /* Length of required data area for Device */
  uchar Reserved[20];
 } ADAPTER_SLOTS, *ROM_SCAN_ADAPTERS_PTR;


typedef struct rom_scan_adapters {
 ADAPTER_SLOTS slots;    /* ROM Scan data for each slot                      */
 int dsi;                /* Data Storage Interrupt Flag: set to 0x524F4D53   */
                         /*   when ROM Scan Expects a DSI to occur           */
 int ROM_location_in_mem; /* Location of ROM in memory */
 int ROM_data_in_mem;   /* Location of Data area for ROM in memory */
} ROM_SCAN_ADAPTERS;

/*****************************************************************************
 *                                                                           *
 *                      VIDEO ROM SCAN Scratch Pad                           *
 *                                                                           *
 *****************************************************************************/
typedef struct video_rom_adapter {

  ROS_TEST_TYPE test_mode;        /* the post test mode                      */
  int detected_error;
  int adapter_present;
  int adapter_bad;

  void * vrs_mem_ptr ;           /* Pointer to Microchannel VRS Intface struc*/
  uint vrs_scratch_ptr;          /* ptr to start of unused memory in VRS     */
  uint vrs_high_boundry_ptr;     /* ptr to end of VRS free memory            */
  uint curx, cury;               /* Current cursor location for all VRS disps*/
  uint font_bitmap;              /* pointer to fonts bitmap used by VRS      */
  uint font_bitmap640;           /* pointer to 640x480 fonts bitmap          */
}  VIDEO_ROM_DATA;

/*****************************************************************************
 *                                                                           *
 *                     BUID 40 VIDEO ROM SCAN Scratch Pad                    *
 *                                                                           *
 *****************************************************************************/
typedef struct vrom40_data {   /* video rom scan buid 40scratch pad area    */
  ROS_TEST_TYPE test_mode;     /* the post test mode                        */
  int detected_error;          /* 0 implies no error detected               */
                               /* != 0 implies device specific error        */
  int adapter_present;         /* 0 implies not present, != 0 is present    */
  int adapter_bad;             /* only valid if adapter_present != 0,       */
                               /* 0 implies good, != 0 implies bad          */
  void *vrs_mem_ptr;           /* Pointer to Buid 40 Video ROM Scan Intface */
                               /* Structure.                                */
} VROM40_DATA, *VROM40_DATA_PTR;
/***************************************************************************/
/*                                                                         */
/*                   SKY    POST and DIR interface                         */
/*                   Power Gt1                                             */
/*                                                                         */
/***************************************************************************/
typedef struct sky_data {                        /*  scratch pad area */
  ROS_TEST_TYPE test_mode;     /* the post test mode                        */
  int detected_error;          /* 0 implies no error detected               */
                               /* != 0 implies device specific error        */
  int adapter_present;         /* 0 implies not present, != 0 is present    */
  int adapter_bad;             /* only valid if adapter_present != 0,       */
                               /* 0 implies good, != 0 implies bad          */
char in_slot[MAX_SLOT_NUM +1];
int curx;
int cury;
uint *font_bitmap;
} SKY_DATA, *SKY_DATA_PTR;

/****************************************************************************/
/*                                                                          */
/*           GLOBAL SCRATCH PAD AREA                                        */
/*                                                                          */
/* The variables that exist in this scratch pad are those variables that    */
/* are initialized by one given function (such as family 2 function), but   */
/* need to be interrogated and/or maintained by more than one function.     */
/*                                                                          */
/*                                                                          */
/****************************************************************************/
typedef struct adapt_info {
  ROS_TEST_TYPE test_mode;     /* the post test mode                        */
  int detected_error;          /* 0 implies no error detected               */
                               /* != 0 implies device specific error        */

  int adapter_present;         /* 0 implies not present, != 0 is present    */
  int adapter_bad;             /* only valid if adapter_present != 0,       */
                               /* 0 implies good, != 0 implies bad          */

  uint pos0_id;                /* 0xff IN BOTH pos0_id and pos1_id fields   */
  uint pos1_id;                /* indicate that there is no adapter present */
                               /* in the corresponding buid/slot. any other */
                               /* values are the values obtained from the   */
                               /* pos registers of the buid/slot            */

  uint buid_value;             /* buid to which this data applies           */
  uint slot_value;             /* the slot number within the given buid     */

  uint scan_code_present;      /* romscan post controls this flag           */
                               /* == 0, no test performed for romscan code  */
                               /* == 1, implies romscan code device boot    */
                               /*       code detected on this card          */
                               /* == 2, implies video romscan code detected */
                               /*       on this card.                       */
                               /* == 3, implies both video and device boot  */
                               /*       code are present on this card       */
                               /* == 4->7, Reserved                         */
                               /* == 8 & slot <= 7, no romscan code found   */
                               /* == 8 & slot >  7, no romscan code allowed */
                               /* == 9, A PS/2 ROM failed Checksum          */
                               /* ==10, RISC/6000 ROM CRC failure           */
                               /* ==11, Address of RISC/6000 ROM Invalid    */
  uint supports_ipl;           /* == 0 => no, != 0 => adapter supports ipl  */

#define HW_ADDR_BYTE_LEN 16    /* Usually six bytes, allow for expansion.   */
                               /* The actual length is dictated by the      */
                               /* adapter type. i.e., token, ethernet, etc. */

  uchar hw_net_addr[HW_ADDR_BYTE_LEN]; /* The network address array,        */
                                       /* beginning in byte 0               */
  struct harr_mbox0_data *hmb_ptr;
  uint user_defined3;
  uint adapter_flag;         /* used to identify bluebonnet or lace adapter */
  uint user_defined1;
  uint user_defined0;
  /* ONE MAY ONLY ADD TO THIS STRUCTURE BY USING THE RESERVED FIELDS!!!     */
  /* IF YOU CHANGE THE LENGTH OF THIS STRUCTURE IN ANY OTHER WAY, WE LOOSE  */
  /* COMPATABILITY WITH EXISTING KERNELS. CREATE A NEW STRUCTURE IF THIS    */
  /* ONE EVER BECOMES EXHAUSTED.                                            */
};

typedef struct global_spad {                          /*  scratch pad area */
  /* these flags controls the route to the menus when in service mode       */
  int  go_to_menues_flag;

  /* this flag == TRUE if and only if this system supports network boot     */
  int network_boot_support_flag;

  /* this flag == TRUE if and only if this system supports ros diagnostics  */
  int diagnostics_support_flag;

  /* this variable is set by ros code to the number of buids supported      */
  /* controlled by low level code, probably isc function                    */
  int num_of_buids;

  /* this variable is the buid value setup via nvram ipl selection and is   */
  /* controlled by ipl controller code (iplc)                               */
  int nvram_buid;

  /* This flag controls the ros "resume" point (i.e., from where to resume) */
  /* whenever loaded ipl code (e.g., romscan code) "returns to ros" via a   */
  /* simple return link (as opposed to returning by pushing the yellow      */
  /* button to attempt a warm ipl).                                         */
  /* The flag is controlled (i.e., set, reset) by a ros initialization      */
  /* routine (see ipl_init.c) and the "return to ros" routine               */
  /* (see the br_to_code() function). The flag must be "reset" by the code  */
  /* that runs at the point of resumption (all ros code will support the    */
  /* resume capability). There are multiple bits defined in this flag.      */
  /* Refer to rosdefs.h for the bit definitions.                            */
  uchar resume_flag;

  /* this flag keeps track of the current normal mode nvram list entry that */
  /* is trying to ipl when a resume has taken place                         */
  uchar resume_nvram_item_count;

  uchar last_dsi_count;    /* number of times the last dsi has been retried */
  uchar resvd0;

  uint last_dsi_addr;      /* the address of the last dsi to occur          */
  uint user_defined8;
  uint user_defined7;
  uint user_defined6;
  uint user_defined5;
  uint user_defined4;
  uint user_defined3;
  uint user_defined2;
  uint user_defined1;
  uint user_defined0;
  /* ONE MAY ONLY CHANGE TO THIS STRUCTURE BY USING THE RESERVED FIELDS!!!  */
  /* IF YOU CHANGE THE LENGTH OF THIS STRUCTURE IN ANY WAY, WE LOOSE        */
  /* COMPATABILITY WITH EXISTING KERNELS. CREATE A NEW STRUCTURE IF THIS    */
  /* ONE EVER BECOMES EXHAUSTED.                                            */

  /* The array defined below is designed to handle all fm2 adapters in the */
  /* environment, including native adapters as well as IO slot adapters. In */
  /* the general case, a slot value logically ranges from 0 - 15 and there  */
  /* may be NUM_OF_BUIDS buids on which slots exist. This results in a      */
  /* total of NUM_OF_BUIDS*(MAX_SLOT_NUM + 1) supported adapters in a given */
  /* environment. To convert from the array_index_value to the buid/slot    */
  /* values (or vica/versa) use the following formulas:                     */
  /* buid_value = (array_index_value/(MAX_SLOT_NUM + 1)) + BUID20;          */
  /* slot_value =  array_index_value%(MAX_SLOT_NUM + 1);                    */
  /* array_index_value =                                                    */
  /*                 (buid_value - BUID20)*(MAX_SLOT_NUM + 1) + slot_value; */

  /* this array is initialized by fm2 code, maintained by each post         */
  struct adapt_info fm2_adapt_info[_NUM_OF_BUIDS*(MAX_SLOT_NUM+1)];

  /* ONE MAY ONLY CHANGE TO THIS STRUCTURE BY USING THE RESERVED FIELDS!!!  */
  /* IF YOU CHANGE THE LENGTH OF THIS STRUCTURE IN ANY WAY, WE LOOSE        */
  /* COMPATABILITY WITH EXISTING KERNELS. CREATE A NEW STRUCTURE IF THIS    */
  /* ONE EVER BECOMES EXHAUSTED.                                            */
} GLOBAL_DATA, *GLOBAL_DATA_PTR;

typedef struct mouse_data {         /* mouse and mouse port scratch pad area */
  ROS_TEST_TYPE test_mode;     /* the post test mode                        */
  int detected_error;          /* 0 implies no error detected               */
                               /* != 0 implies device specific error        */
  int adapter_present;         /* 0 implies not present, != 0 is present    */
  int adapter_bad;             /* only valid if adapter_present != 0,       */
                               /* 0 implies good, != 0 implies bad          */

  int    mouse_present;      /* Mouse Attached */
  int    mouse_bad;          /* Mouse bad */


  ulong     kb_name1;          /* Debug labels that are assigned in kb_post*/
  ulong     kb_name2;

  ulong     ms_path_test;     /* Address of memory where test flags are     */
  ulong     ms_path_result;   /* Address of memory where result flags are   */

/*****************************************************************************/
/*                                                                           */
/*                    Keyboard Internal Trace Table Variables                */
/*                                                                           */
/*****************************************************************************/
#define MS_TT_SIZE   (100)    /* Set up for 100 words of trace            */
   ulong ms_nexthole;          /* Index number to next available trace word */
   ulong ms_tt_label[5];
   ulong ms_table[MS_TT_SIZE]; /* Trace Table array                        */
#define MS_NUM_PATHS 28
#define MS_PR_SIZE (100)       /* Needs to be enough for one path's traces */
   uchar ms_paths[MS_NUM_PATHS];
   ushort ms_current_path;
   ulong ms_path_index;
   ulong ms_path_records[KB_PR_SIZE];

} MOUSE_DATA, *MOUSE_DATA_PTR;

typedef struct taur_data {     /* taurus graphics adapter scratch pad area  */
  ROS_TEST_TYPE test_mode;     /* the post test mode                        */
  int detected_error;          /* 0 implies no error detected               */
                               /* != 0 implies device specific error        */
  int adapter_present;         /* 0 implies not present, != 0 is present    */
  int adapter_bad;             /* only valid if adapter_present != 0,       */
                               /* 0 implies good, != 0 implies bad          */

  /* x, y cursor position */
  int curx;
  int cury;

  /* pointer to fonts */
  uint *font_bitmap;
} TAUR_DATA, *TAUR_DATA_PTR;



/*****************************************************************************/
/*                                                                           */
/*                    3Com Ethernet POST & DIR interface                     */
/*                                                                           */
/*****************************************************************************/
typedef struct ent_data {                      /* ethernet scratch pad area */
  ROS_TEST_TYPE test_mode;     /* the post test mode                        */
  int mau_test_type;           /* Advanced diagnostic ext wrap test type    */
  int detected_error;          /* 0 implies no error detected               */
                               /* != 0 implies device specific error        */
  int adapter_present;         /* 0 implies not present, != 0 is present    */
  int adapter_bad;             /* only valid if adapter_present != 0,       */
                               /* 0 implies good, != 0 implies bad          */

  int transmit_error;           /* Transmit status set by DIR for POST */

  /* Trace tables */
  uint *post_trace_table;
  uint *dir_trace_table;

  /* Pointer to memory allocated for DIR control structures and buffers.     */
  void *ent_ctl;

  /* Pointer to private data of ent_public (general purpose) functions. */
  void *ent_public;

} ENT_DATA, *ENT_DATA_PTR;

typedef struct proc_post_data { /* processor post scratch pad area          */
  ROS_TEST_TYPE test_mode;      /* the post test mode                       */
  int detected_error;           /* 0 implies no error detected              */
                                /* != 0 implies device specific error       */
  int adapter_present;          /* 0 implies not present, != 0 is present   */
  int adapter_bad;              /* only valid if adapter_present != 0,      */
                                /* 0 implies good, != 0 implies bad         */
  void *extension;              /* For further extention                    */
} PROC_POST_DATA, *PROC_POST_DATA_PTR;

typedef struct ram46_data {    /* ram post scratch pad area for model 0x46  */

uint bad_simm_report[8];                /* In physical slot order (A-H),    */
        /*                                                                  *
         *      each bad_simm_report word contains the following for the    *
         *      SIMM represented:                                           *
         *                                                                  *
         *      Bit0-Bit3   : Unused. Always set to 0.                      *
         *      Bit4        : Address bus error flag.                       *
         *      Bit5        : Data bus error flag.                          *
         *      Bit6-Bit18  : Single-bit ECC error counter.                 *
         *      Bit19-Bit31 : Double-bit ECC error counter.                 *
         *                                                                  */

uint simm_size[8];                      /* In physical slot order (A-H),    *
                                         * size of simm in terms of MB.     */

uint mem_config_table[4];
        /*                                                                  *
         *      each mem_config_table word contains the following for the   *
         *      memory extent its index represents:                         *
         *                                                                  *
         *      Bit0-Bit19  : Unused. Always set to 0.                      *
         *      Bit20-Bit21 : Physical extent ID of this memory extent      *
         *      Bit22       : Indicate failing SIMM pair within a quad when *
         *                    failing data bus test. If set, data bus of    *
         *                    odd-double-word-address SIMM pair is bad.     *
         *      Bit23       : Set if addressbus problem found in the extent *
         *      Bit24       : Set if data bus problem found in the extent   *
         *      Bit25-Bit31 : Extent size in MB.                            *
         *                                                                  */

uchar *mem_bitmap_ptr;     /* To be pointing to the beginning of the bitmap */
void *tce_info;            /* For TCE management usage */
uint relocate_constant;    /* RAM relocate constant of IPL ROS code    */
void *extension;           /* For further extention */

} RAM46_DATA, *RAM46_DATA_PTR;

typedef struct buid_post {
  ROS_TEST_TYPE test_mode;      /* the post test mode                       */
  int  detected_error;          /* 0 implies no error detected              */
                                /* != 0 implies device specific error       */
  int  adapter_present;         /* 0 implies not present, != 0 is present   */
  int  adapter_bad;             /* only valid if adapter_present != 0,      */
                                /* 0 implies good, != 0 implies bad         */
  void *info;                   /* pointer to post results structure        */
} BUID_POST, *BUID_POST_PTR;

typedef struct system_info {  /* REFER TO NOTE 1 BELOW                    */
  int num_of_procs;           /* number of processors supported           */
                              /* num_of_procs == 1 => Uni; > 1 => n-way MP*/
                              /* the value refers to the number this      */
                              /* platform is designed to handle, must look*/
                              /* at processor_info structure to determine */
                              /* the number actually present              */
  int coherency_size;         /* size of the coherence block              */
  int resv_size;              /* size of the reservation granule          */
  void *arb_cr_addr;          /* real address of arbiter control reg      */
  void *phys_id_reg_addr;     /* real address of physical id reg          */
                              /* PHYSICAL IDENTIFIER REG                  */
  int num_of_bsrr;            /* number of 4 byte bus slot reset regs     */
  void *bsrr_addr;            /* real address of bus slot reset reg       */
                              /* PSUEDO-COMPLETE CONNECTIVITY RESET REG   */
  int tod_type;               /* type of time of day structure            */
  void *todr_addr;            /* real address of time of day regs         */
                              /* TIME OF DAY REGS                         */
  void *rsr_addr;             /* real address of reset status reg         */
                              /* SYSTEM RESET COUNT REGISTER              */
  void *pksr_addr;            /* real address of power/keylock status reg */
                              /* POWER STATUS/KEYLOCK REG                 */
  void *prcr_addr;            /* real address of power/reset control reg  */
                              /* SOFTWARE POWER ON RESET CONTROL REG      */
  void *sssr_addr;            /* real address of system specific regs     */
                              /* SYSTEM SPECIFIC SYSTEM REGISTERS         */
  void *sir_addr;             /* real address of system interrupt regs    */
                              /* ARCHITECTED SYSTEM INTERRUPT REGS        */
  void *scr_addr;             /* real address of standard config reg      */
                              /* ARCHITECTED CONFIGURATION REGS           */
  void *dscr_addr;            /* real address of device specific cfg reg  */
                              /* DEVICE SPECIFIC CONFIGURATION REGS       */
  int nvram_size;             /* byte size of nvram                       */
  void *nvram_addr;           /* real address of nvram                    */
                              /* NVRAM                                    */
  void *vpd_rom_addr;         /* real address of VPD ROM space            */
                              /* ARCHITECTED FEATURE/VPD ROM SPACE        */
  int ipl_rom_size;           /* byte size IPL ROM space                  */
  void *ipl_rom_addr;         /* real address of IPL ROM space            */
                              /* BOOT ROM                                 */
  void *g_mfrr_addr;          /* real address of global mfrr reg  if != 0 */
  void *g_tb_addr;            /* real address of global time base if != 0 */
  int g_tb_type;              /* global time base type                    */
  int g_tb_mult;              /* global time base multiplier              */
  int SP_Error_Log_Table;     /* offset from base address of NVRAM to     */
                              /* Service Processor Error Logging Table    */
  void *pcccr_addr;           /* real address of connectivity config reg  */
                              /* PSUEDO-COMPLETE CONNECTIVITY CONFIG REG  */
  void *spocr_addr;           /* real address software power off cntrl reg*/
                              /* SOFTWARE POWER OFF CONTROL REG           */
  void *pfeivr_addr;          /* real address of                          */
                              /* SMP POWER FAILURE (EPOW) EXTERNAL        */
                              /* INTERRUPT VECTOR REG                     */

                              /* beginning of APM interface (See Note 2)  */
  int access_id_waddr;        /* type of access to "loc_waddr"            */
  void *loc_waddr;            /* real address of APM space write          */
  int access_id_raddr;        /* type of access to "loc_raddr"            */
  void *loc_raddr;            /* real address of APM space read           */
                              /* end of APM interface                     */
  int architecture;           /* The model architecture of this platform  */
                              /* RS6K => RISC/6000 system architecture    */
                              /* RSPC => "PReP" system architecture       */
                              /* RS6K = 1, RSPC = 2,                      */
                              /* see AIX systemcfg.h for future values    */
  int implementation;         /* The model implementation of this platform*/
                              /* RS6K_UP_MCA => RS1, RS2, RSC, PPC UP     */
                              /* RS6K_SMP_MCA => PPC SMP                  */
                              /* RSCP_UP_PCI => PPC/PReP UP               */
                              /* RS6K_UP_MCA = 1, RS6K_SMP_MCA = 2,       */
                              /* RSCP_UP_PCI = 3                          */
                              /* see AIX systemcfg.h for future values    */
  char pkg_descriptor[16];    /* NULL terminated ASCII string             */
                              /*                                          */
                              /* EX:     'r','s','6','k','\0'             */
                              /* "rs6k" => RS1,RS2,RSC, RS6K/PPC UP models*/
                              /* "rs6ksmp" => RISC6000/PowerPC SMP models */
                              /* "rspc" => PReP/PowerPC models            */
} SYSTEM_INFO, *SYSTEM_INFO_PTR;

/****************************************************************************
   THE FOLLOWING NOTES APPLY TO THE system_info STRUCTURE DEFINED ABOVE

1. Most of these fields had been defined previous to the RISC System/6000
architecture document being approved. Those fields that refer to fields
in the architecture document contain the architecture nameing convention
in capital letters following the structure name. E.G.

  void *bsrr_addr;             * real address of bus slot reset reg       * 
                               * PSUEDO-COMPLETE CONNECTIVITY RESET REG   * 

2. The APM (Available Processor Mask) interface:
The loc_raddr and loc_waddr addresses are 32-bit word aligned addresses
and software is expected to access from them in 32-bit word mode.  The
space represented by *loc_raddr and *loc_waddr (which we will refer to
as APM space) represents a bitmap string in which the state of each bit
defines which processors in an SMP environment, numbered 1 through N,
are enabled and which are disabled. At least N sequential bits of APM
space must be implemented.

Processor number 1 is represented by the high order bit of the first
word with each successive bit representing the next processor number in
turn, and processor number 33 being represented by the high order bit
of the next word, etc.

The number of read words (and write words) reserved in APM space
must be at least QUOTIENT((num_of_procs - 1)/32)+1, were num_of_procs
are the number of processors supported on a given platform. To enable,
disable, or verify a given processor, software must create a 32 bit
mask and the correct APM space read/write address using the following
algorithm or equivalent:

proc_mask = (1 >> REMAINDER((proc_number - 1)/32));
wr_word_number = QUOTIENT((proc_number - 1)/32);

where the single active "1" bit in the 32-bit mask "proc_mask"
cooresponds to the processor whose number is "proc_number"; and where
wr_word_number is the zero-based 32-bit word-offset value into APM
space to be added to the read or write address (loc_raddr or loc_waddr)
to address the proper 32-bit read or write address, in APM space,
which contains the bit that cooresponds to "proc_number". NOTE:
wr_word_number is a 32-bit word offset and will need to be multiplied
by 4 to get a byte offset if required.

access_id_waddr and access_id_raddr defines the type of access required
to access APM space. 0 implies the access is a normal memory mapped
access using loads and stores. 1 implies the access must be through the
AIX machine device driver or equivalent.

if loc_raddr == loc_waddr == 0, this interface is not supported via
the IPL control block.

For more information refer to the RISC System/6000 PowerPC System
Architecture document

   END OF NOTES APPLYING TO THE system_info STRUCTURE DEFINED ABOVE
****************************************************************************/

typedef struct buc_info {
  uint  num_of_structs;       /* contains the run time value corresponding*/
                              /* the number of buc's present              */
  uint  index;                /* 0 <= index <= num_of_structs - 1,        */
                              /* assume index = n, then this              */
                              /* is the n+1th array element               */
  uint struct_size;           /* size of this structure (in bytes)        */
  int bsrr_offset;            /* bus slot reset register offset           */
                              /* (see NOTE 2 below)                       */
  uint bsrr_mask;             /* bus slot reset register mask             */
                              /* a one bit only mask which identifies the */
                              /* bit that controls the reset of this buc  */
                              /* (see NOTE 2 below)                       */
  int bscr_value;             /* configuration register value to enable   */
                              /* configuration of this buc                */
  int cfg_status;             /*  0 => buc is not configured              */
                              /*  1 => buc is configured via config regs  */
                              /*  2 => configured via hardware defaults   */
                              /*       i.e., does not have config regs    */
                              /* -1 => configuration failed               */
  int device_type;            /*  1 => this buc is executable memory      */
                              /*  2 => this buc is a processor            */
                              /*  3 => this buc is an io type             */
  int num_of_buids;           /* num of buids required by this buc (<=4)  */
  struct buid_data {
    int  buid_value;          /* assigned BUID value                      */
                              /* these values have meaning if and only    */
                              /* if the num_of_buids != 0, they are       */
                              /* assigned in order until the num_of_buids */
                              /* is satisfied. Unused buids will = -1     */
    void *buid_Sptr;          /* pointer to this BUID's own post structure*/
                              /* a NULL value => N/A                      */
  } buid_data[4];
  int mem_alloc1;             /* 1st memory allocation required (in MB)   */
  void *mem_addr1;            /* real address of mem_alloc1 area          */
                              /* if mem_alloc1 != 0, otherwise N/A        */
  int mem_alloc2;             /* 2nd memory allocation required (in MB)   */
  void *mem_addr2;            /* real address of mem_alloc2 area          */
                              /* if mem_alloc2 != 0, otherwise N/A        */
  int vpd_rom_width;          /* width of vpd interface in bytes          */
                              /* -1 => not applicable, no vpd rom present */
  int cfg_addr_inc;           /* configuration address increment in bytes */
                              /* Refer to NOTE 4 below                    */
  int device_id_reg;          /* standard configuration register contents */
                              /* -1 => not applicable                     */
  uint aux_info_offset;       /* the iplcb offset to the "device_specific"*/
                              /* array defined for this buc. e.g.,        */
                              /* if this is a processor buc, the auxilary */
                              /* struct is the processor_info array struct*/
                              /* if the device type is io type, the aux   */
                              /* struct is an io_info array struct, etc.  */
                              /* Refer to NOTE 1 below                    */
  uint feature_rom_code;      /* romscan post controls this flag          */
                              /* == 0, no test performed for romscan code */
                              /* == 1, implies romscan code device boot   */
                              /*       code detected on this card         */
                              /* == 2, implies video romscan code detected*/
                              /*       on this card.                      */
                              /* == 3->7, Reserved                        */
                              /* == 8, no romscan code found              */
  uint IOCC_flag;             /* 0 = Not IOCC. 1 = IOCC                   */
  char location[4];           /* Location code of the BUC                 */
} BUC_DATA, *BUC_DATA_PTR;

/****************************************************************************
   THE FOLLOWING NOTES APPLY TO THE buc_info STRUCTURE DEFINED ABOVE

Notes on BUC data:
1. An aux_info_offset will always point to an array element that describes
   the device that is related to the buc. For example, if the buc is a
   processor, the aux_info_offset will point to the appropriate array
   element, within the per processor array, that is associated with this
   buc.  If there is more that one aux array element related to the buc
   (such as an IOCC buc, which can support an IO bus that can contain up
   to sixteen adapters), the aux_info_offset will point to the first array
   element in the array.

2. This value, bsrr_offset, added to the bsrr_address contained in struct
   system_info, provides the address for the appropriate bus slot reset
   register for this BUC. The value bsrr_mask, when effectively or'ed
   into the bus slot reset register, will enable the appropriate buc. The
   bit inversion of bsrr_mask, when effectively and'ed into the bus slot
   reset register, will reset the appropriate buc.

3. This array is pointed to by IPL directory entry "buc_info_offset"

4. This value, cfg_addr_inc, added to the scr_addr contained in struct
   system_info, provides the addressing for the standard configuration
   registers.

   THE ABOVE NOTES APPLY TO THE buc_info STRUCTURE DEFINED ABOVE
****************************************************************************/

typedef struct processor_info {
  uint  num_of_structs;       /* contains the run time value corresponding*/
                              /* to the number of processors supported    */
  uint  index;                /* 0 <= index <= num_of_structs - 1,        */
                              /* assume index = n, then this              */
                              /* is the n+1th array element               */
  uint struct_size;           /* size of this structure (in bytes)        */
  uint per_buc_info_offset;   /* the iplcb offset to the "buc_info"       */
                              /* structure related to this device         */
  void *proc_int_area;        /* Base Address (BA) of this processors     */
                              /* interrupt presentation layer registers   */
                              /* BA+0  (CPPR||XISR without side effects)  */
                              /* BA+4  (CPPR||XISR with    side effects)  */
                              /* BA+8  (DSIER)                            */
                              /* BA+12 (MFRR)                             */
                              /* BA+xx (Additional Optional MFRR's)       */
  uint proc_int_area_size;    /* proc_int_area_size/4 is the  number of   */
                              /* interrupt presentation registers         */
  int processor_present;      /*  0 implies not present, != 0 is present  */
                              /* -1 implies not operational (failed test) */
                              /*  1 implies processor is "running" AIX    */
                              /*  2 implies processor is "looping"        */
                              /*    with link = 0 (see NOTE 1 below)      */
                              /*  3 implies this processor is available   */
                              /*    in the reset state                    */
                              /*  additional values TBDefined             */
  uint test_run;              /* this is a bit significant variable       */
                              /* indicating which tests were run on this  */
                              /* processor. bits are or'ed for each test. */
                              /* the corresponding bit values are herein  */
                              /* defined:                                 */
                              /* bit 0 is the least significant bit       */
                              /* test was run if and only if bit == 1     */
                              /* bit: state definition:                   */
                              /* 0  : local address bus test              */
                              /* 1  : shared address bus test             */
                              /* 2  : local data bus test                 */
                              /* 3  : shared data bus test                */
                              /* 4  : local memory data test              */
                              /* 5  : shared memory data test             */
                              /* 6  : fixed point unit test               */
                              /* 7  : floating point unit test            */
                              /* undefined bits are reserved              */
  uint test_stat;             /* this status is valid whenever test_run   */
                              /* contains a non-zero value. a value of 1  */
                              /* signifies the test corresponding to the  */
                              /* test_run bit has failed, a value of 0    */
                              /* signifies the test passed. the test      */
                              /* results DO NOT necessarily imply the     */
                              /* processor_present value == -1            */
  int  link;                  /* if  = 0, loop until non-zero             */
                              /* if != 0, branch to link_address          */
                              /* (see NOTE 1 below)                       */
  void *link_address;         /* this is branch address when link != 0    */
                              /* (see NOTE 1 below)                       */
  uint phys_id;               /* unique processor identifier              */
                              /* from system register                     */
  int architecture;           /* POWER_RS   processor architecture        */
                              /* POWER_PC   etc                           */
                              /* see AIX systemcfg.h for future values    */
  int implementation;         /* POWER_RS1  processor implementation      */
                              /* POWER_RS2                                */
                              /* POWER_RSC                                */
                              /* POWER_601  etc                           */
                              /* see AIX systemcfg.h for future values    */
  int version;                /* processor version number                 */
                              /* PPC_601    etc                           */
  int width;                  /* max processor data word size (32 or 64)  */
                              /* NOT the current execution mode           */
  int cache_attrib;           /* cache attribute bit field                */
                              /* bit: state definition: (bit 0 = LSB)     */
                              /*  #       0/1                             */
                              /*  0   cache-not-present/cache-present     */
                              /*  1   separate-cache/combined-inst-data   */
                              /* undefined bits are reserved              */
  int coherency_size;         /* size of coherence block                  */
  int resv_size;              /* size of reservation granule              */
  int icache_block;           /* L1 instruction cache block size          */
  int dcache_block;           /* L1 data cache block size                 */
  int icache_size;            /* L1 instruction cache size                */
  int dcache_size;            /* L1 data cache size                       */
  int icache_line;            /* L1 instruction cache line size           */
  int dcache_line;            /* L1 data cache line size                  */
  int icache_asc;             /* L1 instruction cache associativity       */
  int dcache_asc;             /* L1 data cache associativity              */
  int L2_cache_size;          /* L2 cache size; see NOTE 6 below          */
  int L2_cache_asc;           /* L2 cache associativity                   */
  int tlb_attrib;             /* translation look-asside buffer attribute */
                              /* bit: state definition: (bit 0 = LSB)     */
                              /*  #       0/1                             */
                              /*  0   tlb-not-present/tlb-present         */
                              /*  1   separate-tlb/combined-inst-data-tlb */
                              /* undefined bits are reserved              */
  int itlb_size;              /* entries in instruction tlb               */
  int dtlb_size;              /* entries in data tlb                      */
  int itlb_asc;               /* instruction tlb associativity            */
  int dtlb_asc;               /* data tlb associativity                   */
  int slb_attrib;             /* segment tbl look-asside buffer attribute */
                              /* bit: state definition: (bit 0 = LSB)     */
                              /*  #       0/1                             */
                              /*  0   slb-not-present/slb-present         */
                              /*  1   separate-slb/combined-inst-data-slb */
                              /* undefined bits are reserved              */
  int islb_size;              /* entries in instruction slb               */
  int dslb_size;              /* entries in data slb                      */
  int islb_asc;               /* instruction slb associativity            */
  int dslb_asc;               /* data slb associativity                   */
  int priv_lck_cnt;           /* supervisor state spin lock count         */
  int prob_lck_cnt;           /* problem    state spin lock count         */
                              /* refer to notes 4,7,8 for timebase info   */
  int rtc_type;               /* processor's time base type               */
  int rtcXint;                /* nano-seconds per time base tick          */
                              /* integer  multiplier (see NOTE 4 below)   */
  int rtcXfrac;               /* nano-seconds per time base tick          */
                              /* fraction multiplier (see NOTE 4 below)   */
  int busCfreq_HZ;            /* bus clock frequency in HERTZ             */
  int tbCfreq_HZ;             /* effective time base clock freq ( NOTE 7) */
  char proc_descriptor[16];   /* NULL terminated ASCII string for ODM DB  */
                              /* EX:     'P','O','W','E','R','\0'         */
                              /* "POWER" for POWER_RS1 & Power_RSC procs  */
                              /* "POWER2" for POWER_RS2 processors        */
                              /* "PowerPC_601" for PowerPC 601 processors */
                              /* "PowerPC_603" for PowerPC 603 processors */
                              /* "PowerPC_604" for PowerPC 604 processors */
                              /* "PowerPC_620" for PowerPC 620 processors */
} PROCESSOR_DATA, *PROCESSOR_DATA_PTR;

/****************************************************************************
   THE FOLLOWING NOTES APPLY TO THE processor_info STRUCTURE DEFINED ABOVE

Notes on processor data:
1. Each processor_info structure is a template for a processor that may
   exist in a given "slot" (location).  Any processor "slots" that are
   empty will contain a value of 0 in their processor_present
   variable.

   Any processors that are present will have their state defined in the
   processor_present variable.

   When IPL ROS transfers control to AIX, one and only one processor
   will be running the AIX code and its processor_present variable will
   contain a value of "1".  This processor will be referred to as the
   "master" processor. When AIX gets control, R3 will contain the
   IPL_CB_PTR and R4 will contain the address of the "processor_info"
   structure of the running processor.

   Any remaining processors in an MP environment may be running a
   protected, memory resident program provided by IPL ROS (a value of
   "2" in the processor_present variable), or will be "available" but
   in the reset state (a value of "3" in the processor_present
   variable).  In the former case, the code continually interrogates
   the value of the "link" variable within its own processor_info
   structure as follows: If this value is 0, the processor continues to
   interrogate it; if the value is non-zero, the link_address variable
   is interpreted as a branch address and a branch to the address is
   affected by IPL ROS for the coresponding processor. When the branch
   is affected, R3 will contain the IPL_CB_PTR and R4 will contain the
   address of the "processor_info" structure of the processor.  The
   link and link_address variables of the master processor will be set,
   by IPL ROS, to indicate the starting address of the loaded code.

2. num_of_structs is a value that is dynamically determined by IPL
   ROS. Its value will be placed in each processor_info structure
   contained within the array. This value will specify the number of
   processors that is supported (but not neccessarily present) in the
   processor complex that is controlled by the IPL_ROS.

3. To guarantee compatibility between different levels of ROS and AIX,
   one must traverse through the array structure using pointers and
   offset values:

    To get to the beginning of the processor_info array,
    define your pointer variable and assign to it as follows:
    PROCESSOR_DATA_PTR per_proc_info_ptr =
         (struct processor_info *)
           ((uint)IPL_CB_PTR + IPL_CB_PTR->s0.processor_info_offset);

    per_proc_info_ptr may now be used for accessing and traversing
    the array.

    To find the size of the array, read it directly from the
    ipl_directory.

4. rtcXint and rtcXfrac allow for conversion between time base counter
   ticks and time-of-day.

   SUB-NOTE:
   Their values are dependent on the processors time base type,
   rtc_type. For rtc_type "1" (i.e. 601 based machines) these variables
   are set to 0 since the time base counter is defined such that its
   least significant bit is to change once per nanosecond. For other
   rtc_type values, rtcXint will be the integral number of nanoseconds
   per time base tick and rtcXfrac will be the fraction part, expressed
   to the nearest thousandth. The net result is the timebase period in
   picoseconds.

   EX: assume that the time base counter is driven by a 66MHZ clock.
   Therefor, the timebase "tick" period is 15152  rounded to the
   nearest picoseconds, from which rtcXint = 15 and rtcXfrac = 152.

   REFER TO NOTEs 7 & 8 for additional details.

5. This array is pointed to by IPL directory entry
   "processor_info_offset"

6. L2_cache_size indicates the configured or "in use" size (in bytes) of
   the L2 cache. Additional information on the L2 cache is contained in
   the l2_data structure. L2_cache_size will be set to 0 if the L2 cache
   could not be configured.

7. tbCfreq_HZ is the effective time base clock frequency rounded to the
   nearest HERTZ. If this value is set to -1, the time base counter is
   driven by a variable speed clock (as allowed by the PowerPC
   architecture).  For this case, the time base count rate must be
   determined completely by software (refer to Chapter 4 of the PowerPC
   Architecture Book II).

   REFER TO NOTEs 4 & 8 for additional details.

8. The effective time base frequency is defined to be the frequency
   that the least significant bit of the timebase counter is driven,
   assuming that the least significant bit is implemented. If some of
   the low order bits are not implemented, this does not change the
   effective time base frequency. However, it does change the minimum
   perceptible time interval that can be detected from the timebase
   counter. For example:

   assume the effective time base frequency is 500MHZ, which represents
   an effective timebase tick every 2 nanoseconds. If the two low order
   bits of the timebase reg are not implemented, the minimum detectible
   timebase increment would be 8 nanoseconds given that the effective
   frequency is 500MHZ. For this example we would have rtcXint = 2,
   rtcXfrac = 0, tbCfreq_HZ = 500,000,000.

   REFER TO NOTEs 4 & 7 for additional details.

   THE ABOVE NOTES APPLY TO THE processor_info STRUCTURE DEFINED ABOVE
****************************************************************************/

typedef struct fm2_io_info {
                              /* family 2 adapter support i.e. devices     */
                              /* that support Programmable Option Select   */
                              /* feature (POS) registers                   */
  uint  num_of_structs;       /* contains the run time value corresponding */
                              /* to the number of io adapters defined in   */
                              /* this array                                */
  uint  index;                /* 0 <= index <= num_of_structs - 1,         */
                              /* assume index = n, then this               */
                              /* is the n+1th array element                */
  uint struct_size;           /* size of this structure (in bytes)         */
  ROS_TEST_TYPE test_mode;    /* the post test mode                        */
  int detected_error;         /* 0 implies no error detected               */
                              /* != 0 implies device specific error        */
  int adapter_present;        /* 0 implies not present, != 0 is present    */
  int adapter_bad;            /* only valid if adapter_present != 0,       */
                              /* 0 implies good, != 0 implies bad          */
  uint pos0_id;               /* 0xff IN BOTH pos0_id and pos1_id fields   */
  uint pos1_id;               /* indicate that there is no adapter present */
                              /* in the corresponding buid/slot. any other */
                              /* values are the values obtained from the   */
                              /* pos registers of the buid/slot            */
  uint buid_value;            /* buid to which this data applie            */
  uint slot_value;            /* the slot number within the given buid     */
  uint scan_code_present;     /* romscan post controls this flag           */
                              /* == 0, no test performed for romscan code  */
                              /* == 1, implies romscan code device boot    */
                              /*       code detected on this card          */
                              /* == 2, implies video romscan code detected */
                              /*       on this card.                       */
                              /* == 3, implies both video and device boot  */
                              /*       code are present on this card       */
                              /* == 4->7, Reserved                         */
                              /* == 8 & slot <= 7, no romscan code found   */
                              /* == 8 & slot >  7, no romscan code allowed */
                              /* == 9, A PS/2 ROM failed Checksum          */
                              /* ==10, RISC/6000 ROM CRC failure           */
                              /* ==11, Address of RISC/6000 ROM Invalid    */
  uint supports_ipl;          /* == 0 => no, != 0 => adapter supports ipl  */
#define HW_ADDR_BYTE_LEN 16    /* Usually six bytes, allow for expansion.   */
                              /* The actual length is dictated by the      */
                              /* adapter type. i.e., token, ethernet, etc. */
  uchar hw_net_addr[HW_ADDR_BYTE_LEN]; /* The network address array,       */
                                       /* beginning in byte 0              */
  void *hmb_ptr;
  uint user_defined3;
  uint adapter_flag;          /* to identify bluebonnet or lace adapter    */
  uint user_defined1;
  uint user_defined0;

  uint per_buc_info_offset;   /* the iplcb offset to the "buc_info"       */
                              /* structure related to this device         */
} FM2_IO_DATA, *FM2_IO_DATA_PTR;

enum card_state
{
     IS_EMPTY,
     IS_GOOD,
     IS_BAD
};

enum table_validity
{
        INVALID_TABLE,
        VALID_TABLE
};

enum entry_indicator
{
        CARD,
        SIMM
};

typedef struct mem_data_str
{
  uint num_of_structs;        /* contains the run time value corresponding */
                              /* to the number of structures defined in    */
                              /* in this array                             */
  uint struct_size;           /* size of this structure (in bytes)         */
  enum card_state state;
  uint num_of_bad_simms;
  enum entry_indicator card_or_simm_indicator;
  uint card_or_SIMM_size;
  uint EC_level;
  uint PD_bits;
  char location[5][4];

} MEM_DATA, *MEM_DATA_PTR;

typedef struct l2_data {

  ROS_TEST_TYPE test_mode;              /* POST test mode                    */
  int  detected_error;                  /* 0 = no error; !0 = Error(s) found */
  int  adapter_present;                 /* 1 = present; 0 = absent           */
  int  adapter_bad;                     /* 0 = OK; !0 = Bad                  */
  int  mode;                            /* 0 = I/D cache; 1 = I-cache        */
  int  installed_size;                  /* Actual total size of L2 in Kbytes */
  int  configured_size;                 /* size (KB) configured by IPL ROS   */
  char type[16];                        /* See note (1)                      */
  int  size[16];                        /* See note (2)                      */
  char location[16][4];
  uint  num_of_structs;          /* contains the run time value corresponding*/
                                 /* to the number of elements in this array. */
                                 /* refer to notes (3) and (4) below.        */
  uint  index;                   /* 0 <= index <= num_of_structs - 1         */
                                 /* assume index = n, then this              */
                                 /* is the n+1th array element.              */
  uint struct_size;              /* size of this structure (in bytes)        */
  int shared_L2_cache;           /* shared by more than one using resource   */
                                 /* 0 => not shared, 1 => shared.            */
  int using_resource_offset;     /* iplcb offset to the structure of the     */
                                 /* environment resource that uses this      */
                                 /* cache (usually a processor structure).   */
                                 /* refer to NOTE (4) below                  */

/* NOTES: (1) type[i] can have one of these values:                          */
/*            - 'P' means the bad L2 cache is part of the CPU planar.        */
/*            - 'R' means the bad L2 cache is a riser card.                  */
/*            - 'L' means L2 cache consists of SIMMs and the SIMM at         */
/*               physical location location[i][] is bad.                     */
/*            - '0' means this and all subsequent entries are invalid.       */
/*                                                                           */
/*        (2) size[i] gives the size (KB) of the FRU at physical location    */
/*            location[i][].                                                 */
/*        (3) this struct, "l2_data", is an IPL ROS array. refer to the      */
/*            notes after the struct processor_info for general information  */ 
/*            about ROS arrays.                                              */
/*        (4) this array exists if and only if IPL_DIRECTORY.l2_data_offset  */
/*            != NULL, in which case L2_DATA.num_of_structs will indicate    */
/*            the number of L2_DATA elements in the array. if a given L2     */
/*            cache is shared by more than one resource, it's array element  */
/*            will be replicated for each resource. e.g., if two processors  */
/*            use a given L2 cache, there will be two elements in this array */
/*            describing the same physical L2 cache but pointing to the two  */
/*            different using resources (it is done this way because IPL ROS */
/*            is not allowed to change a given array element size            */
/*            dynamically). lastly, in the event of disjoint I and D caches, */
/*            two separate elements may be pointing to the same using        */
/*            resource. recapping, the L2 cache array is all inclusive in    */
/*            terms of the L2 cache compliment of the environment but may    */
/*            have replicated entries describing the same cache, in which    */
/*            case the "shared_L2_cache" flag will indicate shared L2 cache. */

} L2_DATA, *L2_DATA_PTR;

/* FDDI area.         */
typedef struct fddi_data
{
   ROS_TEST_TYPE test_mode; /* test mode flag                   */
   int adapter_present;     /* 0=adapt. not present, 1= present */
   int adapter_bad;         /* 0=POST success, 1=POST failed    */
   int detected_error;      /* POST error.                      */

   uint post_test_ok;       /* POST test flags.                 */
   uint attach_mode;        /* Attachment mode.                 */
   uint sec_error;          /* secondary return code.           */
   uint dir_state;          /* DIR state                        */
   uint wait_for_int;       /* waiting for interrupt falg.      */
   uchar *scratch;          /* To allocate/dealloc. scratch area*/

} FDDI_DATA, *FDDI_DATA_PTR;

typedef struct flash_status_str
{
   int bad_flash_image;
  /*
   * This value is added to the base address of the Golden code to 
   * get the address of the flash update image. It is retrieved from the
   * golden VPD.
   */
   unsigned int flash_image_start_offset;  
   char gold_vpd_ipl_ros_part_number[8];
   char gold_vpd_ipl_ros_ver_lev_id[14];
   char gold_ipl_ros_copyright[49];
   char gold_ipl_ros_timestamp[10];
   char fi_vpd_ipl_ros_part_number[8];
   char fi_vpd_ipl_ros_ver_lev_id[14];
   char fi_ipl_ros_copyright[49];
   char fi_ipl_ros_timestamp[10];

} FLASH_DATA, *FLASH_DATA_PTR;

typedef struct user_info {
  char *user_id_offset;         /* pointer to a NULL terminated ascii  */
                                /* string identifier. Ex: "XYZ company"*/
                                /* Refer to note 4 for additional info */
  void *user_data_offset;       /* pointer to the user's data          */
  uint user_data_len;           /* byte length of *use_data_offset     */
  struct user_info *next_offset;/* pointer to the next user_info struct*/
} USER_INFO, *USER_INFO_PTR;

/************************************************************************
   THE FOLLOWING NOTES APPLY TO THE user_info STRUCTURE DEFINED ABOVE

1. The ipl_directory structure defines the presence of this structure
   as follows:

     If the "user_struct_offset" field and the "user_struct_size"
     fields are present in the ipl_directory structure (as determined
     by the length of the ipl_directory), then the control block
     supports the user_info structure. However, support does not
     neccessarily imply that there is an actual structure present. It
     is present if and only if user_struct_offset != NULL and the
     user_struct_size != 0. This is "business as usual" for determining
     which control block structures are supported and present.

     NOTE: user_struct_size should be set to "sizeof(user_info)".

2. All of the "pointers" in this structure are actually offset values
   relative to the beginning of the ipl control block.

3. Memory allocation is the users responsibility. If it is required
   that any allocated memory be protected from being reused by the OS,
   the memory bit map must be updated by the user to achieve this.

4. The user id string should identify the company (user). It can be
   used to identify to the cooresponding company specific code
   (possibly operating system code) that there is user-specific "boot"
   information in this structure. IBM does not guarantee the uniqueness
   of the user_id string. Of course, the string is assumed to be
   present if and only if user_id_offset != NULL. The memory space for
   this string must be included in the area define by user_data_offset
   and user_data_len.

5. Each user must provide their own data access via "user_data_offset".
************************************************************************/
#endif

#ifdef MSG

typedef struct ipl_cb {                    /* IPL Control Block front end   */
  unsigned int  gpr_save_area[32];         /* Reg save area, ROS interrupts */
  IPL_DIRECTORY                s0;         /* Offsets/sizes directory       */
} IPL_CB, *IPL_CB_PTR;

#endif
#endif
