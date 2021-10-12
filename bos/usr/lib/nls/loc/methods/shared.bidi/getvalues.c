static char sccsid[] = "@(#)42  1.3  src/bos/usr/lib/nls/loc/methods/shared.bidi/getvalues.c, cfgnls, bos411, 9428A410j 4/10/94 22:39:43";
/*
 *   COMPONENT_NAME: LIBMETH
 *
 *   FUNCTIONS: BidiGetValues
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
#include "bdstruct.h"
#include <sys/lc_layoutP.h>
int BidiGetValues (LayoutObjectP Context,LayoutValues layout,
                   int *index_returned)
                   
{
  int i=0;
  LayoutEditSize ConSize;
  LayoutEditSize *PConSize;
  LayoutTextDescriptor Descr;
  LayoutTextDescriptor *PDescr;
  
  BidiValuesRec Values;

  Values=(BidiValuesRec)Context->core->Values;

  while (layout[i].name!=0)  /* while not end of array */
  {
    switch (layout[i].name)
     {
         case ActiveBidirection: 
                 layout[i].value=(BooleanValue *)malloc(sizeof (BooleanValue));
                 *(layout[i].value)=Context->core->Bidirection;
                 break;
         case ActiveShapeEditing: 
                 layout[i].value=(BooleanValue *)malloc(sizeof (BooleanValue));
                 *(layout[i].value)=Context->core->ShapeEditing;
                 break;
         case ShapeCharset:
                 *(layout[i].value)=malloc(strlen(Values->OutCharSet));
                 strcpy((layout[i].value),Values->OutCharSet);
                 break;                 
         case ShapeCharsetSize :
                 layout[i].value=(int *)malloc(sizeof (int));
                 *(layout[i].value)=Values->OutCharSetSize;
                 break;
         case ShapeContextSize :
                 ConSize=(LayoutEditSize)malloc(sizeof(LayoutEditSizeRec));
                 ConSize->front=Values->ContextSize.front;
                 ConSize->back=Values->ContextSize.back;
                 PConSize=layout[i].value;
                 *PConSize=ConSize;
                 break;
         default: /* otherwise it must be text descriptors */ 
                 /* check valid name */
                 if (layout[i].name & MaskAllTextDescptors)
                 {
                   *index_returned=i;
                   return(layout[i].name);
                 } 
                 /* allocate and initialize the record */
                 Descr=(LayoutTextDescriptor)malloc(sizeof(LayoutTextDescriptorRec));
                 Descr->in=0x00000000;
                 Descr->out=0x00000000;
                 /* now check on layout[i].name to see which  
                 values are needed */
                 if (layout[i].name & Orientation)
                 {
                    Descr->in |= Values->Orient.in;
                    Descr->out|= Values->Orient.out;
                 }
                 if (layout[i].name & TypeOfText)
                 {
                    Descr->in |= Values->Text.in;
                    Descr->out|= Values->Text.out;
                 }
                 if (layout[i].name & BidiType)
                 {
                    Descr->in |= Values->Bidi.in;
                    Descr->out|= Values->Bidi.out;
                 }
                 if (layout[i].name & Swapping)
                 {
                    Descr->in |= Values->Swap.in;
                    Descr->out|= Values->Swap.out;
                 }
                 if (layout[i].name & Numerals)
                 {
                    Descr->in |= Values->Num.in;
                    Descr->out|= Values->Num.out;
                 }
                 if (layout[i].name & TextShaping)
                 {
                    Descr->in |= Values->Shaping.in;
                    Descr->out|= Values->Shaping.out;
                 }
                 if (layout[i].name & WordBreak)
                 {
                    Descr->in |= Values->Word.in;
                    Descr->out|= Values->Word.out;
                 }
                 if (layout[i].name & ArabicSpecialShaping)
                 {
                    Descr->in |= Values->SpecialSh.in;
                    Descr->out|= Values->SpecialSh.out;
                 }
                 if (layout[i].name & ArabicOneCellShaping)
                 {
                    Descr->in |= Values->OneCell.in;
                    Descr->out|= Values->OneCell.out;
                 }
                 PDescr=layout[i].value;
                 *PDescr=Descr;
                 break;
     }
     i++;
  }
  return(0);
}

