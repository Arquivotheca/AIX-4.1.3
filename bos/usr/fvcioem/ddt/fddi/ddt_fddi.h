/* @(#)11 1.1 src/bos/usr/fvcioem/ddt/fddi/ddt_fddi.h, fvcioem, bos411, 9428A410j 4/26/94 13:54:54 */

/*--------------------------------------------------------------------------
*
*             DDT_FDDI.H
*
*  COMPONENT_NAME:  Communications Device Driver Tool FDDI Definitions.
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
# define DEVICE         " FDDI Ring"
# define DEVPATH     "/dev/fddi"
# define DEFAULT_DEST      "FFFFFFFFFFFF"
# define DEFAULT_NETID     0xAA
# define DEFAULT_TRF    00
# define DEFAULT_SIZE      4000
# define DEFAULT_WRITES    1
# define DEFAULT_READS     1
# define DEFAULT_STATUSRATE   1
# define RECV_PAD    33

/*
 * Allow testing of driver with a packet that is too big
 */
# define BUFSIZE  (FDDI_MAX_LLC_PACKET+512)
# define MIN_PACKET  0
# define MIN_FRAME   "50"     /* I'm not sure why this is so */
# define RAMSIZE  8192     /* Read/write adapter RAM size */
# define POS_ACC
# define POS_READ
# define POS_WRITE
# define BAD_RANGE
# define INV_CMD
# define NOT_DIAG_MODE
# define GET_VPD
# define NOT_READ_VPD
# define INVALID_VPD
# define VALID_VPD

# define TEST_PROTOCOL  0x08     /* type field for tests */


# define DATALINK_HDR   (sizeof (fddi_hdr_t) + 1)
# define DATASIZE (BUFSIZE - DATALINK_HDR)

typedef fddi_query_stats_t STATISTICS;

typedef fddi_vpd_t   VPDATA;

typedef struct {
   fddi_hdr_t  hdr;
   unsigned char  pad;
   char     data[ DATASIZE ];
} XMIT_FRAME;

typedef struct {
   fddi_hdr_t  hdr;
   unsigned char  pad;
   char     data[ DATASIZE ];
} RECV_FRAME;

# define SWAPSHORT(x)   ((((x) & 0xFF) << 8) | ((x) >> 8))
# define SWAPLONG(x)    ((((x) & 0xFF)<<24) | (((x) & 0xFF00)<<8) | \
          (((x) & 0xFF0000)>>8) | (((x) & 0xFF000000)>>24))

/*
 * fc values based on address size: long (default) and short
 */
#define FDDI_SMT_TYPE      (FDDI_FC_ADDR)
#define FDDI_LLC_TYPE      (FDDI_FC_ADDR | FDDI_FC_LLC)

#define FDDI_SMT_SHORT_TYPE   (~FDDI_FC_ADDR)
#define FDDI_LLC_SHORT_TYPE   (~FDDI_FC_ADDR | FDDI_FC_LLC)
