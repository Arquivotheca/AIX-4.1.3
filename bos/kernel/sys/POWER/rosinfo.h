/* @(#)50	1.7.1.3  src/bos/kernel/sys/POWER/rosinfo.h, rosipl, bos41J, 9514A_all 3/28/95 22:43:44 */
#ifndef _H_ROSINFO
#define _H_ROSINFO

/*
 * COMPONENT_NAME: (ROSIPL) ros code header file rosinfo.h
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1995
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifdef _POWER
typedef struct ros_entry_table {              /* invalid address = -1       */
  unsigned int *warm_ipl;                     /* software reipl           01*/
  unsigned int *reserved_2;                   /* reserved                 02*/
  unsigned int *detmedia;                     /* NIO diskette restore     03*/
  unsigned int *detdata;                      /* NIO diskette read        04*/
  unsigned int *detdrive;                     /* SCSI disk restore        05*/
  unsigned int *scsidata;                     /* SCSI disk read           06*/
  unsigned int *sla_restore;                  /* SLA restore              07*/
  unsigned int *sla_read;                     /* SLA read                 08*/
  unsigned int *crc;                          /* Cylic Redundency Check   09*/
  unsigned int *manfacturing_led_syncaddr;    /* fixed address for mfg use10*/
  unsigned int *AT_BIOS_compressed;           /* PC AT BIOS code,         11*/
  unsigned int *reserved_05;                  /* reserved                 12*/
  unsigned int *reserved_04;                  /* reserved                 13*/
  unsigned int *reserved_03;                  /* reserved                 14*/
  unsigned int *reserved_02;                  /* reserved                 15*/
  unsigned int *reserved_01;                  /* reserved                 16*/
  unsigned int *reserved_00;                  /* reserved                 17*/

  /* the offset to the start of compressed ros in the actual compressed     */
  /* ROS image. this will be the starting location to be uncompressed and   */
  /* loaded into RAM space                                                  */
  unsigned int *start_of_cros;

  /* the offset to the end   of compressed ros in the actual compressed     */
  /* ROS image. this location does not get uncompressed                     */
  unsigned int *end_of_cros;

  /* the offset to the start of the uncompressed ros code                   */
  unsigned int *start_of_ucros;

  /* the offset to the end   of the uncompressed ros code                   */
  unsigned int *end_of_ucros;

  /* the offset to the start of the ros code                                */
  unsigned int *start_of_ros;

  /* the offset to the end   of the ros code                                */
  unsigned int *end_of_ros;

  /* the offset to the start of the token ring ucode1                       */
  unsigned int *tok_start1;

  /* the offset to the end   of the token ring ucode1                       */
  unsigned int *tok_end1;

  /* the offset to the start of the token ring ucode2                       */
  unsigned int *tok_start2;

  /* the offset to the end   of the token ring ucode2                       */
  unsigned int *tok_end2;

  /* the offset to the start of the lega ucode                              */
  unsigned int *lega_start;

  /* the offset to the end   of the lega ucode                              */
  unsigned int *lega_end;

  /* the offset to the start of the scsi script                             */
  unsigned int *scsi_start;

  /* the offset to the end   of the scsi script                             */
  unsigned int *scsi_end;

  /* the offset to the start of the font bitmap                             */
  unsigned int *font_start;

  /* the offset to the end   of the font bitmap                             */
  unsigned int *font_end;

  /* the offset to the start of the format control ascii                    */
  unsigned int *fc_start;

  /* the offset to the end   of the format control ascii                    */
  unsigned int *fc_end;

  /* the offset to the start of the text ascii                              */
  unsigned int *UStext_start;

  /* the offset to the end   of the text ascii                              */
  unsigned int *UStext_end;

  /* the offset to the start of the text ascii                              */
  unsigned int *FRtext_start;

  /* the offset to the end   of the text ascii                              */
  unsigned int *FRtext_end;

  /* the offset to the start of the text ascii                              */
  unsigned int *GRtext_start;

  /* the offset to the end   of the text ascii                              */
  unsigned int *GRtext_end;

  /* the offset to the start of the text ascii                              */
  unsigned int *SVtext_start;

  /* the offset to the end   of the text ascii                              */
  unsigned int *SVtext_end;

  /* the offset to the start of the text ascii                              */
  unsigned int *ITtext_start;

  /* the offset to the end   of the text ascii                              */
  unsigned int *ITtext_end;

  /* the offset to the start of the text ascii                              */
  unsigned int *BEtext_start;

  /* the offset to the end   of the text ascii                              */
  unsigned int *BEtext_end;

  /* the offset to the start of the text ascii                              */
  unsigned int *NOtext_start;

  /* the offset to the end   of the text ascii                              */
  unsigned int *NOtext_end;

  /* the offset to the start of the text ascii                              */
  unsigned int *SPtext_start;

  /* the offset to the end   of the text ascii                              */
  unsigned int *SPtext_end;

  /* the offset to the start of the "buffer zone"                           */
  unsigned int *buff_start;

  /* the offset to the end   of the "buffer zone"                           */
  unsigned int *buff_end;

  /* the offset to the start of the compressed AT BIOS code                 */
  unsigned int *bios_start;

  /* the offset to the end   of the compressed AT BIOS code                 */
  unsigned int *bios_end;

  /* the length of the first compressed ros field                           */
  unsigned int *c1_length;

  /* the offset to the start of the font bitmap                             */
  unsigned int *font_start640;

  /* the offset to the end   of the font bitmap                             */
  unsigned int *font_end640;

 } ROS_ENTRY_TABLE, *ROS_ENTRY_TABLE_PTR;

typedef struct simm_definition {
  unsigned char SIMM_7and8;       /* One SIMM is a nibble wide (four bits)   */
  unsigned char SIMM_3and4;       /* and will have the value of 0x0(good) or */
  unsigned char SIMM_5and6;       /* 0xf(bad).                               */
  unsigned char SIMM_1and2;
} SIMM_DEFINITION;

typedef struct ipl_info {

/* The following two entries are reserved for IPL ROM usage.
 * Pointer to the IPL Controller and Device Interface Routine in Memory. The
 * size is Memory allocated for the IPL Controller and Device Interface Routine
 * NOTE: validity of this pointer is not guaranteed to be correct after
 *       IPL ROM has given control to the loaded code.
 */
  unsigned int     *iplc_and_dir_ptr;
  unsigned int      iplc_and_dir_size;

/* The following two entries are reserved for IPL ROM usage.
 * Pointer to the NVRAM expansion code in Memory.
 * The size is Memory allocated for the NVRAM expansion code.
 * NOTE: validity of this pointer is not guaranteed to be correct after
 *       IPL ROM has given control to the loaded code.
 */
  unsigned int     *ram_exp_code_ptr;
  unsigned int      ram_exp_code_size;

/* The following two entries are reserved for IPL ROM usage.
 * Initial pointer to the IPL ROM Stack's high address in Memory.
 * The size is Memory allocated for the IPL ROM Stack.
 * NOTE: validity of this pointer is not guaranteed to be correct after
 *       IPL ROM has given control to the loaded code.
 */
  unsigned int     *ipl_ros_stack_ptr;
  unsigned int      ipl_ros_stack_size;

/* The following two entries are reserved for IPL ROM usage.
 * Pointer to the IPL Record in Memory.
 * The size is Memory allocated for the IPL Record area.
 * NOTE: validity of this pointer is not guaranteed to be correct after
 *       IPL ROM has given control to the loaded code.
 */
  unsigned int     *ipl_rec_ptr;
  unsigned int      ipl_rec_size;

/* The following entry is reserved for IPL ROM usage.
 * Pointer to the lowest address needed by IPL ROM in Memory.
 * NOTE: validity of this pointer is not guaranteed to be correct after
 *       IPL ROM has given control to the loaded code.
 */
  unsigned int     *ros_workarea_low_boundry;

/* The following two entries are reserved for IPL ROM usage.
 * Pointer to the IPL ROM entry table.
 * The size in ROM for the entry table.
 * NOTE: In order to address IPL ROM, translate mode must be off and
 *       Segment Register 15 set.
 */
  unsigned int     *ros_entry_table_ptr;
  unsigned int     ros_entry_table_size;

/* The following entry is reserved for IPL ROM usage.
 * The memory bit map's number of bytes per bit.
 * NOTE: the default size is 16K/bit
 */
  unsigned int     bit_map_bytes_per_bit;

/* The following entry is reserved for IPL ROM usage.
 * The highest addressable real memory address byte+1
 * Note: All real memory addresses below this value are in range, addressing
 *       memory at this address or above will cause an address out of range
 *       exception. The appropriate Segment Register T-bit must be set to zero
 *       in order to address memory.
 */
  unsigned int      ram_size;

/*
 *  The model field contains information which allows software to determine
 *  hardware type, data cache size, and instruction cache size.
 *
 *  The model field is decoded as follows:
 *        0xWWXXYYZZ
 *
 *  case 1: WW = 0x00 This means that the hardware is SGR ss32 or SGR ss64
 *                    (ss is speed in MH). The instruction cache is 8K bytes.
 *          XX = reserved
 *          YY = reserved
 *          ZZ = the model code:
 *                  bits 0 & 1 (low order bits) - indicate style type
 *                        00 = Tower     01 = Desktop
 *                        10 = Rack      11 = Reserved
 *                  bits 2 & 3 - indicate relative speed of processor
 *                        00 = Low       01 = Medium
 *                        10 = High      11 = Very high
 *                  bit 4 - Indicates number of combo chips.
 *                        0 = 2 combo chips
 *                        1 = 1 combo chip
 *                  bit 5 - Indicates the number of DCU's.
 *                        0 = 4 DCU's, data cache is 64 K bytes
 *                        1 = 2 DCU's, data cache is 32 K bytes
 *                  bits 6 & 7 (high order bits) - Reserved.
 *
 *  case 2: WW is nonzero.
 *          WW = 0x01 This means that the hardware is SGR ss32 or SGR ss64
 *                    (ss is speed in MH) -  RS1 
 *          WW = 0x02 means the hardware is RSC.
 *          WW = 0x04 means the hardware is RS2 ( POWER2).
 *          WW = 0x08 means the hardware is PowerPC. 
 *          XX has the following bit definitions:
 *                  bits 0 & 1 (low order bits) - indicate package type
 *                        00 = Tower     01 = Desktop
 *                        10 = Rack      11 = Entry Server Type 
 *		    bit  2 - AIX Hardware Verification Test  Supported (rspc)
 *                  bit  3 - AIX Hardware Error Log Analysis Supported (rspc)
 *                  bits 4 through 7 are reserved.
 *          YY = reserved.
 *          ZZ = the model code.  No further information can be obtained
 *                  from this byte.
 *
 *          The instruction cache K byte size is obtained from entry icache.
 *          The data cache K byte size is obtained from entry dcache.
 *
 */
   unsigned int      model;

/* The following entry is reserved for IPL ROM usage.
 * Power Status and keylock Register Decode. IO Address 0x04000E4
 * This is a 32 bit register:
 *   Power Status     bits   0 to  9.
 *   reserved         bits  10 to 27.
 *   keylock decode   bits  28 to 31 are as follows:
 *                            28 29 30 31
 *                             1  1  X  X   Workstation
 *                             0  1  X  X   Supermini
 *                             0  0  X  X   Expansion
 *                             X  X  1  1   Normal keylock position
 *                             X  X  1  0   Service keylock position
 *                             X  X  0  1   Secure keylock position
 *                             X is don't care
 */
   unsigned int      Power_Status_and_keylock_reg;

/*
 * This value is initially set to zero during power-on IPL and is incremented
 * by one with each invocation of ROM warm IPL.
 */
  int               soft_ipl_flag;

/* The following entries are reserved for IPL ROM usage.
 * The following entries are set and used by the IPL Controller.
 */
  int               nvram_section_1_valid;    /* 0 if CRC miscompare        */
  int               nvram_exp_code_valid;     /* 0 if CRC miscompare        */
  unsigned char     previpl_device[36];       /* last normal mode ipl device*/
  char              reserved[28];             /* reserved for last ipl needs*/

/* The following entries are reserved for IPL ROM usage.
 * Pointer to the IPL Control Block in Memory.
 * During warm IPL if it is necessary this pointer will be used to reestablish
 * the original location of the IPL Control Block in memory to avoid any
 * collision with the code that is about to be loaded by the IPL process.
 */
  unsigned int     *iplcb_ptr;                /* IPL Control Block pointer  */

/* The following entries are reserved for IPL ROM usage.
 * Pointer to compressed BIOS code in the IPL ROM.
 * The size of BIOS code in bytes in the IPL ROM.
 * NOTE: In order to address IPL ROM, translate mode must be off and
 *       Segment Register 15 set.
 */
  unsigned char    *BIOS_code_ptr;
  unsigned int     BIOS_code_size;

/* The following entriy is reserved for IPL ROM usage.
 * Pointer to the RAM copy of the Core Sequence Controller.
 * Saved by IPLCBinit after moving the ROM to RAM.
 */
  unsigned int     *CSC_in_RAM_ptr;

/*
 * Storage Configuration Registers.
 * This is the value the IPL ROM assigned during the power up phase of IPL.
 * There are 16 elements, one for each of the extents.
 * NOTE: see the IPL Control Block entry extent_errinfo for any error
 *       information that was saved when the cre was set.
 */
  unsigned int      cre[16];

/*
 * Bit Steering Registers.
 * This is the value the IPL ROM assigned during the power up phase of IPL.
 * There are 16 elements, one for each of the extents.
 * NOTE: see the IPL Control Block entry memcd_errinfo for any SIMM error
 *       information that was saved when the bscr was set.
 */
  unsigned int      bscr[16];

/* The following entries are reserved for IPL ROM usage.
 * NOTE: These fields are not currently implemented and there is no future
 *       plans to implement them.
 */
  struct {
    unsigned int      syndrome;
    unsigned int      address;
    unsigned int      status;
  }                 single_bit_error[16];
  unsigned int      reserved_array[5*16];

/* Memory extent test indicator.
 * Marked tested after each memory extent is tested during the ROM IPL.
 * There are 16 elements, one for each of the extents.
 */
  unsigned char     extent_tested_ind[16];    /* 0 = untested, 1 = tested   */

/* Memory bit steering register setting conflict indicator.
 * Marked after each memory extent is tested during the ROM IPL.
 * There are 16 elements, one for each of the extents.
 * Note: the bscr associated with the tested memory extent is compared to the
 *       bscr data in the Non Volatile RAM(NVRAM). If there is a miscompare
 *       then the conflict indicator is set.
 */
  unsigned char     bit_steer_conflict[16];   /* 1 = Bit Steering conflict  */

/* The following entries are reserved for IPL ROM usage.
 * The following entries are set by the IPL Controller.
 */
  unsigned int      ipl_dev_ledval;           /* led value at ipl time      */
  unsigned int      ipl_dev_driveval;         /*ipl device drive# (non scsi)*/

  char              unused[18];               /*                            */

/*
 * The following entries are copied from the IPL ROM Vital Product Data area.
 * These values are used to determine the machine hardware level.
 */
  char              vpd_planar_part_number[8];
  char              vpd_planar_ec_number[8];
  char              vpd_processor_serial_number[8];
  char              vpd_ipl_ros_part_number[8];
  char              vpd_ipl_ros_ver_lev_id[14];
  char              ipl_ros_copyright[49];
  char              ipl_ros_timestamp[10];

/*
 * The following entries are copied from data in the Non Volatile RAM(NVRAM).
 * These values are used to determine the machine hardware level.
 */
union {
  unsigned int chip_signature;
  struct CHIP_SIGNATURE {
    unsigned char cop_bus_address;
    unsigned char obsolete_u_num;
    unsigned char dd_number;
    unsigned char part_number;
  } CHIP_SIGNATURE;
} floating_point, fixed_point, instruction_cache_unit, storage_control_unit,
  combo_1, combo_2, data_control_unit_0, data_control_unit_1,
  data_control_unit_2, data_control_unit_3;

/*
 * Memory card SIMM error information. SIMM is a replaceable memory card part.
 * There are 8 possible cards(slot A to H) with 8 SIMMs(1 to 8) per card.
 * There are two cache line sizes(see the IPL Control Block entry
 * cache_line_size). If the size is 128 then use the memory_card_1n data to
 * find which SIMM(s) are bad. Otherwise use the memory_card_9n or
 * memory_card_Tn data. See the IPL Control Block entry IO_planar_level_reg
 * to determine if there is a table top system present.
 * The n after the 1 or 9 or T represents the slot number(A thru H).
 */
union {
  char            memcd_errinfo[32];     /* holds err info for memory cards  */
  unsigned int    slots_of_SIMMs[8];  /* When 0x0 then no SIMM was marked bad*/
  struct {
    struct simm_definition               /* Cache line size of 128           */
      memory_card_1H, memory_card_1F, memory_card_1G, memory_card_1E,
      memory_card_1D, memory_card_1B, memory_card_1C, memory_card_1A;
  } STUFF_1;
  struct {
    struct simm_definition               /* Cache line size of 64            */
      memory_card_9H, memory_card_9D, memory_card_9F, memory_card_9B,
      memory_card_9G, memory_card_9C, memory_card_9E, memory_card_9A;
  } STUFF_9;
  struct {
    struct simm_definition               /* Cache line size of 64            */
      memory_card_TB, memory_card_TC;
  } STUFF_T;
} SIMM_INFO;

/*
 * MESR error information at IPL ROM memory configuration time.
 * There are two cache line sizes(see the IPL Control Block entry
 * cache_line_size). If the size is 128 then use the ext_n_slot_XandY data to
 * find the extent MESR. Otherwise use the ext_n_slot_X data.
 * The _n_ is the extent number while _X andY are the slots associated with the
 * extent.
 * NOTE: find one of the following values in the word variable,
 *       0x00000000 no MESR error occurred when configuring this extent.
 *       0xe0400000 a timeout error occurred while trying to configuring this
 *                  extent. No memory card is in the slot for this extent.
 *       OTHERWISE an error occurred which means the memory card is present but
 *       not being used.
 */
union {
  char            extent_errinfo[64];    /* holds mesr contents (error info) */
  struct {                        /* Cache line size of 128                  */
    unsigned int
      ext_0_slot_HandD, ext_1_slot_HandD, ext_2_slot_HandD, ext_3_slot_HandD,
      ext_4_slot_FandB, ext_5_slot_FandB, ext_6_slot_FandB, ext_7_slot_FandB,
      ext_8_slot_GandC, ext_9_slot_GandC, ext_10_slot_GandC,ext_11_slot_GandC,
      ext_12_slot_EandA,ext_13_slot_EandA,ext_14_slot_EandA,ext_15_slot_EandA;
  } MESR_err_1;
  struct {                        /* Cache line size of 64                   */
    unsigned int
      ext_0_slot_H,  ext_1_slot_H,  ext_2_slot_D,  ext_3_slot_D,
      ext_4_slot_F,  ext_5_slot_F,  ext_6_slot_B,  ext_7_slot_B,
      ext_8_slot_G,  ext_9_slot_G,  ext_10_slot_C, ext_11_slot_C,
      ext_12_slot_E, ext_13_slot_E, ext_14_slot_A, ext_15_slot_A;
  } MESR_err_9;
} CONFIG_ERR_INFO;

  char            unused_errinfo[64];    /* reserved for future use          */

/*
 * Memory card VPD data area
 * There are two cache line sizes(see the IPL Control Block entry
 * cache_line_size). If the size is 128 then use the ext_n_slot_XandY data to
 * find the extent VPD. Otherwise use the ext_n_slot_X data.
 * The _n_ is the extent number while _X andY are the slots associated with the
 * extent.
 * NOTE: find one of the following values in the word variable,
 *       0xFFFFFFFF memory card is present but did not respond with VPD info.
 *       0x22222222 memory card is present but gave error responses.
 *       0x11111111 No memory card is in the slot for this extent.
 *       OTHERWISE  this must be good VPD.
 */
union {
  char            memcd_vpd[128];
  struct {                        /* Cache line size of 128                  */
    char
      ext_0_slot_HandD[20],
      ext_4_slot_FandB[20],
      ext_8_slot_GandC[20],
      ext_12_slot_EandA[20] ;
  } memory_VPD_1;
  struct {                        /* Cache line size of 64                   */
    char
      ext_0_slot_H[10],  dmy_0[2],
      ext_2_slot_D[10],  dmy_2[2],
      ext_4_slot_F[10],  dmy_4[2],
      ext_6_slot_B[10],  dmy_6[2],
      ext_8_slot_G[10],  dmy_8[2],
      ext_10_slot_C[10], dmy_10[2],
      ext_12_slot_E[10], dmy_12[2],
      ext_14_slot_A[10], dmy_14[2];
  } memory_VPD_9;
} MEMCD_VPD;

/* Cache Line size
 * Double card cache line size is 128 bytes, while single card cache line size
 * is 64 bytes.
 */
int cache_line_size;

/*
 * Component Reset Register test results. For BUID 20
 * There are four data tests 0x00, 0xAA, 0x55 and 0xFF, when all tests are
 * successful then the value found here will be 0x00AA55FF. Any other value
 * indicates stuck bits in the component reset register(stuck on or stuck off).
 */
int CRR_results;


/* IO Planar Level Register
 * Use this value to determine if this is a family 2 or table top system.
 *   0x1YYXXXXX is a family 2.
 *   0x8YYXXXXX is a table/desk top.
 *      YY defines the Engineering Level. X is reserved.
 *
 *   An IO Planar value of -1 indicates NOT present.
 *
 *   The value in the most significant byte has the following meaning:
 *     0x80  table/desk top.
 *     0x40  reserved
 *     0x20  reserved
 *     0x10  Rack Planar
 *     0x08  Standard IO
 *     0x04  Power connector NOT connected
 *     0x02  reserved
 *     0x01  reserved
 *
 */
int IO_planar_level_reg;                      /* base IO planar on BUID 20 */
int IO_planar_level_reg_21;                   /*      IO planar on BUID 21 */
int IO_planar_level_reg_22;                   /*      IO planar on BUID 22 */
int IO_planar_level_reg_23;                   /*      IO planar on BUID 23 */

/*
 * Component Reset Register test results. For (OPTIONAL feature) BUID 21
 * There are four data tests 0x00, 0xAA, 0x55 and 0xFF, when all tests are
 * successful then the value found here will be 0x00AA55FF. Any other value
 * indicates stuck bits in the component reset register(stuck on or stuck off).
 * If the value is 0xffffffff then most likely the IO planar is not connected.
 */
int CRR_results_21;

/*
 * Component Reset Register test results. For (future OPTIONAL feature) BUID 22
 * There are four data tests 0x00, 0xAA, 0x55 and 0xFF, when all tests are
 * successful then the value found here will be 0x00AA55FF. Any other value
 * indicates stuck bits in the component reset register(stuck on or stuck off).
 * If the value is 0xffffffff then most likely the IO planar is not connected.
 */
int CRR_results_22;

/*
 * Component Reset Register test results. For (future OPTIONAL feature) BUID 23
 * There are four data tests 0x00, 0xAA, 0x55 and 0xFF, when all tests are
 * successful then the value found here will be 0x00AA55FF. Any other value
 * indicates stuck bits in the component reset register(stuck on or stuck off).
 * If the value is 0xffffffff then most likely the IO planar is not connected.
 */
int CRR_results_23;

/*
 * Component Reset Register test results. For (BASE feature) BUID 20
 * There are four data tests 0x0s, 0xAs, 0x5s and 0xFs, when all tests are
 * successful then the value found here will be <see below> Any other value
 * indicates stuck bits in the component reset register(stuck on or stuck off).
 * If the value is 0xffffffff in CRR_results_20_0, then most likely the
 * IO planar is not connected.
 */
int CRR_results_20_0;  /* should contain 0x00000000 */
int CRR_results_20_a;  /* should contain 0xaaaaaaaa */
int CRR_results_20_5;  /* should contain 0x55555555 */
int CRR_results_20_f;  /* should contain 0xffffffff */

/*
 * Component Reset Register test results. For (OPTIONAL feature) BUID 21
 * There are four data tests 0x0s, 0xAs, 0x5s and 0xFs, when all tests are
 * successful then the value found here will be <see below> Any other value
 * indicates stuck bits in the component reset register(stuck on or stuck off).
 * If the value is 0xffffffff in CRR_results_20_0, then most likely the
 * IO planar is not connected.
 */
int CRR_results_21_0;  /* should contain 0x00000000 */
int CRR_results_21_a;  /* should contain 0xaaaaaaaa */
int CRR_results_21_5;  /* should contain 0x55555555 */
int CRR_results_21_f;  /* should contain 0xffffffff */

/* This space reserved for future expansion for BUID 22, 23 CRR results */
int CRR_reserved[8];
/* int CRR_results_22_0; */
/* int CRR_results_22_a; */
/* int CRR_results_22_5; */
/* int CRR_results_22_f; */
/* int CRR_results_23_0; */
/* int CRR_results_23_a; */
/* int CRR_results_23_5; */
/* int CRR_results_23_f; */

/*
 * IO INTERRUPT test results. For (OPTIONAL feature) BUID 21
 * There are four data tests 0x00, 0xAA, 0x55 and 0xFF, when all tests are
 * successful then the value found here will be zero. Any other value
 * indicates failures during the interrupt test. A device connected to the
 * the adapter in one of the IO slots could be causing the problem, or the
 * IO planar is defective.
 */
int IO_INTERRUPT_results_21;

/* The following entries are reserved for IPL ROM usage.
 * Pointer to a copy of the IPL ROM code in memory.
 * The size of the IPL ROM code in memory.
 */
  unsigned int    *ROMs_RAM_address;
  unsigned int     ROMs_RAM_size;

/* The following entries are reserved for IPL ROM usage.
 * Read the Storage Control Configuration Register.
 *   CONTROL BITS
 *     BIT_00 Diagnostic mode
 *     BIT_01 DMA Double Line
 *     BIT_02 and 03 Invert Address Bit (toggle one meg)
 *                bits 2 and 3 effect:
 *                00 No effect, normal addressing.
 *                01 Address bit 11 is inverted internally.
 *                10 Address bit 10 is inverted internally.
 *                11 Address bit 10 and 11 are both inverted internally.
 *     BIT_04 No holes in memory
 */
  unsigned int    SCCR_toggle_one_meg;


/* The following entry is read from the OCS NVRAM area
 *   See the comments of the model definition entry(AIX_model_code portion)
 */
  unsigned int    AIX_model_code;   /* 4 bytes from NVRAM address 0xA0003d0 */

/* The following entries are read from the OCS NVRAM area
 *
 * ---------: dcache size = 0x0040     icache size = 0x0008
 * ---------: dcache size = 0x0020     icache size = 0x0008
 * ---------: dcache size = 0x0008     icache size = 0x0008
 * ---------: dcache size = 0x0040     icache size = 0x0020
 * ---------: dcache size = 0x0020     icache size = 0x0008
 */
  int             dcache_size;      /* 4 bytes from NVRAM address 0xA0003d4 */
  int             icache_size;      /* 4 bytes from NVRAM address 0xA0003d8 */
  char            vpd_Model_ID[8];

/* The following entry is reserved for IPL ROM usage.
 * used to save the Pointer to the lowest address needed by IPL ROM in Memory.
 * NOTE: this pointer is invalid when the value is -1.
 */
  unsigned int     *low_boundry_save;

/* The following entries is reserved for IPL ROM usage.
 * Used to save the Pointer to the ROM Scan entry point and data area.
 * NOTE: this pointer is invalid when the value is -1.
 */
  int              *ROM_SCAN_code_in_ram; /* runtime entry point of rom scan */
  int              *rom_boot_data_area;   /* runtime user ram (4K or greater)*/

} IPL_INFO ;
#endif
#endif
