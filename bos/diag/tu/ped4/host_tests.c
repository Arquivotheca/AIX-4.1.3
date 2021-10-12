static char sccsid[] = "@(#)41	1.1  src/bos/diag/tu/ped4/host_tests.c, tu_ped4, bos411, 9428A410j 6/11/93 14:52:47";
/*
 * COMPONENT_NAME: (tu_ped4) Pedernales Graphics Adapter Test Units
 *
 * FUNCTIONS:  host_bim_test, host_mem_test
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * MODULE NAME: host_tests.c
 *
 * DEPENDENCIES:  bim_base_addr must be initialized
 *
 * RESTRICTIONS:  None.
 *
 * EXTERNAL REFERENCES
 *
 *     OTHER ROUTINES:
 *
 *     DATA AREAS: bim_base_addr - the base address of BIM (dd_interface.c)
 *
 *     TABLES:  None
 *
 *     MACROS:  None.
 *
 * COMPILER / ASSEMBLER
 *
 *     TYPE, VERSION: AIX 3.2 XLC Compiler
 *
 *     OPTIONS:
 *
 * NOTES: None.
 *
 */

#include <stdio.h>
#include "bim_defs.h"
#include "tu_type.h"

/* constants for the host_bim_test */
#define IND_CONTROL_MASK             0x3C
#define DSP_CONTROL_MASK             0x1
#define IND_ADDRESS_MASK             0x7FFFF
#define IND_DATA_MASK                0xFFFFFFFF
#define HOST_INTR_MASK_MASK          0x3FF
#define HOST_COMMO_MASK              0xFFFFFFFF

/* ERROR CODES returned by this module */
#define NO_ERROR 0
/* error codes returned by host_bim_test */
#define BIM_REGISTER_PATTERN_TEST_FAILED 0x300
#define WRAP_MODE_PATTERN_TEST_FAILED    0x301

/* error codes returned by host_mem_test */
#define PATTERN_TEST_FAILURE_04 0x400
#define ADDRESS_TEST_FAILURE_04 0x401
#define PATTERN_TEST_FAILURE_34 0x2200
#define ADDRESS_TEST_FAILURE_34 0x2201



/* constants for the host memory test */
/* -- WARNING: it is up to the person who defines the values for the test_
               area_size and TEST_AREA_START to verify that the sum of the
               two values does not cause the test to attempt to write beyond
               the end of the memory. test_area_size is passed in from the
               calling routine (exectu). */
#define TEST_AREA_START 0x0  /* starting address of memory area to be tested */
#define FALSE 0
#define START_PATTERN_1   0x04030201
#define START_PATTERN_2   0x08060402
#define START_PATTERN_3   0x0C090603
#define START_PATTERN_4   0x100C0804


volatile unsigned long *ind_data; /* pointer to the data port */
#define INC_VALUE   0x01010101   /* value to increment data pattern by */

/* Symbols external to this file */
   extern volatile unsigned long *bim_base_addr; /* Adapter Base Address  */
                                                 /*  from dd_interface.c  */


/* patterns for register testing */
static unsigned long pattern[4] =                 /* declare & initialize */
{0xAAAAAAAA, 0x55555555, 0xFFFFFFFF, 0x00000000};

/* patterns for memory testing */
static unsigned long mem_pattern[4] =             /* declare & initialize */
{0xAAAAAAAA, 0x55555555, 0x55555555, 0xAAAAAAAA};


int write_64k_words(unsigned long next_256_start_value,
                    unsigned long *words_tested,
                    unsigned long test_area_size);

int read_64k_words(unsigned long next_256_start_value,
                   unsigned long *secondary_ptr,
                   unsigned long *words_tested,
                   unsigned long test_area_size,
                   int tu_number);



/*
 * NAME: host_bim_test
 *
 * FUNCTION: Performs a pattern test on BIM registers accessible from the
 *           host.
 *
 * EXECUTION ENVIRONMENT:  AIX V3.1 during diagnostics
 *
 * NOTES: None.
 *
 * RECOVERY OPERATION: None.
 *
 * DATA STRUCTURES: bim_base_addr - global pointer to the base of adapter
 *
 * RETURNS: This procedure will return zero if successful.
 *          If error is found then return code is returned plus secondary
 *          information as defined below.
 */
/*
 * INPUT:    secondary_ptr - pointer to secondary information defined below
 *
 * OUTPUT:
 *           secondary_ptr[0] - Register Offset
 *           secondary_ptr[1] - expected data
 *           secondary_ptr[2] - actual data
 */

 int host_bim_test(unsigned long *secondary_ptr)
{
   unsigned long actual_value;
   int error, i;

   /* HOLD DSP in RESET */
   *(bim_base_addr + DSP_CONTROL) = 0;

   /* Pattern test BIM registers which have both read and write access  */
   /* from the host side                                                */

   *(bim_base_addr + IND_CONTROL) = 0;
   usleep(100000);
   /* read back and verify pattern */
   actual_value = *(bim_base_addr + IND_CONTROL) & IND_CONTROL_MASK;
   if (actual_value) {
      *secondary_ptr++ = IND_CONTROL << 2;  /* address offset */
      *secondary_ptr++ = 0;            /* expected value */
      *secondary_ptr++ = actual_value; /* actual value */
      return(BIM_REGISTER_PATTERN_TEST_FAILED);
   } /* endif */

   *(bim_base_addr + IND_CONTROL) = 0x38;
   usleep(100000);
   /* read back and verify pattern */
   actual_value = *(bim_base_addr + IND_CONTROL) & IND_CONTROL_MASK;
   if (actual_value != 0x38) {
      *secondary_ptr++ = IND_CONTROL << 2;  /* address offset */
      *secondary_ptr++ = 0x38;         /* expected value */
      *secondary_ptr++ = actual_value; /* actual value */
      return(BIM_REGISTER_PATTERN_TEST_FAILED);
   } /* endif */

   *(bim_base_addr + IND_CONTROL) = 0x8;

   error = FALSE;
   for (i = 0 ; (i < 4) && !error ; i++) {

      /* write pattern */
      *(bim_base_addr + IND_ADDRESS) = pattern[i] & IND_ADDRESS_MASK;
      *(bim_base_addr + IND_DATA) = pattern[i] & IND_DATA_MASK;
      *(bim_base_addr + HOST_INTR_MASK) = pattern[i] & HOST_INTR_MASK_MASK;
      *(bim_base_addr + HOST_COMMO) = pattern[i] & HOST_COMMO_MASK;

      /* wait a little */
      usleep(100000);

      /* read back and verify pattern */
      actual_value = *(bim_base_addr + IND_ADDRESS) & IND_ADDRESS_MASK;
      if ((actual_value != (pattern[i] & IND_ADDRESS_MASK)) && pattern[i]) {
         *secondary_ptr++ = IND_ADDRESS << 2;  /* address offset */
         *secondary_ptr++ = (pattern[i] & IND_ADDRESS_MASK); /* exp. */
         *secondary_ptr++ = actual_value; /* actual value */
         return(BIM_REGISTER_PATTERN_TEST_FAILED);
      } /* endif */
      actual_value = *(bim_base_addr + IND_DATA) & IND_DATA_MASK;
      if (actual_value != (pattern[i] & IND_DATA_MASK)) {
         *secondary_ptr++ = IND_DATA << 2;  /* address offset */
         *secondary_ptr++ = (pattern[i] & IND_DATA_MASK); /* exp. */
         *secondary_ptr++ = actual_value; /* actual value */
         return(BIM_REGISTER_PATTERN_TEST_FAILED);
      } /* endif */
      actual_value = *(bim_base_addr + HOST_INTR_MASK) & HOST_INTR_MASK_MASK;
      if (actual_value != (pattern[i] & HOST_INTR_MASK_MASK)) {
         *secondary_ptr++ = HOST_INTR_MASK << 2;  /* address offset */
         *secondary_ptr++ = (pattern[i] & HOST_INTR_MASK_MASK); /* exp */
         *secondary_ptr++ = actual_value; /* actual value */
         return(BIM_REGISTER_PATTERN_TEST_FAILED);
      } /* endif */
      actual_value = *(bim_base_addr + HOST_COMMO) & HOST_COMMO_MASK;
      if (actual_value != (pattern[i] & HOST_COMMO_MASK)) {
         *secondary_ptr++ = HOST_COMMO << 2;  /* address offset */
         *secondary_ptr++ = (pattern[i] & HOST_COMMO_MASK); /* exp. */
         *secondary_ptr++ = actual_value; /* actual value */
         return(BIM_REGISTER_PATTERN_TEST_FAILED);
      } /* endif */

   } /* endfor */

   return(0);

} /* end host_bim_test */



/*
 * NAME: host_mem_test
 *
 * FUNCTION: Test a part of DSP memory from host.
 *
 * EXECUTION ENVIRONMENT: AIX 3.2 during diagnostics
 *
 * NOTES:
 *
 * RECOVERY OPERATION: None.
 *
 * DATA STRUCTURES: None
 *
 * RETURNS: This test returns zero if successful, else will return error code
 *          along with secondary information.
 *
 * INPUT: secondary_ptr - Pointer for Secondary Return Codes.
 *        test_area_size - number of words of memory to be tested (max 256K)
 *
 * OUTPUT:
 *           secondary_ptr[0] - failing address
 *           secondary_ptr[1] - expected data
 *           secondary_ptr[2] - actual data
 *
 */

 int host_mem_test(unsigned long *secondary_ptr,
                   unsigned long test_area_size,
                   int tu_number)
{
   int i, error, error1;
   volatile unsigned long *ind_control, *ind_address;
   unsigned long expected, actual;   /* data values as written and read */
   unsigned long expected1, actual1; /* data values as written and read */
   unsigned long words_tested;       /* Counter to determine termination time */
   unsigned long j;                  /* index and loop counter */

   /* HOLD DSP in RESET */
   *(bim_base_addr + DSP_CONTROL) = 0;

   /* Initialize Pointers */
   ind_control = (bim_base_addr + IND_CONTROL);
   ind_address = (bim_base_addr + IND_ADDRESS);
   ind_data = (bim_base_addr + IND_DATA);

   /************************************************************************/
   /* PATTERN TEST - fill memory with alternating patterns; read and verify*/
   /************************************************************************/
   for (i = 0 ; i < 4 ; i+=2) {
      /* get next patterns to be written */
      expected = mem_pattern[i];
      expected1 = mem_pattern[i+1];

      /* fill memory with pattern */
      *ind_control = IND_WRITE | IND_DSPMEM | IND_AUTOINC;
      *ind_address = TEST_AREA_START;
      for (j = 0 ; j < test_area_size ; j+=2) {
         *ind_data = expected;
         *ind_data = expected1;
      } /* endfor */

      /* perform initialization required to read back and verify data */
      error = FALSE;
      error1 = FALSE;
      *ind_control = IND_READ | IND_DSPMEM | IND_AUTOINC;
      *ind_address = TEST_AREA_START;

      /* loop through reading back memory looking for errors */
      for (j = 0 ; (j < test_area_size) && !error && !error1 ; j+=2) {
         actual = *ind_data;              /* read back first pattern */
         actual1 = *ind_data;             /* read back second pattern */
         error = actual != expected;
         error1 = actual1 != expected1;
      } /* endfor */

      /* determine if an error on the first pattern caused loop termination */
      if (error) {
         secondary_ptr[0] = TEST_AREA_START + j - 2;   /* failing address */
         secondary_ptr[1] = expected;
         secondary_ptr[2] = actual;
         /* exit with the error code appropriate for this TU */
         if (tu_number == TU_04) {
            return(PATTERN_TEST_FAILURE_04);  /* exit with TU_04 error code */
         } else {
            return(PATTERN_TEST_FAILURE_34);  /* exit with TU_34 error code */
         } /* endif */
      } /* endif */

      /* determine if an error on the second pattern caused loop termination */
      if (error1) {
         secondary_ptr[0] = TEST_AREA_START + j - 1;   /* failing address */
         secondary_ptr[1] = expected1;
         secondary_ptr[2] = actual1;
         /* exit with the error code appropriate for this TU */
         if (tu_number == TU_04) {
            return(PATTERN_TEST_FAILURE_04);  /* exit with TU_04 error code */
         } else {
            return(PATTERN_TEST_FAILURE_34);  /* exit with TU_34 error code */
         } /* endif */
      } /* endif */

   } /* endfor */


   /************************************************************************/
   /* ADDRESS TEST - fill memory with calculated patterns; read and verify */
   /************************************************************************/

   /* initialize the control and address registers on the BIM */
   *ind_control = IND_WRITE | IND_DSPMEM | IND_AUTOINC;
   *ind_address = TEST_AREA_START;
   words_tested = 0;            /* no words have been tested at start time */

   /* write the first 64K words of data */
   (void)write_64k_words(START_PATTERN_1, &words_tested, test_area_size);

   /* write the second 64K words of data */
   (void)write_64k_words(START_PATTERN_2, &words_tested, test_area_size);

   /* write the third 64K words of data */
   (void)write_64k_words(START_PATTERN_3, &words_tested, test_area_size);

   /* write the last 64K words of data */
   (void)write_64k_words(START_PATTERN_4, &words_tested, test_area_size);


   /*-------------------------------------------*/
   /* now, read the values back and verify them */
   /*-------------------------------------------*/

   /* initialize the control and address registers on the BIM */
   *ind_control = IND_READ | IND_DSPMEM | IND_AUTOINC;
   *ind_address = TEST_AREA_START;
   words_tested = 0;     /* re-initialize the counter of words tested */

   /* read and verify the first 64K words */
   error = read_64k_words(START_PATTERN_1, secondary_ptr, &words_tested,
                          test_area_size, tu_number);
   if (error || (test_area_size <= words_tested)) return(error);

   /* read and verify the second 64K words */
   error = read_64k_words(START_PATTERN_2, secondary_ptr, &words_tested,
                          test_area_size, tu_number);
   if (error || (test_area_size <= words_tested)) return(error);

   /* read and verify the third 64K words */
   error = read_64k_words(START_PATTERN_3, secondary_ptr, &words_tested,
                          test_area_size, tu_number);
   if (error || (test_area_size <= words_tested)) return(error);

   /* read and verify the last 64K words */
   error = read_64k_words(START_PATTERN_4, secondary_ptr, &words_tested,
                          test_area_size, tu_number);
   return(error);         /* return with indication of successfulness of test */
} /* end host_mem_test() */



/******************************************************************************/
/*
 * NAME: write_64k_words
 *
 * FUNCTION: This function will write values to as much of the next
 *           64K words of memory as allowed, given the TEST_AREA_START
 *           and the test_area_size.  Given the starting pattern
 *           (next_256_start_value), this routine will increment each
 *           successive value to be written by 0x01010101 within a 256
 *           word block.
 *
 * EXECUTION ENVIRONMENT: AIX 3.2 during diagnostics
 *
 * NOTES:
 *
 * RECOVERY OPERATION: None.
 *
 * DATA STRUCTURES: None.
 *
 * RETURNS: This function always returns 0
 *
 * INPUT: next_256_start_value - pattern value to start with
 *        *words_tested - count of how many words have been tested
 *        test_area_size - number of words of memory to be tested (max 256K)
 *
 * OUTPUT: return code of 0
 *
 */

int write_64k_words(unsigned long next_256_start_value,
                    unsigned long *words_tested,
                    unsigned long test_area_size)
{
  unsigned long next_value;
  int i, j;

  /* write patterns to up to 64K words, or until the specified number of words
  have been written to. */
  for (i = 0; (i < 256) && (test_area_size > *words_tested); i++) {
    next_value = next_256_start_value;  /* assign next pattern value */

    /* write patterns to up to 256 words, or until the specified number of
    words have been written to. */
    for (j = 0; (j < 256) && (test_area_size > *words_tested); j++) {
      *ind_data = next_value;   /* write the pattern value to memory */
      next_value += INC_VALUE;   /* determine the next value to be written */
      (*words_tested)++;      /* keep track of # words that have been written */
    } /* endfor */

    next_256_start_value += INC_VALUE;  /* increment start value for next 256 */
  } /* endfor */

  return(0);
} /* end write_64k_words() */



/******************************************************************************/
/*
 * NAME: read_64k_words
 *
 * FUNCTION: This function will read and verify the next 64k words of memory
 *           (as much as is allowed by test_area_size).
 *
 * EXECUTION ENVIRONMENT: AIX 3.2 during diagnostics
 *
 * NOTES:
 *
 * RECOVERY OPERATION: None.
 *
 * DATA STRUCTURES: None
 *
 * RETURNS: This function returns zero if successful, else will return error
 *          code along with secondary information.
 *
 * INPUT: next_256_start_value - pattern value to start with
 *        secondary_ptr - Pointer for Secondary Return Codes.
 *        *words_tested - count of how many words have been tested
 *        test_area_size - number of words of memory to be tested (max 256K)
 *
 * OUTPUT:   return code of 0
 *               -OR-
 *           secondary_ptr[0] - failing address
 *           secondary_ptr[1] - expected data
 *           secondary_ptr[2] - actual data
 *           return code of ADDRESS_TEST_FAILURE_xx   (xx is 04 or 34)
 */

int read_64k_words(unsigned long next_256_start_value,
                   unsigned long *secondary_ptr,
                   unsigned long *words_tested,
                   unsigned long test_area_size,
                   int tu_number)
{
  int i, j, error;
  unsigned long actual, expected;      /* data values read and calculated */

  error = FALSE;                       /* default is "no errors" */

  /* calculate the data patterns and compare against what is read back */
  for (i = 0; (i < 256) && (!error) && (test_area_size > *words_tested); i++) {
    expected = next_256_start_value;   /* assign next pattern value */

    /* read back the next 256 words and verify the values */
    for (j = 0; (j<256) && (!error) && (test_area_size > *words_tested); j++) {
      actual = *ind_data;             /* read the data from the next location */
      error = actual != expected;   /* is the data as expected? */
      expected += INC_VALUE;        /* calculate the next expected value */
      (*words_tested)++;            /* increment the RAM address counter */
    } /* endfor */

    next_256_start_value += INC_VALUE;  /* increment start value for next 256 */
  } /* endfor */

   /* if an error occurred, save the error data */
   if (error) {
     secondary_ptr[0] = TEST_AREA_START + (*words_tested)-1;  /* failing addr */
     secondary_ptr[1] = expected - INC_VALUE;          /* calc expected value */
     secondary_ptr[2] = actual;
     /* exit with the error code appropriate for this TU */
     if (tu_number == TU_04) {
        return(ADDRESS_TEST_FAILURE_04);  /* exit with TU_04 error code */
     } else {
        return(ADDRESS_TEST_FAILURE_34);  /* exit with TU_34 error code */
     } /* endif */
   } /* endif */

   return(NO_ERROR);
} /* end read_64k_words() */
