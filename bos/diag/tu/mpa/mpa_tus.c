static char sccsid[] = "@(#)44	1.1  src/bos/diag/tu/mpa/mpa_tus.c, mpatu, bos411, 9428A410j 4/30/93 12:21:12";
/*
 *   COMPONENT_NAME: (MPADIAG) MP/A DIAGNOSTICS
 *
 *   FUNCTIONS: init_pat
 *		tu001
 *		tu002
 *		tu003
 *		tu004
 *		tu005
 *		tu006
 *		tu007
 *		wait_status
 *		
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include <sys/types.h>
#include <sys/mode.h>
#include <sys/comio.h>
#include <sys/time.h>
#include <sys/poll.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/select.h>
#include <string.h>
#include <sys/dmpauser.h>
#include "mpa_tu.h"

#ifndef DIAGS
char                            *hxfcbuf();
#endif

/* ************************************************************************
**  Function: tu001()
**
**   This function tests all possible pos reg accesses to the mpa card.
**   POS 0... (Read only) read it and make sure it is 0xFF
**
**   POS 1... (Read only) read it and make sure it is 0xDE
**
**   POS 2... (Read/Write)  bit D7 not used always 1.
**                          bits D5 and D6 are read only.
**            1. Write 0xE1 to the register, read and compare to 0xE1
**            2. Write 0xE3 to the register, read and compare to 0xE3
**            3. Write 0xE5 to the register, read and compare to 0xE5
**            4. Write 0xE6 to the register, read and compare to 0xE9
**            5. Write 0xF1 to the register, read and compare to 0xF1
**            6. Write 0xF3 to the register, read and compare to 0xF3
**            7. Write 0xF5 to the register, read and compare to 0xF5
**            8. Write 0xF9 to the register, read and compare to 0xF9
**   NOTE: on the compare I don't look at bits D7, D6, and D5
**
**
**   POS 3... (Read/Write)  bits D4-D7 not used always 1.
**            1. Write 0xF0 to the register, read and compare to 0xF0
**            2. Write 0xF1 to the register, read and compare to 0xF1
**            3. Write 0xF2 to the register, read and compare to 0xF2
**            4. Write 0xF4 to the register, read and compare to 0xF4
**            5. Write 0xF8 to the register, read and compare to 0xF8
**
**
**************************************************************************/

tu001(fdes, tucb_ptr)
     int       fdes;
     TUTYPE    *tucb_ptr;
{
  rw_port_t data;
  int       rc=0;
  int       i;
  uchar     testval;

  /* the mpadd has an ioctl routine that we can use to read and write the
  ** cards pos regs. The ioctl is : MPA_RW_POS and it takes as input a
  ** rw_port_t structure ( found in mpauser.h ). There is no need to
  ** start the device before using the pos access ioctl.
  */

   sprintf(msg, "Start Pos reg 0 test.\n");
   RPT_BUGGER(msg);

#ifdef DIAGS
  /* get the origonal values of pos reg 2 and 3 */
  bzero(&data,sizeof(rw_port_t));     /* clear the input structure */
  data.port_addr = 0x102;        /* set up address for pos 2 */
  data.rw_flag |= MPA_READ;      /* tell driver to do read  for us */

  if (ioctl(fdes, MPA_RW_POS, &data)) {
	sprintf(msg, "save read pos 2 failed \n");
	RPT_BUGGER(msg);
	return(RC_SYS_ERRNO);
  }
  tucb_ptr->pr.pos2 = data.value;

  bzero(&data,sizeof(rw_port_t));     /* clear the input structure */
  data.port_addr = 0x103;        /* set up address for pos 2 */
  data.rw_flag |= MPA_READ;      /* tell driver to do read  for us */

  if (ioctl(fdes, MPA_RW_POS, &data)) {
	sprintf(msg, "save read pos 3 failed \n");
	RPT_BUGGER(msg);
	return(RC_SYS_ERRNO);
  }
  tucb_ptr->pr.pos3 = data.value;

#endif

  /*
  **  First read pos reg 0 and make sure it is 0xFF
  */
  bzero(&data,sizeof(rw_port_t));     /* clear the input structure */
  data.port_addr = 0x100;       /* set up address for pos 0 */
  data.rw_flag |= MPA_READ;     /* tell driver to do read for us */

  if (ioctl(fdes, MPA_RW_POS, &data)) {
       sprintf(msg, "Ioctl read pos reg 0 failed :  "
       "%s \n",sys_errlist[errno]);
	RPT_BUGGER(msg);
#ifndef DIAGS
	RPT_INFO(msg);
	hxfupdate(UPDATE, &ps);
#endif
	return(RC_SYS_ERRNO);
  }
  if(data.value != 0XFF) {
       sprintf(msg, "Got bad pos reg 0 value :  "
       "%02X, expected 0xFF",data.value);
	RPT_BUGGER(msg);
#ifndef DIAGS
	RPT_INFO(msg);
	hxfupdate(UPDATE, &ps);
#endif
	return(RC_SOFT);
  }

  sprintf(msg, "Start Pos reg 1 test.\n");
  RPT_BUGGER(msg);

  /*
  **  Now read pos reg 1 and make sure it is 0xDE
  */
  bzero(&data,sizeof(rw_port_t));     /* clear the input structure */
  data.port_addr = 0x101;       /* set up address for pos 1 */
  data.rw_flag |= MPA_READ;     /* tell driver to do read for us */

  if (ioctl(fdes, MPA_RW_POS, &data)) {
       sprintf(msg, "Ioctl read pos reg 1 failed :  "
       "%s \n",sys_errlist[errno]);
	RPT_BUGGER(msg);
#ifndef DIAGS
	RPT_INFO(msg);
#endif
	return(RC_SYS_ERRNO);
  }
  if(data.value != 0XDE) {
       sprintf(msg, "Got bad pos reg 1 value :  "
       "%02X, expected 0xDE",data.value);
	RPT_BUGGER(msg);
#ifndef DIAGS
	RPT_INFO(msg);
#endif
	return(RC_SOFT);
  }


  /*
  **  Now check the usable bits in pos reg 2.  **************************
  */

  /* first I need to disable the card for this entire test, since this
     test will alter the cards io addr and dma_lvl, I don't want it to
     interfear with the other card that may be running some other test */

       /* write testval then read and compare */
       bzero(&data,sizeof(rw_port_t));     /* clear the input structure */
       data.port_addr = 0x102;        /* set up address for pos 2 */
       data.rw_flag |= MPA_WRITE;     /* tell driver to do write for us */
       data.value = 0xF0;             /* turn off card enable.          */

       if (ioctl(fdes, MPA_RW_POS, &data)) {
	    sprintf(msg, "Ioctl write pos reg 2 failed :  "
	    "%s \n",sys_errlist[errno]);
	     RPT_BUGGER(msg);
#ifndef DIAGS
	     RPT_INFO(msg);
#endif
	     rc = RC_SYS_ERRNO;
	     goto pos2_done;
       }
       sprintf(msg, "Write reg 2 with %02X\n",0xF0);
       RPT_BUGGER(msg);



  for (i=0; i<8; i++) {
       switch(i) {
	 case 0: testval = 0xE0; break;
	 case 1: testval = 0xE2; break;
	 case 2: testval = 0xE4; break;
	 case 3: testval = 0xE8; break;
	 case 4: testval = 0xF0; break;
	 case 5: testval = 0xF2; break;
	 case 6: testval = 0xF4; break;
	 case 7: testval = 0xF8; break;
	 default: break;
       }
       /* write testval then read and compare */
       bzero(&data,sizeof(rw_port_t));     /* clear the input structure */
       data.port_addr = 0x102;        /* set up address for pos 2 */
       data.rw_flag |= MPA_WRITE;     /* tell driver to do write for us */
       data.value = testval;          /* set the value to pass to driver */

       if (ioctl(fdes, MPA_RW_POS, &data)) {
	    sprintf(msg, "Ioctl write pos reg 2 failed :  "
	    "%s \n",sys_errlist[errno]);
	     RPT_BUGGER(msg);
#ifndef DIAGS
	     RPT_INFO(msg);
#endif
	     rc = RC_SYS_ERRNO;
	     goto pos2_done;
       }
       sprintf(msg, "Write reg 2 with %02X\n",testval);
       RPT_BUGGER(msg);

       /* now read back the value just written */
       bzero(&data,sizeof(rw_port_t));     /* clear the input structure */
       data.port_addr = 0x102;        /* set up address for pos 2 */
       data.rw_flag |= MPA_READ;      /* tell driver to do read  for us */

       if (ioctl(fdes, MPA_RW_POS, &data)) {
	    sprintf(msg, "Ioctl read pos reg 2 failed :  "
	    "%s \n",sys_errlist[errno]);
	     RPT_BUGGER(msg);
#ifndef DIAGS
	     RPT_INFO(msg);
#endif
	     rc = RC_SYS_ERRNO;
	     goto pos2_done;
       }
       sprintf(msg, "Read back from reg 2 %02X\n",data.value);
       RPT_BUGGER(msg);

       if( (data.value&0x1F) != (testval&0x1F) ) {
	    sprintf(msg, "Got bad pos reg 2 value :  "
	    "%02X, expected %02X",(data.value&0x1F),(testval&0x1F));
	     RPT_BUGGER(msg);
#ifndef DIAGS
	     RPT_INFO(msg);
#endif
	     rc = RC_SOFT;
	     goto pos2_done;
       }

  }

pos2_done:

if(rc)  {
  /* restore pos 2 value */
  bzero(&data,sizeof(rw_port_t));     /* clear the input structure */
  data.port_addr = 0x102;        /* set up address for pos 2 */
  data.value = tucb_ptr->pr.pos2;          /* set origonal value              */
  data.rw_flag |= MPA_WRITE;     /* tell driver to do write for us */

  if (ioctl(fdes, MPA_RW_POS, &data)) {
       sprintf(msg, "Ioctl write pos reg 2 save value failed :  "
       "%s \n",sys_errlist[errno]);
	RPT_BUGGER(msg);
#ifndef DIAGS
	RPT_INFO(msg);
#endif
	return(RC_SYS_ERRNO);
  }
  /* test the pos2 to make sure its correct */
  bzero(&data,sizeof(rw_port_t));     /* clear the input structure */
  data.port_addr = 0x102;        /* set up address for pos 2 */
  data.rw_flag |= MPA_READ;     /* tell driver to do read for us */

  if (ioctl(fdes, MPA_RW_POS, &data)) {
       sprintf(msg, "Ioctl write pos reg 2 save value failed :  "
       "%s \n",sys_errlist[errno]);
	RPT_BUGGER(msg);
#ifndef DIAGS
	RPT_INFO(msg);
#endif
	return(RC_SYS_ERRNO);
  }
  if( data.value != tucb_ptr->pr.pos2 ) {
	sprintf(msg, "Expect %02x for pos2 got %02x\n",tucb_ptr->pr.pos2,data.value);
	RPT_BUGGER(msg);
#ifndef DIAGS
	RPT_INFO(msg);
#endif
	return(RC_SOFT);
  }

#ifndef DIAGS
   hxfupdate(UPDATE, &ps);
#endif
   return(rc);
}

  /*
  **  Now check the usable bits in pos reg 3.  **************************
  */

  for (i=0; i<5; i++) {
       switch(i) {
	 case 0: testval = 0xF0; break;
	 case 1: testval = 0xF1; break;
	 case 2: testval = 0xF2; break;
	 case 3: testval = 0xF4; break;
	 case 4: testval = 0xF8; break;
	 default: break;
       }
       /* write testval then read and compare */
       bzero(&data,sizeof(rw_port_t));     /* clear the input structure */
       data.port_addr = 0x103;        /* set up address for pos 3 */
       data.rw_flag |= MPA_WRITE;     /* tell driver to do write for us */
       data.value = testval;          /* set the value to pass to driver */

       if (ioctl(fdes, MPA_RW_POS, &data)) {
	    sprintf(msg, "Ioctl write pos reg 3 failed :  "
	    "%s \n",sys_errlist[errno]);
	     RPT_BUGGER(msg);
#ifndef DIAGS
	     RPT_INFO(msg);
#endif
	     rc = RC_SYS_ERRNO;
	     goto pos3_done;
       }
       sprintf(msg, "Write to Pos reg 3 %02X\n",testval);
       RPT_BUGGER(msg);

       /* now read back the value just written */
       bzero(&data,sizeof(rw_port_t));     /* clear the input structure */
       data.port_addr = 0x103;        /* set up address for pos 3 */
       data.rw_flag |= MPA_READ;      /* tell driver to do read  for us */

       if (ioctl(fdes, MPA_RW_POS, &data)) {
	    sprintf(msg, "Ioctl read pos reg 3 failed :  "
	    "%s \n",sys_errlist[errno]);
	     RPT_BUGGER(msg);
#ifndef DIAGS
	     RPT_INFO(msg);
#endif
	     rc = RC_SYS_ERRNO;
	     goto pos3_done;
       }
       sprintf(msg, "Read from Pos reg 3 %02X\n",data.value);
       RPT_BUGGER(msg);

       if( (data.value&0x0F) != (testval&0x0F) ) {
	    sprintf(msg, "Got bad pos reg 3 value :  "
	    "%02X, expected %02X",data.value,testval);
	     RPT_BUGGER(msg);
#ifndef DIAGS
	     RPT_INFO(msg);
#endif
	     rc = RC_SOFT;
	     goto pos3_done;
       }

  }

pos3_done:

  /* restore pos 3 value */
  bzero(&data,sizeof(rw_port_t));     /* clear the input structure */
  data.port_addr = 0x103;        /* set up address for pos 3 */
  data.value = tucb_ptr->pr.pos3;          /* set origonal value              */
  data.rw_flag |= MPA_WRITE;     /* tell driver to do write for us */

  if (ioctl(fdes, MPA_RW_POS, &data)) {
       sprintf(msg, "Ioctl write pos reg 3 save value failed :  "
       "%s \n",sys_errlist[errno]);
	RPT_BUGGER(msg);
#ifndef DIAGS
	hxfupdate(UPDATE, &ps);
	RPT_INFO(msg);
#endif
	return(RC_SYS_ERRNO);
  }

#ifndef DIAGS
  hxfupdate(UPDATE, &ps);
#endif

  /* restore pos 2 value */
  bzero(&data,sizeof(rw_port_t));     /* clear the input structure */
  data.port_addr = 0x102;        /* set up address for pos 2 */
  data.value = tucb_ptr->pr.pos2;          /* set origonal value              */
  data.rw_flag |= MPA_WRITE;     /* tell driver to do write for us */

  if (ioctl(fdes, MPA_RW_POS, &data)) {
       sprintf(msg, "Ioctl write pos reg 2 save value failed :  "
       "%s \n",sys_errlist[errno]);
	RPT_BUGGER(msg);
#ifndef DIAGS
	RPT_INFO(msg);
#endif
	return(RC_SYS_ERRNO);
  }
  /* test the pos2 to make sure its correct */
  bzero(&data,sizeof(rw_port_t));     /* clear the input structure */
  data.port_addr = 0x102;        /* set up address for pos 2 */
  data.rw_flag |= MPA_READ;     /* tell driver to do read for us */

  if (ioctl(fdes, MPA_RW_POS, &data)) {
       sprintf(msg, "Ioctl write pos reg 2 save value failed :  "
       "%s \n",sys_errlist[errno]);
	RPT_BUGGER(msg);
#ifndef DIAGS
	RPT_INFO(msg);
#endif
	return(RC_SYS_ERRNO);
  }
  if( data.value != tucb_ptr->pr.pos2 ) {
	sprintf(msg, "Expect %02x for pos2 got %02x\n",tucb_ptr->pr.pos2,data.value);
	RPT_BUGGER(msg);
#ifndef DIAGS
	RPT_INFO(msg);
#endif
	return(RC_SOFT);
  }


  return(rc);
}      /* tu001() */


/* ************************************************************************
**  Function: tu002()
**
**   PIO DIAGNOSTIC EXTERNAL WRAP TEST FOR SDLC/HDLC COMMUNICATION
**
**   This test requires a wrap plug either on the card or at the end
**   of the cable. The test uses PIO to send data out and read it back
**   one byte at a time and at up to 1200 bits per sec.
**
**   This TU will also do internal wrap test that does no require a wrap
**   plug.
**
**************************************************************************/

tu002(fdes, tucb_ptr)
     int       fdes;
     TUTYPE    *tucb_ptr;
{
   int             rc=RC_GOOD;     /* init the routines return code */
   int             src;            /* return code for select call */
   int             dev_started=0;  /* set after the device is started */
   int             loop_count=0;   /* counts loops executed */
   mpa_start_t     start_st;       /* structure passed to ioctl CIO_START */
   int             number=64;      /* used to set seed for rnadom generator */
   t_write_ext     we;             /* mpa's write structure */
   cio_read_ext_t  re;             /* mpa's read structure */
   cio_stat_blk_t  status;         /* status structure for CIO_GET_STAT */
   int             bytesw;         /* return value from writex system call */
   int             bytesr;         /* return value from readx system call */
   int             bytes;          /* number of bytes chosen for each loop */
   frame_t         writbuf;        /* write data buffer for sdlc frames */
   frame_t         readbuf;        /* read data buffer for sdlc frames */
   char            *state;         /* used for random byte count generation */
   fd_set          rd, wr, ex;     /* select set structures */
   struct timeval  timeout;        /* timeout value for the select calls */
   int             pio_retries = 0;

   /* the device was opened in hxempa.c.. here I will start the device */
   /* using the input from the rules files and run the test for the    */
   /* specified number of loops then halt the device before returning  */

   sprintf(msg, "Start TU002..total loops %d, bps %d \n",tucb_ptr->pr.num_oper,tucb_ptr->pr.bps);
   RPT_BUGGER(msg);

   /* read in the pattern file and set the pointer to the pattern data */
   if( (rc=init_pat(tucb_ptr)) ) {
      sprintf(msg,"Failed to init pattern "
		"in mpa_tus\n");
      RPT_BUGGER(msg);
#ifndef DIAGS
      RPT_INFO(msg);
#endif
      return(rc);
   }

   bzero(&start_st,sizeof(mpa_start_t));

   start_st.data_proto |= SDLC;
   start_st.modem_flags |= GATE_INT_CLK;  /* wrap requires internal clock */
   start_st.baud_rate = tucb_ptr->pr.bps;           /* set up bps rate for wrap */
   if(tucb_ptr->pr.nrzi) start_st.data_flags |= SET_NRZI_DATA;
   if(!tucb_ptr->pr.wrap_type) {
      /* if internal wrap is desired */
      start_st.data_flags |= SET_CLOCK_LOOPBACK;
      start_st.data_flags |= SET_DATA_LOOPBACK;
   }

   start_st.xfer_mode |= SET_NO_DMA;      /* PIO required for wrap */
   start_st.station_type |= PRIMARY;      /* set up as primary station */
   start_st.sb.netid = 0;

   if (ioctl(fdes, CIO_START, &start_st) == -1) {
       sprintf(msg,"ioctl, CIO_START "
       "failed %d %s\n",errno,sys_errlist[errno]);
       RPT_BUGGER(msg);
#ifndef DIAGS
       RPT_INFO(msg);
       INC_BAD_OTHERS;
#endif
       rc = RC_SYS_ERRNO;
       goto end_tu2;
   }

   if( (rc = wait_status(fdes,CIO_START_DONE, tucb_ptr)) ) {
       sprintf(msg,"Wait START DONE failed: rc = %d\n",rc);
       RPT_BUGGER(msg);
#ifndef DIAGS
       RPT_INFO(msg);
       INC_BAD_OTHERS;
#endif
       goto end_tu2;
   }

   dev_started = 1;
   /* now the device is started to do the loop wraps */

#ifndef DIAGS
   if(!tucb_ptr->pr.fix_bytes) {
       state = malloc(number);
       initstate(tucb_ptr->pr.seed,state,number);
       setstate(state);
   }
#endif

   we.cio_ext.flag = CIO_ACK_TX_DONE;
   we.cio_ext.write_id = 0XFF;
   we.cio_ext.netid = 0;

   writbuf.addr = 0xFF;
   writbuf.cntl = 0x00;
   memcpy(writbuf.data,pat_ptr,tucb_ptr->pr.recsize);

   while(loop_count++ < tucb_ptr->pr.num_oper ) {
       sprintf(msg, "Start loop %d\n",loop_count);
       RPT_BUGGER(msg);

       if(tucb_ptr->pr.fix_bytes) bytes = tucb_ptr->pr.recsize;
       else {
	   bytes = random();
	   while(1) {
	     if(bytes > tucb_ptr->pr.recsize) {
	       bytes = bytes/3;
	       if(bytes <= 0) {
		 bytes = bytes + (tucb_ptr->pr.recsize/2);
		 break;
	      }
	     }
	     else break;
	   }
       }

pio_write:
       bytesw = writex(fdes,&writbuf,bytes+2,&we);
       if (bytesw == -1) {
	   sprintf(msg,"Write failed %d %s\n",fdes,sys_errlist[errno]);
	   RPT_BUGGER(msg);
#ifndef DIAGS
	   RPT_INFO(msg);
	   ++ps.bad_writes;
#endif
	   rc = RC_SYS_ERRNO;
	   goto end_tu2;
       }
       /* wait for CIO_TX_DONE status */
       if( (rc = wait_status(fdes,CIO_TX_DONE, tucb_ptr)) ) {
	   sprintf(msg,"Wait XMIT DONE failed: rc = %d\n",rc);
	   RPT_BUGGER(msg);
#ifndef DIAGS
	   RPT_INFO(msg);
	   INC_BAD_OTHERS;
#endif
	   goto end_tu2;
       }

       sprintf(msg, "Wrote %d bytes\n",bytesw);
       RPT_BUGGER(msg);
#ifndef DIAGS
       ++ps.good_writes;
#endif
       pio_retries = 0;
       while(1) {
	  FD_ZERO(&rd);
	  FD_ZERO(&wr);
	  FD_ZERO(&ex);
	  FD_SET(fdes, &ex) ;   /* wait for excp */
	  FD_SET(fdes, &rd) ;   /* wait to read  */
	  timeout.tv_sec = READ_TIMEOUT;
	  timeout.tv_usec = 0;
	  src = select(FD_SETSIZE, &rd, &wr, &ex, &timeout);
	  if (src == -1) {
	      sprintf(msg,"Select fail %d %s\n",fdes,sys_errlist[errno]);
	      RPT_BUGGER(msg);
#ifndef DIAGS
	      RPT_INFO(msg);
	      INC_BAD_OTHERS;
#endif
	      rc = RC_SYS_ERRNO;
	      goto end_tu2;
	  }
	  if (src == 0) {
	      sprintf(msg,"Read timeout \n");
	      RPT_BUGGER(msg);
#ifndef DIAGS
	      RPT_INFO(msg);
	      INC_BAD_OTHERS;
#endif
	      rc = RC_SOFT;
	      goto end_tu2;
	  }
	  if (FD_ISSET(fdes,&ex)) {
	      while(1) {          /* loop till null status received */
		if (ioctl(fdes, CIO_GET_STAT, &status)) {
		     sprintf(msg,"IOCTL get_stat %d %s\n",fdes,sys_errlist[errno]);
		     RPT_BUGGER(msg);
#ifndef DIAGS
		     RPT_INFO(msg);
		     INC_BAD_OTHERS;
#endif
		     rc = RC_SYS_ERRNO;
		     goto end_tu2;
		}
		if(status.code == CIO_NULL_BLK) break;
#ifndef DIAGS
		else if( (rc = decode_status(&status,tucb_ptr)) ) {
		     if(rc == RC_HARD) goto end_tu2;
		     rc = 0;
		     goto pio_write;
		}
#else
		rc = RC_HARD;
		goto end_tu2;

#endif

	      }  /* end of loop on get status to clean status q */
	  }
	  if (FD_ISSET(fdes,&rd)) break;
       }           /* end while(1) loop */

       /* do read */
       bzero(&readbuf,sizeof(frame_t));
       bytesr = readx(fdes,&readbuf,MAXBYTES,&re);
       if (bytesr == -1) {
	   sprintf(msg,"READ failed %d %s\n",fdes,sys_errlist[errno]);
	   RPT_BUGGER(msg);
#ifndef DIAGS
	   RPT_INFO(msg);
	   ++ps.bad_reads;
#endif
	   rc = RC_SYS_ERRNO;
	   goto end_tu2;
       }
       if(bytesr!=bytesw) {
	   sprintf(msg,"Expected %d bytes only received %d bytes\n",bytesw,bytesr);
	   RPT_BUGGER(msg);
#ifndef DIAGS
	   RPT_INFO(msg);
	   ++ps.bad_reads;
#endif
	   rc = RC_SOFT;
	   goto end_tu2;
       }

       if (memcmp(&readbuf,&writbuf,bytesw) != 0) {
	  sprintf(msg,"READ: failed data verification! \n");
	  RPT_BUGGER(msg);
#ifndef DIAGS
	  RPT_INFO(msg);
	  strcpy(msg,hxfcbuf(&ps, &writbuf, &readbuf,(bytesw)));
	  ++ps.bad_reads;
	  RPT_INFO(msg);
	  RPT_BUGGER(msg);
#endif
	  rc = RC_SOFT;
	  goto end_tu2;
      }
      sprintf(msg, "Read %d bytes\n",bytesr);
      RPT_BUGGER(msg);

#ifndef DIAGS
      ++ps.good_reads;
      hxfupdate(UPDATE, &ps);     /* update htx stats each loop */
#endif
   }


end_tu2:

   if(dev_started) {
       bzero(&start_st,sizeof(mpa_start_t));
       start_st.sb.netid = 0;
       if (ioctl(fdes, CIO_HALT, &start_st) == -1) {
	   sprintf(msg,"Write failed %d %s\n",fdes,sys_errlist[errno]);
	   RPT_BUGGER(msg);
#ifndef DIAGS
	   RPT_INFO(msg);
	   INC_BAD_OTHERS;
#endif
	   free(pat_ptr);              /* free up the pattern data space */
	   return(RC_SYS_ERRNO);
       }
       if( wait_status(fdes,CIO_HALT_DONE, tucb_ptr) ) {
	   sprintf(msg,"Wait for HALT_DONE failed\n");
	   RPT_BUGGER(msg);
#ifndef DIAGS
	   RPT_INFO(msg);
	   INC_BAD_OTHERS;
#endif
	   free(pat_ptr);              /* free up the pattern data space */
	   return(RC_SYS_ERRNO);
       }
   }
   free(pat_ptr);              /* free up the pattern data space */
   return(rc);
}    /* tu002()  pio wrap test */

/* ************************************************************************
**  Function: tu003()
**
**   DMA DIAGNOSTIC EXTERNAL WRAP TEST FOR SDLC/HDLC COMMUNICATION
**
**   Good test to use in EMC lab for full speed data traffic on the cable.
**   This test requires a wrap plug on the card D-shell or at the
**   end of the cable. This is not a true wrap test. It is designed
**   for putting dma data on the cable for EMC testing. Data is
**   only transmited by the CPU, NO data is received, by CPU, in this DMA test.
**   This will allow bps rates of up to 19200 on the cable.
**   See TU 7 for KEYWORD description.
**   All defaults are the same except for the BPS keyword..........
**                                            its default and max is 19200
**
**
**************************************************************************/

tu003(fdes, tucb_ptr)
     int       fdes;
     TUTYPE    *tucb_ptr;
{
   int             rc=RC_GOOD;     /* init the routines return code */
   int             src;            /* return code for select call */
   int             dev_started=0;  /* set after the device is started */
   int             loop_count=0;   /* counts loops executed */
   mpa_start_t     start_st;       /* structure passed to ioctl CIO_START */
   int             number=64;      /* used to set seed for rnadom generator */
   t_write_ext     we;             /* mpa's write structure */
   cio_read_ext_t  re;             /* mpa's read structure */
   cio_stat_blk_t  status;         /* status structure for CIO_GET_STAT */
   int             bytesw;         /* return value from writex system call */
   int             bytesr;         /* return value from readx system call */
   int             bytes;          /* number of bytes chosen for each loop */
   frame_t         writbuf;        /* write data buffer for sdlc frames */
   frame_t         readbuf;        /* read data buffer for sdlc frames */
   char            *state;         /* used for random byte count generation */
   fd_set          rd, wr, ex;     /* select set structures */
   struct timeval  timeout;        /* timeout value for the select calls */
   char            byte[1];
   int             i;

   /* the device was opened in hxempa.c.. here I will start the device */
   /* using the input from the rules files and run the test for the    */
   /* specified number of loops then halt the device before returning  */

   sprintf(msg, "Start TU003..total loops %d, bps %d \n",tucb_ptr->pr.num_oper,tucb_ptr->pr.bps);
   RPT_BUGGER(msg);

   /* read in the pattern file and set the pointer to the pattern data */
   if( (rc=init_pat(tucb_ptr)) ) {
      sprintf(msg,"Failed to init pattern "
		"in mpa_tus\n");
      RPT_BUGGER(msg);
#ifndef DIAGS
      RPT_INFO(msg);
#endif
      return(rc);
   }

   bzero(&start_st,sizeof(mpa_start_t));

   start_st.data_proto |= SDLC;
   start_st.modem_flags |= GATE_INT_CLK;  /* wrap requires internal clock */
   start_st.baud_rate = tucb_ptr->pr.bps; /* set up bps rate for wrap */
   if(!tucb_ptr->pr.wrap_type) {
      /* if internal wrap is desired */
      start_st.data_flags |= SET_CLOCK_LOOPBACK;
      start_st.data_flags |= SET_DATA_LOOPBACK;
   }
   if(tucb_ptr->pr.nrzi) start_st.data_flags |= SET_NRZI_DATA;
   start_st.diag_flags |= XMIT_ONLY;      /* Set up transmit only */
   start_st.station_type |= PRIMARY;      /* set up as primary station */
   start_st.sb.netid = 0;

   if (ioctl(fdes, CIO_START, &start_st) == -1) {
       sprintf(msg,"ioctl, CIO_START "
       "failed %d %s\n",errno,sys_errlist[errno]);
       RPT_BUGGER(msg);
#ifndef DIAGS
       RPT_INFO(msg);
       INC_BAD_OTHERS;
#endif
       rc = RC_SYS_ERRNO;
       goto end_tu3;
   }

   if( (rc = wait_status(fdes,CIO_START_DONE, tucb_ptr)) ) {
       sprintf(msg,"Wait START DONE failed: rc = %d\n",rc);
       RPT_BUGGER(msg);
#ifndef DIAGS
       RPT_INFO(msg);
       INC_BAD_OTHERS;
#endif
       goto end_tu3;
   }

   dev_started = 1;
   /* now the device is started to do the loop wraps */

#ifndef DIAGS
   if(!tucb_ptr->pr.fix_bytes) {
       state = malloc(number);
       initstate(tucb_ptr->pr.seed,state,number);
       setstate(state);
   }
#endif
   we.cio_ext.flag = CIO_ACK_TX_DONE;
   we.cio_ext.write_id = 0XFF;
   we.cio_ext.netid = 0;

   writbuf.addr = 0xFF;
   writbuf.cntl = 0x00;
   memcpy(writbuf.data,pat_ptr,tucb_ptr->pr.recsize);

   while(loop_count++ < tucb_ptr->pr.num_oper ) {

       sprintf(msg, "Start loop %d\n",loop_count);
       RPT_BUGGER(msg);

       if(tucb_ptr->pr.fix_bytes) bytes = tucb_ptr->pr.recsize;
       else {
	   bytes = random();
	   while(1) {
	     if(bytes > tucb_ptr->pr.recsize) {
	       bytes = bytes/3;
	       if(bytes <= 0) {
		 bytes = bytes + (tucb_ptr->pr.recsize/2);
		 break;
	      }
	     }
	     else break;
	   }
       }
       /* wait for ok to wirte from driver and check for exception */
       while(1) {
	  FD_ZERO(&rd);
	  FD_ZERO(&wr);
	  FD_ZERO(&ex);
	  FD_SET(fdes, &ex) ;   /* wait for excp */
	  FD_SET(fdes, &wr) ;   /* wait to read  */
	  timeout.tv_sec = WRITE_TIMEOUT;
	  timeout.tv_usec = 0;
	  src = select(FD_SETSIZE, &rd, &wr, &ex, &timeout);
	  if (src == -1) {
	      sprintf(msg,"Select fail %d %s\n",fdes,sys_errlist[errno]);
	      RPT_BUGGER(msg);
#ifndef DIAGS
	      RPT_INFO(msg);
	      INC_BAD_OTHERS;
#endif
	      rc = RC_SYS_ERRNO;
	      goto end_tu3;
	  }
	  if (src == 0) {
	      sprintf(msg,"Write timeout \n");
	      RPT_BUGGER(msg);
#ifndef DIAGS
	      RPT_INFO(msg);
	      INC_BAD_OTHERS;
#endif
	      rc = RC_SOFT;
	      goto end_tu3;
	  }
	  if (FD_ISSET(fdes,&ex)) {
	      while(1) {          /* loop till null status received */
		if (ioctl(fdes, CIO_GET_STAT, &status)) {
		     sprintf(msg,"IOCTL get_stat %d %s\n",fdes,sys_errlist[errno]);
		     RPT_BUGGER(msg);
#ifndef DIAGS
		     RPT_INFO(msg);
		     INC_BAD_OTHERS;
#endif
		     rc = RC_SYS_ERRNO;
		     goto end_tu3;
		}
		if(status.code == CIO_NULL_BLK) break;
#ifndef DIAGS
		else if( (rc = decode_status(&status,tucb_ptr)) ) {
		     if(rc == RC_HARD) goto end_tu3;
		     rc = 0;
		     continue;
		}
#endif

	      }  /* end of loop on get status to clean status q */
	  }
	  if (FD_ISSET(fdes,&wr)) break;
       }           /* end while(1) loop waiting for write or exception */

       bytesw = writex(fdes,&writbuf,bytes+2,&we);
       if (bytesw == -1) {
	   sprintf(msg,"Write failed %d %s\n",fdes,sys_errlist[errno]);
	   RPT_BUGGER(msg);
#ifndef DIAGS
	   RPT_INFO(msg);
	   ++ps.bad_writes;
#endif
	   rc = RC_SYS_ERRNO;
	   goto end_tu3;
       }
       /* wait for CIO_TX_DONE status */
       if( (rc = wait_status(fdes,CIO_TX_DONE, tucb_ptr)) ) {
	   sprintf(msg,"Wait XMIT DONE failed: rc = %d\n",rc);
	   RPT_BUGGER(msg);
#ifndef DIAGS
	   RPT_INFO(msg);
	   INC_BAD_OTHERS;
#endif
	   goto end_tu3;
       }

       sprintf(msg, "Wrote %d bytes\n",bytesw);
       RPT_BUGGER(msg);

#ifndef DIAGS
       ++ps.good_writes;
       hxfupdate(UPDATE, &ps);     /* update htx stats each loop */
#endif
   }


end_tu3:

   if(dev_started) {
       bzero(&start_st,sizeof(mpa_start_t));
       start_st.sb.netid = 0;
       if (ioctl(fdes, CIO_HALT, &start_st) == -1) {
	   sprintf(msg,"Write failed %d %s\n",fdes,sys_errlist[errno]);
	   RPT_BUGGER(msg);
#ifndef DIAGS
	   RPT_INFO(msg);
	   INC_BAD_OTHERS;
#endif
	   free(pat_ptr);              /* free up the pattern data space */
	   return(RC_SYS_ERRNO);
       }

       if( wait_status(fdes,CIO_HALT_DONE, tucb_ptr) ) {
	   sprintf(msg,"Wait for HALT_DONE failed\n");
	   RPT_BUGGER(msg);
#ifndef DIAGS
	   RPT_INFO(msg);
	   INC_BAD_OTHERS;
#endif
	   free(pat_ptr);              /* free up the pattern data space */
	   return(RC_SYS_ERRNO);
       }
   }
   free(pat_ptr);              /* free up the pattern data space */
   return(rc);
}   /* tu003() */

#ifndef DIAGS
/* ************************************************************************
**  Function: tu004()
**
**    DMA FUNCTIONAL CARD TO CARD TEST FOR SDLC/HDLC COMMUNICATION
**
**   This test requires two cards connected together through a
**   modem eliminator or a special cable connector that has
**   RTS (Request to Send) tied to CTS (Clear to Send)
**   CD (Carrier Detect). The cards do not have to be in the
**   same system but they can be if you desire.
**      A. When using modem eliminator you can either use the
**         modem clock (set CLOCK keyword to 0) or the internal
**         clock (set CLOCK keyword to 1) and specify
**         BPS (Bits Per Second) keyword to set the data xfer rate.
**      B. If you use the special cable connector you must
**         use the internal clock and set the BPS rate.
**   IMPORTANT: When running this test with interal clocking (that
**              is the data transfer clock is supplied by a counter
**              on the MPA cards) , The rules files for both cards
**              must contain only TU-004 rule and the clock bit rate
**              must be be the same in both machines rules files.
**              THIS TEST IS INCOMPATIBLE WITH ANY OTHER TU.
**
**
**************************************************************************/

tu004(fdes, tucb_ptr)
     int       fdes;
     TUTYPE    *tucb_ptr;
{
   int             rc=RC_GOOD;     /* init the routines return code */
   int             src;            /* return code for select call */
   int             loop_count=0;   /* counts loops executed */
   mpa_start_t     start_st;       /* structure passed to ioctl CIO_START */
   int             number=64;      /* used to set seed for rnadom generator */
   char            *state;         /* used for random byte count generation */
   t_write_ext     we;             /* mpa's write structure */
   cio_read_ext_t  re;             /* mpa's read structure */
   cio_stat_blk_t  status;         /* status structure for CIO_GET_STAT */
   int             bytesw;         /* return value from writex system call */
   int             bytesr;         /* return value from readx system call */
   int             bytes;          /* number of bytes chosen for each loop */
   int             bids_made=0;    /* number of bids for master system     */
   int             note_inc=20;    /* number of read wait times before HTX */
				   /* is UPDATED.                          */
   frame_t         writbuf;        /* write data buffer for sdlc frames */
   frame_t         readbuf;        /* read data buffer for sdlc frames */
   fd_set          rd, wr, ex;     /* select set structures */
   struct timeval  timeout;        /* timeout value for the select calls */
   int             start_test=0;   /* tells code when to start the test */
   int             tmp_bid;
   int             retry_count = 0;
   time_t          ltime;

   /* the device was opened in hxempa.c.. here I will start the device */
   /* using the input from the rules files and run the test for the    */
   /* specified number of loops then halt the device before returning  */

   if(bugit) {
	sprintf(msg, "Start TU004..total loops %d, bps %d \n",tucb_ptr->pr.num_oper,tucb_ptr->pr.bps);
	RPT_BUGGER(msg);
   }


   /* read in the pattern file and set the pointer to the pattern data */
   if( (rc=init_pat(tucb_ptr)) ) {
      sprintf(msg,"Failed to init pattern "
		"in mpa_tus\n");
      RPT_INFO(msg);
      RPT_BUGGER(msg);
      return(rc);
   }

   if(!tucb_ptr->pr.fix_bytes) {
      state = malloc(number);
      initstate(tucb_ptr->pr.seed,state,number);
      setstate(state);
  }

   we.cio_ext.flag = 0;
   we.cio_ext.write_id = 0XFF;
   we.cio_ext.netid = 0;

if(!master_set) {

   bzero(&start_st,sizeof(mpa_start_t));

   start_st.data_proto |= SDLC;
   if(tucb_ptr->pr.clock) {
	 start_st.modem_flags |= GATE_INT_CLK;
	 start_st.mode_reg_flags |= SET_PRE_FRAME_MODE;
   }
   else start_st.modem_flags |= GATE_EXT_CLK;
   start_st.baud_rate = tucb_ptr->pr.bps;           /* set up bps rate for wrap */
   if(tucb_ptr->pr.nrzi) start_st.data_flags |= SET_NRZI_DATA;
   start_st.station_type |= PRIMARY;      /* set up as primary station */
   start_st.sb.netid = 0;

   if (ioctl(fdes, CIO_START, &start_st) == -1) {
       sprintf(msg,"ioctl, CIO_START "
       "failed %d %s\n",errno,sys_errlist[errno]);
       RPT_INFO(msg);
       RPT_BUGGER(msg);
       INC_BAD_OTHERS;
       rc = RC_SYS_ERRNO;
       goto end_tu4;
   }

   if( (rc = wait_status(fdes,CIO_START_DONE, tucb_ptr)) ) {
       sprintf(msg,"Wait START DONE failed: rc = %d\n",rc);
       RPT_INFO(msg);
       RPT_BUGGER(msg);
       INC_BAD_OTHERS;
       goto end_tu4;
   }

   /* now the device is started to do the loop wraps */


   /*
   ** for this test the two cards must extablish which one is to be the
   ** master. This is the porcedure:
   ** Each cards code will get the current time and set the secs value
   ** in writbuf.cntl. ( Hopefully the two machines will never have the
   ** same secs value). Then each cards code will xmit a bid frame with
   ** its writbuf.cntl value and wait for a responce. If no responce comes
   ** within the timeout value, it will xmit its bid packet again and
   ** continue this until it receives a responce or forever whichever
   ** comes first. The HTX screen will be updated every 2 minutes during
   ** this time to keep HTX from declaring us hung.
   **      When the code recieves a responce it will check to see if the
   ** readbuf.cntl value equals its writbuf.cntl value. If it does then
   ** both cards had the same time and a RC_HARD stop will occur.
   ** If writbuf.cntl != readbuf.cntl then the code will compare its
   ** writbuf.cntl to the readbuf.cntl  and if writbuf.cntl > readbuf.cntl
   ** This system is master. It will then send one more bid packet to
   ** confirm itself as master and wait for the slave to send back a
   ** frame with addr of EF which will confirm this system as master.
   ** The master will then send a frame with addr = 0xCF to the slave to
   ** tell it to prepare for the start of the test.
   ** To code will then begin the master slave test as master.
   ** As master the code will write random amounts of data and compare
   ** the recieved data to what was sent.
   **      If writbuf.cntl < readbuf.cntl this code will take up the
   ** salves roll by sending writbuf.addr = 0xEF frame to the master.
   ** Then begins the test by waiting for the master to send it
   ** data. When It receives data from the master it will just wrap it back
   ** to the master.
   */

   writbuf.addr = 0xBD;                      /* set this as bid frame */
   writbuf.cntl = (uchar) getpid();          /* put random num in cntl */

   /* I need to set up different read wait times so the two cards are */
   /* not in sync when they run on the same machine, otherwise neither */
   /* card will ever win the bid, becuase they will both be transmitting */
   /* at the same time.  Use the process id to set the bid_wait time.    */
   if(!bid_wait) {     /* on first pass, first rule */
     bid_wait = writbuf.cntl & 15;
     if (bid_wait >= 8) bid_wait = bid_wait - 7;
     if ( bid_wait >= 4 ) bid_wait = bid_wait - 3;
   }
   memcpy(writbuf.data,pat_ptr,tucb_ptr->pr.recsize);  /* copy in the data    */
   bytes = 10;                               /* send small bid frames */

   /* start the bid routine in a do forever loop */
   while(1) {
bid_write:
       bytesw = writex(fdes,&writbuf,bytes,&we);
       if (bytesw == -1) {
	   sprintf(msg,"Write failed %d %s\n",fdes,sys_errlist[errno]);
	   RPT_INFO(msg);
	   RPT_BUGGER(msg);
	   ++ps.bad_writes;
	   rc = RC_SYS_ERRNO;
	   goto end_tu4;
       }
       if(bugit) {
	   sprintf(msg, "Wrote %d bytes...addr %02X, bid write cntl %02X\n",bytesw,writbuf.addr,writbuf.cntl);
	   RPT_BUGGER(msg);
       }

       ++ps.good_writes;
       ps.bytes_writ += bytesw;
       totbytes += bytesw;
       if(start_test) break;

       while(1) {
	  FD_ZERO(&rd);
	  FD_ZERO(&wr);
	  FD_ZERO(&ex);
	  FD_SET(fdes, &ex) ;   /* wait for excp */
	  FD_SET(fdes, &rd) ;   /* wait to read  */
	  if(bid_wait < 30) {
	      timeout.tv_sec = bid_wait;
	      timeout.tv_usec = 0;
	  }
	  else {
	      timeout.tv_sec = 0;
	      timeout.tv_usec = bid_wait;
	  }
	  src = select(FD_SETSIZE, &rd, &wr, &ex, &timeout);
	  if (src == -1) {
	      sprintf(msg,"Select fail %d %s\n",fdes,sys_errlist[errno]);
	      RPT_INFO(msg);
	      RPT_BUGGER(msg);
	      INC_BAD_OTHERS;
	      rc = RC_SYS_ERRNO;
	      goto end_tu4;
	  }
	  if (src == 0)  goto read_timeout;
	  if (FD_ISSET(fdes,&ex)) {
	      while(1) {          /* loop till null status received */
		if (ioctl(fdes, CIO_GET_STAT, &status)) {
		     sprintf(msg,"IOCTL get_stat %d %s\n",fdes,sys_errlist[errno]);
		     RPT_INFO(msg);
		     RPT_BUGGER(msg);
		     INC_BAD_OTHERS;
		     rc = RC_SYS_ERRNO;
		     goto end_tu4;
		}
		if(status.code == CIO_NULL_BLK) break;
		else if( (rc = decode_status(&status,tucb_ptr)) ) {
		     if(rc == RC_HARD) goto end_tu4;
		     rc = 0;
		     goto bid_write;
		}

	      }  /* end of loop on get status to clean status q */
	  }
	  if (FD_ISSET(fdes,&rd)) break;
       }           /* end while(1) loop */

       /* do read */
       bytesr = readx(fdes,&readbuf,MAXBYTES,&re);
       if (bytesr == -1) {
	   sprintf(msg,"READ failed %d %s\n",fdes,sys_errlist[errno]);
	   RPT_INFO(msg);
	   RPT_BUGGER(msg);
	   ++ps.bad_reads;
	   rc = RC_SYS_ERRNO;
	   goto end_tu4;
       }
       if(bugit) {
	   sprintf(msg, "Read %d bytes...addr %02X, bid write cntl %02X\n",bytesr,readbuf.addr,readbuf.cntl);
	   RPT_BUGGER(msg);
       }

       ++ps.good_reads;
       ps.bytes_read += bytesr;
       totbytes += bytesr;

       if(master_set) {
	   /* send the bid again, I need to get readbuf.addr == 0xEF */
	   /* before I can continue test     */
	   if(bugit) {
	       sprintf(msg,"Got read data after I set master, addr %02X..cntl %02X\n"
			  ,readbuf.addr,readbuf.cntl);
	       RPT_BUGGER(msg);
	   }
	   if( (readbuf.addr==0xEF) && master ) {
	       writbuf.addr = 0xCF;
	       start_test = 1;
	   }
	   if( (readbuf.addr==0xCF) && !master) break;
	   if( (readbuf.addr==0xDA) && !master) {
	      loop_count++;
	      goto slave_write;
	   }
       }
       else {
	   if(writbuf.cntl == readbuf.cntl) {
	       sprintf(msg,"The other machine has same control field "
	       "write %02X, read %02X, addr %02X\n",writbuf.cntl,readbuf.cntl,readbuf.addr);
	       RPT_INFO(msg);
	       RPT_BUGGER(msg);
	       writbuf.cntl = ps.good_reads-1;
	       continue;
	   }
	   if(writbuf.cntl > readbuf.cntl) {   /* I am the master */
	       /* send bid until I get readbuf.addr == 0xEF */
	       master_set = 1;    /* tell code master has been decided */
	       master = 1;        /* set this code as master  */
	       bid_wait= 500000;  /* 1/2 sec */
	       if(bugit) {
		   sprintf(msg, "I am master\n");
		   RPT_BUGGER(msg);
	       }
	   }
	   else {                              /* I am slave */
	       master_set = 1;    /* tell code master has been decided */
	       master = 0;        /* set this code as slave   */
	       writbuf.addr = 0xEF;  /* set up master confirm. */
	       bid_wait= 10;     /* 1 sec */
	       if(bugit) {
		   sprintf(msg, "I am slave sending addr 0xEF.\n");
		   RPT_BUGGER(msg);
	       }
	   }
       }

read_timeout:

       if(++bids_made == note_inc) {
	  if(bids_made > 10000) {
	       sprintf(msg,"Over 10000 bids made assume network down\n");
	       RPT_INFO(msg);
	       RPT_BUGGER(msg);
	       return(RC_HARD);
	  }
	  note_inc += 2;
	  hxfupdate(UPDATE, &ps);     /* update htx stats each loop */
       }
   }   /* end of bid loop */
}      /* end of find master if not already known */



   while( (loop_count++ < tucb_ptr->pr.num_oper) ) {


       if(bugit) {
	   sprintf(msg, "Start loop %d, master = %d\n",loop_count,master);
	   RPT_BUGGER(msg);
       }
       if(master) {
	   writbuf.addr = 0xDA;     /* set this as data frame */
	   writbuf.cntl = loop_count;
	   if(tucb_ptr->pr.fix_bytes) bytes = tucb_ptr->pr.recsize;
	   else {
	       bytes = random();
	       while(1) {
		 if(bytes > tucb_ptr->pr.recsize) {
		   bytes = bytes/3;
		   if(bytes <= 0) {
		     bytes = bytes + (tucb_ptr->pr.recsize/2);
		     break;
		  }
		 }
		 else break;
	       }
	   }

master_write:
	  bytesw = writex(fdes,&writbuf,bytes+2,&we);
	   if (bytesw == -1) {
	       sprintf(msg,"Write failed %d %s\n",fdes,sys_errlist[errno]);
	       RPT_INFO(msg);
	       RPT_BUGGER(msg);
	       ++ps.bad_writes;
	       rc = RC_SYS_ERRNO;
	       goto end_tu4;
	   }
	   if(bugit) {
	       sprintf(msg, "MASTER: Wrote %d bytes, addr %02X\n",bytesw,writbuf.addr);
	       RPT_BUGGER(msg);
	   }
master_read:
	   while(1) {
	      FD_ZERO(&rd);
	      FD_ZERO(&wr);
	      FD_ZERO(&ex);
	      FD_SET(fdes, &ex) ;   /* wait for excp */
	      FD_SET(fdes, &rd) ;   /* wait to read  */
	      timeout.tv_sec = READ_TIMEOUT;
	      timeout.tv_usec = 0;
	      src = select(FD_SETSIZE, &rd, &wr, &ex, &timeout);
	      if (src == -1) {
		  sprintf(msg,"Select fail %d %s\n",fdes,sys_errlist[errno]);
		  RPT_INFO(msg);
		  RPT_BUGGER(msg);
		  INC_BAD_OTHERS;
		  rc = RC_SYS_ERRNO;
		  goto end_tu4;
	      }
	      if (src == 0) {
		++ps.bad_writes;
		if(++retry_count > 4) {
		  sprintf(msg,"master retries failed, timeouts\n");
		  RPT_INFO(msg);
		  RPT_BUGGER(msg);
		  rc = RC_HARD;
		  goto end_tu4;
		}
		goto master_write;
	      }
	      if (FD_ISSET(fdes,&ex)) {
		  while(1) {          /* loop till null status received */
		    if (ioctl(fdes, CIO_GET_STAT, &status)) {
			 sprintf(msg,"IOCTL get_stat %d %s\n",fdes,sys_errlist[errno]);
			 RPT_INFO(msg);
			 RPT_BUGGER(msg);
			 INC_BAD_OTHERS;
			 rc = RC_SYS_ERRNO;
			 goto end_tu4;
		    }
		    if(status.code == CIO_NULL_BLK) break;
		    else if ( (rc = decode_status(&status,tucb_ptr)) ) {
			    if(rc == RC_HARD) goto end_tu4;
			    rc = 0;
			    goto master_write;
		    }
		  }  /* end of loop on get status to clean status q */
	      }
	      if (FD_ISSET(fdes,&rd)) break;
	   }           /* end while(1) loop */

	   /* do read */
	   bytesr = readx(fdes,&readbuf,bytesw,&re);
	   if (bytesr == -1) {
	       sprintf(msg,"READ failed %d %s\n",fdes,sys_errlist[errno]);
	       RPT_INFO(msg);
	       RPT_BUGGER(msg);
	       ++ps.bad_reads;
	       rc = RC_SYS_ERRNO;
	       goto end_tu4;
	   }
	   if(bugit) {
	       sprintf(msg, "MASTER: Read %d bytes, addr %02X\n",bytesr,readbuf.addr);
	       RPT_BUGGER(msg);
	   }
	   if(readbuf.cntl != loop_count) {
	       ++ps.bad_reads;
	       if(++retry_count > 4) {
		    sprintf(msg,"master retries failed, bad sequence\n");
		    RPT_INFO(msg);
		    RPT_BUGGER(msg);
		    rc = RC_HARD;
		    goto end_tu4;
	       }
	       goto master_write;
	   }
	   if(bytesr!=bytesw) {
	       sprintf(msg,"Expected %d bytes only received %d bytes\n",bytesw,bytesr);
	       RPT_INFO(msg);
	       RPT_BUGGER(msg);
	       ++ps.bad_reads;
	       rc = RC_SOFT;
	       goto end_tu4;
	   }

	   if (memcmp(&readbuf,&writbuf,bytesw) != 0) {
	      sprintf(msg,"READ: failed data verification! \n");
	      RPT_INFO(msg);
	      RPT_BUGGER(msg);
	      strcpy(msg,hxfcbuf(&ps, &writbuf, &readbuf,(bytesw)));
	      ++ps.bad_reads;
	      RPT_INFO(msg);
	      RPT_BUGGER(msg);
	      rc = RC_SOFT;
	      goto end_tu4;
	   }
	   retry_count = 0;     /* reset the retry counter */
	   ++ps.good_writes;
	   ps.bytes_writ += bytesw;
	   totbytes += bytesw;
	   ++ps.good_reads;
	   ps.bytes_read += bytesr;
	   totbytes += bytesr;

       }    /* end of if I am master */

       else {            /* I am slave */
	   /* wait for master to write, then send it back */
slave_read:

	   if(bugit) {
	       time(&ltime);
	       sprintf(msg, "SLAVE: waiting for data %s\n",ctime(&ltime));
	       RPT_BUGGER(msg);
	   }

	   while(1) {
	      FD_ZERO(&rd);
	      FD_ZERO(&wr);
	      FD_ZERO(&ex);
	      FD_SET(fdes, &ex) ;   /* wait for excp */
	      FD_SET(fdes, &rd) ;   /* wait to read  */
	      timeout.tv_sec = SLAVE_TIMEOUT;
	      timeout.tv_usec = 0;
	      src = select(FD_SETSIZE, &rd, &wr, &ex, &timeout);
	      if (src == -1) {
		  sprintf(msg,"Select fail %d %s\n",fdes,sys_errlist[errno]);
		  RPT_INFO(msg);
		  RPT_BUGGER(msg);
		  INC_BAD_OTHERS;
		  rc = RC_SYS_ERRNO;
		  goto end_tu4;
	      }
	      if (src == 0) {
		  ++ps.bad_reads;
		  if(++retry_count > 4) {
		    sprintf(msg,"slave retries failed, timeouts\n");
		    RPT_INFO(msg);
		    RPT_BUGGER(msg);
		    rc = RC_HARD;
		    goto end_tu4;
		  }
		  goto slave_read;
	      }
	      if (FD_ISSET(fdes,&ex)) {
		  while(1) {          /* loop till null status received */
		    if (ioctl(fdes, CIO_GET_STAT, &status)) {
			 sprintf(msg,"IOCTL get_stat %d %s\n",fdes,sys_errlist[errno]);
			 RPT_INFO(msg);
			 RPT_BUGGER(msg);
			 INC_BAD_OTHERS;
			 rc = RC_SYS_ERRNO;
			 goto end_tu4;
		    }
		    if(status.code == CIO_NULL_BLK) break;
		    else if( (rc = decode_status(&status,tucb_ptr)) ) {
			    if(rc == RC_HARD) goto end_tu4;
			    rc = 0;
			    goto slave_read;
		    }
		  }  /* end of loop on get status to clean status q */
	      }
	      if (FD_ISSET(fdes,&rd)) break;
	   }           /* end while(1) loop */

	   /* do read */
	   bytesr = readx(fdes,&readbuf,tucb_ptr->pr.recsize+2,&re);
	   if (bytesr == -1) {
	       sprintf(msg,"READ failed %d %s\n",fdes,sys_errlist[errno]);
	       RPT_INFO(msg);
	       RPT_BUGGER(msg);
	       ++ps.bad_reads;
	       rc = RC_SYS_ERRNO;
	       goto end_tu4;
	   }
	   if(bugit) {
	       sprintf(msg, "SLAVE: Read %d bytes, addr %02X\n",bytesr,readbuf.addr);
	       RPT_BUGGER(msg);
	   }
	   if(readbuf.cntl == 0x01) loop_count = 1;
	   ++ps.good_reads;
	   ps.bytes_read += bytesr;
	   totbytes += bytesr;
	   retry_count = 0;
slave_write:
	   bytesw = writex(fdes,&readbuf,bytesr,&we);
	   if (bytesw == -1) {
	       sprintf(msg,"Write failed %d %s\n",fdes,sys_errlist[errno]);
	       RPT_INFO(msg);
	       RPT_BUGGER(msg);
	       ++ps.bad_writes;
	       rc = RC_SYS_ERRNO;
	       goto end_tu4;
	   }
	   if(bugit) {
	       sprintf(msg, "SLAVE: Wrote %d bytes, addr %02X\n",bytesw,readbuf.addr);
	       RPT_BUGGER(msg);
	   }
	   ++ps.good_writes;
	   ps.bytes_writ += bytesw;
	   totbytes += bytesw;

       }                           /* end if I am slave */
       hxfupdate(UPDATE, &ps);     /* update htx stats each loop */

   }                               /* end pr.num_oper tests */

end_tu4:

   free(pat_ptr);              /* free up the pattern data space */
   return(rc);
}                   /* end tu004() */

#endif

/* ************************************************************************
**  Function: tu005()
**
**   DIAGNOSTIC TEST OF 8273 AND 8255 CHIPS USED FOR SDLC COMMUNICATION
**
**   This test does NOT require a wrap plug.  I issues and open and
**   several different starts with halts seperating the starts then a
**   close. This tests all the testable registers on the two chips that
**   are used for SDLC/HDLC.
**
**************************************************************************/

tu005(fdes, tucb_ptr)
     int       fdes;
     TUTYPE    *tucb_ptr;
{
   int             rc=RC_GOOD;     /* init the routines return code */
   mpa_start_t     start_st;       /* structure passed to ioctl CIO_START */
   cio_stat_blk_t  status;         /* status structure for CIO_GET_STAT */

   /* the device was opened in hxempa.c.. here I will start the device */
   sprintf(msg, "Start TU005.. bps %d \n",tucb_ptr->pr.bps);
   RPT_BUGGER(msg);

   bzero(&start_st,sizeof(mpa_start_t));

   start_st.data_proto |= SDLC;
   start_st.modem_flags |= GATE_INT_CLK;  /* wrap requires internal clock */
   start_st.baud_rate = tucb_ptr->pr.bps;           /* set up bps rate for wrap */
   if(tucb_ptr->pr.nrzi) start_st.data_flags |= SET_NRZI_DATA;
   start_st.xfer_mode |= SET_NO_DMA;      /* PIO required for wrap */
   start_st.station_type |= PRIMARY;      /* set up as primary station */
   start_st.sb.netid = 0;

   if (ioctl(fdes, CIO_START, &start_st) == -1) {
       sprintf(msg,"ioctl, CIO_START "
       "failed %d %s\n",errno,sys_errlist[errno]);
       RPT_BUGGER(msg);
#ifndef DIAGS
       RPT_INFO(msg);
       INC_BAD_OTHERS;
#endif
       return RC_SYS_ERRNO;
   }

   if( (rc = wait_status(fdes,CIO_START_DONE, tucb_ptr)) ) {
       sprintf(msg,"Wait START DONE failed: rc = %d\n",rc);
       RPT_BUGGER(msg);
#ifndef DIAGS
       RPT_INFO(msg);
       INC_BAD_OTHERS;
#endif
       return rc;
   }


   bzero(&start_st,sizeof(mpa_start_t));
   start_st.sb.netid = 0;
   if (ioctl(fdes, CIO_HALT, &start_st) == -1) {
       sprintf(msg,"Write failed %d %s\n",fdes,sys_errlist[errno]);
       RPT_BUGGER(msg);
#ifndef DIAGS
       INC_BAD_OTHERS;
       RPT_INFO(msg);
#endif
       return RC_SYS_ERRNO;
   }

   if( (rc = wait_status(fdes,CIO_HALT_DONE, tucb_ptr)) ) {
       sprintf(msg,"Wait for HALT_DONE failed: rc = %d\n",rc);
       RPT_BUGGER(msg);
#ifndef DIAGS
       RPT_INFO(msg);
       INC_BAD_OTHERS;
#endif
       return rc;
   }

   return(RC_GOOD);
}    /* tu005()  */

init_pat(tucb_ptr)
    TUTYPE     *tucb_ptr;
{
#ifndef DIAGS       /* if not diag compile */

   int                     bytesw,rc;
   int                     pat_fd;       /* pattern file descriptor */
   int                     pat_len;      /* pattern file length */
   int                     cur_cpy;
   char                    *cur_end;
   char                    *the_end;
   char                    tmp_buf[80];

   sprintf(tmp_buf,"%s",PATLIB);  /* PATLIB is /usr/lpp/htx/pattern/ */
   strcat(tmp_buf,tucb_ptr->pr.pattern_id);

   /* open the pattern file */
   if ((pat_fd = open(tmp_buf, O_RDONLY)) < 0) {
      sprintf(msg, "Could not open %s: %s\n",
		   tucb_ptr->pr.pattern_id, sys_errlist[errno]);
      RPT_INFO(msg);
      return(RC_SYS_ERRNO);
   }
   /* find pattern file length */
   if ((pat_len = lseek(pat_fd, 0L, 2)) == -1) {
      sprintf(msg, "lseek len pat fail. %s: %s\n",
		   tucb_ptr->pr.pattern_id, sys_errlist[errno]);
      RPT_INFO(msg);
      close(pat_fd);
      return(RC_SYS_ERRNO);
   }
   /* return r/w ptr to start of file */
   if (lseek(pat_fd, 0L, 0) == -1) {
      sprintf(msg, "lseek 0 rw ptr pat: %s: %s\n",
		   tucb_ptr->pr.pattern_id, sys_errlist[errno]);
      RPT_INFO(msg);
      close(pat_fd);
      return(RC_SYS_ERRNO);
   }
   /* get space for RECSIZE bytes of pattern data */
   if ((pat_ptr = (char *) malloc(MAXBYTES)) == NULL) {
      sprintf(msg, "bad malloc pat: %d bytes, %s \n",
	       MAXBYTES,sys_errlist[errno]);
      RPT_INFO(msg);
      close(pat_fd);
      return(RC_SYS_ERRNO);
   }
   if (tucb_ptr->pr.recsize < pat_len) pat_len = tucb_ptr->pr.recsize;
   /* read data file into pat_ptr space and expand to tucb_ptr->pr.recsize */
   if (read(pat_fd, pat_ptr, pat_len) == -1) {
       free(pat_ptr);
       close(pat_fd);
       sprintf(msg, "bad read %s: %s\n",
		 tucb_ptr->pr.pattern_id, sys_errlist[errno]);
       RPT_INFO(msg);
       return(RC_SYS_ERRNO);
   }
   close(pat_fd);


   /* now fill out the pat_ptr buffer by repeating the pattern */
   if (tucb_ptr->pr.recsize != pat_len) {
       the_end = pat_ptr + tucb_ptr->pr.recsize;
       cur_end = pat_ptr + pat_len;
       cur_cpy = pat_len;
       while(1) {
	   if ((cur_cpy+cur_end) < the_end) {
	     memcpy(cur_end,pat_ptr,cur_cpy);
	     cur_end = cur_end + cur_cpy;
	     cur_cpy = cur_cpy*2;
	   }
	   else {
	     cur_cpy = the_end-cur_end;
	     memcpy(cur_end,pat_ptr,cur_cpy);
	     break;
	   }
       }                             /* end while (1)  */
   }                         /* end if tucb_ptr->pr.recsize != pat_len */

#else              /* if it is diagnostic compile */

   int i;

   /* get space for RECSIZE bytes of pattern data */
   if ((pat_ptr = (char *) malloc(tucb_ptr->pr.recsize)) == NULL) {
      return(RC_SYS_ERRNO);
   }

   /* set up sequencial data */
   for(i=0; i<tucb_ptr->pr.recsize; i++) {
       pat_ptr[i] = i;
   }


#endif

   return(0);
}                         /* end init_pat */

/*-----------------------  W A I T _ S T A T U S  ----------------------*/
/*                                                                      */
/*  NAME: wait_status                                                   */
/*                                                                      */
/*  FUNCTION:                                                           */
/*      Polls the status queue of the driver under test until a         */
/*      status block with code equal to "code" is found or until        */
/*      the wait for a status block times out.  All status blocks       */
/*      received before the expected one are thrown out.                */
/*                                                                      */
/*  EXECUTION ENVIRONMENT:                                              */
/*      Can only be executed at the user process level.                 */
/*                                                                      */
/*  DATA STRUCTURES:                                                    */
/*      Places the hardware address in the profile block (Ethernet).    */
/*                                                                      */
/*  RETURNS:                                                            */
/*      TRUE    The expected status block was received.                 */
/*      FALSE   The wait for the expected status timed out or error.    */
/*                                                                      */
/*----------------------------------------------------------------------*/

wait_status (fdes, code, tucb_ptr)
	int     fdes;
	int     code;
	TUTYPE  *tucb_ptr;
{
	struct status_block     status;
	struct pollfd           pollblk;
	int                     i, rc, result;

	while (1) {
	    /*                                                          */
	    /*  Wait for a status block by polling for it:              */
	    /*                                                          */
	    pollblk.fd = fdes;
	    pollblk.reqevents = POLLPRI;        /* wait for exception */
	    pollblk.rtnevents = 0;
	    result = poll( &pollblk, 1, STATUS_TIMEOUT );
	    if (result < 0) {
		sprintf(msg, "Wait_status:  Poll failed ");
		RPT_BUGGER(msg);
#ifndef DIAGS
		RPT_INFO(msg);
		decode_error(errno,tucb_ptr);
#endif
		return(RC_SYS_ERRNO);
	    }
	    if (result == 0) {
		sprintf(msg, "Wait_status:  Poll Timed out!");
		RPT_BUGGER(msg);
#ifndef DIAGS
		RPT_INFO(msg);
#endif
		return(RC_HARD);
	    } else {
		/*                                                      */
		/*  Status block is available -- issue ioctl to get it. */
		/*                                                      */
		result = ioctl( fdes, CIO_GET_STAT, &status );
		if (result < 0) {
		    sprintf(msg, "Wait_status: ioctl Get status fail");
		    RPT_BUGGER(msg);
#ifndef DIAGS
		    RPT_INFO(msg);
		    decode_error(errno,tucb_ptr);
#endif
		    rc = RC_SYS_ERRNO;
		    break;
		}
		/*  Is this the code we are waiting for?  If not, loop  */
		/*  to next wait.                                       */
		/*                                                      */
		if (status.code == code) {
		    if (status.option[0] == CIO_OK) {
			rc = RC_GOOD;                 /* succeeded */
			break;
		    } else {
			rc = RC_HARD;               /* failed */
			sprintf(msg, "Wait_status:  ioctl Get status ");
			RPT_BUGGER(msg);
#ifndef DIAGS
			RPT_INFO(msg);
			if( (rc = decode_status(&status,tucb_ptr)) ) {
			     return rc;
			}
#endif
		    }
		    break;
		} else {
#ifndef DIAGS
		    if( (rc = decode_status(&status,tucb_ptr)) ) {
			 return rc;
		    }
#endif
		    continue;                           /* next block */
		}
	    }
	}
	return(rc);
}     /* wait_status() */

/* ************************************************************************
**  Function: tu006()
**
**   DIAGNOSTIC TEST OF MODEM SIGNALS (WRAP PLUG ONLY TEST)
**
**   This test does require a wrap plug.  It test the various modem
**   signals with the aid of an external wrap plug.. Due to the nature
**   of the hard ware this dose not test DRS change or CTS channge signals.
**
**************************************************************************/

tu006(fdes, tucb_ptr)
     int       fdes;
     TUTYPE    *tucb_ptr;
{
   int             rc=RC_GOOD;     /* init the routines return code */
   mpa_start_t     start_st;       /* structure passed to ioctl CIO_START */
   cio_stat_blk_t  status;         /* status structure for CIO_GET_STAT */
   cmd_phase_t     sess_blk;
   int             i;
   rw_port_t       data;
   uchar           port_b_value;

   /* the device was opened in hxempa.c.. here I will start the device */
   sprintf(msg, "Start TU006.. bps %d \n",tucb_ptr->pr.bps);
   RPT_BUGGER(msg);

   bzero(&start_st,sizeof(mpa_start_t));
   /*
   ** start the card in external wrap mode ( needs wrap plug )
   ** also turn test on. in port_b of 8255.
   */
   start_st.data_proto |= SDLC;
   start_st.modem_flags |= (GATE_EXT_CLK);
   start_st.port_b_8255 |= (SPEED_SEL_OFF|SEL_STANBY_OFF|FREE_STAT_CHG);
   start_st.baud_rate = tucb_ptr->pr.bps;           /* set up bps rate for wrap */
   if(tucb_ptr->pr.nrzi) start_st.data_flags |= SET_NRZI_DATA;
   start_st.xfer_mode |= SET_NO_DMA;      /* PIO required for wrap */
   start_st.station_type |= PRIMARY;      /* set up as primary station */
   start_st.sb.netid = 0;

   if (ioctl(fdes, CIO_START, &start_st) == -1) {
       sprintf(msg,"ioctl, CIO_START "
       "failed %d %s\n",errno,sys_errlist[errno]);
       RPT_BUGGER(msg);
#ifndef DIAGS
       RPT_INFO(msg);
       INC_BAD_OTHERS;
#endif
       return RC_SYS_ERRNO;
   }

   if( (rc = wait_status(fdes,CIO_START_DONE, tucb_ptr)) ) {
       sprintf(msg,"Wait START DONE failed: rc = %d\n",rc);
       RPT_BUGGER(msg);
#ifndef DIAGS
       RPT_INFO(msg);
       INC_BAD_OTHERS;
#endif
       return rc;
   }
   sprintf(msg,"********* TEST rate_select -> xmit and recv clk\n");
   RPT_BUGGER(msg);
   /* first get the current value of port B on 8255 */
   bzero(&data,sizeof(rw_port_t));     /* clear the input structure */
   data.port_addr = PORT_B_8255; /* set up address for pos 0 */
   data.rw_flag |= MPA_READ;     /* tell driver to do read for us */
   if (ioctl(fdes, MPA_RW_PORT, &data)) {
       sprintf(msg, "Ioctl read port B 8255 failed :  "
       "%s \n",sys_errlist[errno]);
	RPT_BUGGER(msg);
#ifndef DIAGS
	RPT_INFO(msg);
	hxfupdate(UPDATE, &ps);
#endif
	rc = RC_SYS_ERRNO;
	goto end_tu006;
   }
   sprintf(msg,"     Inital Value of port B 8255 is %02X\n",data.value);
   RPT_BUGGER(msg);

   /* is test turned on */
   if(data.value & TEST_OFF) {
      /* turn it on */
      sprintf(msg,"TEST MODE  was not on Port B 8255 \n");
      RPT_BUGGER(msg);
#ifndef DIAGS
       INC_BAD_OTHERS;
       RPT_INFO(msg);
#endif
      rc = RC_SOFT;
      goto end_tu006;
   }
   /* speed select is set to 1 by cio_start but check it here to be sure */
   if(!(data.value&SPEED_SEL_OFF)) {
      port_b_value = data.value|SPEED_SEL_OFF;
				/* save the contents of port b */
				/* and turn off rate select = 1 */
   }
   else  port_b_value = data.value;
   sprintf(msg,"     Save port b value is %02X\n",port_b_value);
   RPT_BUGGER(msg);

   usleep(100000);

   /* do rate select and xmit and recv clock test */
   /* first set rate select on and see that xmit&recv clk's are 0 */
   bzero(&data,sizeof(rw_port_t));     /* clear the input structure */
   data.port_addr = PORT_B_8255; /* set up address for pos 0 */
   data.rw_flag |= MPA_WRITE;    /* tell driver to do write for us */
   data.value = port_b_value&0XFE;    /* set rate select to 0 */
   if (ioctl(fdes, MPA_RW_PORT, &data)) {
       sprintf(msg, "Ioctl write port B 8255 failed :  "
       "%s \n",sys_errlist[errno]);
	RPT_BUGGER(msg);
#ifndef DIAGS
	RPT_INFO(msg);
	hxfupdate(UPDATE, &ps);
#endif
	rc = RC_SYS_ERRNO;
	goto end_tu006;
   }
   sprintf(msg,"     wrote %02X to port B 8255 to turn on rate select\n",data.value);
   RPT_BUGGER(msg);

   usleep(100000);

   /* now get port A value with xmit and recv clock pins */
   bzero(&data,sizeof(rw_port_t));     /* clear the input structure */
   data.port_addr = PORT_A_8255;
   data.rw_flag |= MPA_READ;
   if (ioctl(fdes, MPA_RW_PORT, &data)) {
       sprintf(msg, "Ioctl read port A 8255 failed :  "
       "%s \n",sys_errlist[errno]);
	RPT_BUGGER(msg);
#ifndef DIAGS
	RPT_INFO(msg);
	hxfupdate(UPDATE, &ps);
#endif
	rc = RC_SYS_ERRNO;
	goto end_tu006;
   }
   sprintf(msg,"     Read Value of port A 8255 is %02X\n",data.value);
   RPT_BUGGER(msg);
   /* are xmit and recv bits 0 */
   if( (data.value & XMIT_CLK_ON) || (data.value & RECV_CLK_ON) ) {
	sprintf(msg,"Expected xmit and recv clks to be 0\n");
	RPT_BUGGER(msg);
#ifndef DIAGS
       INC_BAD_OTHERS;
       RPT_INFO(msg);
#endif
	rc = RC_SOFT;
	goto end_tu006;
   }

   usleep(100000);
   /* now set rate select off and test xmit and recv clk's == 1 */
   bzero(&data,sizeof(rw_port_t));     /* clear the input structure */
   data.port_addr = PORT_B_8255; /* set up address for pos 0 */
   data.rw_flag |= MPA_WRITE;    /* tell driver to do write for us */
   data.value = port_b_value;
   if (ioctl(fdes, MPA_RW_PORT, &data)) {
       sprintf(msg, "Ioctl write port B 8255 failed :  "
       "%s \n",sys_errlist[errno]);
	RPT_BUGGER(msg);
#ifndef DIAGS
	RPT_INFO(msg);
	hxfupdate(UPDATE, &ps);
#endif
	rc = RC_SYS_ERRNO;
	goto end_tu006;
   }
   sprintf(msg,"     wrote %02X to port B 8255 to turn off rate select\n",data.value);
   RPT_BUGGER(msg);

   usleep(100000);

   /* now get port A value with xmit and recv clock pins */
   bzero(&data,sizeof(rw_port_t));     /* clear the input structure */
   data.port_addr = PORT_A_8255;
   data.rw_flag |= MPA_READ;
   if (ioctl(fdes, MPA_RW_PORT, &data)) {
       sprintf(msg, "Ioctl read port A 8255 failed :  "
       "%s \n",sys_errlist[errno]);
	RPT_BUGGER(msg);
#ifndef DIAGS
	RPT_INFO(msg);
	hxfupdate(UPDATE, &ps);
#endif
	rc = RC_SYS_ERRNO;
	goto end_tu006;
   }
   sprintf(msg,"     Read Value of port A 8255 is %02X\n",data.value);
   RPT_BUGGER(msg);
   /* are xmit and recv bits 1 */
   if( !(data.value & XMIT_CLK_ON) || !(data.value & RECV_CLK_ON) ) {
	sprintf(msg,"Expected xmit and recv clks to be 1\n");
	RPT_BUGGER(msg);
#ifndef DIAGS
       INC_BAD_OTHERS;
       RPT_INFO(msg);
#endif
	rc = RC_SOFT;
	goto end_tu006;
   }

   sprintf(msg,"********* TEST DTR -> DSR and RI.\n");
   RPT_BUGGER(msg);

   /* DTR should be on by default,driver sets DTR on cio_start */
   /* first reset DTR to 0 then check state of DSR and RI..      */
   /* Note the DSR changed bit in 8273 port A never changes      */
   /* but the DSR active bit will change, I will check for DSR   */
   /* toggle in 8273 and RI change in 8255.                      */

for ( i=0; i<4; i++) {
   usleep(500000);

   sprintf(msg,"     *********Turn DTR to off.\n");
   RPT_BUGGER(msg);
   bzero(&sess_blk,sizeof(cmd_phase_t));     /* clear the input structure */
   sess_blk.cmd = RESET_8273_PORT_B_CMD;
   sess_blk.parm_count = 1;
   sess_blk.parm[0]=0xCA;    /* mask to reset all valid bits */
   if (ioctl(fdes, MPA_CMD_8273, &sess_blk) == -1) {
	  sprintf(msg,"ioctl, MPA_CMD_8273 "
	  "failed %d %s\n",errno,sys_errlist[errno]);
	  RPT_BUGGER(msg);
#ifndef DIAGS
	  RPT_INFO(msg);
	  INC_BAD_OTHERS;
#endif
	  rc = RC_SYS_ERRNO;
	  goto end_tu006;
   }
   /* now check to see that it went off */
   usleep(500000);
   /* check to see if DTR is 0 in 8273 port b reg */
   bzero(&sess_blk,sizeof(cmd_phase_t));     /* clear the input structure */
   sess_blk.cmd = READ_8273_PORT_B_CMD;
   sess_blk.parm_count = 0;
   sess_blk.flag = RETURN_RESULT;

   if (ioctl(fdes, MPA_CMD_8273, &sess_blk) == -1) {
	  sprintf(msg,"ioctl, MPA_CMD_8273 "
	  "failed %d %s\n",errno,sys_errlist[errno]);
	  RPT_BUGGER(msg);
#ifndef DIAGS
	  RPT_INFO(msg);
	  INC_BAD_OTHERS;
#endif
	  rc = RC_SYS_ERRNO;
	  goto end_tu006;
   }
   sprintf(msg,"     Read %02X from 8273 port b.check for DTR off.\n",sess_blk.result);
   RPT_BUGGER(msg);

   if(sess_blk.result & SET_8273_PORT_B_PB2) {
      sprintf(msg,"Expected DTR to be off.\n",sess_blk.result);
      RPT_BUGGER(msg);
#ifndef DIAGS
       INC_BAD_OTHERS;
       RPT_INFO(msg);
#endif
      rc = RC_SOFT;
      goto end_tu006;
   }
   sprintf(msg,"     *********Turn DTR to on.\n");
   RPT_BUGGER(msg);
   usleep(500000);

   bzero(&sess_blk,sizeof(cmd_phase_t));     /* clear the input structure */
   sess_blk.cmd = SET_8273_PORT_B_CMD;
   sess_blk.parm_count = 1;
   sess_blk.parm[0]=SET_8273_PORT_B_PB2;
   if (ioctl(fdes, MPA_CMD_8273, &sess_blk) == -1) {
	  sprintf(msg,"ioctl, MPA_CMD_8273 "
	  "failed %d %s\n",errno,sys_errlist[errno]);
	  RPT_BUGGER(msg);
#ifndef DIAGS
	  RPT_INFO(msg);
	  INC_BAD_OTHERS;
#endif
	  rc = RC_SYS_ERRNO;
	  goto end_tu006;
   }
   /* now check to see that it went back on */
   usleep(500000);
   /* check to see if DTR is 1 in 8273 port b reg */
   bzero(&sess_blk,sizeof(cmd_phase_t));     /* clear the input structure */
   sess_blk.cmd = READ_8273_PORT_B_CMD;
   sess_blk.parm_count = 0;
   sess_blk.flag = RETURN_RESULT;

   if (ioctl(fdes, MPA_CMD_8273, &sess_blk) == -1) {
	  sprintf(msg,"ioctl, MPA_CMD_8273 "
	  "failed %d %s\n",errno,sys_errlist[errno]);
	  RPT_BUGGER(msg);
#ifndef DIAGS
	  RPT_INFO(msg);
	  INC_BAD_OTHERS;
#endif
	  rc = RC_SYS_ERRNO;
	  goto end_tu006;
   }
   sprintf(msg,"     Read %02X from 8273 port b.check for DTR on.\n",sess_blk.result);
   RPT_BUGGER(msg);

   if(!(sess_blk.result & SET_8273_PORT_B_PB2)) {
      sprintf(msg,"Expected DTR to be off.\n",sess_blk.result);
      RPT_BUGGER(msg);
#ifndef DIAGS
       INC_BAD_OTHERS;
       RPT_INFO(msg);
#endif
      rc = RC_SOFT;
      goto end_tu006;
   }

}

   sprintf(msg,"     *********Turn DTR to off.\n");
   RPT_BUGGER(msg);
   bzero(&sess_blk,sizeof(cmd_phase_t));     /* clear the input structure */
   sess_blk.cmd = RESET_8273_PORT_B_CMD;
   sess_blk.parm_count = 1;
   sess_blk.parm[0]=0xCA;    /* mask to reset all valid bits */
   if (ioctl(fdes, MPA_CMD_8273, &sess_blk) == -1) {
	  sprintf(msg,"ioctl, MPA_CMD_8273 "
	  "failed %d %s\n",errno,sys_errlist[errno]);
	  RPT_BUGGER(msg);
#ifndef DIAGS
	  RPT_INFO(msg);
	  INC_BAD_OTHERS;
#endif
	  rc = RC_SYS_ERRNO;
	  goto end_tu006;
   }

   usleep(100000);
   /* check to see if DSR active is 0 in 8273 port a reg */
   bzero(&sess_blk,sizeof(cmd_phase_t));     /* clear the input structure */
   sess_blk.cmd = READ_8273_PORT_A_CMD;
   sess_blk.parm_count = 0;
   sess_blk.flag = RETURN_RESULT;

   if (ioctl(fdes, MPA_CMD_8273, &sess_blk) == -1) {
	  sprintf(msg,"ioctl, MPA_CMD_8273 "
	  "failed %d %s\n",errno,sys_errlist[errno]);
	  RPT_BUGGER(msg);
#ifndef DIAGS
	  RPT_INFO(msg);
	  INC_BAD_OTHERS;
#endif
	  rc = RC_SYS_ERRNO;
	  goto end_tu006;
   }
   sprintf(msg,"     Read %02X from 8273 port a.check for DSR off.\n",sess_blk.result);
   RPT_BUGGER(msg);

   if(sess_blk.result & PORT_A_8273_PA2) {
      sprintf(msg,".expected DSR to be off.\n",sess_blk.result);
      RPT_BUGGER(msg);
#ifndef DIAGS
       INC_BAD_OTHERS;
       RPT_INFO(msg);
#endif
      rc = RC_SOFT;
      goto end_tu006;
   }

   /* check RI in 8255 port A reg */
   bzero(&data,sizeof(rw_port_t));     /* clear the input structure */
   data.port_addr = PORT_A_8255;
   data.rw_flag |= MPA_READ;
   if (ioctl(fdes, MPA_RW_PORT, &data)) {
       sprintf(msg, "Ioctl read port A 8255 failed :  "
       "%s \n",sys_errlist[errno]);
	RPT_BUGGER(msg);
#ifndef DIAGS
	RPT_INFO(msg);
	hxfupdate(UPDATE, &ps);
#endif
	rc = RC_SYS_ERRNO;
	goto end_tu006;
   }
   sprintf(msg,"     CHK RI: Read Value of port A 8255 is %02X\n",data.value);
   RPT_BUGGER(msg);
   /* is RI 1 */
   if( !(data.value & RING_ON) ) {
	sprintf(msg,"Expected RI to be 1, not active \n");
	RPT_BUGGER(msg);
#ifndef DIAGS
       INC_BAD_OTHERS;
       RPT_INFO(msg);
#endif
	rc = RC_SOFT;
	goto end_tu006;
   }


   /* Now toggle DTR back on */
   sprintf(msg,"     *********Turn DTR to on.\n");
   RPT_BUGGER(msg);
   usleep(100000);

   bzero(&sess_blk,sizeof(cmd_phase_t));     /* clear the input structure */
   sess_blk.cmd = SET_8273_PORT_B_CMD;
   sess_blk.parm_count = 1;
   sess_blk.parm[0]=SET_8273_PORT_B_PB2;
   if (ioctl(fdes, MPA_CMD_8273, &sess_blk) == -1) {
	  sprintf(msg,"ioctl, MPA_CMD_8273 "
	  "failed %d %s\n",errno,sys_errlist[errno]);
	  RPT_BUGGER(msg);
#ifndef DIAGS
	  RPT_INFO(msg);
	  INC_BAD_OTHERS;
#endif
	  rc = RC_SYS_ERRNO;
	  goto end_tu006;
   }

   usleep(100000);
   /* check to see if DSR active is 1 in 8273 port a reg */
   bzero(&sess_blk,sizeof(cmd_phase_t));     /* clear the input structure */
   sess_blk.cmd = READ_8273_PORT_A_CMD;
   sess_blk.parm_count = 0;
   sess_blk.flag = RETURN_RESULT;

   if (ioctl(fdes, MPA_CMD_8273, &sess_blk) == -1) {
	  sprintf(msg,"ioctl, MPA_CMD_8273 "
	  "failed %d %s\n",errno,sys_errlist[errno]);
	  RPT_BUGGER(msg);
#ifndef DIAGS
	  RPT_INFO(msg);
	  INC_BAD_OTHERS;
#endif
	  rc = RC_SYS_ERRNO;
	  goto end_tu006;
   }
   sprintf(msg,"     Read %02X from 8273 port a.check for DSR on.\n",sess_blk.result);
   RPT_BUGGER(msg);

   if( !(sess_blk.result & PORT_A_8273_PA2) ) {
      sprintf(msg,"Read %02X from 8273 port a..expected DSR to be on.\n",sess_blk.result);
      RPT_BUGGER(msg);
#ifndef DIAGS
       INC_BAD_OTHERS;
       RPT_INFO(msg);
#endif
      rc = RC_SOFT;
      goto end_tu006;
   }

   /* check RI in 8255 port A reg */
   bzero(&data,sizeof(rw_port_t));     /* clear the input structure */
   data.port_addr = PORT_A_8255;
   data.rw_flag |= MPA_READ;
   if (ioctl(fdes, MPA_RW_PORT, &data)) {
       sprintf(msg, "Ioctl read port A 8255 failed :  "
       "%s \n",sys_errlist[errno]);
	RPT_BUGGER(msg);
#ifndef DIAGS
	RPT_INFO(msg);
	hxfupdate(UPDATE, &ps);
#endif
	rc = RC_SYS_ERRNO;
	goto end_tu006;
   }
   sprintf(msg,"     CHK RI: Read Value of port A 8255 is %02X\n",data.value);
   RPT_BUGGER(msg);
   /* is RI 0 */
   if( (data.value & RING_ON) ) {
	sprintf(msg,"Expected RI to be 0, active\n");
	RPT_BUGGER(msg);
#ifndef DIAGS
       INC_BAD_OTHERS;
       RPT_INFO(msg);
#endif
	rc = RC_SOFT;
	goto end_tu006;
   }


   /*
   ** This next test does RTS -> CTS and CD...
   ** The checks for CTS and CD are done in 8255 port a, since
   ** the bits seem to be stuck in 8273 port a. Also the CTS
   ** change bit is stuck in 8273 port a and there are no other
   ** places to check this status, so I can't test it in external
   ** wrap mode.
   */
   sprintf(msg,"********* TEST RTS -> CTS and CD.\n");
   RPT_BUGGER(msg);
   usleep(100000);

   sprintf(msg,"     *********Turn RTS to off.\n");
   RPT_BUGGER(msg);
   bzero(&sess_blk,sizeof(cmd_phase_t));     /* clear the input structure */
   sess_blk.cmd = RESET_8273_PORT_B_CMD;
   sess_blk.parm_count = 1;
   sess_blk.parm[0]=0xCA;    /* mask to reset all valid bits */
   if (ioctl(fdes, MPA_CMD_8273, &sess_blk) == -1) {
	  sprintf(msg,"ioctl, MPA_CMD_8273 "
	  "failed %d %s\n",errno,sys_errlist[errno]);
	  RPT_BUGGER(msg);
#ifndef DIAGS
	  RPT_INFO(msg);
	  INC_BAD_OTHERS;
#endif
	  rc = RC_SYS_ERRNO;
	  goto end_tu006;
   }

   usleep(100000);
   /* check to see if CTS active is 1 in 8255 port A reg (CTS off) */
   /* and check for CD active is 1 in 8255 port A reg (CD off) */
   bzero(&data,sizeof(rw_port_t));     /* clear the input structure */
   data.port_addr = PORT_A_8255;
   data.rw_flag |= MPA_READ;
   if (ioctl(fdes, MPA_RW_PORT, &data)) {
       sprintf(msg, "Ioctl read port A 8255 failed :  "
       "%s \n",sys_errlist[errno]);
	RPT_BUGGER(msg);
#ifndef DIAGS
	RPT_INFO(msg);
	hxfupdate(UPDATE, &ps);
#endif
	rc = RC_SYS_ERRNO;
	goto end_tu006;
   }
   sprintf(msg,"     CHK CTS & CD: Read Value of port A 8255 is %02X\n",data.value);
   RPT_BUGGER(msg);
   /* is CTS 1 */
   if( !(data.value & CLEAR_TO_SEND) ) {
	sprintf(msg,"Expected CTS to be 1, not active \n");
	RPT_BUGGER(msg);
#ifndef DIAGS
       INC_BAD_OTHERS;
       RPT_INFO(msg);
#endif
	rc = RC_SOFT;
	goto end_tu006;
   }
   /* is CD 1 */
   if( !(data.value & CARRIER_DET) ) {
	sprintf(msg,"Expected CD to be 1, not active \n");
	RPT_BUGGER(msg);
#ifndef DIAGS
       INC_BAD_OTHERS;
       RPT_INFO(msg);
#endif
	rc = RC_SOFT;
	goto end_tu006;
   }

   /* Now toggle RTS back on */
   sprintf(msg,"     *********Turn RTS to on.\n");
   RPT_BUGGER(msg);
   usleep(100000);

   bzero(&sess_blk,sizeof(cmd_phase_t));     /* clear the input structure */
   sess_blk.cmd = SET_8273_PORT_B_CMD;
   sess_blk.parm_count = 1;
   sess_blk.parm[0]=SET_8273_PORT_B_RTS;
   if (ioctl(fdes, MPA_CMD_8273, &sess_blk) == -1) {
	  sprintf(msg,"ioctl, MPA_CMD_8273 "
	  "failed %d %s\n",errno,sys_errlist[errno]);
	  RPT_BUGGER(msg);
#ifndef DIAGS
	  RPT_INFO(msg);
	  INC_BAD_OTHERS;
#endif
	  rc = RC_SYS_ERRNO;
	  goto end_tu006;
   }
   usleep(100000);
   /* check to see if CTS active is 0 in 8255 port A reg (CTS on) */
   /* and check for CD active is 0 in 8255 port A reg (CD on) */

   bzero(&data,sizeof(rw_port_t));     /* clear the input structure */
   data.port_addr = PORT_A_8255;
   data.rw_flag |= MPA_READ;
   if (ioctl(fdes, MPA_RW_PORT, &data)) {
       sprintf(msg, "Ioctl read port A 8255 failed :  "
       "%s \n",sys_errlist[errno]);
	RPT_BUGGER(msg);
#ifndef DIAGS
	RPT_INFO(msg);
	hxfupdate(UPDATE, &ps);
#endif
	rc = RC_SYS_ERRNO;
	goto end_tu006;
   }
   sprintf(msg,"     CHK CTS & CD: Read Value of port A 8255 is %02X\n",data.value);
   RPT_BUGGER(msg);
   /* is CTS 0 */
   if( (data.value & CLEAR_TO_SEND) ) {
	sprintf(msg,"Expected CTS to be 0, active \n");
	RPT_BUGGER(msg);
#ifndef DIAGS
       INC_BAD_OTHERS;
       RPT_INFO(msg);
#endif
	rc = RC_SOFT;
	goto end_tu006;
   }
   /* is CD 0 */
   if( (data.value & CARRIER_DET) ) {
	sprintf(msg,"Expected CD to be 0, active \n");
	RPT_BUGGER(msg);
#ifndef DIAGS
       INC_BAD_OTHERS;
       RPT_INFO(msg);
#endif
	rc = RC_SOFT;
	goto end_tu006;
   }

   usleep(100000);
   sprintf(msg,"********* TEST MODEM STATUS CHANGE BIT .\n");
   RPT_BUGGER(msg);


   /* now test the modem status changed bit in 8255 port A */
   /* First reset it with bit in 8255 port b, then make sure it =0 */
   /* Then toggle DTR and see if bit is 1 .                        */
   /* This modem status changed bit is only affected by DSR change */
   /* So this will act as the DSR change test.                     */

   sprintf(msg,"     Turn Reset of modem status changed bit on\n");
   RPT_BUGGER(msg);
   bzero(&data,sizeof(rw_port_t));     /* clear the input structure */
   data.port_addr = PORT_B_8255;
   data.rw_flag |= MPA_WRITE;
   data.value = 0x02;
   if (ioctl(fdes, MPA_RW_PORT, &data)) {
       sprintf(msg, "Ioctl read port A 8255 failed :  "
       "%s \n",sys_errlist[errno]);
	RPT_BUGGER(msg);
#ifndef DIAGS
	RPT_INFO(msg);
	hxfupdate(UPDATE, &ps);
#endif
	rc = RC_SYS_ERRNO;
	goto end_tu006;
   }

   usleep(100000);

   sprintf(msg,"     Turn Reset of modem status changed bit off\n");
   RPT_BUGGER(msg);
   bzero(&data,sizeof(rw_port_t));     /* clear the input structure */
   data.port_addr = PORT_B_8255;
   data.rw_flag |= MPA_WRITE;
   data.value = 0x0A;
   if (ioctl(fdes, MPA_RW_PORT, &data)) {
       sprintf(msg, "Ioctl read port A 8255 failed :  "
       "%s \n",sys_errlist[errno]);
	RPT_BUGGER(msg);
#ifndef DIAGS
	RPT_INFO(msg);
	hxfupdate(UPDATE, &ps);
#endif
	rc = RC_SYS_ERRNO;
	goto end_tu006;
   }
    /* now read port a and make sure modem status changed bit is off */

   bzero(&data,sizeof(rw_port_t));     /* clear the input structure */
   data.port_addr = PORT_A_8255;
   data.rw_flag |= MPA_READ;
   if (ioctl(fdes, MPA_RW_PORT, &data)) {
       sprintf(msg, "Ioctl read port A 8255 failed :  "
       "%s \n",sys_errlist[errno]);
	RPT_BUGGER(msg);
#ifndef DIAGS
	RPT_INFO(msg);
	hxfupdate(UPDATE, &ps);
#endif
	rc = RC_SYS_ERRNO;
	goto end_tu006;
   }
   sprintf(msg,"     CHK MSC: Read Value of port A 8255 is %02X\n",data.value);
   RPT_BUGGER(msg);
   /* is modem status changed bit off 0 */
   if( (data.value & MODEM_STAT_CHG) ) {
	sprintf(msg,"Expected modem status changed bit to be 0, off \n");
	RPT_BUGGER(msg);
#ifndef DIAGS
       INC_BAD_OTHERS;
       RPT_INFO(msg);
#endif
	rc = RC_SOFT;
	goto end_tu006;
   }

   /* now RESET DTR  Then SET DTR in 8273 port b reg */

   sprintf(msg,"     *********Turn DTR to off.\n");
   RPT_BUGGER(msg);
   bzero(&sess_blk,sizeof(cmd_phase_t));     /* clear the input structure */
   sess_blk.cmd = RESET_8273_PORT_B_CMD;
   sess_blk.parm_count = 1;
   sess_blk.parm[0]=0xCA;    /* mask to reset all valid bits */
   if (ioctl(fdes, MPA_CMD_8273, &sess_blk) == -1) {
	  sprintf(msg,"ioctl, MPA_CMD_8273 "
	  "failed %d %s\n",errno,sys_errlist[errno]);
	  RPT_BUGGER(msg);
#ifndef DIAGS
	  RPT_INFO(msg);
	  INC_BAD_OTHERS;
#endif
	  rc = RC_SYS_ERRNO;
	  goto end_tu006;
   }

   /* Now toggle DTR back on */
   sprintf(msg,"     *********Turn DTR to on.\n");
   RPT_BUGGER(msg);
   usleep(100000);

   bzero(&sess_blk,sizeof(cmd_phase_t));     /* clear the input structure */
   sess_blk.cmd = SET_8273_PORT_B_CMD;
   sess_blk.parm_count = 1;
   sess_blk.parm[0]=SET_8273_PORT_B_PB2;
   if (ioctl(fdes, MPA_CMD_8273, &sess_blk) == -1) {
	  sprintf(msg,"ioctl, MPA_CMD_8273 "
	  "failed %d %s\n",errno,sys_errlist[errno]);
	  RPT_BUGGER(msg);
#ifndef DIAGS
	  RPT_INFO(msg);
	  INC_BAD_OTHERS;
#endif
	  rc = RC_SYS_ERRNO;
	  goto end_tu006;
   }

   usleep(100000);

    /* now read port a and make sure modem status changed bit is on */

   bzero(&data,sizeof(rw_port_t));     /* clear the input structure */
   data.port_addr = PORT_A_8255;
   data.rw_flag |= MPA_READ;
   if (ioctl(fdes, MPA_RW_PORT, &data)) {
       sprintf(msg, "Ioctl read port A 8255 failed :  "
       "%s \n",sys_errlist[errno]);
	RPT_BUGGER(msg);
#ifndef DIAGS
	RPT_INFO(msg);
	hxfupdate(UPDATE, &ps);
#endif
	rc = RC_SYS_ERRNO;
	goto end_tu006;
   }
   sprintf(msg,"     CHK MSC: Read Value of port A 8255 is %02X\n",data.value);
   RPT_BUGGER(msg);
   /* is modem status changed bit on  1 */
   if( !(data.value & MODEM_STAT_CHG) ) {
	sprintf(msg,"Expected modem status changed bit to be 1, on \n");
	RPT_BUGGER(msg);
#ifndef DIAGS
       INC_BAD_OTHERS;
       RPT_INFO(msg);
#endif
	rc = RC_SOFT;
	goto end_tu006;
   }


end_tu006:

   bzero(&start_st,sizeof(mpa_start_t));
   start_st.sb.netid = 0;
   if (ioctl(fdes, CIO_HALT, &start_st) == -1) {
       sprintf(msg,"Write failed %d %s\n",fdes,sys_errlist[errno]);
       RPT_BUGGER(msg);
#ifndef DIAGS
       INC_BAD_OTHERS;
       RPT_INFO(msg);
#endif
       return RC_SYS_ERRNO;
   }

   if( wait_status(fdes,CIO_HALT_DONE, tucb_ptr) ) {
       sprintf(msg,"Wait for HALT_DONE failed: \n");
       RPT_BUGGER(msg);
#ifndef DIAGS
       RPT_INFO(msg);
       INC_BAD_OTHERS;
#endif
       return RC_SYS_ERRNO;
   }

   return(rc);
}    /* tu006()  */


/* ************************************************************************
**  Function: tu007()
**
**   DIAGNOSTIC TEST OF MODEM SIGNALS (ELECTRONIC WRAP TEST..NO WRAP PLUG)
**
**   This test does NOT require a wrap plug.
**
**
**************************************************************************/

tu007(fdes, tucb_ptr)
     int       fdes;
     TUTYPE    *tucb_ptr;
{
   int             rc=RC_GOOD;     /* init the routines return code */
   mpa_start_t     start_st;       /* structure passed to ioctl CIO_START */
   cio_stat_blk_t  status;         /* status structure for CIO_GET_STAT */
   cmd_phase_t     sess_blk;
   int             i;
   rw_port_t       data;
   uchar           port_b_value;

   /* the device was opened in hxempa.c.. here I will start the device */
   sprintf(msg, "Start TU007.. bps %d \n",tucb_ptr->pr.bps);
   RPT_BUGGER(msg);

   bzero(&start_st,sizeof(mpa_start_t));
   /*
   ** start the card in electron wrap mode ( needs no wrap plug )
   ** also turn test on. in port_b of 8255.
   ** Gate internal clock on.
   */

   tucb_ptr->pr.bps = 50;           /* fix bps rate for this test */
   start_st.data_proto |= SDLC;
   start_st.modem_flags |= (GATE_INT_CLK|ELECTRONIC_WRAP);
   start_st.port_b_8255 |= (SPEED_SEL_OFF|SEL_STANBY_OFF|FREE_STAT_CHG);
   start_st.baud_rate = tucb_ptr->pr.bps;           /* set up bps rate for wrap */
   if(tucb_ptr->pr.nrzi) start_st.data_flags |= SET_NRZI_DATA;
   start_st.xfer_mode |= SET_NO_DMA;      /* PIO required for wrap */
   start_st.station_type |= PRIMARY;      /* set up as primary station */
   start_st.sb.netid = 0;

   if (ioctl(fdes, CIO_START, &start_st) == -1) {
       sprintf(msg,"ioctl, CIO_START "
       "failed %d %s\n",errno,sys_errlist[errno]);
       RPT_BUGGER(msg);
#ifndef DIAGS
       RPT_INFO(msg);
       INC_BAD_OTHERS;
#endif
       return RC_SYS_ERRNO;
   }

   if( (rc = wait_status(fdes,CIO_START_DONE, tucb_ptr)) ) {
       sprintf(msg,"Wait START DONE failed: rc = %d\n",rc);
       RPT_BUGGER(msg);
#ifndef DIAGS
       RPT_INFO(msg);
       INC_BAD_OTHERS;
#endif
       return rc;
   }

   /* first get the current value of port B on 8255 */
   bzero(&data,sizeof(rw_port_t));     /* clear the input structure */
   data.port_addr = PORT_B_8255; /* set up address for pos 0 */
   data.rw_flag |= MPA_READ;     /* tell driver to do read for us */
   if (ioctl(fdes, MPA_RW_PORT, &data)) {
       sprintf(msg, "Ioctl read port B 8255 failed :  "
       "%s \n",sys_errlist[errno]);
	RPT_BUGGER(msg);
#ifndef DIAGS
	RPT_INFO(msg);
	hxfupdate(UPDATE, &ps);
#endif
	rc = RC_SYS_ERRNO;
	goto end_tu007;
   }
   sprintf(msg,"     Inital Value of port B 8255 is %02X\n",data.value);
   RPT_BUGGER(msg);

   /* is test turned on */
   if(data.value & TEST_OFF) {
      /* turn it on */
      sprintf(msg,"TEST MODE  was not on Port B 8255 \n");
      RPT_BUGGER(msg);
#ifndef DIAGS
       INC_BAD_OTHERS;
       RPT_INFO(msg);
#endif
      rc = RC_SOFT;
      goto end_tu007;
   }
   /* speed select is set to 1 by cio_start but check it here to be sure */
   if(!(data.value&SPEED_SEL_OFF))
      data.value = data.value|SPEED_SEL_OFF;
				/* save the contents of port b */
				/* and turn off rate select = 1 */
   if (!(data.value&SEL_STANBY_OFF))
      data.value = data.value|SEL_STANBY_OFF;
   if (!(data.value&FREE_STAT_CHG))
      data.value = data.value|FREE_STAT_CHG;

   port_b_value = data.value;
   sprintf(msg,"     Save port b value is %02X\n",port_b_value);
   RPT_BUGGER(msg);

   usleep(100000);


   sprintf(msg,"********* TEST timer 0 output.\n");
   RPT_BUGGER(msg);

   /* first get the current value of port C on 8255 */
   bzero(&data,sizeof(rw_port_t));     /* clear the input structure */
   data.port_addr = PORT_C_8255;
   data.rw_flag |= MPA_READ;     /* tell driver to do read for us */
   if (ioctl(fdes, MPA_RW_PORT, &data)) {
       sprintf(msg, "Ioctl read port C 8255 failed :  "
       "%s \n",sys_errlist[errno]);
	RPT_BUGGER(msg);
#ifndef DIAGS
	RPT_INFO(msg);
	hxfupdate(UPDATE, &ps);
#endif
	rc = RC_SYS_ERRNO;
	goto end_tu007;
   }
   sprintf(msg,"     Inital Value of port C 8255 is %02X\n",data.value);
   RPT_BUGGER(msg);

   /* is test indicate active in electronic wrap turned on */
   if(data.value & TEST_ACTIVE) {
      /* turn it on */
      sprintf(msg,"TEST INDICATE was not on Port C 8255 \n");
      RPT_BUGGER(msg);
#ifndef DIAGS
       INC_BAD_OTHERS;
       RPT_INFO(msg);
#endif
      rc = RC_SOFT;
      goto end_tu007;
   }

   sprintf(msg,"********* TEST DTR -> DSR \n");
   RPT_BUGGER(msg);
   usleep(100000);

   /* DTR should be on by default,driver sets DTR on cio_start */
   /* first reset DTR to 0 then check state of DSR               */
   /* Note the DSR changed bit in 8273 port A never changes      */
   /* but the DSR active bit will change, I will check for DSR   */
   /* toggle in 8273 port a reg.                                 */

   sprintf(msg,"     *********Turn DTR to off.\n");
   RPT_BUGGER(msg);
   bzero(&sess_blk,sizeof(cmd_phase_t));     /* clear the input structure */
   sess_blk.cmd = RESET_8273_PORT_B_CMD;
   sess_blk.parm_count = 1;
   sess_blk.parm[0]=0xCA;    /* mask to reset all valid bits */
   if (ioctl(fdes, MPA_CMD_8273, &sess_blk) == -1) {
	  sprintf(msg,"ioctl, MPA_CMD_8273 "
	  "failed %d %s\n",errno,sys_errlist[errno]);
	  RPT_BUGGER(msg);
#ifndef DIAGS
	  RPT_INFO(msg);
	  INC_BAD_OTHERS;
#endif
	  rc = RC_SYS_ERRNO;
	  goto end_tu007;
   }

   usleep(100000);
   /* check to see if DSR active is 0 in 8273 port a reg */
   bzero(&sess_blk,sizeof(cmd_phase_t));     /* clear the input structure */
   sess_blk.cmd = READ_8273_PORT_A_CMD;
   sess_blk.parm_count = 0;
   sess_blk.flag = RETURN_RESULT;

   if (ioctl(fdes, MPA_CMD_8273, &sess_blk) == -1) {
	  sprintf(msg,"ioctl, MPA_CMD_8273 "
	  "failed %d %s\n",errno,sys_errlist[errno]);
	  RPT_BUGGER(msg);
#ifndef DIAGS
	  RPT_INFO(msg);
	  INC_BAD_OTHERS;
#endif
	  rc = RC_SYS_ERRNO;
	  goto end_tu007;
   }
   sprintf(msg,"     Read %02X from 8273 port a.check for DSR off.\n",sess_blk.result);
   RPT_BUGGER(msg);

   if(sess_blk.result & PORT_A_8273_PA2) {
      sprintf(msg,".expected DSR to be off.\n",sess_blk.result);
      RPT_BUGGER(msg);
#ifndef DIAGS
       INC_BAD_OTHERS;
       RPT_INFO(msg);
#endif
      rc = RC_SOFT;
      goto end_tu007;
   }

   usleep(100000);

   /* Now toggle DTR back on */
   sprintf(msg,"     *********Turn DTR to on.\n");
   RPT_BUGGER(msg);

   bzero(&sess_blk,sizeof(cmd_phase_t));     /* clear the input structure */
   sess_blk.cmd = SET_8273_PORT_B_CMD;
   sess_blk.parm_count = 1;
   sess_blk.parm[0]=SET_8273_PORT_B_PB2;
   if (ioctl(fdes, MPA_CMD_8273, &sess_blk) == -1) {
	  sprintf(msg,"ioctl, MPA_CMD_8273 "
	  "failed %d %s\n",errno,sys_errlist[errno]);
	  RPT_BUGGER(msg);
#ifndef DIAGS
	  RPT_INFO(msg);
	  INC_BAD_OTHERS;
#endif
	  rc = RC_SYS_ERRNO;
	  goto end_tu007;
   }
   usleep(100000);
   /* check to see if DSR active is 1 in 8273 port a reg */
   bzero(&sess_blk,sizeof(cmd_phase_t));     /* clear the input structure */
   sess_blk.cmd = READ_8273_PORT_A_CMD;
   sess_blk.parm_count = 0;
   sess_blk.flag = RETURN_RESULT;

   if (ioctl(fdes, MPA_CMD_8273, &sess_blk) == -1) {
	  sprintf(msg,"ioctl, MPA_CMD_8273 "
	  "failed %d %s\n",errno,sys_errlist[errno]);
	  RPT_BUGGER(msg);
#ifndef DIAGS
	  RPT_INFO(msg);
	  INC_BAD_OTHERS;
#endif
	  rc = RC_SYS_ERRNO;
	  goto end_tu007;
   }
   sprintf(msg,"     Read %02X from 8273 port a.check for DSR on.\n",sess_blk.result);
   RPT_BUGGER(msg);

   if( !(sess_blk.result & PORT_A_8273_PA2) ) {
      sprintf(msg,"Read %02X from 8273 port a..expected DSR to be on.\n",sess_blk.result);
      RPT_BUGGER(msg);
#ifndef DIAGS
       INC_BAD_OTHERS;
       RPT_INFO(msg);
#endif
      rc = RC_SOFT;
      goto end_tu007;
   }

   usleep(100000);
   /*
   ** This next test does RTS -> CTS ..
   ** The check for CTS is done in 8255 port a, since
   ** the bits seem to be stuck in 8273 port a. Also the CTS
   ** change bit is stuck in 8273 port a and there are no other
   ** places to check this status, so I can't test it in electronic
   ** wrap mode.
   */
   sprintf(msg,"********* TEST RTS -> CTS.\n");
   RPT_BUGGER(msg);

   sprintf(msg,"     *********Turn RTS to off.\n");
   RPT_BUGGER(msg);
   bzero(&sess_blk,sizeof(cmd_phase_t));     /* clear the input structure */
   sess_blk.cmd = RESET_8273_PORT_B_CMD;
   sess_blk.parm_count = 1;
   sess_blk.parm[0]=0xCA;    /* mask to reset all valid bits */
   if (ioctl(fdes, MPA_CMD_8273, &sess_blk) == -1) {
	  sprintf(msg,"ioctl, MPA_CMD_8273 "
	  "failed %d %s\n",errno,sys_errlist[errno]);
	  RPT_BUGGER(msg);
#ifndef DIAGS
	  RPT_INFO(msg);
	  INC_BAD_OTHERS;
#endif
	  rc = RC_SYS_ERRNO;
	  goto end_tu007;
   }

   usleep(100000);
   /* check to see if CTS active is 1 in 8255 port A reg (CTS off) */
   bzero(&data,sizeof(rw_port_t));     /* clear the input structure */
   data.port_addr = PORT_A_8255;
   data.rw_flag |= MPA_READ;
   if (ioctl(fdes, MPA_RW_PORT, &data)) {
       sprintf(msg, "Ioctl read port A 8255 failed :  "
       "%s \n",sys_errlist[errno]);
	RPT_BUGGER(msg);
#ifndef DIAGS
	RPT_INFO(msg);
	hxfupdate(UPDATE, &ps);
#endif
	rc = RC_SYS_ERRNO;
	goto end_tu007;
   }
   sprintf(msg,"     CHK CTS: Read Value of port A 8255 is %02X\n",data.value);
   RPT_BUGGER(msg);
   /* is CTS 1 */
   if( !(data.value & CLEAR_TO_SEND) ) {
	sprintf(msg,"Expected CTS to be 1, not active \n");
	RPT_BUGGER(msg);
#ifndef DIAGS
       INC_BAD_OTHERS;
       RPT_INFO(msg);
#endif
	rc = RC_SOFT;
	goto end_tu007;
   }

   usleep(100000);
   /* Now toggle RTS back on */
   sprintf(msg,"     *********Turn RTS to on.\n");
   RPT_BUGGER(msg);

   bzero(&sess_blk,sizeof(cmd_phase_t));     /* clear the input structure */
   sess_blk.cmd = SET_8273_PORT_B_CMD;
   sess_blk.parm_count = 1;
   sess_blk.parm[0]=SET_8273_PORT_B_RTS;
   if (ioctl(fdes, MPA_CMD_8273, &sess_blk) == -1) {
	  sprintf(msg,"ioctl, MPA_CMD_8273 "
	  "failed %d %s\n",errno,sys_errlist[errno]);
	  RPT_BUGGER(msg);
#ifndef DIAGS
	  RPT_INFO(msg);
	  INC_BAD_OTHERS;
#endif
	  rc = RC_SYS_ERRNO;
	  goto end_tu007;
   }
   usleep(100000);
   /* check to see if CTS active is 0 in 8255 port A reg (CTS on) */

   bzero(&data,sizeof(rw_port_t));     /* clear the input structure */
   data.port_addr = PORT_A_8255;
   data.rw_flag |= MPA_READ;
   if (ioctl(fdes, MPA_RW_PORT, &data)) {
       sprintf(msg, "Ioctl read port A 8255 failed :  "
       "%s \n",sys_errlist[errno]);
	RPT_BUGGER(msg);
#ifndef DIAGS
	RPT_INFO(msg);
	hxfupdate(UPDATE, &ps);
#endif
	rc = RC_SYS_ERRNO;
	goto end_tu007;
   }
   sprintf(msg,"     CHK CTS: Read Value of port A 8255 is %02X\n",data.value);
   RPT_BUGGER(msg);
   /* is CTS 0 */
   if( (data.value & CLEAR_TO_SEND) ) {
	sprintf(msg,"Expected CTS to be 0, active \n");
	RPT_BUGGER(msg);
#ifndef DIAGS
       INC_BAD_OTHERS;
       RPT_INFO(msg);
#endif
	rc = RC_SOFT;
	goto end_tu007;
   }



   /* do Standby and RI test */
   sprintf(msg,"********* TEST STANDBY -> RI.\n");
   RPT_BUGGER(msg);
   bzero(&data,sizeof(rw_port_t));     /* clear the input structure */
   data.port_addr = PORT_B_8255;
   data.rw_flag |= MPA_WRITE;    /* tell driver to do write for us */
   data.value = port_b_value&0XFD;    /* set standby  0 */
   if (ioctl(fdes, MPA_RW_PORT, &data)) {
       sprintf(msg, "Ioctl write port B 8255 failed :  "
       "%s \n",sys_errlist[errno]);
	RPT_BUGGER(msg);
#ifndef DIAGS
	RPT_INFO(msg);
	hxfupdate(UPDATE, &ps);
#endif
	rc = RC_SYS_ERRNO;
	goto end_tu007;
   }
   sprintf(msg,"     wrote %02X to port B 8255 to turn on rate select\n",data.value);
   RPT_BUGGER(msg);

   usleep(100000);

   /* now get port A value with RI value */
   bzero(&data,sizeof(rw_port_t));     /* clear the input structure */
   data.port_addr = PORT_A_8255;
   data.rw_flag |= MPA_READ;
   if (ioctl(fdes, MPA_RW_PORT, &data)) {
       sprintf(msg, "Ioctl read port A 8255 failed :  "
       "%s \n",sys_errlist[errno]);
	RPT_BUGGER(msg);
#ifndef DIAGS
	RPT_INFO(msg);
	hxfupdate(UPDATE, &ps);
#endif
	rc = RC_SYS_ERRNO;
	goto end_tu007;
   }
   sprintf(msg,"     Read Value of port A 8255 is %02X\n",data.value);
   RPT_BUGGER(msg);
   /* is RI 0 active */
   if( (data.value & RING_ON) ) {
	sprintf(msg,"Expected RI to be 0\n");
	RPT_BUGGER(msg);
#ifndef DIAGS
       INC_BAD_OTHERS;
       RPT_INFO(msg);
#endif
	rc = RC_SOFT;
	goto end_tu007;
   }


   usleep(100000);
   /* now set stadby off and test RI == 1 */
   bzero(&data,sizeof(rw_port_t));     /* clear the input structure */
   data.port_addr = PORT_B_8255; /* set up address for pos 0 */
   data.rw_flag |= MPA_WRITE;    /* tell driver to do write for us */
   data.value = port_b_value;
   if (ioctl(fdes, MPA_RW_PORT, &data)) {
       sprintf(msg, "Ioctl write port B 8255 failed :  "
       "%s \n",sys_errlist[errno]);
	RPT_BUGGER(msg);
#ifndef DIAGS
	RPT_INFO(msg);
	hxfupdate(UPDATE, &ps);
#endif
	rc = RC_SYS_ERRNO;
	goto end_tu007;
   }
   sprintf(msg,"     wrote %02X to port B 8255 to turn off rate select\n",data.value);
   RPT_BUGGER(msg);

   usleep(100000);

   /* now get port A value with RI pins */
   bzero(&data,sizeof(rw_port_t));     /* clear the input structure */
   data.port_addr = PORT_A_8255;
   data.rw_flag |= MPA_READ;
   if (ioctl(fdes, MPA_RW_PORT, &data)) {
       sprintf(msg, "Ioctl read port A 8255 failed :  "
       "%s \n",sys_errlist[errno]);
	RPT_BUGGER(msg);
#ifndef DIAGS
	RPT_INFO(msg);
	hxfupdate(UPDATE, &ps);
#endif
	rc = RC_SYS_ERRNO;
	goto end_tu007;
   }
   sprintf(msg,"     Read Value of port A 8255 is %02X\n",data.value);
   RPT_BUGGER(msg);
   /* is RI 1 */
   if( !(data.value & RING_ON) ) {
	sprintf(msg,"Expected RI to be 1\n");
	RPT_BUGGER(msg);
#ifndef DIAGS
       INC_BAD_OTHERS;
       RPT_INFO(msg);
#endif
	rc = RC_SOFT;
	goto end_tu007;
   }

   /* do rate select and CD */
   sprintf(msg,"********* TEST RATE_SELECT -> CD.\n");
   RPT_BUGGER(msg);
   bzero(&data,sizeof(rw_port_t));     /* clear the input structure */
   data.port_addr = PORT_B_8255;
   data.rw_flag |= MPA_WRITE;    /* tell driver to do write for us */
   data.value = port_b_value&0XFE;    /* set rate select to 0 */
   if (ioctl(fdes, MPA_RW_PORT, &data)) {
       sprintf(msg, "Ioctl write port B 8255 failed :  "
       "%s \n",sys_errlist[errno]);
	RPT_BUGGER(msg);
#ifndef DIAGS
	RPT_INFO(msg);
	hxfupdate(UPDATE, &ps);
#endif
	rc = RC_SYS_ERRNO;
	goto end_tu007;
   }
   sprintf(msg,"     wrote %02X to port B 8255 to turn on rate select\n",data.value);
   RPT_BUGGER(msg);

   usleep(100000);

   /* now get port A value with CD pins */
   bzero(&data,sizeof(rw_port_t));     /* clear the input structure */
   data.port_addr = PORT_A_8255;
   data.rw_flag |= MPA_READ;
   if (ioctl(fdes, MPA_RW_PORT, &data)) {
       sprintf(msg, "Ioctl read port A 8255 failed :  "
       "%s \n",sys_errlist[errno]);
	RPT_BUGGER(msg);
#ifndef DIAGS
	RPT_INFO(msg);
	hxfupdate(UPDATE, &ps);
#endif
	rc = RC_SYS_ERRNO;
	goto end_tu007;
   }
   sprintf(msg,"     Read Value of port A 8255 is %02X\n",data.value);
   RPT_BUGGER(msg);
   /* is CD  0 */
   if( (data.value & CARRIER_DET) ) {
	sprintf(msg,"Expected CD to be 0\n");
	RPT_BUGGER(msg);
#ifndef DIAGS
       INC_BAD_OTHERS;
       RPT_INFO(msg);
#endif
	rc = RC_SOFT;
	goto end_tu007;
   }

   usleep(100000);
   /* now set rate select off and CD == 1 */
   bzero(&data,sizeof(rw_port_t));     /* clear the input structure */
   data.port_addr = PORT_B_8255; /* set up address for pos 0 */
   data.rw_flag |= MPA_WRITE;    /* tell driver to do write for us */
   data.value = port_b_value;
   if (ioctl(fdes, MPA_RW_PORT, &data)) {
       sprintf(msg, "Ioctl write port B 8255 failed :  "
       "%s \n",sys_errlist[errno]);
	RPT_BUGGER(msg);
#ifndef DIAGS
	RPT_INFO(msg);
	hxfupdate(UPDATE, &ps);
#endif
	rc = RC_SYS_ERRNO;
	goto end_tu007;
   }
   sprintf(msg,"     wrote %02X to port B 8255 to turn off rate select\n",data.value);
   RPT_BUGGER(msg);

   usleep(100000);

   /* now get port A value with CD pins */
   bzero(&data,sizeof(rw_port_t));     /* clear the input structure */
   data.port_addr = PORT_A_8255;
   data.rw_flag |= MPA_READ;
   if (ioctl(fdes, MPA_RW_PORT, &data)) {
       sprintf(msg, "Ioctl read port A 8255 failed :  "
       "%s \n",sys_errlist[errno]);
	RPT_BUGGER(msg);
#ifndef DIAGS
	RPT_INFO(msg);
	hxfupdate(UPDATE, &ps);
#endif
	rc = RC_SYS_ERRNO;
	goto end_tu007;
   }
   sprintf(msg,"     Read Value of port A 8255 is %02X\n",data.value);
   RPT_BUGGER(msg);
   /* is CD 1 */
   if( !(data.value & CARRIER_DET) ) {
	sprintf(msg,"Expected CD to be 1\n");
	RPT_BUGGER(msg);
#ifndef DIAGS
       INC_BAD_OTHERS;
       RPT_INFO(msg);
#endif
	rc = RC_SOFT;
	goto end_tu007;
   }


   usleep(100000);
   sprintf(msg,"********* TEST TEST -> TEST INDICATE .\n");
   RPT_BUGGER(msg);


   /* now turn off test in 8255 port b reg and see if test indicate */
   /* follows.                                                     */

   sprintf(msg,"     Turn off test in port b 8255\n");
   RPT_BUGGER(msg);
   bzero(&data,sizeof(rw_port_t));     /* clear the input structure */
   data.port_addr = PORT_B_8255;
   data.rw_flag |= MPA_WRITE;
   data.value = port_b_value|0x04;
   if (ioctl(fdes, MPA_RW_PORT, &data)) {
       sprintf(msg, "Ioctl read port A 8255 failed :  "
       "%s \n",sys_errlist[errno]);
	RPT_BUGGER(msg);
#ifndef DIAGS
	RPT_INFO(msg);
	hxfupdate(UPDATE, &ps);
#endif
	rc = RC_SYS_ERRNO;
	goto end_tu007;
   }

   usleep(100000);

   /* get port c value from 8255 */
   bzero(&data,sizeof(rw_port_t));     /* clear the input structure */
   data.port_addr = PORT_C_8255;
   data.rw_flag |= MPA_READ;     /* tell driver to do read for us */
   if (ioctl(fdes, MPA_RW_PORT, &data)) {
       sprintf(msg, "Ioctl read port C 8255 failed :  "
       "%s \n",sys_errlist[errno]);
	RPT_BUGGER(msg);
#ifndef DIAGS
	RPT_INFO(msg);
	hxfupdate(UPDATE, &ps);
#endif
	rc = RC_SYS_ERRNO;
	goto end_tu007;
   }
   sprintf(msg,"     final Value of port C 8255 is %02X\n",data.value);
   RPT_BUGGER(msg);

   /* is test indicate active in electronic wrap turned off */
   if(!(data.value & TEST_ACTIVE)) {
      /* turn it on */
      sprintf(msg,"TEST INDICATE was not off Port C 8255 \n");
      RPT_BUGGER(msg);
#ifndef DIAGS
       INC_BAD_OTHERS;
       RPT_INFO(msg);
#endif
      rc = RC_SOFT;
      goto end_tu007;
   }


   sprintf(msg,"********* TEST CTS CHANGED logic .\n");
   RPT_BUGGER(msg);


   /* 1. make sure e-wrap is off in port c 8255 */
   bzero(&data,sizeof(rw_port_t));     /* clear the input structure */
   data.port_addr = PORT_C_8255;
   data.rw_flag |= MPA_WRITE;
   data.value = 0xF0;
   if (ioctl(fdes, MPA_RW_PORT, &data)) {
       sprintf(msg, "Ioctl read port B 8255 failed :  "
       "%s \n",sys_errlist[errno]);
	RPT_BUGGER(msg);
#ifndef DIAGS
	RPT_INFO(msg);
	hxfupdate(UPDATE, &ps);
#endif
	rc = RC_SYS_ERRNO;
	goto end_tu007;
   }
   sprintf(msg,"     Write %02X, to port c 8255. e-wrap off\n",data.value);
   RPT_BUGGER(msg);

   /* 2. make sure RTS and DTR are off in port b 8273*/
   usleep(100000);
   bzero(&sess_blk,sizeof(cmd_phase_t));     /* clear the input structure */
   sess_blk.cmd =  RESET_8273_PORT_B_CMD;
   sess_blk.parm_count = 1;
   sess_blk.parm[0]=RESET_8273_PORT_B_RTS&RESET_8273_PORT_B_PB2;
   if (ioctl(fdes, MPA_CMD_8273, &sess_blk) == -1) {
	  sprintf(msg,"ioctl, MPA_CMD_8273 "
	  "failed %d %s\n",errno,sys_errlist[errno]);
	  RPT_BUGGER(msg);
#ifndef DIAGS
	  RPT_INFO(msg);
	  INC_BAD_OTHERS;
#endif
	  rc = RC_SYS_ERRNO;
	  goto end_tu007;
   }
   sprintf(msg,"     Reset RTS and DTR bits in port b 8273\n");
   RPT_BUGGER(msg);

   /* 3. turn off test port b 8255 */
   port_b_value |= TEST_OFF;

   /* 4. reset the status changed logic first */
   bzero(&data,sizeof(rw_port_t));     /* clear the input structure */
   data.port_addr = PORT_B_8255;
   data.rw_flag |= MPA_WRITE;
   data.value = port_b_value & ~FREE_STAT_CHG;
   if (ioctl(fdes, MPA_RW_PORT, &data)) {
       sprintf(msg, "Ioctl read port A 8255 failed :  "
       "%s \n",sys_errlist[errno]);
	RPT_BUGGER(msg);
#ifndef DIAGS
	RPT_INFO(msg);
	hxfupdate(UPDATE, &ps);
#endif
	rc = RC_SYS_ERRNO;
	goto end_tu007;
   }
   sprintf(msg,"     Reset modem stat chg logic write %02X, to port b 8255.\n",data.value);
   RPT_BUGGER(msg);

   /* remove reset condition */
   bzero(&data,sizeof(rw_port_t));     /* clear the input structure */
   data.port_addr = PORT_B_8255;
   data.rw_flag |= MPA_WRITE;
   data.value = port_b_value | FREE_STAT_CHG;
   if (ioctl(fdes, MPA_RW_PORT, &data)) {
       sprintf(msg, "Ioctl write port B 8255 failed :  "
       "%s \n",sys_errlist[errno]);
	RPT_BUGGER(msg);
#ifndef DIAGS
	RPT_INFO(msg);
	hxfupdate(UPDATE, &ps);
#endif
	rc = RC_SYS_ERRNO;
	goto end_tu007;
   }
   sprintf(msg,"     Set modem stat chg logic write %02X, to port b 8255.\n",data.value);
   RPT_BUGGER(msg);


   /* 5. get current state of CTS and DSR changed bits from port a 8273 */
   usleep(100000);
   bzero(&sess_blk,sizeof(cmd_phase_t));     /* clear the input structure */
   sess_blk.cmd = READ_8273_PORT_A_CMD;
   sess_blk.parm_count = 0;
   sess_blk.flag = RETURN_RESULT;

   if (ioctl(fdes, MPA_CMD_8273, &sess_blk) == -1) {
	  sprintf(msg,"ioctl, MPA_CMD_8273 "
	  "failed %d %s\n",errno,sys_errlist[errno]);
	  RPT_BUGGER(msg);
#ifndef DIAGS
	  RPT_INFO(msg);
	  INC_BAD_OTHERS;
#endif
	  rc = RC_SYS_ERRNO;
	  goto end_tu007;
   }
   sprintf(msg,"     Read %02X 8273 port a. check CTS and DSR change=1.\n",sess_blk.result);
   RPT_BUGGER(msg);

   /* save port a value and strip out all but CTS changed bit value */
   /* CTS changed after reset should be 1, so check it here */
   if(!(sess_blk.result&PORT_A_8273_PA3)) {
      sprintf(msg,"CTS change test failed, not 1 after reset.\n");
      RPT_BUGGER(msg);
#ifndef DIAGS
       INC_BAD_OTHERS;
       RPT_INFO(msg);
#endif
      rc = RC_SOFT;
      goto end_tu007;
   }
   if(!(sess_blk.result&PORT_A_8273_PA4)) {
      sprintf(msg,"DSR change test failed, not 1 after reset.\n");
      RPT_BUGGER(msg);
#ifndef DIAGS
       INC_BAD_OTHERS;
       RPT_INFO(msg);
#endif
      rc = RC_SOFT;
      goto end_tu007;
   }

   /* 6. toggle on electronic wrap bit in port c 8255 */

   usleep(100000);
   /* toggle electron wrap bit on */
   bzero(&data,sizeof(rw_port_t));     /* clear the input structure */
   data.port_addr = PORT_C_8255;
   data.rw_flag |= MPA_WRITE;
   data.value = 0xF4;
   if (ioctl(fdes, MPA_RW_PORT, &data)) {
       sprintf(msg, "Ioctl read port C 8255 failed :  "
       "%s \n",sys_errlist[errno]);
	RPT_BUGGER(msg);
#ifndef DIAGS
	RPT_INFO(msg);
	hxfupdate(UPDATE, &ps);
#endif
	rc = RC_SYS_ERRNO;
	goto end_tu007;
   }
   sprintf(msg,"     Change e-bit port C 8255 write %02X\n",data.value);
   RPT_BUGGER(msg);

   /* 7. toggle on RTS in port b 8273 */

   usleep(100000);
   bzero(&sess_blk,sizeof(cmd_phase_t));     /* clear the input structure */
   sess_blk.cmd = SET_8273_PORT_B_CMD;
   sess_blk.parm_count = 1;
   sess_blk.parm[0]=SET_8273_PORT_B_RTS|SET_8273_PORT_B_PB2;
   if (ioctl(fdes, MPA_CMD_8273, &sess_blk) == -1) {
	  sprintf(msg,"ioctl, MPA_CMD_8273 "
	  "failed %d %s\n",errno,sys_errlist[errno]);
	  RPT_BUGGER(msg);
#ifndef DIAGS
	  RPT_INFO(msg);
	  INC_BAD_OTHERS;
#endif
	  rc = RC_SYS_ERRNO;
	  goto end_tu007;
   }
   sprintf(msg,"     Set cmd to port b, mask is %02X.. set RTS and DTR \n",SET_8273_PORT_B_RTS|SET_8273_PORT_B_PB2);
   RPT_BUGGER(msg);

   /* 8. toggle off electronic wrap bit in port c 8255 */

   usleep(100000);
   /* toggle electron wrap bit */
   bzero(&data,sizeof(rw_port_t));     /* clear the input structure */
   data.port_addr = PORT_C_8255;
   data.rw_flag |= MPA_WRITE;
   data.value = 0xF0;
   if (ioctl(fdes, MPA_RW_PORT, &data)) {
       sprintf(msg, "Ioctl read port C 8255 failed :  "
       "%s \n",sys_errlist[errno]);
	RPT_BUGGER(msg);
#ifndef DIAGS
	RPT_INFO(msg);
	hxfupdate(UPDATE, &ps);
#endif
	rc = RC_SYS_ERRNO;
	goto end_tu007;
   }
   sprintf(msg,"     Change e-bit port C 8255 write %02X\n",data.value);
   RPT_BUGGER(msg);


   /* 9. check the state to CTS and DSR changed now and compare to original value */

   usleep(100000);
   bzero(&sess_blk,sizeof(cmd_phase_t));     /* clear the input structure */
   sess_blk.cmd = READ_8273_PORT_A_CMD;
   sess_blk.parm_count = 0;
   sess_blk.flag = RETURN_RESULT;

   if (ioctl(fdes, MPA_CMD_8273, &sess_blk) == -1) {
	  sprintf(msg,"ioctl, MPA_CMD_8273 "
	  "failed %d %s\n",errno,sys_errlist[errno]);
	  RPT_BUGGER(msg);
#ifndef DIAGS
	  RPT_INFO(msg);
	  INC_BAD_OTHERS;
#endif
	  rc = RC_SYS_ERRNO;
	  goto end_tu007;
   }
   sprintf(msg,"     Read %02X from 8273 port a. check CTS and DSR change.\n",sess_blk.result);
   RPT_BUGGER(msg);
   if(sess_blk.result&PORT_A_8273_PA3) {   /* should be 0 here */
      sprintf(msg,"CTS change test failed , expected 0\n");
      RPT_BUGGER(msg);
#ifndef DIAGS
       INC_BAD_OTHERS;
       RPT_INFO(msg);
#endif
      rc = RC_SOFT;
      goto end_tu007;
   }
   if(sess_blk.result&PORT_A_8273_PA4) {   /* should be 0 here */
      sprintf(msg,"DSR change test failed , expected 0\n");
      RPT_BUGGER(msg);
#ifndef DIAGS
       INC_BAD_OTHERS;
       RPT_INFO(msg);
#endif
      rc = RC_SOFT;
      goto end_tu007;
   }



end_tu007:

   bzero(&start_st,sizeof(mpa_start_t));
   start_st.sb.netid = 0;
   if (ioctl(fdes, CIO_HALT, &start_st) == -1) {
       sprintf(msg,"Write failed %d %s\n",fdes,sys_errlist[errno]);
       RPT_BUGGER(msg);
#ifndef DIAGS
       INC_BAD_OTHERS;
       RPT_INFO(msg);
#endif
       return RC_SYS_ERRNO;
   }

   if( wait_status(fdes,CIO_HALT_DONE, tucb_ptr) ) {
       sprintf(msg,"Wait for HALT_DONE failed: \n");
       RPT_BUGGER(msg);
#ifndef DIAGS
       RPT_INFO(msg);
       INC_BAD_OTHERS;
#endif
       return RC_SYS_ERRNO;
   }

   return(rc);
}    /* tu007()  */


