/* @(#)24	1.14.1.17  src/bos/usr/include/diag/diag.h, cmddiag, bos41J, 9514A_all 3/29/95 15:10:51 */
/*
 *   COMPONENT_NAME: CMDDIAG
 *
 *   FUNCTIONS: Diagnostic header file.
 *
 *   ORIGINS: 27, 83
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989, 1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *
 *   LEVEL 1, 5 Years Bull Confidential Information
 *
 */


#ifndef _h_diag
#define _h_diag

#include "diag/diag_define.h"

/* Return codes from the diagnostic supervisor/diagnostic controller */
#define DIAG_EXIT_GOOD		0
#define DIAG_EXIT_DEVICE_ERROR	1
#define DIAG_EXIT_INTERRUPT	2
#define DIAG_EXIT_NO_DEVICE	3
#define DIAG_EXIT_BUSY		4
#define DIAG_EXIT_LOCK_ERROR	5
#define DIAG_EXIT_OBJCLASS_ERROR	6
#define DIAG_EXIT_USAGE		7
#define DIAG_EXIT_SCREEN	8
#define DIAG_EXIT_NoPDiagDev	9
#define	DIAG_EXIT_NO_DIAGSUPPORT  10
#define DIAG_EXIT_DCTRL_RELOAD	98
#define DIAG_EXIT_RESPAWN	201


#define	PTYPE		0
#define	PCLASS		1
#define PSCLASS		2

#define DIAGNOSTICS 		"/usr/lpp/diagnostics"
#define DEFAULT_DADIR 		"/usr/lpp/diagnostics/da"
#define DEFAULT_SLIHDIR 	"/usr/lpp/diagnostics/slih"
#define DEFAULT_UTILDIR		"/usr/lpp/diagnostics/bin"
#define DIAGNLPATH  		"/usr/lpp/diagnostics/catalog"
#define DIAGDATA    		"/etc/lpp/diagnostics/data"
#define DEFAULTOBJPATH 		"/etc/objrepos"
#define CFGMETHODSDIR		"/usr/lib/methods"
#define PORT_CAT 		"dcda.cat"

#ifndef NULL
#define NULL     0
#endif
#ifndef NAMESIZE
#define NAMESIZE 16
#endif
#define MAX_EXPECT 16

#define DIAG_BADRC 0xFF

/* class defininitions, and types */
#define	CPU_DRAWER	"cpu1"
#define	ASYNC_DRAWER	"async1"
#define	SCSI_DRAWER	"scsi1"
#define	MEDIA_DRAWER	"media1"
#define	SYSUNIT     	"sysunit0"

#define CLASS_SYSUNIT 	"sysunit"
#define CLASS_ADAPTER	"adapter"
#define CLASS_DRAWER 	"drawer"
#define CLASS_SYSPLANAR "planar"
#define CLASS_IOPLANAR 	"ioplanar"
#define CLASS_MEMORY	"memory"
#define CLASS_DISK  	"disk"
#define SUBCLASS_SCSI   "scsi"
#define CLASS_DEFAULT	""

#define BASE_OBJ	"sys0"
#define SYSTEM_CHECKOUT  0

/* Definitons for Predefined Diag Device Object Class */
/* SysxFlg - System Exerciser Flags (short) */
#define SYSX_NO		0x0001
#define SYSX_ALONE	0x0002
#define SYSX_MEDIA	0x0004
#define SYSX_INTERACTION	SYSX_MEDIA
#define SYSX_LONG	0x0008

/* SupTests - type of tests supported by the DA's */
#define SUPTESTS_SHR	0x0001
#define SUPTESTS_SUB	0x0002
#define SUPTESTS_FULL	0x0004
#define SUPTESTS_MS1	0x0008
#define SUPTESTS_MS2	0x0010
#define SUPTESTS_HFT    0x0020
#define SUPTESTS_DIAGEX 0x0040

/* Menu - Diagnostic menu identifications */
#define DIAG_DTL 	1
#define DIAG_NOTDLT 	2
#define DIAG_DS		4 
#define DIAG_CON	8 
#define DIAG_DA_SRN	16
#define DIAG_SA_SELECTION 32
#define DIAG_SA_CON_CHILD 64

/* DNext - Resource to be tested next */
#define DIAG_PAR 	1
#define DIAG_SIB 	2

/* Definitions for Customized Diag Device Object Class */
/* TstLvl - level of tests run */
#define TSTLVL_NOTEST 	0
#define TSTLVL_SHR 	1
#define TSTLVL_SUB 	2
#define TSTLVL_FULL 	4

/* RtMenu - Run time menu information */
enum rtinfo{ RTMENU_DEF, RTMENU_DDTL };

/* Other Miscellaneous definitions */
#define DIAG_NO  	0
#define DIAG_YES 	1

#define DIAG_FALSE 	0
#define DIAG_TRUE  	1

/* Change status flag definitions for PdDv and CuDv Object Classes */ 
/*                       see /usr/include/sys/cfgdb.h              */ 
#define NOT_IN_USE	-1
#define	NEW		0
#define	DONTCARE	1
#define	SAME		2
#define MISSING		3

#define INSERT_DSKT_MSG	1
#define RD_DSKT_MSG	2
#define BAD_DSKT_MSG	3
#define WRONG_DSKT_MSG	4

/* Display column information and line types */
#define LINE_LENGTH	74
#define DIAG_SEL_TYPE	 0
#define NTF_TYPE	 1
#define NTF_LOC_COL 	15
#define DS_LOC_COL  	13

/* Menu 801040 - used by controller and SA utility */
#define READING_DSKT	0x801040
#define PROB_REPORT	0x801014
#define DC_SOURCE_SOFT	0x803

/* Miscellaneous flags for device information 	*/
typedef struct {
	unsigned update_database : 1;	/* update device in database 	*/
	unsigned found_in_list   : 1;	/* device found in master list  */
	unsigned defective_device: 1;	/* device is defective - no fru */
	unsigned defective_devfru: 1;   /* device is defective - w/fru  */
	unsigned delete_from_list: 1;	/* delete device from database  */
	unsigned do_not_disp_miss: 1;	/* do not display device in menu*/
	unsigned device_tested   : 1;	/* device has been tested       */
	unsigned device_driver_err:1; 	/* device driver open error     */
} diag_flag_t;

/* structure of key fields from PdDv  - Massive memory saver */
struct Pdv {
	short	led;
	short	detectable;
	short	fru;
	short	setno;
	short 	msgno;
	char	catalog[NAMESIZE];
	};

/* Structure containing all device information needed by the Diag */
/* Controller. Information is derived from the Customized Device  */
/* Object Class, PreDefined Diagnostic Object Class, and the      */
/* Customized Diagnostic Object Class. One entry is made for each */
/* device to be tested. 					  */
typedef struct diag_dev_info_s  {
	char	*Text;			/* describes device 	  */
	char	Asterisk;		/* display asterisk flag  */
	diag_flag_t	    flags;	/* misc device flags	  */
	struct  CuDv     *T_CuDv;    	/* customized device      */
	struct  Pdv	 *T_Pdv;	/* important PdDv items   */
	struct  CuVPD    *CuVPD_HW;	/* hardware vpd           */
	struct  CuVPD    *CuVPD_USER;	/* user entered vpd       */
	struct  CDiagDev *T_CDiagDev;	/* customized diag device */
	struct  PDiagDev *T_PDiagDev;	/* predefined diag device */
	struct  PDiagAtt *T_PDiagAtt;	/* predefined attribute   */
}diag_dev_info_t, *diag_dev_info_ptr_t;

/* Structure containing error log entries for a device.           */
struct errdata  {
	unsigned 	sequence;	/* sequence # of entry 	  */
	unsigned 	time_stamp;	/* entry timestamp     	  */
	unsigned 	err_id;    	/* error id code       	  */
	char		*machine_id;	/* machine id             */ 
	char		*node_id;	/* node id                */ 
	char		*class;     	/* H=hardware, S=software */ 
	char		*type;      	/* PERM,TEMP,PERF,PEND,UNKN*/
	char		*resource;  	/* Configured dev name    */ 
	char		*vpd_data;   	/* VPD info          	  */ 
	char		*conn_where; 	/* connwhere field of CuDv*/ 
	char		*location;   	/* location field of CuDv */ 
	unsigned 	detail_data_len;/* length of detail data  */
	char		*detail_data;   /* detail data            */ 
};

/* Error log operation values */
#define INIT	1
#define	SUBSEQ	2
#define TERMI	3

/* error log types */
#define PERM	"PERM"		/* permanent error	*/
#define TEMP	"TEMP"		/* temporary error	*/
#define PERF	"PERF"		/* performance error    */
#define PEND	"PEND"		/* pending error	*/
#define UNKN	"UNKN"		/* unknown   error	*/

#define HARDWARE 'H'		/* hardware errors	*/
#define SOFTWARE 'S'		/* software errors	*/

/*	ipl_source that matches values in DIAG_IPL_SOURCE	*/

#define	IPLSOURCE_DISK			0
#define	IPLSOURCE_CDROM			1
#define	IPLSOURCE_TAPE			2
#define	IPLSOURCE_LAN			3

/* 	test mode as defined in test_mode attribute		*/

#define SUPTESTS_PERIODIC_MODE	0x0001	/* Can be periodically tested */

/*	default test time					*/

#define DEFAULT_TESTTIME        9999
#define DEFAULT_IOP_TESTTIME 400
#define DEFAULT_DISK_TESTTIME 300

/* name of DAvars error code.			*/

#define	DAVARS_ERRCODE "Error_code"

#endif
