static char sccsid[] = "@(#)24	1.1  src/bos/usr/bin/bprt/prt-c-em.c, libbidi, bos411, 9428A410j 8/27/93 09:57:05";
/*
 *   COMPONENT_NAME: LIBBIDI
 *
 *   FUNCTIONS: MemMove
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
/***************************** SOURCE FILE HEADER *****************************/
/*                                                                            */
/*   SOURCE FILE NAME: PRT-C-EM.C <Emulator of some C functions>              */
/*                                                                            */
/*   FUNCTION: This file contains the implementation of some C functions.     */
/*             These functions are used by the Printer's APIs.                */
/*                                                                            */
/******************************************************************************/




/************************** START OF SPECIFICATIONS ***************************/
/*                                                                            */
/* SUBROUTINE NAME:  MemMove                                                  */
/*                                                                            */
/* DESCRIPTIVE NAME: Memory Move.                                             */
/*                                                                            */
/* FUNCTION: Mimics the C memmove function.                                   */
/*                                                                            */
/* ENTRY POINT: MemMove                                                       */
/*                                                                            */
/* INPUT: ( dest, src, count )                                                */
/*        dest     @Other : pointer to destination buffer.                    */
/*        src      @Other : pointer to source buffer.                         */
/*        count    WORD   : length of destination and source buffers.         */
/*                                                                            */
/* EXIT-NORMAL: copies count bytes from the source to the detination taking   */
/*              the overlapping of source and target in consideration.        */
/*                                                                            */
/* EXIT-ERROR:  None                                                          */
/*                                                                            */
/* INTERNAL REFERENCES                                                        */
/*   LOCAL DATA DEFINITIONS:                                                  */
/*        diff : difference between the start of destinaion and source.       */
/*        OverLap : True if the source and target overlap and the source is   */
/*                  before the target.                                        */
/*        i       : a counter used while coping the buffer.                   */
/*                                                                            */
/*   GLOBAL DATA DEFINITIONS: None.                                           */
/*                                                                            */
/*   ROUTINES: None.                                                          */
/*                                                                            */
/* EXTERNAL REFERENCES:                                                       */
/*   ROUTINES: None.                                                          */
/*                                                                            */
/*************************** END OF SPECIFICATIONS ****************************/

void  *MemMove( void *dest, void *src, unsigned count )
/* removed far , before *, before *dest, before *src */
{
   long diff;
   unsigned OverLap, i;

                                       /* Does the source and target have    */
                                       /* the same segments                  */

   if ( (((unsigned long) dest) & 0xffff0000) ==
        (((unsigned long)  src) & 0xffff0000) ) {
                                       /* Yes : then calculate the           */
                                       /* difference between the source and  */
                                       /* target.                            */
      diff = (long) ((char *)dest - (char *)src);
 /* the original cast was (char huge *), this has bee changed for AIX   */  
                                       /* If the source is before the target */
                                       /* and there is a partial overlap     */
      if ( (diff > 0) && ( diff < count))
         OverLap = 1;                  /* then overlap is True.              */
      else
         OverLap = 0;                  /* else overlap is false              */
   }
   else
      OverLap = 0;                     /* No : then there is no overlap      */
                                       /* If no overlap                      */
   if ( !OverLap ) {
                                       /* then copy the source to the dest-  */
                                       /* ination going from low to high     */
                                       /* memory.                            */
      for( i = 0; i < count; i++ )
        /* removed far, after char  (2) */
         ((char *) dest)[i] = ((char *) src)[i];
   }
   else {
      if (count != 0) {                /* else copy the source to the dest-  */
         i = count;                    /* ination going form high to low     */
         do {                          /* memory.                            */
            i--;
        /* removed far, after char  (2) */
            ((char *) dest)[i] = ((char *) src)[i];
         } while ( i > 0 ); /* enddo */
      }
   }
   return dest;                        /* Return a pointer to the dest-      */
                                       /* ination buffer.                    */
}
