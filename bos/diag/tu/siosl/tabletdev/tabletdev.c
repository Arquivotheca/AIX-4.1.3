static char sccsid[] = "@(#)62  1.5  src/bos/diag/tu/siosl/tabletdev/tabletdev.c, tu_siosl, bos411, 9428A410j 3/21/94 12:53:41";
/*
 * COMPONENT_NAME: tu_siosl 
 *
 * FUNCTIONS: exectu, tu10, tu20, tu30, tu40, tu60, tu70, tuc0,
 *            SendRxTbData, SendTbData, chk_asl_stat, init_menu
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991,1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <string.h>
#include <fcntl.h> 
#include <errno.h> 
#include <ctype.h> 
#include <time.h>
#include <sys/sem.h>
#include <sys/devinfo.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/mode.h>
#include <sys/mdio.h>
#include <diag/atu.h>

#ifdef DIAGNOSTICS
#include "diag/diago.h"
#include "diag/diag_exit.h"
#include "dtablet_msg.h"
#include "tab_def.h"
#include "tab_str.h"
#endif

#include "salioaddr.h"
#include "misc.h"


#ifdef DIAGNOSTICS
/* Declare global variables */
extern long menu_nmbr;
int slctn;
unsigned short ml_1, ml_3;
extern int dsply_tst_msg();
extern int dsply_tst_lst();
extern int dsply_tst_hdr();
#endif

/***************************************************************************
* NOTE: This function is called by Hardware exerciser (HTX),Manufacturing 
*       application and Diagnostic application to invoke a test unit (TU) 
*                                                                         
*       The 'fd' parameter is for the /dev/hft device driver.
*                                                                         
***************************************************************************/

int exectu(fd,tucb_ptr)
   long     fd ;
   struct tucb_t *tucb_ptr;
   {
     short unsigned int i,j;       /* Loop Index */
     int rc = SUCCESS;            /* return code */
     static int firsttime = 1;

    /* The first time through, we will enable tablet adapter and set up
       line parameters. */

     if (firsttime)
     {
	rc = setup_tablet(fd);	
        if (rc != SUCCESS) {
           return(rc);
        }

        firsttime = 0;
     }

     /* Set loop to 1 if preset to 0 */
     if (tucb_ptr->loop == 0)
        tucb_ptr->loop = 1;

     for (i=0; i<tucb_ptr->loop && rc == SUCCESS; i++)
     {
       switch(tucb_ptr->tu)
        {  case   0x10: rc = tu10(fd,tucb_ptr);
                        break; 
           case   0x20: rc = tu20(fd,tucb_ptr);
                        break;
           case   0x30: rc = tu30(fd,tucb_ptr);
                        break;
           case   0x40: rc = tu40(fd,tucb_ptr);
                        break;
           case   0x60: rc = tu60(fd,tucb_ptr);
                        break;
           case   0x70: rc = tu70(fd,tucb_ptr);
                        break;
           case   0xc0: rc = tuc0(fd,tucb_ptr);
                        break; 
           default : rc = WRONG_TU_NUMBER;
        }  /* end case */

     }  /* i for loop */
   return(rc);   /* indicate rc value */
 }  /* End function */

int tu10(fd, tucb_ptr)
   long     fd ;
   struct tucb_t *tucb_ptr;
{
   unsigned char status,data,cdata;	
   unsigned char datasav, tabdata[6];
   int i, rc = SUCCESS;

  /* Disable tablet */
   data = 0x09;
   if ((rc = SendTbData(fd, data)) != SUCCESS) {    
     PRINT("Unsuccessful at sending data to tablet\n"); 
     rc = 0x10;
     return(rc);
   }
   usleep(500 * 1000);

#ifdef DIAGNOSTICS
 /* Initialize menu screen for this TU */
 init_menu(tucb_ptr->tu);
 dsply_tst_hdr(menu_nmbr);
#endif

  PRINT("Send read status/coord. comm.\n"); 

  /* Send 'read current status/coordinate' command */
   data = 0x0b;
   if ((rc = SendRxTbData(fd, data, tabdata)) != SUCCESS) {    
     PRINT("Unsuccessful at sending data to tablet\n"); 

     if (rc == -2)
	rc = 0x11;
     else
        rc = 0x10;

     return(rc);
   }

   datasav = tabdata[0];    /* Save current tablet info */

   PRINT("Reset tablet\n"); 

  /* Reset tablet */
   data = 0x01;

   if ((rc = SendTbData(fd, data)) != SUCCESS) {    
     PRINT("Unsuccessful at sending data to tablet\n"); 
     rc = 0x10;
     return(rc);
   }
   usleep(500 * 1000);


   /* Check for proper reset state - Send 'read configuration command */ 
      
   PRINT("After tablet reset, sending read config. command\n"); 

   data = 0x06;
   if ((rc = SendRxTbData(fd, data, tabdata)) != SUCCESS) {    
     PRINT("Unsuccessful at sending data to tablet\n"); 

     if (rc == -2)
        rc = 0x11;
     else
        rc = 0x10;

     return(rc);
   }

   PRINT("tabdata[0] = %2x\n",tabdata[0]); 

   if (tabdata[0] == 0xEA) {
     PRINT("Device error occurred during BAT completion test\n"); 
     rc = 0x13;
     return(rc); 
   }

   PRINT("Is it the tablet or cursorpad?\n"); 
 
 /* The device is the tablet */
   if (datasav & 0x20) {     /* Model 22 */
     if (tabdata[0] != 0x14) {
       PRINT("tabdata[0] is %2x, datasav is %2x\n",tabdata[0],datasav);
       rc = 0x14;
       return(rc);
     }
   }   

 /* The device is the cursorpad */
    else {
     if (tabdata[0] != 0x15) {    /* Model 21 */
       PRINT("tabdata[0] is %2x\n",tabdata[0]); 
       rc = 0x14;
       return(rc);
     }
   }

 /* Send 'read current status/coordinate' command */
   data = 0x0b;
   if ((rc = SendRxTbData(fd, data, tabdata)) != SUCCESS) {    
     PRINT("Unsuccessful at sending data to tablet\n"); 

     if (rc == -2)
        rc = 0x11;
     else
        rc = 0x10;

     return(rc);
   }

   PRINT("Make sure default settings are correct\n"); 

  /* Make sure default settings for reset are correct */
   if (tabdata[0] & 0x04) {
     PRINT("default settings, tabdata[0] : %2x\n",tabdata[0]); 
     rc = 0x15;
     return(rc);
   } 
    
  /* Restore original conversion setting, if necessary */
   if (datasav & 0x04) {   /* Restore to original metric setting */
    /* Send first byte of 'set conversion' command */
     data = 0x83;
     if ((rc = SendTbData(fd, data)) != SUCCESS) {    
       PRINT("Unsuccessful at sending data to tablet\n"); 
       rc = 0x10;
       return(rc);
     }
    /* Send second byte of 'set conversion' command */  
     data = 0x01;
     if ((rc = SendTbData(fd, data)) != SUCCESS) {    
       PRINT("Unsuccessful at sending data to tablet\n"); 
       rc = 0x10;
       return(rc);
     }
    usleep(10 * 1000);

    /* Check if setting is now metric - Send 'read current status/coord.' */
    /*  command  */
  
    data = 0x0b;
    if ((rc = SendRxTbData(fd, data, tabdata)) != SUCCESS) {    
      PRINT("Unsuccessful at sending data to tablet\n"); 

      if (rc == -2)
         rc = 0x11;
      else
         rc = 0x10;

      return(rc);
    }

    if ((tabdata[0] & 0x04) != 0x04) 
      rc = 0x15;
   }  /* Change back to original metric setting */
      
  /* Enable tablet */
   data = 0x08;
   SendTbData(fd, data);
   usleep(500 * 1000);

return(rc);
}

int tu20(fd, tucb_ptr)
   long     fd ;
   struct tucb_t *tucb_ptr;
{
   unsigned char data, datasav, tabdata[6];	
   int status, rc = SUCCESS;
 
   int i, numpattern = 4;
   static unsigned char datapattern[4] = { 0x55, 0xaa, 0xff, 0x00 };

 /* Disable tablet */
   data = 0x09;
   if ((rc = SendTbData(fd, data)) != SUCCESS) {    
     PRINT("Unsuccessful at sending data to tablet\n"); 
     rc = 0x20;
     return(rc);
   }
   usleep(500 * 1000);

#ifdef DIAGNOSTICS
 /* Initialize menu screen for this TU */
 init_menu(tucb_ptr->tu);
 dsply_tst_hdr(menu_nmbr);
#endif

  /* Send 'read current status/coordinate' command */
   data = 0x0b;
   if ((rc = SendRxTbData(fd, data, tabdata)) != SUCCESS) {
     PRINT("Unsuccessful at sending data to tablet\n"); 

     if (rc == -2)
        rc = 0x21;
     else
        rc = 0x20;

     return(rc);
   }

   datasav = tabdata[0];    /* Save current tablet info */

   PRINT("Set wrap mode\n");

  /* Send 'set wrap mode' command */
   data = 0x0E;
   if ((rc = SendTbData(fd, data)) != SUCCESS) {    
     PRINT("Unsuccessful at sending data to tablet\n"); 
     rc = 0x20;
     return(rc);
   }
  usleep(500 * 1000);
   
 if (rc == SUCCESS) {
   for (i=0; i<numpattern; i++) 
   {
    /* Send and receive data pattern 'datapattern[i]'  */
 
     if (rc == SUCCESS) {
      if ((rc = SendRxTbData(fd,datapattern[i], tabdata)) != SUCCESS) {
        PRINT("Unsuccessful at sending data to tablet\n"); 

        if (rc == -2)
           rc = 0x21;
        else
           rc = 0x20;

      }
     }

     PRINT("datapattern tabdata[0] is : %2x\n",tabdata[0]);

     if (rc == SUCCESS) {
      if (tabdata[0] != datapattern[i])	/* datapattern[i] is test pattern  */
      {
        PRINT("Wrapped data is not matched, data : %2X, pattern : %2x\n",tabdata[0],datapattern[i]);  
        rc = 0x23;
      }
     }
   }
 }
 
  PRINT("Reset wrap mode\n");

   /* Send 'reset wrap mode' command */
  if (rc == SUCCESS) {
   data = 0x0F; 
   if ((rc = SendTbData(fd, data)) != SUCCESS) {    
    PRINT("Unsuccessful at sending data to tablet\n"); 
    rc = 0x20;
   }
  }
  usleep(10 * 1000);

  /* Send one byte of data */
  if (rc == SUCCESS) {
   data = 0x06; 
   if ((rc = SendRxTbData(fd, data, tabdata)) != SUCCESS) {    
     PRINT("Unsuccessful at sending data to tablet\n"); 

     if (rc == -2)
        rc = 0x21;
     else
        rc = 0x20;

   }
  }
  
   PRINT("tabdata[0] is %2x\n",tabdata[0]); 

  if (rc == SUCCESS) {
   if (tabdata[0] == 0x06) {
     PRINT("Tablet still in wrap mode, tablet reset wrap mode error, data = %2X\n", tabdata[0]); 
     rc = 0x24;
   }
  } 
   
 /* Reset tablet */
   data = 0x01;
   SendTbData(fd, data);
   usleep(500 * 1000);

  /* Restore original conversion setting, if necessary */
   if (datasav & 0x04) {   /* Restore to original metric setting */
    /* Send first byte of 'set conversion' command */
     data = 0x83;
     SendTbData(fd, data);
    /* Send second byte of 'set conversion' command */
     data = 0x01;
     SendTbData(fd, data);
     usleep(10 * 1000);
   }  /* Change back to original metric setting */

 /* Enable tablet */
   data = 0x08;
   SendTbData(fd, data);
   usleep(500 * 1000);

  return (rc);
}

int tu30(fd, tucb_ptr)
   long     fd ;
   struct tucb_t *tucb_ptr;
{
   unsigned char data, datasav, tabdata[6];	
   int status, rc = SUCCESS;

 /* Disable tablet */
   data = 0x09;
   if ((rc = SendTbData(fd, data)) != SUCCESS) {    
     PRINT("Unsuccessful at sending data to tablet\n"); 
     rc = 0x30;
     return(rc);
   }
   usleep(500 * 1000);
   
#ifdef DIAGNOSTICS
 /* Initialize menu screen for this TU */
 init_menu(tucb_ptr->tu);
 dsply_tst_hdr(menu_nmbr);
#endif

 /* Save current settings */
   PRINT("Read current status/coord.\n");

  /* Send 'read current status/coord.' command */
   data = 0x0b;
   if ((rc = SendRxTbData(fd, data, tabdata)) != SUCCESS) {    
     PRINT("Unsuccessful at sending data to tablet\n"); 

     if (rc == -2)
        rc = 0x31;
     else
        rc = 0x30;

     return(rc);
   }
   datasav = tabdata[0];

  /* Set conversion mode of tablet to 'Metric' */

   PRINT("set conversion command\n");

  /* Send first byte of 'set conversion' command */
  if (rc == SUCCESS) {
    data = 0x83;
    if ((rc = SendTbData(fd, data)) != SUCCESS) {    
       PRINT("Unsuccessful at sending data to tablet\n"); 
       rc = 0x30;
    }
  }

  /* Send second byte of 'set conversion' command */  
  if (rc == SUCCESS) {
   data = 0x01;
   if ((rc = SendTbData(fd, data)) != SUCCESS) {    
     PRINT("Unsuccessful at sending data to tablet\n"); 
     rc = 0x30;
   }
  usleep(10 * 1000);
  }

   PRINT("set tablet resolution command\n"); 

  /* Set tablet resolution to 200 LPCM by sending 'set resolution' command */
 if (rc == SUCCESS) {
  data = 0x84;
  if ((rc = SendTbData(fd, data)) != SUCCESS) {    
    PRINT("Unsuccessful at sending data to tablet\n"); 
    rc = 0x30;
  }
 }

 if (rc == SUCCESS) {
  data = 0x65;
  if ((rc = SendTbData(fd, data)) != SUCCESS) {    
    PRINT("Unsuccessful at sending data to tablet\n"); 
    rc = 0x30;
  }
 }

 if (rc == SUCCESS) {
  data = 0x85;
  if ((rc = SendTbData(fd, data)) != SUCCESS) {    
    PRINT("Unsuccessful at sending data to tablet\n"); 
    rc = 0x30;
  }
 }

 if (rc == SUCCESS) {
  data = 0x99;
  if ((rc = SendTbData(fd, data)) != SUCCESS) {    
    PRINT("Unsuccessful at sending data to tablet\n"); 
    rc = 0x30;
  }
  usleep(500 * 1000);
 }
 
  /* Check if setting is now metric with 200 LPCM - Send 'read current */
  /* status/coord.' command */

   PRINT("check if setting is metric\n"); 

 if (rc == SUCCESS) {
  data = 0x0b;
  if ((rc = SendRxTbData(fd, data, tabdata)) != SUCCESS) {    
    PRINT("Unsuccessful at sending data to tablet\n"); 

     if (rc == -2)
        rc = 0x31;
     else
        rc = 0x30;

  }
 }
  
 if (rc == SUCCESS) {
  if ((tabdata[0] & 0x04) != 0x04) 
    rc = 0x33;
 }
  
 if (rc == SUCCESS) {
  if ((tabdata[0] & 0x02) != 0x02) 
    rc = 0x34;
 }

   PRINT("return original settings\n"); 

 /* Return original settings if necessary */
  if ((datasav & 0x04) == 0 ) {
    data = 0x83;
    SendTbData(fd, data);
    data = 0x00;
    SendTbData(fd, data);
  }

  /* Set tablet resolution to 500 LPI by sending 'set resolution' command */
  data = 0x84;
  SendTbData(fd, data);
  data = 0x64;
  SendTbData(fd, data);
  data = 0x85;
  SendTbData(fd, data);
  data = 0x00;
  SendTbData(fd, data);

 /* Enable tablet */
   data = 0x08;
   SendTbData(fd, data);
   usleep(500 * 1000);

 return(rc);
}

int tu40(fd, tucb_ptr)
   long     fd ;
   struct tucb_t *tucb_ptr;
{
   unsigned char data, datasav, tabdata[6];	
   int i, status, rc = SUCCESS;
   char sdata[10];

#ifdef DIAGNOSTICS
   long asl_err;
#endif

 /* Disable tablet */
   data = 0x09;
   if ((rc = SendTbData(fd, data)) != SUCCESS) {    
     PRINT("Unsuccessful at sending data to tablet\n"); 
     rc = 0x40;
     return(rc);
   }
   usleep(500 * 1000);

#ifdef DIAGNOSTICS
 /* Initialize menu screen for this TU */
 init_menu(tucb_ptr->tu);

 /* Display message asking user to watch for tablet indicator */
 asl_err = dsply_tst_msg(menu_nmbr,TM_10);
 if ((rc = chk_asl_stat(asl_err)) != SUCCESS)  
   return(rc); 
 
 menu_nmbr += 0x000001;   /* Increment menu number */
#endif

#ifdef SUZTESTING
 printf ("\n\n          Watch for the tablet green indicator status light.  Hit a key when ready : ");
  scanf("%c",&sdata);
#endif

  /* Send 'read current status/coord.' command */
  for(i = 0; i < 30; i++) {
   data = 0x0b;
   if ((rc = SendTbData(fd, data)) != SUCCESS) {    
     PRINT("Unsuccessful at sending data to tablet\n"); 
     rc = 0x40;
     return(rc);
   }
  }

#ifdef DIAGNOSTICS
 /* Ask user if tablet indicator was lit or flickering */
 asl_err = dsply_tst_lst(menu_nmbr,TM_11);
 if ((rc = chk_asl_stat(asl_err)) != SUCCESS)  
   return(rc); 
#endif

#ifdef SUZTESTING
 printf ("\n\n		Was the tablet green tablet indicator status light on or flickering? (y/n) : "); 
  scanf("%s",&sdata); 

 if (strcmp(sdata,"n") == 0) {
#endif /* testing mode */

#ifdef DIAGNOSTICS
 if (slctn == ATU_NO) {
#endif
    rc = 0x43;
    return(rc);
 } 

 /* Enable tablet */
   data = 0x08;
   SendTbData(fd, data);
   usleep(500 * 1000);

 return(rc);
}


int tu60(fd, tucb_ptr)
   long     fd ;
   struct tucb_t *tucb_ptr;
{
   unsigned char data, datasav;	
   int status, rc = SUCCESS;
   char sdata[10];

#ifdef DIAGNOSTICS
   long asl_err;
#endif

 /* Disable tablet */
  data= 0x09; 
  if ((rc = SendTbData(fd, data)) != SUCCESS) {    
    PRINT("Unsuccessful at sending data to tablet\n"); 
    rc = 0x60;
    return(rc);
  }
  usleep(500 * 1000);
      
#ifdef DIAGNOSTICS
 /* Initialize menu screen for this TU */
 init_menu(tucb_ptr->tu);
 

 /* Display message asking to place input device in test area */
 asl_err = dsply_tst_lst(menu_nmbr,TM_22);
 if ((rc = chk_asl_stat(asl_err)) != SUCCESS)  
   return(rc); 
 
 menu_nmbr += 0x000001;   /* Increment menu number */

 if (slctn == ATU_NO) {
     rc = 0x64;
     return(rc);
 }
#endif

#ifdef SUZTESTING
 /* Ask user to place input device in test area */
   printf("\n\n		Place input device on the tablet test area.\n");
   printf("		Is the stylus/cursor indicator on? (y/n) : ");
   scanf("%s",&sdata);

   if (strcmp(sdata,"n") == 0) {
     rc = 0x64;
     return(rc);
   }

 /* Ask user to place input device in tablet active area */
   printf("\n\n		Place input device on the tablet active area.\n");
   printf("		Is the stylus/cursor indicator off? (y/n) : ");
   scanf("%s",&sdata);
#endif /* testing mode */

#ifdef DIAGNOSTICS
 asl_err = dsply_tst_lst(menu_nmbr,TM_23);
 if ((rc = chk_asl_stat(asl_err)) != SUCCESS)  
   return(rc); 
#endif

#ifdef SUZTESTING
 if (strcmp(sdata,"n") == 0) {
#endif /* testing mode */

#ifdef DIAGNOSTICS
 if (slctn == ATU_NO) {
#endif
     rc = 0x63;
     return(rc);
 }

 /* Enable tablet */
  data= 0x08;
  SendTbData(fd, data);
  usleep(500 * 1000);

 return(rc);
}

int tu70(fd, tucb_ptr)
   long     fd ;
   struct tucb_t *tucb_ptr;
{
   unsigned char data, datasav;	
   int status, rc = SUCCESS;
   char sdata[10];

#ifdef DIAGNOSTICS
   long asl_err;
#endif

 /* Disable tablet */
  data= 0x09; 
  if ((rc = SendTbData(fd, data)) != SUCCESS) {    
    PRINT("Unsuccessful at sending data to tablet\n"); 
    rc = 0x70;
    return(rc);
  }
  usleep(500 * 1000);

#ifdef DIAGNOSTICS
 /* Initialize menu screen for this TU */
 init_menu(tucb_ptr->tu);
#endif

#ifdef SUZTESTING
   printf("\n           Is the stylus/cursor indicator still off? (y/n) : ");
   scanf("%s",&sdata);
#endif  /* testing mode */

#ifdef DIAGNOSTICS
 /* Ask user if the light in the center square is still off */
 asl_err = dsply_tst_lst(menu_nmbr,TM_23A);
 if ((rc = chk_asl_stat(asl_err)) != SUCCESS)  
   return(rc); 
 
 menu_nmbr += 0x000001;   /* Increment menu number */
#endif

#ifdef SUZTESTING
 if (strcmp(sdata,"n") == 0) {
#endif  /* testing mode */

#ifdef DIAGNOSTICS
 if (slctn == ATU_NO) {
#endif
     rc = 0x73;
     return(rc);
  }

#ifdef SUZTESTING
  /* Ask user to place input device at active area - to test switches */
   printf("\n\n         Place input device on tablet active area.\n");
   printf("             When the input device button is pressed, is the\n");
   printf("             stylus/cursor indicator on? (y/n) : ");
   scanf("%s",&sdata);
#endif  /* testing mode */


#ifdef DIAGNOSTICS
 /* Prompt user to press all of the buttons on input device */
 asl_err = dsply_tst_lst(menu_nmbr,TM_24A);
 if ((rc = chk_asl_stat(asl_err)) != SUCCESS)  
   return(rc); 
#endif

#ifdef SUZTESTING
 if (strcmp(sdata,"n") == 0) {
#endif  /* testing mode */

#ifdef DIAGNOSTICS
 if (slctn == ATU_NO) {
#endif
     rc = 0x74;
     return(rc);
 }

 /* Enable tablet */
  data = 0x08;
  SendTbData(fd, data);
  usleep(500 * 1000);

  return(rc);
}

int tuc0(fd, tucb_ptr)
long fd;
struct tucb_t *tucb_ptr;
{
   uchar data, datasav, tabdata[6];
   int status, rc = SUCCESS;
   char sdata[10];

#ifdef DIAGNOSTICS
   long asl_err;
#endif

  /* Disable tablet */
  data = 0x09;
  if ((rc = SendTbData(fd, data)) != SUCCESS) {
    PRINT("Unsuccessful at sending data to tablet\n"); 
    rc = 0xc0;
    return(rc);
  }
  usleep(500 * 1000);

#ifdef DIAGNOSTICS
 /* Initialize menu screen for this TU */
 init_menu(tucb_ptr->tu);
 menu_nmbr &= 0xFFF00F;
 menu_nmbr += 0x000120;
#endif


  /* Send 'read current status/coordinate' command */
   data = 0x0b;
   if ((rc = SendRxTbData(fd, data, tabdata)) != SUCCESS) {
     PRINT("Unsuccessful at sending data to tablet\n"); 

     if (rc == -2)
        rc = 0xc1;
     else
        rc = 0xc0;

     return(rc);
   }

   datasav = tabdata[0];    /* Save current tablet info */

 /* Set resolution to 10 lines/inch */
  data = 0x84;
  if ((rc = SendTbData(fd, data)) != SUCCESS) {
    PRINT("Unsuccessful at sending data to tablet\n"); 
    rc = 0xc0;
    return(rc);
  }
  data = 0x02;
  if ((rc = SendTbData(fd, data)) != SUCCESS) {
    PRINT("Unsuccessful at sending data to tablet\n"); 
    rc = 0xc0;
    return(rc);
  }
  data = 0x85;
  if ((rc = SendTbData(fd, data)) != SUCCESS) {
    PRINT("Unsuccessful at sending data to tablet\n"); 
    rc = 0xc0;
    return(rc);
  }
  data = 0x00;
  if ((rc = SendTbData(fd, data)) != SUCCESS) {
    PRINT("Unsuccessful at sending data to tablet\n"); 
    rc = 0xc0;
    return(rc);
  }

 /* Set sampling speed */
  data = 0x8a;
  if ((rc = SendTbData(fd, data)) != SUCCESS) {
    PRINT("Unsuccessful at sending data to tablet\n"); 
    rc = 0xc0;
    return(rc);
  }
  data = 0x32;     /* Sample 50 coord. pairs/sec. */
  if ((rc = SendTbData(fd, data)) != SUCCESS) {
    PRINT("Unsuccessful at sending data to tablet\n"); 
    rc = 0xc0;
    return(rc);
  }
      
 /* Set tablet to incremental data mode */
  data = 0x8d;
  if ((rc = SendTbData(fd, data)) != SUCCESS) {
    PRINT("Unsuccessful at sending data to tablet\n"); 
    rc = 0xc0;
    return(rc);
  }
  data = 0x00;
  if ((rc = SendTbData(fd, data)) != SUCCESS) {
    PRINT("Unsuccessful at sending data to tablet\n"); 
    rc = 0xc0;
    return(rc);
  }

  /* Enable tablet */
  data = 0x08;
  if ((rc = SendTbData(fd, data)) != SUCCESS) {
    PRINT("Unsuccessful at sending data to tablet\n"); 
    rc = 0xc0;
    return(rc);
  }
  usleep(500 * 1000);

#ifdef SUZTESTING
  /* Prompt user to test tablet enable status */
   printf("\n\n		Move the input device and press the device switch,\n");
   printf("		making sure the tablet indicator comes on.\n");
   printf("		This will confirm tablet enable status.\n");
   printf("    		Please indicate with an answer of 'y' if this test is complete\n");
   printf("		and successful.  Respond with an answer of 'n' if this test is\n");
   printf("		complete but unsuccessful.\n");
   printf("		Enter answer (y/n) : ");
   scanf("%s",&sdata);
#endif /* testing mode */

  /* Prompt user to move input device and press switch, making sure tablet
   indicator comes on */

#ifdef DIAGNOSTICS
  asl_err = dsply_tst_lst(menu_nmbr,TM_37);
  if ((rc = chk_asl_stat_c0(asl_err,fd,datasav)) != SUCCESS)
    return(rc);
 
  menu_nmbr += 0x000001;   /* Increment menu number */
#endif

#ifdef SUZTESTING
  if (strcmp(sdata,"n") == 0) {
#endif /* testing mode */

#ifdef DIAGNOSTICS
  if (slctn == ATU_NO) {
#endif
     rc = 0xc4;
     return(rc);
  }

  /* Disable tablet */

    data = 0x09;
    if ((rc = SendTbData(fd, data)) != SUCCESS) {
      PRINT("Unsuccessful at sending data to tablet\n"); 
      rc = 0xc0;
      return(rc);
    }
    usleep(500 * 1000);

#ifdef SUZTESTING
   /* Prompt user to test tablet disable status */
    printf("\n\n		Move input device and press the device switch.  Watch tablet indicator\n");
    printf("		status to confirm that the tablet is disabled.\n");
    printf("		The tablet indicator should be off.\n");
    printf("    		Please indicate with an answer of 'y' if this test is complete\n");
    printf("		and successful.  Respond with an answer of 'n' if this test is\n");
    printf("		complete but unsuccessful.\n");
    printf("		Enter answer (y/n) : ");
    scanf("%s",&sdata);
#endif /* testing mode */

 /* Prompt user to move input device and press switch, making sure indicators
    function correctly */

#ifdef DIAGNOSTICS
   if (rc == SUCCESS) {
    asl_err = dsply_tst_lst(menu_nmbr,TM_38);
    rc = chk_asl_stat_c0(asl_err,fd,datasav);
   }
#endif

#ifdef SUZTESTING
   if (strcmp(sdata,"n") == 0) {
#endif /* testing mode */

#ifdef DIAGNOSTICS
   if (rc == SUCCESS) {
    if (slctn == ATU_NO) 
#endif
      rc = 0xc3;
   }

 /* Reset tablet */
  data = 0x01;
  SendTbData(fd, data);
  usleep(500 * 1000);

  /* Restore original conversion setting, if necessary */
   if (datasav & 0x04) {   /* Restore to original metric setting */
    /* Send first byte of 'set conversion' command */
     data = 0x83;
     SendTbData(fd, data);
    /* Send second byte of 'set conversion' command */
     data = 0x01;
     SendTbData(fd, data);
     usleep(10 * 1000);
   } /* Restore original Metric setting */ 

 /* Enable tablet */
  data = 0x08;
  SendTbData(fd, data);

 return(rc);
}

/* This function sends data, provided by 'value' parameter, to the tablet,
   and it puts received data from tablet into the tabdata array. */

int SendRxTbData(fd, value, tabdata)
int fd;
unsigned char value, *tabdata;
{
  uchar data;
  int i, tlen, count = 1, rc = SUCCESS;

 /* Clear tabdata array */
  for (i = 0; i < 6; i++)
     tabdata[i] = 0;
 
  tlen = 1 ;

  rc = SendTbData(fd, value);
 
  if (rc == SUCCESS) {
 
    if ((rc = rd_byte(fd, &data, TABLET_LINE_CTRL_REG)) != SUCCESS) {
      PRINT("Unsuccessful read of one byte from TABLET_LINE_CTRL_REG\n");
      return(rc);
    }

    /* Set DLAB of Line control reg to 0 */

    data = data & 0x7f;
    if ((rc = wr_byte(fd, &data, TABLET_LINE_CTRL_REG)) != SUCCESS) {
      PRINT("Unsuccessful write of one byte to TABLET_LINE_CTRL_REG\n");
      return(rc);
    }

    /* if asking for tablet status report, read all six bytes */
    if ( value == 0x0b )
       tlen = 6 ;

    while( count > 0 && polltab(fd) != SUCCESS )
       count-- ;
  
    for ( i = 0; i < tlen ; i++ )
    { 
       if ( polltab(fd) == SUCCESS) {    /* Read from Recv. buffer */
          if ((rc = rd_byte(fd, &data, TABLET_RX_BUFF_REG)) != SUCCESS) {
            PRINT("Unsuccessful read of one byte from TABLET_RX_BUFF_REG\n");
            return(rc);
          }

          tabdata[i] = data;
       }
       else 
       {
          return(-2);  /* Return special value if poll timeout occurs */
       }

    }

    PRINT("\n\n");
    for (i = 0; i < 6; i++)
     PRINT("tabdata[%d] = %2x,",i,tabdata[i]); 
    PRINT("\n\n");

 } /* if rc = SUCCESS */

return(rc);
}


/* This function is called by SendRxTbData.  It returns SUCCESS if data
   is in the tablet Rx buffer */ 

polltab(fd)
int fd;
{
   int rc = SUCCESS;
   uchar data;
   long  count  = 2000;

                                       /* BEGIN polling           */
   while (count)
   {                                   /* read line status                   */

      if ((rc = rd_byte(fd, &data, TABLET_LINE_ST_REG)) != SUCCESS) {
        PRINT("Unsuccessful read of one byte from TABLET_LINE_ST_REG\n");
        return(rc);
      }

                                       /* if there is any data ready         */
      if  (data & 0x01 )
      {
                                       /* if no errors on the line           */
         if (!(data & 0x1e)) /* check for overrun, parity, break intrpt. and
			        framing errors */
            break ;
      }
      usleep(2000);   /* Sleep 2 ms */
      count--;                         /* decrement count                    */
   }
                                       /* returning from polltab           */
   return((int) (count ? SUCCESS : -1 ) );
                                       /* END polltab()                    */
} /* polltab() */


/* This function just sends data, which is stored in the 'value' parameter,
   to the tablet. */

int SendTbData(fd, value)
int fd;    /* File descriptor for machine DD */
unsigned char value;
{
  unsigned int count;
  unsigned char Status, cvalue;
  int rc = SUCCESS;

  for(count = 0; count < REPEAT_COUNT; count++)
  {

    clear_transmit(fd);  /* Clear transmitter */

  /* Read from Line status reg. */
    rc = rd_byte(fd, &Status, TABLET_LINE_ST_REG);
    if (rc == SUCCESS)
    {
      if (Status & 0x20)  /* Is Transmitter empty? */
      {
        if ((rc = rd_byte(fd, &cvalue, TABLET_LINE_CTRL_REG)) != SUCCESS) {
         PRINT("Unsuccessful read of one byte from TABLET_LINE_CTRL_REG\n");
         return(rc);
        }
      /* Set DLAB of Line control reg to 0 */

        cvalue = cvalue & 0x7f;
        if ((rc = wr_byte(fd, &cvalue, TABLET_LINE_CTRL_REG)) != SUCCESS) {
         PRINT("Unsuccessful write of one byte to TABLET_LINE_CTRL_REG\n");
         return(rc);
        }
        rc = wr_byte(fd, &value, TABLET_TX_BUFF_REG);  /* Write to r/w reg */
      }
    }
    if ((rc == SUCCESS) && ((Status & 0x20) == 0))
      usleep(10000);
    else
      break;
  }
  if((Status & 0x20) == 0)
  {
    PRINT("Could not send data : TX always busy - Not able to send %x\n",value);
    rc = -1;
  }
return(rc);
}

/* This function makes sure that tablet adapter is ready for more data
   to be sent */

clear_transmit(fd)
int fd;    /* File descriptor for machine DD */
{
  int i;
  uchar data;

  /* Read from Line status reg. */
  i = 0;
  data = 0;

 /* Is transmitter not empty or a character in Recv. buffer reg.? */

  rd_byte(fd, &data, TABLET_LINE_ST_REG);

  while ( ( (!(data & 0x20)) || (data & 0x01) ) && (i < 200) ) {  /* If transmitter not empty or data is ready */
   /* Read from Recv. buffer */
    rd_byte(fd, &data, TABLET_RX_BUFF_REG);
    usleep(10 * 1000);

    rd_byte(fd, &data, TABLET_LINE_ST_REG);
    usleep(10 * 1000);
    i++;
  } 
 
}

/* This function makes sure that tablet adapter is properly enabled and
   line parameters are set up */

int setup_tablet(fd)
int fd;    /* File descriptor for machine DD */
{
   unsigned char data,cdata, baud = 52;
   int rc = SUCCESS;

   /* Grab the semaphore before accessing POS2 */
   rc = set_sem(1);

   if (rc != SUCCESS) {
     PRINT("Error grabbing semaphore, exit'ing test\n"); 
     return(rc);
   }

  /* Read the current setting of SIO control reg (POS2) */
   rd_pos(fd, &cdata, SIO_CONTROL_REG);

   /* Put tablet adapter in reset */
   cdata = cdata | SIO_REG_RESERVED;
   wr_pos(fd, &cdata, SIO_CONTROL_REG);
   usleep(500 * 1000);

   /* Take tablet adapter out of reset */
   cdata = cdata & ~SIO_REG_RESERVED;
   wr_pos(fd, &cdata, SIO_CONTROL_REG);

   usleep(500*1000);

   /* Now release the semaphore after accessing POS2 */
   rc = rel_sem();

   if (rc != SUCCESS) {
      PRINT("Error releasing semaphore\n");
      return(rc);
   }

  /* read from Line control reg */
   rd_byte(fd, &data, TABLET_LINE_CTRL_REG);

  /* Set DLAB to 1 so baud rate can be set */
  data = data | 0x80;
  wr_byte(fd, &data, TABLET_LINE_CTRL_REG);
  usleep(10 * 1000);

  /* Set baud rate to 9600 */
  wr_byte(fd, &baud, TABLET_TX_BUFF_REG);
  usleep(10 * 1000);

  /* Set Line parameters - DLAB bit to 0, 8 bits/char, 1 stop bit, odd parity */
  data = 0x0b;
  wr_byte(fd, &data, TABLET_LINE_CTRL_REG);
  usleep(10 * 1000);

  /* read from Line status reg to clear it */
  rd_byte(fd, &data, TABLET_LINE_ST_REG);

  /* put the adapter in fifo mode       */

  data = 0x07;
  wr_byte(fd, &data, TABLET_FIFO_CTRL_REG);

  /* Clear fifo's, one more time to be sure */
  data = 0x07;
  wr_byte(fd, &data, TABLET_FIFO_CTRL_REG);

 /* write all 1's to scratch reg */
  data = 0xff;
  wr_byte(fd, &data, TABLET_SCRATCH_REG);

  return(rc);

}

#define MAX_SEM_RETRIES 5
#define POS_SEMKEY      0x3141592

/***************************************************************************
 FUNCTION NAME: set_sem(wait_time)

   DESCRIPTION: This function access the POS register 2 via the IPC
                semaphore which should be used by all programs desiring
                the access. The user passes in a "wait_time" to specify
                whether or not to "wait_forever" for the semaphore or
                retry MAX_SEM_RETRIES with a wait_interval of "wait_time"

   NOTES:       This function is used basically to overcome the concurrent
                access of the POS register 2 by the Keyboard/Tablet,
                Diskette, Serial Port 1 and 2, Parallel Port and Mouse
                devices.

***************************************************************************/

int set_sem(wait_time)
int wait_time;
{
        int             semid;
        long            long_time;
        struct sembuf   sembuf_s;
        int             retry_count = (MAX_SEM_RETRIES -1);
        int             rc;
        static          first_time = 1;

        /* Obtain semaphore ID and create it if it does not already
         *  exist */

        semid = semget( (key_t) POS_SEMKEY, 1, IPC_CREAT | IPC_EXCL | S_IRUSR |
                        S_IWUSR | S_IRGRP | S_IWGRP);

        if (semid < 0)
        {
          /* If the error from semget() reveals that we failed for any
           * reason other than the fact that the semaphore already
           * existed, indicate error and return */

           if (errno != EEXIST)
                return(0x201);

          /* Semaphore already exists. Because it already exists it is
           * possible that it was JUST created by another process so
           * let's sleep here for a few clock cycles to let the other
           * process initialize everything properly */

           if (first_time)
           {
                sleep(4);
                first_time = 0;
           }
        /* That should be enough time, so get the semaphore ID without
         * CREATion flags */

        semid = semget( (key_t) POS_SEMKEY, 1, S_IRUSR | S_IWUSR | S_IRGRP |
                        S_IWGRP);

        /* Make sure that we got a valid semaphore ID, else return error */

        if (semid < 0)
            return(0x201);
      }
        else
      {
        /* Semaphore was nearly created so we need to initialize our
         * semaphore value */

        if (semctl(semid, 0, SETVAL, 1))
        {
                return(0x201);
        }
      }

        /* At this point, we have our semaphore ID and is it was the
         * first instance, it had been created and initialized for
         * use */

        /* Indicate semaphore number */

        sembuf_s.sem_num = 0;

        /* Set op to -1 indicating that we want to grab the semaphore */

        sembuf_s.sem_op = -1;

        /* If a non-negative wait_time was passed in, then indicate that
         * we do not want the process to be blocked if the semaphore is
         * unavailable. Note the SEM_UNDO flag. By including this, the
         * semaphore will get properly released should this process be
         * terminated by a signal or something */

        if (wait_time >= 0)
                sembuf_s.sem_flg = IPC_NOWAIT | SEM_UNDO;
        else
                sembuf_s.sem_flg = SEM_UNDO;

        /* See if we can get the semaphore. If the semaphore is available
         * then it has a value of 1 which will get decremented to a value
         * of 0 since our sem_op = -1. Else the semaphore is not available
         * (thus a value of 0).*/

        while (retry_count > -1)
        {
                rc = semop(semid, &sembuf_s, 1);
                if (rc == 0)
                {
                  /* Got it, so return */

                  return(SUCCESS);
                }
            if (errno == EAGAIN)
                {
        /* Semaphore held by someone else, but we indicated not to wait
         * forever. If user specified wait_time of zero, then just
         * return unsuccessfully without retries */

                        if (wait_time == 0)
                        return(0x201);

        /* Sleep for the time specified by the user and then retry */

                        sleep(wait_time);
                        retry_count --;
                }
                else
                        return(0x201);
        }
         return(0x201);
}

/***************************************************************************
* FUNCTION NAME: rel_sem()

   DESCRIPTION: This function releases the semaphore indicating
                completion of access to POS register 2 by that particular
                device.
   NOTES:

***************************************************************************/

int rel_sem()
{
        int             semid;
        struct sembuf   sembuf_s;
        int             pid;
        int             rc;
        int             sempid;

        /* Obtain semaphore ID and create it if it does not already
         *  exist */

        semid = semget( (key_t) POS_SEMKEY, 1, S_IRUSR |  S_IWUSR | S_IRGRP |
                         S_IWGRP);

        /* Make sure that we got a valid semaphore ID, else return
         * error */

        if (semid < 0)
           return(0x201);

        /* Now, we want to make sure that we do not attempt to release
         * the semaphore if we don't already have it. This ensures that
         * the semaphore value remains < 1, therefore binary */

        /* First, get the current process ID */

        pid = getpid();

        /* Next, get the process ID of the process which currently has
         * the semaphore */

        sempid = semctl(semid, 0, GETPID, 0);
        if (sempid < 0)
           return(0x201);

        /* If the current process ID does not equal the semaphore's
         * process ID, then we are not holding it so return an error */

        if (pid != sempid)
           return(0x201);

        /* Release the semaphore by handing it a positive value which
         * get added to the semaphore value indicating that it is now
         * available. Note the SEM_UNDO flag. By including this, the
         * semaphore will get properly handled should this process be
         * terminated by a signal or something. */

        sembuf_s.sem_num = 0;
        sembuf_s.sem_op = 1;
        sembuf_s.sem_flg = SEM_UNDO;
        rc = semop(semid, &sembuf_s, 1);

        return(rc);
}


/* This function uses the machine device driver to read one byte from
   the specified address, returning the information to pdata */

int rd_byte(fd, pdata, addr)
int fd;
unsigned char *pdata;
unsigned int addr;
{
  MACH_DD_IO iob;
  int rc;
       
  iob.md_data = pdata;
  iob.md_incr = MV_BYTE;
  iob.md_size = sizeof(*pdata);
  iob.md_addr = addr;

  rc = ioctl(fd, MIOBUSGET, &iob);
/*
  PRINT("Read byte = %2X\n",*pdata);
  PRINT("Read addr = %4X\n",addr);
*/

  return (rc);
}
    
/* This function uses the machine device driver to write one byte from
   pdata to the specified address */

int wr_byte(fd, pdata, addr)
int fd;
unsigned char *pdata;
unsigned int addr;
{
  MACH_DD_IO iob;
  int rc;
       
  iob.md_data = pdata;
  iob.md_incr = MV_BYTE;
  iob.md_size = sizeof(*pdata);
  iob.md_addr = addr;

  rc = ioctl(fd, MIOBUSPUT, &iob);
/*
  PRINT("Write byte = %2X\n",*pdata);
  PRINT("Write addr = %4X\n",addr);
*/

  return (rc);
}

/* This function uses the machine device driver to read one byte from
   the specified pos register, returning the information to pdata */

int rd_pos(fd, pdata, addr)
int fd;
unsigned char *pdata;
unsigned int addr;
{
  MACH_DD_IO iob;
  int rc;

  iob.md_data = pdata;
  iob.md_incr = MV_BYTE;
  iob.md_size = sizeof(*pdata);
  iob.md_addr = addr;

  rc = ioctl(fd, MIOCCGET, &iob);
/*  printf("Read byte = %2X\n",*pdata);
  printf("Read addr = %4X\n",addr);
*/
  return (rc);
}

/* This function uses the machine device driver to write one byte from
   pdata to the specified pos register address */

int wr_pos(fd, pdata, addr)
int fd;
unsigned char *pdata;
unsigned int addr;
{
  MACH_DD_IO iob;
  int rc;

  iob.md_data = pdata;
  iob.md_incr = MV_BYTE;
  iob.md_size = sizeof(*pdata);
  iob.md_addr = addr;

  rc = ioctl(fd, MIOCCPUT, &iob);
/*  printf("Write byte = %2X\n",*pdata);
  printf("Write addr = %4X\n",addr);
*/
  return (rc);
}


#ifdef DIAGNOSTICS
/* This function checks the return code from the asl msg routine */
int     chk_asl_stat (asl_return)
long    asl_return;
{
	switch(asl_return)
	{
	case ASL_OK:
	case ASL_ENTER:
	        asl_return = 0;
	        break;
	case ASL_ERR_SCREEN_SIZE:
	case ASL_FAIL:
	case ASL_ERR_NO_TERM:
	case ASL_ERR_NO_SUCH_TERM:
	case ASL_ERR_INITSCR:
	        DA_SETRC_ERROR(DA_ERROR_OTHER);
	        asl_return = 0xFA;
	        break;
	case ASL_EXIT:
	        DA_SETRC_USER(DA_USER_EXIT);
	        exit_da();
	        break;
	case ASL_CANCEL:
	        DA_SETRC_USER(DA_USER_QUIT);
	        DA_SETRC_MORE(DA_MORE_NOCONT);
	        exit_da();
	        break;
	}
	return (asl_return);
}

/* This function checks the return code from the asl msg routine */
int     chk_asl_stat_c0 (asl_return,fd,datasav)
long    asl_return, fd;
unsigned char datasav;
{
  unsigned char data;

	switch(asl_return)
	{
	case ASL_OK:
	case ASL_ENTER:
	        asl_return = 0;
	        break;
	case ASL_ERR_SCREEN_SIZE:
	case ASL_FAIL:
	case ASL_ERR_NO_TERM:
	case ASL_ERR_NO_SUCH_TERM:
	case ASL_ERR_INITSCR:
	        DA_SETRC_ERROR(DA_ERROR_OTHER);
	        asl_return = 0xFA;
	        break;
	case ASL_EXIT:
 	/* Reset tablet */
	  data = 0x01;
	  SendTbData(fd, data);
	  usleep(500 * 1000);

 	 /* Restore original conversion setting, if necessary */
	   if (datasav & 0x04) {   /* Restore to original metric setting */
	    /* Send first byte of 'set conversion' command */
	     data = 0x83;
	     SendTbData(fd, data);
	    /* Send second byte of 'set conversion' command */
	     data = 0x01;
	     SendTbData(fd, data);
	     usleep(10 * 1000);
	   } /* Restore original Metric setting */ 

	  /* Enable tablet */
	   data = 0x08;
	   SendTbData(fd, data);

	        DA_SETRC_USER(DA_USER_EXIT);
	        exit_da();
	        break;
	case ASL_CANCEL:
 	/* Reset tablet */
	  data = 0x01;
	  SendTbData(fd, data);
	  usleep(500 * 1000);

 	 /* Restore original conversion setting, if necessary */
	   if (datasav & 0x04) {   /* Restore to original metric setting */
	    /* Send first byte of 'set conversion' command */
	     data = 0x83;
	     SendTbData(fd, data);
	    /* Send second byte of 'set conversion' command */
	     data = 0x01;
	     SendTbData(fd, data);
	     usleep(10 * 1000);
	   } /* Restore original Metric setting */ 

	  /* Enable tablet */
	   data = 0x08;
	   SendTbData(fd, data);

	        DA_SETRC_USER(DA_USER_QUIT);
	        DA_SETRC_MORE(DA_MORE_NOCONT);
	        exit_da();
	        break;
	}
	return (asl_return);
}

/* This function initializes the screen menu number for each TU */
int     init_menu (tu_num)
int     tu_num;
{
	int     tu_menu;

	tu_menu = tu_num;
	tu_menu <<= 4;
	menu_nmbr &= 0xFFF000;
	tu_menu += 1;
	menu_nmbr += tu_menu;
}

/*
 * NAME: tablet_type
 *
 * FUNCTION:  Asks user if they are using 5081 tablet. If not, an additional
 *      screen is displayed directing the user to a hardcopy source of
 *      diagnostics tests. If yes, then tests continue normally.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS: NONE
 */
int tablet_type()
{
        long asl_rc = 0;
        int rtn_c = 0;
        init_menu (0x30);
        menu_nmbr += 0x000001;
        ml_3 = TM_25A;
        asl_rc = dsply_tst_lst (menu_nmbr, ml_3);
        if ((rtn_c = chk_asl_stat (asl_rc)) != TU_SUCCESS)
                return (rtn_c);
        if (slctn == ATU_NO)
        {
                menu_nmbr += 0x000001;
                ml_1 = TM_26A;
                asl_rc = dsply_tst_msg (menu_nmbr, ml_1);
                if ((rtn_c = chk_asl_stat (asl_rc)) != TU_SUCCESS)
                        return (rtn_c);
                rtn_c = 0x36;
                return (rtn_c);
       }
}
#endif
