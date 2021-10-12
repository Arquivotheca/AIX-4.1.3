/* @(#)15       1.6.1.5  src/bos/kernext/disp/ped/pedmacro/hw_ind_mac.h, pedmacro, bos411, 9428A410j 3/24/94 13:53:53 */

/*
 * COMPONENT_NAME: PEDMACRO
 *
 * FUNCTIONS: none
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991,1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#ifndef _H_MID_HW_IND_MAC
#define _H_MID_HW_IND_MAC

#include <mid/hw_locks.h>                   /* PCB and indirect regs locks  */
#include <mid/hw_regs_u.h>

/*---------------------------------------------------------------------------*/
/*                          Indirect Address Macros                          */
/*                                                                           */
/*  The macros in this file are for accessing the information in various     */
/*  Status, Control, Command, and Request Blocks on the adapter using the    */
/*  indirect addressing mode of the adapter.  Before describing the macros,  */
/*  it is important for the user to understand the three steps involved      */
/*  in using the indirect addressing mode:                                   */
/*                                                                           */
/*    1. Load the indirect control register with the proper mode.  The       */
/*       mode determines whether the operation is a READ or a WRITE,         */
/*       and whether the indirect address is to be automatically             */
/*       incremented after each access to the adapter.  All 4 valid modes    */
/*       are defined in hw_regs_u.h.                                         */
/*                                                                           */
/*    2. Load the indirect address with the initial address to be read from  */
/*       or written to.                                                      */
/*                                                                           */
/*    3. Either read data from the indirect data register or write data to   */
/*       the indirect data register, depending on the type of operation      */
/*       requested by the value in the indirect control register.            */
/*                                                                           */
/*  The various Status, Control, Command, and Request Blocks which can be    */
/*  directly accessed by the macros in this file and their corresponding     */
/*  abbreviations are as follows:                                            */
/*                                                                           */
/*      ASCB    = Adapter Status Control Block                               */
/*      FBCSB   = Frame Buffer Control Status Block                          */
/*      CSB     = Current Context Status Block                               */
/*      CSB_X   = Context Status Block (0-15)                                */
/*      CMRB    = Context Memory Request Block                               */
/*      FRB     = Font Request Block                                         */
/*      CCB     = Context Command Block                                      */
/*      CTCB_X  = Color Table Command Block (0-4)                            */
/*      CICB    = Cursor Image Command Block                                 */
/*      M1SB    = Current 3DM1 Status Block                                  */
/*      M1SB_X  = 3DM1 Status Block (0-15)                                   */
/*      M1MSB   = Current 3DM1M Status Block                                 */
/*      M1MSB_X = 3DM1M Status Block (0-15)                                  */
/*                                                                           */
/*  Each of the above blocks has up to nine macros for accessing them.  In   */
/*  the following discussion, substitute the abbreviations specified above   */
/*  for "name" to create the macro names for any specific block.  The macro  */
/*  arguments are defined as follows:                                        */
/*                                                                           */
/*      <number> = This argument is only required for those macros           */
/*                 whose abbreviation ends with "_X".  It must be in the     */
/*                 range indicated in the parenthesis above.  It specifies   */
/*                 the number within the group of that type of block which   */
/*                 is to be accessed.                                        */
/*                                                                           */
/*      offset   = This argument specifies the word offset of the element    */
/*                 within the block which is the first to be accessed.       */
/*                 The macro definitions for each block are proceeded by     */
/*                 defines for the valid offsets.                            */
/*                                                                           */
/*      mode     = This argument specifies the mode for the indirect         */
/*                 control register.  The mode determines if the operation   */
/*                 will be a READ or a WRITE, whether the adapter should     */
/*                 be in RUN or RESET mode, and whether or not the address   */
/*                 will be automatically incremented.  All 8 valid modes     */
/*                 are defined below.                                        */
/*                                                                           */
/*      destin   = A pointer to the destination for data which is to be      */
/*                 read from the adapter.                                    */
/*                                                                           */
/*      source   = A pointer to the source for data which is to be written   */
/*                 to the adapter.                                           */
/*                                                                           */
/*      count    = A count of the number of words to be transferred in a     */
/*                 read or write operation.                                  */
/*                                                                           */
/*      value    = A value to be written to the adapter during a write       */
/*                 value operation or to be compared to the data read        */
/*                 from the adapter during a polling operation.              */
/*                                                                           */
/*      result   = The value read from the adapter during a read value       */
/*                 operation or the last value read from the adapter         */
/*                 during a polling operation which is waiting for a value   */
/*                 which is not equal to the specified value.                */
/*                                                                           */
/*      mask     = A mask to be ANDED with the value read from the adapter   */
/*                 before it is tested against the expected value.           */
/*                                                                           */
/*                                                                           */
/*  The nine macros for each block are defined as below.  The Poll macros    */
/*  are only defined for blocks which have elements which may need to be     */
/*  polled.  Poll macros may easily be added for others if it becomes        */
/*  necessary.                                                               */
/*                                                                           */
/*      MID_RD_name_VALUE ( <number>, offset, result )                       */
/*            Read a single value from the adapter memory location at the    */
/*            offset within the block specified by "name" and assign the     */
/*            value to result.                                               */
/*                                                                           */
/*      MID_WR_name_VALUE ( <number>, offset, value )                        */
/*            Write a single value to the adapter memory location at the     */
/*            offset within the block specified by "name".                   */
/*                                                                           */
/*      MID_RD_name_ARRAY ( <number>, offset, destin, count )                */
/*            Read an array of count words from the adapter memory           */
/*            location at the offset with the block specified by "name"      */
/*            to the host memory location pointed to by destin.              */
/*                                                                           */
/*      MID_WR_name_ARRAY ( <number>, offset, source, count )                */
/*            Write an array of count words from the host memory             */
/*            location pointed to by source to the adapter memory            */
/*            location at the offset within the block specified by "name".   */
/*                                                                           */
/*      MID_POLL_name_EQ ( <number>, offset, value )                         */
/*            Poll the adapter memory location at the offset within the      */
/*            block specified by "name" until its value is equal to          */
/*            the specified value.                                           */
/*                                                                           */
/*      MID_POLL_name_EQ_MASK ( <number>, offset, value, mask )              */
/*            Poll the adapter memory location at the offset within the      */
/*            block specified by "name" until its value ANDED with the       */
/*            mask is equal to the specified value.                          */
/*                                                                           */
/*      MID_POLL_name_EQ_MASK_TO ( <number>, offset, value, mask, time )     */
/*            Poll the adapter memory location at the offset within the      */
/*            block specified by "name" until its value ANDED with the       */
/*            mask is equal to the specified value.  Stop polling after      */
/*            polling the adapter "time" times.                              */
/*                                                                           */
/*      MID_POLL_name_NE ( <number>, offset, value, result )                 */
/*            Poll the adapter memory location at the offset within the      */
/*            block specified by "name" until its value is not equal to      */
/*            the specified value.  Assign the last value to result.         */
/*                                                                           */
/*      MID_POLL_name_NE_MASK ( <number>, offset, value, result, mask )      */
/*            Poll the adapter memory location at the offset within the      */
/*            block specified by "name" until its value ANDED with the       */
/*            mask is not equal to the specified value.  Assign the          */
/*            last value to result.                                          */
/*                                                                           */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*             Adapter Status Control Block (ASCB) Definitions               */
/*---------------------------------------------------------------------------*/

/* Define base address for Adapter Status Control Block */
#define MID_ASCB_BASE                           0x00001100

/* Define valid offsets for elements in the Adapter Status Control Block */
#define MID_ASCB_MICROCODE_LEVEL                0x00000000
#define MID_ASCB_EXPECTED_CRC_VALUE             0x00000001
#define MID_ASCB_ACTUAL_CRC_VALUE               0x00000002
#define MID_ASCB_LAST_FIFO_DATA_LENGTH          0x00000003
#define MID_ASCB_SE_LENGTH                      0x00000004
#define MID_ASCB_C2H_VERSION                    0x00000005
#define MID_ASCB_DMA_TRACE_BUFFER_LENGTH        0x00000006
#define MID_ASCB_PTR_FBCSB                      0x00000007
#define MID_ASCB_PTR_CSB                        0x00000008
#define MID_ASCB_PTR_CSB_X                      0x00000009
#define MID_ASCB_ADAPTER_ID                     0x00000019
#define MID_ASCB_PTR_CMRB                       0x00000020
#define MID_ASCB_PTR_FRB                        0x00000021
#define MID_ASCB_PTR_CCB                        0x00000024
#define MID_ASCB_PTR_CTCB_X                     0x00000025
#define MID_ASCB_PTR_CICB                       0x0000002A
#define MID_ASCB_PTR_TRACE_BEGIN                0x0000002B
#define MID_ASCB_PTR_TRACE_END                  0x0000002C
#define MID_ASCB_PTR_TRACE_NEXT                 0x0000002D
#define MID_ASCB_PTR_FUNCTION_TABLE             0x00000030
#define MID_ASCB_PTR_LOADED_PIPES               0x00000031
#define MID_ASCB_PTR_PIPE_TABLE                 0x00000032
#define MID_ASCB_VERSION                        0x0000003A
#define MID_ASCB_ROM_REVISION                   0x0000005A
#define MID_ASCB_HOST_DMA_BUSY                  0x0000007A
#define MID_ASCB_CURRENT_DMA_TYPE               0x0000007B
#define MID_ASCB_CORRELATOR_1                   0x0000007C
#define MID_ASCB_CORRELATOR_2                   0x0000007D
#define MID_ASCB_ERROR_CODE                     0x0000007E
#define MID_ASCB_PICK_ADDRESS                   0x0000007F
#define MID_ASCB_SECOND_BLAST                   0x00000080
#define MID_ASCB_ELEMENT_COUNTER                0x00000081
#define MID_ASCB_LOAD_SECTION_COUNT             0x00000082
#define MID_ASCB_SECTION_0                      0x00000083
#define MID_ASCB_SECTION_1                      0x00000087
#define MID_ASCB_SECTION_2                      0x0000008B
#define MID_ASCB_SECTION_3                      0x0000008F
#define MID_ASCB_SECTION_4                      0x00000093
#define MID_ASCB_SECTION_5                      0x00000097
#define MID_ASCB_SECTION_6                      0x0000009B
#define MID_ASCB_SECTION_7                      0x0000009F
#define MID_ASCB_SECTION_8                      0x000000A3
#define MID_ASCB_SECTION_9                      0x000000A7
#define MID_ASCB_SIZE                           0x000000AB

/* Point to an element in the Adapter Status Control Block */
#define _PTR_ASCB( adr, mode, number, offset )                                \
	_MID_POINT_IND( adr, mode, (MID_ASCB_BASE + offset) )

/* Read a value from the Adapter Status Control Block */
#define MID_RD_ASCB_VALUE( offset, result )                                   \
	_MID_RD_IND_VALUE( _PTR_ASCB, BLK_ASCB, 0, offset, result )

/* Write a value to the Adapter Status Control Block */
#define MID_WR_ASCB_VALUE( offset, value )                                    \
	_MID_WR_IND_VALUE( _PTR_ASCB, BLK_ASCB, 0, offset, value )

/* Read an array of values from the Adapter Status Control Block */
#define MID_RD_ASCB_ARRAY( offset, destin, count )                            \
	_MID_RD_IND_ARRAY( _PTR_ASCB, BLK_ASCB, 0, offset, destin, count )

/* Write an array of values to the Adapter Status Control Block */
#define MID_WR_ASCB_ARRAY( offset, source, count )                            \
	_MID_WR_IND_ARRAY( _PTR_ASCB, BLK_ASCB, 0, offset, source, count )

/* Poll element in the Adapter Status Control Block until == 'value' */
#define MID_POLL_ASCB_EQ( offset, value )                                     \
	_MID_POLL_IND_EQ( _PTR_ASCB, BLK_ASCB, 0, offset, value )

/* Poll masked element in the Adapter Status Control Block until  == 'value' */
#define MID_POLL_ASCB_EQ_MASK( offset, value, mask )                          \
	_MID_POLL_IND_EQ_MASK( _PTR_ASCB, BLK_ASCB, 0, offset, value, mask )

/* Poll element in the Adapter Status Control Block until != 'value' */
#define MID_POLL_ASCB_NE( offset, value, result )                             \
	_MID_POLL_IND_NE( _PTR_ASCB, BLK_ASCB, 0, offset, value, result )

/* Poll masked element in the Adapter Status Control Block until != 'value' */
#define MID_POLL_ASCB_NE_MASK( offset, value, result, mask )                  \
	_MID_POLL_IND_NE_MASK( _PTR_ASCB,BLK_ASCB,0,offset,value,result,mask )


/*---------------------------------------------------------------------------*/
/*          Frame Buffer Control Status Block (FBCSB) Definitions            */
/*---------------------------------------------------------------------------*/

/* Define valid offsets for elements in the Frame Buffer Control Status Block*/
#define MID_FBCSB_LENGTH                        0x00000000
#define MID_FBCSB_CORRELATOR_1_2                0x00000001
#define MID_FBCSB_CORRELATOR_3_4                0x00000002
#define MID_FBCSB_CORRELATOR_5_6                0x00000003
#define MID_FBCSB_CORRELATOR_7_8                0x00000004
#define MID_FBCSB_CORRELATOR_9_10               0x00000005
#define MID_FBCSB_CORRELATOR_11_12              0x00000006
#define MID_FBCSB_CORRELATOR_13_14              0x00000007
#define MID_FBCSB_CORRELATOR_15_16              0x00000008
#define MID_FBCSB_SIZE                          0x00000009

/* Point to an element in the Frame Buffer Control Status Block */
#define _PTR_FBCSB( adr, mode, number, offset )                               \
	_MID_POINT_IND_IND( adr, mode, (MID_ASCB_BASE + MID_ASCB_PTR_FBCSB),  \
	        offset )

/* Read a value from the Frame Buffer Control Status Block */
#define MID_RD_FBCSB_VALUE( offset, result )                                  \
	_MID_RD_IND_VALUE( _PTR_FBCSB, BLK_FBCSB, 0, offset, result )

/* Write a value to the Frame Buffer Control Status Block */
#define MID_WR_FBCSB_VALUE( offset, value )                                   \
	_MID_WR_IND_VALUE( _PTR_FBCSB, BLK_FBCSB, 0, offset, value )

/* Read an array of values from the Frame Buffer Control Status Block */
#define MID_RD_FBCSB_ARRAY( offset, destin, count )                           \
	_MID_RD_IND_ARRAY( _PTR_FBCSB, BLK_FBCSB, 0, offset, destin, count )

/* Write an array of values to the Frame Buffer Control Status Block */
#define MID_WR_FBCSB_ARRAY( offset, source, count )                           \
	_MID_WR_IND_ARRAY( _PTR_FBCSB, BLK_FBCSB, 0, offset, source, count )

/* Poll element in the Frame Buffer Control Status Block until == 'value' */
#define MID_POLL_FBCSB_EQ( offset, value )                                    \
	_MID_POLL_IND_EQ( _PTR_FBCSB, BLK_FBCSB, 0, offset, value )

/* Poll masked element in the Frame Buffer Control Status Block until == val */
#define MID_POLL_FBCSB_EQ_MASK( offset, value, mask )                         \
	_MID_POLL_IND_EQ_MASK( _PTR_FBCSB, BLK_FBCSB, 0, offset, value, mask )

/* Poll element in the Frame Buffer Control Status Block until != 'value' */
#define MID_POLL_FBCSB_NE( offset, value, result )                            \
	_MID_POLL_IND_NE( _PTR_FBCSB, BLK_FBCSB, 0, offset, value, result )

/* Poll masked element in the Frame Buffer Control Status Block until != val */
#define MID_POLL_FBCSB_NE_MASK( offset, value, result, mask )                 \
	_MID_POLL_IND_NE_MASK(_PTR_FBCSB,BLK_FBCSB,0,offset,value,result,mask)


/*---------------------------------------------------------------------------*/
/*              Current Context Status Block (CSB) Definitions               */
/*---------------------------------------------------------------------------*/

/* Define valid offsets for elements in the Current Context Status Block */
#define MID_CSB_CONTEXT_ID                      0x00000000
#define MID_CSB_TYPE                            0x00000001
#define MID_CSB_STATUS_FLAG                     0x00000002
#define MID_CSB_WINDOW_ID_COLOR_PALETTE         0x00000003
#define MID_CSB_FIFO_DATA_LENGTH                0x00000004
#define MID_CSB_PTR_M1SB                        0x00000005
#define MID_CSB_PTR_M1MSB                       0x00000005
#define MID_CSB_COLOR_PROCESSING_MODE           0x00000006
#define MID_CSB_SET_WINDOW_PARAMETERS_CORR      0x00000007
#define MID_CSB_CLEAR_CONTROL_PLANES_CORR       0x00000007
#define MID_CSB_STALL_TIME                      0x00000008
#define MID_CSB_SIZE                            0x00000009

/* Point to an element in the Current Context Status Block */
#define _PTR_CSB( adr, mode, number, offset )                                 \
	_MID_POINT_IND_IND( adr, mode, (MID_ASCB_BASE + MID_ASCB_PTR_CSB),    \
	        offset)

/* Read a value from the Current Context Status Block */
#define MID_RD_CSB_VALUE( offset, result )                                    \
	_MID_RD_IND_VALUE( _PTR_CSB, BLK_CSB, 0, offset, result )

/* Write a value to the Current Context Status Block */
#define MID_WR_CSB_VALUE( offset, value )                                     \
	_MID_WR_IND_VALUE( _PTR_CSB, BLK_CSB, 0, offset, value )

/* Read an array of values from the Current Context Status Block */
#define MID_RD_CSB_ARRAY( offset, destin, count )                             \
	_MID_RD_IND_ARRAY( _PTR_CSB, BLK_CSB, 0, offset, destin, count )

/* Write an array of values to the Current Context Status Block */
#define MID_WR_CSB_ARRAY( offset, source, count )                             \
	_MID_WR_IND_ARRAY( _PTR_CSB, BLK_CSB, 0, offset, source, count )

/* Poll element in the Current Context Status Block until == 'value' */
#define MID_POLL_CSB_EQ( offset, value )                                      \
	_MID_POLL_IND_EQ( _PTR_CSB, BLK_CSB, 0, offset, value )

/* Poll masked element in the Current Context Status Block until  == 'value' */
#define MID_POLL_CSB_EQ_MASK( offset, value, mask )                           \
	_MID_POLL_IND_EQ_MASK( _PTR_CSB, BLK_CSB, 0, offset, value, mask )

/* Poll element in the Current Context Status Block until != 'value' */
#define MID_POLL_CSB_NE( offset, value, result )                              \
	_MID_POLL_IND_NE( _PTR_CSB, BLK_CSB, 0, offset, value, result )

/* Poll masked element in the Current Context Status Block until != 'value' */
#define MID_POLL_CSB_NE_MASK( offset, value, result, mask )                   \
	_MID_POLL_IND_NE_MASK( _PTR_CSB,BLK_CSB,0,offset,value,result,mask )

/* Poll masked element for 'time' in Current Ctx Status Block till == 'value'*/
#define MID_POLL_CSB_EQ_MASK_TO( offset, value, mask, time )                  \
	_MID_POLL_IND_EQ_MASK_TO( _PTR_CSB,BLK_CSB,0,offset,value,mask,time)


/*---------------------------------------------------------------------------*/
/*                Context Status Block X (CSB_X) Definitions                 */
/*---------------------------------------------------------------------------*/

/* Define valid offsets for elements in the Context Status Block */
#define MID_CSB_X_CONTEXT_ID                    0x00000000
#define MID_CSB_X_TYPE                          0x00000001
#define MID_CSB_X_STATUS_FLAG                   0x00000002
#define MID_CSB_X_WINDOW_ID_COLOR_PALETTE       0x00000003
#define MID_CSB_X_FIFO_DATA_LENGTH              0x00000004
#define MID_CSB_X_PTR_M1SB                      0x00000005
#define MID_CSB_X_PTR_M1MSB                     0x00000005
#define MID_CSB_X_COLOR_PROCESSING_MODE         0x00000006
#define MID_CSB_X_SET_WINDOW_PARAMETERS_CORR    0x00000007
#define MID_CSB_X_CLEAR_CONTROL_PLANES_CORR     0x00000007
#define MID_CSB_X_STALL_TIME                    0x00000008
#define MID_CSB_X_SIZE                          0x00000009

/* Point to an element in a Context Status Block (0-15) */
#define _PTR_CSB_X( adr, mode, number, offset )                               \
	_MID_POINT_IND_IND( adr, mode,                                        \
	        (MID_ASCB_BASE + MID_ASCB_PTR_CSB_X + number), offset )

/* Read a value from a Context Status Block (0-15) */
#define MID_RD_CSB_X_VALUE( number, offset, result )                          \
	_MID_RD_IND_VALUE( _PTR_CSB_X, BLK_CSB_X, number, offset, result )

/* Write a value to a Context Status Block (0-15) */
#define MID_WR_CSB_X_VALUE( number, offset, value )                           \
	_MID_WR_IND_VALUE( _PTR_CSB_X, BLK_CSB_X, number, offset, value )

/* Read an array of values from a Context Status Block (0-15) */
#define MID_RD_CSB_X_ARRAY( number, offset, destin, count )                   \
	_MID_RD_IND_ARRAY( _PTR_CSB_X,BLK_CSB_X,number,offset,destin,count )

/* Write an array of values to a Context Status Block (0-15) */
#define MID_WR_CSB_X_ARRAY( number, offset, source, count )                   \
	_MID_WR_IND_ARRAY( _PTR_CSB_X,BLK_CSB_X,number,offset,source,count )

/* Poll element in a Context Status Block (0-15) until == 'value' */
#define MID_POLL_CSB_X_EQ( number, offset, value )                            \
	_MID_POLL_IND_EQ( _PTR_CSB_X, BLK_CSB_X, number, offset, value )

/* Poll masked element in a Context Status Block (0-15) until == 'value' */
#define MID_POLL_CSB_X_EQ_MASK( number, offset, value, mask )                 \
	_MID_POLL_IND_EQ_MASK( _PTR_CSB_X,BLK_CSB_X,number,offset,value,mask )

/* Poll element in a Context Status Block (0-15) until != 'value' */
#define MID_POLL_CSB_X_NE( number, offset, value, result )                    \
	_MID_POLL_IND_NE( _PTR_CSB_X,BLK_CSB_X,number,offset,value,result )

/* Poll masked element in a Context Status Block (0-15) until != 'value' */
#define MID_POLL_CSB_X_NE_MASK( number, offset, value, result, mask )         \
	_MID_POLL_IND_NE_MASK(_PTR_CSB_X,BLK_CSB_X,number,offset,value,result,\
	        mask)


/*---------------------------------------------------------------------------*/
/*             Context Memory Request Block (CMRB) Definitions               */
/*---------------------------------------------------------------------------*/

/* Define valid offsets for elements in the Context Memory Request Block */
#define MID_CMRB_CONTEXT_ID                     0x00000000
#define MID_CMRB_LENGTH                         0x00000001
#define MID_CMRB_SIZE                           0x00000002

/* Point to an element in the Context Memory Request Block */
#define _PTR_CMRB( adr, mode, number, offset )                                \
	_MID_POINT_IND_IND( adr, mode, (MID_ASCB_BASE + MID_ASCB_PTR_CMRB),   \
	        offset )

/* Read a value from the Context Memory Request Block */
#define MID_RD_CMRB_VALUE( offset, result )                                   \
	_MID_RD_IND_VALUE( _PTR_CMRB, BLK_CMRB, 0, offset, result )

/* Write a value to the Context Memory Request Block */
#define MID_WR_CMRB_VALUE( offset, value )                                    \
	_MID_WR_IND_VALUE( _PTR_CMRB, BLK_CMRB, 0, offset, value )

/* Read an array of values from the Context Memory Request Block */
#define MID_RD_CMRB_ARRAY( offset, destin, count )                            \
	_MID_RD_IND_ARRAY( _PTR_CMRB, BLK_CMRB, 0, offset, destin, count )

/* Write an array of values to the Context Memory Request Block */
#define MID_WR_CMRB_ARRAY( offset, source, count )                            \
	_MID_WR_IND_ARRAY( _PTR_CMRB, BLK_CMRB, 0, offset, source, count )


/*---------------------------------------------------------------------------*/
/*                   Font Request Block (FRB) Definitions                    */
/*---------------------------------------------------------------------------*/

/* Define valid offsets for elements in the Font Request Block */
#define MID_FRB_PIN_FONT_ID                     0x00000000
#define MID_FRB_UNPIN_FONT_ID                   0x00000001
#define MID_FRB_SIZE                            0x00000002

/* Point to an element in the Font Request Block */
#define _PTR_FRB( adr, mode, number, offset )                                 \
	_MID_POINT_IND_IND( adr, mode, (MID_ASCB_BASE + MID_ASCB_PTR_FRB),    \
	        offset)

/* Read a value from the Font Request Block */
#define MID_RD_FRB_VALUE( offset, result )                                    \
	_MID_RD_IND_VALUE( _PTR_FRB, BLK_FRB, 0, offset, result )

/* Write a value to the Font Request Block */
#define MID_WR_FRB_VALUE( offset, value )                                     \
	_MID_WR_IND_VALUE( _PTR_FRB, BLK_FRB, 0, offset, value )

/* Read an array of values from the Font Request Block */
#define MID_RD_FRB_ARRAY( offset, destin, count )                             \
	_MID_RD_IND_ARRAY( _PTR_FRB, BLK_FRB, 0, offset, destin, count )

/* Write an array of values to the Font Request Block */
#define MID_WR_FRB_ARRAY( offset, source, count )                             \
	_MID_WR_IND_ARRAY( _PTR_FRB, BLK_FRB, 0, offset, source, count )


/*---------------------------------------------------------------------------*/
/*                  Context Command Block (CCB) Definitions                  */
/*---------------------------------------------------------------------------*/

/* Define valid offsets for elements in the Context Command Block */
#define MID_CCB_OLD_NEW_CONTEXT_HANDLING        0x00000000
#define MID_CCB_OLD_CONTEXT_ID                  0x00000001
#define MID_CCB_OLD_CONTEXT_ADDRESS             0x00000002
#define MID_CCB_OLD_CONTEXT_LENGTH              0x00000003
#define MID_CCB_NEW_CONTEXT_ID                  0x00000004
#define MID_CCB_NEW_CONTEXT_TYPE                0x00000005
#define MID_CCB_NEW_CONTEXT_ADDRESS             0x00000006
#define MID_CCB_NEW_CONTEXT_LENGTH              0x00000007
#define MID_CCB_NEW_CONTEXT_HIGH_LOW_WATER_MARK 0x00000008
#define MID_CCB_SYNC_CONTEXT_SWITCHING_CORR     0x00000009
#define MID_CCB_SIZE                            0x0000000A

/* Point to an element in the Context Command Block */
#define _PTR_CCB( adr, mode, number, offset )                                 \
	_MID_POINT_IND_IND( adr, mode, (MID_ASCB_BASE + MID_ASCB_PTR_CCB),    \
	        offset)

/* Read a value from the Context Command Block */
#define MID_RD_CCB_VALUE( offset, result )                                    \
	_MID_RD_IND_VALUE( _PTR_CCB, BLK_CCB, 0, offset, result )

/* Write a value to the Context Command Block */
#define MID_WR_CCB_VALUE( offset, value )                                     \
	_MID_WR_IND_VALUE( _PTR_CCB, BLK_CCB, 0, offset, value )

/* Read an array of values from the Context Command Block */
#define MID_RD_CCB_ARRAY( offset, destin, count )                             \
	_MID_RD_IND_ARRAY( _PTR_CCB, BLK_CCB, 0, offset, destin, count )

/* Write an array of values to the Context Command Block */
#define MID_WR_CCB_ARRAY( offset, source, count )                             \
	_MID_WR_IND_ARRAY( _PTR_CCB, BLK_CCB, 0, offset, source, count )


/*---------------------------------------------------------------------------*/
/*             Color Table Command Block X (CTCB_X) Definitions              */
/*---------------------------------------------------------------------------*/

/* Define valid offsets for elements in the Color Table Command Block */
#define MID_CTCB_X_COLOR_TABLE                  0x00000000
#define MID_CTCB_X_SIZE                         0x00000100

/* Point to an element in a Color Table Command Block (0-4) */
#define _PTR_CTCB_X( adr, mode, number, offset )                              \
	_MID_POINT_IND_IND( adr, mode,                                        \
	        (MID_ASCB_BASE + MID_ASCB_PTR_CTCB_X + number), offset )

/* Read a value from a Color Table Command Block (0-4) */
#define MID_RD_CTCB_X_VALUE( number, offset, result )                         \
	_MID_RD_IND_VALUE( _PTR_CTCB_X, BLK_CTCB_X, number, offset, result )

/* Write a value to a Color Table Command Block (0-4) */
#define MID_WR_CTCB_X_VALUE( number, offset, value )                          \
	_MID_WR_IND_VALUE( _PTR_CTCB_X, BLK_CTCB_X, number, offset, value )

/* Read an array of values from a Color Table Command Block (0-4) */
#define MID_RD_CTCB_X_ARRAY( number, offset, destin, count )                  \
	_MID_RD_IND_ARRAY( _PTR_CTCB_X,BLK_CTCB_X,number,offset,destin,count )

/* Write an array of values to a Color Table Command Block (0-4) */
#define MID_WR_CTCB_X_ARRAY( number, offset, source, count )                  \
	_MID_WR_IND_ARRAY( _PTR_CTCB_X,BLK_CTCB_X,number,offset,source,count )


/*---------------------------------------------------------------------------*/
/*             Cursor Image Command Block (CICB) Definitions                 */
/*---------------------------------------------------------------------------*/

/* Define valid offsets for elements in a Cursor Image Command Block */
#define MID_CICB_ID_FLAGS                       0x00000000
#define MID_CICB_CROSS_UPPER_LEFT_XY            0x00000001
#define MID_CICB_CROSS_WIDTH_HEIGHT             0x00000002
#define MID_CICB_COLOR_1                        0x00000003
#define MID_CICB_COLOR_2                        0x00000004
#define MID_CICB_COLOR_3                        0x00000005
#define MID_CICB_HOT_SPOT_XY                    0x00000006
#define MID_CICB_IMAGE_DATA                     0x00000007
#define MID_CICB_SIZE                           0x00000107

/* Point to an element in the Cursor Image Command Block */
#define _PTR_CICB( adr, mode, number, offset )                                \
	_MID_POINT_IND_IND( adr, mode, (MID_ASCB_BASE + MID_ASCB_PTR_CICB),   \
	        offset )

/* Read a value from the Cursor Image Command Block */
#define MID_RD_CICB_VALUE( offset, result )                                   \
	_MID_RD_IND_VALUE( _PTR_CICB, BLK_CICB, 0, offset, result )

/* Write a value to the Cursor Image Command Block */
#define MID_WR_CICB_VALUE( offset, value )                                    \
	_MID_WR_IND_VALUE( _PTR_CICB, BLK_CICB, 0, offset, value )

/* Read an array of values from the Cursor Image Command Block */
#define MID_RD_CICB_ARRAY( offset, destin, count )                            \
	_MID_RD_IND_ARRAY( _PTR_CICB, BLK_CICB, 0, offset, destin, count )

/* Write an array of values to the Cursor Image Command Block */
#define MID_WR_CICB_ARRAY( offset, source, count )                            \
	_MID_WR_IND_ARRAY( _PTR_CICB, BLK_CICB, 0, offset, source, count )

/* Poll element in the Cursor Image Command Block until == 'value' */
#define MID_POLL_CICB_EQ( offset, value )                                     \
	_MID_POLL_IND_EQ( _PTR_CICB, BLK_CICB, 0, offset, value )

/* Poll element in the Cursor Image Command Block until == 'value' */
#define MID_POLL_CICB_EQ_MASK( offset, value, mask )                          \
	_MID_POLL_IND_EQ_MASK( _PTR_CICB, BLK_CICB, 0, offset, value, mask )

/* Poll element in the Cursor Image Command Block until != 'value' */
#define MID_POLL_CICB_NE( offset, value, result )                             \
	_MID_POLL_IND_NE( _PTR_CICB, BLK_CICB, 0, offset, value, result )

/* Poll element in the Cursor Image Command Block until != 'value' */
#define MID_POLL_CICB_NE_MASK( offset, value, result, mask )                  \
	_MID_POLL_IND_NE_MASK( _PTR_CICB,BLK_CICB,0,offset,value,result,mask)

/* Poll element in the Cursor Image Command Block until == 'value' w/timeout */
#define MID_POLL_CICB_EQ_MASK_TO( offset, value, mask, time )                 \
	_MID_POLL_IND_EQ_MASK_TO(_PTR_CICB,BLK_CICB,0,offset,value,mask,time)


/*---------------------------------------------------------------------------*/
/*                Current M1 Status Block (M1SB) Definitions                 */
/*---------------------------------------------------------------------------*/

/* Define valid offsets for elements in the Current M1 Status Block */
#define MID_M1SB_CORRELATOR                    0x00000000
#define MID_M1SB_CONDITION_FLAG                0x00000001
#define MID_M1SB_TEXT_FONT_CORRELATOR          0x00000002
#define MID_M1SB_TEXT_FONT                     0x00000003
#define MID_M1SB_PICK_CORRELATOR_PICK_HITS     0x00000004
#define MID_M1SB_PICK_DATA_LENGTH              0x00000005
#define MID_M1SB_SIZE                          0x00000006
#if 0
#define MID_M1SB_GENERAL_ATTRIBUTE_CORRELATOR  0x00000014
#define MID_M1SB_EDGE FLAG                     0x00000015
#define MID_M1SB_INTERIOR_COLOR                0x00000016
#define MID_M1SB_INTERIOR_STYLE                0x0000001A
#define MID_M1SB_COLOR_QUANTIZATION_DATA       0x0000001B
#define MID_M1SB_SURFACE_APPROX_CRITERIA_DATA  0x0000001E
#define MID_M1SB_CURVE_APPROX_CRITERIA_DATA    0x00000021
#define MID_M1SB_PARAMETRIC_SURFACE_CHAR       0x00000023
#define MID_M1SB_MC_TO_DC_SCALE_FACTOR         0x00000027
#define MID_M1SB_MC_TO_VRC_TRANSFORM           0x00000028
#define MID_M1SB_VRC_TO_DC_TRANSFORM           0x00000038
#define MID_M1SB_SIZE                          0x00000048
#endif /* 0 */

/* Point to an element in the Current M1 Status Block */
#define _PTR_M1SB( adr, mode, number, offset )                                \
	_MID_POINT_IND_IND_IND( adr, mode, (MID_ASCB_BASE + MID_ASCB_PTR_CSB),\
	        MID_CSB_PTR_M1SB, offset )

/* Read a value from the Current M1 Status Block */
#define MID_RD_M1SB_VALUE( offset, result )                                   \
	_MID_RD_IND_VALUE( _PTR_M1SB, BLK_M1SB, 0, offset, result )

/* Write a value to the Current M1 Status Block */
#define MID_WR_M1SB_VALUE( offset, value )                                    \
	_MID_WR_IND_VALUE( _PTR_M1SB, BLK_M1SB, 0, offset, value )

/* Read an array of values from the Current M1 Status Block */
#define MID_RD_M1SB_ARRAY( offset, destin, count )                            \
	_MID_RD_IND_ARRAY( _PTR_M1SB, BLK_M1SB, 0, offset, destin, count )

/* Write an array of values to the Current M1 Status Block */
#define MID_WR_M1SB_ARRAY( offset, source, count )                            \
	_MID_WR_IND_ARRAY( _PTR_M1SB, BLK_M1SB, 0, offset, source, count )

/* Poll element in the Current M1 Status Block until == 'value' */
#define MID_POLL_M1SB_EQ( offset, value )                                     \
	_MID_POLL_IND_EQ( _PTR_M1SB, BLK_M1SB, 0, offset, value )

/* Poll masked element in the Current M1 Status Block until  == 'value' */
#define MID_POLL_M1SB_EQ_MASK( offset, value, mask )                          \
	_MID_POLL_IND_EQ_MASK( _PTR_M1SB, BLK_M1SB, 0, offset, value, mask )

/* Poll element in the Current M1 Status Block until != 'value' */
#define MID_POLL_M1SB_NE( offset, value, result )                             \
	_MID_POLL_IND_NE( _PTR_M1SB, BLK_M1SB, 0, offset, value, result )

/* Poll masked element in the Current M1 Status Block until != 'value' */
#define MID_POLL_M1SB_NE_MASK( offset, value, result, mask )                  \
	_MID_POLL_IND_NE_MASK( _PTR_M1SB,BLK_M1SB,0,offset,value,result,mask )


/*---------------------------------------------------------------------------*/
/*                 M1 Status Block X (M1SB_X) Definitions                    */
/*---------------------------------------------------------------------------*/

/* Define valid offsets for elements in an M1 Status Block (0-15) */
#define MID_M1SB_X_CORRELATOR                    0x00000000
#define MID_M1SB_X_CONDITION_FLAG                0x00000001
#define MID_M1SB_X_TEXT_FONT_CORRELATOR          0x00000002
#define MID_M1SB_X_TEXT_FONT                     0x00000003
#define MID_M1SB_X_PICK_CORRELATOR_PICK_HITS     0x00000004
#define MID_M1SB_X_PICK_DATA_LENGTH              0x00000005
#define MID_M1SB_X_SIZE                          0x00000006

/* Point to an element in an M1 Status Block (0-15) */
#define _PTR_M1SB_X( adr, mode, number, offset )                              \
	_MID_POINT_IND_IND_IND( adr, mode,                                    \
	        (MID_ASCB_BASE + MID_ASCB_PTR_CSB_X + number),                \
	        MID_CSB_PTR_M1SB, offset )

/* Read a value from an M1 Status Block (0-15) */
#define MID_RD_M1SB_X_VALUE( number, offset, result )                         \
	_MID_RD_IND_VALUE( _PTR_M1SB_X, BLK_M1SB_X, number, offset, result )

/* Write a value to an M1 Status Block (0-15) */
#define MID_WR_M1SB_X_VALUE( number, offset, value )                          \
	_MID_WR_IND_VALUE( _PTR_M1SB_X, BLK_M1SB_X, number, offset, value )

/* Read an array of values from an M1 Status Block (0-15) */
#define MID_RD_M1SB_X_ARRAY( number, offset, destin, count )                  \
	_MID_RD_IND_ARRAY( _PTR_M1SB_X,BLK_M1SB_X,number,offset,destin,count )

/* Write an array of values to an M1 Status Block (0-15) */
#define MID_WR_M1SB_X_ARRAY( number, offset, source, count )                  \
	_MID_WR_IND_ARRAY( _PTR_M1SB_X,BLK_M1SB_X,number,offset,source,count )

/* Poll element in a M1 Status Block (0-15) until == 'value' */
#define MID_POLL_M1SB_X_EQ( number, offset, value )                           \
	_MID_POLL_IND_EQ( _PTR_M1SB_X, BLK_M1SB_X, number, offset, value )

/* Poll masked element in a M1 Status Block (0-15) until == 'value' */
#define MID_POLL_M1SB_X_EQ_MASK( number, offset, value, mask )                \
	_MID_POLL_IND_EQ_MASK(_PTR_M1SB_X,BLK_M1SB_X,number,offset,value,mask)

/* Poll element in a M1 Status Block (0-15) until != 'value' */
#define MID_POLL_M1SB_X_NE( number, offset, value, result )                   \
	_MID_POLL_IND_NE( _PTR_M1SB_X,BLK_M1SB_X,number,offset,value,result )

/* Poll masked element in a M1 Status Block (0-15) until != 'value' */
#define MID_POLL_M1SB_X_NE_MASK( number, offset, value, result, mask )        \
	_MID_POLL_IND_NE_MASK( _PTR_M1SB_X, BLK_M1SB_X, number, offset, value,\
	        result, mask )


/*---------------------------------------------------------------------------*/
/*               Current M1M Status Block (M1MSB) Definitions                */
/*---------------------------------------------------------------------------*/

/* Define valid offsets for elements in the Current M1M Status Block */
#define MID_M1MSB_CORRELATOR                    0x00000000
#define MID_M1MSB_WINDOW_CHAR_POSITION          0x00000001
#define MID_M1MSB_SCREEN_CHAR_POSITION          0x00000002
#define MID_M1MSB_RED_COLOR                     0x00000003
#define MID_M1MSB_GREEN_COLOR                   0x00000004
#define MID_M1MSB_BLUE_COLOR                    0x00000005
#define MID_M1MSB_MODELING_MATRIX               0x00000006
#define MID_M1MSB_PROJECTION_MATRIX             0x00000016
#define MID_M1MSB_RENORM_FLAG_MTX_COMP_TYPE     0x00000026
#define MID_M1MSB_INVERSE_TRANSPOSE_MATRIX      0x00000027
#define MID_M1MSB_PICK_CORRELATOR_PICK_HITS     0x00000030
#define MID_M1MSB_PICK_DATA_LENGTH              0x00000031
#define MID_M1MSB_SIZE                          0x00000032

/* Point to an element in the Current M1M Status Block */
#define _PTR_M1MSB( adr, mode, number, offset )                               \
	_MID_POINT_IND_IND_IND( adr, mode, (MID_ASCB_BASE + MID_ASCB_PTR_CSB),\
	        MID_CSB_PTR_M1MSB, offset )

/* Read a value from the Current M1M Status Block */
#define MID_RD_M1MSB_VALUE( offset, result )                                  \
	_MID_RD_IND_VALUE( _PTR_M1MSB, BLK_M1MSB, 0, offset, result )

/* Write a value to the Current M1M Status Block */
#define MID_WR_M1MSB_VALUE( offset, value )                                   \
	_MID_WR_IND_VALUE( _PTR_M1MSB, BLK_M1MSB, 0, offset, value )

/* Read an array of values from the Current M1M Status Block */
#define MID_RD_M1MSB_ARRAY( offset, destin, count )                           \
	_MID_RD_IND_ARRAY( _PTR_M1MSB, BLK_M1MSB, 0, offset, destin, count )

/* Write an array of values to the Current M1M Status Block */
#define MID_WR_M1MSB_ARRAY( offset, source, count )                           \
	_MID_WR_IND_ARRAY( _PTR_M1MSB, BLK_M1MSB, 0, offset, source, count )

/* Poll element in the Current M1M Status Block until == 'value' */
#define MID_POLL_M1MSB_EQ( offset, value )                                    \
	_MID_POLL_IND_EQ( _PTR_M1MSB, BLK_M1MSB, 0, offset, value )

/* Poll masked element in the Current M1M Status Block until  == 'value' */
#define MID_POLL_M1MSB_EQ_MASK( offset, value, mask )                         \
	_MID_POLL_IND_EQ_MASK( _PTR_M1MSB, BLK_M1MSB, 0, offset, value, mask )

/* Poll element in the Current M1M Status Block until != 'value' */
#define MID_POLL_M1MSB_NE( offset, value, result )                            \
	_MID_POLL_IND_NE( _PTR_M1MSB, BLK_M1MSB, 0, offset, value, result )

/* Poll masked element in the Current M1M Status Block until != 'value' */
#define MID_POLL_M1MSB_NE_MASK( offset, value, result, mask )                 \
	_MID_POLL_IND_NE_MASK(_PTR_M1MSB,BLK_M1MSB,0,offset,value,result,mask)


/*---------------------------------------------------------------------------*/
/*                 M1M Status Block X (M1MSB_X) Definitions                  */
/*---------------------------------------------------------------------------*/

/* Define valid offsets for elements in an M1M Status Block (0-15) */
#define MID_M1MSB_X_CORRELATOR                    0x00000000
#define MID_M1MSB_X_WINDOW_CHAR_POSITION          0x00000001
#define MID_M1MSB_X_SCREEN_CHAR_POSITION          0x00000002
#define MID_M1MSB_X_RED_COLOR                     0x00000003
#define MID_M1MSB_X_GREEN_COLOR                   0x00000004
#define MID_M1MSB_X_BLUE_COLOR                    0x00000005
#define MID_M1MSB_X_MODELING_MATRIX               0x00000006
#define MID_M1MSB_X_PROJECTION_MATRIX             0x00000016
#define MID_M1MSB_X_RENORM_FLAG_MTX_COMP_TYPE     0x00000026
#define MID_M1MSB_X_INVERSE_TRANSPOSE_MATRIX      0x00000027
#define MID_M1MSB_X_PICK_CORRELATOR_PICK_HITS     0x00000030
#define MID_M1MSB_X_PICK_DATA_LENGTH              0x00000031
#define MID_M1MSB_X_SIZE                          0x00000032

/* Point to an element in an M1M Status Block (0-15) */
#define _PTR_M1MSB_X( adr, mode, number, offset )                             \
	_MID_POINT_IND_IND_IND( adr, mode,                                    \
	        (MID_ASCB_BASE + MID_ASCB_PTR_CSB_X + number),                \
	        MID_CSB_PTR_M1MSB, offset )

/* Read a value from an M1M Status Block (0-15) */
#define MID_RD_M1MSB_X_VALUE( number, offset, result )                        \
	_MID_RD_IND_VALUE( _PTR_M1MSB_X, BLK_M1MSB_X, number, offset, result )

/* Write a value to an M1M Status Block (0-15) */
#define MID_WR_M1MSB_X_VALUE( number, offset, value )                         \
	_MID_WR_IND_VALUE( _PTR_M1MSB_X, BLK_M1MSB_X, number, offset, value )

/* Read an array of values from an M1M Status Block (0-15) */
#define MID_RD_M1MSB_X_ARRAY( number, offset, destin, count )                 \
	_MID_RD_IND_ARRAY( _PTR_M1MSB_X,BLK_M1MSB_X,number,offset,destin,count)

/* Write an array of values to an M1M Status Block (0-15) */
#define MID_WR_M1MSB_X_ARRAY( number, offset, source, count )                 \
	_MID_WR_IND_ARRAY( _PTR_M1MSB_X,BLK_M1MSB_X,number,offset,source,count)

/* Poll element in a M1M Status Block (0-15) until == 'value' */
#define MID_POLL_M1MSB_X_EQ( number, offset, value )                          \
	_MID_POLL_IND_EQ( _PTR_M1MSB_X, BLK_M1MSB_X, number, offset, value )

/* Poll masked element in a M1M Status Block (0-15) until == 'value' */
#define MID_POLL_M1MSB_X_EQ_MASK( number, offset, value, mask )               \
	_MID_POLL_IND_EQ_MASK( _PTR_M1MSB_X, BLK_M1MSB_X, number, offset,     \
	        value, mask )

/* Poll element in a M1M Status Block (0-15) until != 'value' */
#define MID_POLL_M1MSB_X_NE( number, offset, value, result )                  \
	_MID_POLL_IND_NE( _PTR_M1MSB_X,BLK_M1MSB_X,number,offset,value,result )

/* Poll masked element in a M1M Status Block (0-15) until != 'value' */
#define MID_POLL_M1MSB_X_NE_MASK( number, offset, value, result, mask )       \
	_MID_POLL_IND_NE_MASK( _PTR_M1MSB_X, BLK_M1MSB_X, number, offset,     \
	        value, result, mask )

/*---------------------------------------------------------------------------*/
/*                  Common Indirect Access Macro Definitions                 */
/*                                                                           */
/* All macros below this point are shared by the macros above this point.    */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* This macro defines local copies of the register addresses used by the     */
/* indirect macros.  This avoids several levels of pointer chasing           */
/* required to access the global addresses of these registers by several     */
/* of the models.  The pointer chasing is required to get the address of     */
/* the first register.  Other register addresses are computed as an offset   */
/* of the first register.                                                    */
/*---------------------------------------------------------------------------*/

#define DEFINE_INDIRECT_REGISTER_ADDRESSES                                    \
	volatile ulong *_ind_control;                                         \
	volatile ulong *_ind_address;                                         \
	volatile ulong *_ind_data;                                            \
	                                                                      \
	_ind_control = MID_PTR_IND_CONTROL;                                   \
	_ind_address = _ind_control +                                         \
	        ((MID_ADR_IND_ADDRESS - MID_ADR_IND_CONTROL) >> 2);           \
	_ind_data    = _ind_control +                                         \
	        ((MID_ADR_IND_DATA    - MID_ADR_IND_CONTROL) >> 2);


/*---------------------------------------------------------------------------*/
/* Set indirect control reg = mode, indirect address reg = indadr            */
/*---------------------------------------------------------------------------*/

#define MID_LOAD_IND( adr, mode )                                             \
{                                                                             \
	MID_WR_IND_CTL( mode )                                                \
	MID_WR_IND_ADR( adr )                                                 \
}


/*---------------------------------------------------------------------------*/
/* Set the indirect control reg = mode and the indirect address reg = offset1*/
/*---------------------------------------------------------------------------*/

#define _MID_POINT_IND( adr, mode, offset1 )                                  \
{                                                                             \
	adr = offset1;                                                        \
	_MID_WR_VALUE( _ind_control, mode )                                   \
	_MID_WR_VALUE( _ind_address, adr )                                    \
}


/*---------------------------------------------------------------------------*/
/* This macro provides double indirection.  It reads the value at the        */
/* location specified by offset1, maps the value to a new                    */
/* indirect address, adds offset2 to this address, and stores the final      */
/* address in the indirect address register.  The indirect control register  */
/* is set to the mode.                                                       */
/*---------------------------------------------------------------------------*/

#define _MID_POINT_IND_IND( adr, mode, offset1, offset2 )                     \
{                                                                             \
	if ( (mode & MID_IND_MODE_MASK) == MID_IND_RD_AI )                    \
	        _MID_WR_VALUE( _ind_control, MID_IND_RD_AI )                  \
	else                                                                  \
	        _MID_WR_VALUE( _ind_control, MID_IND_RD_NI )                  \
	_MID_WR_VALUE( _ind_address, offset1 )                                \
	_MID_RD_VALUE( _ind_data, adr )                                       \
	adr = (adr & MID_MAX_DSP_IND_ADR) + offset2;                          \
	_MID_WR_VALUE( _ind_address, adr)                                     \
	_MID_WR_VALUE( _ind_control, mode )                                   \
}


/*---------------------------------------------------------------------------*/
/* This macro provides triple indirection.  It reads the value at the        */
/* location specified by offset1, maps the value to a new                    */
/* indirect address, adds offset2 to this address, reads the value at that   */
/* address, maps the value to a new indirect address, adds offset3 to this   */
/* address, and stores the final address in the indirect address reg.  The   */
/* mode is stored in the indirect control register.                          */
/*---------------------------------------------------------------------------*/

#define _MID_POINT_IND_IND_IND( adr, mode, offset1, offset2, offset3)         \
{                                                                             \
	if ( (mode & MID_IND_MODE_MASK) == MID_IND_RD_AI )                    \
	        _MID_WR_VALUE( _ind_control, MID_IND_RD_AI )                  \
	else                                                                  \
	        _MID_WR_VALUE( _ind_control, MID_IND_RD_NI )                  \
	_MID_WR_VALUE( _ind_address, offset1 )                                \
	_MID_RD_VALUE( _ind_data, adr )                                       \
	adr = (adr & MID_MAX_DSP_IND_ADR) + offset2;                          \
	_MID_WR_VALUE( _ind_address, adr )                                    \
	_MID_RD_VALUE( _ind_data, adr )                                       \
	adr = (adr & MID_MAX_DSP_IND_ADR) + offset3;                          \
	_MID_WR_VALUE( _ind_address, adr)                                     \
	_MID_WR_VALUE( _ind_control, mode )                                   \
}


/*---------------------------------------------------------------------------*/
/* This macro maps a DSP address into a valid indirect address.             */
/*---------------------------------------------------------------------------*/

#define MID_MAP_IND_ADR( out_addr, in_addr )                                  \
	out_addr = (int) in_addr & MID_MAX_DSP_IND_ADR;


/*---------------------------------------------------------------------------*/
/* This macro forms a trace parameter for indirect access macros.            */
/*---------------------------------------------------------------------------*/

#define MID_IND_PARM( block, number, offset )                                 \
	(((block) | (number << 8) | (offset)) << 16)


/*---------------------------------------------------------------------------*/
/* This macro will read a value from the adapter and assign it to result.    */
/* It uses the point function, number, and offset to point the indirect      */
/* address register to the correct adapter source location.                  */
/*---------------------------------------------------------------------------*/

#define _MID_RD_IND_VALUE( point_func, block, number, offset, result )        \
{                                                                             \
	ulong   _ind_adr;                                                     \
	                                                                      \
	DEFINE_INDIRECT_REGISTER_ADDRESSES                                    \
	PROTECT_INDIRECT_ACCESS                                               \
	point_func( _ind_adr, MID_IND_RD_NI, number, offset )                 \
	_MID_RD_VALUE( _ind_data, result )                                    \
	MID_RD_TRACE( MID_ADR_IND_DATA | MID_IND_PARM(block, number, offset), \
	        result )                                                      \
	UNPROTECT_INDIRECT_ACCESS                                             \
}


/*---------------------------------------------------------------------------*/
/* This macro will write the specified value to the adapter memory.          */
/* It uses the point function, number, and offset to point the indirect      */
/* address register to the correct adapter destination location.             */
/*---------------------------------------------------------------------------*/

#define _MID_WR_IND_VALUE( point_func, block, number, offset, value )         \
{                                                                             \
	ulong   _ind_adr;                                                     \
	                                                                      \
	DEFINE_INDIRECT_REGISTER_ADDRESSES                                    \
	PROTECT_INDIRECT_ACCESS                                               \
	point_func( _ind_adr, MID_IND_WR_NI, number, offset )                 \
	_MID_WR_VALUE( _ind_data, value )                                     \
	MID_WR_TRACE( MID_ADR_IND_DATA | MID_IND_PARM(block, number, offset), \
	        value )                                                       \
	UNPROTECT_INDIRECT_ACCESS                                             \
}


/*---------------------------------------------------------------------------*/
/* This macro will read an array of count values from the adapter.  It will  */
/* store the values beginning at the destin location.  It uses the point     */
/* function, number, and offset to point the indirect address register to    */
/* the correct adapter source location.                                      */
/*---------------------------------------------------------------------------*/

#define _MID_RD_IND_ARRAY( point_func, block, number, offset, destin, count ) \
{                                                                             \
	ulong   _ind_adr;                                                     \
	ulong * _ind_ptr;                                                     \
	ulong * _ind_end;                                                     \
	                                                                      \
	DEFINE_INDIRECT_REGISTER_ADDRESSES                                    \
	PROTECT_INDIRECT_ACCESS                                               \
	point_func( _ind_adr, MID_IND_RD_AI, number, offset )                 \
	_ind_ptr = ( ulong * ) (destin);                                      \
	_ind_end = _ind_ptr + count;                                          \
	while (_ind_ptr < _ind_end)                                           \
	{                                                                     \
	        _MID_RD_VALUE( _ind_data, *_ind_ptr )                         \
	        _ind_ptr++;                                                   \
	}                                                                     \
	MID_RD_TRACE_IRD( destin, count, MID_IND_PARM(block, number, offset)) \
	UNPROTECT_INDIRECT_ACCESS                                             \
}


/*---------------------------------------------------------------------------*/
/* This macro will write an array of count values to the adapter.  It will   */
/* read the values beginning from the source location.  It uses the point    */
/* function, number, and offset to point the indirect address register to    */
/* the correct adapter source location.                                      */
/*---------------------------------------------------------------------------*/

#define _MID_WR_IND_ARRAY( point_func, block, number, offset, source, count ) \
{                                                                             \
	ulong   _ind_adr;                                                     \
	ulong * _ind_ptr;                                                     \
	ulong * _ind_end;                                                     \
	                                                                      \
	DEFINE_INDIRECT_REGISTER_ADDRESSES                                    \
	PROTECT_INDIRECT_ACCESS                                               \
	point_func( _ind_adr, MID_IND_WR_AI, number, offset )                 \
	_ind_ptr = ( ulong * ) (source);                                      \
	_ind_end = _ind_ptr + count;                                          \
	while (_ind_ptr < _ind_end)                                           \
	{                                                                     \
	        _MID_WR_VALUE( _ind_data, *_ind_ptr )                         \
	        _ind_ptr++;                                                   \
	}                                                                     \
	MID_WR_TRACE_IWR( source, count, MID_IND_PARM(block, number, offset)) \
	UNPROTECT_INDIRECT_ACCESS                                             \
}


/*---------------------------------------------------------------------------*/
/* This macro will poll an element on the adapter until its value is         */
/* equal to the value specified.  It uses the point function, number, and    */
/* offset to point the indirect address register to the correct adapter      */
/* location.  The first value which is not equal is returned in result.      */
/*---------------------------------------------------------------------------*/

#define _MID_POLL_IND_EQ( point_func, block, number, offset, value )          \
{                                                                             \
	ulong   _ind_adr;                                                     \
	ulong   _ind_result;                                                  \
	                                                                      \
	DEFINE_INDIRECT_REGISTER_ADDRESSES                                    \
	PROTECT_INDIRECT_ACCESS                                               \
	point_func( _ind_adr, MID_IND_RD_NI, number, offset )                 \
	MID_EQ_TRACE( MID_ADR_IND_DATA |                                      \
	        MID_IND_PARM( block, number, offset ), value, -1 )            \
	_MID_POLL_VALUE( _ind_data, _ind_result, MID_EQ_TEST, value )         \
	MID_FV_TRACE( MID_ADR_IND_DATA, value )                               \
	UNPROTECT_INDIRECT_ACCESS                                             \
}


/*---------------------------------------------------------------------------*/
/* This macro will poll an element on the adapter until its value ANDED      */
/* with the mask is equal to the value specified.  It uses the point         */
/* function, number, and offset to point the indirect address register       */
/* to the correct adapter location.  The first value which is not equal      */
/* is returned in result.                                                    */
/*---------------------------------------------------------------------------*/

#define _MID_POLL_IND_EQ_MASK( point_func, block, number, offset, value, mask)\
{                                                                             \
	ulong   _ind_adr;                                                     \
	ulong   _ind_result;                                                  \
	                                                                      \
	DEFINE_INDIRECT_REGISTER_ADDRESSES                                    \
	PROTECT_INDIRECT_ACCESS                                               \
	point_func( _ind_adr, MID_IND_RD_NI, number, offset )                 \
	MID_EQ_TRACE( MID_ADR_IND_DATA |                                      \
	        MID_IND_PARM( block, number, offset ), value, mask )          \
	_MID_POLL_VALUE_MASK( _ind_data, _ind_result, MID_EQ_TEST,            \
	        value, mask )                                                 \
	MID_FV_TRACE( MID_ADR_IND_DATA, (value & mask) )                      \
	UNPROTECT_INDIRECT_ACCESS                                             \
}


/*---------------------------------------------------------------------------*/
/* This macro will poll an element on the adapter until its value ANDED      */
/* with the mask is equal to the value specified or until the adapter has    */
/* been polled "time" number of times.  It uses the point function           */
/* and offset to point the indirect address register to the correct          */
/* adapter location.                                                         */
/*---------------------------------------------------------------------------*/

#define _MID_POLL_IND_EQ_MASK_TO( point_func, block, number, offset, value,   \
	mask, time )                                                          \
{                                                                             \
	ulong   _ind_adr;                                                     \
	ulong   _ind_result;                                                  \
	                                                                      \
	DEFINE_INDIRECT_REGISTER_ADDRESSES                                    \
	PROTECT_INDIRECT_ACCESS                                               \
	point_func( _ind_adr, MID_IND_RD_NI, number, offset )                 \
	MID_EQ_TRACE( MID_ADR_IND_DATA |                                      \
	        MID_IND_PARM( block, number, offset ), value, mask )          \
	_MID_POLL_VALUE_MASK_TO( _ind_data, _ind_result, MID_EQ_TEST,         \
	        value, mask, time )                                           \
	MID_FV_TRACE( MID_ADR_IND_DATA, (value & mask) )                      \
	UNPROTECT_INDIRECT_ACCESS                                             \
}


/*---------------------------------------------------------------------------*/
/* This macro will poll an element on the adapter until its value is not     */
/* equal to the value specified.  It uses the point function, number, and    */
/* offset to point the indirect address register to the correct adapter      */
/* location.  The first value which is not equal is returned in result.      */
/*---------------------------------------------------------------------------*/

#define _MID_POLL_IND_NE( point_func, block, number, offset, value, result )  \
{                                                                             \
	ulong   _ind_adr;                                                     \
	                                                                      \
	DEFINE_INDIRECT_REGISTER_ADDRESSES                                    \
	PROTECT_INDIRECT_ACCESS                                               \
	point_func( _ind_adr, MID_IND_RD_NI, number, offset )                 \
	MID_NE_TRACE( MID_ADR_IND_DATA |                                      \
	        MID_IND_PARM( block, number, offset ), value, -1 )            \
	_MID_POLL_VALUE( _ind_data, result, MID_NE_TEST, value )              \
	MID_FV_TRACE( MID_ADR_IND_DATA, value )                               \
	UNPROTECT_INDIRECT_ACCESS                                             \
}


/*---------------------------------------------------------------------------*/
/* This macro will poll an element on the adapter until its value ANDED      */
/* with the mask is not equal to the value specified.  It uses the point     */
/* function, number, and offset to point the indirect address register       */
/* to the correct adapter location.  The first value which is not equal      */
/* is returned in result.                                                    */
/*---------------------------------------------------------------------------*/

#define _MID_POLL_IND_NE_MASK( point_func, block, number, offset, value,      \
	result, mask )                                                        \
{                                                                             \
	ulong   _ind_adr;                                                     \
	                                                                      \
	DEFINE_INDIRECT_REGISTER_ADDRESSES                                    \
	PROTECT_INDIRECT_ACCESS                                               \
	point_func( _ind_adr, MID_IND_RD_NI, number, offset )                 \
	MID_NE_TRACE( MID_ADR_IND_DATA |                                      \
	        MID_IND_PARM( block, number, offset ), value, mask )          \
	_MID_POLL_VALUE_MASK( _ind_data,result,MID_NE_TEST,value,mask )       \
	MID_FV_TRACE( MID_ADR_IND_DATA, (value & mask) )                      \
	UNPROTECT_INDIRECT_ACCESS                                             \
}


/*---------------------------------------------------------------------------*/
/* This macro reads num_words of data from the indirect data port and stores */
/* them in the location pointed to by pData.                                 */
/*                                                                           */
/* Prerequisites:                                                            */
/*   User must initialize the indirect address and control registers.        */
/*   User must ensure that either they own the PCB lock or that nobody owns  */
/*     the PCB lock and interrupts are disabled.                             */
/*---------------------------------------------------------------------------*/

#define MID_RD_IND( pData, num_words )                                        \
{                                                                             \
	ulong *         _ind_ptr;                                             \
	ulong *         _ind_end;                                             \
	volatile ulong *_ind_data = MID_PTR_IND_DATA;                         \
	                                                                      \
	_ind_ptr = ( ulong * ) (pData);                                       \
	_ind_end = _ind_ptr + num_words;                                      \
	while (_ind_ptr < _ind_end)                                           \
	{                                                                     \
	        _MID_RD_VALUE( _ind_data, *_ind_ptr )                         \
	        _ind_ptr++;                                                   \
	}                                                                     \
	MID_RD_TRACE_IRD( pData, num_words, 0 )                               \
}


/*---------------------------------------------------------------------------*/
/* This macro writes num_words of data from the the location pointed to by   */
/* pData to the indirect data port.                                          */
/*                                                                           */
/* Prerequisites:                                                            */
/*   User must initialize the indirect address and control registers.        */
/*   User must ensure that either they own the PCB lock or that nobody owns  */
/*     the PCB lock and interrupts are disabled.                             */
/*---------------------------------------------------------------------------*/

#define MID_WR_IND( pData, num_words )                                        \
{                                                                             \
	ulong *         _ind_ptr;                                             \
	ulong *         _ind_end;                                             \
	volatile ulong *_ind_data = MID_PTR_IND_DATA;                         \
	                                                                      \
	_ind_ptr = ( ulong * ) (pData);                                       \
	_ind_end = _ind_ptr + num_words;                                      \
	while (_ind_ptr < _ind_end)                                           \
	{                                                                     \
	        _MID_WR_VALUE( _ind_data, *_ind_ptr )                         \
	        _ind_ptr++;                                                   \
	}                                                                     \
	MID_WR_TRACE_IWR( pData, num_words, 0 )                               \
}

#endif  /* _H_MID_HW_IND_MAC */
