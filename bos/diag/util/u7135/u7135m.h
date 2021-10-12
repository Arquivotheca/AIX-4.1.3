/* @(#)56       1.3  src/bos/diag/util/u7135/u7135m.h, dsau7135, bos41J, bai13 3/26/95 22:39:00 */
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

	/* Include files (in alphabetical order). */
#include	<asl.h> 		/* ASL messages, menus, etc.	    */
#include	<errno.h>		/* open(), openx(), etc.	    */
#include	<fcntl.h>		/* File control (ucode, dt, etc.)   */
#include	<limits.h>		/*				    */
#include	<locale.h>		/* Set locale			    */
#include	<memory.h>		/*				    */
#include	<nl_types.h>		/*				    */
#include	<signal.h>		/* Interrupt handler		    */
#include	<stdio.h>		/* Standard I/O 		    */
#include	<sys/buf.h>		/* B_READ, B_WRITE, etc.	    */
#include	<sys/cfgdb.h>		/*				    */
#include	<sys/errids.h>		/*				    */
#include	<sys/scdisk.h>		/* SCSI Disk device driver.	    */
#include	<sys/scsi.h>		/* SCSI device driver.		    */
#include	<sys/stat.h>		/* File stats (ucode, dt, etc.)     */
#include	<sys/time.h>		/* Always nice to know the time.    */
#include	<sys/types.h>		/*				    */

#include	<sys/scarray.h> 	/* scarray header file (DKIOCMD_RS) */

#include	<diag/da_rc.h>		/*  Refer			    */
#include	<diag/diag.h>		/*     to the			    */
#include	<diag/diago.h>		/*	     DIAG CAS		    */
#include	<diag/dascsi.h> 	/*				    */
#include	<diag/class_def.h>	/* object class data structures     */
#include	<diag/scsi_atu.h>	/* SCSI App Test Unit (SCATU).	    */
#include	"u7135_msg.h"           /* Message catalog file.            */
#include	"u7135a.h"              /* 7135 RAIDiant Array header file. */

	/* Diag Function Prototypes */
extern char *diag_cat_gets(nl_catd, int, int);
nl_catd diag_catopen();
nl_catd catd;            /* Message catalog file descriptor          */

	/* Function Prototypes for u7135m.c */
void	int_handler(int);
int	do_seq(int);
void	seq_mgr(int);
int	do_task(void);
int	do_erp(void);
int	test_condition(char *);
char	*get_erp_code(int, int);
int	perform_reactions(char *);
int	init_tucb(char *);
int	run_method(char *, uchar, char *);
int	get_cfg_state(char *);
char	*cat_data(int data_type, char *pad, char *delim, char *code_str);
int	req_sense(void);
int	display_screen(char *);
char	*alloc_msg(char *);
void	check_asl(void);
char	*str_parse(char *, char *, int);
int	clean_up(int);

	/* Function Prototypes for u7135t.c */
int	dev_open(char *);
int	config(char *);
long	get_pdisk_ffc(char *);
char	*get_pdisk_capacity(char *, int, int);
char	*get_pdisk_pid(char *, int, int);
char	*get_data(int, int);
int	retry_mgr(int, int);
int	dev_open_n(char *name, int task_num);
int	check_readx(void);
int	check_disk_sense(int *rec_action, int *rec_type);
int	rs_ioctl(char *);
int	get_eeprom_fptr(void);
int	get_ucode_fname(void);
int	ucode_compatible(char [], char []);

int	format_md_pages(uchar *, int, uchar *);
int	get_mode_data(int, char *, char *);
int	set_mode_data(int, char *, uchar *, uchar *, uchar *);

	/* Function Prototypes for trace. */
void	dt(int, ...);
void	p_data(FILE *, char *, int);

	/* Initialization Defines (used with sa_flags). */
#define SA_INIT_ODM		0x01
#define SA_INIT_ASL		0x02
#define SA_INIT_CAT		0x04
#define SA_RESERVED_LUN 	0x08
#define SA_RESERVED_PDISK	0x10
#define NO_CONSOLE		0x20
#define CMDLINE_DAC_DOWNLOAD	0x40

	/* Task type defines.	     */
#define CONFIG_TASK		"0"     /* 0XXX */
#define CONFIG_DAC_TASK 	"00"    /* 00XX */
#define CONFIG_LUN_TASK 	"01"    /* 01XX */
#define CONFIG_DAR_TASK 	"02"    /* 02XX */
#define OPEN_TASK		"1"     /* 1XXX */
#define OPEN_DAC_TASK		"10"    /* 10XX */
#define OPEN_LUN_TASK		"11"    /* 11XX */
#define OPENX_LUN_TASK		"12"    /* 10XX */
#define OPENX_DAR_TASK		"13"    /* 13XX */
#define SCREEN_TASK		"3"     /* 3XXX */
#define ROUTER_TASK		"4"     /* 4XXX */
#define GET_ROUTER_TASK 	"40"    /* 40XX */
#define SET_ROUTER_TASK 	"41"    /* 41XX */
#define SCARRY_TASK		"5"     /* 5XXX */
#define SCATU_TASK		"6"     /* 6XXX */
#define MISC_TASK		"8"     /* 8XXX */
#define MENUGOAL_TASK		"9"     /* 9XXX */
#define TASK_END_OF_SEQ 	"8999"

#define TASK_IS(x,y) (int)(!strncmp(x, y, strlen(x)))
#define REACTION_IS(x,y) (int)(!strncmp(x, y, strlen(x)))

	/* Reaction type defines. */
#define SRN_REACTION		"2"     /* 2XXX */
#define ASL_SCREEN_REACTION	"3"     /* 3XXX */
#define MENUGOAL_REACTION	"4"     /* 4XXX */
#define SEQUENCE_REACTION	"90"    /* 90XX */
#define RETRY_REACTION		"92"    /* 92XX */

	/* Exit flag status defines. */
#define SA_STATUS_GOOD		0
#define DM_STATUS_GOOD		0
#define DM_USER_CANCEL		-3    
#define DM_USER_EXIT		-10

#define DM_STATUS_CONT		0xFFF1
#define DM_ERROR_EINVAL 	0xFFF2
#define DM_STATUS_BAD		0xFFF3
#define DM_ERROR_OPEN		0xFFF4
#define DM_ERROR_ODM		0xFFF5
#define DM_ERROR_OTHER		0xFFF6
#define DM_ERROR_UNEXPECTED	0xFFF7
#define DM_MORE_CONT		0xFFF8
#define DM_TASK_CONT		0xFFF9

	/* ERP return code defines */
#define ERP_GOOD		1
#define ERP_RETRY		0
#define ERP_RETRY_PREV	       -1
#define ERP_FAIL		0xFFFF

	/* SCATU Task defines (sequence codes) */
#define TASK_SCATU_REQUEST_SENSE	"0002"
#define TASK_SCATU_GET_CH_MODE_DATA	"0003"
#define TASK_SCATU_INQUIRY		"0010"
#define TASK_SCATU_RELEASE_UNIT 	"0011"
#define TASK_SCATU_MODE_SELECT		"0013"

	/* Sequence defines */
#define SEQ_CREATE		0xD0
#define SEQ_DELETE		0xD1
	/* Retry defines */
#define CHECK_RETRY		0xE0
#define CLEAR_RETRY		0xE1
#define RETRY_RETRY		0xE2

	/* Config Methods (for run_method) */
#define CFG_METHOD		0x01
#define CHG_METHOD		0x02
#define UCFG_METHOD		0x04
#define DAC_FFC 		0xD55

	/* Misc */
#define Main_SA_Seq		1
#define DEF_SELECTION		1
#define NO_DEVICES		0xF0
#define CONDITION		0xF1
#define REACTION		0xF2
#define CLEAN_UP		0xF3
#define RETURN			0xF4
#define DONT_DISPLAY_DEV	0xF8
#define DISPLAY_DEV		0xF9
#define DISPLAYED_DEV		0xFA


#define CERTIFY_READ_SIZE	127
#define TU_BUFFER_SIZE		65536
#define FIRST_MD_PG_OFFSET	12
#define SET_ODM_MD		1
#define SET_CUR_MD		2
#define DEV_CFG_FAILED		46

#define CHECK_CONDITION(x)    (int)(x == SCATU_CHECK_CONDITION)

#define UNIT_ATTENTION(x,y)   (int)(CHECK_CONDITION(x) && (y == 0x06))

	/* Structures */
struct sa_seq {
	int step;		/* Current step in the current sequence.    */
	char *task_str; 	/* Pointer to current sequence of tasks.    */
	struct sa_seq *next;	/* Pointer to next sequence of tasks.	*/
};

struct sa_data {
	char *task;		  /* Pointer to current task code.	    */
	char *erp_str;		  /* Pointer to the current ERP data.	    */
	int task_rc;		  /* Task return code.			    */
	struct sa_seq *seq;	  /* Sequence pointer.			    */
	int skey;		  /* Sense Key for CAC's.                   */
	long scode;		  /* Sense Code for CAC's (ASC/ASCQ).       */
	char *sdata;		  /* Sense Data buffer for CAC's.           */
};

struct router {
	struct router_ioctl_state ioctl;
	uchar prev_state;
	int state_change;
	int single_ctrl;
} router;

struct retry {			/*     See check_retry() in utasks.c	    */
	int step;		/* Step number in the sequence. 	    */
	char *task_str; 	/* Sequence requiring a retry.		    */
	int count;		/* Retry count. Decrements until 0.	    */
	struct retry *next;	/* Pointer to next retry struct.	    */
} *retry;

	/* ASL Screen types structure. */
int asl_scr_types[] = { ASL_DIAG_LIST_CANCEL_EXIT_SC,	    /*	0 */
/* See Note 1. */	ASL_DIAG_LIST_COMMIT_SC,	    /*	1 */
			ASL_DIAG_LIST_COMMIT_HELP_SC,	    /*	2 */
			ASL_DIAG_LIST_CANCEL_EXIT_HELP_SC,  /*	3 */
			ASL_DIAG_ENTER_SC,		    /*	4 */
			ASL_DIAG_ENTER_HELP_SC, 	    /*	5 */
			ASL_DIAG_OUTPUT_LEAVE_SC,	    /*	6 */
			ASL_DIAG_KEYS_ENTER_SC, 	    /*	7 */
			ASL_DIAG_NO_KEYS_ENTER_SC,	    /*	8 */
			ASL_DIAG_DIALOGUE_SC,		    /*	9 */
			ASL_DIAG_DIALOGUE_HELP_SC,	    /* 10 */
			ASL_DIAG_DIALOGUE_LIST_SC,	    /* 11 */
			ASL_GENERAL_HELP_SC,		    /* 12 */
			ASL_DIALOGUE_LEAVE_SC,		    /* 13 */
			ASL_OUTPUT_LEAVE_NO_SCROLL_SC,	    /* 14 */
			ASL_DIAG_LEAVE_NO_KEYS_SC,	    /* 15 */
};

	/* TUCB structures. */
				/*					    */
uchar tucb_flags[] = { { B_READ  },
/* See Note 1. */      { B_WRITE }
};
				/*					    */
int tucb_passthru[] = { { DKIOCMD  },
/* See Note 1. */	{ 0	   },
			{ DKIORDSE },
			{ DKIOWRSE }
};

	/* Note 1: Items in array are in order and the index numbers */
	/*	   are used to access the item. ADD NEW ITEMS TO THE */
	/*	   END AND DONT CHANGE EXISTING ITEMS.		     */

struct dev_info {
	int counter;
	int lun;
	long pdisk_type;
	int percent_complete;
	int prev_percent_complete;
	long block_size;
	long current_lba;
	long last_lba;
	char capacity[NAMESIZE+1];
	char location[NAMESIZE+1];
};

	/* Microcode Download structure. */
struct ucode_info {
	int type;
	int rev_level;
	int flags;
#define DOWNLOAD_FROM_DISKETTE		0x01
#define DOWNLOAD_PREVIOUS_LEVEL 	0x02
#define CURRENT_LEVEL_NOT_FOUND 	0x04
#define DL_ALL_PDISKS			0x08
	char current_fname[128];
	char download_fname[128];
	int model_level_start;
	int model_level_length;
	int num_files;
	int num_blocks;
	int block_resid;
	int block_counter;
	int max_block_size;
	int read_size;
	uint address;
	fpos_t file_position;
	char *fbuff;
	struct stat fstat;
	FILE *fptr;
};

struct sa_data sa;		/*					    */
struct sa_seq *main_seq;	/* Pointer to main sequence of tasks.	    */
struct dev_info dev;		/*					    */
struct retry *retry;		/*					    */
struct ucode_info ucode;	/*					    */
struct CuDv *cudv;		/*					    */
struct listinfo linfo;		/*					    */
SCSI_TUTYPE tucb;		/* SCATU structure for SCSI device TU's.    */

long sa_flags;			/* Flags for init status, etc.		    */
int item_selected;		/* Item number from a selection menu.	    */
int dac_selected;		/* The dac selected from a selection menu.  */
int sa_selected;		/* Type of Service Aid selected.	    */
int test_time;			/*					    */
int fdes;			/* Device file descriptor		    */
int dev_cfg_state; 		/* Device config state flag.		    */
int rec_action; 		/* PDG Recovery action (disks). 	    */
int rec_type;			/* PDG Recovery type (disks).		    */
int msg_item;			/*					    */
int dac_ffc;			/*					    */
uchar dac_scsi_id;		/*					    */
uchar lun_id;			/*					    */
char dname[NAMESIZE + 1];	/*					    */
char dnameloc[NAMESIZE + 1];	/*					    */
char parent[NAMESIZE + 1];	/*					    */
char dnameloc[NAMESIZE + 1];	/*					    */
char attr[256]; 		/* Attribute for misc. calls.		    */
char criteria[256];		/* Attribute for ODM calls.		    */
char **ep;			/* Error pointer for strtol().		    */
char tu_buffer[TU_BUFFER_SIZE]; /* TUCB buffer for SCSI commands.	    */
uchar *current_mode_data;	/* Current MODE SELECT data buffer.	    */
uchar *ch_mode_data;		/* Changable MODE SELECT data buffer.	    */
uchar *diag_mode_data;		/* Diag MODE SELECT data buffer.	    */
uchar eeprom_buff[64];		/* EEPROM data buffer.			    */

/* end d7135m.h */

