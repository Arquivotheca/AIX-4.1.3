static char sccsid[] = "@(#)28  1.9  src/bos/diag/tu/corv/corvette.c, tu_corv, bos41J, 9516B_all 4/20/95 07:43:18";
/*
 *   COMPONENT_NAME: TU_CORV
 *
 *   FUNCTIONS: get_corvette_basic_status_register
 *              get_corvette_command_interface
 *              initialize_corvette
 *              send_corvette_command
 *              send_corvette_immediate_command
 *              set_corvette_CIR
 *              set_corvette_access_control
 *              set_corvette_attention_register
 *              set_corvette_basic_register
 *              set_corvette_command_interface
 *              set_corvette_dma_control
 *              set_corvette_io_control
 *              set_corvette_master_control
 *              set_corvette_rom_register
 *              set_corvette_scsi_control
 *              set_corvette_subaddress_control
 *              set_corvette_uchannel_control
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include <sys/errno.h>
#include <unistd.h>
#include "libadapters.h"
#include "getcorvette.h"

#ifdef DIAGEX10
#include "diagex.h"
#elif DIAGEX20
#include "diagex20.h"
#elif DIAGNOSTICS
#include <sys/diagex.h>
#define DIAGEX20 1
#endif

#include "CorvetteLib.h"
#include "libcmdbld.h"

int correlation_id = 0;
CORVETTE_STRUCT corvette_info[8];

/******************************************************************************
*
* NAME:  set_corvette_access_control
*
* FUNCTION:  Enables Corvette, sets I/O Address range, and sets ROM BIOS
*            address select bits.
*
* INPUT PARAMETERS:     adapter_info = general card information.
*                       base_io_range = defines lower two bytes of io address
*                       rom_bios_setting =  defines base ROM address
*
* EXECUTION ENVIRONMENT:  Process only.
*
* RETURN VALUE DESCRIPTION: if successful returns 0 else error_code.
*
* EXTERNAL PROCEDURES CALLED: diag_pos_write
*
******************************************************************************/
unsigned char set_corvette_access_control(ADAPTER_STRUCT *adapter, int upper_io_range, int rom_bios_setting)
{
     int access_control;
     int io_bit_value;

     access_control = CEN;
     switch (upper_io_range) {
        case 0x3540:    io_bit_value = 0x00;
                        break;
        case 0x3548:    io_bit_value = 0x01;
                        break;
        case 0x3550:    io_bit_value = 0x02;
                        break;
        case 0x3558:    io_bit_value = 0x03;
                        break;
        case 0x3560:    io_bit_value = 0x04;
                        break;
        case 0x3568:    io_bit_value = 0x05;
                        break;
        case 0x3570:    io_bit_value = 0x06;
                        break;
        case 0x3578:    io_bit_value = 0x07;
                        break;
        default:        return(-1);
                        break;
     }

     access_control |= (io_bit_value << 1);
     access_control |= (rom_bios_setting << 4);

#ifdef DIAGEX10
     diag_pos_write(adapter->handle, 2, access_control);
#elif DIAGEX20
     diag_pos_write(adapter->handle, 2, access_control, NULL, PROCLEV);
#endif

     return errno;

}

/******************************************************************************
*
* NAME:  set_corvette_dma_control
*
* FUNCTION:  Sets arbitration level and adapter SCSI address.
*
* INPUT PARAMETERS:     adapter_info = general card information.
*                       dma_level = defines uchannel arbitration level.
*                       scsi_id =  defines internal SCSI Id.
*
* EXECUTION ENVIRONMENT:  Process only.
*
* RETURN VALUE DESCRIPTION: if successful returns 0 else error_code.
*
* EXTERNAL PROCEDURES CALLED: diag_pos_write
*
******************************************************************************/
unsigned char set_corvette_dma_control(ADAPTER_STRUCT *adapter, int dma_level,
                                            int internal_scsi_id)
{
     int dma_control;

     dma_control = dma_level;
     dma_control |= (internal_scsi_id << 5);

#ifdef DIAGEX10
     diag_pos_write(adapter->handle, 3, dma_control);
#elif DIAGEX20
     diag_pos_write(adapter->handle, 3, dma_control, NULL, PROCLEV);
#endif

     return errno;

}

/******************************************************************************
*
* NAME:  set_corvette_io_control
*
* FUNCTION:  Sets ROM BIOS Enable, Move Mode Enable, and the External SCSI Id.
*
* INPUT PARAMETERS:     adapter_info = general card information.
*                       interrupt_level = interrupt value of adapter.
*                       rom_bios_enable = enables access to ROM BIOS.
*                       external_scsi_id =  defines adapter external SCSI Id.
*
* EXECUTION ENVIRONMENT:  Process only.
*
* RETURN VALUE DESCRIPTION: if successful returns 0 else error_code.
*
* EXTERNAL PROCEDURES CALLED: diag_pos_write
*
******************************************************************************/
unsigned char set_corvette_io_control(ADAPTER_STRUCT *adapter, int interrupt_level,
                                        int rom_bios_enable, int move_mode_enable,
                                        int external_scsi_id)
{
     int io_control;

     io_control = interrupt_level&0x1;
     io_control |= (rom_bios_enable << 1);
     io_control |= (move_mode_enable << 2);
     io_control |= (external_scsi_id << 3);

#ifdef DIAGEX10
     diag_pos_write(adapter->handle, 4, io_control);
#elif DIAGEX20
     diag_pos_write(adapter->handle, 4, io_control, NULL, PROCLEV);
#endif

     return errno;

}

/******************************************************************************
*
* NAME:  set_corvette_master_control
*
* FUNCTION:  Sets Streaming Enable, Slave Return Checking, Data Parity Enable,
*            Wait State Enable, IO Select Mode, and IOCHCK Status.
*
* INPUT PARAMETERS:     adapter_info = general card information.
*                       streaming_enable = enable streaming data transfers.
*                       slave_return_check =  enable -SFDBKRTN sampling for
*                                             acknowledgement from slave.
*                       parity_enable = enable data parity checking
*                       wait_state_enable = forces minimum of one wait state
*                                           for all bus master bus cycles.
*                       io_select = validates IO bits in POS register 3B.
*                       iochck_status = reflects parity status and resets
*                                       the -IOCHCK signal.
*
* EXECUTION ENVIRONMENT:  Process only.
*
* RETURN VALUE DESCRIPTION: if successful returns 0 else error_code.
*
* EXTERNAL PROCEDURES CALLED: diag_pos_write
*
******************************************************************************/
unsigned char set_corvette_master_control(ADAPTER_STRUCT *adapter, int streaming_enable,
                                          int slave_return_check, int parity_enable,
                                          int wait_state_enable, int io_select)
{
     int master_control;

     master_control = streaming_enable;
     master_control |= (slave_return_check << 1);
     master_control |= (parity_enable << 2);
     master_control |= (wait_state_enable << 3);
     master_control |= (io_select << 5);

#ifdef DIAGEX10
     diag_pos_write(adapter->handle, 5, master_control);
#elif DIAGEX20
     diag_pos_write(adapter->handle, 5, master_control, NULL, PROCLEV);
#endif
     return errno;

}

/******************************************************************************
*
* NAME:  set_corvette_subaddress_control
*
* FUNCTION:  Enables access to POS registers 3B and 4B.
*
* INPUT PARAMETERS:     adapter_info = general card information.
*                       subaddress_enable = enable POS registers 3B and 4B.
*
* EXECUTION ENVIRONMENT:  Process only.
*
* RETURN VALUE DESCRIPTION: if successful returns 0 else error_code.
*
* EXTERNAL PROCEDURES CALLED: diag_pos_write
*
******************************************************************************/
unsigned char set_corvette_subaddress_control(ADAPTER_STRUCT *adapter, int subaddress_enable)
{

#ifdef DIAGEX10
     diag_pos_write(adapter->handle, 6, subaddress_enable);
#elif DIAGEX20
     diag_pos_write(adapter->handle, 6, subaddress_enable, NULL, PROCLEV);
#endif
     return errno;

}

/******************************************************************************
*
* NAME:  set_corvette_uchannel_control
*
* FUNCTION:  Sets address burst management, time release control, and
*            IO address bits.
*
* INPUT PARAMETERS:     adapter_info = general card information.
*                       burst_boundary = defines address boundaries used
*                                        to control bursting of data.
*                       time_release   = delay for release of uchannel bus
*                                        following the assertion of -PREEMPT.
*                       lower_io_range = defines bits 3 through 5 of the
*                                        base IO address.
*
* EXECUTION ENVIRONMENT:  Process only.
*
* RETURN VALUE DESCRIPTION: if successful returns 0 else error_code.
*
* EXTERNAL PROCEDURES CALLED: diag_pos_write, set_corvette_master_control
*
******************************************************************************/
unsigned char set_corvette_uchannel_control(ADAPTER_STRUCT *adapter, int burst_boundary,
                                       int time_release, int lower_io_range)
{
     int dma_control;

     /* initialize error recorvery */
     errno = 0;

     /* Change to alternate POS register set */
     set_corvette_subaddress_control(adapter, _SET_B);

     /* Check for error during master_control */
     if (errno)
          return errno;

     /* Assign POS Register 3B (DMA Control) */
     dma_control = burst_boundary;
     dma_control |= (time_release << 2);
     dma_control |= (lower_io_range << 4);

#ifdef DIAGEX10
     diag_pos_write(adapter->handle, 3, dma_control);
#elif DIAGEX20
     diag_pos_write(adapter->handle, 3, dma_control, NULL, PROCLEV);
#endif

     /* Check for error during master_control */
     if (errno)
          return errno;

     /* Change to initial POS register set */
     set_corvette_subaddress_control(adapter, _SET_A);

     return errno;

}


/******************************************************************************
*
* NAME:  set_corvette_scsi_control
*
* FUNCTION:  Enables external fast SCSI, disables compatibility mode, enables
*            SCSI posted writes, enables external terminator, disables
*            SCSI disconnect, and disables SCSI target mode.
*
* INPUT PARAMETERS:     adapter_info = general card information.
*                       external_fast = enables greater than 5MHz data
*                                       transfer on external SCSI.
*                       compatibility = treats the internal and external
*                                       SCSI buses as a single bus.
*                       posted_writes = allows adapter to reply with command
*                                       complete before device completes
*                                       command.
*                       external_terminator = enables on-card bus termination
*                                             for external SCSI bus.
*                       disconnect_disable = disable device disconnect.
*                       target_mode = disables the adapter to respond to
*                                     other initiators.
*
* EXECUTION ENVIRONMENT:  Process only.
*
* RETURN VALUE DESCRIPTION: if successful returns 0 else error_code.
*
* EXTERNAL PROCEDURES CALLED: diag_pos_write, set_corvette_subaddress_control
*
******************************************************************************/
unsigned char set_corvette_scsi_control(ADAPTER_STRUCT *adapter, int external_fast,
                                        int compatibility, int posted_writes,
                                        int external_terminator, int disconnect_disable,
                                        int target_mode)
{
     int scsi_control;

     /* initialize error recorvery */
     errno = 0;

     /* Change to alternate POS register set */
     set_corvette_subaddress_control(adapter, _SET_B);

     /* Check for error during subaddress_control */
     if (errno)
          return errno;

     /* Assign POS Register 4B (DMA Control) */
     scsi_control = (external_fast <<2);
     scsi_control |= (compatibility << 3);
     scsi_control |= (posted_writes << 4);
     scsi_control |= (external_terminator << 5);
     scsi_control |= (disconnect_disable << 6);
     scsi_control |= (target_mode << 7);

#ifdef DIAGEX10
     diag_pos_write(adapter->handle, 4, scsi_control);
#elif DIAGEX20
     diag_pos_write(adapter->handle, 4, scsi_control, NULL, PROCLEV);
#endif

     /* Check for error during diag_pos_write */
     if (errno)
          return errno;

     /* Change to initial POS register set */
     set_corvette_subaddress_control(adapter, _SET_A);

     return errno;

}

/******************************************************************************
*
* NAME:  set_corvette_command_interface_reg
*
* FUNCTION:  Provides a byte interface to the Command Interface Register.
*
*
* INPUT PARAMETERS:     adapter_info = general card information.
*                       CIR_byte = value to be written to the Command
*                                       interface register.
*                       CIR_pos = CIR byte position to which the CIR_byte
*                                       should be written.
*
*
* EXECUTION ENVIRONMENT:  Process only.
*
* RETURN VALUE DESCRIPTION: if successful returns 0 else error_code.
*
* EXTERNAL PROCEDURES CALLED: diag_io_write
*
******************************************************************************/
unsigned char set_corvette_CIR(ADAPTER_STRUCT *adapter, unsigned char CIR_byte, int CIR_pos)
{

#ifdef DIAGEX10
     diag_io_write(adapter->handle, IOCHAR, CIR_pos, CIR_byte);
#elif DIAGEX20
     diag_io_write(adapter->handle, IOCHAR, CIR_pos, CIR_byte, NULL, PROCLEV);
#endif
     return errno;


}

/******************************************************************************
*
* NAME:  set_corvette_command_interface
*
* FUNCTION:  In move mode, defines the location of the management request
*            record.
*
* INPUT PARAMETERS:     adapter_info = general card information.
*                       request_record = address of the management request
*                                        record.
*
* EXECUTION ENVIRONMENT:  Process only.
*
* RETURN VALUE DESCRIPTION: if successful returns 0 else error_code.
*
* EXTERNAL PROCEDURES CALLED: diag_io_write diag_io_read
*
******************************************************************************/
unsigned char set_corvette_command_interface(ADAPTER_STRUCT *adapter, unsigned long request)
{
     int loop;
     char *request_record;
     unsigned char bus_status_register;

     request_record = (char *)malloc(4);
     request_record = (char *)&request;

  /*- chk/wait for not busy -*/
  diag_io_read(adapter->handle, IOCHAR, 7, &bus_status_register, NULL, PROCLEV);
  while((bus_status_register & 0x01)) {
   print_error(adapter,"Adapter Busy set cir! \n");
   sleep(1);
   diag_io_read(adapter->handle, IOCHAR, 7, &bus_status_register, NULL, PROCLEV);
  }

     for (loop=0; loop<4; loop++)
#ifdef DIAGEX10
         diag_io_write(adapter->handle, IOCHAR, loop, request_record[loop]);
#elif DIAGEX20
         diag_io_write(adapter->handle, IOCHAR, loop, request_record[loop], NULL, PROCLEV);
#endif

     return errno;

}

/******************************************************************************
*
* NAME:  set_corvette_attention_register
*
* FUNCTION:  Requests attention of the adapter to perform some function for
*            the system.
*
* INPUT PARAMETERS:     adapter_info = general card information.
*                       request_code = specifies action to be taken.
*                       unit_number  = the device to which the request
*                                      code is directed.
*
* EXECUTION ENVIRONMENT:  Process only.
*
* RETURN VALUE DESCRIPTION: if successful returns 0 else error_code.
*
* EXTERNAL PROCEDURES CALLED: diag_io_write
*
******************************************************************************/
unsigned char set_corvette_attention_register(ADAPTER_STRUCT *adapter, unsigned long request_code,
                                              int unit_number)
{
     int attention_register;
     unsigned char bus_status_register;

     attention_register = unit_number;
     attention_register |= (request_code << 4);

#ifdef DIAGEX10
     while(diag_io_read(adapter->handle, IOCHAR, 7) & 0x01) {
#elif DIAGEX20
     diag_io_read(adapter->handle, IOCHAR, 7, &bus_status_register, NULL, PROCLEV);

     while((bus_status_register & 0x01)) {
#endif
          print_error(adapter,"Adapter is Busy! set attn reg\n");
          sleep(1);
#ifdef DIAGEX20
          diag_io_read(adapter->handle, IOCHAR, 7, &bus_status_register, NULL, PROCLEV);
#endif
     }

#ifdef DIAGEX10
     diag_io_write(adapter->handle, IOCHAR, 4, attention_register);
#elif DIAGEX20
         diag_io_write(adapter->handle, IOCHAR, 4, attention_register, NULL, PROCLEV);
#endif
     return errno;

}
/******************************************************************************
*
* NAME:  set_corvette_basic_register
*
* FUNCTION:  Enables system interrupt, enables bus master DMAs, resets the
*            exception condition, enables clearing of interrupt, enables
*            subsystem reset.
*
* INPUT PARAMETERS:     adapter_info = general card information.
*                       system_interrupt = enables system interrupts.
*                       bus_master_dma = enables arbitration for bus control,
*                                        and allows data transfers.
*                       reset_exception = resets the the Exception Condition
*                                         bit in the Basic Status Register.
*                       clear_on_read = enables reset of uchannel interrupt if
*                                       the Basic Status Register is read.
*                       subsystem_reset = resets the adapter.
*
* EXECUTION ENVIRONMENT:  Process only.
*
* RETURN VALUE DESCRIPTION: if successful returns 0 else error_code.
*
* EXTERNAL PROCEDURES CALLED: diag_io_write
*
******************************************************************************/
uchar set_corvette_basic_register(ADAPTER_STRUCT *adapter, int system_interrupt,
                                      int bus_master_dma, int reset_exception,
                                      int clear_on_read, int subsystem_reset)
{
     int basic_register;

     basic_register = system_interrupt;
     basic_register |= (bus_master_dma << 1);
     basic_register |= (reset_exception << 5);
     basic_register |= (clear_on_read << 6);
     basic_register |= (subsystem_reset << 7);

#ifdef DIAGEX10
     diag_io_write(adapter->handle, IOCHAR, 5, basic_register);
#elif DIAGEX20
         diag_io_write(adapter->handle, IOCHAR, 5, basic_register, NULL, PROCLEV);
#endif
     return errno;

}

/******************************************************************************
*
* NAME:  set_corvette_rom_register
*
* FUNCTION:  Selects the active 16KB bank of ROM BIOS.
*
* INPUT PARAMETERS:     adapter_info = general card information.
*                       rom_page = page to become active.
*
* EXECUTION ENVIRONMENT:  Process only.
*
* RETURN VALUE DESCRIPTION: if successful returns 0 else error_code.
*
* EXTERNAL PROCEDURES CALLED: diag_io_write
*
******************************************************************************/
uchar set_corvette_rom_register(ADAPTER_STRUCT *adapter, int rom_page)
{

#ifdef DIAGEX10
     diag_io_write(adapter->handle, IOCHAR, 6, _SELECT_ROM_PAGE | rom_page);
#elif DIAGEX20
     diag_io_write(adapter->handle, IOCHAR, 6, _SELECT_ROM_PAGE | rom_page, NULL, PROCLEV);
#endif
     return errno;

}

/******************************************************************************
*
* NAME:  get_corvette_command_interface
*
* FUNCTION:  Provides a byte read interface to the Command Interface Register.
*
*
* INPUT PARAMETERS:     adapter_info = general card information.
*
*
* EXECUTION ENVIRONMENT:  Process only.
*
* RETURN VALUE DESCRIPTION: if successful returns 0 else error_code.
*
* EXTERNAL PROCEDURES CALLED: diag_io_read
*
******************************************************************************/
unsigned long get_corvette_command_interface(ADAPTER_STRUCT *adapter)
{
        int loop;
        unsigned char CIR_reg[4];

        for (loop=0; loop<4; loop++)
#ifdef DIAGEX10
                CIR_reg[loop] = diag_io_read(adapter->handle, IOCHAR, loop);
#elif DIAGEX20
                diag_io_read(adapter->handle, IOCHAR, loop, &(CIR_reg[loop]), NULL, PROCLEV);
#endif

        return((*(unsigned long *)CIR_reg));

}

/******************************************************************************
*
* NAME:  get_corvette_basic_status_register
*
* FUNCTION:  Provides a byte read interface to the Command Interface Register.
*
*
* INPUT PARAMETERS:     adapter_info = general card information.
*
*
* EXECUTION ENVIRONMENT:  Process only.
*
* RETURN VALUE DESCRIPTION: if successful returns 0 else error_code.
*
* EXTERNAL PROCEDURES CALLED: diag_io_read
*
******************************************************************************/
unsigned char get_corvette_basic_status_register(ADAPTER_STRUCT *adapter)
{
     unsigned char basic_status_register;

#ifdef DIAGEX10
     basic_status_register = diag_io_read(adapter->handle, IOCHAR, 7);
#elif DIAGEX20
     diag_io_read(adapter->handle, IOCHAR, 7, &basic_status_register, NULL, PROCLEV);
#endif

     return basic_status_register;
}

/******************************************************************************
*
* NAME:  initialize_corvette
*
* FUNCTION:  Resets corrvette, allocates pipes and signal areas, loads
*            Management Request Element, and configurations.
*
* INPUT PARAMETERS:     adapter_info = general card information.
*
* EXECUTION ENVIRONMENT:  Process only.
*
* RETURN VALUE DESCRIPTION: if successful returns 0 else error_code.
*
* EXTERNAL PROCEDURES CALLED: Initialize_Delivery_Pipes, dma_allocate,
*                             set_corvette_command_interface,
*                             set_corvette_attention_register,
*                             set_corvette_baseic_register
*
******************************************************************************/
unsigned char initialize_corvette(ADAPTER_STRUCT *adapter)
{
     DMA_STRUCT *scb_area;
     corvette_dds *dds;
     int return_code;
     char *management_request_element;
     int io_register;
     int timeout;

     dds = (corvette_dds *)adapter->dds;

     set_corvette_access_control(adapter, dds->bus_io_addr, _BIOS_ADDRESS_NOT_USED);
     /*set_corvette_access_control(adapter, 0, _BIOS_ADDRESS_NOT_USED); */
     set_corvette_dma_control(adapter, dds->dma_lvl, dds->int_scsi_id);
     set_corvette_io_control(adapter, dds->bus_intr_lvl, _DISABLE_BIOS_ROM, _ENABLE_MOVE_MODE, dds->ext_scsi_id);
     set_corvette_master_control(adapter,
                                 _ENABLE_STREAMING,
                                 _ENABLE_RETURN_CHECKING,
                                 _ENABLE_DATA_PARITY,
                                 _ENABLE_WAIT_STATE,
                                 0);
     set_corvette_scsi_control(adapter,
                               _ENABLE_EXTERNAL_FAST_SCSI,
                               _DISABLE_COMPATIBILITY_MODE,
                               _DISABLE_SCSI_POSTED_WRITES,
                               _DISABLE_EXTERNAL_TERMINATOR,
                               _ENABLE_SCSI_DISCONNECT,
                               _ENABLE_TARGET_MODE);

     set_corvette_uchannel_control(adapter,
                                   _NO_ADDRESS_BOUNDARY,
                                   _6_MICROSECOND_RELEASE_DELAY,
                                   0);

     corvette_info[0] = 0x00;
     /* Start Adapter Reset */
     set_corvette_basic_register(adapter, _DISABLE_SYSTEM_INTERRUPT,
                                          _DISABLE_BUS_MASTER_DMA,
                                          _RESERVED,
                                          _DISABLE_CLEAR_ON_READ,
                                          _START_SUBSYSTEM_RESET);
     sleep(1);
     /* Finish Adapter Reset */
     set_corvette_basic_register(adapter, _DISABLE_SYSTEM_INTERRUPT,
                                          _DISABLE_BUS_MASTER_DMA,
                                          _RESERVED,
                                          _DISABLE_CLEAR_ON_READ,
                                          _DISABLE_SUBSYSTEM_RESET);

     /* Enable System Interrupts to catch reset completion */
     set_corvette_basic_register(adapter, _ENABLE_SYSTEM_INTERRUPT,
                                          _ENABLE_BUS_MASTER_DMA,
                                          _RESERVED,
                                          _DISABLE_CLEAR_ON_READ,
                                          _DISABLE_SUBSYSTEM_RESET);


     /* Wait for reset completion interrupt */
     timeout = adapter->environment.timeout_value;
     while((corvette_info[0]==0x00) && timeout){
          usleep(10);
          timeout--;
     }

    if (!timeout) {
         print_error(adapter, "Adapter Reset Timeout!");
                return ADAPTER_TIMEOUT_ERROR;
        }

     return corvette_info[0];
}


/******************************************************************************
*
* NAME:  send_corvette_command
*
* FUNCTION:  Send command block down to corvette.
*
* INPUT PARAMETERS:     adapter_info = general card information.
*                       command      = command block and length.
*
* EXECUTION ENVIRONMENT:  Process only.
*
* RETURN VALUE DESCRIPTION: if successful returns 0 else error_code.
*
* EXTERNAL PROCEDURES CALLED: set_corvette_command_interface.
*
******************************************************************************/
int send_corvette_command(ADAPTER_STRUCT *adapter, COMMAND command,
                          int destination_device)
{

     DMA_STRUCT command_space;
     unsigned long command_dma_location;
     int loop;
     int timeout;
     unsigned char bus_status_register;


     timeout = adapter->environment.timeout_value;
     command_space = dma_allocate(adapter, 1);
     memcpy(command_space.page_address, command.bytes, command.length);

#ifdef DIAGEX10
     if(diag_io_read(adapter->handle, IOCHAR, 7) & 0x01) {
#elif DIAGEX20
     diag_io_read(adapter->handle, IOCHAR, 7, &bus_status_register, NULL, PROCLEV);

     if((bus_status_register & 0x01)) {
#endif
          print_error(adapter,"Adapter is Busy! send corv cmd\n");
          sleep(5);
#ifdef DIAGEX20
          diag_io_read(adapter->handle, IOCHAR, 7, &bus_status_register, NULL, PROCLEV);
#endif
     }

     set_corvette_command_interface(adapter,byte_swap(command_space.dma_address));
     diag_dma_flush(adapter->handle, command_space.dma_page);
     corvette_info[0] = 0x00;
     set_corvette_attention_register(adapter, 0x03, destination_device);

     while ((corvette_info[0]==0x00) && timeout){
          usleep(10);
          timeout--;
     }

     dma_free(adapter, command_space);

     if (!timeout) {
          print_error(adapter, "Command Timeout!\n");
          return ADAPTER_TIMEOUT_ERROR;
     }

     /* mask off device number from Interrupt Status Register */
     corvette_info[0] = corvette_info[0] >> 4;

     return corvette_info[0];
}

/******************************************************************************
*
* NAME:  send_corvette_immediate_command
*
* FUNCTION:  Send an immediage command down to corvette.
*
* INPUT PARAMETERS:     adapter_info = general card information.
*                       command      = command block and length.
*
* EXECUTION ENVIRONMENT:  Process only.
*
* RETURN VALUE DESCRIPTION: if successful returns 0 else error_code.
*
* EXTERNAL PROCEDURES CALLED: set_corvette_command_interface.
*
******************************************************************************/
int send_corvette_immediate_command(ADAPTER_STRUCT *adapter, COMMAND command, int device_no)
{
     unsigned long *command_word;


     command_word = (unsigned long *)command.bytes;

     set_corvette_command_interface(adapter,*(long *)command.bytes);
     corvette_info[0] = 0x00;
     set_corvette_attention_register(adapter, 0x01, device_no);

     while (corvette_info[0]==0x00){
          usleep(10);
     }

     /* mask off device number from Interrupt Status Register */
     corvette_info[0] = corvette_info[0] >> 4;

     return corvette_info[0];
}
