/* @(#)84       1.4  src/bos/kernext/disp/ped/pedmacro/hw_trace.c, pedmacro, bos411, 9428A410j 3/24/94 13:55:28 */

/*
 * COMPONENT_NAME: PEDMACRO
 *
 * FUNCTIONS: Generates the ASCII and BINARY traces.
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

#include   <stdio.h>
#include   <sys/types.h>
#include   <sys/errno.h>                        /* Error definitions         */
#include   <sys/rcm_win.h>
#include   <sys/aixgsc.h>
#include   <mid/hw_trace.h>
#include   <mid/hw_timer.h>
#include   <mid/hw_ind_mac.h>
#include   <mid/hw_dsp.h>
#include   <mid/hw_addrs.h>
#include   <mid/hw_names.h>
#include   <mid/hw_iodefs.h>

/***************************************************************************/
/*                             Define Statements                           */
/***************************************************************************/

#define    BINARY_FILE_ID       0xBBBBBBBB
#define    IND_AUTOINC          0x20
#define    COMMENT_LEN          35
#define    MAX_STATES           3
#define    ONE_SEC_STAMP        1000000000
#define    HALF_SEC_STAMP        500000000
#define    MAX_ULONG            0xFFFFFFFF

#define   ALWAYS_FAILS          0
#define   ALWAYS_PASSES         1
#define   TEST_CONDITION       -1

#define   TEST_FAILED           0
#define   TEST_PASSED           1

#define OPTIONS_ON(name)   {name, 0, MAX_ULONG, -1, -1,         \
	                    ALWAYS_PASSES, ALWAYS_PASSES,       \
	                    ALWAYS_PASSES, ALWAYS_PASSES,       \
	                    TEST_PASSED, TEST_PASSED,           \
	                    TEST_PASSED, TEST_PASSED, 0}

#define OPTIONS_OFF(name)  {name, MAX_ULONG, 0, -1, -1,         \
	                    ALWAYS_FAILS, ALWAYS_FAILS,         \
	                    ALWAYS_PASSES, ALWAYS_PASSES,       \
	                    TEST_FAILED, TEST_FAILED,           \
	                    TEST_PASSED, TEST_PASSED, 0}

/***************************************************************************/
/*                             Type Definitions                            */
/***************************************************************************/

typedef struct
{
  char    *name;      /* Name of option */
  ulong   rng_min;    /* Minimum range to enable option */
  ulong   rng_max;    /* Maximum range to enable option */
  int     usr_num;    /* User id filter number (-1 = disabled) */
  int     opc_num;    /* Opcode  filter number (-1 = disabled) */
  int     all_cond;   /* TEST_CONDITION, ALWAYS_PASSES, or ALWAYS_FAILES */
  int     rng_cond;   /* TEST_CONDITION, ALWAYS_PASSES, or ALWAYS_FAILES */
  int     usr_cond;   /* TEST_CONDITION, ALWAYS_PASSES, or ALWAYS_FAILES */
  int     opc_cond;   /* TEST_CONDITION, ALWAYS_PASSES, or ALWAYS_FAILES */
  int     all_pass;   /* TEST_FAILED or TEST_PASSED */
  int     rng_pass;   /* TEST_FAILED or TEST_PASSED */
  int     usr_pass;   /* TEST_FAILED or TEST_PASSED */
  int     opc_pass;   /* TEST_FAILED or TEST_PASSED */
  int     cnt_pass;   /* Count of tests which passed */
} OPTIONS;

typedef struct
{
  int     last_opcode;
  int     fifo_len_ctr;
  char    fifo_op_name[35];
  int     pcb_len_ctr;
  char    pcb_op_name[35];
  int     ted_len_ctr;
  char    ted_op_name[35];
  int     hcr_len_ctr;
  char    hcr_op_name[35];
} SE_PTRS;

typedef struct
{
  int     save_restore_flag;
  ulong   tr_count;
  int     mid_trace_flags;
  int     mid_gen_ts;
  double  timeold;
  FILE    *asc_file;
  FILE    *bin_file;
  SE_PTRS show_asc_ptrs;
  SE_PTRS file_asc_ptrs;
  OPTIONS file_bin_trace;
  OPTIONS file_asc_trace;
  OPTIONS show_asc_trace;
} STATE;


/***************************************************************************/
/*                            Global Definitions                           */
/***************************************************************************/

int     mid_trace_flags = MID_TRACE_FILE_BIN |
	                  MID_TRACE_FILE_ASC |
	                  MID_TRACE_SHOW_ASC;

int     (*condhdl)();
char    *cvregname();
char    *cvposname();
char    *cvdmaname();
char    *cvhcrname();
char    *cvascbname();
char    *cvadrname();
char    *ind_cmnt();
char    *getenv();

FILE    *out_file = NULL;

FILE    *asc_file = NULL;
char    *asc_name = "mid_trace.asc";

FILE    *bin_file = NULL;
char    *bin_name = "mid_trace.bin";

int     comment_mode = -1;
int     data_loc;

ulong   tr_count = 0;
ulong   out_count = 0;
ulong   in_count = 0;

OPTIONS file_bin_trace   = OPTIONS_ON("Binary File Trace");
OPTIONS file_asc_trace   = OPTIONS_ON("ASCII File Trace");
OPTIONS show_asc_trace   = OPTIONS_ON("ASCII Screen Trace");

SE_PTRS init_asc_ptrs;
SE_PTRS file_asc_ptrs;
SE_PTRS show_asc_ptrs;

/***************************************************************************/
/*                             Local Definitions                           */
/***************************************************************************/

static  int     i;
static  int     adr;
static  int     adr_ind;
static  int     asc_flag;
static  int     hooktype;
static  int     out_hook;
static  int     user_id;
static  int     user_op;
static  int     opcode;
static  int     out_enable = -1;
static  int     bin_id = BINARY_FILE_ID;
static  char    *blank_line = "\n";
static  char    mark = '`';

static  char    badaddr[9];
static  char    ind_name[14];
static  char    ind_com[35];

static  int     ird_len_ctr = -1;
static  int     iwr_len_ctr = -1;

static  int     mid_gen_ts = MID_TRACE_TIME_STAMP;
static  long    tsec;
static  long    timewrap;
static  long    timer_wraps = 0;
static  int     stamp_enable;
static  double  timeold;
static  double  timenow;
static  char    blanks[] = "                  ";
static  char    maskstamp[18];
static  int     time_stamp_bin[] = {MID_TRACE_HOOK_TIME_STAMP,0,
	                            MID_TRACE_HOOK_TIME_STAMP,0};

static  char    comment1[80];
static  char    comment2[80];

static  STATE   states[MAX_STATES];

/***************************************************************************/
/* The device driver has problems with printf formatting.  It cannot left  */
/* justify values and it does not handle maximum field widths.  The        */
/* following defines are to help overcome these problems.                  */
/***************************************************************************/

#if MID_DD

#define FORMAT_COMMENT            "%c  %s [%4u]{%c}"
#define FORMAT_OP                 "%c                         %s"
#define FORMAT_OP_TIME            "%c  %13s                   %8X "
#define FORMAT_OP_VAL             "%c                %8X %s"
#define FORMAT_OP_REG             "%c  %13s          %s"
#define FORMAT_OP_REG_VAL         "%c  %13s %8X %s"
#define FORMAT_OP_REG_BLANK       "%c  %13s %8X          %s"
#define FORMAT_OP_POS_VAL         "%c  %13s       %2X %s"
#define FORMAT_OP_VAL_VAL_VAL     "\n%c                %8X %8X %8X "
#define FORMAT_IND_VAL            "Addr 0x%8X = 0x%8X"
#define FORMAT_NL_NAME            "\n%c                                           "
#define FORMAT_OP_OFFSET          "\n%c  %4X "
#define FORMAT_HEX8               "%8X "
#define FORMAT_HEX18              "%8X          "
#define FORMAT_MARK_HEX8          "%c%8X"
#define FORMAT_BLANK_HEX8         " %8X"
#define FORMAT_COMMENT_TRCNT_USR  "# %23s [%4u]{%c}"
#define FORMAT_COMMENT_SECNT_USR  "# %23s (%x){%c}"
#define FORMAT_IND_DATA           "IND_DATA.%4X"

#else /* not MID_DD */

#define FORMAT_COMMENT            "%c  %-66.66s [%4.4u]{%c}"
#define FORMAT_OP                 "%-25c %s"
#define FORMAT_OP_TIME            "%c  %-31s %8.8X "
#define FORMAT_OP_VAL             "%-16c %8.8X %s"
#define FORMAT_OP_REG             "%c  %-22.22s %s"
#define FORMAT_OP_REG_VAL         "%c  %-13.13s %8.8X %s"
#define FORMAT_OP_REG_BLANK       "%c  %-13.13s %8.8X          %s"
#define FORMAT_OP_POS_VAL         "%c  %-19.19s %2.2X %s"
#define FORMAT_OP_VAL_VAL_VAL     "\n%c                %8.8X %8.8X %8.8X"
#define FORMAT_IND_VAL            "Addr 0x%8.8X = 0x%8.8X"
#define FORMAT_NL_NAME            "\n%c                                           "
#define FORMAT_OP_OFFSET          "\n%c  %4.4X"
#define FORMAT_HEX8               "%8.8X "
#define FORMAT_HEX18              "%-18.8X"
#define FORMAT_BLANK_HEX8         " %8.8X"
#define FORMAT_MARK_HEX8          "%c%8.8X"
#define FORMAT_COMMENT_TRCNT_USR  "# %-23.23s [%4.4u]{%c}"
#define FORMAT_COMMENT_SECNT_USR  "# %-23.23s (%4.4X){%c}"
#define FORMAT_IND_DATA           "IND_DATA.%4.4X"

#endif /* not MID_DD */

mid_trace(hook,parm,val1,val2,val3,val4,stamp,data)
int     hook;
int     parm;
int     val1;
int     val2;
int     val3;
int     val4;
int     stamp;
int     *data;
{
  /* Process only the 518 hooks and hooks with time stamps */
  /* Change all non 518 hooks with time stamps into 518 hooks */
  if ((hook & MID_TRACE_HOOK_ID_MASK) != HKWD_ID)
  {
    if (hook & MID_TRACE_HOOK_TIME_MASK)
    {
      hook = MID_TRACE_HOOK_TIME_STAMP;
      parm = 0;
    }
    else
    {
      return;
    }
  }

  /* Ignore type 0 hooks with zero length */
  hooktype = hook & MID_TRACE_HOOK_TYPE_MASK;
  if (hooktype == 0)
  {
    if ((hook & MID_TRACE_HOOK_BUFLEN_MASK) == 0)
      return;
  }

/*****************************************************************************/
/* If mid_gen_ts is zero, no time stamps will be printed                     */
/* If mid_gen_ts is nonzero:                                                 */
/*   If the time stamp mask is zero, no time stamp will be printed           */
/*   If the time stamp mask is nonzero:                                      */
/*     If the time stamp is not a -1, that value is printed                  */
/*     If the time stamp is -1:                                              */
/*       If we are inside the kernal, no time stamp is generated or printed  */
/*       If we are outside the kernal, a time stamp is generated and printed */
/*****************************************************************************/

  timer_wraps = 0;
  if (mid_gen_ts == 0)
  {
    hook &= ~MID_TRACE_HOOK_TIME_MASK;
  }
  else if ((hook & MID_TRACE_HOOK_TIME_MASK) && (stamp == -1))
  {
#ifdef  MID_DD          /* Kernal cannot handle floats */
    hook &= ~MID_TRACE_HOOK_TIME_MASK;
#else /* not MID_DD */
    timeold = timenow;
    MID_TIMES_SEC( timenow, tsec, stamp )
    if (timeold > 0.0)
    {
      if (timer_wraps = (int) (timenow - timeold))
	timewrap = stamp + HALF_SEC_STAMP;
      if (timewrap > ONE_SEC_STAMP)
	timewrap -= ONE_SEC_STAMP;
    }
#endif /* not MID_DD */
  }

  /* Ignore time stamp hooks if there is no time stamp and we aren't  */
  /* supposed to generate a time stamp                                */
  opcode = parm & MID_TRACE_PARM_OP_MASK;
  if ((opcode == MID_TRACE_OP_TIME_STAMP) &&
      (hook & MID_TRACE_HOOK_TIME_MASK) == 0)
  {
    return;
  }

  tr_count++;

  if (comment_mode)
  {
    blank_line = "\n";
    mark = '`';
  }
  else
  {
    blank_line = "";
    mark = ' ';
  }

#if MID_TRACE_FILE_BIN_SWITCH

  if (mid_trace_flags & MID_TRACE_FILE_BIN)
  {
    if (bin_file == NULL)
      mid_trace_file_bin_start();

    stamp_enable = -1;
    out_hook = hook;
    if (file_bin_trace.all_cond == TEST_CONDITION)
    {
      (*(condhdl))(&file_bin_trace,tr_count,user_op,-1);
      stamp_enable = 0;
      out_hook &= ~MID_TRACE_HOOK_TIME_MASK;
    }

    if (file_bin_trace.all_pass)
    {
      if (timer_wraps && stamp_enable)
      {
	time_stamp_bin[1] = stamp;
	time_stamp_bin[3] = timewrap;
	for (i=0; i<timer_wraps; i++)
	  FWRITE_ERMSG(&time_stamp_bin[0],sizeof(time_stamp_bin),1,bin_file)
      }

      FWRITE_ERMSG(&out_hook,sizeof(int),1,bin_file)

      switch (hook & MID_TRACE_HOOK_TYPE_MASK)
      {
	case MID_TRACE_TYPE_X:
	  FWRITE_ERMSG(&parm,sizeof(int),1,bin_file)
	  FWRITE_ERMSG(data,hook & MID_TRACE_HOOK_BUFLEN_MASK,1,bin_file)
	  break;
	case MID_TRACE_TYPE_0:
	  break;
	case MID_TRACE_TYPE_1:
	  FWRITE_ERMSG(&parm,sizeof(int),1,bin_file)
	  break;
	case MID_TRACE_TYPE_2:
	  FWRITE_ERMSG(&parm,sizeof(int),1,bin_file)
	  FWRITE_ERMSG(&val1,sizeof(int),1,bin_file)
	  FWRITE_ERMSG(&val2,sizeof(int),1,bin_file)
	  FWRITE_ERMSG(&val3,sizeof(int),1,bin_file)
	  FWRITE_ERMSG(&val4,sizeof(int),1,bin_file)
	  break;
      }
      if (out_hook & MID_TRACE_HOOK_TIME_MASK)
	FWRITE_ERMSG(&stamp,sizeof(int),1,bin_file)
    }
  }
#endif /* MID_TRACE_FILE_BIN_SWITCH */

#if     MID_TRACE_FILE_ASC_SWITCH
  if (mid_trace_flags & MID_TRACE_FILE_ASC)
  {
    if (asc_file == NULL)
      mid_trace_file_asc_start();

    out_asc(hook,parm,val1,val2,val3,val4,stamp,data,
      &file_asc_ptrs,&file_asc_trace,asc_file);
  }
#endif /* MID_TRACE_FILE_ASC_SWITCH */

#if     MID_TRACE_SHOW_ASC_SWITCH
  if (mid_trace_flags & MID_TRACE_SHOW_ASC)
  {
    out_asc(hook,parm,val1,val2,val3,val4,stamp,data,
      &show_asc_ptrs,&show_asc_trace,stdout);
  }
#endif /* MID_TRACE_SHOW_ASC_SWITCH */
}

#if     MID_TRACE_SHOW_ASC_SWITCH + MID_TRACE_FILE_ASC_SWITCH
out_asc(hook,parm,val1,val2,val3,val4,stamp,data,sp,op,fp)
int     hook;
int     parm;
int     val1;
int     val2;
int     val3;
int     val4;
int     stamp;
int     *data;
SE_PTRS *sp;
OPTIONS *op;
FILE    *fp;
{
  stamp_enable = -1;
  out_hook = hook;
  if (op->all_cond == TEST_CONDITION)
  {
    (*(condhdl))(op,tr_count,user_op,-1);
    stamp_enable = 0;
    out_hook &= ~MID_TRACE_HOOK_TIME_MASK;
  }

  if (fp == stdout || op->all_pass)
    op->cnt_pass++;

  out_file = fp;
  out_enable = op->all_pass;
  out_count = op->cnt_pass;

  if (timer_wraps && stamp_enable)
  {
    outtext(blank_line);
    for (i=0; i<timer_wraps; i++)
    {
      outtext(FORMAT_OP_TIME,MID_TRACE_ID_TIME_STAMP,
	MID_TRACE_NAME_TIME_STAMP,stamp);
      outcmnt("Time Delta = %d ns",stamp);
      outtext(blank_line);
      outtext(FORMAT_OP_TIME,MID_TRACE_ID_TIME_STAMP,
	MID_TRACE_NAME_TIME_STAMP,timewrap);
      outcmnt("Time Delta = %d ns",timewrap);
      outtext("\n");
    }
    sp->last_opcode = -1;
  }

  adr  = parm & MID_TRACE_PARM_ADDR_MASK;
  adr_ind = parm & (MID_TRACE_PARM_IND_MASK | MID_TRACE_PARM_ADDR_MASK);
  user_op = parm & MID_TRACE_PARM_USER_MASK;
  user_id = cvuserid(user_op);

  if (hooktype == MID_TRACE_TYPE_X)
  {
    if (opcode != sp->last_opcode)
      outtext(blank_line);

    switch (opcode)
    {
      case MID_TRACE_OP_FIFO_BUFFER:
	sp->last_opcode = -1;
	out_buf(out_hook,parm,data,stamp,&sp->fifo_len_ctr,sp->fifo_op_name,
	  MID_TRACE_ID_FIFO_BUFFER,MID_TRACE_NAME_FIFO_BUFFER);
	break;
      case MID_TRACE_OP_PCB_BUFFER:
	sp->last_opcode = -1;
	out_buf(out_hook,parm,data,stamp,&sp->pcb_len_ctr,sp->pcb_op_name,
	  MID_TRACE_ID_PCB_BUFFER,MID_TRACE_NAME_PCB_BUFFER);
	break;
      case MID_TRACE_OP_IRD_BUFFER:
	sp->last_opcode = -1;
	out_buf(out_hook,parm,data,stamp,&ird_len_ctr,ind_cmnt(adr_ind),
	  MID_TRACE_ID_IRD_BUFFER,cvregname(adr_ind));
	break;
      case MID_TRACE_OP_IWR_BUFFER:
	sp->last_opcode = -1;
	out_buf(out_hook,parm,data,stamp,&iwr_len_ctr,ind_cmnt(adr_ind),
	  MID_TRACE_ID_IWR_BUFFER,cvregname(adr_ind));
	break;
      case MID_TRACE_OP_TED_BUFFER:
	sp->last_opcode = -1;
	out_buf(out_hook,parm,data,stamp,&sp->ted_len_ctr,sp->ted_op_name,
	  MID_TRACE_ID_TED_BUFFER,MID_TRACE_NAME_TED_BUFFER);
	break;
      case MID_TRACE_OP_HCR_BUFFER:
	sp->last_opcode = -1;
	out_buf(out_hook,parm,data,stamp,&sp->hcr_len_ctr,sp->hcr_op_name,
	  MID_TRACE_ID_HCR_BUFFER,MID_TRACE_NAME_HCR_BUFFER);
	break;
      case MID_TRACE_OP_ECHO_COMMENT:
	sp->last_opcode = opcode;
	outtext(FORMAT_COMMENT,MID_TRACE_ID_ECHO_COMMENT,data,out_count,user_id);
	break;
      case MID_TRACE_OP_COMMENT:
	sp->last_opcode = opcode;
	outtext(FORMAT_COMMENT,MID_TRACE_ID_COMMENT,data,out_count,user_id);
	break;
    }
    if (comment_mode)
      outtext("\n");
    else
      outtext("\n\n");
  }
  else
  {
    strncpy(maskstamp,blanks,18);
    if ((out_hook & MID_TRACE_HOOK_HOOKLEN_MASK) >= MID_HOOKLEN_3)
      sprintf(&maskstamp[0],FORMAT_HEX18,val2);
    if (out_hook & MID_TRACE_HOOK_TIME_MASK)
      sprintf(&maskstamp[9],FORMAT_HEX8,stamp);

    if ((opcode != MID_TRACE_OP_VALUE) &&
	(adr == MID_ADR_IND_CONTROL || opcode != sp->last_opcode))
    {
      outtext(blank_line);
    }
    sp->last_opcode = opcode;

    switch (opcode)
    {
      case MID_TRACE_OP_WRITE_REG:
	outtext(FORMAT_OP_REG_VAL,MID_TRACE_ID_WRITE_REG,
	  cvregname(adr_ind),val1,maskstamp);
	switch (adr)
	{
	  case MID_ADR_IND_CONTROL:
	    outcmnt("%s, %s, %s",
	      ((val1 & 0x10) ? "Read" : "Write"),
	      ((val1 & 0x20) ? "Auto Inc" : "No Inc"),
	      ((val1 & 0x08) ? "Memory" : "Wrap"));
	    break;
	  case MID_ADR_IND_ADDRESS:
	    if (val1 >= MID_ASCB_BASE && val1 < (MID_ASCB_BASE + MID_ASCB_SIZE))
	      outcmnt(cvascbname(val1));
	    else
	      outcmnt("");
	    break;
	  case MID_ADR_IND_DATA:
	    outcmnt(ind_cmnt(adr_ind));
	    break;
	  case MID_ADR_DSP_CONTROL:
	    if (val1 & 0x01)
	      outcmnt("Put DSP in RUN Mode");
	    else
	      outcmnt("Put DSP in HALT Mode");
	    break;
	  default:
	    outcmnt("");
	    break;
	}
	break;
      case MID_TRACE_OP_READ_REG:
	outtext(FORMAT_OP_REG_VAL,MID_TRACE_ID_READ_REG,
	  cvregname(adr_ind),val1,maskstamp);
	switch (adr)
	{
	  case MID_ADR_FREE_SPACE:
	    outcmnt("Free Space = %d Wds",val1);
	    break;
	  case MID_ADR_DSP_CONTROL:
	    if (val1 & 0x01)
	      outcmnt("DSP is in RUN Mode");
	    else
	      outcmnt("DSP is in HALT Mode");
	    break;
	  case MID_ADR_IND_DATA:
	    outcmnt(ind_cmnt(adr_ind));
	    break;
	  case MID_ADR_CARD_COMMO:
	    outcmnt(cvhcrname(val1 & 0xFFFF0000),val1 & 0xFFFF);
	    break;
	  default:
	    outcmnt("");
	    break;
	}
	break;
      case MID_TRACE_OP_POLL_COUNT:
	outtext(FORMAT_OP_REG_VAL,MID_TRACE_ID_POLL_COUNT,
	  cvregname(adr_ind),val1,maskstamp);
	outcmnt("Poll Count");
	break;
      case MID_TRACE_OP_LESS_EQUAL:
	outtext(FORMAT_OP_REG_VAL,MID_TRACE_ID_LESS_EQUAL,
	  cvregname(adr_ind),val1,maskstamp);
	outcmnt("Poll till reg <= value");
	break;
      case MID_TRACE_OP_MORE_EQUAL:
	outtext(FORMAT_OP_REG_VAL,MID_TRACE_ID_MORE_EQUAL,
	  cvregname(adr_ind),val1,maskstamp);
	outcmnt("Poll till reg >= value");
	break;
      case MID_TRACE_OP_NOT_EQUAL:
	if (val2 == -1)
	{
	  outtext(FORMAT_OP_REG_BLANK,MID_TRACE_ID_NOT_EQUAL,
	    cvregname(adr_ind),val1,&maskstamp[9]);
	  outcmnt("Poll till reg != val");
	}
	else
	{
	  outtext(FORMAT_OP_REG_VAL,MID_TRACE_ID_NOT_EQUAL,
	    cvregname(adr_ind),val1,maskstamp);
	  outcmnt("Poll (reg&mask) != val");
	}
	break;
      case MID_TRACE_OP_EQUAL:
	if (val2 == -1)
	{
	  outtext(FORMAT_OP_REG_BLANK,MID_TRACE_ID_EQUAL,
	    cvregname(adr_ind),val1,&maskstamp[9]);
	  outcmnt("Poll till reg == val");
	}
	else
	{
	  outtext(FORMAT_OP_REG_VAL,MID_TRACE_ID_EQUAL,
	    cvregname(adr_ind),val1,maskstamp);
	  outcmnt("Poll (reg&mask) == val");
	}
	break;
      case MID_TRACE_OP_VALUE:
	outtext(FORMAT_OP_REG_VAL,MID_TRACE_ID_VALUE,
	  cvregname(adr_ind),val1,maskstamp);
	outcmnt("Final Value From Reg");
	break;
      case MID_TRACE_OP_GET_POS:
	outtext(FORMAT_OP_POS_VAL,MID_TRACE_ID_GET_POS,
	  cvposname(adr),val1,maskstamp);
	switch (adr)
	{
	  case MID_ADR_POS_REG_0:
	    outcmnt("High Byte of Card ID");
	    break;
	  case MID_ADR_POS_REG_1:
	    outcmnt("Low Byte of Card ID");
	    break;
	  case MID_ADR_POS_REG_2:
	    outcmnt("Card Feature Enable");
	    break;
	  case MID_ADR_POS_REG_3:
	    outcmnt("Read VPD Information");
	    break;
	  case MID_ADR_POS_REG_4:
	    outcmnt("Read Reg via POS 7");
	    break;
	  case MID_ADR_POS_REG_5:
	    outcmnt("Read Misc POS Data");
	    break;
	  case MID_ADR_POS_REG_6:
	    outcmnt("Channel Check Status");
	    break;
	  case MID_ADR_POS_REG_7:
	    outcmnt("POS Indirect Address");
	    break;
	  default:
	    outcmnt("");
	    break;
	  }
	  break;
      case MID_TRACE_OP_PUT_POS:
	outtext(FORMAT_OP_POS_VAL,MID_TRACE_ID_PUT_POS,
	  cvposname(adr),val1,maskstamp);
	switch (adr)
	{
	  case MID_ADR_POS_REG_2:
	    outcmnt("Card Feature Enable");
	    break;
	  case MID_ADR_POS_REG_3:
	    outcmnt("Write Reg via POS 7");
	    break;
	  case MID_ADR_POS_REG_5:
	    outcmnt("Setup Misc POS Data");
	    break;
	  case MID_ADR_POS_REG_6:
	    outcmnt("Clear Channel Chk Stat");
	    break;
	  case MID_ADR_POS_REG_7:
	    outcmnt("POS Indirect Address");
	    break;
	  default:
	    outcmnt("");
	    break;
	}
	break;
      case MID_TRACE_OP_FREE_SPACE:
	outtext(FORMAT_OP_REG,MID_TRACE_ID_FREE_SPACE,
	  MID_TRACE_NAME_FREE_SPACE,maskstamp);
	outcmnt("Polling FIFO Free Space");
	break;
      case MID_TRACE_OP_PCB_FREE:
	outtext(FORMAT_OP_REG,MID_TRACE_ID_PCB_FREE,
	  MID_TRACE_NAME_PCB_FREE,maskstamp);
	outcmnt("Polling for PCB Free");
	break;
      case MID_TRACE_OP_TIME_STAMP:
	outtext(FORMAT_OP_REG,MID_TRACE_ID_TIME_STAMP,
	  MID_TRACE_NAME_TIME_STAMP,maskstamp);
	outcmnt("Time Delta = %d ns",stamp);
	break;
      case MID_TRACE_OP_SYSTEM_FILE:
	outtext(FORMAT_OP_REG_VAL,MID_TRACE_ID_SYSTEM_FILE,
	  MID_TRACE_NAME_SYSTEM_FILE,val1,maskstamp);
	outcmnt("Turn System Trace %s",(val1 ? "On" : "Off"));
	break;
      case MID_TRACE_OP_ASCII_SCREEN:
	outtext(FORMAT_OP_REG_VAL,MID_TRACE_ID_ASCII_SCREEN,
	  MID_TRACE_NAME_ASCII_SCREEN,val1,maskstamp);
	outcmnt("Turn Screen Trace %s",(val1 ? "On" : "Off"));
	break;
      case MID_TRACE_OP_ASCII_FILE:
	outtext(FORMAT_OP_REG_VAL,MID_TRACE_ID_ASCII_FILE,
	  MID_TRACE_NAME_ASCII_FILE,val1,maskstamp);
	outcmnt("Turn ASCII File Trc %s",(val1 ? "On" : "Off"));
	break;
      case MID_TRACE_OP_BINARY_FILE:
	outtext(FORMAT_OP_REG_VAL,MID_TRACE_ID_BINARY_FILE,
	  MID_TRACE_NAME_BINARY_FILE,val1,maskstamp);
	outcmnt("Turn Bin File Trace %s",(val1 ? "On" : "Off"));
	break;
      case MID_TRACE_OP_SINGLE_STEP:
	outtext(FORMAT_OP_REG_VAL,MID_TRACE_ID_SINGLE_STEP,
	  MID_TRACE_NAME_SINGLE_STEP,val1,maskstamp);
	outcmnt("Turn Single Step %s",(val1 ? "On" : "Off"));
	break;
      case MID_TRACE_OP_DELAY:
	outtext(FORMAT_OP_REG_VAL,MID_TRACE_ID_DELAY,
	  MID_TRACE_NAME_DELAY,val1,maskstamp);
	outcmnt("Delay for %f Secs",(float) val1 / 1000 );
	break;
      case MID_TRACE_OP_RING_BELL:
	outtext(FORMAT_OP_REG,MID_TRACE_ID_RING_BELL,
	  MID_TRACE_NAME_RING_BELL,maskstamp);
	outcmnt("Ring Bell");
	break;
      case MID_TRACE_OP_NAMES:
	outtext(FORMAT_OP_REG,MID_TRACE_ID_NAMES,
	  MID_TRACE_NAME_NAMES,maskstamp);
	outcmnt("Print Register Names");
	break;
      default:
	outtext(FORMAT_OP_VAL,MID_TRACE_ID_COMMENT,parm,maskstamp);
	outcmnt("Invalid Hookword Opcode");
	break;
    }
    outtext("\n");
  }
}
#endif /* MID_TRACE_SHOW_ASC_SWITCH + MID_TRACE_FILE_ASC_SWITCH */

out_buf(hook,parm,se_data,stamp,len_ctr,op_name,id,name)
int     hook;
int     parm;
ulong   se_data[];
int     stamp;
int     *len_ctr;
char    *op_name;
char    *id;
char    *name;
{
  int     i;
  int     j;
  int     length;
  int     begin;
  int     left_ctr = *len_ctr;

  length = (hook & MID_TRACE_HOOK_BUFLEN_MASK) >> 2;
  strncpy(maskstamp,blanks,18);
  if (hook & MID_TRACE_HOOK_TIME_MASK)
    sprintf(&maskstamp[9],FORMAT_HEX8,stamp);
  outtext(FORMAT_OP_REG_VAL,id,name,length,maskstamp);

  if (*len_ctr == 0)
  {
    if ((se_data[0] & 0xFFFF0000) == 0xFFFF0000)
      *len_ctr = 1;
    else
      *len_ctr = (se_data[0] >> 18) & 0x0003FFF;
    strcpy(op_name,opcode_name(se_data[0] & 0xFFFF));
  }

  begin = 0;
  if (*len_ctr == -1)
  {
    outcmnt(op_name);
  }
  else
  {
    for (i = *len_ctr; i < length && *len_ctr != 0; i += *len_ctr)
    {
      if (comment_mode)
      {
	outseid(op_name,begin);
	outtext(FORMAT_NL_NAME,id);
      }
      begin = i;
      if ((se_data[i] & 0xFFFF0000) == 0xFFFF0000)
	*len_ctr = 1;
      else
	*len_ctr = (se_data[i] >> 18) & 0x0003FFF;
      strcpy(op_name,opcode_name(se_data[i] & 0xFFFF));
    }
    if (i < length)
    {
#ifndef MID_DD
      PRINTF_ERMSG(("\nInvalid SE header 0x%8.8X in word %d of output entry %d",
	se_data[i],i,out_count))
      if (data_loc)
	PRINTF_ERMSG((" and input entry %d",in_count))
#ifdef MID_TED
      waitkey('\n',"\nPress enter to continue\n");
#endif /* MID_TED */
#endif /* not MID_DD */
      i = length;
    }
    *len_ctr = (i - length);
    outseid(op_name,begin);
  }

  for (i=0; i<length; i++,left_ctr--)
  {
    if ((i % 8) == 0)
      outtext(FORMAT_OP_OFFSET,id,i);
    if (left_ctr == 0)
    {
      outtext(FORMAT_MARK_HEX8,mark,se_data[i]);
      if ((se_data[i] & 0xFFFF0000) == 0xFFFF0000)
	left_ctr = 1;
      else
	left_ctr = (se_data[i] >> 18) & 0x0003FFF;
    }
    else
    {
      outtext(FORMAT_BLANK_HEX8,se_data[i]);
    }
  }
}

outcmnt(format,p1,p2,p3,p4,p5,p6,p7,p8,p9,p10)
char    *format;
int     p1;
int     p2;
int     p3;
int     p4;
int     p5;
int     p6;
int     p7;
int     p8;
int     p9;
int     p10;
{
  int     length;

  if (comment_mode)
  {
    sprintf(comment1,format,p1,p2,p3,p4,p5,p6,p7,p8,p9,p10);
    sprintf(comment2,FORMAT_COMMENT_TRCNT_USR,comment1,out_count,user_id);
    outtext(comment2);
  }
}

outseid(opc_name,opc_offset)
char    *opc_name;
int     opc_offset;
{
  int     length;

  if (comment_mode)
  {
    if (opc_offset == 0)
      sprintf(comment2,FORMAT_COMMENT_TRCNT_USR,opc_name,out_count,user_id);
    else
      sprintf(comment2,FORMAT_COMMENT_SECNT_USR,opc_name,opc_offset,user_id);
    outtext(comment2);
  }
}

outtext(format,p1,p2,p3,p4,p5,p6,p7,p8,p9,p10)
char    *format;
int     p1;
int     p2;
int     p3;
int     p4;
int     p5;
int     p6;
int     p7;
int     p8;
int     p9;
int     p10;
{
#if     MID_TRACE_FILE_ASC_SWITCH + MID_TRACE_SHOW_ASC_SWITCH
  if (out_enable)
  {
    FPRINTF_ERMSG((out_file,format,p1,p2,p3,p4,p5,p6,p7,p8,p9,p10))
  }
#endif /* MID_TRACE_FILE_ASC_SWITCH + MID_TRACE_SHOW_ASC_SWITCH */
}

cvuserid(opcode)
int     opcode;
{
  return(cvopid(userids,usercnt,opcode));
}

cvopid(id_defs,id_cnt,opcode)
ID_DEF  id_defs[];
int     id_cnt;
int     opcode;
{
  int     i;

  for (i=0; i<id_cnt; i++)
    if (opcode == id_defs[i].opcode)
      return(id_defs[i].id);
  return('?');
}

char *cvregname(adr)
int     adr;
{
  if (adr & MID_TRACE_PARM_IND_MASK)
  {
    sprintf(ind_name,FORMAT_IND_DATA,(adr >> 16) & 0xFFFF);
    return(ind_name);
  }
  return(cvadrname(regnames,namcnt,adr));
}

char *cvposname(adr)
int     adr;
{
  return(cvadrname(posnames,poscnt,adr));
}

char *cvdmaname(adr)
int     adr;
{
  return(cvadrname(dmanames,dmacnt,adr));
}

char *cvhcrname(adr)
int     adr;
{
  return(cvadrname(hcrnames,hcrcnt,adr));
}

char *cvascbname(adr)
int     adr;
{
  return(cvadrname(ascbnames,ascbcnt,adr));
}

char *cvadrname(namdefs,namcnt,adr)
NAMDEF  namdefs[];
int     namcnt;
int     adr;
{
  int     i;

  for (i=0; i<namcnt; i++)
    if (adr == namdefs[i].adr)
      return(namdefs[i].name);
  sprintf(badaddr,FORMAT_HEX8,adr);
  return(badaddr);
}

char *ind_cmnt(parm)
int     parm;
{
  int     opcode;
  int     number;

  opcode = ((parm & MID_TRACE_PARM_IND_OP_MASK) >> 16) & 0xFFFF;
  number = ((parm & MID_TRACE_PARM_IND_NUMBER_MASK) >> 24) & 0x000F;

  sprintf(ind_com,cvadrname(blknames,blkcnt,opcode),number);
  return(ind_com);
}

#ifndef MID_DD
mid_trace_file_sys_start()
{
#if MID_TRACE_FILE_SYS_SWITCH
  int     rc;
  char    *cfgcmd;
  static  char    trcconfig[] = "-a -s -T 5000000 -L 1000000000 -j 518 -o mid_trace.sys";

#ifdef  MID_DD
  rc = trcstart(trcconfig);
#else
  if (cfgcmd = getenv("TRACE"))
    rc = trcstart(cfgcmd);
  else
    rc = trcstart(trcconfig);
#endif
  return(rc);
#endif /* MID_TRACE_FILE_SYS_SWITCH */
}
#endif /* MID_DD */

mid_trace_file_asc_start()
{
#if MID_TRACE_FILE_ASC_SWITCH
  FCLOSEO_ERMSG(asc_file)
  FOPEN_ERMSG(asc_name,"w",asc_file)
  mid_trace_file_asc_on();
#endif /* MID_TRACE_FILE_ASC_SWITCH */
}

mid_trace_file_bin_start()
{
#if MID_TRACE_FILE_BIN_SWITCH
  FCLOSEO_ERMSG(bin_file)
  FOPEN_ERMSG(bin_name,"w",bin_file)
  FWRITE_ERMSG(&bin_id,sizeof(int),1,bin_file)

  mid_trace_file_bin_on();
#endif /* MID_TRACE_FILE_BIN_SWITCH */
}

mid_trace_show_asc_start()
{
#if MID_TRACE_SHOW_ASC_SWITCH
  mid_trace_show_asc_on();
#endif /* MID_TRACE_SHOW_ASC_SWITCH */
}


mid_trace_timestamp_on()
{
  mid_gen_ts = MID_TRACE_TIME_MASK;
}

#ifndef MID_DD
mid_trace_file_sys_on()
{
#if MID_TRACE_FILE_SYS_SWITCH
  return(trcon(0));
#endif /* MID_TRACE_FILE_SYS_SWITCH */
}
#endif /* MID_DD */

mid_trace_file_asc_on()
{
#if MID_TRACE_FILE_ASC_SWITCH
  timeold = 0.0;
  mid_trace_flags |= MID_TRACE_FILE_ASC;
#endif /* MID_TRACE_FILE_ASC_SWITCH */
}

mid_trace_file_bin_on()
{
#if MID_TRACE_FILE_BIN_SWITCH
  timeold = 0.0;
  mid_trace_flags |= MID_TRACE_FILE_BIN;
#endif /* MID_TRACE_FILE_BIN_SWITCH */
}

mid_trace_show_asc_on()
{
#if MID_TRACE_SHOW_ASC_SWITCH
  timeold = 0.0;
  mid_trace_flags |= MID_TRACE_SHOW_ASC;
#endif /* MID_TRACE_SHOW_ASC_SWITCH */
}


mid_trace_timestamp_off()
{
  mid_gen_ts = 0;
}

#ifndef MID_DD
mid_trace_file_sys_off()
{
#if MID_TRACE_FILE_SYS_SWITCH
  return(trcoff(0));
#endif /* MID_TRACE_FILE_SYS_SWITCH */
}
#endif /* MID_DD */

mid_trace_file_asc_off()
{
#if MID_TRACE_FILE_ASC_SWITCH
  mid_trace_flags &= ~MID_TRACE_FILE_ASC;
#endif /* MID_TRACE_FILE_ASC_SWITCH */
}

mid_trace_file_bin_off()
{
#if MID_TRACE_FILE_BIN_SWITCH
  mid_trace_flags &= ~MID_TRACE_FILE_BIN;
#endif /* MID_TRACE_FILE_BIN_SWITCH */
}

mid_trace_show_asc_off()
{
#if MID_TRACE_SHOW_ASC_SWITCH
  mid_trace_flags &= ~MID_TRACE_SHOW_ASC;
#endif /* MID_TRACE_SHOW_ASC_SWITCH */
}

#ifndef MID_DD
mid_trace_file_sys_stop()
{
#if MID_TRACE_FILE_SYS_SWITCH
  return(trcstop(0));
#endif /* MID_TRACE_FILE_SYS_SWITCH */
}
#endif /* MID_DD */

mid_trace_file_asc_stop()
{
#if MID_TRACE_FILE_ASC_SWITCH
  FCLOSEO_ERMSG(asc_file)
  mid_trace_file_asc_off();
#endif /* MID_TRACE_FILE_ASC_SWITCH */
}

mid_trace_file_bin_stop()
{
#if MID_TRACE_FILE_BIN_SWITCH
  FCLOSEO_ERMSG(bin_file)
  mid_trace_file_bin_off();
#endif /* MID_TRACE_FILE_BIN_SWITCH */
}

mid_trace_show_asc_stop()
{
#if MID_TRACE_SHOW_ASC_SWITCH
  mid_trace_show_asc_off();
#endif /* MID_TRACE_SHOW_ASC_SWITCH */
}


mid_trace_timestamp_query()
{
  return(mid_gen_ts);
}

mid_trace_file_sys_query()
{
#ifndef MID_DD
  return(TRC_ISON(0));
#endif
}

mid_trace_file_asc_query()
{
  return(mid_trace_flags & MID_TRACE_FILE_ASC);
}

mid_trace_file_bin_query()
{
  return(mid_trace_flags & MID_TRACE_FILE_BIN);
}

mid_trace_show_asc_query()
{
  return(mid_trace_flags & MID_TRACE_SHOW_ASC);
}

mid_trace_init()
{
  tr_count = 0;
  show_asc_ptrs = init_asc_ptrs;
  file_asc_ptrs = init_asc_ptrs;
  timeold = 0.0;
}

mid_trace_save_state(state)
int     state;
{
  if (state >= 0 && state < MAX_STATES)
  {
    states[state].save_restore_flag = -1            ;
    states[state].tr_count        = tr_count        ;
    states[state].mid_trace_flags = mid_trace_flags ;
    states[state].mid_gen_ts      = mid_gen_ts      ;
    states[state].timeold         = timeold         ;
    states[state].asc_file        = asc_file        ;
    states[state].bin_file        = bin_file        ;
    states[state].show_asc_ptrs   = show_asc_ptrs   ;
    states[state].file_asc_ptrs   = file_asc_ptrs   ;
    states[state].file_bin_trace  = file_bin_trace  ;
    states[state].file_asc_trace  = file_asc_trace  ;
    states[state].show_asc_trace  = show_asc_trace  ;
  }
}

mid_trace_restore_state(state)
int state;
{
  if (state >= 0 && state < MAX_STATES && states[state].save_restore_flag == -1)
  {
    states[state].save_restore_flag = 0             ;
    tr_count        = states[state].tr_count        ;
    mid_trace_flags = states[state].mid_trace_flags ;
    mid_gen_ts      = states[state].mid_gen_ts      ;
    timeold         = states[state].timeold         ;
    asc_file        = states[state].asc_file        ;
    bin_file        = states[state].bin_file        ;
    show_asc_ptrs   = states[state].show_asc_ptrs   ;
    file_asc_ptrs   = states[state].file_asc_ptrs   ;
    file_bin_trace  = states[state].file_bin_trace  ;
    file_asc_trace  = states[state].file_asc_trace  ;
    show_asc_trace  = states[state].show_asc_trace  ;
  }
}

