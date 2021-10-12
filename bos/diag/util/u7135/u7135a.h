/* @(#)54       1.4  src/bos/diag/util/u7135/u7135a.h, dsau7135, bos41J, 9520B_all 5/18/95 09:47:40 */
/*
 *   COMPONENT_NAME: DSAU7135
 *
 *   FUNCTIONS:
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993,1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

	/* Function Prototypes */
int	raid_tasks(int);
int	raid_array_status(int);
void	raid_add_to_status(int, struct lun *);
void	raid_disk_status(void);
void	raid_add_spares(uchar);
char   *raid_get_menu_item(int, int);
char   *raid_pdisk_capacity(int, int);
void	raid_add_buff(char *);
int	raid_certify(void);
int	certify_cac(int []);

	/* Defines */
/* Device types used as param to raid_array_status() function. */
#define CNTRL_STATUS		0x00
#define ALT_CNTRL_STATUS	0x01
#define SPARE_STATUS		0x02
#define LUN_STATUS		0x04
/* Mask used when switching on device type. */
#define DEVICE_TYPE_MASK	0x0F
/* Device type status flags. */
#define DEAD_LUN		0x10
#define OPEN			0x20
#define ALL_PDISKS		0x40
#define SPARE_HOT_SPARE 	0x80

#define MAX_7135_LUNS		7  /* 0-6  */
#define MAX_7135_CNTRLS 	2  /* dac0-dac1  */
#define MAX_IDs 		16 /* 0-15 */
#define MAX_CHs 		15 /* 1-15 */
#define MAX_SUPPORTED_IDs	7  /* 0-6  */
#define MAX_SUPPORTED_CHs	5  /* 1-5  */


#define MAIN_STATUS_MASK	0x0F /* MODE SENSE 2A and 2B */
#define SUB_STATUS_MASK 	0xF0 /*   status masks.      */

#define SPARE			0x02
#define HOT_SPARE		0x12
#define FAILED_PDISK		0x03

	/* Controller status for MODE SENSE Redundant Cntrl (page 2C). */
#define PASSIVE_CNTRL		0x00
#define ACTIVE_CNTRL		0x01
#define FAILED_CNTRL		0x04
#define ALT_CNTRL		0x08
#define BUSY_LUN		0xBB /* Used when open returns busy. */

	/* SA selection defines */
#define SA_CERTIFY_LUN		1
#define SA_CERTIFY_PDISK	2
#define SA_FORMAT_PDISK 	3
#define SA_uCODE_CNTRL		4
#define SA_uCODE_PDISK		5
#define SA_UPDATE_EEPROM	6
#define SA_REPLACE_CNTRL	7

	/* Data file defines */
#define SEQUENCE_DATA		1
#define ASL_SCREEN_DATA 	2
#define SCATU_TUCB_DATA 	3
#define SC_CDB_DATA		4
#define MISC_TASK_DATA		5
#define ERP_DATA		6
#define SC_PL_DATA		7

	/* Misc defines */
#define END_OF_LIST			1
#define BEGINNING_OF_LIST		2
#define INQUIRY_2GB			"0664M1H"
#define INQUIRY_1_3GB			"0664M1S"
#define MS_HD_BD			12
#define GET_MENU_ITEM			0
#define GET_DEV_INFO			1
#define MAX_CERTIFY_RETRY		4
#define LUN_OWN_ODD			0xFFFFFFFF
#define LUN_OWN_EVEN			0x00000000

	/* MACROS */
#define DAC_ID_ODD(a) (int) ((a + 2) % 2)

#define VALID_PDISK_CH(a) (int)((a >= 1) && (a <= MAX_SUPPORTED_CHs))
#define VALID_PDISK_ID(a) (int)((a >= 0) && (a <= (MAX_SUPPORTED_IDs - 2)))

#define CNTRL_IS_ACTIVE 	\
	(int)(((unsigned)tucb.scsitu.data_buffer[47] == 1) || \
	      ((unsigned)tucb.scsitu.data_buffer[47] == 2))

#define CNTRL_IS_PASSIVE	\
	(int)(((unsigned)tucb.scsitu.data_buffer[46] == 1) && \
	     ((unsigned)tucb.scsitu.data_buffer[47] == 0))

#define PASSIVE_CNTRL_IS_HEALTHY    \
	(int)((unsigned)tucb.scsitu.data_buffer[47] == 0)

#define NOT_READY_NO_uCODE	\
	(int)(CHECK_CONDITION(sa.task_rc) && \
	      ((unsigned)tucb.scsitu.data_buffer[7] <= 0x0A))

#define SUBSYS_FRUC	      \
	(int)(CHECK_CONDITION(sa.task_rc) && \
	      ((unsigned)tucb.scsitu.data_buffer[14] == 6))

#define NON_ZERO_FRUC		\
	(int)(CHECK_CONDITION(sa.task_rc) && \
	      ((unsigned)tucb.scsitu.data_buffer[14] != 0))

#define SENSE_DATA_BYTE_14	\
	      ((unsigned)tucb.scsitu.data_buffer[14])

#define PDISK_STATUS	/* This macro will is used to get the disk */  \
			/* status byte in the MODE SENSE 2A data.  */  \
			/* The channel + (id * max_channel) equals */  \
			/* the index into a 1 based data buffer.   */  \
			/* The buff starts at byte 2, so add 1.    */  \
			/* Also add the Mode Sense Header/BlkDesc. */  \
			(uchar)(tucb.scsitu.data_buffer[disk_ptr->ch + \
				(disk_ptr->id * MAX_CHs) + 1 + MS_HD_BD])

	/* Global struct for various array variables. */
struct array_vars {
	struct CuDv *cudv;
	struct listinfo linfo;
	char dar_name[NAMESIZE+1];
	int dar_cfg_state;
	char lun_name[NAMESIZE+1];
	int lun_cfg_state;
	int lun_counter;
	int num_luns;
	char prev_pri_cntrl[NAMESIZE+1];
	int spt_flag;
	int cntrl_counter;
	int num_cntrls;
	int pdisk_ch;
	int pdisk_id;
	int spare_counter;
	int num_spares;
	struct pdisk *pdisk_ptr;
	char fru_info[80];
} arr;

	/* Array status consists of a linked list of LUN's (struct lun) */
	/* where each LUN will contain a linked list of physical disks	*/
	/* (struct physical disk). A LUN struct will also be used (in a */
	/* limited way) to represent each dac and spare disks as well.	*/

	/* Physical disk struct used for array status. */
struct pdisk {
	char name[NAMESIZE];
	int ch;
	int id;
	uchar status_byte;
	int status_msg;
#define PDISK_DL_PASSED 		0x01
#define PDISK_DL_FAILED 		0x02
#define PDISK_DL_NO_FILE 		0x04
	uchar flags;
#define INQ_PID_SIZE			8
	char inq_pid[INQ_PID_SIZE];
#define PDISK_CAPSIZE			9
	char capacity[PDISK_CAPSIZE];
	char ucode_level[8];
	long ffc;
	struct pdisk *next;
};

	/* LUN struct used for array status. */
struct lun {
	char name[NAMESIZE+1];
	char location[NAMESIZE+1];
	char raid_level[7];
	char capacity[9];
	uchar status_byte;
	int status_msg;
	int pdisk_count;
	long pdisk_type;
	struct pdisk *pdisk;
	struct lun *next;
};

	/* Array status head pointer. */
struct lun *raid_hptr = (struct lun *)NULL;

	/* Pointer for the array_status message buffer. */
char *array_status;

	/* Physical disk location codes for the array. This array of */
	/* strings is arranged so an index of [ch][id] will give the */
	/* physical disk location code. 			     */

char *pdisk_locs[][MAX_SUPPORTED_CHs + 1] = {
	{ {"UNKNOWN"},{"n/a "},{"n/a "},{"n/a "},{"n/a "},{"n/a "}  },
 	{ {"LR-3 (10)"},{"LR-4 (11)"},{"LR-5 (12)"},{"UF-8 (13)"},{"UF-6 (14)"},{"UF-7 (15)"}  },
	{ {"LR-6 (20)"},{"LR-7 (21)"},{"LR-8 (22)"},{"UF-5 (23)"},{"UF-3 (24)"},{"UF-4 (25)"}  },
	{ {"LF-1 (30)"},{"LF-2 (31)"},{"LR-1 (32)"},{"LR-2 (33)"},{"UF-1 (34)"},{"UF-2 (35)"}  },
	{ {"LF-6 (40)"},{"LF-7 (41)"},{"LF-8 (42)"},{"UR-5 (43)"},{"UR-3 (44)"},{"UR-4 (45)"}  },
	{ {"LF-3 (50)"},{"LF-4 (51)"},{"LF-5 (52)"},{"UR-8 (53)"},{"UR-6 (54)"},{"UR-7 (55)"}  }
};

int lun_owns_pdisk[MAX_SUPPORTED_CHs+1][MAX_SUPPORTED_IDs+1];
int pdisk_lun_id[MAX_SUPPORTED_CHs+1][MAX_SUPPORTED_IDs+1];

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

	{ 0x04, NO_ACTION     , 0x000 },{ 0x14, FORMAT_INIT   , 0x000 },
	{ 0x24, RECONSTR_INIT , 0x000 },

	{ 0x05, DRIVE_WARNING , 0x150 },

	{ 0x06, PARAM_MISMATCH, 0x160 },{ 0x16, WRONG_SECTOR_S, 0x161 },
	{ 0x26, WRONG_CAPACITY, 0x162 },{ 0x36, INC_MODE_PARAM, 0x163 },
	{ 0x46, WRONG_CNTRL_SN, 0x164 },{ 0x56, CH_MISMATCH   , 0x165 },
	{ 0x66, ID_MISMATCH   , 0x166 },

	{ 0x07, THIS_CNTRL    , 0x000 },

	{ 0x08, DR_FORMAT_INIT, 0x000 },{ 0x18, DR_PEND_FORMAT, 0x181 },

	{ 0x09, WRONG_DR_REPL , 0x190 },

	{ 0xFF, UNKNOWN       , 0x888 } /* Marks the end of array! */
};

	/* Status bytes for the MODE SENSE Logical Array page 2B. */
struct status_bytes sbytes_2B[] = {
	  { 0x00, OPTIMAL_LUN	, 0x000 } , { 0x10, PARAM_MISMATCH, 0x000 },
	  { 0x20, PSCAN 	, 0x000 } , { 0x30, PSCAN_MISMATCH, 0x000 },
	  { 0x50, PSCAN_ERROR	, 0x000 } ,
	  { 0x40, R0_DR_FORMAT	, 0x000 } , { 0x01, DR_FAILURE	  , 0x000 },
	  { 0x11, PARAM_MISMATCH, 0x000 } , { 0x21, CH_MISMATCH   , 0x000 },
	  { 0x31, ID_MISMATCH	, 0x000 } , { 0x41, REPL_DR_FORMAT, 0x000 },
	  { 0x81, COMP_FAILURE	, 0x000 } , { 0x02, RECONSTR_INIT , 0x000 },
	  { 0x12, PARAM_MISMATCH, 0x000 } , { 0x22, CH_MISMATCH   , 0x000 },
	  { 0x82, COMP_FAILURE	, 0x000 } , { 0x04, MULT_DR_FAILED, 0x000 },
	  { 0x14, PARAM_MISMATCH, 0x000 } , { 0x24, CH_MISMATCH   , 0x000 },
	  { 0x34, ID_MISMATCH	, 0x000 } , { 0x44, FORMAT_IN_PROG, 0x000 },
	  { 0x54, AWAIT_FORMAT	, 0x301 } , { 0x74, REPL_WRONG_DR , 0x000 },
	  { 0x84, COMP_FAILURE	, 0x000 },
	  { 0xB0, OPTIMAL_SUB_LUN, 0x000 }, /* Optimal Sub LUN	       */
	  { 0xBB, LUN_IS_BUSY	, 0x000 },  /* Open returned EBUSY     */
	  { 0xFF, UNKNOWN	, 0x888 }   /* Marks the end of array! */
};

/* end of u7135a.h */
