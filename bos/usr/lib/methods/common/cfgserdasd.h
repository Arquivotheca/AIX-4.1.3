/* @(#)17	1.13  src/bos/usr/lib/methods/common/cfgserdasd.h, cfgmethods, bos411, 9428A410j 11/8/93 09:14:41 */
/*
 * COMPONENT_NAME: CFGMETHODS
 *
 * FUNCTIONS: Prototypes for serial DASD config routines
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1990,1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_CFGSERDASD
#define _H_CFGSERDASD
/* Include files required by this header file */

#include <sys/types.h>
#include <sys/scsi.h>
#include <sys/serdasd.h>
#include <stdio.h>

/* Functions defined within each of: */
/* cfgserdasda.c, cfgserdasdc.c, & cfgserdasdd.c */

long generate_minor( char *, long, long * );
int make_special_files( char *, dev_t );
int query_vpd( struct CuDv *, mid_t, dev_t, char * );

/* Functions defined in cfgserdasda.c, and cfgserdasdc.c: */
int download_microcode( char * );
int define_children( char *, int );

/* Functions defined in cfgserdasda.c: */
int exec_daemon(char *,int);
int sd_child_process(char *,int);

/* Functions defined in cfgserdasdd.c: */
int chktype( uchar *, char * );
int get_pvidstr( struct CuDv *, char * );
int disk_present( struct CuDv *, struct PdDv *, struct CuDv *, char *);

/* Functions defined within serdasdtools.c: */
int ucodename( char *, int, int, char * );
int run_scsi_cmd( int, struct sd_iocmd *);
int read_sd_pvid( int, int, char, char *);
int scsi_inq( int, uchar *, int );
void add_desc_fixed( char *, char *, char *, int, int );
int sd_daemon_get_lock(int *);
void sd_daemon_free_lock(int *);
/* Functions defined within dldserdasda.c: */
int get_adap_lvl( int, int *, int *);
int download_adap( int, char *,int);
int download_microcode_adap(char *,int,int);

/* Functions defined within dldserdasdc.c: */
int get_ctrl_lvl( int, int, int *, int * ,int *);
int download_ctrl( int, int, char * ,int, int,int);
int download_microcode_ctrl(int,char *,int,int);

/* Functions defined within serdasd_daemon.c */
void sd_handle_event(void);
void sd_phase_two(char * ,int );
void sd_kill_daemon(void);
void sd_ignore_signal(void);
int sd_unlock_sleep_lock(int ,int *);
int config_dasd(uchar tarlun,char *adap_name,int adap_fd);
int sd_define_then_config(struct PdDv *devpddv,char *parent,int connwhere,char *pvidstr);
int sd_configure(struct PdDv *devpddv,char *name);
void sd_log_daemon_error(uchar ,uchar ,ushort ,uchar , uint ,uchar ,uchar , 
			 uchar ,uchar ,uchar);

/* extern declarations: */
extern	long	*getminor();
extern	long	*genminor();
extern	int	errno;

#define SD_MAX_CTRL_MSG 0x05     /* Number of different controller messages */
#define SD_READ_WITH_RESERVE 0x2  /* allows read extended to work even if    */
				  /* host has the DASD reserved, provided    */
				  /* the controller has the appropriate      */
				  /* microcode 				     */
#define SD_UCODE_FILE_LENGTH    FILENAME_MAX /* length of microcode filena   */
#define SD_DEV_FILE_LENGTH      FILENAME_MAX /* length of special filenam     */
#define SD_MAX_STRING_LEN       256     /* Maximum length of string           */
#define PACK_ID_OFFSET 		198     /* Offset into microcode package where*/
					/* the package identification number  */
					/*   resides			      */

#define DAEMON_NAME 		"d"     /* prefix for daemon's special file   */
					/* for the adapter */
#define SD_CARDID		"8f78"  /* POS1 the POS0, which make up the   */
					/* cardid portion of the microcode    */
					/* filename 			      */

#define GETOBJ( CLASS, SSTRING, OBJADDR, SEARCHFLAG, NOTFOUND ) {	\
		int rc;							\
		DEBUG_3(						\
		"GETOBJ: SSTRING='%s', OBJADDR = '%s', Address=%ld\n ",	\
				SSTRING, #OBJADDR, (long)OBJADDR )	\
		rc=(int)odm_get_obj(CLASS,SSTRING,OBJADDR,SEARCHFLAG);	\
		if( rc == 0 ) {						\
			DEBUG_2("GETOBJ: Object '%s' not found in %s\n",\
				SSTRING, #CLASS )			\
			return NOTFOUND;				\
		}							\
		else if( rc == -1 ) {					\
			DEBUG_0("ODM ERROR\n")				\
			return E_ODMGET;				\
		}							\
	}

#define ODMOPEN( HANDLE, CLASS ) {					\
		if( ( int )( HANDLE = odm_open_class( CLASS ) ) == -1 ){\
			DEBUG_1("Error opening %s\n", #CLASS )		\
			return E_ODMOPEN;				\
		}							\
	}	

#define ODMCLOSE( CLASS ) {						\
		if( odm_close_class( CLASS ) == -1 )			\
			return E_ODMCLOSE;				\
	}

#define GETATT( DEST, TYPE, CUSOBJ, ATTR, NEWATTRS ) {			\
		int rc;							\
		rc = getatt( DEST, TYPE, CuAt_CLASS, PdAt_CLASS,	\
			CUSOBJ.name, CUSOBJ.PdDvLn_Lvalue, ATTR,	\
			(struct attr *)NEWATTRS );			\
		if( rc > 0 )						\
			return rc;					\
	}

#define SETATT( NAME, ATTR, NEWVALUE ) {				\
		struct	CuAt	*cusatt;                                \
		int	how_many, rc;	 				\
		DEBUG_3("setattr(%s,%s,%s)\n",NAME,ATTR,NEWVALUE);	\
		if( ( cusatt = getattr( NAME, ATTR, 0, &how_many ))	\
			== (struct CuAt *)NULL ) {			\
			DEBUG_0("ERROR: getattr() failed\n")		\
			return E_NOATTR;				\
		}							\
		strcpy( cusatt->value, NEWVALUE );			\
		if( putattr( cusatt ) == -1 )				\
			return E_ODMADD;				\
	}

#endif /* _H_CFGSERDASD */





