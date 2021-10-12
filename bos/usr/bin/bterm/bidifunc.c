static char sccsid[] = "@(#)78	1.4  src/bos/usr/bin/bterm/bidifunc.c, libbidi, bos410, 9342A410j 10/26/93 10:46:09";
/*
 *   COMPONENT_NAME: LIBBIDI
 *
 *   FUNCTIONS: BDActive
 *		SetCurSeg
 *		bidi_init
 *		do_SAPV_option
 *		do_bidi_SAPV
 *		do_bidi_SPD
 *		do_bidi_SRS
 *		do_bidi_command_mode
 *		do_bidi_disable_keys
 *		do_bidi_enable_keys
 *		do_bidi_reset_mode_visual
 *		do_bidi_restore_environment
 *		do_bidi_save_environment
 *		do_bidi_screen_reverse
 *		do_bidi_set_LTR_Latin
 *		do_bidi_set_Latin
 *		do_bidi_set_RTL_national
 *		do_bidi_set_aix_mode
 *		do_bidi_set_arabic_nss
 *		do_bidi_set_asd
 *		do_bidi_set_base_csd
 *		do_bidi_set_bilingual_nss
 *		do_bidi_set_final
 *		do_bidi_set_hindu_nss
 *		do_bidi_set_host_mode
 *		do_bidi_set_initial
 *		do_bidi_set_isolated
 *		do_bidi_set_middle
 *		do_bidi_set_mode_implicit
 *		do_bidi_set_national
 *		do_bidi_set_passthru_csd
 *		do_bidi_set_passthru_nss
 *		do_bidi_setoff
 *		do_bidi_seton
 *		do_bidi_symmetric_off
 *		do_bidi_symmetric_on
 *		do_bidi_toggle_autoshape
 *		do_bidi_toggle_column_head
 *		set_bdseg_defaults
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
/*   Documentation : bidifunc.c
**               This file includes all the bidi related functions.
**                
*/

#include <sys/types.h>
#include <fcntl.h>
#include <termios.h>
#include <cur00.h>
#include <memory.h>
#include <string.h>
#include <term.h>
#include <locale.h>
#include <langinfo.h>

#include <stdio.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#include "global.h"
#include "trace.h"

/* definition of bidi structure BDCurSeg */
BDSeg_st *BDCurSeg;
/* defaults structure */
extern BDSeg_st *BDSegDefaults;

/*--------------------------------------------------------------------------*/ 
void bidi_init()
{
  BDCurSeg = (BDSeg_st *) malloc(BDSEGSIZE);
  if (!BDCurSeg) {TRACE(("Could not allocate BDCurSeg.\n"));}
  BDCurSeg->next = BDSEGNULL;    /* pointer to next saved structure */
  set_bidi_mode();               /* bidi mode is enabled */
  BDCurSeg->keys_enabled = TRUE; /* bidi keys enabled */
  set_latin_kbd();               /* LATIN keyboard layer */
  set_LTR_mode();                /* left_to_right screen orientation */
  set_symetric_mode();           /* swap symmetric characters */
  set_aix_mode();                /* aix mode, not host */
  set_onecell_mode();            /* seen family on one cell */
  set_nonulls_mode();              /* initialize buffer with nulls */
  set_left_autopush();           /* autopush enabled */
  set_right_autopush();
  reset_in_autopush();           /* set off all push mode indicators */
  reset_push_mode();
  BDCurSeg->push_start_loc = 0;
  BDCurSeg->push_cur_loc   = 0;
  BDCurSeg->push_count     = 0;
  set_implicit_text();           /* implicit text mode */
  set_asd_on();                  /* autoshaping */
  set_bilingual_nss();           /* upon context numerals */
  ActiveShaping=TRUE;
  ActiveBidi=TRUE;
}
/*------------------ SetCurSeg ----------------------------*/
/* To set the layout values to the values in BDCurSeg.*/

void SetCurSeg()
{

  LayoutValues layout;
  LayoutTextDescriptor Descr;
 int index;

  layout=malloc(2*sizeof(LayoutValueRec));
  Descr=malloc(sizeof(LayoutTextDescriptorRec));
  
  layout[0].name=AllTextDescptors;
  layout[0].value=(caddr_t)Descr;
  layout[1].name=0;
  Descr->in=0x00000000;
  Descr->out=0x00000000;

  if (is_implicit_text()) 
               Descr->in |= TEXT_IMPLICIT;
  if (is_RTL_mode()) 
               Descr->in |= ORIENTATION_RTL;
  if (is_symetric_mode())
               Descr->out |= SWAPPING;
  if (is_bilingual_nss())
               Descr->out |= NUMERALS_CONTEXTUAL;
           else if (is_hindu_nss())
               Descr->out |= NUMERALS_NATIONAL;
  if (is_column_head())
               Descr->out |= BREAK;
  if (is_onecell_mode())
               Descr->out |= ONECELL_SEEN;
  Descr->in |= TEXT_NOMINAL;
  if (is_initial_csd())
                Descr->out |= TEXT_INITIAL;
           else if (is_middle_csd())
                Descr->out |= TEXT_MIDDLE;
           else if (is_final_csd())
                Descr->out |= TEXT_FINAL;
           else if (is_isolated_csd())
                Descr->out |= TEXT_ISOLATED;
           else if (is_basic_csd())
                 Descr->out |= TEXT_NOMINAL;

  layout_object_setvalue(plh,layout,&index);
  free(Descr);
  free(layout);
}

/*------------------ BDActive ----------------------------*/
/* check active bidi functionality and shaping for the layout
   object that is already created. And set layoutvalues according
   to the values in BDCurSeg. */
void BDActive()
{
  LayoutTextDescriptor Descr;
  LayoutValues layout;
  int index;

  layout=malloc(3*sizeof(LayoutValueRec));
  layout[0].name=ActiveBidirection;
  layout[1].name=ActiveShapeEditing;
  layout[2].name=0;

  layout_object_getvalue(plh,layout,&index);
  if (!*layout[0].value)  /* no bidi active */
     {
        ActiveBidi=FALSE;  
        reset_bidi_mode();
     }
  if (!*layout[1].value)  /* no shaping active */
     {
       ActiveShaping=FALSE;  
       set_passthru_csd();
       set_arabic_nss();
     }

  free(layout[0].value);
  free(layout[1].value);
  free(layout);
  SetCurSeg();
}
/*------------------ screen reverse ----------------------------*/
void do_bidi_screen_reverse()
{
 int i;
 LayoutValues layout;
 LayoutTextDescriptor Descr;
 int index;
 
 STATE=0;           /* identified sequence, so reset state */
 if (is_bidi_mode())
 {
  layout=malloc(2*sizeof(LayoutValueRec));
  Descr=malloc(sizeof(LayoutTextDescriptorRec));
  layout[0].name=Orientation;
  layout[0].value=(caddr_t)Descr;
  layout[1].name=0;
  if (is_LTR_mode())
    {
      set_RTL_mode();
      set_nl_kbd();
      Descr->in=ORIENTATION_RTL;
      Descr->out=ORIENTATION_LTR;
    }
  else 
    {
      set_LTR_mode();
      set_latin_kbd();
      Descr->in=ORIENTATION_LTR;
      Descr->out=ORIENTATION_LTR;
    }
  layout_object_setvalue(plh,layout,&index);
  free(Descr);
  free(layout);
  for (i=0;i<LINES;i++)
   update_line(i);
 } /* if bidi mode */
}
/*------------------------ enable keys --------------------------*/
void do_bidi_enable_keys()
{
 STATE=0;           /* identified sequence, so reset state */
 BDCurSeg->keys_enabled = TRUE; /* bidi keys enabled */
}
/*------------------------ disable keys --------------------------*/
void do_bidi_disable_keys()
{
 STATE=0;           /* identified sequence, so reset state */
 BDCurSeg->keys_enabled = FALSE; /* bidi keys disabled */
}
/*------------------------ save environment --------------------------*/
void do_bidi_save_environment()
{
  BDSeg_st *newseg;

  STATE=0;           /* identified sequence, so reset state */
  newseg = (BDSeg_st *) malloc(BDSEGSIZE);
  if (newseg)
  {
    memcpy(newseg, BDCurSeg, BDSEGSIZE);
    newseg->next = BDCurSeg;
    BDCurSeg = newseg;
  }
  else TRACE(("bidi save env: cannot allocate new segment\n"));
}

/*------------------------ restore environment --------------------------*/
void do_bidi_restore_environment()
{

 BDSeg_st *oldseg;
 int bidi,orient,nulls,text;
 int i;

 STATE=0;           /* identified sequence, so reset state */
 if (oldseg = BDCurSeg->next)    /* restore env only if there is a saved one */
 {

    /* save some values, because if these change we will need to refresh */
    bidi   = is_bidi_mode();
    text   = is_implicit_text();
    nulls  = is_nulls_mode();
    orient = is_RTL_mode();
    
    /* replace CurSeg with the saved one */
    oldseg = BDCurSeg->next;
    free(BDCurSeg);
    BDCurSeg = oldseg;

    SetCurSeg();

    /* check if we need to refresh */
    if ((bidi!= (is_bidi_mode())) || (text!=(is_implicit_text())) 
     || (nulls!=(is_nulls_mode())) || (orient!=(is_RTL_mode())))
         for (i=0;i<LINES;i++)
           update_line(i);
 }
}
/*---------------set RTL orientation and national keyboard -----------*/
void do_bidi_set_RTL_national()
{
 int i;
 LayoutValues layout;
 LayoutTextDescriptor Descr;
 int index;

 STATE=0;           /* identified sequence, so reset state */
 if (is_bidi_mode())
 {
  layout=malloc(2*sizeof(LayoutValueRec));
  Descr=malloc(sizeof(LayoutTextDescriptorRec));
  layout[0].name=Orientation;
  layout[0].value=(caddr_t)Descr;
  layout[1].name=0;
  Descr->in=ORIENTATION_RTL;
  Descr->out=ORIENTATION_LTR;
  layout_object_setvalue(plh,layout,&index);
  free(Descr);
  free(layout);
  set_RTL_mode();  /* set RTL screen orientation */
  set_nl_kbd();    /* set national keyboard layer */
  for (i=0;i<LINES;i++)
   update_line(i);
 }
}
/*----------------set LTR orientation and Latin keyboard ----------*/
void do_bidi_set_LTR_Latin()
{
 int i;
 LayoutValues layout;
 LayoutTextDescriptor Descr;
 int index;

 STATE=0;           /* identified sequence, so reset state */
 if (is_bidi_mode())
 {
  layout=malloc(2*sizeof(LayoutValueRec));
  Descr=malloc(sizeof(LayoutTextDescriptorRec));
  layout[0].name=Orientation;
  layout[0].value=(caddr_t)Descr;
  layout[1].name=0;
  Descr->in=ORIENTATION_LTR;
  Descr->out=ORIENTATION_LTR;
  layout_object_setvalue(plh,layout,&index);
  free(Descr);
  free(layout);
  set_LTR_mode();     /* set LTR screen orientation */
  set_latin_kbd();    /* set Latin keyboard layer */
  for (i=0;i<LINES;i++)
   update_line(i);
 }
}
/*------------------------ set national keyboard  ------------------*/
void do_bidi_set_national()
{
 STATE=0;           /* identified sequence, so reset state */
 set_nl_kbd();    /* set national keyboard layer */
}
/*------------------------ set Latin keyboard --------------------------*/
void do_bidi_set_Latin()
{
 STATE=0;           /* identified sequence, so reset state */
 set_latin_kbd();    /* set Latin keyboard layer */
}
/*------------------- seton bidi mode  --------------------------*/
void do_bidi_seton()
{
 int i;
 STATE=0;           /* identified sequence, so reset state */
 if (!is_bidi_mode() && ActiveBidi)
 {
  set_bidi_mode();   
  for (i=0;i<LINES;i++)
   update_line(i);
 }
}
/*------------------- setoff bidi mode  --------------------------*/
void do_bidi_setoff()
{
 int i;
 STATE=0;           /* identified sequence, so reset state */
 if (is_bidi_mode() && ActiveBidi)
 {
  reset_bidi_mode();
  for (i=0;i<LINES;i++)
   update_line(i);
 }
}
/*-------------------- set symmetric swapping on -----------------*/
void do_bidi_symmetric_on()
{
  LayoutValues layout;
  LayoutTextDescriptor Descr;
 int index;

  STATE=0;           /* identified sequence, so reset state */
  layout=malloc(2*sizeof(LayoutValueRec));
  Descr=malloc(sizeof(LayoutTextDescriptorRec));
  layout[0].name=Swapping;
  layout[0].value=(caddr_t)Descr;
  layout[1].name=0;
  Descr->in=NO_SWAPPING;
  Descr->out=SWAPPING;
  layout_object_setvalue(plh,layout,&index);
  free(Descr);
  free(layout);
  set_symetric_mode();
}
/*--------------------- set symmetric swapping off ---------------*/
void do_bidi_symmetric_off()
{
  LayoutValues layout;
  LayoutTextDescriptor Descr;
 int index;

  STATE=0;           /* identified sequence, so reset state */
  layout=malloc(2*sizeof(LayoutValueRec));
  Descr=malloc(sizeof(LayoutTextDescriptorRec));
  layout[0].name=Swapping;
  layout[0].value=(caddr_t)Descr;
  layout[1].name=0;
  Descr->in=NO_SWAPPING;
  Descr->out=NO_SWAPPING;
  layout_object_setvalue(plh,layout,&index);
  free(Descr);
  free(layout);
  reset_symetric_mode();
}
/*--------------------- toggle column heading mode -----------------*/
void do_bidi_toggle_column_head()
{
  int i;
  LayoutValues layout;
  LayoutTextDescriptor Descr;
 int index;

  layout=malloc(2*sizeof(LayoutValueRec));
  Descr=malloc(sizeof(LayoutTextDescriptorRec));
  layout[0].name=WordBreak;
  layout[0].value=(caddr_t)Descr;
  layout[1].name=0;
  if (is_column_head())
   {
    reset_column_head();
    Descr->in=NO_BREAK;
    Descr->out=NO_BREAK;
   }
  else
   {
    set_column_head();
    Descr->in=NO_BREAK;
    Descr->out=BREAK;
   }
  layout_object_setvalue(plh,layout,&index);
  free(Descr);
  free(layout);
 for (i=0;i<LINES;i++)
   update_line(i);
}
/*--------------------- toggle autoshape mode -----------------*/
void do_bidi_toggle_autoshape()
{
  LayoutValues layout;
  LayoutTextDescriptor Descr;
 int index;

 if (ActiveShaping)
 {
  layout=malloc(2*sizeof(LayoutValueRec));
  Descr=malloc(sizeof(LayoutTextDescriptorRec));
  layout[0].name=TextShaping;
  layout[0].value=(caddr_t)Descr;
  layout[1].name=0;
  if (is_asd_on())
   {
      if (is_visual_text() && is_push_mode())
         push_passthru();
      Descr->in=TEXT_NOMINAL;
      Descr->out=TEXT_NOMINAL;
      set_passthru_csd();
   }
  else  
   {
      Descr->in=TEXT_NOMINAL;
      Descr->out=TEXT_SHAPED;
      set_asd_on(); 
   }
  layout_object_setvalue(plh,layout,&index);
  free(Descr);
  free(layout);
 }
}
/*--------------------- set passthru shapes  ----------------------*/
void do_bidi_set_passthru_csd()
{
  LayoutValues layout;
  LayoutTextDescriptor Descr;
 int index;

 if (is_visual_text() && ActiveShaping)
 {
  layout=malloc(2*sizeof(LayoutValueRec));
  Descr=malloc(sizeof(LayoutTextDescriptorRec));
  layout[0].name=TextShaping;
  layout[0].value=(caddr_t)Descr;
  layout[1].name=0;
  Descr->in=TEXT_NOMINAL;
  Descr->out=TEXT_NOMINAL;
  layout_object_setvalue(plh,layout,&index);
  free(Descr);
  free(layout);
  set_passthru_csd();
 }
}
/*--------------------- set base shapes  ----------------------*/
void do_bidi_set_base_csd()
{
  LayoutValues layout;
  LayoutTextDescriptor Descr;
 int index;

 if (is_visual_text() && ActiveShaping)
 {
  layout=malloc(2*sizeof(LayoutValueRec));
  Descr=malloc(sizeof(LayoutTextDescriptorRec));
  layout[0].name=TextShaping;
  layout[0].value=(caddr_t)Descr;
  layout[1].name=0;
  Descr->in=TEXT_SHAPED;
  Descr->out=TEXT_NOMINAL;
  layout_object_setvalue(plh,layout,&index);
  free(Descr);
  free(layout);
  set_basic_csd();
 }
}
/*--------------------- set initial shapes  ----------------------*/
void do_bidi_set_initial()
{
  LayoutValues layout;
  LayoutTextDescriptor Descr;
 int index;

 if (is_visual_text() && ActiveShaping)
 {
  layout=malloc(2*sizeof(LayoutValueRec));
  Descr=malloc(sizeof(LayoutTextDescriptorRec));
  layout[0].name=TextShaping;
  layout[0].value=(caddr_t)Descr;
  layout[1].name=0;
  Descr->in=TEXT_NOMINAL;
  Descr->out=TEXT_INITIAL;
  layout_object_setvalue(plh,layout,&index);
  free(Descr);
  free(layout);
  set_initial_csd();
 }
}
/*--------------------- set middle shapes  ----------------------*/
void do_bidi_set_middle()
{
  LayoutValues layout;
  LayoutTextDescriptor Descr;
 int index;

 if (is_visual_text() && ActiveShaping)
 {
  layout=malloc(2*sizeof(LayoutValueRec));
  Descr=malloc(sizeof(LayoutTextDescriptorRec));
  layout[0].name=TextShaping;
  layout[0].value=(caddr_t)Descr;
  layout[1].name=0;
  Descr->in=TEXT_NOMINAL;
  Descr->out=TEXT_MIDDLE;
  layout_object_setvalue(plh,layout,&index);
  free(Descr);
  free(layout);
  set_middle_csd();
  }
}
/*--------------------- set final shapes  ----------------------*/
void do_bidi_set_final()
{
  LayoutValues layout;
  LayoutTextDescriptor Descr;
 int index;

 if (is_visual_text() && ActiveShaping)
 {
  layout=malloc(2*sizeof(LayoutValueRec));
  Descr=malloc(sizeof(LayoutTextDescriptorRec));
  layout[0].name=TextShaping;
  layout[0].value=(caddr_t)Descr;
  layout[1].name=0;
  Descr->in=TEXT_NOMINAL;
  Descr->out=TEXT_FINAL;
  layout_object_setvalue(plh,layout,&index);
  free(Descr);
  free(layout);
  set_final_csd();
  }
}
/*--------------------- set isolated shapes  ----------------------*/
void do_bidi_set_isolated()
{
  LayoutValues layout;
  LayoutTextDescriptor Descr;
 int index;

 if (is_visual_text() && ActiveShaping)
 {
  layout=malloc(2*sizeof(LayoutValueRec));
  Descr=malloc(sizeof(LayoutTextDescriptorRec));
  layout[0].name=TextShaping;
  layout[0].value=(caddr_t)Descr;
  layout[1].name=0;
  Descr->in=TEXT_NOMINAL;
  Descr->out=TEXT_ISOLATED;
  layout_object_setvalue(plh,layout,&index);
  free(Descr);
  free(layout);
  set_isolated_csd();
 }
}

/*--------------------- set autoshaping  ----------------------*/
void do_bidi_set_asd()
{
  LayoutValues layout;
  LayoutTextDescriptor Descr;
 int index;

 if (ActiveShaping)
 {
  layout=malloc(2*sizeof(LayoutValueRec));
  Descr=malloc(sizeof(LayoutTextDescriptorRec));
  layout[0].name=TextShaping;
  layout[0].value=(caddr_t)Descr;
  layout[1].name=0;
  Descr->in=TEXT_NOMINAL;
  Descr->out=TEXT_SHAPED;
  layout_object_setvalue(plh,layout,&index);
  free(Descr);
  free(layout);
  set_asd_on();
 }
}

/*--------------------- set hindu nss  ----------------------*/
void do_bidi_set_hindu_nss()
{
  LayoutValues layout;
  LayoutTextDescriptor Descr;
 int index;

 if (ActiveShaping)
 {
  layout=malloc(2*sizeof(LayoutValueRec));
  Descr=malloc(sizeof(LayoutTextDescriptorRec));
  layout[0].name=Numerals;
  layout[0].value=(caddr_t)Descr;
  layout[1].name=0;
  Descr->in=NUMERALS_NOMINAL;
  Descr->out=NUMERALS_NATIONAL;
  layout_object_setvalue(plh,layout,&index);
  free(Descr);
  free(layout);
  set_hindu_nss();
  }
}

/*--------------------- set arabic nss  ----------------------*/
void do_bidi_set_arabic_nss()
{
  LayoutValues layout;
  LayoutTextDescriptor Descr;
 int index;

 if (ActiveShaping)
 {
  layout=malloc(2*sizeof(LayoutValueRec));
  Descr=malloc(sizeof(LayoutTextDescriptorRec));
  layout[0].name=Numerals;
  layout[0].value=(caddr_t)Descr;
  layout[1].name=0;
  Descr->in=NUMERALS_NATIONAL;
  Descr->out=NUMERALS_NOMINAL;
  layout_object_setvalue(plh,layout,&index);
  free(Descr);
  free(layout);
  set_arabic_nss();
  }
}

/*--------------------- set passthru nss  ----------------------*/
void do_bidi_set_passthru_nss()
{
  LayoutValues layout;
  LayoutTextDescriptor Descr;
 int index;

 if (ActiveShaping)
 {
  layout=malloc(2*sizeof(LayoutValueRec));
  Descr=malloc(sizeof(LayoutTextDescriptorRec));
  layout[0].name=Numerals;
  layout[0].value=(caddr_t)Descr;
  layout[1].name=0;
  Descr->in=NUMERALS_NOMINAL;
  Descr->out=NUMERALS_NOMINAL;
  layout_object_setvalue(plh,layout,&index);
  free(Descr);
  free(layout);
  set_passthru_nss();
 }
}

/*--------------------- set bilingual nss  ----------------------*/
void do_bidi_set_bilingual_nss()
{
  LayoutValues layout;
  LayoutTextDescriptor Descr;
 int index;

 if (is_implicit_text() && ActiveShaping)
 {
  layout=malloc(2*sizeof(LayoutValueRec));
  Descr=malloc(sizeof(LayoutTextDescriptorRec));
  layout[0].name=Numerals;
  layout[0].value=(caddr_t)Descr;
  layout[1].name=0;
  Descr->in=NUMERALS_NOMINAL;
  Descr->out=NUMERALS_CONTEXTUAL;
  layout_object_setvalue(plh,layout,&index);
  free(Descr);
  free(layout);
  set_bilingual_nss();
 }
}

/*--------------------- set host mode  ----------------------*/
void do_bidi_set_host_mode()
{
  LayoutValues layout;
  LayoutTextDescriptor Descr;
 int index;

  layout=malloc(2*sizeof(LayoutValueRec));
  Descr=malloc(sizeof(LayoutTextDescriptorRec));
  layout[0].name=ArabicSpecialShaping;
  layout[0].value=(caddr_t)Descr;
  layout[1].name=0;
  Descr->in=TEXT_STANDARD;
  Descr->out=TEXT_SPECIAL;
  layout_object_setvalue(plh,layout,&index);
  free(Descr);
  free(layout);
  set_host_mode();
}

/*--------------------- set aix mode  ----------------------*/
void do_bidi_set_aix_mode()
{
  LayoutValues layout;
  LayoutTextDescriptor Descr;
 int index;

  layout=malloc(2*sizeof(LayoutValueRec));
  Descr=malloc(sizeof(LayoutTextDescriptorRec));
  layout[0].name=ArabicSpecialShaping;
  layout[0].value=(caddr_t)Descr;
  layout[1].name=0;
  Descr->in=TEXT_STANDARD;
  Descr->out=TEXT_STANDARD;
  layout_object_setvalue(plh,layout,&index);
  free(Descr);
  free(layout);
  set_aix_mode();
}

/*--------------------- Set bidi mode to implicit ------------------*/
void do_bidi_set_mode_implicit()
{
  int i;
  LayoutValues layout;
  LayoutTextDescriptor Descr;
 int index;

  STATE=0;
  layout=malloc(2*sizeof(LayoutValueRec));
  Descr=malloc(sizeof(LayoutTextDescriptorRec));
  layout[0].name=TypeOfText;
  layout[0].value=(caddr_t)Descr;
  layout[1].name=0;
  Descr->in=TEXT_IMPLICIT;
  Descr->out=TEXT_VISUAL;
  layout_object_setvalue(plh,layout,&index);
  free(Descr);
  free(layout);
  set_implicit_text();
  for (i=0;i<LINES;i++)
       update_line(i);

}
/*--------------------- Set bidi mode to visual ------------------*/
void do_bidi_reset_mode_visual()
{
  LayoutValues layout;
  LayoutTextDescriptor Descr;
  int i;
 int index;

  STATE=0;
  layout=malloc(2*sizeof(LayoutValueRec));
  Descr=malloc(sizeof(LayoutTextDescriptorRec));
  layout[0].name=TypeOfText;
  layout[0].value=(caddr_t)Descr;
  layout[1].name=0;
  Descr->in=TEXT_VISUAL;
  Descr->out=TEXT_VISUAL;
  layout_object_setvalue(plh,layout,&index);
  free(Descr);
  free(layout);
  set_visual_text();
  for (i=0;i<LINES;i++)
       update_line(i);

}
/*--------------------- Set default BDSeg values ---------------------*/
void set_bdseg_defaults()
{
 int bidi,orient,nulls,text;
 int i;
  LayoutValues layout;
  LayoutTextDescriptor Descr;
 
  /* save some values, because if these change we will need to refresh */
    bidi   = is_bidi_mode();
    text   = is_implicit_text();
    nulls  = is_nulls_mode();
    orient = is_RTL_mode();

    /*  move old values to current seg */
    memcpy(BDCurSeg, BDSegDefaults,BDSEGSIZE);

    /* set new values in layout object */
    SetCurSeg();

    /* check active bidi and active shaping */
    if (!ActiveBidi)
        reset_bidi_mode();
    if (!ActiveShaping)
    {
       set_passthru_csd();
       set_arabic_nss();
    }

    /* check if we need to refresh */
    if ((bidi!= (is_bidi_mode())) || (text!=(is_implicit_text()))
     || (nulls!=(is_nulls_mode())) || (orient!=(is_RTL_mode())))
         for (i=0;i<LINES;i++)
           update_line(i);

}
/*--------------------- Set Screen Direction  ----------------------*/
void do_bidi_SPD()
{
 int i;

 STATE=0;           /* identified sequence, so reset state */
 switch (buffer[buffer_index-5])
 {
  case '0': /* turn screen LTR and set LAtin kbd */
          do_bidi_set_LTR_Latin();
          break;
  case '3': /* turn screen RTL and set national kbd */
          do_bidi_set_RTL_national();
          break;
 }
}
/*--------------------- Set Reversed String  ----------------------*/
void do_bidi_SRS()
{
 STATE=0;           /* identified sequence, so reset state */
 switch (buffer[buffer_index-3])
 {
    case '0': /* end push mode */
           do_bidi_end_push();
           break;
    case '1': /* start push mode */
           do_bidi_start_push();
           break;
 }
}
/*---------------- executing the SAPV options --------------*/
void do_SAPV_option(option,index)
int option;
int index;
{
  int i;

  switch (option)
  {
   case 0 :  /* set default values for national language*/
         set_bdseg_defaults();
         break;
   case 1 :
         if (ActiveShaping)
         do_bidi_set_arabic_nss();
         break;
   case 2 :
         if (ActiveShaping)
         do_bidi_set_hindu_nss();
         break;
   case 3 :
         do_bidi_symmetric_on();
         break;
   case 5 :
         if (is_visual_text() && (ActiveShaping) && (buffer[index+1]==0x32) 
           && (buffer[index+2]==0x31)) /* check that a lock follows */
         do_bidi_set_isolated();
         break;
   case 6 :
         if (is_visual_text() && (ActiveShaping) && (buffer[index+1]==0x32) 
           && (buffer[index+2]==0x31)) /* check that a lock follows */
         do_bidi_set_initial();
         break;
   case 7 :
         if (is_visual_text() && (ActiveShaping) && (buffer[index+1]==0x32) 
           && (buffer[index+2]==0x31)) /* check that a lock follows */
         do_bidi_set_middle();
         break;
   case 8 :
         if (is_visual_text() && (ActiveShaping) && (buffer[index+1]==0x32) 
           && (buffer[index+2]==0x31)) /* check that a lock follows */
         do_bidi_set_final();
         break;
   case 13 :
         do_bidi_set_host_mode();
         break;
   case 14 :
         do_bidi_set_aix_mode();
         break;
   case 15 :
         do_bidi_symmetric_off();
         break;
   case 18:
         do_bidi_set_passthru_csd();
         do_bidi_set_passthru_nss();
         break;
   case 19 :
         do_bidi_set_passthru_csd();
         break;
   case 20 :
         if (is_implicit_text()&&(ActiveShaping)) 
           do_bidi_set_bilingual_nss();
         break;
   case 21 :
         /* already considered in options 5,6,7,8 */ 
         break;
   case 22 :
         if (ActiveShaping)
         do_bidi_set_asd();
         break;
   case 23 :
         if (is_implicit_text()) set_nonulls_mode();
         break;
   case 24 :
         if (is_implicit_text()) set_nulls_mode();
         break;
  }
}
/*---------------- Set Alternate Presentation Variants   --------------*/
void do_bidi_SAPV()
{
 int option;
 int index;
 int second_digit;

 STATE=0;           /* identified sequence, so reset state */
 index = buffer_index-3;
 /* sweep buffer backwards until you find "[", i.e. the start of the sequence */
 while (buffer[index]!=0x5b)
     index--;
 index++;
 option = 0;
 second_digit= FALSE;
 do { 
    if ((buffer[index]==0x3b) ||  /* this is a seperator : ";" or space */
        (buffer[index]==0x20))
    {
     do_SAPV_option(option,index);
     second_digit=FALSE;
     option = 0;
    } 
    else
    {   /* calculate option number */ 
       if (second_digit)
         option = option*10+(buffer[index]-48);
       else
         {
           option = option+(buffer[index]-48);
           second_digit=TRUE;
         }
    }
    index++;
  } while (buffer[index]!=0x5d); /* stop at "]" */
}
/*---------------- in bidi command mode to parse next key --------------*/
void do_bidi_command_mode()
{
 unsigned char key;
 

 key = 0xff;
 while (key==0xff)
  key=get_kb_char(); 
 switch (key)
   {
     case 'r' : if (is_fScrRev_on() && is_bidi_mode())
                  do_bidi_screen_reverse();
                else char_to_hft(0x07);  /* bell */
                break;
     case 'n' : if (is_fRTL_on() )
                  do_bidi_set_national();
                else char_to_hft(0x07);  /* bell */
                break;
     case 'l' : if (is_fLTR_on() )
                  do_bidi_set_Latin();
                else char_to_hft(0x07);  /* bell */
                break;
     case 'p' : if (is_fAutoPush_on() && is_bidi_mode())
                  do_bidi_toggle_autopush();
                else char_to_hft(0x07);  /* bell */
                break;
     case 's' : if (is_fPush_on() && is_bidi_mode())
                  do_bidi_start_push();
                else char_to_hft(0x07);  /* bell */
                break;
     case 'e' : if (is_fEndPush_on() && is_bidi_mode())
                  do_bidi_end_push();
                else char_to_hft(0x07);  /* bell */
                break;
     case 'a' : if (is_fASD_on())
                  do_bidi_toggle_autoshape();
                else char_to_hft(0x07);  /* bell */
                break;
     case 'i' : if (is_fShapeIN_on())
                  do_bidi_set_initial();
                else char_to_hft(0x07);  /* bell */
                break;
     case 'b' : if (is_fShapeP_on())
                  do_bidi_set_base_csd();
                else char_to_hft(0x07);  /* bell */
                break;
     case 'o' : if (is_fShapeIS_on())
                  do_bidi_set_isolated();
                else char_to_hft(0x07);  /* bell */
                break;
     case 'f' : if (is_fShapeF_on())
                  do_bidi_set_final();
                else char_to_hft(0x07);  /* bell */
                break;
     case 'm' : if (is_fShapeM_on())
                  do_bidi_set_middle();
                else char_to_hft(0x07);  /* bell */
                break;
     case 'c' : do_bidi_toggle_column_head();
                break;
     case 't' : do_bidi_put_status();
                break;
     case ' ' : kbd_output(0xa0);  /* RSP */
                break;
     default  : char_to_hft(0x07);  /* unrecognized key , sound bell */
   }
}

