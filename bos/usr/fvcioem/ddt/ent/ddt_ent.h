/* @(#)01   1.1  src/bos/usr/fvcioem/ddt/ent/ddt_ent.h, fvcioem, bos411, 9428A410j 4/26/94 13:54:29 */

/*--------------------------------------------------------------------------
*
*             DDT_ENT.H
*
*  COMPONENT_NAME:  Communications Device Driver Tool ENT Definitions.
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

# define DEVICE      "   Ethernet"
# define DEVPATH  "/dev/ent"
# define DEFAULT_DEST   "FFFFFFFFFFFF"
# define DEF_DMA_LENGTH 4096
# define DEFAULT_NETID  0x0806
# define DEF_NETID_SIZE 2
# define DEFAULT_SIZE   1500
# define DEFAULT_WRITES 1
# define DEFAULT_READS  1
# define RECV_PAD 0

# define BUFSIZE  ent_MAX_PACKET
# define MIN_PACKET  ent_MIN_PACKET
# define MIN_FRAME   "50"
# define RAMSIZE  2048
# define MAX_DMA_BUF 2048

# define POS_ACC  CCC_POS_ACC
# define POS_READ CCC_READ_OP
# define POS_WRITE   CCC_WRITE_OP
# define BAD_RANGE   CCC_BAD_RANGE
# define INV_CMD  CCC_INV_CMD
# define NOT_DIAG_MODE  CCC_NOT_DIAG_MODE
# define GET_VPD  CCC_GET_VPD
# define NOT_READ_VPD   VPD_NOT_READ
# define NOT_AVAIL_VPD  VPD_NOT_AVAIL
# define INVALID_VPD VPD_INVALID
# define VALID_VPD   VPD_VALID

# define TEST_PROTOCOL  0x0880      /* type field for tests */

typedef ccc_pos_acc_t      POS;

typedef struct {
    cio_stats_t   cc;
    ent_stats_t   ds;
} STATISTICS;

typedef ccc_vpd_blk_t      VPDATA;

# define DATALINK_HDR   14    /* sizeof datalink header */
# define DATASIZE (BUFSIZE - DATALINK_HDR)

typedef struct {
   unsigned char  dest[6];
   unsigned char  src[6];
   unsigned short netid;
   char     data[ DATASIZE ];
} XMIT_FRAME;

typedef XMIT_FRAME RECV_FRAME;

# define SWAPSHORT(x)   ((((x) & 0xFF) << 8) | ((x) >> 8))
# define SWAPLONG(x)    ((((x) & 0xFF)<<24) | (((x) & 0xFF00)<<8) | \
          (((x) & 0xFF0000)>>8) | (((x) & 0xFF000000)>>24))

