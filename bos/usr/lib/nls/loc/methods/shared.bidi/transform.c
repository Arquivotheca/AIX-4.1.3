static char sccsid[] = "@(#)51	1.2  src/bos/usr/lib/nls/loc/methods/shared.bidi/transform.c, cfgnls, bos411, 9428A410j 9/10/93 11:10:49";
/*
 *   COMPONENT_NAME: LIBMETH
 *
 *   FUNCTIONS: BidiTransform
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
#include <sys/errno.h>
#include <iconv.h>
#include "bdstruct.h"
#include <sys/lc_layoutP.h>
#include <stdio.h>
#include <iconv.h>
#include <errno.h>
/*********************** BIDI attributes format ****************************/
/*                                                                         */
/*   The BIDI attribute block is defined as an "unsigned long". It consists*/
/*   of 4 bytes that are defined as follows :                              */
/*          BYTE 0 :                                                       */
/*                      bits 0..5   -->   CSD                              */
/*                      determines the kind of shaping required            */
/*                                              0         AUTO             */
/*                                              1         PASS             */
/*                                              2..15     Reserved         */ 
/*                                              16        BASE             */
/*                                              17        INITIAL          */
/*                                              18        MIDDLE           */
/*                                              19        FINAL            */
/*                                              20        ISOLATED         */
/*                                              21..31    Reserved         */
/*                      bits 6      -->   AIX/HOST                         */
/*                      determines if shaping is done in AIX or HOST mode. */
/*                      This affects special cases such as whether lam-alef*/
/*                      is written on one or two cells.                    */
/*                                              0        AIX               */
/*                                              1        HOST              */
/*                      bits 7      -->   Onecell                          */
/*                      deterimines if the seen family is written with the */
/*                      with the tail on the same cell or on a separate    */
/*                      cell                                               */
/*                                              0         two cells        */
/*                                              1         one cell         */
/*          BYTE 1 :                                                       */
/*                      bit  0      -->   Symmetric swapping               */
/*                      determines if symmetric swapping of directional    */
/*                      characters is enabled or disabled                  */
/*                                              0         swapping off     */
/*                                              1         swapping on      */
/*                      bits 1      -->   Word Break                       */
/*                      determines if word break is enabled or disabled    */
/*                                              0         heading off      */
/*                                              1         heading on       */
/*                      bits 2..3   -->   Reserved                         */
/*                      bits 4..7   -->   Numerals                         */
/*                      deterimes the form of the numerics, whther Hindi   */
/*                      or Arabic or upon context, or pass through         */
/*                                              0         ARAB             */
/*                                              1         PASSTHR          */
/*                                              2         HINDI            */
/*                                              3         CONTEXT          */
/*                                              4..15     Reserved         */
/*          BYTE 2 :                                                       */
/*                      bits 0..1   -->   Orientation                      */
/*                      specifies the orientation of the buffer.           */
/*                                              0         LTR              */
/*                                              1         RTL              */
/*                                              2..3      Reserved         */
/*                      bits 2..7   -->   Reserved                         */
/*          BYTE 3 :                                                       */
/*                      bits 0..2   -->   Support Type                     */
/*                      specifies the type of support of the buffer        */
/*                                              0         VISUAL           */
/*                                              1         IMPLICIT         */
/*                                              2         EXPLICIT         */
/*                                              3..8      Reserved         */
/*			bit 3       -->   Bidi Type			   */
/*						0 	  Bidi default	   */
/*						1 	  Bidi UCS	   */
/*                      bits 4..6   -->   Level value                      */
/*                      specifiy the level of the BIDI support, wether per */
/*                      screen/session, or user, or system or.....         */
/*                                              0..2      Reserved         */
/*                                              3         Session/Screen   */
/*                                              4..15     Reserved         */
/*                      bits 7      -->   Reserved                         */
/*                                                                         */
/***************************************************************************/
int BidiTransform (LayoutObjectP Context, 
                   const char *InpBuf,
                   size_t *InpSize,
                   void *OutBuf,
                   size_t *OutSize,
                   size_t *ToOutBuf,
                   size_t *ToInpBuf,
                   unsigned char *BidiLvl)
                   
{
  size_t to_copy;
  int num_flag;
  int ShapingOrient;
  ICSPARAMRec IcsRec;
  BidiValuesRec Values;
  int i;
  int RC=0;

  Values=(BidiValuesRec)Context->core->Values;
 /********************* Start of String Conversion ***************************/

    /* decide on length of input string if not given */
    if (*InpSize==-1)
        *InpSize=strlen(InpBuf);
    
    /* if InpSize=0, then resume previously terminated call */
   if (*InpSize==0)
     goto CopyOut;

    if (*OutSize==0)
    /* calculate size of output buffer and return */
    {
      *OutSize=*InpSize*Values->OutCharSetSize; 
      return (RC);
    }

    /* decide on numerical shape flag */
    if (Values->Num.in!=Values->Num.out)
       num_flag=Values->Num.out;
    else num_flag=NUMERALS_NONE;


    /* decide on intial shaping direction */
   if (Values->Text.in==TEXT_IMPLICIT)
      /* for implicit source strings, shaping is always RTL*/
      ShapingOrient=ORIENTATION_RTL;
   else /* for visual source string, shaping is according to string orient */
     if (Values->Orient.in==ORIENTATION_RTL)
         ShapingOrient=ORIENTATION_RTL;
      else ShapingOrient=ORIENTATION_LTR;
      
  /* copy input string to temporary buffer */
  if (Values->temp_buf)   free(Values->temp_buf);
  Values->temp_buf=malloc(*InpSize);
  memcpy(Values->temp_buf,InpBuf,*InpSize);
  Values->temp_count=*InpSize;
  Values->temp_index=0;

   /* check if ICS required  */
   if ((Values->Text.in==TEXT_IMPLICIT)||(Values->Text.out==TEXT_IMPLICIT))
     {
         IcsRec.orient = Values->Orient.in;
         IcsRec.wordbreak = Values->Word.out;
         IcsRec.codeset = Values->InCharSet;
         IcsRec.num_flag = num_flag;
         if (Values->Text.in!=Values->Text.out)
             IcsRec.flip_flag = TRUE;
         else IcsRec.flip_flag = FALSE;
         IcsRec.symmetric =  (Values->Swap.in!=Values->Swap.out);
         IcsRec.num = *InpSize;
         IcsRec.buffer = Values->temp_buf;
         if (Values->Text.out==TEXT_VISUAL)
           IcsRec.Tables=TEXT_VISUAL;
         else IcsRec.Tables=TEXT_IMPLICIT;
         IcsRec.A_level = BidiLvl;
         IcsRec.SrcToTrgMap = ToOutBuf;
         IcsRec.TrgToSrcMap = ToInpBuf;

            ics_block(&IcsRec); 


         /* if we reorder the string with a LTR orient,
            it means that all the Arabic segments will be
            reversed, so we need to reverse the shaping
            orientation */
           if (IcsRec.orient==ORIENTATION_LTR)
              if (ShapingOrient==ORIENTATION_LTR)
                 ShapingOrient=ORIENTATION_RTL;
              else ShapingOrient=ORIENTATION_LTR;

       }
       else /* visual to visual conversion */
       {
         if (ToOutBuf)             /* set maps to default values */
             for (i=0;i<*InpSize;i++)
               ToOutBuf[i]=i;
         if (ToInpBuf)
             for (i=0;i<*InpSize;i++)
               ToInpBuf[i]=i;
         if (BidiLvl)
             for (i=0;i<*InpSize;i++)
               BidiLvl[i]=0;

                           /* check need to handle symmetric swapping alone */
                           /* Do not swap if LTR->LTR */
         if ((Values->Swap.in!=Values->Swap.out)
            && ((Values->Orient.in==ORIENTATION_RTL)
              ||(Values->Orient.out==ORIENTATION_RTL)))
            Swapall(Values->temp_buf,Values->InCharSet,0,*InpSize-1);   

                           /* check need to handle convert numbers alone */
         if (num_flag != NUMERALS_NONE) 
            Handlenumbers(Values->temp_buf,0,*InpSize-1,num_flag);
       }

                                     /* check if inversion is required */
  if (Values->Orient.in != Values->Orient.out)
  {
        inverse(Values->temp_buf,*InpSize,sizeof(char));  
        if (ToInpBuf)
            inverse(ToInpBuf,*InpSize,sizeof(unsigned int));
        if (ToOutBuf)
            for (i=0;i<*InpSize;i++)
               ToOutBuf[i]=*InpSize-1 -ToOutBuf[i];
         /* if we inverse the string ,
            it means that all the Arabic segments will be
            reversed, so we need to reverse the shaping
            orientation */
           if (ShapingOrient==ORIENTATION_LTR)
              ShapingOrient=ORIENTATION_RTL;
           else ShapingOrient=ORIENTATION_LTR;

  } 
                                                /* check if ASD required  */
     if ((Values->Shaping.in != Values->Shaping.out)
        && Context->core->ShapeEditing )
     {
        BidiShape(Values->temp_buf,*InpSize,Values->OneCell.out,
                           Values->SpecialSh.out,ShapingOrient,
                           Values->Shaping.out);
        if ((Values->SpecialSh.out==TEXT_COMPOSED) && 
            (Values->Shaping.out==TEXT_SHAPED))
          BidiCompose(ShapingOrient,Values->temp_buf,*InpSize);
      }

  /* copy temporary buffer to output buffer */
CopyOut : 
  if (OutBuf)
  {
     if (*OutSize>Values->temp_count-Values->temp_index)
        to_copy=Values->temp_count-Values->temp_index;
     else to_copy=*OutSize;
     *OutSize=to_copy;

     /* check if translation is required */
     if (Values->iconv_handle!=-1)
     {
       size_t innum,outnum;
       char *inbuf;
 
       innum=outnum=to_copy;
       inbuf=&(Values->temp_buf[Values->temp_index]);
       RC=iconv(Values->iconv_handle, &inbuf, &innum,&OutBuf,&outnum);
       if (RC) return (errno);
     }
     else memcpy(OutBuf,&(Values->temp_buf[Values->temp_index]),
                 to_copy); /* just copy, no translation */

     Values->temp_index=Values->temp_index+to_copy;
     /* check if there was space for all the data */
     if (Values->temp_index<Values->temp_count)
        RC=E2BIG;
     else 
     {
      free(Values->temp_buf);
      Values->temp_buf=NULL;
      Values->temp_index=0;
      Values->temp_count=0;
     }
  }
 /********************* End of String Conversion ***************************/
      return RC;      /* successfully performed */
}

