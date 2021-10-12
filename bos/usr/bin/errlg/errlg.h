/* @(#)47        1.25  src/bos/usr/bin/errlg/errlg.h, cmderrlg, bos41J, 9507C 2/2/95 18:05:52 */ 

/*
 * COMPONENT_NAME: CMDERRLG   system error logging and reporting facility
 *
 * FUNCTIONS: common header file for CMDERRLG
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#define _SSGENVAR  TRUE

#include <sys/types.h>
#include <ras.h>
#include <cmderrlg_msg.h>
#include <sys/err_rec.h>
#include <sys/erec.h>
#include <sys/limits.h>
#include <sys/errids.h>

extern char *codeptstr();	/* return codepoint string */
extern char *Codeptfile;	/* pathname of codepoint file */

/*
 * Error logging resource name for error log entries we cut.
 */
#define ERRDEMON_RESOURCE_NAME "errdemon"

/*
 * Full pathnames of errlog data files
 */
#define ERRORFILE_DFLT  "/dev/error"
#define DMPFMT          "/usr/lib/ras/dmprtns/dmpfmt"

#define RASDIR			"/var/adm/ras"
#define ERRLG_DIR		"/var/adm/ras"
#define ERRDESC_DFLT	"/usr/lib/ras/errids.desc"
#define ERRTMPLT_DFLT	"/var/adm/ras/errtmplt"

#define ERRLOG_DFLT		"/var/adm/ras/errlog"
#define ERRLOG_OLD		"/var/adm/ras/errlog.old"

/*
 * Default values for errlog memory buffer size and log file size
 */
#define BUFFER_MINIMUM	8192
#define BUFFER_DFLT	8192
#define THRESHOLD_DFLT 	1048576

#define ODM_DATABASE	"/etc/objrepos"
/*
 * Basenames of errlog data files
 */
#define ERRIDS_DESC     "errids.desc"
#define CODEPOINT_DESC  "codepoint.desc"
#define CODEPOINT_CAT   "codepoint.cat"
#define CODEPOINT_BAK	"/var/adm/ras/codepoint.cat"

extern char *CuDv_name;
extern char *CuDv_rclass;
extern char *CuVPD_vpd;
extern char *CuDv_in;
extern char *CuDv_connwhere;
extern char *CuDv_location;

#define MCS_CATALOG "cmderrlg.cat"

extern char Logfile[PATH_MAX];

/*
 * old errpt.h is included here because now errdead is separate from errpt.
 */

extern char *lst_lookup();

extern int templateflg;
extern int quietflg;
extern int detailedflg;
extern int rawflg;
extern int concurrentflg;
extern int Starttime;
extern int Endtime;

extern char *Outfile;
extern char *Infile;

#define NDETAILS 10

/*
 * definitions for use with errpt raw mode.
 */
#define LE_LABEL_MAX        20
#define LE_MACHINE_ID_MAX   32
#define LE_NODE_ID_MAX      32
#define LE_CLASS_MAX        2
#define LE_TYPE_MAX         5
#define LE_RESOURCE_MAX     16
#define LE_RCLASS_MAX       16
#define LE_RTYPE_MAX        16
#define LE_VPD_MAX          512
#define LE_IN_MAX           20
#define LE_CONN_MAX         20
#define LE_DETAIL_MAX       230
#define LE_SYMPTOM_MAX      SSREC_MAX

struct obj_errlog {
        unsigned int   el_sequence;
        char           el_label[LE_LABEL_MAX];
        unsigned int   el_timestamp;
        unsigned int   el_crcid;
        char           el_machineid[LE_MACHINE_ID_MAX];
        char           el_nodeid[LE_NODE_ID_MAX];
        char           el_class[LE_CLASS_MAX];
        char           el_type[LE_TYPE_MAX];
        char           el_resource[LE_RESOURCE_MAX];
        char           el_rclass[LE_RCLASS_MAX];
        char           el_rtype[LE_RTYPE_MAX];
        char           el_vpd_ibm[LE_VPD_MAX];
        char           el_vpd_user[LE_VPD_MAX];
        char           el_in[LE_IN_MAX];
        char           el_connwhere[LE_CONN_MAX];
        unsigned int   el_detail_length;
        char           el_detail_data[LE_DETAIL_MAX];
        unsigned int   el_symptom_length;
        char           el_symptom_data[LE_SYMPTOM_MAX];
};

extern struct obj_errlog T_errlog;

struct obj_errtmplt {
        unsigned int   et_crcid;
        char           et_label[LE_LABEL_MAX];
        char           et_class[LE_CLASS_MAX];
        char           et_type[LE_TYPE_MAX];
        unsigned short et_desc;
        unsigned short et_probcauses[4];
        unsigned short et_usercauses[4];
        unsigned short et_useraction[4];
        unsigned short et_instcauses[4];
        unsigned short et_instaction[4];
        unsigned short et_failcauses[4];
        unsigned short et_failaction[4];
        unsigned short et_detail_length[8];
        unsigned short et_detail_descid[8];
        unsigned short et_detail_encode[8];
        unsigned short et_logflg;
        unsigned short et_alertflg;
        unsigned short et_reportflg;
};

extern struct obj_errtmplt T_errtmplt;

/*
 * End of old errpt.h stuff.
 */

/*
 * This is how code point data is passed in the le_symptom_data
 * field in the error log.
 */
struct sscodept {
        int codept;
        char data[1];
};

 /*
  * error log data structures and definitions from old raslib/log.c.
  */

struct log_hdr {
	char lh_magic[8];		/* "jerrlogj" */
	int  lh_entries;		/* number of entries in the error log file */
	int  lh_inoff;			/* offset to where next input will go */
	int  lh_outoff;			/* offset to next entry to read from */
	int  lh_maxsize;		/* size to allow log file to wrap at */
	int  lh_staleoff;		/* offset to stale data at end of file */
	int  lh_sequence;		/* current sequence number available */
};

struct log_entry0 {
	int le0_magic;			/* magic number to help identify */
	int le0_length;			/* length of variable-length log entry */
	int le0_sequence;		/* sequence number for this entry */
	int le0_datalength;		/* round to 16 bytes */
};

struct log_entry {
	unsigned int le_length;
	unsigned int le_magic;
	int			 le_sequence;
	time_t		 le_timestamp;
	unsigned int le_crcid;
	unsigned int le_mach_length;
	char*		 le_machine_id;		/* machine id from uname() */
	unsigned int le_node_length;
	char*		 le_node_id;		/* node id from uname() */
	unsigned int le_resource_length;
	char*        le_resource;
	unsigned int le_vpd_ibm_length;
	char*        le_vpd_ibm;
	unsigned int le_vpd_user_length;
	char*        le_vpd_user;
	unsigned int le_in_length;
	char*        le_in;
	unsigned int le_connwhere_length;
	char*        le_connwhere;
	unsigned int le_rclass_length;
	char*        le_rclass;
	unsigned int le_rtype_length;
	char*        le_rtype;
	unsigned int le_detail_length;
	char*        le_detail_data;
	unsigned int le_symptom_length;
	char*	     le_symptom_data;
	unsigned int le_length2;				/* for validation, and reverse processing. */
};

extern struct log_entry log_entry;

#define LE_BASE_SIZE sizeof(struct log_entry)
		/* number of dynamic members in log_entry */
#define LE_NUM_DYNAMIC 11

#define	LE_MIN_SIZE	LE_BASE_SIZE - sizeof(void *)*(LE_NUM_DYNAMIC-1)

				/* note there are two LE_VPD_MAX in LE_MAX_SIZE */
#define LE_MAX_SIZE	LE_BASE_SIZE + LE_MACHINE_ID_MAX + LE_NODE_ID_MAX +\
		LE_RESOURCE_MAX  + LE_RCLASS_MAX  + LE_RTYPE_MAX + 2*LE_VPD_MAX +\
		LE_IN_MAX + LE_CONN_MAX + LE_DETAIL_MAX + LE_SYMPTOM_MAX - LE_NUM_DYNAMIC*sizeof(void *)

#define LE_DYNAMIC_SIZES     le.le_mach_length + le.le_node_length +\
		le.le_resource_length  + le.le_rclass_length + \
		le.le_rtype_length + le.le_vpd_ibm_length   + le.le_vpd_user_length + \
		le.le_in_length    + le.le_connwhere_length + le.le_detail_length + le.le_symptom_length

#define LH_MAXSIZE 1*1024*1024		/* 1 meg error log max */
#define LH_MINSIZE 4096				/* 1 block of disk on RS6000 */

#define LH_MAGIC   "aerrlogr"
#define LE_MAGIC   0x0C3DF420

/*
 * End of old raslib/log.c stuff.
 */

/*
 * From errdemon.h
 */

extern char *Errorfile;
extern Errorfd;

/*
 * End of old errdemon.h stuff.
 */

/*
 * error record structure used by errdemon and errdead.
 */
struct buf_entry {
	struct erec0    erec;
	struct err_rec0 err_rec;
	char   detail_data[ERR_REC_MAX];
};
#define BUF_ENTRY_MAX sizeof(struct buf_entry)
#define BUF_ENTRY_MIN sizeof(struct erec0) + sizeof(struct err_rec0)

/*
 *	structure to hold info for a bad log entry.
 */
struct le_bad {
	int	code;		/* code for which member of log entry is bad */
	int	value;		/* value in log entry that is bad */
	int	value2;		/* possible length2 from log entry. */
	int	offset;		/* offset into log file for bad log entry */
};


/*
 * ODM error logging definitions.
 */

#define ERRLG_FILE	"errlg_file"
#define ERRLG_SIZE	"errlg_size"
#define ERRLG_BUF	"errlg_buf "

#define ATTR_SIZE	sizeof(ERRLG_FILE) - 1		/* no NULL */

#define ODM_INIT	"odm_initialize"
#define ODM_READ	"ras_getattr   "
#define ODM_WRITE	"ras_putattr   "
#define ODM_LOCK	"odm_lock      "
#define ODM_UNLOCK	"odm_unlock    "
#define ODM_TERM	"odm_terminate "

#define OPSTR_SIZE	sizeof(ODM_INIT)

/*
 * NVRAM error logging definitions.
 */

#define NVOPEN			"NVOPEN                  "
#define NVWRITE			"NVWRITE                 "
#define NVCLOSE			"NVCLOSE                 "
#define CHECKSTOP_COUNT		"CHECKSTOP_COUNT         "
#define CHECKSTOP_PTR		"CHECKSTOP_PTR           "
#define CHECKSTOP_DATA		"CHECKSTOP_DATA          "
#define MACHINECHECK_DATA	"MACHINECHECK_DATA       "
#define IPL_DIR             "IPL_DIRECTORY           "  
#define SYS_INFO            "SYSTEM_INFO             "
#define BUMPTBL_HDR         "BUMP_TABLE_HEADER       "
#define BUMPTBL_ERRMSGS     "BUMP_TABLE_ERRMSGS      " 

#define STR_ENOMEM	"ENOMEM \n"
#define STR_EFAULT	"EFAULT \n"
#define STR_EINVAL	"EINVAL \n"
#define STR_EIO		"EIO    \n"
#define STR_UNKNOWN	"UNKNOWN\n"

/* 
 * Hold newline plus 2 nv i/o error strings 
 * It is possible to have a read or write and a close.
 */
#define NV_IO_DETAIL_SIZE 1+2*(sizeof(MACHINECHECK_DATA)+sizeof(STR_UNKNOWN))

struct nv_err {
	struct erec0 erec;
	struct err_rec0 err_rec;
	char   detail_data[NV_IO_DETAIL_SIZE];
};

#define LE_BAD_LEN1	1
#define LE_BAD_LEN2	2
#define LE_BAD_MAGIC	3
#define LE_BAD_SEQ	4
#define LE_BAD_MACH_ID  6
#define LE_BAD_NODE_ID  7        
