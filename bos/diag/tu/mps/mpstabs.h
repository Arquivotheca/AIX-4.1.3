/* @(#)40       1.2  src/bos/diag/tu/mps/mpstabs.h, tu_mps, bos412, 9445C412a 11/7/94 14:00:56 */
/*****************************************************************************
 * COMPONENT_NAME: (tu_mps)  Wildwood LAN adapter test units
 *
 * FUNCTIONS: MPS I/O Register Table Definitions Header File
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *****************************************************************************/

/***************************************************************************/
/*                I/O Register Table Definitions                           */
/***************************************************************************/
#define NUMREGS   11

struct _io_table
 {
   uint         read_addr;       /*  Address for read access to register       */
   uint         write_addr;      /*  Address for write access to register      */
   uint         errcode;         /*  Register error code                       */
   ushort       test_val[4];     /*  Test Value                                */
   ushort       exp_val[4];      /*  Expected Return Value                     */
 };

/***************************************************************************/
/*                I/O Register Table Definitions                           */
/***************************************************************************/
struct _io_table  io_table [] =
 {
  {BCtl      , BCtl      , BCtl_ERR      , {0x2a0a, 0x1605, 0x3e0f, 0x0203} , {0x2a0a, 0x1605, 0x3e0f, 0x0203} },
  {LISR      , LISR      , LISR_ERR      , {0xaaaa, 0x0555, 0xafff, 0x2333} , {0xaaaa, 0x0555, 0xafff, 0x2333} },
  {LISR      , LISR_SUM  , LISR_SUM_ERR  , {0x8888, 0x2222, 0x0000, 0x0555} , {0x8888, 0xaaaa, 0xaaaa, 0xafff} },
  {SISR      , SISR      , SISR_ERR      , {0x2aaa, 0x5555, 0x7fff, 0x3333} , {0x2aaa, 0x5555, 0x7fff, 0x3333} },
  {SISR      , SISR_SUM  , SISR_SUM_ERR  , {0x2888, 0x1222, 0x0000, 0x4555} , {0x2888, 0x3aaa, 0x3aaa, 0x7fff} },
  {SISRM     , SISRM     , SISRM_ERR     , {0xaaaa, 0x5555, 0x7fff, 0x3333} , {0xaaaa, 0x5555, 0x7fff, 0x3333} },
  {SISRM     , SISRM_SUM , SISRM_SUM_ERR , {0x4888, 0x2222, 0x0000, 0x1555} , {0x4888, 0x6aaa, 0x6aaa, 0x7fff} },
  {MISRM_SUM , MISRM_SUM , MISRM_SUM_ERR , {0x4422, 0x1111, 0x0088, 0x2204} , {0x4422, 0x5533, 0x5533, 0x7737} },
  {LAPE      , LAPE      , LAPE_ERR      , {0xaa82, 0x5501, 0xff83, 0x3381} , {0x0082, 0x0001, 0x0083, 0x0081} },
  {Timer     , Timer     , Timer_ERR     , {0x20aa, 0x1055, 0x00ff, 0x0033} , {0x20aa, 0x1055, 0x00ff, 0x0033} },
  {BMCtl_SUM , BMCtl_SUM , BMCtl_SUM_ERR , {0xaa00, 0x5540, 0xffff, 0x3333} , {0x4440, 0x4440, 0x4440, 0x4440} }
 };

/***************************************************************************/
/*                I/O RUM Register Table Definitions                       */
/***************************************************************************/
#define NUM_RUM_REGS   5

struct _io_table  io_rum_table [] =
 {
  {LISR       , LISR_RUM   ,  LISR_RUM_ERR   , {0xafee, 0x8dcc, 0xffff, 0x3233} , {0xafee, 0x8dcc, 0x8dcc, 0x0000} },
  {SISR       , SISR_RUM   ,  SISR_RUM_ERR   , {0x6eee, 0x4ccc, 0xffff, 0x3333} , {0x6eee, 0x4ccc, 0x4ccc, 0x0000} },
  {SISRM      , SISRM_RUM  ,  SISRM_RUM_ERR  , {0xeeee, 0xcccc, 0xffff, 0x3333} , {0xeeee, 0xcccc, 0xcccc, 0x0000} },
  {MISRM_SUM  , MISRM_RUM  ,  MISRM_RUM_ERR  , {0x6626, 0x4406, 0xffff, 0x3331} , {0x6626, 0x4406, 0x4406, 0x0000} },
  {BMCtl_SUM  , BMCtl_RUM  ,  BMCtl_RUM_ERR  , {0xeeee, 0xcccc, 0xffff, 0x3333} , {0x4440, 0x4440, 0x4440, 0x0000} }
 };


/***************************************************************************/
/*                I/O Bus Master Register Table Definitions                       */
/***************************************************************************/
#define NUM_BUSM_REGS   12

struct _io_table  io_busm_table [] =
 {
  {RxLBDA_LO , RxLBDA_LO , RxLBDA_LO_ERR   ,  {0xaaa0, 0x5550, 0xfff0, 0x3330} , {0xaaa0, 0x5550, 0xfff0, 0x3330} },
  {RxLBDA_HI , RxLBDA_HI , RxLBDA_HI_ERR   ,  {0xaaaa, 0x5555, 0xffff, 0x3333} , {0xaaaa, 0x5555, 0xffff, 0x3333} },
  {RxBDA_LO  , RxBDA_LO  , RxBDA_LO_ERR    ,  {0xaaa0, 0x5550, 0xfff0, 0x3330} , {0xaaa0, 0x5550, 0xfff0, 0x3330} },
  {RxBDA_HI  , RxBDA_HI  , RxBDA_HI_ERR    ,  {0xaaaa, 0x5555, 0xffff, 0x3333} , {0xaaaa, 0x5555, 0xffff, 0x3333} },
  {Tx1LFDA_LO, Tx1LFDA_LO, Tx1LFDA_LO_ERR  ,  {0xaaa0, 0x5550, 0xfff0, 0x3330} , {0xaaa0, 0x5550, 0xfff0, 0x3330} },
  {Tx1LFDA_HI, Tx1LFDA_HI, Tx1LFDA_HI_ERR  ,  {0xaaaa, 0x5555, 0xffff, 0x3333} , {0xaaaa, 0x5555, 0xffff, 0x3333} },
  {Tx1FDA_LO , Tx1FDA_LO , Tx1FDA_LO_ERR   ,  {0xaaa0, 0x5550, 0xfff0, 0x3330} , {0xaaa0, 0x5550, 0xfff0, 0x3330} },
  {Tx1FDA_HI , Tx1FDA_HI , Tx1FDA_HI_ERR   ,  {0xaaaa, 0x5555, 0xffff, 0x3333} , {0xaaaa, 0x5555, 0xffff, 0x3333} },
  {Tx2LFDA_LO, Tx2LFDA_LO, Tx2LFDA_LO_ERR  ,  {0xaaa0, 0x5550, 0xfff0, 0x3330} , {0xaaa0, 0x5550, 0xfff0, 0x3330} },
  {Tx2LFDA_HI, Tx2LFDA_HI, Tx2LFDA_HI_ERR  ,  {0xaaaa, 0x5555, 0xffff, 0x3333} , {0xaaaa, 0x5555, 0xffff, 0x3333} },
  {Tx2FDA_LO , Tx2FDA_LO , Tx2FDA_LO_ERR   ,  {0xaaa0, 0x5550, 0xfff0, 0x3330} , {0xaaa0, 0x5550, 0xfff0, 0x3330} },
  {Tx2FDA_HI , Tx2FDA_HI , Tx2FDA_HI_ERR   ,  {0xaaaa, 0x5555, 0xffff, 0x3333} , {0xaaaa, 0x5555, 0xffff, 0x3333} }
 };
