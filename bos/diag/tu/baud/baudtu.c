static char sccsid[]= "@(#)94  1.3  R2/cmd/diag/tu/baud/baudtu.c,  tu320 6/6/91 14:15:59";
/*
 * COMPONENT_NAME: TU code for Business Audio Adapter
 *
 * FUNCTIONS: wr,rd,get_vpd, calibrate, sample_rate,
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
/*
 * MODULE NAME: baudtu.c
 *
 * DEPENDENCIES:
 *   1. The Baud device driver must be installed.
 *
 * RESTRICTIONS: None.
 *
 * EXTERNAL REFERENCES
 *   OTHER ROUTINES:
 *
 *****************************************************************************/


#include "baud_exectu.h"    /* MAIN HEADER */
#include "baudregs.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/errno.h>

extern int dd_FileDes;    /* DEFINITION IN exectu.h */


/******************************************************************************
 * NAME: get_vpd
 *
 * FUNCTION: This function reads the VPD of the card and returns it in the
 *           secondary error return registers.
 *
 * EXECUTION ENVIRONMENT: This procedure executes under a process.
 */


int get_vpd(int   sec_ptr[])
{
int
    RetCode;   /* GENERAL PURPOSE RETURN CODE; ASSUME SUCCESS */
return(0);

} /* get_vpd */


/******************************************************************************
 * NAME: cs4231_ready
 *
 * FUNCTION: This function waits for a ready indication from the
 *           CS4231 Codec.
 *
 * EXECUTION ENVIRONMENT: This procedure executes under a process.
 */

/****************************************************************************** */
int   wait_audio_ready()
{
unsigned int   ready;
int   timeout = 0x100000;

do
 {
 ready = rd(AUDIO_INDEX_REG);
 }while( (ready == 0x80) && (--timeout > 0));

if(ready != 0x80) ready = 0;
return(ready);
}


/* NAME: calibrate
 *
 * FUNCTION: This function performs the self-calibration function of the
 *           CS4231 Codec.
 *
 * EXECUTION ENVIRONMENT: This procedure executes under a process.
 */


int calibrate()
{
unsigned int   status, aci_progress;
int            timeout;
int            error = 0;


switch (audio.transfer_type) {
  case CP_PIO:
    wr((MCE_SET | INTERFACE_REG), AUDIO_INDEX_REG);
    wr(0xc8, AUDIO_INDEXED_DATA_REG);        /* enable CPIO-PPIO, set ACAL */
    wr(0x00, AUDIO_INDEX_REG);
    status = wait_audio_ready() | 3;
    break;
  case DUAL_DMA:
    wr((MCE_SET | INTERFACE_REG), AUDIO_INDEX_REG);
    wr(0x08, AUDIO_INDEXED_DATA_REG);                 /* set ACAL only     */
    wr(0x00, AUDIO_INDEX_REG);
    status = wait_audio_ready() | 3;
    break;
  case SINGLE_DMA:
    wr((MCE_SET | INTERFACE_REG), AUDIO_INDEX_REG);
    wr(0x0c, AUDIO_INDEXED_DATA_REG);                 /* set ACAL, set SDC */
    wr(0x00, AUDIO_INDEX_REG);
    status = wait_audio_ready() | 3;
    break;
  } /* endswitch */

     wr(ERR_STATUS_INIT_REG, AUDIO_INDEX_REG);

  /* Waits while autocalibration is not in progress until it in progress
   * --------------------------------------------------------------------- */
     timeout = 0x100000;
     do {
       aci_progress = rd(AUDIO_INDEXED_DATA_REG);
     } while ( ((aci_progress & 0x20) == 0) && (--timeout > 0) ); /* enddo */
     if (timeout == 0) status |= (aci_progress | 4);

  /* Waits while autocalibration is in progress until it is completed
   * --------------------------------------------------------------------- */
     timeout = 0x100000;
     do {
       aci_progress = rd(AUDIO_INDEXED_DATA_REG);
     } while ( ((aci_progress & 0x20) == 1) && (--timeout > 0) ); /* enddo */
     if (timeout == 0) status |= (aci_progress | 5);

return(error);

} /* calibrate */


/**************************************************************************
 *   NAME: audio_setup
 *        Waits for AD1848 to complete initialization then writes to
 *        control registers to setup test, does auto-calibration, sets up
 *        gain/attenuation of auxiliary lines and output lines and of
 *        digital mixing.
 */
int audio_setup(int secondary_ptr[])
  {
  unsigned int status;
  unsigned char index[32], capture_data;

  status = wait_audio_ready() | 1;     /* wait until CS4231 completes */
                                       /* initialization and ready    */


   if (audio.audio_mode == MODE2) {
     wr(MODE_ID_REG, AUDIO_INDEX_REG);    /* Set to MODE 2               */
     wr(0xCA, AUDIO_INDEXED_DATA_REG);
   }

 /* Setup data format, mono / stereo, clock frequency divide select,
  * and crystal clock by writing into Clock & Data Format Register
  * ------------------------------------------------------------------ */
    index[8] = audio.data | audio.type | (audio.cfs << 1) | audio.clock;
    wr((MCE_SET | FS_PLAYBACK_DATA_REG), AUDIO_INDEX_REG);
    wr(index[8], AUDIO_INDEXED_DATA_REG);
    status |= (wait_audio_ready() | 2);

    status |= calibrate();

  /* Setup Left Input with input source, 20dB mic gain, and input gain
   * ------------------------------------------------------------------ */
     index[0] = audio.l_input | audio.l_20db | audio.l_gain;
     wr(L_ADC_INPUT_REG, AUDIO_INDEX_REG);
     wr(index[0], AUDIO_INDEXED_DATA_REG);

  /* Setup Right Input with input source, 20dB mic gain, and input gain
   * ------------------------------------------------------------------ */
     index[1] = audio.r_input | audio.r_20db | audio.r_gain;
     wr(R_ADC_INPUT_REG, AUDIO_INDEX_REG);
     wr(index[1], AUDIO_INDEXED_DATA_REG);

  /* Setup gain/attenuation for Left Auxiliary 1
   * ------------------------------------------------------------------ */
     index[2] = audio.l_aux1;
     wr(LAUX1_INPUT_REG, AUDIO_INDEX_REG);
     wr(index[2], AUDIO_INDEXED_DATA_REG);

  /* Setup gain/attenuation for Right Auxiliary 1
   * ------------------------------------------------------------------ */
     index[3] = audio.r_aux1;
     wr(RAUX1_INPUT_REG, AUDIO_INDEX_REG);
     wr(index[3], AUDIO_INDEXED_DATA_REG);

  /* Setup attenuation for Left Output
   * ------------------------------------------------------------------ */
     index[6] = audio.l_out;
     wr(L_DAC_OUTPUT_REG, AUDIO_INDEX_REG);
     wr(index[6], AUDIO_INDEXED_DATA_REG);

  /* Setup attenuation for Right Output
   * ------------------------------------------------------------------ */
     index[7] = audio.r_out;
     wr(R_DAC_OUTPUT_REG, AUDIO_INDEX_REG);
     wr(index[7], AUDIO_INDEXED_DATA_REG);

  /* Enable/disable Digital Mixing and setup Digital Mix Attenuation
   * ------------------------------------------------------------------ */
     index[13] = (audio.dm_attn << 2) | audio.dme;
     wr(LOOPBACK_CNTL_REG, AUDIO_INDEX_REG);
     wr(index[13], AUDIO_INDEXED_DATA_REG);

   if (audio.audio_mode == MODE2) {

     wr(MODE_ID_REG, AUDIO_INDEX_REG);    /* Set to MODE 2               */
     wr(0xCA, AUDIO_INDEXED_DATA_REG);

     index[16] = 0x80;
     wr(ALT_FEATURE_1_REG, AUDIO_INDEX_REG);
     wr(index[16], AUDIO_INDEXED_DATA_REG);

     index[17] = audio.hp_filter;
     wr(ALT_FEATURE_2_REG, AUDIO_INDEX_REG);
     wr(index[17], AUDIO_INDEXED_DATA_REG);

     index[18] = audio.lline_in;
     wr(L_LINE_INPUT_REG, AUDIO_INDEX_REG);
     wr(index[18], AUDIO_INDEXED_DATA_REG);

     index[19] = audio.lline_in;
     wr(R_LINE_INPUT_REG, AUDIO_INDEX_REG);
     wr(index[19], AUDIO_INDEXED_DATA_REG);

     index[26] = audio.mono_in | audio.mono_out;
     wr(MONO_IO_CNTL_REG, AUDIO_INDEX_REG);
     wr(index[26], AUDIO_INDEXED_DATA_REG);

     index[28] = index[8] & 0xF0;
     wr( (MCE_SET | CAPTURE_DATA_REG), AUDIO_INDEX_REG);
     wr( index[28], AUDIO_INDEXED_DATA_REG);

     status |= (wait_audio_ready() | 0x20);
     status |= calibrate();
   }

return(0);

} /* end of audio_init_setup() */


/**************************************************************************
 *   NAME: digital_loop
 *        Places the Codec into a digital loopback mode, which allows
 *        external testing of the audio paths from mic in to line out.
 */

int   digital_loop(int *secondary_ptr)
{
extern   char  *ioa;
int      status;

default_values();
audio.audio_mode = MODE2;
audio.dme = ENABLE;
audio.clock = XTAL1;
audio.type = STEREO;
audio.cfs = 0;
audio.data = 0;
status = audio_setup(secondary_ptr);
status = audio_setup(secondary_ptr);
return(0);

}


/**************************************************************************
 *   NAME: write_codec
 *        Writes a specific codec register.
 */

void  write_codec(int   data, int   reg)

{
wr((char)reg, AUDIO_INDEX_REG);
wr((char)data, AUDIO_INDEXED_DATA_REG);
}

/**************************************************************************
 *   NAME: manual
 *        Prompts for manual register control. Enter Q for quit.
 */

int   manual()
{
extern   char  *ioa;
int      status = 0;
int      direct = 0;
int      exit = 0;
int      reg,data;
char     input_line[80];

read_control_regs();
do
  {
  if(status)
    {
    printf("Enter data(hex): ");
    }
  else
    {
    printf("Enter reg(i or d, decimal): ");
    };

  if (!gets(input_line))
     {
     printf("\nERROR reading input line. Retry.\n");
     }
  else
     {
     /* verify operator input is of the necessary format */
     if ((input_line[0] == 'q') || (input_line[0] == 'Q'))
       {
       exit = 1;
       }
     else if(status == 1)     /*  data entry */
       {
       sscanf(input_line,"%x", &data);  /* get data */
       if(direct)
         {
         *(ioa + reg) = (char)data;
         }
       else
         {
         write_codec((char)data, (char)reg);
         };
       read_control_regs();
       status = 0;
       }
     else if((input_line[0] == 'd') || (input_line[0] == 'D'))
       {
       status = 1;
       direct = 1;
       sscanf(input_line+1,"%d", &reg);   /* get reg number */
       }
     else if((input_line[0] == 'i') || (input_line[0] == 'I'))
       {
       status = 1;
       direct = 0;
       sscanf(input_line+1,"%d", &reg);   /* get reg number */
       };
     };
  } while(!exit);

return(0);

}

/**************************************************************************
*/


int  record_playback(int   sec_ptr[])
{
  int i, j, byte_count, timeout;
  unsigned int status, feature_status;
  char   *buffer;
  int    retcode;

  default_values();                  /* select defaults */

  audio.audio_mode = MODE2;
/*audio.transfer_type = DUAL_DMA;    */
/* audio.dma_cycle = BURST_CYCLE;    */
/* SET BURST ON IN MCI CHIP */

  status = audio_setup(sec_ptr);     /* setup and auto-calibrate AD1848 */
  status = audio_setup(sec_ptr);     /* Need to re-setup for fast speed */

  /* Allocate a memory buffer for X seconds of audio */
  /* Use stereo mode always */
  /* This will depend on sample rate */
  buffer = (char *)malloc(30000);
  if(buffer == NULL) return(100);

  byte_count = get_base_count(30000);

  printf("Call readx...\n");
  /* Request DMA operation for record (read) */
  readx(dd_FileDes, buffer, 30000, byte_count);

  /* Wait for interrupt to tell us that recording is complete */
  timeout = 8;  /* 20 second timeout */
  do
    {
    sleep(1);
    timeout--;
    ioctl(dd_FileDes, CAPTURE_DMA_STATUS, &status);
    printf("STATUS = %x\n", status);
    }
  while(( status==0x5555 ) && (timeout > 0));

  /* Prompt to begin playback */
  printf("\n\n....Press 'enter' to begin playback.\n");
  {
  char   buff[100];
  gets(buff);
  };

  /* Request DMA operation for playback) */
  writex(dd_FileDes, buffer, 30000, byte_count);

  /* Wait for interrupt to tell us that playback is complete */
  timeout = 8;  /* 20 second timeout */
  do
    {
    sleep(1);
    timeout--;
    ioctl(dd_FileDes, PLAYBACK_DMA_STATUS, &status);
    printf("STATUS = %x\n", status);
    }
  while(( status==0x5555 ) && (timeout > 0));

  free(buffer);

  return(0);
}


/**************************************************************************
   FUNCTION:   record_playback_sim
               Allows user to record and playback line input/output
               under his control.  Approx one second audio.

*/


int  record_playback_sim(int   sec_ptr[])
{
  int i, j, byte_count, timeout;
  unsigned int status, feature_status;
  char   *buffer1,*buffer2;
  int    retcode;
  extern char  *ioa;                /* direct access pointer */

  default_values();                  /* select defaults */

  audio.audio_mode = MODE2;
  audio.sample_rate = KHZ48o0;
  audio.clock = XTAL1;
  audio.cfs = 6;
  audio.type = STEREO;
/*audio.transfer_type = DUAL_DMA;    */
/* audio.dma_cycle = BURST_CYCLE;    */
/* SET BURST ON IN MCI CHIP */

  status = audio_setup(sec_ptr);     /* setup and auto-calibrate AD1848 */
  status = audio_setup(sec_ptr);     /* Need to re-setup for fast speed */


  printf("AUDIO Status = %x\n", (*(ioa + AUDIO_STATUS_REG)));


  /* Allocate a memory buffer for X seconds of audio */
  /* Use stereo mode always */
  /* This will depend on sample rate */
  buffer1 = (char *)malloc(60000);
  if(buffer1 == NULL) return(100);
  buffer2 = (char *)malloc(60000);
  if(buffer2 == NULL) return(100);

  byte_count = get_base_count(60000);

  printf("Call readx...\n");
  /* Request DMA operation for record (read) */
  readx(dd_FileDes, buffer1, 60000, byte_count);

  /* Wait for interrupt to tell us that recording is complete */
  timeout = 10; /* 20 second timeout */
  do
    {
    sleep(1);
    timeout--;
    ioctl(dd_FileDes, CAPTURE_DMA_STATUS, &status);
    printf("CAPTURE STATUS = %x\n", status);
    }
  while(( status==0x5555 ) && (timeout > 0));

  /* copy buffer1 to buffer2 */
  for(i = 60000; i > 0; i--)
     {
     buffer2[i] = buffer1[i];
     };

  printf("AUDIO Status = %x\n", (*(ioa + AUDIO_STATUS_REG)));
  printf("AUDIO ADDR   = %x\n", (*(ioa + AUDIO_INDEX_REG)));

  /* Prompt to begin playback and record to buffer 2 */
  printf("\nBoth buffers are now equal.");
  printf("\n\n....Press 'enter' to begin playback.\n");
  {
  char   buff[100];
  gets(buff);
  };

  /* Request DMA operation for playback) */
  writex(dd_FileDes, buffer1, 60000, byte_count);
  printf("Between write and read\n");
  readx(dd_FileDes, buffer2, 60000, byte_count);
  printf("After read call \n");

  /* Wait for interrupt to tell us that recording is complete */
  timeout = 10; /* 20 second timeout */
  do
    {
    sleep(1);
    timeout--;
    ioctl(dd_FileDes, CAPTURE_DMA_STATUS, &status);
    printf("CAPTURE STATUS = %x\n", status);
    }
  while(( status==0x5555 ) && (timeout > 0));
  printf("AUDIO Status = %x\n", (*(ioa + AUDIO_STATUS_REG)));

  /* Wait for interrupt to tell us that playback is complete */
  timeout =10;  /* 20 second timeout */
  do
    {
    sleep(1);
    timeout--;
    ioctl(dd_FileDes, PLAYBACK_DMA_STATUS, &status);
    printf("PLAYBACK STATUS = %x\n", status);
    }
  while(( status==0x5555 ) && (timeout > 0));
  printf("AUDIO Status = %x\n", (*(ioa + AUDIO_STATUS_REG)));

  /* Prompt to begin playback and record to buffer 2 */
  printf("\n\n....Press 'enter' to begin playback.\n");
  {
  char   buff[100];
  gets(buff);
  };

  /* Request DMA operation for playback) */
  writex(dd_FileDes, buffer2, 60000, byte_count);

  /* Wait for interrupt to tell us that playback is complete */
  timeout = 10; /* 20 second timeout */
  do
    {
    sleep(1);
    timeout--;
    ioctl(dd_FileDes, PLAYBACK_DMA_STATUS, &status);
    printf("PLAYBACK STATUS = %x\n", status);
    }
  while(( status==0x5555 ) && (timeout > 0));
  printf("AUDIO Status = %x\n", (*(ioa + AUDIO_STATUS_REG)));

  free(buffer1);
  free(buffer2);

  return(0);
}

/**************************************************************************
   FUNCTION:   record_playback_htx
               Allows htx to record and playback line input/output
               without operator intervention.  Line in is recorded and
               played back to line out.

*/


int  record_playback_htx(int   sec_ptr[])
{
  int i, j, byte_count, timeout;
  unsigned int status, feature_status;
  char   *buffer1,*buffer2;
  int    retcode;
  extern char  *ioa;                /* direct access pointer */

  default_values();                  /* select defaults */

  audio.audio_mode = MODE2;
  audio.sample_rate = KHZ5o5152;
  audio.clock = XTAL2;
  audio.cfs = 0;
  audio.type = STEREO;

  status = audio_setup(sec_ptr);     /* setup and auto-calibrate AD1848 */
  status = audio_setup(sec_ptr);     /* Need to re-setup for fast speed */

  /* Allocate a memory buffer for X seconds of audio */
  /* Use stereo mode always */
  /* This will depend on sample rate */
  buffer1 = (char *)malloc(30000);
  if(buffer1 == NULL) return(100);
  buffer2 = (char *)malloc(30000);
  if(buffer2 == NULL) return(100);

  byte_count = get_base_count(30000);

  /* Request DMA operation for record (read) */
  readx(dd_FileDes, buffer1, 30000, byte_count);

  /* Wait for interrupt to tell us that recording is complete */
  timeout = 10; /* 20 second timeout */
  do
    {
    sleep(1);
    timeout--;
    ioctl(dd_FileDes, CAPTURE_DMA_STATUS, &status);
    }
  while(( status==0x5555 ) && (timeout > 0));

  if(timeout <= 0)
    {
    free(buffer1);
    free(buffer2);
    return(DMA_CAPTURE);
    };

  /* Request DMA operation for playback) */
  readx(dd_FileDes, buffer2, 30000, byte_count);
  writex(dd_FileDes, buffer1, 30000, byte_count);

  /* Wait for interrupt to tell us that recording is complete */
  timeout = 10; /* 20 second timeout */
  do
    {
    sleep(1);
    timeout--;
    ioctl(dd_FileDes, CAPTURE_DMA_STATUS, &status);
    }
  while(( status==0x5555 ) && (timeout > 0));

  if(timeout <= 0)
    {
    free(buffer1);
    free(buffer2);
    return(DMA_CAPTURE);
    };

  /* Wait for interrupt to tell us that playback is complete */
  timeout = 10; /* 20 second timeout */
  do
    {
    sleep(1);
    timeout--;
    ioctl(dd_FileDes, PLAYBACK_DMA_STATUS, &status);
    }
  while(( status==0x5555 ) && (timeout > 0));

  if(timeout <= 0)
    {
    free(buffer1);
    free(buffer2);
    return(DMA_PLAYBACK);
    };

  /* Request DMA operation for playback) */
  writex(dd_FileDes, buffer2, 30000, byte_count);

  /* Wait for interrupt to tell us that playback is complete */
  timeout = 10; /* 20 second timeout */
  do
    {
    sleep(1);
    timeout--;
    ioctl(dd_FileDes, PLAYBACK_DMA_STATUS, &status);
    }
  while(( status==0x5555 ) && (timeout > 0));

  if(timeout <= 0)
    {
    free(buffer1);
    free(buffer2);
    return(DMA_PLAYBACK);
    };

  free(buffer1);
  free(buffer2);

  return(0);
}

/**************************************************************************
   FUNCTION:   codec_register_test
               Load codec chip with a valid functional register setup
               and verifies that registers are read back properly.

   RETURN:     0 if no error.  Secondary[0] is set to invalid register no.

*/


char  expected[] =
   { 0x00, 0x00, 0x80, 0x80, 0x88, 0x88, 0x00, 0x00,
     0x1C, 0x08, 0x00, 0x00, 0xCA, 0x00, 0x00, 0x00,     /* defect */
     0x80, 0x00, 0x80, 0x80, 0x00, 0x00, 0x80, 0x80,
     0x00, 0x80, 0xC0, 0x80, 0x10, 0x80, 0x00, 0x00
   };


int  codec_register_test(int   sec_ptr[])
{
  int i, j, byte_count, timeout;
  char   readback[32];
  unsigned int status, feature_status;
  int    retcode = 0;
  int    temp;
  extern char  *ioa;                /* direct access pointer */

  default_values();                  /* select defaults */

  audio.audio_mode = MODE2;
  audio.sample_rate = KHZ48o0;
  audio.clock = XTAL1;
  audio.cfs = 6;
  audio.type = STEREO;

  /* THIS WILL SETUP CODEC TO SELECTED VALUES AND DO CALIBRATION */
  status = audio_setup(sec_ptr);     /* setup and auto-calibrate AD1848 */
  status = audio_setup(sec_ptr);     /* Need to re-setup for fast speed */

  /* Waits while autocalibration is in progress
   * --------------------------------------------------------------------- */
  timeout = 0x1000000;
  do {
     temp = rd(AUDIO_INDEXED_DATA_REG);
     } while ( (temp & 0x20) && (--timeout > 0) ); /* enddo */


  if(timeout <= 0)
    {
    retcode = CS4231_BAD;
    }
  else
    {
    /* NOW READ BACK REGISTERS AND VERIFY CORRECT */
    wr(L_ADC_INPUT_REG, AUDIO_INDEX_REG);
    readback[0]  = rd(AUDIO_INDEXED_DATA_REG);
    wr(R_ADC_INPUT_REG, AUDIO_INDEX_REG);
    readback[1]  = rd(AUDIO_INDEXED_DATA_REG);
    wr(LAUX1_INPUT_REG, AUDIO_INDEX_REG);
    readback[2]  = rd(AUDIO_INDEXED_DATA_REG);
    wr(RAUX1_INPUT_REG, AUDIO_INDEX_REG);
    readback[3]  = rd(AUDIO_INDEXED_DATA_REG);
    wr(LAUX2_INPUT_REG, AUDIO_INDEX_REG);
    readback[4]  = rd(AUDIO_INDEXED_DATA_REG);
    wr(RAUX2_INPUT_REG, AUDIO_INDEX_REG);
    readback[5]  = rd(AUDIO_INDEXED_DATA_REG);
    wr(L_DAC_OUTPUT_REG, AUDIO_INDEX_REG);
    readback[6]  = rd(AUDIO_INDEXED_DATA_REG);
    wr(R_DAC_OUTPUT_REG, AUDIO_INDEX_REG);
    readback[7]  = rd(AUDIO_INDEXED_DATA_REG);
    wr(FS_PLAYBACK_DATA_REG, AUDIO_INDEX_REG);
    readback[8]  = rd(AUDIO_INDEXED_DATA_REG);
    wr(INTERFACE_REG, AUDIO_INDEX_REG);
    readback[9]  = rd(AUDIO_INDEXED_DATA_REG);
    wr(PIN_CNTL_REG, AUDIO_INDEX_REG);
    readback[10] = rd(AUDIO_INDEXED_DATA_REG);
    wr(ERR_STATUS_INIT_REG, AUDIO_INDEX_REG);
    readback[11] = rd(AUDIO_INDEXED_DATA_REG);
    wr(MODE_ID_REG, AUDIO_INDEX_REG);
    readback[12] = rd(AUDIO_INDEXED_DATA_REG);
    wr(LOOPBACK_CNTL_REG, AUDIO_INDEX_REG);
    readback[13] = rd(AUDIO_INDEXED_DATA_REG);
    wr(PLAYBACK_UPCOUNT_REG, AUDIO_INDEX_REG);
    readback[14] = rd(AUDIO_INDEXED_DATA_REG);
    wr(PLAYBACK_LOCOUNT_REG, AUDIO_INDEX_REG);
    readback[15] = rd(AUDIO_INDEXED_DATA_REG);

    wr(MODE_ID_REG, AUDIO_INDEX_REG);
    wr(0xCA, AUDIO_INDEXED_DATA_REG);

    wr(ALT_FEATURE_1_REG, AUDIO_INDEX_REG);
    readback[16] = rd(AUDIO_INDEXED_DATA_REG);
    wr(ALT_FEATURE_2_REG, AUDIO_INDEX_REG);
    readback[17] = rd(AUDIO_INDEXED_DATA_REG);
    wr(L_LINE_INPUT_REG, AUDIO_INDEX_REG);
    readback[18] = rd(AUDIO_INDEXED_DATA_REG);
    wr(R_LINE_INPUT_REG, AUDIO_INDEX_REG);
    readback[19] = rd(AUDIO_INDEXED_DATA_REG);
    wr(TIMER_LOBYTE_REG, AUDIO_INDEX_REG);
    readback[20] = rd(AUDIO_INDEXED_DATA_REG);
    wr(TIMER_HIBYTE_REG, AUDIO_INDEX_REG);
    readback[21] = rd(AUDIO_INDEXED_DATA_REG);
    wr( 22, AUDIO_INDEX_REG);
    readback[22] = rd(AUDIO_INDEXED_DATA_REG);
    wr( 23, AUDIO_INDEX_REG);
    readback[23] = rd(AUDIO_INDEXED_DATA_REG);
    wr(ALT_FEATURE_STAT_REG, AUDIO_INDEX_REG);
    readback[24] = rd(AUDIO_INDEXED_DATA_REG);
    wr(VERSION_CHIP_ID_REG, AUDIO_INDEX_REG);
    readback[25] = rd(AUDIO_INDEXED_DATA_REG);
    wr(MONO_IO_CNTL_REG, AUDIO_INDEX_REG);
    readback[26] = rd(AUDIO_INDEXED_DATA_REG);
    wr( 27, AUDIO_INDEX_REG);
    readback[27] = rd(AUDIO_INDEXED_DATA_REG);
    wr(CAPTURE_DATA_REG, AUDIO_INDEX_REG);
    readback[28] = rd(AUDIO_INDEXED_DATA_REG);
    wr( 29, AUDIO_INDEX_REG);
    readback[29] = rd(AUDIO_INDEXED_DATA_REG);
    wr(CAPTURE_UPCOUNT_REG, AUDIO_INDEX_REG);
    readback[30] = rd(AUDIO_INDEXED_DATA_REG);
    wr(CAPTURE_LOCOUNT_REG, AUDIO_INDEX_REG);
    readback[31] = rd(AUDIO_INDEXED_DATA_REG);

    retcode = 0;

    for(i = 0; i < 32; i++)
       {
       if(( expected[i] != readback[i] ) && (retcode == 0))
         {
         retcode = CS4231_BAD;
         sec_ptr[0] = i;
         sec_ptr[1] = expected[i];
         sec_ptr[2] = readback[i];
         sec_ptr[3] = 0x555;
         };
       };
    };
  return(retcode);
}

