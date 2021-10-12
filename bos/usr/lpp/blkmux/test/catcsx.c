static char sccsid[] = "@(#)58	1.2  src/bos/usr/lpp/blkmux/test/catcsx.c, sysxcat, bos411, 9428A410j 9/30/93 17:37:47";
/*
 *
 * COMPONENT_NAME: (SYSXCAT) - Channel Attach device handler
 *
 * FUNCTIONS:  startsub(), opendev(), getout(), wait_status(), writedata(),
 *             readdata(), decode_status(), clawloop(), wait_excp()
 *
 * ORIGINS: 27
 *
 *
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1992
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <time.h>
#include <fcntl.h>
#include <sys/comio.h>
#include <sys/select.h>
#include <sys/poll.h>
#include <string.h>
#include <signal.h>
#include <sys/catuser.h>

time_t               ltime;
int                  device0,device1,device2,device3;
int                  dev0on=0,dev1on=0,dev2on=0,dev3on=0;
int                  subchn;         /* The subchannel to start       */
int                  subson0[20];     /* List of started subs dev0     */
int                  subson1[20];     /* List of started subs.   1     */
int                  subson2[20];     /* List of started subs.   2     */
int                  subson3[20];     /* List of started subs.   3     */
int                  event,opencount;
int                  do_over=0;
int                  link_op=0;
int                  claw=0;
int                  number=64;
int                  cur_bytes=0;
int                  last_bytes=0;
unsigned             seed=0;
int                  forever=0;
int                  donek=0;
char                 dev_id0[20];     /* The device name ex: /dev/cat0 */
char                 dev_id1[20];     /* The device name ex: /dev/cat1 */
char                 dev_id2[20];     /* The device name ex: /dev/cat2 */
char                 dev_id3[20];     /* The device name ex: /dev/cat3 */
char                *state;
struct cat_set_sub   setsub;

main()
{
       int   i;
       for(i=0; i<20; i++) {
	  subson0[i]=257;
	  subson1[i]=257;
	  subson2[i]=257;
	  subson3[i]=257;
       }

       while(1) {
	  while(1) {
	     do_over=0;
	     event = 5;
	     printf("\n\nSelect the action by entering the corresponding number\n\n");
	     printf("0)      Open a device.\n");
	     printf("1)      Start a subchannel on an open device.\n");
	     printf("2)      Write block of sequecnial data to channel\n");
	     printf("3)      Read block of data from channel\n");
	     /* 
		COMMENTED out with reference to DEFECT 72885.
		printf("4)      Run loopback under claw mode to channel.\n");
	     */
	     printf("4)      quit!\n");
	     scanf("%0d",&event);
	     switch(event) {
	       case 0:  opendev();
			break;
	       case 1:  startsub();
			break;
	       case 2:  writedata();
			break;
	       case 3:  readdata();
			break;
	     /* 
		COMMENTED out with reference to DEFECT 72885.
	       case 4:  clawloop();
			time(&ltime);
			printf(" Stop time is %s\n",ctime(&ltime));
 			printf("Current bytes %d, last bytes %d, loops %d\n",
				cur_bytes,last_bytes,donek);
			break;
	     */
	       case 4:  getout(0);
			break;
	       default:
		   printf("Bad choice try again.\n");
		   do_over = 1;
		   break;
	     }
	     if(!do_over) break;
	  }
       }
       getout(10);
}  /*  end main */
startsub()
{
       int        fd,i;
       char       dev_id[20];
       opencount=0;
       if(dev0on) ++opencount;
       if(dev1on) ++opencount;
       if(dev2on) ++opencount;
       if(dev3on) ++opencount;
       if(opencount>1) {
	  /* allow choice of which open device to start subchannel on */
	  while(1) {
	     do_over=0;
	     printf("\n\nSelect a device for the start subchannel.\n\n");
	     printf("0)      Start subchannel on /dev/cat0\n");
	     printf("1)      Start subchannel on /dev/cat1\n");
	     printf("2)      Start subchannel on /dev/cat2\n");
	     printf("3)      Start subchannel on /dev/cat3\n");
	     printf("4)      quit!\n");
	     scanf("%0d",&event);
	     switch(event) {
	       case 0:  fd = device0;
			strcpy(dev_id,dev_id0);
			if(!dev0on) {
			   printf("%s is not open\n",dev_id);
			   continue;
			}
			break;
	       case 1:  fd = device1;
			strcpy(dev_id,dev_id1);
			if(!dev1on) {
			   printf("%s is not open\n",dev_id);
			   continue;
			}
			break;
	       case 2:  fd = device2;
			strcpy(dev_id,dev_id2);
			if(!dev2on) {
			   printf("%s is not open\n",dev_id);
			   continue;
			}
			break;
	       case 3:  fd = device3;
			strcpy(dev_id,dev_id3);
			if(!dev3on) {
			   printf("%s is not open\n",dev_id);
			   continue;
			}
			break;
	       case 4:  return(0);
	       default:
		   printf("Bad choice try again.\n");
		   do_over = 1;
		   break;
	     }
	     if(!do_over) break;
	  }
       }
       else {
	 if(dev0on) {
	   fd = device0;
	   strcpy(dev_id,dev_id0);
	 }
	 if(dev1on) {
	   fd = device1;
	   strcpy(dev_id,dev_id1);
	 }
	 if(dev2on) {
	   fd = device2;
	   strcpy(dev_id,dev_id2);
	 }
	 if(dev3on) {
	   fd = device3;
	   strcpy(dev_id,dev_id3);
	 }
       }
       wait_excp(fd);
      
       /* 
           COMMENTED out with reference to DEFECT 72885.
           printf("Enter 1 for claw mode, 0 for non-claw mode.\n");
           scanf("%d",&claw);
       */

       printf("Enter the hex value of the subchannel you want started\n");
       /* 
           COMMENTED out with reference to DEFECT 72885.
       	   printf("Or the even subchannel of the claw mode subchannel pair.\n");
       */
       scanf("%02X",&subchn);
       /* is the subchannel already started */
     if (!claw) {
       switch(dev_id[8]) {
	  case '0':
		      for(i=0; i<20; i++) {
			 if(subson0[i]==subchn) {
			    printf("Sub %02X already started.\n",subchn);
			    return(1);
			 }
		      }
		      break;
	  case '1':
		      for(i=0; i<20; i++) {
			 if(subson1[i]==subchn) {
			    printf("Sub %02X already started.\n",subchn);
			    return(1);
			 }
		      }
		      break;
	  case '2':
		      for(i=0; i<20; i++) {
			 if(subson2[i]==subchn) {
			    printf("Sub %02X already started.\n",subchn);
			    return(1);
			 }
		      }
		      break;
	  case '3':
		      for(i=0; i<20; i++) {
			 if(subson3[i]==subchn) {
			    printf("Sub %02X already started.\n",subchn);
			    return(1);
			 }
		      }
		      break;
	  default: printf("%s is Bad choice for device\n",dev_id);
		   return(1);
       }
     }         /* end if not claw */
       if(claw) {
	   printf("Set up for claw mode, subs %02x and %02x.\n",subchn,subchn+1);
	   setsub.subset = 2;    /* number of subch's in group */
	   setsub.specmode = CAT_FLUSHX_MOD|CAT_CLAW_MOD;
	   /* the dd will return the linkid in this next field */
	   setsub.claw_blk.linkid = 0; /* init to 0 */
	   strcpy(setsub.claw_blk.WS_appl,"catclaw ");   /* appl name on rios */
	   strcpy(setsub.claw_blk.H_appl,"paceclaw");   /* appl name on host */
	   strcpy(setsub.claw_blk.WS_adap,"cat0rs1 ");   /* adap name on rios */
	   strcpy(setsub.claw_blk.H_name,"pace    ");   /* host name */
       }
       else {
	   setsub.subset = 1;    /* number of subch's in group */
	   setsub.specmode = CAT_FLUSHX_MOD;
       }
       setsub.shrtbusy = CAT_SB_CUE|CAT_SB_SM;;
       if(!claw) {
	  event=0;
	  printf("Enter 1 to start with unsolisited DE.\n");
	  scanf("%02X",&event);
	  if(event==1) setsub.startde = 1;   /* no device end to start */
	  else setsub.startde = 0;
	  setsub.set_default = 0;
       }
       else setsub.set_default = 1;
       setsub.scflags = 1;   /* don't ignore new definitions */
       setsub.reserved = 0;
       setsub.sb.netid = subchn;
       printf("Issue start.\n");

       if (ioctl(fd, CIO_START, &setsub) == -1) {
	  printf("ioctl, CIO_START "
	  "failed for device %s, status %08X\n",dev_id,setsub.sb.status);
	   return(1);
       }
       printf("Wait for start done.\n");
       if( wait_status(CIO_START_DONE,fd) ) {
	      printf("Failed to receive CIO_START_DONE for device %s "
	      "subchannel %02X\n",dev_id,subchn);
	      return(1);
       }
       switch(dev_id[8]) {
	  case '0':
		      for(i=0; i<20; i++) {
			 if(subson0[i]==257) {
			    subson0[i]=subchn;
			    break;
			 }
			 if (i==19) {
			    printf("To many subchannels open for %s.\n",dev_id);
			    return(1);
			 }
		      }
		      break;
	  case '1':
		      for(i=0; i<20; i++) {
			 if(subson1[i]==257) {
			    subson1[i]=subchn;
			    break;
			 }
			 if (i==19) {
			    printf("To many subchannels open for %s.\n",dev_id);
			    return(1);
			 }
		      }
		      break;
	  case '2':
		      for(i=0; i<20; i++) {
			 if(subson2[i]==257) {
			    subson2[i]=subchn;
			    break;
			 }
			 if (i==19) {
			    printf("To many subchannels open for %s.\n",dev_id);
			    return(1);
			 }
		      }
		      break;
	  case '3':
		      for(i=0; i<20; i++) {
			 if(subson3[i]==257) {
			    subson3[i]=subchn;
			    break;
			 }
			 if (i==19) {
			    printf("To many subchannels open for %s.\n",dev_id);
			    return(1);
			 }
		      }
		      break;
	  default: printf("%s is Bad choice for device\n",dev_id);
		   return(1);
       }
       printf("\n   Device %s has subchannel %02X online.\n",dev_id,subchn);
  return(0);
}
opendev()
{
       while(1) {
	  do_over=0;
	  printf("\n\nSelect the action by entering the corresponding number\n\n");
	  printf("NOTE: /dev/cat0 will usually be the card in the lowest # slot\n");
	  printf("0)      Open /dev/cat0\n");
	  printf("1)      Open /dev/cat1\n");
	  printf("2)      Open /dev/cat2\n");
	  printf("3)      Open /dev/cat3\n");
	  printf("4)      quit!\n");
	  scanf("%0d",&event);
	  switch(event) {
	    case 0:  strcpy(dev_id0,"/dev/cat0");
		     if ( (device0 =open(dev_id0,O_RDWR)) == -1) {
			 printf("Open of device: %s failed\n",dev_id0);
			 exit(1);
		     }
		     dev0on=1;
		     printf("\n    /dev/cat0 is open.\n");
		     break;
	    case 1:  strcpy(dev_id1,"/dev/cat1");
		     if ( (device1 =open(dev_id1,O_RDWR)) == -1) {
			 printf("Open of device: %s failed\n",dev_id1);
			 exit(1);
		     }
		     dev1on=1;
		     printf("\n    /dev/cat1 is open.\n");
		     break;
	    case 2:  strcpy(dev_id2,"/dev/cat2");
		     if ( (device2 =open(dev_id2,O_RDWR)) == -1) {
			 printf("Open of device: %s failed\n",dev_id2);
			 exit(1);
		     }
		     dev2on=1;
		     printf("\n    /dev/cat2 is open.\n");
		     break;
	    case 3:  strcpy(dev_id3,"/dev/cat3");
		     if ( (device3 =open(dev_id3,O_RDWR)) == -1) {
			 printf("Open of device: %s failed\n",dev_id3);
			 exit(1);
		     }
		     dev3on=1;
		     printf("\n    /dev/cat3 is open.\n");
		     break;
	    case 4:  return(0);
	    default:
		printf("Bad choice try again.\n");
		do_over = 1;
		break;
	  }
	  if(!do_over) break;
       }
    return(0);
}
getout(code)
	int    code;
{
       printf("Closing all open channel adapters and exit(%d).\n",code);
       close(device0);
       close(device1);
       close(device2);
       close(device3);
       exit(code);
}
wait_status (
	int     code,
	int     fdis)
{
	struct status_block     status;
	struct pollfd           pollblk;
	int                     i, rc, result;

	while (1) {
	    /*                                                          */
	    /*  Wait for a status block by polling for it:              */
	    /*                                                          */
	    pollblk.fd = fdis;
	    pollblk.reqevents = POLLPRI;        /* wait for exception */
	    pollblk.rtnevents = 0;
	    result = poll( &pollblk, 1, 60000 );
	    if (result < 0) {
		printf("Poll subroutine failed.\n");
		return(1);
	    }
	    if (result == 0) {
		printf("Timed out waiting for CIO_START to complete.\n");
		return(1);
	    } else {
		/*                                                      */
		/*  Status block is available -- issue ioctl to get it. */
		/*                                                      */
		result = ioctl( fdis, CIO_GET_STAT, &status );
		if (result < 0) {
		    printf("ioctl (CIO_GET_STATUS) failed.\n");
		    rc = 1;
		    break;
		}
		/*  Is this the code we are waiting for?  If not, loop  */
		/*  to next wait.                                       */
		/*                                                      */
		if (status.code == code) {
		    if (status.option[0] == CIO_OK) {
			rc = 0;                 /* succeeded */
			printf("Link_id returned was %08x\n",status.option[2]);
			link_op = status.option[2];
			printf("option[1] was %08x\n",status.option[1]);
			printf("option[3] was %08x\n",status.option[3]);
			break;
		    } else {
			rc = 1;               /* failed */
			printf("Bad status from CIO_START attempt\n");
			decode_status(&status);
			if(status.option[0]==CIO_NETID_INV) {
			   printf("Check smit cat, to see if subchannel is allowed\n");
			}
		    }
		    break;
		} else
		    continue;                           /* next block */
	    }
	}
	return(rc);
}
writedata ()
{
	cat_write_ext_t         we;
	int                     bytesw,bytes;
	int                     sub,fd,att,i,j,s;
	char                    *pat_ptr;
	char                    dev_id[20];
	char                    *malloc();

       opencount=0;
       if(dev0on) ++opencount;
       if(dev1on) ++opencount;
       if(dev2on) ++opencount;
       if(dev3on) ++opencount;
       if(opencount>1) {
	  /* allow choice of which open device to write */
	  while(1) {
	     do_over=0;
	     printf("\n\nSelect a device for the write.\n\n");
	     printf("0)      Write to /dev/cat0\n");
	     printf("1)      Write to /dev/cat1\n");
	     printf("2)      Write to /dev/cat2\n");
	     printf("3)      Write to /dev/cat3\n");
	     printf("4)      quit!\n");
	     scanf("%0d",&event);
	     switch(event) {
	       case 0:  fd = device0;
			strcpy(dev_id,dev_id0);
			if(!dev0on) {
			   printf("%s is not open\n",dev_id);
			   continue;
			}
			break;
	       case 1:  fd = device1;
			strcpy(dev_id,dev_id1);
			if(!dev1on) {
			   printf("%s is not open\n",dev_id);
			   continue;
			}
			break;
	       case 2:  fd = device2;
			strcpy(dev_id,dev_id2);
			if(!dev2on) {
			   printf("%s is not open\n",dev_id);
			   continue;
			}
			break;
	       case 3:  fd = device3;
			strcpy(dev_id,dev_id3);
			if(!dev3on) {
			   printf("%s is not open\n",dev_id);
			   continue;
			}
			break;
	       case 4:  return(0);
	       default:
		   printf("Bad choice try again.\n");
		   do_over = 1;
		   break;
	     }
	     if(!do_over) break;
	  }
       }
       else {
	 if(dev0on) {
	   fd = device0;
	   strcpy(dev_id,dev_id0);
	 }
	 if(dev1on) {
	   fd = device1;
	   strcpy(dev_id,dev_id1);
	 }
	 if(dev2on) {
	   fd = device2;
	   strcpy(dev_id,dev_id2);
	 }
	 if(dev3on) {
	   fd = device3;
	   strcpy(dev_id,dev_id3);
	 }
       }
       s=0;
       switch(dev_id[8]) {
	  case '0':
		      for(i=0; i<20; i++) {
			 if(subson0[i]!=257) {
			    ++s;
			    j=subson0[i];
			 }
		      }
		      if(s==1) sub=j;
		      else {
			printf("Enter subchannel to write in hex.\n");
			scanf("%d",&sub);
		      }
		      break;
	  case '1':
		      for(i=0; i<20; i++) {
			 if(subson1[i]!=257) {
			    ++s;
			    j=subson1[i];
			 }
		      }
		      if(s==1) sub=j;
		      else {
			printf("Enter subchannel to write in hex.\n");
			scanf("%d",&sub);
		      }
		      break;
	  case '2':
		      for(i=0; i<20; i++) {
			 if(subson2[i]!=257) {
			    ++s;
			    j=subson2[i];
			 }
		      }
		      if(s==1) sub=j;
		      else {
			printf("Enter subchannel to write in hex.\n");
			scanf("%d",&sub);
		      }
		      break;
	  case '3':
		      for(i=0; i<20; i++) {
			 if(subson3[i]!=257) {
			    ++s;
			    j=subson3[i];
			 }
		      }
		      if(s==1) sub=j;
		      else {
			printf("Enter subchannel to write in hex.\n");
			scanf("%d",&sub);
		      }
		      break;
	  default: printf("%s is Bad choice for device\n",dev_id);
		   return(1);
       }
       /* now is the one picked started */
       s=0;
       switch(dev_id[8]) {
	  case '0':
		      for(i=0; i<20; i++) {
			 if(subson0[i]==sub) {
			    s=1;
			 }
		      }
		      if(!s) {
			 printf("subchannel %02X is not started.\n",sub);
			 return(1);
		      }
		      break;
	  case '1':
		      for(i=0; i<20; i++) {
			 if(subson1[i]==sub) {
			    s=1;
			 }
		      }
		      if(!s) {
			 printf("subchannel %02X is not started.\n",sub);
			 return(1);
		      }
		      break;
	  case '2':
		      for(i=0; i<20; i++) {
			 if(subson2[i]==sub) {
			    s=1;
			 }
		      }
		      if(!s) {
			 printf("subchannel %02X is not started.\n",sub);
			 return(1);
		      }
		      break;
	  case '3':
		      for(i=0; i<20; i++) {
			 if(subson3[i]==sub) {
			    s=1;
			 }
		      }
		      if(!s) {
			 printf("subchannel %02X is not started.\n",sub);
			 return(1);
		      }
		      break;
	  default: printf("%s is Bad choice for device\n",dev_id);
		   return(1);
       }

       printf("Enter 1 if you want to send attention with write.\n");
       scanf("%d",&att);

       we.cio_ext.flag = 0;
       if(claw) we.cio_ext.write_id = link_op;
       else we.cio_ext.write_id = 0XFF;
       we.cio_ext.netid = sub;
       if(att!=1) {
	      we.attn_int = 0;           /* no attention w/write */
	      we.use_ccw = 0;           /*  no use ccw           */
	      we.ccw = 0x00;            /*  set ccw to 0x00 */
       }
       else {
	      we.attn_int = 1;           /*  attention w/write */
	      we.use_ccw = 1;           /*  use ccw           */
	      we.ccw = 0x03;            /*  set ccw to 0x03, att bsy fix */
       }

       printf("Enter the number of bytes to write in decimal.\n");
       scanf("%d",&bytes);

       if ((pat_ptr = (char *) malloc(bytes)) == NULL) {
	  printf("malloc failed to get space for %d bytes of write data\n");
	  return(1);
       }
       for(i=0; i<bytes; i++) {
	  pat_ptr[i] = i;
       }

       bytesw = writex(fd,pat_ptr,bytes,&we);

       if (bytesw == -1) {
	   printf("Write failed is device %s open?\n",
		 dev_id);
	   printf("Is subchannel 0x%02X started?\n",
		 sub);
	   printf("Write status = 0x%04x\n", we.cio_ext.status);
	   return(1);
       }
       printf("\n   %d bytes written to subchannel %02X on device %s\n",bytesw,sub,dev_id);
       return(0);
}
readdata()
{
	cat_read_ext_t          re;
	int                     bytesr,bytes,s,i,j;
	int                     sub,fd,att;
	char                    *pat_ptr;
	char                    dev_id[20];
	char                    *malloc();

       opencount=0;
       if(dev0on) ++opencount;
       if(dev1on) ++opencount;
       if(dev2on) ++opencount;
       if(dev3on) ++opencount;
       if(opencount>1) {
	  /* allow choice of which open device to read */
	  while(1) {
	     do_over=0;
	     printf("\n\nSelect a device for the read.\n\n");
	     printf("0)      Read from /dev/cat0\n");
	     printf("1)      Read from /dev/cat1\n");
	     printf("2)      Read from /dev/cat2\n");
	     printf("3)      Read from /dev/cat3\n");
	     printf("4)      quit!\n");
	     scanf("%0d",&event);
	     switch(event) {
	       case 0:  fd = device0;
			strcpy(dev_id,dev_id0);
			if(!dev0on) {
			   printf("%s is not open\n",dev_id);
			   continue;
			}
			break;
	       case 1:  fd = device1;
			strcpy(dev_id,dev_id1);
			if(!dev1on) {
			   printf("%s is not open\n",dev_id);
			   continue;
			}
			break;
	       case 2:  fd = device2;
			strcpy(dev_id,dev_id2);
			if(!dev2on) {
			   printf("%s is not open\n",dev_id);
			   continue;
			}
			break;
	       case 3:  fd = device3;
			strcpy(dev_id,dev_id3);
			if(!dev3on) {
			   printf("%s is not open\n",dev_id);
			   continue;
			}
			break;
	       case 4:  return(0);
	       default:
		   printf("Bad choice try again.\n");
		   do_over = 1;
		   break;
	     }
	     if(!do_over) break;
	  }
       }
       else {
	 if(dev0on) {
	   fd = device0;
	   strcpy(dev_id,dev_id0);
	 }
	 if(dev1on) {
	   fd = device1;
	   strcpy(dev_id,dev_id1);
	 }
	 if(dev2on) {
	   fd = device2;
	   strcpy(dev_id,dev_id2);
	 }
	 if(dev3on) {
	   fd = device3;
	   strcpy(dev_id,dev_id3);
	 }
       }
       s=0;
       switch(dev_id[8]) {
	  case '0':
		      for(i=0; i<20; i++) {
			 if(subson0[i]!=257) {
			    ++s;
			    j=subson0[i];
			 }
		      }
		      if(s==1) sub=j;
		      else {
			printf("Enter subchannel to read in hex.\n");
			scanf("%d",&sub);
		      }
		      break;
	  case '1':
		      for(i=0; i<20; i++) {
			 if(subson1[i]!=257) {
			    ++s;
			    j=subson1[i];
			 }
		      }
		      if(s==1) sub=j;
		      else {
			printf("Enter subchannel to read in hex.\n");
			scanf("%d",&sub);
		      }
		      break;
	  case '2':
		      for(i=0; i<20; i++) {
			 if(subson2[i]!=257) {
			    ++s;
			    j=subson2[i];
			 }
		      }
		      if(s==1) sub=j;
		      else {
			printf("Enter subchannel to read in hex.\n");
			scanf("%d",&sub);
		      }
		      break;
	  case '3':
		      for(i=0; i<20; i++) {
			 if(subson3[i]!=257) {
			    ++s;
			    j=subson3[i];
			 }
		      }
		      if(s==1) sub=j;
		      else {
			printf("Enter subchannel to read in hex.\n");
			scanf("%d",&sub);
		      }
		      break;
	  default: printf("%s is Bad choice for device\n",dev_id);
		   return(1);
       }
       /* now is the one picked started */
       s=0;
       switch(dev_id[8]) {
	  case '0':
		      for(i=0; i<20; i++) {
			 if(subson0[i]==sub) {
			    s=1;
			 }
		      }
		      if(!s) {
			 printf("subchannel %02X is not started.\n",sub);
			 return(1);
		      }
		      break;
	  case '1':
		      for(i=0; i<20; i++) {
			 if(subson1[i]==sub) {
			    s=1;
			 }
		      }
		      if(!s) {
			 printf("subchannel %02X is not started.\n",sub);
			 return(1);
		      }
		      break;
	  case '2':
		      for(i=0; i<20; i++) {
			 if(subson2[i]==sub) {
			    s=1;
			 }
		      }
		      if(!s) {
			 printf("subchannel %02X is not started.\n",sub);
			 return(1);
		      }
		      break;
	  case '3':
		      for(i=0; i<20; i++) {
			 if(subson3[i]==sub) {
			    s=1;
			 }
		      }
		      if(!s) {
			 printf("subchannel %02X is not started.\n",sub);
			 return(1);
		      }
		      break;
	  default: printf("%s is Bad choice for device\n",dev_id);
		   return(1);
       }


       bytes = 24000;
       if ((pat_ptr = (char *) malloc(bytes)) == NULL) {
	  printf("malloc failed to get space for %d bytes of read data\n");
	  return(1);
       }

       bytesr = readx(fd,pat_ptr,bytes,&re);

       if (bytesr == -1) {
	   printf("Read failed is device %s open?\n",
		 dev_id);
	   printf("Is subchannel 0x%02X started?\n",
		 sub);
	   printf("Did the channel write any data\n\n");
	  printf("Read status = 0x%04x\n", re.cio_ext.status);
	  return(1);
       }

       printf("\n   %d bytes read from subchannel %02X on device %s\n",bytesr,sub,dev_id);
       return(0);
}
decode_status (status)
	struct status_block     *status;
{
	switch (status->code) {
	    case CIO_LOST_STATUS:       printf("CIO_LOST_STATUS: ");
					break;
	    case CIO_NULL_BLK:          printf("CIO_NULL_BLK: ");
					break;
	    case CIO_START_DONE:        printf("CIO_START_DONE: ");
					break;
	    case CIO_HALT_DONE:         printf("CIO_HALT_DONE: ");
					break;
	    case CIO_TX_DONE:           printf("CIO_TX_DONE: ");
					break;
	    case CIO_ASYNC_STATUS:      printf("CIO_ASYNC_STATUS: ");
					break;
	    case CIO_CONNECT_DONE:      printf("CIO_CONNECT_DONE: ");
					break;
	    case CIO_DISCONN_DONE:      printf("CIO_DISCONN_DONE: ");
					break;
	    default:                    printf("%04X, UNDEFINED: ",status->code);
					break;
	}

	if (status->code != CIO_NULL_BLK) {
	    printf("op0=");
	    switch (status->option[0]) {
		case CIO_OK:            printf("CIO_OK: ");
					break;
		case CIO_BAD_MICROCODE: printf("CIO_BAD_MICROCODE: ");
					break;
		case CIO_BUF_OVFLW:     printf("CIO_BUF_OVFLW: ");
					break;
		case CIO_HARD_FAIL:     printf("CIO_HARD_FAIL: ");
					break;
		case CIO_LOST_DATA:     printf("CIO_LOST_DATA: ");
					break;
		case CIO_NOMBUF:        printf("CIO_NOMBUF: ");
					break;
		case CIO_NOT_STARTED:   printf("CIO_NOT_STARTED: ");
					break;
		case CIO_TIMEOUT:       printf("CIO_TIMEOUT: ");
					break;
		case CIO_NET_RCVRY_ENTER: printf("CIO_NET_RCVRY_ENTER: ");
					break;
		case CIO_NET_RCVRY_EXIT: printf("CIO_NET_RCVRY_EXIT: ");
					break;
		case CIO_NET_RCVRY_MODE: printf("CIO_NET_RCVRY_MODE: ");
					break;
		case CIO_INV_CMD:       printf("CIO_INV_CMD: ");
					break;
		case CIO_BAD_RANGE:     printf("CIO_BAD_RANGE: ");
					break;
		case CIO_NETID_INV:     printf("CIO_NETID_INV: ");
					break;
		case CIO_NETID_DUP:     printf("CIO_NETID_DUP: ");
					break;
		case CIO_NETID_FULL:    printf("CIO_NETID_FULL: ");
					break;
		case CIO_TX_FULL:       printf("CIO_TX_FULL: ");
					break;
		default:                printf("%08X UNDEFINED: ",status->option[0]);
					break;
	    }
	    printf("Op1=%08X: ", status->option[1]);
	    printf("Op2=%08X: ", status->option[2]);
	    printf("Op3=%08X: \n", status->option[3]);
	}
	return(0);
}
clawloop ()
{
	cat_write_ext_t         we;
	cat_read_ext_t          re;
	cio_stat_blk_t          stat;
	int                     bytesw,bytes,bytesr,k,q_clean,got_31,count;
	int                     sub,fd,att,i,j,s,rc,loops,sys_reset=0,fdp;
	char                    *pat_ptr,*inbuf;
	char                    dev_id[20];
	char                    *malloc();
	fd_set                  rd, wr, ex;
	struct timeval          timeout;
	int                     maxbytes;

       opencount=0;
       if(dev0on) ++opencount;
       if(dev1on) ++opencount;
       if(dev2on) ++opencount;
       if(dev3on) ++opencount;
       if(opencount>1) {
	  /* allow choice of which open device to write */
	  while(1) {
	     do_over=0;
	     printf("\n\nSelect a device for the write.\n\n");
	     printf("0)      Write to /dev/cat0\n");
	     printf("1)      Write to /dev/cat1\n");
	     printf("2)      Write to /dev/cat2\n");
	     printf("3)      Write to /dev/cat3\n");
	     printf("4)      quit!\n");
	     scanf("%0d",&event);
	     switch(event) {
	       case 0:  fd = device0;
			strcpy(dev_id,dev_id0);
			if(!dev0on) {
			   printf("%s is not open\n",dev_id);
			   continue;
			}
			break;
	       case 1:  fd = device1;
			strcpy(dev_id,dev_id1);
			if(!dev1on) {
			   printf("%s is not open\n",dev_id);
			   continue;
			}
			break;
	       case 2:  fd = device2;
			strcpy(dev_id,dev_id2);
			if(!dev2on) {
			   printf("%s is not open\n",dev_id);
			   continue;
			}
			break;
	       case 3:  fd = device3;
			strcpy(dev_id,dev_id3);
			if(!dev3on) {
			   printf("%s is not open\n",dev_id);
			   continue;
			}
			break;
	       case 4:  return(0);
	       default:
		   printf("Bad choice try again.\n");
		   do_over = 1;
		   break;
	     }
	     if(!do_over) break;
	  }
       }
       else {
	 if(dev0on) {
	   fd = device0;
	   strcpy(dev_id,dev_id0);
	 }
	 if(dev1on) {
	   fd = device1;
	   strcpy(dev_id,dev_id1);
	 }
	 if(dev2on) {
	   fd = device2;
	   strcpy(dev_id,dev_id2);
	 }
	 if(dev3on) {
	   fd = device3;
	   strcpy(dev_id,dev_id3);
	 }
       }
       s=0;
       switch(dev_id[8]) {
	  case '0':
		      for(i=0; i<20; i++) {
			 if(subson0[i]!=257) {
			    ++s;
			    j=subson0[i];
			    if(claw) {
			       printf("Write claw channel %02X\n",j);
			       break;
			    }
			 }
		      }
		      if(s==1) sub=j;
		      else {
			printf("Enter subchannel to write in hex.\n");
			scanf("%d",&sub);
		      }
		      break;
	  case '1':
		      for(i=0; i<20; i++) {
			 if(subson1[i]!=257) {
			    ++s;
			    j=subson1[i];
			    if(claw) {
			       printf("Write claw channel %02X\n",j);
			       break;
			    }
			 }
		      }
		      if(s==1) sub=j;
		      else {
			printf("Enter subchannel to write in hex.\n");
			scanf("%d",&sub);
		      }
		      break;
	  case '2':
		      for(i=0; i<20; i++) {
			 if(subson2[i]!=257) {
			    ++s;
			    j=subson2[i];
			    if(claw) {
			       printf("Write claw channel %02X\n",j);
			       break;
			    }
			 }
		      }
		      if(s==1) sub=j;
		      else {
			printf("Enter subchannel to write in hex.\n");
			scanf("%d",&sub);
		      }
		      break;
	  case '3':
		      for(i=0; i<20; i++) {
			 if(subson3[i]!=257) {
			    ++s;
			    j=subson3[i];
			    if(claw) {
			       printf("Write claw channel %02X\n",j);
			       break;
			    }
			 }
		      }
		      if(s==1) sub=j;
		      else {
			printf("Enter subchannel to write in hex.\n");
			scanf("%d",&sub);
		      }
		      break;
	  default: printf("%s is Bad choice for device\n",dev_id);
		   return(1);
       }
       /* now is the one picked started */
       s=0;
       switch(dev_id[8]) {
	  case '0':
		      for(i=0; i<20; i++) {
			 if(subson0[i]==sub) {
			    s=1;
			 }
		      }
		      if(!s) {
			 printf("subchannel %02X is not started.\n",sub);
			 return(1);
		      }
		      break;
	  case '1':
		      for(i=0; i<20; i++) {
			 if(subson1[i]==sub) {
			    s=1;
			 }
		      }
		      if(!s) {
			 printf("subchannel %02X is not started.\n",sub);
			 return(1);
		      }
		      break;
	  case '2':
		      for(i=0; i<20; i++) {
			 if(subson2[i]==sub) {
			    s=1;
			 }
		      }
		      if(!s) {
			 printf("subchannel %02X is not started.\n",sub);
			 return(1);
		      }
		      break;
	  case '3':
		      for(i=0; i<20; i++) {
			 if(subson3[i]==sub) {
			    s=1;
			 }
		      }
		      if(!s) {
			 printf("subchannel %02X is not started.\n",sub);
			 return(1);
		      }
		      break;
	  default: printf("%s is Bad choice for device\n",dev_id);
		   return(1);
       }

       printf("Enter max number of bytes to write in decimal.\n");
       scanf("%d",&maxbytes);

       if ((pat_ptr = (char *) malloc(maxbytes)) == NULL) {
	  printf("malloc failed to get space for bytes of write data\n");
	  return(1);
       }
       event=0;
       printf("Enter 0 for sequencial data or 1 for random.\n");
       scanf("%d",&event);
       if(event) {
	    if ((fdp = open("datafile", O_RDONLY)) < 0) {
	       printf("Could not open pattern file\n");
	       exit(1);
	    }
	    if (read(fdp, pat_ptr, maxbytes) == -1) {
		printf("Read pattern failed\n");
		exit(1);
	    }
	    close(fdp);
       }
       else {
	  for(i=0; i<maxbytes; i++) {
	     pat_ptr[i] = i;
	  }
       }
       if ((inbuf = (char *) malloc(maxbytes)) == NULL) {
	  printf("malloc failed to get space for bytes of read data\n");
	  return(1);
       }
       printf("Enter a seed for this run.\n");
       scanf("%d",&seed);
       state = malloc(number);
       initstate(seed,state,number);
       setstate(state);

       printf("Enter the number of loops, 0=infinite.\n");
       scanf("%d",&loops);
       if(loops==0) forever=1;
       else forever = 0;
       att=0;
       printf("\n   Running to subchannel %02X on device %s\n",sub,dev_id);
       time(&ltime);
       printf("\n   Start time is %s\n",ctime(&ltime));

  k=0;
  while((k<loops)||(forever==1)) {
       we.cio_ext.flag = 0;
       if(claw) we.cio_ext.write_id = link_op;
       else we.cio_ext.write_id = 0XFF;
       we.cio_ext.netid = sub;
       if(att!=1) {
	      we.attn_int = 0;           /* no attention w/write */
	      we.use_ccw = 0;           /*  no use ccw           */
	      we.ccw = 0x00;            /*  set ccw to 0x00 */
       }
       else {
	      we.attn_int = 1;           /*  attention w/write */
	      we.use_ccw = 1;           /*  use ccw           */
	      we.ccw = 0x03;            /*  set ccw to 0x03, att bsy fix */
       }
       bytes = random();
       while(1) {
	 if(bytes > maxbytes) {
	   bytes = bytes/3;
	   if(bytes <= 0) {
	     bytes = bytes + (maxbytes/2);
	     break;
	  }
	 }
	 else break;
       }
  /*     printf("bytes = %d\n",bytes);  */
       last_bytes = cur_bytes;
       cur_bytes = bytes;
       bytesw = writex(fd,pat_ptr,bytes,&we);

       if (bytesw == -1) {
	   printf("Write failed is device %s open?\n",
		 dev_id);
	   printf("Is subchannel 0x%02X started?\n",
		 sub);
	   printf("Write status = 0x%04x\n", we.cio_ext.status);
	   return(1);
       }
  /*
       printf("\n   %d bytes written to subchannel %02X on device %s\n",bytesw,sub,dev_id);
  */
       /* now read the data back */
       while(1) {
	  FD_ZERO(&rd);
	  FD_ZERO(&wr);
	  FD_ZERO(&ex);
	  FD_SET(fd, &ex) ;   /* wait for excp */
	  FD_SET(fd, &rd) ;   /* wait to read  */
	  timeout.tv_sec = 10;
	  timeout.tv_usec = 0;
	  rc = select(FD_SETSIZE, &rd, &wr, &ex, &timeout);
	  if (rc == -1) {
	      printf("MAIN Read select failed:\n");
	      return(1);
	  }
	  if (rc == 0) {
	      printf("Waiting for data :....time out %d sec\n",timeout.tv_sec);
	       return(1);
	  }
	  if (FD_ISSET(fd,&ex)) {
	      q_clean=0;
	      while(1) {          /* loop till null status received */
		if (ioctl(fd, CIO_GET_STAT, &stat)) {
		     printf("CIO_GET_STAT failed :\n");
		    return(1);
		}

		switch(stat.code) {
		   case CIO_NULL_BLK:    q_clean=1; break;
		   case CIO_LOST_STATUS:
			printf("CIO_LOST_STATUS...\n");
			break;
		   case CIO_ASYNC_STATUS:
			printf("CIO_ASYNC_STATUS...\n");
			break;
		   case CIO_TX_DONE:
			printf("CIO_TX_DONE...\n");
			break;
		   default:
			printf("Unknown value in stat.code...%08X\n",
			      stat.code);
		     break;
		}
		if(q_clean==1) break;

		switch(stat.option[0]) {
		  case CIO_OK:
		     printf("!!!! CIO_OK !!!!!\n");
		     break;
		  case CIO_BAD_MICROCODE:
			printf("!!!! UCODE IS DEAD...\n");
			return(1);
		     break;
		  case CIO_BUF_OVFLW:
			printf("Buffer overflow...\n");
		     break;
		  case CIO_HARD_FAIL:
		     printf("Hard fail..opt1=%02X, opt2=%02X..\n",
			stat.option[1],stat.option[2]);
		     return(1);
		  case CIO_LOST_DATA:
			printf("CIO_LOST_DATA REPORTED..\n");
		     return(1);
		     break;
		  case CIO_NOMBUF:
			printf("CIO_NOMBUF...\n");
		     break;
		  case CIO_NOT_STARTED:
			printf("CIO_NOT_STARTED...\n");
		     return(1);
		     break;
		  case CIO_TIMEOUT:
			printf("CIO_TIMEOUT REPORTED..\n");
		     return(1);
		     break;
		  case CIO_INV_CMD:
			printf("Invalid command ..\n");
		     return(1);
		     break;
		  case CIO_NETID_INV:
			printf("Invalid netid ..\n");
		     return(1);
		     break;
		  case 0x00000091:
			printf("PCA CARD IS OFF LINE. !!!!!!!!\n");
		     break;
		  case 0x00000090:
			printf("PCA CARD IS ON LINE. !!!!!!!!\n");
		     break;
		  case 0x00000031:
		     got_31 = 1;
			printf("GOT 31 note:\n");
		     break;
		  case 0x00000082:
		     printf("82 note-opt1=%02X, opt2=%02X, opt3=%02X..\n",
			stat.option[1],stat.option[2],stat.option[3]);
		     break;
		  case 0x00006316:
			printf("System reset received from channel.\n");
			sys_reset=1;
		     got_31 = 0;
		     break;
		  case 0x0000630B:
			if(got_31) {
			  printf("Selective reset received from "
				 "channel...\n");
			}
			else {
			  printf("Halt I/O received from channel...\n");
			}
		     got_31 = 0;
		     break;
		  default:
			printf("Unknown value in option[0] %08X\n",
			      stat.option[0]);
		     break;
		}      /* end switch on opt[0] */
	      }  /* end of loop on get status to clean status q */
	  }
	  if (FD_ISSET(fd,&rd)) break;
       }           /* end while(1) loop */

       /* do read */
       bytesr = readx(fd,inbuf,bytes,&re);
  /*
       printf("\n   %d bytes written to subchannel %02X on device %s\n",bytesr,sub,dev_id);
  */
       if (bytesr == -1) {
	  printf("Read failed: status = 0x%04X. \n",re.cio_ext.status);
	  return(1);
       }
       if (memcmp(pat_ptr, inbuf,bytes) != 0) {
	 printf("data error \n");
	 /* find out where */
	 count = 0;
	 for(i=0; i<bytes; i++) {
	    if(pat_ptr[i]!=inbuf[i]) {
	       printf("adr %08X   expect.....%02X   got.....%02X\n",i,pat_ptr[i],inbuf[i]);
	       ++count;
	       if(count > 10) break;
	     }
	 }
	 return(1);
       }
       k++;
       donek++;
       if(sys_reset) {
	  sys_reset=0;
	  return(0);
       }
  }  /* end of while loops */
  printf("RUN loops DONE...ok\n");
  return(0);
}

wait_excp(int fd)
{
	cio_stat_blk_t          stat;
	int                     q_clean,got_31;
	int                     i,j,rc;
	fd_set                  rd, wr, ex;
	struct timeval          timeout;
	while(1) {
	  FD_ZERO(&rd);
	  FD_ZERO(&wr);
	  FD_ZERO(&ex);
	  FD_SET(fd, &ex) ;   /* wait for excp */
	  timeout.tv_sec = 0;
	  timeout.tv_usec = 0;
	  rc = select(FD_SETSIZE, &rd, &wr, &ex, &timeout);
	  if (rc == -1) {
	      printf("MAIN Read select failed:\n");
	      return(1);
	  }
	  if (rc == 0) {
	      printf("MAIN Read select timeout:\n");
	      return(1);
	  }
	  if (FD_ISSET(fd,&ex)) {
	      q_clean=0;
	      while(1) {          /* loop till null status received */
		if (ioctl(fd, CIO_GET_STAT, &stat)) {
		     printf("CIO_GET_STAT failed :\n");
		    return(1);
		}

		switch(stat.code) {
		   case CIO_NULL_BLK:    q_clean=1; return(0);
		   case CIO_LOST_STATUS:
			printf("CIO_LOST_STATUS...\n");
			break;
		   case CIO_ASYNC_STATUS:
			printf("CIO_ASYNC_STATUS...\n");
			break;
		   case CIO_TX_DONE:
			printf("CIO_TX_DONE...\n");
			break;
		   default:
			printf("Unknown value in stat.code...%08X\n",
			      stat.code);
		     break;
		}
		if(q_clean==1) break;

		switch(stat.option[0]) {
		  case CIO_OK:
		     printf("!!!! CIO_OK !!!!!\n");
		     break;
		  case CIO_BAD_MICROCODE:
			printf("!!!! UCODE IS DEAD...\n");
			return(1);
		     break;
		  case CIO_BUF_OVFLW:
			printf("Buffer overflow...\n");
		     break;
		  case CIO_HARD_FAIL:
		     printf("Hard fail..opt1=%02X, opt2=%02X..\n",
			stat.option[1],stat.option[2]);
		     return(1);
		  case CIO_LOST_DATA:
			printf("CIO_LOST_DATA REPORTED..\n");
		     return(1);
		     break;
		  case CIO_NOMBUF:
			printf("CIO_NOMBUF...\n");
		     break;
		  case CIO_NOT_STARTED:
			printf("CIO_NOT_STARTED...\n");
		     return(1);
		     break;
		  case CIO_TIMEOUT:
			printf("CIO_TIMEOUT REPORTED..\n");
		     return(1);
		     break;
		  case CIO_INV_CMD:
			printf("Invalid command ..\n");
		     return(1);
		     break;
		  case CIO_NETID_INV:
			printf("Invalid netid ..\n");
		     return(1);
		     break;
		  case 0x00000091:
			printf("PCA CARD IS OFF LINE. !!!!!!!!\n");
		     break;
		  case 0x00000090:
			printf("PCA CARD IS ON LINE. !!!!!!!!\n");
		     break;
		  case 0x00000031:
		     got_31 = 1;
			printf("GOT 31 note:\n");
		     break;
		  case 0x00000082:
		     printf("82 note-opt1=%02X, opt2=%02X, opt3=%02X..\n",
			stat.option[1],stat.option[2],stat.option[3]);
		     break;
		  case 0x00006316:
			printf("System reset received from channel.\n");
		     got_31 = 0;
		     break;
		  case 0x0000630B:
			if(got_31) {
			  printf("Selective reset received from "
				 "channel...\n");
			}
			else {
			  printf("Halt I/O received from channel...\n");
			}
		     got_31 = 0;
		     break;
		  default:
			printf("Unknown value in option[0] %08X\n",
			      stat.option[0]);
		     break;
		}      /* end switch on opt[0] */
	      }  /* end of loop on get status to clean status q */
	  }
       }           /* end while(1) loop */
       return(0);
}
