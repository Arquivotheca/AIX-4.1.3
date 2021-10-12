static char sccsid[] = "@(#)31  1.2.1.3  src/bos/diag/tu/artic/tu001.c, tu_artic, bos411, 9428A410j 8/19/93 17:52:30";
/* COMPONENT_NAME:  
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * FUNCTIONS: get_POST_err, post, tu001
 *
 */
#include <stdio.h>
#include <artictst.h>
typedef unsigned char byte;
/*
 * NAME: get_POST_err
 *
 * FUNCTION: Calculate and return the POST error value
 *
 * EXECUTION ENVIRONMENT:
 *
 * (NOTES:) This procedure looks up the POST error value and return it to
 *          the calling routine
 *
 * RETURNS: The POST error value
 *
 */
int get_POST_err (fdes, tucb_ptr, low_en)
   int fdes;  /* Filehandle */
   TUTYPE *tucb_ptr;
   unsigned char low_en;   /* Low Error Number byte 0x414 */
   {
        unsigned short in_addr;
        byte high_en;     /* High error number byte 0x415 */
        int rc;

        extern int dh_diag_mem_read();

        in_addr = 0x415;

        /* Read POST error byte, non-zero=error */
        if ((rc=dh_diag_mem_read(fdes, tucb_ptr, 0,in_addr,&high_en)) != 0)
                return(DDINPR2);

        switch(high_en)
           {
                case 1: return(P_CPU_ER);
                case 2: if (low_en == 0x05)
                                return(HRAM); /*Return High RAM Error*/
                        else
                                return(LRAM);   /*Return Low RAM error*/
                case 3: return(P_ROS_CK_ER);
                case 4: return(P_CIO_ER);
                case 5: return(P_SCC_ER);
                case 6: return(P_GA_ER);
                case 7: return(P_PARITY_ER);
                case 8: return(P_DMA_ER);
                default: return(POSTER);
           }
   }

/*
 * NAME: post
 *
 * FUNCTION: Initiate the POST (Power On Self Test) procedure --
 *
 * EXECUTION ENVIRONMENT:
 *
 * (NOTES:) This function will set things up to run the POST (Power On Self
 *          Test) procedures.
 *
 * RETURNS: A zero (0) for successful completition or non-zero for error.
 *
 */
int post (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
        unsigned char post_error_val;       /* address read from card */
        unsigned short in_addr;             /* address to examine in RAM  */
        int rc;                             /* return code                */
        unsigned short shrc;                /* return code                */
        unsigned char value;

        extern int outby();
        extern int outrm();
        extern int dh_diag_mem_read();
        extern int dh_diag_io_write();
        extern int dh_diag_io_read();

        /* Reset CPU page to zero */
        if ((rc=outby(fdes, tucb_ptr,
                adapter_CPUPG,0x00,REG_ERR3)) != 0)
                return(rc);

        /* Clear the POST ram error bit before test */
        if ((rc=outrm(fdes, tucb_ptr, 0x414,0x00,REG_ERR4)) != 0)
                return(rc);

        /* Clear any previous interrupts */
        if ((rc=dh_diag_io_read(fdes, tucb_ptr,
                        (byte) adapter_TREG,&value)) != 0)
                return(DDINPB1);

        /* Reset the card and run the POST procedure */
        if (shrc=icareset(fdes))
                return(DRV_ERR);

        /* Reset CPU page to zero */
        outby(fdes, tucb_ptr, adapter_CPUPG,0x00,REG_ERR6);

        /* Read POST error byte, non-zero=error */
        in_addr = 0x414;
        post_error_val = 0;
        if ((rc=dh_diag_mem_read(fdes, tucb_ptr, 0,
                                in_addr,&post_error_val)) != 0)
                return(DDINPB3);
        else
                if (post_error_val)
                   {
                        return(get_POST_err(fdes, tucb_ptr, post_error_val));
                   }
        return(0);
   }

/*
 * NAME: tu001
 *
 * FUNCTION: Test Unit 001
 *
 * EXECUTION ENVIRONMENT:
 *
 * (NOTES:) This procedure will initiate the POST (Power On Self Test).
 *          Following checks are performed by POST:
 *
 *              - Checksum Test -- Check the checksum, of the Eprom
 *              - Processor Test -- checks timers, DMA, interrupts, etc.
 *              - CIO Test -- Counter/Timer and Parrallel Input/Output
 *              - SCC Test -- Serial Communication Controller
 *              - Gate Array -- STTIC-3
 *              - DRAM Test
 *              - Parity Circuits
 *              - DMA Allocation Register
 *
 *
 * RETURNS: The return code from the POST function call.
 *
 */
int tu001 (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
        return(post(fdes, tucb_ptr));
   }
