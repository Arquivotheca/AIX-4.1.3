static char sccsid[] = "@(#)68  1.3  src/bos/diag/tu/swmono/exectu.c, tu_swmono, bos411, 9428A410j 1/28/94 13:49:11";
/*
 *   COMPONENT_NAME : (tu_swmono) Grayscale Graphics Display Adapter Test Units
 *
 *   FUNCTIONS: exectu
 *		
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include "skytu.h"
#define SKYTYPE struct _skytu 
struct tucb_t
{   long    tu, mfg, loop;
    long    r1, r2;
};
struct _skytu
{   struct  tucb_t   header;
    struct  tu_info  *sky_ptr;
    lword            nio_fdes;
};
struct cntlparm cptr;
/************************************************************
* Name    : exectu()                                       *
* Function: Interface between the Test Application and      *
*           skytu().                                        *
*                                                           *
*                                                           *
*                                                           *
*                                                           *
*                                                           *
************************************************************/
exectu(ldn, tucb_ptr)
char ldn[8];
struct _skytu *tucb_ptr;
{
  word tu_name;
  int rc;

  if (tucb_ptr->header.tu != GET_DEFAULT_PTRX)
  { tucb_ptr->header.tu &= 0xF0FF; /* ensure that the tu is run in control */
    tu_name = tucb_ptr->header.tu;
  }

  /* assign the skyway device file desciptor */
  tinfo.skyway_ldn = ldn;


  /* assign the loop control parameters */
  cptr.break_on_err = YES;
  cptr.verbose      = YES;
  cptr.max_count    = tucb_ptr->header.loop;
  cptr.err_count    = 0;
  CPTR = &cptr;


  rc = skytu(tu_name,  tucb_ptr->sky_ptr);
  return(rc);
}
