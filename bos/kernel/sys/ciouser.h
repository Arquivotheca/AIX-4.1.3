/* @(#)77 1.3 6/16/90 00:24:59 */
#ifndef _H_CIOUSER
#define _H_CIOUSER

/*
 * COMPONENT_NAME: sysxcio -- Common Communications Code Device Driver Head
 *
 * FUNCTIONS: ciouser.h
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*****************************************************************************/
/* common exception codes for drivers using ciodd.c                          */
/*****************************************************************************/

#define CCC_BAD_RANGE      (CIO_EXCEPT_MAX + 0x01)
#define CCC_INV_CMD        (CIO_EXCEPT_MAX + 0x02)
#define CCC_NOT_DIAG_MODE  (CIO_EXCEPT_MAX + 0x03)
#define CCC_QUE_EMPTY      (CIO_EXCEPT_MAX + 0x04)

#define CCC_EXCEPT_MAX     (CIO_EXCEPT_MAX + 0x40) /* max value in ciouser.h */

/*****************************************************************************/
/* common ioctl's for drivers using ciodd.c                                  */
/*****************************************************************************/

#define CCC_DOWNLOAD  (CIO_IOCTL_MAX + 0x01) /* download microcode           */
#define CCC_GET_VPD   (CIO_IOCTL_MAX + 0x02) /* get vital product data       */
#define CCC_TRCTBL    (CIO_IOCTL_MAX + 0x05) /* return tracetable location   */

#define CCC_MEM_ACC   (CIO_IOCTL_MAX + 0x20) /* Adapter RAM access    diagnos*/
#define CCC_POS_ACC   (CIO_IOCTL_MAX + 0x21) /* POS register access   diagnos*/
#define CCC_REG_ACC   (CIO_IOCTL_MAX + 0x22) /* I/O register access   diagnos*/

#define CCC_IOCTL_MAX (CIO_IOCTL_MAX + 0x40) /* max value in ciouser.h       */

/*****************************************************************************/
/* CCC_DNLD ioctl parameter definitions                                      */
/*****************************************************************************/

typedef struct {
    ulong       status;    /* Returned status                                */
    char       *p_mcload;  /* microcode loader image pointer                 */
    uint        l_mcload;  /* microcode loader length                        */
    char       *p_mcode;   /* microcode image pointer                        */
    uint        l_mcode;   /* microcode length                               */
} ccc_download_t;

/*****************************************************************************/
/* CCC_GET_VPD ioctl parameter definitions                                   */
/*****************************************************************************/

/* vpd status codes returned by CCC_GET_VPD ioctl */
#define VPD_NOT_READ   (0) /* the vpd data has not been obtained from adap   */
#define VPD_NOT_AVAIL  (1) /* the vpd data is not available for this adapter */
#define VPD_INVALID    (2) /* the vpd data was obtained but is invalid       */
#define VPD_VALID      (3) /* the vpd data was obtained and is valid         */
#define VPD_STATUS_MAX (9) /* last one -- make custom codes this+1 and up    */

#define MAX_VPD_LENGTH (256) /* maximum bytes of vpd */

struct vital_product_data {  /* structure returned by CCC_GET_VPD ioctl      */
   ulong status;             /* vpd_status_t value                           */
   ulong length;             /* number of bytes actually returned (may be 0) */
   uchar vpd[MAX_VPD_LENGTH];/* vital product data characters                */
};
typedef struct vital_product_data ccc_vpd_blk_t; /* CCC_GET_VPD parameter    */

/*****************************************************************************/
/* CCC_MEM_ACC, CCC_POS_ACC, CCC_REG_ACC diagnostic ioctl definitions        */
/*****************************************************************************/

/* codes for use with diagnostic ioctl's                                     */
#define CCC_WRITE_OP  (1)  /* Write operation (supply data to adapter)       */
#define CCC_READ_OP   (2)  /* Read operation (obtain data from adapter)      */
#define CCC_READ_Q_OP (3)  /* Read the adapter image reg queue               */

typedef struct {           /* parameter for CCC_MEM_ACC ioctl                */
   ulong  status;          /* exception code                                 */
   ushort opcode;          /* Read or Write Adapter RAM                      */
   uint   ram_offset;      /* RAM Offset                                     */
   uint   length;          /* Length of transfer (0000 to 3FFF)              */
   uchar  *buffer;         /* Data Buffer Pointer                            */
} ccc_mem_acc_t;

typedef struct {           /* parameter for CCC_POS_ACC ioctl                */
   ulong  status;          /* exception code                                 */
   ushort opcode;          /* Read or Write POS register                     */
   uchar  pos_reg;         /* POS register select (0 to 7)                   */
   uchar  pos_val;         /* POS read/write data value                      */
} ccc_pos_acc_t;

typedef struct {           /* parameter for CCC_REG_ACC ioctl                */
   ulong  status;          /* exception code                                 */
   ushort opcode;          /* Read or Write I/O Register                     */
   uchar  io_reg;          /* I/O Register Select                            */
   uchar  io_val;          /* I/O Read/Write Data Value                      */
   uchar  io_val_o;        /* Optional read data value                       */
   uchar  io_status;       /* Optional read status value                     */
} ccc_reg_acc_t;

#endif /* ! _H_CIOUSER */
