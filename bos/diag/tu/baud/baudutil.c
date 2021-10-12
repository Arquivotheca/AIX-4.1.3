static char sccsid[] = "@(#)79	1.2  src/bos/diag/tu/baud/baudutil.c, tu_baud, bos411, 9439B411a 9/29/94 10:57:18";
/*
 * COMPONENT_NAME: tu_baud
 *
 * FUNCTIONS: write_to_base_count, get_base_count, read_control_regs
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "baudregs.h"
#include "baud_exectu.h"

/******************************************************************************
 * NAME: wr(data,reg)
 *
 * FUNCTION: This function writes data to an adapter register.
 *
 * EXECUTION ENVIRONMENT: This procedure executes under a process.
 */


void wr(char data, char reg)
{
extern   char  *ioa;

*(ioa + reg) = data;
} /* wr */

/******************************************************************************
 * NAME: rd(reg)
 *
 * FUNCTION: This function reads data from an adapter register.
 *
 * EXECUTION ENVIRONMENT: This procedure executes under a process.
 */


char rd(char reg)
{
extern   char  *ioa;

return(*(ioa + reg));
} /* wr */


/*
 *   NAME: default_values()
 *        Gives default values to AUDIO_REC audio structure
 */
void default_values()
{
  audio.audio_mode    = MODE1;
  audio.transfer_type = DUAL_DMA;
  audio.dma_operation = CAPTURE;
  audio.dma_cycle     = SINGLE_CYCLE;
  audio.cycle         = 0;
  audio.bytes         = 0;
  audio.sample_rate   = KHZ8o0;        /*  8.0 KHz (XTAL1, cfs = 0)      */
  audio.clock         = XTAL1;         /* Select sample freq             */
  audio.cfs           = 0;             /*   by choose XTAL1 and cfs      */
  audio.type          = STEREO;        /* Stereo is selected for default */
  audio.data_format   = L_8;           /* 8-bit unsigned linear PCM data */
  audio.data          = LINEAR_8;      /* char value for this data format*/
  audio.isource       = LINE;
  audio.l_input       = 0;             /* char value for Line Source     */
  audio.r_input       = 0;             /* char value for Line Source     */
  audio.l_20db        = DISABLE;       /* Left Mic 20dB gain Enable      */
  audio.r_20db        = DISABLE;       /* Right Mic 20dB gain Enable     */
  audio.in_gain       = 0;
  audio.l_gain        = 0;     /* Max gain 22.5dB (15steps: +1.5dB/step) */
  audio.r_gain        = 0;     /* Max gain 22.5dB (15steps: +1.5dB/step) */
  audio.aux1_attn     = MUTE;
  audio.l_aux1        = MUTE;  /* 0 = 12dB, 8 = 0dB, 31 = -34.5dB        */
  audio.r_aux1        = MUTE;  /* 0 = 12dB, 8 = 0dB, 31 = -34.5dB        */
  audio.out_attn      = 0;
  audio.l_out         = 0;     /* Max gain -94.5dB, (63 x (-1.5dB/step)) */
  audio.r_out         = 0;     /* Max gain -94.5dB, (63 x (-1.5dB/step)) */
  audio.dme           = DISABLE;        /* Digital Mix is disabled       */
  audio.dm_attn       = 0;              /* Digital Mix attenuation       */
  audio.hp_filter     = DISABLE;
  audio.line_in_gain  = MUTE;
  audio.lline_in      = MUTE;
  audio.rline_in      = MUTE;
  audio.mono_in       = MUTE;
  audio.mono_out      = MONO_O_MUTE;
  audio.playback_cycle = 0;

} /* end of default_values() */


/*
 *   write_to_base_count(), get_base_count()
 *        This function writes number of transferred samples per DMA into
 *        Lower and Upper Base Count registers. Internal interrupt will
 *        be generated when this number is decremented to zero and next
 *        sample period starts to be transferred.
 *   ---------------------------------------------------------------------
 */
void write_to_base_count_regs(unsigned int buffer_size)
{
  unsigned int bytes_per_sample, samples_per_transfer;

  if (audio.type == STEREO) {
    if (audio.data == LINEAR_16) bytes_per_sample = 4;
    else                          bytes_per_sample = 2;
  } else {
    if (audio.data == LINEAR_16) bytes_per_sample = 2;
    else                          bytes_per_sample = 1;
  } /* endif */

  samples_per_transfer = (buffer_size / bytes_per_sample) - 1;  /* buffer_size is   */
                                                                /* divisible to 4   */
  wr(PLAYBACK_LOCOUNT_REG, AUDIO_INDEX_REG);
  wr( (samples_per_transfer & 0xff), AUDIO_INDEXED_DATA_REG);
  wr(PLAYBACK_UPCOUNT_REG, AUDIO_INDEX_REG);
  wr( ((samples_per_transfer & 0xff00) >> 8), AUDIO_INDEXED_DATA_REG);

  if (audio.audio_mode == MODE2) {
    wr(CAPTURE_LOCOUNT_REG, AUDIO_INDEX_REG);
    wr( (samples_per_transfer & 0xff), AUDIO_INDEXED_DATA_REG);
    wr(CAPTURE_UPCOUNT_REG, AUDIO_INDEX_REG);
    wr( ((samples_per_transfer & 0xff00) >> 8), AUDIO_INDEXED_DATA_REG);
  }

} /* end of write_to_base_count_regs() */

int  get_base_count(unsigned int buffer_size)
{
  unsigned int bytes_per_sample, samples_per_transfer;

  if (audio.type == STEREO) {
    if (audio.data == LINEAR_16) bytes_per_sample = 4;
    else                          bytes_per_sample = 2;
  } else {
    if (audio.data == LINEAR_16) bytes_per_sample = 2;
    else                          bytes_per_sample = 1;
  } /* endif */

  samples_per_transfer = (buffer_size / bytes_per_sample) - 1;  /* buffer_size is   */
                                                                /* divisible to 4   */
  return(samples_per_transfer);

} /* end of get_base_count() */


/*
 *   =====================================================================
 *   4.   c o n t r o l _ r e g s _ r e s e t ( )
 *   =====================================================================
 *        Resets all AD1848 control registers to intial state.
 *   ---------------------------------------------------------------------
 */
void control_regs_reset()
{
  wr(L_ADC_INPUT_REG, AUDIO_INDEX_REG);
  wr(0x00, AUDIO_INDEXED_DATA_REG);
  wr(R_ADC_INPUT_REG, AUDIO_INDEX_REG);
  wr(0x00, AUDIO_INDEXED_DATA_REG);
  wr(LAUX1_INPUT_REG, AUDIO_INDEX_REG);
  wr(0x80, AUDIO_INDEXED_DATA_REG);
  wr(RAUX1_INPUT_REG, AUDIO_INDEX_REG);
  wr(0x80, AUDIO_INDEXED_DATA_REG);
  wr(LAUX2_INPUT_REG, AUDIO_INDEX_REG);
  wr(0x80, AUDIO_INDEXED_DATA_REG);
  wr(RAUX2_INPUT_REG, AUDIO_INDEX_REG);
  wr(0x80, AUDIO_INDEXED_DATA_REG);
  wr(L_DAC_OUTPUT_REG, AUDIO_INDEX_REG);
  wr(0x80, AUDIO_INDEXED_DATA_REG);
  wr(R_DAC_OUTPUT_REG, AUDIO_INDEX_REG);
  wr(0x80, AUDIO_INDEXED_DATA_REG);
/*  wr((MCE_SET | FS_PLAYBACK_DATA_REG), AUDIO_INDEX_REG);
  wr(0x00, AUDIO_INDEXED_DATA_REG);
  wr((MCE_SET | INTERFACE_REG), AUDIO_INDEX_REG);
  wr(0x08, AUDIO_INDEXED_DATA_REG);    */
  wr(PIN_CNTL_REG, AUDIO_INDEX_REG);
  wr(0x00, AUDIO_INDEXED_DATA_REG);
  wr((ERR_STATUS_INIT_REG), AUDIO_INDEX_REG);
  wr(0x00, AUDIO_INDEXED_DATA_REG);
  wr(LOOPBACK_CNTL_REG, AUDIO_INDEX_REG);
  wr(0x00, AUDIO_INDEXED_DATA_REG);
  wr(PLAYBACK_LOCOUNT_REG, AUDIO_INDEX_REG);
  wr(0x00, AUDIO_INDEXED_DATA_REG);
  wr(PLAYBACK_UPCOUNT_REG, AUDIO_INDEX_REG);
  wr(0x00, AUDIO_INDEXED_DATA_REG);

  if (audio.audio_mode = MODE2) {
    wr(MODE_ID_REG, AUDIO_INDEX_REG);
    wr(0x8A, AUDIO_INDEXED_DATA_REG);
    wr(L_LINE_INPUT_REG, AUDIO_INDEX_REG);
    wr(0x88, AUDIO_INDEXED_DATA_REG);
    wr(R_LINE_INPUT_REG, AUDIO_INDEX_REG);
    wr(0x88, AUDIO_INDEXED_DATA_REG);
  }

  wr(0xcc, AUDIO_STATUS_REG);

} /* end of control_regs_reset() */


/**********************************************************************
 *       r e c o r d i n g _ t i m e ( )
 *        Returns number of seconds used to record music data into memory.
 */
int recording_time(RECORD_SIZE* rec_size)
{
  int secs_per_mb, total_time;

  if ( audio.transfer_type == CP_PIO ) {
    switch (audio.sample_rate) {
    case KHZ5o5152:
      secs_per_mb = 96;
      break;
    case KHZ6o615:
      secs_per_mb = 80;
      break;
    case KHZ8o0:
      secs_per_mb = 66;
      break;
    case KHZ9o6:
      secs_per_mb = 54;
      break;
    case KHZ11o025:
      secs_per_mb = 48;
      break;
    case KHZ16o0:
      secs_per_mb = 32;
      break;
    case KHZ18o9:
      secs_per_mb = 28;
      break;
    case KHZ22o05:
      secs_per_mb = 24;
      break;
    case KHZ27o42857:
      secs_per_mb = 20;
      break;
    case KHZ32o0:
      secs_per_mb = 16;
      break;
    case KHZ33o075:
      secs_per_mb = 16;
      break;
    case KHZ37o8:
      secs_per_mb = 14;
      break;
    case KHZ44o1:
      secs_per_mb = 12;
      break;
    case KHZ48o0:
      secs_per_mb = 10;
      break;
    } /* endswitch */
  } else {
    secs_per_mb = 2;
  } /* endif */

  if (audio.data == LINEAR_16) secs_per_mb = secs_per_mb / 2;
  if (audio.type == MONO)      secs_per_mb = 2*secs_per_mb;

  total_time = (secs_per_mb)*(rec_size->size);
  return(total_time);

} /* end of recording_time() */






/**********************************************************************
 *       read_control_regs()
 *        Displays all codec register values.
 */

int  read_control_regs()
{
  unsigned char index[32];
  extern char  *pos;

  printf("DIRECT AND POS R E G I S T E R S\r\n");
  printf("MCI POS reg  0     0x%02X    ", *(pos + 0));
  printf("MCI POS reg  1     0x%02X\r\n", *(pos + 1));
  printf("MCI POS reg  2     0x%02X    ", *(pos + 2));
  printf("MCI POS reg  3     0x%02X\r\n", *(pos + 3));
  printf("MCI POS reg  4     0x%02X    ", *(pos + 4));
  printf("MCI POS reg  5     0x%02X\r\n", *(pos + 5));
  printf("MCI Register 0     0x%02X    ", rd(MCI_REG0));
  printf("MCI Register 1     0x%02X\r\n", rd(MCI_REG1));
  printf("MCI Intr En (2)    0x%02X    ", rd(MCI_INTR_EN));
  printf("MCI Intr St (3)    0x%02X\r\n", rd(MCI_INTR_ST));
  printf("MCI Reg 4 = 0x%02X  Reg 5 = 0x%02X\r\n", rd(MCI_TIMER),rd(MCI_REG5));
  printf("Index Addr  (8)    0x%02X    ", rd(AUDIO_INDEX_REG));
  printf("Indexed Data(9)    0x%02X\r\n", rd(AUDIO_INDEXED_DATA_REG));
  printf("Status Reg (10)    0x%02X    ", rd(AUDIO_STATUS_REG));
  printf("PIO Data   (11)    0x%02X\r\n", rd(AUDIO_PIO_DATA_REG));

  wr(L_ADC_INPUT_REG, AUDIO_INDEX_REG);
  index[0]  = rd(AUDIO_INDEXED_DATA_REG);
  wr(R_ADC_INPUT_REG, AUDIO_INDEX_REG);
  index[1]  = rd(AUDIO_INDEXED_DATA_REG);
  wr(LAUX1_INPUT_REG, AUDIO_INDEX_REG);
  index[2]  = rd(AUDIO_INDEXED_DATA_REG);
  wr(RAUX1_INPUT_REG, AUDIO_INDEX_REG);
  index[3]  = rd(AUDIO_INDEXED_DATA_REG);
  wr(LAUX2_INPUT_REG, AUDIO_INDEX_REG);
  index[4]  = rd(AUDIO_INDEXED_DATA_REG);
  wr(RAUX2_INPUT_REG, AUDIO_INDEX_REG);
  index[5]  = rd(AUDIO_INDEXED_DATA_REG);
  wr(L_DAC_OUTPUT_REG, AUDIO_INDEX_REG);
  index[6]  = rd(AUDIO_INDEXED_DATA_REG);
  wr(R_DAC_OUTPUT_REG, AUDIO_INDEX_REG);
  index[7]  = rd(AUDIO_INDEXED_DATA_REG);
  wr(FS_PLAYBACK_DATA_REG, AUDIO_INDEX_REG);
  index[8]  = rd(AUDIO_INDEXED_DATA_REG);
  wr(INTERFACE_REG, AUDIO_INDEX_REG);
  index[9]  = rd(AUDIO_INDEXED_DATA_REG);
  wr(PIN_CNTL_REG, AUDIO_INDEX_REG);
  index[10] = rd(AUDIO_INDEXED_DATA_REG);
  wr(ERR_STATUS_INIT_REG, AUDIO_INDEX_REG);
  index[11] = rd(AUDIO_INDEXED_DATA_REG);
  wr(MODE_ID_REG, AUDIO_INDEX_REG);
  index[12] = rd(AUDIO_INDEXED_DATA_REG);
  wr(LOOPBACK_CNTL_REG, AUDIO_INDEX_REG);
  index[13] = rd(AUDIO_INDEXED_DATA_REG);
  wr(PLAYBACK_UPCOUNT_REG, AUDIO_INDEX_REG);
  index[14] = rd(AUDIO_INDEXED_DATA_REG);
  wr(PLAYBACK_LOCOUNT_REG, AUDIO_INDEX_REG);
  index[15] = rd(AUDIO_INDEXED_DATA_REG);

  wr(MODE_ID_REG, AUDIO_INDEX_REG);
  wr(0xCA, AUDIO_INDEXED_DATA_REG);

  wr(ALT_FEATURE_1_REG, AUDIO_INDEX_REG);
  index[16] = rd(AUDIO_INDEXED_DATA_REG);
  wr(ALT_FEATURE_2_REG, AUDIO_INDEX_REG);
  index[17] = rd(AUDIO_INDEXED_DATA_REG);
  wr(L_LINE_INPUT_REG, AUDIO_INDEX_REG);
  index[18] = rd(AUDIO_INDEXED_DATA_REG);
  wr(R_LINE_INPUT_REG, AUDIO_INDEX_REG);
  index[19] = rd(AUDIO_INDEXED_DATA_REG);
  wr(TIMER_LOBYTE_REG, AUDIO_INDEX_REG);
  index[20] = rd(AUDIO_INDEXED_DATA_REG);
  wr(TIMER_HIBYTE_REG, AUDIO_INDEX_REG);
  index[21] = rd(AUDIO_INDEXED_DATA_REG);
  wr( 22, AUDIO_INDEX_REG);
  index[22] = rd(AUDIO_INDEXED_DATA_REG);
  wr( 23, AUDIO_INDEX_REG);
  index[23] = rd(AUDIO_INDEXED_DATA_REG);
  wr(ALT_FEATURE_STAT_REG, AUDIO_INDEX_REG);
  index[24] = rd(AUDIO_INDEXED_DATA_REG);
  wr(VERSION_CHIP_ID_REG, AUDIO_INDEX_REG);
  index[25] = rd(AUDIO_INDEXED_DATA_REG);
  wr(MONO_IO_CNTL_REG, AUDIO_INDEX_REG);
  index[26] = rd(AUDIO_INDEXED_DATA_REG);
  wr( 27, AUDIO_INDEX_REG);
  index[27] = rd(AUDIO_INDEXED_DATA_REG);
  wr(CAPTURE_DATA_REG, AUDIO_INDEX_REG);
  index[28] = rd(AUDIO_INDEXED_DATA_REG);
  wr( 29, AUDIO_INDEX_REG);
  index[29] = rd(AUDIO_INDEXED_DATA_REG);
  wr(CAPTURE_UPCOUNT_REG, AUDIO_INDEX_REG);
  index[30] = rd(AUDIO_INDEXED_DATA_REG);
  wr(CAPTURE_LOCOUNT_REG, AUDIO_INDEX_REG);
  index[31] = rd(AUDIO_INDEXED_DATA_REG);

  printf("I0 : 0x%02X     I8 : 0x%02X     I16: 0x%02X     I24: 0x%02X\r\n",
               index[0], index[8], index[16], index[24]);
  printf("I1 : 0x%02X     I9 : 0x%02X     I17: 0x%02X     I25: 0x%02X\r\n",
               index[1], index[9], index[17], index[25]);
  printf("I2 : 0x%02X     I10: 0x%02X     I18: 0x%02X     I26: 0x%02X\r\n",
               index[2], index[10], index[18], index[26]);
  printf("I3 : 0x%02X     I11: 0x%02X     I19: 0x%02X     I27: 0x%02X\r\n",
               index[3], index[11], index[19], index[27]);
  printf("I4 : 0x%02X     I12: 0x%02X     I20: 0x%02X     I28: 0x%02X\r\n",
               index[4], index[12], index[20], index[28]);
  printf("I5 : 0x%02X     I13: 0x%02X     I21: 0x%02X     I29: 0x%02X\r\n",
               index[5], index[13], index[21], index[29]);
  printf("I6 : 0x%02X     I14: 0x%02X     I22: 0x%02X     I30: 0x%02X\r\n",
               index[6], index[14], index[22], index[30]);
  printf("I7 : 0x%02X     I15: 0x%02X     I23: 0x%02X     I31: 0x%02X\r\n",
               index[7], index[15], index[23], index[31]);
  return(0);
}

