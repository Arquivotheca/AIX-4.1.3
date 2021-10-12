static char sccsid[] = "@(#)33  1.2  src/bos/usr/lib/nls/loc/methods/shared.bidi/BDInit.c, cfgnls, bos411, 9428A410j 4/10/94 22:39:40";
/*
 *   COMPONENT_NAME: LIBMETH
 *
 *   FUNCTIONS: BidiClose
 *		BidiOpen
 *		Initiate
 *		SetCurrentCodeSet
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
#include <stdio.h>
#include <locale.h>
#include <langinfo.h>

extern int BidiTransform();
extern int wcsBidiTransform();
extern int BidiEditShape();
extern int wcsBidiEditShape();
extern int BidiShapeBoxChars();
extern int BidiSetValues();
extern int BidiGetValues();
LayoutObjectP BidiOpen();
int BidiClose();

/***********************************************************************/
/* This function returns the current code set. */
static void SetCurrentCodeSet(values)
BidiValuesRec values;
{
        wchar_t    *cp;

        cp = (wchar_t *)nl_langinfo (CODESET);
        values->InCharSet=malloc(strlen(cp)+1);
        if (strcmp(cp,ARABIC_CHARSET2)==0) {
		cp = ARABIC_CHARSET1;
	}
        values->OutCharSet=malloc(strlen(cp)+1);
        strcpy(values->InCharSet,cp);
        strcpy(values->OutCharSet,cp);
        return ;
}
/***********************************************************************/
lc_layout *Initiate()   /* this is the entry point of the module */
{
 static lc_layout cd; 

   cd.lc_hdr.__magic=_LC_MAGIC;      /* set the magic tag for __lc_load */
   cd.lc_hdr.__type_id=_LC_LAYOUT; 
   cd.lc_hdr.__version=_LC_VERSION; 
   cd.lc_hdr.__size=sizeof(LayoutObjectPRec);
   cd.initialize=BidiOpen;   /* set the address of the initialization routine */
   return (&cd);
}
/***********************************************************************/
/* This is the BIDI library initialization routine. It fills the       */
/* structure plh with the addresses of the functions.            */
/***********************************************************************/
LayoutObjectP BidiOpen()
{
    LayoutObjectP plh;
    BidiValuesRec bidivalues;

    plh=malloc(sizeof(LayoutObjectPRec));
    plh->core=malloc(sizeof(LayoutObjectCoreRec));
    plh->core->locale=malloc(sizeof(_LC_object_t));
    plh->methods=malloc(sizeof(LayoutMethodsRec));

    plh->core->locale->__magic=_LC_MAGIC;     
    plh->core->locale->__type_id=_LC_LAYOUT; 
    plh->core->locale->__version=_LC_VERSION; 
    plh->core->locale->__size=sizeof(LayoutObjectPRec);

    /* set function addresses */
    plh->methods->Open=BidiOpen;
    plh->methods->Transform=BidiTransform;
    plh->methods->wcsTransform=wcsBidiTransform;
    plh->methods->EditShape=BidiEditShape;
    plh->methods->wcsEditShape= wcsBidiEditShape;
    plh->methods->ShapeBoxChars=BidiShapeBoxChars;
    plh->methods->SetValues=BidiSetValues;
    plh->methods->GetValues=BidiGetValues;
    plh->methods->Close=BidiClose;

    /* allocate and set layout values */
    bidivalues=malloc(sizeof(BidiValuesCoreRec));

    /* Bidirection supported in Hebrew and Arabic locales  */
    plh->core->Bidirection=TRUE;      

    bidivalues->temp_count=0;
    bidivalues->temp_index=0;
    SetCurrentCodeSet(bidivalues);

    /* check if we need to open iconv */
    if (strcmp(bidivalues->InCharSet,bidivalues->OutCharSet))
        bidivalues->iconv_handle=
               iconv_open(bidivalues->OutCharSet,bidivalues->InCharSet);
    else bidivalues->iconv_handle=-1;

    if ((strcmp(bidivalues->OutCharSet,ARABIC_CHARSET1)==0)
        ||  (strcmp(bidivalues->OutCharSet,ARABIC_CHARSET2)==0))
       /* Shaping supported in Arabic locales */
       plh->core->ShapeEditing=TRUE;     
    /* Shaping not supported in Hebrew */
    else plh->core->ShapeEditing=FALSE;   
    bidivalues->ShapeState='I';              /*Initial state */

    /* current code set size (to be changed) */ 
    bidivalues->OutCharSetSize=1; 

    /* characters to be checked  for ShapeEditing */ 
    bidivalues->ContextSize.front=2; 
    bidivalues->ContextSize.back=3;   

    /* default orientation */
    bidivalues->Orient.in=ORIENTATION_LTR;
    bidivalues->Orient.out=ORIENTATION_LTR;

    /* default text type */
    bidivalues->Text.in=TEXT_IMPLICIT;
    bidivalues->Text.out=TEXT_VISUAL;

    /* default Bidi Type */
    bidivalues->Bidi.in=BIDI_DEFAULT;
    bidivalues->Bidi.out=BIDI_DEFAULT;

    /* default Swapping mode */
    bidivalues->Swap.in=NO_SWAPPING;
    bidivalues->Swap.out=SWAPPING;

    /* default numeric shapes */
    bidivalues->Num.in=NUMERALS_NOMINAL;
    /* if ShapeEditing is inactive, this means we are Hebrew, so 
       set numerics to nimonal */
    if (plh->core->ShapeEditing)
       bidivalues->Num.out=NUMERALS_CONTEXTUAL;
    else bidivalues->Num.out=NUMERALS_NOMINAL;

    /* default Shaping mode */
    bidivalues->Shaping.in=TEXT_NOMINAL;
    /* if ShapeEditing is inactive, this means we are Hebrew, so 
       set shaping to nominal */
    if (plh->core->ShapeEditing)
       bidivalues->Shaping.out=TEXT_SHAPED;
    else bidivalues->Shaping.out=TEXT_NOMINAL;

    /* default word break mode */
    bidivalues->Word.in=NO_BREAK;
    bidivalues->Word.out=NO_BREAK;

    /* default onecell mode */
    bidivalues->OneCell.in=TWOCELL_SEEN;
    bidivalues->OneCell.out=TWOCELL_SEEN;

    /* default AIX/HOST mode */
    bidivalues->SpecialSh.in=TEXT_STANDARD;
    bidivalues->SpecialSh.out=TEXT_STANDARD;

    plh->core->Values=(char *)bidivalues;

    return(plh);
}
/**********************************************************************/
static int BidiClose(plh)
LayoutObjectP plh;
{
   BidiValuesRec bidivalues;

   bidivalues=(BidiValuesRec) plh->core->Values;
   free(bidivalues->InCharSet);
   free(bidivalues->OutCharSet);
   if (bidivalues->iconv_handle!=-1)
          iconv_close(bidivalues->iconv_handle);
   free(plh->core->Values);
   free(plh);
   return(0);
}

