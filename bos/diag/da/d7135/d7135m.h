/* @(#)40       1.3  src/bos/diag/da/d7135/d7135m.h, da7135, bos41J, bai13 3/26/95 22:38:25 */
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

	/* Include files */
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
#include	<sys/errids.h>		/*				    */
#include	<sys/scdisk.h>		/* SCSI Disk device driver.	    */
#include	<sys/scsi.h>		/* SCSI device driver.		    */
#include	<sys/stat.h>		/* This was needed for debug.	    */
#include	<sys/time.h>		/* Always nice to know the time.    */
#include	<sys/types.h>		/*				    */

#include	<sys/scarray.h> 	/* scarray header file. */

	/* Diag Include files */
#include	<diag/da_rc.h>		/*				    */
#include	<diag/diag.h>		/*				    */
#include	<diag/diag_exit.h>	/* Return codes for Diag Controller */
#include	<diag/diago.h>		/*				    */
#include	<diag/dascsi.h> 	/*				    */
#include	<diag/class_def.h>	/* object class data structures     */
#include	<diag/tm_input.h>	/*				    */
#include	<diag/tmdefs.h> 	/*				    */
#include	<diag/da.h>		/*				    */
#include	<diag/scsi_atu.h>	/* SCSI App Test Unit (SCATU).	    */
#include	"d7135_msg.h"           /* Message catalog file.            */
#include	"d7135a.h"              /* 7135 RAIDiant Array header file. */

	/* Function Prototypes for d7135m.c */
void	int_handler(int);
int	do_seq(int);
void	seq_mgr(int);
int	do_task(void);
int	do_erp(void);
int	test_condition(char *);
char	*get_erp_code(int, int);
int	perform_reactions(char *);
int	init_tucb(char *);
int	rs_ioctl(char *);
int	ptrs_ioctl(char *);
char	*get_data(int, int);
int	run_method(char *, uchar, char*);
int	get_cfg_status(char *);
char	*cat_data(int data_type, char *pad, char *delim, char *code_str);
int	req_sense(void);
int	display_screen(char *);
char	*alloc_msg(char *, int *);
void	check_asl(void);
char	*str_parse(char *, char *, int);
int	display_menugoal(char *);
int	clean_up(int);

extern char *diag_cat_gets(nl_catd, int, int);
nl_catd diag_catopen();
nl_catd catd;            /* Message catalog file descriptor          */

	/* Function Prototypes for d7135t.c */
int	dev_open(char *);
int	config(char *);
char    *get_pdisk_capacity(char *, int, int);
char    *get_pdisk_pid(char *, int, int);
int     get_pdisk_ffc(char *);
int	retry_mgr(int, int);

	/* Function Prototypes for trace. */
void	dt(int, ...);
void	p_data(FILE *, char *, int);

	/* Initialization Defines (used with sa_flags). */
#define DA_INIT_ODM		0x01
#define DA_INIT_ASL		0x02
#define DA_INIT_CAT		0x04
#define DA_RESERVED_DEV 	0x08
	/* Task type defines.	     */
#define CONFIG_TASK		"0"     /* 0XXX */
#define CONFIG_DAC_TASK 	"00"    /* 00XX */
#define CONFIG_LUN_TASK 	"01"    /* 01XX */
#define CONFIG_DAR_TASK 	"02"    /* 02XX */
#define OPEN_TASK		"1"     /* 1XXX */
#define OPEN_DAC_TASK		"10"    /* 11XX */
#define OPEN_LUN_TASK		"11"    /* 11XX */
#define OPENX_LUN_TASK		"12"    /* 10XX */
#define OPENX_DAR_TASK		"13"    /* 10XX */
#define SCREEN_TASK		"3"     /* 3XXX */
#define ROUTER_TASK		"4"     /* 4XXX */
#define GET_ROUTER_TASK 	"40"    /* 40XX */
#define SET_ROUTER_TASK 	"41"    /* 41XX */
#define SCARRY_TASK		"5"     /* 5XXX */
#define SCATU_TASK		"6"     /* 6XXX */
#define ELA_TASK		"7"     /* 7XXX */
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
#define DM_STATUS_GOOD		0
#define DM_ERROR_EINVAL 	0xFFF2
#define DM_STATUS_BAD		0xFFF3
#define DM_ERROR_OPEN		0xFFF4
#define DM_ERROR_ODM		0xFFF5
#define DM_ERROR_OTHER		0xFFF6
#define DM_ERROR_UNEXPECTED	0xFFF7
#define DM_MORE_CONT		0xFFF8
#define DM_TASK_CONT		0xFFF9
#define DM_USER_CANCEL		0x30C0
#define DM_USER_EXIT		0x30E0
	/* ERP return code defines */
#define ERP_GOOD		1
#define ERP_RETRY		0
#define ERP_RETRY_PREV	       -1
#define ERP_FAIL		0xFFFF
	/* SCATU Task defines (sequence codes) */
#define TASK_SCATU_RELEASE_UNIT 	"6017"
#define TASK_SCATU_INQUIRY		"6012"
#define TASK_SCATU_REQUEST_SENSE	"6003"
	/* Sequence defines */
#define SEQ_CREATE	   0xD0 /*					    */
#define SEQ_DELETE	   0xD1 /*					    */
	/* Retry defines */
#define CHECK_RETRY	   0xE0 /*					    */
#define CLEAR_RETRY	   0xE1 /*					    */
#define RETRY_RETRY	   0xE2 /*					    */
	/* Misc */
#define Main_DA_Seq	   1    /*					    */
#define DEV_CFG_FAILED	   46
#define PAR_CFG_FAILED	   48
#define NO_DEVICES	   0xF0 /* Error rc for no devices to display.	    */
#define CONDITION	   0xF1 /*					    */
#define REACTION	   0xF2 /*					    */
#define CLEAN_UP	   0xF3 /*					    */
#define RETURN		   0xF4 /*					    */
#define DONT_DISPLAY_DEV   0xF8 /*					    */
#define DISPLAY_DEV	   0xF9 /*					    */
#define DISPLAYED_DEV	   0xFA /*					    */
#define INF_VPD_STR	   "71350210"
#define INF_DAC_FFC	   0xD55
#define DAC_FFC_9302	   0x844
	/* Config Methods (for run_method) */
#define CFG_METHOD	   1
#define CHG_METHOD	   2
#define UCFG_METHOD	   4

	/* Structures */
struct da_seq { 		/*					    */
	int step;		/*					    */
	char *task_str; 	/*					    */
	struct da_seq *next;	/*					    */
};

struct da_data {		/*					    */
	char *task;		/* Points to current task in sequence.	    */
	char *erp_str;		/*					    */
	int task_rc;		/* Return code form last task executed.     */
	struct da_seq *seq;	/*					    */
	int skey;		/*					    */
	long scode;		/*					    */
	char *sdata;		/*					    */
} da;

struct da_cudv {
	int count;		/* Count of divices to display. 	    */
	int *asl;		/*					    */
	int select;		/*					    */
	struct CuDv *cudv;	/*					    */
	struct listinfo linfo;	/*					    */
} dev;

struct retry {			/*     See check_retry() in utasks.c	    */
	int step;		/* Step number in the sequence. 	    */
	char *task_str; 	/* Sequence requiring a retry.		    */
	int count;		/* Retry count. Decrements until 0.	    */
	struct retry *next;	/* Pointer to next retry struct.	    */
} *retry;

	/* Screen structures and defines. */
				/*					    */
#define MENU_GOAL	99
				/*					    */
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

	/* TUCB structures and defines. */
				/*					    */
uchar tucb_flags[] = { { B_READ  },
/* See Note 1. */      { B_WRITE }
};
				/*					    */
int tucb_passthru[] = { { DKIOCMD  },
/* See Note 1. */	{ DKIORDSE },
			{ DKIOWRSE }
};

struct router {
	struct router_ioctl_state ioctl;
	uchar prev_state;
	int state_change;
	int single_ctrl;
} router;

uchar ms_2A_buff[255];

	/* MACROS */

#define CONSOLE 		(int)(tm_input.console == CONSOLE_TRUE)
#define ADVANCED		(int)(tm_input.advanced == ADVANCED_TRUE)
#define SYSTEM			(int)(tm_input.system == SYSTEM_TRUE)
#define SYSX			(int)(tm_input.exenv == EXENV_SYSX)
#define NOTLM			(int)(tm_input.loopmode == LOOPMODE_NOTLM)
#define ENTERLM 		(int)(tm_input.loopmode == LOOPMODE_ENTERLM)
#define INLM			(int)(tm_input.loopmode == LOOPMODE_INLM)
#define EXITLM			(int)(tm_input.loopmode == LOOPMODE_EXITLM)
#define PD_MODE 		(int)(tm_input.dmode == DMODE_PD)
#define ELA_MODE_ONLY		(int)(tm_input.dmode == DMODE_ELA)
#define ELA_NOT_SUPPORTED	(int)(!ELA_MODE_ONLY && !PD_MODE)

#define INTERACTIVE_TEST_MODE	(int)(CONSOLE && (!SYSTEM && (NOTLM||ENTERLM)))
#define SCREEN_IS_INTERACTIVE(a) \
	(int)(asl_scr_types[a] != ASL_DIAG_OUTPUT_LEAVE_SC)

#define CHECK_CONDITION(x)	(int)(x == SCATU_CHECK_CONDITION)
#define UNIT_ATTENTION(x,y)	(int)(CHECK_CONDITION(x) && (y == 0x06))

struct da_seq *main_seq;        /* Pointer to main sequence of tasks.       */
SCSI_TUTYPE tucb;               /* SCATU structure for SCSI device TU's.    */
struct tm_input tm_input;
struct CuDv *cudv;              /*                                          */
struct listinfo linfo;          /*                                          */
int iplmode;                    /*                                          */
int test_time;                  /*                                          */
char attr[128];                 /* Attribute for ODM calls.                 */
char *parent = (char *)NULL;    /* Parent name for cfg etc.                 */
char *child = (char *)NULL;     /* Child name for cfg, etc.                 */
char **ep = (char **)NULL;      /* Error pointer for strtol().              */
char tu_buffer[1024];
uchar dac_scsi_id;
uchar dac_lun_id;
int dac_ffc;


/* NOTES:
 *
 *	Note 1: Add new members to end of array. Order is important
 *		and must be maintained.
 *	Note 2: Allthorn requires command tagged queuing. The tucb.header.r2
 *		will contain the sc_iocmd struct q_tag_msg value. This
 *		requires a change to the SCATU's code to make it work.
 */

/* end d7135m.h */

