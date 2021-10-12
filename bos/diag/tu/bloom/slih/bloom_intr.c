static char sccsid[] = "@(#)46  1.2  src/bos/diag/tu/bloom/slih/bloom_intr.c, tu_bloom, bos41J, 9514A_all 4/3/95 16:44:33";
/*
 *   COMPONENT_NAME: TU_BLOOM
 *
 *   FUNCTIONS: bloom_interrupt
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#ifndef SIXDG
#include <stdio.h>
#include <sys/intr.h>
#include <sys/diagex.h>
#endif
#ifdef SIXDG
#include <fw_api.h>
#include <dgs_api.h>
#endif
#include "bloomtu.h"
#include "ncr8xxreg.h"

#ifndef SIXDG
/******************************************************************************
*
* NAME: print_error
*
* FUNCTION:  prints error information for error debugging at interrupt level
*
* INPUT PARAMETERS:     adapter_info = general card information.
*
* EXECUTION ENVIRONMENT:  Process only.
*
* RETURN VALUE DESCRIPTION: none.
*
* EXTERNAL PROCEDURES CALLED: printf.
*
******************************************************************************/
void print_error(adapter_struc_t *adapter_info, char *error_string)
{
     switch (adapter_info->print) {
        case NO_PRINT:       break;
        case SCREEN_PRINT:   printf("%s",error_string);
                             break;
        case FILE_PRINT:     printf("%s",error_string);
        default:             break;
     }
}

/******************************************************************************
*
* NAME:  read_mem_io
*
* FUNCTION:  This flips the bytes so
*
* INPUT PARAMETERS:     adapter_info
*
* EXECUTION ENVIRONMENT:  Process only.
*
* RETURN VALUE DESCRIPTION: if successful returns address of handle else NULL
*
* EXTERNAL PROCEDURES CALLED:
*
******************************************************************************/
uint read_mem_io(adapter_struc_t *adapter_info,int count,int offset,
                 uchar *data) {
   uint        rc;
   int         i;
   char        msg[256];

   for (count--;count >=0;count--,offset++) {
       if((rc=diag_io_read(adapter_info->handle,IOCHAR,offset,&data[count],
          NULL ,INTRKMEM) != DGX_OK)) {
      sprintf(msg,"memio read error at %x rc %x\n",offset,rc);
      print_error(adapter_info,msg);
      return(BLOOM_IOREGS_E);
      }
   }
   return 0;
}

/******************************************************************************
*
* NAME:  write_mem_io
*
* FUNCTION:  flips the byetes
*
* INPUT PARAMETERS:     adapter_info
*
* EXECUTION ENVIRONMENT:  Process only.
*
* RETURN VALUE DESCRIPTION: if successful returns address of handle else NULL
*
* EXTERNAL PROCEDURES CALLED:
*
******************************************************************************/
uint write_mem_io(adapter_struc_t *adapter_info,int count,int offset,
                  uchar *data) {
   uint        j,rc;
   int         i;
   char        msg[256];

   j=offset;
   for (i=count-1;i>=0;i--,j++){
       if ((rc=diag_io_write(adapter_info->handle,IOCHAR,j,data[i],
                             NULL,INTRKMEM) != DGX_OK)) {
       sprintf(msg,"memio write error at %x rc %x\n",j,rc);
       print_error(adapter_info,msg);
       return(BLOOM_IOREGS_E);
       }
   }
   return 0;
}

/******************************************************************************
*
* NAME:  writec_mem_io
*
* FUNCTION:  flips the byetes
*
* INPUT PARAMETERS:     adapter_info
*
* EXECUTION ENVIRONMENT:  Process only.
*
* RETURN VALUE DESCRIPTION: if successful returns address of handle else NULL
*
* EXTERNAL PROCEDURES CALLED:
*
******************************************************************************/
uint writec_mem_io(adapter_struc_t *adapter_info,int count,int offset,
                  uchar *data) {
   uint        j,rc;
   int         i;
   char        msg[256];
   char        rddata[256];
   

   if(rc=write_mem_io(adapter_info,count,offset,data)) return(rc);
   if((rc=read_mem_io(adapter_info,count,offset,rddata))) {
      return(rc);
   }
   for (i=0;i<count;i++) {
       if ((data[i] != rddata[i])) {
          sprintf(msg,"miscompare memio regs at %x offset %x data %x read %x\n",
                  offset,i,data[i],rddata[i]);
          print_error(adapter_info,msg);
          return(BLOOM_MEMIOMIS_E);
       }
   }
   return 0;
}
#endif

/******************************************************************************
*
* NAME:  bloom_interrupt
*
* FUNCTION:  handles interrupts for bloomer 
*
* INPUT PARAMETERS:     adapter_info and diagex handle
*
* EXECUTION ENVIRONMENT:  Interrupt only.
*
* RETURN VALUE DESCRIPTION: returns info in adapter info
*
* EXTERNAL PROCEDURES CALLED:
*
******************************************************************************/
#ifndef SIXDG
int bloom_interrupt(unsigned long diagex_handle,adapter_struc_t *adapter_info){
#endif
#ifdef SIXDG
#define INTR_SUCC 0
#define INTR_FAIL -1
extern adapter_struc_t *adapter_info;
int bloom_interrupt(void){
#endif
int  rc;
char ucdata;

     adapter_info->intr_ret=read_mem_io(adapter_info,1,ISTAT,&ucdata); 
     if (ucdata) { /* yes it is me clear the INTF */   
        adapter_info->intr_rec=TRUE; 
        adapter_info->intr_ret=writec_mem_io(adapter_info,1,ISTAT,&ucdata); 
        if (ucdata & 0x02) { /* is scsi interrupt */ 
           adapter_info->intr_ret=read_mem_io(adapter_info,1,SIST0,&ucdata); 
/*           usleep(10);  insert 12 clk delay */
           adapter_info->sist0=ucdata; 
           adapter_info->intr_ret=read_mem_io(adapter_info,1,SIST1,&ucdata); 
           adapter_info->sist1=ucdata; 
        }
        if (ucdata & 0x01) { /* is dma interrupt */ 
/*           usleep(10);  insert 12 clk delay */
           adapter_info->intr_ret=read_mem_io(adapter_info,1,DSTAT,&ucdata); 
           adapter_info->dstat=ucdata; 
        }
        return INTR_SUCC;
     } else return INTR_FAIL; 
}
