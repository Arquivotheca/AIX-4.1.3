/* @(#)56       1.98.1.7  src/bos/usr/include/cmdnim_defines.h, cmdnim, bos41J, 9518A_all  5/2/95  15:00:12 */
/*
 *   COMPONENT_NAME: CMDNIM
 *
 *   FUNCTIONS: ./usr/include/cmdnim_defines.h
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_CMDNIM_DEFINES
#define _H_CMDNIM_DEFINES
/*******************************************************************************
*
*                             cmdnim_defines.h
*
* contains common defines for all NIM functions
*******************************************************************************/


/*---------------------------- NIM admin stuff     ---------------------------*/

#define NIM_ATTR_CLASSNAME			"nim_attr" 
#define NIM_PDATTR_CLASSNAME		"nim_pdattr" 
#define NIM_OBJECT_CLASSNAME		"nim_object" 


#define	NIM_MSTR_NET				"mstrnet"
#define	NIM_REG_SERVICE			"nimreg"
#define	NIM_SERVICE					"nim"
#define	NIM_AWK						"/usr/lpp/bos.sysmgt/nim/awk"
#define	NIM_COMMANDS				"/usr/sbin"
#define	NIM_ENV_FILE				"/etc/niminfo"
#define	NIM_LOG_FILE				"/var/adm/ras/nim"
#define	NIM_METHODS					"/usr/lpp/bos.sysmgt/nim/methods"
#define	NIM_VAR_DIR					"/var/adm/nim"
#define	NIM_XOP_DIR					NIM_VAR_DIR"/xop"

#define	EXPORT_FILE					"/etc/xtab"
#define	NIM_EXPORT					"/export/nim"
#define	TFTPBOOT   					"/tftpboot"
#define	NETWORK_DIR					"/SPOT/usr/lib/boot/network"

#define	RC_BOS_INST					"rc.bos_inst"
#define	RC_DD_BOOT					"rc.dd_boot"
#define	RC_DIAG						"rc.diag"

/*------------------------- awk scripts          -----------------------------*/

#define AWK_lsnim_l					"awk -f " NIM_AWK "/lsnim_l.awk"
#define AWK_lsnim_L					"awk -f " NIM_AWK "/lsnim_L.awk"
#define AWK_lsnim_P					"awk -f " NIM_AWK "/lsnim_P.awk"
#define AWK_lsnim_pa					"awk -f " NIM_AWK "/lsnim_pa.awk"

/*---------------- NIM commands and methods  ---------------------------------*/

#define METH_MAX_ARGS				256

#define NIMCLIENT					NIM_COMMANDS"/nimclient"
#define NIMESIS  					NIM_COMMANDS"/nimesis"

#define M_ABORT					NIM_METHODS"/m_abort"
#define M_ALLOCATE				NIM_METHODS"/m_allocate"
#define M_ALLOC_BOOT				NIM_METHODS"/m_alloc_boot"
#define M_ALLOC_NS				NIM_METHODS"/m_alloc_ns"
#define M_ALLOC_PDIR				NIM_METHODS"/m_alloc_pdir"
#define M_ALLOC_SPOT				NIM_METHODS"/m_alloc_spot"
#define M_BOS_INST				NIM_METHODS"/m_bos_inst"
#define M_CHMAC					NIM_METHODS"/m_chmac"
#define M_CHMASTER				NIM_METHODS"/m_chmaster"
#define M_CHMSTATE 				NIM_METHODS"/m_chmstate"
#define M_CHNET					NIM_METHODS"/m_chnet"
#define M_CHRES					NIM_METHODS"/m_chres"
#define M_CHSTATE					NIM_METHODS"/m_chstate"
#define M_CKMAC					NIM_METHODS"/m_ckmac"
#define M_CKSPOT					NIM_METHODS"/m_ckspot"
#define M_CK_LPP_SOURCE			NIM_METHODS"/m_ck_lpp_source"
#define M_CUST						NIM_METHODS"/m_cust"
#define M_DD_BOOT					NIM_METHODS"/m_dd_boot"
#define M_DEALLOCATE				NIM_METHODS"/m_deallocate"
#define M_DEALLOC_BOOT			NIM_METHODS"/m_dealloc_boot"
#define M_DEALLOC_NS				NIM_METHODS"/m_dealloc_ns"
#define M_DEALLOC_PDIR			NIM_METHODS"/m_dealloc_pdir"
#define M_DEALLOC_SPOT			NIM_METHODS"/m_dealloc_spot"
#define M_DESTROY_RES			NIM_METHODS"/m_destroy_res"
#define M_DIAG						NIM_METHODS"/m_diag"
#define M_DKLS_INST				NIM_METHODS"/m_dkls_inst"
#define M_DO_SOP					NIM_METHODS"/m_do_sop"
#define M_DOREBOOT					NIM_METHODS"/m_doreboot"
#define M_DTLS_INST				NIM_METHODS"/m_dtls_inst"
#define M_FIXQUERY                              NIM_METHODS"/m_fixquery"
#define M_GETDATE 				NIM_METHODS"/m_getdate"
#define M_INSTSPOT				NIM_METHODS"/m_instspot"
#define M_LSLPP					NIM_METHODS"/m_lslpp"
#define M_LS_LPP_SOURCE			NIM_METHODS"/m_ls_lpp_source"
#define M_MAINT					NIM_METHODS"/m_maint"
#define M_MKBOSI					NIM_METHODS"/m_mkbosi"
#define M_MK_LPP_SOURCE			NIM_METHODS"/m_mk_lpp_source"
#define M_MKMAC					NIM_METHODS"/m_mkmac"
#define M_MKNET					NIM_METHODS"/m_mknet"
#define M_MKPDIR					NIM_METHODS"/m_mkpdir"
#define M_MKRES					NIM_METHODS"/m_mkres"
#define M_MKSPOT					NIM_METHODS"/m_mkspot"
#define M_NIMREG     			NIM_METHODS"/m_nimreg"
#define M_NNC_SETUP     		NIM_METHODS"/m_nnc_setup"
#define M_PUSHOFF					NIM_METHODS"/m_pushoff"
#define M_PUSHON 					NIM_METHODS"/m_pushon"
#define M_REBOOT				NIM_METHODS"/m_reboot"
#define M_RESET_RES				NIM_METHODS"/m_reset_res"
#define M_RMMAC					NIM_METHODS"/m_rmmac"
#define M_RMNET					NIM_METHODS"/m_rmnet"
#define M_RMPDIR					NIM_METHODS"/m_rmpdir"
#define M_RMRES					NIM_METHODS"/m_rmres"
#define M_RMSPOT					NIM_METHODS"/m_rmspot"
#define M_SCHEDULE				NIM_METHODS"/m_schedule"
#define M_SPOTMAINT				NIM_METHODS"/m_spotmaint"
#define M_SYNC_ROOTS				NIM_METHODS"/m_sync_roots"
#define M_UNCONFIG				NIM_METHODS"/m_unconfig"

#define C_ADD_ROUTES				NIM_METHODS"/c_add_routes"
#define C_ALLOC_BOOT				NIM_METHODS"/c_alloc_boot"
#define C_AT						NIM_METHODS"/c_at"
#define C_BOSINST_ENV			NIM_METHODS"/c_bosinst_env"
#define C_CH_NFSEXP 				NIM_METHODS"/c_ch_nfsexp"
#define C_CH_RHOST				NIM_METHODS"/c_ch_rhost"
#define C_CKLEVEL					NIM_METHODS"/c_cklevel"
#define C_CKSPOT					NIM_METHODS"/c_ckspot"
#define C_DEALLOC_BOOT			NIM_METHODS"/c_dealloc_boot"
#define C_INITIATE_BOOTP		NIM_METHODS"/c_initiate_bootp"
#define C_INSTALLP				NIM_METHODS"/c_installp"
#define C_INSTSPOT				NIM_METHODS"/c_instspot"
#define C_MK_NIMCLIENT			NIM_METHODS"/c_mk_nimclient"
#define C_MK_LPP_SOURCE			NIM_METHODS"/c_mk_lpp_source"
#define C_MKBOOTI					NIM_METHODS"/c_mkbooti"
#define C_MKDIR					NIM_METHODS"/c_mkdir"
#define C_MKDUMP					NIM_METHODS"/c_mkdump"
#define C_MKPAGING				NIM_METHODS"/c_mkpaging"
#define C_MKROOT					NIM_METHODS"/c_mkroot"
#define C_MKSPOT					NIM_METHODS"/c_mkspot"
#define C_NIMINFO					NIM_METHODS"/c_niminfo"
#define C_NIMPUSH 				NIM_METHODS"/c_nimpush"
#define C_NNC_SETUP 				NIM_METHODS"/c_nnc_setup"
#define C_POPXOP 					NIM_METHODS"/c_popxop"
#define C_PUSHOFF					NIM_METHODS"/c_pushoff"
#define C_PUSHON 					NIM_METHODS"/c_pushon"
#define C_RMDIR					NIM_METHODS"/c_rmdir"
#define C_RMSPOT					NIM_METHODS"/c_rmspot"
#define C_SCRIPT					NIM_METHODS"/c_script"
#define C_STAT						NIM_METHODS"/c_stat"
#define C_SYNC_ROOT				NIM_METHODS"/c_sync_root"

#define NIM  						NIM_COMMANDS	"/nim"
#define LSNIM  					NIM_COMMANDS	"/lsnim"

/*---------------- other commands and methods  -------------------------------*/
#define ALOG						"/usr/bin/alog -q -t nim"
#define AT							"/usr/bin/at"
#define BFFCREATE					"/usr/sbin/bffcreate"
#define CAT							"/usr/bin/cat"
#define CHMOD						"/usr/bin/chmod"
#define CHNFSEXP					"/usr/sbin/chnfsexp"
#define CHROOT						"/usr/sbin/chroot"
#define ECHO  						"/usr/bin/echo"
#define FIRSTBOOT					"/etc/firstboot"
#define HOSTCMD					"/usr/bin/host"
#define INSTALLP					"/usr/sbin/installp"
#define KSH							"/bin/ksh"
#define LSSRC						"/usr/bin/lssrc"
#define MAIL						"/usr/bin/mail "
#define MKDIR						"/usr/bin/mkdir"
#define MKNFSEXP					"/usr/sbin/mknfsexp"
#define MKSSYS  					"/usr/bin/mkssys"
#define MOUNT						"/usr/sbin/mount"
#define PING						"/usr/sbin/ping"
#define RM							"/usr/bin/rm"
#define RMDIR						"/usr/bin/rmdir"
#define RMNFSEXP					"/usr/sbin/rmnfsexp"
#define STARTSRC 					"/usr/bin/startsrc"
#define UMOUNT						"/usr/sbin/umount"

/*------------------------- SPOT stuff           -----------------------------*/
#define SPOT_OFFSET        	"/../SPOT"
#define SPOT_ENV_FILE			"/SPOT/niminfo"

/*---------------------------- reserved object info --------------------------*/
#define SIMAGES_OBJ_ID				777
#define SIMAGES_LOCATION			NIM_EXPORT"/simages"
#define BOOT_OBJ_ID					888
#define BOOT_LOCATION				"/tftpboot"
#define NIM_SCRIPT_OBJ_ID			999
#define NIM_SCRIPT_LOCATION		NIM_EXPORT"/scripts"

/*---------------------------- macros              ---------------------------*/
#define ERROR(a,b,c,d)				{errstr(a,b,c,d); return(FAILURE);}
#define ODMQUERY						char query[MAX_CRITELEM_LEN];
#define NIM_OBJECT(a,b)				struct nim_object *a; struct listinfo b;
#define NIM_ATTR(a,b)				struct nim_attr *a; struct listinfo b;
#define NIM_PDATTR(a,b)				struct nim_pdattr *a; struct listinfo b;
#define VERBOSE(a,b,c,d,e)			{if(verbose>=1)fprintf(stderr,a,b,c,d,e);}
#define VERBOSE2(a,b,c,d,e)		{if(verbose>=2)fprintf(stderr,a,b,c,d,e);}
#define VERBOSE3(a,b,c,d,e)		{if(verbose>=3)fprintf(stderr,a,b,c,d,e);}
#define VERBOSE4(a,b,c,d,e)		{if(verbose>=4)fprintf(stderr,a,b,c,d,e);}
#define VERBOSE5(a,b,c,d,e)		{if(verbose>=5)fprintf(stderr,a,b,c,d,e);}

/*---------------------------- function return codes -------------------------*/
#define SUCCESS						1
#define OK								2
#define FAILURE						0

/*---------------------------- misc                ---------------------------*/
#define MAX_SEQNO						512	/* safety stop for auto gen of seqno */
													/*		in mk_attr (libmstr) */
#define DEFAULT_TZ					"CST6CDT"
#define NULL_BYTE						'\0'
#define DEFAULT_CHUNK_SIZE			10		/* # of elements to alloc for LIST */

#define ALL								"all"	/* string value for certain attrs */
#define PIF								"if1"	/* primary interface */

#define NIMCLIENT_VAL_OPS_STR "\
\t\tallocate, bos_inst, change, check, cust, deallocate,\n\
\t\tdiag, reset\n"
/* Hard coded string for printing usage msgs for nimclient operations. */

/*---------------------------- char lengths        ---------------------------*/
#define MAX_NAME_BYTES				40
#define MAX_NAME_CHARS				20
#define MAX_VALUE						2048		/* this is used with vchar fields, */
														/*   so it's not as much as it seems*/
														/* set this high in order to store */
														/*   installp package names */
#define MAX_ENV_NAME					80
#define MAX_TMP						80			/* for temporary strings */
#define MAX_HOSTNAME					256		/* for hostnames */
#define MAX_NET_HARD_ADDR			13			/* for network adapter addresses */
#define MAX_SHORT_DIGITS			5			/* for link field in machine_attr */
#define MAX_NIM_ERRSTR				MAX_VALUE	/* for niminfo.errstr */
#define MAX_ADAPTER_NAME			11			/* for adapter logical device names */

/* note that the next 2 constants go together because of the number of */
/*		chars expected in an IP addr */
#define MAX_IP_ADDR					16			/* for IP addresses */
#define HOSTCMD_SCANF_FORMAT		"%*s %*s %15s"

/*----------------------------  locking      ---------------------------*/
#define NO_LOCK						1			/* No lock exists */
#define I_LOCK							2			/* Lock asserted by this process */
#define P_LOCK							3			/* Lock asserted by this processes */
														/*		parent */
#define O_LOCK							4			/* Lock asserted by other process */

#define LOCK_RETRY					5
#define LOCK_RETRY_DELAY			5

#define NIM_GLOCK						NIM_VAR_DIR "/glock"

/*---------------------------- nim_pdarr mask values    ----------------------*/
/* these masks are used in the cmdnim_db.pd.add file */
/* these values are specified as integer because that's the only way odmadd */
/*		will take them */
#define PDATTR_MASK_NOTHING			0x0000		/* no value */
#define PDATTR_MASK_DISPLAY			0x0001		/* show user the value field */
#define PDATTR_MASK_INFO				0x0002		/* customizable information */
#define PDATTR_MASK_SEQNO_REQ			0x0004		/* sequence num required */
#define PDATTR_MASK_ONLY_ONE			0x0008		/* 1 attr of this type allowed*/
#define PDATTR_MASK_VAL_IS_STATE		0x0010		/* value is STATE - xlate val */
																/*		to STATE msg */

/*------------ mask combinations */
#define PDATTR_MASK_USER_ENTERABLE	0x0003
#define PDATTR_MASK_SEQNO				0x0007
#define PDATTR_MASK_UNIQUE				0x000B
#define PDATTR_MASK_STATE				0x0011
#define PDATTR_MASK_SEQNO_UNIQUE		0x000F

/*---------------------------- niminfo stuff        --------------------------*/
/* NIM_ENV_FILE variable names */
/* NOTE that it is important for each to be exported, thus the "export" */
#define NIM_NAME_T					"NIM_NAME"
#define NIM_HOSTNAME_T				"NIM_HOSTNAME"
#define NIM_CONFIGURATION_T		"NIM_CONFIGURATION"
#define NIM_MASTER_HOSTNAME_T		"NIM_MASTER_HOSTNAME"
#define NIM_MASTER_UID_T			"NIM_MASTER_UID"
#define NIM_MASTER_PORT_T			"NIM_MASTER_PORT"
#define NIM_COMM_RETRIES_T			"NIM_COMM_RETRIES"
#define NIM_COMM_DELAY_T			"NIM_COMM_DELAY"
#define NIM_BOOTP_ENABLED_T		"NIM_BOOTP_ENABLED"
#define NIM_USR_SPOT_T				"NIM_USR_SPOT"
#define NIM_BOSINST_DATA_T			"NIM_BOSINST_DATA"
#define NIM_IMAGE_DATA_T			"NIM_IMAGE_DATA"
#define NIM_BOS_IMAGE_T				"NIM_BOS_IMAGE"
#define NIM_BOS_FORMAT_T			"NIM_BOS_FORMAT"
#define NIM_MENU_T					"NIM_MENU"
#define NIM_CUSTOM_T					"NIM_CUSTOM"
#define NIM_MK_DATALESS_T			"NIM_MK_DATALESS"
#define NIM_DTLS_PAGING_SIZE_T	"DTLS_PAGING_SIZE"
#define NIM_DTLS_LOCAL_FS_T		"DTLS_LOCAL_FS"
#define NIM_HOSTS_T					"NIM_HOSTS"
#define NIM_ROUTES_T					"ROUTES"
#define NIM_MKDIR_T					"MKDIR"
#define NIM_MOUNTS_T					"NIM_MOUNTS"
#define NIM_RC_CONFIG_T				"RC_CONFIG"
#define NIM_ROOT_T					"ROOT"
#define NIM_SPOT_T					"SPOT"
#define NIM_DUMP_T					"DUMP"
#define NIM_LOG_T						"NIM_LOG"
#define NIM_BOSINST_ENV_T			"NIM_BOSINST_ENV"
#define NIM_BOSINST_RECOVER_T		"NIM_BOSINST_RECOVER"

#define NIM_NAME						"export "NIM_NAME_T
#define NIM_HOSTNAME					"export "NIM_HOSTNAME_T
#define NIM_CONFIGURATION			"export "NIM_CONFIGURATION_T
#define NIM_MASTER_HOSTNAME		"export "NIM_MASTER_HOSTNAME_T
#define NIM_MASTER_UID				"export "NIM_MASTER_UID_T
#define NIM_MASTER_PORT				"export "NIM_MASTER_PORT_T
#define NIM_COMM_RETRIES			"export "NIM_COMM_RETRIES_T
#define NIM_COMM_DELAY				"export "NIM_COMM_DELAY_T
#define NIM_BOOTP_ENABLED			"export "NIM_BOOTP_ENABLED_T
#define NIM_USR_SPOT					"export "NIM_USR_SPOT_T
#define NIM_BOSINST_DATA			"export "NIM_BOSINST_DATA_T
#define NIM_IMAGE_DATA				"export "NIM_IMAGE_DATA_T
#define NIM_BOS_IMAGE				"export "NIM_BOS_IMAGE_T
#define NIM_BOS_FORMAT				"export "NIM_BOS_FORMAT_T
#define NIM_MENU						"export "NIM_MENU_T
#define NIM_CUSTOM					"export "NIM_CUSTOM_T
#define NIM_MK_DATALESS				"export "NIM_MK_DATALESS_T
#define NIM_DTLS_PAGING_SIZE		"export "NIM_DTLS_PAGING_SIZE_T
#define NIM_DTLS_LOCAL_FS			"export "NIM_DTLS_LOCAL_FS_T
#define NIM_HOSTS						"export "NIM_HOSTS_T
#define NIM_ROUTES					"export "NIM_ROUTES_T
#define NIM_MKDIR						"export "NIM_MKDIR_T
#define NIM_MOUNTS					"export "NIM_MOUNTS_T
#define NIM_RC_CONFIG				"export "NIM_RC_CONFIG_T
#define NIM_ROOT						"export "NIM_ROOT_T
#define NIM_SPOT						"export "NIM_SPOT_T
#define NIM_DUMP						"export "NIM_DUMP_T
#define NIM_LOG						"export "NIM_LOG_T
#define NIM_BOSINST_ENV				"export "NIM_BOSINST_ENV_T
#define NIM_BOSINST_RECOVER		"export "NIM_BOSINST_RECOVER_T

/* error policy choices - used as a bit mask */
#define ERR_POLICY_DISPLAY			0x0001
#define ERR_POLICY_MAIL				0x0002
#define ERR_POLICY_LOG				0x0004
#define ERR_POLICY_NO_EXIT			0x0008
#define ERR_POLICY_WRITE_RC		0x0010

#define ERR_POLICY_FOREGROUND		(ERR_POLICY_DISPLAY|ERR_POLICY_LOG)
#define ERR_POLICY_BACKGROUND		(ERR_POLICY_MAIL|ERR_POLICY_LOG)

/* all NIM programs should fall into one of the following catagories */
#define ERR_POLICY_DAEMON    		(ERR_POLICY_LOG|ERR_POLICY_NO_EXIT)
#define ERR_POLICY_COMMAND			ERR_POLICY_FOREGROUND
#define ERR_POLICY_METHOD \
							(ERR_POLICY_DISPLAY|ERR_POLICY_WRITE_RC|ERR_POLICY_LOG)

#define ERR_RETURN_TOKEN			"rc="
#define ERR_RETURN_TOKEN_FORMAT	"rc=%d"

/*---------------------------- NIM regular expressions -----------------------*/
/* these are referenced in the nimere global array & initialized in nim_init */
/* WARNING - any change here MUST be reflected in the definition of the */
/*		nimere array which is defined in the "cmdnim_cmd.h" file */
/* the ERE_xxx_NUM values define how many matches are expected + 1; it is */
/*		used when defining the size of the pmatch array wherever regexec is used*/
/*		(num matches + 1 because regexec always stores the whole string in */
/*		pmatch[0]) */
#define ERE_BAD_NAME					".*[][ 	`$^&*()=+:,.;?/\\<>|\"\']+.*"
#define ERE_BAD_NAME_NUM			2


/* NOTE that the character '.' is not included in this expression */
/* this is done intentionally in order to allow CDROMs to be defined as */
/*		lpp_sources (ie, location=<CD mntpnt>/usr/sys/inst.images) */
#define ERE_BAD_LOCATION			".*[][ 	`$^&*()=+:,;?\\<>|\"\']+.*"
#define ERE_BAD_LOCATION_NUM		2

#define ERE_IF							"^if[0123456789]*"
#define ERE_IF_NUM					2

#define ERE_IF_STANZA	"^([^ 	]+) ([^ 	]+) ([^ 	]+)( [^ 	]+)*$"
#define ERE_IF_STANZA_NUM			5

#define ERE_HARD_ADDR				"^[abcdefABCDEF0123456789]{12}$|^0{1,12}$"
#define ERE_HARD_ADDR_NUM			2

#define ERE_TWO_FIELDS				"^([^ 	]+) ([^	 ]+)$"
#define ERE_TWO_FIELDS_NUM			3

#define ERE_THREE_FIELDS			"^([^ 	]+) ([^	 ]+) ([^ 	]+)$"
#define ERE_THREE_FIELDS_NUM		4

#define ERE_FOUR_FIELDS				"^([^ 	]+) ([^	 ]+) ([^ 	]+) ([^ 	]+)$"
#define ERE_FOUR_FIELDS_NUM		5

#define ERE_IP_ADDR					"^[0-9]{1,3}.[0-9]{1,3}.[0-9]{1,3}.[0-9]{1,3}$"
#define ERE_IP_ADDR_NUM				2

#define ERE_HOSTNAME					"^[^01234567890].*"
#define ERE_HOSTNAME_NUM			2

#define ERE_FIRST_FIELD				"^([^ 	]+) .*"
#define ERE_FIRST_FIELD_NUM		3

#define ERE_ATTR_ASS					"^([^ 	=]+)=(.*)"
#define ERE_ATTR_ASS_NUM			3

#define ERE_ATTR_NAME				"^([^0123456789 	]+)([0123456789]*)"
#define ERE_ATTR_NAME_NUM			3

#define ERE_SEQNO						"^([^0123456789 	]+)([0123456789]+)"
#define ERE_SEQNO_NUM				3

#define ERE_WARNING_MSG				"^([0123456789]+-[0123456789]+)(.*)"
#define ERE_WARNING_MSG_NUM		3

#define ERE_DEVICE					"^/dev/([^0123456789].*)"
#define ERE_DEVICE_NUM				2

#define ERE_FILENAME					"^(.*)/([^/]+)"
#define ERE_FILENAME_NUM			3

/* transition events are executed in the exec_trans_event function in libmstr
 * they are formatted thus:
 *		field 1 = string name of state attribute (eg, "mstate")
 *		field 2 = current state
 *		field 3 = new state
 *		field 4 = nim_object.name of target
 *		field 5 = command to execute
 *		6->     = args to the command
 *
 *							      1         2          3         4        5      6->
 */
#define ERE_ATTR_TRANS "^([^ 	]+) ([^	 ]+) ([^ 	]+) ([^ 	]+) ([^ 	]+) (.*)"
#define ERE_ATTR_TRANS_NUM			7

#define ERE_RC							"^rc=([0-9]+)"
#define ERE_RC_NUM					2

#define BAD_NAME_ERE					0
#define BAD_LOCATION_ERE			1
#define IF_ERE							2
#define IF_STANZA_ERE				3
#define HARD_ADDR_ERE				4
#define TWO_FIELDS_ERE				5
#define THREE_FIELDS_ERE			6
#define FOUR_FIELDS_ERE				7
#define IP_ADDR_ERE					8
#define HOSTNAME_ERE					9
#define FIRST_FIELD_ERE				10
#define ATTR_ASS_ERE					11
#define ATTR_NAME_ERE				12
#define SEQNO_ERE						13
#define WARNING_MSG_ERE				14
#define DEVICE_ERE					15
#define FILENAME_ERE					16
#define ATTR_TRANS_ERE				17
#define RC_ERE							18
#define MAX_NIM_ERE					19

#endif /* _H_CMDNIM_DEFINES */
