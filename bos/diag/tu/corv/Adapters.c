static char sccsid[] = "@(#)21  1.4  src/bos/diag/tu/corv/Adapters.c, tu_corv, bos411, 9428A410j 9/1/93 20:19:52";
/*
 *   COMPONENT_NAME: TU_CORV
 *
 *   FUNCTIONS: adapter_close
 *              adapter_open
 *              catchsigs
 *              display_adapter_data
 *              dma_allocate
 *              dma_flush
 *              dma_free
 *              get_card_id
 *              get_environment
 *              initialize_adapter_info
 *              post_error
 *              print_error
 *              print_trace
 *              record_testcase
 *              setsig
 *              signal_close
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 89,91
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <sys/errno.h>
#include <sys/sysconfig.h>
#include <sys/param.h>
#include <signal.h>
#include <time.h>

#ifdef DIAGEX10
#include "diagex.h"
#elif DIAGEX20
#include "diagex20.h"
#elif DIAGNOSTICS
#include <sys/diagex.h>
#define DIAGEX20 1
#endif

#include "libadapters.h"

#ifdef OHIO
#include "getohio.h"
#include "OhioLib.h"
extern OHIO_STRUCT ohio_info;
#endif

#ifdef CORVETTE
#include "getcorvette.h"
#include "CorvetteLib.h"
extern CORVETTE_STRUCT corvette_info[8];
#endif

DMA_STRUCT local_allocations[10];
int allocation_number = -1;

char *signal_handle;

/******************************************************************************
*
* NAME: print_error
*
* FUNCTION:  prints error information for error debugging.
*
* INPUT PARAMETERS:     adapter_info = general card information.
*
* EXECUTION ENVIRONMENT:  Process only.
*
* RETURN VALUE DESCRIPTION: none.
*
* EXTERNAL PROCEDURES CALLED: printf.
*
******************************************************************************/
void print_error(ADAPTER_STRUCT *adapter, char *error_string)
{

     switch (adapter->environment.error_printing) {

        case NO_PRINTING:       break;
        case FILE_PRINTING:     fprintf(adapter->environment.error_fp,"%s",
                                        error_string);
                                fflush(adapter->environment.error_fp);
        case SCREEN_PRINTING:   fprintf(stderr,"%s",error_string);
                                fflush(stderr);
                                break;
        default:                break;
     }

}

/******************************************************************************
*
* NAME: print_trace
*
* FUNCTION:  prints trace information for error debugging.
*
* INPUT PARAMETERS:     adapter_info = general card information.
*
* EXECUTION ENVIRONMENT:  Process only.
*
* RETURN VALUE DESCRIPTION: none.
*
* EXTERNAL PROCEDURES CALLED: none.
*
******************************************************************************/
void print_trace(ADAPTER_STRUCT *adapter, char *trace_string)
{

     switch (adapter->environment.trace_printing) {

        case NO_PRINTING:       break;
        case FILE_PRINTING:     fprintf(adapter->environment.trace_fp,"%s",
                                        trace_string);
                                fflush(adapter->environment.trace_fp);
                                break;
        case SCREEN_PRINTING:   printf("%s",trace_string);
                                break;
        default:                print_error(adapter,"Error in print_trace\n");
                                break;
     }

}

/******************************************************************************
*
* NAME: get_environment
*
* FUNCTION:  retrieves environment variables settings.
*
* INPUT PARAMETERS:     adapter_info = general card information.
*
* EXECUTION ENVIRONMENT:  Process only.
*
* RETURN VALUE DESCRIPTION: none.
*
* EXTERNAL PROCEDURES CALLED: none.
*
******************************************************************************/
void get_environment(ADAPTER_STRUCT *adapter)
{

     char env_value[100];
     long int current_time, days;
     char time_out[50];
     struct tm *time_format;
     char file_name[100];
     char tmp_string[200];


#ifdef OHIO
     strcpy(adapter->environment.interrupt_routine,getenv("OHIO_INTERRUPT"));
     strcpy(adapter->environment.device_name,"ohio");
#elif CORVETTE
     strcpy(adapter->environment.interrupt_routine,getenv("CORVETTE_INTERRUPT"));
     strcpy(adapter->environment.device_name,"corvette");
#endif

     strcpy(env_value,getenv("ERROR_PRINTING"));
     if ((strcmp(env_value,"ON")==0) || (strcmp(env_value,"SCREEN")==0))
          adapter->environment.error_printing = SCREEN_PRINTING;
     else if (strcmp(env_value,"OFF")==0)
          adapter->environment.error_printing = NO_PRINTING;
     else {
          adapter->environment.error_printing = FILE_PRINTING;
          sprintf(tmp_string,"Opening file %s for error messages\n",env_value);
          adapter->environment.error_fp = fopen(env_value,"w");
          if (adapter->environment.error_fp==NULL) {
               exit(-1);
          }
          print_trace(adapter,tmp_string);
     }

     strcpy(env_value,getenv("TRACE_PRINTING"));
     if ((strcmp(env_value,"ON")==0) || (strcmp(env_value,"SCREEN")==0))
          adapter->environment.trace_printing = SCREEN_PRINTING;
     else if (strcmp(env_value,"OFF")==0)
          adapter->environment.trace_printing = NO_PRINTING;
     else {
          adapter->environment.trace_printing = FILE_PRINTING;
          sprintf(tmp_string,"Opening file %s for trace messages\n",env_value);
          adapter->environment.trace_fp = fopen(env_value,"w");
          if (adapter->environment.trace_fp==NULL) {
               print_error(adapter,"Unable to open trace file.\n");
               exit(-1);
          }
          print_trace(adapter,tmp_string);
     }

     strcpy(env_value,getenv("TRACE_FORMAT"));
     if (strcmp(env_value,"VERBOSE")==0)
          adapter->environment.trace_format=VERBOSE;
     else
          adapter->environment.trace_format=CRYPTIC;

     strcpy(env_value,getenv("ERROR_FORMAT"));
     if (strcmp(env_value,"VERBOSE")==0)
          adapter->environment.error_format=VERBOSE;
     else
          adapter->environment.error_format=CRYPTIC;

     strcpy(env_value,getenv("TESTCASE_RECORDING"));
     if ((strcmp(env_value,"ON")==0) || (strcmp(env_value,"SCREEN")==0))
          adapter->environment.record_printing = SCREEN_PRINTING;
     else if (strcmp(env_value,"OFF")==0)
          adapter->environment.record_printing = NO_PRINTING;
     else if (strcmp(env_value,"DEFAULT_DATE")==0) {
          current_time = time(0);
          time_format = localtime(&current_time);
          strftime(time_out,50,"%m%d%y.raw",time_format);

          strcpy(file_name,getenv("TEST_MOUNT_DIR"));
          strcat(file_name,"/reports/raw/");
          strcat(file_name,adapter->environment.device_name);
          strcat(file_name,"/");
          strcat(file_name,time_out);

          adapter->environment.record_printing = FILE_PRINTING;
          sprintf(tmp_string,"Opening file %s for record messages\n",file_name);
          print_trace(adapter,tmp_string);
          adapter->environment.record_fp = fopen(file_name,"a");
          if (adapter->environment.record_fp==NULL) {
               fprintf(stderr,"Unable to open record file.\n");
               exit(-1);
          }
     }
     else {
          adapter->environment.record_printing = FILE_PRINTING;
          printf("Opening file %s for record messages\n",env_value);
          adapter->environment.record_fp = fopen(env_value,"w");
          if (adapter->environment.record_fp==NULL) {
               print_error(adapter,"Unable to open record file.\n");
               exit(-1);
          }
     }

     strcpy(env_value,getenv("TIMEOUT_VALUE"));
     adapter->environment.timeout_value = atoi(env_value);
     sprintf(tmp_string,"timeout value: %d\n",adapter->environment.timeout_value);
     print_trace(adapter, tmp_string);
}

/******************************************************************************
*
* NAME: dma_free
*
* FUNCTION:  frees storage space on page boundaries.
*
* INPUT PARAMETERS:     adapter_info = general card information.
*                       allocated_space = space previously created with dma_allocate.
*
* EXECUTION ENVIRONMENT:  Process only.
*
* RETURN VALUE DESCRIPTION: none.
*
* EXTERNAL PROCEDURES CALLED: none.
*
******************************************************************************/
void dma_free(ADAPTER_STRUCT *adapter, DMA_STRUCT allocated_space)
{

     int rc;

     allocation_number--;
     rc = diag_dma_complete(adapter->handle, allocated_space.dma_page);

     free(allocated_space.user_address);

}

/******************************************************************************
*
* NAME:  signal_close
*
* FUNCTION:  Closes adapter connection following user kill signal
*
* INPUT PARAMETERS:     none.
*
* EXECUTION ENVIRONMENT:  Process only.
*
* RETURN VALUE DESCRIPTION: none.
*
* EXTERNAL PROCEDURES CALLED: cfgad, getad, diag_open, post_error,
*                             initialize_adapter, malloc.
*
******************************************************************************/
void signal_close()
{
    struct cfg_load cfg_ld;
    ADAPTER_STRUCT *adapter_info;
    int free_loop;

    usleep(300);
    adapter_info = (ADAPTER_STRUCT *)signal_handle;

    if (adapter_info->environment.trace_printing == FILE_PRINTING)
       fclose(adapter_info->environment.trace_fp);
    if (adapter_info->environment.error_printing == FILE_PRINTING)
       fclose(adapter_info->environment.error_fp);
    if (adapter_info->environment.record_printing == FILE_PRINTING)
       fclose(adapter_info->environment.record_fp);

    /* Free all allocated microchannel memory */
    if (allocation_number >= 0)
         for (free_loop=0; free_loop<=allocation_number; free_loop++)
              dma_free(adapter_info, local_allocations[free_loop]);

    diag_close(adapter_info->handle);
    /***************************************************
      Unload kernel extentions interrupt handler.
     ***************************************************/

    cfg_ld.path = adapter_info->environment.interrupt_routine;
    if (sysconfig(SYS_QUERYLOAD, (void *)&cfg_ld, (int)sizeof(cfg_ld)))
       print_error(adapter_info,"SYSCONFIG QUERYLOAD");
    if (!cfg_ld.kmid) print_error(adapter_info,"Interrupt is not loaded\n");
    if (sysconfig(SYS_KULOAD, (void *)&cfg_ld, (int)sizeof(cfg_ld)))
       print_error(adapter_info,"SYSCONFIG KULOAD");


    free(adapter_info);
    exit();
}

/******************************************************************************
*
* NAME:  setsig
*
* FUNCTION:  initializes a signal if defaulted.
*
* INPUT PARAMETERS:     none.
*
* EXECUTION ENVIRONMENT:  Process only.
*
* RETURN VALUE DESCRIPTION: none
*
* EXTERNAL PROCEDURES CALLED: setsig
*
******************************************************************************/
int setsig(ADAPTER_STRUCT *adapter_info, int sig, void (*fcn)())
{
    switch((int)signal(sig, SIG_IGN)) {
       case ((int)BADSIG):
            print_error(adapter_info,"Bad Signal");
            return BADSIG;
       case ((int)SIG_IGN):
            return;
       case ((int)SIG_DFL):
            break;
       default:
            print_error(adapter_info,"signal already caught!");
    }
    if (signal(sig, fcn)==BADSIG) {
         print_error(adapter_info,"signal error");
                return BADSIG;
        }
}



/******************************************************************************
*
* NAME:  catchsigs
*
* FUNCTION:  initializes signal processes to capture program exit.
*
* INPUT PARAMETERS:     none.
*
* EXECUTION ENVIRONMENT:  Process only.
*
* RETURN VALUE DESCRIPTION: none
*
* EXTERNAL PROCEDURES CALLED: setsig
*
******************************************************************************/
int catchsigs(char *adapter_handle)
{
        int rc;

    signal_handle = adapter_handle;
    if ((rc = setsig(adapter_handle, SIGHUP, signal_close))==-1)
                return rc;
    if ((rc = setsig(adapter_handle, SIGINT, signal_close))==-1)
                return rc;
    if ((rc = setsig(adapter_handle, SIGQUIT, signal_close))==-1)
                return rc;
    if ((rc = setsig(adapter_handle, SIGTERM, signal_close))==-1)
                return rc;

}

/******************************************************************************
*
* NAME: record_testcase
*
* FUNCTION:  records a testcase completion status.
*
* INPUT PARAMETERS:     testcase_name = testcase variation name.
*                       completion_status = status of testcase variation.
*
* EXECUTION ENVIRONMENT:  Process only.
*
* RETURN VALUE DESCRIPTION: none.
*
* EXTERNAL PROCEDURES CALLED: none.
*
******************************************************************************/
void record_testcase(ADAPTER_STRUCT *adapter, char *testcase_name, int completion_status)
{

     long int current_time, days;
     char time_out[50];
     struct tm *time_format;
     char record_string[100];

     current_time = time(0);
     time_format = localtime(&current_time);
     strftime(time_out,50,"%D %T",time_format);

     days = (current_time / (60*60*24)) - (365*23) - 5;

     if (completion_status == 0)
          sprintf(record_string,"Testcase %s completed %s (%ld) successfully.\n",testcase_name,time_out,days);
     else
          sprintf(record_string,"Testcase %s completed %s (%ld) with failure.\n",testcase_name,time_out,days);

     switch (adapter->environment.record_printing) {

        case NO_PRINTING:       break;
        case FILE_PRINTING:     fprintf(adapter->environment.record_fp,"%s",
                                        record_string);
                                fflush(adapter->environment.record_fp);
        case SCREEN_PRINTING:   printf("%s",record_string);
                                fflush(stdout);
                                break;
        default:                break;
     }

}

/******************************************************************************
*
* NAME: dma_flush
*
* FUNCTION:  flushes user memory out to microchannel memory.
*
* INPUT PARAMETERS:     adapter_info = general card information.
*                       allocated_space = space previously created with dma_allocate.
*
* EXECUTION ENVIRONMENT:  Process only.
*
* RETURN VALUE DESCRIPTION: none.
*
* EXTERNAL PROCEDURES CALLED: none.
*
******************************************************************************/
void dma_flush(ADAPTER_STRUCT *adapter, DMA_STRUCT allocated_space)
{

     int rc;

     rc = diag_dma_flush(adapter->handle, allocated_space.dma_page);


}

/******************************************************************************
*
* NAME: dma_allocate
*
* FUNCTION:  Allocates storage space on page boundaries.
*
* INPUT PARAMETERS:     adapter_info = general card information.
*                       page_size = number of pages needed to be allocated.
*
* EXECUTION ENVIRONMENT:  Process only.
*
* RETURN VALUE DESCRIPTION: none.
*
* EXTERNAL PROCEDURES CALLED: none.
*
******************************************************************************/
DMA_STRUCT dma_allocate(ADAPTER_STRUCT *adapter, int page_size)
{

     DMA_STRUCT allocated_space;
     int loop;
     int rc;
     char msg_string[100];

#ifdef OHIO
     ohio_dds *dds;
     dds = (ohio_dds *)adapter->dds;
#elif CORVETTE
     corvette_dds *dds;
     dds = (corvette_dds *)adapter->dds;
#endif

     if (page_size == 0)
          page_size++;

     allocated_space.user_address = (char *)malloc( (page_size+1)*PAGESIZE );
     allocated_space.page_address = ((unsigned long)allocated_space.user_address & 0xfffff000) + PAGESIZE;

     for(loop=0; loop<page_size*PAGESIZE; loop++)
          allocated_space.page_address[loop] = '\xff';

#ifdef DIAGEX10
     allocated_space.dma_page = diag_dma_master(adapter->handle, DMA_READ | DMA_NOHIDE,
                                                allocated_space.page_address, page_size*PAGESIZE);

     allocated_space.dma_address = /* dds->dma_bus_mem + */ allocated_space.dma_page;
#else
     rc = diag_dma_master(adapter->handle, DMA_READ | DMA_NOHIDE,
          allocated_space.page_address, page_size*PAGESIZE,
          &(allocated_space.dma_page));

     allocated_space.dma_address =  /* dds->dma_bus_mem + */ allocated_space.dma_page;
     sprintf(msg_string,"diag_dma_master : rc = %d\n",rc);
     print_trace(adapter, msg_string);
#endif

     sprintf(msg_string,"dma_handle : %x\n",allocated_space.dma_page);
     print_trace(adapter, msg_string);

     if (allocated_space.dma_page == -1)
          print_error(adapter,"Unable to allocated dma page!\n");

     /* Record dma structure for freeing memory during program interrupt */
     allocation_number++;
     local_allocations[allocation_number] = allocated_space;

     dma_flush(adapter, allocated_space);

     return allocated_space;

}

/******************************************************************************
*
* NAME:  display_adapter_data
*
* FUNCTION:  prints adapter specific information.
*
* INPUT PARAMETERS:     adapter_info = general card information.
*
* EXECUTION ENVIRONMENT:  Process only.
*
* RETURN VALUE DESCRIPTION: none.
*
* EXTERNAL PROCEDURES CALLED: printf
*
******************************************************************************/
void display_adapter_data(ADAPTER_STRUCT *adapter_info)
{
     uchar *adapter_specific_data;
     int adapter_data_length;
     int byte_pos;

     adapter_specific_data = (uchar *)adapter_info->adapter_specific_info;
     adapter_data_length = adapter_info->adapter_specific_info_length;

     printf("addr  0 1 2 3  4 5 6 7  8 9 A B  C D E F");
     for(byte_pos=0; byte_pos<adapter_data_length; byte_pos++) {

          if (!(byte_pos %16))
               printf("\n%04x",byte_pos);

          if (!(byte_pos % 4))
               printf(" ");

          printf("%02x",adapter_specific_data[byte_pos]);

     }
     printf("\n\n");
}

/******************************************************************************
*
* NAME:  post_error
*
* FUNCTION:  displays error and logs in adapter_info
*
* INPUT PARAMETERS:     error = error to be logged.
*                       adapter_info = general card information.
*
* EXECUTION ENVIRONMENT:  Process only.
*
* RETURN VALUE DESCRIPTION: none.
*
* EXTERNAL PROCEDURES CALLED: printf
*
******************************************************************************/
void post_error( int error, ADAPTER_STRUCT *adapter_info )
{

     adapter_info->error_log[adapter_info->num_errors] = error;
     adapter_info->num_errors++;

     print_error(adapter_info,"error!\n");
}

/******************************************************************************
*
* NAME:  get_card_id
*
* FUNCTION:  Retrieves Microchannel Card ID from POS 0 and 1.
*
* INPUT PARAMETERS:     adapter_info = general card information.
*
* EXECUTION ENVIRONMENT:  Process only.
*
* RETURN VALUE DESCRIPTION: if successful returns ID of card else 0
*
* EXTERNAL PROCEDURES CALLED: reg_read8
*
******************************************************************************/
unsigned int get_card_id(ADAPTER_STRUCT *adapter_info)
{

     unsigned char card_id_high;
     unsigned char card_id_low;
     unsigned int card_id;
     char tmp_string[20];

     /* Read Pos Register 0 for upper card id byte */
     errno = 0;

#ifdef DIAGEX10
     card_id_high = diag_pos_read(adapter_info->handle, 1);
#else
     diag_pos_read(adapter_info->handle, 1, &card_id_high, NULL, PROCLEV);
#endif

     if (errno != 0) {
           post_error(errno, adapter_info);
           return 0;
     }

     /* Read Pos Register 1 for lower card id byte */
#ifdef DIAGEX10
     card_id_low = diag_pos_read(adapter_info->handle, 0);
#elif DIAGEX20
     diag_pos_read(adapter_info->handle, 0, &card_id_low, NULL, PROCLEV);
#endif

     if (errno != 0) {
           post_error(errno, adapter_info);
           return 0;
     }

     card_id = (card_id_high << 8) + card_id_low;

     sprintf(tmp_string,"POS ID: %04x\n",card_id);

     return card_id;

}


/******************************************************************************
*
* NAME:  initialize_adapter_info
*
* FUNCTION:  Sets adapter card id and checks for enable timeout
*
* INPUT PARAMETERS:     adapter_info = general card information
*
* EXECUTION ENVIRONMENT:  Process only.
*
* RETURN VALUE DESCRIPTION: if successful returns 0 of card elsE
*                           ADAPTER_TIMEOUT_ERROR.
*
* EXTERNAL PROCEDURES CALLED: reg_read8, adapter_enabled
*
******************************************************************************/
int initialize_adapter_info(ADAPTER_STRUCT *adapter_info)
{

     int timeout_length = 10000;

     adapter_info->card_id = 0;

     while (timeout_length-- && (adapter_info->card_id == 0))
          adapter_info->card_id = get_card_id(adapter_info);

     if (timeout_length==0)
         return ADAPTER_TIMEOUT_ERROR;
     else
         return 0;

}

/******************************************************************************
*
* NAME:  adapter_open
*
* FUNCTION:  Initialize an adapter to be used with diag_ex and
*            verify its usability.
*
* INPUT PARAMETERS:     adapter_name = device name of adapter.
*
* EXECUTION ENVIRONMENT:  Process only.
*
* RETURN VALUE DESCRIPTION: if successful returns address of handle else NULL
*
* EXTERNAL PROCEDURES CALLED: cfgad, getad, diag_open, post_error,
*                             initialize_adapter, malloc.
*
******************************************************************************/

void *adapter_open(char *adapter_name)
{
#ifdef DIAGEX10
    diagex_dds diag_dds;
#elif DIAGEX20
    diagex_dds_t diag_dds;
#endif


    char *adapter_handle;
    int adapter_error;
    ADAPTER_STRUCT *adapter_info;
    struct cfg_load cfg_ld;
    int rc, error_count, i;
    char msg_string[100];

#ifdef OHIO
    ohio_dds *dds;
#elif CORVETTE
    corvette_dds *dds;
#endif

    /* allocate space for adapter configuration */
    adapter_info = (ADAPTER_STRUCT *) malloc(sizeof(ADAPTER_STRUCT));
    if (adapter_info == NULL ) {
         print_error(adapter_info, "Unable to allocate storage for data structure");
         return(0);
    }

    /* Retrive environment settings for adapter */
    get_environment(adapter_info);

    adapter_info->num_errors = 0;

    rc = 1;
    error_count = 5;
    while ((error_count != 0) && (rc != 0)) {
#ifdef OHIO
         rc = getohio(adapter_name, &dds);
#elif CORVETTE
         rc = getcorvette(adapter_name, &dds);
#endif
         error_count--;
         if (rc != 0) sleep(5);
    }

    if (rc != 0) {
         sprintf(msg_string, "Unable to get ODM info: %d\n", rc);
         print_error(adapter_info, msg_string);
         return(0);
    }

    print_trace(adapter_info,"Completed getad\n");
    adapter_info->dds = (char *)dds;

#ifdef DIAGEX20
    strcpy(diag_dds.device_name, dds->adpt_name);
    strcpy(diag_dds.parent_name, dds->par_name);
    diag_dds.maxmaster = 20;
#endif
    diag_dds.slot_num = dds->slot_num;
    diag_dds.bus_intr_lvl   = dds->bus_intr_lvl;
    diag_dds.intr_priority = dds->intr_priority;
    diag_dds.intr_flags = dds->intr_flags;
    diag_dds.dma_lvl = dds->dma_lvl;
    diag_dds.bus_io_addr = dds->bus_io_addr;
    diag_dds.bus_io_length = dds->bus_io_length;
    diag_dds.bus_mem_addr = dds->bus_mem_addr;
    diag_dds.bus_mem_length = dds->bus_mem_length;
    diag_dds.dma_bus_mem = dds->dma_bus_mem;
    diag_dds.dma_bus_length = dds->dma_bus_length;
#ifdef DIAGNOSTICS
    diag_dds.dma_bus_length=0x110000;
#endif
    diag_dds.bus_id = dds->bus_id;
    diag_dds.bus_type = dds->bus_type;
    diag_dds.intr_flags = dds->intr_flags;
    diag_dds.dma_flags = MICRO_CHANNEL_DMA;

    /***************************************************
      Load kernel extentions if not already loaded.
      This code then passes the DDS to the kernel extension.
     ***************************************************/

    cfg_ld.path = adapter_info->environment.interrupt_routine;
    cfg_ld.libpath = NULL;
    if (rc=sysconfig(SYS_KLOAD, (void *)&cfg_ld, (int)sizeof(cfg_ld))) {
        print_error(adapter_info, "Unable to load interrupt routine\n");
        return(0);
    }

    diag_dds.kmid = cfg_ld.kmid;

#ifdef OHIO
    diag_dds.data_ptr = (char *)&ohio_info;
    diag_dds.d_count = sizeof(OHIO_STRUCT);
#elif CORVETTE
    diag_dds.data_ptr = (char *)&corvette_info;
 /* diag_dds.d_count = sizeof(CORVETTE_STRUCT); */
    diag_dds.d_count = 8;
    for (i=0; i<8; i++) corvette_info[i] = 0x00; /* clear intr buff */
#endif

#ifdef DIAGEX10
    adapter_handle = (char *)diag_open(&diag_dds);
#elif DIAGEX20
    rc = diag_open(&diag_dds, &adapter_handle);
#endif

    if (
#ifdef DIAGEX10
(adapter_handle == -1)
#elif DIAGEX20
(rc != DGX_OK)
#endif
) {
         print_error(adapter_info,"Unable to open adapter\n");
         sprintf(msg_string,"Error code = %d\n",errno);
         print_error(adapter_info, msg_string);
         return(0);
    }
    print_trace(adapter_info,"Completed diag_open\n");
    adapter_info->handle = adapter_handle;
    sprintf(msg_string,"diag handle: %x\n",adapter_handle);
    print_trace(adapter_info,msg_string);

    adapter_error =initialize_adapter_info(adapter_info);
    print_trace(adapter_info,"Completed initialize_adapter_info\n");



    return adapter_info;
}

/******************************************************************************
*
* NAME:  adapter_close
*
* FUNCTION:  Initialize an adapter to be used with diag_ex and
*            verify its usability.
*
* INPUT PARAMETERS:     adapter_name = device name of adapter.
*
* EXECUTION ENVIRONMENT:  Process only.
*
* RETURN VALUE DESCRIPTION: if successful returns address of handle else NULL
*
* EXTERNAL PROCEDURES CALLED: cfgad, getad, diag_open, post_error,
*                             initialize_adapter, malloc.
*
******************************************************************************/
int adapter_close(ADAPTER_STRUCT *adapter_info)
{
    struct cfg_load cfg_ld;
    int rc;

#ifdef DIAGEX10
    if ((rc = diag_close(adapter_info->handle))==-1)
                return -1;
#elif DIAGEX20
    rc = diag_close(adapter_info->handle);
    if (rc != DGX_OK)
          return -1;
#endif

    /***************************************************
      Unload kernel extentions interrupt handler.
     ***************************************************/

    cfg_ld.path = adapter_info->environment.interrupt_routine;
    cfg_ld.libpath = NULL;
    if (sysconfig(SYS_QUERYLOAD, (void *)&cfg_ld, (int)sizeof(cfg_ld))) {
        print_error(adapter_info,"SYSCONFIG QUERYLOAD");
                return -1;
        }
    if (!cfg_ld.kmid) {
                print_error(adapter_info,"Interrupt is not loaded\n");
                return -1;
        }
    if (sysconfig(SYS_KULOAD, (void *)&cfg_ld, (int)sizeof(cfg_ld))) {
         print_error(adapter_info,"SYSCONFIG KULOAD");
                return -1;
        }
    if (adapter_info->environment.trace_printing == FILE_PRINTING)
       fclose(adapter_info->environment.trace_fp);
    if (adapter_info->environment.error_printing == FILE_PRINTING)
       fclose(adapter_info->environment.error_fp);
    if (adapter_info->environment.record_printing == FILE_PRINTING)
       fclose(adapter_info->environment.record_fp);

    free(adapter_info);

    return 0;
}
