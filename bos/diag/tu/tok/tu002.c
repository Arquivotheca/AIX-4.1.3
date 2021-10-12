static char sccsid[] = "src/bos/diag/tu/tok/tu002.c, tu_tok, bos411, 9428A410j 6/11/92 10:07:17";
/*
 * COMPONENT_NAME: (TOKENTU) Token Test Unit
 *
 * FUNCTIONS: init_write_buf, init_read_buf, transmit_buf, receive_buf, tu002
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1990
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*****************************************************************************

Function(s) Test Unit 002 - Wrap Data

This test unit attempts to wrap data and compare it (read/write/compare).

*****************************************************************************/
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/devinfo.h>
#include <sys/comio.h>
#include <sys/tokuser.h>
#include <memory.h>

#include "toktst.h"	/* note that his also includes hxihtx.h */

#define DEFAULT_FRAME_SIZE  2048
#define RI_FIELD_LEN          18
#define TX_STAT_ATTEMPTS      64
#define READ_ATTEMPTS         32

#define MAX_TRIES	2	/*
				 * max no. of tries in event of ASYNC
				 * CHANNEL CHECK
				 */

#ifdef debugg
extern void detrace();
#endif

/*****************************************************************************

init_write_buf

*****************************************************************************/

int init_write_buf (fdes, write_buf_p, wr_frame_size_p, tucb_ptr)
   int fdes;
   unsigned char **write_buf_p;
   long *wr_frame_size_p;
   TUTYPE *tucb_ptr;
   {
   	struct htx_data *htx_sp;
	unsigned char *ucp, *buf_end;
	int rc;
	char *malloc();
	extern int mktu_rc();
	extern int fill_buf();

	/*
	 * set up a pointer to HTX data structure to
	 * increment counters in case tu was invoked by
	 * hardware exerciser.
	 */
	htx_sp = tucb_ptr->token_s.htx_sp;

	/*
	 * set up frame size.  If manuf. application is running,
	 * then take DEFAULT_FRAME_SIZE or value passed in r1,
         * else check the value
	 * passed in the tucb_ptr in case it was assigned via
	 * a keyword in the rule file.
	 */
	if (tucb_ptr->header.mfg != INVOKED_BY_HTX)
           {
           if (tucb_ptr->header.r1 == 0)
                 *wr_frame_size_p = DEFAULT_FRAME_SIZE;
           else
                 *wr_frame_size_p = tucb_ptr->header.r1;
           }
	else
	   {
		*wr_frame_size_p = tucb_ptr->token_s.frame_size;
		if (!(*wr_frame_size_p))
			*wr_frame_size_p = DEFAULT_FRAME_SIZE;
	   }

	*write_buf_p = (unsigned char *) malloc(*wr_frame_size_p);
	if (*write_buf_p == NULL)
	   {
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, MEM_ERR));
	   }

	/*
	 * set up frame for transmission.  Set the first two bytes,
	 * PCF0 with zero and PCF1 with 0x40, then the TO network
	 * address (self), zero out the FROM network address.
	 */
	**write_buf_p = 0x00;
	*(*write_buf_p + 1) = 0x40;
	memcpy(*write_buf_p + 2, tucb_ptr->token_s.network_address, NETADD_LEN);
	memset(*write_buf_p + 2 + NETADD_LEN, 0, NETADD_LEN);

	ucp = *write_buf_p + 2 + (2 * NETADD_LEN);

	/*
	 * put in the netid.
	 */
	*ucp++ = (unsigned char) TU_NET_ID;

	buf_end = *write_buf_p + *wr_frame_size_p;

	/*
	 * ok, now go fill up buffer with a pattern....
	 */
	if (rc = fill_buf(ucp, buf_end, tucb_ptr))
	   {
		free(*write_buf_p);
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, rc));
	   }
	
	return(0);
   }

/*****************************************************************************

init_read_buf

*****************************************************************************/

int init_read_buf (fdes, read_buf_p, rd_frame_size_p, wr_frame_size, tucb_ptr)
   int fdes;
   unsigned char **read_buf_p;
   long *rd_frame_size_p;
   long wr_frame_size;
   TUTYPE *tucb_ptr;
   {
	struct htx_data *htx_sp;
	char *malloc();
	extern int mktu_rc();

	/*
	 * set up a pointer to HTX data structure to
	 * increment counters in case tu was invoked by
	 * hardware exerciser.
	 */
	htx_sp = tucb_ptr->token_s.htx_sp;

	/*
	 * Note that on the "read_buf", we allocate RI_FIELD_LEN more
	 * bytes than the write buffer.  This is because the adapter
	 * performs a padding operation which increases the size
	 * of the frame AFTER a "write".
	 */
	*rd_frame_size_p = wr_frame_size + RI_FIELD_LEN;
	*read_buf_p = (unsigned char *) malloc(*rd_frame_size_p);
	if (*read_buf_p == NULL)
	   {
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, MEM_ERR));
	   }
	return(0);
   }

/*****************************************************************************

transmit_buf

*****************************************************************************/

int transmit_buf (fdes, write_buf, wr_frame_size, tucb_ptr)
   int fdes;
   unsigned char *write_buf;
   long wr_frame_size;
   TUTYPE *tucb_ptr;
   {
	int i, rc;
	int save_errno, uncode;
	struct htx_data *htx_sp;
	struct status_block write_status_s;
	struct write_extension ext_s;
	extern int mktu_rc();
	extern int tr_stat();
	extern int tr_uncode();

	/*
	 * set up a pointer to HTX data structure to
	 * increment counters in case tu was invoked by
	 * hardware exerciser.
	 */
	htx_sp = tucb_ptr->token_s.htx_sp;

	/*
	 * since write() only queues up data (not actually
	 * does the transmission), we'll use a writex() so we
	 * can use the extended parameter block to specify that
	 * we want a "transmit acknowledgement" when actually
	 * completed.
	 */
	ext_s.flag = CIO_ACK_TX_DONE;

#ifdef debugg
	detrace(0,"transmit_buf:  Right before the writex with\n");
	detrace(0,"        wr_frame_size at %d bytes.\n", wr_frame_size);
	detrace(0,"        The first 30 bytes (in hex) look like:\n");
	for (i = 0; i < 30; i++)
	   {
		detrace(0,"%02x ", *(write_buf + i));
		if (i == 16)
			detrace(0,"\n");
	   }
	detrace(1,"\n\ntransmit_buf:  Okay, here we go....\n");
#endif

	rc = writex(fdes, write_buf, wr_frame_size, &ext_s);
	save_errno = errno;

#ifdef debugg
	detrace(0,"transmit_buf:  AFTER writex.  cc = %d\n", cc);
	if (rc < 0)
		detrace(1,"transmit_buf:  errno got set to %d\n", errno);
#endif

	if (rc < 0)
	   {
		if (htx_sp != NULL)
			(htx_sp->bad_writes)++;

		if (save_errno == EIO)
		   {
			rc = tr_stat(fdes, &write_status_s, htx_sp,
				TOK_ADAP_CHECK, 10, 1);
			if (rc < 0)
				return(mktu_rc(tucb_ptr->header.tu,
					LOG_ERR, TOK_WR1_ERR));
		
			return(mktu_rc(tucb_ptr->header.tu,
						TOK_ERR, rc));
		   }
		return(mktu_rc(tucb_ptr->header.tu,
					SYS_ERR, save_errno));
	   }

	if (rc != wr_frame_size)
	   {
		if (htx_sp != NULL)
			(htx_sp->bad_writes)++;

		return(mktu_rc(tucb_ptr->header.tu,
					LOG_ERR, WR_TRUNC_ERR));
	   }


#ifdef debugg
detrace(0,"transmit_buf:  Ready to try get-status 'til TX_DONE...\n");
#endif
	/*
	 * now, loop on get status until we get an acknowledgement
	 * that the transmit completed.
	 */
	if (rc = tr_stat(fdes, &write_status_s, htx_sp, CIO_TX_DONE,
			TX_STAT_ATTEMPTS, 1))
	   {
#ifdef debugg
	detrace(0,"transmit_buf:  getstatus failed with rc = %d\n", rc);
		if (rc < 0)
		   {
		detrace(0,"   so status code byte (errno) at hex %0x\n",
				write_status_s.code);
		   }
		else
		   {
			if (rc)
				detrace(0,
				"   so status code at hex %0x\n",
				write_status_s.code);
		   }
#endif
		if (htx_sp != NULL)
			(htx_sp->bad_writes)++;

		if (rc < 0)
		   {
			/*
			 * the tr_stat failed on its own ioctl
			 * so make the return code with the errno
			 * produced from within tr_stat that was sent
			 * back inside write_status_s.code.
			 */
			 return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
				TOK_TX1_ERR));
		   }
		else
		   {
			/*
			 * never got a CIO_TX_DONE, so lookup the
			 * first adapter status code returned in the
			 * write_status_s structure to make the return
			 * value.
			 */
			uncode = tr_uncode(&write_status_s);
			if (uncode < 0)
				return(mktu_rc(tucb_ptr->header.tu,
					LOG_ERR, TOK_TX1_ERR));

			return(mktu_rc(tucb_ptr->header.tu,
					TOK_ERR, uncode));
		   }
	   }
#ifdef debugg
	detrace(0,"transmit_buf:  Made it - got TX_DONE!\n");
#endif

	
	/*
	 * in case we're running from HTX, add the no. of
	 * bytes written.  Also, count good write.
	 */
	if (htx_sp != NULL)
	   {
		htx_sp->bytes_writ += wr_frame_size;
		htx_sp->good_writes++;
	   }
	return(0);
   }

/*****************************************************************************

receive_buf

*****************************************************************************/

int receive_buf (fdes, read_buf, rd_frame_size, tucb_ptr)
   int fdes;
   unsigned char *read_buf;
   long rd_frame_size;
   TUTYPE *tucb_ptr;
   {
	int i, rc;
	int save_errno, uncode;
	struct htx_data *htx_sp;
	struct status_block read_status_s;
	extern int mktu_rc();
	extern int tr_stat();
	extern int tr_uncode();

	/*
	 * set up a pointer to HTX data structure to
	 * increment counters in case tu was invoked by
	 * hardware exerciser.
	 */
	htx_sp = tucb_ptr->token_s.htx_sp;

	/*
	 * zero out read buffer.
	 */
	memset(read_buf, 0, rd_frame_size);

#ifdef debugg
	detrace(0,
	"receive_buf:  After read_buf set to 0s.  Before read loop\n");
	detrace(1,
	"receive_buf:  Since no TX DONE, let's sleep for 5 secs.\n");
	sleep(5);
#endif
	/*
	 * since the device doesn't delay, we'll loop on the
	 * read until we get some data.
	 */
	for (i = 0; i < READ_ATTEMPTS; i++)
	   {
#ifdef debugg
		detrace(!i,
		"receive_buf:  Right before READ...attempt %d\n", i);
#endif
		rc = read(fdes, read_buf, rd_frame_size);
		save_errno = errno;
		if (rc < 0)
		   {
#ifdef debugg
			detrace(0,
			"receive_buf:  read failed with rc %d, errno %d\n",
				rc, errno);
#endif
			if (htx_sp != NULL)
				(htx_sp->bad_reads)++;
			if (save_errno == EIO)
			   {
				rc = tr_stat(fdes, &read_status_s,
					htx_sp, TOK_ADAP_CHECK, 10, 1);
				if (rc < 0)
				   return(mktu_rc(tucb_ptr->header.tu,
						LOG_ERR, TOK_RD1_ERR));
				
				rc = tr_uncode(&read_status_s);
				if (rc < 0)
				   return(mktu_rc(tucb_ptr->header.tu,
						LOG_ERR, TOK_RD1_ERR));
				return(mktu_rc(tucb_ptr->header.tu,
					TOK_ERR, rc));
			   }
			return(mktu_rc(tucb_ptr->header.tu,
						SYS_ERR, errno));
		   }
		if (rc > 0)
			break;
	   }
	if (i == READ_ATTEMPTS)
	   {
#ifdef debugg
		detrace(0,"receive_buf:  hit max read attempts\n");
#endif
		/*
		 * never got read data within READ_ATTEMPTS.
		 * May need to do a get status for hardware failure for
		 * more info.
		 */
		rc = tr_stat(fdes, &read_status_s,
					htx_sp, TOK_ADAP_CHECK, 20, 6);
		
		/*
		 * we tried to check the present status of the adapter
		 * for some kind of hardware error, but the ioctl in
		 * tr_stat failed or didn't find the TOK_ADAP_CHECK code
		 * waiting so let's just report that the read failed.
		 */
		if (rc < 0)
		   {
			htx_sp->bad_others++;
			return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
							TOK_RD2_ERR));
		   } 
		
		/*
		 * ah, we did find a TOK_ADAP_CHECK code waiting so
		 * some kind of hardware or software error did occur
		 * so let's report this one.  First, call tr_uncode
		 * to get the actual adapter error code then use that
		 * to form the return value.
		 */
		uncode = tr_uncode(&read_status_s);
		if (uncode < 0)
			return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
							TOK_RD2_ERR));
		return(mktu_rc(tucb_ptr->header.tu, TOK_ERR, uncode));
	   }

	if (rc != rd_frame_size)
	   {
		if (htx_sp != NULL)
			(htx_sp->bad_reads)++;

		return(mktu_rc(tucb_ptr->header.tu,
					LOG_ERR, RD_TRUNC_ERR));
	   }

	if (htx_sp != NULL)
	   {
		(htx_sp->good_reads)++;
		htx_sp->bytes_read += rd_frame_size;
	   }

	return(0);
   }

/*****************************************************************************

tu002

*****************************************************************************/

int tu002 (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
	unsigned char *write_buf;
	unsigned char *read_buf;
	long wr_frame_size, rd_frame_size;
	struct status_block read_status_s;
	register int i;
	struct htx_data *htx_sp;
	int cc, rc, uncode, save_errno;
	int try_count;

	extern int errno;
	extern int tr_stat();
	extern int tr_uncode();
	extern int mktu_rc();

#ifdef debugg
	detrace(0,"\n\ntu002:  Begins...\n");
#endif
	/*
	 * set up a pointer to HTX data structure to
	 * increment counters in case tu was invoked by
	 * hardware exerciser.
	 */
	htx_sp = tucb_ptr->token_s.htx_sp;

	if (rc = init_write_buf(fdes, &write_buf, &wr_frame_size, tucb_ptr))
	   {
		return(rc);
	   }
	
	if (rc = init_read_buf(fdes, &read_buf, &rd_frame_size, wr_frame_size,
						tucb_ptr))
	   {
		free(write_buf);
		return(rc);
	   }

	for (try_count = 0; try_count < MAX_TRIES; try_count++)
	   {
		if (try_count > 0)
		   {
			rc = tr_stat(fdes, &read_status_s, htx_sp,
				CIO_ASYNC_STATUS, 20, 3);
			if (rc < 0)
			   {
				free(write_buf);
				free(read_buf);
				return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
					TOK_RD2_ERR));
			   }
			if (read_status_s.option[0] != CIO_NET_RCVRY_EXIT)
			   {
				free(write_buf);
				free(read_buf);
				return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
					ASY_CC_ERR));
			   }
		   }

		if (rc = transmit_buf(fdes, write_buf, wr_frame_size, tucb_ptr))
		   {
			free(write_buf);
			free(read_buf);
			return(rc);
		   }
		
		if (rc = receive_buf(fdes, read_buf, rd_frame_size, tucb_ptr))
		   {
			if ((rc & 0x0000ffff) != ASY_CC_ERR)
			   {
				free(write_buf);
				free(read_buf);
				return(rc);
			   }
		   }
		if (!rc)
			break;
	   } /* end max tries */
	
	if (try_count == MAX_TRIES)
	   {
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, ASY_CC_ERR));
	   }
	
#ifdef debugg
	detrace(0,"tu002:  Got a READ!!!!\n");
#endif
	/*
	 * if we've made it this far, we've done a successful
	 * write and read, so let's compare the data portion of
	 * the buffers to see if the data is identical...
	 * Again, note that we add an additional offset of
	 * RI_FIELD_LEN to the read buffer since the adapter
	 * adds pad bytes...
	 */
	
	i = 2 + (2 * NETADD_LEN);
	cc = memcmp(write_buf + i, read_buf + i + RI_FIELD_LEN,
			wr_frame_size - i);
#ifdef debugg
	if (cc)
	   {
		detrace(0, "tu002:  Compare failed to comp %d bytes!\n",
			wr_frame_size -i);
		detrace(0, "tu002:  First 30 of write, comp. starts %d.\n", i);
		for (i = 0; i < 30; i++)
		   {
			detrace(0, "%02x ", *(write_buf + i));
			if (i == 16)
				detrace(0, "\n");
		   }
		detrace(0, "tu002:  First 30 of read, comp starts %d.\n",
				i + RI_FIELD_LEN);
		for (i = 0; i < 30; i++)
		   {
			detrace(0, "%02x ", *(read_buf + i));
			if (i == 16)
				detrace(0, "\n");
		   }
	   }
	else
	   {
		detrace(0,"tu002:  READ compared!  Hoooray!!!\n");
	   }
#endif
	free(write_buf);
	free(read_buf);
	if (cc)
	   {
		htx_sp->bad_others++; 
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, MEM_CMP_ERR));
	   }

	return(0);
   }
