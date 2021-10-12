static char sccsid[] = "@(#)47	1.11  src/bos/kernext/dlc/qlc/qlcoutputf.c, sysxdlcq, bos411, 9428A410j 5/12/94 12:09:31";
/*
 * COMPONENT_NAME: (SYSXDLCQ) X.25 QLLC module
 *
 * FUNCTIONS: outputf
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/types.h>
#include <sys/lockl.h>

/* defect 130555 */
#include <sys/trchkid.h>
/* end defect 130555 */

#include "qlcg.h"   
#include "qlcq.h"  
#include "qlcqmisc.h"
#include "qlcv.h"  
#include "qlcvfac.h"  
#include "qlcb.h"  
#include "qlcp.h"
#include "qlcc.h"  
#include "qlcs.h"  
#include "qlcl.h"
#include "qlclasyn.h"
#include "qlclutil.h"
#include "qlcltime.h"



extern channel_list_type channel_list;

char numtohex[]="0123456789ABCDEF";
char *buffer=NULL;
char *trace_ptr;
/* can use global function address passed back by dh on open in tx_fn */
void (* output_func)();

void outputf(char *string,...)
{
/* defect 130555 */
  char **p=&string;
  struct {
    byte len; 
    char buffer[128];
    } TRCstr;

  sprintf(TRCstr.buffer,string,p[1],p[2],p[3],p[4],p[5],p[6],p[7],p[8],p[9]);
  TRCstr.len = strlen(TRCstr.buffer);

  TRCGEN(0, HKWD_SYSX_DLC_MONITOR, DLC_QLLC, sizeof(TRCstr), &TRCstr); 
/* end defect 130555 */
}

/*************************************************************************
**  print_stat_block
**
**  debug only function
**  print out contents of status block received from DH
**************************************************************************/
void print_stat_block (
  char *status_block_ptr)

{
  struct status_block *status_block_pt;
  status_block_pt = (struct status_block *)status_block_ptr;

  switch(status_block_pt->code )
  {
  case  CIO_START_DONE :
    outputf ("CIO_START_DONE:\n");
    outputf("option[0]=%x\n",status_block_pt->option[0]);
    outputf("option[1]=%x\n",status_block_pt->option[1]);
    outputf("option[2]=%x\n",status_block_pt->option[2]);
    outputf("option[3]=%x\n",status_block_pt->option[3]);
    print_x25_buffer (status_block_pt->option[2] );
    break ;
  case  CIO_HALT_DONE :
    outputf ("CIO_HALT_DONE:\n");
    outputf("option[0]=%x\n",status_block_pt->option[0]);
    outputf("option[1]=%x\n",status_block_pt->option[1]);
    outputf("option[2]=%x\n",status_block_pt->option[2]);
    outputf("option[3]=%x\n",status_block_pt->option[3]);
    break ;
  case  CIO_TX_DONE :
    outputf ("CIO_TX_DONE:\n");
    outputf("option[0]=%x\n",status_block_pt->option[0]);
    outputf("option[1]=%x\n",status_block_pt->option[1]);
    outputf("option[2]=%x\n",status_block_pt->option[2]);
    outputf("option[3]=%x\n",status_block_pt->option[3]);
    break ;
  case  X25_REJECT_DONE :
    outputf ("X25_REJECT_DONE:\n");
    outputf("option[0]=%x\n",status_block_pt->option[0]);
    outputf("option[1]=%x\n",status_block_pt->option[1]);
    outputf("option[2]=%x\n",status_block_pt->option[2]);
    outputf("option[3]=%x\n",status_block_pt->option[3]);
    break ;
  case  CIO_NULL_BLK :
    outputf ("CIO_NULL_BLK: ie no block received  \n");
    break ;
  default :
    outputf ("unrecognised block received \n");
    break;
  }
  return ;
}

/*************************************************************************
**  print_x25_buffer
**
**  debug only function
**  print out contents of status block received from DH
**************************************************************************/
void  print_x25_buffer (
  char *buf_pt)
{
  gen_buffer_type *buf_ptr;
  int  format=0;
  byte packet_type;
  bool d_bit;
  bool q_bit;
  byte cause;
  byte diagnostic;
  char calling_address[50];
  char called_address[50];
  unsigned short facilities_length;
  unsigned short cud_length;
  char data[4096];
  int i,ii;
  int data_length;
  if (!channel_list.debug_control)
    return;
  buf_ptr = (gen_buffer_type *)buf_pt;
  if (buf_ptr == NULL)
  {
    outputf ("PRINT_X25_BLOCK: buf_ptr = NULL\n ");
    return;
  }
  packet_type = QBM_RETURN_PACKET_TYPE(buf_ptr);
  d_bit = QBM_RETURN_D_BIT(buf_ptr);
  q_bit = QBM_RETURN_Q_BIT(buf_ptr);

  outputf("PRINT_X25_BUFFER: buffer_ptr = %x\n",buf_ptr);
  switch(packet_type)
  {
  case PKT_CALL_REQ:
    outputf("PKT_CALL_REQ\n");
    format=1;
    break;
  case PKT_CLEAR_REQ:
    outputf("PKT_CLEAR_REQ\n");
    format=2;
    break;
  case PKT_CLEAR_IND:
    outputf("PKT_CLEAR_IND\n");
    format=2;
    break;
  case PKT_CLEAR_CONFIRM:
    outputf("PKT_CLEAR_CONFIRM\n");
    format=1;
    break;
  case PKT_MONITOR:
    outputf("PKT_MONITOR\n");
    format=3;
    break;
  case PKT_INCOMING_CALL:
    outputf("PKT_INCOMING_CALL\n");
    format=1;
    break;
  case PKT_CALL_ACCEPT:
    outputf("PKT_CALL_ACCEPT\n");
    format=1;
    break;
  case PKT_CALL_CONNECTED:
    outputf("PKT_CALL_CONNECTED\n");
    format=1;
    break;
  case PKT_RESET_REQ:
    outputf("PKT_RESET_REQ\n");
    format=4;
    break;
  case PKT_RESET_IND:
    outputf("PKT_RESET_IND\n");
    format=4;
    break;
  case PKT_RESET_CONFIRM:
    outputf("PKT_RESET_CONFIRM\n");
    format=5;
    break;
  case PKT_INT_CONFIRM:
    outputf("PKT_INT_CONFIRM\n");
    format=5;
    break;
  case PKT_RR:
    outputf("PKT_RR\n");
    format=5;
    break;
  case PKT_INT:
    outputf("PKT_INT\n");
    format=3;
    break;
  case PKT_DATA:
    outputf("PKT_DATA\n");
    format=3;
    break;
  default:
    outputf("Unknown pkt type %d\n",packet_type);
    format=99;
    break;
  }

  switch(format)
  {
  case 2:
    cause = QBM_RETURN_CAUSE(buf_ptr);
    diagnostic = QBM_RETURN_DIAGNOSTIC(buf_ptr);
    outputf("Cause:          0x%x\n",cause);
    outputf("Diagnostic:     0x%x\n",diagnostic);
  case 1:
    QBM_RETURN_CALLING_ADDRESS(buf_ptr,calling_address);
    QBM_RETURN_CALLED_ADDRESS(buf_ptr,called_address);
    outputf("Calling Address [%s]\n",calling_address);
    outputf("Called Address  [%s]\n",called_address);
    facilities_length = QBM_RETURN_FACILITIES_LENGTH(buf_ptr);
    if (facilities_length != 0)
      outputf("Facilities exist in buffer. Length = %d\n",facilities_length);
    cud_length = QBM_RETURN_CUD_LENGTH(buf_ptr);
    if (cud_length!=0)
    {
      outputf("Call User Data in buffer. Length = %d\n",cud_length);
      for (ii=0;ii<cud_length;ii++) /* Defect 110313 */
          {
          outputf("%x\n",QBM_RETURN_CUD1(buf_ptr,ii));
          }
    }
    break;
  case 3:
    data_length=JSMBUF_LENGTH(buf_ptr)-X25_OFFSETOF_USER_DATA;
    outputf(" data_length = %d\n",data_length);
    outputf("Q bit is %d\n",q_bit);
    outputf("D bit is %d\n",d_bit);
    if (data_length!=0)
    {
      data_length=MIN(32,data_length);
      QBM_RETURN_BLOCK(
	buf_ptr,
	X25_OFFSETOF_USER_DATA,
	data,
	data_length);
      hex_data_list(data,data_length);
    }
    break;
  case 4:
    cause = QBM_RETURN_CAUSE(buf_ptr);
    diagnostic = QBM_RETURN_DIAGNOSTIC(buf_ptr);
    outputf("Cause:          0x%x\n",cause);
    outputf("Diagnostic:     0x%x\n",diagnostic);
    break;
  case 5:
    break;
  }
  return ;
}


void output_two_hex_digits(int count)
{
  outputf("%c%c",numtohex[count>>4],numtohex[count&0xF]);
}

/*****************************************************************************/
/* Function HEX DATA LIST                                                    */
/*****************************************************************************/
void  hex_data_list(
  char *data,
  int count)
{
  char c;
  char string[20];
  int i,j;

  string[0]= '\0';
  string[16]=0;

  if (count==0)
    return;

  j=0;i=16;
  for(j=0;j <count;j++)
  {
    c = *data++;
    if ((i==2) || (i==6) || (i==10) || i==14)
      outputf(".");
    if ((i==4) || (i==12))
      outputf("..");
    if (i==8)
      outputf("...");
    if (i==16)
    {
      if (j!=0) outputf(" [%s]",string);
      string[0]= '\0';
      outputf("\n");
      output_two_hex_digits(j>>16);
      output_two_hex_digits(j&0xFFFF);
      outputf("=>");
      i=0;
    }
    output_two_hex_digits(c);

    if(c > 0x1f && c < 0x7f )
      string[i]=c;
    else
      string[i]='.';
    i++;
  }

  for (;i<16;++i)
  {
    if ((i==2) || (i==6) || (i==10) || i==14)
      outputf(".");
    if ((i==4) || (i==12))
      outputf("..");
    if (i==8)
      outputf("...");
    outputf("**");
    string[i]='*';
  }
  string[i]='\0';
  outputf(" [%s]\n",string);
}

/*************************************************************************
**  print_start_data
**
**  debug only function
**  print out contents of devioctl start_data block received from DH
**************************************************************************/
void print_start_data (
  char *start_data_pt)

{
  struct x25_start_data *start_data;
  start_data = (struct x25_start_data *)start_data_pt;

  outputf("start_data:\n");
  outputf("  sb.status        = %d\n", start_data->sb.status);
  outputf("  sb.length        = %d\n", start_data->sb.length);
  outputf("  sb.netid         = %d\n", start_data->sb.netid);
  outputf("  session_name     = %s\n", start_data->session_name);
  outputf("  session_id       = %d\n", start_data->session_id);
  outputf("  session_type     = %d\n", start_data->session_type);
  outputf("  session_protocol = %d\n", start_data->session_protocol);
  outputf("  counter_id       = %d\n", start_data->counter_id);
  return;
}

/*************************************************************************
**  print_halt_data
**
**  debug only function
**  print out contents of devioctl start_halt block sent to DH
**************************************************************************/
void print_halt_data (
  char *halt_data_pt)

{
  struct x25_halt_data *halt_data;
  halt_data = (struct x25_halt_data *)halt_data_pt;

  outputf("halt_data:\n");
  outputf("  sb.status        = %d\n", halt_data->sb.status);
  outputf("  sb.length        = %d\n", halt_data->sb.length);
  outputf("  sb.netid         = %d\n", halt_data->sb.netid);
  outputf("  session_id       = %d\n", halt_data->session_id);
  return;
}


/*************************************************************************
**  print_write_ext
**
**  debug only function
**************************************************************************/
void print_write_ext (
  char *write_ext_pt)
{
  struct x25_write_ext *write_ext;
  write_ext = (struct x25_write_ext *)write_ext_pt;

  outputf("write_ext:\n");
  outputf("  we.flag     = %x\n", write_ext->we.flag);
  outputf("  netid       = %d\n", write_ext->we.netid);
  outputf("  session_id  = %d\n", write_ext->session_id);
  return;
}

/*****************************************************************************/
/* State Check for debug purposes                                            */
/*****************************************************************************/
void qllc_state(void)
{
  channel_type  *channel_ptr;
  sap_type      *sap_ptr;
  station_type  *station_ptr;
  port_type     *port_ptr;

  outputf(" Internal QLLC state:\n");
  outputf(" Channel_list: lock        = %x\n",channel_list.lock);
  outputf(" Channel_list: channel_ptr = %x\n",channel_list.channel_ptr);
  outputf(" Channel_list: port_ptr    = %x\n",channel_list.port_ptr);

  channel_ptr = channel_list.channel_ptr;
  port_ptr = channel_list.port_ptr;
  if (channel_ptr != NULL)
  {
    outputf(" First channel: \n");
    outputf("    lock=%x\n",channel_ptr->lock);
    sap_ptr = channel_ptr->sap_list_ptr;
    if (sap_ptr != NULL)
    {
      outputf(" SAP enabled at %x\n",sap_ptr);
      outputf(" SAP state: \n");
      outputf("    lock = %x\n",sap_ptr->lock);
      station_ptr = sap_ptr->station_list_ptr;
      if (station_ptr != NULL)
      {
	outputf(" Station started at %x\n",station_ptr);
	outputf("   lock = %x\n", station_ptr->lock);
      }
      else
      {
	outputf(" No stations\n");
      }
    }
    else
    {
      outputf(" No SAPs.\n");
    }
  }
  else
  {
    outputf(" No channels are open\n");
  }
  if (port_ptr != NULL)
  {
    outputf(" Port open at %x\n",port_ptr);
    outputf("    lock   =%x\n",port_ptr->lock);
    outputf("    user_count=%d\n",port_ptr->user_count);
  }
  else
  {
    outputf(" No ports are open\n");
  }
}

/*****************************************************************************/
/* Print CB_FAC_T                                                            */
/*****************************************************************************/
void  print_cb_fac(char *fac_ptr)
{
  cb_fac_t *fac;
  fac = (cb_fac_t *)fac_ptr;

  outputf("Contents of cb_fac_t FACILITIES structure:\n");
  outputf("flags        = 0x%x\n",fac->flags);
  outputf("fac_ext_len  = %d\n",fac->fac_ext_len);
  outputf("fac_ext      = 0x%x\n",fac->fac_ext);
  outputf("psiz_clg     = %d\n",fac->psiz_clg);
  outputf("psiz_cld     = %d\n",fac->psiz_cld);
  outputf("wsiz_clg     = %d\n",fac->wsiz_clg);
  outputf("wsiz_cld     = %d\n",fac->wsiz_cld);
  outputf("tcls_clg     = %d\n",fac->tcls_clg);
  outputf("tcls_cld     = %d\n",fac->tcls_cld);
  outputf("rpoa_id_len  = %d\n",fac->rpoa_id_len);
  outputf("rpoa_id      = %s\n",fac->rpoa_id);
  outputf("cug_id       = %d\n",fac->cug_id);
  outputf("nui_data_len = %d\n",fac->nui_data_len);
  if (fac->nui_data_len > 0)
    outputf("nui_data       = %s\n",fac->nui_data);
  outputf("ci_seg_cnt_len = %d\n",fac->ci_seg_cnt_len);
  if (fac->ci_seg_cnt_len > 0)
    outputf("ci_seg_cnt = NOT PRINTED\n");
  outputf("ci_mon_unt_len = %d\n",fac->ci_mon_unt_len);
  if (fac->ci_mon_unt_len > 0)
    outputf("ci_mon_unt = NOT PRINTED\n");
  outputf("ci_call_dur_len = %d\n",fac->ci_call_dur_len);
  if (fac->ci_call_dur_len > 0)
    outputf("ci_call_dur = NOT PRINTED\n");
/*
  outputf("call_redr_addr = %s\n",fac->call_redr_addr);
  outputf("call_redr_reason = %d\n",fac->call_redr_reason);
  outputf("could print others......\n");
*/
  return;
}

/*****************************************************************************/
/* Print sna_fac                                                             */
/*****************************************************************************/
void print_sna_fac(char *fac_ptr)

{
  int i;
  struct sna_facilities_type *fac;

  fac = (struct sna_facilities_type *)fac_ptr;

  outputf("SNA Facilities:\n");
  outputf("facs = %d\n",fac->facs);
  outputf("revc = %d\n",fac->revc);
  outputf("rpoa = %d\n",fac->rpoa);
  outputf("cug = %d\n",fac->cug);
  outputf("cugo = %d\n",fac->cugo);
  outputf("nui = %d\n",fac->nui);
  outputf("wsiz = %d\n",fac->wsiz);
  outputf("psiz = %d\n",fac->psiz);
  outputf("tcls = %d\n",fac->tcls);
  /***********************************************/
  /* the following encoding scheme is used for   */
  /* specifying packet sizes:                    */
  /*      0x06 = 64 octets                       */
  /*      0x07 = 128 octets                      */
  /*      0x08 = 256 octets                      */
  /*      0x09 = 512 octets                      */
  /*      0x0A = 1024 octets                     */
  /*      0x0B = 2048 octets                     */
  /*      0x0C = 4096 octets                     */
  /***********************************************/
  outputf("recipient_tx_psiz = %d\n",fac->recipient_tx_psiz);
  outputf("originator_tx_psiz = %d\n",fac->originator_tx_psiz);
  outputf("recipient_tx_wsiz = %d\n",fac->recipient_tx_wsiz);
  outputf("originator_tx_wsiz = %d\n",fac->originator_tx_wsiz);
  /***********************************************/
  /* the following encoding scheme is used for   */
  /* specifying throughput classes:              */
  /*      0x07 = 1200 bits/s.                    */
  /*      0x08 = 2400 bits/s.                    */
  /*      0x09 = 4800 bits/s.                    */
  /*      0x0A = 9600 bits/s.                    */
  /*      0x0B = 19200 bits/s.                   */
  /*      0x0C = 48000 bits/s.                   */
  /***********************************************/
  outputf("recipient_tx_tcls = %d\n",fac->recipient_tx_tcls);
  outputf("originator_tx_tcls = %d\n",fac->originator_tx_tcls);
  /**********************************************************/
  /* CUG index is four ASCII decimal digits specifying the  */
  /* closed user group selected for this call               */
  /* IDs less than four digits should have zeros            */
  /* prefixed to start                                      */
  /**********************************************************/
  outputf("cug_index = %d\n",fac->cug_index);
  outputf("rpoa_id_count = %d\n",fac->rpoa_id_count);
  for (i=0; i<fac->rpoa_id_count; i++)
    outputf("rpoa_id[%d] = %d\n",i,fac->rpoa_id[i]);
  outputf("nui_length = %d\n",fac->nui_length);
  outputf("nui_data = %s\n",fac->nui_data);
  return;
}
