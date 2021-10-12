static char sccsid[] = "@(#)25  1.5.1.1  src/bos/diag/tu/corv/commands.c, tu_corv, bos41J, 9511A_all 2/28/95 16:40:37";
/*
 *   COMPONENT_NAME: TU_CORV
 *
 *   FUNCTIONS: initialize_device
 *              send_run_self_test_command
 *              run_self_test
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993,1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "corvette.h"
#include "ScsiBld.h"
#include "Adapters.h"

#ifdef DIAGNOSTICS
#include <sys/diagex.h>
#define DIAGEX20 1
#endif

#include "CorvetteLib.h"
#include "libcmdbld.h"
typedef char CORVETTE_STRUCT;
extern CORVETTE_STRUCT corvette_info[8];

/******************************************************************************
*
* NAME:  run_self_test
*
* FUNCTION:  Issues the Run Selected Selft Test command, and handshake the
*            error information out of the system.
*
* INPUT PARAMETERS:     adapter_info = general card information.
*                       command      = command block and length.
*
* EXECUTION ENVIRONMENT:  Process only.
*
* RETURN VALUE DESCRIPTION: if successful returns 0 else error_code.
*
* EXTERNAL PROCEDURES CALLED: set_corvette_command_interface.
*                            set_corvette_attention_register
*
******************************************************************************/
uchar send_run_self_test_command(ADAPTER_STRUCT *adapter, COMMAND command, int device_no)
{

 int i;
 unsigned char c_rc;

 if (!(c_rc = set_corvette_command_interface(adapter,*(long *)command.bytes))) {
   c_rc = set_corvette_attention_register(adapter, 0x01, device_no);
 }

 return (c_rc);
}

/******************************************************************************
*
* NAME:  clear_warm_start
*
* FUNCTION:  Issues the Run Selected Selft Test command, and handshake the
*            error information out of the system.
*
* INPUT PARAMETERS:     adapter_info = general card information.
*                       error_bytes[6] character array pointer.
*
* EXECUTION ENVIRONMENT:  Process only.
*
* RETURN VALUE DESCRIPTION: if successful returns 0 else error_code.
*                           error_bytes array contains handshaked status.
*                           error_bytes = 30 00 1A AF
*                    failing ptc code-----|| || || ||
*                        hi status byte------|| || ||
*                   failing test number code----|| ||
*                immediate command completed-------||
*
* EXTERNAL PROCEDURES CALLED:
*                         send_run_self_test_command()
*          corvette_immediate_run_selected_self_test()
*
******************************************************************************/
int clear_warm_start(ADAPTER_STRUCT *adapter, unsigned char *error_bytes)
{
     ADAPTER_COMMAND command;
     int rc;
     int timeout;
     int i;
     unsigned char c_rc;

 command = corvette_clear_warm_start();

 for (i=0; i<7; i++) corvette_info[i] = 0x00;  /*- clear interrupt buffer -*/
 corvette_info[6] = 0x0f;     /*- flag interrupt handler to force device 15 with EOI -*/

 if (!(c_rc = send_run_self_test_command(adapter,command,_DEVICE_ID_15))) {
  rc = c_rc;

/*timeout = adapter->environment.timeout_value;*/
  timeout = 3000000;
  while ((corvette_info[0]==0) && timeout) {    /*- wait for isr  */
    usleep(10);
    timeout--;
  }
  corvette_info[6] = 0x00; /*- device number 0 in interrupt handler -*/

 }
 else  rc = -1;

 for (i=0; i<8; i++)   /*- extract interrupt handler buffer data into error_bytes -*/
  error_bytes[i] = corvette_info[i];

 return(rc);
}
/******************************************************************************
*
* NAME:  run_self_test
*
* FUNCTION:  Issues the Run Selected Selft Test command, and handshake the
*            error information out of the system.
*
* INPUT PARAMETERS:     adapter_info = general card information.
*                       error_bytes[6] character array pointer.
*
* EXECUTION ENVIRONMENT:  Process only.
*
* RETURN VALUE DESCRIPTION: if successful returns 0 else error_code.
*                           error_bytes array contains handshaked status.
*                           error_bytes = 30 00 1A AF
*                    failing ptc code-----|| || || ||
*                        hi status byte------|| || ||
*                   failing test number code----|| ||
*                immediate command completed-------||
*
* EXTERNAL PROCEDURES CALLED:
*                         send_run_self_test_command()
*          corvette_immediate_run_selected_self_test()
*
******************************************************************************/
int run_self_test(ADAPTER_STRUCT *adapter, unsigned char *error_bytes)
{
     ADAPTER_COMMAND command;
     int rc;
     int timeout;
     int i;
     unsigned char c_rc;

 command = corvette_immediate_run_selected_self_test();

 for (i=0; i<7; i++) corvette_info[i] = 0x00;  /*- clear interrupt buffer -*/
 corvette_info[6] = 0x0f;     /*- flag interrupt handler to force device 15 with EOI -*/
 corvette_info[7] = 0x00;     /*- zero interrupt buffer byte count -*/

 if (!(c_rc = send_run_self_test_command(adapter,command,_DEVICE_ID_15))) {
  rc = c_rc;

/*timeout = adapter->environment.timeout_value;*/
  timeout = 3000000;
  while ((corvette_info[7]<4) && timeout) {    /*- wait for four bytes in interrupt buffer -*/
    usleep(10);
    timeout--;
  }
  corvette_info[6] = 0x00;     /*- turn off force device number 15 in interrupt handler -*/
  if (corvette_info[7] != 4)   /*- flag incorrect byte count as failure -*/
    rc = -1;

 }
 else  rc = -1;

 for (i=0; i<8; i++)   /*- extract interrupt handler buffer data into error_bytes -*/
  error_bytes[i] = corvette_info[i];

 return(rc);
}

int initialize_device(ADAPTER_HANDLE corvette_handle, int device_no)
{

     ADAPTER_COMMAND corvette_command;
     int corvette_response;
     SCSI_COMMAND scsi_command;
     DMA_STRUCT read_buffer,
                TSB;

     read_buffer = dma_allocate(corvette_handle, 1);
     TSB = dma_allocate(corvette_handle, 1);


     /* Issue a request sense for device bringup */
     corvette_command = corvette_request_sense( _AUTO_TSB,
                                                _UNSUPPRESSED_DATA,
                                                _RESERVED,
                                                read_buffer.dma_address,
                                                0x000000ff,
                                                TSB.dma_address,
                                                _SCB_CHAIN_ADDR_0,
                                                _RESERVED);

     corvette_response = send_corvette_command(corvette_handle, corvette_command, device_no);

     /* If Initial request sense failed, then initialize device */
     if (corvette_response == _COMMAND_FAILURE) {
          printf("Spinning up device %d.\n",device_no);

          /* Issue second request sense for device bringup */
          corvette_command = corvette_request_sense( _AUTO_TSB,
                                                     _UNSUPPRESSED_DATA,
                                                     _RESERVED,
                                                     read_buffer.dma_address,
                                                     0x000000ff,
                                                     TSB.dma_address,
                                                     _SCB_CHAIN_ADDR_0,
                                                     _RESERVED);
          send_corvette_command(corvette_handle, corvette_command, device_no);

          /* Start spin-up of device at entity id device_no */
          corvette_command = corvette_spin_up(TSB.dma_address);
          send_corvette_command(corvette_handle, corvette_command, device_no);

          /* Spin-up will fail first time */
          corvette_command = corvette_request_sense( _AUTO_TSB,
                                                     _UNSUPPRESSED_DATA,
                                                     _RESERVED,
                                                     read_buffer.dma_address,
                                                     0x000000ff,
                                                     TSB.dma_address,
                                                     _SCB_CHAIN_ADDR_0,
                                                     _RESERVED);
          send_corvette_command(corvette_handle, corvette_command, device_no);

          /* Re-issue spin up */
          corvette_command = corvette_spin_up(TSB.dma_address);
          corvette_response = send_corvette_command(corvette_handle, corvette_command, device_no);

          if (corvette_response != _SCB_SUCCESS)
               printf("Unable to spin up the device");

     }

     dma_free(corvette_handle, read_buffer);
     dma_free(corvette_handle, TSB);

     return corvette_response;
}
