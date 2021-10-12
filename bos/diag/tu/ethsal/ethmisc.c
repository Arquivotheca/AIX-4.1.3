static char sccsid[] = "@(#)48  1.1.1.3  src/bos/diag/tu/ethsal/ethmisc.c, tu_ethsal, bos411, 9428A410j 10/20/93 14:15:23";
/*
 *   COMPONENT_NAME: tu_ethsal
 *
 *
 *   FUNCTIONS: start_ether, halt_ether, wait_status, Get_byte, put_byte,
 *              get_word, put_word, get_stat, incr_stat
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
#include <sys/devinfo.h>
#include <sys/poll.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/mode.h>
#include <sys/mdio.h>
#include <sys/devinfo.h>
#include <sys/comio.h>

#include "exectu.h"
#include "tu_type.h"


static STATIST_STRUCT statist;     /* to keep statistics */
static BOOL dev_started = FALSE;

static char *get_cio_msg(int);
static char *get_stat_type(int);

/*
 * NAME : start_ether
 *
 * DESCRIPTION :
 *
 *   Starts Ethernet adapter session.
 *
 * INPUT :
 *
 *   None.
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

int start_ether(void)
{
  int rc, fd;
  struct session_blk eth_session;

  rc = SUCCESS;

  if(!dev_started)
  {
    fd = get_fd(ETHS_FD);
    eth_session.netid = NET_ID;
    eth_session.length = NET_ID_LENGTH;

    if(ioctl(fd, CIO_START, &eth_session) < 0)
    {
      set_tu_errno();
      incr_stat((uchar_t) BAD_OTHERS, 1);
      rc = CIO_START_ERR;
    }

    incr_stat((uchar_t) GOOD_OTHERS, 1);

    if(rc == SUCCESS)
      rc = wait_status(fd, CIO_START_DONE, STATUS_TIMEOUT, (ushort) POLLPRI);

    if(rc == SUCCESS)
      dev_started = TRUE;
  }

  return(rc);
}



/*
 * NAME : halt_ether
 *
 * DESCRIPTION :
 *
 *   Halts Ethernet adapter session.
 *
 * INPUT :
 *
 *   None.
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

int halt_ether(void)
{
  int rc, fd;
  struct session_blk eth_session;

  rc = SUCCESS;

  if(dev_started)
  {
    fd = get_fd(ETHS_FD);
    eth_session.netid = NET_ID;
    eth_session.length = NET_ID_LENGTH;

    if(ioctl(fd, CIO_HALT, &eth_session) < 0)
    {
      set_tu_errno();
      incr_stat((uchar_t) BAD_OTHERS, 1);
      rc = CIO_HALT_ERR;
    }

    incr_stat((uchar_t) GOOD_OTHERS, 1);

    if(rc == SUCCESS)
      rc = wait_status(fd, CIO_HALT_DONE, STATUS_TIMEOUT, (ushort) POLLPRI);

    if(rc == SUCCESS)
      dev_started = FALSE;
  }

  return(rc);
}


/*
 * NAME : wait_status
 *
 * DESCRIPTION :
 *
 *      Polls the status queue of the driver under test until a
 *      status block with code equal to "code" is found or until
 *      the wait for a status block times out.  All status blocks
 *      received before the expected one are thrown out.
 *                                                                       *
 * INPUT :
 *
 *   1. Ethernet file descriptor.
 *   2. Code of operation to wait for.
 *   3. Timeout value.
 *   4. Poll type.
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

int wait_status (int fd, int code, int timeout_val, ushort reqevent)
{
  struct status_block status;
  struct pollfd pollblk;
  int rc, result;
  char msg[100];

  rc = SUCCESS;

  while (TRUE)
  {
    /*  Wait for a status block by polling for it:              */

    pollblk.fd = fd;
    pollblk.reqevents = reqevent;
    pollblk.rtnevents = 0;
    result = poll(&pollblk, 1, timeout_val);

    if (result < 0)
    {
      set_tu_errno();
      incr_stat((uchar_t) BAD_OTHERS, 1);
      rc = POLL_ERR;
      break;
    }
    else if(result == 0)
    {
      rc = POLL_TIMEOUT_ERR;
      break;
    }
    else
    {
      if(reqevent == POLLIN)
        break;

      /*  Status block is available -- issue ioctl to get it.   */

      result = ioctl(fd, CIO_GET_STAT, &status );

      if (result < 0)
      {
        set_tu_errno();
        incr_stat((uchar_t) BAD_OTHERS, 1);
        rc = CIO_GET_STAT_ERR;
        sprintf(msg, "CIO_GET_STAT failed : %s", get_stat_type(code));
        LOG_MSG(msg);
        break;
      }

      /*  Is this the code we are waiting for?  If not, loop    */
      /*  to next wait.                                         */

      if (status.code == code)
      {
        if (status.option[0] == CIO_OK)
          break;  /* succeeded */

        else
        {
          sprintf(msg, "%s : %s  (opt_3 = 0x%lx)",
                  get_stat_type(code), get_cio_msg(status.option[0]), status.option[3]);
          DEBUG_MSG(msg);
          rc = CIO_GET_STAT_ERR;
        }

        break;
      }

      else
        continue;  /* get next block */
    }
  }

  return(rc);
}


/*
 * NAME : get_cio_msg
 *
 * DESCRIPTION :
 *
 *   Gets text description for return code of
 *   ioctl(CIO_GET_STAT) function.
 *
 * INPUT :
 *
 *   1. CIO_GET_STAT code.
 *
 * OUTPUT :
 *
 *   None.
 *
 * RETURNS :
 *
 *   Text description of input code.
 *
*/

static char *get_cio_msg(int code)
{
  int i;
  char *msg;

  static struct
  {
    int code;
    char *msg;
  } cio[] =    {   CIO_BAD_MICROCODE,   "invalid microcode",
                   CIO_BUF_OVFLW,       "data too large for buffer",
                   CIO_HARD_FAIL,       "hard failure",
                   CIO_LOST_DATA,       "data were lost",
                   CIO_NOMBUF,          "no mbuf available",
                   CIO_NOT_STARTED,     "start not performed",
                   CIO_TIMEOUT,         "operation timed out",
                   CIO_NET_RCVRY_ENTER, "enter network recovery",
                   CIO_NET_RCVRY_EXIT,  "exit network recovery",
                   CIO_NET_RCVRY_MODE,  "network recovery mode",
                   CIO_INV_CMD,         "invalid command",
                   CIO_BAD_RANGE,       "bad address range",
                   CIO_NETID_INV,       "invalid net id",
                   CIO_NETID_DUP,       "duplicate net id",
                   CIO_NETID_FULL,      "all net are used",
                   CIO_TX_FULL,         "transmit queue is full",
                   CIO_OK,              "",
                 };


  for(i = 0, msg = ""; cio[i].code != CIO_OK; i++)
  {
    if(cio[i].code == code)
    {
      msg = cio[i].msg;
      break;
    }
  }

  return(msg);
}



/*
 * NAME : get_stat_type
 *
 * DESCRIPTION :
 *
 *   Gets text description for CIO_GET_STAT
 *   type
 *
 * INPUT :
 *
 *   1. CIO_GET_STAT code.
 *
 * OUTPUT :
 *
 *   None.
 *
 * RETURNS :
 *
 *   Text description of input code.
 *
*/

#define END_OF_CIO_TBLE  (-1)

static char *get_stat_type(int code)
{
  int i;
  char *msg;

  static struct
  {
    int code;
    char *msg;
  } cio[] =    {   CIO_START_DONE,   "CIO_START_DONE",
                   CIO_HALT_DONE,    "CIO_HALT_DONE",
                   CIO_TX_DONE,      "CIO_TX_DONE",
                   END_OF_CIO_TBLE,  "",
                 };


  for(i = 0, msg = ""; cio[i].code != END_OF_CIO_TBLE; i++)
  {
    if(cio[i].code == code)
    {
      msg = cio[i].msg;
      break;
    }
  }

  return(msg);
}



/*
 * NAME : Get_byte
 *
 * DESCRIPTION :
 *
 *   Gets a byte from I/O space.
 *
 * INPUT :
 *
 *   1. Machine device driver file descriptor.
 *   2. Pointer to byte where data will be returned.
 *   3. I/O address.
 *   4. Operation (MIOCCGET or MIOBUSGET).
 *
 * OUTPUT :
 *
 *   1. Byte read.
 *
 * RETURNS :
 *
 *   Error code or SUCCESS.
 *
*/

int Get_byte(int fd, uchar_t *byte, ulong_t addr, int op)
{
  MACH_DD_IO iob;
  int rc;

  rc = SUCCESS;
  iob.md_incr = MV_BYTE;
  iob.md_size = sizeof(uchar_t);
  iob.md_data = (char *) byte;
  iob.md_addr = addr;

  if(ioctl(fd, op, &iob) == -1)
  {
    set_tu_errno();
    incr_stat((uchar_t) BAD_OTHERS, 1);
    rc = MIOCCGET_ERR;
  }

  incr_stat((uchar_t) GOOD_OTHERS, 1);

  return(rc);
}



/*
 * NAME : put_byte
 *
 * DESCRIPTION :
 *
 *   Writes a byte to I/O space.
 *
 * INPUT :
 *
 *   1. Machine device driver file descriptor.
 *   2. Byte to be written.
 *   3. I/O address.
 *   4. Operation (MIOCCPUT or MIOBUSPUT).
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

int put_byte(int fd, uchar_t byte, ulong_t addr, int op)
{
  MACH_DD_IO iob;
  int rc;

  rc = SUCCESS;
  iob.md_incr = MV_BYTE;
  iob.md_size = sizeof(uchar_t);
  iob.md_data = (char *) &byte;
  iob.md_addr = addr;

  if(ioctl(fd, op, &iob) == -1)
  {
    set_tu_errno();
    incr_stat((uchar_t) BAD_OTHERS, 1);
    rc = MIOCCPUT_ERR;
  }

  incr_stat((uchar_t) GOOD_OTHERS, 1);

  return(rc);
}



/*
 * NAME : get_word
 *
 * DESCRIPTION :
 *
 *   Gets a word (32 bits) from I/O space.
 *
 * INPUT :
 *
 *   1. Machine device driver file descriptor.
 *   2. Pointer to word where data will be returned.
 *   3. I/O address.
 *   4. Operation (MIOCCGET or MIOBUSGET).
 *
 * OUTPUT :
 *
 *   1. Word read.
 *
 * RETURNS :
 *
 *   Error code or SUCCESS.
 *
*/

int get_word(int fd, ulong_t *word, ulong_t addr, int op)
{
  MACH_DD_IO iob;
  int rc;

  rc = SUCCESS;
  iob.md_incr = MV_WORD;
  iob.md_size = 1;
  iob.md_data = (char *) word;
  iob.md_addr = addr;

  if(ioctl(fd, op, &iob) == -1)
  {
    set_tu_errno();
    LOG_SYSERR("ioctl(GET)");
    incr_stat((uchar_t) BAD_OTHERS, 1);
    rc = MIOCCGET_ERR;
  }

  incr_stat((uchar_t) GOOD_OTHERS, 1);

  return(rc);
}



/*
 * NAME : put_word
 *
 * DESCRIPTION :
 *
 *   Writes a word (32 bits) to I/O space.
 *
 * INPUT :
 *
 *   1. Machine device driver file descriptor.
 *   2. Word to be written.
 *   3. I/O address.
 *   4. Operation (MIOCCGET or MIOBUSGET).
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

int put_word(int fd, ulong_t word, ulong_t addr, int op)
{
  MACH_DD_IO iob;
  int rc;

  rc = SUCCESS;
  iob.md_incr = MV_WORD;
  iob.md_size = 1;
  iob.md_data = (char *) &word;
  iob.md_addr = addr;

  if(ioctl(fd, op, &iob) == -1)
  {
    set_tu_errno();
    LOG_SYSERR("ioctl(PUT)");
    incr_stat((uchar_t) BAD_OTHERS, 1);
    rc = MIOCCPUT_ERR;
  }

  incr_stat((uchar_t) GOOD_OTHERS, 1);

  return(rc);
}



/*
 * NAME : get_stat
 *
 * DESCRIPTION :
 *
 *   Gets pointer to operation statistics.
 *
 * INPUT :
 *
 *   None.
 *
 * OUTPUT :
 *
 *   None.
 *
 * RETURNS :
 *
 *   Pointer to STATIST_STRUCT containing operation
 *   statistics.
 *
*/

STATIST_STRUCT *get_stat(void)
{
  return(&statist);
}



/*
 * NAME : incr_stat
 *
 * DESCRIPTION :
 *
 *   Increments statistics counter.
 *
 * INPUT :
 *
 *   1. Statistic counter index.
 *   2. Counter increment.
 *
 * OUTPUT :
 *
 *   Statistic counter incremented.
 *
 * RETURNS :
 *
 *   None.
 *
*/

void incr_stat(uchar_t op, ulong_t num)
{
  int i;

  static struct
  {
    uchar_t op_code;
    ulong_t *pt;
  } stat_count[] = { BAD_OTHERS, &statist.bad_others,
                     BAD_READS,  &statist.bad_reads,
                     BAD_WRITES, &statist.bad_writes,
                     BYTES_READ, &statist.bytes_read,
                     BYTES_WRITTEN, &statist.bytes_writ,
                     GOOD_OTHERS, &statist.good_others,
                     GOOD_READS, &statist.good_reads,
                     GOOD_WRITES, &statist.good_writes,
                     0, NULL,
                    };

  for(i = 0; stat_count[i].pt != NULL; i++)
  {
    if(stat_count[i].op_code == op)
    {
      *(stat_count[i].pt) += num;
      break;
    }
  }

  return;
}




/*
 * NAME : reset_stat
 *
 * DESCRIPTION :
 *
 *   Clears statistics counters.
 *
 * INPUT :
 *
 *  None.
 *
 * OUTPUT :
 *
 *   Statistics counters cleared.
 *
 * RETURNS :
 *
 *   None.
 *
*/

void reset_stat(void)
{
  memset(&statist, 0, sizeof(statist));

  return;
}
