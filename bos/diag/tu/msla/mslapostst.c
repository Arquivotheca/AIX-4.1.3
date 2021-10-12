static char sccsid[] = "@(#)31  1.3.1.1  src/bos/diag/tu/msla/mslapostst.c, tu_msla, bos411, 9428A410j 5/2/94 11:56:05";
/*
 * COMPONENT_NAME: ( mslapostst ) 
 *
 * FUNCTIONS:  postest, pos2_0to0, pos2_1to1, pos5_4to4, parity_lvl2_chk
 *             parity_lvl3_chk, read_id, pos_rw_test
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

/*****************************************************************************/
/*                                                                           */
/*      MODULE NAME :  postest                                               */
/*                                                                           */
/* DESCRIPTIVE NAME :  MSLA   HTX Hardware Exerciser                         */
/*                                                                           */
/*          PURPOSE : Use the HTX interface to test MSLA  for the following  */
/*                    POSTEST to test the pos registers on the microchannel  */
/*                    Interface Card.                                        */
/*                                                                           */
/*                      The following POS bits are tested                    */
/*                           POS#2 bit 0 for adapter enable/disable          */
/*                           POS#2 bit 1 for byte swap on/off                */
/*                              at this moment only word xfer has any affect */
/*                           POS#5 bit 4 for byte  parity enable/disable     */
/*                              at this moment two routines test the level-2 */
/*                              and level-3 parity logic. All is done thru   */
/*                              calls to the device driver.                  */
/*                                                                           */
/* NOTE :   It has to be decided whether all the tests will be run even in   */
/*          case of an earliar test failure . I prefer to stop as soon as    */
/*          one test fails because POS bits are to some extent related.      */
/*                                                                           */
/*            INPUT : Pointer to the mslatu structure.                       */
/*                                                                           */
/*           OUTPUT : Returns the proper code.                               */ 
/*                                                                           */
/* FUNCTIONS CALLED : read_id, pos_wr_test, preload_set_up, mslaclrm, mkerr, */
/*                    pos2_0to0, pos2_1t1, pos5_4to4, postload_set_up.       */
/*                                                                           */
/*           STATUS :  Release 1 Version 0                                   */
/*                                                                           */
/* COMPILER OPTIONS :                                                        */
/*                                                                           */
/*****************************************************************************/

#include <sys/types.h>
#include "mslaerrdef.h"
#include "mslafdef.h"
#include "mslablof.h"
#include "mslatu.h"
#include "mslaposdat.h"
#include "msla_diag.h"

int
postest (gtu)
struct mslatu *gtu;
{
    int rc, fd;
    ushort_t val ;
    ulong_t bussmem, ioaddr ;


    fd      = gtu->fd  ;
    ioaddr  = gtu->msla_iobase ;
    bussmem = gtu->msla_membase;

    /*
    ***********************************
    *  Reset MSLA, Disable parity,    *
    *  Halt MSLA and Enable card      *
    ***********************************
    */
    pre_load_setup(fd,ioaddr);


    /*
    ***********************************
    * clear the RAM of the MSLA       *
    ***********************************
    */
    mslaclrm(fd,bussmem,MAX_16BIT,ioaddr) ;


    /*
    *******************************
    * Tests card enable/disable   *
    *******************************
    */
    rc = pos2_0to0(fd,bussmem);
    if ( rc != SUCCESS )
    {
        mkerr(POSTEST,POS2_0TO0,rc,&rc);
        post_load_setup(fd);
  	return(rc);
    }

    /*
    *********************************
    *  Reset MSLA, Disable parity,  *
    *  Halt MSLA and Enable card    *
    *********************************
    */
    pre_load_setup(fd,ioaddr);


    /*
    *********************************
    * clear the RAM of the MSLA     *
    *********************************
    */
    mslaclrm(fd,bussmem,MAX_16BIT,ioaddr) ;


    /* Discontinued BYTE SWAP TEST 10-05-89 */
    /* Tests the swap bit */
    /* rc = pos2_1to1(fd,slot,bussmem);     */     
    /* if ( rc != SUCCESS )
    {
           mkerr(POSTEST,POS2_1TO1,rc,&rc);
           post_load_setup(fd);
       	   return(rc);
    }
    ***************/


    /*
    ************************
    * Tests the parity     *
    ************************
    */
    /* check level-2 parity logic */
    rc = parity_lvl2_chk(fd,bussmem,ioaddr);
    if ( rc != SUCCESS )
    {
        post_load_setup(fd);
        mkerr(POSTEST,LEVEL2,rc,&rc);
	return(rc);
    }

    mslaclrm(fd,bussmem,M32K_BYTES,ioaddr) ;

    /* check level-3 parity logic */
    rc = parity_lvl3_chk(fd,bussmem,ioaddr);
    if(rc != SUCCESS) 
    {
        post_load_setup(fd);
        mkerr(POSTEST,LEVEL3,rc,&rc);
	return(rc);
    }

    return(rc);
}

/*****************************************************************************/
/*                                                                           */
/*      MODULE NAME :  int pos2_0to0                                         */ 
/*                                                                           */
/*          PURPOSE :  This tests the abitlity to disable/enable memory      */
/*                     via POS.                                              */
/*                                                                           */
/*            INPUT :  fd,slot,bussmem                                       */
/*                                                                           */
/*           OUTPUT :  Returns the proper code.                              */
/*                                                                           */
/* FUNCTIONS CALLED : pos_read, pos_write;                                   */
/*                                                                           */
/*  DATE of Creation: 05/27/1989                                             */
/*                                                                           */
/*           STATUS :  Release 1 Version 0                                   */
/*                                                                           */
/*  STATUS : TESTED                                                          */
/*                                                                           */
/*****************************************************************************/

int
pos2_0to0(fd,bussmem)
int fd ;
unsigned long bussmem ;
{

    /*
    ** Local variable definitions
    */
    int rc, word_test ,loop;
    unsigned long  word_ndx;
    unsigned short *bus_addr;
    char posdata;

    return(0);  /* This test is no longer valid... the newer machines do not
		  take kindly to accessing disabled memory */


   /* TEST with DISABLE CARD */ 
    rc = ioctl (fd, MSLA_MOD_POS, MP_DIS_CARD);
    if (rc < 0)
    {
	rc = POS_DRIVER_IOCTL;
        return(rc);
    }

    for (word_test = 0,loop = SUCCESS; 
        ((word_test < NO_OF_TESTBYTES) && ( loop == SUCCESS ));
              word_test++) 
    { 
        for (word_ndx=0, ( bus_addr=(unsigned short *)(bussmem | word_ndx));
             word_ndx < 16; word_ndx+=2, bus_addr++)
        {
             *bus_addr = msla_detect[word_test] ;
        }

        for(word_ndx=0, bus_addr=(unsigned short *)(bussmem | word_ndx);
            word_ndx < 16; word_ndx+=2)
        {
            if ( msla_detect[word_test] == *bus_addr )
            {
                bus_addr++;
                continue;
            }
            else
            {
                loop = FAIL ;
                break ;
            }
        }
    }

     /*
     *****************************************
     *  With the card disabled the test of   *
     *  r/w to memory should fail            *
     *****************************************
     */
     if ((word_test >= NO_OF_TESTBYTES) && ( loop == SUCCESS ))
     {
         rc = POS2_BIT0_DISABLE_ERR;
	 return(rc);
     }

     /*
     ***************************************
     *  Since, r/w to card with disable    *
     *  failed we proceed further          *
     ***************************************
     */

     rc = ioctl (fd, MSLA_MOD_POS, MP_ENA_CARD);
     if (rc < 0)
     {
	 rc = POS_DRIVER_IOCTL;
    	 return(rc);
     }
	    
     for (word_test = 0,loop = SUCCESS; 
         ((word_test < NO_OF_TESTBYTES) && ( loop == SUCCESS )); word_test++)
     {
          for(word_ndx=0, bus_addr=(unsigned short *)(bussmem | word_ndx);
                    word_ndx < 16; word_ndx+=2, bus_addr++)
          {
               *bus_addr = msla_detect[word_test] ;
          }

          for(word_ndx=0, bus_addr=(unsigned short *)(bussmem | word_ndx);
               word_ndx < 16; word_ndx+=2)
          {
               if ( msla_detect[word_test] == *bus_addr )
               {
                    bus_addr++;
                    continue;
               }
               else
               {
                    loop = FAIL ;
                    break ;
               }
          }
     }

     if (!((word_test >= NO_OF_TESTBYTES) && ( loop == SUCCESS )))
     {
          rc = POS2_BIT0_ENABLE_ERR ;
	  return(rc);
     }

    rc = ioctl (fd, MSLA_MOD_POS, MP_ENA_CARD);
    if (rc < 0)
    {
	rc = POS_DRIVER_IOCTL;
        return(rc);
    }

    return(rc);
}


/*****************************************************************************/
/*                                                                           */
/*      MODULE NAME :  parity_lvl2_chk                                       */
/*                                                                           */
/*          PURPOSE : This tests the abitlity to check parity logic  via POS */
/*                    This also reads the parity bit in the status port      */
/*                    Needs  "mtpparl2.blo" to be loaded                     */
/*                                                                           */
/*            INPUT :  fd, bussmem, ioaddr.                                  */
/*                                                                           */
/*           OUTPUT :  Returns the proper code.                              */
/*                                                                           */
/* FUNCTIONS CALLED : pos_read, pos_write, bload, mslaclrm                   */
/*                                                                           */
/*  DATE of Creation: 05/27/1989                                             */
/*                                                                           */
/*           STATUS :  Release 1 Version 0                                   */
/*                                                                           */
/*           STATUS : TESTED                                                 */
/*                                                                           */
/*****************************************************************************/

int
parity_lvl2_chk(fd,bussmem,ioaddr)
int fd ;
ulong_t bussmem ;
ulong_t ioaddr ;
{
    int rc, counter ;
    char filename[60];
    char posdata;
    ushort_t *bus_addr, bus_data, k ;
    ushort_t *flag0, *flag1, *flag2, *flag3, *flag4;
    char status_data ,status_mask;

    
    strcpy (filename , MSLAPOS_BLO1 );
    rc = bload (filename,bussmem,fd,7,ioaddr);
    if ( rc != SUCCESS )
    {
        return(rc);
    }


   /* for parity error */
    status_mask  = ~(IO_STR_PAR_MASK | IO_STR_INT_MASK);

   /* both the parity & the interrupt status bits are set to 0 */
    flag0 = ( unsigned short *)(bussmem | FTP_FLAG0_LO_WORD );
    flag1 = ( unsigned short *)(bussmem | FTP_FLAG1_LO_WORD );
    flag2 = ( unsigned short *)(bussmem | FTP_FLAG2_LO_WORD );
    flag3 = ( unsigned short *)(bussmem | FTP_FLAG3_LO_WORD );
    flag4 = ( unsigned short *)(bussmem | FTP_FLAG4_LO_WORD );

   /* disable interrupt */
    disb_int_msla(fd,ioaddr);

   /* enable parity-check */
    rc = ioctl (fd, MSLA_MOD_POS, MP_ENA_PARITY);
    if ( rc < 0)
    {
	rc = POS_DRIVER_IOCTL;
	return(rc);
    }


    if ( *flag0 == 0x0000 )
    {
       /* The micro-code is not running  hence start 68K */
        start_msla(fd,ioaddr) ;
        delay(1000);
    }
    else
    {
        rc = POSTEST_MEM_LOADED_INCORRECTLY;
        return(rc);
    }


               /* in each full pass of u-code even & odd parity is tested*/
               /* FLAG1 determines which parity it is testing */
               /* FLAG4 is hand-shaking mechanism between MSLA & LEPB */
               /* MSLA sets FLAG4 until it is cleared by LEPB u-code waits*/

    if ( *flag0 == 0x0001 )
    {
       /* The micro-code is running */
        rc = SUCCESS ;

	counter = 0;
       /* every succesful pass of ucode increments FLAG0 */
       /*       hence, I prefer to check the ucode more then once */
       /* FLAG1 indicates spurious interrupt to 68k which results in hang*/
        while (( *flag1 != 0xFFFF ) && ( (k = *flag0) <4) && (rc ==SUCCESS))
        {
	    if (counter == 10)
	    {
		rc = TIMING_OUT;
		return(rc);
	    }
	    counter++;

            delay(50);
            if ( *flag4 == 0x0001 )            /* there is a parity int */
            {
                switch ( *flag1) 
		{
                     case 0x0001 :
                           if ( *flag2 == 0xE090 )  /* even parity */
                           {
                                status_data = (char )stat_msla(fd,ioaddr);
                                /* The io-status reg. bit-1 is for parity*/
                                if ( status_data  == status_mask )
                                {
				 /* reset of flag 2 informs 68k to exit loop
				    and perform next parity interrupt test,
				    so  flag 4 should be reset before
				    flag 2 in case we are swapped out
				    between instructions                 */
				     *flag4 = 0x0000 ;  /* u-code to proceed*/
                                     *flag2 = 0x0000 ;  /* handshaking with */
                                     delay(100);
                                }
                                else
                                {
                                     rc = PARITY_BIT_NOT_SET ;
                                }
                           }
                           else
			   {
                                 rc = PAR_ENB_EVEN_PARITY_ERR;
			   }
                           break;

                     case 0x0002 :
                           if ( *flag2 == 0xE091 )  /* odd parity */
                           {
                                status_data = (char )stat_msla(fd,ioaddr) ;
                                /* The io-status reg. bit-1 is for parity */
                                if ( status_data  == status_mask )
                                {
				 /* reset of flag 2 informs 68k to exit loop
				    and perform next parity interrupt test,
				    so  flag 4 should be reset before
				    flag 2 in case we are swapped out
				    between instructions                 */
				     *flag4 = 0x0000 ;  /* u-code to proceed*/
				     *flag2 = 0x0000 ;  /* handshaking with */
				     delay(100);
                                }
                                else
                                {
                                     rc = PARITY_BIT_NOT_SET ;
                                }
                           }
                           else
			   {
                                rc = PAR_ENB_ODD_PARITY_ERR;
			   }
                           break;

                     case 0xFFFF :
                           rc = PAR_ENB_NO_LVL2_PAR;
                           break;

                     default  :
                           rc = PAR_ENB_HW_PAR;
                           break;
                }
            }

            if ( *flag3 == 0x0001)
            {
                rc = PAR_ENB_INV_LVL2_PAR;
                break ;
            }

            if ( *flag1 == 0xFFFF)
            {
                rc = PAR_ENB_NO_LVL2_PAR;
                break ;
            }
        }
    }
    else
    {
        rc = POSTEST_UCODE_NOT_STARTED ;
	return(rc);
    }

    reset_msla(fd,ioaddr) ;

    /*
    ***********************************************
    *  If the above test passes the parity logic  *
    *  is working and POS bit settable. Hence,    *
    *  try to see parity disable works or not     *
    ***********************************************
    */

    /* Clear the MSLA RAM */
    mslaclrm(fd,bussmem,MAX_16BIT,ioaddr) ;

    /* Load the ucode */
    rc = bload(filename,bussmem,fd,7,ioaddr);
    if ( rc < 0 )
    {
        return(rc);
    }

    /* disable parity-check */
    rc = ioctl (fd, MSLA_MOD_POS, MP_DIS_PARITY);
    if (rc < 0)
    {
 	rc = POS_DRIVER_IOCTL;
	return(rc);
    }

    delay(500);

   /* Halt the MSLA */
    halt_msla(fd,ioaddr) ;

   /* Start the MSLA */
    start_msla(fd,ioaddr) ;

    delay(5000);
       
   /* start the u-code after reset which clears the parity also */
    if ( *flag0 == 1 )
    {
        if ( (*flag1 ==0xFFFF ) && (*flag2 == 0xE090 ) )
	{
            rc = SUCCESS ;
	}
        else
	{
            rc = PAR_DISB_FAIL ;
	}
    }
    else
    {
        rc = PAR_DISB_HW_PAR;
    }

    return(rc);
}

/*****************************************************************************/
/*                                                                           */
/*      MODULE NAME :  parity_lvl3_chk                                       */
/*                                                                           */
/*          PURPOSE :  This tests the abitlity to check parity logic via POS */
/*                     Needs  "mtpparl3.blo" to be loaded                    */
/*                                                                           */
/*            INPUT :  fd,slot,bussmem,ioaddr.                               */
/*                                                                           */
/*           OUTPUT :  Returns the proper code.                              */
/*                                                                           */
/* FUNCTIONS CALLED : pos_read, pos_write, bload.                            */
/*                                                                           */
/* DATE of Creation : 05/27/1989                                             */
/*                                                                           */
/*           STATUS :  Release 1 Version 0                                   */
/*                                                                           */
/*           STATUS : TESTED                                                 */
/*                                                                           */
/*****************************************************************************/

int
parity_lvl3_chk(fd,bussmem,ioaddr)
int fd ;
ulong_t bussmem ;
ulong_t ioaddr ;
{
   /*
   ** Local variable definitions
   */
    int rc ,i ;
    char filename[60];
    char posdata;
    ushort_t       *bus_addr, bus_data, k ;
    ushort_t       old_val, new_val ;
    ushort_t       *flag0, *flag1, *flag2, *flag3, *flag4, *flag5;

   /*
   ** Start of code
   */

    strcpy (filename , MSLAPOS_BLO2 ) ;
    rc = bload(filename,bussmem,fd,7,ioaddr);
    if ( rc != SUCCESS )
    {
        return(rc);
    }
    else
    {
        rc = SUCCESS ;
        flag0 = ( unsigned short *)(bussmem | FTP_FLAG0_LO_WORD );
        flag1 = ( unsigned short *)(bussmem | FTP_FLAG1_LO_WORD );
        flag2 = ( unsigned short *)(bussmem | FTP_FLAG2_LO_WORD );
        flag3 = ( unsigned short *)(bussmem | FTP_FLAG3_LO_WORD );
        flag4 = ( unsigned short *)(bussmem | FTP_FLAG4_LO_WORD );
        flag5 = ( unsigned short *)(bussmem | FTP_FLAG5_LO_WORD );

       /* disable interrupt */
        disb_int_msla(fd,ioaddr);

        /* disable parity-check */
	rc = ioctl (fd, MSLA_MOD_POS, MP_DIS_PARITY);
	if (rc < 0)
	{
	    perror("Disable Parity Failed\n");
	    return(rc);
	}
  
       /* Start the MLSA */
        start_msla(fd,ioaddr) ;
        
        for ( new_val = *flag0 , i = 4 ; i ; i-- )
        {
            old_val = new_val ;
            delay(100);
            new_val = *flag0 ;
            if (   old_val  == new_val  )
            {
                rc =  PAR_DIS_LVL3_NOT_STARTED ;
            }
            else
            {
                rc = SUCCESS ;
                i = 1 ;
            }
        }
        if ( rc == SUCCESS )
        {
            if (  (  *flag0  )  && (~(*flag1) ) && 
                  (~(*flag2) ) &&  (~(*flag3) )  )
            {

                  reset_msla(fd,ioaddr) ;

                 /* enable parity-check */
		  rc = ioctl (fd, MSLA_MOD_POS, MP_ENA_PARITY);
		  if (rc < 0)
		  {
		      rc = POS_DRIVER_IOCTL;
	    	      return(rc);
		  }

                  /* enable parity-check flag for micro-code */
                  *flag4 = 0x0001  ;

                 /* Halt the MLSA */
                  halt_msla(fd,ioaddr) ;

                 /* Start the MLSA */
                  start_msla(fd,ioaddr) ;

                  delay(100);
                  if ( *flag0 )
                  {
                      switch ( *flag5 )
                      {
                           case 0xFFFF:
                                       rc = PAR_ENB_LVL3_INV_PAR_INT ;
                                break ;
                           case 0x0001:
                                        rc = SUCCESS ;
                                break ;
                           default :
                                       rc = PAR_ENB_LVL3_NO_PAR_INT ;
                                break ;
                      }
                        
                  }
                  else
                  {
                       rc =  PAR_ENB_LVL3_NOT_STARTED ;
                  }
            }
            else
            {
            }
        }
        else
        {
        }
        if ( rc == SUCCESS )
        {

                 /* Reset the MLSA */
                  reset_msla(fd,ioaddr) ;

                 /* enable parity-check */
		  rc = ioctl (fd, MSLA_MOD_POS, MP_ENA_PARITY);
		  if (rc < 0)
		  {
	    	      return(rc);
		  }
                  *flag4 = 0x0000  ;

                 /* Halt the MLSA */
                  halt_msla(fd,ioaddr) ;

                 /* Start the MLSA */
                  start_msla(fd,ioaddr) ;

                  delay(100);
                  if (*flag0)
                  {
                  }
                  else
                  {
                        rc =  PAR_DIS_LVL3_NOT_STARTED_AGAIN ;
                  }
        }
        else
        {
        }
    }
    return(rc);
}

