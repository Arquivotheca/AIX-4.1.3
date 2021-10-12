/* @(#)76	1.6.1.2  src/bos/kernel/io/machdd/pgs_bump.h, machdd, bos41J, bai15 4/11/95 11:45:45 */
/*
 * COMPONENT_NAME:  (MACHDD) Machine Device Driver
 * 
 * FUNCTIONS:  PEGASUS BUMP interface
 * 
 * ORIGINS: 83 
 * 
 * LEVEL 1, 5 Years Bull Confidential Information
 */

#ifdef _RS6K_SMP_MCA

#include <sys/err_rec.h>

/* This used to be defined as INTOFFL0.  But due to the NVRAM
 * workaround this became a variable so the value could be 
 * INTCLASS3 for the WA or INTOFFL0 if running on fixed HW
 */
#define INTBUMP 	intbump		/* BUMP interrupt priority */

struct pgs_bump_msg {
	uchar bump_portid;	/* bump portid */
	uchar cpu_portid;	/* cpu portid */
	uchar tid;		/* transaction id */
	uchar reserved;		/* reserved */
	union {
		struct {	/* misc */
			uchar cmd_status;	/* command / status */
			char res1;
			char res2[10];
		} misc;
		struct {	/* reboot  */
			uchar cmd_status;	/* command / status */
			char res1;
			short res2;
			uint ipl_cb;		/* ipl_cb real address */
		} reboot;
		struct {	/* eeprom access */
			uchar cmd_status;	/* command / status */
			uchar eep_num;		/* eeprom number */
			ushort data_length;	/* data length */
			uint data_off;		/* data buffer offset in NVRAM */
			ushort eep_off;		/* eeprom offset */
			char vpd_desc[2];	/* vpd descriptor */
		} eeprom;
		struct {	/* feprom update */
			uchar cmd_status;	/* command / status */
			uchar res1;
			ushort data_length;	/* data length */
			uint data_off;		/* data buffer offset in NVRAM */
		} feprom;
		struct {	/* power supply */
			uchar cmd_status;	/* command / status */
			uchar data_length;	/* data length */
			uchar cabinet_num;	/* cabinet number */
			uchar func;		/* function */
			uint data_off;		/* data buffer offset in NVRAM */
		} power;
		struct {	/* diagnostic info */
			uchar cmd_status;	/* command / status */
			uchar res1;
			ushort data_length;	/* data length */
			uint data_off;		/* data buffer offset in NVRAM */
			uint type;		/* info type */
		} dinfo;
		struct {	/* RDS */
			uchar cmd_status;	/* command / status */
			uchar rds_state;	/* RDS state */
			uchar cabinet_num;	/* cabinet number */
			uchar disk_num;		/* disk number */
		} rds;
		struct {	/* parity error reading */
			uchar cmd_status;	/* command / status */
			uchar data_length;	/* data length */
			uchar res[2];
			uint data_off;		/* data buffer offset in NVRAM */
		} parity;
		struct {	/* single/multipe parity error */
			uchar cmd_status;	/* command / status */
			uchar cmd_detstat;	/* detailed status */
			uchar reserved1[2];
			uchar cmd_board;	/* memory board location */
			uchar reserved2;
			uchar cmd_mask[2];	/* DIMM mask bits */
			caddr_t cmd_addr;	/* faulting address */
		} perror;
		struct {	/* surveillance */
			uchar cmd_status;	/* command / status */
			uchar start_stop;	/* start=0, stop=1 */
			uchar mode;		/* mode */
			uchar delay;		/* delay (seconds) */
		} surv;
		struct {	/* 601 commands */
			uchar cmd_status;	/* command / status */
			uchar cpu_num;		/* cpu number */
			uchar cpu_status;	/* returned cpu status */
			char res;
			uint start_addr;	/* start address */
		} cpu;
		struct {	/* key switch interrupt */
			uchar cmd_status;	/* command / status */
			uchar phys_key;		/* physical key status */
			uchar elec_key;		/* electronical key status */
		} keyint;
	} data;
};
#define	command data.misc.cmd_status
#define	status data.misc.cmd_status

typedef struct pgs_bump_msg pgs_bump_msg_t; 

/* values of BUMP portids */

#define	BUMP_EEPROM_PORTID	0x80	/* eeprom access */
#define	BUMP_PANEL_PORTID	0x81	/* operator panel */
#define	BUMP_POWER_PORTID	0x82	/* power supply */
#define	BUMP_RDS_PORTID		0x83	/* remote disk switch */
#define	BUMP_SYSTEM_PORTID	0x84	/* system commands */
#define	BUMP_FEPROM_PORTID	0x85	/* feprom update */
#define	BUMP_AUTODIAL_PORTID	0x86	/* autodial */

/* values of CPU portids */

#define	CPU_CMD_PORTID		0x0	/* command */
#define	CPU_UNSOL_PORTID	0x7f	/* unsollicited status message */

/* values of command for BUMP_EEPROM_PORTID */

#define	EEPROM_READ	0x8
#define	EEPROM_LOAD	0x9

/* values of command for BUMP_POWER_PORTID */

#define	POWER_STATUS_READ	0x8
#define	POWER_STATUS_CMD	0x9

/* values of command for BUMP_RDS_PORTID */

#define	RDS_STATE_READ		0x8
#define	RDS_COMMAND		0x9

/* values of command for BUMP_SYSTEM_PORTID */

#define	REBOOT			0x1
#define	CPU_SURVEILLANCE	0x4
#define	PROCESSOR_STOP		0x8
#define	PROCESSOR_START		0x9
#define	PROCESSOR_DISABLE	0xa
#define	PROCESSOR_ENABLE	0xb
#define	PROCESSOR_RESET		0xc
#define	PROCESSOR_STATUS	0xd
#define	READ_MULTI_PERR		0x13

/* values of command for BUMP_FEPROM_PORTID */

#define	FEPROM_LOAD		0x9

/* values of command for BUMP_AUTODIAL_PORTID */

#define	DIAG_INFO_READ	0x8
#define	DIAG_INFO_WRITE	0x9

/* values of status for CPU_UNSOL_PORTID */

#define	KEY_INTERRUPT		0x1
#define	SWITCH_INTERRUPT	0x2
#define	POWER_INTERRUPT		0x3
#define	MEMORY_INTERRUPT	0x4
#define	SINGLE_PARITY_ERR_NEW   0x5

/* values of rds_state for RDS_COMMAND */

#define	RDS_SW_ON	0x1
#define	RDS_SW_OFF	0x0

/* "cmd_detstat" may have the following values as a response for */
/* READ_MULTI_PERR (if SINGLE_PARITY_ERR_NEW, then this field is 0) */

#define	MPERR_FOUND	1		/* multiple parity error found */
#define	NOT_MPERR	2		/* not a memory parity error */
#define	INCOH_ASIC	3		/* incoherent ASIC status */
#define	INCOH_INTERL	4		/* incoherent interleaving table */


/*
 * Single and multiple memory parity error log recod
 *
 * A single memory parity error is indicated by the
 * "cmd_status == SINGLE_PARITY_ERR_NEW" unsolicited BUMP interrupt.
 * In this case "cmd_detstat == PERR_FOUND".
 *
 * A multiple memory parity error is indicated by a machine check interrupt.
 * The BUMP is requested to check to see if this machine check interrupt is
 * due to a multiple memory parity error.
 * In this case "cmd_status == 0" unless there is a communication error
 * to BUMP.
 */
struct errors {
	struct err_rec0 _errlog;	/* common error header */
	struct {			/* single parity error */
		uchar cmd_status;	/* command / status */
		uchar cmd_detstat;	/* detailed status */
		uchar cmd_board;	/* memory board location ( 0 - 3 ) */
		uchar cmd_mask[2];	/* DIMM mask bits */
		uchar cmd_addr[4];	/* faulting address */
	} perror;
};

/* "cmd_detstat" may have the following values: */
/* MPERR_FOUND, NOT_MPERR, INCOH_ASIC, INCOH_INTERL: see at READ_MULTI_PERR */
/* In addition: */

#define	PERR_FOUND	0	/* single parity error found */

#endif /* _RS6K_SMP_MCA */
