static char sccsid[] = "@(#)48	1.2  src/bos/usr/lib/nls/loc/methods/shared.bidi/setvalues.c, cfgnls, bos411, 9428A410j 9/10/93 11:10:40";
/*
 *   COMPONENT_NAME: LIBMETH
 *
 *   FUNCTIONS: BidiSetValues
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
int BidiSetValues (LayoutObjectP Context,LayoutValues layout,
                   int *index_returned) 
{
  int i=0;
  int Invalue,Outvalue;
  LayoutTextDescriptor Descr;
  BidiValuesRec Values;
 
  Values=(BidiValuesRec)Context->core->Values;
  while (layout[i].name!=0)  /* while not end of array */
  {
    switch (layout[i].name)
     {
         case ActiveBidirection: 
         case ActiveShapeEditing: 
         case ShapeCharsetSize :
         case ShapeContextSize :
                 /* should not be set */
                 *index_returned=i;
                 return(layout[i].name);
                 break;
         case ShapeCharset:
                 free(Values->OutCharSet);
                 if (Values->iconv_handle!=-1)
                     iconv_close(Values->iconv_handle);
                 Values->OutCharSet=malloc(strlen(layout[i].value)+1);
                 strcpy(Values->OutCharSet,layout[i].value);
                 Values->OutCharSetSize=1; /* should be derived from charset */
                 /* check if we need to open iconv */
                 if (strcmp(Values->InCharSet,Values->OutCharSet))
                     Values->iconv_handle= 
                       iconv_open(Values->OutCharSet,Values->InCharSet);
                 else Values->iconv_handle=-1;
                 break;                 
         default: /* otherwise it must be text descriptors */ 
                  /* check valid name */
                 if (layout[i].name & MaskAllTextDescptors)
                 {
                   *index_returned=i;
                   return(layout[i].name);
                 }
                 /* now check on layout[i].name to see which  
                 values are needed */
                 Descr=(LayoutTextDescriptor)layout[i].value;
                 if (layout[i].name & TypeOfText)
                 {
                   /* extract needed value */
                   Invalue=Descr->in & MaskTypeOfText;
                   Outvalue=Descr->out & MaskTypeOfText;
                   /* check if valid */
                   switch (Invalue)
                          {
                           case TEXT_VISUAL:
                           case TEXT_IMPLICIT:
                           case TEXT_EXPLICIT:  break;
                           default:
                                 *index_returned=i;
                                 return(layout[i].name);
                                 break;
                          }
                   switch (Outvalue)
                          {
                           case TEXT_VISUAL:
                           case TEXT_IMPLICIT:
                           case TEXT_EXPLICIT:  break;
                           default:
                                 *index_returned=i;
                                 return(layout[i].name);
                                 break;
                          }
                   /* assign value */
                   Values->Text.in=Invalue; 
                   Values->Text.out=Outvalue; 
                  /* check if other values need change */
                  /* V--V and context num */
                  if ((Values->Text.in==TEXT_VISUAL)
                    &&(Values->Text.out==TEXT_VISUAL)
                    &&(Values->Num.out==NUMERALS_CONTEXTUAL))
                       Values->Num.out=NUMERALS_NOMINAL;
                  /* implicit with special shapes */
                  if (((Values->Text.in==TEXT_IMPLICIT) 
                     ||(Values->Text.out==TEXT_IMPLICIT))
                     &&(Values->Shaping.out!=TEXT_SHAPED)
                     &&(Values->Shaping.out!=TEXT_NOMINAL))
                       Values->Shaping.out=TEXT_NOMINAL;
                 }

                 if (layout[i].name & Orientation)
                 {
                   /* extract needed value */
                   Invalue=Descr->in & MaskOrientation;
                   Outvalue=Descr->out & MaskOrientation;
                   /* check if valid */
                   switch (Invalue)
                          {
                           case ORIENTATION_LTR:
                           case ORIENTATION_RTL:  break;
                           default:
                                 *index_returned=i;
                                 return(layout[i].name);
                                 break;

                          }
                   switch (Outvalue)
                          {
                           case ORIENTATION_LTR:
                           case ORIENTATION_RTL:  break;
                           default:
                                 *index_returned=i;
                                 return(layout[i].name);
                                 break;
                          }
                   /* assign value */
                   Values->Orient.in=Invalue; 
                   Values->Orient.out=Outvalue; 
                 }

                 if (layout[i].name & BidiType)
                 {
                   /* extract needed value */
                   Invalue=Descr->in & MaskBidiType;
                   Outvalue=Descr->out & MaskBidiType;
                   /* check if valid */
                   switch (Invalue)
                          {
                           case BIDI_DEFAULT:
                           case BIDI_UCS:  break;
                           default:
                                 *index_returned=i;
                                 return(layout[i].name);
                                 break;
                          }
                   switch (Outvalue)
                          {
                           case BIDI_DEFAULT:
                           case BIDI_UCS:  break;
                           default:
                                 *index_returned=i;
                                 return(layout[i].name);
                                 break;
                          }
                   /* assign value */
                   Values->Bidi.in=Invalue; 
                   Values->Bidi.out=Outvalue; 
                 }

                 if (layout[i].name & Swapping)
                 {
                   /* extract needed value */
                   Invalue=Descr->in & MaskSwapping;
                   Outvalue=Descr->out & MaskSwapping;
                   /* check if valid */
                   switch (Invalue)
                          {
                           case SWAPPING:
                           case NO_SWAPPING:  break;
                           default:
                                 *index_returned=i;
                                 return(layout[i].name);
                                 break;

                          }
                   switch (Outvalue)
                          {
                           case SWAPPING:
                           case NO_SWAPPING:  break;
                           default:
                                 *index_returned=i;
                                 return(layout[i].name);
                                 break;

                          }
                   /* assign value */
                   Values->Swap.in=Invalue; 
                   Values->Swap.out=Outvalue; 
                 }

                 if (layout[i].name & Numerals)
                 {
                   /* extract needed value */
                   Invalue=Descr->in & MaskNumerals;
                   Outvalue=Descr->out & MaskNumerals;
                   /* check if valid */
                   switch (Invalue)
                          {
                           case NUMERALS_NOMINAL:
                           case NUMERALS_NATIONAL:
                           case NUMERALS_CONTEXTUAL:  break;
                           default:
                                 *index_returned=i;
                                 return(layout[i].name);
                                 break;

                          }
                   switch (Outvalue)
                          {
                           case NUMERALS_NOMINAL:
                           case NUMERALS_NATIONAL:
                           case NUMERALS_CONTEXTUAL:  break;
                           default:
                                 *index_returned=i;
                                 return(layout[i].name);
                                 break;

                          }
                   /* check invalid combination V-->V + contextual */
                   if ((Values->Text.in==TEXT_VISUAL)
                     &&(Values->Text.out==TEXT_VISUAL)
                     &&(Outvalue==NUMERALS_CONTEXTUAL))
                     {
                        *index_returned=i;
                        return(layout[i].name);
                     }
                   /* check if special numerics allowed */
                   if (!Context->core->ShapeEditing && (Outvalue!=NUMERALS_NOMINAL))
                     {
                        *index_returned=i;
                        return(layout[i].name);
                     }
                   /* assign value */
                   Values->Num.in=Invalue; 
                   Values->Num.out=Outvalue; 
                 }

                 if (layout[i].name & TextShaping)
                 {
                   /* extract needed value */
                   Invalue=Descr->in & MaskTextShaping;
                   Outvalue=Descr->out & MaskTextShaping;
                   /* check if valid */
                   switch (Invalue)
                          {
                           case TEXT_SHAPED:
                           case TEXT_NOMINAL:
                           case TEXT_INITIAL:
                           case TEXT_MIDDLE:
                           case TEXT_FINAL:
                           case TEXT_ISOLATED:  break;
                           default : *index_returned=i;
                                     return(layout[i].name);
                                     break;
                          }
                   switch (Outvalue)
                          {
                           case TEXT_SHAPED:
                           case TEXT_NOMINAL:
                           case TEXT_INITIAL:
                           case TEXT_MIDDLE:
                           case TEXT_FINAL:
                           case TEXT_ISOLATED:  break;
                           default : *index_returned=i;
                                     return(layout[i].name);
                                     break;
                          }
                   /* check invalid combination Implicit +special shapes */
                   if (((Values->Text.in==TEXT_IMPLICIT)
                     ||(Values->Text.out==TEXT_IMPLICIT))
                     &&(Outvalue!=TEXT_SHAPED)
                     &&(Outvalue!=TEXT_NOMINAL))
                     {
                          *index_returned=i;
                          return(layout[i].name);
                     }
                   /* check if shaping allowed */
                   if (!Context->core->ShapeEditing && (Outvalue!=TEXT_NOMINAL))
                     {
                          *index_returned=i;
                          return(layout[i].name);
                     }
                   /* assign value */
                   Values->Shaping.in=Invalue; 
                   Values->Shaping.out=Outvalue; 
                 }

                 if (layout[i].name & WordBreak)
                 {
                   /* extract needed value */
                   Invalue=Descr->in & MaskWordBreak;
                   Outvalue=Descr->out & MaskWordBreak;
                   /* check if valid */
                   switch (Invalue)
                          {
                           case BREAK:
                           case NO_BREAK:  break;
                           default : *index_returned=i;
                                     return(layout[i].name);
                                     break;
                          }
                   switch (Outvalue)
                          {
                           case BREAK:
                           case NO_BREAK:  break;
                           default : *index_returned=i;
                                     return(layout[i].name);
                                     break;
                          }
                   /* assign value */
                   Values->Word.in=Invalue; 
                   Values->Word.out=Outvalue; 
                 }
                 if (layout[i].name & ArabicSpecialShaping)
                 {
                   /* extract needed value */
                   Invalue=Descr->in & MaskSpecialShaping;
                   Outvalue=Descr->out & MaskSpecialShaping;
                   /* check if valid */
                   switch (Invalue)
                          {
                           case TEXT_STANDARD:
                           case TEXT_COMPOSED:
                           case TEXT_SPECIAL:  break;
                           default : *index_returned=i;
                                     return(layout[i].name);
                                     break;
                          }
                   switch (Outvalue)
                          {
                           case TEXT_STANDARD:
                           case TEXT_COMPOSED:
                           case TEXT_SPECIAL:  break;
                           default : *index_returned=i;
                                     return(layout[i].name);
                                     break;
                          }
                   /* assign value */
                   Values->SpecialSh.in=Invalue; 
                   Values->SpecialSh.out=Outvalue; 
                 }
                 if (layout[i].name & ArabicOneCellShaping)
                 {
                   /* extract needed value */
                   Invalue=Descr->in & MaskOneCellShaping;
                   Outvalue=Descr->out & MaskOneCellShaping;
                   /* check if valid */
                   switch (Invalue)
                          {
                           case ONECELL_SEEN:
                           case TWOCELL_SEEN:  break;
                           default : *index_returned=i;
                                     return(layout[i].name);
                                     break;
                          }
                   switch (Outvalue)
                          {
                           case ONECELL_SEEN:
                           case TWOCELL_SEEN:  break;
                           default : *index_returned=i;
                                     return(layout[i].name);
                                     break;
                          }
                   /* assign value */
                   Values->OneCell.in=Invalue; 
                   Values->OneCell.out=Outvalue; 
                 }
                 break;
     }
     i++;
  }
  return(0);
}

