/* @(#)92 1.1 src/bos/usr/fvcioem/ddt/tok/ddt_tok.h, fvcioem, bos411, 9428A410j 4/26/94 13:54:07 */

/*--------------------------------------------------------------------------
*
*             DDT_TOK.H
*
*  COMPONENT_NAME:  Communications Device Driver Tool TOK Definitions.
*
*  FUNCTIONS:
*
*  ORIGINS: 27
*
*  IBM CONFIDENTIAL -- (IBM Confidential Restricted when
*  combined with the aggregated modules for this product)
*                   SOURCE MATERIALS
*  (C) COPYRIGHT International Business Machines Corp. 1989
*  All Rights Reserved
*
*  US Government Users Restricted Rights - Use, duplication or
*  disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
*
*
*--------------------------------------------------------------------------
*/

/* !!! temp fix for "tokuser.h"  talk to GED */
#define TEST_IOCTL             (CIO_IOCTL |    0x0115)
#define TEST_DO_FREEZE_DUMP    (CIO_IOCTL |    0x0116)

# define DEVICE      " Token Ring"
# define DEVPATH  "/dev/tok"
# define DEFAULT_DEST   "FFFFFFFFFFFF"
# define DEFAULT_NETID  0xAA
# define DEFAULT_AC  0x00
# define DEFAULT_SIZE   4000
# define DEFAULT_WRITES 1
# define DEFAULT_READS  1
# define RECV_PAD 33

# define BUFSIZE  TOK_16M_MAX_PACKET
# define MIN_PACKET  TOK_MIN_PACKET
# define MIN_FRAME   "50"
# define RAMSIZE  8192     /* Read/write adapter RAM size */
# define POS_ACC  TOK_ACCESS_POS
# define POS_READ TOK_READ
# define POS_WRITE   TOK_WRITE
# define BAD_RANGE   TOK_BAD_RANGE
# define INV_CMD  TOK_INV_CMD
# define NOT_DIAG_MODE  TOK_NOT_DIAG_MODE
# define GET_VPD  TOK_QVPD
# define NOT_READ_VPD   TOK_VPD_NOT_READ
# define INVALID_VPD TOK_VPD_INVALID
# define VALID_VPD   TOK_VPD_VALID

# define TEST_PROTOCOL  0x08     /* type field for tests */

typedef tok_pos_reg_t      POS;
typedef tok_vpd_t    VPDATA;
typedef tok_query_stats_t  STATISTICS;

# define DATALINK_HDR   16    /* size of datalink header + pad */
# define MAC_HDR  14
# define DATASIZE (BUFSIZE - DATALINK_HDR)

typedef struct {
   unsigned char  ac;      /* Access Control Field */
   unsigned char  fc;      /* Frame Control Field */
   unsigned char  dest[6];
   unsigned char  src[6];
   char     data[ DATASIZE ];
} MAC_FRAME;

typedef struct ROUTING_INFO  ROUTING_INFO;

struct ROUTING_INFO
{
   unsigned short            rctl;
   unsigned short            segment[8];
};

typedef struct {
   unsigned char  ac;      /* Access Control Field */
   unsigned char  fc;      /* Frame Control Field */
   unsigned char  dest[6];
   unsigned char  src[6];
   ROUTING_INFO   route;
   unsigned char  netid;
   unsigned char  pad;     /* unused */
   char     data[ DATASIZE ];
} XMIT_FRAME;

typedef struct {
   unsigned char  ac;      /* Access Control Field */
   unsigned char  fc;      /* Frame Control Field */
   unsigned char  dest[6];
   unsigned char  src[6];
   ROUTING_INFO   route;
   unsigned char  netid;
   unsigned char  pad;     /* unused */
   char     data[ DATASIZE ];
} RECV_FRAME;

# define SWAPSHORT(x)   ((((x) & 0xFF) << 8) | ((x) >> 8))
# define SWAPLONG(x)    ((((x) & 0xFF)<<24) | (((x) & 0xFF00)<<8) | \
          (((x) & 0xFF0000)>>8) | (((x) & 0xFF000000)>>24))

