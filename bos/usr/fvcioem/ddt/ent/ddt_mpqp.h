/* @(#)03   1.1  src/bos/usr/fvcioem/ddt/ent/ddt_mpqp.h, fvcioem, bos411, 9428A410j 4/26/94 13:54:34 */

/*--------------------------------------------------------------------------
*
*             DDT_MPQP.H
*
*  COMPONENT_NAME:  Communications Device Driver Tool MPQP Definitions.
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

# define DEVICE      "Multi-Protocol"
# define DEVPATH  "/dev/mpq"
# define DEFAULT_NETID  0xAA
# define DEFAULT_SIZE   1800
# define DEFAULT_WRITES 1
# define DEFAULT_READS  1
# define RECV_PAD 0

# define BUFSIZE  4096
# define MIN_PACKET  2
# define MIN_FRAME   "21"
# define RAMSIZE  4096
# define MAX_DMA_BUF 4096
# define ASW_OFFSET  0x10100     /* used for download */

# define BAD_RANGE   (CIO_EXCEPT_MAX - 1)
# define INV_CMD  (CIO_EXCEPT_MAX - 2)
# define NOT_DIAG_MODE  (CIO_EXCEPT_MAX - 3)

# define TEST_PROTOCOL  0x0880      /* type field for tests */

typedef t_query_parms   STATISTICS;

# define DATALINK_HDR   0     /* sizeof datalink header */
# define DATASIZE (BUFSIZE - DATALINK_HDR)

         /* leave room for STX and ETX on transmits */
typedef struct {
   char     data[ DATASIZE + 2 ];
} XMIT_FRAME;

typedef struct {
   char     data[ DATASIZE + 2 ];
} RECV_FRAME;

/* bisync control character defines */

# define SOH   ( (unsigned char)0x01 )
# define STX   ( (unsigned char)0x02 )
# define ITB   ( (unsigned char)0x1F )
# define ETB   ( (unsigned char)p_xlat[3] )
# define ETX   ( (unsigned char)0x03 )
# define DLE   ( (unsigned char)0x10 )
# define ENQ   ( (unsigned char)p_xlat[6] )
# define EOT   ( (unsigned char)p_xlat[7] )
# define NAK   ( (unsigned char)p_xlat[8] )
# define SYN   ( (unsigned char)p_xlat[9] )
# define RVI   ( (unsigned char)p_xlat[10] )
# define ACK0  ( (unsigned char)p_xlat[11] )
# define ACK1  ( (unsigned char)p_xlat[12] )
# define WACK  ( (unsigned char)p_xlat[13] )

/* SDLC control character defines */

# define SDLC_MDM_ADDR  ( (unsigned short)0xFF )
# define SDLC_FNL_CTRL  ( (unsigned short)0x13 )

/*----------------------------------------------------------------------*/
/*  These must be maintained to reflect the values in mpqpasw.map:   */
/*----------------------------------------------------------------------*/

# define ACQ_OFFSET     0x1C038     /* ACQUEUE in mpqpasw.map */
# define CMD_BLK_OFFSET    0x1C840     /* CMDBLK in mpqpasw.map */
# define PCB_BASE    0x1DA06     /* pcb in mpqpasw.map */
# define NUM_CMD_BLOCKS    48
# define PCB_SIZE    210
# define PCQ_OFFSET     0x1D440     /* port_cq in mpqpasw.map */
# define PCQ_NXT_OFFSET    0x38     /* space between port_cq's */

