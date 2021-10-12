/* @(#)49	1.4  src/bos/usr/ccs/lib/libdbx/k_thread.h, libdbx, bos411, 9428A410j 3/16/94 07:58:17 */
#ifndef _h_k_thread
#define _h_k_thread
/*
 * COMPONENT_NAME: (CMDDBX) - dbx symbolic debugger
 *
 * FUNCTIONS: 
 *
 * ORIGINS:  83
 *
 * LEVEL 1, 5 Years Bull Confidential Information
 *
 */


typedef struct pthread pthread_st;
typedef struct stk stk_st;
typedef struct vp vp_st;
typedef struct pthread_attr  pthread_attr_st;
typedef struct thrdsinfo  thrdsinfo_st;
typedef struct mstsave  mstsave_st;
typedef struct procsinfo  procsinfo_st;

typedef struct uthread uthread_st;
extern void thread_k();                       
extern void update_pthreads_types();           
extern int lib_type;
#define  C_THREAD 1 /* program link-edited to Libcma */
#define  KERNEL_THREAD 2   /* program link-edited to Libpthreads */
#define  NO_THREAD 0  /* no threads used */

#define threads(x,y) \
  if(lib_type == KERNEL_THREAD) thread_k(x,y); \
  else threads(x,y);
#define attribute(x,y) \
  if(lib_type == KERNEL_THREAD) attribute_k(x,y); \
  else attribute(x,y);
#define condition(x,y)\
   if(lib_type == KERNEL_THREAD) condition_k(x,y); \
   else condition(x,y);
#define mutex(x,y) \
   if(lib_type == KERNEL_THREAD) mutex_k(x,y); \
   else mutex(x,y);
#define switchThread(x) \
   if(lib_type == KERNEL_THREAD) switchThread_k(x); \
   else switchThread(x);
extern Symbol running_thread, current_thread;
extern boolean isThreadObjectSymbol_k(Symbol); /* checks if symbol is         */
                                               /* thread, mutex, attribute    */
                                               /* or condition variable object*/
extern void    check_thread_stacks(uint,uint*,uint*); /* determines if 
                                                         address is within
                                                         the bounds of one of
                                                         the thread stacks 
                                                      */ 
      

#endif /* _h_k_thread */

