static char sccsid[] = "src/bos/diag/tu/dca/tu001.c, tu_tca, bos411, 9428A410j 6/19/91 15:30:35";
/*
 * COMPONENT_NAME: (TU_TCA) TCA/DCA Test Unit
 *
 * FUNCTIONS: c327DiagRegTest, tu001
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/devinfo.h>
#include <sys/io3270.h>
#include <errno.h>

#include "tcatst.h"

#ifdef debugg
extern void detrace();
#endif

/* define which type of register tests are available */
typedef enum
   {
	DMSEC,           /* delay "register" milliseconds */
	WRITE,           /* write a register */
	TEST,            /* write a register, read back and test for match */
	READ,            /* read a register and check for value */
	ENDOFTABLE       /* marks end of table */
   } REG_TEST_ACTION;

/* table of constants used to drive the register tests */
/* the following define must be changed if table contains more than 256 */
#define NUM_REG_TESTS 256

struct
   {
	REG_TEST_ACTION action; /* WRITE     TEST       READ         DMSEC */
	BYTE regno;		/* regno     regno      regno        msec  */
	BYTE data;		/* written   written    n/a          n/a   */
	BYTE mask;		/* n/a       mask       mask         n/a   */
	BYTE expect;		/* n/a       compared   compared     n/a   */
   } reg_test_table [NUM_REG_TESTS] =
	{
  {WRITE, adapter_conn_ctrl_reg, 0xA0, 0xFF, 0x00}, /* into test mode    000*/
  {WRITE, adapter_conn_ctrl_reg, 0x80, 0xFF, 0x00}, /* and back out      000*/
  {DMSEC, 50,                    0x00, 0xFF, 0x00}, /* wait 50 msec      000*/
  {TEST,  adapter_intr_stat_reg, 0x3F, 0xFF, 0x00}, /* clear intr/stat   000*/
  {READ,  adapter_lsb_cur_reg,   0x00, 0xFF, 0x50}, /* check low addr    000*/
  {READ,  adapter_msb_cur_reg,   0x00, 0xFF, 0x00}, /* check high addr   000*/
  {READ,  adapter_conn_ctrl_reg, 0x00, 0xFF, 0x80}, /* check conn/ctrl   000*/
  {TEST,  adapter_conn_ctrl_reg, 0xC0, 0xFF, 0xC0}, /* test  conn/ctrl   000*/
  {TEST,  adapter_conn_ctrl_reg, 0x90, 0xFF, 0x90}, /* test  conn/ctrl   000*/
  {TEST,  adapter_conn_ctrl_reg, 0x88, 0xFF, 0x98}, /* test  conn/ctrl   000*/
  {TEST,  adapter_conn_ctrl_reg, 0x84, 0xFF, 0x9C}, /* test  conn/ctrl   000*/
  {TEST,  adapter_conn_ctrl_reg, 0x82, 0xFF, 0x9A}, /* test  conn/ctrl   000*/
  {WRITE, adapter_conn_ctrl_reg, 0xA0, 0xFF, 0x00}, /* write conn/ctrl   000*/
  {DMSEC, 50,                    0x00, 0xFF, 0x00}, /* wait 50 msec      000*/
  {READ,  adapter_conn_ctrl_reg, 0x00, 0xFF, 0xA0}, /* check conn/ctrl   000*/
  {TEST,  adapter_scan_code_reg, 0x80, 0xFF, 0x80}, /* test  scan code   000*/
  {TEST,  adapter_scan_code_reg, 0x40, 0xFF, 0x40}, /* test  scan code   000*/
  {TEST,  adapter_scan_code_reg, 0x20, 0xFF, 0x20}, /* test  scan code   000*/
  {TEST,  adapter_scan_code_reg, 0x10, 0xFF, 0x10}, /* test  scan code   000*/
  {TEST,  adapter_scan_code_reg, 0x08, 0xFF, 0x08}, /* test  scan code   000*/
  {TEST,  adapter_scan_code_reg, 0x04, 0xFF, 0x04}, /* test  scan code   000*/
  {TEST,  adapter_scan_code_reg, 0x02, 0xFF, 0x02}, /* test  scan code   000*/
  {TEST,  adapter_scan_code_reg, 0x01, 0xFF, 0x01}, /* test  scan code   000*/
  {WRITE, adapter_scan_code_reg, 0x00, 0xFF, 0x00}, /* write scan code   000*/
  {TEST,  adapter_term_id_reg,   0x80, 0xFF, 0x80}, /* test  term id     000*/
  {TEST,  adapter_term_id_reg,   0x40, 0xFF, 0x40}, /* test  term id     000*/
  {TEST,  adapter_term_id_reg,   0x20, 0xFF, 0x20}, /* test  term id     000*/
  {TEST,  adapter_term_id_reg,   0x10, 0xFF, 0x10}, /* test  term id     000*/
  {TEST,  adapter_term_id_reg,   0x08, 0xFF, 0x08}, /* test  term id     000*/
  {TEST,  adapter_term_id_reg,   0x04, 0xFF, 0x04}, /* test  term id     000*/
  {TEST,  adapter_term_id_reg,   0x02, 0xFF, 0x02}, /* test  term id     000*/
  {TEST,  adapter_term_id_reg,   0x01, 0xFF, 0x01}, /* test  term id     000*/
  {WRITE, adapter_term_id_reg,   0xFF, 0xFF, 0x00}, /* write term id     000*/
  {READ,  adapter_io_ctrl_reg,   0x00, 0x03, 0x00}, /* check io ctrl     000*/
  {WRITE, adapter_io_ctrl_reg,   0x02, 0xFF, 0x00}, /* write io ctrl     000*/
  {READ,  adapter_io_ctrl_reg,   0x00, 0x03, 0x02}, /* check io ctrl     000*/
  {WRITE, adapter_io_ctrl_reg,   0x01, 0xFF, 0x00}, /* write io ctrl     000*/
  {READ,  adapter_io_ctrl_reg,   0x00, 0x03, 0x01}, /* check io ctrl     000*/
  {TEST,  0x1B,                  0x8F, 0xFF, 0x8F}, /* test  intr mask   000*/
  {TEST,  0x1B,                  0x4F, 0xFF, 0x4F}, /* test  intr mask   000*/
  {TEST,  0x1B,                  0x2F, 0xFF, 0x2F}, /* test  intr mask   000*/
  {TEST,  0x1B,                  0x1F, 0xFF, 0x1F}, /* test  intr mask   000*/
  {WRITE, 0x1B,                  0xFF, 0xFF, 0x00}, /* write intr mask   000*/
  {TEST,  0x1C,                  0x10, 0xFF, 0x10}, /* test  featr ctrl  000*/
  {TEST,  0x1C,                  0x08, 0xFF, 0x08}, /* test  featr ctrl  000*/
  {TEST,  0x1C,                  0x04, 0xFF, 0x04}, /* test  featr ctrl  000*/
  {TEST,  0x1C,                  0x02, 0xFF, 0x02}, /* test  featr ctrl  000*/
  {TEST,  0x1C,                  0x01, 0xFF, 0x01}, /* test  featr ctrl  000*/
  {WRITE, 0x1C,                  0x00, 0xFF, 0x00}, /* write featr ctrl  000*/
  {TEST,  0x1D,                  0x10, 0xFF, 0x10}, /* test  buf select  000*/
  {TEST,  0x1D,                  0x08, 0xFF, 0x08}, /* test  buf select  000*/
  {TEST,  0x1D,                  0x04, 0xFF, 0x04}, /* test  buf select  000*/
  {TEST,  0x1D,                  0x02, 0xFF, 0x02}, /* test  buf select  000*/
  {TEST,  0x1D,                  0x01, 0xFF, 0x01}, /* test  buf select  000*/
  {WRITE, 0x1D,                  0x1B, 0xFF, 0x00}, /* write buf select  000*/
  {WRITE, adapter_io_ctrl_reg,   0x02, 0xFF, 0x00}, /* write io ctrl     000*/
  {READ,  adapter_io_ctrl_reg,   0x00, 0x03, 0x02}, /* check io ctrl     000*/
  {TEST,  0x21,                  0x10, 0xFF, 0x10}, /* test  cut/dft buf 000*/
  {TEST,  0x21,                  0x08, 0xFF, 0x08}, /* test  cut/dft buf 000*/
  {TEST,  0x21,                  0x04, 0xFF, 0x04}, /* test  cut/dft buf 000*/
  {TEST,  0x21,                  0x02, 0xFF, 0x02}, /* test  cut/dft buf 000*/
  {TEST,  0x21,                  0x01, 0xFF, 0x01}, /* test  cut/dft buf 000*/
  {WRITE, 0x21,                  0x1B, 0xFF, 0x00}, /* write cut/dft buf 000*/
  {TEST,  0x22,                  0x80, 0xFF, 0x80}, /* test  psea lo     000*/
  {TEST,  0x22,                  0x40, 0xFF, 0x40}, /* test  psea lo     000*/
  {TEST,  0x22,                  0x20, 0xFF, 0x20}, /* test  psea lo     000*/
  {TEST,  0x22,                  0x10, 0xFF, 0x10}, /* test  psea lo     000*/
  {TEST,  0x22,                  0x08, 0xFF, 0x08}, /* test  psea lo     000*/
  {TEST,  0x22,                  0x04, 0xFF, 0x04}, /* test  psea lo     000*/
  {TEST,  0x22,                  0x02, 0xFF, 0x02}, /* test  psea lo     000*/
  {TEST,  0x22,                  0x01, 0xFF, 0x01}, /* test  psea lo     000*/
  {WRITE, 0x22,                  0xFF, 0xFF, 0x00}, /* write psea lo     000*/
  {TEST,  0x23,                  0x20, 0xFF, 0x20}, /* test  psea hi     000*/
  {TEST,  0x23,                  0x10, 0xFF, 0x10}, /* test  psea hi     000*/
  {TEST,  0x23,                  0x08, 0xFF, 0x08}, /* test  psea hi     000*/
  {TEST,  0x23,                  0x04, 0xFF, 0x04}, /* test  psea hi     000*/
  {TEST,  0x23,                  0x02, 0xFF, 0x02}, /* test  psea hi     000*/
  {TEST,  0x23,                  0x01, 0xFF, 0x01}, /* test  psea hi     000*/
  {WRITE, 0x23,                  0x0F, 0xFF, 0x00}, /* write psea hi     000*/
  {TEST,  0x24,                  0x80, 0xFF, 0x80}, /* test  mlea lo     000*/
  {TEST,  0x24,                  0x40, 0xFF, 0x40}, /* test  mlea lo     000*/
  {TEST,  0x24,                  0x20, 0xFF, 0x20}, /* test  mlea lo     000*/
  {TEST,  0x24,                  0x10, 0xFF, 0x10}, /* test  mlea lo     000*/
  {TEST,  0x24,                  0x08, 0xFF, 0x08}, /* test  mlea lo     000*/
  {TEST,  0x24,                  0x04, 0xFF, 0x04}, /* test  mlea lo     000*/
  {TEST,  0x24,                  0x02, 0xFF, 0x02}, /* test  mlea lo     000*/
  {TEST,  0x24,                  0x01, 0xFF, 0x01}, /* test  mlea lo     000*/
  {WRITE, 0x24,                  0xFF, 0xFF, 0x00}, /* write mlea lo     000*/
  {TEST,  0x25,                  0x20, 0xFF, 0x20}, /* test  mlea hi     000*/
  {TEST,  0x25,                  0x10, 0xFF, 0x10}, /* test  mlea hi     000*/
  {TEST,  0x25,                  0x08, 0xFF, 0x08}, /* test  mlea hi     000*/
  {TEST,  0x25,                  0x04, 0xFF, 0x04}, /* test  mlea hi     000*/
  {TEST,  0x25,                  0x02, 0xFF, 0x02}, /* test  mlea hi     000*/
  {TEST,  0x25,                  0x01, 0xFF, 0x01}, /* test  mlea hi     000*/
  {WRITE, 0x25,                  0x0F, 0xFF, 0x00}, /* write mlea hi     000*/
  {READ,  0x26,                  0x00, 0xFF, 0x00}, /* check suppr skp   000*/
  {TEST,  0x27,                  0x80, 0xFF, 0x80}, /* test  work stat   000*/
  {TEST,  0x27,                  0x40, 0xFF, 0x40}, /* test  work stat   000*/
  {TEST,  0x27,                  0x08, 0xFF, 0x08}, /* test  work stat   000*/
  {TEST,  0x27,                  0x04, 0xFF, 0x04}, /* test  work stat   000*/
  {TEST,  0x27,                  0x02, 0xFF, 0x02}, /* test  work stat   000*/
  {TEST,  0x27,                  0x01, 0xFF, 0x01}, /* test  work stat   000*/
  {WRITE, 0x27,                  0x00, 0xFF, 0x00}, /* write work stat   000*/
  {TEST,  0x29,                  0x10, 0xFF, 0x10}, /* test  wrap ctrl   000*/
  {TEST,  0x29,                  0x08, 0xFF, 0x08}, /* test  wrap ctrl   000*/
  {TEST,  0x29,                  0x04, 0xFF, 0x04}, /* test  wrap ctrl   000*/
  {WRITE, 0x29,                  0x00, 0xFF, 0x00}, /* write wrap ctrl   000*/
  {WRITE, adapter_io_ctrl_reg,   0x00, 0xFF, 0x00}, /* write io ctrl     000*/
  {READ,  adapter_io_ctrl_reg,   0x00, 0x03, 0x00}, /* check io ctrl     000*/
  {WRITE, adapter_conn_ctrl_reg, 0x80, 0xFF, 0x00}, /* and back out      000*/
  {DMSEC, 50,                    0x00, 0xFF, 0x00}, /* wait 50 msec      000*/
  {ENDOFTABLE, 0,                0x00, 0x00, 0x00}  /* MARK END OF TABLE 000*/

	};

/*
 * NAME: c3270DiagRegTest
 *
 * FUNCTION: 3270 Connection Adapter test of adapter registers
 *
 * EXECUTION ENVIRONMENT:
 *
 * (NOTES:) This function will test the 3270 Connection Adapter registers
 *          by Writing, Testing and Reading the registers.  The tests will
 *          performed or driven by the table of tests defined above
 *
 * RETURNS: A zero (0) when successful or 0x0104, 0x0105, 0x0106 or 0x0107
 *          on error condition.
 *
 */
int c327DiagRegTest (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {

	int ndx, rc;
	C327DIAG_DATA diag_data;
	struct htx_data *htx_sp;

	extern int writereg();
	extern int testreg();
	extern int readreg();
	extern void short_delay();
	extern int mktu_rc();

	/*
	 * set up a pointer to HTX data structure to
	 * increment counters in case tu was invoked by
	 * hardware exerciser.
	 */
	htx_sp = tucb_ptr->tca_htx_s.htx_sp;

	/* totally table-driven from table defined above */
	for (ndx = 0; (ndx < NUM_REG_TESTS) &&
		      (reg_test_table[ndx].action < ENDOFTABLE); ndx++)
	   {
		switch (reg_test_table[ndx].action)
		   {
			case WRITE:
				rc = writereg(fdes,
					   &diag_data, ndx,
					   reg_test_table[ndx].regno & 0x0F,
					   reg_test_table[ndx].data);
				if (rc)
				   {
					if (htx_sp != NULL)
						(htx_sp->bad_others)++;

					return(mktu_rc(tucb_ptr->header.tu,
						LOG_ERR, 0x0104));
				   }
				if (htx_sp != NULL)
					(htx_sp->good_others)++;
				break;

			case TEST:
				rc = testreg(fdes,
					   &diag_data, ndx,
					   reg_test_table[ndx].regno & 0x0F,
					   reg_test_table[ndx].data,
					   reg_test_table[ndx].mask,
					   reg_test_table[ndx].expect);
				if (rc)
				   {
					if (htx_sp != NULL)
						(htx_sp->bad_others)++;
					return(mktu_rc(tucb_ptr->header.tu,
						LOG_ERR, 0x0105));
				   }
				if (htx_sp != NULL)
					(htx_sp->good_others)++;
				break;

			case READ:
				rc = readreg(fdes,
					   &diag_data, ndx,
					   reg_test_table[ndx].regno & 0x0F,
					   reg_test_table[ndx].mask,
					   reg_test_table[ndx].expect);
				if (rc)
				   {
					if (htx_sp != NULL)
						(htx_sp->bad_others)++;
					return(mktu_rc(tucb_ptr->header.tu,
						LOG_ERR, 0x0106));
				   }
				if (htx_sp != NULL)
					(htx_sp->good_others)++;
				break;

			case DMSEC:
				short_delay(reg_test_table[ndx].regno);
				break;

			case ENDOFTABLE:
				return(0);
			default:
#ifdef C327DIAG_DEBUG_PRINT_MODE
	printf("Invalid test type in register test table at %d\n", ndx);
#endif
				return(mktu_rc(tucb_ptr->header.tu,
					LOG_ERR, 0x0107));
		   } /* end switch */
	   } /* end for */

	/* all tests passed */
	return(0);
   }

/*
 * NAME: tu001
 *
 * FUNCTION: 3270 Connection Adapter test of adapter registers
 *
 * EXECUTION ENVIRONMENT:
 *
 * (NOTES:) 
 *
 * RETURNS: The return code from the c327DiagRegTest function call.
 *
 */
int tu001 (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {

	return(c327DiagRegTest(fdes, tucb_ptr));
   }
