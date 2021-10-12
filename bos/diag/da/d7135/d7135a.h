/* @(#)38       1.2  src/bos/diag/da/d7135/d7135a.h, da7135, bos41J, 9511A_all 3/13/95 08:51:38 */
/*
 * COMPONENT_NAME: DA7135
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993,1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

        /* Function Prototypes */
int     raid_tasks(int);
int     raid_array_status(int);
void    raid_add_to_status(int, struct lun *);
void    raid_disk_status(void);
void    raid_add_spares(void);
void    raid_build_msg(void);
void    raid_add_buff(char *);
int     raid_ela(char *);
int     raid_fru(int);
int     raid_check_fru_2D(int);
int     raid_cntrl_switch(void);
int     lun_ioctl(char *);

        /* Defines */
/* Device types used as param to raid_array_status() function. */
#define CNTRL                   0x00
#define SPARE                   0x02
#define HOT_SPARE               0x12
#define LUN                     0x04
/* Mask used when switching on device type. */
#define DEVICE_TYPE_MASK        0x0F
/* Device type status flags. */
#define DEAD_LUN                0x10
#define OPEN                    0x12

#define MAX_7135_LUNS           7  /* 0-6  */
#define MAX_7135_CNTRLS         2  /* dac0-dac1  */
#define MAX_IDs                 16 /* 0-15 */
#define MAX_CHs                 15 /* 1-15 */
#define MAX_SUPPORTED_IDs       7  /* 0-6  */
#define MAX_SUPPORTED_CHs       5  /* 1-5  */


#define MAIN_STATUS_MASK        0x0F /* MODE SENSE 2A and 2B */
#define SUB_STATUS_MASK         0xF0 /*   status masks.      */

        /* Controller status for MODE SENSE Redundant Cntrl (page 2C). */
#define ACTIVE_CNTRL            0x00
#define PASSIVE_CNTRL           0x01
#define HIR_CNTRL           	0x02
#define FAILED_CNTRL            0x04
#define ALT_CNTRL               0x08
#define BUSY_LUN                0xBB /* Used when open returns busy. */

        /* FRU Type defines */
#define ELA_ASC_3F_FRU_TYPE             0
#define ELA_FRUC_FRU_TYPE               1
#define ELA_CNTRL_FRU_TYPE              2
#define FRUC_FRU_TYPE                   3
#define SPECIFIC_FRU_TYPE               4
#define TASK_FRU_TYPE                   5
#define LUN_FRU_TYPE                    6
        /* PDISK FRU Type defines */
#define PDISK_FRU_TYPE                  20
#define ELA_PDISK_PFA_FRU_TYPE          21
#define ELA_PDISK_FRUC_FRU_TYPE         22
#define PDISK_FRUC_FRU_TYPE             23
#define UNKNOWN_PDISK_FRU_TYPE          24

        /* Data file defines */
#define SEQUENCE_DATA           1
#define ASL_SCREEN_DATA         2
#define SCATU_TUCB_DATA         3
#define SC_CDB_DATA             4
#define MISC_TASK_DATA          5
#define ERP_DATA                6
#define SC_PL_DATA              7

        /* Misc defines */
#define END_OF_LIST                     1
#define BEGINNING_OF_LIST               2
#define TASK_RAID_MODE_SENSE_2D         "6D1A"
#define MS_HD_BD                        12
#define DONT_GET_ERP                    0
#define GET_ERP                         1
#define DEDICATED_DAC_LUN               7

        /* MACROS */
#define DAC_ID_ODD(a) (int) ((a + 2) % 2)

#define CNTRL_IS_ACTIVE         \
        (int)(((unsigned)tucb.scsitu.data_buffer[47] == 1) || \
              ((unsigned)tucb.scsitu.data_buffer[47] == 2))

#define CNTRL_IS_PASSIVE        \
        (int)(((unsigned)tucb.scsitu.data_buffer[46] == 1) && \
             ((unsigned)tucb.scsitu.data_buffer[47] == 0))

#define PASSIVE_CNTRL_IS_HEALTHY    \
        (int)((unsigned)tucb.scsitu.data_buffer[47] == 0)

#define NOT_READY_NO_uCODE      \
        (int)(CHECK_CONDITION(da.task_rc) && \
              ((unsigned)tucb.scsitu.data_buffer[7] <= 0x0A))

#define NON_ZERO_FRUC           \
        (int)(CHECK_CONDITION(da.task_rc) && \
              ((unsigned)tucb.scsitu.data_buffer[14] != 0))

#define SENSE_DATA_BYTE_14      \
              ((unsigned)tucb.scsitu.data_buffer[14])

#define DAC_SELECTED    (int)(!strncmp(tm_input.dname, "dac", 3))
#define HDISK_SELECTED  (int)(!strncmp(tm_input.dname, "hdisk", 5))

#define PDISK_STATUS(c,i)	/* This macro will is used to get the disk */  \
                        	/* status byte in the MODE SENSE 2A data.  */  \
                        	/* The channel + (id * max_channel) equals */  \
                        	/* the index into a 1 based data buffer.   */  \
                        	/* The buff starts at byte 2, so add 1.    */  \
                        	/* Also add the Mode Sense Header/BlkDesc. */  \
			(uchar)(ms_2A_buff[c + (i * MAX_CHs) + 1 + MS_HD_BD])

#define VALID_PDISK_CH(a) (int)((a >= 1) && (a <= MAX_SUPPORTED_CHs))
#define VALID_PDISK_ID(a) (int)((a >= 0) && (a <= (MAX_SUPPORTED_IDs - 2)))

/* Subsystem Macros (Mode Sense 2D) */

#define FRU_2D_200 (int)((unsigned)tu_buffer[15] == 0x01)

#define FRU_2D_201 (int)(((unsigned)tu_buffer[26] == 0x10 && \
                          (unsigned)tu_buffer[25] == 0x10 && \
                          (unsigned)tu_buffer[24] == 0x11) || \
                         ((unsigned)tu_buffer[19] == 0x10 && \
                          (unsigned)tu_buffer[18] == 0x10 && \
                          (unsigned)tu_buffer[17] == 0x10 && \
                          (unsigned)tu_buffer[16] == 0x11))

#define FRU_2D_202 (int)(((unsigned)tu_buffer[26] == 0x10  && \
                          (unsigned)tu_buffer[25] == 0x11  && \
                          (unsigned)tu_buffer[24] == 0x10) || \
                         ((unsigned)tu_buffer[19] == 0x10 && \
                          (unsigned)tu_buffer[18] == 0x10 && \
                          (unsigned)tu_buffer[17] == 0x11 && \
                          (unsigned)tu_buffer[16] == 0x10))

#define FRU_2D_203 (int)(((unsigned)tu_buffer[26] == 0x10  && \
                          (unsigned)tu_buffer[25] == 0x11  && \
                          (unsigned)tu_buffer[24] == 0x11) || \
                         ((unsigned)tu_buffer[19] == 0x10 && \
                          (unsigned)tu_buffer[18] == 0x11 && \
                          (unsigned)tu_buffer[17] == 0x10 && \
                          (unsigned)tu_buffer[16] == 0x10))

#define FRU_2D_204 (int)(((unsigned)tu_buffer[26] == 0x11  && \
                          (unsigned)tu_buffer[25] == 0x10  && \
                          (unsigned)tu_buffer[24] == 0x10) || \
                         ((unsigned)tu_buffer[19] == 0x11 && \
                          (unsigned)tu_buffer[18] == 0x10 && \
                          (unsigned)tu_buffer[17] == 0x10 && \
                          (unsigned)tu_buffer[16] == 0x11))

#define FRU_2D_205 (int)(((unsigned)tu_buffer[26] == 0x11  && \
                          (unsigned)tu_buffer[25] == 0x10  && \
                          (unsigned)tu_buffer[24] == 0x11) || \
                         ((unsigned)tu_buffer[19] == 0x11 && \
                          (unsigned)tu_buffer[18] == 0x10 && \
                          (unsigned)tu_buffer[17] == 0x11 && \
                          (unsigned)tu_buffer[16] == 0x10))

#define FRU_2D_206 (int)(((unsigned)tu_buffer[26] == 0x11  && \
                          (unsigned)tu_buffer[25] == 0x11  && \
                          (unsigned)tu_buffer[24] == 0x10) || \
                         ((unsigned)tu_buffer[19] == 0x11 && \
                          (unsigned)tu_buffer[18] == 0x11 && \
                          (unsigned)tu_buffer[17] == 0x10 && \
                          (unsigned)tu_buffer[16] == 0x10))

#define FRU_2D_207 (int)((unsigned)tu_buffer[19] == 0x10 && \
                         (unsigned)tu_buffer[18] == 0x11 && \
                         (unsigned)tu_buffer[17] == 0x10 && \
                         (unsigned)tu_buffer[16] == 0x11)

#define FRU_2D_208 (int)((unsigned)tu_buffer[19] == 0x10 && \
                         (unsigned)tu_buffer[18] == 0x11 && \
                         (unsigned)tu_buffer[17] == 0x11 && \
                         (unsigned)tu_buffer[16] == 0x10)

#define FRU_2D_209 (int)((unsigned)tu_buffer[19] == 0x11 && \
                         (unsigned)tu_buffer[18] == 0x11 && \
                         (unsigned)tu_buffer[17] == 0x11 && \
                         (unsigned)tu_buffer[16] == 0x10)

#define FRU_2D_210 (int)((unsigned)tu_buffer[19] == 0x11 && \
                         (unsigned)tu_buffer[18] == 0x11 && \
                         (unsigned)tu_buffer[17] == 0x10 && \
                         (unsigned)tu_buffer[16] == 0x11)

#define FRU_2D_211 (int)((unsigned)tu_buffer[19] == 0x11 && \
                         (unsigned)tu_buffer[18] == 0x10 && \
                         (unsigned)tu_buffer[17] == 0x11 && \
                         (unsigned)tu_buffer[16] == 0x11)

#define FRU_2D_212  (int)((unsigned)tu_buffer[29] == 0x11)

#define FRU_2D_213  (int)((unsigned)tu_buffer[27] == 0x11)

#define FRU_2D_214  (int)((unsigned)tu_buffer[28] == 0x11)

/* ELA Macros */
#define ELA_FRUC(e)             (unsigned)(e[34])
#define ELA_INQUIRY_2GB(e)      (int)(!strncmp(&e[103], INQUIRY_2GB, 7))
#define ELA_PFA(e)              (int)((e[34] == 0x5D) && (e[35] == 0x00))
#define ELA_VALID_SENSE(e)      (int)(e[20] & 0x80)

        /* Global struct for various array variables. */
struct array_vars {
        struct CuDv *cudv;
        struct listinfo linfo;
        char dar_name[NAMESIZE];
        char lun_name[NAMESIZE];
        int dar_cfg_state;
        int lun_cfg_state;
        int lun_counter;
        int pdisk_counter;
        int num_pdisks;
        int pdisk_ch;
        int pdisk_id;
	int spt_flag;
        struct pdisk *pdisk_ptr;
        char fru_info[80];
} arr;

        /* Physical disk struct used for array status. */
struct pdisk {
        int ch;
        int id;
        int lun_id;
        uchar status_byte;
        int status_msg;
	int srn;
        int flags;
#define PDISK_BELONG_TO_LUN		0x01
#define PDISK_DIAGNOSED			0x02
#define INQ_PID_SIZE                    8
        char inq_pid[INQ_PID_SIZE];
#define PDISK_CAPSIZE                   9
        char capacity[PDISK_CAPSIZE];
        char ucode_level[20];
        long ffc;
        struct pdisk *next;
};

        /* LUN struct used for array status. */
struct lun {
        char name[NAMESIZE];
        char location[NAMESIZE];
#define RAID_LEVELSIZE                 7
        char raid_level[RAID_LEVELSIZE];
#define LUN_CAPSIZE                     9
        char capacity[LUN_CAPSIZE];
        uchar status_byte;
        int status_msg;
        int pdisk_count;
        struct pdisk *pdisk;
        struct lun *next;
};

        /* Array status head pointer. */
struct lun *raid_hptr = (struct lun *)NULL;

        /* Pointer for the array_status message buffer. */
char *array_status;

        /* Physical disk location codes for the array. This array of */
        /* strings is arranged so an index of [ch][id] will give the */
        /* physical disk location code.                              */
char *pdisk_locs[][MAX_SUPPORTED_CHs + 1] = {
	{ {"UNKNOWN"},{"n/a "},{"n/a "},{"n/a "},{"n/a "},{"n/a "}  },
 	{ {"LR-3 (10)"},{"LR-4 (11)"},{"LR-5 (12)"},{"UF-8 (13)"},{"UF-6 (14)"},{"UF-7 (15)"}  },
	{ {"LR-6 (20)"},{"LR-7 (21)"},{"LR-8 (22)"},{"UF-5 (23)"},{"UF-3 (24)"},{"UF-4 (25)"}  },
	{ {"LF-1 (30)"},{"LF-2 (31)"},{"LR-1 (32)"},{"LR-2 (33)"},{"UF-1 (34)"},{"UF-2 (35)"}  },
	{ {"LF-6 (40)"},{"LF-7 (41)"},{"LF-8 (42)"},{"UR-5 (43)"},{"UR-3 (44)"},{"UR-4 (45)"}  },
	{ {"LF-3 (50)"},{"LF-4 (51)"},{"LF-5 (52)"},{"UR-8 (53)"},{"UR-6 (54)"},{"UR-7 (55)"}  }
};

int lun_owns_pdisk[MAX_SUPPORTED_CHs+1][MAX_SUPPORTED_IDs+1];

struct status_bytes {
        uchar status_byte;
        int status_msg;
        int srn;
};
        /* Status bytes for the MODE SENSE Array Physical page 2A. */
struct status_bytes sbytes_2A[] = {

        { 0x00, OPTIMAL_DRIVE , 0x000 },

        { 0x01, NON_EXISTENT  , 0x000 },{ 0x11, NON_SUPPORT_CH, 0x000 },
        { 0x21, NON_SUPPORT_ID, 0x000 },{ 0x31, NON_SUPP_CH_ID, 0x000 },

        { 0x02, SPARE_DRIVE   , 0x000 },{ 0x12, HOT_SPARE_DRIV, 0x000 },

        { 0x03, FAILED_DRIVE  , 0x130 },{ 0x13, COMP_FAILURE  , 0x131 },
        { 0x23, TUR_FAILURE_S , 0x132 },{ 0x33, FORMAT_FAILURE, 0x133 },
        { 0x43, WRITE_FAILURE , 0x134 },{ 0x53, USER_FAILED_MS, 0x135 },
        { 0x63, S_OF_DAY_FAIL , 0x136 },

        { 0x04, REPLACED_DR   , 0x000 },{ 0x14, FORMAT_INIT   , 0x000 },
        { 0x24, RECONSTR_INIT , 0x000 },

        { 0x05, DRIVE_WARNING , 0x150 },

        { 0x06, PARAM_MISMATCH, 0x160 },{ 0x16, WRONG_SECTOR_S, 0x161 },
        { 0x26, WRONG_CAPACITY, 0x162 },{ 0x36, INC_MODE_PARAM, 0x163 },
        { 0x46, WRONG_CNTRL_SN, 0x164 },{ 0x56, CH_MISMATCH   , 0x165 },
        { 0x66, ID_MISMATCH   , 0x166 },

        { 0x07, THIS_CNTRL    , 0x000 },

        { 0x08, DR_FORMAT_INIT, 0x000 },{ 0x18, DR_PEND_FORMAT, 0x181 },

        { 0x09, WRONG_DR_REPL , 0x190 },

        { 0xFF, UNKNOWN       , 0x000 } /* Marks the end of array! */
};

        /* Status bytes for the MODE SENSE Logical Array page 2B. */
struct status_bytes sbytes_2B[] = {
          { 0x00, OPTIMAL_LUN   , 0x000 } , { 0x10, PARAM_MISMATCH, 0x000 },
          { 0x20, PSCAN         , 0x000 } , { 0x30, PSCAN_MISMATCH, 0x000 },
          { 0x50, PSCAN_ERROR   , 0x000 } , 
          { 0x40, R0_DR_FORMAT  , 0x000 } , { 0x01, DR_FAILURE    , 0x000 },
          { 0x11, PARAM_MISMATCH, 0x000 } , { 0x21, CH_MISMATCH   , 0x000 },
          { 0x31, ID_MISMATCH   , 0x000 } , { 0x41, REPL_DR_FORMAT, 0x000 },
          { 0x81, COMP_FAILURE  , 0x000 } , { 0x02, RECONSTR_INIT , 0x000 },
          { 0x12, PARAM_MISMATCH, 0x000 } , { 0x22, CH_MISMATCH   , 0x000 },
          { 0x82, COMP_FAILURE  , 0x000 } , { 0x04, MULT_DR_FAILED, 0x000 },
          { 0x14, PARAM_MISMATCH, 0x000 } , { 0x24, CH_MISMATCH   , 0x000 },
          { 0x34, ID_MISMATCH   , 0x000 } , { 0x44, FORMAT_IN_PROG, 0x000 },
          { 0x54, AWAIT_FORMAT  , 0x301 } , { 0x74, REPL_WRONG_DR , 0x000 },
          { 0x84, COMP_FAILURE  , 0x000 },
          { 0xBB, BAD_LUN       , 0x000 },  /* Open returned EBUSY     */
          { 0xFF, UNKNOWN       , 0x000 }   /* Marks the end of array! */
};

        /* FRU structs (see da.h)                                       */
        /* struct fru_bucket: dname, ftype, sn, rcode, rmsg, frus[]     */
        /* struct frus: conf, fname, floc, fmsg, fru_flag, fru_exempt   */
        /*                    ^                                         */
        /*    > fname: raid_fru() will convert (atoi) and use the       */
        /*             value as the catalog number. Compile and use the */
        /*             dmedia_msg.h file to fill in the fname values.   */
        /*                                                              */
struct fru_bucket raid_frub[] = {
/* FRUC != 0 */
   { "" , FRUB1 , 0xD55 , 0x101 , RMSG_101 },
   { "" , FRUB1 , 0xD55 , 0x102 , RMSG_102 },
/* MODE SENSE 2D */
   { "" , FRUB1 , 0xD55 , 0x200 , RMSG_200 },
   { "" , FRUB1 , 0xD55 , 0x201 , RMSG_FAN },
   { "" , FRUB1 , 0xD55 , 0x202 , RMSG_FAN },
   { "" , FRUB1 , 0xD55 , 0x203 , RMSG_FAN },
   { "" , FRUB1 , 0xD55 , 0x204 , RMSG_FAN },
   { "" , FRUB1 , 0xD55 , 0x205 , RMSG_FAN },
   { "" , FRUB1 , 0xD55 , 0x206 , RMSG_FAN },
   { "" , FRUB1 , 0xD55 , 0x207 , RMSG_207 },
   { "" , FRUB1 , 0xD55 , 0x208 , RMSG_208 },
   { "" , FRUB1 , 0xD55 , 0x209 , RMSG_209 },
   { "" , FRUB1 , 0xD55 , 0x210 , RMSG_210 },
   { "" , FRUB1 , 0xD55 , 0x211 , RMSG_209 },
   { "" , FRUB1 , 0xD55 , 0x212 , RMSG_212 },
   { "" , FRUB1 , 0xD55 , 0x213 , RMSG_213 },
   { "" , FRUB1 , 0xD55 , 0x214 , RMSG_214 },
/* ELA Subsystem failure */
   { "" , FRUB1 , 0xD55 , 0x215 , RMSG_ELA_SUBSYS },
/* MODE SENSE 2B  (Sub/Main Status 0x54) */
   { "" , FRUB1 , 0xD55 , 0x301 , RMSG_F_LUN ,
      { { 100 , ""   , "" , FORMAT_LUN      , NOT_IN_DB , EXEMPT } } },
/* SCSI Bus Error */
   { "" , FRUB1 , 0xD55 , 0x302 , RMSG_302   },
/* OPENX */
   { "" , FRUB1 , 0xD55 , 0x307 , RMSG_OPENX,
      { { 80  , "" , "" , 0, DA_NAME , EXEMPT },
        { 10  , "" , "" , 0, PARENT_NAME , EXEMPT },
        { 10  , "" , "" , SOFTWARE, NOT_IN_DB , EXEMPT } } },
/* CONFIG */
   { "" , FRUB1 , 0xD55 , 0x308 , RMSG_DEV_CFG,
      { { 80  , "" , "" , 0, DA_NAME , EXEMPT },
        { 20  , "" , "" , 0, PARENT_NAME , EXEMPT } } },
   { "" , FRUB1 , 0xD55 , 0x309 , RMSG_PAR_CFG   ,
      { { 80  , "" , "" , 0, PARENT_NAME , EXEMPT },
        { 10  , "" , "" , 0, DA_NAME , EXEMPT },
        { 10  , "" , "" , BLANK, NOT_IN_DB , EXEMPT } } },
/* UNKNOWN PDISK */
   { "" , FRUB1 , 0xD55 , 0x310 , RMSG_PDISK,
      { { 80 ,  "" , "" , DISK_MOD , NOT_IN_DB , EXEMPT },
        { 20  , "" , "" , 0, DA_NAME  , EXEMPT } } },
   { "" , FRUB1 , 0xD55 , 0x311 , RMSG_311 },
/* ELA */
   { "" , FRUB1 , 0xD55 , 0x312 , RMSG_312 },
   { "" , FRUB1 , 0xD55 , 0x401 , RMSG_401 },
   { "" , FRUB1 , 0xD55 , 0x402 , RMSG_402 },
   { "" , FRUB1 , 0xD55 , 0x404 , RMSG_404 },
   { "" , FRUB1 , 0xD55 , 0x405 , RMSG_405 },
   { "" , FRUB1 , 0xD55 , 0x406 , RMSG_406 },
/* MODE SENSE 2A Status Bytes (rcode is Main/Sub status) */
   { "" , FRUB1 , 0x845 , 0x130 , COMP_F ,
      { { 100 , "" , "" , DISK_MOD        , NOT_IN_DB , EXEMPT } } },
   { "" , FRUB1 , 0x845 , 0x131 , COMP_FAILURE ,
      { { 100 , "" , "" , DISK_MOD        , NOT_IN_DB , EXEMPT } } },
   { "" , FRUB1 , 0x845 , 0x132 , TUR_FAILURE ,
      { { 100 , "" , "" , DISK_MOD        , NOT_IN_DB , EXEMPT } } },
   { "" , FRUB1 , 0x845 , 0x133 , FORMAT_F ,
      { { 100 , "" , "" , DISK_MOD        , NOT_IN_DB , EXEMPT } } },
   { "" , FRUB1 , 0x845 , 0x134 , WRITE_F ,
      { { 100 , "" , "" , DISK_MOD        , NOT_IN_DB , EXEMPT } } },
   { "" , FRUB1 , 0x845 , 0x135 , MS_FAILURE ,
      { { 100 , "" , "" , DISK_MOD   , NOT_IN_DB , EXEMPT } } },
   { "" , FRUB1 , 0x845 , 0x136 , S_OF_DAY_FAILURE ,
      { { 100 , "" , "" , DISK_MOD        , NOT_IN_DB , EXEMPT } } },
   { "" , FRUB1 , 0x845 , 0x150 , DR_WARNING  ,
      { { 100 , "" , "" , DISK_MOD        , NOT_IN_DB , EXEMPT } } },
   { "" , FRUB1 , 0x845 , 0x160 , PARAM_MM_MSG ,
      { { 100 , "" , "" , DISK_MOD       , NOT_IN_DB , EXEMPT } } },
   { "" , FRUB1 , 0x845 , 0x161 , WRONG_SECTOR ,
      { { 100 , "" , "" , DISK_MOD     , NOT_IN_DB , EXEMPT } } },
   { "" , FRUB1 , 0x845 , 0x162 , WRONG_CAPACITY_F ,
      { { 100 , "" , "" , DISK_MOD       , NOT_IN_DB , EXEMPT } } },
   { "" , FRUB1 , 0x845 , 0x163 , INC_MODE_PARAMS ,
      { { 100 , "" , "" , DISK_MOD        , NOT_IN_DB , EXEMPT } } },
   { "" , FRUB1 , 0x845 , 0x164 , WRONG_CNTRL_SN_F ,
      { { 100 , "" , "" , DISK_MOD        , NOT_IN_DB , EXEMPT } } },
   { "" , FRUB1 , 0x845 , 0x165 , CH_MISMATCH_F ,
      { { 100 , "" , "" , DISK_MOD       , NOT_IN_DB , EXEMPT } } },
   { "" , FRUB1 , 0x845 , 0x166 , ID_MISMATCH_F ,
      { { 100 , "" , "" , DISK_MOD       , NOT_IN_DB , EXEMPT } } },
   { "" , FRUB1 , 0x845 , 0x181 , DR_PEND_FORMAT_F ,
      { { 100 , "" , "" , DISK_MOD     , NOT_IN_DB , EXEMPT } } },
   { "" , FRUB1 , 0x845 , 0x190 , WRONG_DRIVE ,
      { { 100 , "" , "" , DISK_MOD        , NOT_IN_DB , EXEMPT } } },
   { "" , FRUB1 , 0x845 , 0x199 , RMSG_PFA ,
      { { 100 , "" , "" , DISK_MOD        , NOT_IN_DB , EXEMPT } } },
   { "" , FRUB1 , 0x845 , 0x200 , RMSG_ELA_PFA ,
      { { 100 , "" , "" , DISK_MOD        , NOT_IN_DB , EXEMPT } } },
   { "" , FRUB1 , 0x845 , 0x403 , RMSG_TTD_CIOP ,
      { { 100 , "" , "" , DISK_MOD        , NOT_IN_DB , EXEMPT } } },
/* *** End of Structure (sn and rcode = 0) *** */
   {"",FRUB1,0,0,0,{{200,"","",0,NOT_IN_DB,EXEMPT} } }
};

/* end of 7135a.h */
