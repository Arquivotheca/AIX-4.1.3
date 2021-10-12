static char sccsid[] = "@(#)48	1.27  src/bos/usr/ccs/lib/libdbx/k_thread.c, libdbx, bos41J, 9516A_all 4/18/95 02:52:51";
#ifdef K_THREADS
/*
 * COMPONENT_NAME: (CMDDBX) - dbx symbolic debugger
 *
 * FUNCTIONS: k_error, thread_k ,
 *            insert_pthreads_obj, createType_pthreads,
 *            search_pthread , update_threads,
 *            switchThread_k, attribute_k, condition_k,
 *            thread_info_k, setThreadreg_k,
 *            getThreadOpt_k, getMutexOpt_k, getConditionOpt_k,
 *            getAttributeOpt_k,  createEnum, create_pthreads_enum,
 *            isRequested_k, getWaiters_k, display_thread_k,
 *            create_pthreads_types, update_pthreads_obj,
 *            thread_k_cont, threadheld, mutex_k, update_running_thread,
 *            getwaittype, search_object_id, verifArgObject_k,
 *            isThreadObjectSymbol_k, check_thread_stacks
 *
 * ORIGINS: 27 83
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1992, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
*/

/*
 * NOTES: libpthreads objects (threads,mutexes,conditon variables,
 *        and attributes) are linked together in circular lists headed
 *        by queue objects such as __dbx_known_pthreads, __dbx_known_mutexes,
 *        __dbx_known_attributes and __dbx_known_conditions.
 *        By following the next (forward link) of these
 *        queue objects, we can get to the portion of the next
 *        element (thread control block in case of a thread list) in 
 *        an object list. To get the starting address of the object element 
 *        use offsetof(structure,element).
 *
 *					   pthread
 *					---------------------
 *     					|	....        |
 *					|		    |
 *     	__dbx_known_pthreads		|                   |
 *      --------------------		---------------------
 * ---->|   queue.next     |----------->| DBXlink.next      |----->
 *	|..................|		|...................|
 * <----|   queue.prev     |<-----------| DBXlink.prev      |<-----
 *	--------------------            ---------------------
 *     	|   ...*	   |	        |		    |
 *     	--------------------		|       ....        |
 *
 */

#ifdef KDBXRT
#include "rtnls.h"		/* MUST BE FIRST */
#endif
/*              include file for message texts          */
#include "dbx_msg.h" 
nl_catd  scmc_catd;   /* Cat descriptor for scmc conversion */

/*
 * include files
 */
#include "internal.h"           /* libpthreads include files */
#include <sys/ptrace.h>
#undef _THREAD_SAFE             /* define global errno       */
#undef malloc
#undef free
#ifdef errno                    /* function)                 */
#undef errno                    /* for next errno.h          */
extern int errno;
#endif
#include <sys/mstsave.h>
#include <sys/types.h>
#include "defs.h"		/* dbx include files */
#include "envdefs.h"
#include "coredump.h"
#include "eval.h"
#include "execute.h"
#include "mappings.h"
#include "names.h"
#include "object.h"
#include "process.h"
#include "runtime.h"
#include "frame.h"
#include "source.h"
#include "symbols.h"
#include <sys/signal.h>		/* system include file */
#include <sys/proc.h>		/* for getwaittype : p_wtype*/
#include <sys/user.h>
#include <sys/thread.h>
#include "cma_thread.h"
#include <sys/uthread.h>
#include <sys/core.h>
#include <stddef.h>
#include <errno.h>
#include <procinfo.h>

private void threadheld();
typedef struct pthread pthread_st;
typedef struct stk stk_st;
typedef struct vp vp_st;
typedef struct pthread_attr  pthread_attr_st;
typedef struct thrdsinfo  thrdsinfo_st;
typedef struct mstsave  mstsave_st;
typedef struct procsinfo  procsinfo_st;

typedef struct uthread uthread_st;
extern boolean traceexec;               /* trace execution */

/*
 * defines
 */
#define OBJ_THREAD 0            /* object thread */
#define OBJ_MUTEX  1            /* object mutex */
#define OBJ_CONDITION  2        /* object condition */
#define OBJ_ATTRIBUTE  3        /* object attribute */

#define MAX_FIELD 12		/* Max. number of field in object types */
#define MAX_NAME_LEN 25		/* Max. length of  object name           */
				/* "$t" prefix is max. two characters       */
				/* Number after prefix is max. len of int   */

#define QUEUE_NEXT(n) ( (n).next == 0xffffffff  ? nil : (n).next )

#define ESIZE(n)   sizeof(n)/sizeof(char *)

/* The  thread responsible of the stop  has in its pthread structure */
/* ti_cursig != 0 (the signal received)                              */
#define IS_RESPONSIBLE(s) ( (s)!= 0 )

#define IS_RUNNING(t) ( (t)== TSRUN )
#define IS_WAIT(t)   ( (t)== TSSLEEP )
#define IS_SUSP(t)    ( (t)== TSSTOP )
#define IS_TERM(t)    ( (t)== TSZOMB )

#define MODE_K  1       /*the thread is stopped in kernel mode */
#define MODE_U  0       /*the thread is stopped in user mode */
#define MODE_UNDEFINED 2 /*the thread is stopped in undefied mode : core file */          
/* define types of attribute object */
#define ATTR_THREAD  0   /* the attribute is a pthread attribute */          
#define ATTR_MUTEX   1   /* the attribute is a mutex attribute */          
#define ATTR_COND    2   /* the attribute is a condition attribute */          

/* define constantes for state of user thread (ss_name) */
#define RUNNING   0      /* the thread is running       */
#define TERMINATED   1   /* the thread is terminated    */
#define BLOCKED   2      /* the thread is waiting       */
#define RUNNABLE   3     /* for M:N only                */

#define NO_POLICY  3
#define s_queue   (int)(sizeof(pthread_queue))

#define offsetof2(s_name, s_member)  offsetof(s_name, s_member) - s_queue
/*
 * typedefs, globals, statics, and externs
 */
public Address save_last_bp = 0;        /* save le last_pb           */
public Symbol running_thread = nil;	/* Pointer to running thread */	
public Symbol current_thread = nil;	/* Pointer to current thread */	
					/* Not always the running thread, */
					/* but the thread dbx displays    */
                                        /* registers and traceback on.    */
private boolean hold_other = false;/* hold_other : resume or not the other*/
                                        /* threads                          */
public Address addr_dbsubn = nil;       /* if the running thread is held */

int nb_k_threads_sig;
extern tid_running_thread;              /* use by ptrace(PTT_CONTINUE)    */
extern int  lib_type;                   /* lib type : libcma or libpthreads */
extern thrdsinfo_st *pthrdsinfo;
extern mstsave_st *pmstinfo;
extern int nb_k_threads;
extern struct procsinfo *pprocsinfo;
extern mstsave_st *mstsave_current;     /* pointer on current mstsave     */

extern boolean coredump;		/* Are we debugging a corefile? */
extern struct core_dump corehdr;	/* Hold info about the corefile */
typedef struct sigcontext sigcontext;	/* hold info for signal switching */


/*
 * In order for pthreads objects be treated as regular variables with
 * normal dbx subcommands, eg. "print $t1" , "assign $m1.islock = 0" 
 * Internal "Symbols" (variables) need to be associated with these pthreads
 * objects, and types for these variables need to be defined. Following
 * typedefs and Symbols are used for defining these Symbol types.
 */
static Symbol e_th_k_state = nil;		/* enum symbol for thread state */
static Symbol e_th_k_policy = nil;	/* enum symbol for thread policy */

/* 
 * Structure used for defining fields and info of the pthreads object types.
 */
typedef struct pthreads_field_info {
	char *name;			/* name of the field */
	Symbol *type;			/* type of this field */
	int offset;			/* actual offset in the pthreads object */
} pthreads_field_info;   

typedef struct pthreads_type_info {
	Symbol sym;			/*"Symbol" to be created for the type */
	String type_name;		/* name of the type (Symbol) */
	int num_field;			/* number of fields for this type */
	pthreads_field_info fields[MAX_FIELD];    /* info for all fields  */
} pthreads_type_info;

/* 
 * Following is actual field information for each pthreads object type.
 * This object info is used in procedure create_pthreads_types() where
 * the pthreads object type "Symbol" is created.
 * Note: Not all fields defined in the actual pthreads object in libpthreads.a
 *       are used here. Only fields useful and feasible to users are 
 *       defined.
 */
static pthreads_type_info thread_object = {	
    	nil, 
	"$$threadk",
	11,
    	{
	  {"thread_id", &t_int, offsetof2(pthread_st,th_id)}, 
	  {"state", &e_th_k_state, offsetof2(pthread_st, ti_stat)},
	  {"state_u", &t_int,offsetof2(pthread_st, state)},
	  {"tid", &t_int,offsetof2(pthread_st, ti_tid)},
	  {"mode", &t_int,offsetof2(pthread_st, ti_flags)},
	  {"held", &t_int, offsetof2(pthread_st, ti_hold)},
	  {"priority", &t_int, offsetof2(pthread_st, ti_pri)},
	  {"policy", &e_th_k_policy, offsetof2(pthread_st, ti_policy)},
	  {"scount", &t_int, offsetof2(pthread_st, ti_scount)},
	  {"cursig", &t_int, offsetof2(pthread_st, ti_cursig)},
          {"attributes", &t_addr, offsetof2(pthread_st, attr)}
    	}
};


static pthreads_type_info mutex_object = {
        nil,
        "$$mutexk",
        5,
        {
          {"mutex_id", &t_int, offsetof(pthread_mutex_t, mtx_id)},
          {"islock", &t_int, offsetof(pthread_mutex_t, lock)},
          {"owner", &t_addr, offsetof(pthread_mutex_t, owner)},
          {"flags", &t_int, offsetof(pthread_mutex_t, flags)},
          {"attributes", &t_addr,offsetof(pthread_mutex_t, attr)},
        }
};


static pthreads_type_info attr_object = {
        nil,
        "$$attrk",
        12,
        {
          {"attr_id", &t_int, offsetof(pthread_attr_st, attr_id)},
          {"type", &t_int, offsetof(pthread_attr_st, type)},
          {"state", &t_int, offsetof(pthread_attr_st, flags)},
          {"stacksize", &t_int, offsetof(pthread_attr_st, stacksize)},
          {"detachedstate", &t_int, offsetof(pthread_attr_st, detachstate)},
          {"process_shared", &t_int, offsetof(pthread_attr_st, process_shared)},
          {"contentionscope", &t_int, offsetof(pthread_attr_st,
                                               contentionscope)},
          {"priority", &t_int, offsetof(pthread_attr_st, 
                                        schedule.sched_priority)},
          {"sched", &t_int, offsetof(pthread_attr_st, schedule.sched_policy)},
          {"inherit", &t_int, offsetof(pthread_attr_st, inherit)},
          {"protocol", &t_int, offsetof(pthread_attr_st,protocol)},
          {"prio_ceiling", &t_int, offsetof(pthread_attr_st, prio_ceiling)}
        }
};

static pthreads_type_info cv_object = {
        nil,
        "$$cvk",
        4,
        {
          {"cv_id", &t_int, offsetof(pthread_cond_t, cv_id)},
          {"lock", &t_int, offsetof(pthread_cond_t, lock)},
          {"semaphore_queue", &t_addr, offsetof(pthread_cond_t,waiters)},
          {"attributes", &t_addr,offsetof(pthread_cond_t, attr)},
        }
};




/* Names for thread state, substate and type */
static char *s_name[] = {"", " ", "run","wait", " ", "susp",
                         "susp"," susp " ,"????" };
/* state user : state of pthread structure */
static char *ss_name[] = {
                                "running",
                                "terminated",
                                "blocked ",
                                "runnable "
				"????"
                          };
static char *s_mode[] = { "u", "k", " ", "?" };

static char *t_policy[] = { "other", "fifo", "rr", " ", "??"};

/*  subcommand attribute */
#define NO_P_SHARED  2    /* nothing to print */
#define NO_PROTOCOL  3    /* nothing to print */ 
#define NO_SCOPE  2       /* nothing to print */ 
#define NO_SCHEDULER 3    /* nothing to print */ 
/* fields corrupted */
#define ERROR_P_SHARED  3  /*  print ??? */
#define ERROR_PROTOCOL  4  /*  print ??? */
#define ERROR_SCOPE  3     /*  print ??? */
#define ERROR_SCHEDULER  4 /*  print ??? */
#define ERROR_KIND  4      /*  print ??? */
#define ERROR_TYPE  3      /*  print ??? */
#define ERROR_STATE  2     /*  print ??? */
#define ERROR_SSNAME  4    /*  print ??? */
#define ERROR_SNAME  8     /*  print ??? */
#define ERROR_MODE  3      /*  print ??? */
#define ERROR_POLICY  4    /*  print ??? */

#define control_value(field , er_code) \
        if ((field < 0) || (field > er_code)) field = er_code;\ 

/* thread mutex and condition attribute  : type  and state */
static char *attr_type[] = { "thr", "mutex","cond", "????" };
static char *attr_state[] = { "", "valid", "????" };
/* thread attribute : contentionscope  and sched */
static char *attr_contention[] = { "system", "process", "", "????"};
static char *attr_sched[] = { "other", "fifo", "rr", " ", "????"};
/* mutex and condition attribute : p-shared */
static char *attr_shared[] = { "no", "yes", "", "???"};
/* mutex attribute : protocol */
static char *attr_protocol[] = { "no_prio", "prio", "protect", "", "????"};
static char *attr_kind[] = { "non-rec", "recursi", "fast", "", "????"};


/*
 * NAME: k_error
 *
 * FUNCTION: Make sure error occured while executing thread subcommand is 
 *           not caused by completion or termination of process. If so, 
 *           alert user that program is no longer active.
 *
 * NOTES: Used by thread subcommand routines only
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: NONE
 *
 * RETURNS: NONE
 */
/* VARARGS1 */
private void k_error(s, a, b, c, d, e, f, g, h, i, j, k, l, m)
{
  if (notstarted(process) or isfinished(process)) { 
       error(catgets(scmc_catd, MS_runtime, MSG_265, "program is not active")); 
  } else { 
       error(s, a, b, c, d, e, f, g, h, i, j, k, l, m); 
  } 
}


/*
 * NAME: getMutexOpt_k
 *
 * FUNCTION: Scans option used with the "mutex" command
 *
 * PARAMETERS:
 *      s      - string typed in as option to the command
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: NONE
 *
 * RETURNS: -1 if option string "s" doesn't match any valid option;
 *          Otherwise the integer value of option matched.
 */
public int getMutexOpt_k(s)
char *s;
{
  lowercase(s);
  if (!strcmp(s,"lock"))
     return (int) mu_lock;
  else if (!strcmp(s,"unlock"))
      return (int) mu_unlock;
  else {
      (*rpt_error)(stderr, catgets(scmc_catd, MS_pthread, MSG_760,
                "Usage: mutex [ lock | unlock | <mutex#>] \n"));
      return -1;
  }
}

/*
 * NAME: getConditionOpt_k
 *
 * FUNCTION: Scans option used with the "condition" command
 *
 * PARAMETERS:
 *      s      - string typed in as option to the command
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: NONE
 *
 * RETURNS: -1 if option string "s" doesn't match any valid option;
 *          Otherwise the integer value of option matched.
 */
public int getConditionOpt_k(s)
char *s;
{
  lowercase(s);
  if (!strcmp(s,"wait"))
     return (int) cv_wait;
  else if (!strcmp(s,"nowait"))
      return (int) cv_nowait;
  else {
      (*rpt_error)(stderr, catgets(scmc_catd, MS_pthread, MSG_761,
                        "Usage: condition [wait | nowait | <condition#>] \n"));
      return -1;
  }
}


/*
 * NAME: getAttributeOpt_k
 *
 * FUNCTION: Prints the usage of subcommand attribute.
 *
 * PARAMETERS:
 *      s      - string typed in as option to the command
 *
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: NONE
 *
 * RETURNS: -1 (always)
 *          
 */
public getAttributeOpt_k(s)
char *s;
{
   (*rpt_error)(stderr, catgets(scmc_catd, MS_pthread, MSG_762,
                                        "Usage: attribute [<attribute#>] \n"));
      return -1;
}



/*
 * NAME: getThreadOpt_k
 *
 * FUNCTION: Scans option used with the "thread" command
 *
 * PARAMETERS: 
 *      s      - string typed in as option to the command
 * 
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: NONE
 *
 * RETURNS: -1 if option string "s" doesn't match any valid option;
 *          Otherwise the integer value of option matched.
 */
public getThreadOpt_k(s)
char *s;
{
  lowercase(s);
  if (!strcmp(s,"hold"))
      return (int) th_hold;
  else if (!strcmp(s,"unhold"))
      return (int) th_unhold;
  else if (!strcmp(s,"run"))
      return (int) th_run;
  else if (!strcmp(s,"wait"))
      return (int) th_wait;
  else if (!strcmp(s,"susp"))
      return (int) th_susp;
  else if (!strcmp(s,"term"))
      return (int) th_term;
  else if (!strcmp(s,"current"))
      return (int) th_current;
  else if (!strcmp(s,"info"))
      return (int) th_info;
  else {
      (*rpt_error)(stderr, catgets(scmc_catd, MS_pthread, MSG_763, 
	"Usage: thread [ hold | unhold  |  \n"));
      (*rpt_error)(stderr, catgets(scmc_catd, MS_pthread, MSG_764, 
	"                info | current | wait \n"));
      (*rpt_error)(stderr, catgets(scmc_catd, MS_pthread, MSG_765, 
	"                run | susp | term ] [thread#]\n"));
      return -1;
  }
}
   
/*
 * NAME: search_pthread
 *
 * FUNCTION: Search  in the pthread queue the pthread structure with tid .
 *
 * NOTES: Called by routine update_threads()  
 *
 * PARAMETERS:
 *      known_thread_addr   - begin of pthread queue 
 *      tid  		     - tid searched of the first thread 
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: NONE
 *
 * RETURNS: address of the pthread structure
 *          nil if the thread is not found
 */
private pthread_queue* search_pthread ( known_thread_addr , tid)
Address known_thread_addr;
tid_t tid;
{
   pthread_queue *ptr;               /* current pointer on pthread structure */
   pthread_queue q_buf;              /* pthread_queue (link) read            */
   tid_t thread_id;                  /* tid read                             */
   vp_st *vp_addr;
   unsigned int state;
   int badread;

   badread = dread(&q_buf, known_thread_addr , sizeof(pthread_queue));
   if (badread) return(nil);
   ptr=QUEUE_NEXT(q_buf);
   while (  ptr != nil && ptr != known_thread_addr) {
      badread = dread(&state, (Address) ((int) ptr +
                    offsetof2(pthread_st, state)), sizeof(unsigned int ));
      if (badread) return(nil);
      if (!(state & PTHREAD_RETURNED)) { /* pthread not actived */

         badread = dread(&vp_addr, (Address) ((int) ptr +
                          offsetof2(pthread_st, vp)), sizeof(Address ));
         if (badread) return(nil);
         if ( vp_addr != nil ) {
            badread = dread(&thread_id, (Address) ((int) vp_addr +
                          offsetof(vp_st,id )), sizeof(unsigned long ));
            if (badread) return(nil);
            if (thread_id == tid) return (ptr); 
         }
      }
      badread = dread(&q_buf,  ptr ,sizeof(pthread_queue));
      if (badread) return(nil);
      ptr=QUEUE_NEXT(q_buf);
   }
   return(nil);
}


/*
 * NAME: update_threads
 *
 * FUNCTION: Update all the pthread structures with kernel informations.
 *           call after each return kernel (getinfo)
 *           If coredump : nothing to do (nothing in corefile !)
 *
 * NOTES: Called by routine thread_k() when  option equals th_get_info 
 *        The libpthreads structures pthread are updated.
 *
 * PARAMETERS:
 *      known_thread_addr      - Address of the pthread_queue
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: NONE
 *
 * RETURNS: NONE
 */
private void  update_threads ( known_thread_addr)
Address known_thread_addr;
{
   thrdsinfo_st *pth;             
   pthread_queue *ptrcur;
   int i, number, n, mode;
   int state;
   unsigned long hold = 0; /* thread not held */
   unsigned long cursig = 0; /* */
   boolean found = false;

   if (!coredump) {
       /* gettid had update pthrdinfo and pmstinfo */
       /* it had also updated the registers         */ 
	nb_k_threads_sig =0;
       for ( i = 0;i < nb_k_threads;i++) {
          pth = &pthrdsinfo[i];
          /* search in user area a thread with the same tid */
          ptrcur  = search_pthread(known_thread_addr ,pth->ti_tid);

          /* if the tid is not found in the pthread queue it's not an error */
          /* the pthread can be in state : PTHREAD_RETURNED                 */

          if (ptrcur != nil) {
            /* update pthread structure */
            /* pthread with tid found */
            dwrite(&pth->ti_tid, (Address) ((int) ptrcur +
                   offsetof2(pthread_st, ti_tid)), sizeof(unsigned long ));
            dwrite(&pth->ti_policy, (Address) ((int) ptrcur +
                   offsetof2(pthread_st, ti_policy)), sizeof(unsigned long ));
            dwrite(&pth->ti_pri, (Address) ((int) ptrcur +
                   offsetof2(pthread_st, ti_pri)), sizeof(unsigned long ));
            dwrite(&pth->ti_wchan, (Address) ((int) ptrcur +
                   offsetof2(pthread_st, ti_wchan)), sizeof(unsigned long));

            state = pth->ti_state;
            /* all threads are stopped : state = run  if waiting for nothing */
            /* state = TSSTOP and ti_wtype= TNOWAIT,TWMEM => run */
            /* state = TSSTOP and ti_wtype= TWCPU         => susp */
            if (state == TSSTOP) {
               state = TSSLEEP;           /* waiting for something */
               if ((pth->ti_wtype == TNOWAIT) ||  (pth->ti_wtype == TWMEM))
                  state = TSRUN;
               else
                  if (pth->ti_wtype == TWCPU) state = TSSTOP;
            }
            dwrite(&state, (Address) ((int) ptrcur +
                   offsetof2(pthread_st, ti_stat)), sizeof(unsigned long ));

            /* mode user or kernel : depends of t_suspend */
            dwrite(&pth->ti_scount, (Address) ((int) ptrcur +
                   offsetof2(pthread_st, ti_flags)), sizeof(unsigned long ));

            cursig = 0;
	    /* only one thread with cursig != 0 in dbx (the running_thread) */
            if (pth->ti_flag  & TTRCSIG) {
		if (!found && (pth->ti_cursig == process->signo)) {
                	cursig = pth->ti_cursig;
			found = true;
		}
                pth->ti_scount = MODE_U;
		nb_k_threads_sig++;
            }

            dwrite(&pth->ti_scount, (Address) ((int) ptrcur +
                   offsetof2(pthread_st, ti_scount)), sizeof(unsigned long ));
            dwrite(&cursig, (Address) ((int) ptrcur +
                   offsetof2(pthread_st, ti_cursig)), sizeof(unsigned long ));
         
         } /* ptrcur  */
      }/*for */
   if (traceexec) {
         (*rpt_output)(stdout, "!! number of threads with signal %d\n",
                       nb_k_threads_sig);
   }

   } /* if coredump */
}
/*
 * NAME: setThreadregs_k
 *
 * FUNCTION: Sets register values for  specified thread.
 *           The pthread is attached to a kernel thread write the 
 *           registers using ptrace()
 *           There is not ptrace(0 to write only one register for
 *           a thread so we are obliged to write all the registers !
 *
 * NOTES: Called by routine procreturn() if the running_thread had changed
 *        (the running_thread was held or after a call proc() the running
 *        can cahnged). We have to restore the registers of the "old"
 *        running thread.
 *
 * PARAMETERS:
 *     tid     - thread id of the old running thread
 *      n      - register number to be modified
 *     addr    - addresse of the saved registers 
 *               
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: NONE
 *
 * RETURNS: NONE
 */
public void setThreadregs_k(tid ,n, addr)
tid_t tid;
int n;
Address addr;
{
   tid_t save_tid;

   /* the thread is attached to a kernel thread  (libpthreads 1:1) */
   /*we can use ptrace */

   save_tid = process->tid;
   process->tid = tid;
   if (n < 0) {      /* floating point registers */
      /* the thread is attached to a kernel thread we can use ptrace */
      writefprs(process,addr);
   } else if (n < NGREG) {               /* general registers */
         writegprs (process,addr);
   } else if (n < NGREG+NSYS) {         /* system registers */
         writesprs (process,addr);
   }
   process->tid = save_tid;
}

/*
 * NAME: isThreadObjectSymbol_k
 *
 * FUNCTION: Determine whether symbol passed is a thread, attribute,
 *           condition variable or mutex object.
 *
 * PARAMETERS:
 *      ThreadObjectSym      - Symbol of object to be checked
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: NONE
 *
 * RETURNS: true -  if symbol is a thread, attribute,
 *                  condition variable or mutex object.
 *          false - otherwise
 */
public boolean isThreadObjectSymbol_k(Symbol ThreadObjectSym)
{
   if ((ThreadObjectSym) &&
       ((ThreadObjectSym->type == thread_object.sym) ||
        (ThreadObjectSym->type == mutex_object.sym)  ||
        (ThreadObjectSym->type == attr_object.sym)   ||
        (ThreadObjectSym->type == cv_object.sym)))
   {
       return(true);
   }
   else
   {
       return(false);
   }
}

/*
 * NAME: switchThread_k
 *
 * FUNCTION: Switch dbx context to a different thread. Context includes
 *           mainly the stack (frame pointer), the registers, and current
 *           line, func, and file.
 *
 * NOTES: Called by routine thread() when the "thread current <#>"
 *        subcommand is used. If thread <#> is nil, meaning no thread
 *        number is issued, dbx should display name of the current thread -
 *        which is handled in routine thread() called from eval().
 *        If a thread Symbol (argument ThreadSym != nil) is provided,
 *        just switch to it.
 *        Called by routine eval(0) to switch on running_thread (cont,step ..)
 *
 *        This routine will be modified for libpthreads M:N.
 *
 * PARAMETERS:
 *      ThreadSym      - Symbol of thread to switch to.
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: NONE
 *
 * RETURNS: NONE
 */
public void switchThread_k(ThreadSym)
Symbol ThreadSym;
{
   Address addr;                /* thread address for thread */
   vp_st *vp_addr;
   tid_t thread_id;             /* tid  : kernel tid         */
   Address saveaddr;            /* address where registers are saved */
   int i, regsize;
   mstsave_st *ptr;              /* pointer on the uthread structure in the */
                                 /*   corefile */
   int badread;
   thrdsinfo_st  *ptrinfo;
   unsigned long mode;                  /* mode kernel or user            */
   Address thread_sp;
   Address iar;

   /* Make sure the symbol is a thread... */
   if (ThreadSym && ThreadSym->type == thread_object.sym) {
      /* Update the registers of running thread before */
      /* switching in case it was modified.            */
      /* update in all cases for kernel threads */
      /* this procedure is called on call, step,callproc,goto,return */
      setregs(process);
      current_thread = ThreadSym;                 /* switch... */
   } else {
      /* probably should never get here... */
      k_error( catgets(scmc_catd, MS_pthread, MSG_767,
               "no thread available"));
      return;
   }

   /* get address of thread  */
   eval(amper(build(O_SYM, ThreadSym)));
   addr = pop(Address);
   badread = dread(&vp_addr, (Address) ((int) addr +
                    offsetof2(pthread_st, vp)), sizeof(Address ));
   if (badread) return;
   if ( vp_addr != nil ) {
       badread = dread(&thread_id, (Address) ((int) vp_addr +
                        offsetof(vp_st,id )), sizeof(unsigned long ));
       if (badread) return;
   }

   process->tid = thread_id; 
   if ( coredump) {
      /* if we are debugging corefile, we cannot use readreg(ptrace) */
      /* to get values of regs, they are saved in the corefile.      */
      /* if kernel threads the thread_info structures follow the core_dump */
      /* structure, so we have to find the structure of this thread  */
      ptr=find_thread(thread_id);
      mstsave_current = ptr;
      if (ptr)
         copyregs(ptr->gpr, process->reg,
                         ptr->fpr,process->freg);
      else
         k_error( catgets(scmc_catd, MS_pthread, MSG_768,
                 "kernel thread id=%d not found"),thread_id);
   } else {
      /* if the thread is stopped in kernel mode we have to read the stack */
      /* pointer in thdsinfo structure (ptrace() doesn't work)             */
      badread = dread(&mode, (Address) ((int) addr +
                offsetof2(pthread_st, ti_scount)), sizeof(unsigned long));

      if (badread) return;
      if (mode == MODE_K) { /* we can't use ptrace to read registers */
         /* getthrds() returns stack pointer */
         /* iar not returned by getthrds() */
         /* take the next frame in stack : return address in here */

         for (i=0; i < NGREG+NSYS; i++) {
                process->reg[i] = 0xdeadbeef;
         }
         for (i = 0; i <= fpregs; i++ ) {
                process->freg[i] = 0.0;
         }

         for (i = 0; i < nb_k_threads; i++) {
             ptrinfo=&pthrdsinfo[i];
             if (ptrinfo->ti_tid == thread_id) break;
         }
         if (i != nb_k_threads) {
             thread_sp = ptrinfo->ti_ustk; /* stack pointer */
             badread = dread(&thread_sp,thread_sp , sizeof(int));
             if (badread) return;
             process->reg[GPR1] = thread_sp;
             thread_sp = thread_sp + 2*sizeof(int); /* iar */
             badread = dread(&iar, thread_sp , sizeof(int));
             if (badread) return;
             pc = process->reg[SYSREGNO(IAR)] = iar;
         }
         else
             k_error( catgets(scmc_catd, MS_pthread, MSG_768,
                     "kernel thread id=%d not found"),thread_id);

      }
      else {


         regsize = sizeof(Word);

         /* read the 32 general purpose registers */
         /* read the special purpose registers */
         /* read the 32 floating point registers */
         readgprs(process,&process->reg[0]);
         readsprs(process,&process->reg[NGREG]);
         readfprs(process,&process->freg[0]);
         /* registers updated  */
         for (i = 0; i <= NGREG+NSYS+MAXFREG; i++) {
             /* set the valid bit so that dbx would not go read it again */
             process->dirty[regword(i)] |= regbit(i);
             process->dirty[regword(i)] = 0;
         }

      }
   }
   pc = process->reg[SYSREGNO(IAR)];                     /* set global pc */
   cursource = srcfilename(pc);                        /* set source file */
   cursrcline = srcline(pc);    /* set source line */
   setcurfunc(whatblock(pc));   /* set current func */
   action_mask |= EXECUTION;
}

/*
 * NAME: insert_pthreads_obj
 *
 * FUNCTION: Creates and inserts a Symbol associated with a pthreads object
 *           into the dbx symbol table
 *
 * NOTES: This routine is called by other thread routines then a new pthreads 
 *        object is found and a dbx Symbols assiocated with this new 
 *        object is needed.
 *
 * PARAMETERS: 
 *      n      - name of the object, eg. ($t1, $m2, $a3,...)
 *      t      - Symbol of type of object
 *      addr   - address of pthreads object this symbol is associated with
 * 
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: NONE
 *
 * RETURNS: a Symbol representing the pthreads object
 */
private Symbol insert_pthreads_obj(n, t, addr)
Name n;
Symbol t;
Address addr;
{
   Symbol s;

   s = insert(n);
   s->language = t->language;
   s->class = VAR;
   s->storage = EXT;
   s->level = 3;		
   s->type = t;
   s->symvalue.offset = addr;
   return s;
}

/*
 * NAME: update_running_thread
 *
 * FUNCTION: Creates or updates dbx Symbol $running_thread as an  int object.
 *
 * NOTES: call by thread_k with th_get_info option.
 *        This object allows : stop at 30 if ($running_thread == 7)
 *
 * PARAMETERS:
 *      addr        - address of pthreads object this symbol is associated with
 *                    address of the thread_id field.
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: NONE
 *
 * RETURNS: Updated Symbol representing the  object
 */
private Symbol update_running_thread(addr)
Address  addr;
{
   char name[MAX_NAME_LEN];
   Symbol objectSym;
   Name objectName;
   String prefix;
   prefix = "$running_thread";

   sprintf(name, "%s", prefix);

   objectName = identname(name, false);
   /* Is object symbol already created? */
   if ((objectSym = lookup(objectName)) != nil)
      objectSym->symvalue.offset = addr;
   else
      objectSym = insert_pthreads_obj(objectName,t_int,addr);
   return objectSym;
}




/*
 * NAME: update_pthreads_obj
 *
 * FUNCTION: Creates and updates dbx Symbol representing a pthreads object.
 *
 * NOTES: 
 *
 * PARAMETERS: 
 *      prefix      - string prefix for names of pthreads objects, eg. "$t", 
 *                    "$c"...
 *      id          - ID of the object
 *      addr        - address of pthreads object this symbol is associated with
 *      object_type - type of pthreads object (thread or mutex or ... )
 * 
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: NONE
 *
 * RETURNS: Updated Symbol representing the pthreads object
 */
private Symbol update_pthreads_obj(prefix, id, addr, object_type)
String prefix;
int id;
Address addr;
Symbol object_type;
{
   char name[MAX_NAME_LEN];
   Symbol objectSym;
   Name objectName; 

   sprintf(name, "%s%d", prefix, id);
   objectName = identname(name, false); 
   /* Is object symbol already created? */
   if ((objectSym = lookup(objectName)) != nil)
      objectSym->symvalue.offset = addr;
   else
      objectSym = insert_pthreads_obj(objectName, object_type, addr);
   return objectSym;
}



/*
 * NAME: createEnum
 *
 * FUNCTION: Creates a Symbol representing an enum type, 
 *           giving an array of enum element and the number of element.
 *
 * NOTES: Used in creating Symbols for types of fields in
 *        pthreads object symbols.
 *
 * PARAMETERS: 
 *      ename    - name of the enum type
 *      nlist    - array of enum element
 *      nfield   - number of element in enum type
 * 
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: NONE
 *
 * RETURNS: A Symbol representing the enum type
 */
private Symbol createEnum(ename, nlist, nfields)
char *ename;
char *nlist[];
int nfields;
{
   int count;
   Symbol s, u;

   s = newSymbol(identname(ename, true), 1, SCAL, nil, nil);
   s->language = cLang;
   s->type = t_int;
   s->symvalue.iconval = nfields;
   u = s;
   for ( count = 0; count < nfields; count++) {
      u->chain = insert(identname(nlist[count],true));
      u = u->chain;
      u->language = cLang;
      u->class = CONST;
      u->level = 2;
      u->type = s;
      u->symvalue.constval = cons(O_LCON, (long) count);
      u->symvalue.constval->nodetype = s;
   }
   return s;
}


/*
 * NAME: create_pthreads_enum
 *
 * FUNCTION: Creates Symbols of enum types used by the pthreads object Symbols.
 *
 * PARAMETERS: NONE
 * 
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: NONE
 *
 * RETURNS: NONE
 */
private void create_pthreads_enum()
{
   e_th_k_state = createEnum("$th_state_k", s_name, ESIZE(s_name));
   e_th_k_policy = createEnum("$th_policy_k", t_policy, ESIZE(t_policy));
}


/*
 * NAME: createType_pthreads
 *
 * FUNCTION: Creates Symbols for types of pthreads objects.
 *
 * NOTE: The fields are based upon the fields in actual pthreads object 
 *       declarations, not all fields are used here.
 *
 * PARAMETERS: 
 *      t      - pthreads_type_info structure containing info about
 *               the pthreads object
 * 
 * RECOVERY OPERATION: NONE NEEDET
 *
 * DATA STRUCTURES: NONE
 *
 * RETURNS: NONE
 */
private void createType_pthreads(t)
pthreads_type_info *t;
{
   Symbol s;
   int i;
   int typesize;

   s = t->sym = newSymbol(identname(t->type_name, true), 1, RECORD, nil, nil);
   t->sym->symvalue.offset = 0; 
   t->sym->language = cLang;
   for (i = 0; i < t->num_field; i++) {
      s->chain = newSymbol(identname(t->fields[i].name, true), 2, 
			   FIELD, *(t->fields[i].type), nil);
      /* we can't assign a $ti variable */
      if(!strcmp(t->type_name,"$$threadk"))
         s->chain->isConst = 1; 
      s = s->chain;
      /* Fill in the offset from start of struct */
      s->symvalue.field.offset = t->fields[i].offset * BITSPERBYTE;
      /* Fill in the size of the field */
      if (*(t->fields[i].type)) 	/* make sure it's not null... */
         s->symvalue.field.length = size(*(t->fields[i].type))*BITSPERBYTE;
      else
         s->symvalue.field.length = 0;
      s->language = cLang;
      /* Update size of pthreads symbol struct... */
      if ((typesize = 
	     (s->symvalue.field.offset + s->symvalue.field.length)/BITSPERBYTE)
		    > t->sym->symvalue.offset)        
         /* size of structure is largestOffset of any field + it's size;*/
	 t->sym->symvalue.offset = typesize;
   }
}


/*
 * NAME: create_pthreads_types
 *
 * FUNCTION: Created dbx Symbols representing all pthreads object types
 *
 * NOTES: Called by routine symbols_init() at dbx startup.
 *
 * PARAMETERS: NONE
 * 
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: NONE
 *
 * RETURNS: NONE
 */
public void create_pthreads_types()
{

  /* Create Symbols for enum types used in pthreads */
  create_pthreads_enum();

  /* Create Symbol for pthreads objects */
  createType_pthreads(&thread_object);

  createType_pthreads(&mutex_object);
  createType_pthreads(&attr_object);
  createType_pthreads(&cv_object);

}


/*
 * NAME: isRequested_k
 *
 * FUNCTION: Checks if an object id is contained in a list of request
 *
 * NOTES: Used by thread subcommand routines to decided if an action is
 *        requested for an pthreads object.
 *
 * PARAMETERS: 
 *      p      - Node containing a list of requested id's
 *      id     - the id to look for in the list
 * 
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: NONE
 *
 * RETURNS: True if id is in list of requested
 *          Otherwise return false.
 */
private boolean isRequested_k(p, id)
Node p;
unsigned long id;
{
   Node n1 = p;
   /* many element in the list */
   while (n1 && n1->op == O_COMMA) {
      if (n1->value.arg[0]->op == O_LCON &&   
          n1->value.arg[0]->value.lcon == id) {
         return true;
      }
      n1 = n1->value.arg[1];
   } 
   /* Only one element */ 
   if (n1 && n1->op == O_LCON && n1->value.lcon == id)
      return true;                         
   else 
      return false;
}

/*
 * NAME: search_object_id
 *
 * FUNCTION: Search  in the pthread queue the pthread structure with tid .
 *
 * NOTES: Called by routine verifArgObject_k()
 *
 * PARAMETERS:
 *      known_thread_addr  - begin of  queue
 *      type               - type of object:thread attribute mutex or condition
 *      tid                - number searched
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: NONE
 *
 * RETURNS: NONE
 *
 */
private void search_object_id (known_addr , type , tid)
Address known_addr;
int type;
int tid;
{
   pthread_queue *ptr;               /* current pointer on pthread structure */
   pthread_queue q_buf;              /* pthread_queue (link) read            */
   int object_id;                    /* tid read                             */
   unsigned int state;               /* for thread only                      */
   int badread;
   boolean found;                    /* tid found in queue                   */
   Address p_obj;                    /* for mutexes and condition var */

   found = false;
   badread = dread(&q_buf, known_addr , sizeof(pthread_queue));
   if (badread) return;
   ptr=QUEUE_NEXT(q_buf);
   while (  ptr != nil && ptr != known_addr) {
     /* case mutex or cond : the list contains the pointer */
     if (( type == OBJ_MUTEX) || (type == OBJ_CONDITION)) {
        badread = dread(&p_obj, (Address) ((int)ptr +
                        offsetof(__dbx_cond, pt_cond)), sizeof(int));
        if (badread) return;
     }
     switch(type) {

       case OBJ_THREAD:
       {
          badread = dread(&state, (Address) ((int) ptr +
                          offsetof2(pthread_st, state)), sizeof(unsigned int ));
          if (badread) return;
          if (!(state & PTHREAD_RETURNED)) { /* pthread not actived */

             badread = dread(&object_id, (Address) ((int) ptr +
                             offsetof2(pthread_st,th_id)), sizeof(int ));
             if (badread) return;
             if (object_id == tid) found = true;
          }
          break;
       }
       case OBJ_MUTEX:
       {
          badread = dread(&object_id, (Address) ((int) p_obj +
                       offsetof(pthread_mutex_t,mtx_id)), sizeof(int ));
          if (badread) return;
          if (object_id == tid) found = true;
          break;
       }
       case OBJ_ATTRIBUTE:
       {
          badread = dread(&object_id, (Address) ((int) ptr +
                        offsetof(pthread_attr_st,attr_id)), sizeof(int ));
          if (badread) return;
          if (object_id == tid) found = true;
          break;
       }
       case OBJ_CONDITION:
       {
          badread = dread(&object_id, (Address) ((int) p_obj +
                          offsetof(pthread_cond_t,cv_id)), sizeof(int ));
          if (badread) return;
          if (object_id == tid) found = true;
          break;
       }
        default:        /* unknown */
          return;
      } /* switch */
      if (found) break;

      badread = dread(&q_buf,  ptr ,sizeof(pthread_queue));
      if (badread) return;
      ptr=QUEUE_NEXT(q_buf);
   }
   if (!found) {
     switch(type) {
       case OBJ_THREAD:
       {
          k_error( catgets(scmc_catd, MS_pthread, MSG_766,
                   "\'$t%d\' is not an existing thread."),tid);
          break;
       }
       case OBJ_MUTEX:
       {
          k_error( catgets(scmc_catd, MS_pthread, MSG_777,
                   "\'$m%d\' is not an existing mutex."),tid);
          break;
       }
       case OBJ_ATTRIBUTE:
       {
          k_error( catgets(scmc_catd, MS_pthread, MSG_776,
                   "\'$a%d\' is not an existing attribute."),tid);
          break;
       }
       case OBJ_CONDITION:
       {
          k_error( catgets(scmc_catd, MS_pthread, MSG_778,
                   "\'$c%d\' is not an existing condition."),tid);
          break;
       }
      } /* switch */
   }
   return;
}

/*
 * NAME: verifArgObject_k
 *
 * FUNCTION: Verifies  the arguments of  subcommand
 *
 * NOTES: Used by  subcommand routines(thread_k,attribute_k....)
 *
 * PARAMETERS:
 *      known_addr      - begin of pthread queue
 *      type            - type of object : thread mutex attribute or condition
 *      p               - Node containing a list of verified id's
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: NONE
 *
 * RETURNS: Nothing
 *
 */
private void verifArgObject_k(known_addr,type,p)
Address known_addr;
int type;
Node p;
{
   Node n1 = p;
   /* many element in the list */
   while (n1 && n1->op == O_COMMA) {
      if (n1->value.arg[0]->op == O_LCON)
         search_object_id(known_addr,type,n1->value.arg[0]->value.lcon);
      n1 = n1->value.arg[1];
   }
   /* Only one element */
   if (n1 && n1->op == O_LCON )
      search_object_id(known_addr,type,n1->value.lcon );
}

   
/*
 * NAME: getWaiters_k
 *
 * FUNCTION: Return the numbers of waiters or print names of
 *           waiters if requested.
 *
 * NOTES: Used by thread subcommand routines to display the list of
 *        waiters for a mutex or a condition variable
 *
 * PARAMETERS:
 *      semaphore_queue  - address of queue containing list of waiters
 *      doprt            - to print names of waiters or not
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: NONE
 *
 * RETURNS: number of waiters in the queue
 */
private int getWaiters_k(semaphore_queue, doprt)
Address semaphore_queue;
Boolean doprt;
{
   __ptq_queue th_q_buf;       /* buffer for holding one queue section */
   __ptq_queue *th_ptr;        /* pointer used to advance queue        */
   int thread_id;
   int num_wait = 0;
   int badread;

   /* read first section of semaphore.queue in pthreads object... */
   badread = dread(&th_q_buf, semaphore_queue, sizeof(__ptq_queue));
   if (badread) return(0);
   th_ptr = QUEUE_NEXT(th_q_buf);
   while ( th_ptr != semaphore_queue) {
       if (doprt) {
           /* If printing is requested, get thread id and print... */
           badread = dread(&thread_id, (Address) ( (int)th_ptr +
                              offsetof(pthread_st, th_id)), sizeof(int));
           if (badread) return(0);
           (*rpt_output)(stdout, "$t%d ", thread_id);
       }
       /* Get contents of next thread queue section */
       badread = dread(&th_q_buf, th_ptr, sizeof(__ptq_queue));
       if (badread) return(0);
       th_ptr = QUEUE_NEXT(th_q_buf);
       /* advance counter */
       num_wait++;
   }
   return num_wait;
}

/*
 * NAME: getwaittype
 *
 * FUNCTION: convert a waitchannel to some sort of symbolic form.
 *              These events are hardcoded from proc.h
 *
 * NOTES:  this procedure is the same procedure used by command ps.
 *         this procudure is not used, wchan is printed (address of 
 *             the wait channel )
 *
 * RETURNS: pointer to a symbolic name for the event type.
 */
/*
 * types of process waits, p_wtype
 */

char *
getwaittype(wtype)
char    wtype;

{
     switch(wtype) {

                case TNOWAIT:
                                return ("NOWAIT");

                case TWEVENT:   /* waiting for event(s) signal?     */
                                return ("EVENT");

                case TWLOCK:    /* waiting for serialization lock   */
                                return ("LOCK");

                case TWTIMER:   /* waiting for timer                */
                                return ("TIMER");

                case TWCPU:     /* waiting for CPU (in ready queue) */
                                return ("CPU");

                case TWPGIN:    /* waiting for page in              */
                                return ("PGIN");

                case TWPGOUT:   /* waiting for page out level       */
                                return ("PGOUT");

                case TWPLOCK:   /* waiting for physical lock        */
                                return ("PLOCK");

                case TWFREEF:   /* waiting for a free page frame    */
                                return ("FREEF");

		case TWMEM:     /* waiting for memory */
				return ("MEM");

                default:        /* unknown */
                                return("");
        }
}

/*
 * NAME: display_thread_k
 *
 * FUNCTION: Gathers and displays regular info about a thread
 *
 * NOTES: Used by rountine thread_k() when the "thread" command with no 
 *        special option is issued
 *
 * PARAMETERS: 
 *      pthread_addr    - address of the thread control block
 *      thread_id   - id of thread we are dealing with
 *      state       - state of thread is in (kernel state)
 *      print_title - print title info or not
 *      fullinfo    - print full info or not
 *      thisthread  - pointer to symbol of this thread
 * 
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: NONE
 *
 * RETURNS: NONE
 */
private void display_thread_k(pthread_addr, thread_id, state,
                            print_title, fullinfo, thisthread)
Address pthread_addr;
int thread_id;
unsigned long state;
Boolean *print_title;
Boolean fullinfo;
Symbol thisthread;
{
   uthread_st *ptr;
   unsigned state_u;                    /* state__u of thread list       */
   tid_t save_tid;                        /* save the process->tid */
   vp_st *vp_addr;
   int is_held;                         /* is thread on held?             */
   unsigned long mode;			/* mode kernel or user   	  */
   unsigned long t_id;			/* thread id        	  	  */
   unsigned long pri;                   /* thread priority                */
   unsigned long policy;                /* scheduling policy of thread    */
   unsigned long scount;                /* count of thread preemptions    */
   pthread_func_t iar;                  /* iar of thread                  */
   thrdsinfo_st  *ptrinfo;              /* pointer on thrdinfo struct     */
   Address thread_sp;                   /* stack pointer                  */
   int i;
   unsigned long wchan;                 /* event for wchich thread is waiting*/
   unsigned long wtype;                 /* type of thread wait            */
   int scope = 0;                       /* local or global                */
   pthread_attr_t attr_addr;            /* attribute address              */  
   Symbol f;                            /* Symbol of function thread in   */
   int badread;
   Word save_reg[NSYS];                 /* save special registers         */

   badread = dread(&attr_addr, (Address) ((int) pthread_addr + 
         offsetof2(pthread_st, attr)), sizeof(Address ));
   if (badread) return;
   if (attr_addr)
       badread = dread(&scope, (Address) ((int) attr_addr + 
                  offsetof(pthread_attr_st,contentionscope )), sizeof(int));
       if (badread) return;
   /* read needed field info from pthread */
   if(coredump) {
         badread = dread(&vp_addr, (Address) ((int) pthread_addr +
                          offsetof2(pthread_st, vp)), sizeof(Address ));
         if (badread) return;
         if ( vp_addr != nil ) 
            badread = dread(&t_id, (Address) ((int) vp_addr +
                          offsetof(vp_st,id )), sizeof(unsigned long ));
            if (badread) return;
   } else {

   	badread = dread(&t_id, (Address) ((int)pthread_addr +
                       offsetof2(pthread_st, ti_tid)), sizeof(unsigned));
        if (badread) return;
   }
   badread = dread(&state_u, (Address) ((int)pthread_addr +
                       offsetof2(pthread_st, state)), sizeof(unsigned));
   if (badread) return;
   /* state of libpthreads is an "OR" of constants */
   if (state_u & PTHREAD_RETURNED)
        state_u = TERMINATED;
   else
        if(state_u & PTHREAD_WAITING)
                state_u = BLOCKED;
        else
                state_u = RUNNING;


   badread = dread(&is_held, (Address) ((int)pthread_addr +
                   offsetof2(pthread_st, ti_hold)), sizeof(unsigned long));
   if (badread) return;
   badread = dread(&mode, (Address) ((int)pthread_addr +
                offsetof2(pthread_st, ti_scount)), sizeof(unsigned long));
   if (badread) return;
   badread = dread(&wchan, (Address) ((int)pthread_addr +
                  offsetof2(pthread_st, ti_wchan)), sizeof(unsigned long));
   if (badread) return;
   badread = dread(&wtype, (Address) ((int)pthread_addr +
                 offsetof2(pthread_st, ti_wtype)), sizeof(unsigned long));
   if (badread) return;

/* in case of core file :                                       */
/*  state-k = TSNONE :   if the kernel thread exists          */
/*            STERM : term if the kernel thread does not exist  */
/*  mode    = nothing                                           */
/*  wchan   = nothing                                           */
   if(coredump) {
      ptr = find_thread(t_id);
      wchan = 0;  
      if( ptr) state = TSNONE;
      else state = TSZOMB;
      mode =   MODE_UNDEFINED;
      if (ptr) 
        iar = (Address) ptr->ut_save.iar;
      else iar = nil;
   } else {
     /* read all special registers  of the thread to have iar */
      if (mode == MODE_K) { /* we can't use ptrace to read registers */
         /* iar not returned by getthrds() */
         /* take the next frame in stack : return address is here */
         for (i = 0; i < nb_k_threads; i++) {
             ptrinfo=&pthrdsinfo[i];
             if (ptrinfo->ti_tid == t_id) break;
         }
         if (i != nb_k_threads) {
             thread_sp = ptrinfo->ti_ustk; /* stack pointer */
             badread = dread(&thread_sp,thread_sp , sizeof(int));
             if (badread) return;
             thread_sp = thread_sp + 2*sizeof(int); /* iar */
             badread = dread(&iar, thread_sp , sizeof(int));
             if (badread) return;
         }
         else
             k_error( catgets(scmc_catd, MS_pthread, MSG_768,
                     "kernel thread id=%d not found"),t_id);

      }
      else { /* user mode */
        for (i=0; i < NSYS; i++) {
               save_reg[i] = process->reg[NGREG+i];
        }
        save_tid = process->tid;
        process->tid = t_id;   /* for readsprs  iar  */
        readsprs(process,&process->reg[NGREG]);
        iar = (Address) reg(SYSREGNO(PROGCTR)); 
        /* restore special registers of the current thread */
        process->tid = save_tid;
        for (i=0; i < NSYS; i++) {
               process->reg[NGREG+i] = save_reg[i];
        }
        /* ptrace(PTT_READ_SPRS) breaks floating point registers */
         readfprs(process,&process->freg[0]);
     }
   }

/* Get iar of thread and display function thread is running.    */
/* for  thread, iar is in register                              */
   if (iar) 
      f = whatblock((Address)iar);
   else f = nil;

   /* control fields */
   control_value(state_u, ERROR_SSNAME);
   control_value(state, ERROR_SNAME);
   control_value(mode, ERROR_MODE);
   control_value(scope, ERROR_SCOPE);

   /* display title if needed... */
   if (*print_title || fullinfo) {
      (*rpt_output)(stdout, 
      " thread  state-k   wchan state-u   k-tid mode held scope function\n");
      *print_title = false;
   }

   /* display the arrow pointer to current thread */
   /* the current thread : ">"                    */
   /* the running_thread : "*"                    */

   if ( thisthread == current_thread )
      (*rpt_output)(stdout, "%c",'>');
   else
      (*rpt_output)(stdout, "%c", (thisthread == running_thread)?'*':' ');


   (*rpt_output)(stdout, 
          "$t%-4d  %-5s   ",
          (int) thread_id, s_name[state]);
   /* display wchan*/
   if (wchan)
         (*rpt_output)(stdout, "%7lx", wchan);
      else
         (*rpt_output)(stdout, "%7s", "       ");
         
   (*rpt_output)(stdout, 
          " %-8s%7d   %-1s  %3s   %-3.3s  %-20s\n",
          ss_name[state_u], t_id , s_mode[mode],
          (Boolean)is_held?"yes":"no",attr_contention[scope], 
	  (f ? symname(f) : nil));
}
/*
 * NAME: thread_info_k
 *
 * FUNCTION: Gathers and displays extended info about a thread
 *
 * NOTES: Used by rountine thread_k() when the "thread info" option is issued
 *
 * PARAMETERS:
 *      pthread_addr    - address of the thread control block
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: NONE
 *
 * RETURNS: NONE
 */
private void thread_info_k(pthread_addr)
Address pthread_addr;
{
   int ti_pri_k;                  /* priority                        */
   int ti_pri_u;                  /* priority                        */
   int ti_policy_k;               /* policy                          */
   int ti_policy_u;               /* policy                          */
   int thd_errno;                 /* per thread errno                */
   Address start_pc;              /* start routine address           */
   int event;                     /* event                           */
   int size;                      /* size of pthread                 */
   int attached = 1;              /* kernel thread attached          */
   Address stack_limit;           /* address for top of stack        */
   Address stack_base;            /* address for base of stack       */
   Address stack_size;            /* size of stack                   */
   tid_t ti_tid;
   tid_t save_tid;
   mstsave_st *ptr;               /* pointer on the uthread structure in the */
   thrdsinfo_st  *ptrinfo;
   int i;
   unsigned long mode;
   Address thread_sp;             /* stack pointer for the thread    */

   Symbol proc = nil;
   Name n;

   pthread_attr_t attr_addr;
   vp_st *vp_addr;
   int *errno_addr;
   int badread;

   badread = dread(&start_pc, (Address) ((int) pthread_addr + 
         offsetof2(pthread_st, func)), sizeof(Address ));
   if (badread) return;

   if (start_pc == NILFUNC(void *)) { /* it is the initial thread : main */
      n = identname("main",true);
      find(proc, n) where isroutine(proc) endfind(proc);
      start_pc  = codeloc(proc);
   }

   badread = dread(&ti_pri_k, (Address) ((int) pthread_addr + 
         offsetof2(pthread_st, ti_pri)), sizeof(unsigned long ));
   if (badread) return;

   badread = dread(&ti_tid, (Address) ((int) pthread_addr + 
         offsetof2(pthread_st, ti_tid)), sizeof(int));
   if (badread) return;

   badread = dread(&event, (Address) ((int) pthread_addr + 
         offsetof2(pthread_st, event)), sizeof(int));
   if (badread) return;

   badread = dread(&ti_policy_k,(Address)((int) pthread_addr +
         offsetof2(pthread_st, ti_policy)), sizeof(int ));
   if (badread) return;

   badread = dread(&attr_addr, (Address) ((int) pthread_addr + 
         offsetof2(pthread_st, attr)), sizeof(Address ));
   if (badread) return;
   if (attr_addr) {
      badread =dread(&ti_pri_u, (Address) ((int) attr_addr + 
            offsetof(pthread_attr_st,schedule.sched_priority )), sizeof(int));
      if (badread) return;
      badread = dread(&ti_policy_u, (Address) ((int) attr_addr + 
            offsetof(pthread_attr_st,schedule.sched_policy)), sizeof(int));
      if (badread) return;
   }
   badread = dread(&vp_addr, (Address) ((int) pthread_addr +
         offsetof2(pthread_st, vp)), sizeof(Address ));
   if (badread) return;
   badread = dread(&errno_addr, (Address) ((int) pthread_addr +
         offsetof2(pthread_st, thread_errno)), sizeof(Address ));
   if (badread) return;
   badread = dread(&thd_errno, (Address) ((int) errno_addr ), sizeof(int ));
   if (badread) return;
   badread = dread(&stack_limit, (Address) ((int) vp_addr +
         offsetof(vp_st, stack.limit)), sizeof(unsigned long ));
   if (badread) return;
   badread = dread(&stack_base, (Address) ((int) vp_addr +
         offsetof(vp_st, stack.base)), sizeof(unsigned long ));
   if (badread) return;
   badread = dread(&stack_size, (Address) ((int) vp_addr +
         offsetof(vp_st, stack.size)), sizeof(unsigned long ));
   if (badread) return;

   if (coredump) {
      /* if coredump : no  kernel scheduler information */
      ti_policy_u = NO_POLICY;
      ti_pri_u = 0;
      /* kernel thread id is in vp structure */
      badread = dread(&vp_addr, (Address) ((int) pthread_addr +
                       offsetof2(pthread_st, vp)), sizeof(Address ));
      if (badread) return;
      if ( vp_addr != nil )
         badread = dread(&ti_tid, (Address) ((int) vp_addr +
                         offsetof(vp_st,id )), sizeof(unsigned long ));
      if (badread) return;

      /* we can't use ptrace do read stack pointer */
      save_tid = process->tid;
      ptr=find_thread(ti_tid);
      mstsave_current = ptr;
      if (ptr)
         copyregs(ptr->gpr, process->reg,
                         ptr->fpr,process->freg);
      else
         k_error( catgets(scmc_catd, MS_pthread, MSG_768,
                 "kernel thread id=%d not found"),ti_tid);
      thread_sp =  reg(GPR1);
      process->tid = save_tid;
      ptr=find_thread(save_tid);
      mstsave_current = ptr;
      if (ptr)
         copyregs(ptr->gpr, process->reg,
                         ptr->fpr,process->freg);
      else
         k_error( catgets(scmc_catd, MS_pthread, MSG_768,
                 "kernel thread id=%d not found"),save_tid);

   }
   else {
      badread = dread(&mode, (Address) ((int) pthread_addr +
                offsetof2(pthread_st, ti_scount)), sizeof(unsigned long));
      if (mode == MODE_K) { /* we can't use ptrace to read registers */
         /* if the thread is stooped in kernel mode don't use ptrace() */
         /* getthrds() provides the stack pointer */
         for (i = 0; i < nb_k_threads; i++) {
             ptrinfo=&pthrdsinfo[i];
             if (ptrinfo->ti_tid == ti_tid) break;
         }
         if (i != nb_k_threads) {
             thread_sp = ptrinfo->ti_ustk; /* stack pointer */
             /* take next frame */
             badread = dread(&thread_sp,thread_sp , sizeof(int));
             if (badread) return;
         }
         else
             k_error( catgets(scmc_catd, MS_pthread, MSG_768,
                     "kernel thread id=%d not found"),ti_tid);
      }
      else { /* user mode : user ptrace() */
         /* read all general registers  of the thread to have stack pointer */
         save_tid = process->tid;
         process->tid = ti_tid;   /* for readgprs  sp  */
         readgprs(process,&process->reg[0]);
         thread_sp =  reg(GPR1);
         /* restore special registers of the current thread */
         process->tid = save_tid;
         readgprs(process,&process->reg[0]);
      }

    }
   control_value(ti_policy_k, ERROR_POLICY);
   control_value(ti_policy_u, ERROR_POLICY);


   /* Output the info found... */
   (*rpt_output)(stdout, "      scheduler:\n");
   (*rpt_output)(stdout, "         kernel       = %d (%s)\n",
                                          ti_pri_k,t_policy[ti_policy_k]);
   (*rpt_output)(stdout, "         user         = %d (%s)\n",
                                          ti_pri_u,t_policy[ti_policy_u]);
   (*rpt_output)(stdout, "      general:\n");
   (*rpt_output)(stdout, "         thread errno = %d\n",thd_errno);
   (*rpt_output)(stdout, "         start pc     = 0x%x\n", (Address) start_pc);
   (*rpt_output)(stdout, "         detached     = %s\n",
                                          (Boolean) attached?"no":"yes");
   (*rpt_output)(stdout, "      event :\n");
   (*rpt_output)(stdout, "         event        = 0x%x\n", event);
/*  M:N : thread state instead of thread addr */
/* (*rpt_output)(stdout, "      tstate (thread state):\n"); */
   (*rpt_output)(stdout, "      thread (pthread addr):\n");
   (*rpt_output)(stdout, "         address      = 0x%x", pthread_addr);
   (*rpt_output)(stdout, "         size         = 0x%x\n",
                                           sizeof(pthread_st));
   (*rpt_output)(stdout, "      stack storage:\n");
   (*rpt_output)(stdout, "         base         = 0x%x",(Address)stack_base);
   (*rpt_output)(stdout, "         size         = 0x%x\n",stack_size);
   (*rpt_output)(stdout, "         limit        = 0x%x\n",(Address)stack_limit);
   (*rpt_output)(stdout, "         sp           = 0x%x\n",
                                         (Address) thread_sp);
}

/*
 * NAME: thread_k_cont
 *
 * FUNCTION: continue all  threads not held
 *
 * NOTE: called by pcont() 
 *       in case of k_thread we have to use ptrace(PTT_CONTINUE) with
 *       a list of kernel threads to resume
 *       if the running_thread is held a special subroutine is called
 *       to hold this thread
 *
 * PARAMETERS: p = current process
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: NONE
 *
 * RETURNS: NONE
 */

public void thread_k_cont(p)
Process p;
{
   pthread_queue *ptr;               /* current pointer on pthread structure */
   pthread_queue q_buf;              /* pthread_queue (link) read            */
   Address known_thread_addr;        /* hold address of __dbx_known_threads */
   unsigned long hold;               /* field of pthread held or unheld     */
   Symbol known_threads;             /* Symbol for thread object list       */
   thrdsinfo_st *pth;
   pthread_queue *ptrcur;
   Boolean held_running = 0;
   int i,n,i_buf = 0;                /* for loop */
   struct ptthreads buf_ptthreads;    /* buffer for ptrace(PTT_CONTINUE) */
   unsigned int state;
   int badread;
   int nb_thread =0;                 /* number of threads to resume     */

   /* Find address for thread queue start - __dbx_known_pthreads */
   /* if the process is not linked with libpthreads.a             */
   known_threads = lookup(identname("__dbx_known_pthreads",true));
   if (known_threads) {
      eval(amper(build(O_SYM, known_threads)));
      known_thread_addr = pop(Address);
      badread = dread(&q_buf, known_thread_addr , sizeof(pthread_queue));
      if (badread) return;
      ptr=QUEUE_NEXT(q_buf);
   }


   if ((known_threads == nil) or (ptr == nil) or (hold_other)) {
      nb_thread = 1;
      buf_ptthreads.th[0] = NULL;
   } else {
      for ( i = 0;i < nb_k_threads;i++) {
          pth = &pthrdsinfo[i];
          /* search in user area a thread with the same tid */
          ptrcur  = search_pthread(known_thread_addr ,pth->ti_tid);
	  state = pth->ti_state;
          if (!(state == TSIDL || state == TSZOMB || state == TSSWAP
               || state == TSNONE)) {
             if (ptrcur != nil) {
                badread = dread(&hold, (Address) ((int) ptrcur + 
                          offsetof2(pthread_st, ti_hold)), sizeof(unsigned long ));
             }
             else hold = 0;
             if ( !hold) { /* not held */
                 if( pth->ti_tid != tid_running_thread )
                     buf_ptthreads.th[i_buf++] = pth->ti_tid;
                     nb_thread++;
                 } else 
                  /* test if the running_thread is held */
/* hold running_thread is looping, so unhold it */
                     if( pth->ti_tid == tid_running_thread ) {
                        held_running = 1;
                        hold = 0;
                        badread = dwrite(&hold, (Address) ((int) ptrcur +
                        offsetof2(pthread_st, ti_hold)), sizeof(unsigned long ));
                      }
         } /* kernel state */
         buf_ptthreads.th[i_buf] = NULL;
       }
   }
   /* all the threads are held */
   /* if we have only one thread : case of child after a fork() */
   /* or case of debug the libpthreads -> the pthread structure */
   /* is not yet created  : do not display  an error            */
   if (( nb_thread == 0 ) && (nb_k_threads > 1)){
      if (save_last_bp != 0)
         unsetbp(save_last_bp);
      else
         unsetallbps();
      isstopped = true;     /* we can continue after unhold threads */
      k_error( catgets(scmc_catd, MS_pthread, MSG_769,
                                                "all threads held"));
      return;
   }


   if (held_running) { 
      if (traceexec) {
            (*rpt_output)(stdout, "call threadheld\n");
      }
      threadheld();
      p->signo = 0;
   }
   if (traceexec) {
         (*rpt_output)(stdout, "!! ptrace(%d,%d,%d,%d,%x)\n",
             PTT_CONTINUE, tid_running_thread, 1, p->signo, &buf_ptthreads);
         (*rpt_output)(stdout, "!! number of threads resumed %d\n",nb_thread);
   }

   if (ptrace(PTT_CONTINUE, tid_running_thread,1,p->signo,&buf_ptthreads) < 0){
      panic( catgets(scmc_catd, MS_process, MSG_285,
                     "error %d trying to continue process"), errno);
   }
   return;

}

/*
 * NAME: thread_k
 *
 * FUNCTION: lists all existing pthreads threads
 *
 * NOTE: See diagram (top of file) for method used in transversing thread queue
 *       Part of "thread current" (with id) is handled in eval() and routine
 *       switchThread_k.
 *
 * PARAMETERS: 
 *      option  - option issued with the thread subcommand
 *      p       - Node contain list of thread id's to act on
 * 
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: NONE
 *
 * RETURNS: NONE
 */
public void thread_k(option, p)
thread_op option;
Node p;
{
   Address known_thread_addr;	   /* hold address of __dbx_known_pthreads*/
   pthread_queue q_buf;     	   /* hold queue portion of cur thread    */
   pthread_queue *ptr;      	   /* pointer to queue buffer             */
   int thread_id;		   /* id of thread                        */
   unsigned long t_id;	           /* thread id of thread                 */
   unsigned long state;		   /* state of thread                     */
   unsigned long state_u;	   /* state of thread                     */
   unsigned long cursig;	   /* cursig of thread                    */
   int is_held;			   /* Using int here to held holding thread*/
   Symbol known_threads;	   /* Symbol for thread object list       */
   Symbol thisthread;		   /* Symbol for this thread              */
   Boolean print_title = true;	   /* set if we want the title displayed  */
   Boolean list_this;		   /* set if this thread is selected      */
   Boolean fullinfo = false;       /* set if we have "thread info"        */
   int badread;			   /* set if we can dread failed          */
   Boolean thread_found = false;   /* set if we find specified thread     */
   vp_st *vp_addr;                 /* address of vp                       */

   /* Find address for thread queue start - __dbx_known_pthreads */ 
   known_threads = lookup(identname("__dbx_known_pthreads",true));
   if (known_threads == nil) {
	/* complain if we are setting non-existing threads... */
	if (option == th_hold || 
	    option == th_unhold ||
	    (option == th_current && p))
        k_error( catgets(scmc_catd, MS_pthread, MSG_767, 
						"no thread available"));
        return;
   } else if (!isactive(known_threads->block)) {
        /* don't complain if we are just getting thread info... */
	if (option != th_get_info)
           error(catgets(scmc_catd,MS_runtime, MSG_265, 
						"program is not active")); 
	return;
   } else {
	eval(amper(build(O_SYM, known_threads)));
	known_thread_addr = pop(Address);

	/* Fill in content of the first queue section */
	badread = dread(&q_buf, known_thread_addr, sizeof(pthread_queue));
	/* escape out if cannot read known thread queue */
	if (badread) {
	   /* warning message already displayed by dread */
	   return;
	}
	ptr = QUEUE_NEXT(q_buf);

        /* reset running and current thread if nothing in queue */
        if ( ptr == known_thread_addr) {
            running_thread = current_thread = nil;
        }
        /* update the kernel informations */
        if ( option == th_get_info && ptr != nil) 
            update_threads(known_thread_addr);

        verifArgObject_k(known_thread_addr,OBJ_THREAD,p);
	while ( ptr != nil  && ptr != known_thread_addr) {  
				     /* Does it point at active_pthreads */

	    /* Find out what thread id we are dealing with... */
	    badread = dread(&thread_id, (Address) ( (int)ptr + 
		            offsetof2(pthread_st, th_id)), sizeof(unsigned));
            if (badread) return;

            /* check and see if it is requested by number */
	    /* if no arg (p == nil), list all...          */
	    list_this = (!p || isRequested_k(p,thread_id));

	    /* find any thread yet? */
	    thread_found = thread_found || list_this;
	    /* get state  of the thread... */
	    badread= dread(&state, (Address) ((int)ptr+
                          offsetof2(pthread_st,ti_stat)),sizeof(unsigned long));
            if (badread) return;
	    badread = dread(&state_u, (Address) ((int)ptr+
                      offsetof2(pthread_st,state)), sizeof(unsigned long));
            if (badread) return;

            /* selected thread and state = actived */
	    /* eliminate the pthread structure with state  PTHREAD_RETURNED */
            /* this structure is no  more attached to a pthread             */
            if ( state_u & PTHREAD_RETURNED) list_this = false;
	    if (list_this ) {
	       badread = dread(&cursig, (Address) ((int)ptr+offsetof2
                        (pthread_st,ti_cursig)),sizeof(unsigned long));
               if (badread) return;
	       /* find dbx symbol for this thread... */
               thisthread = update_pthreads_obj("$t", thread_id, ptr, 
							thread_object.sym);
               /* if coredump select all threads */
               if (coredump) state = TSRUN ;

	       /* handle different options... */
	       if ((option == th_run   && !IS_RUNNING(state)) ||
	           (option == th_susp  && !IS_SUSP(state)) ||
	           (option == th_term  && !IS_TERM(state)) ||
	           (option == th_wait  && !IS_WAIT(state)) ){
		  /* check and see if it's in the state requested */
	          list_this = false;
               } else if (option == th_get_info) {
                 /* update running_thread and current thread in case of core */
                 if(coredump) {
                     badread = dread(&vp_addr, (Address) ((int) ptr +
                          offsetof2(pthread_st, vp)), sizeof(Address ));
                     if (badread) return;
                     if (vp_addr != nil ){
                          badread = dread(&t_id, (Address) ((int) vp_addr +
                                  offsetof(vp_st,id )), sizeof(unsigned long ));
                          if (badread) return;
                     }
                     /* the running thread is the thread responsible of
                        core dump : the mst of this thread is in the header */
                     if (find_thread(t_id) == &corehdr.c_mst) {
                         running_thread = thisthread;
			 process->tid = t_id;
                         current_thread = running_thread;
                     }
                 }
	          /* updating thread info quickly */
                  if (IS_RESPONSIBLE(cursig)) {
                     /* create or update $running_thread symbol */
                     update_running_thread( (Address) ( (int)ptr +
                                 offsetof2(pthread_st, th_id)));
                     running_thread = thisthread;
                     current_thread = running_thread;
                  }
                  list_this = false;
	       } else if (option == th_current) {
		  if (p) {
		     /* argument is provided to change current thread */
		     switchThread_k(thisthread);
		     list_this = false;
		  } else if (current_thread != thisthread)
		     /* no argument, just list the current thread */
		     list_this = false;
	       } else if (option == th_info) {
		  fullinfo = true;
	       } else if (option == th_hold || 
			  option == th_unhold) {
		  /* set the field to hold or unhold threads... */
		     /* cannot hold or unhold null thread */
	             is_held = (option == th_hold);
                     dwrite(&is_held, (Address) ((int)ptr +
				       offsetof2(pthread_st,ti_hold)), 
		  		       sizeof(int));
	       } else if (option == th_hold_other || 
			  option == th_unhold_other) {
 		   /* If $hold_next is set...				     */
                   /* Suspend non-running thread, used when dbx does single- */
		   /* steps.                                                 */
                     if (option == th_hold_other)
                         hold_other = true;
                      else
                         hold_other = false;

	               list_this = false;
               }
	    }
	
	    /* If we still want it, display it... */
	    if (list_this) {
		display_thread_k(ptr, thread_id, state, &print_title, 
						fullinfo, thisthread);
	       if (fullinfo) {
		 thread_info_k(ptr);
	       }
	    } /* display thread */

	    /* Get contents of next queue section */
	    badread = dread(&q_buf, ptr, sizeof(pthread_queue));
            if (badread) return;
	    ptr = QUEUE_NEXT(q_buf);		/* advances forward link */
	}
	   
     }
}
/*
 * NAME: attribute_k
 *
 * FUNCTION: Lists all existing pthreads attribute objects
 *
 * NOTE: See diagram (top of file) for method used in transversing thread queue
 *
 * PARAMETERS:
 *      option  - option issued with the attribute subcommand
 *      p       - Node contain list of attribute id's to act on
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: NONE
 *
 * RETURNS: NONE
 */
public void attribute_k(option, p)
attribute_op option;
Node p;
{
   Address acb_addr;                    /* address of attr control block   */
   Address known_attr_addr;             /*address of __dbx_known_attributes */
   pthread_queue q_buf;                 /* hold queue portion of attribute */
   pthread_queue *ptr;                  /* pointer to queue buffer         */
   int type;                            /* attribute type                  */
   int state;                           /* attribute state                 */
   int attr_id; 	                /* attribute id                    */
   int priority;                        /* priority of new thread          */
   int policy;			        /* sched policy of thread          */
   int detachstate;		        /* sched policy of thread          */
   int processshared;		        /* sched policy of thread          */
   int prio_ceil;		        /* sched policy of thread          */
   int inherit_sched;                   /* is scheduling inherited?        */
   int scheduler;                       /* schesuling policy               */
   int scope;                           /*  contentionscope:global or local*/
   int protocol;                        /*  for mutex                      */
   unsigned int stack_size;             /* size of stack (bytes)           */
   Symbol known_attr = lookup(identname("__dbx_known_attributes",true));
   Boolean print_title = true;
   Boolean list_this;
   int badread;

   if (known_attr == nil) {
        return;
   } else if (!isactive(known_attr->block)) {
        /* don't complain if we are just getting thread info... */
        if (option != th_get_info)
           error(catgets(scmc_catd,MS_runtime, MSG_265,
                                               "program is not active"));
        return;
   } else {
        eval(amper(build(O_SYM, known_attr)));
        known_attr_addr = pop(Address);

        badread = dread(&q_buf, known_attr_addr, sizeof(pthread_queue));
        /* escape out if cannot read known attribute queue */
        if (badread) {
           /* warning message already displayed by dread */
           return;
        }
        ptr = QUEUE_NEXT(q_buf);
        verifArgObject_k(known_attr_addr,OBJ_ATTRIBUTE,p);
        while ( ptr != nil && ptr != known_attr_addr) {
            /* get addr of attr object */
            acb_addr = (Address) ptr;

            /* get id for this attribute object */
            badread = dread(&attr_id, (Address) ((int)acb_addr +
                      offsetof(pthread_attr_st, attr_id)), sizeof(int));
            if (badread) return;

            /* check and see if we are asked to display this... */
            list_this = (!p || isRequested_k(p,attr_id));
            if (option == th_get_info) list_this = false;
            /* if so, read other info needed for display... */
            if (list_this) {
                badread = dread(&type, (Address) ((int)acb_addr +
                      offsetof(pthread_attr_st, type)), sizeof(int));
                if (badread) return;
                badread = dread(&priority, (Address) ((int)acb_addr +
                          offsetof(pthread_attr_st, schedule.sched_priority)),
                               sizeof(int));
                if (badread) return;
                badread = dread(&state, (Address) ((int)acb_addr +
                      offsetof(pthread_attr_st, flags)), sizeof(int));
                if (badread) return;
                badread = dread(&scope, (Address) ((int)acb_addr +
                      offsetof(pthread_attr_st, contentionscope)), sizeof(int));
                if (badread) return;
                badread = dread(&protocol, (Address) ((int)acb_addr +
                      offsetof(pthread_attr_st, protocol)), sizeof(int));
                if (badread) return;
                badread = dread(&scheduler, (Address) ((int)acb_addr +
                      offsetof(pthread_attr_st, schedule.sched_policy)),
                               sizeof(int));
                if (badread) return;
                badread = dread(&prio_ceil, (Address) ((int)acb_addr +
                      offsetof(pthread_attr_st, prio_ceiling)), sizeof(int));
                if (badread) return;
                badread = dread(&stack_size, (Address) ((int)acb_addr +
                      offsetof(pthread_attr_st, stacksize)), sizeof(int));
                if (badread) return;
                badread = dread(&detachstate, (Address) ((int)acb_addr +
                      offsetof(pthread_attr_st, detachstate)), sizeof(int));
                if (badread) return;
                badread = dread(&processshared, (Address) ((int)acb_addr +
                      offsetof(pthread_attr_st, process_shared)), sizeof(int));
                if (badread) return;

                /* prepare the output */
                if (type == ATTR_THREAD) { /* thread attribute */
                   processshared = NO_P_SHARED;
                   protocol = NO_PROTOCOL;
                } else { /* mutex or condition attribute */
                   scope = NO_SCOPE;
                   priority = 0;
                   stack_size = 0;
                   scheduler = NO_SCHEDULER;
                   if (type == ATTR_COND) protocol = NO_PROTOCOL;
                }
		/* control fields */
		control_value(type, ERROR_TYPE);
		control_value(scope, ERROR_SCOPE);
		control_value(scheduler, ERROR_SCHEDULER);
		control_value(processshared, ERROR_P_SHARED);
		control_value(protocol, ERROR_PROTOCOL);
		control_value(state, ERROR_STATE);

                /* display title is needed... */
                if (print_title) {
                        (*rpt_output)(stdout,
" attr     obj_addr   type  state  stack   scope    prio sched p-shar protocol \n");
                        print_title = false;
                }

                /* Output attr object info... */
                (*rpt_output)(stdout,
                " $a%-4d  0x%-8x  %-5.5s %-5.5s ",
                          (int)attr_id, acb_addr, attr_type[type],
                           attr_state[state]);
                if (type == 0) {
                   (*rpt_output)(stdout,"%6d    %-3.3s   %6d %-5.5s",
                   (int)stack_size,
                           attr_contention[scope],
                           priority,
                           attr_sched[scheduler]);
                }
                else {
                   (*rpt_output)(stdout,"                              ");
                   (*rpt_output)(stdout," %-3.3s",
                           attr_shared[processshared]);
                   if (type == 1) {
                      (*rpt_output)(stdout,"  %-8.8s",
                       attr_protocol[protocol]);
                   }
                }
                 
                (*rpt_output)(stdout, "\n");

            }

            /* create attribute object and insert into dbx symbol table */
            update_pthreads_obj("$a", attr_id, acb_addr, attr_object.sym);

            /* Get contents of next queue section */
            badread = dread(&q_buf, ptr, sizeof(pthread_queue));
            if (badread) return;
            ptr = QUEUE_NEXT(q_buf);            /* advances forward link */
        }
    }
}



/*
 * NAME: condition_k
 *
 * FUNCTION: Lists all existing pthreads condition variable
 *
 * NOTE: See diagram (top of file) for method used in transversing thread queue
 *
 * PARAMETERS:
 *      option  - option issued with the condition subcommand
 *      p       - Node contain list of condition variable id's to act on
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: NONE
 *
 * RETURNS: NONE
 */
public void condition_k(option, p)
condition_op option;
Node p;
{
   Address ccb_addr;                    /* address of cv control block     */
   Address known_attr_addr;             /* address of  __dbx_known_conditions*/
   pthread_queue q_buf;                 /* hold queue portion of attribute */
   pthread_queue *ptr;                  /* pointer to queue buffer         */
   int cv_id;        	                /* id of condition variable        */
   __ptq_queue semaphore_queue;         /* semaphore queue for cond. wait  */
   int num_wait;                        /* number of waiter cv has         */
   Symbol known_attr = lookup(identname("__dbx_known_conditions",true));
   Boolean print_title = true;
   Boolean list_this;
   int badread;
   Address p_cond;                      /* pointer on condition */

   if (known_attr == nil) {
        return;
   } else if (!isactive(known_attr->block)) {
        /* don't complain if we are just getting thread info... */
        if (option != th_get_info)
           error(catgets(scmc_catd,MS_runtime, MSG_265, 
                                               "program is not active"));
        return;
   } else {
        eval(amper(build(O_SYM, known_attr)));
        known_attr_addr = pop(Address);

        /* Fill in content of the first queue section */
        known_attr_addr = known_attr_addr +
                                        offsetof(pthread_cond_t, link),
        badread = dread(&q_buf, known_attr_addr, sizeof(pthread_queue));
        /* escape out if cannot read known cvs queue */
        if (badread) {
           /* warning message already displayed by dread */
           return;
        }
        ptr = QUEUE_NEXT(q_buf);
        verifArgObject_k(known_attr_addr,OBJ_CONDITION,p);
        while ( ptr != nil &&  ptr != known_attr_addr) {
            num_wait = 0;               /* reset number of waiters */
            p_cond = (Address) ptr;   /* get addr of cv object   */
	    /* the list of condition contains the pointer */
            badread = dread(&ccb_addr, (Address) ((int)p_cond +
                      offsetof(__dbx_cond, pt_cond)), sizeof(int));
            if (badread) return;

            /* get id and semaphore queue for this condition variable */
            badread = dread(&cv_id, (Address) ((int)ccb_addr +
                      offsetof(pthread_cond_t, cv_id)), sizeof(int));
            if (badread) return;
            
            badread = dread(&semaphore_queue, (Address) ((int)ccb_addr +
                      offsetof(pthread_cond_t, waiters)), sizeof(__ptq_queue));
            if (badread) return;

            /* check and see if we are asked to display this... */
            list_this = (!p || isRequested_k(p,cv_id));
            if (option == th_get_info) list_this = false;
            /* check if only wait or nowait conditions are requested... */
            if (list_this) {
                /* find out how many waiters... */
                   num_wait = getWaiters_k((Address)((int)ccb_addr +
                              offsetof(pthread_cond_t, waiters)),false);
                if ( ((option == cv_wait) && !num_wait) ||
                     ((option == cv_nowait) && num_wait) )
                   list_this = false;
            }

            /* if we still want this cv, display it... */
            if (list_this) {
                if (print_title) {
                    (*rpt_output)(stdout,
              " cv      obj_addr     num_wait  waiters\n");
                    print_title = false;
                }
                /* Output cv object info... */
                (*rpt_output)(stdout, " $c%-4d  0x%-8x  %8d  ",
                                (int)cv_id, ccb_addr, num_wait);
                /* call getWaiters_k() to print names of waiters... */
                if (num_wait > 0)
                        getWaiters_k((Address)((int)ccb_addr +
                                offsetof(pthread_cond_t, waiters)),true);
                (*rpt_output)(stdout, "\n");
            } /* display cv info */

            /* create attribute object and insert into dbx symbol table */
            update_pthreads_obj("$c", cv_id, ccb_addr, cv_object.sym);

            /* Get contents of next queue section */
            badread = dread(&q_buf, ptr, sizeof(pthread_queue));
            if (badread) return;
            ptr = QUEUE_NEXT(q_buf);            /* advances forward link */
        }
    }
}


/*
 * NAME: mutex_k
 *
 * FUNCTION: Lists all existing  mutex objects
 *
 * NOTE: See diagram (top of file) for method used in transversing thread queue
 *
 * PARAMETERS:
 *      option  - option issued with the mutex subcommand
 *      p       - Node contain list of mutex id's to act on
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: NONE
 *
 * RETURNS: NONE
 */
public void mutex_k(option, p)
mutex_op option;
Node p;
{
   Address mcb_addr;                    /* address of mutex control block */
   Address known_mutex_addr;            /* address of dbx__known_mutexes  */
   pthread_queue q_buf;                 /* hold queue portion of mutex    */
   pthread_queue *ptr;                  /* pointer to queue buffer        */
   int mutex_id;	                /* id of mutex                    */
   pthread_t owner_thread;              /* address of mutex owner thread  */
   int owner_id;                        /* id of mutex owner              */
   int kind;                            /* kind of attr of mutex          */
   __ptlock_type islock;                /* set if mutex is locked         */
   Symbol known_mutexes = lookup(identname("__dbx_known_mutexes",true));
   Boolean print_title = true;
   Boolean list_this;
   int badread;
   Address p_mutex;                     /* pointer on __dbx_mutex */


   if (known_mutexes == nil) {
        return;
   } else if (!isactive(known_mutexes->block)) {
        /* don't complain if we are just getting thread info... */
        if (option != th_get_info)
           error(catgets(scmc_catd,MS_runtime, MSG_265,
                                               "program is not active"));
        return;
   } else {
        eval(amper(build(O_SYM, known_mutexes)));
        known_mutex_addr = pop(Address);

        /* Fill in content of the first queue section */
        badread = dread(&q_buf, known_mutex_addr, sizeof(__ptq_queue));
        /* escape out if cannot read known mutex queue */
        if (badread) {
           /* warning message already displayed by dread */
           return;
        }
        ptr = QUEUE_NEXT(q_buf);
        verifArgObject_k(known_mutex_addr,OBJ_MUTEX,p);
        while ( ptr != nil && ptr != known_mutex_addr) {
            owner_id = 0;               /* reset mutex owner id   */
            p_mutex = (Address) ptr;   /* get addr of mutex object */
            badread = dread(&mcb_addr, (Address) ((int)p_mutex +
                      offsetof(__dbx_mutex, pt_mutex)), sizeof(int));
            if (badread) return;

            /* get mutex id for this mutex object */
            badread = dread(&mutex_id, (Address) ((int)mcb_addr +
                      offsetof(pthread_mutex_t, mtx_id)), sizeof(int));
            if (badread) return;

            /* check and see if we are asked to display this... */
            list_this = (!p || isRequested_k(p,mutex_id));
            if (option == th_get_info) list_this = false;
            /* check if only locked or unlocked mutexes are requested... */
            if (list_this) {
               /* read mutex info islock, and owner... */
               badread = dread(&islock, (Address) ((int)mcb_addr +
                         offsetof(pthread_mutex_t, lock)), sizeof(int));
               if (badread) return;
               badread = dread(&kind, (Address) ((int)mcb_addr +
                         offsetof(pthread_mutex_t, mtx_kind)), sizeof(int));
               if (badread) return;
               badread = dread(&owner_thread, (Address) ((int)mcb_addr +
                         offsetof(pthread_mutex_t, owner)), sizeof(pthread_t));
               if (badread) return;
               badread = dread(&owner_id, (Address) ( (int)owner_thread +
                         offsetof(pthread_st, th_id)), sizeof(unsigned));
               if (badread) return;

               if ( ((option == mu_lock) && !islock) ||
                    ((option == mu_unlock) && islock) ) {
                   list_this = false;
               }

	  }
          control_value(kind, ERROR_KIND);

            /* if we still want this mutex, display it... */
            if (list_this) {
                if (print_title) {
                    (*rpt_output)(stdout,
      " mutex    obj_addr  type    lock  owner  \n");
                    print_title = false;
                }
                /* Output mutex object info... */
                (*rpt_output)(stdout, " $m%-3d  0x%8x %-7s %4s  ",
                              (int)mutex_id,
                              mcb_addr,
			      attr_kind[kind],
                              (int)islock?"yes":"no");
                if (islock && (owner_thread != nil))
                    (*rpt_output)(stdout, "$t%-3d  ", (int)owner_id);
                else
                    (*rpt_output)(stdout, "%7s", "");
                (*rpt_output)(stdout, "\n");
            } /* display mutex info */

            /* create mutex object and insert into dbx symbol table */
            update_pthreads_obj("$m", mutex_id, mcb_addr, mutex_object.sym);

            /* Get contents of next queue section */
            badread = dread(&q_buf, ptr, sizeof(pthread_queue));
            if (badread) return;
            ptr = QUEUE_NEXT(q_buf);            /* advances forward link */
        }
    }
}

extern tid_t tid_func_thread;
extern Frame callframe;
extern  Word caller_reg[NGREG+NSYS+1];
extern double caller_freg[MAXSAVEFREG];
extern Boolean call_command;
extern boolean notderefed;
extern boolean specificptrtomember;

/*
 * NAME: threadheld
 *
 * FUNCTION: Allows the holding of the running_thread
 *           The running_thread will execute the subroutine __funcbloc_np
 *            
 *           Modify the pc register
 *
 * NOTE: It's the same sequence (approximatly) that callproc()
 *       Used by thread_k_cont() before calling ptrace
 *
 * PARAMETERS: NONE
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: NONE
 *
 * RETURNS: NONE
 */

private void threadheld ()
{
    Symbol proc = nil;
    integer argc, i;
    static struct Frame frame;
    Name n;
    Address funcAddr;
    Address orig_pc;

    funcAddr = nil;
    orig_pc = reg(SYSREGNO(IAR));

    n = identname("__funcblock_np",true);

    find(proc, n) where isroutine(proc) endfind(proc);
    if (proc == nil) {
        error( catgets(scmc_catd, MS_execute, MSG_162,
                     "Failed establishing calling point (libg.a not linked)"));
    }
    /* Save off the current frame before we start the call */
    callframe = &frame;
    getcurframe(callframe);
    pushenv();

    /* Also save off all registers before the call */
    for (i = 0; i < NGREG + NSYS + 1; i++) {
	caller_reg[i] = reg(i);
    }
    for (i = 0; i < fpregs; i++) {
	caller_freg[i] = fpregval(i);
    }
    if (funcAddr) {
       pc = funcAddr;
    }
    else {
       pc = codeloc(proc);
    }

    /*argc = pushargs(proc, arglist, thisptr);*/

    orig_pc = reg(SYSREGNO(IAR));
    tid_func_thread = tid_running_thread;
    call_command = true;
    /* to indicate __funcblock_np called */
    addr_dbsubn = codeloc(proc);
    beginproc(proc,  0, funcAddr);

/* build a special command run after return function : eval calls procreturn */
/* cont - bpact - evalcmdlist - eval - procreturn - flushoutput - beginproc -*/
/* (fflush) stepto ..                                                        */
/* fflush is executed */


    event_once(
        build(O_EQ, build(O_SYM, pcsym), build(O_SYM, retaddrsym)),
        buildcmdlist(build(O_PROCRTN, proc))
    );
    n = identname("__dbsubn",true);

    find(proc, n) where isroutine(proc) endfind(proc);
    if (proc == nil) {
        error( catgets(scmc_catd, MS_execute, MSG_162,
                     "Failed establishing calling point (libg.a not linked)"));
    }

    setbp(codeloc(proc));
    isstopped = false;
    if (not bpact()) {
        notderefed = true;
        specificptrtomember = false;
        isstopped = true;
    }
    /*  indicate to the function bpact() that the running_thread was held */
    /*  the next break-point will call procreturn() to restore its registers*/
    addr_dbsubn = codeloc(proc);
}

/*
 * NAME: check_thread_stacks
 *
 * FUNCTION: Determines whether a passed address is in one of the
 *           stacks for an active thread.
 *           If it is, then returns the start and end of the stack.
 *
 * RECOVERY OPERATION: none
 *
 * PARAMETERS:
 *      address  - address to be checked (input)
 *      start    - if the passed address is in a stack, then the
 *                 start of the stack is returned in this
 *                 parameter.
 *                 Otherwise, 0 is returned in this parameter.
 *                 (output)
 *      end      - if the passed address is in a stack, then the
 *                 end of the stack is returned in this
 *                 parameter.
 *                 Otherwise, 0 is returned in this parameter.
 *                 (output)
 *
 * DATA STRUCTURES: none
 *
 * RETURNS: void
 *      start and end parameters are returned as defined above.
 *
 */
public void check_thread_stacks(uint address,
                                uint *start,
                                uint *end)
{
   Address known_thread_addr;      /* hold address of __dbx_known_pthreads */
   pthread_queue q_buf;            /* hold queue portion of current thread */
   pthread_queue *ptr;             /* pointer to queue buffer              */
   unsigned long t_id;             /* thread id of thread                  */
   unsigned long state;            /* state of thread                      */
   unsigned long state_u;          /* state of thread                      */
   Symbol known_threads;           /* Symbol for thread object list        */
   vp_st *vp_addr;                 /* address of vp                        */
   int i;
   Address stack_limit;            /* address for top of stack        */
   int badread;
   tid_t ti_tid;
   tid_t save_tid;
   Address stack_pointer;          /* stack pointer for the thread    */

   *start = *end = 0;
   /* Find address for thread queue start - __dbx_known_pthreads */
   known_threads = lookup(identname("__dbx_known_pthreads",true));
   if ((known_threads != nil) && (isactive(known_threads->block)))
   {
      eval(amper(build(O_SYM, known_threads)));
      known_thread_addr = pop(Address);

      /* Fill in content of the first queue section */
      badread = dread(&q_buf, known_thread_addr, sizeof(pthread_queue));
      /* escape out if cannot read known thread queue */
      if (badread) {
         /* warning message already displayed by dread */
         return;
      }
      ptr = QUEUE_NEXT(q_buf);
      while ((ptr != nil)  &&
             (ptr != known_thread_addr) &&
             (*start == 0) &&
             (*end == 0))
      {

          badread = dread(&ti_tid, (Address) ((int) ptr +
                          offsetof2(pthread_st, ti_tid)), sizeof(int));
          if (badread) return;

          /* get state  of the thread... */
          badread= dread(&state, (Address) ((int)ptr+
                        offsetof2(pthread_st,ti_stat)),sizeof(unsigned long));
          if (badread) return;
          badread = dread(&state_u, (Address) ((int)ptr+
                    offsetof2(pthread_st,state)), sizeof(unsigned long));
          if (badread) return;

          /*
             skip a thread with state = PTHREAD_RETURNED
             since it is longer attached to a pthread
          */
          if ( !(state_u & PTHREAD_RETURNED))
          {
               badread = dread(&vp_addr, (Address) ((int) ptr +
                     offsetof2(pthread_st, vp)), sizeof(Address ));
               if (badread) return;

               badread = dread(&stack_limit, (Address) ((int) vp_addr +
                     offsetof(vp_st, stack.limit)), sizeof(unsigned long ));
               if (badread) return;

               /*
                  Read general registers for thread in order
                  to get value of stack pointer
               */
               save_tid = process->tid;
               process->tid = ti_tid;   /* for readgprs  sp  */
               readgprs(process,&process->reg[0]);
               stack_pointer =  reg(GPR1);

               /* restore registers of the current thread */
               process->tid = save_tid;
               readgprs(process,&process->reg[0]);

               /*
                  check if address is within the bounds of the
                  stack for the thread being checked
               */
               if ((address <= stack_limit && address >= stack_pointer)                         )
               {
                   *start = stack_pointer;
                   *end = stack_limit;
               }
          }

          /* Get contents of next queue section */
          badread = dread(&q_buf, ptr, sizeof(pthread_queue));
          if (badread) return;

          /* move pointer to next element in thread queue */
          ptr = QUEUE_NEXT(q_buf);
      }

   }
}

#else
#include "defs.h"
#include "symbols.h"
getMutexOpt_k(){}
getConditionOpt_k() {}
getAttributeOpt_k() {}
getThreadOpt_k() {}
thread_k() {}
condition_k() {}
mutex_k() {}
attribute_k() {}
switchThread_k() {}
public boolean isThreadObjectSymbol_k(ThreadObjectSym)
Symbol ThreadObjectSym;
{
   return(false);
}
#endif /* K_THREADS */
