static char sccsid[] = "@(#)56  1.1.1.3  src/bos/diag/tu/ethsal/wraptu.c, tu_ethsal, bos411, 9428A410j 10/20/93 14:15:12";
/*
 *   COMPONENT_NAME: tu_ethsal
 *
 *   FUNCTIONS: lb_82596_tu, int lb_82501_tu, lb_ext_tu, full_duplex_tu
 * 
 *   ORIGINS: 27
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/types.h>
#include <sys/device.h>
#include <sys/devinfo.h>
#include <sys/ioctl.h>
#include <sys/comio.h>
#include <sys/poll.h>
#include <sys/ciouser.h>
#include <sys/entuser.h>
#include <stdio.h>
#include <memory.h>

#include "exectu.h"
#include "tu_type.h"

#define CRC_BYTES           4
#define MED_FRAME_SIZE      ((MIN_FRAME_SIZE + MAX_FRAME_SIZE) / 2)
#define DEFAULT_NUM_FRAMES  3
#define FRAME_MATCH         0
#define DEFAULT_RETRIES     20
#define UNKNOWN_ERR         (MAX_ERR + 1)


typedef struct
{
  uchar_t dest_addr[NETW_ADDR_LENGTH];
  uchar_t src_addr[NETW_ADDR_LENGTH];
  netid_t type;
  ushort id;    /* frame ID */
} FRAME_HEADER;


static int wrap_test(uchar_t, BOOL);
static BOOL diff_count(char *, ulong_t, ulong_t);
static int init_frame(uchar_t [], int, BOOL);
static int get_adapter_status(int, ent_query_stats_t *);
static int write_frame(int, uchar_t [], int);
static int query_diff(ent_stats_t *, ent_stats_t *);
static int read_frame(int, uchar_t [], int, int *);
static int get_network_address (uchar_t []);

static ulong_t frame_size = USE_DEFAULT_FRAME_SIZE;
static ulong_t num_frames = USE_DEFAULT_NUM_FRAMES;

/*
 * NAME : lb_82596_tu
 *
 * DESCRIPTION :
 *
 *  Loopback at the 82596.
 *
 * INPUT :
 *
 *  None.
 *
 * OUTPUT :
 *
 *   None.
 *
 * RETURNS :
 *
 *   Error code or SUCCESS.
 *
*/

int lb_82596_tu(void)
{
  return(wrap_test((uchar_t) LB_82596, FALSE));
}



/*
 * NAME : lb_82501_tu
 *
 * DESCRIPTION :
 *
 *  Loopback at the 82596.
 *
 * INPUT :
 *
 *  None.
 *
 * OUTPUT :
 *
 *   None.
 *
 * RETURNS :
 *
 *   Error code or SUCCESS.
 *
*/

int lb_82501_tu(void)
{
  return(wrap_test((uchar_t) LB_82501, FALSE));
}



/*
 * NAME : lb_ext_tu
 *
 * DESCRIPTION :
 *
 *  External loopback (at the DIX connector)
 *
 * INPUT :
 *
 *  None.
 *
 * OUTPUT :
 *
 *   None.
 *
 * RETURNS :
 *
 *   Error code or SUCCESS.
 *
*/

int lb_ext_tu(void)
{
  return(wrap_test((uchar_t) LB_EXT, FALSE));
}



/*
 * NAME : lb_ext_emc_tu
 *
 * DESCRIPTION :
 *
 *  External loopback (at the DIX connector)
 *  (Same as lb_ext_tu but only ASCCII Hs are
 *  sent as specified by EMC standards).
 *
 * INPUT :
 *
 *  None.
 *
 * OUTPUT :
 *
 *   None.
 *
 * RETURNS :
 *
 *   Error code or SUCCESS.
 *
*/

int lb_ext_emc_tu(void)
{
  return(wrap_test((uchar_t) LB_EXT, TRUE));
}



/*
 * NAME : wrap_test
 *
 * DESCRIPTION :
 *
 *   Sets adapter into loopback mode as specified by
 *   input parameter.
 *   A number of frames as specified by global num_frames
 *   of size specified by global frame_size are transmitted,
 *   received and compared. If frame_size is found to be
 *   USE_DEFAULT_FRAME_SIZE then 3 frames of sizes
 *   MIN_FRAME_SIZE, MED_FRAME_SIZE, MAX_FRAME_SIZE are used.
 *   Each frame is retried as specified by DEFAULT_RETRIES.
 *   If input flag is set to EMC then frame data will be
 *   initialized to al H's as specified by EMC requirements.
 *
 * INPUT :
 *
 *   1. Loopback type.
 *   2. EMC flag.
 *
 * OUTPUT :
 *
 *   None.
 *
 * RETURNS :
 *
 *   SUCCESS if frame transmitted matches frame received, else
 *   error code.
 *
*/

static int wrap_test(uchar_t lb_type, BOOL emc)
{
  int fsize, save_rc, rc, eths_fd, halt_rc, rbytes, retry;
  uchar_t rx_frame[MAX_FRAME_SIZE + CRC_BYTES];
  uchar_t tx_frame[MAX_FRAME_SIZE];
  ulong_t numf, i;
  ent_query_stats_t before, after;
  char msg[100];
  BOOL write_failed, read_failed;

  static int default_frame_size[DEFAULT_NUM_FRAMES] = {  MIN_FRAME_SIZE,
                                                         MED_FRAME_SIZE,
                                                         MAX_FRAME_SIZE,
                                                       };

  rc = halt_rc = SUCCESS;
  eths_fd = get_fd(ETHS_FD);

  rc = start_ether();

  if(rc == SUCCESS)
    rc = config_ether(lb_type);

  numf = (num_frames == USE_DEFAULT_NUM_FRAMES) ? DEFAULT_NUM_FRAMES : num_frames;

  for(i = 0; i < numf && rc == SUCCESS; i++)
  {
    if(frame_size == USE_DEFAULT_FRAME_SIZE)
    {
      /* use default frame size(s) */

      if(lb_type != LB_82596)
      {
        /* limit frame size to min. value when looping back */
        /* at the 82C501 and at the DIX connector to avoid  */
        /* too many underruns/overruns                      */

        fsize = MIN_FRAME_SIZE;
      }
      else
        fsize = default_frame_size[i % DEFAULT_NUM_FRAMES];
    }
    else
    {
      /* use user supplied frame size */
      fsize = frame_size;
    }

    /* get adapter's status before sending frame  */

    rc = get_adapter_status(eths_fd, &before);

    if(rc == SUCCESS)
      rc = init_frame(&tx_frame[0], fsize, emc);

    if(rc == SUCCESS)
    {
      /* attempt to write/read a frame until success, up to a max. of        */
      /* DEFAULT_RETRIES                                                     */

      for(retry = 0; retry < DEFAULT_RETRIES; retry++)
      {
        write_failed = read_failed = FALSE;
        rc = write_frame(eths_fd, &tx_frame[0], fsize); /* write frame */

        if(rc == SUCCESS)
        {
          /* write succeeded, read frame */

          incr_stat((uchar_t) BYTES_WRITTEN, fsize);
          incr_stat((uchar_t) GOOD_WRITES, 1);

          /* Debug */
          sprintf(msg, "debug: Reading frame (retry=%d  fsize=%d  lb_type=%d)",retry, fsize, lb_type);
          DEBUG_MSG(msg);

          rc = read_frame(eths_fd, &rx_frame[0], sizeof(rx_frame), &rbytes); /* read frame */

          if(rc == SUCCESS)
          {
            /* read suceeded                */

            incr_stat((uchar_t) BYTES_READ, rbytes);
            incr_stat((uchar_t) GOOD_READS, 1);
          }
          else
          {
            read_failed = TRUE;
            sprintf(msg, "debug: READ FAILED (retry=%d  fsize=%d  lb_type=%d)",
                    retry, fsize, lb_type);
            DEBUG_MSG(msg);
          }
        }
        else
        {
          write_failed = TRUE;
          sprintf(msg, "debug: WRITE FAILED (retry=%d  fsize=%d  lb_type=%d)",
                    retry, fsize, lb_type);
          DEBUG_MSG(msg);
        }

        if(rc == SUCCESS)
        {
          if(rbytes != fsize)
          {
            sprintf(msg, "Wrote %u bytes, read %u bytes", fsize, rbytes);
            LOG_ERROR(msg);
            rc = READ_ERR;
          }

          else if(memcmp(&rx_frame[0], &tx_frame[0], fsize) != FRAME_MATCH)
            rc = FRAME_COMPARE_ERR;

          if(rc == SUCCESS)
            break;
          else
            read_failed = TRUE;
        }
      } /* FOR DEFAULT_RETRIES */

      if(write_failed)
        incr_stat((uchar_t) BAD_WRITES, 1);
      if(read_failed)
        incr_stat((uchar_t) BAD_READS, 1);

      if(rc != SUCCESS)
      {
        /* find out why operation failed */

        save_rc = rc;
        rc = get_adapter_status(eths_fd, &after);

        if(rc == SUCCESS)
        {
          rc = query_diff(&before.ds, &after.ds);

          if(rc == UNKNOWN_ERR)
            rc = save_rc;
        }
      }
    }
  } /* FOR */

  halt_rc = halt_ether();

  if(rc == SUCCESS)
    rc = halt_rc;

  return(rc);
}




/*
 * NAME : write_frame
 *
 * DESCRIPTION :
 *
 *  Writes a frame.
 *
 * INPUT :
 *
 *  1. Ethernet device driver file descriptor.
 *  2. Buffer containing frame to be transmitted.
 *  3. Frame size.
 *
 * OUTPUT :
 *
 *   None.
 *
 * RETURNS :
 *
 *   Error code or SUCCESS.
 *
*/

static int write_frame(int fd, uchar_t tx_frame[], int fsize)
{
  int rc, size;
  struct write_extension ext;
  FRAME_HEADER *hdr;
  char msg[100];

/* retry each frame DEFAULT_RETRIES times */

  rc = SUCCESS;
  hdr = (FRAME_HEADER *) &tx_frame[0];
  memset(&ext, 0, sizeof(ext));
  ext.write_id = hdr->id;
  ext.flag |= CIO_ACK_TX_DONE;
  size = writex(fd, &tx_frame[0], fsize, &ext);

  if(size < 0)
  {
    set_tu_errno();
    incr_stat((uchar_t) BAD_WRITES, 1);
    rc = WRITE_ERR;

    sprintf (msg, ">>> Write Error <<<");
    DEBUG_MSG(msg);
  }

  if(rc == SUCCESS)
    rc = wait_status(fd, CIO_TX_DONE, STATUS_TIMEOUT, (ushort) POLLPRI);

  if(rc == POLL_TIMEOUT_ERR)
    rc = POLL_WRITE_TIMEOUT_ERR;

  return(rc);
}





/*
 * NAME : read_frame
 *
 * DESCRIPTION :
 *
 *  Waits and reads a frame.
 *
 *
 * INPUT :
 *
 *  1. Ethernet device driver file descriptor.
 *  2. Buffer where to store frame.
 *  3. Input buffer size.
 *  4. Pointer to return number of bytes read.
 *
 * OUTPUT :
 *
 *   1. Frame read, if any.
 *   2. Bytes read (CRC_BYTES are discarded).
 *   3. Operation statics.
 *
 * RETURNS :
 *
 *   Error code or SUCCESS.
 *
 * NOTES :
 *
 *  1. readx() always returns extra CRC_BYTES.
*/

static int read_frame(int eths_fd, uchar_t rx_buff[], int rx_buff_size,
                                                    int *rbytes)
{
  int rc;
  char msg[80];
  struct read_extension ext;

  rc = SUCCESS;
  *rbytes = 0;
  rc = wait_status(eths_fd, 0, READ_TIMEOUT, (ushort) POLLIN);

  if(rc == POLL_TIMEOUT_ERR)
    rc = POLL_READ_TIMEOUT_ERR;

  else if(rc == SUCCESS)
  {
    memset(&rx_buff[0], 0, rx_buff_size);

    /* read frame */

    *rbytes = readx(eths_fd, &rx_buff[0], rx_buff_size, &ext);

    if(*rbytes < 0)
    {
      set_tu_errno();
      incr_stat((uchar_t) BAD_READS, 1);
      rc = READ_ERR;

      sprintf (msg, ">>> Read Error <<<");
      DEBUG_MSG(msg);
    }
  }

  if(rc == SUCCESS)   /* substract CRC_BYTES */
    *rbytes = (*rbytes >= CRC_BYTES) ? (*rbytes - CRC_BYTES) : 0;

  return(rc);
}




/*
 * NAME : get_adapter_status
 *
 * DESCRIPTION :
 *
 *  Gets Ethernet adapter status by issuing a
 *  CIO_QUERY ioctl.
 *
 * INPUT :
 *
 *  1. Ethernet device driver file descriptor.
 *  2. Pointer to ent_query_stats_t struct. to
 *     return status.
 *
 * OUTPUT :
 *
 *   1. Adapter status.
 *
 * RETURNS :
 *
 *   Error code or SUCCESS.
 *
*/

static int get_adapter_status(int fd, ent_query_stats_t *st)
{
  int rc;
  cio_query_blk_t queryst;
  char msg[80];

  rc = SUCCESS;
  queryst.bufptr = (caddr_t) st;
  queryst.buflen = sizeof(ent_query_stats_t);
  queryst.clearall = 0;

  if(ioctl(fd, CIO_QUERY, &queryst) < 0)
  {
    set_tu_errno();
    incr_stat((uchar_t) BAD_OTHERS, 1);
    rc = CIO_QUERY_ERR;
  }

  incr_stat((uchar_t) GOOD_OTHERS, 1);

  return(rc);
}



/*
 * NAME : init_frame
 *
 * DESCRIPTION :
 *
 *  Initializes a frame as follows :
 *
 *  |-----------------------------------------------------------------|
 *  | Byte  | 0| 1| 2| 3| 4| 5| 6| 7| 8| 9|10|11|12|13|14|15|16|17|...|
 *  |-------|-----------------|-----------------|-----|-----|---------|
 *  | Ether.|   Destination   |     Source      | Type| ID  |  Test   |
 *  | field |     address     |     address     |     |     |  data   |
 *  |-------|-----------------|-----------------|-----|-----|---------|
 *
 *  1. The source and destination addresses are set to the network address
 *     found in VPD.
 *  2. The type field is set to 0x5001.
 *  3. The frame ID is a 2-byte running counter.
 *  4. If the EMC flag is not set, the test data will be formatted on a word
 *     boundary by shifting left 1 bit as follows :
 *
 * 00 00 00 00 01 01 01 01 02 02 02 02 04 04 04 04 08 08 08 08 10 10 10 10 ..
 *
 *  This format was selected considering the fact that the DMA transfers words
 *  of data (32 bits).
 *
 *  If the EMC flag is set then all data will be ASCCII H, as per FCC
 *  requirements.
 *
 * INPUT :
 *
 *  1. Frame buffer.
 *  2. Frame size.
 *
 * OUTPUT :
 *
 *   Frame initialized.
 *
 * RETURNS :
 *
 *   Error code or SUCCESS.
 *
*/

static int init_frame(uchar_t frame[], int size, BOOL emc)
{
  int rc, i;
  uchar_t test_pattern, *buff;
  FRAME_HEADER *hdr;

  static ushort frame_id = 0;
  static uchar_t last_test_pattern = 0;

  rc = SUCCESS;
  hdr = (FRAME_HEADER *) &frame[0];
  buff = frame + sizeof(FRAME_HEADER);
  rc = get_network_address(&hdr->dest_addr[0]);

  if(rc == SUCCESS)
  {
    hdr->id = frame_id++;
    memcpy(&hdr->src_addr[0], &hdr->dest_addr[0], sizeof(hdr->src_addr));
    hdr->type = NET_ID;

    if(emc)
      memset(&buff[0], 'H', size - sizeof(FRAME_HEADER));

    else  /* not EMC */
    {
      test_pattern = last_test_pattern;

      for(i = 0; i < (size - sizeof(FRAME_HEADER)); i++)
      {
        if(i != 0 && ((i % 4) == 0))
          test_pattern = (test_pattern == 0) ? 1 : test_pattern << 1;

        buff[i] = test_pattern;
      }
/* next frame should have different data, besides different IDs */

      last_test_pattern = (last_test_pattern == 0) ?
        1 : last_test_pattern << 1;
    }
  }

  return(rc);
}



/*
 * NAME : query_diff
 *
 * DESCRIPTION :
 *
 *  Finds if there is any difference between Ethernet status
 *  structures.
 *
 * INPUT :
 *
 *  1. Pointer to ent_stats_t structure.
 *  2. Pointer to ent_stats_t structure to be compared with
 *     (1).
 *
 * OUTPUT :
 *
 *   None.
 *
 * RETURNS :
 *
 *   Code specifying first field found different.
 *
*/

static int query_diff(ent_stats_t *before, ent_stats_t *after)
{
  int rc;

  if(diff_count("max_collision", before->max_collision, after->max_collision))
    rc = COLLISION_ERR;

  else if(diff_count("carrier_lost", before->carrier_lost, after->carrier_lost))
    rc = CARRIER_LOST_ERR;

  else if (diff_count("underrun", before->underrun, after->underrun))
    rc = UNDERRUN_ERR;

  else if (diff_count("overrun", before->overrun, after->overrun))
    rc = OVERRUN_ERR;

  else
    rc = UNKNOWN_ERR;

/******** Next fields could be used later if necessary

    diff_count("intr_lost", before->intr_lost, after->intr_lost);
    diff_count("wdt_lost", before->wdt_lost, after->wdt_lost);
    diff_count("timo_lost", before->timo_lost, after->timo_lost);
    diff_count("sta_que_overflow", before->sta_que_overflow, after->sta_que_overflow);
    diff_count("rec_que_overflow", before->rec_que_overflow, after->rec_que_overflow);
    diff_count("recv_intr_cnt", before->recv_intr_cnt, after->recv_intr_cnt);
    diff_count("xmit_intr_cnt", before->xmit_intr_cnt, after->xmit_intr_cnt);
    diff_count("crc_error", before->crc_error, after->crc_error);
    diff_count("align_error", before->align_error, after->align_error);
    diff_count("too_short", before->too_short, after->too_short);
    diff_count("too_long", before->too_long, after->too_long);
    diff_count("no_resources", before->no_resources, after->no_resources);
    diff_count("pckts_discard", before->pckts_discard, after->pckts_discard);
    diff_count("late_collision", before->late_collision, after->late_collision);
    diff_count("cts_lost", before->cts_lost, after->cts_lost);
    diff_count("xmit_timeouts", before->xmit_timeouts, after->xmit_timeouts);
    diff_count("par_err_cnt", before->par_err_cnt, after->par_err_cnt);
    diff_count("diag_over_flow", before->diag_over_flow, after->diag_over_flow);
    diff_count("exec_over_flow", before->exec_over_flow, after->exec_over_flow);
    diff_count("exec_cmd_errors", before->exec_cmd_errors, after->exec_cmd_errors);
    diff_count("rec_dma_to", before->rec_dma_to, after->rec_dma_to);

***********************************************/

  return(rc);
}




/*
 * NAME : diff_count
 *
 * DESCRIPTION :
 *
 *  Compares 2 words.
 *
 * INPUT :
 *
 *  1. Field ID.  (Used in debugging).
 *  2. Word to be compared.
 *  3. Word to be compared.
 *
 *
 * OUTPUT :
 *
 *   None.
 *
 * RETURNS :
 *
 *   TRUE if words are different, else FALSE.
 *
*/

static BOOL diff_count(char *id, ulong_t before, ulong_t after)
{
  return(before != after);
}



/*
 * NAME : set_frame_size
 *
 * DESCRIPTION :
 *
 *  Updates global variable frame_size. If
 *  the size specified by input parameter is
 *  outside frame size limits then frame_size
 *  is set to USE_DEFAULT_FRAME_SIZE which
 *  forces wrap_test to send a sequence of
 *  3 test frames. If the input size is within
 *  limits, then frame_size is set to this size.
 *
 * INPUT :
 *
 *  1. Proposed frame size.
 *
 * OUTPUT :
 *
 *   Global frame_size updated.
 *
 * RETURNS :
 *
 *  TRUE if input size is within limits, else FALSE.
 *
*/

BOOL set_frame_size(ulong_t size)
{
  BOOL ok;

  if((MIN_FRAME_SIZE <= size) && (size <= MAX_FRAME_SIZE))
  {
    frame_size = size;
    ok = TRUE;
  }
  else
  {
    frame_size = USE_DEFAULT_FRAME_SIZE;
    ok = FALSE;
  }

  return(ok);
}



/*
 * NAME : set_frame_num
 *
 * DESCRIPTION :
 *
 *  Updates global num_frames with input
 *  value.
 *
 * INPUT :
 *
 *  1. Number of frames to be transmitted.
 *
 * OUTPUT :
 *
 *   num_frames updated.
 *
 * RETURNS :
 *
 *   None.
 *
*/

void set_frame_num(ulong_t num)
{
  num_frames = num;

  return;
}



/*
 * NAME : get_network_address
 *
 * DESCRIPTION :
 *
 *  Retrieves Ethernet network address from Ethernet Device info (devinfo)
 *
 * INPUT :
 *
 *   None.
 *
 * OUTPUT :
 *
 *   Network address, if successful.
 *
 * RETURNS :
 *
 *   Error code or SUCCESS.
 *
*/

static int get_network_address (uchar_t net_addr[])
{
  int  rc;
  struct devinfo ent_dev_info;
  static uchar_t addr [NETW_ADDR_LENGTH] = "";
  static found_net_addr = FALSE;

  rc = SUCCESS;

  if (!found_net_addr)
  {
    if (ioctl (get_fd (ETHS_FD), IOCINFO, &ent_dev_info) < 0)
    {
      set_tu_errno();
      rc = ETHER_NADDR_NOT_FOUND_ERR;
    }
    else
    {
#ifdef DEBUG_ETHER
      {
        int i;

        printf ("\nDevice type     : %c", ent_dev_info.devtype);
        printf ("\nDevice subtype  : %c", ent_dev_info.devsubtype);

        printf ("\nHardware address: 0x");
        for (i = 0; i < NETW_ADDR_LENGTH; i++) {
          printf ("%02x", ent_dev_info.un.ethernet.haddr [i]);
        } /* endfor */

        printf ("\nNetwork address : 0x");
        for (i = 0; i < NETW_ADDR_LENGTH; i++) {
          printf ("%02x", ent_dev_info.un.ethernet.net_addr [i]);
        } /* endfor */
      }
#endif

     /* check that multicast bit is not on in the VPD Ethernet hardware      */
     /* address.  This is a requirement for the IEEE Ethernet transceiver.   */

      rc = ((ent_dev_info.un.ethernet.net_addr[0] & MULTICAST_BIT) != 0) ?
                ETHER_NADDR_MULTICAST_ERR : SUCCESS;

      if (rc == SUCCESS)
      {
        found_net_addr = TRUE;
        memcpy (addr, ent_dev_info.un.ethernet.net_addr, NETW_ADDR_LENGTH);
        memcpy (net_addr, addr, NETW_ADDR_LENGTH);
      } /* endif */
    }
  }
  else
  {
     memcpy (net_addr, addr, NETW_ADDR_LENGTH);
  }

#ifdef DEBUG_ETHER
{
  int i;

  printf ("\nEthernet address: 0x");
  for (i = 0; i < NETW_ADDR_LENGTH; i++) {
    printf ("%02x", net_addr [i]);
  } /* endfor */
}
#endif

  return (rc);
}



