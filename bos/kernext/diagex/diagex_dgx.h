/* @(#)92	1.4  src/bos/kernext/diagex/diagex_dgx.h, sysxdiag, bos411, 9431A411a 7/18/94 12:51:46 */
/*
 *
 * COMPONENT_NAME: sysxdiag
 *
 * FUNCTIONS: Diagnostic Kernel Extension Interface definition
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1993,1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include <sys/intr.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <sys/trcmacros.h>

#define HKWD_DGX_ERR 0x35500000

/*********************************************************************
* Macro Constants
*********************************************************************/
#define DGX_MSGSZ       50       /* Maximum Error/Warning MSG stringsize*/
#define DGX_UNSET       -1       /* When unset,use an invalid daddr     */
#define DGX_SLAVE_PG    0        /* index to dma_info() member w/ slave info */
#define DGX_WRITEOP     1        /* Write operation for diag_rw() */
#define DGX_READOP      0        /* Read operation for diag_rw() */
#define DGX_IO_OP       1        /* I/O Operation for diag_rw() */
#define DGX_MEM_OP      2        /* Memory Operation for diag_rw() */
#define DGX_POS_OP      3        /* POS Operation for diag_rw() */
#define DGX_SPECIAL_IO_OP 4      /* I/O Operation for diag_rw() using io_att */
#define DGX_SINGLELOC   0        /* Single Location Access for diag_rw */
#define DGX_MULTI_LOC   1        /* MultiplLocation Access for diag_rw */
#define DGX_SCRPADSZ    PAGESIZE /* scratch pad should be 1 page */
#define DGX_BYPASS      0x20     /*IOCC not to authority check w/ bus_mem TCWs*/
#define DGX_REALMEM_BUID 0x87F00000 /* OR  mask BUID for access to real memory*/
#define DGX_NO_BUID      0xF00FFFFF /* AND mask to remove BUID */
#define DGX_POS_LENGTH   8        /* POS length */


/*----------------------------------------------------------------------------
*  TRACE Marcos
*----------------------------------------------------------------------------*/
#define DGX_TRACE_TOP   "diagexTrcTOP!!!!"      /* marks next entry in table */
#define DGX_TRACE_CUR   "diagexTrcCUR!!!!"      /* marks next entry in table */
#define DGX_TRACE_BOT   "diagexTrcBOT!!!!"      /* marks next entry in table */
#ifndef   NO_HOOK_YET
#	define DGX_OTHER     (HKWD_DGX_ERR)
#else  /* NO_HOOK_YET */
#	define DGX_OTHER	NULL
#endif /* NO_HOOK_YET */

/*----------------------------------------------------------------------------
* Internal trace table size and trace macros
*   When TDEBUG is defined,
*	TRACE_SYS:	outputs to system trace table
*	TRACE_DBG:	outputs to system trace table AND internal trace table
*	TRACE_BOTH:	outputs to system trace table AND internal trace table
*   When TDEBUG is NOT defined,
*	TRACE_SYS:	outputs to system trace table
*	TRACE_DBG:	does nothing 
*	TRACE_BOTH:	outputs to system trace table AND internal trace table
*   Usage of 'tag'
*	general
*				"Fnc*"	- 1st 3 Characters are function code
*				"***X"	- 4th   Character  is  location code
*				"Fnc+"	- +     indicates a continuation of
*						the last Trace Table Entry
*	TRACE_SYS:	for specific functional code paths (WRITES,READS,...)
*				---not used currently----
*	TRACE_DBG:	Infomational 
*				"FncB"		- Beginning of Function
*				"FncE"		- End       of Function
*				"Fnce"		- End       of Function (if 2)
*				"Fnc[a-d,f-z]"	- Other info site
*	TRACE_BOTH:	Error Detection ([1-99])
*				"Fnc[1-99]"	- Error Detection Site
*
*   NOTE: diag_trace() calls
*		"TRCHKGT(hook | HKTY_GT | 4, tag, arg1, arg2, arg3, 0)"
*         if NO_HOOK_YET is not defined
*----------------------------------------------------------------------------*/
#ifdef TDEBUG
#define DGX_TRACE_SIZE	(1024<<2)  /* 1024 traces w/4 ulong args/trace*/
#define TRACE_SYS(hook, tag, arg1, arg2, arg3)	diag_trace(hook, tag, arg1, arg2, arg3)
#define TRACE_DBG(hook, tag, arg1, arg2, arg3)	diag_trace(hook, tag, arg1, arg2, arg3)
#	define TRACE_BOTH(hook, tag, arg1, arg2, arg3) diag_trace(hook, tag, arg1, arg2, arg3)
#else  /* TDEBUG */
#   define DGX_TRACE_SIZE	(32<<2)     /* 32 traces w/4 ulong args/trace */
#   ifndef   NO_HOOK_YET
#	define TRACE_SYS(hook, tag, arg1, arg2, arg3)	\
	          TRCHKGT(hook | HKTY_GT | 4, tag, arg1, arg2, arg3, 0)
#   else  /* NO_HOOK_YET */
#	define TRACE_SYS(hook, tag, arg1, arg2, arg3)	\
	      diag_trace(hook, tag, arg1, arg2, arg3)
#   endif /* NO_HOOK_YET */
#	define TRACE_DBG(hook, tag, arg1, arg2, arg3)	
	      /* do not output debug traces */
#	define TRACE_BOTH(hook, tag, arg1, arg2, arg3)	\
	      diag_trace(hook, tag, arg1, arg2, arg3)
#endif  /* TDEBUG */





/*********************************************************************
* Diagex Internal Structures
*********************************************************************/

/*----------------------------------------------------------------------------*
*           trace table 
*----------------------------------------------------------------------------*/
typedef struct {
   int          next_entry;			/* index for the next entry */
   ulong        table[DGX_TRACE_SIZE];		/* trace table              */
} dgx_trace_t;

/*----------------------------------------------------------------------------
*  DIAG_CNTL_T
*     Global Structure for  DIAGEX
*----------------------------------------------------------------------------*/
typedef struct {
   lock_t openlock;                /* serialization lock for diag_open/close */
   lock_t dmaindexlock;            /* serialization lock for get_dma_index() */
   diag_struc_t *top_handle;       /* top of linked list of handles */
   char timeout;                   /* TRUE if watch dog timer timed out */
   dgx_trace_t trace;              /* device driver trace table */
} diag_cntl_t;



/*********************************************************************
* SETERR macro
* Allows diagex() to
*     1)  return a diagnostic error code
*    2a)  set a diagerrno to the kernel service RC
*    2b)  set errno to the kernel service RC
* Currently using (1) and (2b)
*********************************************************************/
#define SETERR(drc,krc) {     \
              errcode = drc;  \
              setuerror(krc); \
        }

/*******************************************************************
* Function Prototypes
*******************************************************************/
/*-------------diagex_load.c---------------------------------------*/
int diag_cleanup(diag_struc_t **handle );
/*-------------diagex_pin.c---------------------------------------*/
int diag_intr(struct intr *intr_ptr);
void add_handle (diag_struc_t *handle);
diag_struc_t *find_handle (diag_struc_t *handle);
int remove_handle (diag_struc_t *handle);
ulong diag_rw(diag_struc_t *handle,int wr_op, int memio, int type,int offset,
 int count, int range_flag, void *data, struct timestruc_t *times, int intrlev);
int get_ioatt_ptr
  (diag_struc_t *handle,int memio,int offset,int extent,caddr_t *addr);


/******************************************************************************/
#ifdef DGX_DEBUG /* { */ /*****************************************************/
/******************************************************************************/

/*********************************************************************
* MSG macro routine names
*     active when DGX_DEBUG is defined
*********************************************************************/

#define DGX_OPEN_ROUTINE		char routine[] = {"diag_open"};
#define DGX_CLOSE_ROUTINE		char routine[] = {"diag_close"};
#define DGX_MASTER_ROUTINE		char routine[] = {"diag_dma_master"};
#define DGX_SLAVE_ROUTINE		char routine[] = {"diag_dma_slave"};
#define DGX_CLEAN_ROUTINE		char routine[] = {"diag_cleanup"};
#define DGX_INTR_ROUTINE		char routine[] = {"diag_intr"};
#define DGX_WATCH4INTR_ROUTINE		char routine[] = {"diag_watch4intr"};
#define DGX_COMPLETE_ROUTINE		char routine[] = {"diag_dma_complete"};
#define DGX_FLUSH_ROUTINE		char routine[] = {"diag_dma_flush"};
#define DGX_UNMASK_ROUTINE		char routine[] = {"diag_dma_unmask"};
#define DGX_MASK_ROUTINE		char routine[] = {"diag_dma_mask"};
#define DGX_IOWR_ROUTINE		char routine[] = {"diag_io_write"};
#define DGX_IORD_ROUTINE		char routine[] = {"diag_io_read"};
#define DGX_MEMWR_ROUTINE		char routine[] = {"diag_mem_write"};
#define DGX_MEMRD_ROUTINE		char routine[] = {"diag_mem_read"};
#define DGX_POSWR_ROUTINE		char routine[] = {"diag_pos_write"};
#define DGX_POSRD_ROUTINE		char routine[] = {"diag_pos_read"};
#define DGX_IOWRST_ROUTINE		char routine[] = {"diag_io_wr_stream"};
#define DGX_IORDST_ROUTINE		char routine[] = {"diag_io_rd_stream"};
#define DGX_MEMWRST_ROUTINE		char routine[] = {"diag_mem_wr_stream"};
#define DGX_MEMRDST_ROUTINE		char routine[] = {"diag_mem_rd_stream"};
#define DGX_ADDHANDLE_ROUTINE		char routine[] = {"diag_add_handle"};
#define DGX_FINDHANDLE_ROUTINE	char routine[] = {"diag_find_handle"};
#define DGX_RMHANDLE_ROUTINE		char routine[] = {"diag_remove_handle"};
#define DGX_RW_ROUTINE			char routine[] = {"diag_rw"};
#define DGX_IOATT_ROUTINE		char routine[] = {"diag_get_ioatt_ptr"};
#define DGX_GETINDX_ROUTINE		char routine[] = {"get_dma_index"};
#define DGX_FINDINDX_ROUTINE		char routine[] = {"find_dma_index"};
#define DGX_FREEINDX_ROUTINE		char routine[] = {"free_dma_index"};
#define DGX_SPECIAL_IO_WR_ROUTINE   char routine[] = {"diag_io_write"};
#define DGX_SPECIAL_IO_RD_ROUTINE   char routine[] = {"diag_io_read"};



/*********************************************************************
* MSG macros
*     active when DGX_DEBUG is defined
*********************************************************************/
#define eMSGn1(routine,s1,n1) {                                                \
   char h1[DGX_MSGSZ],h2[DGX_MSGSZ];                                           \
   char ss1[DGX_MSGSZ];                                                        \
   sprintf(h1, "Error in Routine %s :",routine);                               \
   sprintf(h2, "File %s, line %d  :",__FILE__,__LINE__);                       \
   sprintf(ss1, "%s = 0x%X",s1,n1);                                            \
   MSG(h1,h2,ss1,"","","","","","","","");                                     \
}
#define eMSGn2(routine,s1,n1,s2,n2) {                                          \
   char h1[DGX_MSGSZ],h2[DGX_MSGSZ];                                           \
   char ss1[DGX_MSGSZ],ss2[DGX_MSGSZ];                                         \
   sprintf(h1, "Error in Routine %s :",routine);                               \
   sprintf(h2, "File %s, line %d  :",__FILE__,__LINE__);                       \
   sprintf(ss1, "%s = 0x%X",s1,n1);                                            \
   sprintf(ss2, "%s = 0x%X",s2,n2);                                            \
   MSG(h1,h2,ss1,ss2,"","","","","","","");                                    \
}
#define eMSGn3(routine,s1,n1,s2,n2,s3,n3) {                                    \
   char h1[DGX_MSGSZ],h2[DGX_MSGSZ];                                           \
   char ss1[DGX_MSGSZ],ss2[DGX_MSGSZ],ss3[DGX_MSGSZ];                          \
   sprintf(h1, "Error in Routine %s :",routine);                               \
   sprintf(h2, "File %s, line %d  :",__FILE__,__LINE__);                       \
   sprintf(ss1, "%s = 0x%X",s1,n1);                                            \
   sprintf(ss2, "%s = 0x%X",s2,n2);                                            \
   sprintf(ss3, "%s = 0x%X",s3,n3);                                            \
   MSG(h1,h2,ss1,ss2,ss3,"","","","","","");                                   \
}
#define eMSGn4(routine,s1,n1,s2,n2,s3,n3,s4,n4) {                              \
   char h1[DGX_MSGSZ],h2[DGX_MSGSZ];                                           \
   char ss1[DGX_MSGSZ],ss2[DGX_MSGSZ],ss3[DGX_MSGSZ],ss4[DGX_MSGSZ];           \
   sprintf(h1, "Error in Routine %s :",routine);                               \
   sprintf(h2, "File %s, line %d  :",__FILE__,__LINE__);                       \
   sprintf(ss1, "%s = 0x%X",s1,n1);                                            \
   sprintf(ss2, "%s = 0x%X",s2,n2);                                            \
   sprintf(ss3, "%s = 0x%X",s3,n3);                                            \
   sprintf(ss4, "%s = 0x%X",s4,n4);                                            \
   MSG(h1,h2,ss1,ss2,ss3,ss4,"","","","","");                                  \
}
#define eMSGn5(routine,s1,n1,s2,n2,s3,n3,s4,n4,s5,n5) {                        \
   char h1[DGX_MSGSZ],h2[DGX_MSGSZ];                                           \
   char ss1[DGX_MSGSZ],ss2[DGX_MSGSZ],ss3[DGX_MSGSZ],                          \
        ss4[DGX_MSGSZ],ss5[DGX_MSGSZ];                                         \
   sprintf(h1, "Error in Routine %s :",routine);                               \
   sprintf(h2, "File %s, line %d  :",__FILE__,__LINE__);                       \
   sprintf(ss1, "%s = 0x%X",s1,n1);                                            \
   sprintf(ss2, "%s = 0x%X",s2,n2);                                            \
   sprintf(ss3, "%s = 0x%X",s3,n3);                                            \
   sprintf(ss4, "%s = 0x%X",s4,n4);                                            \
   sprintf(ss5, "%s = 0x%X",s5,n5);                                            \
   MSG(h1,h2,ss1,ss2,ss3,ss4,ss5,"","","","");                                 \
}
#define eMSG1(routine,s1) {                                                    \
   char h1[DGX_MSGSZ],h2[DGX_MSGSZ];                                           \
   sprintf(h1, "Error in Routine %s :",routine);                               \
   sprintf(h2, "File %s, line %d  :",__FILE__,__LINE__);                       \
   MSG(h1,h2,s1,"","","","","","","","");                                      \
}
#define eMSG2(routine,s1,s2) {                                                 \
   char h1[DGX_MSGSZ],h2[DGX_MSGSZ];                                           \
   sprintf(h1, "Error in Routine %s :",routine);                               \
   sprintf(h2, "File %s, line %d  :",__FILE__,__LINE__);                       \
   MSG(h1,h2,s1,s2,"","","","","","","");                                      \
}
#define eMSG3(routine,s1,s2,s3) {                                              \
   char h1[DGX_MSGSZ],h2[DGX_MSGSZ];                                           \
   sprintf(h1, "Error in Routine %s :",routine);                               \
   sprintf(h2, "File %s, line %d  :",__FILE__,__LINE__);                       \
   MSG(h1,h2,s1,s2,s3,"","","","","","");                                      \
}
#define eMSG4(routine,s1,s2,s3,s4) {                                           \
   char h1[DGX_MSGSZ],h2[DGX_MSGSZ];                                           \
   sprintf(h1, "Error in Routine %s :",routine);                               \
   sprintf(h2, "File %s, line %d  :",__FILE__,__LINE__);                       \
   MSG(h1,h2,s1,s2,s3,s4,"","","","","");                                      \
}
#define eMSG5(routine,s1,s2,s3,s4,s5) {                                        \
   char h1[DGX_MSGSZ],h2[DGX_MSGSZ];                                           \
   sprintf(h1, "Error in Routine %s :",routine);                               \
   sprintf(h2, "File %s, line %d  :",__FILE__,__LINE__);                       \
   MSG(h1,h2,s1,s2,s3,s4,s5,"","","","");                                      \
}
#define MSG(l1,l2,l3,l4,l5,l6,l7,l8,l9,lA,lB) {                                \
   printf("\n********************************************************\n");     \
   printf("* %s\n* %s\n* %s\n* %s\n* %s\n* %s\n* %s\n* %s\n",                  \
              l1,l2,l3,l4,l5,l6,l7,l8);                                        \
   printf("********************************************************\n\n");     \
}

/******************************************************************************/
#else /* DGX_DEBUG }{ */ /*****************************************************/
/******************************************************************************/

#define DGX_OPEN_ROUTINE
#define DGX_CLOSE_ROUTINE
#define DGX_MASTER_ROUTINE
#define DGX_SLAVE_ROUTINE
#define DGX_CLEAN_ROUTINE
#define DGX_INTR_ROUTINE
#define DGX_WATCH4INTR_ROUTINE
#define DGX_COMPLETE_ROUTINE
#define DGX_FLUSH_ROUTINE
#define DGX_UNMASK_ROUTINE
#define DGX_MASK_ROUTINE
#define DGX_IOWR_ROUTINE
#define DGX_IORD_ROUTINE
#define DGX_MEMWR_ROUTINE
#define DGX_MEMRD_ROUTINE
#define DGX_POSWR_ROUTINE
#define DGX_POSRD_ROUTINE
#define DGX_IOWRST_ROUTINE
#define DGX_IORDST_ROUTINE
#define DGX_MEMWRST_ROUTINE
#define DGX_MEMRDST_ROUTINE
#define DGX_ADDHANDLE_ROUTINE
#define DGX_FINDHANDLE_ROUTINE
#define DGX_RMHANDLE_ROUTINE
#define DGX_RW_ROUTINE
#define DGX_IOATT_ROUTINE
#define DGX_GETINDX_ROUTINE
#define DGX_FINDINDX_ROUTINE
#define DGX_FREEINDX_ROUTINE
#define DGX_SPECIAL_IO_RD_ROUTINE
#define DGX_SPECIAL_IO_WR_ROUTINE


#define eMSGn5(a,b,c,d,e,f,g,h,i,j,k) 
#define eMSGn4(a,b,c,d,e,f,g,h,i) 
#define eMSGn3(a,b,c,d,e,f,g) 
#define eMSGn2(a,b,c,d,e) 
#define eMSGn1(a,b,c) 
#define eMSG5(a,b,c,d,e,f) 
#define eMSG4(a,b,c,d,e) 
#define eMSG3(a,b,c,d) 
#define eMSG2(a,b,c) 
#define eMSG1(a,b) 

/******************************************************************************/
#endif /* DGX_DEBUG } */ /*****************************************************/
/******************************************************************************/

#ifdef DGX_PRE_V325
#define d_bflush(a,b,c) 0
#define BUS_60X 3
#define XMEM_ACC_CHK 0
#endif /* DGX_PRE_V325 */
