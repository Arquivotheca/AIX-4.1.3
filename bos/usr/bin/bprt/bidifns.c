static char sccsid[] = "@(#)18	1.2  src/bos/usr/bin/bprt/bidifns.c, libbidi, bos411, 9428A410j 11/4/93 15:25:47";
/*
 *   COMPONENT_NAME: LIBBIDI
 *
 *   FUNCTIONS: IsCurrentCodePage
 *		pioflags_value
 *		prn_num
 *		set_new_layout_values
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

#include <stdio.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/mode.h>
#include <sys/stat.h>
#include <locale.h>
#include <langinfo.h>
#include <sys/lc_layout.h>
#include "codeset.h"
#include "bdprtdos.h"


#include "debug.h"
extern unsigned char debug;
int prn_num (char *prn)
{

   if (strncmp (prn, "3812", 4)==0)
     return _3812;
   else 
   if (strncmp (prn, "4019", 4)==0)
     return _4019;
   else
   if (strncmp (prn, "4201", 4)==0)
     return _4201;
   else
   if (strncmp (prn, "4202", 4)==0)
     return _4202;
   else
   if (strncmp (prn, "4207", 4)==0)
     return _4207;
   else
   if (strncmp (prn, "4208", 4)==0)
     return _4208;
   else
   if (strncmp (prn, "4216", 4)==0)
     return _4216;
   else
   if (strncmp (prn, "4234", 4)==0)
     return _4234;
   else
   if (strncmp (prn, "5201", 4)==0)
     return _5201;
   else
   if (strncmp (prn, "5202", 4)==0)
     return _5202;
   else
   if (strncmp (prn, "5204", 4)==0)
     return _5204;
   else
   if (strncmp (prn, "4224", 4)==0)
     return _4224;
   else
     return _4201;
}


char *pioflags_value (char * flags, char flag)
{
char * i;
   for (i=flags; *i != flag; i++)
      ;
   return (i);
}


int IsCurrentCodePage()
{
int value;

        char    *cp;

        cp = nl_langinfo (CODESET);
        if (!cp)
           return(FALSE);

        if (strcmp (cp,"IBM-856") == 0)
            value = BDCP_IBM856;
        else if (strcmp (cp,"IBM-862") == 0)
            value = BDCP_IBM862;
        else if (strcmp (cp,"ISO8859-8") == 0)
            value = BDCP_ISO8859_8;
        else if (strcmp (cp,"ISO8859-6") == 0)
            value = BDCP_ISO8859_6;
        else if (strcmp (cp,"IBM-1046") == 0)
            value = BDCP_IBM1046;

        return (value);
}


/*
*******************************************************************************
*******************************************************************************
** NAME:        set_new_layout_values()
**
** DESCRIPTION: This function is used to set the PRTBIDIATTRs in order for
**              the Arabic String Handling Library to do the correct processing.
**              
**
** ROUTINES
**   CALLED:    _BIDI_SEQ_FOUND() 
**		ProcessorInitialize()
**		PSM_WriteToBuffer()
**
**
** PARAMETERS:  From bidi attributes, To bidi attributes
**
**
*******************************************************************************
*******************************************************************************
*/

set_new_layout_values(layout_value_in, layout_value_out)
unsigned int layout_value_in, layout_value_out;
{
  LayoutValueRec         Layout[2];
  LayoutTextDescriptor   Descrp;
  int RC, index_error=0;
  extern LayoutObject plh;

  /*extern char TempStr[200];
  extern char debug;
  #include "debug.h"*/
  
  Layout[0].name       = AllTextDescptors;
  Descrp               = (LayoutTextDescriptor)malloc(sizeof(LayoutTextDescriptorRec));
  Descrp->in  = layout_value_in & 0xFFFFFFFF;
  Descrp->out = layout_value_out & 0xFFFFFFFF;
  Layout[0].value      = (caddr_t)Descrp;
  Layout[1].name       = 0;
  RC = layout_object_setvalue(plh, Layout, &index_error);

  /* TRACE("/aix3.2/maya/out", "RC from layout_object_setvalue = %x\n", RC); 
  TRACE("/aix3.2/maya/out", "index_error = %d\n", index_error); 
  sprintf(TempStr, "Descrp->in=%x, Descrp->out=%x\n", Descrp->in , Descrp->out);
  write(1, TempStr, strlen(TempStr)); */

  free(Descrp);
}
