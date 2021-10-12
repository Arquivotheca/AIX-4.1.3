static char sccsid[] = "@(#)22  1.1  src/bos/diag/tu/corv/CmdBld.c, tu_corv, bos411, 9428A410j 7/22/93 18:54:09";
/*
 *   COMPONENT_NAME: TU_CORV
 *
 *   FUNCTIONS: build_cmd
 *              byte_swap
 *              int_swap
 *              random_data
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
/*  DRF rewritten version of the Command Build routine.  6/15/92  */
/*  DRF fixed bug 6/19/92 where cmd_lngth < # of parameters.      */
/*  DRF fixed strtok bug, where format was corrupted.  6/23/92     */
#include <stdarg.h>
#include <string.h>
#define TOT_BYTE  200
typedef struct {
     char bytes[TOT_BYTE];
     int  length;
     char *dma_buffer;
     int  dma_length;
} COMMAND;

unsigned long byte_swap(unsigned long old_long)
{

     unsigned long swapped_long;


     swapped_long = ((old_long & 0x000000ff) << 24) |
                    ((old_long & 0x0000ff00) << 8)  |
                    ((old_long & 0x00ff0000) >> 8)  |
                    ((old_long & 0xff000000) >> 24);

     return swapped_long;
}

unsigned int int_swap(unsigned int old_int)
{

     unsigned int swapped_int;


     swapped_int = ((old_int & 0x000000ff) << 8)  |
                   ((old_int & 0x0000ff00) >> 8);

     return swapped_int;
}

COMMAND build_cmd(const char *format,int cmd_lngth, ...)
{
    va_list arg_pointer;
    char *d;
    char tmp_fmt[TOT_BYTE];
    char format_command[7];
    int format_size;
    int swap;
    int bmsk, bcnt, loopr, firstflg;
    ulong next_val;
    int new_val, strtst, tmp;
    int      n_bit,            /* dec number of bits in current value */
             array_pos,
             bit_place;

    COMMAND command;

    va_start(arg_pointer, cmd_lngth);
    bmsk = bcnt = firstflg = 0;
    bit_place = 0;
    n_bit = 1;
    array_pos = 0;
    d = command.bytes;
    *d = '\0';

/*  Check to see if we will exceed the allocated space for this operation */
    if (cmd_lngth >= TOT_BYTE)
      cmd_lngth = TOT_BYTE;

/* Place to clear out the array before using it   */
    for (array_pos=0; array_pos<cmd_lngth ; array_pos++)
        command.bytes[array_pos] = '\0';
    array_pos=0;

/* copy the format string into a temporary place to work on it.  */
    strtst = strcpy(tmp_fmt,format);

/* Loop as long as the cmn_lngth that is passed to us,  Then parse out
** the values from the format that are the 'mask' values we need.
** then mask those against the passed values.
*/
    while(n_bit != 0)
      {
       if (firstflg == 0) {
         strcpy(format_command, strtok(tmp_fmt,"%"));
         format_size = strlen(format_command);
         if (format_command[format_size-1]=='s') {
              format_command[format_size-1] = '\0';
              swap = TRUE;
         }
         else
              swap = FALSE;
         n_bit = (atoi(format_command));
         firstflg++;
       }
       else {
         strcpy(format_command, strtok(0,"%"));
         format_size = strlen(format_command);
         if (format_command[format_size-1]=='s') {
              format_command[format_size-1] = '\0';
              swap = TRUE;
         }
         else
              swap = FALSE;
         n_bit = (atoi(format_command));
       }

/*     Dont even allow the user to format more than 32 bits   */
       if (n_bit >= 33)
         n_bit = 32;

/*      'And' the mask against the new value passed    */
       bmsk = ((1 << n_bit) -1);
       next_val = ((va_arg(arg_pointer,int)) & bmsk);
       if (swap)
            switch (n_bit) {
                 case 32:      next_val = byte_swap(next_val);
                               break;
                 case 16:      next_val = int_swap(next_val);
                               break;
                 default:      break;
            }

/*       See if the new value and the remainder from the old value > 32  */
/*       Process the extra bits first  before doing the main group of bits */
       if (bit_place + n_bit >= 33)   {
         tmp = 0;
         tmp = next_val >> (32 - bit_place);
         new_val = *(int *)d;
         *(int *)d = (new_val | tmp);
         next_val = (next_val << bit_place);
         next_val = (next_val >> bit_place);
         d++;
         n_bit -= bit_place;
         bit_place = 0;
       }
/*      Now do the main group of bits, or here if we had less than 32 total */
/*      Shift the new value into place to write into the stream             */
       next_val = next_val << (32 - bit_place - n_bit);
       new_val = *(int *)d;
       new_val = new_val | next_val;
       *(int *)d = new_val;
       d = d + ((n_bit + bit_place)/8);
       bit_place = (n_bit + bit_place) % 8;
       loopr++;
      }
    va_end(arg_pointer);

    command.length = cmd_lngth;
    command.dma_length = 0;
    command.dma_buffer=0;

    return command;
}

void random_data(char *buffer, int buffer_len, int seed)
{

     srandom(seed);

     *(buffer + buffer_len--) = '\0';

     while ( buffer_len >= 0 )
         *(buffer + buffer_len--) = (random() & 0xff);

}
