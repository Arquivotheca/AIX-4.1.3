static char sccsid[] = "@(#)14	1.2  src/bos/usr/ccs/lib/libi18n/defaults.c, libi18n, bos411, 9428A410j 9/10/93 10:09:51";
/*
 *   COMPONENT_NAME: LIBI18N
 *
 *   FUNCTIONS: DefaultClose
 *		DefaultGetValues
 *		DefaultOpen
 *		DefaultSetValues
 *		DefaultTransform
 *		dummy
 *		inverse
 *		wcsDefaultTransform
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
#include <sys/lc_layout.h>
#include <sys/lc_layoutP.h>
#include "defaults.h"
#include <stdlib.h>
#include <errno.h>

int DefaultClose();
int dummy();
int DefaultSetValues ();
int DefaultGetValues ();
int wcsDefaultTransform ();
int DefaultTransform ();
/*----------------------------------------------------------------*/
LayoutObjectP DefaultOpen()
{
    LayoutObjectP plh;
    DefaultValuesRec defaultvalues;

    plh=malloc(sizeof(LayoutObjectPRec));
    plh->core=malloc(sizeof(LayoutObjectCoreRec));
    plh->methods=malloc(sizeof(LayoutMethodsRec));

    /* set function addresses */
    plh->methods->Open=DefaultOpen;
    plh->methods->Transform=DefaultTransform;
    plh->methods->wcsTransform=wcsDefaultTransform;
    plh->methods->EditShape=dummy;
    plh->methods->wcsEditShape= dummy;
    plh->methods->ShapeBoxChars=dummy;
    plh->methods->SetValues=DefaultSetValues;
    plh->methods->GetValues=DefaultGetValues;
    plh->methods->Close=DefaultClose;

    /* allocate and set layout values */
    defaultvalues=malloc(sizeof(DefaultValuesCoreRec));

    /* Bidirection and ShapeEditing not supported.  */
    plh->core->Bidirection=FALSE;      
    plh->core->ShapeEditing=FALSE;   

    /* default orientation */
    defaultvalues->Orient=malloc(sizeof(LayoutValueRec));
    defaultvalues->Orient->in=ORIENTATION_LTR;
    defaultvalues->Orient->out=ORIENTATION_LTR;

    plh->core->Values=(char *)defaultvalues;

    return(plh);
}
/*----------------------------------------------------------------*/
static int DefaultClose(plh)
LayoutObjectP plh;
{
   DefaultValuesRec defaultvalues;

   defaultvalues=(DefaultValuesRec) plh->core->Values;
   free(defaultvalues->Orient);
   free(plh->core->Values);
   free(plh->core);
   free(plh->methods);
   free(plh);
   return(0);
}

/*----------------------------------------------------------------*/
/* A dummy function that returns 0. */
static int dummy()
{
   return(0);
}
/*----------------------------------------------------------------*/
static int DefaultSetValues (LayoutObjectP Context,LayoutValues layout,
                             int *index_returned)
{
  int i=0;
  int Invalue,Outvalue;
  LayoutTextDescriptor Descr;
  DefaultValuesRec Values;
 
  Values=(DefaultValuesRec)Context->core->Values;
  while (layout[i].name!=0)  /* while not end of array */
  {
       Descr=(LayoutTextDescriptor)layout[i].value;
       switch(layout[i].name)
       {
         case Orientation :
             /* extract needed value */
             Invalue=Descr->in;
             Outvalue=Descr->out;
             /* check if valid */
              if ((Invalue!=ORIENTATION_LTR)&&(Invalue!=ORIENTATION_RTL))
                return(layout[i].name);
              if ((Outvalue!=ORIENTATION_LTR)&&(Outvalue!=ORIENTATION_RTL))
                return(layout[i].name);
              /* assign value */
              Values->Orient->in=Invalue;
              Values->Orient->out=Outvalue;
              break;
         case  TypeOfText :
         case  BidiType   :
         case  Swapping   :
         case  Numerals   :
         case  TextShaping:
         case  WordBreak  :
               break;
         default:
              {
                 *index_returned=i; 
                 return(layout[i].name);
              }
       }
     i++;
  }
  return(0);
}

/*----------------------------------------------------------------*/
static int DefaultGetValues (LayoutObjectP Context,LayoutValues layout,
                             int *index_returned)
{
  int i=0;
  LayoutTextDescriptor Descr;
  LayoutTextDescriptor *PDescr;
  DefaultValuesRec Values;

  Values=(DefaultValuesRec)Context->core->Values;

  while (layout[i].name!=0)  /* while not end of array */
  {
     switch (layout[i].name)
     {
     case ActiveBidirection:
        layout[i].value = (BooleanValue *) malloc (sizeof (BooleanValue));
        *(layout[i].value)=Context->core->Bidirection;
        break;

     case ActiveShapeEditing:
        layout[i].value=(BooleanValue *)malloc(sizeof (BooleanValue));
        *(layout[i].value)=Context->core->ShapeEditing;
        break;

     case Orientation:
        Descr=(LayoutTextDescriptor)malloc(sizeof(LayoutTextDescriptorRec));
        Descr->in = Values->Orient->in;
        Descr->out= Values->Orient->out;
        PDescr=layout[i].value;
        *PDescr=Descr;
        break;
     default: 
        *index_returned=i;
        return(layout[i].name);
        break;
     }
     i++;
  }
  return(0);
}
/*----------------------------------------------------------------*/
static int wcsDefaultTransform (LayoutObject plh, 
                   const wchar_t *InpBuf,
                   size_t *InpSize,
                   void *OutBuf,
                   size_t *OutSize,
                   size_t *ToOutBuf,
                   size_t *ToInpBuf,
                   unsigned char *BidiLvl)
                   
{

   char *mbsin;
   char *mbsout;
   int  mlenin;
   int  mlenout;
   int RC;


   mbsin = malloc(*InpSize * MB_CUR_MAX);  /* enough memory */
   mlenin = *InpSize * MB_CUR_MAX;
   wcstombs(mbsin, InpBuf, mlenin);

   mbsout = malloc(*OutSize * MB_CUR_MAX); 
   mlenout = *OutSize * MB_CUR_MAX;

   /* send multi-byte string */
   RC = DefaultTransform(mbsin,&mlenin,
                      mbsout,&mlenout,
                      ToOutBuf, ToInpBuf,
                      BidiLvl, plh);

   *OutSize=mbstowcs(OutBuf, mbsout, mlenout);
   return(RC);
}
/*----------------------------------------------------------------*/
static void inverse (buffer,num_of_elements,element_size)
/* inverse a stream of elements, where each elemnt is of size element_size */
char *buffer;
size_t num_of_elements;
int element_size;

{
 char *temp; 
 int i,j;
 int start,end;

 temp = malloc (element_size);
 memset (temp,'\0',element_size);
 for (i=0, j=num_of_elements-1; i<j; i++, j--)
 {
   memcpy(temp,&(buffer[i*element_size]),element_size);
   memcpy(&(buffer[i*element_size]),&(buffer[j*element_size]),element_size);
   memcpy(&(buffer[j*element_size]),temp,element_size);
 }
 free (temp);
}
/*----------------------------------------------------------------*/
static int DefaultTransform (LayoutObjectP Context,
                   const char *InpBuf,
                   size_t *InpSize,
                   void *OutBuf,
                   size_t *OutSize,
                   size_t *ToOutBuf,
                   size_t *ToInpBuf,
                   unsigned char *BidiLvl)
                   
{
  size_t to_copy;
  DefaultValuesRec Values;
  int i;
  int RC=0;

  Values=(DefaultValuesRec)Context->core->Values;

    /* decide on length of input string if not given */
    if (*InpSize==-1)
        *InpSize=strlen(InpBuf);
    
    /* if InpSize=0, then resume previously terminated call */
   if (*InpSize==0)
     goto CopyOut;

    if (*OutSize==0)
    /* calculate size of output buffer and return */
    {
      *OutSize=*InpSize; 
      return (RC);
    }

  /* copy input string to temporary buffer */
  if (Values->temp_buf)   free(Values->temp_buf);
  Values->temp_buf=malloc(*InpSize);
  memcpy(Values->temp_buf,InpBuf,*InpSize);
  Values->temp_count=*InpSize;
  Values->temp_index=0;

   if (ToOutBuf)             /* set maps to default values */
      for (i=0;i<*InpSize;i++)
          ToOutBuf[i]=i;
   if (ToInpBuf)
      for (i=0;i<*InpSize;i++)
          ToInpBuf[i]=i;
   if (BidiLvl)
      for (i=0;i<*InpSize;i++)
          BidiLvl[i]=0;

                                     /* check if inversion is required */
  if (Values->Orient->in != Values->Orient->out)
  {
        inverse(Values->temp_buf,*InpSize,sizeof(char));  
        if (ToInpBuf)
            inverse(ToInpBuf,*InpSize,sizeof(unsigned int));
        if (ToOutBuf)
            for (i=0;i<*InpSize;i++)
               ToOutBuf[i]=*InpSize-1 -ToOutBuf[i];
  } 

  /* copy temporary buffer to output buffer */
CopyOut : 
  if (OutBuf)
  {
     if (*OutSize>Values->temp_count-Values->temp_index)
        to_copy=Values->temp_count-Values->temp_index;
     else to_copy=*OutSize;
     *OutSize=to_copy;
     memcpy(OutBuf,&(Values->temp_buf[Values->temp_index]),
                 to_copy); /* just copy, no translation */

     Values->temp_index=Values->temp_index+to_copy;
     /* check if there was space for all the data */
     if (Values->temp_index<Values->temp_count)
        RC=E2BIG;
     else
     {
      free(Values->temp_buf);
      Values->temp_buf=NULL;
     }
  }
      return RC;      /* successfully performed */
}
/*----------------------------------------------------------------*/


