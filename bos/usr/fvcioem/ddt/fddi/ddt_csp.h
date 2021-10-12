/* @(#)09 1.1 src/bos/usr/fvcioem/ddt/fddi/ddt_csp.h, fvcioem, bos411, 9428A410j 4/26/94 13:54:47 */

/*--------------------------------------------------------------------------
*
*             DDT_CSP.H
*
*  COMPONENT_NAME:  Communications Device Driver Tool
*                   Client/Server Protocol Definitions.
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

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*                       CLIENT/SERVER TEST PROTOCOL                        */
/*                                                                          */
/*  The Client/Server test protocol tests the network device driver by      */
/*  establishing a session between two adapters (either on two different    */
/*  machines or on the same machine).  The "Client" side of the session     */
/*  sends test frames to a "Server" that verifies the test data and         */
/*  responds with an ack for every N test frames received.  To prevent      */
/*  the Client side from receiving its own test frames, an address          */
/*  resolution handshake is performed before the tests are started -- the   */
/*  Client sends an ARP broadcast with its hardware address to the Server.  */
/*  The Server receives this packet and saves the hardware address (for use */
/*  in ack'ing the Client) before returning an ARP response back to the     */
/*  Client with the Server hardware address.                                */
/*     After the address resolution procedure is completed, the Client      */
/*  sends a test parameters packet that configures the Server side with the */
/*  parameters of the next battery of tests (these parameters are prompted  */
/*  for at the Client side).  The Server responds with an ack and waits for */
/*  test frames from the Client.  The Client sends N test frames to the     */
/*  Server and awaits an ack before continuing with the next N test         */
/*  frames.  Each side has a receive timeout in the event of failure at     */
/*  the other end of the session.  When all test frames have been sent,     */
/*  the Client sends a halt packet; the Server responds with a "test        */
/*  results" packet that contains error statistics logged on the Server     */
/*  side of the session.  The Client combines this information with its     */
/*  own statistics and displays them at the end of the tests.         */
/* Two types of errors are checked for; lost frames and corrupted     */
/*  test data frames.  Each frame has a sequence number that reveals lost   */
/*  frames (indicated by frames arriving without consecutive sequence       */
/*  numbers).  Each test data frame is validated byte-for-byte at the       */
/*  Server end when zero, ones, or incrementing data patterns are sent.     */
/* Both the Client and Server are equipped with error recovery logic   */
/*  in the event of dropped or corrupted frames; retries are performed at   */
/*  each end of the connection to avoid deadlock situations.  If the Client */
/*  times out awaiting an ack, it sends the next burst of test frames and   */
/*  listens again for a response before giving up.  If the Server times out */
/*  awaiting a test packet, it sends an ack and waits again for a test      */
/*  packet before giving up.                                                */
/*                                                                          */
/*--------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                     Client/Server Protocol Definitions                    */
/*---------------------------------------------------------------------------*/

# define ARP_REQUEST 1     /* Op code for ARP request */
# define ARP_REPLY   2     /* Op code for ARP response */
# define HLEN     6     /* number of octets in ether addr */
# define PLEN     4     /* number of octets in IP addr */
# define CLIENT_PA   "TPC"    /* test protocol client */
# define SERVER_PA   "TPS"    /* test protocol server */

               /* Test protocol operations: */
# define TEST_PARMS  1
# define TEST_PATTERN   2
# define TEST_HALT   3
# define TEST_ACK 4
# define TEST_ARP 5
# define TEST_RESULTS   6

# define ARP_RETRY      6  /* Retry server this many times */
# define CLIENT_TIMEOUT    10000 /* about 10 seconds */
# define SERVER_TIMEOUT    (CLIENT_TIMEOUT*ARP_RETRY)
# define ARP_TIMEOUT    60000 /* about 1 minute */
# define WRITE_RETRIES     10 /* retry on write EAGAIN */

/*
 * saved frame pathnames:
 */
# define CFPATH      "/tmp/clientdata"
# define SFPATH      "/tmp/serverdata"

               /* sizeof header in following TP: */
# define TEST_HEADER (PLEN + PLEN + (3 * sizeof(int)))

typedef struct {        /* Test Protocol Packet */
   char     dest[ PLEN ];  /* to who? */
   char     src [ PLEN ];  /* from who? */
   unsigned int   seq_number; /* sequence number */
   unsigned int   operation;  /* what are we doing? */
   unsigned int   length;     /* length of data */
               /* test data starts here */
   char     data[ DATASIZE - TEST_HEADER ];
} TP;

typedef struct {        /* Test protocol addr resolution */
   unsigned char  sender[ HLEN ];
   unsigned char  target[ HLEN ];
   unsigned int   type;
} TP_ARP;

typedef struct {        /* Test protocol parameters */
   unsigned int   ack_spacing;
   unsigned int   pattern_type;
   unsigned int   max_errors;
} TP_PARMS;

typedef struct {        /* Test results (Server) */
   unsigned int   lost_acks;
   unsigned int   lost_frames;
   unsigned int   bad_frames;
   unsigned int   xmit_frames;
   unsigned int   recv_frames;
} TP_RESULT;

# define TP_HDR_SIZE (DATALINK_HDR + TEST_HEADER)
# define ARP_SIZE (TP_HDR_SIZE + sizeof(TP_ARP))
# define PARM_SIZE   (TP_HDR_SIZE + sizeof(TP_PARMS))
# define RESULT_SIZE (TP_HDR_SIZE + sizeof(TP_RESULT))

