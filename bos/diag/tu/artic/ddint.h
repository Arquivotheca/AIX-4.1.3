/* @(#)97       1.1  src/bos/diag/tu/artic/ddint.h, tu_artic, bos411, 9428A410j 8/19/93 17:27:26 */
/*
 * COMPONENT_NAME: 
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * FUNCTIONS:  ARTIC diagnostic device driver library interface
 *
*/


#include <sys/ioctl.h>
#include <sys/types.h>

/* IOCTL Command code definitions */
#define ICARESET                   _IOWR('x',0x00,ICARESET_PARMS)
#define ICAREADMEM                 _IOWR('x',0x01,ICAREADMEM_PARMS)
#define ICAWRITEMEM                _IOWR('x',0x02,ICAWRITEMEM_PARMS)
#define ICAINTREG                  _IOWR('x',0x03,ICAINTREG_PARMS)
#define ICAINTWAIT                 _IOWR('x',0x04,ICAINTWAIT_PARMS)
#define ICAINTDEREG                _IOWR('x',0x05,ICAINTDEREG_PARMS)
#define ICAISSUECMD                _IOWR('x',0x06,ICAISSUECMD_PARMS)
#define ICAGETPARMS                _IOWR('x',0x08,ICAGETPARMS_PARMS)
#define ICAGETBUFADDRS             _IOWR('x',0x09,ICAGETBUFADDRS_PARMS)
#define ICAGETPRIMSTAT             _IOWR('x',0x0A,ICAGETPRIMSTAT_PARMS)
#define ICAIOWRITE                 _IOWR('x',0x0F,ICAIOWRITE_PARMS)
#define ICAIOREAD                  _IOWR('x',0x10,ICAIOREAD_PARMS)
#define ICAPOSWRITE                _IOWR('x',0xF6,ICAPOSWRITE_PARMS)
#define ICAPOSREAD                 _IOWR('x',0xF7,ICAPOSREAD_PARMS)
#define ICADMASETUP                _IOWR('x',0xF4,ICADMASETUP_PARMS)
#define ICADMAREL                  _IOWR('x',0xF5,ICADMAREL_PARMS)
#define ICASENDCFG                 _IOWR('x',0xF0,ICAGETPARMS_PARMS)
#define ICAGETADAPTYPE             _IOWR('x',0xF2,ICAGETADAPTYPE_PARMS)



/* Error codes returned by device driver and C Library routines */
#define         NO_ERROR                                0x0000
#define         E_ICA_INVALID_COPROC                    0xFF05
#define         E_ICA_INVALID_TASK_STATUS               0xFF06
#define         E_ICA_INVALID_PAGE                      0xFF07
#define         E_ICA_INVALID_OFFSET                    0xFF08
#define         E_ICA_INVALID_FORMAT                    0xFF09
#define         E_ICA_TIMEOUT                           0xFF0B
#define         E_ICA_INVALID_CONTROL                   0xFF0D
#define         E_ICA_BAD_PCSELECT                      0xFF11
#define         E_ICA_CMD_REJECTED                      0xFF12
#define         E_ICA_NO_CMD_RESPONSE                   0xFF13
#define         E_ICA_OB_SIZE                           0xFF14
#define         E_ICA_ALREADY_REG                       0xFF15
#define         E_ICA_NOT_REG                           0xFF17
#define         E_ICA_INVALID_FD                        0xFF23
#define         E_ICA_XMALLOC_FAIL                      0xFF25
#define         E_ICA_ALREADY_OPEN                      0xFF26
#define         E_ICA_INTR                              0xFF28
#define         E_ICA_DMASETUP                          0xFF29
#define         E_ICA_DMAREL                            0xFF2A
#define         E_ICA_DMAPARMS                          0xFF2B
#define         E_ICA_NOMEM                             0xFF2C
#define         E_ICA_INVALID_POSREG                    0xFF2D

/* This structure contains configuration information about */
/* information about a specific adapter                    */
typedef struct {
           ushort io_addr;   /* Base address of adapter's I/O ports */
           uchar maxtask;    /* Max task number for adapter         */
           uchar maxpri;     /* Number of task priorities           */
           uchar maxqueue;   /* Max number of queues for adapter    */
           uchar maxtime;    /* Max number of timers for adapter    */
           uchar int_level;  /* Adapters interrupt level            */
           uchar ssw_size;   /* Size of the shared storage window   */
         } ICAPARMS;


/* This structure contains the length and address of an */
/* input, output or secondary status buffer.            */
typedef struct {
           ushort length;    /* Length of buffer           */
           ushort offset;    /* Offset of buffer's address */
           uchar  page;      /* Page of buffer's address   */
        } ICABUFFER;


/* Structures used for IOCTL input and output parameters */
typedef struct
{
   uchar reserved1;         /* Reserved field, must be 0   */
   ushort retcode;          /* Return Code                 */
   ulong reserved;          /* Reserved field, must be 0   */
} ICARESET_PARMS;

typedef struct
{
   uchar reserved1;         /* Reserved field, must be 0   */
   ulong length;            /* Length                      */
   ushort segpage;          /* Segment/Page                */
   ushort offset;           /* Offset                      */
   uchar *dest;             /* Destination buffer pointer  */
   uchar addr_format;       /* Address format              */
   ushort retcode;          /* Return Code                 */
   ulong reserved;          /* Reserved field, must be 0   */
} ICAREADMEM_PARMS;

typedef struct
{
   uchar reserved1;         /* Reserved field, must be 0   */
   ulong length;            /* Length                      */
   ushort segpage;          /* Segment/Page                */
   ushort offset;           /* Offset                      */
   uchar *dest;             /* Destination buffer pointer  */
   uchar addr_format;       /* Address format              */
   ushort retcode;          /* Return Code                 */
   ulong reserved;          /* Reserved field, must be 0   */
} ICAWRITEMEM_PARMS;

typedef struct
{
   uchar reserved1;         /* Reserved field, must be 0   */
   uchar reserved2;         /* Reserved field, must be 0   */
   ushort retcode;          /* Return Code                 */
   ulong reserved;          /* Reserved field, must be 0   */
} ICAINTREG_PARMS;

typedef struct
{
   uchar reserved1;         /* Reserved field, must be 0   */
   uchar reserved2;         /* Reserved field, must be 0   */
   uint timeout;            /* Timeout                     */
   ushort retcode;          /* Return Code                 */
   ulong reserved;          /* Reserved field, must be 0   */
} ICAINTWAIT_PARMS;

typedef struct
{
   uchar reserved1;         /* Reserved field, must be 0   */
   uchar reserved2;         /* Reserved field, must be 0   */
   ushort retcode;          /* Return Code                 */
   ulong reserved;          /* Reserved field, must be 0   */
} ICAINTDEREG_PARMS;



typedef struct
{
   uchar reserved1;         /* Reserved field, must be 0   */
   uchar reserved2;         /* Reserved field, must be 0   */
   uchar cmdcode;           /* Command Code                */
   ushort length;           /* Length of parameter buffer  */
   uint timeout;            /* Timeout                     */
   uchar *prms;             /* Pointer to parameters       */
   ushort retcode;          /* Return Code                 */
   ulong reserved;          /* Reserved field, must be 0   */
} ICAISSUECMD_PARMS;

typedef struct
{
   uchar reserved1;         /* Reserved field, must be 0   */
   ICAPARMS cfgparms;       /* Configuration parameters    */
   ushort retcode;          /* Return Code                 */
   ulong reserved;          /* Reserved field, must be 0   */
} ICAGETPARMS_PARMS;

typedef struct
{
   uchar reserved1;         /* Reserved field, must be 0   */
   uchar reserved2;         /* Reserved field, must be 0   */
   ICABUFFER ib;            /* Input buffer info           */
   ICABUFFER ob;            /* Output buffer info          */
   ICABUFFER ssb;           /* SS buffer info              */
   ushort retcode;          /* Return Code                 */
   ulong reserved;          /* Reserved field, must be 0   */
} ICAGETBUFADDRS_PARMS;

typedef struct
{
   uchar reserved1;         /* Reserved field, must be 0   */
   uchar reserved2;         /* Reserved field, must be 0   */
   uchar psb;               /* Primary status byte         */
   ushort retcode;          /* Return Code                 */
   ulong reserved;          /* Reserved field, must be 0   */
} ICAGETPRIMSTAT_PARMS;

/*      structure for ICAIOWRITE ioctl  */

typedef struct
{
        uchar   reserved1;              /* reserved field                */
        uchar   portnum;                /* I/O port offset              */
        uchar   value;                  /* value to write to port       */
        ushort  retcode;                /* return code                  */
        uint    reserved;               /* reserved field               */
} ICAIOWRITE_PARMS;

/*      structure for ICAIOREAD ioctl   */

typedef struct
{
        uchar   reserved1;              /* reserved field                */
        uchar   portnum;                /* I/O port offset              */
        uchar   value;                  /* returned value               */
        ushort  retcode;                /* return code                  */
        uint    reserved;               /* reserved field               */
} ICAIOREAD_PARMS;

typedef struct
{
        uchar   reserved1;              /* reserved field                */
        caddr_t uaddr;                  /* User space buffer             */
        uint    count;                  /* Number of bytes being xferred */
        ulong   physaddr;               /* DMA Xfer physical address     */
        int     type;                   /* Type of DMA transfer          */
        ushort  retcode;                /* return code                   */
        uint    reserved;               /* reserved field                */
} ICADMASETUP_PARMS;

typedef struct
{
        uchar   reserved1;              /* reserved field                */
        ushort  retcode;                /* return code                   */
        uint    reserved;               /* reserved field                */
} ICADMAREL_PARMS;

typedef struct
{
        uchar   reserved1;              /* reserved field                */
        uint    type;                   /* adapter type                  */
        ushort  retcode;                /* return code                   */
        uint    reserved;               /* reserved field                */
} ICAGETADAPTYPE_PARMS;

typedef struct
{
        uchar   reserved1;              /* reserved field                */
        uchar   regnum;                 /* POS register number          */
        uchar   value;                  /* returned value               */
        ushort  retcode;                /* return code                  */
        uint    reserved;               /* reserved field               */
} ICAPOSREAD_PARMS;

typedef struct
{
        uchar   reserved1;              /* reserved field                */
        uchar   regnum;                 /* POS regsiter number          */
        uchar   value;                  /* value to write to register   */
        ushort  retcode;                /* return code                  */
        uint    reserved;               /* reserved field               */
} ICAPOSWRITE_PARMS;

/* Union of all IOCTL parameter structures */

typedef union
{
ICARESET_PARMS       icareset;
ICAREADMEM_PARMS     icareadmem;
ICAWRITEMEM_PARMS    icawritemem;
ICAINTREG_PARMS      icaintreg;
ICAINTWAIT_PARMS     icaintwait;
ICAINTDEREG_PARMS    icaintdereg;
ICAISSUECMD_PARMS    icaissuecmd;
ICAGETPARMS_PARMS    icagetparms;
ICAGETBUFADDRS_PARMS icagetbufaddrs;
ICAGETPRIMSTAT_PARMS icagetprimstat;
ICAGETPARMS_PARMS    icasendconfig;
ICAIOWRITE_PARMS     icaiowrite;
ICAIOREAD_PARMS      icaioread;
ICAPOSWRITE_PARMS    icaposwrite;
ICAPOSREAD_PARMS     icaposread;
ICADMASETUP_PARMS    icadmasetup;
ICADMAREL_PARMS      icadmarel;
ICAGETADAPTYPE_PARMS icagetadaptype;
} ARTIC_IOCTL_PARMS;

