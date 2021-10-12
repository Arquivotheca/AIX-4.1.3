static char sccsid[] = "@(#)40  1.19.1.4 src/bos/kernext/c327/tcaopen.c, sysxc327, bos411, 9430C411a 7/27/94 09:34:12";
/*
 * COMPONENT_NAME: (SYSXC327) c327 tca device driver entry points
 *
 * FUNCTIONS:    beginSession(), checkOpenParams(), endSession(),
 *               setupSession(), tcaopen()              
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1990
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

/*
** INCLUDE FILES
*/
#include <sys/types.h>
#include <sys/sysmacros.h>
#include <sys/malloc.h>
#include <sys/sleep.h>
#include <sys/errno.h>
#include <sys/intr.h>
#include <fcntl.h>
#include <sys/io3270.h>
#include "c327dd.h"
#include "dftnsDcl.h"
#include "tcadefs.h"
#include "tcadecls.h"
#include "tcaexterns.h"

#ifdef _POWER_MP
#include <sys/lock_def.h> 
extern Simple_lock c327_intr_lock;
extern Complex_lock c327_lock;
#else
extern lock_t c327_lock;
#endif

/*
** static function prototypes
*/
static int setupSession (int, int);
static int beginSession (int, int);
static int endSession (int, int);
static int checkOpenParams (int, int);

void dftnsProcessInterrupt(DDS_DATA *);

/*
** static variables
*/
static char     local_wait_to_open = 0;
static int      sleep_local_wait_to_open = EVENT_NULL;
/*PAGE*/
/*******************************************************************
**
** Function Name:       tcaopen
**
** Description: establish a physical link between the AIX
**              application and the host and verify that the
**              security of the link address has not been
**              violated
**
** Inputs:      dev             device minor number
**              flag            indicates opening option
**              laNum           session number
**              dds_ptr         pointer to dds structure
**
** Outputs:     0               operation was successful
**              errno value     operation not successful
**
** Externals    tca_sess_type
** Referenced 
**
** Externals  
** Modified  
**
*******************************************************************/
int tcaopen (dev_t devt, int flag, int laNum, DDS_DATA *dds_ptr)
{
   int dev;

   dev = minor(devt);

   /* ------------------------------------------- */
   /* declare pointer to device data structure    */
   /* ------------------------------------------- */
   tca_data[dev].cd_ddsPtr = (DDS_DATA *)dds_ptr;
   C327TRACE5("OpnS", dev, flag, laNum, tca_data[dev].cd_ddsPtr);
   
   /* -------------------------------------------   */
   /* check for errors in the open parameters       */
   /* if failure, something wrong w/open parameters */
   /* -------------------------------------------   */
   if(checkOpenParams(dev, flag)){
      return(EINVAL);
   }
   
   /* ----------------------------------------------------- */
   /* allocate and initialize link address structure */
   /* if failure, something is wrong with memory allocation */
   /* ----------------------------------------------------- */
   if(setupSession(dev, laNum)){
      return(ENOMEM);
   }
   
   /* ------------------------------------------- */
   /* if failure, something wrong w/begin session */
   /* ------------------------------------------- */
   if(beginSession(dev, laNum)){
      endSession(dev, laNum);
      return(EIO);
   }

   /* ------------------------------------------- */
   /* clear the open path */
   /* ------------------------------------------- */
   C327TRACE3("OpnE",dev,laNum);

   return( SUCCESS );
}
/*PAGE*/
/*******************************************************************
** Function Name: 
**
** Description: 
**             
** Inputs:   
**
** Output: 
**        
** Externals 
** Modified
********************************************************************/
static int setupSession (int dev, int laNum)
{
   linkAddr       *laP;

   /* ---------------------------------------------------------- */
   /* now that we know the link address, see if there is already */
   /* an existing link structure for this link address...        */
   /* ---------------------------------------------------------- */
   laP = tca_data[dev].mlnk_ptrs[laNum];

   /* ------------------------------------------------------- */
   /* get the memory for the link address structure if needed */
   /* ------------------------------------------------------- */
   if(laP == NULL){
      laP = (linkAddr *) xmalloc((uint)sizeof(linkAddr),
                                 (uint)3, pinned_heap);
      /* ----------------------- */
      /* if the malloc failed... */
      /* ----------------------- */
      if(laP == NULL){
         C327TRACE5("Bop3",dev,laNum,laP,ENOMEM);
         return(FAILURE);
      }

      /* -------------------------------------- */
      /* initialize link address struct to zero */
      /* -------------------------------------- */
      bzero((void *)laP,sizeof(linkAddr));

      /* ----------------------------------------------- */
      /* initialize all system sleep flags to EVENT_NULL */
      /* ----------------------------------------------- */
      laP->sleep_link_state = EVENT_NULL;
      laP->sleep_waiting_on_io_in_prog = EVENT_NULL;
      laP->sleep_la_waitClear = EVENT_NULL;
      laP->sleep_waiting_for_data = EVENT_NULL;
      laP->sleep_waiting_for_unmlocked = EVENT_NULL;
      laP->sleep_waiting_for_write_buf = EVENT_NULL;


      setLinkState(laP, LS_LINKDOWN);
      laP->writeBuffer = laP->la_WSFdbP = NULL;
      laP->writeBuffer_used = TRUE;

      /* ------------------------------------------- */
      /* allocate receive data buffer, to be used to */
      /* move data from card to kernel area          */
      /* ------------------------------------------- */
      laP->la_recvDbP =
             bufferAlloc(dev, laP->la_recvDbP, (struct uio *)NULL);
      /* ----------------------- */
      /* if the malloc failed... */
      /* ----------------------- */
      if(laP->la_recvDbP == NULL){
         C327TRACE5("Bop5",dev,laNum,laP,ENOMEM);
         return(FAILURE);
      }

      /* ----------------------------*/
      /* set up type of session info */
      /* ----------------------------*/
      laP->la_sess_type = tca_sess_type;
      laP->la_printerAddr = printerAddr;

      /* ----------------------------------------------- */
      /* Structure is initialized, ok for others to use. */
      /* ----------------------------------------------- */
      tca_data[dev].mlnk_ptrs[laNum] = laP;
   }
/*
** !!! assumes that a close can come through after 
** first open and before the second
*/
   laP->num_processes++;   /* indicate we're on this session */

   C327TRACE5("Sop0",laP,laNum,laP->num_processes,laP->la_sess_type);
   return( SUCCESS );
}
/*PAGE*/
/*******************************************************************
** Function Name: 
**
** Description: 
**             
** Inputs:   
**
** Output: 
**        
** Externals 
** Modified
********************************************************************/


static int beginSession (int dev, int laNum)
{
   int      didOpenPort;
   int      plX;
   linkAddr *laP;

   laP = tca_data[dev].mlnk_ptrs[laNum];

   /*
   ** if we haven't opened the port yet, open it
   ** and remember we did the open here
   */
   didOpenPort = 0;
   if (tca_data[dev].open_first_time != 1){
      if (mdepOpenPort(dev) == MDEP_SUCCESS){
	/* XXX - necessary? */
         DISABLE_INTERRUPTS(plX);
         tca_data[dev].open_first_time = 1;
         tca_data[dev].sleep_open_link_address = EVENT_NULL;
         RESTORE_INTERRUPTS(plX);
         didOpenPort = 1;
      }
      else{
          mfree_la_struct(laNum, laP, dev);
          return(EIO);
      }
   }

   /* -------------------------------------------- */
   /* If the link state is not LS_LINKDOWN then    */
   /* link has previouly been opened, open() done  */
   /* -------------------------------------------- */
   if (getLinkState(laP) != LS_LINKDOWN){
      C327TRACE3("Gop0",getLinkState(laP), MDEP_SUCCESS);
      return( SUCCESS );
   }

   /* -------------------------------------------- */
   /* issue start device on link address - if the  */
   /* mdepStartLA() failed or if the link state is */
   /* still LS_LINKDOWN, an error happened. */
   /* -------------------------------------------- */
   if (mdepStartLA(laP, laNum, Open, dev, &c327_lock) == MDEP_FAILURE ||
           getLinkState(laP) == LS_LINKDOWN){
      mfree_la_struct(laNum, laP, dev);       /* cleanup */
      if(didOpenPort) {       /* if we opened the port here */
         mdepClosePort(laNum, dev);      /* close it */
         C327TRACE3("upbs",1, dftnsProcessInterrupt);
      }
      C327TRACE5("Bop6",dev,laNum,laP,getLinkState(laP));
      return( FAILURE );      /* something wrong w/start */
   }

   C327TRACE2("Gop1",MDEP_SUCCESS);
   return( SUCCESS );
}
/*PAGE*/
/*******************************************************************
** Function Name: 
**
** Description: 
**             
** Inputs:   
**
** Output: 
**        
** Externals 
** Modified
********************************************************************/
static int endSession (int dev, int laNum)
{
   /* ---------------------------------- */
   /* see if there are any open sessions */
   /* if none, close the port */
   /* ---------------------------------- */
   if(tca_data[dev].open_first_time &&
            mscan_link_ptrs(dev)){
      mdepClosePort(laNum, dev);
   }
}
/*PAGE*/
/*******************************************************************
**
** Function Name:       checkOpenParams()
**
** Description: checks the parameters sent to open, performing the
**              following tests:
**                      - checks the open flags for validity
**
** Inputs:      dev     device minor number
**              flag    open flags
**
** Output:      1       open parameters OK
**              0       open parameters bad
**
** Externals    tca_DDS
**
********************************************************************/
static int checkOpenParams (int dev, int flag)
{
   /* ------------------------------ */
   /* make sure the flag are correct */
   /* ------------------------------ */
   if(flag == O_RDONLY || flag == O_WRONLY || flag == O_NDELAY ||
                          flag == O_APPEND || flag == O_EXCL){
      C327TRACE4("Bop2",dev,EINVAL,flag);
      return( FAILURE );
   }

   return( SUCCESS );
}
/*PAGE*/
