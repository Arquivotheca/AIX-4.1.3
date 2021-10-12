/*
 *   COMPONENT_NAME: tu_pcitok
 *
 *   FUNCTIONS: mps_fastwrt
 *		mps_tx_done
 *		set_strings
 *		transmit_data
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/*** header files ***/
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mdio.h>
#include <sys/errno.h>
#include <sys/dma.h>
#include <sys/param.h>

/* local header files */
#include "skytu_type.h"
#include "sky_regs.h"

/**************************************************************************** *
*
* FUNCTION NAME =  transmit_data
*
* DESCRIPTION   =  This function transmits a packet of data
*
* INPUT   =    adapter_info   - ptr to a struct containing diagex info
*              tucb_ptr       - ptr to test unit control block
*
* RETURN-NORMAL =  0
*
* RETURN-ERROR = error code
*
* INVOKED FROM =
*
*****************************************************************************/
int transmit_data(ADAPTER_STRUCT *adapter_info, TUTYPE *tucb_ptr, int ttype)
{
  int         int_level, rc = 0;
  int         pid;

  /*******************************************
   * Identify action in case time out occurs.
   *******************************************/
  adapter_info->wd_setter = WDT_XMIT;

  /*******************************************
   * Start a new process to initiate transmit
   *******************************************/
  mps_fastwrt(adapter_info,tucb_ptr->header.pat_size);
  rc = diag_watch4intr(adapter_info->handle, MISR_INT, TIMEOUT_LIMIT);
  if (rc) {
	rc = process_timeout(adapter_info, tucb_ptr, ttype, MISR_INT);
	if (rc) return(rc);
  }
  else 
  {
  	if (rc = process_interrupts(adapter_info,tucb_ptr,ttype,MISR_INT))
  	return(rc);
  }
  return(0);

} /* end transmit_data */

/****************************************************************************
*
* FUNCTION NAME =  mps_fastwrt
*
* DESCRIPTION   =  Fast write function for kernel.
*
* INPUT   =    adapter_info   - ptr to a struct containing diagex info
*
* RETURN-NORMAL =  0
*
* RETURN-ERROR =   non-zero
*
* INVOKED FROM = transmit_data
*
*****************************************************************************/
int mps_fastwrt (ADAPTER_STRUCT *adapter_info,int wrap_len)
{
  register int        i, x;
  ulong               cdata;
  uint                Lo_Addr, Hi_Addr, Addr;
  int                 rc = 0;
  ushort              tr_header[8];
  ushort	      data_val[2];

  DEBUG_0("Entering mps_fastwrt\n");
  rc = io_rd_swap(adapter_info,IOSHORT,BMCtl_SUM, &data_val[0]);

  /******************************************
    *  Copy test string into transmit buffer
    ******************************************/
  DEBUG_1("Copying test string into buffer, len = %d\n",wrap_len);
  

  /**************************
   *  Build Transmit Header
   **************************/
  tr_header[0] = 0x1040;

  for (i = 1; i < 4; i++) {
	tr_header[i] = adapter_info->dest_address[i-1];
	tr_header[i+3] = adapter_info->source_address[i-1];
  }

  tr_header[7] = 0xaa00;

  /************************************
    *  Copy header into transmit buffer
    ************************************/
  DEBUG_0("Copying header data into buffer\n");
  bcopy(tr_header, adapter_info->transmit_string, 16);

  bcopy(adapter_info->transmit_string, adapter_info->xmit_buf[0],wrap_len);

  /* set up buffer table entry  */
  adapter_info->xmit_list[0]->buf_count    = SWAP16(0x0001);
  adapter_info->xmit_list[0]->frame_len    = SWAP16(wrap_len);
  adapter_info->xmit_list[0]->buf_len      = SWAP16(wrap_len);
  adapter_info->xmit_list[0]->fw_pointer = SWAP32((ulong) 
	adapter_info->xmit_desc_addr[0]);
  adapter_info->xmit_list[0]->data_pointer = SWAP32((ulong)
	adapter_info->xmit_dma_addr[0]);

#ifdef BEST
	DEBUG_1("best orw = %d\n",adapter_info->best_orw);
  	if (adapter_info->best_orw == 1) 
	{
		adapter_info->xmit_list[0]->buf_count = SWAP16(2);
		adapter_info->xmit_list[0]->buf_len = SWAP16(16);
		adapter_info->xmit_list[0]->fw_pointer = SWAP32((ulong)
			adapter_info->xmit_desc_addr[1]);

		adapter_info->xmit_list[1]->buf_count = SWAP16(2);
		adapter_info->xmit_list[1]->frame_len = SWAP16(wrap_len);
		adapter_info->xmit_list[1]->buf_len = SWAP16(wrap_len - 16);
		adapter_info->xmit_list[1]->fw_pointer = SWAP32((ulong)
			adapter_info->xmit_desc_addr[0]);
		adapter_info->xmit_list[1]->data_pointer = 
			SWAP32(adapter_info->buf_addr);
	}
#endif
  DEBUG_2("%08x,data addr = %08x\n",&adapter_info->xmit_list[0]->data_pointer,
			     	     adapter_info->xmit_list[0]->data_pointer);
  DEBUG_2("%08x,fw_pointer = %08x\n",&adapter_info->xmit_list[0]->fw_pointer,
				      adapter_info->xmit_list[0]->fw_pointer);
  DEBUG_2("%08x,buf_count = %08x\n",&adapter_info->xmit_list[0]->buf_count,
				     adapter_info->xmit_list[0]->buf_count);
  DEBUG_2("%08x,frame_len = %08x\n",&adapter_info->xmit_list[0]->frame_len,
				     adapter_info->xmit_list[0]->frame_len);
  DEBUG_2("%08x,buf_len = %08x\n",&adapter_info->xmit_list[0]->buf_len,
				   adapter_info->xmit_list[0]->buf_len);

  DEBUG_1("Transmit len = %d\n", wrap_len);
  DEBUG_0("DUMP LOCAL BUF\n");
#ifdef DEBUG
  hexdump(adapter_info->xmit_buf[0], wrap_len);
  DEBUG_0("DUMP LOCAL TX DESC\n");
  hexdump((char *)&adapter_info->xmit_list[0]->fw_pointer,20);
#endif

  /************************************************
   * Give the buffer descriptor address to adapter
   ************************************************/

  Addr    = (uint)adapter_info->xmit_desc_addr[0];
  rc = io_wr_swap(adapter_info,IOLONG,Tx2FDA_LO, Addr);
  rc = io_wr_swap(adapter_info,IOLONG,Tx2LFDA_LO, Addr);
  sleep(1);
  rc = io_rd_swap(adapter_info,IOSHORT,BMCtl_SUM, &data_val[0]);
  rc = io_rd_swap(adapter_info,IOLONG,Tx2FDA_LO, &data_val[0]);
  rc = io_rd_swap(adapter_info,IOLONG,Tx2LFDA_LO, &data_val[0]);
  
  DEBUG_0("Exiting mps_fastwrt\n");
  DEBUG_0("DUMP LOCAL TX DESC\n");
#ifdef DEBUG
  hexdump((char *)&adapter_info->xmit_list[0]->fw_pointer,20);
#endif
  return(0);

}  /* end mps_fastwrt */
/****************************************************************************
*
* FUNCTION NAME =  mps_tx_done
*
* DESCRIPTION   =  process a completed transmission
*
* INPUT   =    adapter_info   - ptr to a struct containing diagex info
*
* RETURN-NORMAL =  0
*
* RETURN-ERROR =   non-zero
*
* INVOKED FROM = process_misr_cmd
*
*****************************************************************************/
int mps_tx_done (ADAPTER_STRUCT *adapter_info)
{
  ulong               status;
  int                 rc = 0;
  ulong 	      data;


  status = SWAP32(adapter_info->xmit_list[0]->xmit_status);
  (void)io_rd_swap(adapter_info,IOSHORT,0xb8, &data);
  (void)io_rd_swap(adapter_info,IOSHORT,0xba, &data);
  DEBUG_1("TRANSMIT STATUS = 0x%x\n",status);
  DEBUG_0("DUMP LOCAL TX DESC\n");
#ifdef DEBUG
  hexdump((char *)&adapter_info->xmit_list[0]->fw_pointer,20);
#endif
  if (status & 0x78837E00) {
	if (status & 0x00020000) {
		DEBUG_1("mps_tx UNDERRUN. Status = %x\n", status);
		rc = TX_UNDERRUN_ERR;	
	} else if (status & 0x00007E00) {
		DEBUG_1("Tx2 protocol error. Status = %x\n", status);
		rc = TX_PROTOCOL_ERR;	
	} else {
		DEBUG_1("Error in status. Status = %x\n", status);
		rc = TX_DESC_ERR;	
	} 
  }

  return(rc);

} /* mps_tx_done */

/**************************************************************************** *
*
* FUNCTION NAME =  set_strings
*
* DESCRIPTION   =  This function sets the strings for the wrap tests
*
* INPUT   =    adapter_info   - ptr to a struct containing diagex info
*              tucb_ptr       - ptr to test unit control block
*
* RETURN-NORMAL =  0
*
* RETURN-ERROR = error code
*
* INVOKED FROM =
*
*****************************************************************************/
int set_strings(ADAPTER_STRUCT *adapter_info, TUTYPE *tucb_ptr, int ttype)
{
  int         frame_len, i, rc = 0;
  char        *test_string, ch;
  FILE        *fptr;

  /******************************************
   * Check if file specified for test string
   ******************************************/
  if ( (tucb_ptr->header.pattern != NULL) && 
      (strcmp(tucb_ptr->header.pattern, NO_FILE) != 0) ) {
  	if ((fptr = fopen(tucb_ptr->header.pattern, "r")) == NULL) {
  		return (error_handler(tucb_ptr, ttype, FILE_ERR,
			    FILE_ERR, 0, 0, 0));
	}

	DEBUG_0("pattern file open \n");
	/*****************************
         * Check for valid frame size
         *****************************/
	frame_len = tucb_ptr->header.pat_size;
	if ((frame_len < MIN_FRAME_LEN) || (frame_len > MAX_FRAME_LEN)) {
		fclose(fptr);
		return (error_handler(tucb_ptr, ttype, FRAME_LEN_ERR,
			    FRAME_LEN_ERR, 0, 0, 0));
	}

	/*******************
         * Fill test string
         *******************/
	test_string = (char *) malloc(frame_len+1);
	if (test_string == NULL) {
		fclose(fptr);
		return(MEMORY_ERR);
	}

	DEBUG_0("allocated space for test string\n");
	ch = fgetc(fptr);
	for (i = 0; ((i < frame_len) && (ch != EOF)); i++) {
		*(test_string + i) = ch;
		ch = fgetc(fptr);
	} /* end for */
	fclose(fptr);

	/*****************************************************
         * Pad string to specified frame length, if necessary
         *****************************************************/
	for ( ;i < frame_len; i++) {
		*(test_string + i) = 'a';
	}

	DEBUG_0("filled in test string\n");
  } else {/* no pattern file specified, use default string */
	test_string = (char *) malloc(strlen(DEFAULT_TEST_STRING + 1));
	if (test_string == NULL) {
		return(MEMORY_ERR);
	}
	strcpy(test_string, DEFAULT_TEST_STRING);
  }
  test_string[frame_len] = '\0';
  bcopy(test_string, adapter_info->transmit_string, frame_len + 1);

  DEBUG_0("copied test string to transmit string\n");
  /*******************
   * Free test string
   *******************/
  free(test_string) ;
  
  memset(adapter_info->receive_string,0,PAGESIZE);

  return(0);

} /* end set_strings */

/****************************************************************************
 * NAME: hexdump
 *
 * FUNCTION: Display an array of type char in ASCII, and HEX.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This routine is ONLY AVAILABLE IF COMPILED WITH DEBUG DEFINED
 *
 * RETURNS: NONE
 ****************************************************************************/

hexdump(data,len)
char *data;
long len;
{

        int     i,j,k;
        char    str[18];

        fprintf(stdout,"hexdump(): length=%ld: addr=0x%08x\n",len,data);
        i=j=k=0;
        while(i<len)
        {
                j=(int) data[i++];
                if(j>=32 && j<=126)
                        str[k++]=(char) j;
                else
                        str[k++]='.';
                fprintf(stdout,"%02x",j);
                if(!(i%8))
                {
                        fprintf(stdout,"  ");
                        str[k++]=' ';
                }
                if(!(i%16))
                {
                        str[k]='\0';
                        fprintf(stdout,"     %s\n",str);
                        k=0;
                }
        }
        while(i%16)
        {
                if(!(i%8))
                        fprintf(stdout,"  ");
                fprintf(stdout,"   ");
                i++;
        }
        str[k]='\0';
        fprintf(stdout,"       %s\n",str);
}
