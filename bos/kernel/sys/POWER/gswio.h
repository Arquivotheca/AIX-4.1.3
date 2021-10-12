/* @(#)74	1.4.1.1  12/15/93 19:08:16 */

/*
 * COMPONENT_NAME: (SYSXPSLA) IBM MSLA-PSLA ioctl Header file
 *
 * FUNCTIONS: NONE
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_GSWIO
#define _H_GSWIO

#include <sys/types.h>
#include <sys/devinfo.h>

#define gIOC            ('W'<<8)
#define G_SET_MEM       (gIOC|10)       /* set current memory area id   */
#define G_ALARM         (gIOC|11)       /* sound audible alarm          */
#define G_STOP          (gIOC|12)       /* stop buffer execution        */
#define G_START         (gIOC|13)       /* start buffer execution       */
#define G_SET_BF        (gIOC|14)       /* set buffer address           */
#define G_SBF_START     (gIOC|15)       /* set buffer address and start */
#define G_SET_CUR       (gIOC|16)       /* set cursor address           */
#define G_RESET_CUR     (gIOC|17)       /* reset cursor address         */
#define G_SET_IND       (gIOC|18)       /* set indicators               */
#define G_SET_IND_ON    (gIOC|19)       /* set indicators on            */
#define G_SET_IND_OFF   (gIOC|20)       /* set indicators on            */
#define G_WRITESF       (gIOC|21)       /* write structured field       */
#define G_DEF_MEM       (gIOC|22)       /* define memory area           */
#define G_DEL_MEM       (gIOC|23)       /* delete memory area           */
#define G_REN_MEM       (gIOC|24)       /* rename memory area           */
#define G_LOAD_BLNK     (gIOC|25)       /* load blinking patterns       */
#define G_LOAD_LINE     (gIOC|26)       /* load blinking patterns       */
#define G_READ_CUR      (gIOC|27)       /* read cursor                  */
#define G_SENSE         (gIOC|28)       /* get sense                    */
#define G_LOAD_MEM_AREA (gIOC|29)       /* build a load memory area sf  */
#define G_IOCTL         (gIOC|31)       /* used for io in ioctl         */
/* values 32 - 45 are reserved */
#define K_ENABLE        (gIOC|65)       /* enable input device          */
#define K_DISABLE       (gIOC|66)       /* disable input device         */
#define K_ENA_SIG       (gIOC|67)       /* enable signal                */
#define K_DIS_SIG       (gIOC|68)       /* disable signal               */
#define K_POLL          (gIOC|69)       /* poll input q                 */
#define K_WAIT          (gIOC|70)       /* wait on input q              */
#define K_FLUSH         (gIOC|71)       /* flush input q                */
#define K_REQUEST       (gIOC|72)       /* request mode operation       */
#define K_STOP_DEVICE   (gIOC|73)       /* issue a detach device        */
#define K_LCW           (gIOC|74)       /* link command words           */
#define FPGI_CONNECT    (gIOC|75)
#define FPGI_DISCONNECT (gIOC|76)
#define FPGI_INFREE     (gIOC|77)
#define FPGI_SEND       (gIOC|78)
#define FPGI_WAIT       (gIOC|79)
#define K_STOP_ALL_DEVS (gIOC|80)       /* hydra mode - stop all devices*/

#define GREQUEST   1
#define GSAMPLE    2
#define GEVENT     3
#define GENABLE    1
#define GDISABLE   0
#define G_READ     0x01
#define G_WRITE    0x02
#define SEN_MAX    24           /* 24 bytes of sense data            */

/*-------------------------------------------------------------------*/
/* g_type  - used as an index for accessing q_hdr entries, since     */
/*           each type has a separate q_hdr.                         */
/* input_type - analogous to 'g_type', but uses 'bit shifts' to      */
/*           match the type. For example, Gank is g_type 2 and       */
/*           GANK is 2nd rightmost bit.                              */
/*           'input_type' is used in the G_ENABLE IOCTL call to      */
/*           permit the user to specify any combination of input     */
/*           types with a single ioctl call, since each type uniquely*/
/*           matches a bit in a 32-bit field.                        */
/*                                                                   */
/*-------------------------------------------------------------------*/


enum g_type {Gpick,Gank,Gpfk,Glpfk,Gtablet,Gsmi,Ggeop,Gpgm_err,
		Kno_5080, Klink_sw, Knot_to_ready, fpgi_nobuf,
		fpgi_rdcomplete, fpgi_wtcomplete, fpgi_progerr, fpgi_sioerr,
                fp_noinbuf, fp_wtmax };
enum input_type { GPICK    = 0x0001, GANK     = 0x0002, GPFK  = 0x0004,
		  GLPFK    = 0x0008, GTABLET  = 0x0010, GSMI  = 0x0020,
		  GGEOP    = 0x0040, GPGM_ERR = 0x0080, GALL  = 0x00ff,
		  GNULL    = 0x0000,
		  KNO_5080         = 0x0100,  KLINK_SW        = 0x0200,
		  KNOT_TO_READY    = 0x0400,  FPGI_NOBUF      = 0x0800,
		  FPGI_RD_COMPLETE = 0x1000,  FPGI_WTCOMPLETE = 0x2000,
		  FPGI_PROGERR     = 0x4000,  FPGI_SIOERR     = 0X8000,
		  FP_NOINBUF       = 0x10000, FP_WTMAX        = 0x20000 };


/*----------------------------------------------------------------------*/
/* q_qel   interrupt queue element                                      */
/*                                                                      */
/* Interrupt information passed to user consists of a q_qel followed    */
/* by any associated data. The q_qel describes the interrupt type       */
/* and the status of the data appended to it.                           */
/*                                                                      */
/*----------------------------------------------------------------------*/

typedef struct qqel {                   /* queue element                */
    enum input_type type;               /* enum input_type value        */
    int time;                           /* timestamp                    */
    struct {
	unsigned bad_data       :1;     /* GEOP/SMI extra data io failed*/
	unsigned no_data        :1;     /* queue empty (for K_POLL)     */
	unsigned reserved       :6;     /* reserved                     */
	char rsvd[3];                   /* reserved                     */
    } flags;
    int data_len;                       /* length of appended data      */
} q_qel;

struct k_enable {                       /* K_ENABLE IOCTL call          */
    enum input_type type;               /* types to enable              */
    int   qualifier;                    /* qualifier for this type      */
    char *buf_adr;                      /* 5080 bfr adr to read         */
    int   data_len;                     /* len of 5080 bfr adr to read  */
    char *w_adr;                        /* for GEOP/SMI- 5080 write adr */
};                                      /*      or restart. See oemi doc*/

struct k_disable {                      /* K_DISABLE IOCTL call         */
    enum input_type type;               /* types to disble              */
    int  qualifier;                     /* for GEOP/SMI disables        */
};

struct k_request {                      /* K_REQUEST IOCTL call         */
    enum input_type type;               /* type to enable (no multiples)*/
    int   qualifier;                    /* qualifier for this type      */
    char *buf_adr;                      /* 5080 bfr adr to read         */
    int   data_len;                     /* len of 5080 bfr adr to read  */
    char *w_adr;                        /* for GEOP/SMI- 5080 write adr */
					/*      or restart. See oemi doc*/
    char *target_adr;                   /* usr bfr adr to return the qel*/
};
struct g_writesf {                      /* G_WRITESF IOCTL call         */
	char *buf_adr;                  /* usr buffer to send to 508x   */
	int   data_len;                 /* usr buffer length            */
	unsigned async_io : 1;          /* use async_io                 */
	unsigned rsvd     : 31;         /* reserved                     */
};

struct g_def_mem {                      /* G_DEF_MEM IOCTL call (define)*/
	ushort memid;                   /* 508x memory area id          */
	ushort type;                    /* 508x memory area type        */
	ushort size;                    /* 508x memory area size        */
};

struct g_ren_mem {                      /* G_REN_MEM IOCTL call (rename)*/
	ushort oldid;                   /* 508x old memory area id      */
	ushort newid;                   /* 508x new memory area id      */
	ushort newtype;                 /* 508x new memory area type    */
};
					/* load blinking patterns       */
struct g_ld_blnk {                      /* G_LD_BLNK IOCTL call         */
	short pattern;                  /* starting blnk pattern #      */
	short number;                   /* # of blnk patterns to load   */
	char *buf_adr;                  /* usr bfr adr of blink patterns*/
};
					/* load line patterns           */
struct g_ld_line {                      /* G_LD_LINE IOCTL call         */
	short pattern;                  /* starting line pattern #      */
	short number;                   /* # of line patterns to load   */
	char *buf_adr;                  /* usr bfr adr of line patterns */
};
					/* linked command words(chained)*/
struct k_lcw {                          /* K_LCW IOCTL call             */
	char *adr;                      /* usr bfr adr of lcws          */
	unsigned async_io : 1;          /* use async io                 */
	unsigned rsvd     :31;          /* reserved                     */
};

/*----------------------------------------------------------------------*/
/* structure for 'ext' parm on read-write                               */
/*----------------------------------------------------------------------*/
struct rwparms {
    unsigned rsvd     : 4;              /* reserved                     */
    unsigned async_io : 1;              /* use async io                 */
    unsigned rsverd   : 1;              /* reserved                     */
    unsigned start    : 1;              /* restart 508x bfr after r/w   */
    unsigned stop     : 1;              /* stop 508x bfr before r/w     */
    char rvd[3];                        /* reserved                     */
    union {
	struct {
		ushort page;
		ushort offset;
	} buf;
	ulong dlb_adr;                  /* 508x bfr adr for r/w or      */
    } adr;                              /*    if -1, continue from last */
};                                      /*    r/w 508x bfr adr.         */

union ctlparms {                        /* structs for IOCTL calls      */
	struct k_enable k_ena;
	struct k_disable k_dis;
	struct k_request k_req;
	struct g_writesf g_wsf;
	struct g_ld_blnk g_blnk;
	struct g_ld_line g_line;
	struct k_lcw k_lcw;
	char data[sizeof(struct devinfo)];
};

struct opnparms {                       /* struct for 'ext' on OPENX    */
    unsigned rsvd        : 2;           /* reserved. Set to 0.          */
    unsigned diag_mode   : 1;           /* used with diagnostics only   */
    unsigned mode        : 1;           /* reserved. Set to 0.          */
    unsigned change_adrs : 1;           /* reserved. Set to 0.          */
    unsigned start_msla  : 1;           /* used with 'start_msla' only  */
    unsigned stop_msla   : 1;           /* used with 'stop_msla'  only  */
    unsigned link_sw     : 1;           /* capture link switch intrpts  */
    char devmode;                       /* device mode.                 */
    char rvd[2];                        /* reserved. Set to 0.          */
    int signal;                         /* signal for intrpt q non-empty*/
    char *u_err_area;                   /* usr bfr adr for error data   */
};

union rw_args {                         /* 'ext' field on READX/WRITEX  */
    int value;
    struct rwparms *valuep;
};

union ctl_args {                        /* 'arg' field on IOCTL         */
    int value;
    union ctlparms *valuep;
};



typedef struct lcw_ccw                  /* struct for K_LCW of IOCTL    */
{   unsigned cmdcode:8;                 /* command code                 */
    char     flags;                     /* flag field                   */
    ushort   count;                     /* byte count                   */
    unsigned dataptr:32;                /* data address                 */
}ccw_t;

					/* err log into returned in     */
struct log_info {                       /* u_err_area on OPENX call     */
	char class;                     /* driver class    (see errpt)  */
	char subclass;                  /* driver subclass (see errpt)  */
	char mask;                      /* driver mask     (see errpt)  */
	char type;                      /* driver type     (see errpt)  */
	int  len;                       /* error log info length        */
	int  err_fn;                    /* driver function in error     */
	int  err_reason;                /* reason for error             */
	int  err_typ;                   /* error type                   */
	char data[SEN_MAX];             /* error data (e.g. sense)      */
	int  err_dev;                   /* minor device                 */
};

#endif /* _H_GSWIO  */
