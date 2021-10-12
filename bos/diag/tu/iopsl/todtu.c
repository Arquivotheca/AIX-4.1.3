static char sccsid[] = "@(#)42  1.2.1.2  src/bos/diag/tu/iopsl/todtu.c, tu_iopsl, bos411, 9428A410j 7/8/93 08:14:08";
/*
 *   COMPONENT_NAME: TU_IOPSL
 *
 *   FUNCTIONS: *               RAM_test
 *              dump_tod_ram
 *              get_data_block
 *              get_tod_data
 *              put_data_block
 *              put_tod_data
 *              reg_test
 *              seconds_test
 *              tod_tu
 *              update_mode
 *
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/* This file contains limited tests for the Time Of Day (TOD) chip. */
/* The following tests are performed :
 *
 *  1. Registers write/read test.
 *  2. Seconds increment test. 
 *  3. RAM write/read test.
 *
*/

#include <sys/types.h>
#include <stdio.h>
#include <signal.h>
#include <sys/mdio.h>

#include <diag/atu.h>
#include "misc.h"
#include "salioaddr.h"

#define ACCESS_DELAY (1 * 1000)  /* usec */

static int reg_test(int, TUCB *);
static int RAM_test(int);
static int seconds_test(int);
static int update_mode(int, BOOL);
static int get_data_block(int, ulong_t, uchar_t [], int);
static int put_data_block(int, ulong_t, uchar_t [], int);
/* static int incr_test(int);*/

int tod_tu(int fd, TUCB *pt)
{
  int rc;

  rc = SUCCESS;
  rc = reg_test(fd, pt);

  if(rc == SUCCESS)
    rc = seconds_test(fd);

  if(rc == SUCCESS)
    rc = RAM_test(fd);

  return(rc);
}


#define UIP_BIT   0x80  /* in register A */
#define SQWE_BIT  0x08  /* in register B */

int reg_test(int fd, TUCB *pt)
{
  int rc, i;
  uchar_t tmp;

  struct
  {
    uchar_t val, save;
  } reg;

  rc = SUCCESS;

/* write/read test of register A (bit 7 is read only) */

  rc = get_tod_data(fd, (ulong_t)REG_A, &reg.save);  /* save register */

  if(rc == SUCCESS)
  {
    reg.val = ~reg.save;                 /* toggle bits   */
    rc = put_tod_data(fd, (ulong_t)REG_A, reg.val); 

    if(rc == SUCCESS)
    {
      rc = get_tod_data(fd, (ulong_t)REG_A, &tmp); 

      if((reg.val & ~UIP_BIT) != (tmp & ~UIP_BIT))
	rc = TOD_REGA_WR_ERROR;
    
      put_tod_data(fd, (ulong_t)REG_A, reg.save);     /* restore register */
    }
  }

/* write/read test of register B (SQWE bit only) */

  if(rc == SUCCESS)
  {
    rc = get_tod_data(fd, (ulong_t)REG_B, &reg.save);  /* save register */

    if(rc == SUCCESS)
    {
/* Toggle SQWE bit  */

      reg.val = reg.save | SQWE_BIT;

      if(reg.save & SQWE_BIT)
	reg.val ^= SQWE_BIT;

      rc = put_tod_data(fd, (ulong_t)REG_B, reg.val); 
    }

    if(rc == SUCCESS)
    {
      rc = get_tod_data(fd, (ulong_t)REG_B, &tmp); 

      if(rc == SUCCESS && reg.val != tmp)
	rc = TOD_REGB_WR_ERROR;

      put_tod_data(fd, (ulong_t)REG_B, reg.save);  /* restore */
    }
  }

/* register D is read only. VRT (Valid RAM and Time) should */
/* always be 1. This is tested by BATTERY_TU.               */

  if (rc == SUCCESS) {
    if ((rc = battery_tu(fd, pt)) == BATTERY_ERROR)
     {
       rc = TOD_BATT_ERROR;
     }
  } 

  return(rc);
}



#define SET_BIT 0x80

static int update_mode(int fd, BOOL on)
{
  int rc;
  uchar_t reg;

  if((rc = get_tod_data(fd, (ulong_t)REG_B, &reg)) == SUCCESS)
  {
    if(on)
      reg |= SET_BIT;
    else
      reg = (reg | SET_BIT) ^ SET_BIT;
  }
  
  rc = put_tod_data(fd, (ulong_t)REG_B, reg);

  return(rc);
}



int RAM_test(int fd)
{
  int i, j, rc;

  struct
  {
    uchar_t save[TOD_NVRAM_SIZE];
    uchar_t val[TOD_NVRAM_SIZE];
  } data;

  static uchar_t test_val[] = { 0x55, 0xaa };

  if((rc = get_data_block(fd, (ulong_t)NVRAM, &data.save[0], sizeof(data.save))) == SUCCESS)
  {
    for(i = 0; rc == SUCCESS && i < sizeof(test_val) / sizeof(uchar_t); i++)
    {
      memset(&data.val[0], test_val[i], sizeof(data.val));
      rc = put_data_block(fd, (ulong_t)NVRAM, &data.val[0], sizeof(data.val));

      if(rc == SUCCESS)
	rc = get_data_block(fd, (ulong_t)NVRAM, &data.val[0], sizeof(data.val));

      if(rc == SUCCESS)
      {
	for(j = 0; j < TOD_NVRAM_SIZE; j++)
	  if(data.val[j] != test_val[i])
	  {
	    rc = TOD_RAM_ERROR;
	    break;
	  }
      }
    }

    put_data_block(fd, (ulong_t)NVRAM, &data.save[0], sizeof(data.save));  /* restore */
  }
    
  return(rc);
}




#define TIMER_ENABLED  0x20
#define DV_MASK        0x70

static int seconds_test(int fd)
{
  int rc;
  uchar_t reg, data[2];
  BOOL save_reg;

  rc = SUCCESS;
  rc = get_tod_data(fd, (ulong_t)REG_A, &reg);

/* check if timer is enabled */

  if((reg & DV_MASK) != TIMER_ENABLED)
  {
    data[0] = (reg & ~DV_MASK) | TIMER_ENABLED;
    rc = put_tod_data(fd, (ulong_t)REG_A, data[0]);
    save_reg = TRUE;
  }
  else
    save_reg = FALSE;

  if(rc == SUCCESS)
    rc = get_tod_data(fd, (ulong_t)SECONDS, &data[0]);

  if(rc == SUCCESS)
  {
    sleep(1);             /* wait for increment       */
    rc = get_tod_data(fd, (ulong_t)SECONDS, &data[1]); 
  }

  if(rc == SUCCESS)
  {
    if(data[0] == data[1])
      rc = SEC_INCR_ERROR;   /* seconds didn't increment */
  }

  if(save_reg)
    put_tod_data(fd, (ulong_t)REG_A, reg);  /* restore */

  return(rc);
}



/* Read a byte from TOD */

int get_tod_data(int fd, ulong_t tod_addr, uchar_t *data)
{
  int rc;

  rc = SUCCESS;
/*  usleep(ACCESS_DELAY);*/

  /* If we are using a POWER product */
  if (power_flag) {
     rc = put_byte(fd, (uchar_t)tod_addr, (ulong_t)tod_idx_reg, MIOTODPUT);

     if(rc == SUCCESS) {
       rc = getbyte(fd, data, (ulong_t)tod_data_reg, MIOTODGET);
     }
  }

 /* If not using a POWER product */
  else {
    rc = put_byte(fd, (uchar_t)tod_addr, (ulong_t)tod_idx_reg, MIOCCPUT);

    if(rc == SUCCESS)
    {
  /*    usleep(ACCESS_DELAY);*/
      rc = getbyte(fd, data, (ulong_t)tod_data_reg, MIOCCGET);
    }
  }
    
  return(rc);
}



/* Write a byte to TOD  */

int put_tod_data(int fd, ulong_t tod_addr, uchar_t data)
{
  int rc;

  rc = SUCCESS;
/*  usleep(ACCESS_DELAY);*/

  /* If we are using a POWER product */
  if (power_flag) {
     rc = put_byte(fd, (uchar_t)tod_addr, (ulong_t)tod_idx_reg, MIOTODPUT);

     if(rc == SUCCESS) {
       rc = put_byte(fd, data, (ulong_t)tod_data_reg, MIOTODPUT);
     }
  }

 /* If not using a POWER product */
  else {
    rc = put_byte(fd, (uchar_t)tod_addr, (ulong_t)tod_idx_reg, MIOCCPUT);

    if(rc == SUCCESS)
    {
  /*    usleep(ACCESS_DELAY);*/
      rc = put_byte(fd, data, (ulong_t)tod_data_reg, MIOCCPUT);
    }
  }
    
  return(rc);
}




/* Read a block of data from TOD */

static int get_data_block(int fd, ulong_t addr, uchar_t data[], int size)
{
  int i, rc;

  rc = SUCCESS;

  for(i = addr; i < size && rc == SUCCESS; i++)
    rc = get_tod_data(fd, i, &data[i - addr]);

  return(rc);
}



/* Write a block of data to TOD */

static int put_data_block(int fd, ulong_t addr, uchar_t data[], int size)
{
  int i, rc;

  rc = SUCCESS;

  for(i = addr; i < size && rc == SUCCESS; i++)
    rc = put_tod_data(fd, i, data[i - addr]);

  return(rc);
}



static int dump_tod_ram(int fd)
{
  uchar_t data[TOD_SIZE];
  int i, rc;

  rc = get_data_block(fd, 0, &data[0], sizeof(data));

  if(rc == SUCCESS)
    for(i = 0; i < TOD_SIZE; i++)
      printf("\n%d  %02x", i, data[i]);

  return(rc);
}



